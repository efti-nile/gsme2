#ifndef _UCS2_H
#define _UCS2_H

#include "string.h"
#include "types.h"

#define ENG_BEGIN 0x20
#define ENG_END 0x7E

#define UCS2_RUS_TABLE_BEGIN 0xC0
#define UCS2_RUS_TABLE_END 0xFF
#define UCS2_RUS_TABLE_SIZE (UCS2_RUS_TABLE_END - UCS2_RUS_TABLE_BEGIN  + 1)

#define W1251_RUS_TABLE_BEGIN 0x0410
#define W1251_RUS_TABLE_END 0x044E
#define W1251_RUS_TABLE_SIZE (W1251_RUS_TABLE_BEGIN + W1251_RUS_TABLE_END + 1)

const u16 ucs2_rus[UCS2_RUS_TABLE_SIZE];

const u8 w1251_rus[W1251_RUS_TABLE_SIZE];

u8 ucs2_rv[5];

u8 *toUCS2(u8 c);
u8 fromUCS2(u8 *c);
static u8 hextodig(u8 c);
static u8 digtohex(u8 n);
#endif
