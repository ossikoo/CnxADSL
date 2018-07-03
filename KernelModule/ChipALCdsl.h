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
**		ChipALCdsl.h
**
**	ABSTRACT:
**		Public Definitions for ChipAL Module
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/KernelModule/ChipALCdsl.h $
**	$Revision: 4 $
**	$Date: 5/17/01 9:47a $
*******************************************************************************
******************************************************************************/

#ifndef _CHIPALCDSL_H_					//	FILE WRAPPER,
#define _CHIPALCDSL_H_					//	prevents multiple inclusions

#include "../CommonData.h"
#include "Version.h"


/*******************************************************************************
Module Configuration Flags
*******************************************************************************/
#ifdef DBG
	#define DMA_STATS 1
#else
	#define DMA_STATS 0
#endif

//*
//* Define Optional Services Flags - All should be set to 0.  The appropriate
//* module will redefine, if installed.
//*

#if ! defined(CHIPAL_REMOTE)
	#define CHIPAL_REMOTE 0
#endif 

//*
//* Required Modules.  These flags should be set if they are present.
//*
#if PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
	#define CAL_CTRLE136_INSTALLED	1
	#define CAL_SEMW_INSTALLED		1
	#define CAL_PM_INSTALLED		1
	#define CAL_BASIC2_INSTALLED	1
	#define CAL_FALCON_INSTALLED	0
	#define CAL_P46_INSTALLED		0
	#define	CAL_IOP46_INSTALLED		0
	#define MAX_RESPONSE_SIZE		0
	#define CAL_SOFT_TIMER			0
	#define CAL_REVISION			0

#elif PROJECTS_NAME == PROJECT_NAME_TIGRIS
	#if (1) // (CHIPAL_REMOTE==0)
		#define CAL_CTRLE136_INSTALLED	0
		#define CAL_SEMW_INSTALLED		0
		#define CAL_PM_INSTALLED		0
		#define CAL_BASIC2_INSTALLED	0
		#define CAL_FALCON_INSTALLED	1
		#define CAL_P46_INSTALLED		1
		#define	CAL_IOP46_INSTALLED		1
		#define MAX_RESPONSE_SIZE		4
		#define CAL_SOFT_TIMER			1
		#define CAL_REVISION			1
	#else
		#define CAL_CTRLE136_INSTALLED	0
		#define CAL_SEMW_INSTALLED		0
		#define CAL_PM_INSTALLED		0
		#define CAL_BASIC2_INSTALLED	0
		#define CAL_FALCON_INSTALLED	1
		#define CAL_P46_INSTALLED		0
		#define	CAL_IOP46_INSTALLED		0
		#define MAX_RESPONSE_SIZE		4
		#define CAL_SOFT_TIMER			1
		#define CAL_REVISION			1
	#endif 	

#else
	#define CAL_CTRLE136_INSTALLED	0
	#define CAL_SEMW_INSTALLED		0
	#define CAL_PM_INSTALLED		0
	#define CAL_BASIC2_INSTALLED	0
	#define CAL_FALCON_INSTALLED	0
	#define CAL_P46_INSTALLED		0
	#define MAX_RESPONSE_SIZE		0
	#define CAL_SOFT_TIMER			0
	#define CAL_REVISION			0

#endif

#define BITS_PER_EVENT_REG	32			// Max Events per Register 

// defines for the PCI function selection
#define DSL_FUNCTION  0
#define ARM_FUNCTION  1
#define NUM_FUNCTIONS 2

typedef enum _CHIPAL_CHANNELS
{
    MIN_COM_CHANNEL = 0,
    CDSL_CHANNEL = 0,
    ADSL_CHANNEL,
    BEGIN_LOOPBACK_CHAN,
    ADSL_TX_LOOPBACK = BEGIN_LOOPBACK_CHAN,
    ADSL_RX_LOOPBACK,
    END_LOOPBACK_CHAN =ADSL_RX_LOOPBACK,
    MAX_COM_CHANNEL
} CHIPAL_CHANNELS_T;

