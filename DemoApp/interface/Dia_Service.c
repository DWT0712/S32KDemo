#include "Servce.h"
//#include "Dianogse.h"
#include <stdio.h>
#include <string.h>
//#include "programming.h"
#include "Diagnose_Communication.h"

/*********************************************************************************************************
** Function name:       Dianogstic_SendService
** Descriptions:        ��Ϸ��������
** input parameters:    Service:          �������Ϸ���
**											buff��						��Ϸ���Ĳ���
**											len��							��Ϸ���ĳ���
**											ServiceType��			��Ϸ�������ͣ���������Ӧ������Ӧ
** Returned value:      NULL
*********************************************************************************************************/
void Dianogstic_SendService(unsigned char Service,unsigned char *buff,unsigned char len,unsigned char ServiceType)
{
	u8 SendService=0;
	u8 Request[256];
	switch(ServiceType)
	{
		case eRequestMessage:
			SendService=Service;
			break;
		case ePostiveResponseMessage:
			SendService=Service+0x40;
			break;
		case eNegativeResponseCodes:
			SendService=0x7f;
			break;
		default:
			return;
	}
	Request[0] = SendService;
	memcpy(Request+1,buff,len);
	Diagnose_Communication_Request(Request,len+1);


}
/*********************************************************************************************************
** Function name:       Service_NRC
** Descriptions:        ���뷢��
** input parameters:    Service					����SID
**											NRC							������NRC
** Returned value:      NULL
*********************************************************************************************************/
void Service_NRC(unsigned char Service,unsigned char NRC)
{
	unsigned char buf[2];
	buf[0] = Service;
	buf[1] = NRC;
	Dianogstic_SendService(Service,buf,2,eNegativeResponseCodes);
}

