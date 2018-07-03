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
**		ChipAlCdsl.c
**
**	ABSTRACT:
**		Chip Abstraction Layer Public Interface File
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4_ATM_Driver/KernelModule/ChipALCdsl.c $
**	$Revision: 8 $
**	$Date: 5/23/01 8:23a $
*******************************************************************************
******************************************************************************/


/*******************************************************************************
Includes
*******************************************************************************/
#define _CHIPALCDSL_C_
#define MODULE_ID	DBG_ENABLE_CHIPAL

#include "CardMgmt.h"
#include "FrameAL.h"
#include "../CommonData.h"
#include "LnxTools.h"

#include "ChipALCdslV.h"

#include "SmSysIf.h" 

/*******************************************************************************
ChipAl Module Definitions
*******************************************************************************/

#define ERROR_FATAL(error)		(LEVEL_FATAL   | MODULE_CHIPAL | INSTANCE_BASIC2 | error)
#define ERROR_WARNING(error)	(LEVEL_WARNING | MODULE_CHIPAL | INSTANCE_BASIC2 | error)
#define ERROR_NORMAL(error)		(LEVEL_NORMAL  | MODULE_CHIPAL | INSTANCE_BASIC2 | error)


#ifndef MEMORY_TAG
	#define MEMORY_TAG			'MCLC'
#endif

#define DEFAULT_MAX_LATENCY		65			// Preferred Milliseconds of Data in Channel Queue 
#define DEFAULT_MIN_LATENCY		45			// Tolerable Milliseconds of Data in Channel Queue
#define DEFAULT_INTERRUPT_RATE	15			// Interrupt Rate in Milliseconds
#define DEFAULT_SPEED			1000000		// Channel Speed in Bits per second
#define MAX_KEY_LENGTH 			40			// Maximum Length of a ChipAL Registery key name
#define BITS_PER_BYTE			8
#define BITS_PER_DWORD			(sizeof(DWORD) * BITS_PER_BYTE)
#define CHANNEL_SIZE_BY_SPEED	0
#define MAX_SYNC_WAIT			20


#define DEFAULT_RX_SEGMENT_NO	96
#if (PROJECTS_NDIS_IF == PROJECT_NDIS_IF_WAN)
#define DEFAULT_RX_SEGMENT_SIZE 2048
#else
#define DEFAULT_RX_SEGMENT_SIZE 512
#endif 
#define DEFAULT_RX_CHANNEL_SIZE	(DEFAULT_RX_SEGMENT_NO *  DEFAULT_RX_SEGMENT_SIZE)


#define DEFAULT_TX_SEGMENT_NO	8
#define DEFAULT_TX_SEGMENT_SIZE 2048
#define DEFAULT_TX_CHANNEL_SIZE	(DEFAULT_TX_SEGMENT_NO *  DEFAULT_TX_SEGMENT_SIZE)


#define CHIPAL_ERROR_FATAL(error)		(CHIPAL_FATAL   | CHIPAL_MODULE | CHIPAL_BASIC2 | error)
#define CHIPAL_ERROR_WARNING(error)		(CHIPAL_WARNING | CHIPAL_MODULE | CHIPAL_BASIC2 | error)
#define CLEAR_ALL_INTERRUPTS 0

#ifdef DBG
#define DBG_DPC_TRACE		0
#else
#define DBG_DPC_TRACE		0
#endif

// soft timer period is 10 ms
#define SOFT_TIMER_PERIOD	10

#define ONE_SEC 1000

/*******************************************************************************
User Defined Data Types
*******************************************************************************/

/*******************************************************************************
Private Data
*******************************************************************************/

UCHAR  ReverseByteTable[BYTE_SIZE];

#ifdef DBG
#define  TRACE_QUEUE_LEN 500
static WORD IntCount = 0;
static	 UCHAR TraceQueue[TRACE_QUEUE_LEN];
static   WORD TraceQueueIdx = 0;
#define TRACE_MESSAGE(n) \
	TraceQueue[TraceQueueIdx] = n; \
	TraceQueueIdx = ++TraceQueueIdx % TRACE_QUEUE_LEN;

typedef enum _TRACE_MESSAGES
{
    ISR_ENTRY = 1,
    ISR_EXIT,
    SYNC_ENTRY,
    SYNC_EXIT,
    TX_ENTRY,
    TX_EXIT,
    RX_ENTRY,
    RX_EXIT,
    DPC_ENTRY,
    DPC_EXIT
} TRACE_TYPE;
#else
#define TRACE_MESSAGE(n)
#endif


LOCAL CHIPAL_TUNING_T Tune;				// Module Tuning Parameters that are loaded before Initialization

/*******************************************************************************
Private Prototypes
*******************************************************************************/

LOCAL INLINE CHIPAL_T * chipALAllocateInterface (
    void);

LOCAL INLINE void chipALReleaseInterface
(CHIPAL_T * AdapterToRelease);

LOCAL INLINE void chipALProcessInterrupt (
    CHIPAL_T 		* pThisInterface,
    BUS_RESOURCE_T	* pBusResource,
    DWORD			  FunctSel,
    DWORD 			* Interrupt);

LOCAL INLINE BOOLEAN chipALClearInterrupt (
    CHIPAL_T	* pThisInterface,
    DWORD		FunctSel,
    DWORD		IntStatus);

LOCAL INLINE void chipALGetStats (
    CDSL_ADAPTER_T * pThisAdapter,
    BACK_DOOR_T	*	pBackDoorBuf);

LOCAL INLINE void chipALClearStats (
    CDSL_ADAPTER_T * pThisAdapter,
    BACK_DOOR_T	*	pBackDoorBuf);

LOCAL HW_CH_ENTRY_T * chipALFindLink (
    CHIPAL_T 			* pThisInterface,
    CDSL_LINK_T			* pCdslLink,
    CHAN_DESCRIPT_T		Description,
    DWORD				Mask );


#if CAL_SOFT_TIMER
VOID chipAlSoftTimer(
    IN ULONG	pInterf
);
#endif

#ifdef DBG								// Development Test Routines
// *
// * The ChipALTestxxx Prototypes are for development purposes only
// *
DWORD chipALTestReadBuff (char * Destination, DWORD BuffSize, DWORD * SizeWritten);
DWORD chipALTestWriteBuff(CDSL_ADAPTER_T * ThisAdapter, char * Destination, DWORD Size);
DWORD chipALTestReadConfig (CDSL_ADAPTER_T * ThisAdapter, BD_CHIPAL_CONFIG_T * pConfigStatus);
#endif 									// DBG
LOCAL CHIPAL_FAILCODE_T  lChipALRead (
    VOID				* pAdapter,
    HARDWARE_DEVICE_T	HwDevice,
    DWORD				LogicalDevice,
    DWORD				OffsetAddress,
    DWORD				RegisterLength,
    PVOID				pValue );

LOCAL CHIPAL_FAILCODE_T  lChipALWrite (
    VOID				* pAdapter,
    HARDWARE_DEVICE_T	HwDevice,
    DWORD				LogicalDevice,
    DWORD				OffsetAddress,
    DWORD				RegisterLength,
    DWORD				Value,
    DWORD				Mask);


LOCAL CHIPAL_FAILCODE_T lChipALAddEventHandler (
    VOID						* pAdapter,
    IN DWORD					FunctSelect,    // selects either the arm or dsl function
    IN CHIPAL_EVENT_T			Event,			// Enumerated Event to Set
    IN HW_EVENT_TYPE_T			IrqLevel,		// When Event should be called - DIRQl or Dispatch IRQL
    IN CHIPAL_EVENT_HANDLER_T	UserEventHandler,// Function to Call
    IN void						* UserContext	// Function Context - Unused by ChipAL
);

LOCAL CHIPAL_FAILCODE_T lChipALRemoveEventHandler (
    IN VOID						* pAdapter,
    IN DWORD					FunctSel,       // selects the arm or dsl function
    IN CHIPAL_EVENT_T			Event,			// Event to Remove
    IN HW_EVENT_TYPE_T			IrqLevel,		// When Event was being called
    IN CHIPAL_EVENT_HANDLER_T	UserEventHandler// Handler to remove
);

LOCAL void lChipALSetEvent (
    VOID				* pAdapter,
    DWORD				FunctSel,
    CHIPAL_EVENT_T		Event);

LOCAL CHIPAL_FAILCODE_T	lChipAlGetConfig (
    IN VOID						* pAdapter,
    I_O	VOID					* pConfigArg);

LOCAL	CHIPAL_FAILCODE_T	lChipAlSetConfig (
    IN VOID						* pAdapter,
    I_O	VOID					* pConfigArg);

LOCAL NTSTATUS lChipALLinkShutdown(PVOID pLink,
                                   DWORD ComChannel
                                  );

LOCAL NTSTATUS lChipALLinkDisable (
    PVOID				pAdapter,		// Adapter Instance
    DWORD				ComChannel		// Communications Channel
);

LOCAL NTSTATUS lChipALStartTx (
    PVOID		pLink,
    DWORD 		ComChannel
);
LOCAL NTSTATUS lChipALLinkEnable (
    PVOID				pAdapter,		// Adapter Instance
    DWORD				ComChannel	   	);

/*******************************************************************************
Public Functions
*******************************************************************************/

/******************************************************************************
FUNCTION NAME:
	ChipALCfgInit

ABSTRACT:
	Requirement of the Architecture to read tuning parameters from a Windowxxx 
	Registry


RETURN:
	NDIS_STATUS


DETAILS:
******************************************************************************/
GLOBAL NDIS_STATUS ChipALCfgInit (
    IN	CDSL_ADAPTER_T	*pThisAdapter,		// Driver supplied adapter context
    // (&ThisAdapter)  Required for events that
    // are not associated with a link
    IN	PTIG_USER_PARAMS pParams                // parameter struct from the download app
)
{
	#if CHANNEL_SIZE_BY_SPEED
	DWORD			BytesPerSecond;
	DWORD			BytesPerInterrupt;
	#endif

	ChipALDebugFlag = pThisAdapter->PersistData.DebugFlag;

	Tune.PhysicalBase = NULL_MEMORY;
	Tune.IRQ          = NULL_IRQ;

	Tune.RxMaxLatency = pParams->RxMaxLatency;
	Tune.TxMaxLatency = pParams->TxMaxLatency;
	Tune.RxMinLatency = pParams->RxMinLatency;
	Tune.TxMinLatency = pParams->TxMinLatency;
	Tune.RxInterruptRate = pParams->RxInterruptRate;
	Tune.TxInterruptRate = pParams->TxInterruptRate;
	Tune.RxSpeed         = pParams->RxSpeed;
	Tune.TxSpeed         = pParams->TxSpeed;
	Tune.TxFifoSize      = pParams->TxFifoSize;
	Tune.RxFifoSize      = pParams->RxFifoSize;

	Tune.RxChannelSize   = pParams->RxChannelSize;
	Tune.RxSegments      = pParams->RxSegments;
	Tune.TxChannelSize   = pParams->TxChannelSize;
	Tune.TxSegments      = pParams->TxSegments;

	Tune.TxFifoSize = FIFO_SIZE_TO_MASK(HOST_TO_DEVICE,Tune.TxFifoSize);

	// ensure there was a registry value else replace with default
	if ( !  pParams->RxChannelSizeCfg )
	{
		Tune.RxChannelSize = DEFAULT_RX_CHANNEL_SIZE;
	}
	if ( !  pParams->RxSegmentsCfg )
	{
		Tune.RxSegments = DEFAULT_RX_SEGMENT_NO;
	}
	if ( !  pParams->TxChannelSizeCfg )
	{
		Tune.TxChannelSize = DEFAULT_TX_CHANNEL_SIZE;
	}
	if ( !  pParams->TxSegmentsCfg )
	{
		Tune.TxSegments = DEFAULT_TX_SEGMENT_NO;
	}

	// Calculate Default Channel Size
	#if CHANNEL_SIZE_BY_SPEED
	// *
	// * ChannelSize and Segments are rounded to DWORDS.
	// *

	// *
	// * Calculate Receive Channel
	// *
	BytesPerSecond = Tune.RxSpeed / BITS_PER_BYTE;
	BytesPerInterrupt = (BytesPerSecond / (1000 / Tune.RxInterruptRate)) >> 2 << 2;
	Tune.RxChannelSize =(WORD) ((BytesPerSecond / (1000 / Tune.RxMaxLatency)) >> 2 << 2 );
	Tune.RxSegments = (WORD) (Tune.RxChannelSize / BytesPerInterrupt);

	// *
	// * Calculate Transmit Channel
	// *
	BytesPerSecond = Tune.TxSpeed / BITS_PER_BYTE;
	BytesPerInterrupt = (BytesPerSecond / (1000 / Tune.TxInterruptRate)) >> 2 << 2;
	Tune.TxChannelSize =(WORD) ((BytesPerSecond / (1000 / Tune.TxMaxLatency)) >> 2 << 2 );
	Tune.TxSegments = (WORD) (Tune.TxChannelSize / BytesPerInterrupt);

	#else
	// Tune.RxChannelSize = DMA_DEFAULT_CHANNEL_SIZE;
	// Tune.RxSegments = DMA_DEFAULT_SEGMENTS;
	// Tune.TxChannelSize = DMA_DEFAULT_CHANNEL_SIZE / 8;
	// Tune.TxSegments = 2;
	#endif

	Tune.TotalSharedMemory = Tune.RxChannelSize + Tune.TxChannelSize;
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, ChipALDebugFlag, ("Tx Channel Defaults Size: %d, Segments %d\n",Tune.TxChannelSize,Tune.TxSegments));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, ChipALDebugFlag, ("Rx Channel Defaults Size: %d, Segments %d\n",Tune.RxChannelSize,Tune.RxSegments));

	return(STATUS_SUCCESS);
}