#if (CAL_P46_INSTALLED)||(CHIPAL_REMOTE==1)
	#include "ChipALBusCtlP46.h"
#endif


#if	MAX_RESPONSE_SIZE 			// ChipAl supplies Response Param(s) to ISR / DPC Events

typedef struct _RESPONSE_S
{
	BYTE 	Param[MAX_RESPONSE_SIZE];
} RESPONSE_T;
//*
//* Function Pointer Type for Chipal Event Call Back Routines
//*
typedef void (* CHIPAL_EVENT_HANDLER_T)
(
    IN void			* pUserContext,
    OUT RESPONSE_T	* pParameters
);
#else
//*
//* Function Pointer Type for Chipal Event Call Back Routines
//*
typedef void (* CHIPAL_EVENT_HANDLER_T)
(
    IN void		* pUserContext
);
#endif			// Max Response Size

//*
//* Interrupt Masks are now defined by the Bus Controller public module.
//*
#if CAL_BASIC2_INSTALLED
typedef enum _INTERRUPT_EVENT
{
    DMA_COUNT_REACHED=		0x00003fff,
    TIMER_INTERRUPT	=		0x00004000,
    CI_BITS			=		0x00008000,
    INPUT_0_INTR	=		0x00010000,
    INPUT_1_INTR	=		0x00020000,
    INPUT_2_INTR	=		0x00040000,
    INPUT_3_INTR	=		0x00080000,
    INPUT_4_INTR	=		0x00100000,
    INPUT_5_INTR	=		0x00200000,
    INPUT_6_INTR	=		0x00400000,
    INPUT_7_INTR	=		0x00800000,
    L85_INTR		=		0x01000000,
    VOICE_INTR		=		0x02000000,
    ISDN_INTR		=		0x04000000,
    ORURINTR		=		0x08000000,
    PERR_OBS_IRQ	=		0x10000000,
    TARGET_ABORT_IRQ=		0x20000000,
    MASTER_ABORT_IRQ=		0x40000000,
    PERR_OCC_IRQ	=		0x80000000,
    INT_INVALID		=		0xffffffff
} INTERRUPT_EVENT_T;
#endif

typedef struct _HW_EVENT_INDEX
{
	union _HW_EVENT_U
	{
		DWORD Value;							// Complete BitMapped Value

		struct
		{
DWORD	Event:			5;			// Events Per Register
DWORD	Register:		27;			// Number of Event Registers
		} Bits;
	}EvntUnion;
} HW_EVENT_INDEX_T;


//*
//* Hardware Event Types
//*
typedef enum _HW_EVENT_TYPE {
    HW_EVENT_TYPE_BEGIN = 0,
    HW_DPC_EVENT = 0,
    HW_ISR_EVENT,
    HW_EVENT_TYPE_END
} HW_EVENT_TYPE_T;

// Preprocessor equivalent to HW_EVENT_TYPE_END.  This is needed due to typdefs
// based on the Max Number ofEvent types .  Note- These Values MUST be
// the same.
#define HW_MAX_EVENT_TYPES				0x00000002

