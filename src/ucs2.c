#include "ucs2.h"

u8 fromUCS2(u8 *c){
	u8 l = strlen((char const *)c);
	u16 t = 0;
	for(u8 i = 0; i < l; i++){
		t *= 16;
		t += hextodig(c[i]);
	}
	if(ENG_BEGIN <= t && t <= ENG_END){
		return t; // english characters in Windows 1251 the same as in Unicode
	}else if(t >= W1251_RUS_TABLE_BEGIN && t <= W1251_RUS_TABLE_END){
		return w1251_rus[t - W1251_RUS_TABLE_BEGIN];
	}
	return 0x20; // It must not be executed. Return space character if there is no such symbol in table.
}

u8 *toUCS2(u8 c){
	u16 t;
	if(ENG_BEGIN <= c && c <= ENG_END){
		t = c; // english characters in Windows 1251 the same as in Unicode
	}else if(UCS2_RUS_TABLE_BEGIN <= c && c <= UCS2_RUS_TABLE_END){
		t = ucs2_rus[c];
	}else{
		t = 0x0020;// space charachter
	};
	for(u8 i = 3; i != 0xFF; i--){
		ucs2_rv[i] = digtohex(t % 16);
		t /= 16;
	}
	ucs2_rv[4] = '\0';
	return ucs2_rv;
}

static u8 hextodig(u8 c){
	if(c >= '0' && c <= '9'){
		return c - '0';
	}else if(c >= 'A' && c <= 'F'){
		return 10 + (c - 'A');
	}else{
		return 0;
	}
}

static u8 digtohex(u8 n){
	if(n <= 9){
		return '0' + n;
	}else if (n >= 10 && n <= 15){
		return 'A' + n;
	}else{
		return 0;
	}
}