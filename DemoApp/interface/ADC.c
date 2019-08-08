


#include "Interface.h"


edma_chn_state_t dmaController1Chn0_State;
edma_chn_state_t dmaController1Chn1_State;

volatile uint32_t ADC0ConvertBuffr[ADC0ConvertBuffrtSize] __attribute__ ((aligned (32)));
volatile uint32_t ADC1ConvertBuffr[ADC1ConvertBuffrtSize] __attribute__ ((aligned (32)));
/**************************************************************
**** �������� ADC_ConfigChan
**** �������ܣ� ����back to backģʽ�ļĴ�����
**** ��ע��  PDBֻʹ��ͨ��0�����ܳ���8��ADCͨ��ת����
****************************************************************/
static inline void ADC_ConfigChan(u32 adcVa)
{
			u8 R=adcVa>>16;
			u8 instance=(adcVa>>8)&0xff;
			adc_chan_config_t adConv1_ChnConfig1 = {.interruptEnable = false,.channel=(adc_inputchannel_t)(adcVa&0xff)};
			if(instance==0)
			{
				 ADC_DRV_ConfigChan(instance, R, &adConv1_ChnConfig1);//���ø��ԵļĴ���
			PDB0->CH[0].C1 |=PDB_C1_BB(R==0?0:0x1<<R)  | PDB_C1_TOS(R==0?0x1:0) | PDB_C1_EN(0x1<<R);
		
			}
			else if(instance==1)
			{
		 ADC_DRV_ConfigChan(instance, R, &adConv1_ChnConfig1);//���ø��ԵļĴ���
			PDB1->CH[0].C1 |=PDB_C1_BB(R==0?0:0x1<<R)  | PDB_C1_TOS(R==0?0x1:0) | PDB_C1_EN(0x1<<R);
			}
}