/*******************************************************************************
Name:		ChipALAdapterInit
Function:	Create new Instance of an adapter
Description:
			  
			    
*******************************************************************************/
NDIS_STATUS ChipALAdapterInit (
    IN CDSL_ADAPTER_T					* CdslAdapter
)
{
	NDIS_STATUS Status = STATUS_FAILURE ;
	CHIPAL_T * pThisInterface;
	WORD Index,FunctSel;
	WORD EventIndex;
	CHIPAL_INF_T * pChipALRemoteInf=NULL;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALAdapterInit() ",
	    (CdslAdapter != (CDSL_ADAPTER_T *)0 )
	)

	Status = ALLOCATE_MEMORY((VOID*)&pChipALRemoteInf, sizeof(CHIPAL_INF_T), MEMORY_TAG );
	if (Status != STATUS_SUCCESS)
	{
		return(STATUS_FAILURE);
	}

	CLEAR_MEMORY(pChipALRemoteInf, sizeof(CHIPAL_INF_T));

	CdslAdapter->pChipALRemoteInf = pChipALRemoteInf;

	// Use the local ChipAL Interface.
	pChipALRemoteInf->CALIntf.pCALRead 					=  lChipALRead;
	pChipALRemoteInf->CALIntf.pCALWrite 				= lChipALWrite;
	//pChipALRemoteInf->CALIntf.pCalEnableInterrupt 		= lChipALEnableInterruptHandler;
	//pChipALRemoteInf->CALIntf.pCalDisableInterrupt 		= lChipALDisableInterruptHandler;
	pChipALRemoteInf->CALIntf.pCalAddEventHandler  		= lChipALAddEventHandler;
	pChipALRemoteInf->CALIntf.pCalRemoveEventHandler  	= lChipALRemoveEventHandler;
	pChipALRemoteInf->CALIntf.pCalSetEvent 				= lChipALSetEvent;

	pChipALRemoteInf->CALIntf.iface.Context = CdslAdapter;
	pChipALRemoteInf->ConfigInterface.iface.Context = CdslAdapter;
	pChipALRemoteInf->ConfigInterface.pCalGetConfig =  	lChipAlGetConfig;
	pChipALRemoteInf->ConfigInterface.pCalSetConfig = 	lChipAlSetConfig ;

	// The context for the link interface a link handle
	// the link handle is not available until LinkInit, and LinkInit will save
	// the link handle in the structure
	pChipALRemoteInf->LinkInterface.pCalLinkShutDown =	 lChipALLinkShutdown;
	pChipALRemoteInf->LinkInterface.pCalLinkEnable	 =	 lChipALLinkEnable;
	pChipALRemoteInf->LinkInterface.pCalLinkDisable	 =	 lChipALLinkDisable;
	pChipALRemoteInf->LinkInterface.pCalStartTx		 =	 lChipALStartTx;

	pThisInterface = chipALAllocateInterface();

	if (pThisInterface == NULL_CHIPAL_T)
	{
		return(STATUS_FAILURE);
	}
	// *
	// * Exchange SAP's
	// *
	pThisInterface->pNdisAdapter = CdslAdapter;
	pThisInterface->pNdisAdapter->ChipMod = pThisInterface;

	// *
	// * Initialize Event Handler List
	// *
	for (FunctSel = 0; FunctSel < NUM_FUNCTIONS; FunctSel++)
	{
		for (Index = 0; Index < HW_EVENT_TYPE_END ; Index ++)
		{
			for (EventIndex = 0; EventIndex < HW_EVENT_END ; EventIndex ++)
			{
				InitializeListHead ( &(pThisInterface->EventHandler[FunctSel][Index][EventIndex]));
			}
		}
	}

	// *
	// * Load Tuning Parameters
	// *
	pThisInterface->Tune = &Tune;

	// *
	// * Save External Event Handler
	// *
	pThisInterface->MgmtEventHandler = CardALChipUnknownIntrptEventHdlr;

	// *
	// * Load Bus Control Service
	// *
	pThisInterface->BusControl = &BUS_CONTROLLER_SERVICE;

	// *
	// * Allocate Bus Resources
	// *
	pThisInterface->BusResources = (* pThisInterface->BusControl->Init) (
	                                   pThisInterface->pNdisAdapter,
	                                   pThisInterface->Tune,
	                                   &MyAdapterCount);

	if(pThisInterface->BusResources == NULL_PCI_RECOURCES)
	{
		pThisInterface->pNdisAdapter->ChipMod =  NULL_CHIPAL_T;
		chipALReleaseInterface(pThisInterface);
		return(STATUS_FAILURE);
	}

	// *
	// * Load Reverse Bit Order Table
	// *
	for (Index = 0;Index < BYTE_SIZE ;++Index )
	{
		ReverseByteTable[Index] = REVERSE_BYTE(Index);
	}

	// *
	// *
	// * Load RegIo Handlers
	// *

	// Load Register I/O Controller Table
	COPY_MEMORY( &(pThisInterface->RegIoControl[0]),
	             &IoDevice[0],
	             sizeof(IoDevice));

	INIT_SPIN_LOCK(&pThisInterface->IsrLock);

	(* pThisInterface->BusControl->Enable)(pThisInterface->BusResources, ARM_FUNCTION, 0);
	(* pThisInterface->BusControl->Enable)(pThisInterface->BusResources, DSL_FUNCTION, 0);

	// *
	// * For each controller in the IoDevice table, call the RegIoContInit
	// * routine, if supplied.
	// *
	for (Index = 0; Index < MAX_HARDWARE_DEVICES ; Index ++)
	{
		INIT_SPIN_LOCK(&pThisInterface->RegIoLock[Index]);

		if (pThisInterface->RegIoControl[Index].RegIoContInit != NULL)
		{
			pThisInterface->RegIoContext[Index] =
			    (* pThisInterface->RegIoControl[Index].RegIoContInit) (
			        pThisInterface,
			        pThisInterface->RegIoControl[Index].FunctionSelect);

			// *
			// * Verify Controller Loaded Without Errors
			// *
			if (pThisInterface->RegIoContext[Index] == NULL)
			{
				CHIPAL_ASSERT_TRACE("Register IO Controller Load Failure",TRUE);
				ChipALAdapterHalt(CdslAdapter);
				return(STATUS_FAILURE);
			}
		}
	}

	// *
	// * Load Optional Services
	// *
	COPY_MEMORY( &(pThisInterface->Service[0]),
	             &OptionalServices[0],
	             sizeof(OptionalServices));

	#if CAL_SOFT_TIMER
	pThisInterface->CalTimerState = TMR_TIMING;

	INIT_TIMER( &pThisInterface->CalTimer, chipAlSoftTimer, (ULONG)pThisInterface );

	INIT_EVENT( &pThisInterface->CalTimerStoppedEvent ) ;

	START_TIMER( &pThisInterface->CalTimer, SOFT_TIMER_PERIOD );
	#endif

	// Return Adapter Count to Card Managment Adapter Structure
	CdslAdapter->InstanceNumber =  (UCHAR) MyAdapterCount;

	return(STATUS_SUCCESS);
}


/*******************************************************************************
Name:		ChipALInitServices
Function:	Initializes the adapter services
Description:
			  
			    
*******************************************************************************/
NDIS_STATUS ChipALInitServices(IN CDSL_ADAPTER_T		* CdslAdapter)
{
	int Index;
	CHIPAL_T * pThisInterface = (CHIPAL_T *)CdslAdapter->ChipMod ;

	// *
	// * For each service in the Service table, call the ServiceInit
	// * routine, if supplied.
	// *
	for (Index = 0; Index < MAX_CHIPAL_SERVICES ; Index ++)
	{
		if (pThisInterface->Service[Index].ServiceInit != NULL)
		{
			pThisInterface->ServiceContext[Index] =
			    (* pThisInterface->Service[Index].ServiceInit) (pThisInterface);

			// *
			// * Verify Controller Loaded Without Errors
			// *
			if (pThisInterface->ServiceContext[Index]== NULL)
			{
				CHIPAL_ASSERT_TRACE("ChipAl Service Load Failure",TRUE);
				ChipALAdapterHalt(CdslAdapter);
				return(STATUS_FAILURE);
			}
		}
	}

	return STATUS_SUCCESS;
}


/*******************************************************************************
Name:		ChipALAdapterHalt
Function:	Release all resources allocated by the Adapter
Description:
			  
			    
*******************************************************************************/
NDIS_STATUS ChipALAdapterHalt(IN CDSL_ADAPTER_T		* CdslAdapter)
{
	CHIPAL_T 			*pThisInterface;
	WORD				Index,FunctSel;
	WORD				EventIndex;
	CAL_EVENT_LIST_T	*ReleasedEvent;
	BOOLEAN				TimerCancelled;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALAdapterHalt() ",
	    (CdslAdapter != (CDSL_ADAPTER_T *) 0)
	)
	pThisInterface = (CHIPAL_T *) CdslAdapter->ChipMod;

	if (pThisInterface == NULL_CHIPAL_T)
	{
		return(STATUS_FAILURE);
	}

	#if CAL_SOFT_TIMER
	if ( pThisInterface->CalTimerState == TMR_TIMING )
	{
		pThisInterface->CalTimerState = TMR_CANCELLED;

		if ( ! WAIT_EVENT( &pThisInterface->CalTimerStoppedEvent, ONE_SEC ) )
		{
			DBG_CDSL_DISPLAY(
				DBG_LVL_MISC_INFO,
				ChipALDebugFlag,
				("ChipALAdapterHalt: ERROR  Wait on Timer timed out!\n"));

			// Timer is still running.  Try to Cancel...
			CANCEL_TIMER( &pThisInterface->CalTimer, &TimerCancelled ) ;

			// if still on queue - something is wrong
			if ( !TimerCancelled )
			{
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("pThisAdapter: ERROR;  Could not remove timer!\n"));
			}
		}
	}
	#endif
	// *
	// * Release all Event Handlers
	// *
	for (FunctSel = 0; FunctSel < NUM_FUNCTIONS; FunctSel++ )
	{
		for (Index = 0; Index < HW_EVENT_TYPE_END ; Index ++)
		{
			for (EventIndex = 0; EventIndex < HW_EVENT_END ; EventIndex ++)
			{
				while (!IsListEmpty(&(pThisInterface->EventHandler[FunctSel][Index][EventIndex])))
				{
					ReleasedEvent = (CAL_EVENT_LIST_T *) RemoveTailList (
					                    &(pThisInterface->EventHandler[FunctSel][Index][EventIndex]));

					FREE_MEMORY
					(
					    ReleasedEvent,
					    sizeof(CAL_EVENT_LIST_T),
					    CHIPAL_CONTEXT_MEM_FLAGS
					);
				}
			}
		}
	}

	// *
	// * Release Optional Services
	// *
	for (Index = 0; Index < MAX_CHIPAL_SERVICES ; Index ++)
	{
		if (pThisInterface->Service[Index].ServiceInit != NULL)
		{
			(* pThisInterface->Service[Index].ServiceShutdown) (pThisInterface->ServiceContext[Index]);
		}
	}

	// *
	// * Release all RegIo Controllers
	// *
	for (Index = 0; Index < MAX_HARDWARE_DEVICES ; Index++ )
	{
		if (pThisInterface->RegIoContext[Index] != NULL)
		{
			(* pThisInterface->RegIoControl[Index].RegIoShutdown) (
			    pThisInterface->RegIoContext[Index],
			    pThisInterface->RegIoControl[Index].FunctionSelect	) ;
		}
	}

	// *
	// * Release Bus Resources
	// *
	if (pThisInterface->BusResources != NULL_PCI_RECOURCES)
	{
		(* pThisInterface->BusControl->Release) (
		    CdslAdapter,
		    pThisInterface->BusResources);
	}

	// *
	// * Release Tuning Allocations
	// *
	#if PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
	if (pThisInterface->Tune->TxLoopBackFileA.MaximumLength > 0
	        && pThisInterface->Tune->TxLoopBackFileA.Buffer != NULL)
	{
		FREE_MEMORY
		(
		    pThisInterface->Tune->TxLoopBackFileA.Buffer,
		    pThisInterface->Tune->TxLoopBackFileA.MaximumLength,
		    CHIPAL_CONTEXT_MEM_FLAGS
		);
	}
	pThisInterface->Tune->TxLoopBackFileA.MaximumLength = 0;
	pThisInterface->Tune->TxLoopBackFileA.Buffer = NULL;

	if (pThisInterface->Tune->TxLoopBackFileB.MaximumLength > 0
	        && pThisInterface->Tune->TxLoopBackFileB.Buffer != NULL)
	{
		FREE_MEMORY
		(
		    pThisInterface->Tune->TxLoopBackFileB.Buffer,
		    pThisInterface->Tune->TxLoopBackFileB.MaximumLength,
		    CHIPAL_CONTEXT_MEM_FLAGS
		);
		pThisInterface->Tune->TxLoopBackFileB.MaximumLength = 0;
		pThisInterface->Tune->TxLoopBackFileB.Buffer = NULL;
	}

	if (pThisInterface->Tune->FpgaProgramFile.MaximumLength > 0
	        && pThisInterface->Tune->FpgaProgramFile.Buffer != NULL)
	{
		FREE_MEMORY
		(
		    pThisInterface->Tune->FpgaProgramFile.Buffer,
		    pThisInterface->Tune->FpgaProgramFile.MaximumLength,
		    CHIPAL_CONTEXT_MEM_FLAGS
		);
	}
	pThisInterface->Tune->FpgaProgramFile.MaximumLength = 0;
	pThisInterface->Tune->FpgaProgramFile.Buffer = NULL;

	#endif

	// *
	// * Release Interface
	// *
	pThisInterface->pNdisAdapter->ChipMod = NULL_CHIPAL_T;
	chipALReleaseInterface(pThisInterface);

	FREE_MEMORY(CdslAdapter->pChipALRemoteInf,sizeof(CHIPAL_REMOTE_INF), 0);

	MyAdapterCount--;

	return(STATUS_SUCCESS);

}


/*******************************************************************************
Name:		ChipALSetFirmGo
Function:	Set ARM FIRMGO bit in Control Reg
Description:
			  
			    
*******************************************************************************/
GLOBAL NTSTATUS ChipALSetFirmGo (
    IN CDSL_ADAPTER_T *	pThisAdapter )
{
	extern int						CnxtDslPhysicalDriverType;
	extern ARM_CONTROL_REGISTER_T	ArmControlRegister [DEVICE_TYPE_MAX] ;

	DWORD							Ctl_Val ;
	NTSTATUS						Status;
	
	//Ctl_Val = 0x40 ; //TIGRIS_CSR_CTRL_FIRMGO
	Ctl_Val = ArmControlRegister [CnxtDslPhysicalDriverType].CsrCtrlFirmGo ;

	Status = ChipALWrite(
	             pThisAdapter,
	             P46_ARM,
	             ADDRESS_ARM_BAR_0,
	             CSR_ARM_CTRL,
	             sizeof(DWORD),
	             Ctl_Val,
	             Ctl_Val
	         );

	return Status ;
}
/*******************************************************************************
Name:		ChipALEnableInterruptHandler
Function:	Enable Interrupts
Description:
			  
			    
*******************************************************************************/
GLOBAL VOID ChipALEnableInterruptHandler (IN CDSL_ADAPTER_T *ThisAdapter)
 {
 CHIPAL_T *pThisInterface;

 CHIPAL_ASSERT_TRACE("ChipALEnableInterruptHandler() ", (ThisAdapter != (CDSL_ADAPTER_T *) 0));

 pThisInterface = (CHIPAL_T *)ThisAdapter->ChipMod;

 if (pThisInterface == NULL_CHIPAL_T)
  {
  return;							//(CHIPAL_ERROR_FATAL(CHIPAL_PARAM_OUT_OF_RANGE));
  }
 else if (pThisInterface->BusControl == NULL)
  {
  return;							//(CHIPAL_ERROR_FATAL(CHIPAL_PARAM_OUT_OF_RANGE));
  }

 (* pThisInterface->BusControl->Enable)(pThisInterface->BusResources, ARM_FUNCTION, 0);
 (* pThisInterface->BusControl->Enable)(pThisInterface->BusResources, DSL_FUNCTION, 0);
 }

