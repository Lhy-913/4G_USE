#include "stm32f1xx_hal.h"
#include "4G_WIFI_TCP.h"
#include "Tx_String.h"
#include "4G_dma.h"
#include "oled.h"    //显示屏
#include "user_rtc.h"//RTC时钟
#include "http_parse.h"//解析HTTP数据

#include <stdio.h>
#include <stdlib.h> 
#include "String.h"

/*
1.运行结束后再次启动链接时获取异常，目前要推FreeRTOS进度，先不处理
2.天气数据的解析有问题，后续处理

1.问题着手点先放着：time和weather标志位应该是冲突的，逻辑需要改

    2026.5.15
*/

#define LOOK_AT 1    //查看单片机发送给ESP32-C命令的调试接口

#define CWMODE "AT+CWMODE=1\r\n"    //2：混合模式；1：无线网卡（蹭WIFI的）；0：局域网模式（让别人连自己）
#define CWJAP "AT+CWJAP\r\n"        //若先前没有链接过WIFI，就改为AT+CWJAP="WIFI名","密码"
#define CIPMODE "AT+CIPMODE=1\r\n"  //1：多连接；0：单连接 
#define CIPMUX "AT+CIPMUX=0\r\n"    //1：WIFI透传；0：普通传输

//需要访问的网址

//拉取现实时间的网址
#define HTTP_TIME_Connect "AT+CIPSTART=\"SSL\",\"timeapi.io\",443\r\n"
#define HTTP_TIME "GET /api/time/current/zone?timeZone=Asia/Shanghai HTTP/1.1\r\nHost: timeapi.io\r\nConnection: close\r\n\r\n"

//拉取心知天气数据的网址
#define HTTP_Weather_Connect "AT+CIPSTART=\"SSL\",\"api.seniverse.com\",443\r\n"
#define HTTP_Weather "GET /v3/weather/daily.json?key=SRvk9Z1YniJKVTf8d&location=dongguan&language=zh-Hans&unit=c&start=0&days=1 HTTP/1.1\r\nHost: api.seniverse.com\r\nConnection: close\r\n\r\n"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern sequeue_t QUART2; 
extern uint8_t RX2_Buf[DMA_BUF_SIZE];                //引用串口2的DMA接收数组，该数组仅在接收HTTP数据时不使用
extern volatile uint16_t RX2_Offset;                 //引用串口2的偏移量

uint8_t User_HTTP_Buf[HTTP_BUF_SIZE] = {0};          //接收网页数据的数组

volatile uint8_t WIFI_CMD_Status = 0;                //命令执行结果
uint8_t User_Rx_HTTP_FLAG = 0;                       //网址数据接收标志位
uint8_t User_Rx_TIME_FLAG = 0;                       //时间数据接收标志位
uint8_t User_Rx_Weather_FLAG = 0;                    //天气数据接收标志位



uint8_t TX_fifo_buf[DMA_BUF_SIZE] = {0};             //接收并存储队列数据的数组
uint8_t Parse_Substr[32] = {0};                      //用于判断指令成功的状态
User_weather_data User1 = {0};

uint32_t tick_start = 0; // 记录发送HTTP请求前的时间
uint32_t tick_over  = 0; // 计算耗时（秒）

uint8_t WIFI_FIND_TIME(uint16_t out_time)//设置超时时间，并返回结果
{
    uint8_t lej = 0;

    while(WIFI_CMD_Status == WIFI_NOT)  
    {
        if(out_time % 1000 == 0 && ++lej > 1)//每1s打印一次等待点。当超时时间低于1s时，为保证文本美观性，不执行该打印操作
        {
            Tx_Printf_uart1(".");
        }
        if((out_time--) == 0)
        {
            return WIFI_NOT;
        }
        HAL_Delay(1);
    }
    return WIFI_CMD_Status;
}

