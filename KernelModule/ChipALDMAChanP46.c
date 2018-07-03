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
**		ChipAL (Chip Abstraction Layer)
**
**	FILE NAME:
**		ChipALDmaChanP46.c
**
**	ABSTRACT:
**		Provide Abstraction Layer for all hardware functions
**
**	DETAILS:
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.4.3_ATM_Driver/KernelModule/ChipALDMAChanP46.c $
**	$Revision: 7 $
**	$Date: 8/13/01 5:05p $
*******************************************************************************
******************************************************************************/




/*******************************************************************************
Includes
*******************************************************************************/
#define _CHIPALDMACHANP46_C				// Module Id

#define CONTINUOUS_TRANSMIT 1			// Transmit will continue as long as 
// data is available

#define MAX_PIO_USERS				1

#include "LnxTools.h"
#include "CardMgmt.h"
#include "ChipALCdslV.h"

#include "ChipALDmaChanP46.h"

#if ! defined(CHIPAL_ASSERT_TRACE)
	#define CHIPAL_ASSERT_TRACE(mess,assertion)								\
		if(assertion)														\
		{																	\
			P46ARB_DEBUG_MESSAGE((mess));									\
		}																	\
		else																\
		{																	\
			P46ARB_DEBUG_MESSAGE((mess));									\
			P46ARB_DEBUG_MESSAGE(("Assertion Failed !\n"));					\
			ASSERT(FALSE);													\
		}																	

#endif


static BOOLEAN		TraceTx = 0;
static BOOLEAN		TraceRx = 0;
#ifdef DBG
	#define EXTENDED_VALIDATION	1
#else
	#define EXTENDED_VALIDATION	0
#endif


#ifndef MEMORY_TAG
	#define MEMORY_TAG			'mDLC'
#endif

// * Minimum value of a system that supports cache. The value 1 indicates we can allign on
// * any boundry
#define MIN_CACHE_SIZE				1	

#if ! defined(NdisGetCacheFillSize)
	#define  NdisGetCacheFillSize ndisgetcachefillsize
#endif

extern CHAR	PG;

/*******************************************************************************
Private Data Types
*******************************************************************************/

#define ATM_PAYLOAD_SIZE	48
#define ATM_HEADER_SIZE		5
#define	ATM_ALIGN_FILL		3
#define SEG_PTR_DATA_LEN	(ATM_PAYLOAD_SIZE+ATM_HEADER_SIZE+ATM_ALIGN_FILL)
#define	DEFAULT_DATA_ELEMENTS	1
// *
// * This is the minimum number of segments required for a DMA channel. At minimum,
// * a channel will alternate between two DMA segments.  Interrupts have a one to one
// * relationship with segments, as do cache segments.
// *
#define MIN_NUMBER_SEGMENTS 8 
#define	MAX_NUMBER_SEGMENTS 96
#define MAX_BLOCKS_PER_SEGMENT	256

typedef struct HW_DATA_S
{
	CHAR	Header[ATM_HEADER_SIZE];
	CHAR	Data[ATM_PAYLOAD_SIZE];
	CHAR	Fill[ATM_ALIGN_FILL];
} HW_DATA_T;

typedef enum DMA_SEGMENT_STATE_E
{
    DMA_SEGMENT_READY = 0,
    DMA_SEGMENT_ARM_PROCESSING,
    DMA_SEGMENT_PENDING,
    DMA_SEGMENT_CANCELLED

} DMA_SEGMENT_STATE_T;

// *
// * Segment descriptors describe an area of Cache aligned memory. That is accessed by the
// * device.
// *
typedef struct HW_SEGMENT_DES_S
{
	#if DMA_CACHE_ENABLED
	PNDIS_BUFFER			pNdisBuffer;			// Handle to Cached Area (All Data Segments)
	UINT					NdisBufferLength;		// Length of Cached Area (All Data Segments)
	#endif

	HW_DATA_T				* AddressDataSegment;	// Virtual Address of Data Segment
	DWORD					PhysStartData;			// Physical Address Data Segment
	DMA_SEGMENT_STATE_T		State;					// State of This Segment
	union
	{
		COMMAND_START_TX_DMA_T	DmaCommand;				// Control string sent to arm to start this segment
		COMMAND_START_TX_DMA_T	RxDmaCommand;			// Control string sent to arm to start this segment
	}CommandU;
	DWORD					NumHardwareDescriptors;	// Number of data blocks in this segment

} HW_SEGMENT_DES_T;


typedef enum DMA_CHAN_STATE_E
{
    DMA_CHAN_UNINITIALIZED = 0,
    DMA_CHAN_ENABLED,
    DMA_CHAN_DISABLED,

} DMA_CHAN_STATE_T;

// *
// * Private Context for DMA Controller
// *
typedef struct CHAN_EXTENSION_S
{
	CHIPAL_T * 				pChipAl;

	#if DMA_CACHE_ENABLED
	NDIS_HANDLE				PoolHandle;
	#endif

	ULONG					CacheFillSize;		// Size used to Allocate Cache Aligned Segments
	WORD					MaxBlocksPerSegment;// Number of Data Blocks per Segment

	PVOID					AddressCached;		// Pointer to beginning of Cached Memory
	DWORD					CachedSize;			// Bytes of Cached Data Storage
	char*					PhysCached;			// Physical Address of Cached Memory

	HW_SEGMENT_DES_T		* Segment;			// Structures required to manage Shared Memory
	DWORD					SharedMgmtSize;		// Number of Bytes required to manage Shared Memory

	SPIN_LOCK				SegmentLock;		// Used for List Manipulation
	DWORD					ActiveSegment;		// Chache Flushed Segments available to hardware

	DWORD					OutOfSequence;		// Segments not Acknowledged from ARM

	DMA_CHAN_STATE_T		State;				// Current State of DMA Controller
	BYTE					CurrentSegment;		// Segment Currently being Transmitted
	BYTE					NextSegment;		// Next available Segment to Place Data for Transmit


} CHAN_EXTENSION_T;

#define NEXT_SEGMENT(seg,pch)\
	( 	seg = 																					\
		(((CHAN_EXTENSION_T *)(&pch->Extension))->ActiveSegment + 1) % pch->NumberOfSegments	\
	)

#define PERVIOUS_SEGMENT(seg,pch)\
	( 	seg = (((CHAN_EXTENSION_T *)(&pch->Extension))->ActiveSegment == ((DWORD) pch->NumberOfSegments-1))	\
		?0:--((CHAN_EXTENSION_T *)(&pch->Extension))->ActiveSegment									\
	)

/*******************************************************************************
Private Prototypes
*******************************************************************************/
LOCAL INLINE NTSTATUS calDmaP46AllocateSharedMemory(
    HW_CH_ENTRY_T *  pCh
);

LOCAL VOID calDMAFreeSharedMemory(
    HW_CH_ENTRY_T *  pCh
);

LOCAL INLINE NTSTATUS calDMAInitSharedMemory(
    HW_CH_ENTRY_T *  pCh
);




