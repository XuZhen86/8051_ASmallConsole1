#ifndef __LCD12864_H_
#define __LCD12864_H_

#include<Sources/Main/STC15W4K48S4.h>
#include<Sources/PWM/PWM.h>
#include<Sources/Universal/Universal.h>
#include<Sources/SPI/SPI.h>
#include<Sources/I23LC512/I23LC512.h>
// #include<Sources/Timer/Timer.h>

#include<stdlib.h>

void lcd12864SpiSend(bit b,unsigned char c);
void lcd12864SpiSend2Bytes(bit b,unsigned char c1,unsigned char c2);
void lcd12864HwReset();
void lcd12864PwmInitialize();
void lcd12864BrightnessSet(unsigned int brightness);
unsigned int lcd12864BrightnessGet();
void lcd12864FlushAtNextUpdate();

void lcd12864SpiInitialize();
void lcd12864GdramFlush(bit forceFlush);
void lcd12864CharSet(unsigned char row,unsigned char col,unsigned char c,bit flush);
void lcd12864StringSet(unsigned char row,unsigned char col,unsigned char *str,bit flush);

#endif
