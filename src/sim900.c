#include "sim900.h"

u32 SIM900_PowerOn(void){
    GPIO_WriteBit(g_HPWR_GPIO, g_HPWR_Pin, Bit_SET);
	return 0;
}

u32 SIM900_PowerOff(void){
    GPIO_WriteBit(g_HPWR_GPIO, g_HPWR_Pin, Bit_RESET);
    return 0;
}

u32 SIM900_SoftReset(void){
    GPIO_WriteBit(g_PWR_GPIO, g_PWR_Pin, Bit_RESET);
    Delay_DelayMs(1000);
    GPIO_WriteBit(g_PWR_GPIO, g_PWR_Pin, Bit_SET);
    return 0;
}

u32 SIM900_HoldReset(void){
    GPIO_WriteBit(g_PWR_GPIO, g_PWR_Pin, Bit_RESET);
    return 0;
}

u32 SIM900_GetStatus(void){
    return GPIO_ReadInputDataBit(g_STS_GPIO, g_STS_Pin);
}

u32 SIM900_AlarmSms(void){
    SIM900_SendStr("FLOOD");
    return 0;
}

u32 SIM900_CloseAllOkReport(void){
    SIM900_SendStr("CLOSE ALL OK REPORT");
    return 0;
}

u32 SIM900_OpenAllOkReport(void){
    SIM900_SendStr("OPEN ALL OK REPORT");
    return 0;
}

u32 SIM900_CloseAllFailReport(void){
    SIM900_SendStr("CLOSE ALL FAIL REPORT");
    return 0;
}

u32 SIM900_OpenAllFailReport(void){
    SIM900_SendStr("OPEN ALL FAIL REPORT");
    return 0;
}

u32 SIM900_ChangeBatteryRequest(void){
    SIM900_SendStr("CHANGE BATTERY");
    return 0;
}

u32 SIM900_RfLinkLostReport(void){
    SIM900_SendStr("RF LINK LOST");
    return 0;
}

