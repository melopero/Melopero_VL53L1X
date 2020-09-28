#include "vl53l1_api.h"
#include "vl53l1_platform.h"
#include "vl53l1_error_strings.h"

static int devices = 0;

VL53L1_Error StartConnection(void** dev_pointer, uint8_t i2c_address, uint8_t bus_number){
    VL53L1_Error Status = VL53L1_ERROR_NONE;

    VL53L1_Dev_t *dev = (VL53L1_Dev_t *)malloc(sizeof(VL53L1_Dev_t));
    memset(dev, 0, sizeof(VL53L1_Dev_t));

    if (devices == 0){
        Status = StartI2CConnection(bus_number);
        if (Status < 0) return Status;
    }
    devices++;

    dev->I2cDevAddr = i2c_address;

    Status = VL53L1_software_reset(dev);
    if (Status < 0) return Status;

    Status = VL53L1_WaitDeviceBooted(dev);
    if (Status < 0) return Status;
    
    Status = VL53L1_DataInit(dev);
    if (Status < 0) return Status;
    
    Status = VL53L1_StaticInit(dev);
    if (Status < 0) return Status;



    Status = VL53L1_PerformRefSpadManagement(dev);
    if (Status < 0) return Status;
   
    Status = VL53L1_SetXTalkCompensationEnable(dev, 0); 
    if (Status < 0) return Status;

    Status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(dev, 66000);
    if (Status < 0) return Status;

    Status = VL53L1_SetInterMeasurementPeriodMilliSeconds(dev, 70);
    if (Status < 0) return Status;

    *dev_pointer = dev;

    return Status;
}

VL53L1_Error SetI2CAddress(void* dev_pointer, uint8_t i2c_address){    
    VL53L1_Dev_t *device = (VL53L1_Dev_t*) dev_pointer;
    
    VL53L1_Error status = VL53L1_ERROR_NONE;
    status = VL53L1_SetDeviceAddress(device, i2c_address);
    return status;
}

/*VALID MODES ARE :
VL53L1_DISTANCEMODE_SHORT   1
VL53L1_DISTANCEMODE_MEDIUM  2
VL53L1_DISTANCEMODE_LONG    3
*/
VL53L1_Error StartRanging(void* dev_pointer, int mode){
    VL53L1_Dev_t *device = (VL53L1_Dev_t*) dev_pointer;

    VL53L1_Error Status = VL53L1_ERROR_NONE;
    Status = VL53L1_SetDistanceMode(device, mode);
    if (Status < 0) return Status;

    Status = VL53L1_StartMeasurement(device);
    return Status;
}

/* return distance in millimeter */
int32_t GetMeasurement(void* dev_pointer){
    VL53L1_Dev_t *device = (VL53L1_Dev_t*) dev_pointer;
    VL53L1_RangingMeasurementData_t RangingMeasurementData;
    VL53L1_RangingMeasurementData_t *pRangingMeasurementData = &RangingMeasurementData;

    VL53L1_Error Status = VL53L1_ERROR_NONE;
    Status = VL53L1_WaitMeasurementDataReady(device);
    if (Status < 0) return Status;

    Status = VL53L1_GetRangingMeasurementData(device, pRangingMeasurementData);
    if (Status < 0) return Status;
    
    Status = VL53L1_ClearInterruptAndStartMeasurement(device);
    if (Status < 0) return Status;

    return pRangingMeasurementData->RangeMilliMeter;
}

VL53L1_Error StopRanging(void *dev_pointer){
    VL53L1_Dev_t *device = (VL53L1_Dev_t*) dev_pointer;
    return VL53L1_StopMeasurement(device);
}

VL53L1_Error SetTimingBudget(void *dev_pointer, int micros){
    VL53L1_Dev_t *device = (VL53L1_Dev_t*) dev_pointer;
	return VL53L1_SetMeasurementTimingBudgetMicroSeconds(device, micros);
}

int32_t GetTimingBudget(void *dev_pointer) {
    VL53L1_Dev_t *device = (VL53L1_Dev_t*) dev_pointer;
    VL53L1_Error Status;
    uint32_t timing_budget = 0;
    Status = VL53L1_GetMeasurementTimingBudgetMicroSeconds(device, &timing_budget);
    if (Status == VL53L1_ERROR_NONE)
        return (int32_t) timing_budget;

    return (int32_t) Status;
}

VL53L1_Error SetInterPeriod(void* dev_pointer, int millis){
    VL53L1_Dev_t *device = (VL53L1_Dev_t*) dev_pointer;
	return VL53L1_SetInterMeasurementPeriodMilliSeconds(device, millis);
}

int32_t GetInterPeriod(void *dev_pointer){
    VL53L1_Dev_t *device = (VL53L1_Dev_t*) dev_pointer;
    VL53L1_Error Status;
    uint32_t inter_period = 0;
    Status = VL53L1_GetInterMeasurementPeriodMilliSeconds(device, &inter_period);
    if (Status == VL53L1_ERROR_NONE)
        return (int32_t) inter_period;

    return (int32_t) Status;
}

VL53L1_Error SetROI(void* dev_pointer, uint8_t top_left_x, uint8_t top_left_y, uint8_t bottom_right_x, uint8_t bottom_right_y){
    VL53L1_Dev_t *device = (VL53L1_Dev_t*) dev_pointer;
    VL53L1_UserRoi_t roiConfig;
    roiConfig.TopLeftX = top_left_x;
    roiConfig.TopLeftY = top_left_y;
    roiConfig.BotRightX = bottom_right_x;
    roiConfig.BotRightY = bottom_right_y;
    return VL53L1_SetUserROI(device, &roiConfig);
}

void CloseConnection(void **dev_pointer){
    devices--;
    if (devices == 0) CloseI2CConnection();
    free(*dev_pointer);
    *dev_pointer = NULL;
}
