/*
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * @file flexcan_driver.c
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 15.5, Return statement before end of function.
 * The return statement before end of function is used for simpler code structure
 * and better readability.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 1.3, Taking address of near auto variable
 * The code is not dynamically linked. An absolute stack address is obtained when
 * taking the address of the near auto variable. A source of error in writing
 * dynamic code is that the stack segment may be different from the data segment.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, cast from unsigned char to pointer
 * The cast is needed for a function which has a generic parameter of type void*.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, conversion between a pointer and integer
 * type.
 * The cast is needed to obtain an address for a DMA call.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, cast from pointer to unsigned long
 * The cast is needed to obtain an address for a DMA call.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 2.2, Highest operation, function
 * 'FLEXCAN_UnlockRxMsgBuff', lacks side-effects.
 * The function is used to unlock the mailbox, which is done by reading the
 * free running timer.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External could be made static.
 * Function is defined for usage by application code.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.3, cast performed between a pointer to
 * object type and a pointer to a different object type
 * The cast is used for casting a bytes buffer into an words buffer in order to
 * optimize copying data to/from the message buffer.
 */

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "flexcan_hw_access.h"
#include "flexcan_irq.h"
#include "interrupt_manager.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define FLEXCAN_MB_HANDLE_RXFIFO    0U

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Table of base addresses for CAN instances. */
static CAN_Type * const g_flexcanBase[] = CAN_BASE_PTRS;

/* Tables to save CAN IRQ enum numbers defined in CMSIS header file. */
#if FEATURE_CAN_HAS_WAKE_UP_IRQ
static const IRQn_Type g_flexcanWakeUpIrqId[] = CAN_Wake_Up_IRQS;
#endif
static const IRQn_Type g_flexcanErrorIrqId[] = CAN_Error_IRQS;
static const IRQn_Type g_flexcanBusOffIrqId[] = CAN_Bus_Off_IRQS;
static const IRQn_Type g_flexcanOredMessageBufferIrqId[FEATURE_CAN_MB_IRQS_MAX_COUNT][CAN_INSTANCE_COUNT]
    = FEATURE_CAN_MB_IRQS;

/* Pointer to runtime state structure.*/
static flexcan_state_t * g_flexcanStatePtr[CAN_INSTANCE_COUNT] = { NULL };

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
static status_t FLEXCAN_StartSendData(
                    uint8_t instance,
                    uint8_t mb_idx,
                    const flexcan_data_info_t *tx_info,
                    uint32_t msg_id,
                    const uint8_t *mb_data,
                    bool isBlocking
                    );
static status_t FLEXCAN_StartRxMessageBufferData(
                    uint8_t instance,
                    uint8_t mb_idx,
                    flexcan_msgbuff_t *data,
                    bool isBlocking
                    );
static status_t FLEXCAN_StartRxMessageFifoData(
                    uint8_t instance,
                    flexcan_msgbuff_t *data,
                    bool isBlocking
                    );
static void FLEXCAN_CompleteTransfer(uint8_t instance, uint32_t mb_idx);
static void FLEXCAN_CompleteRxMessageFifoData(uint8_t instance);
#if FEATURE_CAN_HAS_DMA_ENABLE
static void FLEXCAN_CompleteRxFifoDataDMA(void *parameter, edma_chn_status_t status);
#endif
										
extern void FLEXCAN_ReadRxFifo2(const CAN_Type * base, flexcan_msgbuff_t *rxFifo);
										
/*******************************************************************************
 * Code
 ******************************************************************************/

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_SetBitrate
 * Description   : Set FlexCAN baudrate.
 * This function will set up all the time segment values for classical frames or the
 * extended time segments for the arbitration phase of FD frames. Those time segment
 * values are passed in by the user and are based on the required baudrate.
 *
 * Implements    : FLEXCAN_DRV_SetBitrate_Activity
 *END**************************************************************************/
