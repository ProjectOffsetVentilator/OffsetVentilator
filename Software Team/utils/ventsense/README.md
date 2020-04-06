##Ventsense Software

###Purpose
####Ventsense Firmware
This code runs on an Arduino Uno, reading temperature and pressure data from a BMP-388 sensor 
via the SPI interface at a 10 Hz rate and sending it out on the serial port. Calibration and 
temperature compensation is performed on the data prior to sending.

####Ventsense Client
This code runs on a PC that is connected to the Arduino via USB cable. It reads streaming 
temperature and pressure sensor data from serial port and saves it to a Comma-Separated 
Value (.csv) file. 


###Requirements
Hardware
   1x Arduino Uno
   1x BMP-388
   USB cable
   Computer
    
Software
   Arduino v1.8.5 or later
   ventsense_fw.ino v0.1-x
   either
      pre-built ventsense client executable for Windows or Linux (located in bin folder)
   or
      ventsense.py
      Python (tested on v2.7.10 and v3.6.8)
      pySerial package
    
###Setup
Wire BMP-388 to Arduino, per the Circuit section, below. Connect Arduino to computer via USB cable.

####Ventsense Firmware
Load Arduino with the ventsense firmware.

####Ventsense Client
Launch ventsense.py (or pre-built executable) from the command line, specifying the serial port name 
(e.g. something like "COM10" on Windows or "/dev/ttyACM0" on Linux). Optionally, the user can have 
the live sensor data printed to the console, as well. To see command line options, execute ventsense
client with the "-h" option. Examples:
   Linux executable:
      ./ventsense -p "/dev/ttyACM0"
   Windows executable:
      ventsense.exe -p "COM11"
   Python script:
      python ventsense.py -p COM10


###Circuit
Level-shifting is required to safely interface the 5V Arduino with the 3.3V BMP-388 on the following 
pins (SCK, SDI, CSB). Also, the module I am using has a 10K pullup resistor on SDO. Not sure if it is 
required. Example level-shifting circuit:
 
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
Arduino             BMP-388
Pin#  Name          Pin#  Name
10    CS ---------- 6     CSB         (level-shifting required)
11    MOSI -------- 4     SDI         (level-shifting required)
12    MISO -------- 5     SDO
13    SCK --------- 2     SCK         (level-shifting required)
 -    3.3V -------- 1,10  VDDIO,VDD
 -    GND --------- 3,8,9 VSS
      
      
Notes:
Baud rate is 115200.

Press CTRL+C to exit ventsense client.
 
Only 1 sensor supported, at the moment.

Serial output format is:
<timestamp>,<temp 1>,<pressure 1>

The temperature is in degrees Celsius and the pressure is in hPa.

Look up the correct serial port name before executing the ventsense client. It will fail if given a
wrong or invalid serial port.

Ventsense client works best if it is already running when the Arduino starts, as otherwise it starts
listening to the serial data mid-stream, which can result in capturing only a fragment of the first 
line. However, this script cannot be running when you program the Arduino, as it will conflict with 
the Arduino IDE over serial port access. So, the best thing to do is load the Arduino, then run this 
app, and then reset the Arduino.

A .csv file will be created when you launch the client and then another one will be created each 
time you reset the Arduino.

The .csv files are named as follows, based on the date and time at creation:
    ventsense_log_<YYYY-MM-DD_hhmmss>.csv 

