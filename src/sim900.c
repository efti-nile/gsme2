#include "sim900.h"

u8 SIM900_PowerOn(void){
    g_HPWR_SET;
	return 0;
}

u8 SIM900_PowerOff(void){
    g_HPWR_CLEAR;
    return 0;
}

u8 SIM900_SoftReset(void){
    g_PWR_CLEAR;
    Delay_DelayMs(1000);
    g_PWR_SET;
    return 0;
}

u8 SIM900_HoldReset(void){
    g_PWR_CLEAR;
    return 0;
}

u8 SIM900_GetStatus(void){
    return g_STS_READ;
}



