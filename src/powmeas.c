#include "powmeas.h"

/*!
    \brief The last status of the battery

    Remembers the last status of battery: 1 - charged, 0 - discharged.
    It helps to implement hysteresis.
*/
static u8 PowMeas_PrevBatteryStatus;

/*!
    \brief Initializes IO, ADC_10A and REF for power measurement.

    Initializes IO.
    Initializes ADC_10A module. See comments in source fot details.
    Initializes REF module for ADC_10A. See comments in source fot details.

    Initializes PowMeas_PrevBatteryStatus variable with the current battery status.
*/
void PowMeas_Init(void){
    // IO Inintialization
    ENBAT_INIT; // Enable battery measurement circuit
    INBAT_INIT; // Battery power measurement ADC channel
    INPWR_INIT; // External power measurement  ADC channel
    TEMP_INIT;  // Tepmerature sensor input
    LMT84_INIT; // Temperature sensor supply turning off

    // ADC_10A Initialization
    ADC10CTL0 = 0X00; // Switch ADC_10A off
    ADC10CTL0 |= ADC10ON; // Switch ADC_10A on
    ADC10CTL0 |= 0x8 << 8; // Sample time is 256 cycles of ADC10CLK
    ADC10CTL1 |= 0x4 << 5 | 0x3 << 3 | ADC10SHP; // Use SMCLK divided by 5 as ADC clock, use sample timer
    ADC10CTL2 |= ADC10RES; // 10-bit result
    ADC10MCTL0 |= 1 << 4; // Use internal reference as ADC reference
    REFCTL0 |= REFMSTR | 0X3 << 4 | REFON; // Configure internal 2.5V reference
    ADC10CTL0 |= ADC10ENC; // Enable conversion


    ENBAT_SET;
    PowMeas_PrevBatteryStatus = (PowMeas_AdcGet(INPWR_ADC_CH) > BATTERY_CRYT_VOLTAGE);
    ENBAT_CLR;
}

/*!
    \brief Returns 1 if battery is OK
*/
u8 PowMeas_BatteryStatus(void){
    u8 retval;

    ENBAT_SET;

    // TODO: Check if is it necessary to introduce some delay here. It may
    // allow to charge the ADC input capacitor fully.

    retval = (
        PowMeas_PrevBatteryStatus = (
            PowMeas_AdcGet(INPWR_ADC_CH)
             >
            BATTERY_CRYT_VOLTAGE - (PowMeas_PrevBatteryStatus * 2 - 1) * BATTERY_HYSTERESIS
        )
    );

    ENBAT_CLR;

    return retval;
}

/*!
    \brief Returns 1 if external power supply is OK.
*/
u8 PowMeas_ExternSupplyStatus(void){
    return PowMeas_AdcGet(INPWR_ADC_CH) > EXTERNAL_SUPPLY_CRYT_VOLTAGE;
}

/*!
    \brief Returns ADC value of specified channel.
*/
u16 PowMeas_AdcGet(u8 channel){
  ADC10MCTL0 = (ADC10MCTL0 & 0xF0) | (channel & 0x0F); // Set channel
  ADC10CTL0 |= ADC10SC; // Start the conversion
  while(!(ADC10CTL1 & ADC10BUSY)); // Wait until conversion completion
  return ADC10MEM0; // Return the conversion value
}

/*!
    \brief Returns temperature in celcium degree
*/
s8 PowMeas_GetTemp(void){
    u16 tmp;
    LMT84_ON;
    Delay_DelayMs(2);
    tmp = PowMeas_AdcGet(TEMP_ADC_CH);
    LMT84_OFF;
    return TEMP_MIN + ((ADC_VALUE_AT_TEMP_MAX - tmp) * (TEMP_MAX - TEMP_MIN))/(ADC_VALUE_AT_TEMP_MAX - ADC_VALUE_AT_TEMP_MIN);
}
