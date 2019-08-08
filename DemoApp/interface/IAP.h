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
** File name:           IAP.h
** Last modified Date:  2013-05-04
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
#ifndef __IAP_H
#define __IAP_H

//#include "LPC17xx.h"                                                    /* LPC17xx����Ĵ���            */
#define INT32U    unsigned long
#define INT16U    unsigned short
#define INT8U     unsigned char

/* 
 *  ����CCLKֵ��С����λΪKHz 
 */
#define  IAP_FCCLK            (24000UL)
#define  IAP_ENTER_ADR        0x1FFF1FF1                                /* IAP��ڵ�ַ����              */


/* 
 *  ����IAP������
 */
#define     IAP_Prepare                                50               /* ѡ������                     */
                                                                        /* ����ʼ�����š����������š�   */
#define     IAP_RAMTOFLASH                             51               /* �������� FLASHĿ���ַ       */
                                                                        /* RAMԴ��ַ    ����д���ֽ���  */
                                                                        /* ϵͳʱ��Ƶ�ʡ�               */
#define     IAP_ERASESECTOR                            52               /* ��������    ����ʼ������     */
                                                                        /* ���������š�ϵͳʱ��Ƶ�ʡ�   */
#define     IAP_BLANKCHK                               53               /* �������    ����ʼ�����š�   */
                                                                        /* ���������š�                 */
#define     IAP_READPARTID                             54               /* ������ID    ���ޡ�           */
#define     IAP_BOOTCODEID                             55               /* ��Boot�汾�š��ޡ�           */
#define     IAP_COMPARE                                56               /* �Ƚ�����    ��Flash��ʼ��ַ  */
                                                                        /* RAM��ʼ��ַ����Ҫ�Ƚϵ�      */
                                                                        /* �ֽ�����                     */
#define     IAP_REINVOKEISP                            57               /* Reinvoke ISP                 */
#define     IAP_READUID                                58               /* Read UID                     */
   
/*
 *  ����IAP����״̬��
 */
#define     CMD_SUCCESS                                0
#define     INVALID_COMMAND                            1
#define     SRC_ADDR_ERROR                             2
#define     DST_ADDR_ERROR                             3
#define     SRC_ADDR_NOT_MAPPED                        4
#define     DST_ADDR_NOT_MAPPED                        5
#define     COUNT_ERROR                                6
#define     INVALID_SECTOR                             7
#define     SECTOR_NOT_BLANK                           8
#define     SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION    9
#define     COMPARE_ERROR                              10
#define     BUSY                                       11

extern void (*IAP_Entry) (INT32U param_tab[], INT32U result_tab[]);

/*********************************************************************************************************
** Function name:       sectorPrepare
** Descriptions:        IAP��������ѡ���������50
** input parameters:    sec1:           ��ʼ����
**                      sec2:           ��ֹ����
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
extern INT32U  sectorPrepare (INT8U sec1, INT8U sec2);

/*********************************************************************************************************
** Function name:       ramCopy
** Descriptions:        ����RAM�����ݵ�FLASH���������51
** input parameters:    dst:            Ŀ���ַ����FLASH��ʼ��ַ����512�ֽ�Ϊ�ֽ�
**                      src:            Դ��ַ����RAM��ַ����ַ�����ֶ���
**                      no:             �����ֽڸ�����Ϊ512/1024/4096/8192
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
extern INT32U  ramCopy(INT32U dst, INT32U src, INT32U no);

/*********************************************************************************************************
** Function name:       sectorErase
** Descriptions:        �����������������52
** input parameters:    sec1            ��ʼ����
**                      sec2            ��ֹ����92
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
extern INT32U  sectorErase (INT8U sec1, INT8U sec2);

/*********************************************************************************************************
** Function name:       blankChk
** Descriptions:        ������գ��������53
** input parameters:    sec1:           ��ʼ����
**                      sec2:           ��ֹ����92
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
extern INT32U  blankChk (INT8U sec1, INT8U sec2);

/*********************************************************************************************************
** Function name:       parIdRead
** Descriptions:        ������գ��������54
** input parameters:    ��
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
extern  INT32U  parIdRead (void);

/*********************************************************************************************************
** Function name:       codeIdBoot
** Descriptions:        ������գ��������55
** input parameters:    ��
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
extern INT32U  codeIdBoot (void);

/*********************************************************************************************************
** Function name:       dataCompare
** Descriptions:        У�����ݣ��������56
** input parameters:    dst:            Ŀ���ַ����RAM/FLASH��ʼ��ַ����ַ�����ֶ���
**                      src:            Դ��ַ����FLASH/RAM��ַ����ַ�����ֶ���
**                      no:             �����ֽڸ����������ܱ�4����
** output parameters:   paramout[0]:    IAP����״̬��,IAP����ֵ
** Returned value:      paramout[0]:    IAP����״̬��,IAP����ֵ
*********************************************************************************************************/
extern INT32U  dataCompare (INT32U dst, INT32U src, INT32U no);

#endif                                                                  /*  _IAP_H_                     */

/*********************************************************************************************************
  End Of File
*********************************************************************************************************/

