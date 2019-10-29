/* programming.c                              Flyaudio 2019
 * ����: BCM Bootloader������ģ��
 *              for S32K144
 * 2019 02 20 ��ֲԤ�н׶ε��������ģ��
 * 
 * 
 */
#include "programming.h"
#include "Servce.h"
#include "def.h"
#include "Flash.h"  
#include "Diagnose_Communication.h"
#include "Systick.h"
#include "stdio.h"
#include "MemoryConfig.h"
#include "Bootloader.h"
#include "string.h"
#define  eraRoutineDID 0x1111
#define  BOOTRoutineDID 0x2222
#define  USERRoutineDID 0x3333
#define  CheckisBootDID 0x4444
#define  SetSystemFlagDID 0x5555
#define  DownloadmaxNumberOfBlockLength 16

unsigned char RTSession=fuc_defaultSession;
unsigned int  SessionResetTime=0;
	
unsigned char RTDTC=fuc_eDTCSettingOff;
unsigned int RoutineDelaytime=0;
unsigned int T0WhatStatusAdder;

void Programming_SetUpdataSystemFlag(void);
void Programming_ClearUpdataSystemFlag(void);
u32 Bootloader_GetUpdataSystemFlag(void);
typedef struct 
{
	u8  DownloadBuff[4096];
	u32 DownloadAdderStart;//������ʼ��ַ
	u32 DownloadAdderSize;//���ص�ַ�Ĵ�С
	unsigned long DownloadorUploadAdder;//��ǰ���ػ��ϴ��ĵ�ַ
	u8 	BlockNo;//��У��
	u8  Type;//���ػ����ϴ�
	
	u16  DownloadbufVail;
}__DownloadService;
static __DownloadService  gDownloadService;
void GoT0WhatStatus(uint32_t Address);

/***************************
***	���ƣ�	Programming_Init
***	������ 
*** ��ע��  ��ʱ���� 
****************************/
void Programming_Init(void)
{
	
}


/***************************
			BootorUser
		�ж��Ƿ���ת��User
****************************/
void 	BootorUser()
{
//	if(Bkp_ReadBootMode()!=0x1111)
//	{
//			printf("\r\nBootorUser1 %04x",*(u32*)(UserProgramAddressStart));
//			if(*(u32*)(UserProgramAddressStart)==0x12345678)
//			{
//				
//				T0WhatStatusAdder=UserProgramAddressEntry;
//				RoutineDelaytime=1;
//			}
//	}
}