/*******************************************************************************
Name:		ChipALDisableInterruptHandler
Function:	Disable Interrupts
Description:
			  
			    
*******************************************************************************/
GLOBAL VOID ChipALDisableInterruptHandler (
    IN CDSL_ADAPTER_T *	ThisAdapter )
{
	CHIPAL_T 	* pThisInterface;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALDisableInterruptHandler() ",
	    (ThisAdapter != (CDSL_ADAPTER_T *) 0)
	)

	pThisInterface = (CHIPAL_T *) ThisAdapter->ChipMod;

	if (pThisInterface == NULL_CHIPAL_T)
	{
		return;							//(CHIPAL_ERROR_FATAL(CHIPAL_PARAM_OUT_OF_RANGE));
	}
	else if (pThisInterface->BusControl == NULL)
	{
		return;							//(CHIPAL_ERROR_FATAL(CHIPAL_PARAM_OUT_OF_RANGE));
	}

	(pThisInterface->BusControl->Disable)(pThisInterface->BusResources, ARM_FUNCTION, DISABLE_INTERRUPT);
	(pThisInterface->BusControl->Disable)(pThisInterface->BusResources, DSL_FUNCTION, DISABLE_INTERRUPT);
}


#if PROJECTS_NAME == PROJECT_NAME_TIGRIS
/******************************************************************************
FUNCTION NAME:
	ChipALLinkInit		(PROJECT_NAME_TIGRIS)

ABSTRACT:
	Open Full Duplex Data Stream


RETURN:
	ChipAL FailCode


DETAILS:
	This is a Project Specific Routine.  Legacy  Versions are at the end of
	this file.
******************************************************************************/
GLOBAL DWORD ChipALLinkInit (
    CDSL_ADAPTER_T *	CdslAdapter,	// Adapter Instance
    CDSL_LINK_T *		CdslLink,		// Link Instance
    DWORD				Speed,			// Link Speed in BPS
    DWORD				ComChannel,		// Communications Channel
    BYTE				Mode			// Framed or NonFramed
)
{
	CHIPAL_T 	* pThisInterface;
	HW_CH_ENTRY_T ChanConfig;
	DWORD TxChannel;
	DWORD RxChannel;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALLinkInit() ",
	    (CdslAdapter != (CDSL_ADAPTER_T *) 0 &&
	     CdslAdapter->ChipMod != NULL_CHIPAL_T )
	)


	#ifdef DBG
	TraceQueueIdx = 0;
	IntCount = 0;
	#endif

	pThisInterface = (CHIPAL_T *) CdslAdapter->ChipMod;

	if (pThisInterface == NULL_CHIPAL_T || ComChannel < MIN_COM_CHANNEL || ComChannel >= MAX_COM_CHANNEL )
	{
		return(CHIPAL_ERROR_FATAL(CHIPAL_PARAM_OUT_OF_RANGE));
	}

	CdslAdapter->pChipALRemoteInf->LinkInterface.iface.Context = 	CdslLink;

	// Map FullDuplex Communications Channel to DMA Channel(s)
	switch (ComChannel)
	{
	case ADSL_CHANNEL:
		TxChannel = ADSL_DMA_CHAN_TX;
		RxChannel = ADSL_DMA_CHAN_RX;

		break;

	default:
		return(CHIPAL_ERROR_FATAL(CHIPAL_INVALID_PARAM));
		break;

	}

	// *
	// * Load DMA Control Service
	// *
	pThisInterface->DmaControl = &DMA_CHAN_CONTROLLER;


	// *
	// * Configure and Allocate Transmit Channel
	// *
	ChanConfig.UserLink = CdslLink;
	ChanConfig.Description.ElUnion.Element.Direction = HOST_TO_DEVICE;

	ChanConfig.ProcessData = FrameALGetTxFullBuffer;
	ChanConfig.TxComplete =  FrameALPutTxEmptyBuffer;
	ChanConfig.SegmentSize = (WORD) ( (pThisInterface->Tune->TxChannelSize / pThisInterface->Tune->TxSegments)
	                                  >> 2 << 2);
	ChanConfig.NumberOfSegments = pThisInterface->Tune->TxSegments;
	ChanConfig.Description.ElUnion.Element.Channel = TxChannel;
	ChanConfig.Description.ElUnion.Element.Event = HW_TX_DMA_COMPLETE;

	pThisInterface->DmaChannel[TxChannel] = (* pThisInterface->DmaControl->ChanOpen) (pThisInterface, &ChanConfig);

	if (pThisInterface->DmaChannel[TxChannel] == NULL_HW_CH_ENTRY )
	{
		return(CHIPAL_ERROR_FATAL(CHIPAL_RESOURCE_ALLOCATION_FAILURE));
	}

	// *
	// * Allocate Receive Channel
	// *
	ChanConfig.ProcessData = FrameALPutRxFullBuffer;
	ChanConfig.TxComplete =  NULL;
	ChanConfig.Description.ElUnion.Element.Direction = DEVICE_TO_HOST;
	ChanConfig.SegmentSize =  (WORD) ((pThisInterface->Tune->RxChannelSize / pThisInterface->Tune->RxSegments)
	                                  >> 2 << 2 );
	ChanConfig.NumberOfSegments = pThisInterface->Tune->RxSegments;
	ChanConfig.Description.ElUnion.Element.Channel = RxChannel;
	ChanConfig.Description.ElUnion.Element.Event = HW_RX_DMA_COMPLETE;

	pThisInterface->DmaChannel[RxChannel] = (* pThisInterface->DmaControl->ChanOpen) (pThisInterface, &ChanConfig);

	if (pThisInterface->DmaChannel[RxChannel] == NULL_HW_CH_ENTRY )
	{
		return(CHIPAL_ERROR_FATAL(CHIPAL_RESOURCE_ALLOCATION_FAILURE));
	}

	return(CHIPAL_SUCCESS);
}
#endif



/******************************************************************************
FUNCTION NAME:
	lChipALLinkShutdown

ABSTRACT:
	Close Full Duplex Datastream.  If a Null link is supplied, ALL chipal
	datastreams will be closed.


RETURN:
	ChipAL FailCode

DETAILS:
******************************************************************************/
LOCAL NTSTATUS lChipALLinkShutdown(PVOID pLink,
                                   DWORD ComChannel
                                  )
{
	CDSL_LINK_T * pThisLink=(CDSL_LINK_T *)pLink;
	CHIPAL_T 	* 	pThisInterface = NULL_CHIPAL_T;
	SHORT			Index;
	SHORT			ActiveChannels = 0;

	SHORT			LinkReleased = 0;


	// *
	// * Find Chipal Adapter Context
	// *
	if (pThisLink == (CDSL_LINK_T *) 0 )
	{
		// *
		// * Link was not supplied.  Use last adapter instance if loaded.
		// *
		pThisInterface = pChipALHead;
		while (pThisInterface->Next != NULL_CHIPAL_T)
		{
			pThisInterface = pThisInterface->Next;
		}


		if (pThisInterface == NULL_CHIPAL_T)
		{
			return(CHIPAL_ERROR_FATAL(CHIPAL_PARAM_OUT_OF_RANGE));
		}
	}
	else if ( pThisLink->pThisAdapter == (CDSL_ADAPTER_T *) 0 )
	{
		return(CHIPAL_ERROR_FATAL(CHIPAL_PARAM_OUT_OF_RANGE));
	}
	else if ( (CHIPAL_T 	*) pThisLink->pThisAdapter->ChipMod == NULL_CHIPAL_T)
	{
		return(CHIPAL_ERROR_FATAL(CHIPAL_PARAM_OUT_OF_RANGE));

	}

	// *
	// * Get Chipal Adapter Context from Supplied link
	// *
	if (pThisInterface == NULL_CHIPAL_T)
	{
		pThisInterface = (CHIPAL_T 	*) pThisLink->pThisAdapter->ChipMod;
	}

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALLinkShutdown() ",
	    (pThisInterface != NULL_CHIPAL_T)
	)

	// *
	// * Find and release all Links associated with a Hardware Channel
	// *
	for (Index = 0; Index < MAX_DATA_CHANNEL ; Index++ )
	{
		if (pThisInterface->DmaChannel[Index] != NULL_HW_CH_ENTRY)
		{
			ActiveChannels++;
			if (pThisLink == pThisInterface->DmaChannel[Index]->UserLink
			        || pThisLink == (CDSL_LINK_T *) 0)
			{
				// Disable Interrupts/Channel and Release Allocated Memory
				(* pThisInterface->DmaControl->ChanClose) (pThisInterface->DmaChannel[Index]);
				LinkReleased++;
				pThisInterface->DmaChannel[Index] = NULL_HW_CH_ENTRY;
			}
		}
	}

	if (LinkReleased)
	{
		return(CHIPAL_SUCCESS);
	}
	else
	{
		return( CHIPAL_ERROR_FATAL(CHIPAL_INVALID_PARAM) );
	}
}

#if CAL_REVISION >= 1
/******************************************************************************
FUNCTION NAME:
	lChipALLinkEnable

ABSTRACT:
	Disable Full Duplex Data Link


RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
LOCAL NTSTATUS lChipALLinkEnable    (
    PVOID			pLink,
    DWORD			ComChannel
)
{
	CDSL_LINK_T *		CdslLink=(CDSL_LINK_T *)pLink;

	CHIPAL_T 	* pThisInterface;

	CDSL_ADAPTER_T * CdslAdapter = CdslLink->pThisAdapter;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALLinkEnable() ",
	    (CdslAdapter != (CDSL_ADAPTER_T *) 0 &&
	     CdslAdapter->ChipMod != NULL_CHIPAL_T )
	)

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALLinkEnable() ",
	    (CdslAdapter != (CDSL_ADAPTER_T *) 0 &&
	     CdslAdapter->ChipMod != NULL_CHIPAL_T )
	)

	pThisInterface = (CHIPAL_T *) CdslAdapter->ChipMod;

	switch (ComChannel)
	{
	case ADSL_CHANNEL:
		if(pThisInterface->DmaChannel[ADSL_DMA_CHAN_TX] == NULL
		        || pThisInterface->DmaChannel[ADSL_DMA_CHAN_RX] == NULL)
		{
			return(-EINVAL);
		}
		(* pThisInterface->DmaControl->ChanEnable) ( pThisInterface->DmaChannel[ADSL_DMA_CHAN_TX]);
		(* pThisInterface->DmaControl->ChanEnable) ( pThisInterface->DmaChannel[ADSL_DMA_CHAN_RX]);

		break;


	default:
		return(-EINVAL);
		break;

	}
	return(CHIPAL_SUCCESS);
}
#endif	// CAL_REVISION



#if CAL_REVISION >= 1
/******************************************************************************
FUNCTION NAME:
	lChipALLinkDisable

ABSTRACT:
	Enable Full Duplex Data Link


RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
LOCAL NTSTATUS lChipALLinkDisable (
    PVOID		pLink,	// Adapter Instance
    DWORD		ComChannel		// Communications Channel
)
{
	CDSL_LINK_T *		CdslLink=(CDSL_LINK_T *)pLink;	// Adapter Instance

	CHIPAL_T 	* pThisInterface;

	CDSL_ADAPTER_T * CdslAdapter = CdslLink->pThisAdapter;


	CHIPAL_ASSERT_TRACE
	(
	    "ChipALLinkDIsable() ",
	    (CdslAdapter != (CDSL_ADAPTER_T *) 0 &&
	     CdslAdapter->ChipMod != NULL_CHIPAL_T )
	)

	#ifdef DBG
	TraceQueueIdx = 0;
	IntCount = 0;
	#endif

	pThisInterface = (CHIPAL_T *) CdslAdapter->ChipMod;

	switch (ComChannel)
	{
	case ADSL_CHANNEL:
		if(pThisInterface->DmaChannel[ADSL_DMA_CHAN_TX] == NULL
		        || pThisInterface->DmaChannel[ADSL_DMA_CHAN_RX] == NULL)
		{
			return(-EINVAL);
		}
		(* pThisInterface->DmaControl->ChanDisable) ( pThisInterface->DmaChannel[ADSL_DMA_CHAN_TX]);
		(* pThisInterface->DmaControl->ChanDisable) ( pThisInterface->DmaChannel[ADSL_DMA_CHAN_RX]);

		break;

	default:
		return(-EINVAL);
		break;

	}
	return(CHIPAL_SUCCESS);
}
#endif

/******************************************************************************
FUNCTION NAME:
	lChipALStartTx

ABSTRACT:
	 Restart the ChipAL Transmit process

RETURN:
	ChipAL Fail Code


DETAILS:
	The ChipAL Transmit function "Pulls" data from the protocol stack. 
	Normally, this is done based on a Hardware Interrupt.  In the case
	where the stack does not have any data to deliver data to the transmitter, 
	the transmit process will terminate.  As a result, a function is required 
	to restart the transmit cycle.  The cycle will again continue until the 
	protocol stack fails to deliver data when requested.
******************************************************************************/
LOCAL NTSTATUS lChipALStartTx (
    PVOID pLink,
    DWORD ComChannel
)
{
	CDSL_LINK_T * pCdslLink=(CDSL_LINK_T *)pLink;
	CHIPAL_T 			* pThisInterface;	// ChipAL Adapter Instance
	HW_CH_ENTRY_T		* pTxChannelLink;	// ChipAL Link Instance
	CHAN_DESCRIPT_T		TxChannelDes;		// ChipAL Link Description
	CHAN_DESCRIPT_T		SearchMask;			// ChipAL Link Description Mask Value
	LIST_ENTRY			* pHeadList;
	CAL_EVENT_LIST_T	*pEventHandler=NULL;

	pThisInterface = (CHIPAL_T *) pCdslLink->pThisAdapter->ChipMod;

	if (pThisInterface == NULL_CHIPAL_T)
	{
		return(CHIPAL_ERROR_FATAL(CHIPAL_PARAM_OUT_OF_RANGE));
	}

	// Clear all Elements in Mask
	SearchMask.ElUnion.Value = 0L;

	// Clear all Elements in Description
	TxChannelDes.ElUnion.Value = 0L;

	// Indicate a search for the Transmit Channel
	TxChannelDes.ElUnion.Element.Direction = HOST_TO_DEVICE;

 	#if PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
	TxChannelDes.ElUnion.Element.Port = PIO_DATA;
	SearchMask.ElUnion.Element.Port = PIO_DATA;
	#endif


	// *
	// * We only care about the Direction and Port fields
	// * of the CHAN_DESCRIPT_T Structure
	// *
	SearchMask.ElUnion.Element.Direction = TRUE;

	pTxChannelLink = chipALFindLink (
	                     pThisInterface,
	                     pCdslLink,
	                     TxChannelDes,
	                     SearchMask.ElUnion.Value );

	// *
	// * Verify we were able to locate the Transmit Channel
	// *
	if (pTxChannelLink == NULL_HW_CH_ENTRY)
	{
		return(CHIPAL_ERROR_FATAL(CHIPAL_PARAM_OUT_OF_RANGE));
	}

	// *
	// * If defined, Call the transmit Data handler.  This would normally be a result
	// * of an ISR / DPC call.
	// *

	// *
	// * Locate Head of the Data Events DPC call back list
	// *
	pHeadList = &pThisInterface->EventHandler[DSL_FUNCTION][HW_DPC_EVENT][pTxChannelLink->Description.ElUnion.Element.Event];

	pEventHandler = (CAL_EVENT_LIST_T *) pHeadList->pNext;

	// *
	// * Check for any attached DPC routine(s) responsible for processing
	// * an the data event on this link.  If no routines are assigned, an
	// * error is not recorded.  This would only happen if the link were not
	// * initialized, which is the responsibility of the caller.
	// *
	while ( (LIST_ENTRY *) pEventHandler != pHeadList)
	{
		if (pEventHandler->EventHandler != NULL)
		{
			#if	MAX_RESPONSE_SIZE		
			// *  DPC Routines Reads Params. They should be considered Constant Values
			(*pEventHandler->EventHandler)(pEventHandler->UserContext,
			                               &pThisInterface->ResponseParams[pTxChannelLink->Description.ElUnion.Element.Event]);
			#else
(*pEventHandler->EventHandler)(pEventHandler->UserContext);
			#endif
		}

		pEventHandler = (CAL_EVENT_LIST_T *) pEventHandler->HandlerList.pNext;
	}
	return STATUS_SUCCESS;
}