uint8_t WIFI_Parse_Queue(sequeue_t *p)
{
    uint8_t i = 0;
    uint8_t *buf;

    // 2. 判断队列
    if(!User_Rx_HTTP_FLAG)
    {
        if(p != NULL)
        {
            while(!Queue_Empty(p))
            {
                Dequeue(p, TX_fifo_buf + i);

                #if LOOK_AT
                HAL_UART_Transmit(&huart1, TX_fifo_buf + i, 1, 1000);
                #endif

                i++;
            }

            TX_fifo_buf[i] = '\0';  // 字符串结束符
            buf = TX_fifo_buf;
        }
    }
    else
    {       
        #if LOOK_AT
            for(uint16_t i=0;i<HTTP_BUF_SIZE;i++)//打印HTTP数组内数据到串口1
        {
            HAL_UART_Transmit(&huart1,User_HTTP_Buf + i, 1, 1000);
        }
            Tx_Printf_uart1("\n以上是HTTP数组内数据\n\n");
        #endif
        buf = User_HTTP_Buf;
    }


    // 3. 统一解析
    if(strstr((const char *)buf, (const char *)Parse_Substr))
    {
        return WIFI_OK;
    }
    else if(strstr((const char *)buf, "ERROR\r\n"))
    {
        return WIFI_ERROR;
    }

    return WIFI_NOT;
}


uint8_t ESP32C3_WIFI_CONNECT(void)      //连接WiFi，并返回连接结果
{
    uint8_t TX_AT_buf[64] = {0};//AT固件的命令发送数组
    uint8_t wifiStatus; //获取WFIF当前链接状态
    
    Tx_Printf_uart1("正在链接WIFI...\n\n");
    
    strcpy((char *)Parse_Substr,"OK\r\n");
    strcpy((char *)TX_AT_buf,CWMODE);
        
    WIFI_CMD_Status = WIFI_NOT;
    
    HAL_UART_Transmit(&huart2,TX_AT_buf,strlen((char *)TX_AT_buf),1000);
    if(WIFI_FIND_TIME(5*1000) != WIFI_OK)
    {
        Tx_Printf_uart1("WIFI链接模式确定失败，检查命令是否正确，或链接未结束时勿进行操作\n\n");
        OLED_ShowString(1,15,"NO");
        
        return  WIFI_NOT;
    }
//    Tx_Printf_uart1("已确定WIFI链接模式\n\n");
    strcpy((char *)Parse_Substr,"OK\r\n");
    strcpy((char *)TX_AT_buf,CWJAP);
    
//    Tx_Printf_uart1("WIFI链接中...\n\n");
    
    WIFI_CMD_Status = WIFI_NOT;
    HAL_UART_Transmit(&huart2,TX_AT_buf,strlen((char *)TX_AT_buf),1000);
    
    wifiStatus = WIFI_FIND_TIME(5*1000);
    if(wifiStatus == WIFI_OK)
    {
//        Tx_Printf_uart1("WIFI链接成功\n\n");
        OLED_ShowString(1,15,"OK");
        return WIFI_OK;
    }
    else if(wifiStatus != WIFI_OK)
    {
        Tx_Printf_uart1("WIFI链接无结果\n");
        OLED_ShowString(2,15,"NOT");
        return WIFI_NOT;
    }
//    else//返回ERROR
//    {
//        //使用app一键配网（ESP32-C没有airkiss一键配网，先不写）
//    }
    return WIFI_OK;
}

uint8_t ESP32C3_WIFI_TCP_CONNECT(void)    //进入TCP透传模式
{
    uint32_t TX_AT_buf[512]={0};
    strcpy((char *)TX_AT_buf,CIPMUX);
    strcpy((char *)Parse_Substr,"OK\r\n");
    WIFI_CMD_Status = WIFI_NOT;
    
    Tx_Printf_uart1("TCP模式设定中...\n\n");

    HAL_UART_Transmit(&huart2,(uint8_t *)TX_AT_buf,strlen((char *)TX_AT_buf),1000);
    
    if(WIFI_FIND_TIME(5*1000) != WIFI_OK)
    {
        Tx_Printf_uart1("TCP模式设定失败，检查命令准确性，或重启模块再试\n\n");
        return WIFI_NOT;
    }
        
    strcpy((char *)TX_AT_buf,CIPMODE);
    strcpy((char *)Parse_Substr,"OK\r\n");
    WIFI_CMD_Status = WIFI_NOT;
    HAL_UART_Transmit(&huart2,(uint8_t *)TX_AT_buf,strlen((char *)TX_AT_buf),1000);
    
    if(WIFI_FIND_TIME(5*1000) != WIFI_OK)
    {
        Tx_Printf_uart1("TCP模式设定失败，检查命令准确性，或重启模块再试\n\n");
        OLED_ShowString(2,15,"NOT");
        return WIFI_NOT;
    }
    Tx_Printf_uart1("TCP模式设定成功\n\n");
    OLED_ShowString(2,15,"OK");

    return WIFI_OK;
}

