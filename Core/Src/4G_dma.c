#include "4G_dma.h"
#include "stm32f1xx_hal.h"
#include "fifo.h"
#include "4G_WIFI_TCP.h"

/*
串口1链接的是TTL转串口到电脑，串口2链接ESP32-C
ESP32-C 引脚：5-Rx  6-Tx
*/


extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart3_rx;

extern TIM_HandleTypeDef htim4;

extern volatile uint8_t WIFI_CMD_Status;                //命令执行结果


//串口1偏移量/存储变量
uint16_t RX1_Offset =0;
uint8_t RX1_Buf[DMA_BUF_SIZE];

uint16_t RX2_Offset =0;
uint8_t RX2_Buf[DMA_BUF_SIZE];

uint16_t RX3_Offset =0;
uint8_t RX3_Buf[DMA_BUF_SIZE];

volatile uint16_t Get_HTTP_lej =0;
volatile uint8_t HTTP_again_FLAG = 0;

void User_DMA_INIT(void)//开启各DMA空闲中断功能，并定义其存储变量地址
{

  __HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart1, RX1_Buf, DMA_BUF_SIZE);

  __HAL_UART_ENABLE_IT(&huart2,UART_IT_IDLE);
	HAL_UART_Receive_DMA(&huart2, RX2_Buf, DMA_BUF_SIZE);
    
    //  __HAL_UART_ENABLE_IT(&huart3,UART_IT_IDLE);//不用就注释，串口三的RX在不接任何引脚时会一直触发中断导致程序卡死，原因未知 2026.5.8
//	HAL_UART_Receive_DMA(&huart3, RX3_Buf, DMA_BUF_SIZE);
}

//半满DMA中断
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uint16_t Length = DMA_BUF_SIZE/2 - RX1_Offset;
        #if DMA_MODE
        HAL_UART_Transmit(&huart2,RX1_Buf + RX1_Offset,Length,HAL_MAX_DELAY);
        #endif
        RX1_Offset += Length;
    }
    
    else if (huart->Instance == USART2)
    {
        uint16_t Length = DMA_BUF_SIZE/2 - RX2_Offset;
        #if DMA_MODE
        HAL_UART_Transmit(&huart1,RX2_Buf + RX2_Offset,Length,HAL_MAX_DELAY);
        #else
        if(Enqueue_Bytes(&QUART2,RX2_Buf + RX2_Offset,Length)<0)
        {
            //已满
        }
        #endif
        RX2_Offset += Length;
    }
    
    else if (huart->Instance == USART3)
    {
        uint16_t Length = DMA_BUF_SIZE/2 - RX3_Offset;
        HAL_UART_Transmit(&huart3,RX3_Buf + RX3_Offset,Length,HAL_MAX_DELAY);
        RX3_Offset += Length;
    }
}

//全满中断
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uint16_t Length = DMA_BUF_SIZE - RX1_Offset;
        #if DMA_MODE
        HAL_UART_Transmit(&huart2,RX1_Buf + RX1_Offset,Length,HAL_MAX_DELAY);
        #endif
        RX1_Offset = 0;
    }
    
    else if (huart->Instance == USART2)
    {
        uint16_t Length = DMA_BUF_SIZE - RX2_Offset;
         #if DMA_MODE
        HAL_UART_Transmit(&huart1,RX2_Buf + RX2_Offset,Length,HAL_MAX_DELAY);
        #else
        if(Enqueue_Bytes(&QUART2,RX2_Buf + RX2_Offset,Length)<0)
        {
            //已满
        }
        #endif
        RX2_Offset = 0;
    }
    
    else if (huart->Instance == USART3)
    {
        uint16_t Length = DMA_BUF_SIZE - RX3_Offset;
        HAL_UART_Transmit(&huart3,RX3_Buf + RX3_Offset,Length,HAL_MAX_DELAY);
        RX3_Offset = 0;
    }
}

/*
闲时中断（这里得专门在stm...it的系统头文件内的IRQ函数内添加，不然无法执行！！！
因为中断触发后，会先执行它自己的IRQ函数，而不会自动触发该函数，因为这不在IRQ函数内）
*/
void USER_UART_IRQHandler(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)  // 判断串口
    {
        if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET)  // 判断是否空闲中断
        {
            __HAL_UART_CLEAR_IDLEFLAG(huart);  // 清空闲标志

            // 计算从上次处理到现在接收到的数据长度
            uint16_t Length = DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx) - RX1_Offset;
            #if DMA_MODE
            HAL_UART_Transmit(&huart2,RX1_Buf + RX1_Offset,Length,HAL_MAX_DELAY);
            #endif
            RX1_Offset += Length;
            }
        }
    
    
        if (huart->Instance == USART2)  // 判断串口
    {
        if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET)  // 判断是否空闲中断
        {
            __HAL_UART_CLEAR_IDLEFLAG(huart);  // 清空闲标志

            // 计算从上次处理到现在接收到的数据长度
            uint16_t Length = DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx) - RX2_Offset;
            #if DMA_MODE
            HAL_UART_Transmit(&huart1,RX2_Buf + RX2_Offset,Length,HAL_MAX_DELAY);
            #else
            if(Enqueue_Bytes(&QUART2,RX2_Buf + RX2_Offset,Length)<0)
            {
                //已满
            }
            HAL_TIM_Base_Start_IT(&htim4);
            #endif
            RX2_Offset += Length;
            }
        }
    
        if (huart->Instance == USART3)  // 判断串口
    {
        if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET)  // 判断是否空闲中断
        {
            __HAL_UART_CLEAR_IDLEFLAG(huart);  // 清空闲标志

            // 计算从上次处理到现在接收到的数据长度
            uint16_t Length = DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart3_rx) - RX3_Offset;
            HAL_UART_Transmit(&huart3,RX3_Buf + RX3_Offset,Length,HAL_MAX_DELAY);
            RX3_Offset += Length;
            }
        }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)//定时器4回调函数
{
    if(htim->Instance == TIM4)
    {
        HAL_TIM_Base_Stop(&htim4);//关闭定时器，等待下一次触发
        WIFI_CMD_Status = WIFI_Parse_Queue(&QUART2);//完成传输后确定WIFI当前链接状态
    }
    
    if(htim->Instance == TIM3)
    {
        if((Get_HTTP_lej++) >= 10)
        {
            HTTP_again_FLAG = 1;
            Get_HTTP_lej = 0;
        }        
    }
}
