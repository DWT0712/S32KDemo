
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
void I2CMasterInit(uint32_t baud)
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
uint32_t I2C_GetMasterState()
{
	return lpi2c_master_state.status;
}
/*****************************************************************
***	������ I2C_TransferBuff
**  ���ܣ� ����I2C���ݹ��ܵĵײ�ӿں���
***	��ע�� ��slaveAddress bit0���ֶ�д
******************************************************************/
uint32_t I2C_TransferBuff(uint8_t slaveAddress,uint8_t *buffer,uint8_t len)
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
***	��ע�� addrΪ8λ��ַ
******************************************************************/
void Interface_I2C_Write(uint8_t addr,uint8_t *buffer,uint8_t len)
{
	while(STATUS_BUSY==I2C_GetMasterState());
	I2C_TransferBuff(addr,buffer,len);	
}
/*****************************************************************
***	������ Interface_I2C_Read
**  ���ܣ� I2Cд����
***	��ע�� addrΪ8λ
******************************************************************/
bool Interface_I2C_Read(uint8_t addr,uint8_t *buffer,uint8_t len)
{
	while(STATUS_BUSY==I2C_GetMasterState());//�ȴ�����

	I2C_TransferBuff((addr|0x1),buffer,len);

	while(lpi2c_master_state.status==STATUS_BUSY)
				;
	return lpi2c_master_state.status?false:true;	
}
/*****************************************************************
***	������ Interface_I2C_SlaveAddr_Read
**  ���ܣ� I2Cд����
***	��ע�� addrΪ8λ��ַ��SlaveAddr�Ĵ�����ַ
******************************************************************/
bool Interface_I2C_SlaveAddr_Read(uint8_t addr,uint8_t SlaveAddr,uint8_t *buffer,uint8_t len)
{
	uint8_t rStartBuf[2];
	rStartBuf[0] = addr;
	rStartBuf[1] = SlaveAddr;

	while(STATUS_BUSY==I2C_GetMasterState());
	I2C_TransferBuff((addr | 0x1), rStartBuf, 2);
	while(STATUS_BUSY==I2C_GetMasterState());
	I2C_TransferBuff((addr|0x1),buffer,len);
	while(STATUS_BUSY==I2C_GetMasterState());
	return I2C_GetMasterState();	
}

/*****************************************************************
***	������ Interface_I2C_Write
**  ���ܣ� I2Cд����
***	��ע�� addrΪ8λ��ַ
******************************************************************/
bool Interface_I2C_SlaveAddr_Write(uint8_t addr, uint8_t *buffer, uint8_t len)
{
	while (STATUS_BUSY == I2C_GetMasterState());
	I2C_TransferBuff(addr, &addr, 1);
	while (STATUS_BUSY == I2C_GetMasterState());
	I2C_TransferBuff((addr & 0xFE), buffer, len);
	while (STATUS_BUSY == I2C_GetMasterState());
	return I2C_GetMasterState();
}