void User_HTTP_Connect(void)                //链接网页并存储返回数据
{
    while(!User_Rx_TIME_FLAG || !User_Rx_Weather_FLAG)
    {
    if(User_Rx_TIME_FLAG)
    {
        ESP32C3_WIFI_TCP_SSL_OUT(); 
    }
    uint32_t TX_AT_buf[512]={0};
    if(!User_Rx_TIME_FLAG)//判断时间数据有无读取，有则读天气数据
    {
     strcpy((char *)TX_AT_buf,HTTP_TIME_Connect);
     Tx_Printf_uart1("获取时间数据...\r\n");
    }
    else
    {
     strcpy((char *)TX_AT_buf,HTTP_Weather_Connect);
     Tx_Printf_uart1("获取天气数据...\r\n");
    }
   
    strcpy((char *)Parse_Substr,"OK\r\n");
    WIFI_CMD_Status = WIFI_NOT;

    HAL_UART_Transmit(&huart2,(uint8_t *)TX_AT_buf,strlen((char *)TX_AT_buf),1000);
    if(WIFI_FIND_TIME(15*1000) != WIFI_OK)
    {
        Tx_Printf_uart1("定位网页操作失败");
        return;
    }
    Tx_Printf_uart1("定位网页操作成功，正在拉取数据...\n\n");

    strcpy((char *)TX_AT_buf,"AT+CIPSEND\r\n");
    strcpy((char *)Parse_Substr,">");
    WIFI_CMD_Status = WIFI_NOT;

    HAL_UART_Transmit(&huart2,(uint8_t *)TX_AT_buf,strlen((char *)TX_AT_buf),1000);

    if(WIFI_FIND_TIME(8*1000) != WIFI_OK)
    {
        Tx_Printf_uart1("传输已中断\n\n");
        OLED_ShowString(3,15,"NOT");
        return;
    }
    Tx_Printf_uart1("\n数据发送中...\n\n");
    
   if(!User_Rx_TIME_FLAG)//判断时间数据有无读取，有则读天气数据
    {
        strcpy((char *)TX_AT_buf,HTTP_TIME);
    }
    else
    {
        memset(User_HTTP_Buf,0,HTTP_BUF_SIZE);
        strcpy((char *)TX_AT_buf,HTTP_Weather);
    }
    strcpy((char *)Parse_Substr,"OK");
    WIFI_CMD_Status = WIFI_NOT;
    
/********************************************************测试代码区间********************************************************/
    //DMA接收对象暂时切换
    User_Rx_HTTP_FLAG = 1;
    HAL_UART_DMAStop(&huart2);
    __HAL_UART_CLEAR_IDLEFLAG(&huart2);
    HAL_UART_Receive_DMA(&huart2, User_HTTP_Buf, HTTP_BUF_SIZE);
/****************************************************************************************************************************/
    
    tick_start = HAL_GetTick();//开始记录获取HTTP数据前的时间消耗
    HAL_UART_Transmit(&huart2,(uint8_t *)TX_AT_buf,strlen((char *)TX_AT_buf),1000);
    
    if(WIFI_FIND_TIME(10*1000) != WIFI_OK)
    {
        Tx_Printf_uart1("\n数据抓取成失败\n\n");
        return;
    }
    
    /********************************************************测试代码区间********************************************************/
    
    if(!User_Rx_TIME_FLAG)
    {
        tick_over = (HAL_GetTick() - tick_start)/1000; // 计算耗时（转换为秒）

        time_update();//更新时间到用户结构体内
        User1.seconds += (tick_over+3);//"+3” 是根据现实时间差值做补偿
        if(User1.seconds > 60)
        {
            User1.minute  += User1.seconds/60;
            User1.seconds  = User1.seconds%60;
        }
        if(User1.minute > 60)
        {
            User1.hour    += User1.minute/60;
            User1.minute   = User1.minute%60;
        }
        if(User1.hour > 24)
        {
            User1.hour = User1.hour%24;;
        }
        user_clock_init();
        User_Rx_TIME_FLAG=1;
        memset(User_HTTP_Buf,0,HTTP_BUF_SIZE);//清空数据，塞入新数据
    }
   
    
    Tx_Printf_uart1("\n数据抓取成功\n\n");
    OLED_ShowString(3,15,"OK");  
    
    //DMA接收对象切换
    User_Rx_HTTP_FLAG = 0;
    RX2_Offset = 0;
    
    HAL_UART_DMAStop(&huart2);
    __HAL_UART_CLEAR_IDLEFLAG(&huart2);
    HAL_UART_Receive_DMA(&huart2, RX2_Buf, DMA_BUF_SIZE);
    QUART2.front = QUART2.rear;//强制清空队列
    
    if(User_Rx_TIME_FLAG && strstr((const char *)User_HTTP_Buf, "WS0GHKN5ZP7T"))
    {
        weather_update();
        User_Rx_Weather_FLAG = 1;
        memset(User_HTTP_Buf,0,HTTP_BUF_SIZE);//清空数据，塞入新数据
    }
    HTTP_again_FLAG = 0;
/****************************************************************************************************************************/
    }
}

