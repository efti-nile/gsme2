#include "teldir.h"

struct TelDir_TypeDef TelDir;

void TelDir_Init(void){
    u32 i;
    
    // If it is the first device start
    if(*((u32*)FLASH_TELDIR) == 0xFFFFFFFF){
        TelDir_Clean();
    }

    for(i = 0; i < sizeof(TelDir); ++i){
        ((u8 *)&TelDir)[i] = *(u8*)(FLASH_TELDIR + sizeof(u8)*i);
    }
}

u32 TelDir_Clean(void){
    TelDir.NumItems = 0;
    TelDir.IfBalanceTelNumSet = 0;
    if(TelDir_WriteToFlash()){
        return TELDIR_CLEAN_RES_CLEANED;
    }else{
        return TELDIR_RES_FLASH_ERROR;
    }
}

u32 TelDir_Push(u8 *TelNum){
    u32 i;
    for(i = 0; i < TelDir.NumItems; ++i){
        if(strcmp(TelDir.List[i], TelNum) == 0){
            return TELDIR_PUSH_RES_ALREADY_PUSHED;
        }
    }
    if(TelDir.NumItems < TELDIR_SIZE){
        strcpy(TelDir.List[TelDir.NumItems++], TelNum);
        if(TelDir_WriteToFlash()){
            return TELDIR_PUSH_RES_PUSHED;
        }else{
            return TELDIR_RES_FLASH_ERROR;
        }
    }else{
        return TELDIR_PUSH_RES_NO_MEMORY;
    }
}

u32 TelDir_IfBalanceTelNumSet(void){
    return TelDir.IfBalanceTelNumSet;
}

u32 TelDir_SetBalanceNumber(u8 *TelNum){
    strcpy(TelDir.BalanceTelNum, TelNum);
    if(TelDir_WriteToFlash()){
        TelDir.IfBalanceTelNumSet = 1;
        return TELDIR_SET_BALANCE_TELNUM_RES_OK;
    }else{
        return TELDIR_RES_FLASH_ERROR;
    }
}

u8 *TelDir_GetBalanceNumber(void){
    return TelDir.BalanceTelNum;
}

u32 TelDir_Del(u8 *TelNum){
    u32 i, j;
    for(i = 0; i < TelDir.NumItems; ++i){
        if(strcmp(TelDir.List[i], TelNum) == 0){
            for( j = i; j + 1 < TelDir.NumItems; ++j){
                strcpy(TelDir.List[j], TelDir.List[j + 1]);
            }
            TelDir.NumItems--;
        }
    }
    if(TelDir_WriteToFlash()){
        return TELDIR_DEL_RES_DELETED;
    }else{
        return TELDIR_RES_FLASH_ERROR;
    }
}

s32 TelDir_FindTelNumber(u8 *TelNum){
    u32 i;
    for(i = 0; i < TelDir.NumItems; ++i){
        if(strcmp(TelDir.List[i], TelNum) == 0){
            return i;
        }
    }
    return -1;
}

void TelDir_Iterator_Init(void){
    TelDir.Iterator = 0;
}

u32 TelDir_GetNextTelNum(u8 *TelNum){
    if(TelDir.Iterator < TelDir.NumItems){
        strcpy(TelNum, TelDir.List[TelDir.Iterator++]);
        return 1;
    }else{
        return 0;
    }
}

u32 TelDir_WriteToFlash(void)
{
    u32 i;
    
    FLASH_Unlock();
    
    for(i = FLASH_TELDIR; i < FLASH_END; i += FLASH_PAGESIZE){
        FLASH_ErasePage(i);
    }
    
    for(i = 0; i < sizeof(TelDir)/4; ++i){
        FLASH_ProgramWord(FLASH_TELDIR + sizeof(u32)*i, ((u32 *)&TelDir)[i]);
    }
    
    FLASH_Lock();
    
    return 1;
}
