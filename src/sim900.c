#include "sim900.h"

extern struct TelDir_TypeDef TelDir;

u8 SMS_Balance[SMS_TEXT_MAXLEN]; // TODO: maybe put it in stack?

/*!
    \brief Tryes to initialize SIM900 untill success.

    LED switched on for 0.5 second on eache attemption to initialize
    SIM900.
*/
void SIM900_ReInit(void){
func_begin:
    SIM900_PowerOff(); // Power-off SIM900

    /*
    BUG: System timer ISR turns of the led right after switched on.
    */

    LED_OFF;
    Delay_DelayMs(500);
    LED_ON;
    Delay_DelayMs(500);
    LED_OFF;

    SIM900_PowerOn(); // Start SIM900

    Delay_DelayMs(100); // To have in concern that this time may be not enough

    SIM900_SoftReset();

    Delay_DelayMs(3000);

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

    // Choose character Set
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
    Delay_DelayMs(3000);

    if(!SIM900_GetStatus()){
        goto func_begin;
    }

    LED_OFF;
    Delay_DelayMs(500);
    LED_ON;
    Delay_DelayMs(500);
    LED_OFF;
}

void SIM900_ReadSms(void){
    u8 TelNum[SMS_TELNUM_LEN], TelNum_Balance[SMS_TELNUM_LEN];

    SIM900_CircularBuffer_Purge(); // Just in case

    SIM900_SendStr("AT+CMGR=1\r"); // Read received SMS

    if(!SIM900_WaitForResponse("OK", "ERROR")){
        // If we didn't receive response delete all
        // SMSs then return to the main loop
        SIM900_SendStr("AT+CMGD=1,4\r");
        if(!SIM900_WaitForResponse("OK", "ERROR")){
            SIM900_SendStr("ERROR: 7");
            ErrorHandler(7);
        }
        SIM900_CircularBuffer_Purge();
        return;
    }

    // Extract telephone number which sent the SMS
    SIM900_CircularBuffer_ExtractTelNum(TelNum);

    TelNum[3] = '8'; // Change '7' to '8' TODO: WTF?

    // Add the telephone number to the telephone dictionary
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_ADD) != -1){
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
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_DEL) != -1){
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
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_CLEAN) != -1 &&
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
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_CHECK) != -1){
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
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_CLOSE) != -1 && TelDir_FindTelNumber(TelNum) != -1){
        u8 valveNameBuffer[VALVE_NAME_MAXLEN * 4];
        // Get the name of the group which open command will be executed on
        if(1 != SIM900_CircularBuffer_Extract(SIM900_SMS_CMD_CLOSE_, valveNameBuffer, sizeof(State.current_valves_group), '"')){
            strcpy((char *)State.current_valves_group, "all");
        }
        strToCP1251(State.current_valves_group, valveNameBuffer);
        strcpy((char *)State.TelNumOfSourceOfRequest, (char const *)TelNum);
        State.request_close_valves = 1;
        State.close_valves_timeout = CLOSE_VALVES_TIMEOUT; // Timeout
    }else
    // Request to open all valves
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_OPEN) != -1 && TelDir_FindTelNumber(TelNum) != -1){
        u8 valveNameBuffer[VALVE_NAME_MAXLEN * 4];
        // Get the name of the group which open command will be executed on
        if(1 != SIM900_CircularBuffer_Extract(SIM900_SMS_CMD_OPEN_, valveNameBuffer, sizeof(State.current_valves_group), '"')){
            strcpy((char *)State.current_valves_group, "all");
        }
        strToCP1251(State.current_valves_group, valveNameBuffer);
        strcpy((char *)State.TelNumOfSourceOfRequest, (char const *)TelNum);
        State.request_open_valves = 1;
        State.open_valves_timeout = OPEN_VALVES_TIMEOUT; // Timeout
    }else
    // Set number for balance checking
    if(SIM900_CircularBuffer_ExtractBalanceNum(SIM900_SMS_CMD_SET_BALANCE, TelNum_Balance, sizeof(TelNum_Balance) - 1) &&
        TelDir_FindTelNumber(TelNum) != -1){
        if(TelDir_SetBalanceNumber(TelNum_Balance) == TELDIR_SET_BALANCE_TELNUM_RES_OK){
            SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_BALANCE_SET_OK, SMS_LIFETIME);
        }else{
            SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_BALANCE_SET_ERROR, SMS_LIFETIME);
        }
    }else
    // Request balance
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_CHECK_BALANCE) != -1 && TelDir_FindTelNumber(TelNum) != -1){
        // Check if user set the telephone number for balance check
        if(TelDir_isBalanceNumberSet()){
            // Make up command to request balance
            // TODO: Why do not send this long command seperately?
            u8 CMD[sizeof("AT+CUSD=1,\"AAAABBBBCCCCDDDDEEEE\"\r") + 8] = "AT+CUSD=1,\""; // 8 for just in case
            strcat((char *)CMD, (char const *)TelDir_GetBalanceNumber());
            strcat((char *)CMD, "\"\r");

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
    }else
    // Turn load on
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_TURN_LOAD_ON) != -1 && TelDir_FindTelNumber(TelNum) != -1){
        if(SIM900_CircularBuf_Search("00200031") != -1){
            Loads_Command(LOAD1_ON);
			SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_LOAD1_ON, SMS_LIFETIME);
        }else
        if(SIM900_CircularBuf_Search("00200032") != -1){
            Loads_Command(LOAD2_ON);
            SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_LOAD2_ON, SMS_LIFETIME);
        }
    }
    // Turn load off
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_TURN_LOAD_OFF) != -1 && TelDir_FindTelNumber(TelNum) != -1){
        if(SIM900_CircularBuf_Search("00200031") != -1){
            Loads_Command(LOAD1_OFF);
			SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_LOAD1_OFF, SMS_LIFETIME);
        }else
        if(SIM900_CircularBuf_Search("00200032") != -1){
            Loads_Command(LOAD2_OFF);
			SMS_Queue_Push(TelNum, SIM900_SMS_REPORT_LOAD2_OFF, SMS_LIFETIME);
        }
    }else
    // Get temperature
    if(SIM900_CircularBuf_Search(SIM900_SMS_CMD_TEMPERATURE) != -1){
        u8 i = 0, d;
        u8 temp_sms[4*8+4];
        s8 temp = PowMeas_GetTemp();

        if(temp < 0){ // Write a minus in UCS2 if the temperature is negative
            temp_sms[i++] = '0';
            temp_sms[i++] = '0';
            temp_sms[i++] = '2';
            temp_sms[i++] = 'D';
            temp = -temp;
        }

        d = temp / 10;
        if(d > 0){
            temp_sms[i++] = '0';
            temp_sms[i++] = '0';
            temp_sms[i++] = '3';
            temp_sms[i++] = 0x30 + d;
        }

        temp_sms[i++] = '0';
        temp_sms[i++] = '0';
        temp_sms[i++] = '3';
        temp_sms[i++] = 0x30 + temp % 10;

        temp_sms[i++] = '0';
        temp_sms[i++] = '0';
        temp_sms[i++] = '2';
        temp_sms[i++] = '0';

        strcpy((char *)(temp_sms + i), (const char *)SIM900_SMS_REPORT_DEGREE_MARK);

        i += strlen((char const *)SIM900_SMS_REPORT_DEGREE_MARK);

        temp_sms[i++] = '\0';

        SMS_Queue_Push(TelNum, temp_sms, SMS_LIFETIME);
    }

    SIM900_CircularBuffer_Purge();

    SIM900_SendStr("AT+CMGD=1,4\r");
    if(!SIM900_WaitForResponse("OK", "ERROR")){
        SIM900_SendStr("ERROR: 8");
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
        SIM900_SendStr("ERROR 9");
        ErrorHandler(9);
        SIM900_CircularBuffer_Purge();
        return;
    }
    SIM900_CircularBuffer_Purge();

    Delay_DelayMs(5000);

    SIM900_SendStr(SmsText); // Send SMS text
    SIM900_SendStr("\x1A\r"); // End of SMS text

    // Try during several time intervals to receive acknoledgment
    for(u8 i = 0; i < 4; i++){
      if(SIM900_WaitForResponse("OK", "ERROR")){
        return;
      }
    }

    // If we didn't receive one, put outgoing SMS in queue again with decreased
    // lifetime
    SMS_Queue_Push(TelNum, SmsText, LifeTime - 1);

    // Then restart SIM900
    SIM900_SendStr("ERROR 6");
    ErrorHandler(6);
    SIM900_CircularBuffer_Purge();
}

