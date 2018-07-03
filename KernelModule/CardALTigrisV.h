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
**		CardAL (Card Abstraction Layer)
**
**	FILE NAME:
**		CardALTigrisV.h
**
**	ABSTRACT:
**		Card level Abstraction Layer for CDSL.  This file/module abstracts
**		(hides the details of) card specific (CDSL) functions and presents
**		(hopefully) a constant interface to the upper level modules/functions
**		(CardMgmt) no matter what the actual card is (CDSL, ADSL, ISDN, etc.).
**		This files contains private defines for Card Abstraction Layer Module
**		that is specific to Tigris.
**		This private context structure is defined in this file.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/CardALTigrisV.h $
** $Revision: 14 $
** $Date: 7/13/01 8:07a $
*******************************************************************************
******************************************************************************/
#ifndef _CARDALTIGRISV_H_
#define _CARDALTIGRISV_H_


// type declaration that will be equivilent to TIGRIS_CONTEXT_T for type checking purposes
typedef struct
{
	int Dummy ;
} TIGRIS_CONTEXT_S ;


#include "Common.h"
#include "types.h"
#include "datapump.h"
#include "KThread.h"


#define NUM_VIRTUAL_DEVICE_TIMERS 3			// one for each work item

// array of GPIOs to assert for each wiring combination
typedef struct
{
	UINT8 Array [AUTOSENSE_WIRING_COMBO_MAX_NUM_COMBO][AUTOSENSE_WIRING_COMBO_MAX_NUM_GPIO] ;
} AUTOWIRING_COMBO_GPIO_T ;

// array of combination indexes in order that they should be tried
typedef struct
{
	UINT8 Array [AUTOSENSE_WIRING_COMBO_MAX_NUM_COMBO] ;
} AUTOWIRING_ORDER_T ;

// should we verify ptrs?
// NOTE: THIS IS NOT MULTI-INSTANCE SAFE AND ***MUST*** ***NOT*** BE LEFT ON
/*CLN!!!*/
#ifndef VERIFY_CONTEXT
#define VERIFY_CONTEXT 0
#endif

// define EXTERN so same line can be declaration and prototype
#ifdef EXTERN
#undef EXTERN
#endif

#ifdef CARDAL_TIGRIS_OWNER
#define EXTERN
#else
#define EXTERN extern
#endif

// macros to verify ptrs
#if VERIFY_CONTEXT
	EXTERN	void *pAdapterStatic ;	// CDSL_ADAPTER_T		
	EXTERN	void *pCardAlStatic ;	// CARDAL_ADAPTER_T	
	EXTERN	void *pTigrisStatic ;	// TIGRIS_CONTEXT_T	
	EXTERN	void *pDatapumpStatic ;	// DATAPUMP_CONTEXT	

	#define VERIFY_ADAPTER(pAdapter)   if ( pAdapter  != pAdapterStatic  ) printk ( "pAdapter  invalid %p != %p at %s %d\n", pAdapter,  pAdapterStatic,  __FILE__, __LINE__ ) ;
	#define VERIFY_CARDAL(pCardAl)     if ( pCardAl   != pCardAlStatic   ) printk ( "pCardAl   invalid %p != %p at %s %d\n", pCardAl,   pCardAlStatic,   __FILE__, __LINE__ ) ;
	#define VERIFY_TIGRIS(pTigris)     if ( pTigris   != pTigrisStatic   ) printk ( "pTigris   invalid %p != %p at %s %d\n", pTigris,   pTigrisStatic,   __FILE__, __LINE__ ) ;
	#define VERIFY_DATAPUMP(pDatapump) if ( pDatapump != pDatapumpStatic ) printk ( "pDatapump invalid %p != %p at %s %d\n", pDatapump, pDatapumpStatic, __FILE__, __LINE__ ) ;
#else
	#define VERIFY_ADAPTER(pAdapter)
	#define VERIFY_CARDAL(pCardAl)
	#define VERIFY_TIGRIS(pTigris)
	#define VERIFY_DATAPUMP(pDatapump)
#endif


///////////////////////////////////////////////////////////////////////////////
//	Structure  Declarations
///////////////////////////////////////////////////////////////////////////////
//*
//* Primary Context structure for this Module
//*
typedef struct
{
	ULONG					  LastLoResCallTime ;		// Last time low resolution timer was called

	EVENT_HNDL				  BGThreadDeviceRdyEvent ;	// Event to signal DSL_RDY command rxed
	BOOLEAN					  BGThreadDisableFlag ;		// Halt to BG Loop flag

	EVENT_HNDL				  SITrapEvent ;				// Event to signal Trap request
	BOOLEAN					  SIThreadDisableFlag ;		// Halt to SI Loop flag

	BOOLEAN					  TmrThreadDisableFlag ;	// Halt to Tmr Loop flag

	EVENT_HNDL				  TimerExpiredEvent[ NUM_VIRTUAL_DEVICE_TIMERS] ;
	// Event that we wait on
	SPIN_LOCK				  TimerStackLock ;			// Lock for accessing Timer Stack
	BYTE					  TimerStackPtr ;			// Index (SP) to TimerStack (Post increment store - pre decrement load )
	BYTE					  TimerStackArray [NUM_VIRTUAL_DEVICE_TIMERS] ;
	// Stack for allocating timer IDs
	WORD					  Qedfer_Wait_Flag  ;
	EVENT_HNDL				  Qedfer_Event ;

	VOID					* SysIfHandle;				// Handle for SysIf (COM ports)
	CDSL_ADAPTER_T			* pThisAdapter;				// Legacy Handle to Private Card Mgmt Interface
	VOID					* pCardAl;					// Handle to parent module - CardAl
    void			        * pDatapump;
} TIGRIS_CONTEXT_T ;

