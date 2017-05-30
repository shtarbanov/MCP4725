#include "Arduino.h"
#include <Wire.h>
#include "MCP4725.h"
MCP4725 dac; //create a dac object

const PROGMEM uint16_t DACLookup_FullSine_6Bit[64] ={
  2048, 2248, 2447, 2642, 2831, 3013, 3185, 3346,  3495, 3630, 3750, 3853, 3939, 4007, 4056, 4085,  4095, 4085, 4056, 4007, 3939, 3853, 3750, 3630,
  3495, 3346, 3185, 3013, 2831, 2642, 2447, 2248,  2048, 1847, 1648, 1453, 1264, 1082,  910,  749,   600,  465,  345,  242,  156,   88,   39,   10,
     0,   10,   39,   88,  156,  242,  345,  465,   600,  749,  910, 1082, 1264, 1453, 1648, 1847};

void setup(void) {
  dac.begin(0x62); //addres for the dac is 0x62 (default) or 0x63 (A0 pin tied to VCC)
  dac.setFastMode(); //comment this out to unset fastmode
}
void loop(void) {
    uint16_t i;
    for (i = 0; i < 64; i++){
      //dac.setVoltage(pgm_read_word(&(DACLookup_FullSine_6Bit[i]))); //gives: 37Hz with FastMode unset; 110Hz with FastMode set.
      dac.setVoltageFast(pgm_read_word(&(DACLookup_FullSine_6Bit[i]))); //gives: 48Hz with FastMode unset; 143Hz with FastMode set.
    }
    dac.powerDown500kPullDown();
    delay(5);    
}
