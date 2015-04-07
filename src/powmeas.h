#ifndef _POWMEAS_H_
#define _POWMEAS_H_

#include "io430.h"
#include "types.h"

#define BATTERY_CRYT_VOLTAGE 1200
#define BATTERY_HYSTERESIS   200

#define EXTERNAL_SUPPLY_CRYT_VOLTAGE 500

#define POWMEAS_ENABLE_BATTERY_MEAS_PIN             GPIO_Pin_1
#define POWMEAS_ENABLE_BATTERY_MEAS_GPIO            GPIOA

#define POWMEAS_BATTERY_MEAS_PIN                    GPIO_Pin_5
#define POWMEAS_BATTERY_MEAS_GPIO                   GPIOA
#define POWMEAS_BATTERY_MEAS_ADC_CHANNEL            5

#define POWMEAS_EXTERNAL_SUPPLY_MEAS_PIN            GPIO_Pin_4
#define POWMEAS_EXTERNAL_SUPPLY_MEAS_GPIO           GPIOA
#define POWMEAS_EXTERNAL_SUPPLY_MEAS_ADC_CHANNEL    4

#define POWMEAS_ENABLE_CLOCK RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_ADC1EN

void PowMeas_Init(void);
u32 PowMeas_BatteryStatus(void);
u32 PowMeas_ExternSupplyStatus(void);
static u32 PowMeas_AdcGet(u32 channel);
static void PowMeas_BatteryMeasEnable(void);
static void PowMeas_BatteryMeasDisable(void);

#endif
