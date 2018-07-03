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
**		????
**
**	FILE NAME:
**		ARMAbstract.h
**
**	ABSTRACT:
**		This file is the main include file for the ARM abstraction layer
**		which is responsible for the interface to the ARM processor.
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4_ATM_Driver/KernelModule/ARMAbstract.h $
**	$Revision: 4 $
**	$Date: 5/23/01 8:19a $
*******************************************************************************
******************************************************************************/

#ifndef _ARMABSTRACT_H_
#define _ARMABSTRACT_H_

#include "CardMgmt.h"

////////////////////////////////////////
// Function Prototypes
////////////////////////////////////////
NDIS_STATUS
ARMALCfgInit (
    IN CDSL_ADAPTER_T	*pThisAdapter,
    IN PTIG_USER_PARAMS  pUserParams
);

NDIS_STATUS
ARMALInit(
    IN CDSL_ADAPTER_T	*pThisAdapter
);

NDIS_STATUS
ARMALHalt(
    IN CDSL_ADAPTER_T	*pThisAdapter
);

NDIS_STATUS
ARMALIoctl(
    IN CDSL_ADAPTER_T	*pThisAdapter,
    IN unsigned int		Command,
    IN PVOID			CmdArg
);

NTSTATUS
ARMIsCommandSetLoaded(
    INOUT	CDSL_ADAPTER_T	*pThisAdapter
);

//===================================================================================
//	This is the ARM abstraction vector table. These are the the generic
//	functions that will be called from within the Card Management module.
//	The function prototypes are defined in CardMgmt.h. If a module requires one
//	of these predefined functions then its name is placed into the #define
//	otherwise NULL is entered. In this way a consistent interface is maintained
//	with the Card Management module who will call these functions.
//===================================================================================
#define ARMAL_DESCRIPTION	"Arm Abstraction Module"
#define ARMAL_FNS	{										\
	ARMAL_DESCRIPTION,	/* Module Description			*/	\
	ARMALCfgInit,		/* Module CfgInit Function		*/	\
	ARMALInit,			/* Module AdapterInit Function	 */	\
	ARMALHalt,			/* Module shutdown function		*/	\
	NULL		        /* Ioctl handler */                 \
}

typedef struct
{
	// ******************************************
	// Arm Abstraction Module Context structure
	// ******************************************

	DWORD		NotUsedRightNow;

} ARM_ABSTRACT_T;


#endif // _ARMABSTRACT_H_

