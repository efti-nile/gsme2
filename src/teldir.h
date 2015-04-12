#ifndef _TELDIR_H_
#define TELDIR

#include "io430.h"
#include "types.h"
#include <string.h>

// Results
// Function working with flash
#define TELDIR_RES_FLASH_ERROR          3

// Function TelDir_Push
#define TELDIR_PUSH_RES_PUSHED          0
#define TELDIR_PUSH_RES_ALREADY_PUSHED  1
#define TELDIR_PUSH_RES_NO_MEMORY       2

// Function TelDir_Del
#define TELDIR_DEL_RES_DELETED          0
#define TELDIR_DEL_RES_NOT_FOUND        1

// Function TelDir_Clean
#define TELDIR_CLEAN_RES_CLEANED        0

// Function TelDir_SetBalanceNumber
#define TELDIR_SET_BALANCE_TELNUM_RES_OK   0

// Telephone dirictory size
#define TELDIR_TELNUM_LEN (44+1) // Length of the one telephone number (UCS2 coding)
#define TELDIR_SIZE 10 // Number of slots for telephone numbers

struct TelDir_TypeDef{
    u8 List[TELDIR_SIZE][TELDIR_TELNUM_LEN];
	u8 BalanceTelNum[TELDIR_TELNUM_LEN];
    u8 isBalanceTelNumSet;
    u8 NumItems;
    u8 Iterator;
};

#define FLASH_END 0x00FF70 // 0x00FF80 Actualy
#define FLASH_SEGSIZE 512 // 512 bytes segment (MSP430F5)
#define FLASH_TELDIR (FLASH_END - (sizeof(TelDir)/FLASH_PAGESIZE + 1)*FLASH_PAGESIZE)

void TelDir_Init(void);
u8 TelDir_isBalanceTelNumSet(void);
u8 TelDir_SetBalanceNumber(u8 *TelNum);
u8 *TelDir_GetBalanceNumber(void);
u8 TelDir_Clean(void);
u8 TelDir_Push(u8 *TelNum);
u8 TelDir_Del(u8 *TelNum);
s8 TelDir_FindTelNumber(u8 *TelNum);
void TelDir_Iterator_Init(void);
u8 TelDir_GetNextTelNum(u8 *TelNum);
u8 TelDir_WriteToFlash(void);

#endif
