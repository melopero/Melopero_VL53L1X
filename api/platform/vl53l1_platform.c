
/*
* Copyright (c) 2017, STMicroelectronics - All Rights Reserved
*
* This file is part of VL53L1 Core and is dual licensed,
* either 'STMicroelectronics
* Proprietary license'
* or 'BSD 3-clause "New" or "Revised" License' , at your option.
*
********************************************************************************
*
* 'STMicroelectronics Proprietary license'
*
********************************************************************************
*
* License terms: STMicroelectronics Proprietary in accordance with licensing
* terms at www.st.com/sla0081
*
* STMicroelectronics confidential
* Reproduction and Communication of this document is strictly prohibited unless
* specifically authorized in writing by STMicroelectronics.
*
*
********************************************************************************
*
* Alternatively, VL53L1 Core may be distributed under the terms of
* 'BSD 3-clause "New" or "Revised" License', in which case the following
* provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* License terms: BSD 3-clause "New" or "Revised" License.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*
********************************************************************************
*
*/


#include "vl53l1_platform.h"
#include "vl53l1_api.h"
#include <sys/ioctl.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
// Terrible portability hack between arm-linux-gnueabihf-gcc on Mac OS X and native gcc on raspbian.
#ifndef I2C_M_RD
#include <linux/i2c.h>
#endif
 
static int i2c_file = -1;

