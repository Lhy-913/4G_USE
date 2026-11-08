#include "http_parse.h"
#include "4G_WIFI_TCP.h"
#include "4G_dma.h"

#include <stdio.h>
#include <stdlib.h> 
#include "String.h"

void time_update(void)
{
    char *start = NULL;//指向需要读取的数据名指针
    char buf[8] = {0};

    start = strstr((char *)User_HTTP_Buf, "\"year\":");
    sscanf(start, "\"year\":%4[^,]", buf);
    User1.year = (uint16_t)atoi(buf);

    start = strstr((char *)User_HTTP_Buf, "\"month\":");
    sscanf(start, "\"month\":%2[^,]", buf);
    User1.month = (uint8_t)atoi(buf);

    start = strstr((char *)User_HTTP_Buf, "\"day\":");
    sscanf(start, "\"day\":%2[^,]", buf);
    User1.day = (uint8_t)atoi(buf);

    start = strstr((char *)User_HTTP_Buf, "\"hour\":");
    sscanf(start, "\"hour\":%2[^,]", buf);
    User1.hour = (uint8_t)atoi(buf);

    start = strstr((char *)User_HTTP_Buf, "\"minute\":");
    sscanf(start, "\"minute\":%2[^,]", buf);
    User1.minute = (uint8_t)atoi(buf);
    
    start = strstr((char *)User_HTTP_Buf, "\"seconds\":");
    sscanf(start, "\"seconds\":%2[^,]", buf);
    User1.seconds = (uint8_t)atoi(buf);
}

void weather_update(void)
{
    char *start = NULL;
    char buf[64] = {0};

    start = strstr((char *)User_HTTP_Buf, "\"code_day\":");
    sscanf(start, "\"code_day\":\"%hhu",&User1.code_day);

    start = strstr((char *)User_HTTP_Buf, "\"code_night\":");
    sscanf(start, "\"code_night\":\"%hhu",&User1.code_night);

    start = strstr((char *)User_HTTP_Buf, "\"high\":");
    sscanf(start, "\"high\":\"%2[^\"]", buf);
    User1.high = (uint8_t)atoi(buf);

    start = strstr((char *)User_HTTP_Buf, "\"low\":");
    sscanf(start, "\"low\":\"%2[^\"]", buf);
    User1.low = (uint8_t)atoi(buf);

    start = strstr((char *)User_HTTP_Buf, "\"humidity\":");
    sscanf(start, "\"humidity\":\"%3[^\"]", buf);
    User1.humidity = (uint8_t)atoi(buf);

    start = strstr((char *)User_HTTP_Buf, "\"wind_scale\":");
    sscanf(start, "\"wind_scale\":\"%2[^\"]", buf);
    User1.wind_scale = (uint8_t)atoi(buf);

    start = strstr((char *)User_HTTP_Buf, "\"wind_speed\":");
    sscanf(start, "\"wind_speed\":\"%5[^\"]", buf);
    User1.wind_speed = atof(buf);

    start = strstr((char *)User_HTTP_Buf, "\"rainfall\":");
    sscanf(start, "\"rainfall\":\"%6[^\"]", buf);
    User1.rainfall = atof(buf);

    start = strstr((char *)User_HTTP_Buf, "\"precip\":");
    sscanf(start, "\"precip\":\"%4[^\"]", buf);
    User1.precip = atof(buf);
}