//*
//* Hardware Events that can be trapped by other modules.  These are
//* now defined by the Bus Controller public module.
//*
#if CAL_BASIC2_INSTALLED
typedef enum _CHIPAL_EVENT
{
    HW_EVENT_BEGIN = 0,

    HW_CHANNEL_0_COUNT_REACHED= 0,		//	0x00000000	// Begin 1st Event Register
    HW_CHANNEL_1_COUNT_REACHED,			//	0x00000001
    HW_CHANNEL_2_COUNT_REACHED,			//	0x00000002
    HW_CHANNEL_3_COUNT_REACHED,			//	0x00000003
    HW_CHANNEL_4_COUNT_REACHED,			//	0x00000004
    HW_CHANNEL_5_COUNT_REACHED,			//	0x00000005
    HW_CHANNEL_6_COUNT_REACHED,			//	0x00000006
    HW_CHANNEL_7_COUNT_REACHED,			//	0x00000007
    HW_CHANNEL_8_COUNT_REACHED,			//	0x00000008
    HW_CHANNEL_9_COUNT_REACHED,			//	0x00000009
    HW_CHANNEL_10_COUNT_REACHED,		//	0x0000000A
    HW_CHANNEL_11_COUNT_REACHED,		//	0x0000000B
    HW_CHANNEL_12_COUNT_REACHED,		//	0x0000000C
    HW_CHANNEL_13_COUNT_REACHED,		//	0x0000000D
    HW_TIMER_INTERRUPT,					//	0x0000000E
    HW_CI_BITS,							//	0x0000000F
    HW_INPUT_0,							//	0x00000010
    HW_INPUT_1,							//	0x00000011
    HW_INPUT_2,							//	0x00000012
    HW_INPUT_3,							//	0x00000013
    HW_INPUT_4,							//	0x00000014
    HW_INPUT_5,							//	0x00000015
    HW_INPUT_6,							//	0x00000016
    HW_INPUT_7,							//	0x00000017
    HW_L85_INTERRUPT,					//	0x00000018
    HW_VOICE_INTERRUPT,					//	0x00000019
    HW_ISDN_INTERRUPT,					//	0x0000001A
    HW_ERROR,							//	0x0000001B
    HW_PERR_OBS,						//	0x0000001C
    HW_TARGET_ABORT,					//	0x0000001D
    HW_MASTER_ABORT,					//	0x0000001E
    HW_PERR_OCC,						//	0x0000001F
    HW_TX_EMPTY,						//	0x00000020	Begin 2nd Event Register
    HW_TX_AVAILABLE,					//	0x00000021
    HW_TX_FULL,							//	0x00000022
    HW_TX_THRESHOLD, 					//	0x00000023
    HW_RX_IDLE,							//	0x00000024
    HW_GENERAL_TIMER_A,					//	0x00000025
    HW_LINE_CONN_OPENED,				//	0x00000026
    HW_LINE_CONN_CLOSED,				//	0x00000027
    HW_LINE_CONN_LOST,					//	0x00000028
    HW_LINE_OPEN_FAIL_OR_REJ,			//	0x00000029
    HW_LINE_SELFTEST,					//	0x0000002A
    HW_LINE_OPERATIONAL_MODE_AVAIL,		//	0x0000002B
    HW_LINE_SUICIDE_REQ,				//	0x0000002C
    HW_LINE_ORDERLY_SHUTDOWN_EXECUTED,	//	0x0000002D
    HW_LINE_ORDERLY_SHUTDOWN_REJECTED,	//	0x0000002E
    HW_EVENT_END						//	0x0000002F

} CHIPAL_EVENT_T;

// Preprocessor equivalent to HW_EVENT_END.  This is needed due to typdefs
// based on the Max Number of Hardware Events.  Note- These Values MUST be
// the same.
#define	HW_MAX_EVENTS					0x0000002F
#endif									// CAL_BASIC2_INSTALLED

#define HW_MAX_EVENT_REGISTERS ((HW_MAX_EVENTS+BITS_PER_EVENT_REG-1)/BITS_PER_EVENT_REG)

#if PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
#define RX_BLOCK_SIZE		RECEIVE_CELL_SIZE
#define TX_BLOCK_SIZE		ATM_CELL_SIZE
#define IDLE_CHAR_LEN		2
#define ALCATEL_INTR		ISDN_INTR
#else
#define RX_BLOCK_SIZE		53
#define TX_BLOCK_SIZE		RX_BLOCK_SIZE
#endif

#define MAX_VALUE(_v1,_v2)	( ((_v1) > (_v2)) ? (_v1) : (_v2) ) 
#define MIN_VALUE(_v1,_v2)	( ((_v1) < (_v2)) ? (_v1) : (_v2) ) 


/*******************************************************************************
Public Data Types
*******************************************************************************/
typedef enum CAL_MODULE_TYPE_E
{
    MODULE_TYPE_START = 0,
    MODULE_TYPE_IO_CONTROLLER = MODULE_TYPE_START,
    MODULE_TYPE_DMA_CONTROLLER,
    MODULE_TYPE_BUS_CONTROLLER,
    MODULE_SERVICE,
    MODULE_TYPE_END
}CAL_MODULE_TYPE_T;