MODULAR void DmaRxDataHandler (
    void			* pChanEntry,
    OUT RESPONSE_T	* Response
);
MODULAR void DmaRxCompleteHandler (
    void			* pCh,
    OUT RESPONSE_T	* Response
);
MODULAR void DmaRxFullHandler(
    void			* pCh,
    OUT RESPONSE_T	* Response
);


MODULAR void DmaTxDataHandler (
    void * pCh,
    OUT RESPONSE_T	* Response
);
MODULAR void DmaTxCompleteHandler (
    void			* pCh,
    OUT RESPONSE_T	* Response
);

MODULAR void DmaTxFullHandler(
    void			* pCh,
    OUT RESPONSE_T	* Response
);

/*******************************************************************************
Name:		ChipALDmaChanP46Open
Function:	Open Transmit and Receive Channels
Description:
			  
			    
*******************************************************************************/
MODULAR HW_CH_ENTRY_T *  ChipALDmaChanP46Open(
    VOID * 			UserHandle,
    HW_CH_ENTRY_T	* pChConfig)
{
	HW_CH_ENTRY_T		* pCh;
	NDIS_STATUS 		Status = STATUS_FAILURE ;
	CHIPAL_FAILCODE_T	Success;
	CHAN_EXTENSION_T	* pChanExtension;
	P46_IO_COMMAND_T	Command;

	// *
	// * Allocate Bus Interface Context Block
	// *
	CHIPAL_ASSERT_TRACE
	(
	    "ChipALDmaChanP46Open() ",
	    (pChConfig != NULL_HW_CH_ENTRY)
	    && UserHandle != NULL
	)

	Status = ALLOCATE_MEMORY(
	             (PVOID)&pCh,
	             sizeof(HW_CH_ENTRY_T) + sizeof(CHAN_EXTENSION_T) - sizeof(pCh->Extension),
	             MEMORY_TAG );

	if (Status != STATUS_SUCCESS)
	{
		return(NULL_HW_CH_ENTRY);
	}


	// * Copy Configuration from User Space
	*pCh = *pChConfig;

	// *
	// * Init DMA Channel Extension
	// *
	pChanExtension = (CHAN_EXTENSION_T *) &pCh->Extension;
	CLEAR_MEMORY(&pCh->Extension, sizeof(CHAN_EXTENSION_T));
	pChanExtension->pChipAl = (CHIPAL_T * )UserHandle;

	INIT_SPIN_LOCK(&pChanExtension->SegmentLock);

	// *
	// * Calculate Cached and Non Chached Requirements, Allocate Memory,
	// * and setup buffer descriptor pointers.
	// *
	Status = calDmaP46AllocateSharedMemory(pCh);

	if (Status != STATUS_SUCCESS)
	{
		FREE_MEMORY
		(
		    pCh,
		    sizeof(HW_CH_ENTRY_T),
		    CHIPAL_CONTEXT_MEM_FLAGS
		);
		return(NULL_HW_CH_ENTRY);
	}

	pCh->Busy = 0;

	// Assign DPC Handler for data event
	switch(pCh->Description.ElUnion.Element.Direction)
	{
	case HOST_TO_DEVICE:
		// *
		// * Transmit Channel
		// *
		Success = ChipALAddEventHandler (
		              pChanExtension->pChipAl->pNdisAdapter,	// Adapter Instance
		              DSL_FUNCTION,
		              pCh->Description.ElUnion.Element.Event,
		              HW_DPC_EVENT,
		              DmaTxDataHandler,
		              pCh
		          );

		Success = ChipALAddEventHandler (
		              pChanExtension->pChipAl->pNdisAdapter,	// Adapter Instance
		              DSL_FUNCTION,
		              pCh->Description.ElUnion.Element.Event,
		              HW_ISR_EVENT,
		              DmaTxCompleteHandler,
		              pCh
		          );

		Success = ChipALAddEventHandler (
		              pChanExtension->pChipAl->pNdisAdapter,	// Adapter Instance
		              DSL_FUNCTION,
		              HW_ARM_TX_QUEUE_FULL,
		              HW_ISR_EVENT,
		              DmaTxFullHandler,
		              pCh
		          );


		Command.CsrParam.NotifyOnComplete = NULL;
		Command.CsrParam.Command = DSL_SET_ADSL_MODE;
		Command.CsrParam.Param.SetAdslMode.Mode = ATM_MODE_IDLE_INS_ATM;
		Status = ChipALWrite(
		             pChanExtension->pChipAl->pNdisAdapter,
		             P46_ARM_DSL,
		             ADDRESS_BAR_0,
		             CSR_ARM_CMD,
		             sizeof (COMMAND_SET_ADSL_MODE_T),
		             (DWORD) &Command,
		             0 									// Mask	Value is unused	by CSR_ARM_CMD
		         );

		Status = ChipALWrite(
		             pChanExtension->pChipAl->pNdisAdapter,
		             P46_ARM_DSL,
		             ADDRESS_BAR_0,
		             CSR_ARM_CTRL,
		             sizeof (DWORD),
					 CSR_DSL_AFE_MUX | CSR_DSL_IDLE_INSERT | CSR_DSL_RX_FIFO_MUX,	//  Host has Access to AFE Interface
					 CSR_DSL_AFE_MUX | CSR_DSL_IDLE_INSERT | CSR_DSL_RX_FIFO_MUX	// 	// Mask
		         );

		//PG = 0;

		break;

	case DEVICE_TO_HOST:
		// *
		// * Receive Channel
		// *
		Success = ChipALAddEventHandler (
		              pChanExtension->pChipAl->pNdisAdapter,	// Adapter Instance
		              DSL_FUNCTION,
		              pCh->Description.ElUnion.Element.Event,
		              HW_DPC_EVENT,
		              DmaRxDataHandler,
		              pCh
		          );

		Success = ChipALAddEventHandler (
		              pChanExtension->pChipAl->pNdisAdapter,	// Adapter Instance
		              DSL_FUNCTION,
		              pCh->Description.ElUnion.Element.Event,
		              HW_ISR_EVENT,
		              DmaRxCompleteHandler,
		              pCh
		          );

		Success = ChipALAddEventHandler (
		              pChanExtension->pChipAl->pNdisAdapter,	// Adapter Instance
		              DSL_FUNCTION,
		              HW_ARM_RX_QUEUE_FULL,
		              HW_ISR_EVENT,
		              DmaRxFullHandler,
		              pCh
		          );

		break;

	default:
		calDMAFreeSharedMemory(pCh);
		FREE_MEMORY
		(
		    pCh,
		    sizeof(HW_CH_ENTRY_T),
		    CHIPAL_CONTEXT_MEM_FLAGS
		);
		return(NULL_HW_CH_ENTRY);
	}

	return(pCh);
}