/*******************************************************************************
Name:		HandleBus
Function:	Handle irq for bus if appropriate
Description:
*******************************************************************************/
static int HandleBus(CHIPAL_T *pThisInterface, DWORD FunctSel, struct tasklet_struct *t)
{
	int handled = 0;
	DWORD InterruptChan;

	/* Read Interrupt Status and Mask registers */
	(*pThisInterface->BusControl->GetStatus)(pThisInterface->BusResources, FunctSel, &InterruptChan);

	/* Determine if this interrupt is for our device and if it requires bottom half processing */
	if (InterruptChan) {
		if (chipALClearInterrupt(pThisInterface, FunctSel, InterruptChan))
			tasklet_schedule(t);
		handled = 1;
	}

	return handled;
}

/*******************************************************************************
Name:		ChipALIsrHandler
Function:	Interrupt Service Routine (Device IRQL)
Description:
*******************************************************************************/
irqreturn_t ChipALIsrHandler(IN int irq, IN PVOID pAdapter)
{
	CDSL_ADAPTER_T *pThisAdapter = (CDSL_ADAPTER_T*)pAdapter;
	CHIPAL_T *pThisInterface = (CHIPAL_T*)pThisAdapter->ChipMod;
	irqreturn_t retval = IRQ_NONE;

	if (pThisInterface->BusResources!=NULL_PCI_RECOURCES) {
		if (HandleBus(pThisInterface, ARM_FUNCTION, &pThisAdapter->ARMInterruptQueStr))
			retval = IRQ_HANDLED;
		if (HandleBus(pThisInterface, DSL_FUNCTION, &pThisAdapter->DSLInterruptQueStr))
			retval = IRQ_HANDLED;
	}

	return retval;
}

/*******************************************************************************
Name:		ChipALDSLIsrDPCHandler
Function:	Handler Interrupt Event
Description:
			  
			    
*******************************************************************************/
VOID ChipALDSLIsrDPCHandler (	IN PVOID pAdapter )
{
	CDSL_ADAPTER_T *		ThisAdapter=(CDSL_ADAPTER_T *)pAdapter;
	CHIPAL_T			* pThisInterface;
	BUS_RESOURCE_T 		        * pBusResource;
	DWORD				InterruptReg[NUM_FUNCTIONS][HW_MAX_EVENT_REGISTERS];
	WORD				Index;
	BOOL				InterruptFound;

	#if DBG_DPC_TRACE
	DWORD						RegValue;
	#endif

	TRACE_MESSAGE(DPC_ENTRY)

#ifdef PIM_DEBUG
if (irqs_disabled())
  printk(KERN_ALERT "CnxADSL: ChipALDSLIsrDPCHandler IRQs disabled?\n");
#endif

	// *
	// * Get Adapter/Interface Context
	// *
	if (ThisAdapter->ChipMod ==  NULL_CHIPAL_T )
	{
		return;
	}
	pThisInterface = (CHIPAL_T *) ThisAdapter->ChipMod;

	// *
	// * Make sure BusResources are defined and Interrupts are Initialized.
	// *
	if ( pThisInterface->BusResources == NULL_PCI_RECOURCES )
	{
		return;
	}
	else if ( pThisInterface->BusResources->InterruptsInitialized == FALSE )
	{
		return;
	}

	pBusResource = pThisInterface->BusResources;


	do
	{
		DWORD LockFlag;

		// *
		// * Enter Critical Section.
		// *

		// Disable interrupts and protect against other threads - MultiProcessor
		ACQUIRE_LOCK(&pThisInterface->IsrLock,LockFlag);

		InterruptFound = FALSE;
		for (Index = 0;Index < HW_MAX_EVENT_REGISTERS; Index++)
		{
			//Get a Copy of the Interrupt Status Register
			InterruptReg[DSL_FUNCTION][Index] = pThisInterface->InterruptStatus[DSL_FUNCTION][Index];

			if (pThisInterface->InterruptStatus[DSL_FUNCTION][Index])
			{
				//Clear Static Interrupt Value - (Only the bits copied)
				InterruptFound = TRUE;
				pThisInterface->InterruptStatus[DSL_FUNCTION][Index] &= ~InterruptReg[DSL_FUNCTION][Index];
			}
		}

		RELEASE_LOCK(&pThisInterface->IsrLock,LockFlag);

		// *
		// * Exit Critical Section
		// *

		// *
		// * Process Interrupts. NOTE: InterruptReg is a static copy of the actual
		// * interrupt Status register.  Therefore, if an interrupt occurs while
		// * we are processing this group of interrupts, it is not a problem.
		// *
		if (InterruptFound)
		{
			chipALProcessInterrupt(pThisInterface, pBusResource, DSL_FUNCTION, InterruptReg[DSL_FUNCTION]);
		}

	}while (InterruptFound);

	TRACE_MESSAGE(DPC_EXIT)

	return;
}

/*******************************************************************************
Name:		ChipALARMIsrDPCHandler
Function:	Handler Interrupt Event
Description:
			  
			    
*******************************************************************************/
VOID ChipALARMIsrDPCHandler (	IN PVOID pAdapter )
{
	CDSL_ADAPTER_T *	ThisAdapter=(CDSL_ADAPTER_T *)pAdapter;
	CHIPAL_T			* pThisInterface;
	BUS_RESOURCE_T 		        * pBusResource;
	DWORD				InterruptReg[NUM_FUNCTIONS][HW_MAX_EVENT_REGISTERS];
	WORD				Index;
	BOOL				InterruptFound;

	#if DBG_DPC_TRACE
	DWORD						RegValue;
	#endif

	TRACE_MESSAGE(DPC_ENTRY)

	// *
	// * Get Adapter/Interface Context
	// *
	if (ThisAdapter->ChipMod ==  NULL_CHIPAL_T )
	{
		return;
	}
	pThisInterface = (CHIPAL_T *) ThisAdapter->ChipMod;

	// *
	// * Make sure BusResources are defined and Interrupts are Initialized.
	// *
	if ( pThisInterface->BusResources == NULL_PCI_RECOURCES )
	{
		return;
	}
	else if ( pThisInterface->BusResources->InterruptsInitialized == FALSE )
	{
		return;
	}

	pBusResource = pThisInterface->BusResources;


	do
	{
		DWORD LockFlag;

		// *
		// * Enter Critical Section.
		// *

		// Disable interrupts and protect against other threads - MultiProcessor
		ACQUIRE_LOCK(&pThisInterface->IsrLock,LockFlag);

		InterruptFound = FALSE;

		for (Index = 0;Index < HW_MAX_EVENT_REGISTERS; Index++)
		{
			//Get a Copy of the Interrupt Status Register
			InterruptReg[ARM_FUNCTION][Index] = pThisInterface->InterruptStatus[ARM_FUNCTION][Index];

			if (pThisInterface->InterruptStatus[ARM_FUNCTION][Index])
			{
				//Clear Static Interrupt Value - (Only the bits copied)
				InterruptFound = TRUE;
				pThisInterface->InterruptStatus[ARM_FUNCTION][Index] &= ~InterruptReg[ARM_FUNCTION][Index];
			}
		}

		RELEASE_LOCK(&pThisInterface->IsrLock,LockFlag);

		// *
		// * Exit Critical Section
		// *

		// *
		// * Process Interrupts. NOTE: InterruptReg is a static copy of the actual
		// * interrupt Status register.  Therefore, if an interrupt occurs while
		// * we are processing this group of interrupts, it is not a problem.
		// *
		if (InterruptFound)
		{
			chipALProcessInterrupt(pThisInterface, pBusResource, ARM_FUNCTION, InterruptReg[ARM_FUNCTION]);
		}

	}while (InterruptFound);

	TRACE_MESSAGE(DPC_EXIT)

	return;

}
/******************************************************************************
FUNCTION NAME:
	ChipALSetInformation

ABSTRACT:
	Requirment of the Project Architecture to process NDIS OIDs


RETURN:
	NDIS_STATUS


DETAILS:
******************************************************************************/
GLOBAL NDIS_STATUS ChipALSetInformation(
    IN CDSL_ADAPTER_T	* pThisAdapter,
    IN PVOID			InfoBuffer,
    IN ULONG			InfoBufferLength,
    OUT PULONG			BytesWritten,
    OUT PULONG			BytesNeeded )
{
	//	return NDIS_STATUS_INVALID_OID;
	return STATUS_FAILURE;
}

/******************************************************************************
FUNCTION NAME:
	ChipALRead

ABSTRACT:
	Perform a Device write


RETURN:
	CHIPAL_FAILCODE_T


DETAILS:
	The Read request is mapped to the responsible Register I/O control module.
	If a module does not require any special functions, the vector table
	entry will be mapped to a default memory Read routine.
******************************************************************************/
LOCAL CHIPAL_FAILCODE_T  lChipALRead (
    VOID				* pAdapter,
    HARDWARE_DEVICE_T	HwDevice,
    DWORD				LogicalDevice,
    DWORD				OffsetAddress,
    DWORD				RegisterLength,
    PVOID				pVal )
{
	CHIPAL_T			* pThisInterface;
	CHIPAL_FAILCODE_T	ReturnValue;
	DWORD				BaseAddress;
	CDSL_ADAPTER_T		* CdslAdapter=(CDSL_ADAPTER_T*)pAdapter;
	DWORD volatile		* pValue =(DWORD volatile *)pVal;

	pThisInterface = (CHIPAL_T *) CdslAdapter->ChipMod;

	switch(LogicalDevice)
	{
	case ADDRESS_BAR_0:
		BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressBAR0;
			#ifdef DBG
		if(	 OffsetAddress >= pThisInterface->BusResources->LenBar0
		        || (void *)BaseAddress == NULL )
		{
			return(-EINVAL);
		}
			#endif
		break;

	case ADDRESS_BAR_1:
		BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressBAR1;
			#ifdef DBG
		if(	 OffsetAddress >= pThisInterface->BusResources->LenBar1
		        || (void *)BaseAddress == NULL )
		{
			return(-EINVAL);
		}
			#endif
		break;

	case ADDRESS_ARM_BAR_0:
		BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressARMBAR0;
			#ifdef DBG
		if(	 OffsetAddress >= pThisInterface->BusResources->LenARMBar0
		        || (void *)BaseAddress == NULL )
		{
			return(-EINVAL);
		}
			#endif
		break;

	case  REMOTE_ADDRESS_BAR_2:
		BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressARMBAR0;
		break;

	default:
		return(CHIPAL_PARAM_OUT_OF_RANGE);
		break;
	}

	ReturnValue = (* pThisInterface->RegIoControl[HwDevice].RegIoRead) (
	                  pThisInterface->RegIoContext[HwDevice],
	                  (PVOID)BaseAddress,
	                  OffsetAddress,
	                  RegisterLength,
	                  (void*)pValue);

	return(ReturnValue);

}