typedef enum _HardwareDevice
{
    MIN_HARDWARE_DEVICES = 0,
    MEMORY_MAPPED = 0,
    BASIC2_DEVICE = 0,
    HOBBES_FPGA = 0,					// HOBBES FPGA does not require special R/W routines
    P46_ARM_DSL,
    P46_ARM,
    EE_PROM,

    MAX_HARDWARE_DEVICES
} HARDWARE_DEVICE_T;

//*
//* Optional ChipAl Services.  These serve as ChipAl Extenstions.  This level
//* of functionality would normally be found in ChipAlcdsl.c
//*

//*
//* Generic Service Functions
//*
typedef enum CHIPAL_SERVICE_E
{
    MIN_CHIPAL_SERVICES= 0,
    CAL_SERVICE_PM = MIN_CHIPAL_SERVICES,
	#if CAL_FALCON_INSTALLED
    CAL_SERVICE_FALCON,
	#endif
    MAX_CHIPAL_SERVICES
} CHIPAL_SERVICE_T;

typedef enum CHIPAL_SERVICE_FN_E
{
    MIN_CHIPAL_SERVICES_FN= 0,
    SVC_INIT,
    SVC_SHUTDOWN,
    SVC_COMMAND,
    SVC_CONFIGURE,
    MAX_CHIPAL_SERVICES_FN
} CHIPAL_SERVICE_FN_T;

typedef enum CHIPAL_SERVICE_PM_E
{
    MIN_SVC_PM= MIN_CHIPAL_SERVICES_FN,
    SVC_PM_SET_STATE = SVC_COMMAND,
    SVC_PM_SET_POLICY = SVC_CONFIGURE,
    MAX_SVC_PM = MAX_CHIPAL_SERVICES_FN
} CHIPAL_SERVICE_PM_T;

typedef enum CHIPAL_DEV_TYPE_E
{
    DEV_TYPE_START = 0,
    DEV_TYPE_MEM_MAPPED = DEV_TYPE_START,
    DEV_TYPE_LINEAR,
    DEV_TYPE_END
} CHIPAL_DEV_TYPE_T;

typedef enum _CHIPAL_FAILCODE
{
    CHIPAL_SUCCESS = 0,
    CHIPAL_FAILURE,
    CHIPAL_BUSY,
    CHIPAL_RESOURCE_ALLOCATION_FAILURE,
    CHIPAL_RESOURCE_CONFLICT,
    CHIPAL_PARAM_OUT_OF_RANGE,			// 5
    CHIPAL_ADAPTER_NOT_FOUND = RSS_INIT_ERROR_NO_ADAPTER_FOUND, // 6
    CHIPAL_INVALID_PARAM,
    CHIPAL_MODULE_NOT_INITIALIZED,
    CHIPAL_SERVICE_UNAVAILABLE,
    CHIPAL_DEVICE_READ_ERROR,
    CHIPAL_DEVICE_WRITE_ERROR,
    CHIPAL_BASIC2 = 		0x00000200,
    CHIPAL_P46=				0x00000300,
    CHIPAL_DMA_CHANNEL =	0X00002000,
    CHIPAL_SYSTEM_SERVICE = 0X00004000,
    CHIPAL_REGISTER_IO=		0X00006000,
    CHIPAL_SVC_PM=			0x00008000,
    CHIPAL_SVC_FALCON=		0x00010000,
    CHIPAL_MODULE =			0X00200000,
    CHIPAL_NORMAL =			0X20000000,
    CHIPAL_WARNING =		0X40000000,
    CHIPAL_FATAL =			0X60000000
} CHIPAL_FAILCODE_T	;

//*
//* Power Management Structures
//*
typedef enum _CHIPAL_PM_STATE
{
    PM_STATE_START = 0,
    PM_STATE_D0 = PM_STATE_START,		/* Fully Operational */
    PM_STATE_D1,						/* PCI Clock Running, Process PME Event*/
    PM_STATE_D2,						/* PCI Clock Off, Process PME Event */
    PM_STATE_D3_HOT,					/* Power Off to Devices, Context Maintained*/
    PM_STATE_D3_COLD,					/* Power Off to Devices, Context lost */
    PM_STATE_END
} CHIPAL_PM_STATE_T;

