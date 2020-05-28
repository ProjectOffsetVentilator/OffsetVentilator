# OffsetVentilator - Software

## Google drive files

Code specifications (draft) https://docs.google.com/document/d/1Vykw4cnE0Rh32LuTK7wvXugAaXiXU2Qhy337S_AlgQQ/edit?usp=sharing (Anyone can comment. Contact @ammondayley on slack for edit privileges)

## Resources and Learning 

Alternative to delay() https://www.arduino.cc/reference/en/language/functions/time/millis/

Blink without delay sketch https://www.arduino.cc/en/Tutorial/BlinkWithoutDelay

## Software
**Ventsense** - utility for logging pressure and temperature data from BMP-388 sensor. Consists of firmware to run on 
an Arduino Uno (ventsense_fw), as well as software to run on a host PC connected to the Arduino by a USB cable 
(ventsense_client). The ventsense firmware interfaces with up to 3 BMP-388 chips and streams their data over serial 
at a 10 Hz rate. The ventsense client receives the data over serial and logs it to a Comma-Seperated Value (.csv) file. 
Located at: utils/ventsense/

**OffsetVentilatorControl** - is a first draft of a control software, based on some software from the OpenVent Bristol project.
Currently it does some pressure controlled ventilation. The software is current just tested on a testrig, with to little power for the motor. PID values may be off...

**OffsetVentilatorLibrary** - currently contains a class to handle our sensor cluster, and is used to calculate flow based on the variable orifice design we currently are testing.
