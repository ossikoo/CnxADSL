/******************************************************************************
********************************************************************************
****	Copyright (c) 1997, 1998, 1999, 2000, 2001
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
**		CardAL
**
**	FILE NAME:
**		CardALTigrisDp.h
**
**	ABSTRACT:
**		CardALTigris Data Pump interface Header File
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4_ATM_Driver/KernelModule/CardALTigrisDp.h $
**	$Revision: 8 $
**	$Date: 7/05/01 9:47p $
*******************************************************************************
******************************************************************************/

#ifndef _CARDALTIGRISDP_H				// File Wrapper
#define	_CARDALTIGRISDP_H

#include "types.h"
#include "CardMgmt.h"
#include "CardALTigrisHalV.h"
#include "CardALTigrisV.h"
#include "xcvrinfo.h"
#include "sysinfo.h"

#ifndef _DATAPUMP_H_
	#define _DATAPUMP_H_
	typedef struct DATAPUMP * PDATAPUMP_CONTEXT;
#endif



/*******************************************************************************
Function Prototypes - Called from ChipAlTigrisHal
*******************************************************************************/


void ChipAlTigrisDPInit
(
    IN CDSL_ADAPTER_T 			* pThisAdapterParm,
    IN TIGRIS_HAL_CONTEXT_T		* pContext
) ;






/*******************************************************************************
Function Prototypes - Called from Pairgain Data Pump Controller
*******************************************************************************/

DPCCALL TICK_TYPE ChipAlTigrisDpGetCurrentTick
(
	IN PDATAPUMP_CONTEXT pDatapump
) ;

DPCCALL void ChipAlTigrisDpWaitQedfer
(
    IN WORD Flags,
    IN WORD Resolution,
    IN WORD Timeout
) ;

DPCCALL void ChipAlTigrisDpSignalQedfer
(
    IN WORD Flags
) ;

DPCCALL void ChipAlTigrisDPIndicateLinkUp
(
	IN TIGRIS_CONTEXT_S		* pControllerContextS
) ;

DPCCALL void ChipAlTigrisDPIndicateLinkDown
(
	IN TIGRIS_CONTEXT_S		* pControllerContextS
) ;



DPCCALL MODULAR BOOLEAN ChipAlTigrisDpTestAndSet
(
    IN volatile LOCK_TYPE		* x
) ;

DPCCALL MODULAR VOID ChipAlTigrisDpAtomicStart
(
	void
);

DPCCALL MODULAR VOID ChipAlTigrisDpAtomicEnd
(
	void
);

DPCCALL MODULAR VOID ChipAlTigrisDpSITrap
(
	void
) ;

DPCCALL MODULAR VOID ChipAlTigrisDpIsrStart
(
	void
);

DPCCALL MODULAR VOID ChipAlTigrisDpIsrEnd
(
	void
);

DPCCALL MODULAR VOID ChipAlTigrisDpDelayMsec
( 
	IN TIGRIS_CONTEXT_S		* pControllerContextS,
	DWORD					  duration 
) ;


DPCCALL MODULAR VOID ChipAlTigrisDpGetNonVolatileParms
(
	IN PDATAPUMP_CONTEXT		  pDatapump,
	IN XCVR_OTHER_CFG_TYPE		* pXcvr
);


DPCCALL MODULAR VOID ChipAlTigrisDpDisableIRQ1_2
(
    void
) ;


DPCCALL MODULAR VOID ChipAlTigrisDpEnableIRQ1_2
(
    void
) ;


DPCCALL MODULAR VOID ChipAlTigrisDpSetRstXcvr
(
    IN BOOLEAN					  State
) ;


DPCCALL MODULAR VOID ChipAlTigrisDpSetGpio
(
    IN BYTE						  Pin,
    IN BOOLEAN					  State
) ;


DPCCALL MODULAR VOID ChipAlTigrisDpSetAFEHybridSelect
(
    IN UINT8					  Select_Line,
    IN BOOLEAN					  State
) ;


DPCCALL MODULAR VOID ChipAlTigrisDpSetPwrDwnXcvr
(
    IN BOOLEAN					  State
) ;


DPCCALL MODULAR VOID ChipAlTigrisDpWriteAFE
(
    IN BYTE						  Address,
    IN BYTE						  Data
) ;


DPCCALL MODULAR VOID ChipAlTigrisDpPendingReadAFE
(
	IN BYTE						  Address
);

DPCCALL MODULAR BYTE ChipAlTigrisDpReadAFE
(
	void
);

DPCCALL MODULAR BOOLEAN ChipAlTigrisDpIsAFEDone
(
    void
) ;


DPCCALL GLOBAL BOOLEAN ChipAlDpSwitchHookStateEnq
(
    void
) ;

DPCCALL MODULAR VOID ChipAlTigrisDpTraceWrite
(
    IN BYTE	Offset,
    IN DWORD Value
) ;

DPCCALL GLOBAL UINT16 ChipAlTigrisDpGetVendorId
(
    void
) ;

GLOBAL UINT16 ChipAlTigrisDpGetAutoWiring
(
    void
) ;

GLOBAL UINT16 ChipAlTigrisDpGetAutoHandshake
(
    void
) ;

GLOBAL UINT16 ChipAlTigrisDpGetAutoSenseWiresFirst
(
    void
) ;

DPCCALL void ChipAlTigrisDpWaitQedfer
(
    IN WORD Flags,
    IN WORD Resolution,
    IN WORD Timeout
) ;

DPCCALL void ChipAlTigrisDpSignalQedfer
(
    IN WORD Flags
) ;

DPCCALL GLOBAL void ChipAlTigrisDpSetWiring
(
	BOOLEAN						Automatic,
    SYS_WIRING_SELECTION_TYPE	WiringSelection
) ;

GLOBAL void ChipAlTigrisDpSER_InitMsgQ
(
	IN CDSL_ADAPTER_T				* pThisAdapter
) ;

GLOBAL void ChipAlTigrisDpTRM_TerminalInit
(
	IN CDSL_ADAPTER_T				* pThisAdapter
) ;

GLOBAL void ChipAlTigrisDpSER_DetectTerminalConnected
(
	IN CDSL_ADAPTER_T				* pThisAdapter
) ;

GLOBAL void ChipAlTigrisDpTRM_AutoUpdateDisable
(
	IN CDSL_ADAPTER_T				* pThisAdapter,
	SYS_LOGON_MODE_TYPE				  mode,
	BOOLEAN							  disable
) ;

GLOBAL void ChipAlTigrisDpSER_ClearRemoteLogon
(
	IN CDSL_ADAPTER_T				* pThisAdapter
) ;


GLOBAL void ChipAlTigrisDpSER_RcvIntrHandler
(
	IN CDSL_ADAPTER_T				* pThisAdapter,
	IN char							  input_char
) ;

#endif	// File Wrapper