void FLEXCAN_DRV_SetBitrate(uint8_t instance, const flexcan_time_segment_t *bitrate)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);
    DEV_ASSERT(bitrate != NULL);

    CAN_Type * base = g_flexcanBase[instance];
    bool fdEnabled = FLEXCAN_IsFDEnabled(base);

    FLEXCAN_EnterFreezeMode(base);

    if (fdEnabled)
    {
        /* Set extended time segments*/
        FLEXCAN_SetExtendedTimeSegments(base, bitrate);
    }
    else
    {
        /* Set time segments*/
        FLEXCAN_SetTimeSegments(base, bitrate);
    }

    FLEXCAN_ExitFreezeMode(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_SetBitrateCbt
 * Description   : Set FlexCAN bitrate.
 * This function will set up all the time segment values for the data phase of
 * FD frames. Those time segment values are passed in by the user and are based
 * on the required baudrate.
 *
 * Implements    : FLEXCAN_DRV_SetBitrateCbt_Activity
 *END**************************************************************************/
void FLEXCAN_DRV_SetBitrateCbt(uint8_t instance, const flexcan_time_segment_t *bitrate)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);
    DEV_ASSERT(bitrate != NULL);

    CAN_Type * base = g_flexcanBase[instance];

    FLEXCAN_EnterFreezeMode(base);

    /* Set time segments*/
    FLEXCAN_SetFDTimeSegments(base, bitrate);

    FLEXCAN_ExitFreezeMode(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_GetBitrate
 * Description   : Get FlexCAN baudrate.
 * This function will be return the current bit rate settings for classical frames
 * or the arbitration phase of FD frames.
 *
 * Implements    : FLEXCAN_DRV_GetBitrate_Activity
 *END**************************************************************************/
void  FLEXCAN_DRV_GetBitrate(uint8_t instance, flexcan_time_segment_t *bitrate)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);
    DEV_ASSERT(bitrate != NULL);

    CAN_Type * base = g_flexcanBase[instance];

    FLEXCAN_EnterFreezeMode(base);

    /* Get the time segments*/
    FLEXCAN_GetTimeSegments(base, bitrate);

    FLEXCAN_ExitFreezeMode(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_GetBitrateFD
 * Description   : Get FlexCAN baudrate.
 * This function will be return the current bit rate settings for the data phase
 * of FD frames.
 *
 * Implements    : FLEXCAN_DRV_GetBitrateFD_Activity
 *END**************************************************************************/
void  FLEXCAN_DRV_GetBitrateFD(uint8_t instance, flexcan_time_segment_t *bitrate)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);
    DEV_ASSERT(bitrate != NULL);

    CAN_Type * base = g_flexcanBase[instance];

    FLEXCAN_EnterFreezeMode(base);

    /* Get the time segments*/
    FLEXCAN_GetFDTimeSegments(base, bitrate);

    FLEXCAN_ExitFreezeMode(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_SetMasktype
 * Description   : Set RX masking type.
 * This function will set RX masking type as RX global mask or RX individual
 * mask.
 *
 * Implements    : FLEXCAN_DRV_SetRxMaskType_Activity
 *END**************************************************************************/
void  FLEXCAN_DRV_SetRxMaskType(uint8_t instance, flexcan_rx_mask_type_t type)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type * base = g_flexcanBase[instance];

    FLEXCAN_EnterFreezeMode(base);

    FLEXCAN_SetRxMaskType(base, type);

    FLEXCAN_ExitFreezeMode(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_SetRxFifoGlobalMask
 * Description   : Set Rx FIFO global mask as the 11-bit standard mask or the
 * 29-bit extended mask.
 *
 * Implements    : FLEXCAN_DRV_SetRxFifoGlobalMask_Activity
 *END**************************************************************************/
void FLEXCAN_DRV_SetRxFifoGlobalMask(
    uint8_t instance,
    flexcan_msgbuff_id_type_t id_type,
    uint32_t mask)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type * base = g_flexcanBase[instance];

    FLEXCAN_EnterFreezeMode(base);

    if (id_type == FLEXCAN_MSG_ID_STD)
    {
        /* Set standard global mask for RX FIOF*/
        FLEXCAN_SetRxFifoGlobalStdMask(base, mask);
    }
    else if (id_type == FLEXCAN_MSG_ID_EXT)
    {
        /* Set extended global mask for RX FIFO*/
        FLEXCAN_SetRxFifoGlobalExtMask(base, mask);
    }
    else {
        /* Should not get here */
    }

    FLEXCAN_ExitFreezeMode(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_SetRxMbGlobalMask
 * Description   : Set Rx Message Buffer global mask as the 11-bit standard mask
 * or the 29-bit extended mask.
 *
 * Implements    : FLEXCAN_DRV_SetRxMbGlobalMask_Activity
 *END**************************************************************************/
void FLEXCAN_DRV_SetRxMbGlobalMask(
    uint8_t instance,
    flexcan_msgbuff_id_type_t id_type,
    uint32_t mask)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type * base = g_flexcanBase[instance];

    FLEXCAN_EnterFreezeMode(base);

    if (id_type == FLEXCAN_MSG_ID_STD)
    {
        /* Set standard global mask for RX MB*/
        FLEXCAN_SetRxMsgBuffGlobalStdMask(base, mask);
    }
    else if (id_type == FLEXCAN_MSG_ID_EXT)
    {
        /* Set extended global mask for RX MB*/
        FLEXCAN_SetRxMsgBuffGlobalExtMask(base, mask);
    }
    else {
        /* Should not get here */
    }

    FLEXCAN_ExitFreezeMode(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_SetRxMb14Mask
 * Description   : Set Rx Message Buffer 14 mask as the 11-bit standard mask
 * or the 29-bit extended mask.
 *
 * Implements    : FLEXCAN_DRV_SetRxMb14Mask_Activity
 *END**************************************************************************/
void FLEXCAN_DRV_SetRxMb14Mask(
    uint8_t instance,
    flexcan_msgbuff_id_type_t id_type,
    uint32_t mask)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type * base = g_flexcanBase[instance];

    FLEXCAN_EnterFreezeMode(base);

    if (id_type == FLEXCAN_MSG_ID_STD)
    {
        /* Set standard global mask for RX MB*/
        FLEXCAN_SetRxMsgBuff14StdMask(base, mask);
    }
    else if (id_type == FLEXCAN_MSG_ID_EXT)
    {
        /* Set extended global mask for RX MB*/
        FLEXCAN_SetRxMsgBuff14ExtMask(base, mask);
    }
    else {
        /* Should not get here */
    }

    FLEXCAN_ExitFreezeMode(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_SetRxMb15Mask
 * Description   : Set Rx Message Buffer 15 mask as the 11-bit standard mask
 * or the 29-bit extended mask.
 *
 * Implements    : FLEXCAN_DRV_SetRxMb15Mask_Activity
 *END**************************************************************************/
void FLEXCAN_DRV_SetRxMb15Mask(
    uint8_t instance,
    flexcan_msgbuff_id_type_t id_type,
    uint32_t mask)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type * base = g_flexcanBase[instance];

    FLEXCAN_EnterFreezeMode(base);

    if (id_type == FLEXCAN_MSG_ID_STD)
    {
        /* Set standard global mask for RX MB*/
        FLEXCAN_SetRxMsgBuff15StdMask(base, mask);
    }
    else if (id_type == FLEXCAN_MSG_ID_EXT)
    {
        /* Set extended global mask for RX MB*/
        FLEXCAN_SetRxMsgBuff15ExtMask(base, mask);
    }
    else {
        /* Should not get here */
    }

    FLEXCAN_ExitFreezeMode(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_SetRxIndividualMask
 * Description   : Set Rx individual mask as the 11-bit standard mask or the
 * 29-bit extended mask.
 *
 * Implements    : FLEXCAN_DRV_SetRxIndividualMask_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_SetRxIndividualMask(
    uint8_t instance,
    flexcan_msgbuff_id_type_t id_type,
    uint8_t mb_idx,
    uint32_t mask)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type * base = g_flexcanBase[instance];

    FLEXCAN_EnterFreezeMode(base);

    if (mb_idx >= FLEXCAN_GetMaxMsgBuffNum(base))
    {
        FLEXCAN_ExitFreezeMode(base);
        return STATUS_CAN_BUFF_OUT_OF_RANGE;
    }

    if (id_type == FLEXCAN_MSG_ID_STD)
    {
        /* Set standard individual mask*/
        FLEXCAN_SetRxIndividualStdMask(base, mb_idx, mask);
    }
    else if (id_type == FLEXCAN_MSG_ID_EXT)
    {
        /* Set extended individual mask*/
        FLEXCAN_SetRxIndividualExtMask(base, mb_idx, mask);
    }
    else {
        /* Should not get here */
    }

    FLEXCAN_ExitFreezeMode(base);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_Init
 * Description   : Initialize FlexCAN driver.
 * This function will select a source clock, reset FlexCAN module, set maximum
 * number of message buffers, initialize all message buffers as inactive, enable
 * RX FIFO if needed, mask all mask bits, disable all MB interrupts, enable
 * FlexCAN normal mode, and enable all the error interrupts if needed.
 *
 * Implements    : FLEXCAN_DRV_Init_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_Init(
   uint8_t instance,
   flexcan_state_t *state,
   const flexcan_user_config_t *data)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);
    DEV_ASSERT(state != NULL);

    status_t result;
    CAN_Type * base = g_flexcanBase[instance];
    flexcan_time_segment_t bitrate;
    status_t osifStat;
    uint32_t i, j;
	  
	  if(instance == 0)
		{
		PCC->PCCn[PCC_FlexCAN0_INDEX] |= PCC_PCCn_CGC_MASK; /* CGC=1: enable clock to FlexCAN0 */
		}
		else if(instance == 1)
		{
		PCC->PCCn[PCC_FlexCAN1_INDEX] |= PCC_PCCn_CGC_MASK; /* CGC=1: enable clock to FlexCAN0 */	
		}
    FLEXCAN_Disable(base);

#if FEATURE_CAN_HAS_PE_CLKSRC_SELECT
    /* Select a source clock for the FlexCAN engine */
    FLEXCAN_SelectClock(base, data->pe_clock);
#endif

    /* Enable the CAN clock */
    FLEXCAN_Enable(base);

    FLEXCAN_EnterFreezeMode(base);

    /* Initialize FLEXCAN device */
    FLEXCAN_Init(base);

    /* Enable/Disable FD and check FD was set as expected. Setting FD as enabled
     * might fail if the current CAN instance does not support FD. */
    FLEXCAN_SetFDEnabled(base, data->fd_enable);
    if (FLEXCAN_IsFDEnabled(base) != data->fd_enable)
    {
        return STATUS_ERROR;
    }

    /* If the FD feature is enabled, enable the Stuff Bit Count, in order to be
     * ISO-compliant. */
    FLEXCAN_SetStuffBitCount(base, data->fd_enable);

    /* Disable the self reception feature if FlexCAN is not in loopback mode. */
    if (data->flexcanMode != FLEXCAN_LOOPBACK_MODE)
    {
        FLEXCAN_SetSelfReception(base, false);
    }

    /* Enable RxFIFO feature, if requested. This might fail if the FD mode is
     * enabled. */
    if (data->is_rx_fifo_needed)
    {
        result = FLEXCAN_EnableRxFifo(base, (uint32_t)data->num_id_filters);
        if (result != STATUS_SUCCESS)
        {
            return result;
        }
    }

#if FEATURE_CAN_HAS_DMA_ENABLE
    /* Enable DMA support for RxFIFO transfer, if requested. */
    if (data->transfer_type == FLEXCAN_RXFIFO_USING_DMA)
    {
        if (FLEXCAN_IsRxFifoEnabled(base))
        {
            FLEXCAN_SetRxFifoDMA(base, true);
        }
        else
        {
            return STATUS_ERROR;
        }
    }
#endif

    /* Select mode */
    FLEXCAN_SetOperationMode(base, data->flexcanMode);

    /* Set payload size. */
    FLEXCAN_SetPayloadSize(base, data->payload);

    result = FLEXCAN_SetMaxMsgBuffNum(base, data->max_num_mb);
    if (result != STATUS_SUCCESS)
    {
        return result;
    }

    /* Set bit rate. */
    if (FLEXCAN_IsFDEnabled(base))
    {
        bitrate = data->bitrate;
        FLEXCAN_SetExtendedTimeSegments(base, &bitrate);
        bitrate = data->bitrate_cbt;
        FLEXCAN_SetFDTimeSegments(base, &bitrate);
    }
    else
    {
        bitrate = data->bitrate;
        FLEXCAN_SetTimeSegments(base, &bitrate);
    }
  //  base->CTRL1|= CAN_CTRL1_BOFFREC_MASK;//Automatic recovering from Bus Off state disabled
    FLEXCAN_ExitFreezeMode(base);

    /* Enable FlexCAN interrupts.*/
#if FEATURE_CAN_HAS_WAKE_UP_IRQ
    if (g_flexcanWakeUpIrqId[instance] != NotAvail_IRQn)
    {
        INT_SYS_EnableIRQ(g_flexcanWakeUpIrqId[instance]);
    }
#endif
    INT_SYS_EnableIRQ(g_flexcanErrorIrqId[instance]);
    INT_SYS_EnableIRQ(g_flexcanBusOffIrqId[instance]);
    for (i = 0; i < FEATURE_CAN_MB_IRQS_MAX_COUNT; i++)
    {
        if (g_flexcanOredMessageBufferIrqId[i][instance] != NotAvail_IRQn)
        {
            INT_SYS_EnableIRQ(g_flexcanOredMessageBufferIrqId[i][instance]);
        }
    }

    for (i = 0; i < FEATURE_CAN_MAX_MB_NUM; i++)
    {
        osifStat = OSIF_SemaCreate(&state->mbs[i].mbSema, 0U);
        if (osifStat != STATUS_SUCCESS)
        {
            for (j = 0; j < i; j++)
            {
                (void)OSIF_SemaDestroy(&state->mbs[j].mbSema);
            }
            return STATUS_ERROR;
        }
        state->mbs[i].isBlocking = false;
        state->mbs[i].mb_message = NULL;
        state->mbs[i].state = FLEXCAN_MB_IDLE;
    }

    /* Store transfer type and DMA channel number used in transfer */
    state->transferType = data->transfer_type;
#if FEATURE_CAN_HAS_DMA_ENABLE
    state->rxFifoDMAChannel = data->rxFifoDMAChannel;
#endif

    /* Save runtime structure pointers so irq handler can point to the correct state structure */
    g_flexcanStatePtr[instance] = state;
//			FLEXCAN_SetErrIntCmd(base,FLEXCAN_INT_ERR,true);
		//	FLEXCAN_SetErrIntCmd(base,FLEXCAN_INT_BUSOFF,true);
   
    return (STATUS_SUCCESS);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_ConfigTxMb
 * Description   : Configure a Tx message buffer.
 * This function will first check if RX FIFO is enabled. If RX FIFO is enabled,
 * the function will make sure if the MB requested is not occupied by RX FIFO
 * and ID filter table. Then this function will set up the message buffer fields,
 * configure the message buffer code for Tx buffer as INACTIVE, and enable the
 * Message Buffer interrupt.
 *
 * Implements    : FLEXCAN_DRV_ConfigTxMb_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_ConfigTxMb(
    uint8_t instance,
    uint8_t mb_idx,
    const flexcan_data_info_t *tx_info,
    uint32_t msg_id)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);
    DEV_ASSERT(tx_info != NULL);

    flexcan_msgbuff_code_status_t cs;
    CAN_Type * base = g_flexcanBase[instance];

    /* Initialize transmit mb*/
    cs.dataLen = tx_info->data_length;
    cs.msgIdType = tx_info->msg_id_type;
    if (tx_info->is_remote)
    {
        cs.code = (uint32_t)FLEXCAN_TX_REMOTE;
    }
    else
    {
        cs.code = (uint32_t)FLEXCAN_TX_INACTIVE;
    }
    return FLEXCAN_SetTxMsgBuff(base, mb_idx, &cs, msg_id, NULL);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_SendBlocking
 * Description   : This function sends a CAN frame using a configured message
 * buffer. The function blocks until either the frame was sent, or the specified
 * timeout expired.
 *
 * Implements    : FLEXCAN_DRV_SendBlocking_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_SendBlocking(
    uint8_t instance,
    uint8_t mb_idx,
    const flexcan_data_info_t *tx_info,
    uint32_t msg_id,
    const uint8_t *mb_data,
    uint32_t timeout_ms)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);
    DEV_ASSERT(tx_info != NULL);

    status_t result;
    flexcan_state_t * state = g_flexcanStatePtr[instance];
    CAN_Type * base  = g_flexcanBase[instance];

    result = FLEXCAN_StartSendData(instance, mb_idx, tx_info, msg_id, mb_data, true);

    if (result == STATUS_SUCCESS)
    {
        status_t status;

        /* Enable message buffer interrupt*/
        (void)FLEXCAN_SetMsgBuffIntCmd(base, mb_idx, true);
        /* Enable error interrupts */
        FLEXCAN_SetErrIntCmd(base,FLEXCAN_INT_ERR,true);

        status = OSIF_SemaWait(&state->mbs[mb_idx].mbSema, timeout_ms);

        if (status == STATUS_TIMEOUT)
        {
            /* Disable message buffer interrupt */
            (void)FLEXCAN_SetMsgBuffIntCmd(base, mb_idx, false);
            /* Disable error interrupts */
            FLEXCAN_SetErrIntCmd(base,FLEXCAN_INT_ERR,false);

            result = STATUS_TIMEOUT;
        }

        state->mbs[mb_idx].state = FLEXCAN_MB_IDLE;
    }

    return result;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_Send
 * Description   : This function sends a CAN frame using a configured message
 * buffer. The function returns immediately. If a callback is installed, it will
 * be invoked after the frame was sent.
 *
 * Implements    : FLEXCAN_DRV_Send_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_Send(
    uint8_t instance,
    uint8_t mb_idx,
    const flexcan_data_info_t *tx_info,
    uint32_t msg_id,
    const uint8_t *mb_data)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);
    DEV_ASSERT(tx_info != NULL);

    status_t result;
   // CAN_Type * base = g_flexcanBase[instance];

    result = FLEXCAN_StartSendData(instance, mb_idx, tx_info, msg_id, mb_data, false);
   // if(result == STATUS_SUCCESS)
    {
        /* Enable message buffer interrupt*/
    //    result = FLEXCAN_SetMsgBuffIntCmd(base, mb_idx, true);
		//	printf("\r\nCAN0->IMASK1=%X",CAN0->IMASK1);
        /* Enable error interrupts */
     }

    return result;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_ConfigMb
 * Description   : Configure a Rx message buffer.
 * This function will first check if RX FIFO is enabled. If RX FIFO is enabled,
 * the function will make sure if the MB requested is not occupied by RX FIFO
 * and ID filter table. Then this function will set up the message buffer fields,
 * configure the message buffer code for Rx message buffer as NOT_USED, enable
 * the Message Buffer interrupt, configure the message buffer code for Rx
 * message buffer as INACTIVE, copy user's buffer into the message buffer data
 * area, and configure the message buffer code for Rx message buffer as EMPTY.
 *
 * Implements    : FLEXCAN_DRV_ConfigRxMb_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_ConfigRxMb(
    uint8_t instance,
    uint8_t mb_idx,
    const flexcan_data_info_t *rx_info,
    uint32_t msg_id)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);
    DEV_ASSERT(rx_info != NULL);

    status_t result;
    flexcan_msgbuff_code_status_t cs;
    CAN_Type * base = g_flexcanBase[instance];

    cs.dataLen = rx_info->data_length;
    cs.msgIdType = rx_info->msg_id_type;
    cs.fd_enable = rx_info->fd_enable;
    /* Initialize rx mb*/
    cs.code = (uint32_t)FLEXCAN_RX_NOT_USED;
    result = FLEXCAN_SetRxMsgBuff(base, mb_idx, &cs, msg_id);
    if (result != STATUS_SUCCESS)
    {
         return result;
    }

    /* Initialize receive MB*/
    cs.code = (uint32_t)FLEXCAN_RX_INACTIVE;
    result = FLEXCAN_SetRxMsgBuff(base, mb_idx, &cs, msg_id);
    if (result != STATUS_SUCCESS)
    {
         return result;
    }

    /* Set up FlexCAN message buffer fields for receiving data*/
    cs.code = (uint32_t)FLEXCAN_RX_EMPTY;
    return FLEXCAN_SetRxMsgBuff(base, mb_idx, &cs, msg_id);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_ConfigRxFifo
 * Description   : Confgure RX FIFO ID filter table elements.
 * This function will confgure RX FIFO ID filter table elements, and enable RX
 * FIFO interrupts.
 *
 * Implements    : FLEXCAN_DRV_ConfigRxFifo_Activity
 *END**************************************************************************/
void FLEXCAN_DRV_ConfigRxFifo(
    uint8_t instance,
    flexcan_rx_fifo_id_element_format_t id_format,
    const flexcan_id_table_t *id_filter_table)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type * base = g_flexcanBase[instance];

    FLEXCAN_EnterFreezeMode(base);

    /* Initialize rx fifo*/
    FLEXCAN_SetRxFifoFilter(base, id_format, id_filter_table);

    FLEXCAN_ExitFreezeMode(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_ReceiveBlocking
 * Description   : This function receives a CAN frame into a configured message
 * buffer. The function blocks until either a frame was received, or the
 * specified timeout expired.
 *
 * Implements    : FLEXCAN_DRV_ReceiveBlocking_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_ReceiveBlocking(
    uint8_t instance,
    uint8_t mb_idx,
    flexcan_msgbuff_t *data,
    uint32_t timeout_ms)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    status_t result;
    flexcan_state_t * state = g_flexcanStatePtr[instance];
    CAN_Type * base = g_flexcanBase[instance];

    result = FLEXCAN_StartRxMessageBufferData(instance, mb_idx, data, true);

    if(result == STATUS_SUCCESS)
    {
        status_t status;

        status = OSIF_SemaWait(&state->mbs[mb_idx].mbSema, timeout_ms);

        if (status == STATUS_SUCCESS)
        {
            result = FLEXCAN_GetMsgBuff(base, mb_idx, data);
        }
        else
        {
            /* Disable message buffer interrupt */
            (void)FLEXCAN_SetMsgBuffIntCmd(base, mb_idx, false);
            /* Disable error interrupts */
            FLEXCAN_SetErrIntCmd(base,FLEXCAN_INT_ERR,false);

            result = STATUS_TIMEOUT;
        }

        state->mbs[mb_idx].state = FLEXCAN_MB_IDLE;
    }

    return result;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_Receive
 * Description   : This function receives a CAN frame into a configured message
 * buffer. The function returns immediately. If a callback is installed, it will
 * be invoked after the frame was received and read into the specified buffer.
 *
 * Implements    : FLEXCAN_DRV_Receive_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_Receive(
    uint8_t instance,
    uint8_t mb_idx,
    flexcan_msgbuff_t *data)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    status_t result;

    result = FLEXCAN_StartRxMessageBufferData(instance, mb_idx, data, false);

    return result;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_RxFifoBlocking
 * Description   : This function receives a CAN frame using the Rx FIFO. The
 * function blocks until either a frame was received, or the specified timeout
 * expired.
 *
 * Implements    : FLEXCAN_DRV_RxFifoBlocking_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_RxFifoBlocking(
    uint8_t instance,
    flexcan_msgbuff_t *data,
    uint32_t timeout_ms)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    status_t result;
    flexcan_state_t * state = g_flexcanStatePtr[instance];
    CAN_Type * base = g_flexcanBase[instance];

    result = FLEXCAN_StartRxMessageFifoData(instance, data, true);

    if (result == STATUS_SUCCESS)
    {
        result = OSIF_SemaWait(&state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].mbSema, timeout_ms);

        if (result == STATUS_TIMEOUT)
        {
            /* Disable RX FIFO interrupts*/
            (void)FLEXCAN_SetMsgBuffIntCmd(base, FEATURE_CAN_RXFIFO_FRAME_AVAILABLE, false);
            (void)FLEXCAN_SetMsgBuffIntCmd(base, FEATURE_CAN_RXFIFO_WARNING, false);
            (void)FLEXCAN_SetMsgBuffIntCmd(base, FEATURE_CAN_RXFIFO_OVERFLOW, false);

            /* Disable error interrupts */
            FLEXCAN_SetErrIntCmd(base,FLEXCAN_INT_ERR,false);

            result = STATUS_TIMEOUT;
        }

        state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].state = FLEXCAN_MB_IDLE;
    }

    return result;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_RxFifo
 * Description   : This function receives a CAN frame using the Rx FIFO. The
 * function returns immediately. If a callback is installed, it will be invoked
 * after the frame was received and read into the specified buffer.
 *
 * Implements    : FLEXCAN_DRV_RxFifo_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_RxFifo(
    uint8_t instance,
    flexcan_msgbuff_t *data)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    status_t result;

    result = FLEXCAN_StartRxMessageFifoData(instance, data, false);

    return result;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_Deinit
 * Description   : Shutdown a FlexCAN module.
 * This function will disable all FlexCAN interrupts, and disable the FlexCAN.
 *
 * Implements    : FLEXCAN_DRV_Deinit_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_Deinit(uint8_t instance)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    const flexcan_state_t * state = g_flexcanStatePtr[instance];
    status_t result = STATUS_SUCCESS;
    status_t osifStat;
    uint32_t i;

    /* Disable FlexCAN interrupts.*/
