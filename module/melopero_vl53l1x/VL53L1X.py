#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Jan 23 12:01:42 2019

@author: Leonardo La Rocca
"""

from ctypes import CDLL, c_uint8
import os


class VL53L1X():
    
    SHORT_DST_MODE = 1
    MEDIUM_DST_MODE = 2
    LONG_DST_MODE = 3
    
    def __init__(self, i2c_addr = 0x29, i2c_bus = 1):
        self._cfuncs = CDLL(os.path.join(os.getcwd(),'libvl53l1_api.so'))
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
        if status != 0:
            print('something went wrong error : {}'.format(status))
            
    def set_measurement_timing_budget(self, millis):
        '''Timing budget is the time required by the sensor to perform one range measurement.
        The minimum and maximum timing budgets are [20 ms, 1000 ms]'''
        time_b = millis
        if millis > 1000:
            time_b = 1000
        elif millis < 20:
            time_b = 20
            
        status = self._cfuncs.SetTimingBudget(time_b*1000)
        if status != 0:
            print('something went wrong error : {}'.format(status))
            
    
    def close_connection(self):
        '''Closes the open i2c connection'''
        self._cfuncs.CloseConnection()
        
        