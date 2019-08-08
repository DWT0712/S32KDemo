/*********************************************Copyright (c)***********************************************
**                                Guangzhou ZLG MCU Technology Co., Ltd.
**
**                                        http://www.zlgmcu.com
**
**      ������������Ƭ���Ƽ����޹�˾���ṩ�����з�������ּ��Э���ͻ����ٲ�Ʒ���з����ȣ��ڷ�����������ṩ
**  ���κγ����ĵ������Խ����������֧�ֵ����Ϻ���Ϣ���������ο����ͻ���Ȩ��ʹ�û����вο��޸ģ�����˾��
**  �ṩ�κε������ԡ��ɿ��Եȱ�֤�����ڿͻ�ʹ�ù��������κ�ԭ����ɵ��ر�ġ�żȻ�Ļ��ӵ���ʧ������˾��
**  �е��κ����Ρ�
**                                                                        ����������������Ƭ���Ƽ����޹�˾
**
**--------------File Info---------------------------------------------------------------------------------
** File name:           IAP.c
** Last modified Date:  2013-05-20
** Last Version:        V1.0
** Descriptions:        IAP����-IAP����
**
**--------------------------------------------------------------------------------------------------------
** Created by:          ChenQiliang
** Created date:        2013-05-24
** Version:             V1.0
** Descriptions:        ����û�Ӧ�ó���
**
**--------------------------------------------------------------------------------------------------------
** Modified by:         
** Modified date:       
** Version:             
** Descriptions:        
** Rechecked by:        
*********************************************************************************************************/
#include "IAP.h"

/*
 *  ���庯��ָ��  
 */
void (*IAP_Entry)(INT32U *param_tab, INT32U *result_tab) = (void(*)())IAP_ENTER_ADR;

INT32U  paramin[5];                                                     /* IAP��ڲ���������            */
INT32U  paramout[4];                                                    /* IAP���ڲ���������            */

/*********************************************************************************************************
** Function name:       sectorPrepare
** Descriptions:        IAP��������ѡ���������50
** input parameters:    sec1:           ��ʼ����
**                      sec2:           ��ֹ����
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
INT32U  sectorPrepare (INT8U sec1, INT8U sec2)
{  
    paramin[0] = IAP_Prepare;                                           /* ����������                   */
    paramin[1] = sec1;                                                  /* ���ò���                     */
    paramin[2] = sec2;                            
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */
   
    return (paramout[0]);                                               /* ����״̬��                   */
}

/*********************************************************************************************************
** Function name:       ramCopy
** Descriptions:        ����RAM�����ݵ�FLASH���������51
** input parameters:    dst:            Ŀ���ַ����FLASH��ʼ��ַ����512�ֽ�Ϊ�ֽ�
**                      src:            Դ��ַ����RAM��ַ����ַ�����ֶ���
**                      no:             �����ֽڸ�����Ϊ512/1024/4096/8192
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
INT32U  ramCopy(INT32U dst, INT32U src, INT32U no)
{  
    paramin[0] = IAP_RAMTOFLASH;                                        /* ����������                   */
    paramin[1] = dst;                                                   /* ���ò���                     */
    paramin[2] = src;
    paramin[3] = no;
    paramin[4] = IAP_FCCLK;
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */
    
    return (paramout[0]);                                               /* ����״̬��                   */
}

/*********************************************************************************************************
** Function name:       sectorErase
** Descriptions:        �����������������52
** input parameters:    sec1            ��ʼ����
**                      sec2            ��ֹ����92
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
INT32U  sectorErase (INT8U sec1, INT8U sec2)
{  
    paramin[0] = IAP_ERASESECTOR;                                       /* ����������                   */
    paramin[1] = sec1;                                                  /* ���ò���                     */
    paramin[2] = sec2;
    paramin[3] = IAP_FCCLK;
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */
   
    return (paramout[0]);                                               /* ����״̬��                   */
}

/*********************************************************************************************************
** Function name:       blankChk
** Descriptions:        ������գ��������53
** input parameters:    sec1:           ��ʼ����
**                      sec2:           ��ֹ����92
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
INT32U  blankChk (INT8U sec1, INT8U sec2)
{  
    paramin[0] = IAP_BLANKCHK;                                          /* ����������                   */
    paramin[1] = sec1;                                                  /* ���ò���                     */
    paramin[2] = sec2;
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */

    return (paramout[0]);                                               /* ����״̬��                   */
}

/*********************************************************************************************************
** Function name:       parIdRead
** Descriptions:        ������ID���������54
** input parameters:    ��
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
INT32U  parIdRead (void)
{  
    paramin[0] = IAP_READPARTID;                                        /* ����������                   */
    
    paramin[1] = 0x00;
    paramin[2] = 0x00;
    paramin[3] = 0x00;
    paramin[4] = 0x00;
    
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */

    
    
    return (paramout[0]);                                               /* ����״̬��                   */
}

/*********************************************************************************************************
** Function name:       codeIdBoot
** Descriptions:        ��Boot�汾�ţ��������55
** input parameters:    ��
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
INT32U  codeIdBoot (void)
{  
    paramin[0] = IAP_BOOTCODEID;                                        /* ����������                   */
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */

    return (paramout[0]);                                               /* ����״̬��                   */
}

/*********************************************************************************************************
** Function name:       dataCompare
** Descriptions:        У�����ݣ��������56
** input parameters:    dst:            Ŀ���ַ����RAM/FLASH��ʼ��ַ����ַ�����ֶ���
**                      src:            Դ��ַ����FLASH/RAM��ַ����ַ�����ֶ���
**                      no:             �����ֽڸ����������ܱ�4����
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
INT32U  dataCompare (INT32U dst, INT32U src, INT32U no)
{  
    paramin[0] = IAP_COMPARE;                                           /* ����������                   */
    paramin[1] = dst;                                                   /* ���ò���                     */
    paramin[2] = src;
    paramin[3] = no;
    (*IAP_Entry)(paramin, paramout);                                    /* ����IAP�������              */

    return (paramout[0]);                                               /* ����״̬��                   */
}

/*********************************************************************************************************
  End Of File
*********************************************************************************************************/

