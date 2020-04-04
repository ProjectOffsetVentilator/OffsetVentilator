/**
* Adapted in part from the following example provided with the Arduino IDE:
* SCP1000 Barometric Pressure Sensor Display
* created 31 July 2010
* modified 14 August 2010
* by Tom Igoe
* ---------------------------------------------------------------------------
* Adapted in part from code provided by the manufacturer for the BMP-388 sensor:
* Copyright (c) 2020 Bosch Sensortec GmbH. All rights reserved.
*
* BSD-3-Clause
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its
*    contributors may be used to endorse or promote products derived from
*    this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
* ---------------------------------------------------------------------------
* ventsense_fw.ino
* v0.1-0
* Created 4/3/2020
* Clay Gilmore
* Helpful Engineering
* 
* Purpose:
* This code reads temperature and pressure data from a BMP-388 sensor
* via the SPI interface at a 10 Hz rate and sends it out on the serial port.
* Calibration and temperature compensation is performed on the data prior to
* sending. 
* 
* Requirements:
*   Hardware
*     1x Arduino
*     1x BMP-388
*     USB cable
*     Computer
*     
*   Software
*     ventsense.py v0.1-x (or serial terminal app)
*     Arduino v1.8.5 or later
*     
* Setup:
* Wire BMP-388 to Arduino, per the Circuit section, below. Connect Arduino 
* to computer via USB cable. Load Arduino with this code. Launch ventsense.py
* or open a serial terminal app.
*
* Circuit:
* Level-shifting is required to safely interface the 5V Arduino with the 
* 3.3V BMP-388 on the following pins (SCK, SDI, CSB). Also, the module
* I am using has a 10K pullup resistor on SDO. Not sure if it is required.
* Example level-shifting circuit:
* 
*        Arduino output
*             |
*             \
*             / R1
*             \ 1K
*             /
*             |
*             +----->> BMP-388 input
*             |
*             \
*             / R2
*             \ 2K
*             /
*             |
*            GND
* 
* Connect the pins as follows:
* Arduino             BMP-388
* Pin#  Name          Pin#  Name
* 10    CS ---------- 6     CSB         (level-shifting required)
* 11    MOSI -------- 4     SDI         (level-shifting required)
* 12    MISO -------- 5     SDO
* 13    SCK --------- 2     SCK         (level-shifting required)
*  -    3.3V -------- 1,10  VDDIO,VDD  
*  -    GND --------- 3,8,9 VSS
*       
*       
* Notes:
* Baud rate is 115200.
* 
* Only 1 sensor supported, at the moment.
* 
* Serial output format is:
* <timestamp>,<temp 1>,<pressure 1>
* 
* The temperature is in degrees Celsius and the pressure is in hPa.
*/

// the sensor communicates using SPI, so include the library:
#include <SPI.h>

//Sensor's memory register addresses:
const byte NVM_PAR_T1_7_0 = 0x31;
const byte DATA_0    = 0x04;
const byte DATA_1    = 0x05;
const byte DATA_2    = 0x06;
const byte PWR_CTRL  = 0x1B;
const byte ODR       = 0x1D;
const byte CONFIG    = 0x1F;

const byte READ  = 0b10000000;
const byte WRITE = 0b01111111;

const int CAL_DATA_READ_LEN = 21;
const int SENSOR_DATA_READ_LEN = 6;

byte rxBuffer[CAL_DATA_READ_LEN+2] = {0};

// pins used for the connection with the sensor
// the other you need are controlled by the SPI library):
const int chipSelectPin = 10;

unsigned long previousMillis = 0;
const long interval = 100;          // 100 ms sample period

struct bmp3_quantized_calib_data
{
    double par_t1;
    double par_t2;
    double par_t3;
    double par_p1;
    double par_p2;
    double par_p3;
    double par_p4;
    double par_p5;
    double par_p6;
    double par_p7;
    double par_p8;
    double par_p9;
    double par_p10;
    double par_p11;
    double t_lin;
} cal_data;