#if FEATURE_CAN_HAS_WAKE_UP_IRQ
    if (g_flexcanWakeUpIrqId[instance] != NotAvail_IRQn)
    {
        INT_SYS_DisableIRQ(g_flexcanWakeUpIrqId[instance]);
    }
#endif
    INT_SYS_DisableIRQ(g_flexcanErrorIrqId[instance]);
    INT_SYS_DisableIRQ(g_flexcanBusOffIrqId[instance]);
    for (i = 0; i < FEATURE_CAN_MB_IRQS_MAX_COUNT; i++)
    {
        if (g_flexcanOredMessageBufferIrqId[i][instance] != NotAvail_IRQn)
        {
            INT_SYS_DisableIRQ(g_flexcanOredMessageBufferIrqId[i][instance]);
        }
    }

    /* Disable FlexCAN.*/
    FLEXCAN_Disable(g_flexcanBase[instance]);

    for (i = 0; i < FEATURE_CAN_MAX_MB_NUM; i++)
    {
        osifStat = OSIF_SemaDestroy(&state->mbs[i].mbSema);
        if (osifStat != STATUS_SUCCESS)
        {
            result = STATUS_ERROR;
        }
    }

    return result;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_SetTDCOffset
 * Description   : Enables/Disables the Transceiver Delay Compensation feature and sets
 * the Transceiver Delay Compensation Offset.
 *
 * Implements    : FLEXCAN_DRV_SetTDCOffset_Activity
 *END**************************************************************************/
