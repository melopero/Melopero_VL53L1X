# Melopero VL53L1X
![melopero logo](images/sensor.jpg?raw=true)

## Getting Started
### Prerequisites
You will need:
- a python3 version, which you can get here: [download python3](https://www.python.org/downloads/)
- the VL53L1X sensor: [buy here](https://www.melopero.com/)

### Installing
You can install the melopero-vl53l1x module by typing this line in the terminal: 
`pip3 install melopero-vl53l1x`

## Module description
The module contains a class to easily access the VL53L1X's sensor functions.

### Usage
First you need to import the module in your file: 
```python
import melopero-vl53l1x as mp
```
Then you can create a simple vl53l1x object and access it's methods, the sensor object will be initialized with the i2c address set to `0x29` and the i2c bus to `1` alias `(dev/i2c-1)` which is the standard i2c bus in a Raspberry pi. 
```python 
sensor = mp.VL53L1X()
```
Alternatively you can modify it's parameters by typing
```python
sensor = mp.VL53L1X(i2c_addr = myaddress, i2c_bus = mybus)
```

The sensor has the following methods
```python 
sensor.start_ranging(range_mode) # Starts ranging, range_mode can be one of SHORT_DST_MODE, MEDIUM_DST_MODE, LONG_DST_MODE 
sensor.get_measurement() #returns the last measurement taken
sensor.stop_ranging() #stops the continuous ranging
sensor.set_intermeasurement_period(millis) #sets the milliseconds between measurements
sensor.set_measurement_timing_budget(millis) #Sets the milliseconds the sensor has to take a measurement, must be between [20ms, 1000ms]
```
Remember to close the connection after using the sensor
```python 
sensor.close_connection()
```

## Example
The following example will print out the measured distance a 100 times
```python 
import time
import melopero-vl53l1x as mp

sensor = VL53L1X()
sensor.start_ranging(mp.VL53L1X.MEDIUM_DST_MODE)
for i in range(100):
    print(sensor.get_measurement())
    time.sleep(0.1)

sensor.stop_ranging()
sensor.close_connection()