/******************************************************************************
FUNCTION NAME:
	ChipALWrite

ABSTRACT:
	Perform a Device write


RETURN:
	CHIPAL_FAILCODE_T


DETAILS:
	The write request is mapped to the responsible Register I/O control module.
	If a module does not require any special functions, the vector table
	entry will be mapped to a default memory write routine.
******************************************************************************/
LOCAL CHIPAL_FAILCODE_T  lChipALWrite (
    VOID				* pAdapter,
    HARDWARE_DEVICE_T	HwDevice,
    DWORD				LogicalDevice,
    DWORD				OffsetAddress,
    DWORD				RegisterLength,
    DWORD				Value,
    DWORD				Mask)
{
	CDSL_ADAPTER_T		* CdslAdapter=(CDSL_ADAPTER_T*)pAdapter;
	CHIPAL_T		* pThisInterface;
	CHIPAL_FAILCODE_T	ReturnValue;
	DWORD			BaseAddress;
	DWORD                   FunctSel;
	DWORD			LockFlag=0;

	pThisInterface = (CHIPAL_T *) CdslAdapter->ChipMod;

	switch(LogicalDevice)
	{
	case ADDRESS_BAR_0:
		BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressBAR0;
		FunctSel = DSL_FUNCTION;
//		DBG_CDSL_DISPLAY(
//			DBG_LVL_MISC_INFO,
//			ChipALDebugFlag,
//			("CnxADSL  command add bar 0 %lx",(DWORD)BaseAddress));
			#ifdef DBG
		if(	 OffsetAddress >= pThisInterface->BusResources->LenBar0
		        || (void *)BaseAddress == NULL )
		{
			return(-EINVAL);
		}
			#endif
		break;

	case ADDRESS_BAR_1:
		BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressBAR1;
//		DBG_CDSL_DISPLAY(
//			DBG_LVL_MISC_INFO,
//			ChipALDebugFlag,
//			("CnxADSL  command add bar 1 %lx",(DWORD)BaseAddress));
		FunctSel = DSL_FUNCTION;
			#ifdef DBG
		if(	 OffsetAddress >= pThisInterface->BusResources->LenBar1
		        || (void *)BaseAddress == NULL )
		{
			return(-EINVAL);
		}
			#endif
		break;

	case ADDRESS_ARM_BAR_0:
		BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressARMBAR0;
//		DBG_CDSL_DISPLAY(
//			DBG_LVL_MISC_INFO,
//			ChipALDebugFlag,
//			("CnxADSL  command add arm bar 0 %lx",(DWORD)BaseAddress));
		FunctSel = ARM_FUNCTION;
			#ifdef DBG
		if(	 OffsetAddress >= pThisInterface->BusResources->LenARMBar0
		        || (void *)BaseAddress == NULL )
		{
			DBG_CDSL_DISPLAY(
				DBG_LVL_GENERIC,
				DBG_FLAG_DONT_CARE,
				(	"CnxADSL invalid offset %lx  from %lx",
					(DWORD)OffsetAddress,
					(DWORD)BaseAddress));
			return(-EINVAL);
		}
			#endif
		break;

	case  REMOTE_ADDRESS_BAR_2:
		FunctSel = ARM_FUNCTION;
//		DBG_CDSL_DISPLAY(
//			DBG_LVL_MISC_INFO,
//			ChipALDebugFlag,
//			("CnxADSL  command add remote bar 0 %lx",(DWORD)BaseAddress));
		BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressARMBAR0;
		break;

	default:
//		DBG_CDSL_DISPLAY(
//			DBG_LVL_MISC_INFO,
//			ChipALDebugFlag,
//			("CnxADSL device out of range",(DWORD)BaseAddress));
		return(CHIPAL_PARAM_OUT_OF_RANGE);
		break;
	}

	// *
	// * If a Mask value is present, this is a Read+Modify+Write cycle.  Therefore,
	// * it will be the beginning of a CRITICAL section.  All Interrupts must be
	// * Disabled.
	// *
	if (Mask)
	{
		ACQUIRE_LOCK(&pThisInterface->IsrLock,LockFlag);
	}

//	DBG_CDSL_DISPLAY(
//		DBG_LVL_MISC_INFO,
//		ChipALDebugFlag,
//		("CnxADSL issue command",(DWORD)BaseAddress));
	ReturnValue = (* pThisInterface->RegIoControl[HwDevice].RegIoWrite) (
	                  pThisInterface->RegIoContext[HwDevice],
	                  (PVOID)BaseAddress,
	                  OffsetAddress,
	                  RegisterLength,
	                  Mask,
	                  (void*)Value);

	// *
	// * If Mask, End Critical Section.
	// *
	if (Mask)
	{
		RELEASE_LOCK(&pThisInterface->IsrLock,LockFlag);
	}

	#if BUS_BURST_NOT_ALLOWED
	{
		volatile DWORD		RegisterValue;	// Accesses to this variable get optimized
		// out without the keyword volatile

		// *
		// * This is a 'DUMMY' read.  The read operation prevents two back to back
		// * writes, which may could lead to a burst mode operation.
		// *
		RegisterValue = *((DWORD *)(BaseAddress + BASIC2_STATUS));
	}
	#endif

	return(ReturnValue);

}


/******************************************************************************
FUNCTION NAME:
	ChipALReadSync

ABSTRACT:
	Perform	synchronous	Device read.  

RETURN:
	CHIPAL_FAILCODE_T


DETAILS:
	This routine will stay in a	dead loop until	the	device becomes ready.

******************************************************************************/
GLOBAL CHIPAL_FAILCODE_T  ChipALReadSync (
    CDSL_ADAPTER_T			* CdslAdapter,
    HARDWARE_DEVICE_T		HwDevice,
    DWORD					LogicalDevice,
    DWORD					OffSetAddress,
    DWORD					RegisterLength,
    DWORD 					* pValue )
{
	CHIPAL_FAILCODE_T		ReturnValue;

	CHIPAL_T				* pThisInterface;
	#ifdef DBG
	DWORD					MaxLoop	= 0;
	#endif
	DWORD					BaseAddress;
	DWORD					LockFlag;

	pThisInterface = (CHIPAL_T *) CdslAdapter->ChipMod;

	switch(LogicalDevice)
	{
	case ADDRESS_BAR_0:
		BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressBAR0;
		break;
	case ADDRESS_BAR_1:
		BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressBAR1;
		break;
	case ADDRESS_ARM_BAR_0:
		BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressARMBAR0;
		break;
	default:
		return(CHIPAL_PARAM_OUT_OF_RANGE);
		break;
	}

	if (BaseAddress == 0L )
	{
		return(-EINVAL);
	}

	ACQUIRE_LOCK(&pThisInterface->RegIoLock[HwDevice],LockFlag);

	// *
	// *	Wait for device	to become ready
	// *
	do
	{
		ReturnValue	= (* pThisInterface->RegIoControl[HwDevice].RegIoRead)	(
		                  pThisInterface->RegIoContext[HwDevice],
		                  (PVOID)BaseAddress,
		                  OffSetAddress,
		                  RegisterLength,
		                  pValue
		              );

		#ifdef DBG
		if(MaxLoop++ > MAX_SYNC_WAIT)
		{
			ReturnValue	= -EBUSY;
			break;
		}
		#endif
	}
	while(ReturnValue == -ENOTREADY);

	RELEASE_LOCK(&pThisInterface->RegIoLock[HwDevice],LockFlag);

	return(ReturnValue);

}


/******************************************************************************
FUNCTION NAME:
	ChipALWriteSync

ABSTRACT:
	Perform	synchronous	Device write.  This	in requires	the	completion of the
	write cycle	with a STATUS_SUCCESS or a fail	code other than	-EBUSY,
	or STATUS_DEVICE_PENDING.


RETURN:
	CHIPAL_FAILCODE_T


DETAILS:
	This routine will stay in a	dead loop until	the	device becomes ready.

******************************************************************************/
/*
GLOBAL CHIPAL_FAILCODE_T  ChipALWriteSync (
	CDSL_ADAPTER_T			* CdslAdapter,
	HARDWARE_DEVICE_T		HwDevice,
	DWORD					LogicalDevice,
	DWORD					OffSetAddress,
	DWORD					RegisterLength,
	VOID					* UserValue,
	DWORD					MaskValue)
{
	CHIPAL_FAILCODE_T		ReturnValue;

	CHIPAL_T				* pThisInterface;
	VOID					* pRegister;
	DWORD					RegisterValue;
	DWORD					BaseAddress;
	DWORD                   LockFlag;

	CHIPAL_DEVICE_STATUS_T	DeviceStatus;
	#ifdef DBG
	DWORD					MaxLoop	= 0;
	#endif

	pThisInterface = (CHIPAL_T *) CdslAdapter->ChipMod;

	switch(LogicalDevice)
	{
		case ADDRESS_BAR_0:
			BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressBAR0;
			break;
		case ADDRESS_BAR_1:
			BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressBAR1;
			break;
		case ADDRESS_ARM_BAR_1:
			BaseAddress =(DWORD) pThisInterface->BusResources->VirtualAddressARMBAR1;
			break;
		default:
			return(CHIPAL_PARAM_OUT_OF_RANGE);
			break;
	}

	if (BaseAddress == 0L )
	{
		return(-EINVAL);
	}

	ACQUIRE_LOCK(&pThisInterface->RegIoLock[HwDevice], LockFlag);

	// *	
	// *	Wait for device	to become ready
	// *
	do
	{
		ReturnValue	= (* pThisInterface->RegIoControl[HwDevice].RegIoWrite) (
			pThisInterface->RegIoContext[HwDevice],
			BaseAddress,
			OffSetAddress,
			RegisterLength,
			MaskValue,
			UserValue );
		#ifdef DBG
		if(MaxLoop++ > MAX_SYNC_WAIT)
		{
			ReturnValue	= -EBUSY;
			break;
		}
		#endif
	} 
	while(ReturnValue == -EBUSY);

	RELEASE_LOCK(&pThisInterface->RegIoLock[HwDevice], LockFlag);

	if (ReturnValue == STATUS_PENDING)
	{
		return(STATUS_SUCCESS);
	}
	return(ReturnValue);
	
}
*/

/******************************************************************************
FUNCTION NAME:
	ChipALService

ABSTRACT:
	Entry Point for Optional Chipal Services


RETURN:
	CHIPAL_FAILCODE_T


DETAILS:
	
******************************************************************************/
GLOBAL CHIPAL_FAILCODE_T ChipALService (
    CDSL_ADAPTER_T		* pThisAdapter,
    CHIPAL_SERVICE_T	Service,
    DWORD				Function,
    CAL_MODULE_PARAMS_T	* Parameter
)
{
	CHIPAL_T			* pThisInterface;
	CHIPAL_FAILCODE_T	Results;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALService() ",
	    (
	        pThisAdapter != (CDSL_ADAPTER_T *) 0
	        && (Service >=MIN_CHIPAL_SERVICES && Service < MAX_CHIPAL_SERVICES)
	        && (Function >=MIN_CHIPAL_SERVICES_FN && Function  < MAX_CHIPAL_SERVICES_FN)
	        && Parameter != NULL
	    )
	)

	// * Critical Entry Point.  Must also do Runtime check.
	if	(!(
	            pThisAdapter != (CDSL_ADAPTER_T *) 0
	            && (Service >=MIN_CHIPAL_SERVICES && Service < MAX_CHIPAL_SERVICES)
	            && (Function >=MIN_CHIPAL_SERVICES_FN && Function  < MAX_CHIPAL_SERVICES_FN)
	            && Parameter != NULL
	        ))
	{
		return(CHIPAL_ERROR_FATAL(CHIPAL_INVALID_PARAM));
	}

	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod;
	Parameter->Context = pThisInterface->ServiceContext[Service];

	switch (Function)
	{
	case SVC_COMMAND:
		Results = (* pThisInterface->Service[Service].ServiceCommand)(Parameter);
		break;

	case SVC_CONFIGURE:
		Results = (* pThisInterface->Service[Service].ServiceConfigure)(Parameter);
		break;

	default:
		Results = CHIPAL_ERROR_FATAL(CHIPAL_INVALID_PARAM);
		break;
	}
	return(Results);

}

/******************************************************************************
FUNCTION NAME:
	ChipALAddEventHandler

ABSTRACT:
	Add Event Handler for any ChipAL Event (Interrupt).  This allows
	external entities to attach an Event Handler to specific Chipal Event.  The
	Handler may be called at ISR or DPC time.

RETURN:
	ChipAL Fail Code:
		CHIPAL_SUCCESS - Event Registered Successfully
		CHIPAL_RESOURCE_ALLOCATION_FAILURE - Allocate Memory Failed
		CHIPAL_MODULE_NOT_INITIALIZED - ChipAl is not Initialized
DETAILS:
******************************************************************************/
LOCAL CHIPAL_FAILCODE_T lChipALAddEventHandler (
    IN PVOID					pAdapter,		// Driver Adapter Instance
    IN DWORD					FunctSel,       // selects arm or dsl function
    IN CHIPAL_EVENT_T			Event,			// Enumerated Event to Set
    IN HW_EVENT_TYPE_T			IrqLevel,		// When Event should be called - DIRQl or Dispatch IRQL
    IN CHIPAL_EVENT_HANDLER_T	UserEventHandler,// Function to Call
    IN void						* UserContext	// Function Context - Unused by ChipAL
)
{
	IN CDSL_ADAPTER_T 		* CdslAdapter=(CDSL_ADAPTER_T*)pAdapter;
	CHIPAL_T 				* pThisInterface;
	NDIS_STATUS Status =	 STATUS_FAILURE ;
	CAL_EVENT_LIST_T		* NewHandler=NULL;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALAddEventHandler() ",
	    (CdslAdapter != (CDSL_ADAPTER_T *) 0 &&
	     Event >= HW_EVENT_BEGIN &&
	     Event < HW_EVENT_END &&
	     HW_EVENT_END == HW_MAX_EVENTS &&
	     UserEventHandler != NULL &&
	     IrqLevel >= HW_EVENT_TYPE_BEGIN  &&
	     IrqLevel <  HW_EVENT_TYPE_END )
	)

	// *
	// * Get ChipAL Context
	// *
	pThisInterface = (CHIPAL_T *) CdslAdapter->ChipMod;
	if (pThisInterface == NULL_CHIPAL_T )
	{
		return(CHIPAL_ERROR_FATAL(CHIPAL_MODULE_NOT_INITIALIZED));
	}

	// *
	// * Allocate Memory for New Event Handler
	// *

	Status = ALLOCATE_MEMORY( (VOID*)&NewHandler, sizeof(CAL_EVENT_LIST_T), MEMORY_TAG );

	if (Status != STATUS_SUCCESS)
	{
		return(CHIPAL_ERROR_FATAL(CHIPAL_RESOURCE_ALLOCATION_FAILURE));
	}
	CLEAR_MEMORY(NewHandler, sizeof(CAL_EVENT_LIST_T));

	// *
	// * Load Handler Structure and Add to Event's Linked List of Handlers
	// *
	NewHandler->EventHandler = UserEventHandler;
	NewHandler->UserContext	= UserContext;

	InsertHeadList (
	    &(pThisInterface->EventHandler[FunctSel][IrqLevel][Event]),
	    &NewHandler->HandlerList);

	return(CHIPAL_SUCCESS);
}

