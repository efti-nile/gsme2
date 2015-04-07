#include "powmeas.h"

// Remembers the last status of battery, 1 - charged, 0 - discharged
// It is proposed to implement hysteresis only
u32 PowMeas_PrevBatteryStatus;

// Initializes GPIO and ADC, sets initial value for PowMeas_PrevBatteryStatus
void PowMeas_Init(void){
    GPIO_InitTypeDef  GPIO_InitStruct;
    ADC_InitTypeDef   ADC_InitStruct;
    
    POWMEAS_ENABLE_CLOCK;
    
    GPIO_InitStruct.GPIO_Pin   = POWMEAS_ENABLE_BATTERY_MEAS_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(POWMEAS_ENABLE_BATTERY_MEAS_GPIO, &GPIO_InitStruct);
    GPIO_WriteBit(POWMEAS_ENABLE_BATTERY_MEAS_GPIO, POWMEAS_ENABLE_BATTERY_MEAS_PIN, Bit_RESET);
    
    GPIO_InitStruct.GPIO_Pin   = POWMEAS_BATTERY_MEAS_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(POWMEAS_BATTERY_MEAS_GPIO, &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin   = POWMEAS_EXTERNAL_SUPPLY_MEAS_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(POWMEAS_EXTERNAL_SUPPLY_MEAS_GPIO, &GPIO_InitStruct);
    
    ADC_InitStruct.ADC_Mode                  = ADC_Mode_Independent;
    ADC_InitStruct.ADC_ScanConvMode          = DISABLE;
    ADC_InitStruct.ADC_ContinuousConvMode    = DISABLE;
    ADC_InitStruct.ADC_ExternalTrigConv      = ADC_ExternalTrigConv_None;
    ADC_InitStruct.ADC_DataAlign             = ADC_DataAlign_Right;
    ADC_InitStruct.ADC_NbrOfChannel          = 1;
    
    ADC_DeInit(ADC1);
    ADC_Init(ADC1, &ADC_InitStruct);
    ADC_Cmd(ADC1, ENABLE);
    
    // Enable ADC1 reset calibaration register
    ADC_ResetCalibration(ADC1);
    // Check the end of ADC1 reset calibration register
    while(ADC_GetResetCalibrationStatus(ADC1));
    // Start ADC1 calibaration
    ADC_StartCalibration(ADC1);
    // Check the end of ADC1 calibration
    while(ADC_GetCalibrationStatus(ADC1));
    
    PowMeas_PrevBatteryStatus = 
        (PowMeas_AdcGet(POWMEAS_BATTERY_MEAS_ADC_CHANNEL) > BATTERY_CRYT_VOLTAGE);  
}

// Returns 1 if battery charged, otherwise 0
u32 PowMeas_BatteryStatus(void){
    u32 retval;
    
    PowMeas_BatteryMeasEnable();
    
    retval = (
        PowMeas_PrevBatteryStatus = (
            PowMeas_AdcGet(POWMEAS_BATTERY_MEAS_ADC_CHANNEL) 
             > 
            BATTERY_CRYT_VOLTAGE - (PowMeas_PrevBatteryStatus * 2 - 1) * BATTERY_HYSTERESIS 
        )
    );
    
    PowMeas_BatteryMeasDisable();
    
    return retval;
}

// Returns 1 if proper external power supply presents, otherwise 0
u32 PowMeas_ExternSupplyStatus(void){
    return PowMeas_AdcGet(POWMEAS_EXTERNAL_SUPPLY_MEAS_ADC_CHANNEL) > EXTERNAL_SUPPLY_CRYT_VOLTAGE;
}

// Returns ADC value from given channel
u32 PowMeas_AdcGet(u32 channel){
  ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_41Cycles5);
  // Start the conversion
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  // Wait until conversion completion
  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
  // Get the conversion value
  return ADC_GetConversionValue(ADC1);
}

static void PowMeas_BatteryMeasEnable(void){
     GPIO_WriteBit(POWMEAS_ENABLE_BATTERY_MEAS_GPIO, POWMEAS_ENABLE_BATTERY_MEAS_PIN, Bit_SET);
}

static void PowMeas_BatteryMeasDisable(void){
     GPIO_WriteBit(POWMEAS_ENABLE_BATTERY_MEAS_GPIO, POWMEAS_ENABLE_BATTERY_MEAS_PIN, Bit_RESET);
}
