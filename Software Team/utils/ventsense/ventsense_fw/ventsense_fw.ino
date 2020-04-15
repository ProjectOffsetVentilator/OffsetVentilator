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
* v0.2-0
* Created 4/3/2020
* Clay Gilmore
* Helpful Engineering
* 
* Purpose:
* This code reads temperature and pressure data from a bank of 3 BMP-388 sensors
* via the SPI interface at a 10 Hz rate and sends it out on the serial port.
* Calibration and temperature compensation is performed on the data prior to
* sending. 
* 
* Requirements:
*   Hardware
*     1x Arduino Uno
*     3x BMP-388
*     USB cable
*     Computer
*     
*   Software
*     ventsense client v0.2-x (or serial terminal app)
*     Arduino v1.8.5 or later
*     
* Setup:
* Wire BMP-388 to Arduino, per the Circuit section, below. Connect Arduino 
* to computer via USB cable. Load Arduino with this code. Launch ventsense.py
* or open a serial terminal app.
*
* Circuit:
* Level-shifting is required to safely interface the 5V Arduino with the 
* 3.3V BMP-388 on the following pins (SCK, SDI, CSB). If you are using an
* evaluation module, check its specifications to see whether it already does
* the level shifting for you. If not, or if you are using the Offset Ventilator
* sensor board, you will need to add your own level-shifting circuitry if you
* are interfacing it with an off-the-shelf Arduino Uno. Also, the module
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
* Arduino                       BMP-388 #1           BMP-388 #2            BMP-38 #3
* Pin#  Name                    Pin#  Name           Pin#  Name            Pin#  Name
*  8    CS1 ---<level shift>--- 6     CSB                                            
*  9    CS2 ---<level shift>------------------------ 6     CSB                       
* 10    CS3 ---<level shift>---------------------------------------------- 6     CSB
* 11    MOSI --<level shift>--- 4     SDI ---------- 4     SDI ----------- 4     SDI
* 12    MISO ------------------ 5     SDO ---------- 5     SDO ----------- 5     SDO
* 13    SCK ---<level shift>--- 2     SCK ---------- 2     SCK ----------- 2     SCK
*  -    3.3V ------------------ 1,10  VDDIO,VDD ---- 1,10  VDDIO,VDD ----- 1,10  VDDIO,VDD
*  -    GND ------------------- 3,8,9 VSS ---------- 3,8,9 VSS ----------- 3,8,9 VSS
*       
*       
* Notes:
* Baud rate is 115200.
* 
* Up to 3 sensors are supported.
* 
* Serial output format is:
* <timestamp>,<temp 1>,<pressure 1>,<temp 2>,<pressure 2>,<temp 3>,<pressure 3>
* 
* The temperature is in degrees Celsius and the pressure is in hPa.
*/

// the sensor communicates using SPI, so include the library:
#include <SPI.h>

//#define SIM_SENSOR_2
//#define SIM_SENSOR_3

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

unsigned long previousMillis = 0;
const long interval = 100;          // 100 ms sample period

typedef enum
{
    SENSOR_1,
    SENSOR_2,
    SENSOR_3,
    MAX_SENSORS
} sensor_ID_t;

// pins used for the connection with the sensor
// the other you need are controlled by the SPI library):
const int chipSelects[3] = {8, 9, 10};

typedef struct 
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
} bmp3_quantized_calib_data;

bmp3_quantized_calib_data cal_data[3] = {0};