/******************************************************************************
FUNCTION NAME:
	ChipALRemoveEventHandler

ABSTRACT:
	Remove Event Handler for Any  ChipAL Event (Interrupt)


RETURN:
	ChipAL Fail Code:
		CHIPAL_SUCCESS - Handler Found and Released
		CHIPAL_MODULE_NOT_INITIALIZED - ChipAL is not Initialized
		CHIPAL_PARAM_OUT_OF_RANGE - - Could Not locate indicated handler.


DETAILS:
******************************************************************************/
LOCAL CHIPAL_FAILCODE_T lChipALRemoveEventHandler (
    IN PVOID					pAdapter,		// Driver Adapter Instance
    IN DWORD					FunctSel,       // selects the ARM or DSL function
    IN CHIPAL_EVENT_T			Event,			// Event to Remove
    IN HW_EVENT_TYPE_T			IrqLevel,		// When Event was being called
    IN CHIPAL_EVENT_HANDLER_T	UserEventHandler// Handler to remove
)
{
	CDSL_ADAPTER_T 			* CdslAdapter=(CDSL_ADAPTER_T*)pAdapter;
	CHIPAL_T 				* pThisInterface;
	LIST_ENTRY				* HandlerEntry;
	BOOL					FoundHandler = FALSE;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALRemoveEventHandler() ",
	    (CdslAdapter != (CDSL_ADAPTER_T *) 0 &&
	     Event >= HW_EVENT_BEGIN &&
	     Event < HW_EVENT_END &&
	     UserEventHandler != NULL&&
	     IrqLevel >= HW_EVENT_TYPE_BEGIN  &&
	     IrqLevel <  HW_EVENT_TYPE_END )
	)

	// *
	// * Get ChipAL Context
	// *
	pThisInterface = (CHIPAL_T *) CdslAdapter->ChipMod;
	if (pThisInterface == NULL_CHIPAL_T )
	{
		return(CHIPAL_ERROR_FATAL(CHIPAL_MODULE_NOT_INITIALIZED));
	}

	// Get First Entry on Event's Linked List
	HandlerEntry = pThisInterface->EventHandler[FunctSel][IrqLevel][Event].pNext;

	// *
	// * Search All Handlers on List for the Indicated User Handler
	// *
	while ( HandlerEntry !=
	        &pThisInterface->EventHandler[FunctSel][IrqLevel][Event] )
	{
		if (((CAL_EVENT_LIST_T *)HandlerEntry)->EventHandler == UserEventHandler )
		{
			RemoveListEntry(HandlerEntry);

			FREE_MEMORY
			(
			    HandlerEntry,
			    sizeof(CAL_EVENT_LIST_T),
			    CHIPAL_CONTEXT_MEM_FLAGS
			);

			FoundHandler = TRUE;
			break;
		}
		else
		{
			HandlerEntry = HandlerEntry->pNext;
		}
	}

	if (FoundHandler)
	{
		return(CHIPAL_SUCCESS);
	}
	else
	{
		return(CHIPAL_ERROR_WARNING(CHIPAL_PARAM_OUT_OF_RANGE));
	}
}

/******************************************************************************
FUNCTION NAME:
	ChipALSetEvent

ABSTRACT:
	Convert an Enumerated event to a Register / Event index. Use these
	indexes to set a bit in the static interrupt event register that is 
	processed at DPC time.

RETURN:
	void

DETAILS:
	Optionally, if the event is an event that was
	decoded from a bit in the real hardware register, a bit will be set in
	the ISR event register that is currently being processed.  Therefore,
	any decoded event set in this manner should be numerically greater than the
	event that was used in the event decode process.  For efficiency, there 
	are no checks for this potential problem.

	NOTE: 
	This code is expected to run in an ISR routine.  For optimization, it is 
	an INLINE procedure. In a release version, Events will NOT be range checked.
	Passing invalid parameters will certainly cause a crash.
******************************************************************************/
LOCAL void lChipALSetEvent (
    PVOID				pAdapter,
    DWORD			    FunctSel,
    CHIPAL_EVENT_T		Event)
{
	CDSL_ADAPTER_T 		* pThisAdapter=(CDSL_ADAPTER_T*)pAdapter;
	HW_EVENT_INDEX_T	Index ;
	CHIPAL_T			* pThisInterface;

	Index.EvntUnion.Value  = (DWORD) Event;

	#if VERIFY_EVENTS
	if ( !(Event >= HW_EVENT_BEGIN &&
	        Event < HW_EVENT_END &&
	        pThisAdapter != NULL &&
	        pThisAdapter->ChipMod != NULL))
	{
		return;
	}
	#endif

	pThisInterface = (CHIPAL_T *)pThisAdapter->ChipMod;

	// Set Bit in DPC structure to allow deferred processing
	pThisInterface->InterruptStatus[FunctSel][Index.EvntUnion.Bits.Register] |=
	    1 << Index.EvntUnion.Bits.Event;

	// *
	// * If bit is not reflected in the real hardware register, post an event
	// * to the IsrEvent structure.
	// *
	if (Index.EvntUnion.Bits.Register != HARDWARE_INTERRUPT_REGISTER )
	{
		pThisInterface->IsrEvent[FunctSel][Index.EvntUnion.Bits.Register] |=
		    1 << Index.EvntUnion.Bits.Event;
	}
}

/******************************************************************************
FUNCTION NAME:
	ChipAlGetConfig

ABSTRACT:
	Get Current Configuration of ChipAl Module


RETURN:
	CHIPAL_FAILCODE_T - Fail Code as defined by Called Module;
	CHIPAL_MODULE_NOT_INITIALIZED - ChipAl is not Initialized
	CHIPAL_SERVICE_UNAVAILABLE - Called Module does not support GetConfig Function


DETAILS:
	
******************************************************************************/
LOCAL	CHIPAL_FAILCODE_T	lChipAlGetConfig (
    IN  VOID				* pAdapter,	// Adapter Instance
    I_O	VOID				* pConf)
{
	CHIPAL_T *		pThisInterface;
	CDSL_ADAPTER_T 		* CdslAdapter=(CDSL_ADAPTER_T*)pAdapter;
	CAL_CONFIG_T		* Config = (CAL_CONFIG_T*)pConf;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipAlGetConfig() ",
	    (CdslAdapter != (CDSL_ADAPTER_T *) 0 &&
	     Config != NULL_CAL_CONFIG)
	)

	// *
	// * Get ChipAL Context
	// *
	pThisInterface = (CHIPAL_T *) CdslAdapter->ChipMod;
	if (pThisInterface == NULL_CHIPAL_T )
	{
		return(CHIPAL_ERROR_FATAL(CHIPAL_MODULE_NOT_INITIALIZED));
	}

	switch (Config->ModuleType)
	{
	case MODULE_TYPE_BUS_CONTROLLER:
		if (pThisInterface->BusControl->GetConfig != (PF_BUS_GET_CONFIG)0 )
		{
			return (* pThisInterface->BusControl->GetConfig)(CdslAdapter,&Config->Params);
		}
		else
		{
			return(CHIPAL_ERROR_WARNING(CHIPAL_SERVICE_UNAVAILABLE));
		}
		break;

	case MODULE_TYPE_IO_CONTROLLER:
	case MODULE_TYPE_DMA_CONTROLLER:
	case MODULE_SERVICE:
	default:
		return(CHIPAL_ERROR_WARNING(CHIPAL_SERVICE_UNAVAILABLE));

		break;

	}

}


/******************************************************************************
FUNCTION NAME:
	ChipAlSetConfig

ABSTRACT:
	Set Current Configuration of ChipAl Module


RETURN:
	CHIPAL_FAILCODE_T - Fail Code as defined by Called Module;
	CHIPAL_MODULE_NOT_INITIALIZED - ChipAl is not Initialized
	CHIPAL_SERVICE_UNAVAILABLE - Called Module does not support GetConfig Function


DETAILS:
	
******************************************************************************/
LOCAL	CHIPAL_FAILCODE_T	lChipAlSetConfig (
    IN  VOID	 		* pAdapter,	// Adapter Instance
    I_O	VOID			* pConf)
{
	CHIPAL_T			* pThisInterface;
	CDSL_ADAPTER_T 		* CdslAdapter=(CDSL_ADAPTER_T*)pAdapter;
	CAL_CONFIG_T		* Config=(CAL_CONFIG_T*)pConf;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipAlGetConfig() ",
	    (CdslAdapter != (CDSL_ADAPTER_T *) 0 &&
	     Config != NULL_CAL_CONFIG)
	)

	// *
	// * Get ChipAL Context
	// *
	pThisInterface = (CHIPAL_T *) CdslAdapter->ChipMod;
	if (pThisInterface == NULL_CHIPAL_T )
	{
		return(CHIPAL_ERROR_FATAL(CHIPAL_MODULE_NOT_INITIALIZED));
	}

	switch (Config->ModuleType)
	{
	case MODULE_TYPE_BUS_CONTROLLER:
		if (pThisInterface->BusControl->SetConfig != (PF_BUS_SET_CONFIG)0 )
		{
			return (* pThisInterface->BusControl->GetConfig)(CdslAdapter,&Config->Params);
		}
		else
		{
			return(CHIPAL_ERROR_WARNING(CHIPAL_SERVICE_UNAVAILABLE));
		}
		break;

	case MODULE_TYPE_IO_CONTROLLER:
	case MODULE_TYPE_DMA_CONTROLLER:
	case MODULE_SERVICE:
	default:
		return(CHIPAL_ERROR_WARNING(CHIPAL_SERVICE_UNAVAILABLE));

		break;

	}

}


#if CAL_SOFT_TIMER
/******************************************************************************
FUNCTION NAME:
	chipAlSoftTimer

ABSTRACT:
	Provides Software timer when Hardware timer is not available. This timer
	will call all functions attachted to HW_GENERAL_TIMER_A.  The period of
	the timer is determined by SOFT_TIMER_PERIOD.


RETURN:
	VOID


DETAILS:
******************************************************************************/
VOID chipAlSoftTimer(
    IN ULONG	pInterf
)
{
	CHIPAL_T			* pThisInterface=(CHIPAL_T*)pInterf;
	CAL_EVENT_LIST_T	* pEventHandler;
	DWORD 				CurrentEvent = 0;
	BOOL				EventHandled;
	LIST_ENTRY			* pHeadList;
	static BYTE Temp = 0;
	static DWORD Count = 0;
	if(pThisInterface == NULL)
	{
		return;
	}

	if ( pThisInterface->CalTimerState != TMR_TIMING )
	{
		// Tell Halt ok to proceed - we have exited
		SET_EVENT( &pThisInterface->CalTimerStoppedEvent ) ;
		return;
	}

	switch (Temp)
	{
	case 1:
		ChipALLinkEnable(pThisInterface->pNdisAdapter,ADSL_CHANNEL);
		Temp = 0;
		break;
	case 2:
		ChipALLinkDisable(pThisInterface->pNdisAdapter,ADSL_CHANNEL);
		Temp = 0;
		break;
	case 3:
		++Count;
		if(Count == 1)
		{
			ChipALLinkDisable(pThisInterface->pNdisAdapter,ADSL_CHANNEL);
		}
		if(Count == 200)
		{
			ChipALLinkEnable(pThisInterface->pNdisAdapter,ADSL_CHANNEL);
			Count=0;
		}
		break;
	case 4:
		++Count;
		if(Count == 500)
		{
			ChipALLinkDisable(pThisInterface->pNdisAdapter,ADSL_CHANNEL);
		}
		else if(Count == 1000)
		{
			ChipALLinkEnable(pThisInterface->pNdisAdapter,ADSL_CHANNEL);
			Count = 0;
		}
		break;
	}


	pHeadList = &pThisInterface->EventHandler[DSL_FUNCTION][HW_DPC_EVENT][HW_GENERAL_TIMER_A];
	pEventHandler = (CAL_EVENT_LIST_T *) pHeadList->pNext;

	EventHandled = FALSE;

	while ( (LIST_ENTRY *) pEventHandler != pHeadList)
	{
		EventHandled = TRUE;
		if (pEventHandler->EventHandler != NULL)
		{
			#if	MAX_RESPONSE_SIZE		// ChipAl supplies Response Param(s) to ISR / DPC Events
			// *  DPC Routines Reads Params. They should be considered Constant Values
			(*pEventHandler->EventHandler)(pEventHandler->UserContext, &pThisInterface->ResponseParams[CurrentEvent]);
			#else
(*pEventHandler->EventHandler)(pEventHandler->UserContext);
			#endif
		}
		pEventHandler = (CAL_EVENT_LIST_T *) pEventHandler->HandlerList.pNext;
	}

	START_TIMER( &pThisInterface->CalTimer, SOFT_TIMER_PERIOD  ) ;

}
#endif


/******************************************************************************
FUNCTION NAME:
	chipALReadMemory

ABSTRACT:
	Perform Memory Mapped I/O Register Read


RETURN:
	CHIPAL_FAILCODE_T


DETAILS:
******************************************************************************/
MODULAR INLINE CHIPAL_FAILCODE_T chipALReadMemory (
    void				* RegIoContext,
    PVOID				BaseAddress,
    DWORD				OffSetAddress,
    DWORD				RegisterLength,
    void 				* pReturnValue )
{
	void 				* pSourceRegister;

	if (BaseAddress == 0L || RegisterLength == 0)
	{
		return(CHIPAL_PARAM_OUT_OF_RANGE);
	}

	pSourceRegister = (void *) (BaseAddress + OffSetAddress);

	// *
	// * Read IO Register
	// *
	switch (RegisterLength)
	{
	case sizeof(BYTE):
					*( (BYTE *) pReturnValue) = *((BYTE *) pSourceRegister);
		break;
	case sizeof(WORD):
					*( (WORD *) pReturnValue) = *((WORD *) pSourceRegister);
		break;
	case sizeof(DWORD):
					*( (DWORD *) pReturnValue) = *((DWORD *) pSourceRegister);
		break;
	}
	return(CHIPAL_SUCCESS);

}

/******************************************************************************
FUNCTION NAME:
	chipALWriteMemory

ABSTRACT:
	Perform Memory Mapped I/O Register Write


RETURN:
	CHIPAL_FAILCODE_T


DETAILS:
******************************************************************************/
MODULAR INLINE CHIPAL_FAILCODE_T chipALWriteMemory (
    void				* RegIoContext,
    PVOID				BaseAddress,
    DWORD				OffSetAddress,
    DWORD				RegisterLength,
    DWORD				MaskValue,
    void*				pValue )
{
	DWORD Value = (DWORD)pValue;
	DWORD	RegisterValue = 0l;
	void	* pRegister;

	pRegister = (void *) (BaseAddress + OffSetAddress);

	// If Critical Section Applies, add Disable Interrupt code.

	// *
	// * Determine if Write is Bitmapped or if complete value should be written
	// *
	if (MaskValue)
	{
		// *
		// * Read IO Register
		// *
		switch (RegisterLength)
		{
		case sizeof(BYTE):
						RegisterValue = (DWORD) *((BYTE *) pRegister);
			break;
		case sizeof(WORD):
						RegisterValue = (DWORD) *((WORD *) pRegister);
			break;
		case sizeof(DWORD):
						RegisterValue = *((DWORD *) pRegister);
			break;
		}
		RegisterValue &= ~MaskValue;
		RegisterValue |= MaskValue & Value;
		Value = RegisterValue;
	}

	// *
	// * Write IO Register
	// *
	switch (RegisterLength)
{
	case sizeof(BYTE):
					*((BYTE *) pRegister) = (BYTE) Value;
		break;
	case sizeof(WORD):
					*((WORD *) pRegister) = (WORD) Value;
		break;
	case sizeof(DWORD):
					*((DWORD *) pRegister) = (DWORD) Value;
		break;
	}

	// End Critical Section

	return(CHIPAL_SUCCESS);
}

