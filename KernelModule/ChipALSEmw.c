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
**		ChipAL
**
**	FILE NAME:
**		ChipAlSEmw.c
**
**	ABSTRACT:
**		Serial EEPROM routines for Fairchild Microwire device
**
**	DETAILS:
**		Routines are specific for The Basic2 PCI controller and the Fairchild 
**		NM93C66A device.  There should be minimal requirements to adapt 
**		this module to other Microwire devices.	All pins to PCI controller mapping
**		are done via Variable Definitions.  A recompile is not necessary.  To Change
**		dynamically, a new public Congig function would need to be added to the 
**		vector table.
**
**		This Module was created based on Fairchild's July 1998 Spec. Sheet - 
**		www.fairchildsemi.com
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux 2.4 ATM Driver/KernelModule/ChipALSEmw.c $
**	$Revision: 3 $
**	$Date: 3/19/01 12:48p $
*******************************************************************************
******************************************************************************/
#define _ChipAlSEmw_C 	1			// File Id

#define __FILEID__	101

#include "LnxTools.h"
#include "Product.h"
#include "CardMgmt.h"
#include "ChipALCdslV.h"
#include "ChipALSEmw.h"

#if ! defined(CHIPAL_ASSERT_TRACE)
	#define CHIPAL_ASSERT_TRACE
	#define CHIPAL_DEBUG_MESSAGE 
#endif


#ifndef PASSIVE_LEVEL
	#define PASSIVE_LEVEL 0
#endif

#define MAX_WRITE_ATTEMPTS	100

/*******************************************************************************
Private Module Definitions
*******************************************************************************/
#define CHIPAL_ERROR_FATAL(error)		(CHIPAL_FATAL | CHIPAL_REGISTER_IO | CHIPAL_BASIC2 | error)
#define CHIPAL_ERROR_WARNING(error)	    (CHIPAL_WARNING | CHIPAL_REGISTER_IO | CHIPAL_BASIC2 | error)

#ifndef MEMORY_TAG
	#define MEMORY_TAG			'ECNC'
#endif

//*
//* Timing Requirements - all values in nano secs
//*
#define USE_BUS_READ_DELAY	0			// Nano second delays are perfomed by 
// multiple reads to PCI bus if set
// to 1.  Otherwise, Nanoseconds are
// are rounded to the nearest Microsecond
// This is a Tuning Value Only.
// For Optimal performance set this value
// to 1.  For most flexible, set to 0.


#define TIME_SK_CLOCK_HIGH	1000
#define TIME_SK_CLOCK_LOW	1000
#define TIME_CS_SETUP		250
#define TIME_CS_SETUP_200	200
#define TIME_WRITE_CYCLE	15000000
#define TIME_WRITE_CYCLE_10	10000000

#define TIME_STATUS_VALID		70
#define TIME_STATUS_VALID_100	1000
#define PCI_MIN_READ_CYCLE	3
#define PCI_MIN_WRITE_CYCLE	4
#define PCI_CLOCK_33		(30*PCI_MIN_READ_CYCLE)
#define PCI_CLOCK_66		(15*PCI_MIN_READ_CYCLE)
#define BUS_CLOCK			PCI_CLOCK_66
#define RETRY_TIME			1000
#define UNDEFINED_VALUE		0xFFFFFFFF

#define TO_MICRO_SEC(n) 	(n/1000)
#define BITS_TO_BYTES(n)		(n/8)
#define UCHAR_BIT_POSITION(n)	(n%8)

/*******************************************************************************
Private  Module Data Types
*******************************************************************************/
typedef enum _PIN_STATE_T
{
	STATE_CLEAR = 0,
	STATE_SET = 1
} PIN_STATE_T;

#define START_BIT			0x01
#define OPCODE_READ			0x02
#define OPCODE_EWEN			0x18
#define OPCODE_EWDS			0x00
#define OPCODE_ERASE		0x06
#define OPCODE_WRITE		0x04
#define OPCODE_ERAL			0x08
#define OPCODE_WRAL			0x10

#define OPCODE_SIZE_READ	0x03
#define OPCODE_SIZE_WRITE	0x03
#define OPCODE_SIZE_EWEN	0x05
#define OPCODE_SIZE_EWDS	0x05
#define OPCODE_SIZE_ERASE	0x03
#define OPCODE_SIZE_ERAL	0x05
#define OPCODE_SIZE_WRAL	0x05

//*
//* 256x16 (16 bit data word)
//*
#define ORG_16_ADDRESS		8
#define	ORG_16_DATA			16

//*
//* 512x8 (8 bit data)
//*
#define ORG_8_ADDRESS		9
#define	ORG_8_DATA			8

//*
//* 64x16 (16 bit data word)
//*
#define ORG_6_ADDRESS		6

#define ORG_7_ADDRESS		7

//*
//* Number of Bits between Opcode+Address and the start of Data
//*
#define READ_FILL			0

//*
//* When ORG is not connected, the defult state is 1 (16 bit data)
//*


#if ORG_DEFAULT_STATE == 1				/* WORD Organization (0 Byte)*/
	#if CONFIG_64_16	
		#define ORG_ADDRESS_DEFAULT	 	ORG_6_ADDRESS
	#else
		#define ORG_ADDRESS_DEFAULT	 	ORG_16_ADDRESS
	#endif
	#define ORG_DATA_DEFAULT	 		ORG_16_DATA
#else									/* Some Other Organization (Byte, Etc... ) UNTESTED*/
	#if CONFIG_64_16	
		#define ORG_ADDRESS_DEFAULT	 	ORG_6_ADDRESS
	#else
		#define ORG_ADDRESS_DEFAULT	 	ORG_8_ADDRESS
	#endif
	#define ORG_DATA_DEFAULT	 		ORG_8_DATA
#endif

#define EEPROM_SIZE_128		128
#define EEPROM_SIZE_256		256





typedef enum _BYTE_ORDER_T
{
	MSB_TO_LSB = 0,
	LSB_TO_MSB = 1,
} BYTE_ORDER_T;

//*
//* Commands  created as follows:
//* Bit:
//*		0 - Start Bit (always 1)
//*		1 - First Opcode Bit
//*		2 - Third Opcode Bit
//*		4-7 -Reserverd for Number Address Bits
//*		8-11 - Reserved for Number of Data Bits
//*
typedef enum _SE_COMMAND_T
{
	CMD_READ_REG 		= 	START_BIT | OPCODE_READ,
	CMD_ENABLE_PROG 	= 	START_BIT | OPCODE_EWEN,
	CMD_DISABLE_PROG	= 	START_BIT | OPCODE_EWDS,
	CMD_ERASE_REG		=	START_BIT | OPCODE_ERASE,
	CMD_WRITE_REG		=	START_BIT | OPCODE_WRITE,
	CMD_ERASE_ALL		=	START_BIT | OPCODE_ERAL,
	CMD_WRITE_ALL		=	START_BIT | OPCODE_WRAL
} SE_COMMAND_T;

typedef struct _COMMAND_OPERATION_T
{
	union SE_COMMAND_U
	{
		SE_COMMAND_T	Commmand;

		struct
		{
			DWORD			StartBit:		1;
			DWORD			Opcode:			4;
			DWORD			Reserved2:		27;				// Unused
		} Operation;
	} CommandU;

	DWORD			OpCodeSize;
	DWORD			AddressSize;
	DWORD			DataSize;
	BYTE_ORDER_T	OpCodeOrder;
	BYTE_ORDER_T	AddressOrder;
	BYTE_ORDER_T	DataOrder;
	UCHAR			ReadFill;			// Dummy Read Count Between Address and Data

} COMMAND_OPERATION_T;