void setup() {
  Serial.begin(115200);

  // start the SPI library:
  SPI.begin();

  // initalize the chip select pins:
  pinMode(chipSelects[SENSOR_1], OUTPUT);
  pinMode(chipSelects[SENSOR_2], OUTPUT);
  pinMode(chipSelects[SENSOR_3], OUTPUT);

  // briefly assert chip select in order to configure BMP-388 for SPI mode
  digitalWrite(chipSelects[SENSOR_1], LOW);
  digitalWrite(chipSelects[SENSOR_2], LOW);
  digitalWrite(chipSelects[SENSOR_3], LOW);
  delay(10);
  digitalWrite(chipSelects[SENSOR_1], HIGH);
  digitalWrite(chipSelects[SENSOR_2], HIGH);
  digitalWrite(chipSelects[SENSOR_3], HIGH);

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // 1 MHz clock

  //put to sleep (soft reset)
  writeRegister(SENSOR_1, PWR_CTRL, 0x00);
  writeRegister(SENSOR_2, PWR_CTRL, 0x00);
  writeRegister(SENSOR_3, PWR_CTRL, 0x00);

  delay(10);

  //get calibration data
  readRegisters(SENSOR_1, NVM_PAR_T1_7_0, CAL_DATA_READ_LEN, rxBuffer);
  parse_calib_data(&rxBuffer[2], &cal_data[SENSOR_1]);

  readRegisters(SENSOR_2, NVM_PAR_T1_7_0, CAL_DATA_READ_LEN, rxBuffer);
  parse_calib_data(&rxBuffer[2], &cal_data[SENSOR_2]);

  readRegisters(SENSOR_3, NVM_PAR_T1_7_0, CAL_DATA_READ_LEN, rxBuffer);
  parse_calib_data(&rxBuffer[2], &cal_data[SENSOR_3]);
  
  //Configure BMP-388:
  writeRegister(SENSOR_1, PWR_CTRL, 0x33);  // enable temp, enable press, normal mode
  writeRegister(SENSOR_2, PWR_CTRL, 0x33);  // enable temp, enable press, normal mode
  writeRegister(SENSOR_3, PWR_CTRL, 0x33);  // enable temp, enable press, normal mode

  SPI.endTransaction();

  Serial.println("timestamp,temp 1,press 1,temp 2,press 2,temp 3,press 3");
  
  // give the sensor time to set up:
  delay(500);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    double temperature[3] = {0};
    double pressure[3] = {0};

    getPressureSensorData(SENSOR_1, &temperature[SENSOR_1], &pressure[SENSOR_1]);
    
    #ifndef SIM_SENSOR_2
      getPressureSensorData(SENSOR_2, &temperature[SENSOR_2], &pressure[SENSOR_2]);
    #else
      simTriangle(SENSOR_2, &temperature[SENSOR_2], &pressure[SENSOR_2]);
    #endif

    #ifndef SIM_SENSOR_3
      getPressureSensorData(SENSOR_3, &temperature[SENSOR_3], &pressure[SENSOR_3]);
    #else
      simTriangle(SENSOR_3, &temperature[SENSOR_3], &pressure[SENSOR_3]);
    #endif 
    
    // display the sensor data
    Serial.print(currentMillis);
    Serial.print(",");
    Serial.print(temperature[SENSOR_1]);
    Serial.print(",");
    Serial.print(pressure[SENSOR_1] / 100.0);
    Serial.print(",");
    Serial.print(temperature[SENSOR_2]);
    Serial.print(",");
    Serial.print(pressure[SENSOR_2] / 100.0);
    Serial.print(",");
    Serial.print(temperature[SENSOR_3]);
    Serial.print(",");
    Serial.println(pressure[SENSOR_3] / 100.0);

    previousMillis = currentMillis;
  }
}

unsigned int readRegisters(sensor_ID_t sensor, byte thisRegister, int bytesToRead, byte* rxbuff)
{
  unsigned int result = 0;   // result to return
  int i = 0;

  // now combine the address and the command into one byte
  rxbuff[0] = thisRegister | READ;

  // take the chip select low to select the device:
  digitalWrite(chipSelects[sensor], LOW);
  
  // send the device the register you want to read:
  SPI.transfer(rxbuff, bytesToRead+2);

  // take the chip select high to de-select:
  digitalWrite(chipSelects[sensor], HIGH);
  
  // return the result:
  return (result);
}


void writeRegister(sensor_ID_t sensor, byte thisRegister, byte thisValue)
{
  // now combine the register address and the command into one byte:
  byte dataToSend = thisRegister & WRITE;

  // take the chip select low to select the device:
  digitalWrite(chipSelects[sensor], LOW);

  SPI.transfer(dataToSend); //Send register location
  SPI.transfer(thisValue);  //Send value to record into register

  // take the chip select high to de-select:
  digitalWrite(chipSelects[sensor], HIGH);
}

unsigned int getPressureSensorData(sensor_ID_t sensor, double* temperature, double* pressure)
{
  unsigned long temperatureData = 0;
  unsigned long pressureData = 0;

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // 1 MHz clock

  readRegisters(sensor, DATA_0, SENSOR_DATA_READ_LEN, rxBuffer);

  SPI.endTransaction();

  pressureData = (unsigned long)rxBuffer[2];
  pressureData |= (unsigned long)rxBuffer[3] << 8;
  pressureData |= (unsigned long)rxBuffer[4] << 16;

  temperatureData = (unsigned long)rxBuffer[5];
  temperatureData |= (unsigned long)rxBuffer[6] << 8;
  temperatureData |= (unsigned long)rxBuffer[7] << 16;

  *temperature = compensate_temperature(temperatureData, &cal_data[sensor]);
  *pressure = compensate_pressure(pressureData, &cal_data[sensor]);
}

unsigned int simTriangle(sensor_ID_t sensor, double* temperature, double* pressure)
{
  static bool up[MAX_SENSORS] = {true,true,true};
  static int i[MAX_SENSORS] = {0, 15, 30};
  static double pres[MAX_SENSORS] = {101300.0,102800.0,104300.0};
  static double temp[MAX_SENSORS] = {23.3,26.3,29.3};

  if (up[sensor])
  {
    if (i[sensor] > 40)
      up[sensor] = false;

    pres[sensor] = pres[sensor] + 100;
    temp[sensor] = temp[sensor] + 0.02;
      
    i[sensor]++;
  }
  else
  {
    if (i[sensor] < 0)
      up[sensor] = true;

    pres[sensor] = pres[sensor] - 100;
    temp[sensor] = temp[sensor] - 0.02;
    
    i[sensor]--;
  }

  *temperature = temp[sensor];
  *pressure = pres[sensor];
}

static void parse_calib_data(const uint8_t *raw_cal, bmp3_quantized_calib_data *cal)
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

static double compensate_temperature(unsigned long uncomp_temperature, bmp3_quantized_calib_data *cal)
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

static double compensate_pressure(unsigned long uncomp_pressure_in, const bmp3_quantized_calib_data *cal)
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