/******************************************************************************
FUNCTION NAME:
	ChipALDmaChanP46Close

ABSTRACT:
	Disable DMA channel and release allocated memory


RETURN:
	void


DETAILS:
******************************************************************************/
MODULAR void ChipALDmaChanP46Close(HW_CH_ENTRY_T * pCh)
{
	CHAN_EXTENSION_T	* pChExt;


	// Disable Interrupts and DMA Channels
	CHIPAL_ASSERT_TRACE
	(
	    "ChipALDmaChanP46Close() ",
	    (pCh != NULL_HW_CH_ENTRY)
	)

	pChExt = (CHAN_EXTENSION_T	*) &pCh->Extension;

	// * Close Data path.  Don't care about return value.
	ChipALDmaChanP46Disable(pCh);

	// *
	// * Remove Bindings to ChipAl Event Handlers
	// *
	switch(pCh->Description.ElUnion.Element.Direction)
	{
	case HOST_TO_DEVICE:

		// *
		// * Transmit Channel
		// *
		ChipALRemoveEventHandler (
		    pChExt->pChipAl->pNdisAdapter,	// Adapter Instance
		    DSL_FUNCTION,
		    pCh->Description.ElUnion.Element.Event,
            HW_DPC_EVENT,
		    DmaTxDataHandler
		);

		ChipALRemoveEventHandler (
		    pChExt->pChipAl->pNdisAdapter,	// Adapter Instance
		    DSL_FUNCTION,
		    pCh->Description.ElUnion.Element.Event,
		    HW_ISR_EVENT,
		    DmaTxCompleteHandler
		);

		ChipALRemoveEventHandler (
		    pChExt->pChipAl->pNdisAdapter,	// Adapter Instance
		    DSL_FUNCTION,
		    HW_ARM_TX_QUEUE_FULL,
		    HW_ISR_EVENT,
		    DmaTxFullHandler
		);
		break;


	case DEVICE_TO_HOST:

		// *
		// * Receive Channel
		// *
		ChipALRemoveEventHandler (
		    pChExt->pChipAl->pNdisAdapter,	// Adapter Instance
		    DSL_FUNCTION,
		    pCh->Description.ElUnion.Element.Event,
            HW_DPC_EVENT,
		    DmaRxDataHandler
		);

		ChipALRemoveEventHandler (
		    pChExt->pChipAl->pNdisAdapter,	// Adapter Instance
		    DSL_FUNCTION,
		    pCh->Description.ElUnion.Element.Event,
		    HW_ISR_EVENT,
		    DmaRxCompleteHandler
		);

		ChipALRemoveEventHandler (
		    pChExt->pChipAl->pNdisAdapter,	// Adapter Instance
		    DSL_FUNCTION,
		    HW_ARM_RX_QUEUE_FULL,
		    HW_ISR_EVENT,
		    DmaRxFullHandler
		);
		break;

	default:
		break;
	}


	// *
	// * Free Memory Allocations
	// *
	calDMAFreeSharedMemory(pCh);
	FREE_MEMORY
	(
	    pCh,
	    sizeof(HW_CH_ENTRY_T),
	    CHIPAL_CONTEXT_MEM_FLAGS
	);

}


/******************************************************************************
FUNCTION NAME:
	ChipALDmaChanP46Enable

ABSTRACT:
	Enable DMA Channel


RETURN:
	NTSTATUS:


DETAILS:
	For Transmit, the Channel state is updated then the DmaTxDataHandler is
	called to fill the any empty segment descriptors.

	For Receive, the state is updated, the DmaRxDataHandler is called to pass
	any empty descriptors to the ARM

******************************************************************************/
MODULAR NTSTATUS ChipALDmaChanP46Enable (HW_CH_ENTRY_T * pCh)
{
	CHAN_EXTENSION_T	* pChExt;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALDmaChanP46Enable() ",
	    (pCh != NULL_HW_CH_ENTRY)
	)

	pChExt = (CHAN_EXTENSION_T	*) &pCh->Extension;

	// *
	// * Determine if we are Transmit or Receive
	// *
	// Assign DPC Handler for data event
	switch(pCh->Description.ElUnion.Element.Direction)
	{
	case HOST_TO_DEVICE:
		// *
		// * Transmit Channel - The transmitter is driven by the upper layer. There is
		// * nothing required by the hardware to start the operation.
		// *
		pChExt->State = DMA_CHAN_ENABLED;

		// * Watch Dog Timer
		ChipALAddEventHandler (
		    pChExt->pChipAl->pNdisAdapter,	// Adapter Instance
		    DSL_FUNCTION,
		    HW_GENERAL_TIMER_A,
            HW_DPC_EVENT,
		    DmaTxDataHandler,
		    pCh
		);
		return(STATUS_SUCCESS);

		break;

	case DEVICE_TO_HOST:
		// *
		// * Receive Channel. Call the receive data handler, this will send any ready
		// * segment descriptors to the hardware.
		// *
		pChExt->State = DMA_CHAN_ENABLED;
		DmaRxDataHandler(pCh,NULL);
		return(STATUS_SUCCESS);
		break;

	default:
		// *
		CHIPAL_ASSERT_TRACE
		(
		    "ChipALDmaChanP46Enable(): Unknown Channel Type ",
		    FALSE
		)

		return(-EINVAL);

	}


}


/******************************************************************************
FUNCTION NAME:
	ChipALDmaChanP46Disable

ABSTRACT:
	Disable DMA Channel. The Channel State is updated, then all the 
	appropriate segment descripors are marked as pending or ready.


RETURN:
	NTSTATUS:
		-EINVAL - Invalid Channel Direction
		STATUS_SUCCESS - Stop command written to Arm Queue.  The ISR routine
			will update the descriptor where the ARM actually stopped.


DETAILS:
******************************************************************************/
MODULAR NTSTATUS ChipALDmaChanP46Disable (HW_CH_ENTRY_T * pCh)
{

	NTSTATUS			Status;
	CHAN_EXTENSION_T	* pChExt;
	P46_IO_COMMAND_T	Command;
	DWORD				Index;
	HW_SEGMENT_DES_T	* pSegment;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALDmaChanP46Disable() ",
	    (pCh != NULL_HW_CH_ENTRY)
	)
	pChExt = (CHAN_EXTENSION_T	*) &pCh->Extension;
	pChExt->State = DMA_CHAN_DISABLED;

	switch(pCh->Description.ElUnion.Element.Direction)
	{
	case HOST_TO_DEVICE:

		// * Remove Transmit Watchdog timer
		ChipALRemoveEventHandler(
		    pChExt->pChipAl->pNdisAdapter,
		    DSL_FUNCTION,
		    HW_GENERAL_TIMER_A,
            HW_DPC_EVENT,
		    DmaTxDataHandler
		);

		Command.CsrParam.Param.StopDma.Direction = CHANNEL_TX;
		DBG_DISPLAY("Disable DMA Tx: Current Segment: %x  Next Segment: %x \n",pChExt->CurrentSegment,pChExt->NextSegment);
		break;

	case DEVICE_TO_HOST:
		Command.CsrParam.Param.StopDma.Direction = CHANNEL_RX;
		DBG_DISPLAY("Disable DMA Rx: Current Segment: %x  Next Segment: %x \n",pChExt->CurrentSegment,pChExt->NextSegment);
		break;

	default:
		return(-EINVAL);
		break;
	}

	// * Stop Arm
	Command.CsrParam.NotifyOnComplete = NULL;
	Command.CsrParam.Command = DSL_STOP_DMA;
	Status = ChipALWrite(
	             pChExt->pChipAl->pNdisAdapter,
	             P46_ARM_DSL,
	             ADDRESS_BAR_0,
	             CSR_ARM_CMD,
	             sizeof (COMMAND_STOP_DMA_T),	// Not used at this time
	             (DWORD) &Command,
	             0 								// Mask	Value is unused	by CSR_ARM_CMD
	         );


	// *
	// * Reset All segment states.  Being we have already lost data, there is little
	// * point in trying to salvage any data remaining in the buffers.
	// *
	for
	(
	    Index = 0;
	    Index <  pCh->NumberOfSegments;
	    Index++
	)
	{
		pSegment = pChExt->Segment + Index;
		pSegment->State = DMA_SEGMENT_READY;
	}
	pChExt->NextSegment = 0;
	pChExt->CurrentSegment = pChExt->NextSegment;


	return(STATUS_SUCCESS);
}


