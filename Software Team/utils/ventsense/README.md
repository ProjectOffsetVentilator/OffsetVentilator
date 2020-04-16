## Ventsense Software

### Purpose
#### Ventsense Firmware
This code runs on an Arduino Uno, reading temperature and pressure data from a BMP-388 sensor 
via the SPI interface at a 10 Hz rate and sending it out on the serial port. Calibration and 
temperature compensation is performed on the data prior to sending.

#### Ventsense Client
This code runs on a PC that is connected to the Arduino via USB cable. It reads streaming 
temperature and pressure sensor data from serial port and saves it to a Comma-Separated 
Value (.csv) file. 


### Requirements
Hardware  
   - 1x Arduino Uno
   - 3x BMP-388
   - USB cable
   - Computer
    
Software  
   - Arduino v1.8.5 or later
   - ventsense_fw.ino v0.2-x
   - either
     - pre-built ventsense client executable (v2.0-x) for Windows or Linux (located in bin folder)
   - or
     - ventsense.py v0.2-x
     - Python (v3.5.2 or later)
     - pySerial package
	 - numpy package
	 - matplotlib package
    
### Setup
Wire BMP-388 to Arduino, per the Circuit section, below. Connect Arduino to computer via USB cable.

#### Ventsense Firmware
Load Arduino with the ventsense firmware.

#### Ventsense Client
Launch ventsense.py (or pre-built executable) from the command line, specifying the serial port name 
(e.g. something like "COM10" on Windows or "/dev/ttyACM0" on Linux). Optionally, the user can have 
the live sensor data printed to the console, as well. To see command line options, execute ventsense
client with the "-h" option. Examples:  
- Linux executable:  
   `./ventsense -p "/dev/ttyACM0"`  
- Windows executable:  
   `ventsense.exe -p "COM11"`  
- Python script:  
   `python ventsense.py -p COM10`


### Circuit
Level-shifting is required to safely interface the 5V Arduino with the 3.3V BMP-388 on the following 
pins (SCK, SDI, CSB). Also, the module I am using has a 10K pullup resistor on SDO. Not sure if it is 
required. Example level-shifting circuit:
```
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
``` 
      
### Notes:
Baud rate is 115200.

Press CTRL+C to exit ventsense client.
 
Supports up to 3 sensors.

Serial output format is:
<timestamp>,<temp 1>,<pressure 1>,<temp 2>,<pressure 2>,<temp 3>,<pressure 3>

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