void FLEXCAN_DRV_SetTDCOffset(uint8_t instance, bool enable, uint8_t offset)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type * base = g_flexcanBase[instance];

    FLEXCAN_EnterFreezeMode(base);

    /* Enable/Disable TDC and set the TDC Offset */
    FLEXCAN_SetTDCOffset(base, enable, offset);

    FLEXCAN_ExitFreezeMode(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_GetTDCValue
 * Description   : Gets the value of the Transceiver Delay Compensation.
 *
 * Implements    : FLEXCAN_DRV_GetTDCValue_Activity
 *END**************************************************************************/
uint8_t FLEXCAN_DRV_GetTDCValue(uint8_t instance)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    const CAN_Type * base = g_flexcanBase[instance];

    return (uint8_t)((base->FDCTRL & CAN_FDCTRL_TDCVAL_MASK) >> CAN_FDCTRL_TDCVAL_SHIFT);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_GetTDCFail
 * Description   : Gets the value of the TDC Fail flag.
 *
 * Implements    : FLEXCAN_DRV_GetTDCFail_Activity
 *END**************************************************************************/
bool FLEXCAN_DRV_GetTDCFail(uint8_t instance)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    const CAN_Type * base = g_flexcanBase[instance];

    return (((base->FDCTRL & CAN_FDCTRL_TDCFAIL_MASK) >> CAN_FDCTRL_TDCFAIL_SHIFT) != 0U);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_ClearTDCFail
 * Description   : Clears the TDC Fail flag.
 *
 * Implements    : FLEXCAN_DRV_ClearTDCFail_Activity
 *END**************************************************************************/
void FLEXCAN_DRV_ClearTDCFail(uint8_t instance)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type * base = g_flexcanBase[instance];

    base->FDCTRL = base->FDCTRL | CAN_FDCTRL_TDCFAIL_MASK;
}
void CAN0_Error_IRQHandler(void)
{

	printf("\r\nCAN0_Error_IRQHandler");
//		if(CAN0->ESR1&0x4)
//	{
//		 FLEXCAN_Disable(CAN0);
//		FLEXCAN_SetErrIntCmd(CAN0,FLEXCAN_INT_BUSOFF,false);
//	}
	//printf("\r\nCAN0->ESR1=%x",CAN0->ESR1);
	//CAN0->ESR1=0x2;

	//FLEXCAN_ClearErrIntStatusFlag(CAN0);


   // FLEXCAN_IRQHandler(0U);
}