/***************************
			ProgramPro
		
****************************/
void ProgramPro(void)//
{
		/* ����������ݺ�5S��û�н��ܵ��������ݣ�����SYSTEM*/
			if(ReadUserTimer(&SessionResetTime)>5000&&SessionResetTime!=0)
			{
				Programming_ClearUpdataSystemFlag();
		    CheckPermission();//���г���ʱ����SYSTEM
				GoT0WhatStatus(BootloaderProgramAddressEntry);//û�г��������bootloader
			}
			/*��ʱ500ms��ִ�е�ַ��ת*/
			if(ReadUserTimer(&RoutineDelaytime)>500&&RoutineDelaytime!=0)
			{
			//	chipDeInit();
				RoutineDelaytime=0;
		   GoT0WhatStatus(T0WhatStatusAdder);
			}
}
void ClaerSessionTime(void)
{
	if(SessionResetTime!=0)
	ResetUserTimer(&SessionResetTime);
}
/***************************
			erasureRoutine
			USER��ַ��������
****************************/
void  erasureRoutine()//�û���ַȫ����
{
//	INT32U re0;
////	re0=sectorPrepare(APP_START_SECTOR, APP_END_SECTOR);
//	if(re0)
//		printf("\r\nerasureRoutine:sectorPrepare");
////	re0=sectorErase(APP_START_SECTOR, APP_END_SECTOR);
//	if(re0)
//		printf("\r\nerasureRoutine:sectorPrepare");

}
/***************************
			Prog_SetDiagSession
			�Ự����
****************************/
void Prog_SetDiagSession(unsigned char Session)
{
	RTSession=Session;
	ResetUserTimer(&SessionResetTime);
}
/***************************
			Prog_SetDTC
				DTC����
****************************/
void Prog_SetDTC(unsigned char DTC)
{
	RTDTC=DTC;
}
/***************************
			Prog_CommunicationControl
			ͨ��ʹ��ֹͣ����
****************************/
void Prog_CommunicationControl(unsigned char Communication)
{
			

}
/***************************
			BOOTRoutine
			BOOT��ת����
****************************/
void 	BOOTRoutine()//��ת��BOOT��ַ
{
  	printf("\r\nBOOTRoutine");
  	ResetUserTimer(&RoutineDelaytime);
	 
  	T0WhatStatusAdder=BootloaderProgramAddressEntry;
}
/***************************
			USERRoutine
			USER��ת����
****************************/
void USERRoutine()//��ת��USER��ַ
{
	printf("\r\nUSERRoutine");
	ResetUserTimer(&RoutineDelaytime);
	
	T0WhatStatusAdder=SystemProgramAddressEntry;
}
/***************************
			Prog_RoutineControl
			���̿���
****************************/
unsigned char Prog_RoutineControl(unsigned char Routine,unsigned short int DID)
{//���̿���
	switch(Routine)
	{ 
		case fuc_startRoutine:
			switch(DID)
			{
				case eraRoutineDID://��������
				{
					bool ret;
					Programming_ClearHaveSystemFlag();
					ret= EraseSystemFlashSpace();
					printf("\r\nbl--Erase System Flash ");
					if(ret==true)
					{
						
						printf("ok");
					}
					else
					{
						printf("failure");
					}
						
				}
					
					break;
				case BOOTRoutineDID://��תBOOT����
					printf("\r\nbl--boot to bootloader ");
					 Programming_SetUpdataSystemFlag();
					BOOTRoutine();
					break;
				case USERRoutineDID://��תUSER����
					printf("\r\nbl--boot to system ");
					Programming_ClearUpdataSystemFlag();
					USERRoutine();
					break;
				case CheckisBootDID://
					printf("\r\nbl--checkin bootloader ");
					break;
				case SetSystemFlagDID://
						{
						printf("\r\nbl--set flag ");
            Programming_SetHaveSystemFlag();
							//	printf("Bootloader_GetHaveSystemFlag()=%x,",Bootloader_GetHaveSystemFlag());
						}
					break;
			
				
				default :
					printf("\r\nbl--DID=%x",DID);
					return NRC_0x31_ROOR;
			}
		break;
		case fuc_stopRoutine:
			break;
		case fuc_requestRoutineResults:
			break;
	}
		return 0;
}
/***************************
	Prog_RequestDownloadservice
	   �������ط���
****************************/

unsigned char Prog_RequestDownloadservice(unsigned char *buff,unsigned char len,unsigned char *Para)
{
	unsigned char i;
	if((buff[0]&0xf)+(buff[0]>>4)!=len-1)
	return NRC_0x31_ROOR;
	gDownloadService.DownloadAdderStart=0;
	gDownloadService.DownloadAdderSize=0;
	for(i=0;i<(buff[0]>>4);i++)
	{
		gDownloadService.DownloadAdderStart=gDownloadService.DownloadAdderStart<<8;
		gDownloadService.DownloadAdderStart+=buff[i+1];
	}
	for(;i<(buff[0]&0xf)+(buff[0]>>4);i++)
	{
		gDownloadService.DownloadAdderSize=gDownloadService.DownloadAdderSize<<8;
		gDownloadService.DownloadAdderSize+=buff[i+1];
	}
	printf("\r\nDownloadAdderStart=%x DownloadAdderSize=%d",gDownloadService.DownloadAdderStart,gDownloadService.DownloadAdderSize);
	//if(gDownloadService.DownloadAdderStart>(UserProgramAddressSize))
//	return NRC_0x31_ROOR;
	//if(gDownloadService.DownloadAdderSize>UserProgramAddressSize)
	//return NRC_0x31_ROOR;
	//FLASH_UnlockBank1();
	//printf("\r\nUnlock");
	Para[0]=0x10;
	Para[1]=DownloadmaxNumberOfBlockLength;
	 gDownloadService.Type=RequestDownloadservice;
	gDownloadService.DownloadorUploadAdder=gDownloadService.DownloadAdderStart;
	gDownloadService.BlockNo=0;
	SetCFrameBTmin(5);//����CF������
	memset(gDownloadService.DownloadBuff,0,256);
	gDownloadService.DownloadbufVail=0;
		memset(gDownloadService.DownloadBuff,0,4096);

	return 0;
}

