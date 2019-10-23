#include "vl53l1_api.h"
#include "vl53l1_platform.h"
#include "vl53l1_error_strings.h"

static VL53L1_Dev_t *device;
static int32_t last_distance;
static VL53L1_RangingMeasurementData_t RangingMeasurementData;
static VL53L1_RangingMeasurementData_t *pRangingMeasurementData = &RangingMeasurementData;

VL53L1_Error StartConnection(uint8_t i2c_address, uint8_t bus_number){
    VL53L1_Error Status = VL53L1_ERROR_NONE;

    VL53L1_Dev_t *dev = (VL53L1_Dev_t *)malloc(sizeof(VL53L1_Dev_t));
    memset(dev, 0, sizeof(VL53L1_Dev_t));

    Status = StartI2CConnection(bus_number);
    if (Status < 0) return Status;

    dev->I2cDevAddr = i2c_address;

    Status = VL53L1_software_reset(dev);
    if (Status < 0) return Status;
    Status = VL53L1_WaitDeviceBooted(dev);
    if (Status < 0) return Status;
    //printf("wait device booted: %d\n", Status);
    Status = VL53L1_DataInit(dev);
    if (Status < 0) return Status;
    //printf("Data init: %d\n", Status);
    Status = VL53L1_StaticInit(dev);
    if (Status < 0) return Status;
    //printf("Static init: %d\n", Status);


    /*VL53L1_DeviceInfo_t DeviceInfo;
    Status = VL53L1_GetDeviceInfo(dev, &DeviceInfo);
    if(Status == VL53L1_ERROR_NONE){
        printf("VL53L0X_GetDeviceInfo:\n");
        printf("Device Name : %s\n", DeviceInfo.Name);
        printf("Device Type : %s\n", DeviceInfo.Type);
        printf("Device ID : %s\n", DeviceInfo.ProductId);
        printf("ProductRevisionMajor : %d\n", DeviceInfo.ProductRevisionMajor);
        printf("ProductRevisionMinor : %d\n", DeviceInfo.ProductRevisionMinor);
    }*/


    Status = VL53L1_PerformRefSpadManagement(dev);
    if (Status < 0) return Status;
    //printf("perform ref spad management: %d\n", Status);
    Status = VL53L1_SetXTalkCompensationEnable(dev, 0); // Disable crosstalk compensation (bare sensor)
    if (Status < 0) return Status;
    //printf("set X talk compensation enable: %d\n", Status);

    device = dev;

    Status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(device, 66000);
    if (Status < 0) return Status;
    Status = VL53L1_SetInterMeasurementPeriodMilliSeconds(device, 70);
    if (Status < 0) return Status;

    return Status;
}

/*VALID MODES ARE :
VL53L1_DISTANCEMODE_SHORT   1
VL53L1_DISTANCEMODE_MEDIUM  2
VL53L1_DISTANCEMODE_LONG    3
*/
VL53L1_Error StartRanging(int mode){
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    Status = VL53L1_SetDistanceMode(device, mode);
    if (Status < 0) return Status;
    //printf("setting distance mode %d: %d\n",mode, Status);
    Status = VL53L1_StartMeasurement(device);
    //printf("starting Measurement: %d\n", Status);
    last_distance = -1;
    return Status;
}

/* return distance in millimeter */
int32_t getMeasurement(){
    VL53L1_Error Status = VL53L1_ERROR_NONE;
    Status = VL53L1_WaitMeasurementDataReady(device);
    if (Status < 0) return Status;

    Status = VL53L1_GetRangingMeasurementData(device, pRangingMeasurementData);
    if (Status < 0) return Status;
    //printf("getting Measurement: %d\n", Status);
    last_distance = pRangingMeasurementData->RangeMilliMeter;
    if (Status < 0) return Status;
    //printf("distance: %d\n", last_distance);
    Status = VL53L1_ClearInterruptAndStartMeasurement(device);
    if (Status < 0) return Status;
    //printf("clear interrupt : %d\n", Status);

    return last_distance;
}

void StopRanging(){
    VL53L1_StopMeasurement(device);
}


VL53L1_Dev_t* GetDevicePointer(){
    return device;
}

VL53L1_Error SetTimingBudget(int micros){
	return VL53L1_SetMeasurementTimingBudgetMicroSeconds(device, micros);
}

int32_t GetTimingBudget() {
    VL53L1_Error Status;
    uint32_t timing_budget = 0;
    Status = VL53L1_GetMeasurementTimingBudgetMicroSeconds(device, &timing_budget);
    if (Status == VL53L1_ERROR_NONE)
        return (int32_t) timing_budget;

    return (int32_t) Status;
}

VL53L1_Error SetInterPeriod(int millis){
	return VL53L1_SetInterMeasurementPeriodMilliSeconds(device, millis);
}

int32_t GetInterPeriod(){
    VL53L1_Error Status;
    uint32_t inter_period = 0;
    Status = VL53L1_GetInterMeasurementPeriodMilliSeconds(device, &inter_period);
    if (Status == VL53L1_ERROR_NONE)
        return (int32_t) inter_period;

    return (int32_t) Status;
}

VL53L1_Error SetROI(uint8_t top_left_x, uint8_t top_left_y, uint8_t bottom_right_x, uint8_t bottom_right_y){
    VL53L1_UserRoi_t roiConfig;
    roiConfig.TopLeftX = top_left_x;
    roiConfig.TopLeftY = top_left_y;
    roiConfig.BotRightX = bottom_right_x;
    roiConfig.BotRightY = bottom_right_y;
    return VL53L1_SetUserROI(device, &roiConfig);
}


void CloseConnection(){
    CloseI2CConnection();
}