void CAN0_ORed_0_15_MB_IRQHandler2(void)
{
	//printf("\r\nCAN0->IMASK1=%x",CAN0->IMASK1);
//	printf("\r\n1 CAN0->IFLAG1=%x",CAN0->IFLAG1);
	if((CAN0->IFLAG1&0x20))
	{
		flexcan_state_t * state = g_flexcanStatePtr[0];
		 /* Get RX FIFO field values */
		
                FLEXCAN_ReadRxFifo2(CAN0, state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].mb_message);
							//	CAN0->IFLAG1&=0xe0;
                /* Invoke callback */
                if (state->callback != NULL)
                {
								
                 //   state->callback(0,FLEXCAN_EVENT_RXFIFO_COMPLETE,FLEXCAN_MB_HANDLE_RXFIFO,state);
                }
	}
	//printf("\r\n2 CAN0->IFLAG1=%x",CAN0->IFLAG1);
		
}
void CAN0_ORed_0_15_MB_IRQHandler(void)
{
	//printf("\r\nCAN0->IMASK1=%x",CAN0->IMASK1);
//	printf("\r\n1 CAN0->IFLAG1=%x",CAN0->IFLAG1);
	if((CAN0->IFLAG1&0x20))
	{
		flexcan_state_t * state = g_flexcanStatePtr[0];
		 /* Get RX FIFO field values */
                FLEXCAN_ReadRxFifo(CAN0, state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].mb_message);
								CAN0->IFLAG1&=0xe0;
                /* Invoke callback */
                if (state->callback != NULL)
                {
								
                    state->callback(0,FLEXCAN_EVENT_RXFIFO_COMPLETE,FLEXCAN_MB_HANDLE_RXFIFO,state);
                }
	}
	//printf("\r\n2 CAN0->IFLAG1=%x",CAN0->IFLAG1);
		
}
/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_IRQHandler
 * Description   : Interrupt handler for FLEXCAN.
 * This handler read data from MB or FIFO, and then clear the interrupt flags.
 * This is not a public API as it is called whenever an interrupt occurs.
 *
 *END**************************************************************************/
void FLEXCAN_IRQHandler(uint8_t instance)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    uint32_t flag_reg = 0;
    CAN_Type * base = g_flexcanBase[instance];
    flexcan_state_t * state = g_flexcanStatePtr[instance];
//    status_t result = STATUS_SUCCESS;

    /* Get the interrupts that are enabled and ready */
    uint32_t mb_idx = 0;
    flag_reg = FLEXCAN_GetMsgBuffIntStatusFlag(base, mb_idx);
//	printf("\r\nCAN0->ESR1=%x",CAN0->ESR1);
//	printf("\r\nCAN0->IFLAG1=%x",CAN0->IFLAG1);
//	printf("\r\nCAN0->IMASK1=%x",CAN0->IMASK1);
	if(CAN0->ESR1&0x4)
	{
		CAN0->ESR1=0x4;
		state->busoff=true;
	//	FLEXCAN_Bus_Off_isRecovery(base,1);
	}
		

		
	

	
//		if(CAN0->ESR1&0x4)
//	{
//		 FLEXCAN_Disable(CAN0);
//		FLEXCAN_SetErrIntCmd(base,FLEXCAN_INT_BUSOFF,false);
//	}
	
   
	//CAN0->ESR1=0x4;
//    while ((flag_reg & 1U) == 0U)
//    {
//        mb_idx++;
//        flag_reg = FLEXCAN_GetMsgBuffIntStatusFlag(base, mb_idx);

//        if (mb_idx > FEATURE_CAN_MAX_MB_NUM)
//        {
//            break;
//        }
//    }

//    /* Check Tx/Rx interrupt flag and clear the interrupt */
//    if(flag_reg != 0U)
//    {
//        bool rxfifoEnabled = FLEXCAN_IsRxFifoEnabled(base);
//        if ((mb_idx == FEATURE_CAN_RXFIFO_FRAME_AVAILABLE) && rxfifoEnabled)
//        {
//            if (state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].state == FLEXCAN_MB_RX_BUSY)
//            {
//                /* Get RX FIFO field values */
//                FLEXCAN_ReadRxFifo(base, state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].mb_message);

//                /* Complete receive data */
//                FLEXCAN_CompleteRxMessageFifoData(instance);
//                FLEXCAN_ClearMsgBuffIntStatusFlag(base, mb_idx);

//                /* Invoke callback */
//                if (state->callback != NULL)
//                {
//                    state->callback(instance,
//                    		        FLEXCAN_EVENT_RXFIFO_COMPLETE,
//									FLEXCAN_MB_HANDLE_RXFIFO,
//									state);
//                }
//            }
//        }
//        else
//        {
//            /* Check mailbox completed reception */
//            if (state->mbs[mb_idx].state == FLEXCAN_MB_RX_BUSY)
//            {
//                /* Lock RX message buffer and RX FIFO*/
//                result = FLEXCAN_LockRxMsgBuff(base, mb_idx);
//                if (result == STATUS_SUCCESS)
//                {
//                    /* Get RX MB field values*/
//                    result = FLEXCAN_GetMsgBuff(base, mb_idx, state->mbs[mb_idx].mb_message);
//                }
//                if (result == STATUS_SUCCESS)
//                {
//                    /* Unlock RX message buffer and RX FIFO*/
//                    FLEXCAN_UnlockRxMsgBuff(base);

//                    /* Complete receive data */
//                   // FLEXCAN_CompleteTransfer(instance, mb_idx);
//                    FLEXCAN_ClearMsgBuffIntStatusFlag(base, mb_idx);

//                    /* Invoke callback */
//                    if (state->callback != NULL)
//                    {
//                        state->callback(instance, FLEXCAN_EVENT_RX_COMPLETE, mb_idx, state);
//                    }
//                }
//            }
//        }

