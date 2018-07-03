/******************************************************************************
****	
****	Copyright (c) 1997, 1998
****	Rockwell International
****	All Rights Reserved
****	
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****	
****	Technical Contact:
****		Rockwell Semiconductor Systems
****		Personal Computing Division
****		Huntsville Design Center
****		6703 Odyssey Drive, Suite 303
****		Huntsville, AL   35806
****	
*******************************************************************************


MODULE NAME:
	ChipAL

FILE NAME:
	ChipAlSEmw.h

ABSTRACT:
	Public Header file for MicroWire EEProm I/O via the Basic2 Device


DETAILS:
	

KEYWORDS:
	$Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/ChipALSEmw.h $
	$Revision: 1 $
	$Date: 1/09/01 10:53a $

******************************************************************************/
#ifndef  _ChipAlSEmw_h
#define  _ChipAlSEmw_h					// File Wrapper

/*******************************************************************************
Module Definitions
*******************************************************************************/
#undef	CAL_SEMW_INSTALLED
#define CAL_SEMW_INSTALLED 1
/*******************************************************************************
Module Public  Functions
*******************************************************************************/
#include "SmSysIf.h"

#if ! defined(REG_EEPROM)
// For Legacy Systems
	#define REG_EEPROM 	BASIC2_EEPROM
#endif

#define ORG_DEFAULT_STATE	1					/* WORD Organization (0 Byte)*/
#define CONFIG_64_16		1
#define MAX_EEPROM_SIZE		128
#define	QUEUE_IO_REQUESTS	0
#define IO_SIZE_BYTE		1					/* I/O Size is Byte.  Convert to Match Config */

typedef enum REG_IO_OPERATION_TYPE_E
{
    REG_IO_READ,
    REG_IO_WRITE
} REG_IO_OPERATION_TYPE_T;

typedef struct REG_IO_EXT_S
{
	union
	{
		BYTE	*				Source;				// Source information for Write Operation
		BYTE	*				Destination;		// Destination for Read Operation
	}SrcDestU;
	REG_IO_OPERATION_TYPE_T		IoType;				// Type of Io to be performed

	FN_IO_COMPLETE_HANDLER_T	IoComplete;			// User Callback location used to free Source/Dest. Memory
	VOID						* IoContext;		// User Defined Callback Context

}REG_IO_EXT_T;


GLOBAL void * CalSeInit(void * pThisAdapter,
                        DWORD  FunctSel);
GLOBAL void CalSeShutdown(I_O VOID * RegIo,
                          DWORD  FunctSel);


MODULAR CHIPAL_FAILCODE_T CalSeRead (
    I_O VOID		 	* Cfg,
    IN PVOID			BaseAddress,
    IN DWORD			OffSetAddress,
    IN DWORD			RegisterLength,
    OUT VOID 			* pReturnValue );

MODULAR CHIPAL_FAILCODE_T CalSeWrite (
    I_O VOID 			* Cfg,
    IN PVOID			BaseAddress,
    IN DWORD			OffSetAddress,
    IN DWORD			RegisterLength,
    IN DWORD			MaskValue,
    IN VOID				* Value );

#if QUEUE_IO_REQUESTS
MODULAR CHIPAL_FAILCODE_T CalSeReadPassive (
    I_O VOID 			* pRegContext,
    IN PVOID			BaseAddress,
    IN DWORD			OffSetAddress,
    IN DWORD			RegisterLength,
    OUT DWORD 			* pReturnValue );

MODULAR CHIPAL_FAILCODE_T CalSeWritePassive (
    I_O VOID 			* pSeContext,
    IN PVOID			BaseAddress,
    IN DWORD			OffSetAddress,
    IN DWORD			RegisterLength,
    IN DWORD			MaskValue,
    VOID				* Value );

#endif

//*
//* Public Vector table entry for ChipAL
//*


#ifdef DBG
#define SE_MODULE_DESCRIPTION {"EEProm - MicroWire I/F  "},
#else
#define SE_MODULE_DESCRIPTION
#endif

#define IO_CONTROL_SEMW 														\
	{																			\
		SE_MODULE_DESCRIPTION													\
		DEV_TYPE_LINEAR,														\
		0,								/* no function number */				\
		CalSeInit,						/* RegIoContInit	*/					\
		CalSeShutdown,					/* RegIoShutdown*/						\
		CalSeRead,						/* RegIoRead	*/						\
		CalSeWrite,						/* RegIoWrite	*/						\
		NULL							/* RegIoGetStatus */					\
	}

#endif 									// File Wrapper _ChipALCtrle136_h
