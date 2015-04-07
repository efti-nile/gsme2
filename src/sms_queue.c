#include "sms_queue.h"

struct SMS_Queque_TypeDef SMS_Queue;

void SMS_Queue_Init(void){
    SMS_Queue.FirstItem = 0;
    SMS_Queue.LastItem = 0;
    SMS_Queue.NumItems = 0;
}

void SMS_Queue_Push(u8 *TelNum, const u8 *SmsText, u32 LifeTime){
    TelNum[SMS_TELNUM_LEN-1] = '\0';
    strcpy(SMS_Queue.List[SMS_Queue.LastItem].TelNum, TelNum);
    SMS_Queue.List[SMS_Queue.LastItem].SmsText = (u8 *)SmsText;
    SMS_Queue.List[SMS_Queue.LastItem].LifeTime = LifeTime;
    SMS_Queue.LastItem = SMS_Queue.LastItem < SMS_QUEUE_MAXSIZE - 1 ? ++SMS_Queue.LastItem : 0;
    if(SMS_Queue.LastItem == SMS_Queue.FirstItem){
        SMS_Queue.FirstItem = SMS_Queue.FirstItem < SMS_QUEUE_MAXSIZE - 1 ? ++SMS_Queue.FirstItem : 0;
    }
    SMS_Queue.NumItems++;
}

u32 SMS_Queue_Pop(u8 *TelNum, u8 **SmsText){
    u32 LifeTime;
    if(SMS_Queue.NumItems > 0){
        strcpy(TelNum, SMS_Queue.List[SMS_Queue.FirstItem].TelNum);
        *SmsText = SMS_Queue.List[SMS_Queue.FirstItem].SmsText;
        LifeTime = SMS_Queue.List[SMS_Queue.FirstItem].LifeTime;
        SMS_Queue.FirstItem = SMS_Queue.FirstItem < SMS_QUEUE_MAXSIZE - 1 ? ++SMS_Queue.FirstItem : 0;
        SMS_Queue.NumItems--;
        return LifeTime;
    }else{
        return 0;
    }
}

u32 SMS_Queue_NumItems(void){
    return SMS_Queue.NumItems;
}
