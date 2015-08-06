#ifndef _MSP430_SIM900_H_
#define _MSP430_SIM900_H_

#include "io430.h"

// Local headers ///////////////////////////////////////////////////////////////

#include "types.h"
#include "delay.h"
#include "msp430_uart.h"
#include "sms_queue.h"
#include "teldir.h"
#include "loads.h"

// Circular buffer for SIM900 communication

extern u8 CirBuf[CIRBUF_SIZE];
extern u16 CirBuf_Tail;
extern u16 CirBuf_Head;
extern u16 CirBuf_NumBytes;


// Simple buffer for SMS with information about balance //////////////

// Power SIM900
#define g_HPWR_INIT {P2OUT &= BIT0^0xFF; P2DIR |= BIT0; P2REN &= BIT0^0xFF; P2SEL &= BIT0^0xFF; P2DS &= BIT0^0xFF;}
#define g_HPWR_SET (P2OUT |= BIT0)
#define g_HPWR_CLEAR (P2OUT &= BIT0^0xFF)
// Reset SIM900 (PWRKEY)
#define g_PWR_INIT {P2OUT &= ~BIT1; P2DIR &= ~BIT1; P2REN &= ~BIT1; P2SEL &= ~BIT1; P2DS &= ~BIT1;}
#define g_PWR_SET {P2OUT &= ~BIT1; P2DIR &= ~BIT1;}
#define g_PWR_CLEAR {P2OUT &= ~BIT1; P2DIR |= BIT1;}

// State SIM900 (STS)
#define g_STS_INIT {P2OUT &= ~BIT2; P2DIR &= ~BIT2; P2REN &= ~BIT2; P2SEL &= ~BIT2; P2DS &= ~BIT2;}
#define g_STS_READ (P2IN & BIT2)

// Reset SIM900 (NRST)
#define g_NRST_INIT {P2OUT &= ~BIT3; P2DIR &= ~BIT3; P2REN &= ~BIT3; P2SEL &= ~BIT3; P2DS &= ~BIT3;}
#define g_NRST_SET {P2DIR |= BIT3;}
#define g_NRST_CLEAR {P2DIR &= ~BIT3;}

// Net light SIM900 (NETLIGHT)
#define g_NETLIGHT_INIT {P2OUT &= ~BIT4; P2DIR &= ~BIT4; P2REN &= ~BIT4; P2SEL &= ~BIT4; P2DS &= ~BIT4;}
#define g_NETLIGHT_READ {P2IN & BIT4;}

// Control USART1 pin
#define g_USART1_TX_DISABLE {P4SEL &= ~BIT4; P4DIR &= ~BIT4; P4OUT &= ~BIT4;}
#define g_USART1_TX_ENABLE {P4SEL |= BIT4; P4DIR |= BIT4; P4OUT |= BIT4;}

static const u8 SIM900_SMS_CMD_ADD[] = "043E04310430043204380442044C";
static const u8 SIM900_SMS_CMD_DEL[] = "04340430043B04380442044C";
static const u8 SIM900_SMS_CMD_CLEAN[] = "044704380441044204380442044C";
static const u8 SIM900_SMS_CMD_CLOSE[] = "0430043A0440044B0442044C";
static const u8 SIM900_SMS_CMD_CLOSE_[] = "0430043A0440044B0442044C0020"; // Added space in end
static const u8 SIM900_SMS_CMD_OPEN[] = "0442043A0440044B0442044C";
static const u8 SIM900_SMS_CMD_OPEN_[] = "0442043A0440044B0442044C0020"; // Added space in end
static const u8 SIM900_SMS_CMD_CHECK[] = "0440043E043204350440043A0430";
static const u8 SIM900_SMS_CMD_CHECK_BALANCE[] = "0430043B0430043D0441";
static const u8 SIM900_SMS_CMD_SET_BALANCE[] = "0430043B0430043D04410020043D043E043C043504400020";
static const u8 SIM900_SMS_CMD_TURN_LOAD_ON[] = "043A043B044E044704380442044C";
static const u8 SIM900_SMS_CMD_TURN_LOAD_OFF[] = "044B043A043B044E044704380442044C";
static const u8 SIM900_SMS_CMD_TEMPERATURE[] = "0435043C043F0435044004300442044304400430";

static const u8 SIM900_SMS_REPORT_ADD_OK[] = "041D043E043C04350440002004430441043F04350448043D043E00200434043E043104300432043B0435043D";
static const u8 SIM900_SMS_REPORT_ADD_ALREADY_ADDED[] = "041D043E043C04350440002004430436043500200434043E043104300432043B0435043D";
static const u8 SIM900_SMS_REPORT_ADD_NO_MEMORY[] = "04220435043B04350444043E043D043D044B043900200441043F044004300432043E0447043D0438043A002004370430043F043E043B043D0435043D";
static const u8 SIM900_SMS_REPORT_ADD_FLASH_ERROR[] = "00310034";
static const u8 SIM900_SMS_REPORT_ADD_FATAL_ERROR[] = "00310035";

static const u8 SIM900_SMS_REPORT_DEL_OK[] = "041D043E043C04350440002004430441043F04350448043D043E0020044304340430043B0435043D";
static const u8 SIM900_SMS_REPORT_DEL_NOT_FOUND[] = "00320032";
static const u8 SIM900_SMS_REPORT_DEL_FLASH_ERROR[] = "00320033";
static const u8 SIM900_SMS_REPORT_DEL_FATAL_ERROR[] = "00320034";

static const u8 SIM900_SMS_REPORT_CLEAN_OK[] = "0412044104350020043D043E043C0435044004300020044304340430043B0435043D044B";
static const u8 SIM900_SMS_REPORT_CLEAN_FLASH_ERROR[] = "00330032";
static const u8 SIM900_SMS_REPORT_CLEAN_FATAL_ERROR[] = "00330033";

