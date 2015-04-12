#ifndef _SMS_QUEUE_H_
#define _SMS_QUEUE_H_

#include "io430.h"
#include "types.h"
#include <string.h>

#define SMS_QUEUE_MAXSIZE (10+1)
#define SMS_TEXT_MAXLEN (200+1)
#define SMS_TELNUM_LEN (44+1)

#define SMS_LIFETIME 5

struct SMS_Queque_TypeDef{
    struct SMS_TypeDef{
        u8 LifeTime;
        u8 TelNum[SMS_TELNUM_LEN];
        u8 *SmsText;
    }List[SMS_QUEUE_MAXSIZE];
    u8 FirstItem;
    u8 LastItem;
    u8 NumItems;
};

void SMS_Queue_Init(void);
void SMS_Queue_Push(u8 *TelNum, const u8 SmsText[], u8 LifeTime);
u8 SMS_Queue_Pop(u8 *TelNum, u8 **SmsText);
u8 SMS_Queue_NumItems(void);

#endif
