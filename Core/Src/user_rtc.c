#include "user_rtc.h"
#include "main.h"
#include "oled.h"
#include "4G_WIFI_TCP.h"

extern RTC_HandleTypeDef hrtc;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim3;

uint32_t RUN_TIME = 0;

RTC_TimeTypeDef  User1_time={0};
RTC_DateTypeDef  User1_data={0};

void user_clock_init(void)
{
    User1_time.Hours=User1.hour;
    User1_time.Minutes=User1.minute;
    User1_time.Seconds=User1.seconds;

    User1_data.Year=User1.year % 100;
    User1_data.Month=User1.month;
    User1_data.Date=User1.day;
    
    HAL_RTC_SetTime(&hrtc, &User1_time, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &User1_data, RTC_FORMAT_BIN);
}

void user_clock_printf(void)
{
    HAL_RTC_GetTime(&hrtc, &User1_time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &User1_data, RTC_FORMAT_BIN);
    
    OLED_ShowNum(1,1,2000 + User1_data.Year,4);
    OLED_ShowString(1,5,".");
    OLED_ShowNum(1,6,User1_data.Month,2);
    OLED_ShowString(1,8,".");
    OLED_ShowNum(1,9,User1_data.Date,2);

    OLED_ShowNum(2,1,User1_time.Hours,2);
    OLED_ShowString(2,3,":");
    OLED_ShowNum(2,4,User1_time.Minutes,2);
    OLED_ShowString(2,6,":");
    OLED_ShowNum(2,7,User1_time.Seconds,2); 
        
    if(User1_time.Seconds == 0)RUN_TIME++;
}

void user_weather_printf(void)
{
//    OLED_ShowNum(3,1,User1.code_day,2);
//    OLED_ShowString(3,3,".");
//    OLED_ShowNum(3,4,User1.code_night,2);
//    OLED_ShowString(3,6,".");
//    OLED_ShowNum(3,7,User1.high,2);
//    OLED_ShowString(3,9,".");
//    OLED_ShowNum(3,10,User1.low,2);
//    
//    OLED_ShowNum(4,1,User1.humidity,2);
//    OLED_ShowString(4,3,".");
//    OLED_ShowNum(4,4,User1.wind_scale,2);
//    OLED_ShowString(4,6,".");
//    OLED_ShowNum(4,7,User1.wind_speed,2);
//    OLED_ShowString(4,9,".");
//    OLED_ShowNum(4,10,User1.rainfall,2);
}