// *
// * Instructions for Device
// *
#define INST_READ \
{ \
	{CMD_READ_REG},					/* Command					*/			\
	OPCODE_SIZE_READ,				/* Opcode size  in Bits		*/			\
	ORG_ADDRESS_DEFAULT,			/* Address Size in Bits		*/			\
	ORG_DATA_DEFAULT,				/* Data Size in Bits		*/			\
	LSB_TO_MSB,						/* Order ofOpcode			*/			\
	MSB_TO_LSB,						/* Order of Address			*/			\
	MSB_TO_LSB,						/* Order of Data			*/			\
	READ_FILL						/* Number of Bytes Between Address and Data */ \
}

#define INST_WRITE \
{																			\
	{CMD_WRITE_REG},				/* Command					*/			\
	OPCODE_SIZE_WRITE,				/* Opcode size  in Bits		*/			\
	ORG_ADDRESS_DEFAULT,			/* Address Size in Bits		*/			\
	ORG_DATA_DEFAULT,				/* Data Size in Bits		*/			\
	LSB_TO_MSB,						/* Order ofOpcode			*/			\
	MSB_TO_LSB,						/* Order of Address			*/			\
	MSB_TO_LSB,						/* Order of Data			*/			\
	0								/* Number of Bytes Between Address and Data */ \
}

#define INST_ENABLE_PROG \
{																			\
	{CMD_ENABLE_PROG},				/* Command					*/			\
	OPCODE_SIZE_EWEN,				/* Opcode size  in Bits		*/			\
	ORG_ADDRESS_DEFAULT-2,			/* Address Size in Bits		*/			\
	0,								/* Data Size in Bits		*/			\
	LSB_TO_MSB,						/* Order ofOpcode			*/			\
	MSB_TO_LSB,						/* Order of Address			*/			\
	MSB_TO_LSB,						/* Order of Data			*/			\
	READ_FILL						/* Number of Bytes Between Address and Data */ \
}

#define INST_DISABLE_PROG \
{																			\
	{CMD_DISABLE_PROG},				/* Command					*/			\
	OPCODE_SIZE_EWDS,				/* Opcode size  in Bits		*/			\
	ORG_ADDRESS_DEFAULT-2,			/* Address Size in Bits		*/			\
	0,								/* Data Size in Bits		*/			\
	LSB_TO_MSB,						/* Order ofOpcode			*/			\
	MSB_TO_LSB,						/* Order of Address			*/			\
	MSB_TO_LSB,						/* Order of Data			*/			\
	READ_FILL						/* Number of Bytes Between Address and Data */ \
}

typedef struct _EE_TIMING_T
{
	DWORD					SkHigh;
	DWORD					SkLow;
	DWORD					CsSetup;
	DWORD					WriteCycle;
	DWORD					StatusValid;
	DWORD					BusSpeed;
} EE_TIMING_T;


typedef struct REGIO_COMMAND_S
{
	LIST_ENTRY				Next;				// Next Queued Item
	IN 	volatile PVOID		BaseAddress;		// Base Address for this IO operation
	IN 	DWORD				OffSetAddress;		// Starting Location
	IN 	DWORD				RegisterLength;		// Length of Access
	IN 	DWORD				MaskValue;			// Mask Value if used
	I_O	REG_IO_EXT_T		Ext;				// Extension to paramater List
} REGIO_COMMAND_T;

typedef enum DATA_ORDER_E
{
	BIG_ENDIAN_DO = 0,
	LITTLE_ENDIAN_DO
} DATA_ORDER_T;

typedef struct _EE_CONTEXT_T
{
	DWORD					DeviceSize;
	PIN_STATE_T				Organization;
	DATA_ORDER_T			DataOrder;
	UCHAR					CsPosition;
	UCHAR					SkPosition;
	UCHAR					DiPosition;
	UCHAR					DoPosition;
	UCHAR					EnPosition;
	UCHAR					Reserved[2];		// Fill to Maintain DWORD Alignment
	DWORD					ControlOffset;
	volatile DWORD 			* BaseAddress;
	EE_TIMING_T				Timing;

	DWORD					AddressSize;
	DWORD					DataSize;

	#if QUEUE_IO_REQUESTS
	//* For Non Ndis Drivers This module expects to run at passive Level.  Therefore,
	//* a system Threads and Queues are used
	LIST_ENTRY				IoQueue;
	NDIS_SPIN_LOCK 			IoLock;

	THREAD_CONTROL_T		IO;
	#endif

} EE_CONTEXT_T;

//*
//* To Add a new EEProm context definition, create a define and add to the
//* gEeContext table.  The table is processed based on the registry
//* entry that defines the Physical Driver Type.  In current drivers, 
//* this is a value stored in the registry key PhysicalDriverType.
//*
#define EE_CONTEXT_P46 \
{ \
	EEPROM_SIZE_128					,	/* DeviceSize */				\
	(PIN_STATE_T) ORG_DEFAULT_STATE	,	/* Organization */				\
	(DATA_ORDER_T) BIG_ENDIAN_DO	,	/* DataOrder */					\
	(UCHAR) SROMCS_BIT_POSITION		,	/* CsPosition */				\
	(UCHAR) SROMSK_BIT_POSITION		,	/* SkPosition */				\
	(UCHAR) SROMDO_BIT_POSITION		,	/* DiPosition */				\
	(UCHAR) SROMDI_BIT_POSITION		,	/* DoPosition */				\
	(UCHAR) SROMEN_BIT_POSITION		,	/* EnPosition UNUSED on P46*/	\
	{(UCHAR) 0x00,(UCHAR) 0x00}		,	/* Reserved */					\
	REG_EEPROM						,	/* ControlOffset */				\
	0								,	/* *volatile* BaseAddress */	\
	{ /* Timing */						/* Timing */					\
		TIME_SK_CLOCK_HIGH			,	/* SkHigh */					\
		TIME_SK_CLOCK_LOW			,	/* SkLow */						\
		TIME_CS_SETUP				,	/* CsSetup */					\
		TIME_WRITE_CYCLE			,	/* WriteCycle */				\
		TIME_STATUS_VALID			,	/* StatusValid */				\
		BUS_CLOCK					,	/* BusSpeed */					\
	}								,									\
	ORG_6_ADDRESS					,	/* AddressSize */				\
	ORG_16_DATA						,	/* DataSize */					\
}
#define EE_CONTEXT_P51 \
{ \
	EEPROM_SIZE_256					,	/* DeviceSize */				\
	(PIN_STATE_T) ORG_DEFAULT_STATE	,	/* Organization */				\
	(DATA_ORDER_T) BIG_ENDIAN_DO	,	/* DataOrder */					\
	(UCHAR) SROMCS_BIT_POSITION		,	/* CsPosition */				\
	(UCHAR) SROMSK_BIT_POSITION		,	/* SkPosition */				\
	(UCHAR) SROMDO_BIT_POSITION		,	/* DiPosition */				\
	(UCHAR) SROMDI_BIT_POSITION		,	/* DoPosition */				\
	(UCHAR) SROMEN_BIT_POSITION		,	/* EnPosition UNUSED on P46*/	\
	{(UCHAR) 0x00,(UCHAR) 0x00}		,	/* Reserved */					\
	REG_EEPROM						,	/* ControlOffset */				\
	0								,	/* *volatile* BaseAddress */	\
	{ /* Timing */						/* Timing */					\
		TIME_SK_CLOCK_HIGH			,	/* SkHigh */					\
		TIME_SK_CLOCK_LOW			,	/* SkLow */						\
		TIME_CS_SETUP_200			,	/* CsSetup */					\
		TIME_WRITE_CYCLE_10			,	/* WriteCycle */				\
		TIME_STATUS_VALID_100		,	/* StatusValid */				\
		BUS_CLOCK					,	/* BusSpeed */					\
	}								,									\
	ORG_7_ADDRESS					,	/* AddressSize */				\
	ORG_16_DATA						,	/* DataSize */					\
}

