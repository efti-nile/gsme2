#include "io430.h"
#include "types.h"

#include "msp430_uart.h"

#include "delay.h"
#include "sim900.h"
#include "sms_queue.h"
#include "teldir.h"
#include "powmeas.h"

// Pins description //////////////////////////////////////////////////

// R\E\ RS485
#define RxTx_RS485_GPIO GPIOA
#define RxTx_RS485_Pin GPIO_Pin_8
#define RxTx_RS485_RxEnable Bit_RESET
#define RxTx_RS485_TxEnable Bit_SET

#define LED_GPIO GPIOB
#define LED_Pin GPIO_Pin_5

// State variables ///////////////////////////////////////////////////

struct State_TypeDef{
    // Telephone number of the last received SMS command
    u8 TelNumOfSourceOfRequest[SMS_TELNUM_LEN];
    // Controller address in the RS-485 network
    u8 controller_address;
    // Flags
    u32 sim900_initialized;
    u32 request_close_all_valves;
    u32 request_open_all_valves;
    u32 leak_prev;
    u32 leak_now;
    u32 check_battery_prev;
    u32 check_battery_now;
    u32 link_lost_prev;
    u32 link_lost_now;
    u32 leak_removed_prev;
    u32 leak_removed_now;
    u32 link_ok_with_main_controller_prev;
    u32 link_ok_with_main_controller_now;
    u32 battery_ok_in_gsm_extender_prev;
    u32 battery_ok_in_gsm_extender_now;
    u32 ext_supply_ok_prev;
    u32 ext_supply_ok_now;
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

//u8 buf[10];
//u32 cnt = 0;

// Circular buffer for USART connected to SIM900 /////////////////////

#define CIRBUF_SIZE 300
u8 CirBuf[CIRBUF_SIZE];
u32 CirBuf_Tail = 0;
u32 CirBuf_Head = 0;
u32 CirBuf_NumBytes = 0;

// Simple buffer for SMS with information about balance //////////////

u8 SMS_Balance[SMS_TEXT_MAXLEN];

// Functions prototypes //////////////////////////////////////////////

void MSP430_UCS_Init(void);

void ErrorHandler(u32 ErrNum);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void TIM3_IRQHandler(void);
void SendCmd(u8 cmd);
void OkStatus_Update(void);
u8 CRC_Calc(u8* src, u32 num);
u32 CirBuf_Str(const u8 *template);
void SIM900_ReInit(void);
u32 Init(void);
u32 TIM3_Start(u16 timeout);
void LED_On(void);
void LED_Off(void);
void SIM900_SendStr(u8* str);
u32 SIM900_CircularBuf_Search(const u8 pattern[]);
void SIM900_CircularBuffer_Purge(void);
void SIM900_ReadSms(void);
void SIM900_SendSms(void);
u32 IsTelNumberSymbol(const u8 symbol[]);
u32 SIM900_WaitForResponse(u8 *pos_resp, u8 *neg_resp);
u32 SIM900_CircularBuffer_ExtractTelNum(u8 *Dst);
u32 SIM900_CircularBuffer_ExtractBalanceNum(const u8 Pattern[], u8 *Dst, u32 Num);
u32 SIM900_CircularBuffer_Extract(const u8 Pattern[], u8 *Dst, u32 Num, u8 DelChar);
