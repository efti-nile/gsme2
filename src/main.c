#include "main.h"

struct State_TypeDef State;
struct InPack_TypeDef InPack;
struct OutPack_TypeDef OutPack;
volatile s8 tmp;

int main(void)
{
    // Stop watchdog timer to prevent time out reset
    // It is good to enable watchdog timer in production
    WDTCTL = WDTPW + WDTHOLD;

    //TelDir_SetBalanceNumber("002A0031003000300023"); // Delete in production
    MSP430_UCS_Init();
    TelDir_Clean();
    MSP430_UART_Init();
    Loads_Init();
    Delay_Init();
    LED_INIT;
    SMS_Queue_Init();
    PowMeas_Init();
    __bis_SR_register(GIE);

    g_PWR_INIT;
    g_HPWR_INIT;
    g_STS_INIT;

    SIM900_ReInit();
    SysTimer_Start();

    State.sim900_initialized = 1;

    while(1){
        State.sim900_initialized = SIM900_GetStatus();
				
				P5OUT |= BIT3;

        Delay_DelayMs(10000);
        if(SIM900_CircularBuf_Search("+CMTI")){
            SIM900_ReadSms();
        }
        SIM900_SendSms();

        // Check link with main controller
        State.link_ok_with_main_controller_prev = State.link_ok_with_main_controller_now;
        State.link_ok_with_main_controller_now = (State.ok_timeout > 0);
        if( State.link_ok_with_main_controller_prev &&
            !State.link_ok_with_main_controller_now )
        {
            u8 TelNum[SMS_TELNUM_LEN];
            TelDir_Iterator_Init();
            while(TelDir_GetNextTelNum(TelNum)){
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_LINK_LOST_WITH_MAIN_CONTROLLER, SMS_LIFETIME);
            }
        }


        // Check battery in GSM Extender
        State.battery_ok_in_gsm_extender_prev = State.battery_ok_in_gsm_extender_now;
        State.battery_ok_in_gsm_extender_now = PowMeas_BatteryStatus();
        if( State.battery_ok_in_gsm_extender_prev &&
            !State.battery_ok_in_gsm_extender_now )
        {
            u8 TelNum[SMS_TELNUM_LEN];
            TelDir_Iterator_Init(); // TODO: I changed ...Iterator() to ...Iterator_Init()
            while(TelDir_GetNextTelNum(TelNum)){
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_BATTERY_LOW_IN_GSM_EXTENDER, SMS_LIFETIME);
            }
        }

        // Check battery in GSM Extender
        State.ext_supply_ok_prev = State.ext_supply_ok_now;
        State.ext_supply_ok_now = PowMeas_ExternSupplyStatus();
        if( State.ext_supply_ok_prev &&
            !State.ext_supply_ok_now )
        {
            u8 TelNum[SMS_TELNUM_LEN];
            TelDir_Iterator_Init();
            while(TelDir_GetNextTelNum(TelNum)){
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_EXTERNAL_SUPPLY_LOST, SMS_LIFETIME);
            }
        }

    }
}

/*!
    \brief Blinkes the error and restarts SIM900

    The function blinks out the number of the error by the LED.
*/
void ErrorHandler(u32 ErrNum){
    u32 i;

    LED_OFF;

    Delay_DelayMs(4000);

    for(i = 0; i < ErrNum; ++i){
        Delay_DelayMs(300);
        LED_ON;
        Delay_DelayMs(300);
        LED_OFF;
    }

    SIM900_ReInit();
}

/*!
    \brief Updates global OK-flag and turns on the LED
*/
void OkStatus_Update(void){
	State.ok_timeout = OK_TIMEOUT;
	LED_ON;
}

/*!
    \brief Starts 150ms system timer, enables interrupt uppon timer every 150ms
*/
void SysTimer_Start(u16 timeout){
    TA1CTL = 0x0000; // Hault the timer
    TA1CTL = 0X00C0; // ID = 8 (i.e. /8 divider)
    TA1CTL |= 0X2 << 8; // Use SMCLK as a source clock
    TA1EX0 = 0X7; // Set /8 additional divider
    TA1R = 0x0000; // Zero the timer
    TA1CCR0 = 58650; // Set period 58650 is 150 ms period with SMCLK == 25MHz, IDEX = 8, ID = 8
    TA1CTL |= 0X1 << 4; // Launch the timer in up mode
    TA1CTL |= TAIE;
}