static EE_CONTEXT_T		gEeContext[DEVICE_TYPE_MAX] =
{
	EE_CONTEXT_P46,			/* Unused */
	EE_CONTEXT_P46,			/* P46 Based Devices */
	EE_CONTEXT_P51			/* P51 Based Devices */
};


//*
//* Predefined constant commands.  Note: These values should not be changed
//*
static	COMMAND_OPERATION_T gEnableCode = INST_ENABLE_PROG;
static	COMMAND_OPERATION_T gDisableCode = INST_DISABLE_PROG;
static	COMMAND_OPERATION_T gWriteCode = INST_WRITE;
static	COMMAND_OPERATION_T gReadCode = INST_READ;

#define NULL_EE_CONTEXT 	((EE_CONTEXT_T *)0)

#if ! defined(NDIS_MINIPORT_DRIVER)
KDPC					WaitCompleteDpc;
KTIMER					WaitTimer;
KEVENT					StallTimeout;
#endif

/*******************************************************************************
Private Prototypes 
*******************************************************************************/
LOCAL INLINE void	calSeReadBlock(
	I_O EE_CONTEXT_T		* Cfg,
	OUT CHAR				* pReturnValue,
	IN DWORD				OffSetAddress,
	IN DWORD				Length);

LOCAL INLINE void	calSeWriteBlock(
	I_O EE_CONTEXT_T 		* Cfg,
	IN CHAR					* pSourceValue,
	IN DWORD				OffSetAddress,
	IN DWORD				Length);

LOCAL INLINE void	calSeReadReg(
	I_O EE_CONTEXT_T 		* Cfg,
	IN DWORD					EEAddress,
	IN COMMAND_OPERATION_T	* pEeCode,
	IN CHAR					* OutData,
	OUT DWORD				* BitsRead);

LOCAL INLINE void calSeWriteReg(
	I_O EE_CONTEXT_T 		* Cfg,
	IN DWORD				EEAddress,
	IN COMMAND_OPERATION_T	* pEeCode,
	IN	CHAR				* SourceData,
	OUT DWORD				* BitsRead);

LOCAL INLINE void	calSeSetChipSelect(
	IN EE_CONTEXT_T			* Cfg,
	IN PIN_STATE_T			State);

LOCAL void calSeSetClockPhase(
	IN EE_CONTEXT_T			* Cfg,
	IN PIN_STATE_T			Phase);

LOCAL void	calSeStallExecution(
	IN EE_CONTEXT_T			* Cfg,
	IN DWORD 				NsDelay);

LOCAL  void	calSeCycleClock(
	IN EE_CONTEXT_T			* Cfg);

LOCAL void	calSeSetBits(
	I_O volatile DWORD		* Register,
	IN DWORD				Mask,
	IN DWORD				Value);

#if !defined(NDIS_MINIPORT_DRIVER)

LOCAL VOID calSeIoThread(
	IN EE_CONTEXT_T			* pContext
);

CalSeWaitComplete(
	IN PKDPC				Dpc,
	IN PVOID				DeferredContext,
	IN PVOID				SystemArgument1,
	IN PVOID				SystemArgument2 );

#endif

/*******************************************************************************
Module Functions
*******************************************************************************/

/******************************************************************************
FUNCTION NAME:
	CalSeInit

ABSTRACT:
	Allocate Necessary Resources


RETURN:
	Context Block for This instance or NULL if failure.


DETAILS:
******************************************************************************/
GLOBAL void * CalSeInit(void * pThisAdapter,
                        DWORD  FunctSel   )
{
	EE_CONTEXT_T				* pRegContext;
	NDIS_STATUS					Status;
	PHYSICAL_DRIVER_TYPE_T		PhysDriverType;
	CHIPAL_T					* pThisInterface;
	CDSL_ADAPTER_T				* pCdslAdapter;

	//*
	//* Allocate memory for  Context Block
	//*
	Status = ALLOCATE_MEMORY( (void*)&pRegContext, sizeof(EE_CONTEXT_T), MEMORY_TAG );

	if (Status != STATUS_SUCCESS)
	{
		return(NULL_EE_CONTEXT);
	}
	CLEAR_MEMORY(pRegContext, sizeof(EE_CONTEXT_T));

	//*
	//* Initialize Context structure to default state
	//*
	pThisInterface = pThisAdapter;
	pCdslAdapter = pThisInterface->pNdisAdapter;
	PhysDriverType = pCdslAdapter->PersistData.CnxtPhysicalDriverType;
	pRegContext->DeviceSize			= gEeContext[PhysDriverType].DeviceSize;
	pRegContext->Organization		= gEeContext[PhysDriverType].Organization;
	pRegContext->DataOrder			= gEeContext[PhysDriverType].DataOrder;
	pRegContext->AddressSize		= gEeContext[PhysDriverType].AddressSize;
	pRegContext->DataSize			= gEeContext[PhysDriverType].DataSize;
	pRegContext->CsPosition			= gEeContext[PhysDriverType].CsPosition;
	pRegContext->SkPosition			= gEeContext[PhysDriverType].SkPosition;
	pRegContext->DiPosition			= gEeContext[PhysDriverType].DiPosition;
	pRegContext->DoPosition			= gEeContext[PhysDriverType].DoPosition;
	pRegContext->EnPosition			= gEeContext[PhysDriverType].EnPosition;
	pRegContext->ControlOffset		= gEeContext[PhysDriverType].ControlOffset;
	pRegContext->Timing.SkHigh		= gEeContext[PhysDriverType].Timing.SkHigh;
	pRegContext->Timing.SkLow		= gEeContext[PhysDriverType].Timing.SkLow;
	pRegContext->Timing.BusSpeed	= gEeContext[PhysDriverType].Timing.BusSpeed;
	pRegContext->Timing.CsSetup		= gEeContext[PhysDriverType].Timing.CsSetup;
	pRegContext->Timing.WriteCycle	= gEeContext[PhysDriverType].Timing.WriteCycle;
	pRegContext->Timing.StatusValid	= gEeContext[PhysDriverType].Timing.StatusValid;

	gEnableCode.AddressSize = pRegContext->AddressSize - 2;
	gDisableCode.AddressSize = pRegContext->AddressSize - 2;
	gWriteCode.AddressSize = pRegContext->AddressSize;
	gWriteCode.DataSize = pRegContext->DataSize;
	gReadCode.AddressSize = pRegContext->AddressSize;
	gReadCode.DataSize = pRegContext->DataSize;

	#if QUEUE_IO_REQUESTS
	InitializeListHead(&pRegContext->IoQueue);
	KeInitializeEvent(&pRegContext->IO.StartThread, SynchronizationEvent, FALSE);
	KeInitializeEvent(&pRegContext->IO.ExitThread, SynchronizationEvent, FALSE);
	NdisAllocateSpinLock(&pRegContext->IoLock);
	pRegContext->IO.CurrentState = THREAD_STATE_INIT;

	Status = PsCreateSystemThread(
	             &pRegContext->IO.Name,			// Thread Handle
	             THREAD_ALL_ACCESS,				// Desired Access
	             NULL,							// Object Attributes
	             NULL,							// Process Handle
	             NULL,							// Client Id
	             calSeIoThread,
	             pRegContext
	         );
	#endif

	#if !defined(NDIS_MINIPORT_DRIVER)
	KeInitializeEvent( &StallTimeout, SynchronizationEvent, FALSE);
	KeInitializeTimer( &WaitTimer );
	KeInitializeDpc( &WaitCompleteDpc, CalSeWaitComplete, &StallTimeout );
	#endif
	//*
	//* Return Context Pointer
	//*
	return(pRegContext);

}


