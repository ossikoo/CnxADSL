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
**		CardALTigrisHal.h
**
**	ABSTRACT:
**		CardALTigrisHal Tigris Hardware Interface Header File
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/KernelModule/CardALTigrisHal.h $
**	$Revision: 8 $
**	$Date: 7/05/01 12:06p $
*******************************************************************************
******************************************************************************/

#ifndef _CARDALTIGRISHAL_H				// File Wrapper
#define	_CARDALTIGRISHAL_H

#include "types.h"
#include "Common.h"

#ifndef _DATAPUMP_H_
	#define _DATAPUMP_H_
	typedef struct DATAPUMP * PDATAPUMP_CONTEXT;
#endif

#include "CardMgmt.h"
#include "xcvrinfo.h"


/*******************************************************************************
		Module Functions - These are NOT public to non ChipAl Modules
*******************************************************************************/
#if defined(_CHIPALCDSL_C_ ) || defined(_CHIPALCDSLV_H_)



MODULAR VOID *ChipAlTigrisHalInit
(
    IN const VOID				* UserHandle
) ;
MODULAR VOID ChipAlTigrisHalShutdown
(
    IN const VOID				* UserHandle
) ;



//*
//* Public Vector table entry for ChipAL
//*
#ifdef DBG
#define FALCON_MODULE_DESCRIPTION {"Falcon Hal Interface"},
#else
#define FALCON_MODULE_DESCRIPTION
#endif


#define CAL_SVC_TIGRIS															\
	{																			\
		FALCON_MODULE_DESCRIPTION												\
		ChipAlTigrisHalInit,			/* Service Init	*/						\
		ChipAlTigrisHalShutdown,		/* Service Shutdown*/					\
		NULL,							/* Service Command	*/					\
		NULL							/* Service Configure*/					\
	}

#endif



/*******************************************************************************
			General Public Module Functions
*******************************************************************************/

MODULAR VOID ChipAlTigrisHalDeviceRdyNotifyReq
(
    MODULAR void ( *ParmDeviceRdyNotify )
    (
        IN CDSL_ADAPTER_T			* pThisAdapter
    ),
    IN CDSL_ADAPTER_T			* pThisAdapter
) ;

MODULAR VOID ChipAlTigrisHalSetRstXcvr
(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN BOOLEAN					  state
) ;


MODULAR VOID ChipAlTigrisSetPwrDwnXcvr
(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN BOOLEAN					  State
) ;


MODULAR VOID ChipAlTigrisSetGpio
(
	IN CDSL_ADAPTER_T			* pThisAdapter,
	IN BYTE						  Pin,
	IN BOOLEAN					  state
) ;


MODULAR VOID ChipAlTigrisHalSetAFEHybridSelect
(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN UINT8					  Select_Line,
    IN BOOLEAN					  State
) ;


GLOBAL VOID ChipAlTigrisHalDisableIRQ1_2_ISR
(
    IN CDSL_ADAPTER_T			* pThisAdapter
) ;


MODULAR VOID ChipAlTigrisHalDisableIRQ1_2
(
    IN CDSL_ADAPTER_T			* pThisAdapter
) ;

MODULAR VOID ChipAlTigrisHalEnableIRQ1_2
(
    IN CDSL_ADAPTER_T			* pThisAdapter
) ;


GLOBAL VOID ChipAlTigrisHalSetAfePrint
(
	IN CDSL_ADAPTER_T			* pThisAdapter,
	IN BOOLEAN					  Setting
) ;

MODULAR VOID ChipAlTigrisHalWriteAfe
(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN BYTE						  Address,
    IN BYTE						  Data
) ;

MODULAR VOID ChipAlTigrisHalPendingReadAfe
(
	IN CDSL_ADAPTER_T			* pThisAdapter,
	IN BYTE						  Address
);

MODULAR BYTE ChipAlTigrisHalReadAfe
(
	IN CDSL_ADAPTER_T			* pThisAdapter
);

MODULAR BOOLEAN ChipAlTigrisHalIsAfeDone
(
    IN CDSL_ADAPTER_T			* pThisAdapter
) ;

GLOBAL BOOLEAN ChipAlSwitchHookStateEnq
(
    IN CDSL_ADAPTER_T			* pThisAdapter
) ;



MODULAR void * ChipAlTigrisHalGetFalconSpace
(
    IN CDSL_ADAPTER_T			* pThisAdapter
) ;


MODULAR VOID ChipAlTigrisHalTraceWrite
(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN BYTE					  	Offset,
    IN DWORD					Value
);


GLOBAL void ChipAlTigrisHalSetWiring
(
	IN  CDSL_ADAPTER_T				* pThisAdapter,
	IN BOOLEAN						  Automatic,
	IN SYS_WIRING_SELECTION_TYPE	  Wiring_Selection
);


#endif	// File Wrapper
