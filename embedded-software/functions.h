void mux_write(uint8_t code){
    digitalWrite(S0, (code & 1));
    digitalWrite(S1, (code & 2)>>1);
    digitalWrite(S2, (code & 4)>>2);
    digitalWrite(S3, (code & 8)>>3);    
}

float read_voltage(uint8_t code, float p1, float p2, float p3){
  mux_write(code);
  delay(100); // to allow RC filter to reach steady-state
  float x = (analogRead(COM_IO)/4095.0)*3.3*(101.0/33.0);
  return p1*x*x + p2*x + p3;
}

float read_current(uint8_t code, float p1, float p2){
  mux_write(code);
  delay(100); // to allow RC filter to reach steady-state
  float x = (analogRead(COM_IO)/4095.0)*3.3*(1/(0.033*100.0));
  return p1*x + p2;
}

float read_cellcurrent(uint8_t code){
  
  mux_write(code);
  delay(100); // to allow RC filter to reach steady-state
  return ((analogRead(COM_IO)/4095.0)*3.3 - 1.01)/(0.015*50.0);

}

float compute_initial_soc(float p1, float p2, float p3){
    // switch off all nodes and wait for cell voltage to stabilize
    digitalWrite(PV_GATE, 1);
    digitalWrite(US_GATE, 1);
    digitalWrite(IM_GATE, 1);
    digitalWrite(EX_GATE, 1);
    digitalWrite(LO1_GATE, 0);
    digitalWrite(LO2_GATE, 0);
    digitalWrite(LO3_GATE, 0);
    delay(10*1000);

    float v = read_voltage(V_CELL_CODE,p1,p2,p3);
    // Assuming vmax = 4.2V and vmin = 3V
    float initial_soc = 100 + (100/(4.2-3))*(v-4.2);
    return initial_soc;
}


// Calculate state of charge
float update_charge_soc(float prev_soc, float i, float c_max, float last_soc_update){
    float charge_soc = prev_soc - i*(Time.now() - last_soc_update)*100.0/(c_max*3600);
    return charge_soc;
}

struct switch_state{
    bool pv;
    bool us;
    bool im;
    bool lo1;
    bool lo2;
    bool lo3;
    bool ex;
};

// Can be used for digital actuation of channels
void actuate(switch_state s){
    if (s.pv == true) {
        digitalWrite(PV_GATE, 0);
    }
    else {
        digitalWrite(PV_GATE, 1);
    }

    if (s.us == true) {
        digitalWrite(US_GATE, 0);
    }
    else{
        digitalWrite(US_GATE, 1);
    }
    if (s.im == true) {
        digitalWrite(IM_GATE, 0);
    }
    else {
        digitalWrite(IM_GATE, 1);
    }
    if (s.lo1 == true) {
        digitalWrite(LO1_GATE, 1);
    }
    else {
        digitalWrite(LO1_GATE, 0);
    }
    if (s.lo2 == true) {
        digitalWrite(LO2_GATE, 1);
    }
    else {
        digitalWrite(LO2_GATE, 0);
    }
    if (s.lo3 == true) {
        digitalWrite(LO3_GATE, 1);
    }
    else {
        digitalWrite(LO3_GATE, 0);
    }
    if (s.ex == true) {
        digitalWrite(EX_GATE, 0);
    }
    else {
        digitalWrite(EX_GATE, 1);
    }
    return;
}