/******************************************************************************
FUNCTION NAME:
	chipALFindLink

ABSTRACT:
	Locate a ChipAL Link based on a Cdsl Link


RETURN:
	Pointer to the ChipAL Link


DETAILS:
	A CDSL_LINK_T has a one to many relationship to the ChipAL Link structue.
	First, all possible ChipAL Links must be located.  Next, the 
	CHAN_DESCRIPT_T union is used to determine which link is required. The Mask
	value is used to determine which elements of the description should be 
	examined.

	A static LastIndex value is kept as an attempt to avoid a linear search.
	This should improve performance if the same link is being sought.

******************************************************************************/
LOCAL HW_CH_ENTRY_T * chipALFindLink (
    CHIPAL_T 			* pThisInterface,
    CDSL_LINK_T			* pCdslLink,
    CHAN_DESCRIPT_T		Description,
    DWORD				Mask )
{
	DWORD			LinkIndex;
	static DWORD	LastIndex =0;
	BOOL			FoundEntry = FALSE;

	if (pThisInterface == NULL_CHIPAL_T)
	{
		return(NULL_HW_CH_ENTRY);
	}

	// *
	// * Checked cached Index entry in an attempt to avoid a linear search
	// *
	if (pThisInterface->DmaChannel[LastIndex] != NULL_HW_CH_ENTRY)
	{
		if
		(	( (pThisInterface->DmaChannel[LastIndex]->Description.ElUnion.Value & Mask)
		        == Description.ElUnion.Value)
		        && pThisInterface->DmaChannel[LastIndex]->UserLink == pCdslLink
		)
		{
			FoundEntry = TRUE;
		}
	}

	// *
	// * Check All Possible ChipAL Link structures
	// *

	for
	(
	    LinkIndex = 0;
	    LinkIndex < MAX_DATA_CHANNEL && FoundEntry == FALSE;
	    LinkIndex++
	)
	{
		if (pThisInterface->DmaChannel[LinkIndex] == NULL_HW_CH_ENTRY)
		{
			continue;
		}

		// *
		// * If the specified description elements and the requested link match,
		// * return the context for this link.
		// *
		if
		( 	( (pThisInterface->DmaChannel[LinkIndex]->Description.ElUnion.Value & Mask)
		        == Description.ElUnion.Value)
		        && pThisInterface->DmaChannel[LinkIndex]->UserLink == pCdslLink
		)
		{
			LastIndex = LinkIndex;
			FoundEntry = TRUE;
			break;
		}
	}

	if (FoundEntry == TRUE)
	{
		return (pThisInterface->DmaChannel[LastIndex]);
	}
	else
	{
		return(NULL_HW_CH_ENTRY);
	}
}


/******************************************************************************
FUNCTION NAME:
	chipALClearInterrupt

ABSTRACT:
	Clear Interrupt and any device specific causes of the interrupt. Any bit 
	that is set will be saved in a static copy of the Interrupt Status array
	(pThisInterface->InterruptStatus[]). This is a cumulative value that is 
	updated during each ISR call.  The value will be processed and cleared 
	in the DPC routine.

	NOTE:  Isr routines called from this procedure are processed in order.  
	if ChipALSetEvent() is called 

RETURN:
	void


DETAILS:
******************************************************************************/
LOCAL INLINE BOOLEAN chipALClearInterrupt (
    CHIPAL_T	* pThisInterface,
    DWORD		FunctSel,
    DWORD		HardWareInterrupt)
{
	//DWORD 				ProcessedInterrupts[HW_MAX_EVENT_REGISTERS];
	DWORD 				RegisterIndex;
	DWORD 				EventIndex;
	DWORD 				Mask;
	CAL_EVENT_LIST_T	* pEventHandler;
	DWORD				CurrentEvent;
	LIST_ENTRY			* pHeadList;
	BOOLEAN             DPCRequired=FALSE;

	// *
	// * Update static copy of Interrupt Status register. Any bit set in
	// * InterruptStatus will result in a call to a channel specific
	// * handler in the DPC procedure.  Any special case mapping of the
	// * channel handler to interrupt status is handled in this routine.
	// *
	// * Note: InterruptStatus is cleared in the DPC
	// *
	(pThisInterface->BusControl->Disable)(pThisInterface->BusResources, FunctSel, DISABLE_INTERRUPT);

	// Copy the hardware interrupt register to the Interrupt Event structure (array).
	pThisInterface->IsrEvent[FunctSel][HARDWARE_INTERRUPT_REGISTER] = HardWareInterrupt;
	
	// *
	// * Call Bus Controller to clear Interrupt Status bit(s).
	// *
	(*pThisInterface->BusControl->ClearStatus)(pThisInterface->BusResources,FunctSel,HardWareInterrupt);

	// *
	// * Process all event array elements
	// *
	for (RegisterIndex = 0; RegisterIndex < HW_MAX_EVENT_REGISTERS  ; RegisterIndex++ )
	{
		if (pThisInterface->IsrEvent[FunctSel][RegisterIndex] == 0)
			continue;
			
		// *
		// * Process All Bits in an event array element
		// *
		for( EventIndex = 0;
		        (EventIndex < BITS_PER_EVENT_REG) &&
		        (pThisInterface->IsrEvent[FunctSel][RegisterIndex]);
		        EventIndex++ )
		{
			Mask = 1 << EventIndex ;

			// *
			// * If Bit is Set, Check for ISR Event Handlers
			// *
			if ( Mask & pThisInterface->IsrEvent[FunctSel][RegisterIndex])
			{
				// Encode Register Indexes into an Event
				CurrentEvent =BITS_PER_EVENT_REG * RegisterIndex + EventIndex;

				pHeadList = &pThisInterface->EventHandler[FunctSel][HW_ISR_EVENT][CurrentEvent];
				pEventHandler = (CAL_EVENT_LIST_T *) pHeadList->pNext;

				// *
				// * Check for any attached ISR routines responsible for clearing
				// * an interrupt.  If on or more are found, they must decode the
				// * event and decide which DPC level or secondary ISR event(s)
				// * should be posted.
				// *
				while ( (LIST_ENTRY *) pEventHandler != pHeadList)
				{
					if (pEventHandler->EventHandler != NULL)
					{
						// Call Event Handler
						#if	MAX_RESPONSE_SIZE   // ChipAl supplies Response Param(s) to ISR / DPC Events

						// * ISR routine Decodes Event and Writes Response Params to the appropriate
						// * ResponseRegister.  DPC Reads Params as Constant Values
						(*pEventHandler->EventHandler)(pEventHandler->UserContext, &pThisInterface->ResponseParams[0]);
						#else
						(*pEventHandler->EventHandler)(pEventHandler->UserContext);
						#endif
					}

					// Get Next Entry on the ISR Handler List
					pEventHandler = (CAL_EVENT_LIST_T *) pEventHandler->HandlerList.pNext;
				}

				// *
				// * Save Bit to allow DPC processing
				// *
				pHeadList = &pThisInterface->EventHandler[FunctSel][HW_DPC_EVENT][CurrentEvent];
				if (pHeadList->pNext != pHeadList )
				{
					ChipALSetEvent(pThisInterface->pNdisAdapter, FunctSel, CurrentEvent);
					DPCRequired = TRUE;
				}

				// *
				// * Clear Processed bit to allow early exit of loop
				// *
				pThisInterface->IsrEvent[FunctSel][RegisterIndex] &= ~Mask;

				if (pThisInterface->IsrEvent[FunctSel][RegisterIndex] == 0)
				{
					// All Interrupts processed	in current register. Go to next register.
					break;
				}
			}								// If Mask
		}									// For EventIndex
	}										// For RegisterIndex

	(pThisInterface->BusControl->Enable)(pThisInterface->BusResources, FunctSel, 0);

	return DPCRequired;
}

/*******************************************************************************
Name:		chipALProcessInterrupt
Function:	Decode Interrupt and Do and Create Interrupt Descriptor
Description:
			  
			    
*******************************************************************************/
INLINE void chipALProcessInterrupt (
    CHIPAL_T		* pThisInterface,
    BUS_RESOURCE_T	* pBusResource,
    DWORD			  FunctSel,
    DWORD 			* Interrupt )
{
	DWORD 				Mask;
	DWORD				RegisterIndex;
	DWORD				EventIndex;
	CAL_EVENT_LIST_T	* pEventHandler;
	DWORD 				CurrentEvent;
	BOOL				EventHandled;
	LIST_ENTRY			* pHeadList;
	#ifdef DBG
	static DWORD		UnhandledEvents = 0;
	#endif

	for (RegisterIndex = 0; RegisterIndex < HW_MAX_EVENT_REGISTERS  ; RegisterIndex++ )
	{
		if ( Interrupt[RegisterIndex] )
		{
			// *
			// * Process All Bits in the Event Register
			// *
			for(EventIndex = 0; EventIndex < BITS_PER_EVENT_REG; EventIndex++ )
			{
				Mask = 1 << EventIndex ;

				// *
				// * If Bit is Set, Check for ISR Event Handlers
				// *
				if ( Mask & Interrupt[RegisterIndex])
				{
					// Encode Register Indexes into an Event
					CurrentEvent = BITS_PER_EVENT_REG * RegisterIndex + EventIndex;

					#ifdef DBG
					pThisInterface->IntStats[CurrentEvent].InterruptCount++;
					#endif

					// *
					// * Execute Channel Handler if Defined
					// *
					pHeadList = &pThisInterface->EventHandler[FunctSel][HW_DPC_EVENT][CurrentEvent];
					pEventHandler = (CAL_EVENT_LIST_T *) pHeadList->pNext;

					// *
					// * Check for any attached DPC routines responsible for processing
					// * an interrupt.
					// *
					EventHandled = FALSE;
					while ( (LIST_ENTRY *) pEventHandler != pHeadList)
					{
						EventHandled = TRUE;
						if (pEventHandler->EventHandler != NULL)
						{
							#if	MAX_RESPONSE_SIZE		// ChipAl supplies Response Param(s) to ISR / DPC Events
							// *  DPC Routines Reads Params. They should be considered Constant Values
							(*pEventHandler->EventHandler)(pEventHandler->UserContext, &pThisInterface->ResponseParams[CurrentEvent]);
							#else
							(*pEventHandler->EventHandler)(pEventHandler->UserContext);
							#endif
						}
						pEventHandler = (CAL_EVENT_LIST_T *) pEventHandler->HandlerList.pNext;
					}

					if (EventHandled == FALSE)
					{
						// *
						// * This is an Enabled Interrupt that does not have a Channel
						// * Event Handler.  Call the Management Handler if available.
						// * Otherwise, Ignore - Take NO ACTION.
						// *
						if (pThisInterface->MgmtEventHandler != NULL)
						{
							#if REPORT_UNHADLED_EVENTS
							(*pThisInterface->MgmtEventHandler)
							(pThisInterface->pNdisAdapter, CurrentEvent);
							#endif

							#ifdef DBG
							++UnhandledEvents;
							#endif
						}
					}

					// Clear current interrupt bit to allow early exit of processing loop
					Interrupt[RegisterIndex] &= ~Mask;
					if (Interrupt[RegisterIndex] == 0)
					{
						//No more Interrupt bits set
						break;
					}
				}							// If Mask & Interrupt
			}								// For EventIndex
		}
	}
	// For RegisterIndex
}


/*******************************************************************************
Name:		chipALAllocateInterface
Function:	Add New Interface to  Linked List
Description:
			  
			    
*******************************************************************************/
INLINE CHIPAL_T * chipALAllocateInterface(void)
{
	NDIS_STATUS Status = STATUS_FAILURE ;
	CHIPAL_T * pThisInterface;

	CHIPAL_ASSERT_TRACE
	(
	    "chipALAllocateInterface() ",
	    TRUE
	)

	// *
	// * First Time Setup
	// *
	if (pChipALHead == NULL_CHIPAL_T)
	{
		pChipALHead = &ChipALHead;
	}

	// *
	// * Locate New Hardware InterfaceContext
	// *
	pThisInterface = pChipALHead;
	while (pThisInterface->Next != NULL_CHIPAL_T)
	{
		pThisInterface = pThisInterface->Next;
	}

	Status = ALLOCATE_MEMORY( (PVOID)&(pThisInterface->Next), sizeof(CHIPAL_T), MEMORY_TAG );
	if (Status != STATUS_SUCCESS)
	{
		return(NULL_CHIPAL_T);
	}
	CLEAR_MEMORY(pThisInterface->Next, sizeof(CHIPAL_T));


	pThisInterface = pThisInterface->Next;
	pThisInterface->Next = NULL_CHIPAL_T;

	return(pThisInterface);

}

/*******************************************************************************
Name:		chipALReleaseInterface
Function:	Release Memory for Adapter Instance	and relink list if necessary
Description:
			  
			    
*******************************************************************************/
INLINE void chipALReleaseInterface(CHIPAL_T * AdapterToRelease)
{
	CHIPAL_T * pThisInterface;
	CHIPAL_T * pTempInterface;

	CHIPAL_ASSERT_TRACE
	(
	    "chipALReleaseInterface() ",
	    TRUE
	)

	pThisInterface = pChipALHead;

	while (pThisInterface->Next != NULL_CHIPAL_T && pThisInterface->Next != AdapterToRelease)
	{
		pThisInterface = pThisInterface->Next;
	}
	if(pThisInterface->Next == AdapterToRelease)
	{
		pTempInterface = pThisInterface->Next->Next;

		FREE_MEMORY
		(
		    pThisInterface->Next,
		    sizeof(CHIPAL_T),
		    CHIPAL_CONTEXT_MEM_FLAGS
		);

		pThisInterface->Next = 	pTempInterface;
	}
}