u8 SIM900_PowerOn(void){
    g_HPWR_SET;
    g_PWR_SET;
    g_USART1_TX_ENABLE;
	return 0;
}

u8 SIM900_PowerOff(void){
    g_HPWR_CLEAR;
    g_PWR_CLEAR;
    g_USART1_TX_DISABLE;
    return 0;
}

u8 SIM900_SoftReset(void){
    g_PWR_CLEAR;
    Delay_DelayMs(1000);
    g_PWR_SET;
    return 0;
}

u8 SIM900_HoldReset(void){
    g_PWR_CLEAR;
    return 0;
}

u8 SIM900_GetStatus(void){
    return g_STS_READ;
}

void SIM900_SendStr(u8* str){
    MSP430_UART_Send(UART_SIM900, str, strlen((const char *)str));
}

void SIM900_CircularBuffer_Purge(void){
    CirBuf_Tail = CirBuf_Head;
    CirBuf_NumBytes = 0;
}

s8 SIM900_CircularBuf_Search(const u8 pattern[]){
    u16 i, j, k, l, p;

    // Find length of given pattern
    for(l = 0; pattern[l] != '\0'; ++l);

    // Check if circular buffer can fit in given pattern
    if(l > CirBuf_NumBytes){
        return -1;
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
                return i;
            }
            j = j < CIRBUF_SIZE - 1 ? ++j : 0;
        }
        i = i > 0 ? --i : CIRBUF_SIZE - 1;
    }

    return -1;
}

u8 SIM900_CircularBuffer_Extract(const u8 Pattern[], u8 *Dst, u16 Num, u8 DelChar){
    u16 i, j, k, l, p, q;

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

u8 SIM900_CircularBuffer_ExtractTelNum(u8 *Dst){
    u16 i, j, k, l, p, q;
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

u8 SIM900_CircularBuffer_ExtractBalanceNum(const u8 Pattern[], u8 *Dst, u16 Num){
    u16 i, j, k, l, p, q;

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

u8 SIM900_WaitForResponse(u8 *pos_resp, u8 *neg_resp){
    u32 timeout = SIM900_WAIT_FOR_RESPONSE_TIMEOUT; // TODO: may be not u42?
    while(timeout--){
        Delay_DelayMs(100);
        if(SIM900_CircularBuf_Search(pos_resp) != -1){
            return 1;
        }
        if(SIM900_CircularBuf_Search(neg_resp) != -1){
            return 0;
        }
    }
    return 0;
}

/*!
    \brief Returns true if given UCS2-symbol may be in tel. number

    The function returns true if spcified UCS2-symbol may be
    in telephone number. I.e. if the symbol is 0..9, * or #.
*/
u8 IsTelNumberSymbol(const u8 symbol[]){
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