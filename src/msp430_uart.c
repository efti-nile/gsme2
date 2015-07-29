#include "msp430_uart.h"

u8 CirBuf[CIRBUF_SIZE];
u16 CirBuf_Tail = 0;
u16 CirBuf_Head = 0;
u16 CirBuf_NumBytes = 0;

/*!
    \brief Initializes USCI_A0 (for RS486) and USCI_A1 (for SIM900) as UARTs
*/
void MSP430_UART_Init(void){
    // USCI_A0 Initialization //////////////////////////////////////////////////
    RxTx_RS485_INIT;
    RxTx_RS485_RxEnable;

    UCA0CTL1 |= UCSWRST; // Hault UART

    UCA0CTL1 |= UCSSEL_2; // BRCLK drawn from SMCLK (I.e. UART clocked by SMCLK)

    UCA0CTL0 |= BIT2; // Use multiprocessor mode with address-bit

    // Set IO for UART operation P3.4 - RXD, P3.3 - TXD
    /*P3OUT &= BIT4^0xFF; P3DIR |= BIT4; P3REN &= BIT4^0xFF;*/ P3SEL |= BIT4; /*P3DS &= BIT4^0xFF;*/ // UCA1TXD
    /*P3OUT &= BIT5^0xFF; P3DIR |= BIT5; P3REN &= BIT5^0xFF;*/ P3SEL |= BIT3; /*P3DS &= BIT5^0xFF;*/ // UCA1RXD

    UCA0BRW = 2628; // Baud rate configuring. 0x0068 for 115200 @ 12MHz BRCLK

    UCA0CTL1 &= ~UCSWRST; // Release UART

    UCA0IE |= UCRXIE;
    //UCA0IE |= UCRXIE | UCTXIE; // Enable interrupts

    // USCI_A1 Initialization //////////////////////////////////////////////////
    UCA1CTL1 |= UCSWRST; // Hault UART

    UCA1CTL1 |= UCSSEL_2; // BRCLK drawn from SMCLK (I.e. UART clocked by SMCLK)

    // Set IO for UART operation P4.4 - TXD, P4.5 - RXD
    /*P4OUT &= BIT4^0xFF; P4DIR |= BIT4; P4REN &= BIT4^0xFF;*/ P4SEL |= BIT4; /*P4DS &= BIT4^0xFF;*/ // UCA1TXD
    /*P4OUT &= BIT5^0xFF; P4DIR |= BIT5; P4REN &= BIT5^0xFF;*/ P4SEL |= BIT5; /*P4DS &= BIT5^0xFF;*/ // UCA1RXD

    UCA1BRW = 219; // Baud rate configuring. 219 for 115200 @ 25MHz BRCLK

    UCA1CTL1 &= ~UCSWRST; // Release UART

    UCA1IE |= UCRXIE;
    //UCA1IE |= UCRXIE | UCTXIE; // Enable interrupts
}

/*!
    \brief Sends address-byte via specified USCI
*/
void MSP430_UART_SendAddress(u8 interface, u8 address){
    switch(interface){
        case UART_RS485: // USCI_A0
            UCA0CTL1 |= UCTXADDR; // To send the next byte as address
            UCA0TXBUF = address;
            while (!(UCA0IFG&UCTXIFG));
            break;
        case UART_SIM900: // USCI_A1 doesn't use the address bytes in this application
            break;
        default:
            break;
    }
}

/*!
    \brief Sends data via specified USCI
*/
void MSP430_UART_Send(u8 interface, u8 *src, u16 num){
    switch(interface){
        case UART_RS485: // USCI_A0
            for(; num != 0; num--){
                UCA0TXBUF = *(src++);
                while (!(UCA0IFG&UCTXIFG));
            }
            break;
        case UART_SIM900: // USCI_A1
            for(; num != 0; num--){
                UCA1TXBUF = *(src++);
                while (!(UCA1IFG&UCTXIFG));
            }
            break;
        default:
            break;
    }
}

