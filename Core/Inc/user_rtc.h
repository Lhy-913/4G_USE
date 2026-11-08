#ifndef __USER_RTC_H_
#define __USER_RTC_H_
#include "stdint.h"                  // Device header

extern uint32_t RUN_TIME;

void user_clock_init(void);
void user_clock_printf(void);
void user_weather_printf(void);


#endif