static const u8 SIM900_SMS_REPORT_LEAK[] = "041F0440043E04380437043E0448043B04300020043F0440043E044204350447043A0430";
static const u8 SIM900_SMS_REPORT_CHECK_BATTERY[] = "0420043004370440044F04340438043B04380441044C00200431043004420430044004350439043A0438002E0020041F0440043E043204350440044C044204350020043604430440043D0430043B";
static const u8 SIM900_DMD_REPORT_LINK_LOST[] = "041F043E044204350440044F043D0430002004410432044F0437044C0020044100200443044104420440043E04390441044204320430043C0438002E0020041F0440043E043204350440044C044204350020043604430440043D0430043B";
static const u8 SIM900_SMS_REPORT_LINK_LOST_WITH_MAIN_CONTROLLER[] = "041F043E044204350440044F043D0430002004410432044F0437044C0020044100200433043B04300432043D044B043C0020043A043E043D04420440043E043B043B04350440043E043C002000570061007400650072004C00650061006B";
static const u8 SIM900_SMS_REPORT_BATTERY_LOW_IN_GSM_EXTENDER[] = "0411043004420430044004350439043A043800200432002000470053004D002D0053004D00530020043C043E04340443043B0435002004410435043B0438";
static const u8 SIM900_SMS_REPORT_EXTERNAL_SUPPLY_LOST[] = "0412043D04350448043D043504350020043F043804420430043D043804350020043C043E04340443043B044F002000470053004D002D0053004D00530020043F043E044204350440044F043D043E";

static const u8 SIM900_LEAK_REMOVED[] = "041A043B0430043F0430043D044B002004370430043A0440044B0442044B";

static const u8 SIM900_SMS_REPORT_CLOSE_OK[] = "0412044104350020043A043B0430043F0430043D044B002004370430043A0440044B0442044B";
static const u8 SIM900_SMS_REPORT_CLOSE_NOT_ALL[] = "041D043500200432044104350020043A043B0430043F0430043D044B002004370430043A0440044B0442044B";

static const u8 SIM900_SMS_REPORT_OPEN_OK[] = "0412044104350020043A043B0430043F0430043D044B0020043E0442043A0440044B0442044B";
static const u8 SIM900_SMS_REPORT_OPEN_NOT_ALL[] = "041D043500200432044104350020043A043B0430043F0430043D044B0020043E0442043A0440044B0442044B";

static const u8 SIM900_SMS_REPORT_CHECK_ALL_CLOSED[] = "0412044104350020043A043B0430043F0430043D044B002004370430043A0440044B0442044B";
static const u8 SIM900_SMS_REPORT_CHECK_ALL_OPENED[] = "0412044104350020043A043B0430043F0430043D044B0020043E0442043A0440044B0442044B";
static const u8 SIM900_SMS_REPORT_CHECK_NOT_ALL_OPENED[] = "041D043500200432044104350020043A043B0430043F0430043D044B0020043E0442043A0440044B0442044B";

static const u8 SIM900_SMS_REPORT_BALANCE_SET_OK[] = "041D043E043C043504400020043F04400438043D044F0442";
static const u8 SIM900_SMS_REPORT_BALANCE_SET_ERROR[] = "00340031";

static const u8 SIM900_SMS_REPORT_BALANCE_TELNUM_NOT_SET[] = "041D043E043C043504400020043F0440043E043204350440043A0438002004310430043B0430043D0441043000200435044904350020043D043500200443044104420430043D043E0432043B0435043D";
static const u8 SIM900_SMS_REPORT_BALANCE_ERROR[] = "041E044804380431043A04300020043F04400438002004370430043F0440043E04410435002004310430043B0430043D04410430";

static const u8 SIM900_SMS_REPORT_DEGREE_MARK[] = "00200433044004300434002E";

static const u8 SIM900_SMS_REPORT_LOAD1_ON[] =  "0423044104420440043E0439044104420432043E0020003100200432043A043B044E04470435043D043E";
static const u8 SIM900_SMS_REPORT_LOAD1_OFF[] = "0423044104420440043E0439044104420432043E0020003100200432044B043A043B044E04470435043D043E";
static const u8 SIM900_SMS_REPORT_LOAD2_ON[] = "0423044104420440043E0439044104420432043E0020003200200432043A043B044E04470435043D043E";
static const u8 SIM900_SMS_REPORT_LOAD2_OFF[] = "0423044104420440043E0439044104420432043E0020003200200432044B043A043B044E04470435043D043E";

#define SIM900_WAIT_FOR_RESPONSE_TIMEOUT 150

void SIM900_ReInit(void);
void SIM900_ReadSms(void);
void SIM900_SendSms(void);
u8 SIM900_PowerOn(void);
u8 SIM900_PowerOff(void);
u8 SIM900_SoftReset(void);
u8 SIM900_HoldReset(void);
u8 SIM900_GetStatus(void);
void SIM900_CircularBuffer_Purge(void);
s16 SIM900_CircularBuf_Search(const u8 pattern[]);
u8 SIM900_CircularBuffer_Extract(const u8 Pattern[], u8 *Dst, u16 Num, u8 DelChar);
u8 SIM900_CircularBuffer_ExtractTelNum(u8 *Dst);
u8 SIM900_CircularBuffer_ExtractBalanceNum(const u8 Pattern[], u8 *Dst, u16 Num);
u8 SIM900_WaitForResponse(u8 *pos_resp, u8 *neg_resp);
u8 IsTelNumberSymbol(const u8 symbol[]);
void SIM900_SendStr(u8* str);


#endif