/*!
    \brief USCI_A0 ISR (Used for RS485 communication)
*/
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){
    static u8 num_received_bytes = 0; // TODO: Can it be u8?
    volatile u8 tmp, if_address;
    switch(__even_in_range(UCA0IV,4))
    {
    case 0: // Vector 0 - no interrupt
        break;
    case 2: // Vector 2 - RXIFG
        if_address = UCA0STAT & UCADDR;
        tmp = UCA0RXBUF;

        // Save received byte
        ((u8*)&InPack)[num_received_bytes++] = tmp;

        // If the first byte after the idle state isn't marked as address
        // byte
        if(num_received_bytes == 1 && (!if_address || InPack.DevID != MY_ADDRESS)){
            num_received_bytes = 0;
        }else
        // If the following bytes in pack have the eighth bit is one then
        // reset too.
        if(num_received_bytes > 1 && if_address){
            num_received_bytes = 0;
        }else
        if(num_received_bytes == 3){
            State.controller_address = InPack.SourceAddress;
        }
        // If we managed to receive all pack's bytes until time-out expired
        if(num_received_bytes >= sizeof(struct InPack_TypeDef)){
            UCA1IE &= ~UCRXIE; // TODO: If is it necessary?
            if(InPack.crc == CRC_Calc((u8*)&InPack, sizeof(InPack)-1)){

                if(SIM900_GetStatus() && State.sim900_initialized){ // TODO: We have decided to change this
                    OkStatus_Update(); // Ready-to-work LED
                }

                if(InPack.COMMAND & IN_COMMAND_AVC){
                    if(State.request_close_valves){ // TODO: But what happens if this condition checked right after request gotten?
                        State.request_close_valves = 0;
                        SMS_Queue_Push(State.TelNumOfSourceOfRequest, SIM900_SMS_REPORT_CLOSE_OK, SMS_LIFETIME); // TODO: What is the warrnty that the source of the request still held here?
                    }
                }else
                if(InPack.COMMAND & IN_COMMAND_AVO){
                    if(State.request_open_valves){ // TODO: See above
                        State.request_open_valves = 0;
                        SMS_Queue_Push(State.TelNumOfSourceOfRequest, SIM900_SMS_REPORT_OPEN_OK, SMS_LIFETIME);
                    }
                }

                // Check the new state of the water leak flag.
                // If it has been just risen than send alarm everyone in
                // the telephone directory
                State.leak_prev = State.leak_now; // TODO: That is discussed to change this behaviour
                if(InPack.COMMAND & IN_COMMAND_LEAK){
                    State.leak_now = 1;
                    if(!State.leak_prev && State.leak_now){
                        u8 TelNum[SMS_TELNUM_LEN];
                        TelDir_Iterator_Init();
                        while(TelDir_GetNextTelNum(TelNum)){
                            SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_LEAK, SMS_LIFETIME);
                        }
                    }
                }else{
                    State.leak_now = 0;
                }

                // Check the new state of the low battery flag.
                // If it has been just risen than send a warning to everyone in
                // the telephone directory.
                State.check_battery_prev = State.check_battery_now; // TODO: That's said to report a type of the low battery device
                if(InPack.COMMAND & IN_COMMAND_CHB){
                    State.check_battery_now = 1;
                    if(!State.check_battery_prev && State.check_battery_now){
                        u8 TelNum[SMS_TELNUM_LEN];
                        TelDir_Iterator_Init();
                        while(TelDir_GetNextTelNum(TelNum)){
                            SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_CHECK_BATTERY, SMS_LIFETIME);
                        }
                    }
                }else{
                    State.check_battery_now = 0;
                }

                // Check the new state of the some-device-lost flag.
                // If it has been just risen than send a warning to everyone in
                // the telephone directory.
                State.link_lost_prev = State.link_lost_now; // TODO: That's said to report a name of the lost device (What's about wireless extenders?)
                if(InPack.COMMAND & IN_COMMAND_CHL){
                    State.link_lost_now = 1;
                    if(!State.link_lost_prev && State.link_lost_now){
                        u8 TelNum[SMS_TELNUM_LEN];
                        TelDir_Iterator_Init();
                        while(TelDir_GetNextTelNum(TelNum)){
                            SMS_Queue_Push(TelNum, SIM900_DMD_REPORT_LINK_LOST, SMS_LIFETIME);
                        }
                    }
                }else{
                    State.link_lost_now = 0;
                }

                // Check the new state of the water-leak-removed flag.
                // If it has been just risen than send a message to everyone in
                // the telephone directory.
                State.leak_removed_prev = State.leak_removed_now;
                if(InPack.COMMAND & IN_COMMAND_LEAKR){ // TODO: Check the sense of this flag on the WaterLeak controller
                    State.leak_removed_now = 1;
                    if(!State.leak_removed_prev && State.leak_removed_now
                       && State.leak_now){
                        u8 TelNum[SMS_TELNUM_LEN];
                        TelDir_Iterator_Init();
                        while(TelDir_GetNextTelNum(TelNum)){
                            SMS_Queue_Push(TelNum, SIM900_LEAK_REMOVED, SMS_LIFETIME);
                        }
                    }
                }else{
                    State.leak_removed_now = 0;
                }

                // If there is a some pending flag to execute user's request than
                // send proper command to the WaterLeak controller.
                if(State.request_close_valves){
									if(strcmp((const char *)State.current_valves_group, "all") == 0){
										 OutPack.Length = 4; // Excluding DevID & Length
									}else{
										 strcpy((char *)&OutPack.Optional, (const char *)State.current_valves_group);
										 OutPack.Length = 4 + strlen((const char *)State.current_valves_group); // Excluding DevID & Length
									}
									OutPack.DevID = State.controller_address;
									OutPack.COMMAND = RESPONSE_CLOSE_ALL;
									SendCmd();
                }else
                if(State.request_open_valves){
									if(strcmp((const char *)State.current_valves_group, "all") == 0){
										 OutPack.Length = 4; // Excluding DevID & Length
									}else{
										 strcpy((char *)&OutPack.Optional, (const char *)State.current_valves_group);
										 OutPack.Length = 4 + strlen((const char *)State.current_valves_group); // Excluding DevID & Length
									}
									OutPack.DevID = State.controller_address;
									OutPack.COMMAND = RESPONSE_OPEN_ALL;
									SendCmd();
                }else{
									  OutPack.Length = 4;
									  OutPack.COMMAND = RESPONSE_NO_EVENTS;
									  SendCmd();
                }
            }

            UCA1IE |= UCRXIE;

            num_received_bytes = 0;
        }
        break;
    case 4: // Vector 4 - TXIFG
        break;
    default:
        break;
    }
}

/*!
    \brief USCI_A1 ISR (Used for SIM900 communication)
*/
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void){
    switch(__even_in_range(UCA1IV,4))
    {
    case 0: // Vector 0 - no interrupt
        break;
    case 2: // Vector 2 - RXIFG
        CirBuf[CirBuf_Head++] = UCA1RXBUF;

        if(CirBuf_Head >= CIRBUF_SIZE){
            CirBuf_Head = 0;
        }

        if(CirBuf_Tail == CirBuf_Head){
            if(++CirBuf_Tail >= CIRBUF_SIZE){
                CirBuf_Tail = 0;
            }
        }

        ++CirBuf_NumBytes;
        break;
    case 4: // Vector 4 - TXIFG
        break;
    default:
        break;
    }
}