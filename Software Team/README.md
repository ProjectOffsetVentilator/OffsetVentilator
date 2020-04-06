# OffsetVentilator - Software

## Google drive files

Code specifications (draft) https://docs.google.com/document/d/1Vykw4cnE0Rh32LuTK7wvXugAaXiXU2Qhy337S_AlgQQ/edit?usp=sharing (Anyone can comment. Contact @ammondayley on slack for edit privileges)

## Resources and Learning 

Alternative to delay() https://www.arduino.cc/reference/en/language/functions/time/millis/

Blink without delay sketch https://www.arduino.cc/en/Tutorial/BlinkWithoutDelay

Hobby Servo control https://www.youtube.com/watch?v=kUHmYKWwuWs

Hobby servo teardown https://youtu.be/OcgF4lYRHnc?t=47

## Software
Ventsense - utility for logging pressure and temperature data from BMP-388 sensor. Consists of firmware to run on 
an Arduino Uno (ventsense_fw), as well as software to run on a host PC connected to the Arduino by a USB cable 
(ventsense_client). The ventsense firmware interfaces with a BMP-388 and streams its data over serial at a 10 Hz 
rate. The ventsense client receives the data over serial and logs it to a Comma-Seperated Value (.csv) file. 
Located at: utils/ventsense/
