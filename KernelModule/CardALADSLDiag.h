/******************************************************************************
********************************************************************************
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
**		CardAl
**
**	FILE NAME:
**		CardAlFalconDiag.h
**
**	ABSTRACT:
**		Public Header file for Diagnostics.  There should be NO direct
**		function calls into this module.
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/KernelModule/CardALADSLDiag.h $
**	$Revision: 4 $
**	$Date: 7/03/01 4:41p $
*******************************************************************************
******************************************************************************/
#ifndef _CARDALFALCONDIAG_H_					// File Wrapper
#define	_CARDALFALCONDIAG_H_

#include "Product.h"
#include "CardMgmt.h"
#include "CardALV.h"
#include "SmSysIf.h"
#include "sysinfo.h"

/*******************************************************************************
Module Public Functions
*******************************************************************************/
MODULAR VOID * CdalADSLDiagInit(
    IN void		* pAdapterContext,
    IN VOID		* pCardAlContext
);

#ifdef DBG
	#define CARDAL_ADSL_DIAG_DESCRIPTION "CardAl ADSL Diag Module",
#else
	#define CARDAL_ADSL_DIAG_DESCRIPTION
#endif

// * CardAL Interface Table
#define CARDAL_ADSL_DIAG_MOD \
	,{																										\
		CARDAL_ADSL_DIAG_DESCRIPTION	/* Description of Module Debug Only */								\
		CdalADSLDiagInit,	 			/* Init - Optional Init Card. Allocate resources	 		*/		\
		NULL,							/* Shutdown - Optional - Shutdown Card and remove Allocations*/		\
		NULL,							/* Enable - Optional */												\
		NULL							/* Disable - Optional */											\
	}

//define CARDAL_FALCON_DIAG_MOD

DPCCALL GLOBAL void CardALADSLLogPairgain
(
	PDATAPUMP_CONTEXT	  pDatapump, 			
    const char			* String,
    DIAG_SUBCODE_TYPE	  Subcode
);


GLOBAL UINT16 CdalADSLGetVendorId
(
    CARDAL_ADAPTER_T		* pCardALAdapter
) ;


GLOBAL UINT16 CdalADSLGetAutoWiring
(
    CARDAL_ADAPTER_T		* pCardALAdapter
) ;

GLOBAL UINT16 CdalADSLGetAutoHandshake
(
    CARDAL_ADAPTER_T		* pCardALAdapter
) ;

GLOBAL UINT16 CdalADSLGetAutoSenseWiresFirst
(
    CARDAL_ADAPTER_T		* pCardALAdapter
) ;


#endif									// File Wrapper _CARDALTIGRISDIAG_H_