//*
//* Line Card Modules
//*
VOID * CDALTigrisInit(
    IN	VOID 					* AdapterContext,
    IN VOID 					* CardAlContext
);

VOID CDALTigrisShutdown(
    IN	VOID					* pCardALAdapter
);

NDIS_STATUS CDALTigrisReset(
    VOID						* pCardALAdapter
);

NDIS_STATUS CDALTigrisSetLineState(
    VOID						* pCardALAdapter,
    CARD_LINE_STATE_T			NewState
);

//*
//*	Optional Modules
//*
MODULAR VOID * CardALDeviceAdapterInit
(
    IN VOID		* pAdapterContext,
    IN VOID		* pCardAlContext
);

MODULAR VOID CardALDeviceAdapterHalt
(
    IN VOID							* pContext
);

MODULAR NTSTATUS CardALDeviceEnable
(
    IN VOID		* pModuleContext
);

MODULAR NTSTATUS CardALDeviceDisable
(
    IN VOID		* pModuleContext
);

MODULAR int cardALDeviceSIThread
(
	IN struct CDSL_ADAPTER_S *pAdapter
);

MODULAR int cardALDeviceTmrThread
(
	IN struct CDSL_ADAPTER_S *pAdapter
);

MODULAR int cardALDeviceBGThread
(
	IN struct CDSL_ADAPTER_S *pAdapter
);

typedef enum ADSL_DATA_PATH_SELECTION_E
{
    ADSL_PATH_SELECT_UNKNOWN		= 0,
    ADSL_PATH_SELECT_AUTO,
    ADSL_PATH_SELECT_FAST,
    ADSL_PATH_SELECT_INTERLEAVED
} ADSL_DATA_PATH_SELECTION_T;

typedef struct CARDAL_ADAPTER_S
{
	CDSL_ADAPTER_T				* pCdslAdapter;

	BOOLEAN						AdslLineUp;

	ADSL_DATA_PATH_SELECTION_T	AdslDataPathSelected;

	DWORD						LineAutoActivation;
	DWORD						LinePersistentAct;
	BYTE						AdslDpRateMode;

	CHIPAL_PM_STATE_T			PMDeviceState;

	DWORD						MacAddressLocation;
	WORD						PhysicalDriverType;
	BYTE						MacAddress [ETH_ADDRESS_LENGTH];

	#ifdef NDIS50_MINIPORT
	NDIS_DEVICE_POWER_STATE		NextNdisDeviceState;
	#endif

	//	maintain a temp media connect status

	DWORD						CurrentCRCErrors; // number of CRC errors at the time of connect


} CARDAL_ADAPTER_T;



//*
//* Other Project Specific Prototypes - Tigris to Tigris or Tigris to Public modules calls ONLY!
//* Non Tigris modules should NOT call these functions.
//*

void CardALTigrisSITrap 
(
    IN CDSL_ADAPTER_T			* pThisAdapter
) ;


TICK_TYPE CardAlTigrisGetCurrentTick
(
    IN CDSL_ADAPTER_T			* pThisAdapter
) ;

void CardAlTigrisWaitQedfer
(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN WORD						  Flags,
    IN WORD						  Resolution,
    IN WORD						  Timeout		//milliseconds
) ;

void CardAlTigrisSignalQedfer
(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN WORD						  Flags
) ;

DPCCALL DWORD cardALTigrisAFEEEPROM
(
	PVOID *		pContext
);

#ifdef DBG
	#define CARDAL_LC_DESCRIPTION  "Tigris Line Card Controller",
	#define CARDAL_OPT_DESCRIPTION "Tigris Card Support Utilities",
#else
	#define CARDAL_LC_DESCRIPTION
	#define CARDAL_OPT_DESCRIPTION
#endif

#define CARDAL_LC_FNS {  													\
		CARDAL_LC_DESCRIPTION			/* Description of Module Debug Only */								\
		CDALTigrisInit,					/* Init - Optional Init Card. Allocate resources	 		*/		\
		CDALTigrisShutdown,				/* Shutdown - Optional - Shutdown Card and remove Allocations*/		\
		NULL,							/* Enable - Optional */												\
		NULL,							/* Disable - Optional */											\
		CDALTigrisReset, 				/* Reset - Optional - Bring Card back to Init State			*/ 		\
		CDALTigrisSetLineState			/* Set Line State											*/		\
	}

#define CARDAL_OPT_MOD {  													\
		CARDAL_OPT_DESCRIPTION			/* Description of Module Debug Only */								\
		CardALDeviceAdapterInit,		/* Init - Optional Init Card. Allocate resources	 		*/		\
		CardALDeviceAdapterHalt,		/* Shutdown - Optional - Shutdown Card and remove Allocations*/		\
		CardALDeviceEnable,				/* Enable - Optional */												\
		CardALDeviceDisable				/* Disable - Optional */											\
	}

//* Provide a less generic name to reference CardAlTigris.c Context
#define CARDAL_TIGRIS_CONTROLLER	CDAL_OPTIONAL_UTIL

#endif		//#ifndef _CARDALTIGRISV_H_

