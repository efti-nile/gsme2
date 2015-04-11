#ifndef _POWMEAS_H_
#define _POWMEAS_H_

#include "io430.h"
#include "types.h"

// Voltage levels
#define BATTERY_CRYT_VOLTAGE            366 // 366 = 4.4V
#define BATTERY_HYSTERESIS              8 // 8 = 0.1V
#define EXTERNAL_SUPPLY_CRYT_VOLTAGE    491 // 491 = 4.8V

// Enable battery measurement circuit
#define ENBAT_INIT {P5OUT &= BIT2^0xFF; P5DIR |= BIT2; P5REN &= BIT2^0xFF; P5SEL &= BIT2^0xFF; P5DS &= BIT2^0xFF;}
#define ENBAT_SET (P5OUT |= BIT2)
#define ENBAT_CLR (P5OUT &= BIT2^0xFF)

// External power measurement ADC channel
#define INBAT_INIT {P6OUT &= BIT0^0xFF; P6DIR &= BIT0^0xFF; P6REN &= BIT0^0xFF; P6SEL |= BIT0; P6DS &= BIT0^0xFF;}
#define INBAT_ADC_CH 0

// Battery power measurement  ADC channel
#define INPWR_INIT {P6OUT &= BIT1^0xFF; P6DIR &= BIT1^0xFF; P6REN &= BIT1^0xFF; P6SEL |= BIT1; P6DS &= BIT1^0xFF;}
#define INPWR_ADC_CH 1

void PowMeas_Init(void);
u8 PowMeas_BatteryStatus(void);
u8 PowMeas_ExternSupplyStatus(void);
static u16 PowMeas_AdcGet(u8 channel);

#endif