/*!
    \brief TIMER_A1 ISR counts down timeouts in the system

    The timer catches several timeouts.

    1. Close Valves Timeout
    This timeout is set when command to close all valves received and the
    timeout is being expired until acknoledgement of succesful command execution
    gotten.

    2. Open Valves Timeout
    This timeout is set when command to open all valves received and the
    timeout is being expired until acknoledgement of succesful command execution
    gotten.

    3. OK Timeout
    This timeout is reset each time WaterLeak controller riches us. If timeout
    eventually expired that's concerned as the WaterLeak controller lost.
*/
#pragma vector=TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR(void){
    if(TA1CTL & TAIFG){
        TA1R = 0x0000;
        if(State.close_valves_timeout > 0){
            State.close_valves_timeout--;
            if(!State.close_valves_timeout && State.request_close_all_valves){
                State.request_close_all_valves = 0;
                SMS_Queue_Push(State.TelNumOfSourceOfRequest, SIM900_SMS_REPORT_CLOSE_NOT_ALL, SMS_LIFETIME);
            }
        }
        if(State.open_valves_timeout > 0){
            State.open_valves_timeout--;
            if(!State.open_valves_timeout && State.request_open_all_valves){
                State.request_open_all_valves = 0;
                SMS_Queue_Push(State.TelNumOfSourceOfRequest, SIM900_SMS_REPORT_OPEN_NOT_ALL, SMS_LIFETIME);
            }
        }
		if(State.ok_timeout > 0){
			State.ok_timeout--;
		}else{
			LED_OFF;
		}
        TA1CTL &= ~TAIFG; // TODO: If this is necessary to clear the flag?
    }
}

/*!
    \brief The function sends 1 byte command to a main WaterLeak controller

    The function builds the outgoing packet, calculates the check sum and
    sends all that.
*/
void SendCmd(u8 cmd){
    u16 i;

    OutPack.DevID = State.controller_address;
    OutPack.Length = sizeof(OutPack) - 2; // Excluding DevID & Length
    OutPack.SourceAddress = MY_ADDRESS;
    OutPack.TID = 0;
    OutPack.COMMAND = cmd;

    OutPack.crc = CRC_Calc((u8*)&OutPack, sizeof(OutPack)-1);

    RxTx_RS485_TxEnable;
	
    i = 7000; // TODO: Rewrite this shit
    while(i--);

    // Send the head of the outgoing packet - address byte
    MSP430_UART_SendAddress(UART_RS485, OutPack.DevID);

    // Send the rest of the outgoing packet - data bytes
    MSP430_UART_Send(UART_RS485, (u8 *)&OutPack + 1, sizeof(OutPack) - 1);

    i = 7000*3; // TODO: Bljad!
    while(i--);

    RxTx_RS485_RxEnable;
}

/*!
    \brief MCU peripheral initialization

    The function configures the following modules:
    1. USCI_AO for RS-485 communication
    2. USCI_A1 for SIM900 communication
    3. GPIO
        3.1 Power switch pin for SIM900
        3.2 Off/on SIM900 pin
        3.3 Input pin to obtain SIM900 status
        3.4 R\E\ RS485 (Toggling between Rx and Tx)
        3.5 LED pin initialization
    4. Delay module initialization
    5. SMS Queue module initialization
    6. Talephone Directory module initialzation
    7. Power measurements initialization
    8. SIM900 Start
    9. Main timer start
*/
void Init(void){
    // TODO: USCI_A0 initialization call
    // TODO: USCI_A1 initialization call

    //////////////////////// GPIO ////////////////////////////////////
    g_HPWR_INIT;
    g_PWR_INIT;

    RxTx_RS485_INIT;
    LED_INIT;

    Delay_Init();
    SMS_Queue_Init();
    // TelDir_Init(); TODO: Port it.
    PowMeas_Init();
    SIM900_ReInit();
}

/*!
    \brief Calculates simple control sum via XOR with 0xAA
*/
u8 CRC_Calc(u8* src, u16 num){
    u8 crc = 0xAA;
    while(num--) crc ^= *src++;
    return crc;
}

/*!
    \brief Configures UCS for GSME2 needs

    The function sets DCOCLK and DCOCLKDIV to 25MHz

    Source clock for FFL: REFOCLK 32768Hz
    FLLREFDIV = 1
    FLLN = 762
    DCORSEL = 6
    DCO = 15
    FLLD = 1
*/
void MSP430_UCS_Init(void){
    UCSCTL3 = UCSCTL3 & 0xFF8F | 0x0020; // Set REFOCLK as FLLREFCLK
    UCSCTL2 = UCSCTL2 & 0xFC00 | 762; // Set N = 762
    UCSCTL1 = UCSCTL1 & 0xFF8F | 0x0060; // Set DCORSEL = 6
    UCSCTL1 = UCSCTL1 & 0xE0FF | 0x0F00; // Set DCO = 15
}