//        /* Check mailbox completed transmission */
//        if (state->mbs[mb_idx].state == FLEXCAN_MB_TX_BUSY)
//        {
//            /* Complete transmit data */
//            FLEXCAN_CompleteTransfer(instance, mb_idx);

//            if (state->mbs[mb_idx].isRemote)
//            {
//                /* If the frame was a remote frame, clear the flag only if the response was
//                 * not received yet. If the response was received, leave the flag set in order
//                 * to be handled when the user calls FLEXCAN_DRV_RxMessageBuffer. */
//                flexcan_msgbuff_t mb;
//                (void) FLEXCAN_LockRxMsgBuff(base, mb_idx);
//                (void) FLEXCAN_GetMsgBuff(base, mb_idx, &mb);
//                FLEXCAN_UnlockRxMsgBuff(base);

//                if (((mb.cs & CAN_CS_CODE_MASK) >> CAN_CS_CODE_SHIFT) == (uint32_t)FLEXCAN_RX_EMPTY)
//                {
//                    FLEXCAN_ClearMsgBuffIntStatusFlag(base, mb_idx);
//                }
//            }
//            else
//            {
//                FLEXCAN_ClearMsgBuffIntStatusFlag(base, mb_idx);
//            }

//            /* Invoke callback */
//            if (state->callback != NULL)
//            {
//                state->callback(instance, FLEXCAN_EVENT_TX_COMPLETE, mb_idx, state);
//            }
//        }
//    }

    /* Clear all other interrupts in ERRSTAT register (Error, Busoff, Wakeup) */
  //  FLEXCAN_ClearErrIntStatusFlag(base);
	//printf("\r\n1 CAN0->ESR1=%X",CAN0->ESR1);
	//	CAN0->ESR1|=0X1<<21;
			//printf("\r\n2 CAN0->ESR1=%X",CAN0->ESR1);
    return;
}

#if FEATURE_CAN_HAS_WAKE_UP_IRQ

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_WakeUpHandler
 * Description   : Wake up handler for FLEXCAN.
 * This handler verifies the event which caused the wake up and invokes the
 * user callback, if configured.
 * This is not a public API as it is called whenever an wake up event occurs.
 *
 *END**************************************************************************/
void FLEXCAN_WakeUpHandler(uint8_t instance)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type * base = g_flexcanBase[instance];
    flexcan_state_t * state = g_flexcanStatePtr[instance];

    /* Invoke callback */
    if (state->callback != NULL)
    {
#if FEATURE_CAN_HAS_PRETENDED_NETWORKING
        if (FLEXCAN_IsPNEnabled(base))
        {
            if (FLEXCAN_GetWTOF(base) != 0U)
            {
                FLEXCAN_ClearWTOF(base);
                state->callback(instance, FLEXCAN_EVENT_WAKEUP_TIMEOUT, 0U, state);
            }
            if (FLEXCAN_GetWUMF(base) != 0U)
            {
                FLEXCAN_ClearWUMF(base);
                state->callback(instance, FLEXCAN_EVENT_WAKEUP_MATCH, 0U, state);
            }
        }
#endif

#if FEATURE_CAN_HAS_SELF_WAKE_UP
        if (FLEXCAN_IsSelfWakeUpEnabled(base))
        {
            if (FLEXCAN_GetWAKINT(base) != 0U)
            {
                FLEXCAN_ClearWAKINT(base);
                state->callback(instance, FLEXCAN_EVENT_SELF_WAKEUP, 0U, state);
            }
        }
#endif
    }
}
#endif /* FEATURE_CAN_HAS_WAKE_UP_IRQ */

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_GetTransferStatus
 * Description   : This function returns whether the previous FLEXCAN receive is
 *                 completed.
 * When performing a non-blocking receive, the user can call this function to
 * ascertain the state of the current receive progress: in progress (or busy)
 * or complete (success).
 *
 * Implements    : FLEXCAN_DRV_GetTransferStatus_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_GetTransferStatus(uint8_t instance, uint8_t mb_idx)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    const flexcan_state_t * state = g_flexcanStatePtr[instance];
    status_t status;

    if (state->mbs[mb_idx].state == FLEXCAN_MB_IDLE)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_BUSY;
    }

    return status;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_AbortTransfer
 * Description   : This function shuts down the FLEXCAN by disabling interrupts and
 *                 the transmitter/receiver.
 * This function disables the FLEXCAN interrupts, disables the transmitter and
 * receiver.
 *
 * Implements    : FLEXCAN_DRV_AbortTransfer_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_AbortTransfer(uint8_t instance, uint8_t mb_idx)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    const flexcan_state_t * state = g_flexcanStatePtr[instance];

    /* Check if a transfer is running. */
    if (state->mbs[mb_idx].state == FLEXCAN_MB_IDLE)
    {
        return STATUS_CAN_NO_TRANSFER_IN_PROGRESS;
    }

    /* Stop the running transfer. */
    FLEXCAN_CompleteTransfer(instance, mb_idx);

    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_StartSendData
 * Description   : Initiate (start) a transmit by beginning the process of
 * sending data.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static status_t FLEXCAN_StartSendData(
                    uint8_t instance,
                    uint8_t mb_idx,
                    const flexcan_data_info_t *tx_info,
                    uint32_t msg_id,
                    const uint8_t *mb_data,
                    bool isBlocking
                    )
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);
    DEV_ASSERT(tx_info != NULL);

    status_t result;
    flexcan_msgbuff_code_status_t cs;
    flexcan_state_t * state = g_flexcanStatePtr[instance];
    CAN_Type * base = g_flexcanBase[instance];

    if (state->mbs[mb_idx].state != FLEXCAN_MB_IDLE)
    {
        //return STATUS_BUSY;
    }
    state->mbs[mb_idx].state = FLEXCAN_MB_TX_BUSY;
    state->mbs[mb_idx].isBlocking = isBlocking;
    state->mbs[mb_idx].isRemote = tx_info->is_remote;

    cs.dataLen = tx_info->data_length;
    cs.msgIdType = tx_info->msg_id_type;

    cs.fd_enable = tx_info->fd_enable;
    cs.fd_padding = tx_info->fd_padding;
    cs.enable_brs = tx_info->enable_brs;
    if (tx_info->is_remote)
    {
        cs.code = (uint32_t)FLEXCAN_TX_REMOTE;
    }
    else
    {
        cs.code = (uint32_t)FLEXCAN_TX_DATA;
    }
    result = FLEXCAN_SetTxMsgBuff(base, mb_idx, &cs, msg_id, mb_data);

    if (result != STATUS_SUCCESS)
    {
        state->mbs[mb_idx].state = FLEXCAN_MB_IDLE;
    }

    return result;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_StartRxMessageBufferData
 * Description   : Initiate (start) a receive by beginning the process of
 * receiving data and enabling the interrupt.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static status_t FLEXCAN_StartRxMessageBufferData(
                    uint8_t instance,
                    uint8_t mb_idx,
                    flexcan_msgbuff_t *data,
                    bool isBlocking
                    )
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    status_t result = STATUS_SUCCESS;
    CAN_Type * base = g_flexcanBase[instance];
    flexcan_state_t * state = g_flexcanStatePtr[instance];

    /* Start receiving mailbox */
    if(state->mbs[mb_idx].state != FLEXCAN_MB_IDLE)
    {
        return STATUS_BUSY;
    }
    state->mbs[mb_idx].state = FLEXCAN_MB_RX_BUSY;
    state->mbs[mb_idx].mb_message = data;
    state->mbs[mb_idx].isBlocking = isBlocking;

    /* Enable MB interrupt*/
    result = FLEXCAN_SetMsgBuffIntCmd(base, mb_idx, true);
    /* Enable error interrupts */
   // FLEXCAN_SetErrIntCmd(base,FLEXCAN_INT_ERR,true);

    if (result != STATUS_SUCCESS)
    {
        state->mbs[mb_idx].state = FLEXCAN_MB_IDLE;
    }

    return result;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_StartRxMessageFifoData
 * Description   : Initiate (start) a receive by beginning the process of
 * receiving data and enabling the interrupt.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static status_t FLEXCAN_StartRxMessageFifoData(
                    uint8_t instance,
                    flexcan_msgbuff_t *data,
                    bool isBlocking
                    )
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type * base = g_flexcanBase[instance];
    flexcan_state_t * state = g_flexcanStatePtr[instance];
