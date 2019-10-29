


#include "Interface.h"
#define	DEBUG_UART_PORT      0          //���Դ��ڶ˿ں�
#define MCU_DEBUG_TXD   GPIO_PTB0 //���Դ��������
#define MCU_DEBUG_RXD   GPIO_PTB1	//���Դ��������
#define DEBUG_BAUD      115200		//���Դ��ڲ�����
typedef void (*UART_handler)(uint8_t data);
static void LpuartInit(unsigned char uart_Nu,U32 BAUD,LPGPIO_PinType txPort,LPGPIO_PinType RxPort,UART_handler Handler);
static void  Debug_Handler(u8 data);
static inline void UartPin_Init(LPGPIO_PinType Pin);



static UART_handler UARTHandler[3] = {NULL, NULL, NULL};
void DebugInit(void)
{
	
	LpuartInit(DEBUG_UART_PORT,DEBUG_BAUD,MCU_DEBUG_TXD,MCU_DEBUG_RXD,Debug_Handler);
	
}

/*****************************************************************************
**��������:	 	LpuartInit
**��������:	 ���ڹ��ܳ�ʼ������
**��ڲ���:
**���ز���:
******************************************************************************/
static void LpuartInit(unsigned char uart_Nu,U32 BAUD,LPGPIO_PinType txPort,LPGPIO_PinType RxPort,UART_handler Handler)
{
	
				IRQn_Type LPIrq[]={LPUART0_RxTx_IRQn,LPUART1_RxTx_IRQn,LPUART2_RxTx_IRQn};
					if(uart_Nu>2||txPort>=GPIO_END||RxPort>=GPIO_END)
						return;
						/*������������*/
					{
						UartPin_Init(txPort);
						UartPin_Init(RxPort);
					}
				////////////////////////////////////////////////////////////////////
	
				/*��������*/
				{
					LPUART_Type * LPUARTx[]= LPUART_BASE_PTRS;
					tlpuart_config UARTCOF;
					UARTCOF.baudRate=BAUD;//������
					UARTCOF.bitCountPerChar=LPUART_8_BITS_PER_CHAR;//8λ����λ
					UARTCOF.parityMode=LPUART_PARITY_DISABLED;//����żУ��
					UARTCOF.stopBitCount=LPUART_ONE_STOP_BIT;//1ֹͣλ
					lpuart_device_Init(LPUARTx[uart_Nu],UARTCOF);
					UARTHandler[uart_Nu]=Handler;
					INT_SYS_EnableIRQ(LPIrq[uart_Nu]);
					INT_SYS_SetPriority(LPIrq[uart_Nu], 5);	
			}
			////////////////////////////////////////////////////////////////////
	 
	
}

/*****************************************************************************
**��������:	 	UartPin_Init
**��������:	 �������ų�ʼ������
**��ڲ���:
**���ز���:
******************************************************************************/
static inline void UartPin_Init(LPGPIO_PinType Pin)
{
						PORT_Type *Port_PRRS[]=PORT_BASE_PTRS;
						GPIO_Type *Gpio_PTRS[]=GPIO_BASE_PTRS;
						pin_settings_config_t config;
						memset(&config,0,sizeof(pin_settings_config_t));
						config.base=Port_PRRS[Pin/18];
						config.gpioBase=Gpio_PTRS[Pin/18];
					//	config.mux=PORT_MUX_ALT2;
						config.pinPortIdx=Pin%18;
						config.pullConfig=PORT_INTERNAL_PULL_DOWN_ENABLED;	
	
						switch((unsigned char)Pin)
						{
							case GPIO_PTA3:
							case GPIO_PTA2:
								config.mux=PORT_MUX_ALT6;
								break;
							case GPIO_PTB0:
							case GPIO_PTB1:
							case GPIO_PTD7:
							case GPIO_PTD6:		
							case GPIO_PTC8:
							case GPIO_PTC9:
							case GPIO_PTC6:
							case GPIO_PTC7:
								config.mux=PORT_MUX_ALT2;
								break;
							case GPIO_PTE12:
							case GPIO_PTD17:
									config.mux=PORT_MUX_ALT3;
							break;
							default:
								config.mux=PORT_MUX_AS_GPIO;
							return;

						}
						
						PINS_Init(&config);

}



/*****************************************************************
***	������ fputc
**  ���ܣ� 
***	��ע�� 
******************************************************************/
int fputc(int ch, FILE *f)
{
	
	#define DMADebug 0
	#define   D_LPUARTx       LPUART0

	uint8_t c = (uint8_t)ch& 0xFF;
	uint8_t i;
	char timestr[16];
	int size = 0;

	switch(c)
	{
	case '\n':
		c = '\r';
		#if DMADebug
		FIFO_PUT_ONE(DebugTxFifo,c);
	#else
	lpuart_Putchar(D_LPUARTx, c);
	#endif
		c = '\n';
	#if DMADebug
		FIFO_PUT_ONE(DebugTxFifo,c);
	#else
	lpuart_Putchar(D_LPUARTx, c);
	#endif
		size = snprintf(timestr, 16, "[%10u] ", TimerGetSystemTimer());
		for(i = 0; i < size; i++)
		#if DMADebug
		FIFO_PUT_ONE(DebugTxFifo,timestr[i]);
	#else
	lpuart_Putchar(D_LPUARTx, timestr[i]);
	#endif
		break;
	case '\r':
		break;
	default:
		#if DMADebug
		FIFO_PUT_ONE(DebugTxFifo,c);
	#else
	lpuart_Putchar(D_LPUARTx, c);
	#endif
		break;
	}
	
	

	return 1;
}
/*****************************************************************
***	������ LPUARTx_IRQHandler
**  ���ܣ� 
***	��ע�� 
******************************************************************/
void LPUARTx_IRQHandler(u8 No)
{
	U8 receive;
	LPUART_Type * LPUARTx[]= LPUART_BASE_PTRS;
	if(No>2)
		return;

	  if((LPUARTx[No]->STAT & LPUART_STAT_RDRF_MASK)>>LPUART_STAT_RDRF_SHIFT)
		{
			
			  receive = LPUARTx[No]->DATA;            /* Read received data*/
			//	printf("(%c) ",receive);
		}
		if(UARTHandler[No]!=NULL)
		UARTHandler[No](receive);
		if(LPUARTx[No]->STAT&0x80000)
		{
			LPUARTx[No]->STAT|=0x80000;
		}
}
void LPUART0_RxTx_IRQHandler()
{
	
		LPUARTx_IRQHandler(0);
//	printf("\r\nLPUART0_RxTx_IRQHandler");
}
/*****************************************************************
***	������ LPUART1_RxTx_IRQHandler
**  ���ܣ� 
***	��ע�� 
******************************************************************/
void LPUART1_RxTx_IRQHandler()
{
	  LPUARTx_IRQHandler(1);
	//	printf("\r\nLPUART1_RxTx_IRQHandler");
}
/*****************************************************************
***	������ LPUART2_RxTx_IRQHandler
**  ���ܣ� 
***	��ע�� 
******************************************************************/
void LPUART2_RxTx_IRQHandler()
{
	   LPUARTx_IRQHandler(2);
}

/*****************************************************************
***	������ Debug_Handler
**  ���ܣ� 
***	��ע�� 
******************************************************************/
static void  Debug_Handler(u8 data)
{
	//PutAndroidCommRxFifoMsg(data);
}






