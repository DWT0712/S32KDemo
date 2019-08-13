/************************************************************************************
*  Copyright (c), 2015, HelTec Automation Technology co.,LTD.
*            All rights reserved.
*
* File name: OLED.h
* Project  : HelTec
* Processor: STM32F103C8T6
* Compiler : MDK for ARM - 4.72.1.0
* 
* Author : Aaron.Lee
* Version: 1.01
* Date   : 2014.5.10
* Email  : leehunter8801@gmail.com
* Modification: none
* 
* Description:128*64�����OLED��ʾ�������ļ����������ڻ����Զ���(heltec.taobao.com)��SD1306����SPIͨ�ŷ�ʽ��ʾ��
*
* Others: none;
*
* Function List:
*
* 1. void OLED_WrDat(unsigned char dat) -- ��OLEDд����
* 2. void OLED_WrCmd(unsigned char cmd) -- ��OLEDд����
* 3. void OLED_SetPos(unsigned char x, unsigned char y) -- ������ʼ������
* 4. void OLED_Fill(unsigned char bmp_dat) -- ȫ�����(0x00��������������0xff��������ȫ������)
* 5. void OLED_CLS(void) -- ����
* 6. void OLED_Init(void) -- OLED��ʾ����ʼ��
* 7. void OLED_6x8Str(unsigned char x, y,unsigned char ch[]) -- ��ʾ6x8��ASCII�ַ�
* 8. void OLED_8x16Str(unsigned char x, y,unsigned char ch[]) -- ��ʾ8x16��ASCII�ַ�
* 9. void OLED_16x16CN(unsigned char x, y, N) -- ��ʾ16x16���ĺ���,����Ҫ����ȡģ�����ȡģ
* 10.void OLED_BMP(unsigned char x0, y0,x1, y1,unsigned char BMP[]) -- ȫ����ʾ128*64��BMPͼƬ
*
* History: none;
*
*************************************************************************************/

#ifndef __OLED_H
#define __OLED_H

#include "string.h"
#include "interface.h"

#define Max_Column	128
#define Max_Row			64
#define X_WIDTH 		128
#define Y_WIDTH 		64
#define	Brightness	0xCF 

//-----------------OLED�˿ڶ���----------------
#define OLED_CS_Clr() PINS_GPIO_ClearPins(PTB, 0x1<<5)
#define OLED_CS_Set() PINS_GPIO_SetPins(PTB, 0x1<<5)    //��OLEDģ��CS1��PTB5��

#define OLED_DC_Clr() PINS_GPIO_ClearPins(PTE, 0x1<<8)
#define OLED_DC_Set() PINS_GPIO_SetPins(PTE, 0x1<<8)    //��OLEDģ��D/C��PTE8��

#define OLED_SDA_Clr() PINS_GPIO_ClearPins(PTD, 0x1<<16)
#define OLED_SDA_Set() PINS_GPIO_SetPins(PTD, 0x1<<16)    //��OLEDģ��SDI ��PTD16��

#define OLED_SCL_Clr() PINS_GPIO_ClearPins(PTD, 0x1<<15)
#define OLED_SCL_Set() PINS_GPIO_SetPins(PTD, 0x1<<15)    //��OLEDģ���CLK��PTD15��



//OLED�����ú���
void OLED_WrDat(unsigned char dat);//д����
void OLED_WrCmd(unsigned char cmd);//д����
void OLED_SetPos(unsigned char x, unsigned char y);//������ʼ������
void OLED_Fill(unsigned char bmp_dat);//ȫ�����
void OLED_CLS(void);//����
void OLED_Init(void);//��ʼ��

void LcdDisplay_ASCII(uint8_t yPos,uint8_t *GBCodeptr);
void LcdDisChar(uint8_t xPos,uint8_t yPos,uint8_t zknum,uint8_t *zkzip);
uint8_t LcdDisplay_HZ(uint8_t xPos,uint8_t yPos,uint8_t *GBCodeptr);
void LcdDisplay_Chinese(uint8_t xPos,uint8_t yPos,uint8_t *GBCodeptr);
void LcdDisplay_char(uint8_t xPos,uint8_t yPos,uint8_t *GBCodeptr);
void OLED_BMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1,unsigned char BMP[]);
void OLED_BMP1(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1,unsigned char BMP[]);
void OLE_Display_Char(uint8_t *GBCodeptr0,uint8_t *GBCodeptr1,uint8_t *GBCodeptr2);
#endif