/******************************************************************************
FUNCTION NAME:
	CalSeShutdown

ABSTRACT:
	Release All Resources


RETURN:
	None


DETAILS:
******************************************************************************/
GLOBAL void CalSeShutdown(I_O VOID * RegIo,
                          DWORD  FunctSel)
{
	EE_CONTEXT_T * Context = (EE_CONTEXT_T *)RegIo;

	#if	QUEUE_IO_REQUESTS 
	KIRQL 					OldIrql = 0x00;
	#endif

	CHIPAL_ASSERT_TRACE
	(
	    "CalSeShutdown() ",
	    (Context != NULL_EE_CONTEXT )
	);


	#if	QUEUE_IO_REQUESTS  
	//* Signal the thread to Terminate
	NdisAcquireSpinLock(&Context->IoLock);
	Context->IO.CurrentState = THREAD_STATE_REQUEST_CANCEL;
	KeSetEvent(&Context->IO.StartThread, 0,FALSE);
	NdisReleaseSpinLock(&Context->IoLock);
	KeWaitForSingleObject(
	    &Context->IO.ExitThread,
	    Executive,
	    KernelMode,
	    FALSE,
	    NULL
	);
	#endif


	FREE_MEMORY
	(
	    Context,
	    sizeof(EE_CONTEXT_T),
	    CHIPAL_CONTEXT_MEM_FLAGS
	);
}

