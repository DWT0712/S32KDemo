#include "interface.h"
void WatchdogInit()
{
	wdog_user_config_t wdog_user_config;
	wdog_user_config.clkSource = WDOG_SOSC_CLOCK;
	wdog_user_config.intEnable = false;
	wdog_user_config.opMode.wait = false;
	wdog_user_config.opMode.stop = false;
	wdog_user_config.opMode.debug = true;
	wdog_user_config.prescalerEnable = true;
	wdog_user_config.timeoutValue = 0x7a12; //8M CLOCK ���Ź�Լ1s��λ
	wdog_user_config.updateEnable = true;
	wdog_user_config.windowValue = 0;
	wdog_user_config.winEnable = false;

	WDOG_DRV_Init(0, &wdog_user_config);
}
/*****************************************************************************
**��������:	 	FeedWdt
**��������:	 	
**��ڲ���:
**���ز���:
******************************************************************************/
void FeedWdt()
{
	WDOG_DRV_Trigger(0);
}
