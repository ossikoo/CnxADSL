/******************************************************************************
*******************************************************************************
****	Copyright (c) 1997, 1998
****	Rockwell Semiconductor Systems
****	Personal Computing Division
****	All Rights Reserved
****
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****
*******************************************************************************
**
**	MODULE NAME:
**		CardMgmt (Card Management)
**
**	FILE NAME:
**		CardMgmtCtl.h
**
**	ABSTRACT:
**		This files contains public defines for Card Manager Module.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CardMgmtCtl.h $
** $Revision: 1 $
** $Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
///////////////////////////////////////////////////////////////////////////////

NDIS_STATUS cmCtlAdapterHalt(   IN  CDSL_ADAPTER_T      * pThisAdapter );

NDIS_STATUS cmCtlAdapterInitialize(	IN CDSL_ADAPTER_T   * pThisAdapter );

NDIS_STATUS cmCtlAdapterReset(	IN  CDSL_ADAPTER_T      * pThisAdapter );

NDIS_STATUS cmCtlChipALAdapterInit(	IN CDSL_ADAPTER_T   * pThisAdapter );

NDIS_STATUS cmCtlCfgInit(	IN CDSL_ADAPTER_T   * pThisAdapter,
                          IN PTIG_USER_PARAMS  pParams );     // parameter struct from the download app

void cmCtlAutoStartLine( IN CDSL_ADAPTER_T	* pThisAdapter );

BOOLEAN cmCtlSimulateRemoteDisc(	IN CDSL_VC_T	* pThisVc );

VOID cmCtlTapiCallTimerHandler(
    IN ULONG		pThisVc
);

VOID cmCtlMainTimerInterrupt(
    IN PVOID				SystemSpecific1,
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN PVOID				SystemSpecific2,
    IN PVOID				SystemSpecific3
);

void cmCtlCallDiscReqHandler(	IN CDSL_VC_T * pThisVc );
void cmCtlIdleHdlr(	IN CDSL_VC_T * pThisVc );
BOOLEAN cmCtlMakeCall(	IN CDSL_VC_T		* pThisVc );
BOOLEAN cmCtlAnswer(	IN CDSL_VC_T		* pThisVc );

NDIS_STATUS cmCtlDeviceSpecific(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    IN BACK_DOOR_T					* pBackDoorBuf,
    OUT PULONG						pNumBytesNeeded,
    OUT PULONG						pNumBytesWritten
);

VOID cmCtlTapiCallStateHandler(
    IN CDSL_VC_T		* pThisVc,
    IN ULONG			CallState,
    IN ULONG			StateParam
);

