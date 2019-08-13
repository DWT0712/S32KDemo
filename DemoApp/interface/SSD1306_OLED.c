/************************************************************************************
*  Copyright (c), 2015, HelTec Automation Technology co.,LTD.
*            All rights reserved.
*
* File name: OLED.c
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

#include "SSD1306_OLED.h"
#include "SSD1306_flash.h"

#include "interface.h"
#include "ConfigFile.h"
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "codetab.h"

extern void DelayUs(u8 time);

void OLED_WrDat(unsigned char dat) //д����
{
	unsigned char i;
	OLED_DC_Set();
	DelayUs(10);
	for (i = 0; i < 8; i++)
	{
		if ((dat << i) & 0x80)
		{
			OLED_SDA_Set();
		}
		else
			OLED_SDA_Clr();
		OLED_SCL_Clr();
		DelayUs(10);
		OLED_SCL_Set();
	}
}

void OLED_WrCmd(unsigned char cmd) //д����
{
	unsigned char i;
	OLED_DC_Clr();
	DelayUs(10);
	for (i = 0; i < 8; i++) //����һ����λ����
	{
		if ((cmd << i) & 0x80)
		{
			OLED_SDA_Set();
		}
		else
		{
			OLED_SDA_Clr();
		}
		OLED_SCL_Clr();
		DelayUs(10);
		OLED_SCL_Set();
	}
}

void OLED_SetPos(unsigned char x, unsigned char y) //������ʼ������
{
	OLED_WrCmd(0xb0 + y);
	OLED_WrCmd(((x & 0xf0) >> 4) | 0x10);
	OLED_WrCmd((x & 0x0f) | 0x01);
}

void OLED_Fill(unsigned char bmp_dat) //ȫ�����
{
	unsigned char y, x;

	OLED_CS_Clr();
	for (y = 0; y < 8; y++)
	{
		OLED_WrCmd(0xb0 + y);
		OLED_WrCmd(0x01);
		OLED_WrCmd(0x10);
		for (x = 0; x < X_WIDTH; x++)
		{
			OLED_WrDat(bmp_dat);
		}
	}
	OLED_CS_Set();
}

// iLine < 8
static void OLED_Line_Fill(unsigned char iLine, unsigned char bmp_dat) //ȫ�����
{
	if (iLine > 7)
		return;

	unsigned char x;

	OLED_CS_Clr();

	OLED_WrCmd(0xb0 + iLine);
	OLED_WrCmd(0x01);
	OLED_WrCmd(0x10);
	for (x = 0; x < X_WIDTH; x++)
	{
		OLED_WrDat(bmp_dat);
	}
	OLED_CS_Set();
}

void OLED_CLS(void) //����
{
	OLED_Fill(0x00);
}

void OLED_Init(void)
{
	IO_Write(OLED_CS1, TRUE);
	IO_Write(OLED_DC, TRUE);
	IO_Write(OLED_SDI, TRUE);
	IO_Write(OLED_CLK, TRUE);

	IO_Write(IO_SPI_MCU_SEND_FLAG, FALSE);

	Delayms(200);

	OLED_CS_Clr();
	OLED_WrCmd(0xae);
	OLED_WrCmd(0xae);		//--turn off oled panel
	OLED_WrCmd(0x00);		//---set low column address
	OLED_WrCmd(0x10);		//---set high column address
	OLED_WrCmd(0x40);		//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	OLED_WrCmd(0x81);		//--set contrast control register
	OLED_WrCmd(Brightness); // Set SEG Output Current Brightness
	OLED_WrCmd(0xa1);		//--Set SEG/Column Mapping
	OLED_WrCmd(0xc8);		//Set COM/Row Scan Direction
	OLED_WrCmd(0xa6);		//--set normal display
	OLED_WrCmd(0xa8);		//--set multiplex ratio(1 to 64)
	OLED_WrCmd(0x3F);		//--1/64 duty
	OLED_WrCmd(0xd3);		//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	OLED_WrCmd(0x00);		//-not offset
	OLED_WrCmd(0xd5);		//--set display clock divide ratio/oscillator frequency
	OLED_WrCmd(0x80);		//--set divide ratio, Set Clock as 100 Frames/Sec
	OLED_WrCmd(0xd9);		//--set pre-charge period
	OLED_WrCmd(0xf1);		//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	OLED_WrCmd(0xda);		//--set com pins hardware configuration
	OLED_WrCmd(0x12);
	OLED_WrCmd(0xdb); //--set vcomh
	OLED_WrCmd(0x40); //Set VCOM Deselect Level
	OLED_WrCmd(0x20); //-Set Page Addressing Mode (0x00/0x01/0x02)
	OLED_WrCmd(0x00); //
					  //	OLED_WrCmd(0x21);//-Setup column start and end address
					  //	OLED_WrCmd(0x00);//
					  //	OLED_WrCmd(0x7F);//
					  //	OLED_WrCmd(0x22);//-Setup page start and end address
					  //	OLED_WrCmd(0xB7);//
					  //	OLED_WrCmd(0x03);//
	OLED_WrCmd(0x8d); //--set Charge Pump enable/disable
	OLED_WrCmd(0x14); //--set(0x10) disable
	OLED_WrCmd(0xa4); // Disable Entire Display On (0xa4/0xa5)
	OLED_WrCmd(0xa6); // Disable Inverse Display On (0xa6/a7)
	OLED_WrCmd(0xaf); //--turn on oled panel
	OLED_Fill(0x00);
	OLED_SetPos(0, 0);
	OLED_CS_Set();
}

void LcdDisChar(u8 xPos, u8 yPos, u8 zknum, u8 *zkzip)
{
	u8 i;
	OLED_SetPos(xPos, yPos);
	for (i = 0; i < zknum; i++)
	{
		OLED_WrDat(zkzip[i]);
	}
	OLED_SetPos(xPos, yPos + 1);
	for (i = zknum; i < zknum * 2; i++)
	{
		OLED_WrDat(zkzip[i]);
	}
}

u8 LcdDisplay_HZ(u8 xPos, u8 yPos, u8 *GBCodeptr)
{
	u8 msb, lsb, zknum;
	u8 zkzip[32]; //��ȡ�ֿ����ݵĻ�����
	u32 offset;   //�ֿ��ַ����
	u32 i;

	OLED_CS_Clr();
	if (xPos >= Max_Column || yPos >= Max_Row)
		return 0;				//����Χ�˳�
	msb = *GBCodeptr;			//���ֻ�ASCII�Ļ�����ĵ�8λ��
	lsb = *(GBCodeptr + 1);		//���ֻ�ASCII�Ļ�����ĸ�8λ��
	if (msb > 128 && lsb > 128) //����Ϊ����
	{
		if (xPos + 16 > Max_Column || yPos + 16 > Max_Row)
			return 0;												//����Χ�˳�
		offset = 0x00000 + ((msb - 0xA1) * 94 + (lsb - 0xa1)) * 32; //�����㷨��ϸ�鿴�ֿ�ԭ��
		zknum = 16;													//����Ϊ16*16���ֿ�
	}
	else //����ΪASCII��
	{
		if (xPos + 8 > Max_Column || yPos + 16 > Max_Row)
			return 0;					   //����Χ�˳�
		offset = 0x8100 + (msb - 32) * 16; //�鿴�ṩ��2012_KZ.txt�ĵ��еġ�!�����׵�ַ
		zknum = 8;						   // ASCII��λ8*16���ֿ�
	}
	Read_FLASH(zkzip, offset, zknum * 2); //��FLASH�ж�ȡ�ֿ����ݡ�

	printf("\r\n zk data = ");
	for (i = 0; i < sizeof(zkzip); i++)
	{
		printf(" %d", zkzip[i]);
	}
	LcdDisChar(xPos, yPos, zknum, zkzip);
	OLED_CS_Set();

	return 1;
}

u8 LcdDisplay_ASCII_MAP(u8 *GBCodeptr)
{
	u8 data;
	u32 i;
	data = *GBCodeptr;
	for (i = 0; i < 8; i++)
	{
		OLED_WrDat(ASCII_MAP[data][i]);
	}
	return 1;
}
/*
void LcdDisplay_Chinese(u8 xPos,u8 yPos,u8 *GBCodeptr)
{
	u8 i,len;
	len =  strlen((const char*)GBCodeptr);
	for(i=0;i<len;i++)	
	{
	   	LcdDisplay_HZ(xPos+i*8,yPos,GBCodeptr+i);
		i++;
	}
}

void LcdDisplay_char(u8 xPos,u8 yPos,u8 *GBCodeptr)
{
  u8 i, len;
	len =  strlen((const char*)GBCodeptr);
  for(i=0;i<len;i++)	
	{
		LcdDisplay_HZ(xPos+i*8,yPos,GBCodeptr+i);
	}
}
*/
void LcdDisplay_ASCII(u8 yPos, u8 *GBCodeptr)
{
	u8 i, len;
	len = strlen((const char *)GBCodeptr);
	OLED_CS_Clr();
	OLED_SetPos(0, yPos);

	if (len > 16)
		len = 16;

	for (i = 0; i < len; i++)
	{
		LcdDisplay_ASCII_MAP(GBCodeptr + i);
	}

	if (len < 16)
	{
		for (i = len; i < 16; i++)
		{
			LcdDisplay_ASCII_MAP(" ");
		}
	}
	OLED_CS_Set();
}
//------------------------------------------------------
//void OLED_BMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1,unsigned char BMP[])
//��ָ��������ʾBMPͼƬ
//x0(0~127),yo(0~7) -- ͼƬ��ʼ���꣬x1(1~128),y1(1~8)ͼƬ����������
//��BMPͼƬ����ȡģ���������ģ,�ٽ���ģ�ŵ������̵�codetab.h��
//------------------------------------------------------

void OLED_BMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[])
{
	unsigned int j = 0;
	unsigned char x, y;

	OLED_CS_Clr();
	if (y1 % 8 == 0)
		y = y1 / 8;
	else
		y = y1 / 8 + 1;
	for (y = y0; y < y1; y++)
	{
		OLED_SetPos(x0, y);

		for (x = x0; x < x1; x++)
		{
			OLED_WrDat(BMP[j++]);
		}
	}
	OLED_CS_Set();
}

void OLE_Display_Char(u8 *GBCodeptr0, u8 *GBCodeptr1, u8 *GBCodeptr2)
{
	//OLED_CLS();
	OLED_Line_Fill(7, 0x00);
	Delayms(1);
	LcdDisplay_ASCII(0, GBCodeptr0);
	LcdDisplay_ASCII(1, GBCodeptr1);
	LcdDisplay_ASCII(2, GBCodeptr2);
	LcdDisplay_ASCII(3, "Fuck three Line");
	LcdDisplay_ASCII(4, "what 4");
	LcdDisplay_ASCII(5, "shazi 5");
	LcdDisplay_ASCII(6, "shit 6");
	LcdDisplay_ASCII(7, "test 7");
}
