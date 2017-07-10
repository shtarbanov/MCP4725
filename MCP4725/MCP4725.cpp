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
 *
 *The address byte for our dac is (1,1,0,0,0,1,A0). By default A0=GND, but we can connect it to VCC.
 *For A0=GND the hex value of address is 0x62 (1100010)
 *For A0=VCC the hex value of address is 0x63 (1100011)
 *We must pass the address in our Arduino code in the argument of the function .begin(). But the argument must
 *be an 8-bit value and the address is only 7-bit long. What is happening internally is left shifting by 1 bit.
 *You can see this if you trace the definition of the .begin() function and the definition of the twi_setAddress()
 *in the file twi.c. The register TWAR is set to (address << 1). The R/W bit (bit 8) is actually determined based 
 *on the function you send after .begin()
 */
MCP4725::MCP4725() {}
void MCP4725::begin(uint8_t addr) { //in the Arduino code, we'd send 0x62 as arumenment when A0_bit=GND (default).
  _i2caddr = addr;                    //or if A0_bit=vcc then we would pass 0x63 as argument
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
  /*The 12-bit output is in 16-bit form (0,0,0,0,D11.D10.D9.D8.D7.D6.D5.D4,D3.D2.D1.D0).*/
  uint8_t firstbyte=(output>>4);//(0,0,0,0,0,0,0,0,D11.D10.D9.D8.D7.D6.D5.D4) of which only the 8 LSB's survive
  output = output << 12;	//(D3.D2.D1.D0,0,0,0,0,0,0,0,0,0,0,0,0) 
  uint8_t secndbyte=(output>>8);//(0,0,0,0,0,0,0,0,D3,D2,D1,D0,0,0,0,0)	of which only the 8 LSB's survive.
  Wire.write(firstbyte); 
  Wire.write(secndbyte);
  Wire.endTransmission();
}
void MCP4725::setVoltage(uint16_t output){
 /*For Normal-Speed:
  *[Addr.Byte]+[C2,C1,C0,x,x,PD1,PD0,x]+[D11,D10,D9,D8,D7,D6,D5,D4]+[D3,D2,D1,D0,x,x,x,x]  */
  Wire.beginTransmission(_i2caddr);
  Wire.write(WRITEDAC); //[C2,C1,C0,x,x,PD1,PD0,x]=[0,1,0,0,0,0,0,0]
  /*The 12-bit output is in 16-bit form (0,0,0,0,D11.D10.D9.D8.D7.D6.D5.D4,D3.D2.D1.D0).*/
  uint8_t firstbyte=(output>>4);//(0,0,0,0,0,0,0,0,D11.D10.D9.D8.D7.D6.D5.D4) of which only the 8 LSB's survive
  output = output << 12;	//(D3.D2.D1.D0,0,0,0,0,0,0,0,0,0,0,0,0) 
  uint8_t secndbyte=(output>>8);//(0,0,0,0,0,0,0,0,D3,D2,D1,D0,0,0,0,0)	of which only the 8 LSB's survive.
  Wire.write(firstbyte); 
  Wire.write(secndbyte);
  Wire.endTransmission();
}
void MCP4725::setVoltageFast( uint16_t output){
 /*For Fast-Speed:
  *[Addr.Byte]+[C2,C1,PD1,PD0,D11,D10,D9,D8],[D7,D6,D5,D4,D3,D2,D1,D0]  */
  Wire.beginTransmission(_i2caddr);
  //output is a 12-bit value in 16-bit form, namely: [0,0,0,0,D11,D10,D9,D8,D7,D6,D5,D4,D3,D2,D1,D0]
  uint8_t firstbyte=(output>>8); //[0,0,0,0,0,0,0,0,0,0,0,0,D11,D10,D9,D8] only the 8 LSB's survive
  uint8_t secndbyte=(output); //only the 8 LSB's survive.
  Wire.write(firstbyte);  // Upper data bits (0,0,0,0,D11,D10,D9,D8)       
  Wire.write(secndbyte);  // Lower data bits (D7,D6,D5,D4,D3,D2,D1,D0)
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

uint16_t MCP4725::readCurrentDacVal(){
  Wire.requestFrom(_i2caddr, (uint8_t) 5);
  while(Wire.available()!=5){
    Serial.println("Waiting for readValFromEEPROM() to complete.");
    //just wait for a while until the DAC sends the data to the reabuffer
  }
  Wire.read(); //status
  uint8_t upper8bits = Wire.read(); //DAC Register data (D11,D10,D9,D8,D7,D6,D5,D4)
  uint8_t lower8bits = Wire.read(); //DAC Register data (D3,D2,D1,D0,0,0,0,0)
  Wire.read(); //forth returned byte is EEPROM data (x,PD1,PD0,x,D11,D10,D9,D8) 
  Wire.read(); //fifth returned byte is EEPROM data (D7,D6,D5,D4,D3,D2,D1,D0)

  /*We now need to return the value (0,0,0,0,D11,D10,D9,D8,D7,D6,D5,D4,D3,D2,D1,D0). */
  return (upper8bits<<4) | (lower8bits>>4); //This is how we get the 16-bit result we want to return.
}


uint16_t MCP4725::readValFromEEPROM(){
  Wire.requestFrom(_i2caddr, (uint8_t) 5);
  while(Wire.available()!=5){
    Serial.println("Waiting for readValFromEEPROM() to complete.");
    //just wait for a while until the DAC sends the data to the reabuffer
  }
  uint8_t statusBit = Wire.read() >> 7; 
  Wire.read(); //secnd returned byte is DAC Register data (upper 8 bits)
  Wire.read(); //third returned byte is DAC Register data (lower 4 bits + 0000)
  uint8_t upper8bits = Wire.read(); //forth returned byte is EEPROM data (x,PD1,PD0,x,D11,D10,D9,D8)
  uint8_t lower8bits = Wire.read(); //fifth returned byte is EEPROM data (D7,D6,D5,D4,D3,D2,D1,D0)
  
  if(statusBit==0){
    Serial.println("Currently writing to EEPROM. Trying to read again...");
    return readValFromEEPROM();
  }
  else{
    /*We now need to return the value (0,0,0,0,D11,D10,D9,D8,D7,D6,D5,D4,D3,D2,D1,D0). */
    upper8bits = upper8bits & 0b00001111; //clear the first 4 bits.
    return (upper8bits<<8) | lower8bits; //This is how we get the 16-bit result we want to return.
  }
}