/*
 * Project Picogrid
 * Description: Embedded code for Pico boards
 * Author: Maitreyee Marathe, Varun Balan
 */


#include "device_config.h"
#include "functions.h"
#include <string>

// If cloud dashboard is developed using ThingSpeak:
// #include "ThingSpeak.h"
// TCPClient client;

// Define variables for sensor reading and miscellaneous operations
float v_pv = 0.0;
float i_pv = 0.0;
float v_us = 0.0;
float i_us = 0.0;
float v_im = 0.0;
float i_im = 0.0;
float v_cell = 0.0;
float i_cell = 0.0;
float v_boost = 0.0;
float i_lo1 = 0.0;
float i_lo2 = 0.0;
float i_lo3 = 0.0;
float i_ex = 0.0;
float i_cell_estimated = 0.0;
float charge_soc = -1.0;
unsigned long int last_soc_update, simulation_start_time, last_duty_update;
// Define duty cycles if implementing variable power at sources/loads
// int pv_duty[<>] = {<>};
// int lo1_duty[<>] = {<>};
// int lo2_duty[<>] = {<>};
int duty_index = 0;
float time_step = 10; // in seconds
device d = get_device_data(); // Get device data

// If using cloud dashboard developed using Azure
// // Integration for "azure-cloud-data" created in Particle Dashboard for the device
// char event[17] = "azure-cloud-data";
// // Function which runs when thresholds are sent to picoboard
// int get_setpoints(String data) {
//  // Process the received_data here
//  Particle.publish("setpoints_received", data, PRIVATE);
//  return 0;
//}


void setup() {
  
  // Pin mappings are defined in device_config.h

  // Define whether pins are input/output
  pinMode(COM_IO, INPUT);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(LO1_GATE, OUTPUT);
  pinMode(LO2_GATE, OUTPUT);
  pinMode(LO3_GATE, OUTPUT);
  pinMode(EX_GATE, OUTPUT);
  pinMode(PV_GATE, OUTPUT);
  pinMode(US_GATE, OUTPUT);
  pinMode(IM_GATE, OUTPUT);

  charge_soc = compute_initial_soc(d.cell_voltage_p[0],d.cell_voltage_p[1],d.cell_voltage_p[2]);
  last_soc_update = Time.now();
  simulation_start_time = Time.now();
  last_duty_update = -1;
  
  // If cloud dashboard is developed using ThingSpeak:
  // ThingSpeak.begin(client);  
  
  // If cloud dashboard is developed using Azure:
  // // register the cloud function for obtaining thresholds
  // Particle.function("get_thresholds", get_thresholds);

}


void loop() {

  // To ensure that code executes once every time_step seconds
  if (duty_index > 0)
    if (Time.now() - last_duty_update < time_step)
      delay((time_step - (Time.now() - last_duty_update))*1000);
  
  // Digital write
  // Switch states in the order pv, us, im, lo1, lo2, lo3, ex
  // switch_state s = {true, false, false, true, false, false, false};
  // actuate(s);

  // Combination of analog and digital write
  // analogWrite(PV_GATE, 255-pv_duty[duty_index]);
  // digitalWrite(US_GATE, 1);
  // digitalWrite(IM_GATE, 1);
  // analogWrite(LO1_GATE, lo1_duty[duty_index]);
  // digitalWrite(LO2_GATE, 0);
  // digitalWrite(LO3_GATE, 0);
  // digitalWrite(EX_GATE, 1);
  
  last_duty_update = Time.now();

  // Reading sensors
  v_cell = read_voltage(V_CELL_CODE,d.cell_voltage_p[0],d.cell_voltage_p[1],d.cell_voltage_p[2]);
  i_cell = read_cellcurrent(I_CELL_CODE);
  v_pv = read_voltage(V_PV_CODE,0,1.1,0);
  i_pv = read_current(I_PV_CODE,d.current_p1[0],d.current_p2[0]);
  v_us = read_voltage(V_US_CODE,0,1.1,0);
  i_us = read_current(I_US_CODE,d.current_p1[1],d.current_p2[1]);
  v_im = read_voltage(V_IM_CODE,0,1.1,0);
  i_im = read_current(I_IM_CODE,d.current_p1[2],d.current_p2[2]);
  v_boost = read_voltage(V_BOOST_CODE,0,1.1,0);
  i_lo1 = read_current(I_LO1_CODE,d.current_p1[3],d.current_p2[3]);
  i_lo2 = read_current(I_LO2_CODE,d.current_p1[4],d.current_p2[4]);
  i_lo3 = read_current(I_LO3_CODE,d.current_p1[5],d.current_p2[5]);
  i_ex = read_current(I_EX_CODE,d.current_p1[6],d.current_p2[6]);

  // Estimating i_cell
  i_cell_estimated = -(i_pv + i_us + i_im) + (5/v_cell)*(1/0.9)*(i_lo1 + i_lo2 + i_lo3 + i_ex);

  // Updating soc
  charge_soc = update_charge_soc(charge_soc, i_cell_estimated, d.c_max, last_soc_update); 
  last_soc_update = Time.now();

  // To ensure that cloud dashboard is updated once every two timesteps
  if (duty_index % 2 == 0)
  {  
    ////////////////////////////////////////   
    // Send sensor data to cloud dashboard
    ////////////////////////////////////////
    // If cloud dashboard is developed using ThingSpeak:   
    //  // update channel a
    //  ThingSpeak.setField(1,v_cell);
    //  ThingSpeak.setField(2,i_cell);
    //  ThingSpeak.setField(3,v_pv);
    //  ThingSpeak.setField(4,i_pv);
    //  ThingSpeak.setField(5,v_us);
    //  ThingSpeak.setField(6,i_us);
    //  ThingSpeak.setField(7,v_im);
    //  ThingSpeak.setField(8,i_im);
    //  ThingSpeak.writeFields(d.channel_a_number, d.channel_a_writeapikey);
    //  // update channel b
    //  ThingSpeak.setField(1, v_boost);
    //  ThingSpeak.setField(2, i_lo1);
    //  ThingSpeak.setField(3, i_lo2);
    //  ThingSpeak.setField(4, i_lo3);
    //  ThingSpeak.setField(5, i_ex);
    //  ThingSpeak.setField(6, charge_soc);
    //  ThingSpeak.writeFields(d.channel_b_number, d.channel_b_writeapikey);

    // If cloud dashboard is developed using Azure:
    // // Put data in a JSON-like string to send to Azure dashboard
    // char picogrid_data_json[1024];
    // snprintf(picogrid_data_json, sizeof(picogrid_data_json), "{\"v_pv\":%f,\"i_pv\":%f,\"v_us\":%f,\"i_us\":%f,\"v_im\":%f,\"i_im\":%f,\"v_cell\":%f,\"i_cell\":%f,\"v_boost\":%f,\"i_lo1\":%f,\"i_lo2\":%f,\"i_lo3\":%f,\"i_ex\":%f,\"soc\":%f}", v_pv, i_pv, v_us, i_us, v_im, i_im, v_cell, i_cell, v_boost, i_lo1, i_lo2, i_lo3, i_ex, charge_soc);
    // // Send the string using the event created with Azure integration
    // Particle.publish(event, picogrid_data_json, PRIVATE);

    /////////////////////////////////////////////////////////// 
    // Read setpoints from ThingSpeak-based cloud dashboard
    ///////////////////////////////////////////////////////////    
    // setpoint = ThingSpeak.readFloatField(d.channel_setpoints_number, 1, d.channel_setpoints_readapikey);

  }

  duty_index = duty_index + 1;

}