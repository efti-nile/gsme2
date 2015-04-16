#include "msp430_uart.h"

/// \brief Initializes specified UART with predefined settings in the header
void MSP430_UART_Init(void){
    UCA1CTL1 |= UCSWRST; // Hault UART

    UCA1CTL1 |= UCSSEL_2; // BRCLK drawn from SMCLK (I.e. UART clocked by SMCLK)

    // Set IO for UART operation
    /*P4OUT &= BIT4^0xFF; P4DIR |= BIT4; P4REN &= BIT4^0xFF;*/ P4SEL |= BIT4; /*P4DS &= BIT4^0xFF;*/ // UCA1TXD
    /*P4OUT &= BIT5^0xFF; P4DIR |= BIT5; P4REN &= BIT5^0xFF;*/ P4SEL |= BIT5; /*P4DS &= BIT5^0xFF;*/ // UCA1RXD

    UCA1BRW = 0x04E2; // Baud rate configuring. 0x04E2 for 9600 @ 12MHz BRCLK

    UCA1CTL1 &= ~UCSWRST; // Release UART

    UCA1IE |= UCRXIE | UCTXIE; // Enable interrupts
}

void

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void){
      switch(__even_in_range(UCA0IV,4))
    {
    case 0: // Vector 0 - no interrupt
        break;
    case 2: // Vector 2 - RXIFG
        break;
    case 4: // Vector 4 - TXIFG
        break;
    default:
        break;
    }
}