#ifndef _MSP430_UART_H_
#define _MSP430_UART_H_

// TODO Generalize thid module for any USCI module. That allow to use the module for gsm module and for RS-485 communication

#include "io430.h"
#include "types.h"

void MSP430_UART_Init(void);
u8 MSP430_UART_RxByte(void);
void MSP430_UART_TxByte(u8 data);

#endif