#if DMA_STATS
/******************************************************************************
FUNCTION NAME:
	ChipALDmaChanP46GetStats

ABSTRACT:
	Get Channals Statistics


RETURN:
	void


DETAILS:
******************************************************************************/
MODULAR void ChipALDmaChanP46GetStats(HW_CH_ENTRY_T * pCh, BD_CHIPAL_STATS_T * Stats)
{
	if (pCh->Description.ElUnion.Element.Direction == HOST_TO_DEVICE)
	{
		Stats->BytesTransmitted = (DWORD) pCh->BytesProcessed / 1024;
		Stats->BytesReceived = 0l;
	}
	else
	{
		Stats->BytesReceived = (DWORD) pCh->BytesProcessed / 1024;
		Stats->BytesTransmitted = 0l;
	}
}

/******************************************************************************
FUNCTION NAME:
	ChipALDmaChanP46ClearStats

ABSTRACT:
	Clear Channals Statistics


RETURN:
	void


DETAILS:
******************************************************************************/
MODULAR void ChipALDmaChanP46ClearStats(HW_CH_ENTRY_T * pCh)
{

	pCh->BytesProcessed = 0;

}

#endif

/******************************************************************************
FUNCTION NAME:
	DmaRxDataHandler

ABSTRACT:
	DPC Level Data handler for passing up cells to the data protocol stack


RETURN:
	VOID


DETAILS:
	DPC Level Routine.

	If the segment's state is DMA_SEGMENT_READY all available data will be passed
	up to the stack.  When complete, the empty segment descriptor will be returned
	to the hardware via DSL_START_RX_DMA.
******************************************************************************/
MODULAR void DmaRxDataHandler (
    void			* pChan,
    OUT RESPONSE_T	* Response
)
{
	HW_CH_ENTRY_T		* pCh=(HW_CH_ENTRY_T*)pChan;
	NTSTATUS			Status;
	CHAN_EXTENSION_T	* pChanExt;
	HW_SEGMENT_DES_T	* pSegment;
	MAC_RESERVED_T		* ExtendedDesc;
	SK_BUFF_T 			* pActualDesc;
	SK_BUFF_T 			FillDesc;
	HW_DATA_T			* pRawData;
	DWORD				Count;
	P46_IO_COMMAND_T	Command;
	DWORD				LockFlag;

	ASSERT(pCh != NULL);

#ifdef PIM_DEBUG
if (irqs_disabled())
  printk(KERN_ALERT "CnxADSL: DmaRxDataHandler IRQs disabled?\n");
#endif
	pChanExt = (CHAN_EXTENSION_T	*) &pCh->Extension;

	if(pChanExt->State != DMA_CHAN_ENABLED)
	{
		return;
	}

	if (TraceRx){DBG_DISPLAY(" DmaChanP46RxDataHandler Start Processing. Segment %x\n",pChanExt->NextSegment );}

	ACQUIRE_LOCK(&pChanExt->SegmentLock, LockFlag);

	FillDesc.len = sizeof(pRawData->Data);
	ExtendedDesc = (MAC_RESERVED_T *) &FillDesc.cb[0];
	ExtendedDesc->HeaderLength = sizeof(pRawData->Header);

	pSegment = pChanExt->Segment + pChanExt->NextSegment;

	Command.CsrParam.NotifyOnComplete = NULL;
	Command.CsrParam.Command = DSL_START_RX_DMA;

	while(pSegment->State == DMA_SEGMENT_READY)
	{
		pRawData = pSegment->AddressDataSegment;

		// *
		// * Pass up everything from the start of the segment up to the position of the
		// * limit value. Reset each buffer Descriptor as completed.
		// *
		ExtendedDesc = (MAC_RESERVED_T *) &FillDesc.cb[0];
		ExtendedDesc->HeaderLength = sizeof(pRawData->Header);
		FillDesc.len = sizeof(pRawData->Data);
		pRawData = pSegment->AddressDataSegment;

		for(Count = 0; Count < pSegment->CommandU.DmaCommand.Control.Count; Count++)
		{
			if (pCh->ProcessData != NULL)
			{

				// Create Packet Descriptor and Notify Host
				ExtendedDesc->HeaderInfo = pRawData->Header;
				FillDesc.data = pRawData->Data;
				pActualDesc = (* pCh->ProcessData) (pCh->UserLink, &FillDesc);
				#if DMA_STATS
				pCh->BytesProcessed += FillDesc.len + ExtendedDesc->HeaderLength;
				#endif

			}
			++pRawData;
		}

		// * Reset Segment Descriptor and send to hardware
		Command.CsrParam.Param.StartRxDma.Address = pSegment->PhysStartData;
		Command.CsrParam.Param.StartRxDma.Control.Handle = pChanExt->NextSegment;
		Command.CsrParam.Param.StartRxDma.Control.Count	= pSegment->NumHardwareDescriptors;
		pSegment->CommandU.RxDmaCommand = Command.CsrParam.Param.StartRxDma;
		pSegment->State = DMA_SEGMENT_ARM_PROCESSING;

		// * Send Segment to Hardware
		Status = ChipALWrite(
		             pChanExt->pChipAl->pNdisAdapter,
		             P46_ARM_DSL,
		             ADDRESS_BAR_0,
		             CSR_ARM_CMD,
		             sizeof (COMMAND_START_TX_DMA_T),	// Not used at this time
		             (DWORD) &Command,
		             0 								// Mask	Value is unused	by CSR_ARM_CMD
		         );

		if(Status != STATUS_PENDING && Status != STATUS_SUCCESS)
		{
			// * Some kind of error. Mark the Ready Segment as containing no data.  Therefore,
			// * we will try to give it to the hardware on the next interrupt.
			if (TraceRx){DBG_DISPLAY(" DMA Rx Cannot Send Empty Buffer to ARM: Status: %lx. \n",Status);}
			pSegment->State = DMA_SEGMENT_READY;
			break;
		}

		pChanExt->NextSegment = (pChanExt->NextSegment + 1) % pCh->NumberOfSegments;
		pSegment = pChanExt->Segment + pChanExt->NextSegment;

	}	// * Segment Loop

	RELEASE_LOCK(&pChanExt->SegmentLock, LockFlag);
}


