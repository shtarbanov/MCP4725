#include "Arduino.h"
#include <Wire.h>

#define WRITEDAC        (0x40)  //(0100 0000) Writes to the DAC
#define WRITEDACEEPROM  (0x60)  //(0110 0000) Writes to the DAC and the EEPROM (persisting the assigned value after reset)

class MCP4725{
 public:
  MCP4725();
  void begin(uint8_t a);  
  void setFastMode();  
  void setVoltageAndSave(uint16_t output);
  void setVoltage(uint16_t output);
  void setVoltageFast(uint16_t output);
  void powerDown500kPullDown();
  void powerDown100kPullDown();
  void powerDown1kPullDown();
  uint16_t readValFromEEPROM();
  uint16_t readCurrentDacVal();
 private:
  uint8_t _i2caddr;
};
