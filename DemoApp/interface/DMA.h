
#ifndef __DMA__H__
#define __DMA__H__

#define DMA_ADC0_CHAN    0
#define DMA_ADC1_CHAN    1
#define DMA_CANFIFORX_CHAN    2
#define DMA_CAN1FIFORX_CHAN    3
#define DMA_CAN2FIFORX_CHAN    4

void DMA_Init(void);
void DMA_DeInit(void) ;

status_t DMA__ChannelInit(uint8_t instance ,edma_chn_state_t *edmaChannelState,
                              const edma_channel_config_t *edmaChannelConfig,bool enableInt);
status_t DMA_ConfigSingleTransfer(uint8_t virtualChannel,
																						edma_transfer_type_t type,
                                            uint32_t srcAddr,//Դ��ַ
                                            uint32_t destAddr,//Ŀ���ַ
                                            edma_transfer_size_t transferSize,//����bit
                                            uint32_t dataBufferSize,//�����С
																						bool isRestAdder);//�Ƿ��ڴ�����ɺ����贫���ַ
#endif