/***************************
Prog_RequestUploadserviceservice
	  �����ϴ�����
****************************/
unsigned char Prog_RequestUploadserviceservice(unsigned char *buff,unsigned char len,unsigned char *Para)
{
//	unsigned char i;
//	if(len<3)
//			{
//				return NRC_0x13_IMLOIF;
//			}
//	if((buff[0]&0xf)+(buff[0]>>4)!=len-1)
//	return NRC_0x31_ROOR;
//	gDownloadService.DownloadAdderStart=0;
//	gDownloadService.DownloadAdderSize=0;
//	for(i=0;i<(buff[0]>>4);i++)
//	{
//		gDownloadService.DownloadAdderStart=gDownloadService.DownloadAdderStart<<8;
//		gDownloadService.DownloadAdderStart+=buff[i+1];
//	}
//	for(;i<(buff[0]&0xf)+(buff[0]>>4);i++)
//	{
//		gDownloadService.DownloadAdderSize=gDownloadService.DownloadAdderSize<<8;
//		gDownloadService.DownloadAdderSize+=buff[i+1];
//	}
//	printf("\r\nUploadAdderStart=%x UploadAdderSize=%d",gDownloadService.DownloadAdderStart,gDownloadService.DownloadAdderSize);

//	Para[0]=0x10;
//	Para[1]=DownloadmaxNumberOfBlockLength;
//	 gDownloadService.Type=RequestUploadservice;
//	gDownloadService.DownloadorUploadAdder=gDownloadService.DownloadAdderStart;
//	gDownloadService.BlockNo=0;
//	SetCFrameBTmin(5);//����CF������
	return 0;
}
/***************************
Prog_RequestTransferExitservice
	 �������ݴ���ֹͣ����
****************************/
unsigned char Prog_RequestTransferExitservice(unsigned char *buff,unsigned char len)
{
//		u32 ret;
//	if( gDownloadService.Type==RequestDownloadservice)
//	{
//		//	if(gDownloadService.DownloadbufVail!=0&&gDownloadService.DownloadorUploadAdder>ConfigFileParameterAddressEntry)
//	{
//	//printf("\r\neDownloadAdderStart=%x %d",(u32)gDownloadService.DownloadorUploadAdder,gDownloadService.DownloadbufVail);
//	
////		ret=sectorPrepare(APP_START_SECTOR, APP_END_SECTOR);
////		if(ret)
////		{
////			printf("\r\nsectorPrepare|ramCopy0=%x",ret);
////			return NRC_0x72_GPF;//��̴���
////		}
//		//ret=WriteSystemDataToFlash((unsigned long)gDownloadService.DownloadorUploadAdder, gDownloadService.DownloadBuff, 4096);
//		//memset(gDownloadService.DownloadBuff,0,4096);
//		//if(ret==false)
//		{
//		//	printf("\r\nWriteSystemDataToFlash error");
//		//	return NRC_0x72_GPF;//��̴���
//		}
//	}
//	}
	SetCFrameBTmin(100);
	gDownloadService.Type=0;
	return 0;
}
/*********************************************************************************************************
** Function name:       Prog_TransferDataservice
** Descriptions:        ���ݴ������
** input parameters:    buff						��ϲ���
**											len							��ϲ�������
** Returned value:      NULL
*********************************************************************************************************/
unsigned char Prog_TransferDataservice(unsigned char *buf,unsigned char len)
{
	
	if( gDownloadService.Type==RequestDownloadservice)
	{
//		u8 i;
		u32 ret;
//	volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
//if(gDownloadService.DownloadorUploadAdder>(UserProgramAddressSize))
//	return NRC_0x31_ROOR;//��ַ��Χ����
//	if(len-1!=DownloadmaxNumberOfBlockLength)
		//if(len-1!=16)
//	return NRC_0x31_ROOR;
	if(gDownloadService.DownloadorUploadAdder>gDownloadService.DownloadAdderStart+gDownloadService.DownloadAdderSize+DownloadmaxNumberOfBlockLength)
	{
		printf("\r\ngDownloadService.DownloadorUploadAdder=%x",gDownloadService.DownloadorUploadAdder);
			printf("\r\ngDownloadService.DownloadAdderStart=%x",gDownloadService.DownloadAdderStart);
			printf("\r\ngDownloadService.DownloadAdderSize=%x",gDownloadService.DownloadAdderSize);
		return NRC_0x24_RSE;//���������ַ��С
	}
	
	if(gDownloadService.BlockNo!=buf[0])
		return NRC_0x73_WBSC;//���������
	/********************************************************/
	//memcpy(gDownloadService.DownloadBuff+gDownloadService.DownloadbufVail,buf+1,DownloadmaxNumberOfBlockLength);
	//gDownloadService.DownloadbufVail+=DownloadmaxNumberOfBlockLength;
	//if(gDownloadService.DownloadbufVail==4096)
	{
		printf("\r\nDownAdder=%x ",(u32)gDownloadService.DownloadorUploadAdder);
		
		if((len-1)>DownloadmaxNumberOfBlockLength)
			return NRC_0x31_ROOR;
		else
		{
			u8 buffer[DownloadmaxNumberOfBlockLength];
			memcpy(buffer,buf+1,len-1);
				ret=WriteSystemDataToFlash(gDownloadService.DownloadorUploadAdder, buf+1,DownloadmaxNumberOfBlockLength);
		}
	
		//memset(gDownloadService.DownloadBuff,0,4096);
		//ret=sectorPrepare(APP_START_SECTOR, APP_END_SECTOR);
		if(ret==false)
		{
			printf("\r\nWriteSystemDataToFlash error");
			return NRC_0x72_GPF;//��̴���
		}
	//	ret=ramCopy(gDownloadService.DownloadorUploadAdder, (u32)gDownloadService.DownloadBuff, 4096);
		if(ret)
		{
		//	printf("\r\nsectorPrepare|ramCopy3=%x",ret);
			//return NRC_0x72_GPF;//��̴���
		}

		gDownloadService.DownloadorUploadAdder+=DownloadmaxNumberOfBlockLength;
		gDownloadService.DownloadbufVail=0;
	}
		gDownloadService.BlockNo++;
	return 0;
	}
//	else if( gDownloadService.Type==RequestUploadservice)
//	{
//		printf("\r\n read Address=%x :",(unsigned int)gDownloadService.DownloadorUploadAdder);
////		if(gDownloadService.DownloadorUploadAdder>(UserProgramAddressSize))
////			return NRC_0x31_ROOR;
//	if(gDownloadService.DownloadorUploadAdder>gDownloadService.DownloadAdderStart+gDownloadService.DownloadAdderSize)
//		return NRC_0x24_RSE;
//	/********************************************************/
//		if(gDownloadService.BlockNo!=buf[0])
//		return NRC_0x73_WBSC;
//		{
//			u8 i;
//			for(i=0;i<32;i++)
//			{
//				printf(" %02x",*( uint32_t*)(gDownloadService.DownloadorUploadAdder+i)&0xff);
//			}
//		}
//		gDownloadService.DownloadorUploadAdder+=32;
//		gDownloadService.BlockNo++;
//		return 0;
//		
//	}
	else{
		printf("\r\ngDownloadService.Type=%x",gDownloadService.Type);
			return NRC_0x24_RSE;
	}
}
