# -*- coding: utf-8 -*-
# ventsense.py
# v0.2-1
# Created 4/03/2020
# Clay Gilmore
# Helpful Engineering
#
# Purpose:
# Reads streaming temperature and pressure sensor data from serial port and saves it to a Comma-
# Separated Value (.csv) file. It is intended to interface with an Arduino reading sensor
# data from up to 3 BMP-388 sensors. 
#
# Requirements:
#   Hardware
#     1x Arduino Uno
#     3x BMP-388
#     USB cable
#     Computer
#     
#   Software
#     ventsense_fw v0.2-x
#     either
#        pre-built ventsense client executable (v0.2-x) for Windows or Linux (located in bin folder)
#     or
#        ventsense.py v0.2-x
#        Python (v3.5 or later)
#        pySerial package
#        matplotlib package
#        numpy package
#
# Setup:
# Wire BMP-388 to Arduino, per the Circuit section in the ventsense_fw readme. Connect Arduino 
# to computer via USB cable. Load Arduino with this ventsense_fw. Launch ventsense.py from the 
# command line, specifying the serial port name (e.g. something like "COM10" on Windows or 
# "/dev/ttyACM0" on Linux). Optionally, the user can have the live sensor data printed to the 
# console, as well. See the printHelp() function below or execute this script with the "-h" 
# option for more info on command line arguments. Example:
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
import traceback
from matplotlib.ticker import (MultipleLocator, FormatStrFormatter, AutoLocator, AutoMinorLocator)
import matplotlib.pyplot as plt
import numpy as np
import configparser

SW_VERSION = 'v0.2-1'

SENSOR_1 = 0
SENSOR_2 = 1
SENSOR_3 = 2
MAX_SENSORS = 3

TEMP_IDX = 0
PRESS_IDX = 1

SAMPLE_RATE = 10.0 #Hz

#if running python 3, import open
if (sys.version_info > (3, 0)):
    from io import open

list = []

def printHelp():
    print ('ventsense_client ' + SW_VERSION)
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
    ser_str = u''
#    serial_port_name = ''
#    console_output = False
#    plot_enabled = True
    first_read = True
    units_cmh2o = False
