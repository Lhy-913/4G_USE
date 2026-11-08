#include <string.h>
#include "fifo.h"

sequeue_t QUART2;//串口2的队列 


//队列初始化及查询剩余空间
int8_t Queue_init(sequeue_t *q)
{
    q->front = q->rear = NSize-1;
    memset(q->data,0,NSize);//将缓存数组清理
    return 0;
}
uint8_t Queue_size(sequeue_t *q)
{
    if(q->front>=q->rear)
    {
        return q->front - q->rear;
    }
    else
    {
//      return NSize - q->front + q->rear;
        return (q->front + NSize - q->rear - 1) % NSize;
    }
}

//队列内存状态
int8_t Queue_Empty(sequeue_t *q)
{
    return (q->front == q->rear);
}
int8_t Queue_full(sequeue_t *q)
{
    return ((q->rear+1)%NSize == q->front);
}
//队列元素操作
int8_t Enqueue(sequeue_t *q, uint8_t value)//入队
{
    if(Queue_full(q))return -1;
    q->rear = (q->rear + 1) % NSize;
    q->data[q->rear] = value;
    return 0;
}

int8_t Dequeue(sequeue_t *q, uint8_t *pvalue)//出队
{
    if(Queue_Empty(q))return -1;
    q->front = (q->front + 1) % NSize;
    *pvalue = q->data[q->front];
    return 0;
}

//把一段连续的数据（多个字节）批量放进队列里
int8_t Enqueue_Bytes(sequeue_t *q, uint8_t *pvalue, uint32_t Size)
{
    for (uint16_t i = 0; i < Size; i++, pvalue++)
    {
        if (Enqueue(q, *pvalue) < 0)
        {
            return -1;
        }
    }
    return 0;
}

