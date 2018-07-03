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
**		ChipAl
**
**	FILE NAME:
**		CardAlTigrisDiag.h
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
**	$Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CardALTigrisDiag.h $
**	$Revision: 1 $
**	$Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/
#ifndef _CARDALTIGRISDIAG_H_					// File Wrapper
#define	_CARDALTIGRISDIAG_H_

#include "Product.h"
#include "CardMgmt.h"
#include "CardALV.h"
#include "SmSysIf.h"

/*******************************************************************************
Module Public Functions
*******************************************************************************/
MODULAR VOID * CdalDiagInit(
    IN void		* pAdapterContext,
    IN VOID		* pCardAlContext
);

GLOBAL NTSTATUS CdalDiagGetAdslConfig(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagGetTxtLog(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

#ifdef DBG
	#define CARDAL_DIAG_DESCRIPTION "CardAl API Module",
#else
	#define CARDAL_DIAG_DESCRIPTION
#endif

//* CardAL Interface Table
#define CARDAL_DIAG_MOD \
	,{																										\
		CARDAL_DIAG_DESCRIPTION			/* Description of Module Debug Only */								\
		CdalDiagInit,					/* Init - Optional Init Card. Allocate resources	 		*/		\
		NULL,							/* Shutdown - Optional - Shutdown Card and remove Allocations*/		\
		NULL,							/* Enable - Optional */												\
		NULL							/* Disable - Optional */											\
	}

//define CARDAL_DIAG_MOD

#endif									// File Wrapper _CARDALTIGRISDIAG_H_