/*********************************************************************************************************
** Function name:       DiagnosticSessionConTrolServiceDeal
** Descriptions:        ��ϻỰ���Ʒ�����
** input parameters:    buff						��ϲ���
**											len							��ϲ�������
** Returned value:      NULL
*********************************************************************************************************/
void DiagnosticSessionConTrolServiceDeal(unsigned char *buff,unsigned char len)
{
	unsigned char para[10];
	if(len!=1)
			{
				printf("\r\nNRC: 0x13");
				Service_NRC(DiagnosticSessionConTrolService,NRC_0x13_IMLOIF);
				return ;
			}
				switch(buff[0])
				{
					case fuc_defaultSession:
						//��Ӧ������
					 Prog_SetDiagSession(fuc_defaultSession);
						break;
					case fuc_ProgrammingSession:
						//
					 Prog_SetDiagSession(fuc_ProgrammingSession);
						break;
					case fuc_extendedDiagnosticSession:
						//
					 Prog_SetDiagSession(fuc_extendedDiagnosticSession);
						break;
					case fuc_safetySystemDiagnosticSession:
						//
					 Prog_SetDiagSession(fuc_safetySystemDiagnosticSession);
						break;
					default:
						Service_NRC(DiagnosticSessionConTrolService,NRC_0x12_SFNS);
						printf("\r\nNRC: 0x12");
						return ;
				}
				para[0]=buff[0];
				para[1]=(P2server_max>>8)&0xff;
				para[2]=(P2server_max)&0xff;
				para[3]=(P2Nserver_max>>8)&0xff;
				para[4]=(P2Nserver_max)&0xff;
				Dianogstic_SendService(DiagnosticSessionConTrolService,para,5,ePostiveResponseMessage);
				printf("\r\nPosRespMess: DiagnosticSessionConTrolService");
}
/*********************************************************************************************************
** Function name:       ECUResetServiceDeal
** Descriptions:        ECU���������
** input parameters:    buff						��ϲ���
**											len							��ϲ�������
** Returned value:      NULL
*********************************************************************************************************/
void ECUResetServiceDeal(unsigned char *buff,unsigned char len)
{
	unsigned char para[10];
	if(len>2)
			{
				printf("\r\nNRC: 0x13");
				Service_NRC(ECUResetService,NRC_0x13_IMLOIF);
				return ;
			}
				switch(buff[0])
				{
					case fuc_hardReset:
						//��Ӧ������
						break;
					case fuc_keyOffOnReset:
						//
						break;
					case fuc_softReset:
						//
						break;
					case fuc_enableRapidPowerShutDown:
						//
						break;
					case fuc_disableRapidPowerShutDown:
						break;
					default:
						Service_NRC(ECUResetService,NRC_0x12_SFNS);
						printf("\r\nNRC: 0x12");
						return ;
				}
				para[0]=buff[0];
				if(fuc_enableRapidPowerShutDown!=para[0])
						Dianogstic_SendService(ECUResetService,para,1,ePostiveResponseMessage);
				else{
					para[1]=para_powerDownTime;
						Dianogstic_SendService(ECUResetService,para,2,ePostiveResponseMessage);
	
				}
				printf("\r\nPosRespMess: ECUResetServiceDeal");
}
/*********************************************************************************************************
** Function name:       ControlDTCSettingserviceDeal
** Descriptions:        DTC�������÷�����
** input parameters:    buff						��ϲ���
**											len							��ϲ�������
** Returned value:      NULL
*********************************************************************************************************/
void 	ControlDTCSettingserviceDeal(unsigned char *buff,unsigned char len)
{
	unsigned char para[10];
	if(len<1)
			{
				printf("\r\nNRC: 0x13");
				Service_NRC(ControlDTCSettingservice,NRC_0x13_IMLOIF);
				return ;
			}
			switch(buff[0])
				{
					case fuc_eDTCSettingOn:
					case fuc_eDTCSettingOff:
						Prog_SetDTC(buff[0]);
						break;
					default :
						Service_NRC(ControlDTCSettingservice,NRC_0x12_SFNS);
						printf("\r\nNRC: 0x12");
						return;
				}
				para[0]=buff[0];
		Dianogstic_SendService(ControlDTCSettingservice,para,1,ePostiveResponseMessage);
		printf("\r\nPosRespMess: ControlDTCSettingservice");
}
/*********************************************************************************************************
** Function name:       CommunicationControlserviceDeal
** Descriptions:        ͨ�ſ��Ʒ���
** input parameters:    buff						��ϲ���
**											len							��ϲ�������
** Returned value:      NULL
*********************************************************************************************************/
void 	CommunicationControlserviceDeal(unsigned char *buff,unsigned char len)
{
	unsigned char para[10];
	if(len<2)
			{
				printf("\r\nNRC: 0x13");
				Service_NRC(CommunicationControlservice,NRC_0x13_IMLOIF);
				return ;
			}
			switch(buff[0])
				{
					case fuc_enableRxAndTx:
					case fuc_enableRxAndDisableTx:
					case fuc_DisableRxAndenableTx:
					case fuc_DisableRxAndTx:
					case fuc_enableRxAndDisableTxWithEnhancedAddressInformation:
					case fuc_enableRxAndTxWithEnhancedAddressInformation:
					 Prog_CommunicationControl(buff[0]);
						//Prog_SetDTC(buff[0]);
						break;
					default :
						Service_NRC(CommunicationControlservice,NRC_0x12_SFNS);
						printf("\r\nNRC: 0x12");
						return;
				}
				para[0]=buff[0];
		Dianogstic_SendService(CommunicationControlservice,para,1,ePostiveResponseMessage);
		printf("\r\nPosRespMess: CommunicationControlservice");
}
/*********************************************************************************************************
** Function name:       RoutineControlserviceDeal
** Descriptions:        ���̿��Ʒ�����
** input parameters:    buff						��ϲ���
**											len							��ϲ�������
** Returned value:      NULL
*********************************************************************************************************/
void 	RoutineControlserviceDeal(unsigned char *buff,unsigned char len)
{
	unsigned char para[10];
	unsigned char ret=0;
	if(len<3)
			{
				printf("\r\nNRC: 0x13");
				Service_NRC(RoutineControlservice,NRC_0x13_IMLOIF);
				return ;
			}
			switch(buff[0])
				{
				case fuc_startRoutine:
				case fuc_stopRoutine:
				case fuc_requestRoutineResults:
			ret=Prog_RoutineControl(buff[0],((unsigned short int)buff[1]<<8)+buff[2]);
						break;
					default :
						ret=NRC_0x12_SFNS;
			
						return;
				}
				para[0]=buff[0];
				if(ret)
				{
					Service_NRC(RoutineControlservice,ret);
						printf("\r\nNRC: 0x%02x",ret);
					return;
				}
		Dianogstic_SendService(RoutineControlservice,para,1,ePostiveResponseMessage);
		printf("\r\nPosRespMess: RoutineControlservice");
}
/*********************************************************************************************************
** Function name:       ClearDiagnosticInformationServiceDeal
** Descriptions:        �����Ϸ���
** input parameters:    buff						��ϲ���
**											len							��ϲ�������
** Returned value:      NULL
*********************************************************************************************************/
void 	ClearDiagnosticInformationServiceDeal(unsigned char *buff,unsigned char len)
{
	unsigned char para[10];
	unsigned char ret=0;
	if(len!=4)
			{
				printf("\r\nNRC: 0x13");
				Service_NRC(ClearDiagnosticInformationService,NRC_0x13_IMLOIF);
				return ;
			}
				para[0]=buff[0];
				if(ret)
				{
					Service_NRC(ClearDiagnosticInformationService,ret);
						printf("\r\nNRC: 0x%02x",ret);
					return;
				}
		Dianogstic_SendService(ClearDiagnosticInformationService,para,1,ePostiveResponseMessage);
		printf("\r\nPosRespMess: ClearDiagnosticInformationService");
}
/*********************************************************************************************************
** Function name:       RequestDownloadserviceDeal
** Descriptions:        ���������������
** input parameters:    buff						��ϲ���
**											len							��ϲ�������
** Returned value:      NULL
*********************************************************************************************************/
void 	RequestDownloadserviceDeal(unsigned char *buff,unsigned char len)
{
	unsigned char para[10];
	unsigned char ret=0;
	if(len<2)
			{
				printf("\r\nNRC: 0x13");
				Service_NRC(RequestDownloadservice,NRC_0x13_IMLOIF);
				return ;
			}
				para[0]=buff[0];
			ret=Prog_RequestDownloadservice(buff,len,para);
				if(ret)
				{
					Service_NRC(RequestDownloadservice,ret);
						printf("\r\nNRC: 0x%02x",ret);
					return;
				}
		Dianogstic_SendService(RequestDownloadservice,para,1+(para[0]>>4),ePostiveResponseMessage);
		printf("\r\nPosRespMess: RequestDownloadservice");
}
/*********************************************************************************************************
** Function name:       RequestUploadserviceDeal
** Descriptions:       	�ϴ����ݷ�����
** input parameters:    buff						��ϲ���
**											len							��ϲ�������
** Returned value:      NULL
*********************************************************************************************************/
void 	RequestUploadserviceDeal(unsigned char *buff,unsigned char len)
{
	unsigned char para[10];
	unsigned char ret=0;
	ret=Prog_RequestUploadserviceservice(buff,len,para);
	if(ret)
		{
			Service_NRC(RequestUploadservice,ret);
			printf("\r\nNRC: 0x%02x",ret);
			return;
		}
		Dianogstic_SendService(RequestUploadservice,para,1+(para[0]>>4),ePostiveResponseMessage);
		printf("\r\nPosRespMess: RequestUploadservice");
}
/*********************************************************************************************************
** Function name:       RequestTransferExitserviceDeal
** Descriptions:        ֹͣ���ݴ������
** input parameters:    buff						��ϲ���
**											len							��ϲ�������
** Returned value:      NULL
*********************************************************************************************************/
void 	RequestTransferExitserviceDeal(unsigned char *buff,unsigned char len)
{
	unsigned char para[10];
	unsigned char ret=0;
				para[0]=buff[0];
			ret=Prog_RequestTransferExitservice(buff,len);
				if(ret)
				{
					Service_NRC(RequestTransferExitservice,ret);
						printf("\r\nNRC: 0x%02x",ret);
					return;
				}
		Dianogstic_SendService(RequestTransferExitservice,para,1,ePostiveResponseMessage);
		printf("\r\nPosRespMess: RequestTransferExitservice");
}
/*********************************************************************************************************
** Function name:       TransferDataserviceDeal
** Descriptions:        ���ݴ��������
** input parameters:    buff						��ϲ���
**											len							��ϲ�������
** Returned value:      NULL
*********************************************************************************************************/
void 	TransferDataserviceDeal(unsigned char *buff,unsigned char len)
{
	unsigned char para[10];
	unsigned char ret=0;
	if(len==0)
			{
				printf("\r\nNRC: 0x13");
				Service_NRC(TransferDataservice,NRC_0x13_IMLOIF);
				return ;
			}
			para[0]=buff[0];
			ret=Prog_TransferDataservice(buff,len);
				if(ret)
				{
					Service_NRC(TransferDataservice,ret);
						printf("\r\nNRC: 0x%02x",ret);
					return;
				}
		Dianogstic_SendService(TransferDataservice,para,1,ePostiveResponseMessage);
		printf("\r\nPosRespMess: TransferDataservice");
}
/*********************************************************************************************************
** Function name:       Dianogstic_ReceiveService
** Descriptions:        ������յ���Ϸ���
** input parameters:    buff						��ϲ���
**											len							��ϲ�������
** Returned value:      NULL
*********************************************************************************************************/
void Diagnose_Communication_Recive(unsigned char *buff,unsigned char len)
{
	unsigned char i;
	
	printf("\r\nDianogstic_ReceiveService:");
		for(i=0;i<len;i++)
		{
			printf(" %02x",buff[i]);
		}
	switch(buff[0])
	{
		case DiagnosticSessionConTrolService:
				DiagnosticSessionConTrolServiceDeal(buff+1,len-1);
				break;
		case ECUResetService:
				ECUResetServiceDeal(buff+1,len-1);
				break;
		case RequestDownloadservice:
			RequestDownloadserviceDeal(buff+1,len-1);
		break;
		case RequestUploadservice:	
			RequestUploadserviceDeal(buff+1,len-1);
			break;
		case TransferDataservice:	
			TransferDataserviceDeal(buff+1,len-1);
			break;
		case RequestTransferExitservice:	
			RequestTransferExitserviceDeal(buff+1,len-1);
			break;
		case RequestFileTransferservice:	
				break;
		case ControlDTCSettingservice:
			ControlDTCSettingserviceDeal(buff+1,len-1);
			break;
		case CommunicationControlservice:
			CommunicationControlserviceDeal(buff+1,len-1);
			break;
		case RoutineControlservice:
			RoutineControlserviceDeal(buff+1,len-1);
			break;
		case ClearDiagnosticInformationService:
			ClearDiagnosticInformationServiceDeal(buff+1,len-1);
			break;

	}
}