/******************************************************************************
FUNCTION NAME:
	CalSeRead

ABSTRACT:
	Read specified number of bytes from EEProm

RETURN:
	CHIPAL_FAILCODE_T
		CHIPAL_PARAM_OUT_OF_RANGE - Request Exceeds Eeprom Size
		CHIPAL_SUCCESS -All Ok

DETAILS:
******************************************************************************/
MODULAR CHIPAL_FAILCODE_T CalSeRead (
    I_O PVOID		 		pCfg,
    IN PVOID				BaseAddress,
    IN DWORD				OffSetAddress,
    IN DWORD				RegisterLength,
    OUT VOID 				* pReturnValue )
{
	//	BYTE * pSourceRegister;
	//	BYTE RegisterValue;
	//	DWORD *	EeControl;
	BYTE	BlockSize;
	EE_CONTEXT_T 		* Cfg=(EE_CONTEXT_T*)pCfg;

	ASSERT((BaseAddress != NULL && pReturnValue != NULL && GET_IRQL == PASSIVE_LEVEL ));

	Cfg->BaseAddress = (DWORD *) (((BYTE *)BaseAddress) + Cfg->ControlOffset);

	if((OffSetAddress + RegisterLength) > Cfg->DeviceSize)
	{
		CHIPAL_DEBUG_MESSAGE("CalSeRead:	Parameter Out of Range.\n");
		DBG_CDSL_DISPLAY(
			DBG_LVL_ERROR,
			DBG_FLAG_DONT_CARE,
			(
				"CnxADSL=>ChipALSEMw:CalSeRead: Parameter Out of Range  OffSetAddress=%lX, RegisterLength=%lX, DeviceSize=%lX",
				OffSetAddress,
				RegisterLength,
				Cfg->DeviceSize));
		return (-EINVAL);
	}

	BlockSize = (Cfg->Organization == STATE_SET)?sizeof(WORD):sizeof(BYTE);

	if (RegisterLength % BlockSize)
	{
		CHIPAL_DEBUG_MESSAGE("CalSeRead:	Data Size is Not Correct.\n");
		DBG_CDSL_DISPLAY(
			DBG_LVL_ERROR,
			DBG_FLAG_DONT_CARE,
			(
				"CnxADSL=>ChipALSEMw:CalSeRead: Data Size is Not Correct  RegisterLength=%lX, BlockSize=%X",
				RegisterLength,
				BlockSize));
		return (-EINVAL);
	}

	#if IO_SIZE_BYTE
	if(BlockSize != sizeof(BYTE) )
	{
		/* Adjust Starting Address to consider how many Bytes are in a Block */
		OffSetAddress /= BlockSize;

		/* Length is not Adjusted.  ReadBlock always expects Bytes */
	}
	#endif


	//*
	//* Set Programing Enable Mode for Basic 2 and Set the Clock to the High State
	//*
	if (Cfg->EnPosition)
	{
		calSeSetBits(
		    Cfg->BaseAddress,
		    1<<Cfg->EnPosition,
		    1<<Cfg->EnPosition);
	}
	calSeSetClockPhase(Cfg,STATE_SET);

	//*
	//* Read Block of Data from EEProm
	//*
	calSeReadBlock(Cfg,(CHAR *)pReturnValue,OffSetAddress,RegisterLength);

	//*
	//* Set scratchpad access Mode for Basic 2 and restore Clock to Low State
	//*
	calSeSetClockPhase(Cfg,STATE_CLEAR);

	if (Cfg->EnPosition)
	{
		calSeSetBits(
		    Cfg->BaseAddress,
		    1<<Cfg->EnPosition,
		    STATE_CLEAR);
	}
	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	CalSeWrite

ABSTRACT:
	Write specified number of bytes to EEProm

RETURN:
	CHIPAL_FAILCODE_T

		CHIPAL_PARAM_OUT_OF_RANGE - Too much data for eeprom
		CHIPAL_SUCCESS - All Ok

DETAILS:
******************************************************************************/
MODULAR CHIPAL_FAILCODE_T CalSeWrite (
    I_O void		 		* pCfg,
    IN PVOID				BaseAddress,
    IN DWORD				OffSetAddress,
    IN DWORD				RegisterLength,
    IN DWORD				MaskValue,
    VOID					* Value )
{
	BYTE	BlockSize;
	EE_CONTEXT_T 		* Cfg=(EE_CONTEXT_T*)pCfg;

	ASSERT
	(
	    (BaseAddress != NULL  && GET_IRQL == PASSIVE_LEVEL )
	);

	Cfg->BaseAddress = (DWORD *) (((BYTE *)BaseAddress) + Cfg->ControlOffset);

	if((OffSetAddress + RegisterLength) > Cfg->DeviceSize)
	{
		CHIPAL_DEBUG_MESSAGE("CalSeWrite:	Parameter Out of Range.\n");
		DBG_CDSL_DISPLAY(
			DBG_LVL_ERROR,
			DBG_FLAG_DONT_CARE,
			(
				"CnxADSL=>ChipALSEMw:CalSeWrite: Parameter Out of Range  OffSetAddress=%lX, RegisterLength=%lX, DeviceSize=%lX",
				OffSetAddress,
				RegisterLength,
				Cfg->DeviceSize));
		return (-EINVAL);
	}

	BlockSize = (Cfg->Organization == STATE_SET)?sizeof(WORD):sizeof(BYTE);
	if (RegisterLength % BlockSize)
	{
		CHIPAL_DEBUG_MESSAGE("CalSeWrite:	Data Size is Not Correct.\n");
		DBG_CDSL_DISPLAY(
			DBG_LVL_ERROR,
			DBG_FLAG_DONT_CARE,
			(
				"CnxADSL=>ChipALSEMw:CalSeRead: Data Size is Not Correct  RegisterLength=%lX, BlockSize=%X",
				RegisterLength,
				BlockSize));
		return (-EINVAL);
	}

	#if IO_SIZE_BYTE
	if(BlockSize != sizeof(BYTE) )
	{
		/* Adjust Starting Address to consider how many Bytes are in a Block */
		OffSetAddress /= BlockSize;

		/* Length is not Adjusted.  ReadBlock always expects Bytes */
	}
	#endif

	//*
	//* Set Programing Enable Mode for Basic 2 and Set the Clock to the High State
	//*
	if (Cfg->EnPosition)
	{
		calSeSetBits(
		    Cfg->BaseAddress,
		    1<<Cfg->EnPosition,
		    1<<Cfg->EnPosition);
	}
	calSeSetClockPhase(Cfg,STATE_SET);

	//*
	//* Read Block of Data from EEProm
	//*
	calSeWriteBlock(Cfg,(CHAR *)Value,OffSetAddress,RegisterLength);

	//*
	//* Set scratchpad access Mode for Basic 2 and restore Clock to Low State
	//*
	calSeSetClockPhase(Cfg,STATE_CLEAR);

	if (Cfg->EnPosition)
	{
		calSeSetBits(
		    Cfg->BaseAddress,
		    1<<Cfg->EnPosition,
		    STATE_CLEAR);
	}
	return(STATUS_SUCCESS);
}

/******************************************************************************
FUNCTION NAME:
	calSeReadBlock

ABSTRACT:
	Read 1 block of Data from EEProm


RETURN:
	None


DETAILS:
******************************************************************************/
LOCAL INLINE void	calSeReadBlock(
    I_O EE_CONTEXT_T 	* Cfg,
    OUT CHAR 			* pReturnValue,
    IN DWORD 			OffSetAddress,
    IN DWORD 			Length)
{
	DWORD				BitsRead;
	DWORD				BytesRead = 0;

	//*
	//* Read Data Blocks from Device
	//*
	while (BytesRead < Length)
	{
		BitsRead = 0;
		calSeReadReg(
		    Cfg,
		    OffSetAddress,
		    &gReadCode,
		    pReturnValue,
		    &BitsRead);
		BytesRead += BITS_TO_BYTES(BitsRead);
		OffSetAddress++;
		pReturnValue +=BITS_TO_BYTES(BitsRead);
	}
}

/******************************************************************************
FUNCTION NAME:
	calSeWriteBlock

ABSTRACT:
	Write Block of data to EEprom


RETURN:
	None


DETAILS:
******************************************************************************/
LOCAL INLINE void	calSeWriteBlock(
    I_O EE_CONTEXT_T 	* Cfg,
    IN CHAR 			* pSourceValue,
    IN DWORD 			OffSetAddress,
    IN DWORD 			Length)
{
	DWORD				BitsWritten;
	DWORD				BytesWritten = 0;

	//*
	//* Enable Device Programing
	//*

	calSeWriteReg(
	    Cfg,
	    OffSetAddress,
	    &gEnableCode,
	    pSourceValue,
	    &BitsWritten);
	//*
	//* Read Data Blocks from Device
	//*

	while (BytesWritten < Length)
	{
		BitsWritten = 0;
		calSeWriteReg(
		    Cfg,
		    OffSetAddress,
		    &gWriteCode,
		    pSourceValue,
		    &BitsWritten);
		BytesWritten += BITS_TO_BYTES(BitsWritten);
		OffSetAddress++;
		pSourceValue +=BITS_TO_BYTES(BitsWritten);
	}

	//*
	//* Disable Device Programing
	//*
	calSeWriteReg(
	    Cfg,
	    OffSetAddress,
	    &gDisableCode,
	    pSourceValue-BytesWritten,
	    &BitsWritten);

}
/******************************************************************************
FUNCTION NAME:
	calSeReadReg

ABSTRACT:
	Read 1 Register from EEprom


RETURN:
	None


DETAILS:
******************************************************************************/
LOCAL INLINE void	calSeReadReg(
    I_O EE_CONTEXT_T 		* Cfg,
    IN DWORD				EEAddress,
    IN COMMAND_OPERATION_T	* pEeCode,
    IN	CHAR				* OutData,
    OUT DWORD 				* BitsRead)
{
	LONG				BitPosition;
	DWORD				Count;
	//	BYTE				ByteOffSet;
	SHORT				Scaler;

	calSeSetChipSelect(Cfg,STATE_SET);
	//*
	//* Write Opcode
	//*
	if (pEeCode->OpCodeOrder == LSB_TO_MSB)
	{
		Scaler = 1;
		BitPosition = 0;
	}
	else
	{
		Scaler = -1;
		BitPosition = pEeCode->OpCodeSize-1;
	}
	for (Count= 0; Count < pEeCode->OpCodeSize; Count++, BitPosition+=Scaler)
	{
		calSeSetBits(Cfg->BaseAddress,
		             1 << Cfg->DiPosition,
		             ((pEeCode->CommandU.Commmand >> BitPosition) & 1) << Cfg->DiPosition);
		calSeCycleClock(Cfg);
	}

	//*
	//* Write Address
	//*
	if (pEeCode->AddressOrder == LSB_TO_MSB)
	{
		Scaler = 1;
		BitPosition = 0;
	}
	else
	{
		Scaler = -1;
		BitPosition = pEeCode->AddressSize-1;
	}

	for (Count= 0; Count < pEeCode->AddressSize; Count++, BitPosition+=Scaler)
	{
		calSeSetBits(Cfg->BaseAddress,
		             1 << Cfg->DiPosition,
		             ((EEAddress >> BitPosition) & 1) <<Cfg->DiPosition );
		calSeCycleClock(Cfg);
	}

	//*
	//* Read Dummy Bit(s)
	//*
	for (Count=0;Count < pEeCode->ReadFill;Count++)
	{
		calSeCycleClock(Cfg);
	}

	//*
	//* Read Data
	//*
	if (pEeCode->DataOrder == LSB_TO_MSB)
	{
		Scaler = 1;
		BitPosition = 0;
	}
	else
	{
		Scaler = -1;
		BitPosition = pEeCode->DataSize-1;
	}

	//*
	//* Clear Destination Location
	//*
	CLEAR_MEMORY(OutData, BITS_TO_BYTES(pEeCode->DataSize));

	//*
	//* Fill Destination
	//*
	for (Count= 0;
	        Count < pEeCode->DataSize;
	        Count++,BitPosition+=Scaler)
	{
		calSeCycleClock(Cfg);
		if(Cfg->DataOrder == BIG_ENDIAN_DO)
		{
			*(OutData + BITS_TO_BYTES(Count) ) |=
			    ((*(Cfg->BaseAddress) >> Cfg->DoPosition)&1)
			    << UCHAR_BIT_POSITION(BitPosition);

		}
		else
		{
			*(OutData + BITS_TO_BYTES(BitPosition)) |= ((*(Cfg->BaseAddress) >> Cfg->DoPosition)&1)
			        << UCHAR_BIT_POSITION(BitPosition);
		}
	}

	calSeSetChipSelect(Cfg,STATE_CLEAR);
	*BitsRead = pEeCode->DataSize;
}

/******************************************************************************
FUNCTION NAME:
	calSeWriteReg

ABSTRACT:
	Write one Register in EEprom


RETURN:
	None


DETAILS:
******************************************************************************/
LOCAL INLINE void calSeWriteReg(
    I_O EE_CONTEXT_T 		* Cfg,
    IN 	DWORD					EEAddress,
    IN 	COMMAND_OPERATION_T	* pEeCode,
    IN	CHAR				* SourceData,
    OUT DWORD 					* BitsRead)
{

	LONG				BitPosition;
	DWORD				Count;
	//	BYTE				ByteOffSet;
	SHORT				Scaler;
	DWORD				Ready = FALSE;

	calSeSetChipSelect(Cfg,STATE_SET);
	//*
	//* Write Opcode
	//*
	if (pEeCode->OpCodeOrder == LSB_TO_MSB)
	{
		Scaler = 1;
		BitPosition = 0;
	}
	else
	{
		Scaler = -1;
		BitPosition = pEeCode->OpCodeSize-1;
	}
	for (Count= 0; Count < pEeCode->OpCodeSize; Count++, BitPosition+=Scaler)
	{
		calSeSetBits(Cfg->BaseAddress,
		             1 << Cfg->DiPosition,
		             ((pEeCode->CommandU.Commmand >> BitPosition) & 1) << Cfg->DiPosition);
		calSeCycleClock(Cfg);
	}

	//*
	//* Write Address
	//*
	if (pEeCode->AddressOrder == LSB_TO_MSB)
	{
		Scaler = 1;
		BitPosition = 0;
	}
	else
	{
		Scaler = -1;
		BitPosition = pEeCode->AddressSize-1;
	}
	for (Count= 0; Count < pEeCode->AddressSize; Count++, BitPosition+=Scaler)
	{
		calSeSetBits(Cfg->BaseAddress,
		             1 << Cfg->DiPosition,
		             ((EEAddress >> BitPosition) & 1) <<Cfg->DiPosition );
		calSeCycleClock(Cfg);
	}

	//*
	//* Read Dummy Bit(s)
	//*
	for (Count=0;Count < pEeCode->ReadFill;Count++)
	{
		calSeCycleClock(Cfg);
	}

	//*
	//* Write Data
	//*
	if (pEeCode->DataOrder == LSB_TO_MSB)
	{
		Scaler = 1;
		BitPosition = 0;
	}
	else
	{
		Scaler = -1;
		BitPosition = pEeCode->DataSize-1;
	}

	for (Count= 0;
	        Count < pEeCode->DataSize;
	        Count++,BitPosition+=Scaler)
	{

		if(Cfg->DataOrder == BIG_ENDIAN_DO)
		{
			calSeSetBits(Cfg->BaseAddress,
			             1 << Cfg->DiPosition,
			             ((*(SourceData + BITS_TO_BYTES(Count)) >> UCHAR_BIT_POSITION(BitPosition))
			              & 1) << Cfg->DiPosition);
		}
		else
		{
			calSeSetBits(Cfg->BaseAddress,
			             1 << Cfg->DiPosition,
			             ((*(SourceData + BITS_TO_BYTES(BitPosition)) >> UCHAR_BIT_POSITION(BitPosition))
			              & 1) << Cfg->DiPosition);
		}
		calSeCycleClock(Cfg);
	}

	calSeSetChipSelect(Cfg,STATE_CLEAR);
	calSeSetChipSelect(Cfg,STATE_SET);

	if (pEeCode->DataSize != 0)
	{
		MICRO_DELAY(
		    TO_MICRO_SEC(Cfg->Timing.WriteCycle));

		//*
		//* Make sure Device is finished programming
		//*
		Ready = (*(Cfg->BaseAddress) >> Cfg->DoPosition) & 1;

		if (Ready != TRUE)
		{
			for(Count = 0; Count < MAX_WRITE_ATTEMPTS && Ready != TRUE; ++Count)
			{
				//*
				//* This should never happen.  The WriteCycle time should be
				//* long enough to allow the device to complete the programming
				//* cycle.
				//*
				MICRO_DELAY(
				    TO_MICRO_SEC(Cfg->Timing.WriteCycle));
				Ready = (*(Cfg->BaseAddress) >> Cfg->DoPosition) & 1;
			}

			if (Ready != TRUE)
			{
				Count = Count;
				CHIPAL_DEBUG_MESSAGE("calSeWriteReg(): EEprom Write Error\n");
			}
			else
			{
				Count = Count;
			}
		}
	}
	calSeSetChipSelect(Cfg,STATE_CLEAR);
	*BitsRead = pEeCode->DataSize;
}

/******************************************************************************
FUNCTION NAME:
	calSeSetChipSelect

ABSTRACT:
	Assert or Clear Chip Select and Wait specified time


RETURN:
	None


DETAILS:
******************************************************************************/
LOCAL INLINE void	calSeSetChipSelect(
    IN EE_CONTEXT_T 		* Cfg,
    IN PIN_STATE_T			State)
{
	calSeSetBits(Cfg->BaseAddress,1<<Cfg->CsPosition,State);
	calSeStallExecution(Cfg, Cfg->Timing.CsSetup);
}

/******************************************************************************
FUNCTION NAME:
	calSeSetClockPhase

ABSTRACT:
	Set Clock to a specific phase (high or low).  Delay specified time.


RETURN:
	None


DETAILS:
******************************************************************************/
LOCAL void calSeSetClockPhase(
    IN EE_CONTEXT_T 		* Cfg,
    IN PIN_STATE_T 			Phase)
{
	switch (Phase)
	{
	case STATE_SET:
		calSeSetBits(
		    Cfg->BaseAddress,
		    1 << Cfg->SkPosition,
		    1 << Cfg->SkPosition);
		MICRO_DELAY(
		    TO_MICRO_SEC(Cfg->Timing.SkHigh));
		break;

	case STATE_CLEAR:
		calSeSetBits(
		    Cfg->BaseAddress,
		    1 << Cfg->SkPosition,
		    STATE_CLEAR);
		MICRO_DELAY(
		    TO_MICRO_SEC(Cfg->Timing.SkLow));

		break;

	default:
		break;
	}
}
/******************************************************************************
FUNCTION NAME:
	calSeStallExecution

ABSTRACT:
	Stall Execution in Nanoseconds.


RETURN:
	None


DETAILS:The operation of this function depends on the definition of 
		USE_BUS_READ_DELAY. If defined to 1, the delay is performed by 
		reads to the pci bus.  Being this is dependent on the speed of the 
		pci bus, it is not optimal.  If using this method, the speed should
		be set to the fastest possible speed (66 mhz).  This will double the
		delay in a 33 mhz system.  A longer is ok, but a delay too short 
		can result in unexpected commands to the device (very bad).

		If USE_BUS_READ_DELAY is not defined, delays are performed by an
		external function.  All Nanosecond inputs are converted to Microseconds.
		Then, udelay is called to perform the actual delay. Note -
		any Nanosecond values that are not multiples of 1 microsecond will
		result in the value being rounded up 1 microsecond.
******************************************************************************/
LOCAL	void	calSeStallExecution(
    IN EE_CONTEXT_T 		* Cfg,
    IN DWORD 				NsDelay)
{

	#if USE_BUS_READ_DELAY
	volatile DWORD	CurrentDelay;
	volatile DWORD	TempValue;

	for(CurrentDelay = 0;
	        CurrentDelay < NsDelay;
	        CurrentDelay += Cfg->Timing.BusSpeed )
	{
		// Read Pci Bus.  Loop is for delay only
		TempValue = *Cfg->BaseAddress;

	}
	#else
DWORD	MicroSecDelay = 0;

	//*
	//* Convert NanoSeconds (NsDelay) to MicroSeconds
	//*
	if (NsDelay < 1000)
	{
		MicroSecDelay = 1;
	}
	else
	{
		MicroSecDelay = TO_MICRO_SEC(NsDelay);			// Convert to MicroSec
		MicroSecDelay += (NsDelay % 1000 != 0)?1:0;	// Round Up
	}

	MICRO_DELAY(MicroSecDelay);

	#endif

}

/******************************************************************************
FUNCTION NAME:
	calSeCycleClock

ABSTRACT:
	Cycle Clock 1 time


RETURN:
	None


DETAILS:
******************************************************************************/
LOCAL  void	calSeCycleClock(
    IN EE_CONTEXT_T 		* Cfg)
{
	calSeSetClockPhase(Cfg,STATE_CLEAR);
	calSeSetClockPhase(Cfg,STATE_SET);
}

/******************************************************************************
FUNCTION NAME:
	calSeSetBits

ABSTRACT:
	Perform Read Modify Write Cycle on selected Register


RETURN:
	None


DETAILS:
******************************************************************************/
LOCAL void	calSeSetBits(
    I_O  volatile DWORD 	* Register,
    IN DWORD				Mask,
    IN DWORD				Value)
{
	DWORD CurrentValue = 0L;

	CurrentValue = *Register;
	CurrentValue &= (~Mask);
	CurrentValue |= (Value & Mask);
	*Register = CurrentValue;
}

/*******************************************************************************
Optional Functions - These functions are not specifically required to read or 
write the EEPprom.  They provide compatibility between various Platforms (NDIS
WDM, etc...)
*******************************************************************************/
#if	QUEUE_IO_REQUESTS					//* Queue All Read / Write Requests 

/******************************************************************************
FUNCTION NAME:
	CalSeReadPassive

ABSTRACT:
	Store parameters for Read and signal Thread


RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
MODULAR CHIPAL_FAILCODE_T CalSeReadPassive (
    I_O EE_CONTEXT_T 		* pSeContext,
    IN PVOID				BaseAddress,
    IN DWORD				OffSetAddress,
    IN DWORD				RegisterLength,
    OUT DWORD 				* pReturnValue )
{
	KIRQL 					OldIrql = 0x00;
	NTSTATUS				Status;
	REGIO_COMMAND_T			* StoredIo;
	REG_IO_EXT_T			* pRegIoExt;

	// If irql is already passive, we do not call Routines to adapt to passive
	if (KeGetCurrentIrql() == PASSIVE_LEVEL)
	{
		pRegIoExt = (REG_IO_EXT_T *) pReturnValue;
		// * Ok to do read
		Status = CalSeRead (
		             pSeContext,
		             BaseAddress,
		             OffSetAddress,
		             RegisterLength,
		             (DWORD *)pRegIoExt->Destination);

		return(Status);
	}

	NdisAcquireSpinLock(&pSeContext->IoLock);

	if(pSeContext->IO.CurrentState >= THREAD_STATE_REQUEST_CANCEL)
	{
		//* IO Thread has terminated.  We cannot restart so the request will be
		//* failed.
		NdisReleaseSpinLock(&pSeContext->IoLock);
		return(NDIS_STATUS_RESOURCES);
	}

	Status = ALLOCATE_MEMORY( &StoredIo, sizeof(REGIO_COMMAND_T), MEMORY_TAG );

	if (Status != STATUS_SUCCESS)
	{
		NdisReleaseSpinLock(&pSeContext->IoLock);
		return(NDIS_STATUS_RESOURCES);
	}
	CLEAR_MEMORY(StoredIo, sizeof(REGIO_COMMAND_T) );
	StoredIo->Ext.IoType = REG_IO_READ;
	StoredIo->BaseAddress = BaseAddress;
	StoredIo->OffSetAddress = OffSetAddress;
	StoredIo->RegisterLength = RegisterLength;

	//* Copy All Extension Elements
	StoredIo->Ext = *((REG_IO_EXT_T *)pReturnValue);

	InsertTailList(&pSeContext->IoQueue, &StoredIo->Next);
	NdisReleaseSpinLock(&pSeContext->IoLock);
	OldIrql = KeGetCurrentIrql();

	//* Signal the thread to start new IO request
	KeSetEvent(&pSeContext->IO.StartThread, 0,FALSE);

	return(STATUS_PENDING);
}

/******************************************************************************
FUNCTION NAME:
	CalSeWritePassive

ABSTRACT:
	Store parameters for Write and signal Thread


RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
MODULAR CHIPAL_FAILCODE_T CalSeWritePassive (
    I_O EE_CONTEXT_T 		* pSeContext,
    IN PVOID				BaseAddress,
    IN DWORD				OffSetAddress,
    IN DWORD				RegisterLength,
    IN DWORD				MaskValue,
    VOID					* Value )
{
	KIRQL 					OldIrql = 0x00;
	NTSTATUS				Status;
	REGIO_COMMAND_T			* StoredIo;
	REG_IO_EXT_T			* pRegIoExt;

	// If irql is already passive, we do not call Routines to adapt to passive
	if (KeGetCurrentIrql() == PASSIVE_LEVEL)
	{
		pRegIoExt = (REG_IO_EXT_T *) Value;
		// * Ok to do read
		Status = CalSeWrite (
		             pSeContext,
		             BaseAddress,
		             OffSetAddress,
		             RegisterLength,
		             MaskValue,
		             (DWORD *)pRegIoExt->Source);
		return(Status);
	}

	NdisAcquireSpinLock(&pSeContext->IoLock);
	if(pSeContext->IO.CurrentState >= THREAD_STATE_REQUEST_CANCEL)
	{
		//* IO Thread has terminated.  We cannot restart so the request will be
		//* failed.
		NdisReleaseSpinLock(&pSeContext->IoLock);
		return(NDIS_STATUS_RESOURCES);
	}

	Status = ALLOCATE_MEMORY( &StoredIo, sizeof(REGIO_COMMAND_T), MEMORY_TAG );

	if (Status != STATUS_SUCCESS)
	{
		NdisReleaseSpinLock(&pSeContext->IoLock);
		return(NDIS_STATUS_RESOURCES);
	}
	CLEAR_MEMORY(StoredIo, sizeof(REGIO_COMMAND_T));
	StoredIo->Ext.IoType = REG_IO_WRITE;
	StoredIo->BaseAddress = BaseAddress;
	StoredIo->OffSetAddress = OffSetAddress;
	StoredIo->RegisterLength = RegisterLength;
	StoredIo->MaskValue = MaskValue;
	StoredIo->Ext = *((REG_IO_EXT_T *)Value);
	InsertTailList(&pSeContext->IoQueue, &StoredIo->Next);
	NdisReleaseSpinLock(&pSeContext->IoLock);

	//* Signal the thread to start new IO request
	KeSetEvent(&pSeContext->IO.StartThread, 0,FALSE);

	return(STATUS_PENDING);
}


/******************************************************************************
FUNCTION NAME:
	calSeIoThread

ABSTRACT:
	PASSIVE_LEVEL Thread for Reading and Writing EEProm.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
LOCAL VOID calSeIoThread(
    IN EE_CONTEXT_T			* pSeContext
)
{
	REGIO_COMMAND_T			* pCurrentIo;
	CHIPAL_FAILCODE_T		Status;
	//	KIRQL 					OldIrql;
	//	KPRIORITY				OldPriority;

	//*
	//* The default Priorty of a system thread is 0x8 (w2k 2031 checked).
	//*.LOW_REALTIME_PRIORITY is 0xF. If priority is left at 8, the
	//* PsTerminateSystemThread code may not exit before the driver
	//* is unloaded.  Therefore, you will get a Bug Check 0xCE -
	//* DRIVER_UNLOADED_WITHOUT_CANCELLING_PENDING_OPERATIONS
	//*
	KeSetPriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY );
	pSeContext->IO.CurrentState = THREAD_STATE_RUNNING;

	//*
	//* Main Thread Loop. Thread will continue running until it is cancelled by
	//* an external source.
	//*
	NdisAcquireSpinLock(&pSeContext->IoLock);
	while(pSeContext->IO.CurrentState < THREAD_STATE_REQUEST_CANCEL)
	{
		//*
		//* Drain all entries on the I/O Queue
		//*
		while( !IsListEmpty(&pSeContext->IoQueue) && pSeContext->IO.CurrentState == THREAD_STATE_RUNNING )
		{
			pCurrentIo = (REGIO_COMMAND_T *) RemoveHeadList(&pSeContext->IoQueue);
			NdisReleaseSpinLock(&pSeContext->IoLock);
			switch(pCurrentIo->Ext.IoType)
			{
			case REG_IO_READ:
				Status = CalSeRead (
				             pSeContext,
				             pCurrentIo->BaseAddress,
				             pCurrentIo->OffSetAddress,
				             pCurrentIo->RegisterLength,
				             (DWORD *) pCurrentIo->Ext.Destination);
				break;

			case REG_IO_WRITE:
				Status = CalSeWrite (
				             pSeContext,
				             pCurrentIo->BaseAddress,
				             pCurrentIo->OffSetAddress,
				             pCurrentIo->RegisterLength,
				             pCurrentIo->MaskValue,
				             pCurrentIo->Ext.Source);
				break;

			default:
				//* This can only happen if the Source Code is incorrect
				ASSERT(FALSE);
				break;
			}	// Switch

			if(pCurrentIo->Ext.IoComplete)
			{
				(* pCurrentIo->Ext.IoComplete)(
				    pCurrentIo->Ext.IoContext,
				    Status,
				    pCurrentIo->Ext.Source,
				    pCurrentIo->RegisterLength
				);
			}
			FREE_MEMORY(pCurrentIo, sizeof(REGIO_COMMAND_T), 0);
			NdisAcquireSpinLock(&pSeContext->IoLock);
		} //* while IoQueue

		if (pSeContext->IO.CurrentState == THREAD_STATE_RUNNING)
		{
			NdisReleaseSpinLock(&pSeContext->IoLock);
			//*
			//* Nothing else to do. Wait for next StartIo signal.
			//*
			KeWaitForSingleObject(
			    &pSeContext->IO.StartThread,
			    Executive,
			    KernelMode,
			    FALSE,
			    NULL
			);
			NdisAcquireSpinLock(&pSeContext->IoLock);
		}
	}		//* Main Thread Loop


	pSeContext->IO.CurrentState = THREAD_STATE_TERMINATED;

	//*
	//* Release any remaining queued Items
	//*
	while( !IsListEmpty(&pSeContext->IoQueue) )
	{
		pCurrentIo = (REGIO_COMMAND_T *) RemoveHeadList(&pSeContext->IoQueue);
		if(pCurrentIo->Ext.IoComplete)
		{
			(* pCurrentIo->Ext.IoComplete)(
			    pCurrentIo->Ext.IoContext,
			    STATUS_CANCELLED,
			    pCurrentIo->Ext.Source,
			    pCurrentIo->RegisterLength
			);

		}
		FREE_MEMORY(pCurrentIo, sizeof(REGIO_COMMAND_T), 0);
	}
	NdisReleaseSpinLock(&pSeContext->IoLock);
	KeSetEvent(&pSeContext->IO.ExitThread, 0, FALSE);
	PsTerminateSystemThread(STATUS_SUCCESS);
}

#endif 									//* QUEUE_IO_REQUESTS


#if !defined(NDIS_MINIPORT_DRIVER)
KEVENT	gStallTimeout;

/******************************************************************************
FUNCTION NAME:
	udelay

ABSTRACT:
	Simulate Micro Second stall function for WDM drivers.  This is a 
	PASSIVE level routine.


RETURN:
	NONE


DETAILS:
******************************************************************************/
void udelay(
    DWORD	MicroSecDelay
)
{
	int				Count = 0;
	KIRQL 			CurrentIrql = KeGetCurrentIrql();
	LARGE_INTEGER 	NsecDelay;
	LARGE_INTEGER 	StartTime;
	LARGE_INTEGER 	CurrentTime;

	KeQuerySystemTime(&StartTime);
	CurrentTime = StartTime;

	if(CurrentIrql != PASSIVE_LEVEL)
	{
		ASSERT(FALSE);  				// This Should Never happen
		return;
	}

	//* Convert MicroSeconds to units of 100 NanoSeconds
	NsecDelay.QuadPart = -((LONGLONG) MicroSecDelay * 10);

	//*
	//* Win98 Timers seem to fire before the minimum timeout expires.  Therefore,
	//* we make sure the Elapsed time has really expired before we exit.  If
	//* it has not, we simply wait again.
	//*
	while((CurrentTime.QuadPart - StartTime.QuadPart) < (NsecDelay.QuadPart * -1 ) )
	{
		KeSetTimer( &WaitTimer, NsecDelay, &WaitCompleteDpc );
		KeWaitForSingleObject(
		    &StallTimeout,
		    Executive,
		    KernelMode,
		    FALSE,
		    NULL
		);
		KeQuerySystemTime(&CurrentTime);
		++Count;
	}

	if(Count > 1)
	{
		Count = 0;
	}
}


/******************************************************************************
FUNCTION NAME:
	CalSeWaitComplete

ABSTRACT:
	Signal a waiting thread that a timer has expired


RETURN:
	VOID


DETAILS:
******************************************************************************/
CalSeWaitComplete(
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2 )
{
	KEVENT		* pEvent = DeferredContext;

	KeSetEvent(pEvent,
	           1,		// Priority	increment  for waiting thread.
	           FALSE);	// Flag	this call is not immediately followed by wait.
}

#endif