/******************************************************************************
FUNCTION NAME:
	ChipALDeviceSpecific

ABSTRACT:
	Process Device Specific OIDs


RETURN:


DETAILS:
******************************************************************************/
NDIS_STATUS ChipALDeviceSpecific(
    CDSL_ADAPTER_T				* pThisAdapter,
    BACK_DOOR_T	*				pBackDoorBuf,
    PULONG						pNumBytesNeeded,
    PULONG						pNumBytesWritten )
{
	NDIS_STATUS     status = STATUS_SUCCESS;
	CHIPAL_T 	*	pThisInterface;
	BUS_RESOURCE_T	* pBusResource;
	SK_BUFF_T         ReturnPacket;

	if ( pBackDoorBuf->TotalSize < (SIZEOF_DWORD_ALIGN( BACK_DOOR_T )) )
	{
		if ( pBackDoorBuf->TotalSize != (SIZEOF_DWORD_ALIGN( MINI_BACK_DOOR_T )) )
		{
			pBackDoorBuf->ResultCode = ERROR_WARNING( RESULT_REQ_BUFF_TOO_SMALL );
			pBackDoorBuf->NeededSize = SIZEOF_DWORD_ALIGN( BACK_DOOR_T );
			*pNumBytesNeeded = SIZEOF_DWORD_ALIGN(BACK_DOOR_T);
			return ERROR_WARNING( RESULT_REQ_BUFF_TOO_SMALL );
		}
	}

	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod;

	if ( pThisInterface == NULL_CHIPAL_T )
	{
		pBackDoorBuf->ResultCode = ERROR_FATAL( RESULT_ADAPTER_NOT_FOUND );
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		return ERROR_FATAL( RESULT_ADAPTER_NOT_FOUND );
	}
	pBusResource = pThisInterface->BusResources;

	switch (pBackDoorBuf->ReqCode)
	{

	case BD_CHIPAL_GET_STATS:
		chipALGetStats(pThisAdapter, pBackDoorBuf);

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded =  pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdChipALStats )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CHIPAL_CLEAR_STATS:
		chipALClearStats(pThisAdapter, pBackDoorBuf);

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded =  pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdChipALStats )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CHIPAL_ATM_GET_TX_CELL:
		CmgmtIndicateLinkUp(pThisAdapter );
		if (pThisInterface->DmaChannel[ADSL_DMA_CHAN_TX] == NULL)
		{
			// The Receive path is not configured.
			pBackDoorBuf->Params.BdChipALAtmCell.Length = 0;

			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
			*pNumBytesNeeded =  pBackDoorBuf->TotalSize;
			*pNumBytesWritten = sizeof( pBackDoorBuf->Params.BdChipALAtmCell )
			                    + sizeof( pBackDoorBuf->ResultCode )
			                    + sizeof( pBackDoorBuf->NeededSize );

			break;
		}
		else if (pThisInterface->DmaControl->GetLastBuffer == NULL)
		{
			// The Receive path is not configured.
			pBackDoorBuf->Params.BdChipALAtmCell.Length = 0;

			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
			*pNumBytesNeeded =  pBackDoorBuf->TotalSize;
			*pNumBytesWritten = sizeof( pBackDoorBuf->Params.BdChipALAtmCell )
			                    + sizeof( pBackDoorBuf->ResultCode )
			                    + sizeof( pBackDoorBuf->NeededSize );

			break;
		}
		(* pThisInterface->DmaControl->GetLastBuffer)
		( pThisInterface->DmaChannel[ADSL_DMA_CHAN_TX], &ReturnPacket);

		pBackDoorBuf->Params.BdChipALAtmCell.Length = MIN_VALUE (
		            ReturnPacket.len, RX_BLOCK_SIZE);

		COPY_MEMORY(	pBackDoorBuf->Params.BdChipALAtmCell.DataBytes,
		             ReturnPacket.data,
		             pBackDoorBuf->Params.BdChipALAtmCell.Length);


		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded =  pBackDoorBuf->TotalSize;

		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdChipALAtmCell )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CHIPAL_ATM_GET_RX_CELL:
		if (pThisInterface->DmaChannel[ADSL_DMA_CHAN_RX] == NULL)
		{
			// The Receive path is not configured.
			pBackDoorBuf->Params.BdChipALAtmCell.Length = 0;

			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
			*pNumBytesNeeded =  pBackDoorBuf->TotalSize;
			*pNumBytesWritten = sizeof( pBackDoorBuf->Params.BdChipALAtmCell )
			                    + sizeof( pBackDoorBuf->ResultCode )
			                    + sizeof( pBackDoorBuf->NeededSize );

			break;
		}
		else if (pThisInterface->DmaControl->GetLastBuffer == NULL)
		{
			// The Receive path is not configured.
			pBackDoorBuf->Params.BdChipALAtmCell.Length = 0;

			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
			*pNumBytesNeeded =  pBackDoorBuf->TotalSize;
			*pNumBytesWritten = sizeof( pBackDoorBuf->Params.BdChipALAtmCell )
			                    + sizeof( pBackDoorBuf->ResultCode )
			                    + sizeof( pBackDoorBuf->NeededSize );

			break;
		}
		(* pThisInterface->DmaControl->GetLastBuffer)
		( pThisInterface->DmaChannel[ADSL_DMA_CHAN_RX], &ReturnPacket);

		pBackDoorBuf->Params.BdChipALAtmCell.Length = MIN_VALUE (
		            ReturnPacket.len, RX_BLOCK_SIZE);

		COPY_MEMORY(	pBackDoorBuf->Params.BdChipALAtmCell.DataBytes,
		             ReturnPacket.data,
		             pBackDoorBuf->Params.BdChipALAtmCell.Length);


		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded =  pBackDoorBuf->TotalSize;

		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdChipALAtmCell )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

		#ifdef DBG
	case BD_CARDMGMT_SET_DEBUG_FLAG:
		ChipALDebugFlag &= DEBUG_MODULE_ID_MASK;
		ChipALDebugFlag |= pBackDoorBuf->Params.BdCardMgmtDebugFlag;
		break;

#endif //DBG

	default:
		//	These BackDoor ReqCodes have not been implemented yet.
		//		BD_CHIPAL_START_LOOPBACK
		//		BD_CHIPAL_START_1_LOOPBACK
		//		BD_CHIPAL_STOP_LOOPBACK
		//		BD_CHIPAL_GET_TEST_RESULTS
		//		BD_CHIPAL_ENQ_TEST_COMPLETE
		//		BD_CHIPAL_GET_TUNING_PARAMS
		//		BD_CHIPAL_SET_TUNING_PARAMS
		status = STATUS_UNSUPPORTED;
		break;
	}

	return status;
}


/******************************************************************************
FUNCTION NAME:
	chipALGetStats

ABSTRACT:
	Get All Hal Statistics


RETURN:
	void


DETAILS:
******************************************************************************/
LOCAL INLINE void chipALGetStats(CDSL_ADAPTER_T * pThisAdapter, BACK_DOOR_T	*	pBackDoorBuf)
{
	CHIPAL_T		*	 pThisInterface;
	#if DMA_STATS
	BD_CHIPAL_STATS_T	TempStats;
	#endif

	CLEAR_MEMORY(&(pBackDoorBuf->Params), sizeof(BD_CHIPAL_STATS_T));

	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod;

	if ( pThisInterface == NULL_CHIPAL_T )
	{
		return;
	}

#if DMA_STATS
	{
		DWORD				 Index;

		// *
		// * Sum DMA Transmit and Receive Numbers
		// *
		for (Index = 0; Index < MAX_DATA_CHANNEL ; Index++ )
		{
			if (pThisInterface->DmaChannel[Index] != NULL_HW_CH_ENTRY )
			{
				(* pThisInterface->DmaControl->ChanGetStats) (
				    pThisInterface->DmaChannel[Index],
				    &TempStats );
				pBackDoorBuf->Params.BdChipALStats.BytesTransmitted	+= TempStats.BytesTransmitted;
				pBackDoorBuf->Params.BdChipALStats.BytesReceived	+= TempStats.BytesReceived;
			}
		}
	}

	// *
	// * Data was summed in the standard BD_CHIPAL_STATS_T structure.  It must
	// * now be copied to the BD_CHIPAL_ATM_STATS_T struct. Being xxx.Params is a
	// * union, the TempStats location is used to avoid overwriting data.  Both
	// * structs report data in Kbytes.
	// *
	TempStats = pBackDoorBuf->Params.BdChipALStats;
	pBackDoorBuf->Params.BdChipALAtmStats.NumTxCells = TempStats.BytesTransmitted;
	pBackDoorBuf->Params.BdChipALAtmStats.NumRxCells = TempStats.BytesReceived;
	return;


#endif								// DMA_STATS

}

/******************************************************************************
FUNCTION NAME:
	chipALClearStats

ABSTRACT:
	Get All Hal Statistics

RETURN:
	void

DETAILS:
******************************************************************************/
LOCAL INLINE void chipALClearStats(CDSL_ADAPTER_T * pThisAdapter, BACK_DOOR_T	*	pBackDoorBuf)
{
	CHIPAL_T	*	 pThisInterface;

	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod;

	if ( pThisInterface == NULL_CHIPAL_T )
	{
		return;
	}

	#if DMA_STATS
	{
		DWORD Index;

		// *
		// * Clear DMA Stats
		// *
		for (Index = 0; Index < MAX_DATA_CHANNEL ; Index++ )
		{
			if (pThisInterface->DmaChannel[Index] != NULL_HW_CH_ENTRY )
			{
				(* pThisInterface->DmaControl->ChanClearStats) (
				    pThisInterface->DmaChannel[Index] );
			}
		}
	}
	#endif								// DMA_STATS

}


#ifdef DBG								// Development Test Routines

/*******************************************************************************
Name:		ChipALTestReadBuff
Function:	Read Buffer from ChipAL
Description:
			ChipALTestxxx functions are used for development testing with the 
			ControlPanel Applet. They are not intended for the final 
			product.  
			    
*******************************************************************************/
DWORD chipALTestReadBuff (char * Destination, DWORD BuffSize, DWORD * SizeWritten)
{
	DWORD WriteLen;
	#define TEST_STR "The Quick Brown Ugly Fox"

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALTestReadBuff ",
	    (Destination != (char *) 0 && BuffSize != 0l &&
	     SizeWritten != (DWORD *) 0 )
	)


	WriteLen = sizeof(TEST_STR);

	if (BuffSize < WriteLen)
	{
		return(STATUS_FAILURE);
	}
	COPY_MEMORY(Destination, TEST_STR, WriteLen);
	*SizeWritten = WriteLen;

	return(STATUS_SUCCESS);
}

/*******************************************************************************
Name:		ChipALTestWriteBuff
Function:	Write Buffer to ChipAl
Description:
			ChipALTestxxx functions are used for development testing with the 
			ControlPanel Applet. They are not intended for the final 
			product.  
			    
*******************************************************************************/
DWORD chipALTestWriteBuff(CDSL_ADAPTER_T * ThisAdapter, char * Destination, DWORD SizeInBuff)
{
	#define TEST_BUFF_SIZE 80

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALTestWriteBuff() ",
	    (Destination != (char *) 0  && SizeInBuff != 0l && ThisAdapter != (CDSL_ADAPTER_T *) 0)
	)

	if (ThisAdapter == (CDSL_ADAPTER_T *) 0 )
	{
		return(STATUS_FAILURE);
	}
	else if (ThisAdapter->ChipMod == NULL_CHIPAL_T)
	{
		return(STATUS_FAILURE);
	}

	return(STATUS_SUCCESS);
}

/******************************************************************************
FUNCTION NAME:
	ChipALTestReadConfig

ABSTRACT:
	Get Adapter Status and Configuration


RETURN:
	ChipAL Fail Code


DETAILS:
******************************************************************************/
DWORD chipALTestReadConfig
(
    CDSL_ADAPTER_T		*	ThisAdapter,
    BD_CHIPAL_CONFIG_T	*	pConfigStatus
)
{
	CHIPAL_T * pThisInterface;


	CHIPAL_ASSERT_TRACE
	(
	    "ChipALTestReadConfig() ",
	    (ThisAdapter != (CDSL_ADAPTER_T *) 0 )
	)

	if (ThisAdapter == (CDSL_ADAPTER_T *) 0 )
	{
		return(STATUS_FAILURE);
	}


	if (ThisAdapter->ChipMod == NULL_CHIPAL_T)
	{
		pConfigStatus->Status			=	CHIPAL_STATUS_DOWN;
		pConfigStatus->Irq				=	0l;
		pConfigStatus->BaseAddress		=	0l;
		pConfigStatus->SharedAddress	=	0l;

		return(STATUS_SUCCESS);
	}

	pThisInterface = (CHIPAL_T *) ThisAdapter->ChipMod;

	// *
	// * Check  Bus Resources
	// *
	if (pThisInterface->BusResources != NULL_PCI_RECOURCES)
	{
		pConfigStatus->Status			=	CHIPAL_STATUS_CONFIGURED;
		pConfigStatus->Irq				=	pThisInterface->BusResources->PriIntVector;
		pConfigStatus->BaseAddress		=	(DWORD) pThisInterface->BusResources->VirtualAddressBAR0;
		pConfigStatus->SharedAddress	=	(DWORD) pThisInterface->BusResources->VirtualAddressSharedMem;
	}
	else
	{
		pConfigStatus->Status			=	CHIPAL_STATUS_INITIALIZED;
		pConfigStatus->Irq				=	0l;
		pConfigStatus->BaseAddress		=	0l;
		pConfigStatus->SharedAddress	=	0l;
	}
	return(STATUS_SUCCESS);
}

#ifdef TRACING_ENABLED
/******************************************************************************
FUNCTION NAME:
	chipALDisplayTraceQueue

ABSTRACT:
	Display Memory Trace.  DEBUG ONLY


RETURN:
	void


DETAILS:
******************************************************************************/
LOCAL void chipALDisplayTraceQueue(void)
{
	WORD	TempIdx;

	for (TempIdx = 0; TempIdx < TRACE_QUEUE_LEN  ; ++TempIdx)
	{
		switch (TraceQueue[TempIdx])
		{
			case ISR_ENTRY:
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("Trace: %4d ISR_ENTRY", TraceQueue[TempIdx]));
				break;
			case ISR_EXIT:
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("Trace: %4d ISR_EXIT", TraceQueue[TempIdx]));
				break;
			case SYNC_ENTRY:
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("Trace: %4d SYNC_ENTRY", TraceQueue[TempIdx]));
				break;
			case SYNC_EXIT:
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("Trace: %4d SYNC_EXIT", TraceQueue[TempIdx]));
				break;
			case TX_ENTRY:
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("Trace: %4d TX_ENTRY", TraceQueue[TempIdx]));
				break;
			case TX_EXIT:
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("Trace: %4d TX_EXIT", TraceQueue[TempIdx]));
				break;
			case RX_ENTRY:
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("Trace: %4d RX_ENTRY", TraceQueue[TempIdx]));
				break;
			case RX_EXIT:
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("Trace: %4d RX_EXIT", TraceQueue[TempIdx]));
				break;
			case DPC_ENTRY:
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("Trace: %4d DPC_ENTRY", TraceQueue[TempIdx]));
				break;
			case DPC_EXIT:
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("Trace: %4d DPC_EXIT", TraceQueue[TempIdx]));
				break;
			default:
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("Trace: %4d UDEFINED", TraceQueue[TempIdx]));
				break;
		}
	}
}
#endif // TRACING_ENABLED
#endif // DBG	 Development Test Routines