#if FEATURE_CAN_HAS_DMA_ENABLE
    status_t edmaStat;
#endif

    /* Start receiving fifo */
    if(state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].state != FLEXCAN_MB_IDLE)
    {
        return STATUS_BUSY;
    }
    /* Check if RxFIFO feature is enabled */
    if (!FLEXCAN_IsRxFifoEnabled(base))
    {
        return STATUS_ERROR;
    }

    state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].state = FLEXCAN_MB_RX_BUSY;

    state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].isBlocking = isBlocking;

    /* This will get filled by the interrupt handler */
    state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].mb_message = data;

#if FEATURE_CAN_HAS_DMA_ENABLE
//    if (state->transferType == FLEXCAN_RXFIFO_USING_DMA)
//    {
//        status_t edmaStatus;

//        edmaStatus = EDMA_DRV_InstallCallback(state->rxFifoDMAChannel,
//                                              FLEXCAN_CompleteRxFifoDataDMA,
//                                              (void *)((uint32_t)instance));

//        if (edmaStatus != STATUS_SUCCESS)
//        {
//            state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].state = FLEXCAN_MB_IDLE;
//            return STATUS_ERROR;
//        }

//        edmaStatus = EDMA_DRV_ConfigSingleBlockTransfer(state->rxFifoDMAChannel,
//                                                        EDMA_TRANSFER_MEM2MEM,
//                                                        (uint32_t)(base->RAMn),
//                                                        (uint32_t)(state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].mb_message),
//                                                        EDMA_TRANSFER_SIZE_4B,
//                                                        16U);

//        if (edmaStatus != STATUS_SUCCESS)
//        {
//            state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].state = FLEXCAN_MB_IDLE;
//            return STATUS_ERROR;
//        }

//        edmaStat = EDMA_DRV_StartChannel(state->rxFifoDMAChannel);
//        if (edmaStat != STATUS_SUCCESS)
//        {
//            state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].state = FLEXCAN_MB_IDLE;
//            return STATUS_ERROR;
//        }
//    }
#endif

    /* Enable RX FIFO interrupts*/
    (void)FLEXCAN_SetMsgBuffIntCmd(base, FEATURE_CAN_RXFIFO_FRAME_AVAILABLE, true);
    (void)FLEXCAN_SetMsgBuffIntCmd(base, FEATURE_CAN_RXFIFO_WARNING, true);
    (void)FLEXCAN_SetMsgBuffIntCmd(base, FEATURE_CAN_RXFIFO_OVERFLOW, true);

    /* Enable error interrupts */
//    FLEXCAN_SetErrIntCmd(base,FLEXCAN_INT_ERR,true);
	//	printf("\r\nCAN0->IMASK1=%x",CAN0->IMASK1);
    return STATUS_SUCCESS;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_CompleteTransfer
 * Description   : Finish up a transmit by completing the process of sending
 * data and disabling the interrupt.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static void FLEXCAN_CompleteTransfer(uint8_t instance, uint32_t mb_idx)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type * base = g_flexcanBase[instance];
    flexcan_state_t * state = g_flexcanStatePtr[instance];

    /* Disable the transmitter data register empty interrupt */
    (void)FLEXCAN_SetMsgBuffIntCmd(base, mb_idx, false);
    /* Disable error interrupts */
    FLEXCAN_SetErrIntCmd(base,FLEXCAN_INT_ERR,false);

    /* Update the information of the module driver state */
    if (state->mbs[mb_idx].isBlocking)
    {
        (void)OSIF_SemaPost(&state->mbs[mb_idx].mbSema);
    }
    state->mbs[mb_idx].state = FLEXCAN_MB_IDLE;
}

#if FEATURE_CAN_HAS_DMA_ENABLE
/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_CompleteRxFifoDataDMA
 * Description   : Finish up a DMA transfer (this is just a wrapper over
 * FLEXCAN_DRV_CompleteRxMessageFifoData).
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static void FLEXCAN_CompleteRxFifoDataDMA(void *parameter, edma_chn_status_t status)
{
    uint32_t instance = (uint32_t)parameter;
    (void)status;

    FLEXCAN_CompleteRxMessageFifoData((uint8_t)instance);
}
#endif
 void FLEXCAN_CompleteRxMessageFifoData2()
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

  //  CAN_Type * base = g_flexcanBase[0];
    flexcan_state_t * state = g_flexcanStatePtr[0];

//    if (state->transferType == FLEXCAN_RXFIFO_USING_INTERRUPTS)
//    {
//        /* Disable RX FIFO interrupts*/
//       // (void)FLEXCAN_SetMsgBuffIntCmd(base, FEATURE_CAN_RXFIFO_FRAME_AVAILABLE, false);
//        //(void)FLEXCAN_SetMsgBuffIntCmd(base, FEATURE_CAN_RXFIFO_WARNING, false);
//        //(void)FLEXCAN_SetMsgBuffIntCmd(base, FEATURE_CAN_RXFIFO_OVERFLOW, false);
//		printf("\r\nCAN0->IMASK1=%x",CAN0->IMASK1);
//        /* Disable error interrupts */
//        //FLEXCAN_SetErrIntCmd(base,FLEXCAN_INT_ERR,false);
//    }
//#if FEATURE_CAN_HAS_DMA_ENABLE
//    else
    {
        flexcan_msgbuff_t *fifo_message = state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].mb_message;
        uint32_t *msgData_32 = (uint32_t *)fifo_message->data;

       // (void) EDMA_DRV_StopChannel(state->rxFifoDMAChannel);
        /* Adjust the ID if it is not extended */
        if (((fifo_message->cs) & CAN_CS_IDE_MASK) == 0U)
        {
            fifo_message->msgId = fifo_message->msgId  >> CAN_ID_STD_SHIFT;
        }
        /* Extract the data length */
        fifo_message->dataLen = (uint8_t)((fifo_message->cs & CAN_CS_DLC_MASK) >> CAN_CS_DLC_SHIFT);
        /* Reverse the endianness */
        FlexcanSwapBytesInWord(msgData_32[0], msgData_32[0]);
        FlexcanSwapBytesInWord(msgData_32[1], msgData_32[1]);
    }
//#endif
    /* Clear fifo message*/
    state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].mb_message = NULL;

    /* Update status for receive by using fifo*/
    //if (state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].isBlocking)
    {
   //     (void)OSIF_SemaPost(&state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].mbSema);
    }
    state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].state = FLEXCAN_MB_IDLE;
}
/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_CompleteRxMessageFifoData
 * Description   : Finish up a receive by completing the process of receiving
 * data and disabling the interrupt.
 * This is not a public API as it is called from other driver functions.
 *
 *END**************************************************************************/
static void FLEXCAN_CompleteRxMessageFifoData(uint8_t instance)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

//    CAN_Type * base = g_flexcanBase[instance];
    flexcan_state_t * state = g_flexcanStatePtr[instance];

