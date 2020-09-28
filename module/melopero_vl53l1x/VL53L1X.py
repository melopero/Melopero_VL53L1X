#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Jan 23 12:01:42 2019

@author: Leonardo La Rocca
"""

from ctypes import *
import os

# Code to find the c library
# Folder structure :
# some_folder(usr/local/lib/python3.*/):
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
    print(
        'Something went wrong while loading the vl53l1x api, make sure there is a file named vl53l1x_api.*.so in this directory ',
        lib_loc)
    raise FileNotFoundError("Error while loading library")

vl53l1x_api.StartConnection.argtypes = [POINTER(c_void_p), c_uint8, c_uint8]
vl53l1x_api.StartConnection.restype = c_int8
vl53l1x_api.StartRanging.argtypes = [c_void_p, c_int]
vl53l1x_api.StartRanging.restype = c_int8
vl53l1x_api.GetMeasurement.argtypes = [c_void_p]
vl53l1x_api.GetMeasurement.restype = c_int
vl53l1x_api.StopRanging.argtypes = [c_void_p]
vl53l1x_api.StopRanging.restype = c_int8
vl53l1x_api.SetTimingBudget.argtypes = [c_void_p, c_int]
vl53l1x_api.SetTimingBudget.restype = c_int8
vl53l1x_api.GetTimingBudget.argtypes = [c_void_p]
vl53l1x_api.GetTimingBudget.restype = c_int
vl53l1x_api.SetInterPeriod.argtypes = [c_void_p, c_int]
vl53l1x_api.SetInterPeriod.restype = c_int8
vl53l1x_api.GetInterPeriod.argtypes = [c_void_p]
vl53l1x_api.GetInterPeriod.restype = c_int
vl53l1x_api.SetROI.argtypes = [c_void_p, c_uint8, c_uint8, c_uint8, c_uint8]
vl53l1x_api.SetROI.restype = c_int8
vl53l1x_api.CloseConnection.argtypes = [POINTER(c_void_p)]
vl53l1x_api.SetI2CAddress.argtypes = [c_void_p, c_uint8]
vl53l1x_api.SetI2CAddress.restype = c_int8

class VL53L1X:
    SHORT_DST_MODE = 1
    MEDIUM_DST_MODE = 2
    LONG_DST_MODE = 3

    ERROR_NONE = 0

    def __init__(self, i2c_addr=0x29, i2c_bus=1):
        self._i2c_address = i2c_addr
        self._i2c_bus = i2c_bus
        self._dev_pointer = c_void_p()
        status = vl53l1x_api.StartConnection(pointer(self._dev_pointer), c_uint8(self._i2c_address), c_uint8(self._i2c_bus))
        self._exception_check(status)
        #print(f'Initialized device at i2c address {self._i2c_address} pointer : {self._dev_pointer}')

    def set_i2c_address(self, i2c_address):
        """ Sets a ne i2c address for the device. """
        status = vl53l1x_api.SetI2CAddress(self._dev_pointer, i2c_address)
        self._exception_check(status)

    def start_ranging(self, mode=MEDIUM_DST_MODE):
        """starts ranging: \nSHORT_DST_MODE for short distance mode\n
        MEDIUM_DST_MODE for medium distance\nLONG_DST_MODE for long distance"""
        status = vl53l1x_api.StartRanging(self._dev_pointer, mode)
        self._exception_check(status)

    def get_measurement(self):
        """ returns the last recorded measurement, -1 if there isn't one"""
        dst = vl53l1x_api.GetMeasurement(self._dev_pointer)
        if dst < 0:
            self._exception_check(dst)
        return dst

    def stop_ranging(self):
        error = vl53l1x_api.StopRanging(self._dev_pointer)
        self._exception_check(error)

    def set_intermeasurement_period(self, millis):
        """ Sets the inter-measurement period (the delay between two ranging
        operations) in milliseconds. \n
        The minimum inter-measurement period must be longer than the timing budget + 4 ms."""
        if millis < self.get_measurement_timing_budget(self._dev_pointer) + 4:
            raise ValueError("inter-measurement period must be longer than the timing budget + 4 ms.")

        status = vl53l1x_api.SetInterPeriod(self._dev_pointer, millis)
        self._exception_check(status)

    def set_measurement_timing_budget(self, millis):
        """Timing budget is the time required by the sensor to perform one range measurement.
        The minimum and maximum timing budgets are [20 ms, 1000 ms]"""
        if 1000 < millis < 20:
            raise ValueError("timing budget has to be between 20ms and 1000ms")

        status = vl53l1x_api.SetTimingBudget(self._dev_pointer, millis * 1000)
        self._exception_check(status)

    def get_measurement_timing_budget(self):
        """Returns the timing budget in milliseconds."""
        timing_budget_micros = vl53l1x_api.GetTimingBudget(self._dev_pointer)
        if timing_budget_micros < 0:
            self._exception_check(timing_budget_micros)
        else:
            return timing_budget_micros / 1000

    def set_ROI(self, top_left_x, top_left_y, bottom_right_x, bottom_right_y):
        """Sets the region of interest of the sensor. The roi must be a rectangle
        with side lengths between 4 and 16.
        Parameters:
            top_left_x --- the top left x coordinate\n
            top_left_y --- the top left y coordinate\n
            bottom_right_x --- the bottom right x coordinate\n
            bottom_right_y --- the bottom right y coordinate\n"""
        if bottom_right_x - top_left_x < 3 or top_left_y - bottom_right_y < 3:
            raise ValueError("The region of interest must be at least a 4*4 rectangle")

        status = vl53l1x_api.SetROI(self._dev_pointer, top_left_x, top_left_y, bottom_right_x, bottom_right_y)
        self._exception_check(status)

    def __del__(self):
        self.close_connection()

    def close_connection(self):
        """Closes the open i2c connection"""
        # TODO: add warning deprecated
        vl53l1x_api.CloseConnection(pointer(self._dev_pointer))

    def _exception_check(self, status):
        if status == -1:
            raise Exception("VL53L1_ERROR_CALIBRATION_WARNING: Invalid calibration data")
        if status == -4:
            raise Exception("VL53L1_ERROR_INVALID_PARAMS: Invalid parameter is set in a function")
        if status == -5:
            raise Exception(
                "VL53L1_ERROR_NOT_SUPPORTED: Requested parameter is not supported in the programmed configuration")
        if status == -6:
            raise Exception("VL53L1_ERROR_RANGE_ERROR: Interrupt status is incorrect")
        if status == -7:
            raise Exception("VL53L1_ERROR_TIME_OUT: Ranging is aborted due to timeout")
        if status == -8:
            raise Exception("VL53L1_ERROR_MODE_NOT_SUPPORTED Requested mode is not supported")
        if status == -10:
            raise Exception("VL53L1_ERROR_CALIBRATION_WARNING Supplied buffer is larger than I2C supports")
        if status == -14:
            raise Exception("VL53L1_ERROR_INVALID_COMMAND Command is invalid in current mode")
        if status == -16:
            raise Exception("VL53L1_ERROR_REF_SPAD_INIT An error occurred during reference SPAD calibration")
        if status == -22:
            raise Exception(
                "VL53L1_ERROR_XTALK_EXTRACTION_FAIL Thrown when crosstalk calibration function has no successful samples to compute the crosstalk. In this case there is not enough information to generate new crosstalk parameter information. The function will exit and leave the current crosstalk parameters unaltered.")
        if status == -23:
            raise Exception(
                "VL53L1_ERROR_XTALK_EXTRACTION_SIGMA_LIMIT_FAIL Thrown when crosstalk calibration function has found that the sigma estimate is above the maximal limit allowed. In this case the crosstalk sample is too noisy for measurement. The function will exit and leave the current crosstalk parameters unaltered.")
        if status == -24:
            raise Exception(
                "VL53L1_ERROR_OFFSET_CAL_NO_SAMPLE_FAIL Thrown when offset calibration function has found no valid ranging.")
        if status == -28:
            raise Exception(
                "VL53L1_WARNING_REF_SPAD_CHAR_NOT_ENOUGH_SPADS Thrown if there are less than five good SPADs available. Ensure calibration setup is in line with ST recommendations.")
        if status == -29:
            raise Exception(
                "VL53L1_WARNING_REF_SPAD_CHAR_RATE_TOO_HIGH Thrown if the final reference rate is greater than the upper reference rate limit - default is 40 Mcps. Ensure calibration setup is in line with ST recommendations.")
        if status == -30:
            raise Exception(
                "VL53L1_WARNING_REF_SPAD_CHAR_RATE_TOO_LOW Thrown if the final reference rate is less than the lower reference rate limit - default is 10 Mcps. Ensure calibration setup is in line with ST recommendations.")
        if status == -31:
            raise Exception(
                "VL53L1_WARNING_OFFSET_CAL_MISSING_SAMPLES Thrown if there is less than the requested number of valid samples. Ensure offset calibration setup is in line with ST recommendations.")
        if status == -32:
            raise Exception(
                "VL53L1_WARNING_OFFSET_CAL_SIGMA_TOO_HIGH Thrown if the offset calibration range sigma estimate is too high. Ensure offset calibration setup is in line with ST recommendations.")
        if status == -33:
            raise Exception(
                "VL53L1_WARNING_OFFSET_CAL_RATE_TOO_HIGH Thrown when signal rate is greater than a limit and sensor is saturating. Ensure offset calibration setup is in line with ST recommendations.")
        if status == -34:
            raise Exception(
                "VL53L1_WARNING_OFFSET_CAL_SPAD_COUNT_TOO_LOW Thrown when not enough SPADS can be used. Ensure offset calibration setup is in line with ST recommendations.")
        if status == -41:
            raise Exception("VL53L1_ERROR_NOT_IMPLEMENTED Function called is not implemented")
        if status != VL53L1X.ERROR_NONE and status < 0:
            raise Exception("Unknown Error!, error code: {}".format(status))
