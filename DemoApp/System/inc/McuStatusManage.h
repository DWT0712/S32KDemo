#ifndef _MCU_STATUS_MANAGE_H_
#define _MCU_STATUS_MANAGE_H_

//#define ACCSLEEPTIME  (T_1S * 30)     //ACC=0ʱά�ֶ೤ʱ����������
#define ACCSLEEPTIME  (T_1S * 5)     //ACC=0ʱά�ֶ೤ʱ����������
#define ACCFILTERTIME (T_1S * 2)      //����Ƶ�����ػ���ʱ����ֵ
#define MSM1PROTECTTTIME (T_1S * 3)   //��MSM1û�����߳�ʱ,��ʾ��ͨ�����쳣
#define ASYNCRESETMSM1PROTECTTTIME (T_1S * 15)   //��LPC�����첽��λʱ�ӳ�MSM1���ŵļ��ʱ��

typedef enum
{
	MCUMANAGE_ST_S0 = 1,
	MCUMANAGE_ST_S1,
	MCUMANAGE_ST_S2_1_0,
	MCUMANAGE_ST_S2_1_0_0,
	MCUMANAGE_ST_S2_1_0_1,
	MCUMANAGE_ST_S2_1_0_2,
	MCUMANAGE_ST_S2_1_1,
	MCUMANAGE_ST_S2_1_2,
	MCUMANAGE_ST_S2_2_0,
	MCUMANAGE_ST_S2_2_1,
	MCUMANAGE_ST_S2_3,
	MCUMANAGE_ST_S3_1,
	MCUMANAGE_ST_S3_2,
	MCUMANAGE_ST_S4_0, //�쳣״̬
}enumMcuStatus;


void McuStatusManageInit(void);
void McuStatusManageProc(void);
void McuStatusManageIpcEventRegister(void);


#endif

