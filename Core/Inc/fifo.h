#ifndef __FIFO_H_
#define __FIFO_H_
#include "stm32f1xx_hal.h"

#define NSize 1024//设置队列内存大小（内存大小需覆盖外界返回数据的字节数，否则会溢出，数据接收异常）

typedef struct
{
    uint8_t data[NSize]; // 用数组作为队列的储存空间
    int32_t front;       // 指示队头位置
    int32_t rear;        // 指示队尾位置
} sequeue_t;

extern sequeue_t QUART2; 

//队列初始化及查询剩余空间
int8_t Queue_init(sequeue_t *q);
uint8_t Queue_size(sequeue_t *q);

//队列内存状态
int8_t Queue_full(sequeue_t *q);
int8_t Queue_Empty(sequeue_t *q);

//队列元素操作
int8_t Enqueue(sequeue_t *q, uint8_t value);
int8_t Dequeue(sequeue_t *q, uint8_t *pvalue);

//把一段连续的数据（多个字节）批量放进队列里
int8_t Enqueue_Bytes(sequeue_t *q, uint8_t *pvalue, uint32_t Size);


#endif

