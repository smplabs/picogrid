using Dates
using HTTP
using JSON
using CSV

function main()
    PARTICLE_DEVICE = "" # insert device ID
    FUNCTION_TO_ACCESS = "" # insert cloud function name registered with Particle
    ACCESS_TOKEN = "" # insert access token
    url = "https://api.particle.io/v1/devices/"*PARTICLE_DEVICE*"/"*FUNCTION_TO_ACCESS; 
    setpoint1 = "" # insert setpoint
    setpoint2 = "" # insert setpoint
    # Creating the query url string
    float_data = string(setpoint1,",",setpoint2)
    data = Dict(
        "access_token" => ACCESS_TOKEN,
        "args" => float_data
    )
    request = HTTP.post(url, body = data);
    # Printing information about the request
    println("Status: ", request.status, "\n");
    println("Body of Result obtained: \n", String(request.body));
end

main()