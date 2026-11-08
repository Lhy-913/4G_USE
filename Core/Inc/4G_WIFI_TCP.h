#ifndef __4G_WIFI_TCP_H_
#define __4G_WIFI_TCP_H_

#include "fifo.h"
#include <stdint.h>
#include "4G_dma.h"

//WIFI状态枚举
enum
{
    WIFI_NOT = 0,
    WIFI_OK,
    WIFI_ERROR
};

typedef struct
{
    uint16_t year;           // 年
    uint8_t month;           // 月
    uint8_t day;             // 日
    uint8_t hour;            // 时
    uint8_t minute;          // 分
    uint8_t seconds;         // 秒    
    
                                               //例:
    uint8_t code_day;        // 白天天气代码   → "13"
    uint8_t code_night;      // 夜间天气代码   → "14"
    uint8_t high;            // 最高温度       → "30"
    uint8_t low;             // 最低温度       → "24"
    uint8_t humidity;        // 湿度           → "98"
    uint8_t wind_scale;      // 风力等级       → "4"  
    float wind_speed;        // 风速           → "23.4"
    float rainfall;          // 降雨量         → "19.15"
    float precip;            // 降水概率       → "0.84"
    
  //char wind_direction[8];  // 风向           → "南"（中文需要char数组）
} User_weather_data;
extern User_weather_data User1;
extern uint8_t User_HTTP_Buf[HTTP_BUF_SIZE];

void WIFI_TCP_init(void);                    //重置当前wifi状态

uint8_t WIFI_FIND_TIME(uint16_t out_time);   //设置超时时间，并返回结果

uint8_t WIFI_Parse_Queue(sequeue_t *p);      //临时数据转移到队列做解析，并返回解析结果
uint8_t ESP32C3_WIFI_CONNECT(void);          //连接WiFi，并返回连接结果

uint8_t ESP32C3_WIFI_TCP_CONNECT(void);      //进入TCP透传模式

void User_HTTP_Connect(void);                //链接网页并存储返回数据

void ESP32C3_WIFI_TCP_SSL_OUT(void);         //退出透传模式
void ESP32C3_WIFI_TCP_SSL_OVER(void);         //结束透传模式

void WIFI_TEXT(void);

void time_update(void);
#endif