typedef struct _PM_CONFIG_REG
{
	DWORD					Address;	/* Offset Address of Register to Maintain */
	DWORD					Len;		/* Length of Register	*/

	union PM_MASK_U
	{
		DWORD				BitMask;	/* Bits to Manage. Must be in one Register */
		VOID				* pBitMask;	/* Bits to Manage in multiple sequential registers */
		/* NOTE: pBitmask has predcedence over BitMask */
	}BitMaskU;

	union PM_REG_VALUE_U
	{
		DWORD				dwValue;	 /* ChipAL Reserved Field */
		VOID				* pRegBuffer;
	} RegBufU;
	DWORD					ClearValue;	/* For NonLinear devices, Write ClearValue to location */
	BOOL					ClearReg;	/* Enable ClearValue*/

} PM_CONFIG_REG_T;

#define NULL_PM_CONFIG_REG ((PM_CONFIG_REG_T *)0)

//* Config for IO Controller
typedef enum PCI_FN_NAME
{
    PCI_FN_START = 0,
    PCI_FN_P46ARB = PCI_FN_START,
    PCI_FN_MADMAX = PCI_FN_START,
    PCI_FN_DSL,
    PCI_FN_MODEM,
    PCI_FN_END
}PCI_FN_NAME_T;

typedef struct REGIO_CONFIG
{
	PCI_FN_NAME_T	PciFunction;
} REGIO_CONFIG_T;

//*
//* RegIo Device Status
//*
typedef enum _CHIPAL_DEVICE_STATUS
{
    CHIPAL_DEVICE_READY = 0,					/* Ready for Operation */
    CHIPAL_DEVICE_BUSY = -EBUSY		/* Busy */
} CHIPAL_DEVICE_STATUS_T;

#define ADDRESS_BAR_0				0
#define ADDRESS_BAR_1				1
#define ADDRESS_ARM_BAR_0           2
#define REMOTE_ADDRESS_BAR_2        3

typedef union IO_ADDRESS_U
{
	VOID *		pVoid;

	BYTE *		pByte;

	WORD *		pWord;

	DWORD *		pDword;

	DWORD		Dword;
} IO_ADDRESS_T;


typedef struct _PM_CONFIG_DEV
{
	HARDWARE_DEVICE_T		PmDevice;	/* Device to Manage */
	DWORD					Elements;	/* Number of structures supplied by Config */
	PM_CONFIG_REG_T			* Config;	/* Pointer to array of PM_CONFIG_REG	*/
} PM_CONFIG_DEV_T;

#define NULL_PM_CONFIG_DEV ((PM_CONFIG_DEV_T *)0)

#define		CHIPAL_ASSERT_TRACE_ALWAYS 	1	// Unconditionally Display Assert Message
#define		CHIPAL_ASSERT_TRACE_FULL 	1	// Use Expanded Details for Assert Message Trace
#define		CHIPAL_BREAK_ON_ASSERT 		0	// Issue Software Break on Assertion Failure

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __WARNING__ __FILE__ "("__STR1__(__LINE__)") : Warning Msg: "

// Usage:  To issue a compiler warning, do the following:
//	#pragma message(__WARNING__"Message Text")

#ifdef DBG

#if CHIPAL_ASSERT_TRACE_FULL
#define DBG_DISPLAY_DETAIL DBG_DISPLAY(__FILE__ " Line #%d  ",__LINE__)
#else
#define DBG_DISPLAY_DETAIL
#endif

#define	CHIPAL_DEBUG_MESSAGE(Mess)												\
if ( (ChipALDebugFlag & DBG_ENABLE_CHIPAL)										\
	 && (DEBUG_MESSAGE_LEVEL & ChipALDebugFlag) >=DBG_ENA_MISC_MSG)				\
{ 																				\
	DBG_DISPLAY_DETAIL;															\
	DBG_DISPLAY("ChipAl: " Mess "\n");										\
}