/******************************************************************************
FUNCTION NAME:
	DmaRxCompleteHandler

ABSTRACT:
	ISR routine to indicate hardware has finished filling this segment. 
	The Count of cells received is updated to reflect what was actually filled
	and the State is marked as ready.

RETURN:
	VOID


DETAILS:
	ISR Level Routine.

******************************************************************************/
MODULAR void DmaRxCompleteHandler (
    void			* pChan,
    OUT RESPONSE_T	* Response
)
{
	HW_CH_ENTRY_T		* pCh=(HW_CH_ENTRY_T*)pChan;
	DMA_BUFF_CONTROL_T		* pRxComplete;
	CHAN_EXTENSION_T		* pChanExt;
	HW_SEGMENT_DES_T		* pSegment;

	ASSERT(pCh != NULL && Response != NULL);

	pChanExt = (CHAN_EXTENSION_T *) &pCh->Extension;

	if(pChanExt->State != DMA_CHAN_ENABLED)
	{
		return;
	}

	// * Set pointer to Event as indicated by the channel configuration.
	pRxComplete = (DMA_BUFF_CONTROL_T *)&Response[pCh->Description.ElUnion.Element.Event];


	if(pRxComplete->Handle >= pCh->NumberOfSegments)
	{
		return;
	}

	pSegment = pChanExt->Segment + (BYTE) (pRxComplete->Handle);

	if (pRxComplete->Handle != pChanExt->CurrentSegment)
	{
		// *
		// * This Will Only Happen if an acknowledgement is lost.  Normally,
		// * this will never happen.
		// *
		// * We must invalidate everything that was lost and mark as ready. We will not send
		// * any data in the lost segment(s) to the host.
		// *
		DBG_DISPLAY("DmaRxCompleteHandler Acknowledge out of sequence: Got: %x  Expected: %x Prev. Total: %ld\n",
		            pRxComplete->Handle,
		            pChanExt->CurrentSegment,
		            pChanExt->OutOfSequence);
		for
		(
		    ;
		    pChanExt->CurrentSegment != pRxComplete->Handle;
		    pChanExt->CurrentSegment = (pChanExt->CurrentSegment + 1)%pCh->NumberOfSegments
		)
		{
			pSegment = pChanExt->Segment + pChanExt->CurrentSegment;
			pSegment->CommandU.DmaCommand.Control.Count = 0;
			pSegment->State = DMA_SEGMENT_READY;
			++pChanExt->OutOfSequence;
		}
		pSegment = pChanExt->Segment + pRxComplete->Handle;
	}

	pChanExt->CurrentSegment = ((BYTE)(pRxComplete->Handle + 1))% pCh->NumberOfSegments;

	// * Update Count to allow for the ARM not completely filling the segment.
	pSegment->CommandU.DmaCommand.Control.Count = pRxComplete->Count;
	pSegment->State = DMA_SEGMENT_READY;
}


/******************************************************************************
FUNCTION NAME:
	DmaRxFullHandler

ABSTRACT:
	This handler is called when too many StartDma commands are issued to the 
	ARM.  If the max number of segments is consistent with the RxDma queue
	of the ARM, this routine will never be called.  If it is called, it
	only marks the state as Ready and sets the filled data to 0.


RETURN:
	VOID


DETAILS:
	ISR Level Routine.

******************************************************************************/
MODULAR void DmaRxFullHandler(
    void			* pChan,
    OUT RESPONSE_T	* Response
)
{
	HW_CH_ENTRY_T		* pCh=(HW_CH_ENTRY_T*)pChan;
	DMA_BUFF_CONTROL_T		* pRxFull;
	CHAN_EXTENSION_T		* pChanExt;
	HW_SEGMENT_DES_T		* pSegment;

	ASSERT(pCh != NULL && Response != NULL);

	pChanExt = (CHAN_EXTENSION_T *) &pCh->Extension;

	if(pChanExt->State != DMA_CHAN_ENABLED)
	{
		return;
	}

	pRxFull = (DMA_BUFF_CONTROL_T *)&Response[HW_ARM_RX_QUEUE_FULL];
	pSegment = pChanExt->Segment + (BYTE) (pRxFull->Handle);

	#if EXTENDED_VALIDATION
	if (TraceRx){ DBG_DISPLAY("DmaRxCompleteHandler Processing Complete \n");}
	if((pRxFull->Handle) >= pCh->NumberOfSegments || pRxFull->Count != pSegment->CommandU.DmaCommand.Control.Count)
	{
		// * Make sure Handle is in range.  For a RxFull Indication, the count should not be changed
		if (TraceRx){DBG_DISPLAY("DmaRxFullHandler Invalid Handle:%x or Count: %x \n",pRxFull->Handle,pRxFull->Count);}
		ASSERT(FALSE);
		return;
	}
	if (TraceRx){DBG_DISPLAY("DmaTxFullHandler Acknowledge Handle:%x \n",pRxFull->Handle);}
	#endif

	pSegment = pChanExt->Segment + (BYTE) (pRxFull->Handle);
	pSegment->CommandU.DmaCommand.Control.Count = 0;
	pSegment->State = DMA_SEGMENT_READY;

}


