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

ATMOSPHERIC_BASELINE = 1013 #hPa at sea level

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
    print ('    -a, --atmospheric=<sensor ID>   Set the sensor which is used as the base for relative pressure plot. Must be 1, ' +
           '                                    2, or 3. Defaults to 1')
    print ('    -c, --show-console=<true/false> If true, echo sensor data to the console. Else, hide console output')
    print ('    -d, --show-plot=<true/false>    If true, draw plot of sensor data in a new window. Else, hide plot')
    print ('    -h                              Help. I.e., print this screen')
    print ('    -p <serial port>                Name of serial port Arduino is attached to (required)')
    print ('    -r, --relative=<true/false>     If true, plot pressures relative to atmospheric sensor. Else, plot\n' + 
           '                                    absolute pressure values')
    print ('    -w, --x-width=<seconds>         Number of seconds worth of data to display on plot')
    print ('    --combined=<true/false>         If true, plot all sensors on one pressure and one temperature plot, respectively. ' +
           '                                    Else, draw a separate plot for each sensor')
    print ('    --press-y-max=<number>          Set the upper bound on the pressure plot\'s Y axis')
    print ('    --press-y-min=<number>          Set the lower bound on the pressure plot\'s Y axis')
    print ('    --temp-y-max=<number>           Set the upper bound on the temperature plot\'s Y axis')
    print ('    --temp-y-min=<number>           Set the lower bound on the temperature plot\'s Y axis')
    print ('    --use-cmh2o=<true/false>        If true, display pressure data in cmH2O on plot. Else, use hPa')
    

def startNewLogFile():
    timestr = time.strftime("%Y-%m-%d_%Hh%Mm%Ss")
    csv_str = "ventsense_log_" + timestr + ".csv"

    return open(csv_str, "a");
    
def isStrTrue(in_str):
    return in_str.lower() in ['true', 'yes', 'y', '1', 'show', 'enable', 'on']

