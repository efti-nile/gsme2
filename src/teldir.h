#ifndef _TELDIR_H_
#define TELDIR

#include "io430.h"
#include "types.h"
#include <string.h>

// Results
#define TELDIR_RES_FLASH_ERROR          3

#define TELDIR_PUSH_RES_PUSHED          0
#define TELDIR_PUSH_RES_ALREADY_PUSHED  1
#define TELDIR_PUSH_RES_NO_MEMORY       2

#define TELDIR_DEL_RES_DELETED          0
#define TELDIR_DEL_RES_NOT_FOUND        1

#define TELDIR_CLEAN_RES_CLEANED        0

#define TELDIR_SET_BALANCE_TELNUM_RES_OK   0

#define TELDIR_TELNUM_LEN (44+1)
#define TELDIR_SIZE 10

struct TelDir_TypeDef{
    u8 List[TELDIR_SIZE][TELDIR_TELNUM_LEN];
	u8 BalanceTelNum[TELDIR_TELNUM_LEN];
    u32 IfBalanceTelNumSet;
    u32 NumItems;
    u32 Iterator;
};

#define FLASH_END 0x004000
#define FLASH_PAGESIZE 50
#define FLASH_TELDIR (FLASH_END - (sizeof(TelDir)/FLASH_PAGESIZE + 1)*FLASH_PAGESIZE)

void TelDir_Init(void);
u32 TelDir_IfBalanceTelNumSet(void);
u32 TelDir_SetBalanceNumber(u8 *TelNum);
u8 *TelDir_GetBalanceNumber(void);
u32 TelDir_Clean(void);
u32 TelDir_Push(u8 *TelNum);
u32 TelDir_Del(u8 *TelNum);
s32 TelDir_FindTelNumber(u8 *TelNum);
void TelDir_Iterator_Init(void);
u32 TelDir_GetNextTelNum(u8 *TelNum);
u32 TelDir_WriteToFlash(void);
#endif
