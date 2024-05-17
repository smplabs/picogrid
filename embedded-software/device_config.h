// File to store device specific parameters

// Each voltage/current sensor's output is connected to an input pin on the MUX.
// The following are access codes for each input pin on the MUX.
const uint8_t V_PV_CODE = 0x0;
const uint8_t I_PV_CODE = 0x1;
const uint8_t V_US_CODE = 0x2;
const uint8_t I_US_CODE = 0x3;
const uint8_t V_IM_CODE = 0x4;
const uint8_t I_IM_CODE = 0x5;
const uint8_t V_CELL_CODE = 0x6;
const uint8_t I_CELL_CODE = 0x7;
const uint8_t V_BOOST_CODE = 0x8;
const uint8_t I_LO1_CODE = 0x9;
const uint8_t I_LO2_CODE = 0xA;
const uint8_t I_LO3_CODE = 0xB;
const uint8_t I_EX_CODE = 0xC;
// Defining pin mapping for the Argon microcontroller
const pin_t COM_IO = A0;
const pin_t S0 = D2;
const pin_t S1 = D3;
const pin_t S2 = D4;
const pin_t S3 = D5;
const pin_t LO1_GATE = A1;
const pin_t LO2_GATE = A2;
const pin_t LO3_GATE = A3;
const pin_t EX_GATE = A4;
const pin_t PV_GATE = D6;
const pin_t US_GATE = D7;
const pin_t IM_GATE = D8;

struct device {
  String device_id;
  String channel_setpoints_readapikey;
  float e_max; // max energy capacity in Wh
  float c_max; // max charge capacity in Ah
  float current_p1[7]; // current calibration coefficients p1,p2 in order - pv,us,im,lo1,lo2,lo3,ex
  float current_p2[7]; // y = p1*x + p2
  float cell_voltage_p[3]; // cell voltage calibration coefficients; y = p1*x^2 + p2*x +p3
  float user_schedule[7][60]; // the user's schedule for on and off times of channel in minutes. 
  // Order of channels: PV, US, IM, LO1, LO2, LO3, EX
  // e.g.: user_schedule[4][4] = {5,10,18,22} means that the 4th channel (LO1) will be switched on between minute 5 and 10, 
  // and then between minute 18 and 22

  // If cloud dashboard is developed using ThingSpeak (refer cloud-dashboard/thingspeak for setting up ThingSpeak channels)
  // unsigned long channel_a_number;
  // String channel_a_writeapikey;
  // unsigned long channel_b_number;
  // String channel_b_writeapikey;
  // unsigned long channel_setpoints_number;  

};

device get_device_data(){
  device d;
  for (int i=0; i<7; i++)
    for (int j=0; j<60; j++)
      d.user_schedule[i][j] = 360;
  d.device_id = System.deviceID();
  /* Add values here
  if (d.device_id == <>) {
    d.channel_a_number = <>;
    d.channel_a_writeapikey = <>;
    d.channel_b_number = <>;
    d.channel_b_writeapikey = <>;
    d.channel_setpoints_number = <>;
    d.channel_setpoints_readapikey = <>;
    d.e_max = <>
    d.c_max = <>
    float current_p1[7] = <>;
    float current_p2[7] = <>;
    for (int i=0; i<7; i++) d.current_p1[i] = current_p1[i];
    for (int i=0; i<7; i++) d.current_p2[i] = current_p2[i];
    float cell_voltage_p[3] = <>;
    for (int i=0; i<3; i++) d.cell_voltage_p[i] = cell_voltage_p[i];
    float pv_schedule[<>] = <>;
    float us_schedule[<>] = <>;
    float im_schedule[<>] = <>;
    float lo1_schedule[<>] = <>;
    float lo2_schedule[<>] = <>;
    float lo3_schedule[<>] = <>;
    float ex_schedule[<>] = <>; 
    for (int i=0; i<sizeof(pv_schedule)/sizeof(pv_schedule[0]); i++) d.user_schedule[0][i] = pv_schedule[i];
    for (int i=0; i<sizeof(us_schedule)/sizeof(us_schedule[0]); i++) d.user_schedule[1][i] = us_schedule[i];
    for (int i=0; i<sizeof(im_schedule)/sizeof(im_schedule[0]); i++) d.user_schedule[2][i] = im_schedule[i];
    for (int i=0; i<sizeof(lo1_schedule)/sizeof(lo1_schedule[0]); i++) d.user_schedule[3][i] = lo1_schedule[i];
    for (int i=0; i<sizeof(lo2_schedule)/sizeof(lo2_schedule[0]); i++) d.user_schedule[4][i] = lo2_schedule[i];
    for (int i=0; i<sizeof(lo3_schedule)/sizeof(lo3_schedule[0]); i++) d.user_schedule[5][i] = lo3_schedule[i];
    for (int i=0; i<sizeof(ex_schedule)/sizeof(ex_schedule[0]); i++) d.user_schedule[6][i] = ex_schedule[i];
  } 
  */
 return d;

}