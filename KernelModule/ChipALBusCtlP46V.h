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
**		ChipAL
**
**	FILE NAME:
**		ChipALBusCtlP46v.h
**
**	ABSTRACT:
**		Basic 2 Bus Control service private (module) Header file. 
**		This is a component of the  ChipAL module.
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/ChipALBusCtlP46V.h $
**	$Revision: 1 $
**	$Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/
#ifndef _CHIPALBUSCTLP46V_H_				//	File Wrapper,
#define _CHIPALBUSCTLP46V_H_				//	prevents multiple inclusions


/*******************************************************************************
Macro Defines
*******************************************************************************/
// Calculate the byte offset of a field in a structure of type type.
//

#define PCI_ENABLE_IO_SPACE					0x0001
#define PCI_ENABLE_MEMORY_SPACE				0x0002
#define PCI_ENABLE_BUS_MASTER				0x0004
#define PCI_ENABLE_SPECIAL_CYCLES			0x0008
#define PCI_ENABLE_WRITE_AND_INVALIDATE		0x0010
#define PCI_ENABLE_VGA_COMPATIBLE_PALETTE	0x0020
#define PCI_ENABLE_PARITY					0x0040
#define PCI_ENABLE_WAIT_CYCLE				0x0080
#define PCI_ENABLE_SERR						0x0100
#define PCI_ENABLE_FAST_BACK_TO_BACK		0x0200

//*
//* Max PCI Configuration Values
//*
#define	PCI_MAX_DEVICES						32
#define PCI_MAX_FUNCTIONS					8
#define PCI_MAX_BUSES						256

#define	NULL_MEMORY							0
#define NULL_IRQ							0xffffffff

#if ! defined (DISABLE_INTERRUPT)
	#define DISABLE_INTERRUPT				0
#endif
/*******************************************************************************
Data Type Definitions
*******************************************************************************/

//*
//* From miniport.h or 	ntddk.h.  Due to redefinitions caused by directly
//* including these files, this structue is copied to this local file.  Future
//* versions ofthe  DDK may correct this problem.
//*
#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)	((LONG)&(((type *)0)->field))
#endif

/*******************************************************************************
Public Prototypes -  Note: These are provided as Vector Table entries. They
should NOT be call directly.  To call these functions, use the 
BusControlP46 table.	 These functions are intended to be replaced as needed
within this module.  By using the vector table, the changes will be confined to
this file/module.
*******************************************************************************/

GLOBAL BUS_RESOURCE_T * CalBusCtlP46Init (
    CDSL_ADAPTER_T * pAdapter,
    CHIPAL_TUNING_T * Tune,
    USHORT * MyAdapterCount );

GLOBAL void CalBusCtlP46Release (
    CDSL_ADAPTER_T * pAdapter,
    I_O BUS_RESOURCE_T * );

GLOBAL void 	CalBusCtlP46EnableInterrupt (
    BUS_RESOURCE_T	* pBusResources,
    DWORD                 FunctSel,
    DWORD		  EnableInterrupts );

GLOBAL void CalBusCtlP46DisableInterrupt (
    BUS_RESOURCE_T	* pBusResources,
    DWORD                 FunctSel,
    DWORD		  DisableInterrupts );

GLOBAL VOID CalBusCtlP46GetInterruptStatus (
    IN BUS_RESOURCE_T	* pBusResources,
    IN DWORD              FunctSel,
    OUT DWORD           * pARMInterrupts );

GLOBAL void CalBusCtlP46ClearInterruptStatus (
    IN BUS_RESOURCE_T	* pBusResources,
    IN  ULONG	  FunctSel,
    IN DWORD		  ClearInterrupts );

GLOBAL DWORD CalBusCtlP46GetErrorCount (
    IN BUS_RESOURCE_T	* pBusResources,
    IN DWORD			FunctSel );

GLOBAL DWORD CalBusCtlP46GetVersion (
    IN BUS_RESOURCE_T	* pBusResources,
    CDSL_ADAPTER_T		* pAdapter);

MODULAR CHIPAL_FAILCODE_T CalBusCtlP46GetConfig(
    IN	 CDSL_ADAPTER_T 		* CdslAdapter,
    CAL_CONFIG_PARAMS_T			* pConfig);

MODULAR CHIPAL_FAILCODE_T CalBusCtlP46SetConfig(
    IN	 CDSL_ADAPTER_T 		* CdslAdapter,
    CAL_CONFIG_PARAMS_T			* pConfig);

//*
//* BusControlP46 is the Public service/vector table of type "BUS_CONTROL_T",
//* (See ChipAl.doc). All upper layer routines should use the vector table
//* functions.  This will allow all or part of this service class to replaced by
//* a new module.
//*
#ifdef CHIPALBUSCTLP46_C
BUS_CONTROL_T BusControlP46 =
    {
        CalBusCtlP46Init,
        CalBusCtlP46Release,
        CalBusCtlP46EnableInterrupt,
        CalBusCtlP46DisableInterrupt,
        CalBusCtlP46GetInterruptStatus,
        CalBusCtlP46ClearInterruptStatus,
        CalBusCtlP46GetErrorCount,
        CalBusCtlP46GetVersion,
        CalBusCtlP46GetConfig,
        CalBusCtlP46SetConfig
    };
#else
extern  BUS_CONTROL_T BusControlP46;
#endif

#if CAL_P46_INSTALLED
	#define  BUS_CONTROLLER_SERVICE  BusControlP46	
#endif


#ifndef TRACE_CRITICAL
//* Debug Macro for use with a logic analyzer
	#define TRACE_CRITICAL(tag)
#endif

#endif									// _BUSCONTP46_H_







