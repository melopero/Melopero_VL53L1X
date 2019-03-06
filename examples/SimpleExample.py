#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Feb 12 18:23:18 2019

@author: Leonardo La Rocca
MIT license
"""

import time
import melopero_vl53l1x.VL53L1X as mp

sensor = mp.VL53L1X()
sensor.start_ranging(mp.VL53L1X.MEDIUM_DST_MODE)
for i in range(100):
    print(sensor.get_measurement())
    #time.sleep(0.1)

sensor.stop_ranging()
sensor.close_connection()
