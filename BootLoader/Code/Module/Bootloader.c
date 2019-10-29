

#include "Interface.h"


#define SysGuideCfgAddressEntry ParameterSection2FlashEntry

//�������ж��Ƿ�Ϊ��дbootloader������һ���ϵ�����
const unsigned CRP_Key __attribute__((at(BirthmarkAddress))) = BIRTHMARKDATA; //δ����̥��

/**/
void Programming_SetHaveSystemFlag(void)
{
//		CRP_VERSION1=0x12345678;
//	CRP_VERSION2=0x87654321;
	u8 buf[]={0x78,0x56,0x34,0x12,0x21,0x43,0x65,0x87};
	printf("\r\nProgramming_SetHaveSystemFlag");
	WriteSystemDataToFlash(ParameterSection2FlashEntry,buf,8);
	
	
	
}
//���ص�ǰflash���Ƿ���system�ĳ���,true��ʾ��,false��ʾû��

static bool CheckWetherHaveSystemProgram(void)
{
    bool bRes = false;


   printf("\r\nParameterSection2FlashEntry=%x",(*(unsigned int*)ParameterSection2FlashEntry));
	printf("\r\nParameterSection2FlashEntry2=%x",(*(unsigned int*)(ParameterSection2FlashEntry+4)));
		if((0x12345678==(*(unsigned int*)ParameterSection2FlashEntry))&&(0x87654321==(*(unsigned int*)((ParameterSection2FlashEntry)+4))))
		{

			bRes = true;
		}
		
    return bRes;
}
//���ص�ǰ�Ƿ�Ҫ����system����

static bool CheckWetherUpdateProgram(void)
{
    bool bRes = false;
   
	
	
    if ( USER_WANT2UPDATE_PARA == Bootloader_GetUpdataSystemFlag() )
    {
        bRes = true;
    }
    
    return bRes;
}


//���ص�ǰ�Ƿ�Ϊ��һ���ϵ����г���
//ԭ���л���bootloader_encrypt�����в�ִ�в�������,������bootloader�����,��ʱ������Ҫbootloader_encrypt����
static void CheckWetherFirstPowerOn(void)
{
   if ( (*(unsigned int*)ParameterSection2FlashEntry) == 0X77777777 &&(*(unsigned int*)(ParameterSection2FlashEntry+0X4))==0X33333333 )
    {

        if ( EraseSystemFlashSpace() )
        {
					printf("\r\nEarase FlashSpace successed.");
           printf("\r\n Switch to BootloaderProgramAddressEntry.");  

        }

    }
}
void CheckPermission(void)
{
    bool bHaveSystemProgram = false;
	  bool bUpdateSystemProgram = false;
    bHaveSystemProgram = CheckWetherHaveSystemProgram();
	  CheckWetherFirstPowerOn();
    bUpdateSystemProgram = CheckWetherUpdateProgram();
	 printf("\r\nbHaveSystemProgram:%d,bUpdateSystemProgram:%d",bHaveSystemProgram,bUpdateSystemProgram);
    if ( bHaveSystemProgram &&!bUpdateSystemProgram)
    {
        printf("\r\n boot into system.");
        GoT0WhatStatus(SystemProgramAddressEntry);
    } 
}


























