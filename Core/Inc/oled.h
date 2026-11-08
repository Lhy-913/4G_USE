#ifndef __OLED_H
#define __OLED_H

#include "main.h"

// 初始化 OLED 屏幕（必须在主函数开始时调用一次）
void OLED_Init(void);

// 清空整个屏幕（把显示内容清除）
void OLED_Clear(void);

// 在指定位置显示一个字符
// Line：行号（1~4）    Column：列号（1~16）    Char：要显示的字符
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);

// 在指定位置显示字符串（自动按列顺序显示）
// Line：行号（1~4）    Column：列号（1~16）    String：C语言字符串
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);

// 显示无符号整数（十进制）
// Number：要显示的数   Length：显示的总位数，不足前面补 0
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

// 显示有符号整数（十进制）
// Number：要显示的数   Length：显示的总位数（包括符号），不足补 0
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);

// 显示十六进制数字（大写）
// 例如：0x7F 显示为“7F”
// Number：要显示的数   Length：显示的总位数（不足补0）
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

// 显示二进制数字（补足指定位数）
// 例如：Number=5，Length=4  → 显示“0101”
// Line：行号，Column：列号，Number：要显示的值，Length：显示位数
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

/*******用户自添加*******/

// 在OLED指定位置显示浮点数
// Line：行号    Column：列号   Number：要显示的浮点数
// Precision_int：整数位数（例如2，4 → 显示 xx.xxxx）
// Precision：小数位数
void OLED_ShowFloat(uint8_t Line, uint8_t Column, float Number, uint8_t Precision_int, uint8_t Precision);
#endif