/******************************************************************************
FUNCTION NAME:
	DmaTxDataHandler

ABSTRACT:
	Main Data processing routine for the Transmitter. Any pended segments will
	be transmitted first, then the Data protocol stack is called to fill any 
	empty segments.


RETURN:
	VOID


DETAILS:
	DPC Level Routine.

******************************************************************************/
MODULAR void DmaTxDataHandler (
    void			* pChan,
    OUT RESPONSE_T	* Response
)
{
	HW_CH_ENTRY_T		* pCh=(HW_CH_ENTRY_T*)pChan;
	NTSTATUS			Status;
	CHAN_EXTENSION_T	* pChanExt;
	HW_SEGMENT_DES_T	* pSegment;
	MAC_RESERVED_T		* ExtendedDesc;
	SK_BUFF_T 			* pActualDesc;
	SK_BUFF_T 			FillDesc;
	HW_DATA_T			* pRawData;
	DWORD				Count;
	P46_IO_COMMAND_T	Command;
	DWORD				LockFlag;

	ASSERT(pCh != NULL);

	pChanExt = (CHAN_EXTENSION_T	*) &pCh->Extension;

	if (TraceTx){DBG_DISPLAY(" DmaChanP46TxDataHandler Start Processing. Segment %x\n",pChanExt->NextSegment);}


	// *
	// * Only One user is allowed
	// *
	if (pCh->Busy || pChanExt->State != DMA_CHAN_ENABLED)
	{
		if (TraceTx){DBG_DISPLAY(" DmaChanP46TxDataHandler Busy \n");}
		return;
	}

	ACQUIRE_LOCK(&pChanExt->SegmentLock, LockFlag);
	++pCh->Busy;

	pSegment = pChanExt->Segment + pChanExt->NextSegment;

	FillDesc.len = sizeof(pRawData->Data);
	ExtendedDesc = (MAC_RESERVED_T *) &FillDesc.cb[0];
	ExtendedDesc->HeaderLength = sizeof(pRawData->Header);
	Command.CsrParam.Command = DSL_START_TX_DMA;
	Command.CsrParam.NotifyOnComplete = NULL;

	while(pSegment->State == DMA_SEGMENT_READY || pSegment->State == DMA_SEGMENT_PENDING)
	{
		pRawData = pSegment->AddressDataSegment;
		if (TraceTx){DBG_DISPLAY(" DMA Filling Segment= %x \n",pChanExt->NextSegment);}

		if(pSegment->State == DMA_SEGMENT_PENDING)
		{
			// * Resend This Segment
			Command.CsrParam.Param.StartTxDma = pSegment->CommandU.DmaCommand;
			Count =  Command.CsrParam.Param.StartTxDma.Control.Count;
		}
		else
		{
			// *
			// * Fill Empty Data Segment
			// *
			for
			(
			    Count = 0;
			    Count < pSegment->NumHardwareDescriptors;
			    Count++
			)
			{
				// Create Packet Descriptor and Call Data Stack
				pActualDesc = (* pCh->ProcessData) (pCh->UserLink, &FillDesc);
				if (FillDesc.len == 0)
				{
					// There was not any data to transmit
					break;
				}

				// *
				// * Write Cell Header
				// *
				if (ExtendedDesc->HeaderLength > 0 && ExtendedDesc->HeaderInfo != NULL)
				{
					ASSERT(ExtendedDesc->HeaderLength == sizeof(pRawData->Header) );

					COPY_MEMORY(pRawData->Header,
					            ExtendedDesc->HeaderInfo,
					            ExtendedDesc->HeaderLength);
				}

				// *
				// * Write Cell Info Buffer
				// *
				if (FillDesc.len  > 0 && FillDesc.data != NULL)
				{
					ASSERT(FillDesc.len == sizeof(pRawData->Data) );

					COPY_MEMORY(pRawData->Data,
					            FillDesc.data,
					            FillDesc.len);
						#if DMA_STATS
					pCh->BytesProcessed += FillDesc.len + ExtendedDesc->HeaderLength;
						#endif
				}
				else
				{
					break;
				}

				if (pCh->TxComplete != NULL)
				{
					(* pCh->TxComplete) ( pCh->UserLink, &FillDesc);
				}
				++pRawData;
			}

			// *
			// * If any buffers filled in the current segment we must:
			// *
			// * FlushCache Segment, Set the Fill Postion of this segment, Terminate
			// * chain at this positon, Reconnect the new chain at the old position.
			// *
			if(Count)
			{
					#if DMA_CACHE_ENABLED
				NdisFlushBuffer(pSegment->pNdisBuffer,TRUE);
					#endif

				// * Restart receiver at end of Buffer Descriptor List
				Command.CsrParam.Param.StartTxDma.Address = pSegment->PhysStartData;
				Command.CsrParam.Param.StartTxDma.Control.Handle = pChanExt->NextSegment;
				Command.CsrParam.Param.StartTxDma.Control.Count	= Count;

				pSegment->CommandU.DmaCommand = Command.CsrParam.Param.StartTxDma;
				if (TraceTx)
				{
					DBG_DISPLAY(" DMA Sending Cells.  Count: %x \n",Command.CsrParam.Param.StartTxDma.Control.Count);
				}
			}
			else
			{
				break;
			}

		} // End New SegmentFill

		pSegment->State = DMA_SEGMENT_ARM_PROCESSING;

		// * Send Segment to Hardware
		Status = ChipALWrite(
		             pChanExt->pChipAl->pNdisAdapter,
		             P46_ARM_DSL,
		             ADDRESS_BAR_0,
		             CSR_ARM_CMD,
		             sizeof (COMMAND_START_TX_DMA_T),	// Not used at this time
		             (DWORD) &Command,
		             0 								// Mask	Value is unused	by CSR_ARM_CMD
		         );

		if(Status != STATUS_PENDING && Status != STATUS_SUCCESS)
		{
			// * Some kind of error.  Hold on to data and wait for next Interrupt
			if (TraceTx)
			{DBG_DISPLAY(" DMA Arm Write Failed: Status: %lx. Pending Data Segment \n",Status);}
			pSegment->State = DMA_SEGMENT_PENDING;
			break;
		}

		pChanExt->NextSegment = (pChanExt->NextSegment + 1) % pCh->NumberOfSegments;
		pSegment = pChanExt->Segment + pChanExt->NextSegment;

		if(Count < pSegment->NumHardwareDescriptors)
		{
			// Data Stack could not fill segment.  We are done for now
			break;
		}

	}	// * Segment Loop
	--pCh->Busy;
	RELEASE_LOCK(&pChanExt->SegmentLock, LockFlag);
}


