#include "delay.h"

u32 Delay_Init(void){
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // enable clock for TIM2

    TIM2->PSC   = 24000;  // prescaller
    TIM2->CNT   = 0x0000; // counter
    TIM2->ARR   = 0xFFFF; // auto reload period
    TIM2->DIER |= 1 << 0; // enable overflow interrupt
    TIM2->CR1  |= 1 << 0; // start timer

    return 0;
}

void Delay_DelayMs(u32 value){
    if( value != 0 ){
        TIM2->CNT = 0;
            TIM2->EGR |= TIM_EGR_UG;
        while((u32)TIM2->CNT < value);
    }
}
