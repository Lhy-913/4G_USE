#ifndef __4G_DAM_H
#define __4G_DAM_H
#include "stm32f1xx_hal.h"

#define HTTP_BUF_SIZE 1024  //接收网页数据的数组存储大小
 
#define DMA_BUF_SIZE 1024  //DMA的数组存储大小
#define DMA_MODE 0        //0为使用队列存储（且禁止外部数据传输给ESP32-C，一切按程序运行），1使用数组直接存储数据测试AT命令


#include <stdint.h>

extern volatile uint16_t Get_HTTP_lej;
extern volatile uint8_t HTTP_again_FLAG ;

void User_DMA_INIT(void);

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

void USER_UART_IRQHandler(UART_HandleTypeDef *huart);

#endif
