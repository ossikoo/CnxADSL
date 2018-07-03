/******************************************************************************
*******************************************************************************
****	Copyright (c) 1997, 1998, 1999
****	Conexant Systems Inc. (formerly Rockwell Semiconductor Systems)
****	Personal Computing Division
****	All Rights Reserved
****
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****
*******************************************************************************
**
**	MODULE NAME:
**		CardAL (Card Abstraction Layer)
**
**	FILE NAME:
**		CardALBdDp.c
**
**	ABSTRACT:
**		Card level Abstraction Layer for CDSL.  This file/module abstracts
**		(hides the details of) card specific (CDSL) functions and presents
**		(hopefully) a constant interface to the upper level modules/functions
**		(CardMgmt) no matter what the actual card is (CDSL, ADSL, ISDN, etc.).
**		This file provides the functions that process the BackDoor requests
**		from an application to interface to the CardAL module that relate to the
**		data pump code (Pairgain DMT).
**
**	DETAILS:
**		Functions are arranged in top-down order.
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/KernelModule/CardALBdDp.c $
** $Revision: 3 $
** $Date: 7/03/01 4:41p $
*******************************************************************************
******************************************************************************/
#define		MODULE_ID			DBG_ENABLE_CARDAL



//	Unique file ID for error logging
#define		__FILEID__			7


#include "CardALTigrisV.h"
#include "CardMgmt.h"
#include "CardALV.h"
#include "dmtapi.h"
#include "timer.h"
#include "rtbgstat.h"
#include "rtrxstat.h"
#include "rttxstat.h"






/******************************************************************************
*	FUNCTION NAME:
*		cardALBdDpGetAdslTransceiverStatus
*
*	ABSTRACT:
*		Function for performing the BackDoor ADSL Get ADSL Transceiver Status.
*		This is accomplished by calling pairgain DMT API core routines.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/

typedef union
{
	DATE_T		Date_T ;
	DATE_TYPE	Date_Type ;
} DATE_U ;

typedef union
{
	TIME_T		Time_T ;
	TIME_TYPE	Time_Type ;
} TIME_U ;

MODULAR DWORD cardALBdDpGetAdslTransceiverStatus
(
    IN CDSL_ADAPTER_T						* pThisAdapter,
    CARDAL_ADAPTER_T						* pCardALAdapter,
    I_O BD_CARDAL_ADSL_TRANSCEIVER_STATUS_T	* pBdAdslTransceiverStatus
)
{
	TIGRIS_CONTEXT_T			* pContext ;
	DATAPUMP_CONTEXT			* pDatapump ;
	XCVR_DATABASE_TYPE			* xcvr_db ;
	DATE_U						  Date ;
	TIME_U						  Time ;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;
	xcvr_db = &pDatapump->XCVR_database ;

	// Date & Time
	// (Have to use intermediate variable because although the source and destination
	// structures are actully the same, the function's is declared in terms of
	// Pairgain types and the Backdoor one in terms of Conextant types)
	Date.Date_Type = TM_GetSysDate(pDatapump).return_value;
	pBdAdslTransceiverStatus->Date = Date.Date_T ;

	Time.Time_Type = TM_GetSysTime(pDatapump).return_value;
	pBdAdslTransceiverStatus->Time = Time.Time_T ;


	// TX, RX and BG States
	pBdAdslTransceiverStatus->Transmit_State = TX_GetCurrentState ( pDatapump ) ;
	pBdAdslTransceiverStatus->Receive_State = RX_GetCurrentState ( pDatapump ) ;
	pBdAdslTransceiverStatus->Process_State = BG_GetCurrentState ( pDatapump ) ;


	// Up and Down SNR Margins
	pBdAdslTransceiverStatus->Up_SNR_Margin.Mar_Cur = pDatapump->SYS_measurement[UP_STREAM].mar_cur ;
	pBdAdslTransceiverStatus->Up_SNR_Margin.Mar_Min = pDatapump->SYS_measurement[UP_STREAM].mar_min ;
	pBdAdslTransceiverStatus->Up_SNR_Margin.Mar_Min_Bin = pDatapump->SYS_measurement[UP_STREAM].mar_min_bin ;

	pBdAdslTransceiverStatus->Down_SNR_Margin.Mar_Cur = pDatapump->SYS_measurement[DWN_STREAM].mar_cur ;
	pBdAdslTransceiverStatus->Down_SNR_Margin.Mar_Min = pDatapump->SYS_measurement[DWN_STREAM].mar_min ;
	pBdAdslTransceiverStatus->Down_SNR_Margin.Mar_Min_Bin = pDatapump->SYS_measurement[DWN_STREAM].mar_min_bin ;


	// Up & Down Attenuation
	pBdAdslTransceiverStatus->Up_Attn = pDatapump->SYS_measurement[UP_STREAM].line_atn ;
	pBdAdslTransceiverStatus->Down_Attn = pDatapump->SYS_measurement[DWN_STREAM].line_atn ;


	// Tx power
//	pBdAdslTransceiverStatus->Tx_Power = AFE_GetTxGain(pContext->Xcvr_Num) ;
	pBdAdslTransceiverStatus->Tx_Power = pDatapump->XCVR_database.TxGain  +
										 XCVR_NORMINAL_UP_TX_POWER_DENSITY         +
										 pDatapump->XCVR_database.up_tx_power ;


	// Up and Down Bits/Frame
	pBdAdslTransceiverStatus->Up_Bits_Per_Frame = xcvr_db->C_MSGS2.bits_per_sym ;
	pBdAdslTransceiverStatus->Down_Bits_Per_Frame = xcvr_db->R_MSGS2.bits_per_sym ;


	// Startup Attempts
	pBdAdslTransceiverStatus->Startup_Attempts = pDatapump->sys_mon_data.restart_count ;


	return RESULT_SUCCESS ;
}













