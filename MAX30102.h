/*
*  MAX30102 driver - uses a combination of ideas from the Maxim & Sparkfun drivers
*                    used Technolbogy's TinyI2C 
*
* j.n.magee 15-10-2019 https://github.com/jeffmer/tinyPulsePPG
* 
* add read temperature
*/
#include <arduino.h>
#define MAX30105_ADDRESS 0x57 

//register addresses
#define REG_INTR_STATUS_1 0x00
#define REG_INTR_STATUS_2 0x01
#define REG_INTR_ENABLE_1 0x02
#define REG_INTR_ENABLE_2 0x03
#define REG_FIFO_WR_PTR 0x04
#define REG_OVF_COUNTER 0x05
#define REG_FIFO_RD_PTR 0x06
#define REG_FIFO_DATA 0x07
#define REG_FIFO_CONFIG 0x08
#define REG_MODE_CONFIG 0x09
#define REG_SPO2_CONFIG 0x0A
#define REG_LED1_PA 0x0C
#define REG_LED2_PA 0x0D
#define REG_PILOT_PA 0x10
#define REG_MULTI_LED_CTRL1 0x11
#define REG_MULTI_LED_CTRL2 0x12
#define REG_TEMP_INTR 0x1F
#define REG_TEMP_FRAC 0x20
#define REG_TEMP_CONFIG 0x21
#define REG_PROX_INT_THRESH 0x30
#define REG_REV_ID 0xFE
#define REG_PART_ID 0xFF

#define STORAGE_SIZE 3 // buffer size in samples


static const uint8_t REG_INTR_DIE_TEMP_RDY_MASK = (byte)~0b00000010;
static const uint8_t REG_INTR_DIE_TEMP_RDY_ENABLE = 0x02;
static const uint8_t REG_INTR_DIE_TEMP_RDY_DISABLE = 0x00;

//spo2_table is approximated as  -45.060*ratioAverage* ratioAverage + 30.354 *ratioAverage + 94.845 ;
const uint8_t spo2_table[184] PROGMEM =
{ 95, 95, 95, 96, 96, 96, 97, 97, 97, 97, 97, 98, 98, 98, 98, 98, 99, 99, 99, 99,
  99, 99, 99, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 99, 99, 99, 99, 99, 99, 99, 99, 98, 98, 98, 98, 98, 98, 97, 97,
  97, 97, 96, 96, 96, 96, 95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91,
  90, 90, 89, 89, 89, 88, 88, 87, 87, 86, 86, 85, 85, 84, 84, 83, 82, 82, 81, 81,
  80, 80, 79, 78, 78, 77, 76, 76, 75, 74, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67,
  66, 66, 65, 64, 63, 62, 62, 61, 60, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50,
  49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 31, 30, 29,
  28, 27, 26, 25, 23, 22, 21, 20, 19, 17, 16, 15, 14, 12, 11, 10, 9, 7, 6, 5,
  3, 2, 1
} ;


class MAX30102{
 public: 
    MAX30102(void);
  
    boolean begin(uint8_t i2caddr = MAX30105_ADDRESS);  
    
    // Setup the IC with 
    // powerLevel = 0x1F,  sampleAverage = 4,  Mode = Red and IR,  
    // sampleRate = 100,  pulseWidth = 411,  adcRange = 4096  
    void setup();
    void off() {writeRegister8(REG_MODE_CONFIG,0x80);}
    
    //FIFO Reading
    uint16_t check(void);     //Checks for new data and fills FIFO
    uint8_t  available(void); //returns number of samples  available (head - tail)
    void     nextSample(void);//Advances the tail of the sense array
    uint32_t getRed(void);    //Returns the FIFO sample pointed to by tail
    uint32_t getIR(void);     //Returns the FIFO sample pointed to by tail
  
    void bitMask(uint8_t reg, uint8_t mask, uint8_t thing);
   void enableDIETEMPRDY(void);
   void disableDIETEMPRDY(void);

    // Die Temperature
      float readTemperature();
      float readTemperatureF();
      
    // Low-level I2C communication
    uint8_t  readRegister8(uint8_t reg);
    uint32_t readFIFOSample(void);
    void     writeRegister8(uint8_t reg, uint8_t value);
 
 private:
    uint8_t _i2caddr;
    struct {
      uint32_t red[STORAGE_SIZE];
      uint32_t IR[STORAGE_SIZE];
      byte head;
      byte tail;
    } sense; //Circular buffer of readings from the sensor

};