#if CHIPAL_BREAK_ON_ASSERT											
#define ASSERT_FAIL(Mess,Exp)													\
	ASSERTMSG((Mess "\n"), Exp);
#else																
#define ASSERT_FAIL(Mess,Exp)													\
	DBG_DISPLAY_DETAIL;															\
	DBG_DISPLAY("FATAL ASSERTION ERROR: " Mess "\n");					
#endif									// CHIPAL_BREAK_ON_ASSERT

#define CHIPAL_ASSERT_TRACE(Mess, Exp)											\
		if (Exp)																\
		{																		\
			if ( (ChipALDebugFlag & DBG_ENABLE_CHIPAL)							\
				 && (DEBUG_MESSAGE_LEVEL & ChipALDebugFlag) >=DBG_ENA_MISC_MSG)	\
			{																	\
				DBG_DISPLAY_DETAIL;												\
				DBG_DISPLAY("ChipAl: " Mess "\n");							\
			}																	\
																				\
		}																		\
		else																	\
		{																		\
			ASSERT_FAIL(Mess,Exp);												\
		}
#else									// DBG

#define	CHIPAL_DEBUG_MESSAGE(Mess)
#define CHIPAL_ASSERT_TRACE(Mess, Exp)

#endif									// DBG

typedef VOID (*PINTERFACE_REFERENCE)(PVOID Context);
typedef VOID (*PINTERFACE_DEREFERENCE)(PVOID Context);

// IRP_MN_QUERY_INTERFACE code and structures
typedef struct _INTERFACE {
	USHORT Size;
	USHORT Version;
	PVOID Context;
	PINTERFACE_REFERENCE InterfaceReference;
	PINTERFACE_DEREFERENCE InterfaceDereference;
	// interface specific entries go here
} INTERFACE, *PINTERFACE;

#include "ChipALInterf.h"
#include "ChipALRemoteI.h"

/*******************************************************************************
Public Prototypes
*******************************************************************************/

DWORD ChipALLinkInit (
    CDSL_ADAPTER_T *	CdslAdapter,	// Adapter Instance
    CDSL_LINK_T *		CdslLink,		// Link Instance
    DWORD				Speed,			// Link Speed in BPS
    DWORD				Channel,		// Communications Channel
    BYTE				Mode			// Framed (1) or Non-Framed (0)
);

NDIS_STATUS ChipALInitServices(IN CDSL_ADAPTER_T * CdslAdapter);

DWORD ChipALDataStreamEnable (
    CDSL_ADAPTER_T		* CdslAdapter,	// Adapter Instance
    DWORD				ComChannel		// Communications Channel
);

irqreturn_t ChipALIsrHandler (
    IN int   irq,
    IN PVOID ThisAdapter );

VOID ChipALDSLIsrDPCHandler (	IN PVOID pAdapter );
VOID ChipALARMIsrDPCHandler (	IN PVOID pAdapter );

NDIS_STATUS ChipALAdapterHalt(CDSL_ADAPTER_T * CdslAdapter);

NDIS_STATUS ChipALAdapterInit (
    IN CDSL_ADAPTER_T					* CdslAdapter );

NDIS_STATUS ChipALDeviceSpecific(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN BACK_DOOR_T				* pBackDoorBuf,
    IN PULONG					pNumBytesNeeded,
    OUT PULONG					pNumBytesWritten );

//*
//* Architect Requirement Functions
//*
GLOBAL NDIS_STATUS			ChipALSetInformation(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN PVOID				InfoBuffer,
    IN ULONG				InfoBufferLength,
    OUT PULONG				BytesWritten,
    OUT PULONG				BytesNeeded );

GLOBAL NDIS_STATUS			ChipALCfgInit (
    IN	CDSL_ADAPTER_T		* AdprCtx,
    IN	PTIG_USER_PARAMS        pParams
);

GLOBAL NTSTATUS ChipALSetFirmGo (
    IN CDSL_ADAPTER_T *	ThisAdapter ) ;