void ESP32C3_WIFI_TCP_SSL_OUT(void)         //退出透传模式
{
    uint8_t TX_AT_buf[128]={0};
    strcpy((char *)TX_AT_buf,"+++");
    Tx_Printf_uart1("已发送+++\n\n");
    //发送+++时前后都等待1s（发送前的时间可以少一些，但1s是充足量），避免数据过快导致发送+++异常影响后续AT指令
    HAL_UART_Transmit(&huart2,(uint8_t *)TX_AT_buf,strlen((char *)TX_AT_buf),1000);
    HAL_Delay(1000);
    
     strcpy((char *)TX_AT_buf,"AT+CIPCLOSE\r\n");
    strcpy((char *)Parse_Substr,"OK\r\n");
    WIFI_CMD_Status = WIFI_NOT;    
    HAL_UART_Transmit(&huart2,(uint8_t *)TX_AT_buf,strlen((char *)TX_AT_buf),1000);
    if(WIFI_FIND_TIME(7*1000) != WIFI_OK)
    {
        Tx_Printf_uart1("\n数据抓取成失败\n\n");
        return;
    }
}

void ESP32C3_WIFI_TCP_SSL_OVER(void)        //结束透传模式
{
    User_Rx_Weather_FLAG = 0;
    User_Rx_TIME_FLAG = 1;
    uint8_t TX_AT_buf[128]={0};
    strcpy((char *)TX_AT_buf,"+++");
    Tx_Printf_uart1("\r已发送+++\n\n");
    //发送+++时前后都等待1s（发送前的时间可以少一些，但1s是充足量），避免数据过快导致发送+++异常影响后续AT指令
    HAL_UART_Transmit(&huart2,(uint8_t *)TX_AT_buf,strlen((char *)TX_AT_buf),1000);
    HAL_Delay(1000);
    
    strcpy((char *)TX_AT_buf,"AT+CIPMODE=0\r\n");
    strcpy((char *)Parse_Substr,"OK\r\n");
    WIFI_CMD_Status = WIFI_NOT;    
        
    HAL_UART_Transmit(&huart2,(uint8_t *)TX_AT_buf,strlen((char *)TX_AT_buf),1000);

    if(WIFI_FIND_TIME(10*1000)!=WIFI_OK)
    {
        Tx_Printf_uart1("TCP模式退出失败,需重启\n\n");
        return;
    }
    
    Tx_Printf_uart1("退出TCP中...\n\n");
    strcpy((char *)TX_AT_buf,"AT+CIPCLOSE\r\n");
    strcpy((char *)Parse_Substr,"OK\r\n");
    WIFI_CMD_Status = WIFI_NOT;
    HAL_UART_Transmit(&huart2,(uint8_t *)TX_AT_buf,strlen((char *)TX_AT_buf),1000);
    
    if(WIFI_FIND_TIME(10*1000) != WIFI_OK)
    {
        Tx_Printf_uart1("退出TCP异常,需重启\n\n");
        return;
    }
    Tx_Printf_uart1("成功\n\n");
    OLED_ShowString(4,15,"OK");
}

void WIFI_TCP_init(void)                //初始化wifi、TCP模式
{
    OLED_ShowString(1,1,"WIFI");
    OLED_ShowString(2,1,"TCP");
    OLED_ShowString(3,1,"HTTP");
    OLED_ShowString(4,1,"TCP_Exit");

	if (ESP32C3_WIFI_CONNECT() == WIFI_OK)
	{
        Tx_Printf_uart1("WIFI成功接入\n\n");
        ESP32C3_WIFI_TCP_CONNECT();        
	}
}

void WIFI_TEXT(void)
{        
    User_HTTP_Connect();
    ESP32C3_WIFI_TCP_SSL_OVER();//接收完数据后退出TCP链接
    OLED_Clear();
    user_weather_printf();

}
