#include "main.h"



int main(void)
{
    // Stop watchdog timer to prevent time out reset
    // It is good to enable watchdog timer in production
    WDTCTL = WDTPW + WDTHOLD;

    //TelDir_Clean();
    //TelDir_SetBalanceNumber("002A0031003000300023"); // Delete in production
    MSP430_UCS_Init();
    MSP430_UART_Init();
    while(1){
        while (!(UCA1IFG&UCTXIFG));
        UCA1TXBUF = 0xAA;
        for(u16 i = 0; i < 10000; ++i);
    }


    /*
    Init();

    State.sim900_initialized = 1;


    while(1){
        State.sim900_initialized = SIM900_GetStatus();

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
            TelDir_Iterator_Init();
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

    }*/
}
/*
void SIM900_ReadSms(void){
    u8 TelNum[SMS_TELNUM_LEN], TelNum_Balance[SMS_TELNUM_LEN];

    SIM900_CircularBuffer_Purge(); // Just in case

    SIM900_SendStr("AT+CMGR=1\r"); // Read received SMS

    if(!SIM900_WaitForResponse("OK", "ERROR")){
        // If we didn't receive response delete all
        // SMSs then return to the main loop
        Delay_DelayMs(5000);
        SIM900_SendStr("AT+CMGD=1,4\r");
        if(!SIM900_WaitForResponse("OK", "ERROR")){
            ErrorHandler(7);
        }
        SIM900_CircularBuffer_Purge();
        return;
    }

    // Extract telephone number which sent the SMS
    SIM900_CircularBuffer_ExtractTelNum(TelNum);

    TelNum[3] = '8'; // Change '7' to '8'

    // Add the telephone number to the telephone dictionary
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_ADD)){
        switch(TelDir_Push(TelNum)){
        // Telephone number has been pushed successfully
            case TELDIR_PUSH_RES_PUSHED:{
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_ADD_OK, SMS_LIFETIME);
                break;
            }
        // Telephone number has been already pushed
            case TELDIR_PUSH_RES_ALREADY_PUSHED:{
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_ADD_ALREADY_ADDED, SMS_LIFETIME);
                break;
            }
        // There isn't enough memory to push telephone number
            case TELDIR_PUSH_RES_NO_MEMORY:{
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_ADD_NO_MEMORY, SMS_LIFETIME);
                break;
            }
        // Flash writing failed
            case TELDIR_RES_FLASH_ERROR:{
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_ADD_FLASH_ERROR, SMS_LIFETIME);
                break;
            }
        // It never must be
            default:{
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_ADD_FATAL_ERROR, SMS_LIFETIME);
                break;
            }
        }
    }else
    // Delete telephone number from the dictionary
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_DEL)){
        switch(TelDir_Del(TelNum)){
        // Telephone number has been deleted successfully
            case TELDIR_DEL_RES_DELETED:{
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_DEL_OK, SMS_LIFETIME);
                break;
            }
        // Flash writing failed
            case TELDIR_RES_FLASH_ERROR:{
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_DEL_FLASH_ERROR, SMS_LIFETIME);
                break;
            }
        // It never must be
            default:{
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_DEL_FATAL_ERROR, SMS_LIFETIME);
                break;
            }
        }
    }else
    // Clean the telephone dictionary
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_CLEAN) &&
       TelDir_FindTelNumber(TelNum) != -1){
        switch(TelDir_Clean()){
        // The telephone dictionary cleaned
            case TELDIR_CLEAN_RES_CLEANED:{
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_CLEAN_OK, SMS_LIFETIME);
                break;
            }
        // Flash writing failed
            case TELDIR_RES_FLASH_ERROR:{
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_CLEAN_FLASH_ERROR, SMS_LIFETIME);
                break;
            }
        // It never must be
            default:{
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_CLEAN_FATAL_ERROR, SMS_LIFETIME);
                break;
            }
        }
    }else
    // Check GSM-link and valves-state simultaneously
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_CHECK)){
        if(State.ok_timeout > 0){
            if(InPack.COMMAND & IN_COMMAND_AVC){
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_CHECK_ALL_CLOSED, SMS_LIFETIME);
            }else
            if(InPack.COMMAND & IN_COMMAND_AVO){
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_CHECK_ALL_OPENED, SMS_LIFETIME);
            }else{
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_CHECK_NOT_ALL_OPENED, SMS_LIFETIME);
            }
        }else{
            SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_LINK_LOST_WITH_MAIN_CONTROLLER, SMS_LIFETIME);
        }
    }else
    // Request to close all valves
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_CLOSE) && TelDir_FindTelNumber(TelNum) != -1){
        strcpy(State.TelNumOfSourceOfRequest, TelNum);
        State.request_close_all_valves = 1;
        State.close_valves_timeout = CLOSE_VALVES_TIMEOUT; // Timeout
    }else
    // Request to open all valves
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_OPEN) && TelDir_FindTelNumber(TelNum) != -1){
        strcpy(State.TelNumOfSourceOfRequest, TelNum);
        State.request_open_all_valves = 1;
        State.open_valves_timeout = OPEN_VALVES_TIMEOUT; // Timeout
    }else
    // Set number for balance checking
    if(SIM900_CircularBuffer_ExtractBalanceNum(SIM900_SMS_CMD_SET_BALANCE, TelNum_Balance, sizeof(TelNum_Balance) - 1) &&
        TelDir_FindTelNumber(TelNum) != -1)
    {
        if(TelDir_SetBalanceNumber(TelNum_Balance) == TELDIR_SET_BALANCE_TELNUM_RES_OK){
            SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_BALANCE_SET_OK, SMS_LIFETIME);
        }else{
            SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_BALANCE_SET_ERROR, SMS_LIFETIME);
        }
    }else
    // Request balance
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_CHECK_BALANCE) && TelDir_FindTelNumber(TelNum) != -1){
        // Check if user set the telephone number for balance check
        if(TelDir_IfBalanceTelNumSet()){
            // Make up command to request balance
            u8 CMD[sizeof("AT+CUSD=1,\"AAAABBBBCCCCDDDDEEEE\"\r") + 8] = "AT+CUSD=1,\""; // 8 for just in case
            strcat(CMD, TelDir_GetBalanceNumber());
            strcat(CMD, "\"\r");

            SIM900_SendStr(CMD);

            if(SIM900_WaitForResponse("+CUSD: 0,\"", "ERROR")){
                SIM900_CircularBuffer_Extract(",\"", SMS_Balance, sizeof(SMS_Balance) - 1, '"');
                SMS_Queue_Push(TelNum, SMS_Balance, SMS_LIFETIME);
            }else{
                SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_BALANCE_ERROR, SMS_LIFETIME);
            }
        }else{
            // Ask user to set this telephone number
            SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_BALANCE_TELNUM_NOT_SET, SMS_LIFETIME);
        }
    }

    SIM900_CircularBuffer_Purge();

    SIM900_SendStr("AT+CMGD=1,4\r");
    if(!SIM900_WaitForResponse("OK", "ERROR")){
        ErrorHandler(8);
        SIM900_CircularBuffer_Purge();
        return;
    }

    SIM900_CircularBuffer_Purge();
}

void SIM900_SendSms(void){
    u8 TelNum[SMS_TELNUM_LEN];
    u8 *SmsText;
    u32 LifeTime;

    // Pop a SMS from the queue. If life-time elapsed -- return from here
    // and don't send the SMS
    if((LifeTime = SMS_Queue_Pop(TelNum, &SmsText)) == 0){
        return;
    }

    // SMS sending...
    SIM900_SendStr("AT+CMGS=\"");
    SIM900_SendStr(TelNum);
    SIM900_SendStr("\"\r");

    if(!SIM900_WaitForResponse(">", "ERROR")){
        ErrorHandler(9);
        SIM900_CircularBuffer_Purge();
        return;
    }
    SIM900_CircularBuffer_Purge();

    Delay_DelayMs(5000);

    SIM900_SendStr(SmsText); // Send SMS text
    SIM900_SendStr("\x1A\r"); // End of SMS text

    // If SMS wasn't sent -- push it in the queue to send it one more
    // time later
    if(!SIM900_WaitForResponse("OK", "ERROR")){
        SMS_Queue_Push(TelNum, SmsText, LifeTime - 1);
    }
}

u32 SIM900_CircularBuf_Search(const u8 pattern[]){
    u32 i, j, k, l, p;

    // Find length of given pattern
    for(l = 0; pattern[l] != '\0'; ++l);

    // Check if circular buffer can fit in given pattern
    if(l > CirBuf_NumBytes){
        return 0;
    }

    // Index of byte before the last received one
    p = CirBuf_Tail > 0 ? --CirBuf_Tail : CIRBUF_SIZE - 1;

    // Calculate offset for search
    if(CirBuf_Head > l - 1){
        i = CirBuf_Head - l;
    }else{
        i = (CIRBUF_SIZE - 1) - (l - CirBuf_Head - 1);
    }

    while(i != p){
        for(j = i, k = 0; k < l; ++k){
            if(pattern[k] != CirBuf[j]){
                break;
            }else
            if(k == l - 1){
                return 1;
            }
            j = j < CIRBUF_SIZE - 1 ? ++j : 0;
        }
        i = i > 0 ? --i : CIRBUF_SIZE - 1;
    }

    return 0;
}

u32 SIM900_CircularBuffer_Extract(const u8 Pattern[], u8 *Dst, u32 Num, u8 DelChar){
    u32 i, j, k, l, p, q;

    // Find length of given pattern
    for(l = 0; Pattern[l]; ++l);

    // Check if circular buffer can fit in given pattern
    if(CirBuf_NumBytes < l){
        return 0;
    }

    // Index of byte before the last received one
    p = CirBuf_Tail > 0 ? --CirBuf_Tail : CIRBUF_SIZE - 1;

    // Calculate offset for search
    if(CirBuf_Head >= l){
        i = CirBuf_Head - l;
    }else{
        i = CIRBUF_SIZE - (l - CirBuf_Head);
    }

    for( ; i != p; i = i > 0 ? --i : CIRBUF_SIZE - 1){
        for(    j = i, k = 0;
                Pattern[k] == CirBuf[j];
                j = j < CIRBUF_SIZE - 1 ? ++j : 0 ){
            if(k++ >= l - 1){
                j = j < CIRBUF_SIZE - 1 ? ++j : 0; // set pointer to the begin
                for(    q = 0;
                        q < Num && CirBuf[j] != DelChar;
                        j = j < CIRBUF_SIZE - 1 ? ++j : 0, ++q  ) Dst[q] = CirBuf[j];
                Dst[q] = '\0';
                return 1;
            }
        }
    }
    return 0;
}

u32 SIM900_CircularBuffer_ExtractTelNum(u8 *Dst){
    u32 i, j, k, l, p, q;
    u8 pattern[] = "\",\"002B";

    // Find length of the pattern
    l = sizeof(pattern) - 1;

    // Check if circular buffer can fit in the pattern
    if(CirBuf_NumBytes < l){
        return 0;
    }

    // Index of byte before the last received one
    p = CirBuf_Tail > 0 ? --CirBuf_Tail : CIRBUF_SIZE - 1;

    // Calculate offset for search
    if(CirBuf_Head > l - 1){
        i = CirBuf_Head - l;
    }else{
        i = (CIRBUF_SIZE - 1) - (l - CirBuf_Head - 1);
    }

    while(i != p){
        for(j = i, k = 0; k < l; ++k){
            if(pattern[k] != CirBuf[j]){
                break;
            }else
            if(k == l - 1){
                j = j < CIRBUF_SIZE - 1 ? ++j : 0; // set pointer to the begin
                for(q = 0; q < 44; ++q){
                    Dst[q] = CirBuf[j];
                    j = j < CIRBUF_SIZE - 1 ? ++j : 0;
                }
                Dst[q] = '\0';
                return 1;
            }
            j = j < CIRBUF_SIZE - 1 ? ++j : 0;
        }
        i = i > 0 ? --i : CIRBUF_SIZE - 1;
    }

    return 0;
}

u32 SIM900_CircularBuffer_ExtractBalanceNum(const u8 Pattern[], u8 *Dst, u32 Num){
    u32 i, j, k, l, p, q;

    // Find length of given pattern
    for(l = 0; Pattern[l]; ++l);

    // Check if circular buffer can fit in given pattern
    if(CirBuf_NumBytes < l){
        return 0;
    }

    // Index of byte before the last received one
    p = CirBuf_Tail > 0 ? --CirBuf_Tail : CIRBUF_SIZE - 1;

    // Calculate offset for search
    if(CirBuf_Head >= l){
        i = CirBuf_Head - l;
    }else{
        i = CIRBUF_SIZE - (l - CirBuf_Head);
    }

    for( ; i != p; i = i > 0 ? --i : CIRBUF_SIZE - 1){
        for(    j = i, k = 0;
                Pattern[k] == CirBuf[j];
                j = j < CIRBUF_SIZE - 1 ? ++j : 0 )
        {
            if(k++ >= l - 1){
                j = j < CIRBUF_SIZE - 1 ? ++j : 0; // set pointer to the begin
                for(    q = 0;
                        q < Num;
                        j = j < CIRBUF_SIZE - 1 ? ++j : 0, ++q  )
                {
                    Dst[q] = CirBuf[j];
                    if( q + 1 > 0 && (q + 1) % 4 == 0 ){
                        if(!IsTelNumberSymbol( Dst + (q - 3) )){
                            Dst[q-3] = '\0';
                            return 1;
                        }
                    }
                }

                Dst[q] = '\0';
                return 1;
            }
        }
    }
    return 0;
}

u32 SIM900_WaitForResponse(u8 *pos_resp, u8 *neg_resp){
    u32 timeout = SIM900_WAIT_FOR_RESPONSE_TIMEOUT;
    while(timeout--){
        Delay_DelayMs(100);
        if(SIM900_CircularBuf_Search(pos_resp)){
            return 1;
        }
        if(SIM900_CircularBuf_Search(neg_resp)){
            return 0;
        }
    }
    return 0;
}

void SIM900_ReInit(void){
func_begin:
    SIM900_PowerOff(); // Power-off SIM900

    LED_Off();
    Delay_DelayMs(500);
    LED_On();
    Delay_DelayMs(500);
    LED_Off();

    SIM900_PowerOn(); // Start SIM900

    Delay_DelayMs(10);

    SIM900_SoftReset();

    Delay_DelayMs(10000);

    // Switch off echo
    SIM900_SendStr("ATE0\r");
    if(!SIM900_WaitForResponse("OK", "ERROR")){
        goto func_begin;
    }
    SIM900_CircularBuffer_Purge();

    // Set full error mode
    SIM900_SendStr("AT+CMEE=2\r");
    if(!SIM900_WaitForResponse("OK", "ERROR")){
        goto func_begin;
    }
    SIM900_CircularBuffer_Purge();

    // Text mode
    SIM900_SendStr("AT+CMGF=1\r");
    if(!SIM900_WaitForResponse("OK", "ERROR")){
        goto func_begin;
    }
    SIM900_CircularBuffer_Purge();

    // Character Set
    SIM900_SendStr("AT+CSCS=\"UCS2\"\r");
    if(!SIM900_WaitForResponse("OK", "ERROR")){
        goto func_begin;
    }
    SIM900_CircularBuffer_Purge();

    // Extra settings
    SIM900_SendStr("AT+CSMP=17,167,0,25\r");
    if(!SIM900_WaitForResponse("OK", "ERROR")){
        goto func_begin;
    }
    SIM900_CircularBuffer_Purge();

    // Delete all SMS
    SIM900_SendStr("AT+CMGD=1,4\r");
    if(!SIM900_WaitForResponse("OK", "ERROR")){
        goto func_begin;
    }
    SIM900_CircularBuffer_Purge();

    // Give SIM900 some time for thinking
    Delay_DelayMs(10000);

    if(!SIM900_GetStatus()){
        goto func_begin;
    }
}

void ErrorHandler(u32 ErrNum){
    u32 i;

    LED_Off();

    Delay_DelayMs(4000);

    for(i = 0; i < ErrNum; ++i){
        Delay_DelayMs(300);
        LED_On();
        Delay_DelayMs(300);
        LED_Off();
    }

    SIM900_ReInit();
}

void SIM900_SendStr(u8* str){
    while( *str != '\0' ){
        USART_SendData(USART2, *str);
        while(!(USART2->SR & USART_SR_TXE));
        str++;
    }
}

void SIM900_CircularBuffer_Purge(void){
    CirBuf_Tail = CirBuf_Head;
    CirBuf_NumBytes = 0;
}

u32 IsTelNumberSymbol(const u8 symbol[]){
    // Returns if symbol[5] is telephone number symbol,
    // i.e. if it is representation of 0..9, * or #
    // symbol in UCS2 code table.
    if(symbol[0] == '0' && symbol[1] == '0'){
        if(symbol[2] == '3'){
            return 1;
        }else
        if(symbol[2] == '2'){
            if(symbol[3] == '3' || symbol[3] == 'A'){
                return 1;
            }
        }
    }
    return 0;
}

void TIM3_IRQHandler(void){
    if(TIM3->SR & TIM_SR_UIF){
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
			LED_Off();
		}
    }
    TIM3->SR = 0;
}

void OkStatus_Update(void){
	State.ok_timeout = OK_TIMEOUT;
	LED_On();
}

u32 TIM3_Start(u16 timeout){
    NVIC_InitTypeDef  NVIC_InitStruct;

    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // enable clock for TIM3

    TIM3->PSC   = 24000;  // prescaller
    TIM3->CNT   = 0x0000; // counter
    TIM3->ARR   = 1000; // auto reload period

    NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStruct); // configure NVIC
    TIM3->DIER |= TIM_DIER_UIE; // enable overflow interrupt

    TIM3->CR1 |= TIM_CR1_CEN; // start timer

    return 0;
}

void USART2_IRQHandler(void){
    if(USART2->SR & USART_SR_RXNE){
        u8 tmp = USART2->DR & 0xFF;

        CirBuf[CirBuf_Head++] = tmp;

        if(CirBuf_Head >= CIRBUF_SIZE){
            CirBuf_Head = 0;
        }

        if(CirBuf_Tail == CirBuf_Head){
            if(++CirBuf_Tail >= CIRBUF_SIZE){
                CirBuf_Tail = 0;
            }
        }

        ++CirBuf_NumBytes;
    }
}

void USART1_IRQHandler(void){
    static u32 num_received_bytes = 0;
    volatile u32 tmp, if_address;

    if(USART_GetITStatus(USART1, USART_IT_RXNE) != SET){
        return;
    }

    tmp = (u32)USART_ReceiveData(USART1);
    if_address = tmp >> 8 & 0x01;

    // Save received byte
    ((u8*)&InPack)[num_received_bytes++] = (u8)(tmp & 0xFF);

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
        USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
        if(InPack.crc == CRC_Calc((u8*)&InPack, sizeof(InPack)-1)){

			if(SIM900_GetStatus() && State.sim900_initialized){
                OkStatus_Update(); // Ready-to-work LED
            }

            if(InPack.COMMAND & IN_COMMAND_AVC){
                if(State.request_close_all_valves){
                    State.request_close_all_valves = 0;
                    SMS_Queue_Push(State.TelNumOfSourceOfRequest, SIM900_SMS_REPORT_CLOSE_OK, SMS_LIFETIME);
                }
            }else
            if(InPack.COMMAND & IN_COMMAND_AVO){
                if(State.request_open_all_valves){
                    State.request_open_all_valves = 0;
                    SMS_Queue_Push(State.TelNumOfSourceOfRequest, SIM900_SMS_REPORT_OPEN_OK, SMS_LIFETIME);
                }
            }

            State.leak_prev = State.leak_now;
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

            State.check_battery_prev = State.check_battery_now;
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

            State.link_lost_prev = State.link_lost_now;
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

            State.leak_removed_prev = State.leak_removed_now;
            if(InPack.COMMAND & IN_COMMAND_LEAKR){
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

            if(State.request_close_all_valves){
                SendCmd(RESPONSE_CLOSE_ALL);
            }else
            if(State.request_open_all_valves){
                SendCmd(RESPONES_OPEN_ALL);
            }else{
                SendCmd(RESPONSE_NO_EVENTS);
            }
        }

        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

        num_received_bytes = 0;
    }
}

void SendCmd(u8 cmd){
    u32 i;

    OutPack.DevID = State.controller_address;
    OutPack.Length = sizeof(OutPack) - 2; // Excluding DevID & Length
    OutPack.SourceAddress = MY_ADDRESS;
    OutPack.TID = 0;
    OutPack.COMMAND = cmd;

    OutPack.crc = CRC_Calc((u8*)&OutPack, sizeof(OutPack)-1);

    GPIO_WriteBit(RxTx_RS485_GPIO, RxTx_RS485_Pin, RxTx_RS485_TxEnable);
	
    i = 7000;
    while(i--) __NOP();

    // Send address byte
    USART_SendData(USART1, OutPack.DevID | 1<<8);
    while(!(USART1->SR & USART_SR_TXE));

    // Send data bytes
    for(i = 1; i < sizeof(OutPack); ++i){
        USART_SendData(USART1, ((u8*)&OutPack)[i] & 0xFF);
        while(!(USART1->SR & USART_SR_TXE));
    }

    i = 7000*3;
    while(i--) __NOP();

    GPIO_WriteBit(RxTx_RS485_GPIO, RxTx_RS485_Pin, RxTx_RS485_RxEnable);
}

u32 Init(void){
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    //////////////////////// USART1 //////////////////////////////////
    // USART1_TX
    GPIO_InitStruct.GPIO_Pin    = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_10MHz;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // USART1_RX
    GPIO_InitStruct.GPIO_Pin    = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_10MHz;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
	
    USART_InitStruct.USART_BaudRate	= 9600;
    USART_InitStruct.USART_WordLength = USART_WordLength_9b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_HardwareFlowControl
                                     = USART_HardwareFlowControl_None;

    USART_Init(USART1, &USART_InitStruct);
    USART_Cmd(USART1, ENABLE);

    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    //////////////////////// USART2 //////////////////////////////////
    // USART2_TX
    GPIO_InitStruct.GPIO_Pin    = GPIO_Pin_2;
    GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_10MHz;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // USART2_RX
    GPIO_InitStruct.GPIO_Pin    = GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_10MHz;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
	
    USART_InitStruct.USART_BaudRate	= 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_HardwareFlowControl
                                     = USART_HardwareFlowControl_None;

    USART_Init(USART2, &USART_InitStruct);
    USART_Cmd(USART2, ENABLE);

    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    //////////////////////// GPIO ////////////////////////////////////
    g_HPWR_INIT;
    g_PWR_INIT;
    g_STS_INIT;

    GPIO_InitStruct.GPIO_Pin    = RxTx_RS485_Pin;
    GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_Out_PP;
    GPIO_Init(RxTx_RS485_GPIO, &GPIO_InitStruct);
    GPIO_WriteBit(RxTx_RS485_GPIO, RxTx_RS485_Pin, RxTx_RS485_RxEnable);

    GPIO_InitStruct.GPIO_Pin    = LED_Pin;
    GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_Out_PP;
    GPIO_Init(LED_GPIO, &GPIO_InitStruct);
    GPIO_WriteBit(LED_GPIO, LED_Pin, Bit_RESET);

    Delay_Init();
    SMS_Queue_Init();
    TelDir_Init();
    PowMeas_Init();
    SIM900_ReInit();

    TIM3_Start(1000); // GSM Extender System Timer

    return 0;
}

void LED_On(void){
    GPIO_WriteBit(LED_GPIO, LED_Pin, Bit_SET);
}

void LED_Off(void){
    GPIO_WriteBit(LED_GPIO, LED_Pin, Bit_RESET);
}

u8 CRC_Calc(u8* src, u32 num){
    u8 crc = 0xAA;
    while(num--) crc ^= *src++;
    return crc;
}*/

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