void StartI2CConnection(int bus_num){
    char filename[20];
    snprintf(filename, 19, "/dev/i2c-%d", bus_num);
    i2c_file = open(filename, O_RDWR);
    if (i2c_file < 0) {
        printf("Error occurred while opening file %s! %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
} 

void CloseI2CConnection(){
    close(i2c_file);
}

static int i2c_write(uint8_t slave_addr, uint16_t reg, uint8_t *data, uint8_t len){
    int retval;
    uint8_t outbuf[len + 2];

    struct i2c_msg msgs[1];
    struct i2c_rdwr_ioctl_data msgset[1];

    //insert 16 bits reg addr in 8 bits int buf
    outbuf[0] = reg >> 8;
    outbuf[1] = reg & 0xff;

    for (int i = 2; i < len + 2; i++)
        outbuf[i] = data[i-2];

    msgs[0].addr = slave_addr;
    msgs[0].flags = 0;
    msgs[0].len = len + 2;
    msgs[0].buf = outbuf;

    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 1;

    if (ioctl(i2c_file, I2C_RDWR, &msgset) < 0) {
        perror("ioctl(I2C_RDWR) in i2c_write");
        return -1;
    }

    return 0;
}

static int i2c_read(uint8_t slave_addr, uint16_t reg, uint8_t *result, uint8_t len) {
    int retval;
    uint8_t outbuf[2], inbuf[len];
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset[1];

    msgs[0].addr = slave_addr;
    msgs[0].flags = 0;
    msgs[0].len = 2;
    msgs[0].buf = outbuf;

    msgs[1].addr = slave_addr;
    msgs[1].flags = I2C_M_RD | I2C_M_NOSTART;
    msgs[1].len = len;
    msgs[1].buf = inbuf;

    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 2;

    //insert 16 bits reg addr in 8 bits int buf
    outbuf[0] = reg >> 8;
    outbuf[1] = reg & 0xff;

    *result = 0;
    if (ioctl(i2c_file, I2C_RDWR, &msgset) < 0) {
        perror("ioctl(I2C_RDWR) in i2c_read");
        return -1;
    }
    for (int i = 0; i < len ; i++)
        *(result + i) = inbuf[i];
    return 0;
}


VL53L1_Error VL53L1_WriteMulti(VL53L1_DEV Dev, uint16_t index, uint8_t *pdata, uint32_t count) {    
    return i2c_write(Dev->I2cDevAddr, index, pdata, count);
}

// the ranging_sensor_comms.dll will take care of the page selection
VL53L1_Error VL53L1_ReadMulti(VL53L1_DEV Dev, uint16_t index, uint8_t *pdata, uint32_t count) {
    return i2c_read(Dev->I2cDevAddr, index, pdata, count);
}

VL53L1_Error VL53L1_WrByte(VL53L1_DEV Dev, uint16_t index, uint8_t data) {
    return i2c_write(Dev->I2cDevAddr, index, &data, 1);
}

VL53L1_Error VL53L1_WrWord(VL53L1_DEV Dev, uint16_t index, uint16_t data) {
    uint8_t buffer[2];
	buffer[1] = data>>0&0xFF;
	buffer[0] = data>>8&0xFF;
    return i2c_write(Dev->I2cDevAddr, index, buffer, 2);
}

VL53L1_Error VL53L1_WrDWord(VL53L1_DEV Dev, uint16_t index, uint32_t data) {
    uint8_t buffer[4];
	buffer[3] = data>>0&0xFF;
	buffer[2] = data>>8&0xFF;
	buffer[1] = data>>16&0xFF;
	buffer[0] = data>>24&0xFF;
	return i2c_write(Dev->I2cDevAddr, index, buffer, 4);
}

VL53L1_Error VL53L1_UpdateByte(VL53L1_DEV Dev, uint16_t index, uint8_t AndData, uint8_t OrData) {
    int32_t status_int;
	uint8_t data;

	status_int = i2c_read(Dev->I2cDevAddr, index, &data, 1);

	if (status_int <= 0){
		return  status_int;
	}

	data = (data & AndData) | OrData;
	return i2c_write(Dev->I2cDevAddr, index, &data, 1);
}

VL53L1_Error VL53L1_RdByte(VL53L1_DEV Dev, uint16_t index, uint8_t *data) {
    uint8_t tmp = 0;
    int status = i2c_read(Dev->I2cDevAddr, index, &tmp, 1);
    *data = tmp;
    return status;
}

VL53L1_Error VL53L1_RdWord(VL53L1_DEV Dev, uint16_t index, uint16_t *data) {
    uint8_t buffer[2];
	int status = i2c_read(Dev->I2cDevAddr, index, buffer, 2);
	uint16_t tmp = 0;
	tmp |= buffer[1]<<0;
	tmp |= buffer[0]<<8;
	*data = tmp;
    return status;
}

VL53L1_Error VL53L1_RdDWord(VL53L1_DEV Dev, uint16_t index, uint32_t *data) {
    uint8_t buffer[4];
	int status = i2c_read(Dev->I2cDevAddr, index, buffer, 4);
	uint32_t tmp = 0;
    tmp |= buffer[3]<<0;
    tmp |= buffer[2]<<8;
	tmp |= buffer[1]<<16;
	tmp |= buffer[0]<<24;
	*data = tmp;
    return status;
}

VL53L1_Error VL53L1_GetTickCount(
	uint32_t *ptick_count_ms)
{
	VL53L1_Error status  = VL53L1_ERROR_NONE;
	return status;
}

//#define trace_print(level, ...) \
//	_LOG_TRACE_PRINT(VL53L1_TRACE_MODULE_PLATFORM, \
//	level, VL53L1_TRACE_FUNCTION_NONE, ##__VA_ARGS__)

//#define trace_i2c(...) \
//	_LOG_TRACE_PRINT(VL53L1_TRACE_MODULE_NONE, \
//	VL53L1_TRACE_LEVEL_NONE, VL53L1_TRACE_FUNCTION_I2C, ##__VA_ARGS__)

VL53L1_Error VL53L1_GetTimerFrequency(int32_t *ptimer_freq_hz)
{
	VL53L1_Error status  = VL53L1_ERROR_NONE;
	return status;
}

VL53L1_Error VL53L1_WaitMs(VL53L1_Dev_t *pdev, int32_t wait_ms){
	VL53L1_Error status  = VL53L1_ERROR_NONE;
    usleep(wait_ms * 1000);
	return status;
}

VL53L1_Error VL53L1_WaitUs(VL53L1_Dev_t *pdev, int32_t wait_us){
	VL53L1_Error status  = VL53L1_ERROR_NONE;
    usleep(wait_us);
	return status;
}

VL53L1_Error VL53L1_WaitValueMaskEx(
	VL53L1_Dev_t *pdev,
	uint32_t      timeout_ms,
	uint16_t      index,
	uint8_t       value,
	uint8_t       mask,
	uint32_t      poll_delay_ms)
{
    uint8_t  register_value = 0;
	VL53L1_Error status  = VL53L1_ERROR_NONE;
	int32_t attempts = timeout_ms / poll_delay_ms;

	for(int32_t x = 0; x < attempts; x++){
		status = VL53L1_RdByte(pdev, index, &register_value);
		if (status == VL53L1_ERROR_NONE && (register_value & mask) == value) {
			return VL53L1_ERROR_NONE;
		}
		usleep(poll_delay_ms * 1000);
	}

    return VL53L1_ERROR_TIME_OUT;
}




