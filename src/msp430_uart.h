#ifndef _MSP430_UART_H_
#define _MSP430_UART_H_

#include "io430.h"
#include "types.h"
#include "main.h"

#define UART_RS485 0 // USCI_A0
#define UART_SIM900 1 // USCI_A1

// R\E\ RS485 - P3.2
#define RxTx_RS485_INIT {P3OUT &= ~BIT2; P3DIR |= BIT2; P3REN &= ~BIT2; P3SEL &= ~BIT2; P3DS &= ~BIT2;}
#define RxTx_RS485_RxEnable P3OUT &= ~BIT2
#define RxTx_RS485_TxEnable P3OUT |= BIT2

// Circular buffer for USART connected to SIM900 /////////////////////

void MSP430_UART_Init(void);
void MSP430_UART_SendAddress(u8 interface, u8 address);
void MSP430_UART_Send(u8 interface, u8 *src, u16 num);
__interrupt void USCI_A0_ISR(void);
__interrupt void USCI_A1_ISR(void);

#endif
