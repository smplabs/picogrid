import io
import os
import json
import csv
import pytz
import shutil
import re
import requests
import config
from datetime import datetime, timedelta
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure
from flask import Flask, render_template, request, redirect, session, url_for, Response
from azure.identity import DefaultAzureCredential, InteractiveBrowserCredential, ManagedIdentityCredential
from azure.storage.blob import BlobServiceClient, BlobClient, ContainerClient
from azure.keyvault.secrets import SecretClient

# Variables to send thresholds to Particle Devices
ACCESS_TOKEN = config.ACCESS_TOKEN
FUNCTION_TO_ACCESS = config.FUNCTION_TO_ACCESS

# Creating an instance of the Flask class (this is how Flask works)
app = Flask(__name__)

# Secret key used in sessions (signing a session cookie - transferring user data between parts of app)
Flask.secret_key = config.FLASK_SECRET_KEY

# Credentials for Azure Blob Storage
account_url = config.STORAGE_ACCOUNT_URL
container_name = config.CONTAINER_NAME

# TODO: choose which of the 3 options depending on use case
default_credential = DefaultAzureCredential() # Uses "az login" in terminal
# default_credential = InteractiveBrowserCredential() # Uses a browser client - good when running locally
# default_credential = ManagedIdentityCredential() # Uses Azure App service instance's credentials (will need to add Managed Identity to accessing blobs + vault)

# Picogrid board mapping
picoboards = {
    # TODO: add mappings of name to device ID (add all the devices that are present)
    }

""" 
    Function to get Azure Blob entries within a date range 

    Inputs:
        start_dt: datetime object with start time of data retrieval
        end_dt: datetime object with end time of data retrieval

    Output:
        All data as JSON list which falls within range of dates input
"""
def getBlobEntriesWithinTimeRange(start_dt, end_dt):
    # Create the BlobServiceClient object
    blob_service_client = BlobServiceClient(account_url, credential=default_credential)
    # Create the ContainerClient
    container_client = blob_service_client.get_container_client(container_name)
    # Get all the blobs associated with the container
    blob_list = container_client.list_blobs()

    blob_data_all = []
    for blob in blob_list:
        # Only consider JSON files (Azure may comtain some misc AVRO files)
        if blob.name.__contains__("json"):            
            
            # Parsing the way below because data stored in Azure Blob Storage in this format: 
            # {iothub}/{partition}/{YYYY}/{MM}/{DD}/{HH}/{mm}.json

            # Obtain all the date values (year, month, day, hour)
            split_name = blob.name.split("/")[2:6]
            split_name.append(blob.name.split("/")[6][0:2]) # Add the minute too
            split_name = [ int(i) for i in split_name ]
            
            # Make a new datetime from the blob folder structure - Azure Blob storage stores in UTC
            blob_dt = datetime(split_name[0], split_name[1], split_name[2], split_name[3], split_name[4], tzinfo=pytz.UTC)
            
            # Take blobs stored between the date range
            if blob_dt >= start_dt and blob_dt <= end_dt:
                # Get blob client - needed to download blob data
                blob_client = container_client.get_blob_client(blob.name)
                blob_data = blob_client.download_blob()
                
                # Each blob also has multiple lines - need to process each line seperately for python JSON parser to work
                temp_blob_data = blob_data.readall().decode("utf-8").splitlines()
                for line in temp_blob_data: 
                    blob_data_all.append(line)
    
    return blob_data_all

# Flask routes are URLs - determines which URL should trigger which function
@app.route("/")
def main():
    # Delete all old files in the directory for save space + cleanup
    # NOTE: this method only works when the git repo with static/files has a .gitkeep within it (empty folder is pushed to git)
    folder = 'static/files'
    for filename in os.listdir(folder):
        file_path = os.path.join(folder, filename)
        try:
            if os.path.isfile(file_path) or os.path.islink(file_path):
                os.unlink(file_path)
            elif os.path.isdir(file_path):
                shutil.rmtree(file_path)
        except Exception as e:
            print('Failed to delete %s. Reason: %s' % (file_path, e))
    # Returns the HTML template for the main page
    return render_template('index.html')

@app.route('/data')
def data():
    return render_template('retrieval_page.html')

@app.route('/thresholds')
def thresholds():
    return render_template('thresholds_page.html')