/******************************************************************************
FUNCTION NAME:
	DmaTxCompleteHandler

ABSTRACT:
	Indicates a segment was tranferred.	 In the normal case, Segment Descriptor 
	is updated to Ready.  If the segment was not completely tranferred, the
	BaseAddress of the segment and the Count are updated based on input from
	the ARM, the segment is marked as Pending, then the pointer to the next
	active segment is backed up to this point.


RETURN:
	void


DETAILS: 
	ISR Level Routine.

******************************************************************************/
MODULAR void DmaTxCompleteHandler (
    void			* pChan,
    OUT RESPONSE_T	* Response
)
{
	HW_CH_ENTRY_T		* pCh=(HW_CH_ENTRY_T*)pChan;
	DMA_BUFF_CONTROL_T		* pTxComplete;
	CHAN_EXTENSION_T		* pChanExt;
	HW_SEGMENT_DES_T		* pSegment;

	ASSERT(pCh != NULL && Response != NULL);

	pChanExt = (CHAN_EXTENSION_T *) &pCh->Extension;

	if(pChanExt->State != DMA_CHAN_ENABLED)
	{
		printk("<1>CnxADSL Xmit complete without xmit enabled!!\n");
		return;
	}

	// * Set pointer to Event as indicated by the channel configuration.
	pTxComplete = (DMA_BUFF_CONTROL_T *)&Response[pCh->Description.ElUnion.Element.Event];

	if((pTxComplete->Handle) >= pCh->NumberOfSegments)
	{
		printk("<1>CnxADSL Xmit handle out of range!!\n");

		// * Make sure Handle is in range.  This is likely a Fatal error
		return;
	}

	if (pTxComplete->Handle != pChanExt->CurrentSegment)
	{
		// *
		// * This Will Only Happen if an acknowledgement is lost.  Normally
		// * this will never happen.
		// *
		// * We will mark the segment(s) as empty and hope it was actually transmitted
		// *
		printk("<1>DmaTxCompleteHandler Acknowledge out of sequence: Got: %x  Expected: %x  Previous Total: %ld\n",
		            pTxComplete->Handle,
		            pChanExt->CurrentSegment,
		            pChanExt->OutOfSequence);
		DBG_DISPLAY("DmaTxCompleteHandler Acknowledge out of sequence: Got: %x  Expected: %x  Previous Total: %ld\n",
		            pTxComplete->Handle,
		            pChanExt->CurrentSegment,
		            pChanExt->OutOfSequence);
		for
		(
		    ;
		    pChanExt->CurrentSegment != pTxComplete->Handle;
		    pChanExt->CurrentSegment = (pChanExt->CurrentSegment + 1)%pCh->NumberOfSegments
		)
		{
			pSegment = pChanExt->Segment + pChanExt->CurrentSegment;
			pSegment->State = DMA_SEGMENT_READY;
			++pChanExt->OutOfSequence;
		}
	}


	pSegment = pChanExt->Segment + pTxComplete->Handle;

	if(pTxComplete->Count != pSegment->CommandU.DmaCommand.Control.Count)
	{
		if (TraceTx)
		{
			DBG_DISPLAY("DmaTxCompleteHandler Segment Cancelled Count: %x Handle: %x \n",pTxComplete->Count,pTxComplete->Handle);
		}
		// *
		// * Transmit did not complete  Adjust Segment pointers to account for data that
		// * may have been transmitted, back up NextSegment pointer to the partially transmitted
		// * segment, then  mark as Pending.  This should only happen as a result of a stop command.
		// *
		pSegment->CommandU.DmaCommand.Control.Count -= pTxComplete->Count;
		pSegment->CommandU.DmaCommand.Address +=	pTxComplete->Count*SEG_PTR_DATA_LEN ;
		pSegment->State = DMA_SEGMENT_PENDING;
		pChanExt->NextSegment = (BYTE) (pTxComplete->Handle);
		pChanExt->CurrentSegment = pChanExt->NextSegment;
	}
	else
	{
		pChanExt->CurrentSegment = ((BYTE)(pTxComplete->Handle + 1))% pCh->NumberOfSegments;
	}

	pSegment->State = DMA_SEGMENT_READY;
}


/******************************************************************************
FUNCTION NAME:
	DmaTxFullHandler

ABSTRACT:
	This handler is called when too many StartDma commands are issued to the 
	ARM.  If the max number of segments is consistent with the TxDma queue
	of the ARM, this routine will never be called.  If it is called, it
	marks the state as Pending for this segment and all additional segments
	that were sent to the ARM.


RETURN:
	VOID


DETAILS:
	ISR Level Routine.

******************************************************************************/
MODULAR void DmaTxFullHandler(
    void			* pChan,
    OUT RESPONSE_T	* Response
)
{
	HW_CH_ENTRY_T		* pCh=(HW_CH_ENTRY_T*)pChan;
	DMA_BUFF_CONTROL_T		* pTxFull;
	CHAN_EXTENSION_T		* pChanExt;
	HW_SEGMENT_DES_T		* pSegment;
	DWORD					Index;


	ASSERT(pCh != NULL && Response != NULL);

	pChanExt = (CHAN_EXTENSION_T *) &pCh->Extension;

	if(pChanExt->State != DMA_CHAN_ENABLED)
	{
		return;
	}

	pTxFull = (DMA_BUFF_CONTROL_T *)&Response[HW_ARM_TX_QUEUE_FULL];

	#if EXTENDED_VALIDATION
	if (TraceTx){ DBG_DISPLAY("DmaTxCompleteHandler Processing Complete \n");}
	if((pTxFull->Handle) >= pCh->NumberOfSegments)
	{
		// * Make sure Handle is in range
		if (TraceTx){DBG_DISPLAY("DmaTxFullHandler Invalid Handle:%x \n",pTxFull->Handle);}

		ASSERT(FALSE);
		return;
	}
	if (TraceTx){DBG_DISPLAY("DmaTxFullHandler Acknowledge Handle:%x \n",pTxFull->Handle);}
	#endif

	pSegment = pChanExt->Segment + (BYTE) (pTxFull->Handle);

	if
	(
	    pTxFull->Count != pSegment->CommandU.DmaCommand.Control.Count
	    || pTxFull->Handle != pSegment->CommandU.DmaCommand.Control.Handle
	)
	{
		pSegment->State = DMA_SEGMENT_PENDING;

		for
		(
		    Index = ((pTxFull->Count) + 1) % pCh->NumberOfSegments;
		    Index != (pTxFull->Count);
		    Index = (Index + 1) % pCh->NumberOfSegments
		)
		{
			pSegment = pChanExt->Segment + Index;
			if(pSegment->State == DMA_SEGMENT_ARM_PROCESSING)
			{
				pSegment->State = DMA_SEGMENT_PENDING;
			}
			else
			{
				break; 					// * If already pending or empty, we are done
			}
		}

		// * Reset start of queue back to the first pended segment
		pChanExt->NextSegment = (BYTE) (pTxFull->Handle);
	}
}


