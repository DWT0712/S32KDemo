#include "interface.h"

edma_state_t DMAController_State;

/*FUNCTION**********************************************************************
 *
 * Function Name : DMA_Init
 * Description   : DMA��ʼ��
 *
 * Implements    : 
 *END**************************************************************************/
void DMA_Init()
{
	const edma_user_config_t DMAController1_InitConfig = {
		.chnArbitration = EDMA_ARBITRATION_FIXED_PRIORITY,
		.notHaltOnError = false,
	};
	EDMA_DRV_Init(&DMAController_State, &DMAController1_InitConfig, NULL, NULL, 0); //DMA��ʼ��
}

/*FUNCTION**********************************************************************
 *
 * Function Name : DMA_DeInit
 * Description   : 
 *
 * Implements    : 
 *END**************************************************************************/
void DMA_DeInit(void)
{
	EDMA_DRV_Deinit();
}

/*FUNCTION**********************************************************************
 *
 * Function Name : DMA__ChannelInit
 * Description   : DMAͨ����ʼ��
 *
 * Implements    : EDMA_DRV_ChannelInit_Activity
 *END**************************************************************************/
status_t DMA__ChannelInit(u8 instance, edma_chn_state_t *edmaChannelState,
						  const edma_channel_config_t *edmaChannelConfig, BOOL enableInt)
{
	EDMA_DRV_ChannelInit(edmaChannelState, edmaChannelConfig);
	if (!enableInt)
		INT_SYS_DisableIRQ(instance);
	EDMA_DRV_StartChannel(instance); //ʹ��DMAͨ��0
	return 0;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : DMA_ConfigSingleTransfer
 * Description   : DMAͨ����������
 *
 * Implements    : EDMA_DRV_ConfigSingleBlockTransfer_Activity
 *END**************************************************************************/
status_t DMA_ConfigSingleTransfer(uint8_t virtualChannel,
								  edma_transfer_type_t type,
								  uint32_t srcAddr,					 //Դ��ַ
								  uint32_t destAddr,				 //Ŀ���ַ
								  edma_transfer_size_t transferSize, //����bit
								  uint32_t dataBufferSize,			 //�����С
								  BOOL isRestAdder)					 //�Ƿ��ڴ�����ɺ����贫���ַ
{
	EDMA_DRV_ConfigSingleBlockTransfer(virtualChannel, type, srcAddr, destAddr, transferSize, dataBufferSize); //��ַ����
	if (isRestAdder)																						   //���DMA���������ԴĿ���ַ
	{
		EDMA_DRV_SetSrcLastAddrAdjustment(virtualChannel, -dataBufferSize);  //����ɹ���ַ����
		EDMA_DRV_SetDestLastAddrAdjustment(virtualChannel, -dataBufferSize); //����ɹ���ַ����
	}
	return 0;
}
