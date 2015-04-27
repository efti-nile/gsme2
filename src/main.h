#include "io430.h"
#include "types.h"

#include "msp430_uart.h"

#include "delay.h"
#include "sim900.h"
#include "sms_queue.h"
#include "teldir.h"
#include "powmeas.h"

// Pins description //////////////////////////////////////////////////

// R\E\ RS485 - P3.2
#define RxTx_RS485_INIT {P3OUT &= BIT2^0xFF; P3DIR |= BIT2; P3REN &= BIT2^0xFF; P3SEL &= BIT2^0xFF; P3DS &= BIT2^0xFF}
#define RxTx_RS485_RxEnable (P3OUT &= BIT2^0xFF)
#define RxTx_RS485_TxEnable (P3OUT |= BIT2)

// Indicating LED - P5.3
#define LED_INIT {P5OUT &= BIT3^0xFF; P5DIR |= BIT3; P5REN &= BIT3^0xFF; P5SEL &= BIT3^0xFF; P5DS &= BIT3^0xFF}
#define LED_ON P5OUT |= BIT3
#define LED_OFF P5OUT &= ~BIT3

// State variables ///////////////////////////////////////////////////

struct State_TypeDef{
    // Telephone number of the last received SMS command
    u8 TelNumOfSourceOfRequest[SMS_TELNUM_LEN];
    // Controller address in the RS-485 network
    u8 controller_address;
    // Flags
    u8 sim900_initialized;
    u8 request_close_all_valves;
    u8 request_open_all_valves;
    u8 leak_prev;
    u8 leak_now;
    u8 check_battery_prev;
    u8 check_battery_now;
    u8 link_lost_prev;
    u8 link_lost_now;
    u8 leak_removed_prev;
    u8 leak_removed_now;
    u8 link_ok_with_main_controller_prev;
    u8 link_ok_with_main_controller_now;
    u8 battery_ok_in_gsm_extender_prev;
    u8 battery_ok_in_gsm_extender_now;
    u8 ext_supply_ok_prev;
    u8 ext_supply_ok_now;
    // Timers and timeouts
    u32 sms_timer;
    u32 close_valves_timeout;
    u32 open_valves_timeout;
    u32 ok_timeout;
};

struct State_TypeDef State;

// Communication via RS485 ///////////////////////////////////////////

#define MY_ADDRESS 0xC1

#define COMMAND_VALVES_STATE_UNKNOWN 0x00
#define COMMAND_SEND_ALARM           0x01
#define COMMAND_ALL_VALVES_CLOSED    0x02
#define COMMAND_ALL_VALVES_OPENED    0x03
#define COMMAND_CHANGE_BATTERY       0x04

#define RESPONSE_NO_EVENTS 0x00
#define RESPONSE_RESERVED  0x01
#define RESPONSE_CLOSE_ALL 0x02
#define RESPONES_OPEN_ALL  0x03

#define IN_COMMAND_AVC   (1<<0)
#define IN_COMMAND_AVO   (1<<1)
#define IN_COMMAND_CHB   (1<<2)
#define IN_COMMAND_CHL   (1<<3)
#define IN_COMMAND_LEAK  (1<<4)
#define IN_COMMAND_LEAKR (1<<5)

struct InPack_TypeDef{
    u8 DevID;
    u8 Length;
    u8 SourceAddress;
    u8 TID;
    u8 COMMAND;
    u8 crc;
};

struct OutPack_TypeDef{
    u8 DevID;
    u8 Length;
    u8 SourceAddress;
    u8 TID;
    u8 COMMAND;
    u8 crc;
};

struct InPack_TypeDef InPack;
struct OutPack_TypeDef OutPack;

// Timeouts in seconds ///////////////////////////////////////////////

//#define SMS_TIMER_MOD 20
#define CLOSE_VALVES_TIMEOUT 10
#define OPEN_VALVES_TIMEOUT 10
#define OK_TIMEOUT 5

// Simple buffer for SMS with information about balance //////////////

u8 SMS_Balance[SMS_TEXT_MAXLEN]; // TODO: maybe put it in stack?

// Functions prototypes //////////////////////////////////////////////

void MSP430_UCS_Init(void);

void ErrorHandler(u32 ErrNum);
void TIM3_IRQHandler(void);
void SendCmd(u8 cmd);
void OkStatus_Update(void);
u8 CRC_Calc(u8* src, u32 num);
u32 CirBuf_Str(const u8 *template);
void SIM900_ReInit(void);
u32 Init(void);
u32 TIM3_Start(u16 timeout);
u32 IsTelNumberSymbol(const u8 symbol[]);
__interrupt void T_A1_ISR(void);