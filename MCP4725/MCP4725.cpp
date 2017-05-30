#include "Arduino.h"
#include <Wire.h>
#include "MCP4725.h"
/*See page 19 of the MCP4726 DataSheet.
 *[C2,C1]=00 -- Fast Mode
 *[C2,C1,C0]=010 --- Write to DAC register only
 *[C2,C1,C0]=011 --- Write to DAC and EEPROM registers
 *[PD1,PD0]=00 -- Power Normal mode
 *[PD1,PD0]=01 -- Power Down mode with 1k pull down
 *[PD1,PD0]=10 -- Power Down mode with 100k pull down
 *[PD1,PD0]=11 -- Power Down mode with 500k pull down
 *The contents of bytes 2nd,3rd,4th are orgnized differently
 *based on whether the DAC is operated in Normal-Speed or Fast-Speed. Let
 *D denote data byte, x denote don't-care. Then:
 *
 *For Normal-Speed:
 *[Addr.Byte]+[C2,C1,C0,x,x,PD1,PD0,x]+[D11,D10,D9,D8,D7,D6,D5,D4]+[D3,D2,D1,D0,x,x,x,x]
 *
 *For Fast-Speed:
 *[Addr.Byte]+[C2,C1,PD1,PD0,D11,D10,D9,D8],[D7,D6,D5,D4,D3,D2,D1,D0]
 */
MCP4725::MCP4725() {}
void MCP4725::begin(uint8_t addr) {
  _i2caddr = addr;
  Wire.begin();
}
void MCP4725::setFastMode(){
#ifdef TWBR // in case this library is used on a chip that does not have TWBR reg defined.  
  TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency for the ATMega to 400kHz
#endif  
}
/*
 *Wire.write() takes an 8-bit unsigned int. We thus need to convert our 16-bit
 *argument to 2 8-bit variables. 16 to 8-bit, conversion keeps the 8 LSBs.
 */
void MCP4725::setVoltageAndSave(uint16_t output){
 /*For Normal-Speed: (Only normal speed includes the C3 bit, needed for writing to EEPROM)
  *[Addr.Byte]+[C2,C1,C0,x,x,PD1,PD0,x]+[D11,D10,D9,D8,D7,D6,D5,D4]+[D3,D2,D1,D0,x,x,x,x]  */
  Wire.beginTransmission(_i2caddr);
  Wire.write(WRITEDACEEPROM); //[C2,C1,C0,x,x,PD1,PD0,x]=[0,1,1,0,0,0,0,0]
  uint8_t firstbyte=(output>>4);//(D11.D10.D9.D8.D7.D6.D5.D4)
  uint8_t secndbyte=(output<<8);//(D3.D2.D1.D0.x.x.x.x)
  Wire.write(firstbyte); 
  Wire.write(secndbyte);
  Wire.endTransmission();
}
void MCP4725::setVoltage(uint16_t output){
 /*For Normal-Speed:
  *[Addr.Byte]+[C2,C1,C0,x,x,PD1,PD0,x]+[D11,D10,D9,D8,D7,D6,D5,D4]+[D3,D2,D1,D0,x,x,x,x]  */
  Wire.beginTransmission(_i2caddr);
  Wire.write(WRITEDAC); //[C2,C1,C0,x,x,PD1,PD0,x]=[0,1,0,0,0,0,0,0]
  /*The 16-bit output is in the form (0,0,0,0,D11.D10.D9.D8.D7.D6.D5.D4,D3.D2.D1.D0).*/
  uint8_t firstbyte=(output>>4);//(D11.D10.D9.D8.D7.D6.D5.D4)
  uint8_t secndbyte=(output<<8);//(D3.D2.D1.D0.x.x.x.x)
  Wire.write(firstbyte); 
  Wire.write(secndbyte);
  Wire.endTransmission();
}
void MCP4725::setVoltageFast( uint16_t output){
 /*For Fast-Speed:
  *[Addr.Byte]+[C2,C1,PD1,PD0,D11,D10,D9,D8],[D7,D6,D5,D4,D3,D2,D1,D0]  */
  Wire.beginTransmission(_i2caddr);
  uint8_t firstbyte=(output>>8); //[0,0,0,0,0,0,0,0,0,0,0,0,x11,x10,x9,x8]
  uint8_t secndbyte=(output);
  Wire.write(firstbyte);  // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4)
  Wire.write(secndbyte);  // Lower data bits          (D3.D2.D1.D0.x.x.x.x)
  Wire.endTransmission();
}
void MCP4725::powerDown1kPullDown(){ //[PD1,PD0]=01; [C2,C1,C0]=010 - Write to DAC only
 /*For Normal-Speed:
  *[Addr.Byte]+[C2,C1,C0,x,x,PD1,PD0,x]+[D11,D10,D9,D8,D7,D6,D5,D4]+[D3,D2,D1,D0,x,x,x,x]  */
  Wire.beginTransmission(_i2caddr);
  Wire.write(0b01000010);
  Wire.write(0b00000000);
  Wire.write(0b00000000);
  Wire.endTransmission();
}
void MCP4725::powerDown100kPullDown(){//[PD1,PD0]=10; [C2,C1,C0]=010 - Write to DAC only
 /*For Normal-Speed:
  *[Addr.Byte]+[C2,C1,C0,x,x,PD1,PD0,x]+[D11,D10,D9,D8,D7,D6,D5,D4]+[D3,D2,D1,D0,x,x,x,x]  */
  Wire.beginTransmission(_i2caddr);
  Wire.write(0b01000100);
  Wire.write(0b00000000);
  Wire.write(0b00000000);
  Wire.endTransmission();  
}
void MCP4725::powerDown500kPullDown(){//[PD1,PD0]=11; [C2,C1,C0]=010 - Write to DAC only
 /*For Normal-Speed:
  *[Addr.Byte]+[C2,C1,C0,x,x,PD1,PD0,x]+[D11,D10,D9,D8,D7,D6,D5,D4]+[D3,D2,D1,D0,x,x,x,x]  */
  Wire.beginTransmission(_i2caddr);
  Wire.write(0b01000110);
  Wire.write(0b00000000);
  Wire.write(0b00000000);
  Wire.endTransmission();
}