# Route for submitting the date input form
@app.route('/submit-datetimes', methods=['POST'])
def submit_datetimes():
    start_year = int(request.form['startYear'])
    start_month = int(request.form['startMonth'])
    start_day = int(request.form['startDay'])
    start_hour = int(request.form['startHour'])
    start_minute = int(request.form['startMinute'])
    end_year = int(request.form['endYear'])
    end_month = int(request.form['endMonth'])
    end_day = int(request.form['endDay'])
    end_hour = int(request.form['endHour'])
    end_minute = int(request.form['endMinute'])

    # Validate date inputs
    try:
        start_dt = datetime(start_year, start_month, start_day, start_hour, start_minute, tzinfo=pytz.timezone("America/New_York"))
        end_dt = datetime(end_year, end_month, end_day, end_hour, end_minute, tzinfo=pytz.timezone("America/New_York"))
        
        # Check that end date is after start date
        if (start_dt >= end_dt):
            raise ValueError
        
        # Check that dates are within a week of each other
        diff = abs(end_dt - start_dt)
        week = timedelta(days=7)
        if diff > week:
            raise ValueError

        # Save to sessions for future retrival in other Flask routes
        session['startYear'] = start_year
        session['startMonth'] = start_month
        session['startDay'] = start_day
        session['startHour'] = start_hour
        session['startMinute'] = start_minute
        session['endYear'] = end_year
        session['endMonth'] = end_month
        session['endDay'] = end_day
        session['endHour'] = end_hour
        session['endMinute'] = end_minute

        # Succeeded - move to next step
        return render_template('retrieval_page.html', datetime_input_success=True, datetime_input_failure=False)
    
    # Validation failed
    except (ValueError, TypeError) as e:
        print(str(e))
        return render_template('retrieval_page.html', datetime_input_success=False, datetime_input_failure=True)
    
# Route for picoboard input form
@app.route('/submit_chosen_picoboard', methods=['POST'])
def submit_picoboard_chosen():
    picoboard_chosen = request.form.get("picoboard")
    session['picoboard'] = picoboard_chosen
    return render_template('retrieval_page.html', datetime_input_success=True, picoboard_chosen=True, picoboard_variable=picoboard_chosen)