def main(argv):
    global ATMOSPHERIC_BASELINE
    ser_str = u''
    first_read = True
    units_cmh2o = False
    
    #attempt to read config file
    config = configparser.ConfigParser()
    config.read('settings.ini')
    console_output = config.getboolean('SETTINGS', 'console_output', fallback=False)
    plot_enabled = config.getboolean('SETTINGS', 'plot_enabled', fallback=True)
    combined_plot = config.getboolean('SETTINGS', 'combined_plot', fallback=False)#TODO
    relative_plot = config.getboolean('SETTINGS', 'relative_plot', fallback=False)
    atmospheric_sensor = config.getint('SETTINGS', 'atmospheric_sensor', fallback=1)
    x_width = config.getfloat('SETTINGS', 'x_width', fallback=20.0)
    y_upper_bound_press = config.getfloat('SETTINGS', 'pressure_y_upper_bound', fallback=1100)
    y_lower_bound_press = config.getfloat('SETTINGS', 'pressure_y_lower_bound', fallback=1000)
    y_upper_bound_temp = config.getfloat('SETTINGS', 'temperature_y_upper_bound', fallback=35)
    y_lower_bound_temp = config.getfloat('SETTINGS', 'temperature_y_lower_bound', fallback=15)
    units_cmh2o = config.getboolean('SETTINGS', 'use_cmh2o', fallback=True)
    serial_port_name = config.get('SETTINGS', 'serial_port', fallback=None)
    
    if (atmospheric_sensor > 3) or (atmospheric_sensor < 1):
        atmospheric_sensor = 1
    atmospheric_sensor = atmospheric_sensor - 1
    
    if (x_width > 1000.0) or (x_width < 0):
        x_width = 20.0
        
    x_upper_bound = x_width
    x_lower_bound = 0.0
    
    #parse command line options
    try:
        opts, args = getopt.getopt(argv,"hc:r:w:p:a:d:", ["combined=", "relative=", "atmospheric=", "press-y-max=", "press-y-min=", "temp-y-max=", "temp-y-min=", "x-width=", "use-cmh2o=", "show-plot=","show-console="])
    except getopt.GetoptError:
        printHelp()
        sys.exit(2)
    for opt, arg in opts:
        if opt.lower() == '-h':
            printHelp()
            sys.exit()
        elif opt.lower() == '-p':
            serial_port_name = arg
        elif opt.lower() in ('-c','--show-console'):
            console_output = isStrTrue(arg)
        elif opt.lower() == '-p':
            serial_port_name = arg
        elif opt == '--combined':
            combined_plot = isStrTrue(arg)
        elif opt.lower() in ('-r', '--relative'):
            relative_plot = isStrTrue(arg)
        elif opt.lower() in ('-a', '--atmospheric'):
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
        elif opt.lower() == '--press-y-max':
            y_upper_bound_press = float(arg)
        elif opt == '--press-y-min':
            y_lower_bound_press = float(arg)
        elif opt.lower() == '--temp-y-max':
            y_upper_bound_temp = float(arg)
        elif opt.lower() == '--temp-y-min':
            y_lower_bound_temp = float(arg)
        elif opt.lower() in ('-w', '--x-width'):
            x_width = float(arg)
            if (x_width > 1000.0) or (x_width < 0):
                print("x-width value must be between 0 and 1000")
                sys.exit()
            x_upper_bound = x_width
            x_lower_bound = 0.0
        elif opt.lower() == '--use-cmh2o':
            units_cmh2o = isStrTrue(arg)
        elif opt.lower() in ('-d', '--show-plot'):
            plot_enabled = isStrTrue(arg)
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
        config.set('SETTINGS', 'use_cmh2o', str(units_cmh2o))
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
        axs_idx = [SENSOR_1, SENSOR_2, SENSOR_3]
        lines = [[None, None],[None, None],[None, None]]
        y_data = [[None, None],[None, None],[None, None]]
        x_data = None
        leg = [None, None]
        
        if (combined_plot):
            axs = [[None, None]]
            axs_idx = [SENSOR_1, SENSOR_1, SENSOR_1]
        
        i=0
        
        units_str = 'hPa'
        c = 1.0
        if units_cmh2o:
            units_str = 'cmH2O'
            c = 1.01974
            
        ATMOSPHERIC_BASELINE = ATMOSPHERIC_BASELINE * c

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
                            y_data[SENSOR_1][PRESS_IDX] = [float(str_tokens[2]) * c] + y_data[SENSOR_1][PRESS_IDX]
                            y_data[SENSOR_2][PRESS_IDX] = [float(str_tokens[4]) * c] + y_data[SENSOR_2][PRESS_IDX]
                            y_data[SENSOR_3][PRESS_IDX] = [float(str_tokens[6]) * c] + y_data[SENSOR_3][PRESS_IDX]
                            
                            y_data[SENSOR_1][TEMP_IDX] = [float(str_tokens[1])] + y_data[SENSOR_1][TEMP_IDX]
                            y_data[SENSOR_2][TEMP_IDX] = [float(str_tokens[3])] + y_data[SENSOR_2][TEMP_IDX]
                            y_data[SENSOR_3][TEMP_IDX] = [float(str_tokens[5])] + y_data[SENSOR_3][TEMP_IDX]
                            
                            x_data = np.append(x_data,float(i)/SAMPLE_RATE)
                            
                            if (relative_plot):
                                rel_base = y_data[atmospheric_sensor][PRESS_IDX][0]
                                
                                rel_sen_1 = atmospheric_sensor + 1
                                rel_sen_2 = atmospheric_sensor + 2
                                
                                if rel_sen_1 > 2:
                                    rel_sen_1 = rel_sen_1 - 3
                                if rel_sen_2 > 2:
                                    rel_sen_2 = rel_sen_2 - 3
                                    
                                y_data[rel_sen_1][PRESS_IDX][0] = y_data[rel_sen_1][PRESS_IDX][0] - rel_base
                                y_data[rel_sen_2][PRESS_IDX][0] = y_data[rel_sen_2][PRESS_IDX][0] - rel_base
                                y_data[atmospheric_sensor][PRESS_IDX][0] = y_data[atmospheric_sensor][PRESS_IDX][0] - ATMOSPHERIC_BASELINE
                            
                            for k in (PRESS_IDX, TEMP_IDX):
                                for j in range(len(axs)):
                                    axs[j][k].draw_artist(axs[j][k].patch)
                                
                                for j in range(len(lines)):
                                    lines[j][k].set_ydata(y_data[j][k])
                                    lines[j][k].set_xdata(x_data)
                                    axs[axs_idx[j]][k].draw_artist(lines[j][k])
                                    
                                if combined_plot:
                                    axs[SENSOR_1][k].draw_artist(leg[k])
                                
                                for j in range(len(axs)):
                                    fig.canvas.blit(axs[j][k].bbox)
                            
                            fig.canvas.flush_events()
                        else:
                            y_data[SENSOR_1][PRESS_IDX] = [float(str_tokens[2]) * c]
                            y_data[SENSOR_2][PRESS_IDX] = [float(str_tokens[4]) * c]
                            y_data[SENSOR_3][PRESS_IDX] = [float(str_tokens[6]) * c]
                            
                            y_data[SENSOR_1][TEMP_IDX] = [float(str_tokens[1])]
                            y_data[SENSOR_2][TEMP_IDX] = [float(str_tokens[3])]
                            y_data[SENSOR_3][TEMP_IDX] = [float(str_tokens[5])]
                            
                            x_data = np.array(float(i))
                            
                            if (relative_plot):
                                rel_base = y_data[atmospheric_sensor][PRESS_IDX][0]
                                
                                rel_sen_1 = atmospheric_sensor + 1
                                rel_sen_2 = atmospheric_sensor + 2
                                
                                if rel_sen_1 > 2:
                                    rel_sen_1 = rel_sen_1 - 3
                                if rel_sen_2 > 2:
                                    rel_sen_2 = rel_sen_2 - 3
                                    
                                y_data[rel_sen_1][PRESS_IDX][0] = y_data[rel_sen_1][PRESS_IDX][0] - rel_base
                                y_data[rel_sen_2][PRESS_IDX][0] = y_data[rel_sen_2][PRESS_IDX][0] - rel_base
                                y_data[atmospheric_sensor][PRESS_IDX][0] = y_data[atmospheric_sensor][PRESS_IDX][0] - ATMOSPHERIC_BASELINE
        
                            if (combined_plot):
                                fig, axs2 = plt.subplots(1, 2, figsize=(10,6), gridspec_kw={'width_ratios': [1, 5]})
                                axs[axs_idx[SENSOR_1]] = axs2
                            else:
                                fig, axs = plt.subplots(MAX_SENSORS, 2, figsize=(10,6), gridspec_kw={'width_ratios': [1, 5]})
                            
                            fig.subplots_adjust(hspace=.5)
                            
                            lines[SENSOR_1][PRESS_IDX], = axs[axs_idx[SENSOR_1]][PRESS_IDX].plot(x_data, y_data[SENSOR_1][PRESS_IDX],'tab:red', label='press 1')
                            lines[SENSOR_2][PRESS_IDX], = axs[axs_idx[SENSOR_2]][PRESS_IDX].plot(x_data, y_data[SENSOR_2][PRESS_IDX],'tab:green', label='press 2')
                            lines[SENSOR_3][PRESS_IDX], = axs[axs_idx[SENSOR_3]][PRESS_IDX].plot(x_data, y_data[SENSOR_3][PRESS_IDX],'tab:blue', label='press 3')
                            
                            lines[SENSOR_1][TEMP_IDX], = axs[axs_idx[SENSOR_1]][TEMP_IDX].plot(x_data, y_data[SENSOR_1][TEMP_IDX],'tab:pink', label='temp 1')
                            lines[SENSOR_2][TEMP_IDX], = axs[axs_idx[SENSOR_2]][TEMP_IDX].plot(x_data, y_data[SENSOR_2][TEMP_IDX],'tab:olive', label='temp 2')
                            lines[SENSOR_3][TEMP_IDX], = axs[axs_idx[SENSOR_3]][TEMP_IDX].plot(x_data, y_data[SENSOR_3][TEMP_IDX],'tab:cyan', label='temp 3')
                            
                            if (combined_plot):
                                axs[axs_idx[SENSOR_1]][PRESS_IDX].set_title('Pressure')
                            else:
                                axs[axs_idx[SENSOR_1]][PRESS_IDX].set_title('Pressure 1')
                                axs[axs_idx[SENSOR_2]][PRESS_IDX].set_title('Pressure 2')
                                axs[axs_idx[SENSOR_3]][PRESS_IDX].set_title('Pressure 3')
                            
                            axs[axs_idx[SENSOR_2]][PRESS_IDX].set_ylabel(units_str)
                            axs[axs_idx[SENSOR_2]][PRESS_IDX].yaxis.set_label_position("right")
                            axs[axs_idx[SENSOR_3]][PRESS_IDX].set_xlabel('t - seconds')
                            
                            if (combined_plot):
                                axs[axs_idx[SENSOR_1]][TEMP_IDX].set_title('Temperature')
                            else:
                                axs[axs_idx[SENSOR_1]][TEMP_IDX].set_title('Temperature 1')
                                axs[axs_idx[SENSOR_2]][TEMP_IDX].set_title('Temperature 2')
                                axs[axs_idx[SENSOR_3]][TEMP_IDX].set_title('Temperature 3')
                            
                            axs[axs_idx[SENSOR_2]][TEMP_IDX].set_ylabel('°C')
                            
                            for j in range(len(axs)):
                                axs[j][PRESS_IDX].set_xlim(x_upper_bound, x_lower_bound)
                                axs[j][PRESS_IDX].set_ylim(y_lower_bound_press, y_upper_bound_press)
                                axs[j][TEMP_IDX].set_xlim(x_upper_bound, x_lower_bound)
                                axs[j][TEMP_IDX].set_ylim(y_lower_bound_temp, y_upper_bound_temp)
                                
                                axs[j][PRESS_IDX].yaxis.set_major_locator(AutoLocator())
                                axs[j][PRESS_IDX].yaxis.set_major_formatter(FormatStrFormatter('%d'))
                                axs[j][PRESS_IDX].yaxis.set_minor_locator(AutoMinorLocator())
                                axs[j][PRESS_IDX].yaxis.tick_right()
                                axs[j][PRESS_IDX].xaxis.set_minor_locator(AutoMinorLocator())
                                
                                axs[j][TEMP_IDX].yaxis.set_major_locator(AutoLocator())
                                axs[j][TEMP_IDX].yaxis.set_major_formatter(FormatStrFormatter('%d'))
                                axs[j][TEMP_IDX].yaxis.set_minor_locator(AutoMinorLocator())
                                
                                if (combined_plot):
                                    leg[PRESS_IDX] = axs[j][PRESS_IDX].legend()
                                    leg[TEMP_IDX] = axs[j][TEMP_IDX].legend()
                            
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
