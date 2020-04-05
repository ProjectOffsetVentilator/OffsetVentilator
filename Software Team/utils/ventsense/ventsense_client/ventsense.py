# ventsense.py
# v0.1-1
# Created 4/03/2020
# Clay Gilmore
# Helpful Engineering
#
# Purpose:
# Reads streaming temperature and pressure sensor data from serial port and saves it to a Comma
# Separated Variable (.csv) file. It is intended to interface with an Arduino reading sensor
# data from a BMP-388. 
#
# Requirements:
#   Hardware
#     1x Arduino
#     1x BMP-388
#     USB cable
#     Computer
#     
#   Software
#     ventsense_fw.ino v0.1-x
#     either
#        pre-built ventsense executable for Windows or Linux (located in bin folder)
#     or
#        ventsense.py
#        Python (tested on v2.7.10 and v3.6.8)
#        pySerial package
#
# Setup:
# Wire BMP-388 to Arduino, per the Circuit section in the ventsense_fw readme. Connect Arduino 
# to computer via USB cable. Load Arduino with this ventsense_fw. Launch ventsense.py from the 
# command line, specifying the serial port name (e.g. something like "COM10" on Windows or 
# "/dev/ttyACM0" on Linux). Optionally, the user can have the live sensor data printed to the 
# console, as well. See the printHelp() function below or execute this script with the "-h" 
# info on command line arguments. Example:
#     python ventsense.py -p COM10
#
# Notes:
# Press CTRL+C to exit.
#
# Look up the correct serial port name before executing the script. It will fail if given a
# wrong or invalid serial port.
# 
# This script works best if it is already running when the Arduino starts, as otherwise it starts
# listening to the serial data mid-stream, which can result in capturing only a fragment of the first 
# line. However, this script cannot be running when you program the Arduino, as it will conflict with 
# the Arduino IDE over serial port access. So, the best thing to do is load the Arduino, then run this 
# app, and then reset the Arduino.
# 
# A .csv file will be created when you launch this script and then another one will be created each 
# time you reset the Arduino.
#
# The .csv files are named as follows, based on the date and time at creation:
#     ventsense_log_<YYYY-MM-DD_hhmmss>.csv

import serial
import time
import csv
import sys
import getopt
from io import open

list = []

def printHelp():
    print ('ventsense_client v0.1-1')
    print ('')
    print ('usage: python ventsense.py -p <serial port> [-c]')
    print ('')
    print ('options:')
    print ('    -c                   echo sensor data to the console')
    print ('    -h                   help. I.e., print this screen')
    print ('    -p <serial port>     name of serial port Arduino is attached to (required)')

def startNewLogFile():
    timestr = time.strftime("%Y-%m-%d_%Hh%Mm%Ss")
    csv_str = "ventsense_log_" + timestr + ".csv"

    return open(csv_str, "a");
    

def main(argv):
    ser_str = ''
    serial_port_name = ''
    console_output = False
    first_read = True

    #parse command line options
    try:
        opts, args = getopt.getopt(argv,"hcp:")
    except getopt.GetoptError:
        printHelp()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            printHelp()
            sys.exit()
        elif opt == '-p':
            serial_port_name = arg
        elif opt == '-c':
            console_output = True

    if not serial_port_name:
        printHelp()
        sys.exit(2)

    else:
        #open serial port
        ser = serial.Serial(serial_port_name, 115200)
        ser.flushInput()

        file = startNewLogFile()

        #continually listen to serial stream and pass it through to CSV log file
        while True:
            try:
                #read serial data
                ser_bytes = ser.readline()
                
                #check whether running python 3. If so, convert bytes to string
                if (sys.version_info > (3, 0)):
                    ser_str = ser_bytes.decode('utf-8')
                elif:
                    ser_str = ser_bytes

                #remove whitespace and line endings
                ser_str = ser_str.strip()

                #echo data on console, if requested
                if console_output:
                    print(ser_str)

                #if Arduino resets while listening, then start a new log file
                if ser_str[0] == 't':
                    file.close()
                    file = startNewLogFile()
                #if Arduino was already running when we started listening, write 
                #table heading to log file
                elif first_read:
                    file.write("timestamp,temp 1,press 1")
                    
                first_read = False
                
                #log data to CSV
                file.write(ser_str + '\n')

            except KeyboardInterrupt:
                #user has exited with CTRL+C
                print("Exiting...")
                break
            except Exception as e:
                #any other exception
                print(e)
                break

        file.close()


if __name__ == "__main__":
    main(sys.argv[1:])