GLOBAL VOID ChipALEnableInterruptHandler (
    IN CDSL_ADAPTER_T *	ThisAdapter );

GLOBAL VOID ChipALDisableInterruptHandler (
    IN CDSL_ADAPTER_T *	ThisAdapter );


// The following are macro redefinitions of existing ChipAL functions
// to use the

#define ChipALLinkShutdown(CdslLink) 	    \
						 	(*CdslLink->pThisAdapter->pChipALRemoteInf->LinkInterface.pCalLinkShutDown)	   \
						 	(CdslLink->pThisAdapter->pChipALRemoteInf->LinkInterface.iface.Context, ADSL_CHANNEL) 	



#if CAL_REVISION >= 1

#define ChipALLinkEnable(CdslAdapter,ComChannel) 				    \
						 (*CdslAdapter->pChipALRemoteInf->LinkInterface.pCalLinkEnable)  	\
						  (CdslAdapter->pChipALRemoteInf->LinkInterface.iface.Context, 	\
						 ComChannel)								\

#define ChipALLinkDisable(CdslAdapter,ComChannel) 				\
						 (*CdslAdapter->pChipALRemoteInf->LinkInterface.pCalLinkDisable) 	\
						  (CdslAdapter->pChipALRemoteInf->LinkInterface.iface.Context, 	\
						  ComChannel)							  	\

#endif	// CAL_REVISION 1


#define ChipALStartTx(CdslLink )				 				\
						 (*CdslLink->pThisAdapter->pChipALRemoteInf->LinkInterface.pCalStartTx) 	\
						  (CdslLink->pThisAdapter->pChipALRemoteInf->LinkInterface.iface.Context, 	\
						   ADSL_CHANNEL /* what is parameter*/ ) 					\



#define ChipALRead(CdslAdapter,HwDevice,LogicalDevice, OffsetAddress,RegisterLength,pValue ) 				\
						 (*CdslAdapter->pChipALRemoteInf->CALIntf.pCALRead) 	\
						  (CdslAdapter->pChipALRemoteInf->CALIntf.iface.Context, 		\
						  HwDevice,							  	\
						  LogicalDevice,						\
						  OffsetAddress,						\
						  RegisterLength,						\
						  pValue )								\


#define ChipALWrite( CdslAdapter, HwDevice, LogicalDevice, OffsetAddress, RegisterLength, Value, Mask ) 				\
						 (*CdslAdapter->pChipALRemoteInf->CALIntf.pCALWrite) 	\
						  (CdslAdapter->pChipALRemoteInf->CALIntf.iface.Context, 		\
						  HwDevice,							  	\
						  LogicalDevice,						\
						  OffsetAddress,						\
						  RegisterLength,						\
						  Value,								\
						  Mask )								\


#define ChipALAddEventHandler(CdslAdapter,FunctSel, Event,IrqLevel,UserEventHandler, UserContext) 	\
						 (*CdslAdapter->pChipALRemoteInf->CALIntf.pCalAddEventHandler) 	\
						  (CdslAdapter->pChipALRemoteInf->CALIntf.iface.Context, 				\
						   FunctSel,									\
						   Event,										\
						   IrqLevel,									\
						   UserEventHandler,							\
						   UserContext)									\


#define ChipALRemoveEventHandler(CdslAdapter,FunctSel,Event,IrqLevel,UserEventHandler) 				\
						 (*CdslAdapter->pChipALRemoteInf->CALIntf.pCalRemoveEventHandler) 	\
						  (CdslAdapter->pChipALRemoteInf->CALIntf.iface.Context, 				\
						   FunctSel,									\
						   Event,										\
						   IrqLevel,									\
						   UserEventHandler)							\


#define ChipALSetEvent(CdslAdapter, FunctSel, Event) 				\
						 (*CdslAdapter->pChipALRemoteInf->CALIntf.pCalSetEvent) 	\
						  (CdslAdapter->pChipALRemoteInf->CALIntf.iface.Context, 			\
						   FunctSel,								\
						   Event)									\



//*
//* Architect Requirement Functions
//*


