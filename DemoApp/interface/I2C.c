
#include "Interface.h"

lpi2c_master_state_t lpi2c_master_state;

///////////////////////////////////////////////////////////////////////////////////////

/*****************************************************************
***	������ i2cmasterCallback
**  ���ܣ� 
***	��ע�� 
******************************************************************/
void i2cmasterCallback(i2c_master_event_t event,void *data)
{
	

	if(lpi2c_master_state.status)
	{
		printf("\r\nI2C0 status=%x",lpi2c_master_state.status);
	}
	 
	 if(lpi2c_master_state.status==STATUS_I2C_RECEIVED_NACK)
	{
		printf("\r\nNACK signal received");
	}
	else if(lpi2c_master_state.status==STATUS_I2C_TX_UNDERRUN)
	{
		printf("\r\nTX underrun error");
	}
	else if(lpi2c_master_state.status==STATUS_I2C_RX_OVERRUN)
	{
		printf("\r\nRX overrun error");
	}
	else if(lpi2c_master_state.status==STATUS_I2C_ARBITRATION_LOST)
	{
		printf("\r\nArbitration lost");
	}
	else if(lpi2c_master_state.status==STATUS_I2C_ABORTED)
	{
		printf("\r\nA transfer was aborted");
	}

}

/*****************************************************************
***	������ I2CMasterInit
**  ���ܣ� 
***	��ע�� 
******************************************************************/
void I2CMasterInit(u32 baud)
{
		/*����ʱ�ӿ���*/
		module_clk_config_t module_clk_config;
		module_clk_config.clkSrcFreq =LOWEST_FREQUENCY;//
		module_clk_config.clkSrcType=XOSC_CLK_SRC_TYPE;
		CLOCK_DRV_SetModuleClock(LPI2C0_CLK,&module_clk_config);
		
		pin_settings_config_t g_pin_mux_InitConfigArr[2] = 
		{
				{
					 .base          = PORTA,
					 .pinPortIdx    = 3u,
					 .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
					 .passiveFilter = false,
					 .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
					 .mux           = PORT_MUX_ALT3,
					 .pinLock       = false,
					 .intConfig     = PORT_DMA_INT_DISABLED,
					 .clearIntFlag  = false,
					 .gpioBase      = NULL,
				},
				{
						 .base          = PORTA,
						 .pinPortIdx    = 2u,
						 .pullConfig    = PORT_INTERNAL_PULL_NOT_ENABLED,
						 .passiveFilter = false,
						 .driveSelect   = PORT_LOW_DRIVE_STRENGTH,
						 .mux           = PORT_MUX_ALT3,
						 .pinLock       = false,
						 .intConfig     = PORT_DMA_INT_DISABLED,
						 .clearIntFlag  = false,
						 .gpioBase      = NULL,
					}
		};
		
		PINS_DRV_Init(2, g_pin_mux_InitConfigArr);

		lpi2c_master_user_config_t lpi2c_master_user_config;
		lpi2c_master_user_config.baudRate=baud;
		lpi2c_master_user_config.operatingMode=LPI2C_FAST_MODE;
		lpi2c_master_user_config.slaveAddress=0;
		lpi2c_master_user_config.transferType=LPI2C_USING_INTERRUPTS;
		lpi2c_master_user_config.masterCallback=i2cmasterCallback;				
		LPI2C_DRV_MasterInit(0,&lpi2c_master_user_config,&lpi2c_master_state);
		INT_SYS_EnableIRQ(LPI2C0_Master_IRQn);
		INT_SYS_SetPriority(LPI2C0_Master_IRQn, 5);	
}

/*****************************************************************
***	������ I2CMasterDeInit
**  ���ܣ� 
***	��ע�� 
******************************************************************/
void I2CMasterDeInit(void)
{
		LPI2C_DRV_MasterDeinit(0);
		INT_SYS_DisableIRQ(LPI2C0_Master_IRQn);
}
/*****************************************************************
***	������ I2C_GetMasterState
**  ���ܣ� ��ȡI2C��ǰ״̬
***	��ע�� 
******************************************************************/
u32 I2C_GetMasterState()
{
	return lpi2c_master_state.status;
}
/*****************************************************************
***	������ I2C_TransferBuff
**  ���ܣ� ����I2C���ݹ��ܵĵײ�ӿں���
***	��ע�� ��slaveAddress bit0���ֶ�д
******************************************************************/
u32 I2C_TransferBuff(u8 slaveAddress,u8 *buffer,u8 len)
{
	if(lpi2c_master_state.status==STATUS_BUSY)
		return STATUS_BUSY;
	LPI2C_DRV_MasterSetSlaveAddr(0,slaveAddress>>1,false);
	
	if(slaveAddress&0x1)	
	LPI2C_DRV_MasterReceiveData(0, buffer, len, true);
			else
	LPI2C_DRV_MasterSendData(0,buffer,len,true);
	return 0;
}
/*****************************************************************
***	������ Interface_I2C_Write
**  ���ܣ� I2Cд����
***	��ע�� adderΪ8λ��ַ
******************************************************************/
void Interface_I2C_Write(u8 adder,u8 *buffer,u8 len)
{
	while(STATUS_BUSY==I2C_GetMasterState());
	I2C_TransferBuff(adder,buffer,len);	
}
/*****************************************************************
***	������ Interface_I2C_Read
**  ���ܣ� I2Cд����
***	��ע�� adderΪ8λ
******************************************************************/
bool Interface_I2C_Read(u8 adder,u8 *buffer,u8 len)
{
	while(STATUS_BUSY==I2C_GetMasterState());//�ȴ�����

	I2C_TransferBuff((adder|0x1),buffer,len);

	while(lpi2c_master_state.status==STATUS_BUSY)
				;
	return lpi2c_master_state.status?false:true;	
}
/*****************************************************************
***	������ Interface_I2C_Read
**  ���ܣ� I2Cд����
***	��ע�� adderΪ8λ��ַ��SlaveAddr�Ĵ�����ַ
******************************************************************/
bool Interface_I2C_SlaveAddr_Read(u8 adder,u8 SlaveAddr,u8 *buffer,u8 len)
{
	while(STATUS_BUSY==I2C_GetMasterState());
	I2C_TransferBuff(adder,&adder,1);
	while(STATUS_BUSY==I2C_GetMasterState());
	I2C_TransferBuff((adder|0x1),buffer,len);
	while(STATUS_BUSY==I2C_GetMasterState());
	return I2C_GetMasterState();	
}