#    combined_plot = False
#    relative_plot = False
#    atmospheric_sensor = SENSOR_1
#    x_width = 200.0
#    x_upper_bound_press = x_width / SAMPLE_RATE
#    x_lower_bound_press = 0.0 / SAMPLE_RATE
#    x_upper_bound_temp = x_width / SAMPLE_RATE
#    x_lower_bound_temp = 0.0 / SAMPLE_RATE
#    y_upper_bound_press = 1080
#    y_lower_bound_press = 1000
#    y_upper_bound_temp = 35
#    y_lower_bound_temp = 15
    
    #attempt to read config file
    config = configparser.ConfigParser()
    config.read('settings.ini')
    console_output = config.getboolean('SETTINGS', 'console_output', fallback=False)
    plot_enabled = config.getboolean('SETTINGS', 'plot_enabled', fallback=True)
    combined_plot = config.getboolean('SETTINGS', 'combined_plot', fallback=False)
    relative_plot = config.getboolean('SETTINGS', 'relative_plot', fallback=False)
    atmospheric_sensor = config.getint('SETTINGS', 'atmospheric_sensor', fallback=1)    
    x_width = config.getint('SETTINGS', 'x_width', fallback=200)
    y_upper_bound_press = config.getint('SETTINGS', 'pressure_y_upper_bound', fallback=1080)
    y_lower_bound_press = config.getint('SETTINGS', 'pressure_y_lower_bound', fallback=1000)
    y_upper_bound_temp = config.getint('SETTINGS', 'temperature_y_upper_bound', fallback=35)
    y_lower_bound_temp = config.getint('SETTINGS', 'temperature_y_lower_bound', fallback=15)
    units_str = config.get('SETTINGS', 'pressure_units', fallback='cmh2o')
    serial_port_name = config.get('SETTINGS', 'serial_port', fallback=None)
    
    if units_str in ('cmh2o', 'cmH2O'):
        units_cmh2o = True
    
    if (atmospheric_sensor > 3) or (atmospheric_sensor < 1):
        atmospheric_sensor = 1
    atmospheric_sensor = atmospheric_sensor - 1
    
    if (x_width > 10000) or (x_width < 0):
        x_width = 200
        
    x_upper_bound_press = float(x_width) / SAMPLE_RATE
    x_lower_bound_press = 0.0 / SAMPLE_RATE
    x_upper_bound_temp = float(x_width) / SAMPLE_RATE
    x_lower_bound_temp = 0.0 / SAMPLE_RATE
    
    #parse command line options
    try:
        opts, args = getopt.getopt(argv,"hcrwp:a:", ["combined", "relative", "atmospheric=", "press-y-max=", "press-y-min=", "temp-y-max=", "temp-y-min=", "x-width=", "cmh2o", "noplot"])
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
        elif opt == '-p':
            serial_port_name = arg
        elif opt == '--combined':
            combined_plot = True
        elif opt in ('-r', '--relative'):
            relative_plot = True
        elif opt in ('-a', '--atmospheric'):
            if (arg == '1'):
                atmospheric_sensor = SENSOR_1
            elif (arg == '2'):
                atmospheric_sensor = SENSOR_2
            elif (arg == '3'):
                atmospheric_sensor = SENSOR_3
            else:
                print("Sensor value be equal to 1, 2, or 3. Invalid sensor: " + arg)
                printHelp()
                sys.exit()
        elif opt == '--press-y-max':
            y_upper_bound_press = float(arg)
        elif opt == '--press-y-min':
            y_lower_bound_press = float(arg)
        elif opt == '--temp-y-max':
            y_upper_bound_temp = float(arg)
        elif opt == '--temp-y-min':
            y_lower_bound_temp = float(arg)
        elif opt in ('-w', '--x-width'):
            x_width = int(arg)
            if (x_width > 10000) or (x_width < 0):
                print("x-width value must be between 0 and 10000")
                sys.exit()
            x_upper_bound_press = float(x_width) / SAMPLE_RATE
            x_lower_bound_press = 0.0 / SAMPLE_RATE
            x_upper_bound_temp = float(x_width) / SAMPLE_RATE
            x_lower_bound_temp = 0.0 / SAMPLE_RATE
        elif opt in ('--cmh2o', '--cmH2O'):
            units_cmh2o = True
        elif opt == '--noplot':
            plot_enabled = False
        else:
            print("Unknown argument: " + opt)
            printHelp()
            sys.exit()

    if not serial_port_name:
        print('Please specify serial port with -p option on command line or with "SerialPort" value in settings.ini')
        printHelp()
        sys.exit(2)

    else:
        #save settings
        units_str = 'hpa'
        if units_cmh2o:
            units_str = 'cmh2o'
            
        if (not config.has_section('SETTINGS')):
            config.add_section('SETTINGS')
            
        config.set('SETTINGS', 'console_output', str(console_output))
        config.set('SETTINGS', 'plot_enabled', str(plot_enabled))
        config.set('SETTINGS', 'combined_plot', str(combined_plot))
        config.set('SETTINGS', 'relative_plot', str(relative_plot))
        config.set('SETTINGS', 'atmospheric_sensor', str(atmospheric_sensor))    
        config.set('SETTINGS', 'x_width', str(x_width))
        config.set('SETTINGS', 'pressure_y_upper_bound', str(y_upper_bound_press))
        config.set('SETTINGS', 'pressure_y_lower_bound', str(y_lower_bound_press))
        config.set('SETTINGS', 'temperature_y_upper_bound', str(y_upper_bound_temp))
        config.set('SETTINGS', 'temperature_y_lower_bound', str(y_lower_bound_temp))
        config.set('SETTINGS', 'pressure_units', units_str)
        config.set('SETTINGS', 'serial_port', serial_port_name)
        
        with open('settings.ini', 'w') as configfile:
            config.write(configfile)
        
        #open serial port (we have to use this roundabout way of opening the serial port in order to avoid
        #resetting the Arduino upon opening the serial port. See https://github.com/pyserial/pyserial/issues/124
        #for more info)
        ser = serial.serial_for_url(serial_port_name, 115200,rtscts=False,dsrdtr=False,do_not_open=True)
        ser.dtr = 0
        ser.rts = 0
        ser.open()
        ser.flushInput()

        file = startNewLogFile()
        
        fig = None
        axs = [[None, None],[None, None],[None, None]]
        lines = [[None, None],[None, None],[None, None]]
        y_data = [[None, None],[None, None],[None, None]]
        x_data = None
        
        i=0

        #continually listen to serial stream and pass it through to CSV log file
        while True:
            try:
                #read serial data
                ser_bytes = ser.readline()
                
                #if running python 3, convert bytes to string
                if (sys.version_info > (3, 0)):
                    ser_str = ser_bytes.decode('utf-8', 'ignore')

                else:
                    ser_str = ser_bytes

                #remove whitespace and line endings
                ser_str = ser_str.strip()

                #if string is empty, skip the rest
                if ser_str:
                    #echo data on console, if requested
                    if console_output:
                        print(ser_str)

                    #if Arduino resets while listening, then start a new log file
                    if ser_str[0:4] == 'time':
                        file.close()
                        file = startNewLogFile()
                    #if Arduino was already running when we started listening, write 
                    #table heading to log file
                    elif first_read:
                        file.write("timestamp,temp 1,press 1,temp 2,press 2,temp 3,press 3\n")
                        
                    first_read = False
                    
                    #log data to CSV
                    file.write(ser_str + '\n')
                    
                    #plot data
                    str_tokens = ser_str.split(',')

                    if ((len(str_tokens) >= 7) and (ser_str[0:4] != 'time')) and plot_enabled:
                        if i > 0:
                            y_data[SENSOR_1][PRESS_IDX] = [float(str_tokens[2])] + y_data[SENSOR_1][PRESS_IDX]
                            y_data[SENSOR_2][PRESS_IDX] = [float(str_tokens[4])] + y_data[SENSOR_2][PRESS_IDX]
                            y_data[SENSOR_3][PRESS_IDX] = [float(str_tokens[6])] + y_data[SENSOR_3][PRESS_IDX]
                            
                            y_data[SENSOR_1][TEMP_IDX] = [float(str_tokens[1])] + y_data[SENSOR_1][TEMP_IDX]
                            y_data[SENSOR_2][TEMP_IDX] = [float(str_tokens[3])] + y_data[SENSOR_2][TEMP_IDX]
                            y_data[SENSOR_3][TEMP_IDX] = [float(str_tokens[5])] + y_data[SENSOR_3][TEMP_IDX]
                            
                            x_data = np.append(x_data,float(i)/SAMPLE_RATE)
                            
                            for j in range(len(lines)):
                                for k in range(len(lines[j])):
                                    lines[j][k].set_ydata(y_data[j][k])
                                    lines[j][k].set_xdata(x_data)
                                    axs[j][k].draw_artist(axs[j][k].patch)
                                    axs[j][k].draw_artist(lines[j][k])
                                    fig.canvas.blit(axs[j][k].bbox)
                                
                            fig.canvas.flush_events()
                        else:
                            y_data[SENSOR_1][PRESS_IDX] = [str_tokens[2]]
                            y_data[SENSOR_2][PRESS_IDX] = [str_tokens[4]]
                            y_data[SENSOR_3][PRESS_IDX] = [str_tokens[6]]
                            
                            y_data[SENSOR_1][TEMP_IDX] = [str_tokens[1]]
                            y_data[SENSOR_2][TEMP_IDX] = [str_tokens[3]]
                            y_data[SENSOR_3][TEMP_IDX] = [str_tokens[5]]
                            
                            x_data = np.array(float(i))
        
                            fig, axs = plt.subplots(MAX_SENSORS, 2, figsize=(10,6), gridspec_kw={'width_ratios': [1, 5]})
                            
                            fig.subplots_adjust(hspace=.5)
                            
                            lines[SENSOR_1][PRESS_IDX], = axs[SENSOR_1][PRESS_IDX].plot(x_data, y_data[SENSOR_1][PRESS_IDX],'tab:red')
                            lines[SENSOR_2][PRESS_IDX], = axs[SENSOR_2][PRESS_IDX].plot(x_data, y_data[SENSOR_2][PRESS_IDX],'tab:green')
                            lines[SENSOR_3][PRESS_IDX], = axs[SENSOR_3][PRESS_IDX].plot(x_data, y_data[SENSOR_3][PRESS_IDX],'tab:blue')
                            
                            lines[SENSOR_1][TEMP_IDX], = axs[SENSOR_1][TEMP_IDX].plot(x_data, y_data[SENSOR_1][TEMP_IDX],'tab:pink')
                            lines[SENSOR_2][TEMP_IDX], = axs[SENSOR_2][TEMP_IDX].plot(x_data, y_data[SENSOR_2][TEMP_IDX],'tab:olive')
                            lines[SENSOR_3][TEMP_IDX], = axs[SENSOR_3][TEMP_IDX].plot(x_data, y_data[SENSOR_3][TEMP_IDX],'tab:cyan')
                            
                            axs[SENSOR_1][PRESS_IDX].set_title('Pressure 1')
                            axs[SENSOR_2][PRESS_IDX].set_title('Pressure 2')
                            axs[SENSOR_3][PRESS_IDX].set_title('Pressure 3')
                            
                            axs[SENSOR_2][PRESS_IDX].set_ylabel('hPa')
                            axs[SENSOR_2][PRESS_IDX].yaxis.set_label_position("right")
                            axs[SENSOR_3][PRESS_IDX].set_xlabel('t - seconds')
                            
                            axs[SENSOR_1][TEMP_IDX].set_title('Temperature 1')
                            axs[SENSOR_2][TEMP_IDX].set_title('Temperature 2')
                            axs[SENSOR_3][TEMP_IDX].set_title('Temperature 3')
                            axs[SENSOR_2][TEMP_IDX].set_ylabel('°C')
                            
                            for j in range(len(axs)):
                                axs[j][PRESS_IDX].set_xlim(x_upper_bound_press, x_lower_bound_press)
                                axs[j][PRESS_IDX].set_ylim(y_lower_bound_press, y_upper_bound_press)
                                axs[j][TEMP_IDX].set_xlim(x_upper_bound_temp, x_lower_bound_temp)
                                axs[j][TEMP_IDX].set_ylim(y_lower_bound_temp, y_upper_bound_temp)
                                
                                axs[j][PRESS_IDX].yaxis.set_major_locator(AutoLocator())
                                axs[j][PRESS_IDX].yaxis.set_major_formatter(FormatStrFormatter('%d'))
                                axs[j][PRESS_IDX].yaxis.set_minor_locator(AutoMinorLocator())
                                axs[j][PRESS_IDX].yaxis.tick_right()
                                axs[j][PRESS_IDX].xaxis.set_minor_locator(AutoMinorLocator())
                                
                                axs[j][TEMP_IDX].yaxis.set_major_locator(AutoLocator())
                                axs[j][TEMP_IDX].yaxis.set_major_formatter(FormatStrFormatter('%d'))
                                axs[j][TEMP_IDX].yaxis.set_minor_locator(AutoMinorLocator())
                            
                            plt.show(block=False)
                            
                            fig.canvas.set_window_title('Ventsense ' + SW_VERSION)
   
                            fig.canvas.draw()
                            
                        i+=1


            except KeyboardInterrupt:
                #user has exited with CTRL+C
                print("Exiting...")
                break
            except Exception as e:
                #any other exception
                print(e)
                print(traceback.format_exc())
                break

        file.close()


if __name__ == "__main__":
    main(sys.argv[1:])
