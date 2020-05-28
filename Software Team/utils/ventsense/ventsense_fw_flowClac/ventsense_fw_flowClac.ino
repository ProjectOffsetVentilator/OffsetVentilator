/**
  Adapted in part from the following example provided with the Arduino IDE:
  SCP1000 Barometric Pressure Sensor Display
  created 31 July 2010
  modified 14 August 2010
  by Tom Igoe
  ---------------------------------------------------------------------------
  Adapted in part from code provided by the manufacturer for the BMP-388 sensor:
  Copyright (c) 2020 Bosch Sensortec GmbH. All rights reserved.

  BSD-3-Clause

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
  ---------------------------------------------------------------------------
  ventsense_fw.ino
  v0.2-0
  Created 4/3/2020
  Clay Gilmore
  Helpful Engineering

  Purpose:
  This code reads temperature and pressure data from a bank of 3 BMP-388 sensors
  via the SPI interface at a 10 Hz rate and sends it out on the serial port.
  Calibration and temperature compensation is performed on the data prior to
  sending.

  Requirements:
    Hardware
      1x Arduino Uno
      3x BMP-388
      USB cable
      Computer

    Software
      ventsense client v0.2-x (or serial terminal app)
      Arduino v1.8.5 or later

  Setup:
  Wire BMP-388 to Arduino, per the Circuit section, below. Connect Arduino
  to computer via USB cable. Load Arduino with this code. Launch ventsense.py
  or open a serial terminal app.

  Circuit:
  Level-shifting is required to safely interface the 5V Arduino with the
  3.3V BMP-388 on the following pins (SCK, SDI, CSB). If you are using an
  evaluation module, check its specifications to see whether it already does
  the level shifting for you. If not, or if you are using the Offset Ventilator
  sensor board, you will need to add your own level-shifting circuitry if you
  are interfacing it with an off-the-shelf Arduino Uno. Also, the module
  I am using has a 10K pullup resistor on SDO. Not sure if it is required.
  Example level-shifting circuit:

         Arduino output
              |
              \
              / R1
              \ 1K
              /
              |
              +----->> BMP-388 input
              |
              \
              / R2
              \ 2K
              /
              |
             GND

  Connect the pins as follows:
  Arduino                       BMP-388 #1           BMP-388 #2            BMP-38 #3
  Pin#  Name                    Pin#  Name           Pin#  Name            Pin#  Name
   8    CS1 ---<level shift>--- 6     CSB
   9    CS2 ---<level shift>------------------------ 6     CSB
  10    CS3 ---<level shift>---------------------------------------------- 6     CSB
  11    MOSI --<level shift>--- 4     SDI ---------- 4     SDI ----------- 4     SDI
  12    MISO ------------------ 5     SDO ---------- 5     SDO ----------- 5     SDO
  13    SCK ---<level shift>--- 2     SCK ---------- 2     SCK ----------- 2     SCK
   -    3.3V ------------------ 1,10  VDDIO,VDD ---- 1,10  VDDIO,VDD ----- 1,10  VDDIO,VDD
   -    GND ------------------- 3,8,9 VSS ---------- 3,8,9 VSS ----------- 3,8,9 VSS


  Notes:
  Baud rate is 115200.

  Up to 3 sensors are supported.

  Serial output format is:
  <timestamp>,<temp 1>,<pressure 1>,<temp 2>,<pressure 2>,<temp 3>,<pressure 3>

  The temperature is in degrees Celsius and the pressure is in hPa.
*/

//#include "SFM3200.h"
#include <Wire.h>
#include "SFM3X00.h"

#include "SensorClusterHandler.h"
#include "PumpControl.h"

#define FLOW_SENSOR_ADDRESS 0x40

SFM3X00 sfm3200(FLOW_SENSOR_ADDRESS);  // I2C pins A4 & A5

SensorClusterHandler sensorCluster(7, 8, 9); // SPI PINS & CS1-3
PumpControl pumpControl(10); // PWM pin

unsigned long previousMillis = 0;
const long interval = 20;          // 100 ms sample period

bool doCalcDiff = false;

const float filterAmt = 0.0;
float flowCalcFiltered;
float flowSFMFiltered;


void setup() {
  //sfm3200.init();
  Wire.begin();
  sfm3200.begin();
  sensorCluster.begin();
  pumpControl.init();

  Serial.begin(115200);
  Serial.println("timestamp,temp 1,press 1,temp 2,press 2,temp 3,press 3, flowSFM, flowCalc, differentialPressure, relativePressureA, relativePressureB, relativePressureC");
}

void loop() {

  if (Serial.available() > 0) {
    int receivedChar = Serial.read();
    if (receivedChar == 'x') {
      pumpControl.toggleProcedure();
    }
    else if (receivedChar == 'c') {
      doCalcDiff = true;
    }
    else  pumpControl.setPwm(receivedChar);
  }


  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    pumpControl.update();

    // sfm3200.readSensor();
    flowSFMFiltered = flowSFMFiltered * filterAmt + (1 - filterAmt) * sfm3200.readFlow();

    sensorCluster.readSensor(20, doCalcDiff);
    if (doCalcDiff == true) doCalcDiff = false;

    flowCalcFiltered = sensorCluster.getFlow();

    // display the sensor data
    Serial.print(currentMillis);
    Serial.print(",");
    Serial.print(sensorCluster.getTemperature(SENSOR_1));
    Serial.print(",");
    Serial.print(sensorCluster.getPressure(SENSOR_1) / 100);
    Serial.print(",");
    Serial.print(sensorCluster.getTemperature(SENSOR_2));
    Serial.print(",");
    Serial.print(sensorCluster.getPressure(SENSOR_2) / 100);
    Serial.print(",");
    Serial.print(sensorCluster.getTemperature(SENSOR_3));
    Serial.print(",");
    Serial.print(sensorCluster.getPressure(SENSOR_3) / 100);
    Serial.print(",");
    Serial.print(flowSFMFiltered);
    Serial.print(",");
    Serial.print(flowCalcFiltered);
    Serial.print(",");
    Serial.print(sensorCluster.getDifferentialPressure(),6);
    Serial.print(",");
    Serial.print(sensorCluster.getRelativePressureA(),6);
    Serial.print(",");
    Serial.print(sensorCluster.getRelativePressureB(),6);
    Serial.print(",");
    Serial.print(sensorCluster.getRelativePressureC(),6);
    Serial.println();

    previousMillis = currentMillis;
  }
}