/**************************************************************
**** ��������ADC_init
**** �������ܣ���ʼ����ͨ����ADC�ɼ�
**** ��ע��
****************************************************************/
void ADC_init(void) 
{
		/*ADC0������*/
		const adc_converter_config_t adConv1_ConvConfig1 = 
		{
			.clockDivide = ADC_CLK_DIVIDE_1,
			.sampleTime = 255,
			.resolution = ADC_RESOLUTION_12BIT,//12λ����
			.inputClock = ADC_CLK_ALT_1,
			.trigger = ADC_TRIGGER_HARDWARE,//Ӳ��������������������ö�ͨ���ɼ���
			.pretriggerSel = ADC_PRETRIGGER_SEL_PDB,
			.triggerSel = ADC_TRIGGER_SEL_PDB,
			.dmaEnable = false,//��dma����
			.voltageRef = ADC_VOLTAGEREF_VREF,
			.continuousConvEnable = false,//ADC��������������ʹ��PDB����������
			.supplyMonitoringEnable = false,
		};
		
		/*ͨ��������*/
		//***********************        PDB            ****************************
		const pdb_timer_config_t pdly1_InitConfig0 = 
		{
			.loadValueMode = PDB_LOAD_VAL_IMMEDIATELY,
			.seqErrIntEnable = false,
			.clkPreDiv = PDB_CLK_PREDIV_BY_128,
			.clkPreMultFactor = PDB_CLK_PREMULT_FACT_AS_10,
			.triggerInput = PDB_SOFTWARE_TRIGGER,
			.continuousModeEnable = true,//����ģʽ
			.dmaEnable = true,//DMA��������
			.intEnable = false,
		};
			
		//***********************        DMA            ****************************
		edma_channel_config_t dmaController1Chn0_Config = 
		{
					.channelPriority = EDMA_CHN_DEFAULT_PRIORITY,
					.virtChnConfig = DMA_ADC0_CHAN,//ʹ��DMAͨ��0
					.source = EDMA_REQ_PDB0,//PDB����DMA����
					.callback = NULL,//����Ҫ�ж�
					.callbackParam = NULL
		};
		edma_channel_config_t dmaController1Chn1_Config = 
		{
				.channelPriority = EDMA_CHN_DEFAULT_PRIORITY,
				.virtChnConfig = DMA_ADC1_CHAN,//ʹ��DMAͨ��0
				.source = EDMA_REQ_PDB1,//PDB����DMA����
				.callback = NULL,//����Ҫ�ж�
				.callbackParam = NULL
		};
	
		ADC_DRV_ConfigConverter(DMA_ADC0_CHAN, &adConv1_ConvConfig1);//��ʼ��
		ADC_DRV_ConfigConverter(DMA_ADC1_CHAN, &adConv1_ConvConfig1);
		/************************************************************************************/
		DMA__ChannelInit(DMA_ADC0_CHAN,&dmaController1Chn0_State,&dmaController1Chn0_Config,false);
  	DMA__ChannelInit(DMA_ADC1_CHAN,&dmaController1Chn1_State,&dmaController1Chn1_Config,false);	
		DMA_ConfigSingleTransfer(DMA_ADC0_CHAN, EDMA_TRANSFER_MEM2MEM, (uint32_t)&ADC0->R[0],//����ADC0�����ַ
										(uint32_t)ADC0ConvertBuffr, EDMA_TRANSFER_SIZE_4B, sizeof(ADC0ConvertBuffr),true);
		DMA_ConfigSingleTransfer(DMA_ADC1_CHAN, EDMA_TRANSFER_MEM2MEM, (uint32_t)&ADC1->R[0],//����ADC1�����ַ
										(uint32_t)ADC1ConvertBuffr, EDMA_TRANSFER_SIZE_4B, sizeof(ADC1ConvertBuffr),true);

		/************************************************************************************/
		PDB_DRV_Init(0, &pdly1_InitConfig0);//PDB��ʼ��
		PDB_DRV_Init(1, &pdly1_InitConfig0);
		PDB_DRV_Enable(0);
		PDB_DRV_Enable(1);
		PDB_DRV_SetValueForTimerInterrupt(0,0);//ÿ�δ���ADCת��������һ��ת����ɵ�����ͨ��DMA����buffer
		PDB_DRV_SetValueForTimerInterrupt(1,0);//ÿ�δ���ADCת��������һ��ת����ɵ�����ͨ��DMA����buffer
		PDB_DRV_SetTimerModulusValue(0, (uint32_t) 6250);//10ms PDB������ͨ���ɼ���Ҫʱ��=��ͨ��ת��ʱ��(��ǰ���õ�ͨ��ADCת��ʱ��Ϊ0.2ms)+(2~3)ms
		PDB_DRV_SetTimerModulusValue(1, (uint32_t) 6250);//10ms PDB������ͨ���ɼ���Ҫʱ��=��ͨ��ת��ʱ��(��ǰ���õ�ͨ��ADCת��ʱ��Ϊ0.2ms)+(2~3)ms
		PDB_DRV_LoadValuesCmd(0);//PDB����װ��
		PDB_DRV_LoadValuesCmd(1);//PDB����װ��
		/************************************************************************************/
	
		ADC_ConfigChan(CHANNEL_ADC_LEFTTURNLAMPERRDETECT);
		ADC_ConfigChan(CHANNEL_ADC_LIGHT);
		ADC_ConfigChan(CHANNEL_ADC_BATVOLDETECT);
		ADC_ConfigChan(CHANNEL_ADC_FRONTWIPERSENITYADJ);
		ADC_ConfigChan(CHANNEL_ADC_HANDBRAKE_CURRENT);
		ADC_ConfigChan(CHANNEL_ADC_HANDBRAKE);
		ADC_ConfigChan(CHANNEL_ADC_HANDBRAKE_RELEASE);
	
		PDB_DRV_SoftTriggerCmd(0);//PDB���� ����һ�Σ������Զ�����
		PDB_DRV_SoftTriggerCmd(1);//PDB����		
}

/*****************************************************************************
**��������:	 	ADC_Deinit
**��������:	 	
**��ڲ���:
**���ز���:
******************************************************************************/
void ADC_Deinit(void)  {
	ADC_DRV_Reset(0);
	ADC_DRV_Reset(1);
	EDMA_DRV_Deinit();
	PDB_DRV_Deinit(0);
	PDB_DRV_Deinit(1);
}

/*****************************************************************
***	������ GetAdcChannelValue
**  ���ܣ� ��ȡָ��ADCͨ��ֵ
***	��ע�� 
******************************************************************/
uint16_t GetAdcChannelValue(uint32_t Channel)
{
	if((Channel&0xff00)==0)
	{
		return ADC0ConvertBuffr[Channel>>16];
	}
	else if((Channel&0xff00)==0x100)
	{
		return ADC1ConvertBuffr[Channel>>16];
	}
	else
		return 0;
}


