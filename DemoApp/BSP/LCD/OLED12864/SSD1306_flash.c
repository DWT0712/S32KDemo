/************************************************************************************
*  Copyright (c), 2015, HelTec Automation Technology co.,LTD.
*            All rights reserved.
*
* File name: flash.c
* Project  : HelTec
* Processor: STM32F103C8T6
* Compiler : MDK for ARM - 4.72.1.0
* 
* Author : Aaron.Lee
* Version: 1.00
* Date   : 2014.4.20
* Email  : leehunter8801@gmail.com
* Modification: none
* 
* Description: �������Զ��������ֿ�оƬ�������ļ����������ڻ������Զ���(Heltec.taobao.com)�����ֿ��OLED��ʾ��
*
* Others: none;
*
* Function List:
*
* 1. void FF_Init(void) -- �ֿ�оƬ��ʼ��
* 2. uint8_t Write_FF(uint8_t value) -- ���ֿ�оƬдһ��byte������(����)
* 3. void Read_FLASH(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead) -- ��ȡ�ֿ�оƬ�е�����
*
* History: none;
*
*************************************************************************************/

#include "SSD1306_flash.h"
#include "SSD1306_OLED.h"
#include "interface.h"

	
void DelayUs(uint8_t time)
{
	int i;
	for(;time>0;time--)
	{
		for(i=0; i<6; i++)
		{
			;
			;
			;
		}
  }
}
	
void FF_CS(uint8_t value)
{
	IO_Write(OLED_CS2, value);    //CS2
}
void FF_SO(uint8_t value)
{
	IO_Write(OLED_SDI, value);    //SDI
}
void FF_CLK(uint8_t value)
{
	IO_Write(OLED_CLK, value); //CLK
}

void FF_Init(void)
{	
	FF_CS(1);
	FF_CLK(1);
}

uint8_t Write_FF(uint8_t value)
{
	uint8_t i;
	uint8_t temp=0;
	FF_CLK(1);
	for(i=0;i<8;i++)
	{
		FF_CLK(0);
		DelayUs(2);
		if((value&0x80)==0x80)
			FF_SO(1);
		else
			FF_SO(0); 
		value<<=1;
		DelayUs(2);
		FF_CLK(1);
		DelayUs(2);
		temp<<=1;
		if(IO_Read(GPIO_PTD13)) //GPIOD13 ΪFSO��
			temp++;
	}
	return(temp);
}

void Read_FLASH(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	OLED_CS_Set();//ȡ��OLEDѡ��
	FF_CS(0);//ѡ��FLASHоƬ
	Write_FF(0x03);
	Write_FF((ReadAddr & 0xFF0000) >> 16);
	Write_FF((ReadAddr& 0xFF00) >> 8);
	Write_FF(ReadAddr & 0xFF);
	while(NumByteToRead--)
	{
		*pBuffer = Write_FF(0xA5);
		pBuffer++;
	}
	FF_CS(1);
	OLED_CS_Clr();
}