/******************************************************************************
FUNCTION NAME:
	calDmaP46AllocateSharedMemory

ABSTRACT:
	Allocated Shared memory that roughly corresponds to the requested channel 
	configuration. The segment sizes are rounded and a Min and Max number
	of segments is imposed due to restrictions of the ARM.


RETURN:
	NTSTATUS:
		-ENOMEM - A memory allocation failed
		STATUS_SUCCESS - All Ok


DETAILS:
******************************************************************************/
LOCAL INLINE NTSTATUS calDmaP46AllocateSharedMemory(
    HW_CH_ENTRY_T *  pCh
)
{
	CHAN_EXTENSION_T	* pChExt;
	DWORD				Count;
	NTSTATUS			Status;

	CHIPAL_ASSERT_TRACE
	(
	    "calDmaP46AllocateSharedMemory() ",
	    (pCh != NULL_HW_CH_ENTRY)
	)

	pChExt = (CHAN_EXTENSION_T	*) &pCh->Extension;
	pCh->NumberOfSegments = MAX_VALUE(pCh->NumberOfSegments,MIN_NUMBER_SEGMENTS);
	pCh->NumberOfSegments = MIN_VALUE(pCh->NumberOfSegments,MAX_NUMBER_SEGMENTS);


	// *
	// * Count the number of blocks that will be transferred. A Block is comprised
	// * of the length in bytes of Header and Payload that will be transferred to/from
	// * the device based on a single buffer descriptor
	// *
	Count = pCh->SegmentSize / SEG_PTR_DATA_LEN;
	pChExt->MaxBlocksPerSegment = (Count <= MAX_BLOCKS_PER_SEGMENT)?(BYTE)Count:MAX_BLOCKS_PER_SEGMENT;


	// *
	// * Round CachedSize up if necessary to maintain Cache Alignment. If we can
	// * align on any boundry, there is no need to round up.
	// *
	pChExt->CachedSize = pChExt->MaxBlocksPerSegment * SEG_PTR_DATA_LEN * pCh->NumberOfSegments;

	#if DMA_CACHE_ENABLED
	// *
	// * Calculate the max amount of cached memory.  This area us used to transfer
	// * data that will not be updated by the device, after originally written.
	// * The NonCached area is used to indicate which areas were written.
	// *
	pChExt->CacheFillSize = NdisGetCacheFillSize();

	if(pChExt->CacheFillSize > MIN_CACHE_SIZE)
	{
		// * Round up Segment Size
		pChExt->CachedSize = (pChExt->CachedSize >= pChExt->CacheFillSize)
		                     ? pChExt->CachedSize + pChExt->CachedSize % pChExt->CacheFillSize
		                     : pChExt->CacheFillSize;

	}
	#endif

	// *
	// * Attempt Allocation of Cached Memory
	// *
	pChExt->AddressCached = kmalloc( pChExt->CachedSize,
	                                 GFP_KERNEL | GFP_DMA );
	if(pChExt->AddressCached == NULL)
	{
		calDMAFreeSharedMemory(pCh);
		return(-ENOMEM);
	}

	pChExt->PhysCached = (char *)virt_to_bus(pChExt->AddressCached);

	// *
	// * Allocate private memory required to manage shared memory areas
	// *
	pChExt->SharedMgmtSize = sizeof(HW_SEGMENT_DES_T) * pCh->NumberOfSegments;

	Status = ALLOCATE_MEMORY(
	             (PVOID)&pChExt->Segment,
	             pChExt->SharedMgmtSize,
	             MEMORY_TAG );

	if(Status != STATUS_SUCCESS)
	{
		calDMAFreeSharedMemory(pCh);
		return(-ENOMEM);
	}
	CLEAR_MEMORY(pChExt->Segment, pChExt->SharedMgmtSize);

	#if DMA_CACHE_ENABLED
	// *
	// * Allocate Buffer Pool to describe each Cached Segment
	// *
	NdisAllocateBufferPool(
	    &Status,
	    &pChExt->PoolHandle,
	    pCh->NumberOfSegments
	);

	if(Status != STATUS_SUCCESS)
	{
		calDMAFreeSharedMemory(pCh);
		return(-ENOMEM);
	}
	#endif

	Status = calDMAInitSharedMemory(pCh);

	if(Status != STATUS_SUCCESS)
	{
		calDMAFreeSharedMemory(pCh);
		return(-ENOMEM);
	}
	return(Status);
}


/******************************************************************************
FUNCTION NAME:
	calDMAInitSharedMemory

ABSTRACT:
	Initializes Shared Memory and the related segment descriptors.


RETURN:
	NTSTATUS:
		STATUS_SUCCESS - All Ok

DETAILS:
******************************************************************************/
LOCAL INLINE NTSTATUS calDMAInitSharedMemory(
    HW_CH_ENTRY_T *  pCh
)
{
	CHAN_EXTENSION_T	* pChExt;
	DWORD				SegmentCount;
	HW_SEGMENT_DES_T	* pSegment;
	DWORD				SegmentOffset;
	DWORD				SegmentCacheLen;

	CHIPAL_ASSERT_TRACE
	(
	    "calDMAInitSharedMemory() ",
	    (pCh != NULL_HW_CH_ENTRY)
	)
	pChExt = (CHAN_EXTENSION_T	*) &pCh->Extension;
	SegmentCacheLen = pChExt->CachedSize / pCh->NumberOfSegments;
	pSegment = pChExt->Segment;

	for
	(
	    SegmentCount = 0;
	    SegmentCount < pCh->NumberOfSegments;
	    SegmentCount++
	)
	{
		pSegment = pChExt->Segment + SegmentCount;
		SegmentOffset = pChExt->MaxBlocksPerSegment * SEG_PTR_DATA_LEN * SegmentCount;

		// * Set address of first data block in cache segment
		pSegment->AddressDataSegment =
		    (HW_DATA_T *)
		    (
		        ((DWORD)(pChExt->AddressCached))
		        + SegmentOffset
		    );
		pSegment->PhysStartData =
		    ((DWORD)pChExt->PhysCached)
		    + SegmentOffset;

		// * Set Number of hardware descriptors to the max, which indicates all block
		// * are connected within a segment.
		pSegment->NumHardwareDescriptors = pChExt->MaxBlocksPerSegment;
		pSegment->State = DMA_SEGMENT_READY;
	}
	pChExt->CurrentSegment = 0;
	pChExt->NextSegment = 0;

	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	calDMAFreeSharedMemory

ABSTRACT:
	Frees any channel allocated shared memory and the relative segment 
	descriptors


RETURN:
	VOID


DETAILS:
******************************************************************************/
LOCAL VOID calDMAFreeSharedMemory(
    HW_CH_ENTRY_T *  pCh
)
{
	CHAN_EXTENSION_T	* pChExt;

	CHIPAL_ASSERT_TRACE
	(
	    "calDMAFreeSharedMemory() ",
	    (pCh != NULL_HW_CH_ENTRY)
	)


	pChExt = (CHAN_EXTENSION_T	*) &pCh->Extension;


	if(pChExt->AddressCached)
	{
		kfree( pChExt->AddressCached );
		pChExt->AddressCached = NULL;
	}

	if(pChExt->Segment)
	{
		FREE_MEMORY
		(
		    pChExt->Segment,
		    pChExt->SharedMgmtSize,
		    CHIPAL_CONTEXT_MEM_FLAGS
		);
		pChExt->Segment = NULL;
	}

}

MODULAR void ChipALDmaChanP46GetLastBuffer (
    HW_CH_ENTRY_T * pChanEntry,
    SK_BUFF_T *UserPkt )
{
	MAC_RESERVED_T			* ExtendedDesc;
	CHAN_EXTENSION_T	* pChExt;
	HW_SEGMENT_DES_T	* pSegment;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALDmaChanB2GetLastBuffer() ",
	    (pChanEntry != NULL_HW_CH_ENTRY)
	)


	pChExt = (CHAN_EXTENSION_T	*) &pChanEntry->Extension;
	pSegment = pChExt->Segment + pChExt->NextSegment;

	CHIPAL_ASSERT_TRACE
	(
	    "ChipALDmaChanB2GetLastBuffer",
	    (pChanEntry != NULL_HW_CH_ENTRY && UserPkt != (SK_BUFF_T *) 0)
	)
	ExtendedDesc = (MAC_RESERVED_T *) &(UserPkt->cb[0]);

	// There is no Header Data
	ExtendedDesc->HeaderLength = 0;

	// *
	// * Return from the Last Address processed. The length is set from this
	// * point to the end of the circular buffer.
	// *
	UserPkt->data = (PUCHAR) pSegment->AddressDataSegment;
	UserPkt->len  = SEG_PTR_DATA_LEN;
}