//*
//* Load public header files
//*
#if	CAL_CTRLE136_INSTALLED
	#include "ChipALHOBBES.h"
	#include "ChipALCtrle136.h"				// Alcatel CTRLE Interface 
#endif

#if CAL_BASIC2_INSTALLED
	#include "ChipALB2Regs.h"					  // Basic2 Register Definitions
#endif

#if CAL_SEMW_INSTALLED
	#include "ChipALSEmw.h"					// Serial EEProm
#endif

#if CAL_PM_INSTALLED
	#include "ChipALPM.h"
#endif

#if CAL_FALCON_INSTALLED
	#include "CardALTigrisHal.h"
#endif

#if (CAL_IOP46_INSTALLED)||(CHIPAL_REMOTE ==1)
	#include "ChipALIoP46.h"
#endif


//*
//* Create associations to data types define by IO Control or Optional modules
//*
typedef struct CAL_MODULE_PARAMS_S
{
	VOID * Context;

	union CAL_MODULE_PARAMS_U
	{
		DWORD					Reserved;

		#if CAL_PM_INSTALLED
		PARAM_PM_SET_POLICY_T	PmSetPolicy;
		PARAM_PM_SET_STATE_T	PmSetState;
		#endif

	} ModParsU;
} CAL_MODULE_PARAMS_T;

typedef union CAL_CONFIG_U
{
	PVOID					Reserved;

	#if (CAL_P46_INSTALLED)||(CHIPAL_REMOTE == 1)
	CONFIG_BUS_GET_T		BusConfig;
	#endif
} CAL_CONFIG_PARAMS_T;

#define NULL_CAL_CONFIG_PARAMS (CAL_CONFIG_PARAMS_T *)0

typedef struct CAL_CONFIG_S
{
	CAL_MODULE_TYPE_T			ModuleType;
	DWORD						Id;			// Future use for when multiple Module Types exist
	CAL_CONFIG_PARAMS_T			Params;

} CAL_CONFIG_T;

#define NULL_CAL_CONFIG		(CAL_CONFIG_T *)0


#if PROJECTS_NAME == PROJECT_NAME_TIGRIS
	#define AUTO_DISABLE_INTERRUPT	1
#else
	#define AUTO_DISABLE_INTERRUPT	0x00000000
#endif

/*******************************************************************************
Project Specific Public Functions 
*******************************************************************************/
GLOBAL CHIPAL_FAILCODE_T ChipALService (
    CDSL_ADAPTER_T		* pThisAdapter,
    CHIPAL_SERVICE_T	Service,
    DWORD				Function,
    CAL_MODULE_PARAMS_T	* Parameter
);

#define ChipAlGetConfig(CdslAdapter,Config)  \
		 (*CdslAdapter->pChipALRemoteInf->ConfigInterface.pCalGetConfig)   \
		 (CdslAdapter->pChipALRemoteInf->ConfigInterface.iface.Context,		   \
		  Config)															\

#define ChipAlSetConfig(CdslAdapter,Config)  \
		 (*CdslAdapter->pChipALRemoteInf->ConfigInterface.pCalSetConfig)   \
		 (CdslAdapter->pChipALRemoteInf->ConfigInterface.iface.Context,		   \
		  Config)															\

#endif									// _CHIPALCDSL_H_

//===================================================================================
//	This is the Chip Abstraction Layer vector table. These are the the generic
//	functions that will be called from within the Card Management module.
//	The function prototypes are defined in CardMgmt.h. If a module requires one
//	of these predefined functions then its name is placed into the #define
//	otherwise NULL is entered. In this way a consistent interface is maintained
//	with the Card Management module.
//===================================================================================
#define CHIPAL_DESCRIPTION	"Chip Abstraction Layer Module"
#define CHIPAL_FNS {							\
	CHIPAL_DESCRIPTION,	/*Module Description		*/		\
	ChipALCfgInit,		/*Module CfgInit Function	*/		\
	ChipALAdapterInit,	/*Module AdapterInit Function	*/	\
	ChipALAdapterHalt,	/*Module shutdown function	*/		\
	ChipALDeviceSpecific/*Ioctl handler */					\
}

