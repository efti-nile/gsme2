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
    INBAT_INIT; // External power measurement ADC channel
    INPWR_INIT; // Battery power measurement  ADC channel

    // ADC_10A Initialization
    ADC10CTL0 &= ~ ADC10ON; // Switch ADC_10A off
    ADC10CTL0 |= ADC10ON; // Switch ADC_10A on
    ADC10CTL0 |= 0x8 << 8; // Sample time is 256 cycles of ADC10CLK
    ADC10CTL1 |= 0x4 << 5 | 0x3 << 3 | ADC10SHP; // Use SMCLK divided by 5 as ADC clock, use sample timer
    ADC10CTL2 |= ADC10RES; // 12-bit result
    ADC10MCTL0 |= 1 << 4; // Use internal reference as ADC reference
    REFCTL0 |= REFMSTR | 0X3 << 4 | REFON; // Configure internal 2.5V reference


    ENBAT_SET;
    PowMeas_PrevBatteryStatus = (PowMeas_AdcGet(INPWR_ADC_CH) > BATTERY_CRYT_VOLTAGE);
    ENBAT_CLR;
}

/*!
    \brief Returns 1 if battery is OK
*/
u8 PowMeas_BatteryStatus(void){
    u32 retval;

    ENBAT_SET;

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
  ADC10MCTL0 |= channel & 0x0F; // Set channel
  ADC10CTL0 |= ADC10ENC | ADC10SC; // Start the conversion
  while(ADC10IFG & ADC10IFG0); // Wait until conversion completion
  return ADC10MEM0; // Return the conversion value
}