# Route for submitting the variable input form
@app.route('/submit_variable_to_plot', methods=['POST'])
def submit_variables_to_plot():
    # Get variable chosen from dropdown
    chosen_var = request.form.get("variables")
    # Parse user date input data (from session) into datetimes
    # NOTE: the timezones need to be updated in the lines below (may need to finetune based on DST)
    start_dt = datetime(session["startYear"], session["startMonth"], session["startDay"], session["startHour"], session["startMinute"], tzinfo=pytz.timezone("America/New_York"))
    end_dt = datetime(session["endYear"], session["endMonth"], session["endDay"], session["endHour"], session["endMinute"], tzinfo=pytz.timezone("America/New_York"))
    # Get the data within the time range
    az_data = getBlobEntriesWithinTimeRange(start_dt, end_dt)
    
    dates = []
    data = []
    full_data = []
    # If not empty, add to x and y variables to plot
    if (az_data):
        for line in az_data:
            temp_json_line = json.loads(line)
            time_str = temp_json_line["EnqueuedTimeUtc"] # Get the time in UTC from Azure data
            # Add to data only if for correct picoboard user chosen
            if picoboards[session["picoboard"]] == temp_json_line["SystemProperties"]["connectionDeviceId"]:
                dates.append(time_str)
                # Handling case of when data received is nan (error in sending)
                json_nan_handled = re.sub(r'\bnan\b', 'NaN', temp_json_line["Body"]["data"])
                body_data = json.loads(json_nan_handled)
                full_data.append(body_data)
                data.append(body_data[chosen_var])

    # Convert the UTC datetimes extracted from Azure to CST
    datetimes_plot = [ datetime.strptime(date[:-2], '%Y-%m-%dT%H:%M:%S.%f').replace(tzinfo=pytz.UTC).astimezone(pytz.timezone("America/Chicago")) for date in dates ]

    # ===================================================================================================================================
    # Below code is for writing to 2 CSVs
    
    # 1. For the specific variable chosen (e.g. v_boost) - just timestamp and specific data
    cwd = os.getcwd()
    # Ensuring a unique + descriptive file name for download
    fname = f"{session['picoboard']}_{chosen_var}_{start_dt.strftime('%Y-%m-%d-%H-%M-%S')}_{end_dt.strftime('%Y-%m-%d-%H-%M-%S')}"
    final_fname = "/static/files/" + fname + ".csv"
    # Writing to CSV file - specific variable one
    with open(cwd + final_fname, "w", newline="") as file:
        writer = csv.writer(file)
        fields = ["timestamp", str(chosen_var)]
        writer.writerow(fields)
        for i in range(len(datetimes_plot)):
            writer.writerow([datetimes_plot[i], data[i]])

    # 2. Complete data for all variables in the specific timestamp range
    large_fname = f"{session['picoboard']}_data_{start_dt.strftime('%Y-%m-%d-%H-%M-%S')}_{end_dt.strftime('%Y-%m-%d-%H-%M-%S')}"
    final_large_fname = "/static/files/" + large_fname + ".csv"
    vars_list = ["v_pv", "i_pv", "v_us", "i_us", "v_im", "i_im", "v_cell", "i_cell", "v_boost", "i_lo1", "i_lo2", "i_lo3", "i_ex", "soc"]
    # Writing to CSV file - large one with all variables
    with open(cwd + final_large_fname, "w", newline="") as file:
        writer = csv.writer(file)
        fields = ["timestamp"]
        for var in vars_list:
            fields.append(var)
        writer.writerow(fields)
        for i in range(len(datetimes_plot)):
            data_to_wrt = [datetimes_plot[i]]
            for var in vars_list:
                data_to_wrt.append(full_data[i][var])
            writer.writerow(data_to_wrt)
    # ===================================================================================================================================

    # Create a Figure and Axes
    fig = Figure()
    axis = fig.add_subplot(1, 1, 1)

    # Plot data on the Axes
    axis.plot(datetimes_plot, data)
    axis.set_xlabel("Time (CST)")
    axis.set_ylabel(f"{chosen_var} Value")
    axis.tick_params('x', labelrotation=90)

    # Convert the Figure to a PNG image
    fig.tight_layout()
    output = io.BytesIO()
    FigureCanvas(fig).print_png(output)

    # Save the PNG image to a file
    with open(cwd + '/static/images/plot.png', 'wb') as f:
        f.write(output.getvalue())
    
    return render_template('retrieval_page.html', datetime_input_success=True, picoboard_chosen=True, plot_graph=True, specific_download_file=final_fname, full_download_file=final_large_fname, chosen_var=chosen_var, picoboard_variable=session['picoboard'], empty_data = (len(data) == 0))

@app.route("/submit-thresholds", methods=['POST'])
def submit_thresholds():
    try:
        pv_thresh = float(request.form['pv_thresh'])
        us_thresh = float(request.form['us_thresh'])
        im_thresh = float(request.form['im_thresh'])
        lo1_thresh = float(request.form['lo1_thresh'])
        lo2_thresh = float(request.form['lo2_thresh'])
        lo3_thresh = float(request.form['lo3_thresh'])
        ex_thresh = float(request.form['ex_thresh'])
    except ValueError:
        return render_template("index.html", input_failed=True)

    # Pack all data into a string - the payload of the HTTP POST request will be this packed string
    float_data = f"{pv_thresh},{us_thresh},{im_thresh},{lo1_thresh},{lo2_thresh},{lo3_thresh},{ex_thresh}"
    # print(float_data)

    # Get API token to write data to picogrid (Azure Key Vault)
    keyvault_uri = config.KEY_VAULT_URI
    secret_client = SecretClient(vault_url=keyvault_uri, credential=default_credential)
    secret_name = config.SECRET_NAME

    # Retrieve the secret from Azure Key Vault
    secret_value = secret_client.get_secret(secret_name).value
    ACCESS_TOKEN = secret_value


    # Define the data packet to send
    # Format is from https://docs.particle.io/reference/device-os/api/cloud-functions/particle-function/
    data = {
        'access_token': ACCESS_TOKEN,
        'args': float_data
    }

    PARTICLE_DEVICE = picoboards[request.form["chosen_threshold_board"]]

    # Create the URL for the Particle API
    url = f'https://api.particle.io/v1/devices/{PARTICLE_DEVICE}/{FUNCTION_TO_ACCESS}'

    # Send a POST request to the Particle API
    response = requests.post(url, data=data)

    # Check if the request was successful
    if response.status_code == 200:
        print('Data sent successfully!')
        return render_template("thresholds_page.html", input_success=True)

    else:
        print('Failed to send data.')
        return render_template("thresholds_page.html", input_failed=True)
