/*
*  MAX30102 driver - uses a combination of ideas from the Maxim & Sparkfun drivers
*                    used Technolbogy's Wire 
*
* j.n.magee 15-10-2019 https://github.com/jeffmer/tinyPulsePPG
* 
* add read temperature
*/

#include "Wire.h"
#include "MAX30102.h"



static const uint8_t MAX_30102_ID = 0x15;

MAX30102::MAX30102() {
  // Constructor
}

boolean MAX30102::begin(uint8_t i2caddr) {
  _i2caddr = i2caddr;
  if (readRegister8(REG_PART_ID) != MAX_30102_ID)  return false; 
  return true;
}

void MAX30102::setup() {
   writeRegister8(REG_MODE_CONFIG,0x40); //reset
   delay(500);
   writeRegister8(REG_FIFO_WR_PTR,0x00);//FIFO_WR_PTR[4:0]
   writeRegister8(REG_OVF_COUNTER,0x00);//OVF_COUNTER[4:0]
   writeRegister8(REG_FIFO_RD_PTR,0x00); //FIFO_RD_PTR[4:0]
   writeRegister8(REG_FIFO_CONFIG,0x4f); //sample avg = 4, fifo rollover=false, fifo almost full = 17
   writeRegister8(REG_MODE_CONFIG,0x03); //0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
   writeRegister8(REG_SPO2_CONFIG,0x27); // SPO2_ADC=4096nA, SPO2 sample rate(100Hz), pulseWidth (411uS)
   writeRegister8(REG_LED1_PA,0x17); //Choose value for ~ 6mA for LED1 (IR)
   writeRegister8(REG_LED2_PA,0x17); // Choose value for ~ 6mA for LED2 (Red)
   writeRegister8(REG_PILOT_PA,0x1F); // Choose value for ~ 6mA for Pilot LED
}

//Tell caller how many samples are available
uint8_t MAX30102::available(void) {
  int8_t numberOfSamples = sense.head - sense.tail;
  if (numberOfSamples < 0) numberOfSamples += STORAGE_SIZE;
  return (numberOfSamples);
}

//Report the next Red value in the FIFO
uint32_t MAX30102::getRed(void) {
  return (sense.red[sense.tail]);
}

//Report the next IR value in the FIFO
uint32_t MAX30102::getIR(void) {
  return (sense.IR[sense.tail]);
}

//Advance the tail
void MAX30102::nextSample(void) {
  if(available()) {
    sense.tail++;
    sense.tail %= STORAGE_SIZE; //Wrap condition
  }
}

// check sensor for new samples and upload if available
uint16_t MAX30102::check(void) {
  byte readPointer = readRegister8(REG_FIFO_RD_PTR);
  byte writePointer = readRegister8(REG_FIFO_WR_PTR);
  int numberOfSamples = 0;
  if (readPointer != writePointer) {
    //Calculate the number of readings we need to get from sensor
    numberOfSamples = writePointer - readPointer;
    if (numberOfSamples < 0) numberOfSamples += 32; //Wrap condition
    int bytesLeftToRead = numberOfSamples * 6; //3 bytes each for Red and IR    
    Wire.beginTransmission(_i2caddr);
    Wire.write(REG_FIFO_DATA);
    Wire.endTransmission();
    bytesLeftToRead = bytesLeftToRead<=32? bytesLeftToRead : 32;
    Wire.requestFrom(_i2caddr, bytesLeftToRead);      
    while (bytesLeftToRead > 0) {
        sense.head++; //Advance the head of the storage struct
        sense.head %= STORAGE_SIZE; //Wrap condition
        sense.IR[sense.head] = readFIFOSample(); 
        //Burst read three more bytes - IR  
		    sense.red[sense.head] = readFIFOSample();
        bytesLeftToRead -= 6;
    }
    Wire.endTransmission();
  } 
  return (numberOfSamples);
}


//Given a register, read it, mask it, and then set the thing
void MAX30102::bitMask(uint8_t reg, uint8_t mask, uint8_t thing)
{
  // Grab current register context
  uint8_t originalContents = readRegister8(reg);

  // Zero-out the portions of the register we're interested in
  originalContents = originalContents & mask;

  // Change contents
  writeRegister8(reg, originalContents | thing);
}

void MAX30102::enableDIETEMPRDY(void) {
  bitMask(REG_INTR_ENABLE_2, REG_INTR_DIE_TEMP_RDY_MASK, REG_INTR_DIE_TEMP_RDY_ENABLE);
}
void MAX30102::disableDIETEMPRDY(void) {
  bitMask(REG_INTR_ENABLE_2, REG_INTR_DIE_TEMP_RDY_MASK, REG_INTR_DIE_TEMP_RDY_DISABLE);
}

// Die Temperature
// Returns temp in C
float MAX30102::readTemperature() 
{
  //DIE_TEMP_RDY interrupt must be enabled
  //See issue 19: https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library/issues/19
  
  // Step 1: Config die temperature register to take 1 temperature sample
  writeRegister8(REG_TEMP_CONFIG, 0x01);

  // Poll for bit to clear, reading is then complete
  // Timeout after 100ms
  unsigned long startTime = millis();
  while (millis() - startTime < 100)
  {
    //uint8_t response = readRegister8(REG_TEMP_CONFIG); //Original way
    //if ((response & 0x01) == 0) break; //We're done!
    
	//Check to see if DIE_TEMP_RDY interrupt is set
	uint8_t response = readRegister8(REG_INTR_STATUS_2);
    if ((response & REG_INTR_DIE_TEMP_RDY_ENABLE) > 0) break; //We're done!
    delay(1); //Let's not over burden the I2C bus
  }
  //TODO How do we want to fail? With what type of error?
  //? if(millis() - startTime >= 100) return(-999.0);

  // Step 2: Read die temperature register (integer)
  int8_t tempInt = readRegister8(REG_TEMP_INTR);
  uint8_t tempFrac = readRegister8(REG_TEMP_FRAC); //Causes the clearing of the DIE_TEMP_RDY interrupt

  // Step 3: Calculate temperature (datasheet pg. 23)
  return (float)tempInt + ((float)tempFrac * 0.0625);
}



// Returns die temp in F
float MAX30102::readTemperatureF() 
{
  float temp = readTemperature();

  if (temp != -999.0) temp = temp * 1.8 + 32.0;

  return (temp);
}

  
//
// Low-level I2C Communication
//
uint8_t MAX30102::readRegister8(uint8_t reg) {
    uint8_t value;
    Wire.beginTransmission(_i2caddr);
    Wire.write((uint8_t)reg);
    Wire.endTransmission();
    Wire.requestFrom(_i2caddr, (byte)1);
    value = Wire.read();
    Wire.endTransmission();
    return value;
}

uint32_t MAX30102::readFIFOSample() {
    byte temp[4]; 
    uint32_t temp32;
    temp[3] = 0;
    temp[2] = Wire.read();
    temp[1] = Wire.read();
    temp[0] = Wire.read();
    memcpy(&temp32, temp, 4);	
    return temp32 & 0x3FFFF;	
}

void MAX30102::writeRegister8(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(_i2caddr);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}
