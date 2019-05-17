#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Jan 23 12:01:42 2019

@author: Leonardo La Rocca
"""

from ctypes import CDLL, c_uint8
import os

# Code to find the c library
#Folder structure :
#some_folder(usr/local/lib/python3.*/):
#   melopero_vl53l1x
#       -VL53L1X.py
#   vl53l1x_api.*.so
lib_loc = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
lib_file = ''
for file in os.listdir(lib_loc):
    if file.startswith('vl53l1x_api') and file.endswith('.so'):
        lib_file = file

vl53l1x_api = None

try:
    vl53l1x_api = CDLL(os.path.join(lib_loc, lib_file))
except:
    print('Something went wrong while loading the vl53l1x api, make sure there is a file named vl53l1x_api.*.so in this directory ', lib_loc)

class VL53L1X():
    
    SHORT_DST_MODE = 1
    MEDIUM_DST_MODE = 2
    LONG_DST_MODE = 3
    
    def __init__(self, i2c_addr = 0x29, i2c_bus = 1):
        self._cfuncs = vl53l1x_api
        self._i2c_address = i2c_addr
        self._i2c_bus = i2c_bus
        self._cfuncs.StartConnection(c_uint8(self._i2c_address), c_uint8(self._i2c_bus))
        
    def start_ranging(self, mode=MEDIUM_DST_MODE):
        '''starts ranging: \nSHORT_DST_MODE for short distance mode\n
        MEDIUM_DST_MODE for medium distance\nLONG_DST_MODE for long distance'''
        self._cfuncs.StartRanging(mode)
        
    def get_measurement(self):
        ''' returns the last recorded measurement, -1 if there isn't one'''
        return self._cfuncs.getMeasurement()
    
    def stop_ranging(self):
        self._cfuncs.StopRanging()
        
    def set_intermeasurement_period(self, millis):
        ''' Sets the inter-measurement period (the delay between two ranging 
        operations) in milliseconds''' 
        status = self._cfuncs.SetInterPeriod(millis)
        self._print_status(status)
            
    def set_measurement_timing_budget(self, millis):
        '''Timing budget is the time required by the sensor to perform one range measurement.
        The minimum and maximum timing budgets are [20 ms, 1000 ms]'''
        time_b = millis
        if millis > 1000:
            time_b = 1000
        elif millis < 20:
            time_b = 20
            
        status = self._cfuncs.SetTimingBudget(time_b*1000)
        self._print_status(status)
            
    def setROI(self, top_left_x, top_left_y, bottom_right_x, bottom_right_y):
        """Sets the region of interest of the sensor. The roi must be a rectangle
        with side lengths between 4 and 16.
        Parameters:
            top_left_x --- the top left x coordinate\n
            top_left_y --- the top left y coordinate\n
            bottom_right_x --- the bottom right x coordinate\n
            bottom_right_y --- the bottom right y coordinate\n"""
        if bottom_right_x - top_left_x < 3 or top_left_y - bottom_right_y < 3:
            raise Exception("The region of interest must be at least a 4*4 rectangle")
        
        status = self._cfuncs.SetROI(top_left_x, top_left_y, bottom_right_x, bottom_right_y)
        self._print_status(status)
    
    def _print_status(self, status):
        if status != 0:
            print('something went wrong error : {}'.format(status))
    
    def close_connection(self):
        '''Closes the open i2c connection'''
        self._cfuncs.CloseConnection()
        
        