void setup() {
  Serial.begin(115200);

  // start the SPI library:
  SPI.begin();

  // initalize the chip select pins:
  pinMode(chipSelectPin, OUTPUT);

  // briefly assert chip select in order to configure BMP-388 for SPI mode
  digitalWrite(chipSelectPin, LOW);
  delay(20);
  digitalWrite(chipSelectPin, HIGH);

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // 1 MHz clock
  //put to sleep (soft reset)
  writeRegister(PWR_CTRL, 0x00);

  readRegisters(NVM_PAR_T1_7_0, CAL_DATA_READ_LEN, rxBuffer);
  parse_calib_data(&rxBuffer[2], &cal_data);
  
  //Configure BMP-388:
  writeRegister(PWR_CTRL, 0x33);  // enable temp, enable press, normal mode
  SPI.endTransaction();

  Serial.println("timestamp,temp 1,press 1");
  
  // give the sensor time to set up:
  delay(500);
}

void loop() {
  unsigned long temperatureData = 0;
  unsigned long pressureData = 0;

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;  

    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // 1 MHz clock
  
    readRegisters(DATA_0, SENSOR_DATA_READ_LEN, rxBuffer);

    SPI.endTransaction();
  
    pressureData = (unsigned long)rxBuffer[2];
    pressureData |= (unsigned long)rxBuffer[3] << 8;
    pressureData |= (unsigned long)rxBuffer[4] << 16;
  
    temperatureData = (unsigned long)rxBuffer[5];
    temperatureData |= (unsigned long)rxBuffer[6] << 8;
    temperatureData |= (unsigned long)rxBuffer[7] << 16;

    double dtemp = compensate_temperature(temperatureData, &cal_data);
    double dpress = compensate_pressure(pressureData, &cal_data);
    
    // display the sensor data
    Serial.print(currentMillis);
    Serial.print(",");
    Serial.print(dtemp);
    Serial.print(",");
    Serial.println(dpress / 100.0);
  }
}

unsigned int readRegisters(byte thisRegister, int bytesToRead, byte* rxbuff)
{
  unsigned int result = 0;   // result to return
  int i = 0;

  // now combine the address and the command into one byte
  rxbuff[0] = thisRegister | READ;

  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);
  
  // send the device the register you want to read:
  SPI.transfer(rxbuff, bytesToRead+2);

  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);
  
  // return the result:
  return (result);
}


void writeRegister(byte thisRegister, byte thisValue)
{
  // now combine the register address and the command into one byte:
  byte dataToSend = thisRegister & WRITE;

  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(dataToSend); //Send register location
  SPI.transfer(thisValue);  //Send value to record into register

  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);
}