//    if (state->transferType == FLEXCAN_RXFIFO_USING_INTERRUPTS)
//    {
//        /* Disable RX FIFO interrupts*/
//       // (void)FLEXCAN_SetMsgBuffIntCmd(base, FEATURE_CAN_RXFIFO_FRAME_AVAILABLE, false);
//        //(void)FLEXCAN_SetMsgBuffIntCmd(base, FEATURE_CAN_RXFIFO_WARNING, false);
//        //(void)FLEXCAN_SetMsgBuffIntCmd(base, FEATURE_CAN_RXFIFO_OVERFLOW, false);
//		printf("\r\nCAN0->IMASK1=%x",CAN0->IMASK1);
//        /* Disable error interrupts */
//        //FLEXCAN_SetErrIntCmd(base,FLEXCAN_INT_ERR,false);
//    }
//#if FEATURE_CAN_HAS_DMA_ENABLE
//    else
//    {
//        flexcan_msgbuff_t *fifo_message = state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].mb_message;
//        uint32_t *msgData_32 = (uint32_t *)fifo_message->data;

//       // (void) EDMA_DRV_StopChannel(state->rxFifoDMAChannel);
//        /* Adjust the ID if it is not extended */
//        if (((fifo_message->cs) & CAN_CS_IDE_MASK) == 0U)
//        {
//            fifo_message->msgId = fifo_message->msgId  >> CAN_ID_STD_SHIFT;
//        }
//        /* Extract the data length */
//        fifo_message->dataLen = (uint8_t)((fifo_message->cs & CAN_CS_DLC_MASK) >> CAN_CS_DLC_SHIFT);
//        /* Reverse the endianness */
//        FlexcanSwapBytesInWord(msgData_32[0], msgData_32[0]);
//        FlexcanSwapBytesInWord(msgData_32[1], msgData_32[1]);
//    }
//#endif
    /* Clear fifo message*/
    state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].mb_message = NULL;

    /* Update status for receive by using fifo*/
    if (state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].isBlocking)
    {
        (void)OSIF_SemaPost(&state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].mbSema);
    }
    state->mbs[FLEXCAN_MB_HANDLE_RXFIFO].state = FLEXCAN_MB_IDLE;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_InstallEventCallback
 * Description   : Installs a callback function for the IRQ handler.
 *
 * Implements    : FLEXCAN_DRV_InstallEventCallback_Activity
 *END**************************************************************************/
void FLEXCAN_DRV_InstallEventCallback(uint8_t instance,
                                      flexcan_callback_t callback,
                                      void *callbackParam)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    flexcan_state_t * state = g_flexcanStatePtr[instance];

    state->callback = callback;
    state->callbackParam = callbackParam;
}

#if FEATURE_CAN_HAS_PRETENDED_NETWORKING

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_ConfigPN
 * Description   : Configures Pretended Networking settings.
 *
 * Implements    : FLEXCAN_DRV_ConfigPN_Activity
 *END**************************************************************************/
void FLEXCAN_DRV_ConfigPN(uint8_t instance, bool enable, const flexcan_pn_config_t *pnConfig)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type * base = g_flexcanBase[instance];

    FLEXCAN_EnterFreezeMode(base);

    if (enable)
    {
        FLEXCAN_ConfigPN(base, pnConfig);
#if FEATURE_CAN_HAS_SELF_WAKE_UP
        (void)FLEXCAN_SetSelfWakeUp(base, false);
#endif
    }

    FLEXCAN_SetPN(base, enable);

    FLEXCAN_ExitFreezeMode(base);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_GetWMB
 * Description   : Extracts one of the frames which triggered the wake up event.
 *
 * Implements    : FLEXCAN_DRV_GetWMB_Activity
 *END**************************************************************************/
void FLEXCAN_DRV_GetWMB(uint8_t instance, uint8_t wmbIndex, flexcan_msgbuff_t *wmb)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);
    DEV_ASSERT(wmb != NULL);

    uint32_t *tmp, wmbData;
    const CAN_Type *base  =  g_flexcanBase[instance];

    tmp = (uint32_t *)&wmb->data[0];
    wmbData = base->WMB[wmbIndex].WMBn_D03;
    REV_BYTES_32(wmbData, *tmp);

    tmp = (uint32_t *)&wmb->data[4];
    wmbData = base->WMB[wmbIndex].WMBn_D47;
    REV_BYTES_32(wmbData, *tmp);

    wmb->cs = base->WMB[wmbIndex].WMBn_CS;

    if ((wmb->cs & CAN_CS_IDE_MASK) != 0U)
    {
        wmb->msgId = base->WMB[wmbIndex].WMBn_ID;
    }
    else
    {
        wmb->msgId = base->WMB[wmbIndex].WMBn_ID >> CAN_ID_STD_SHIFT;
    }

    wmb->dataLen = (uint8_t)((wmb->cs & CAN_CS_DLC_MASK) >> 16);
}

#endif /* FEATURE_CAN_HAS_PRETENDED_NETWORKING */

#if FEATURE_CAN_HAS_SELF_WAKE_UP

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_ConfigSelfWakeUp
 * Description   : Enables/Disables Self Wake Up mode and low pass filter
 * applied to Rx input.
 *
 * Implements    : FLEXCAN_DRV_ConfigSelfWakeUp_Activity
 *END**************************************************************************/
status_t FLEXCAN_DRV_ConfigSelfWakeUp(uint8_t instance, bool enable, bool lowPassFilterEn)
{
    DEV_ASSERT(instance < CAN_INSTANCE_COUNT);

    CAN_Type *base  =  g_flexcanBase[instance];
    status_t status = STATUS_SUCCESS;

    /* Enable Self Wake Up */
    status = FLEXCAN_SetSelfWakeUp(base, enable);

    if (status == STATUS_SUCCESS)
    {
        FLEXCAN_EnterFreezeMode(base);

        /* Enable Low Pass Filter */
        FLEXCAN_SetWakSrc(base, lowPassFilterEn);

        FLEXCAN_ExitFreezeMode(base);
    }

    return status;
}

#endif /* FEATURE_CAN_HAS_SELF_WAKE_UP */

/*FUNCTION**********************************************************************
 *
 * Function Name : FLEXCAN_DRV_GetDefaultConfig
 * Description   : Gets the default configuration structure
 *
 * Implements    : FLEXCAN_DRV_GetDefaultConfig_Activity
 *END**************************************************************************/
void FLEXCAN_DRV_GetDefaultConfig(flexcan_user_config_t *config)
{
    /* Checks input parameter. */
    DEV_ASSERT(config != NULL);

#if FEATURE_CAN_HAS_PE_CLKSRC_SELECT
    /* Table to save FlexCAN clock indexes in PE clock configuration */
    static const flexcan_clk_source_t flexcanPEClkNames[FEATURE_CAN_PE_CLK_NUM] = FLEXCAN_PE_CLOCK_NAMES;
#endif

    /* Time segments computed for PE clock = 8 MHz, bitrate = 500 Kbit/s, sample point = 87.5 */
    flexcan_time_segment_t bitrate = {
        .propSeg = 7,
        .phaseSeg1 = 4,
        .phaseSeg2 = 1,
        .preDivider = 0,
        .rJumpwidth = 1
    };

    /* Maximum number of message buffers */
    config->max_num_mb = 16;
    /* Flexible data rate is disabled */
    config->fd_enable = false;
    /* Rx FIFO is disabled */
    config->is_rx_fifo_needed = false;
    /* Number of Rx FIFO ID filters */
    config->num_id_filters = FLEXCAN_RX_FIFO_ID_FILTERS_8;
    /* Normal operation mode */
    config->flexcanMode = FLEXCAN_NORMAL_MODE;
    /* Payload size */
    config->payload = FLEXCAN_PAYLOAD_SIZE_8;
#if FEATURE_CAN_HAS_PE_CLKSRC_SELECT
    /* Protocol engine clock is System Oscillator div 2 */
    config->pe_clock = flexcanPEClkNames[0];
#endif
    /* Time segments for the arbitration phase */
    config->bitrate = bitrate;
    /* Time segments for the data phase of FD frames */
    config->bitrate_cbt = bitrate;
    /* Rx FIFO transfer type */
    config->transfer_type = FLEXCAN_RXFIFO_USING_INTERRUPTS;
    /* Rx FIFO DMA channel */
    config->rxFifoDMAChannel = 0U;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