static void parse_calib_data(const uint8_t *raw_cal, struct bmp3_quantized_calib_data *cal)
{
  /* Temporary variable */
  double temp_var;

  uint16_t nvm_par_t1;
  uint16_t nvm_par_t2;
  int8_t nvm_par_t3;
  int16_t nvm_par_p1;
  int16_t nvm_par_p2;
  int8_t nvm_par_p3;
  int8_t nvm_par_p4;
  uint16_t nvm_par_p5;
  uint16_t nvm_par_p6;
  int8_t nvm_par_p7;
  int8_t nvm_par_p8;
  int16_t nvm_par_p9;
  int8_t nvm_par_p10;
  int8_t nvm_par_p11;

  /* 1 / 2^8 */
  temp_var = 0.00390625f;
  nvm_par_t1 = (((uint16_t)raw_cal[1] << 8) | (uint16_t)raw_cal[0]);
  cal->par_t1 = ((double)nvm_par_t1 / temp_var);

  nvm_par_t2 = (((uint16_t)raw_cal[3] << 8) | (uint16_t)raw_cal[2]);
  temp_var = 1073741824.0f;
  cal->par_t2 = ((double)nvm_par_t2 / temp_var);

  nvm_par_t3 = (int8_t)raw_cal[4];
  temp_var = 281474976710656.0f;
  cal->par_t3 = ((double)nvm_par_t3 / temp_var);

  nvm_par_p1 = (int16_t)(((uint16_t)raw_cal[6] << 8) | (uint16_t)raw_cal[5]);
  temp_var = 1048576.0f;
  cal->par_p1 = ((double)(nvm_par_p1 - (16384)) / temp_var);

  nvm_par_p2 = (int16_t)(((uint16_t)raw_cal[8] << 8) | (uint16_t)raw_cal[7]);
  temp_var = 536870912.0f;
  cal->par_p2 = ((double)(nvm_par_p2 - (16384)) / temp_var);

  nvm_par_p3 = (int8_t)raw_cal[9];
  temp_var = 4294967296.0f;
  cal->par_p3 = ((double)nvm_par_p3 / temp_var);

  nvm_par_p4 = (int8_t)raw_cal[10];
  temp_var = 137438953472.0f;
  cal->par_p4 = ((double)nvm_par_p4 / temp_var);

  nvm_par_p5 = (((uint16_t)raw_cal[12] << 8) | (uint16_t)raw_cal[11]);
  /* 1 / 2^3 */
  temp_var = 0.125f;
  cal->par_p5 = ((double)nvm_par_p5 / temp_var);

  nvm_par_p6 = (((uint16_t)raw_cal[14] << 8) | (uint16_t)raw_cal[13]);
  temp_var = 64.0f;
  cal->par_p6 = ((double)nvm_par_p6 / temp_var);

  nvm_par_p7 = (int8_t)raw_cal[15];
  temp_var = 256.0f;
  cal->par_p7 = ((double)nvm_par_p7 / temp_var);

  nvm_par_p8 = (int8_t)raw_cal[16];
  temp_var = 32768.0f;
  cal->par_p8 = ((double)nvm_par_p8 / temp_var);

  nvm_par_p9 = (int16_t)(((uint16_t)raw_cal[18] << 8) | (uint16_t)raw_cal[17]);
  temp_var = 281474976710656.0f;
  cal->par_p9 = ((double)nvm_par_p9 / temp_var);

  nvm_par_p10 = (int8_t)raw_cal[19];
  temp_var = 281474976710656.0f;
  cal->par_p10 = ((double)nvm_par_p10 / temp_var);

  nvm_par_p11 = (int8_t)raw_cal[20];
  temp_var = 36893488147419103232.0f;
  cal->par_p11 = ((double)nvm_par_p11 / temp_var);
}

static double compensate_temperature(unsigned long uncomp_temperature, struct bmp3_quantized_calib_data *cal)
{
    double partial_data1;
    double partial_data2;

    partial_data1 = (double)(uncomp_temperature - cal->par_t1);
    partial_data2 = (double)(partial_data1 * cal->par_t2);

    /* Update the compensated temperature in calib structure since this is
     * needed for pressure calculation */
    cal->t_lin = partial_data2 + (partial_data1 * partial_data1) * cal->par_t3;

    /* Returns compensated temperature */
    return cal->t_lin;
}

static double compensate_pressure(unsigned long uncomp_pressure_in, const struct bmp3_quantized_calib_data *cal)
{
    /* Variable to store the compensated pressure */
    double comp_pressure;
    double uncomp_pressure = uncomp_pressure_in;

    /* Temporary variables used for compensation */
    double partial_data1;
    double partial_data2;
    double partial_data3;
    double partial_data4;
    double partial_out1;
    double partial_out2;

    partial_data1 = cal->par_p6 * cal->t_lin;
    partial_data2 = cal->par_p7 * (cal->t_lin * cal->t_lin);
    partial_data3 = cal->par_p8 * (cal->t_lin * cal->t_lin * cal->t_lin);
    partial_out1 = cal->par_p5 + partial_data1 + partial_data2 + partial_data3;
    partial_data1 = cal->par_p2 * cal->t_lin;
    partial_data2 = cal->par_p3 * (cal->t_lin * cal->t_lin);
    partial_data3 = cal->par_p4 * (cal->t_lin * cal->t_lin * cal->t_lin);
    partial_out2 = uncomp_pressure * (cal->par_p1 + partial_data1 + partial_data2 + partial_data3);
    partial_data1 = (uncomp_pressure * uncomp_pressure);
    partial_data2 = cal->par_p9 + cal->par_p10 * cal->t_lin;
    partial_data3 = partial_data1 * partial_data2;
    partial_data4 = partial_data3 + (uncomp_pressure * uncomp_pressure * uncomp_pressure) * cal->par_p11;
    comp_pressure = partial_out1 + partial_out2 + partial_data4;

    return comp_pressure;
}
