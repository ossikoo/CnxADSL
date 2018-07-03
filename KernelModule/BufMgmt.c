/******************************************************************************
****    
****    Copyright (c) 1997, 1998
****    Rockwell International
****    All Rights Reserved
****    
****    CONFIDENTIAL and PROPRIETARY --
****            No Dissemination or use without prior written permission.
****    
****    Technical Contact:
****            Rockwell Semiconductor Systems
****            Personal Computing Division
****            Huntsville Design Center
****            6703 Odyssey Drive, Suite 303
****            Huntsville, AL   35806
****    
*******************************************************************************


MODULE NAME:
        Buffer Management

FILE NAME:
        BufMgmt.c

ABSTRACT:
        Contains the functions to support the driver data path interface 
        with NDIS.

DETAILS:
		The functions are grouped as follows and in this order, transmit,
		receive, and management.

KEYWORDS:
        $Archive:: /Projects/Tigris and Derivatives/Reference Designs/T $
        $Revision: 1 $
        $Date: 1/09/01 10:53a $

******************************************************************************/

#include "BufMgmt.h"

#ifndef MEMORY_TAG
#define	MEMORY_TAG	' WMB' 
#endif

#ifndef FUNCTION_ENTRY
	#define FUNCTION_ENTRY
#endif

#define ERROR_FATAL(error)		(LEVEL_FATAL   | MODULE_BUFFMGMT | INSTANCE_WAN | error)
#define ERROR_WARNING(error)	(LEVEL_WARNING | MODULE_BUFFMGMT | INSTANCE_WAN | error)
#define ERROR_NORMAL(error)		(LEVEL_NORMAL  | MODULE_BUFFMGMT | INSTANCE_WAN | error)


// ***************************************
//		FILE STRUCTURES
// ***************************************

typedef struct
{
	// ******************************************
	// Adapter Buffer Management Module Context structure
	// ******************************************

	// All the following are filled with values
	// obtained from reading the registry

	// The maximum receive buffer size in bytes
	DWORD		RxMaxFrameSize;

	// The maximum transmit buffer size in bytes
	DWORD		TxMaxFrameSize;

	// The maximum outstanding transmit buffer limit in bytes for NDIS
	DWORD		MaxTransmit;

	// The limit on the number of receive buffers that can be allocated
	DWORD		RXBufAllocLimit;

	// The limit for the ratio of the number of receive buffers that can be
	// indicated to NDIS before calling the NDIS receive complete function
	DWORD		RXBufRatioLimit;

} CDSL_ADAPTER_BUF_MGMT_T;


typedef struct
{
	// ******************************************
	// Link Buffer Management Module Context structure
	// ******************************************

	//********************************
	// TX Buffer Management Parameters
	//********************************

	// Packets waiting to be submitted to the controller for transmission.
	SK_BUFF_QUEUE_T TXBufRdyQ;

	// Packets currently submitted to the controller for transmission.
	SK_BUFF_QUEUE_T TXBufInuseQ;

	// The maximum transmit buffer size in bytes
	// Equals the value of the variable TxMaxFrameSize + MAX_HDR_SIZE
	// NOTE:	This parameter only exists to keep whatever MaxSendFrameSize
	//		 	value may come down in a NDIS_WAN_SET_LINK_INFO OID.

	DWORD TXBufMaxSize;

	// NDIS transmit packets currently in our process
	DWORD TXBufInprocCnt;

	// The maximum outstanding transmit buffer limit for NDIS
	DWORD MaxTransmit;

	//********************************
	// RX Buffer Management Parameters
	//********************************

	// Empty Receive Packets that can be filled when data is received.
	SK_BUFF_QUEUE_T RXBufRdyQ;

	// Receive Packets given to the controller to be filled with data
	SK_BUFF_QUEUE_T RXBufInuseQ;

	// The maximum receive buffer size in bytes
	// Equals the value of the variable RxMaxFrameSize + MAX_HDR_SIZE

	DWORD RXBufMaxSize;

	// The limit on the number of receive buffers that can be allocated
	DWORD RXBufAllocLimit;

	// The number of receive buffers that have been allocated
	DWORD RXBufAllocCnt;

	// The limit for the ratio of the number of receive buffers that can be
	// indicated to NDIS before calling the NDIS receive complete function

	DWORD RXBufRatioLimit;

	// The number of receive buffers that have been indicated to NDIS without
	// having called the NDIS receive complete function

	DWORD RXBufRatioCnt;

	// parent vc context
	CDSL_VC_T*	VcContext;

} CDSL_VC_BUF_MGMT_T;
/*
static
void BufDump(unsigned char* Buffer, long lth)
{
	long cnt;
	for (cnt=1; cnt<(lth+1); cnt++)
	{
		printk("%02x ", Buffer[cnt]);
		if (!(cnt%16)) printk("\n");
	}

	printk("\n");
}
*/
// *********************************
// TX BUFFER FUNCTIONS
// *********************************

/*******************************************************************************

FUNCTION NAME:
		BufMgmtSend

ABSTRACT:
		The function that is exported to NDIS as MiniportWanSend	

DETAILS:
		This function first checks the status of the VC context that was
		passed to this function.  If the VC status is invalid for some reason,
		this will immediately return with a STATUS_FAILURE status.	 If the
		VC is active this function will provide any necessary conversion from
		NDIS Wan packet format into the Athena packet format.  After performing
		any conversion this function will then put the Athena packet into a per-VC
		TXBufRdyQ using NDIS supplied queuing macros.   If TXBufRdyQ goes from
		empty to non-empty when this function is called, it will generate a
		"tickle" indication to the FrameAL to kick start the transmit operation.
		If the transmitter returns from this call this function will return with
		a COMMAND_PENDING status.

		If the "tickle" fails, then the packet is removed from the transmit ready
		queue and the function returns with a STATUS_FAILURE.
		
		NOTE: Spinlocking is built into the skb queueing mechanism

*******************************************************************************/
GLOBAL NDIS_STATUS BufMgmtSend
(
    IN CDSL_VC_T*		pVCStr,		// identifies the connection
    IN SK_BUFF_T*		BufDescPtr		// Buffer descriptor to be sent
)
{
	FUNCTION_ENTRY

	BOOL TXBufRdyQEmpty;

	// Get the VC Buffer Management context
	CDSL_VC_BUF_MGMT_T *VcBufMgmtCtx = pVCStr->BufMgmtMod;

	// Check that the VC has been initialized and that the buffer
	// descriptor is not null.   Also verify that the buffer descriptor
	// actually has some data in it.

	if(	VcBufMgmtCtx == NULL ||	BufDescPtr == NULL )
	{
		// Null pointers are not good
		return STATUS_FAILURE;
	}
	else
	{
		if(	BufDescPtr->data == NULL || BufDescPtr->len == 0	)
		{
			// No data to send!!
			return STATUS_FAILURE;
		}

		// Note if the transmit ready queue is empty to begin with.
		TXBufRdyQEmpty = skb_queue_empty( &VcBufMgmtCtx->TXBufRdyQ );

		// Indicate one more NDIS xmit packet in process
		VcBufMgmtCtx->TXBufInprocCnt++;

		// Place the packet on the transmit ready queue.
		skb_queue_tail( &VcBufMgmtCtx->TXBufRdyQ, BufDescPtr );

		return COMMAND_PENDING;
	}
}


/*******************************************************************************

FUNCTION NAME:
		BufMgmtTxGetFullBuf

ABSTRACT:
		Gives FrameAL transmit packets when it asks for them.

DETAILS:
		This function will take Athena packets from TXBfrRdyQ of the VC
		pointed to by the VC context and put them onto TXBfrInuseQ for the same
		VC.   The function will then return a pointer to the Athena packet buffer
		descriptor that was just put onto TXBfrInuseQ.

		NOTE: The interface accepts a buffer descriptor pointer in order to conform
		to the "standard" data interface. The standard allows the function to use
		the provided descriptor or to allocate its own. This implementation always
		ignores this supplied parameter, providing its own buffer descriptors.
				    
		NOTE: Spinlocking is built into the skb queueing mechanism

*******************************************************************************/

GLOBAL SK_BUFF_T *BufMgmtTxGetFullBuf
(
    IN	CDSL_VC_T		*VcCtx,			// VC context of data stream
    IN	SK_BUFF_T	*BufDescPtr		// Buffer descriptor that can be filled
)
{
	FUNCTION_ENTRY

	// Get the VC Buffer Management context
	CDSL_VC_BUF_MGMT_T *VcBufMgmtCtx = VcCtx->BufMgmtMod;

	if ( skb_queue_empty( &VcBufMgmtCtx->TXBufRdyQ ) )
	{
		// TXBufRdyQ does not have any packets waiting to be transmitted.
		BufDescPtr = NULL;
	}
	else
	{
		// TXBufRdyQ contains one or more packets waiting to be transmitted.
		BufDescPtr = (SK_BUFF_T *)skb_dequeue(&VcBufMgmtCtx->TXBufRdyQ);

//		printk("\n**********************\n      TRANSMIT \n");
//		BufDump( BufDescPtr->data, BufDescPtr->len );

		skb_queue_tail( &VcBufMgmtCtx->TXBufInuseQ, BufDescPtr);
	}

	return BufDescPtr;
}


/*******************************************************************************

FUNCTION NAME:
		BufMgmtTxPutEmptyBuf

ABSTRACT:
		Notifies NDIS that a buffer has been transmitted by returning it to NDIS

DETAILS:
		This function will take Athena packets from TXBfrInuseQ of the VC
		pointed to by the VC context and put them back onto TXBfrIdleQ for the
		same VC.   The function will convert the Athena packet structure back to the
		NDIS Wan packet structure as required to make a call to NdisMWanSendComplete
		to return it back to NDISWan.   Return status for the buffer will be either
		an STATUS_SUCCESS or STATUS_FAILURE.  The function will then return
		to the caller without any return value.

		NOTE: The TX and RX buffer queues operate in a FIFO manner.  Buffers must be
		returned to either the TX or RX buffer managers in the same order in which
		they were gotten.   This routine checks for this to be the case.   The routine
		will generate a call to	NDIS to indicate trasmission completion with a failure
		status if the two are not the same.  
				    
		NOTE: Spinlocking is built into the skb queueing mechanism

*******************************************************************************/

GLOBAL void BufMgmtTxPutEmptyBuf
(
    IN	CDSL_VC_T		*VcCtx,			// VC context of data stream
    IN	SK_BUFF_T		*BufDescPtr		// Buffer descriptor that has been transmitted
)
{
	FUNCTION_ENTRY

	// Get the VC Buffer Management context
	CDSL_VC_BUF_MGMT_T *VcBufMgmtCtx = VcCtx->BufMgmtMod;

	if ( skb_queue_empty(&VcBufMgmtCtx->TXBufInuseQ) )
	{
		// Somehow we were given an empty buffer from FrameAL that we did not
		// have on our inuse queue.   This should not have occured so we need
		// to generate some kind of an error condition or log at this point.

		// !!! PROBABLY NEED TO LOG AN ERROR CONDITION HERE
	}
	else
	{
		SK_BUFF_T *Packet;
		NDIS_STATUS NdisStatus;

		// Remove a transmit packet from the FIFO transmit ready list.
		// The packet removed from the queue and BufDescPtr should be the same.
		Packet = (SK_BUFF_T *)skb_dequeue(&VcBufMgmtCtx->TXBufInuseQ);

		// Verify that the buffer descriptors are the same.
		if( BufDescPtr == Packet )
		{
			// Still in sync
			NdisStatus = STATUS_SUCCESS;
		}
		else
		{
			// The transmit inuse queue list buffer descriptors and those being
			// returned to us are out of sync.
			// !!! PROBABLY NEED TO LOG AN ERROR CONDITION HERE
			NdisStatus = STATUS_FAILURE;
		}

		// release the transmit buffer back to the protocol
		RELEASE_SKB( VcBufMgmtCtx->VcContext->pVcc, Packet);

		// Indicate one less NDIS xmit packet in process
		VcBufMgmtCtx->TXBufInprocCnt--;
	}

	return;
}


// *********************************
// RX buffer functions
// *********************************

/*******************************************************************************

FUNCTION NAME:
		BufMgmtRxGetEmptyBuf

ABSTRACT:
		Supplies FrameAL with empty buffers	for receive data to put into

DETAILS:
		This function when called by FrameAL will check RXBfrRdyQ for an
		empty buffer on the VC pointed to by the VC context pointer.   If there
		is not an empty buffer on the queue and the limit as set by RXBfrAllocLimit
		has not been reached, this function will allocate a new Athena RX buffer
		descriptor and RX buffer and increment RXBfrAllocCnt.   The function will
		then initialize the Athena buffer descriptor with the appropriate values,
		put the newly allocated buffer on the RXBfrInuseQ and then return to FrameAL
		with a pointer to the empty Athena RX buffer descriptor.   If RX buffer
		allocation limit has already been reached, this function will return to
		FrameAL with a NULL pointer.

		NOTE: The memory requested from NDIS for a buffer descriptor and buffer
		is requested as one block.   The buffer descriptor structure will occupy the
		the memory block starting from the beginning address.   The buffer will occupy
		the remainder of the memory block starting from the end of the buffer
		descriptor structure. 

		NOTE: Spinlocking is built into the skb queueing mechanism

*******************************************************************************/

GLOBAL SK_BUFF_T *BufMgmtRxGetEmptyBuf
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
)
{
	FUNCTION_ENTRY

	// Get the VC Buffer Management context
	CDSL_VC_BUF_MGMT_T *VcBufMgmtCtx = VcCtx->BufMgmtMod;

	// Temporary pointer to the Buffer Descriptor
	SK_BUFF_T *BufDescPtr=NULL;

	if ( skb_queue_empty(&VcBufMgmtCtx->RXBufRdyQ) )
	{
		// allocation length
		DWORD Length =	VcBufMgmtCtx->RXBufMaxSize
		               +	MAX_HDR_PADDING
		               +	MAX_TAIL_PADDING;

		// allocate a buffer and put in the header and trailer padding
		BufDescPtr = alloc_skb(Length, GFP_ATOMIC);

		if (!BufDescPtr )
		{
			ErrPrt("<1>CnxADSL Insufficient memory\n");
		}
		else
		{
			BufDescPtr->len  = 0;
			BufDescPtr->tail = BufDescPtr->data + VcBufMgmtCtx->RXBufMaxSize;
			skb_reserve( BufDescPtr, MAX_HDR_PADDING );
		}
	}
	else
	{
		// Remove the receive packet from the receive ready queue list.
		BufDescPtr = (SK_BUFF_T *)skb_dequeue(&VcBufMgmtCtx->RXBufRdyQ);
	}

	// Put the buffer onto the inuse queue
	if ( BufDescPtr )
		skb_queue_tail(&VcBufMgmtCtx->RXBufInuseQ, BufDescPtr );

	return BufDescPtr;
}


/*******************************************************************************

FUNCTION NAME:
		BufMgmtRxPutFullBuf

ABSTRACT:
		Passes received frames from FrameAL on to NDIS	

DETAILS:
		This function is called by FrameAL when it has a full RX buffer that
		it has completed processing on and is ready for it to be delivered to NDIS.
		BufMgmtRxPutFullBuf then performs any Athena packet structure to NDIS Wan
		packet structure translation as necessary to make a call to the NDIS function
		NdisMWanIndicateReceive to give the buffer to NDIS.   When the call to this
		NDIS function returns, the buffer has been emptied by NDIS.   After returning
		from the call to NdisMWanIndicateReceive, this function will remove the buffer
		from the RXBfrInuseQ, put the empty buffer onto the RXBfrRdyQ and increment
		RXBfrRatioCnt.   If the RXBfrRatioCnt count is less than the
		NdisMWanIndicateReceive to NdisMWanIndicateReceiveComplete ratio limit as
		set by RXBfrRatioLimit the function returns immediately to FrameAL.  If
		RXBfrRatioCnt has reached the ratio limit then a call to the function
		NdisMWanIndicateReceiveComplete will be made.   This call to NDIS will cause
		the previously put buffers to be delivered to the system applications.   Upon
		returning from this call RXBfrRatioCnt is reset and the function returns to FrameAL.

 		NOTE: The TX and RX buffer queues operate in a FIFO manner.  Buffers must be
		returned to either the TX or RX buffer managers in the same order in which
		they were gotten.   This routine checks for this to be the case.   The routine
		will not generate a call to	NDIS to indicate a received frame if the two are
		not the same.  

		NOTE: Spinlocking is built into the skb queueing mechanism

*******************************************************************************/

GLOBAL void BufMgmtRxPutFullBuf(IN CDSL_VC_T *VcCtx, IN	SK_BUFF_T *BufDescPtr)
 {
 FUNCTION_ENTRY

  // Get the VC Buffer Management context
  CDSL_VC_BUF_MGMT_T *VcBufMgmtCtx=VcCtx->BufMgmtMod;
  struct atm_vcc* pVcc=VcBufMgmtCtx->VcContext->pVcc;

  if ( skb_queue_empty(&VcBufMgmtCtx->RXBufInuseQ) )
   {
   // Somehow we were given an full buffer from FrameAL that we did not
   // have on our inuse queue.   This should not have occured so we need
   // to generate some kind of an error condition or log at this point.

   // !!! PROBABLY NEED TO LOG AN ERROR CONDITION HERE
   ErrPrt("<1>CnxADSL Receive buffer queueing empty\n");
   }
  else
   {
   SK_BUFF_T *Packet;

   // Remove the receive packet from the receive inuse queue list.
   // The packet removed from the queue and BufDescPtr should be the same.
   Packet = (SK_BUFF_T *)skb_dequeue(&VcBufMgmtCtx->RXBufInuseQ);

   // Verify that the buffer descriptors are the same.  Perform the needed
   // operation if they are otherwise log an error.
   if( BufDescPtr == Packet )
    {
    // send the buffer to the protocol
    BufDescPtr->tail = BufDescPtr->data + BufDescPtr->len;

    /*CLN!!! begin*//*Do this stuff correctly s/t */
    if (atm_charge(pVcc, BufDescPtr->truesize) == 0)
     {
     }
    /*CLN!!! end*/
    pVcc->push( pVcc, BufDescPtr);
    }
   else
    {
    // The receive inuse queue list buffer descriptors and
    // those being returned to us are out of sync.
    // !!! PROBABLY NEED TO LOG AN ERROR CONDITION HERE
    ErrPrt("<1>CnxADSL Receive buffer queueing out of sync\n");
    }

   // right now just keep one buffer on the receive ready queue, we may want to
   // increase it later
   if ( skb_queue_empty(&VcBufMgmtCtx->RXBufRdyQ) )
    {
    // allocation length
    DWORD Length=VcBufMgmtCtx->RXBufMaxSize + MAX_HDR_PADDING + MAX_TAIL_PADDING;

    // allocate a buffer and put in the header and trailer padding
    BufDescPtr=alloc_skb(Length, GFP_ATOMIC);

    if (BufDescPtr)
     {
     BufDescPtr->len  = 0;
     BufDescPtr->tail = BufDescPtr->data + VcBufMgmtCtx->RXBufMaxSize;
     skb_reserve( BufDescPtr, MAX_HDR_PADDING );
     skb_queue_tail(&VcBufMgmtCtx->RXBufRdyQ, BufDescPtr );
     }
    }
   }

 return;
 }


/*******************************************************************************

FUNCTION NAME:
		BufMgmtRxPutEmptyBuf

ABSTRACT:
		Provides a mechanism for FrameAL to return empty buffers back to BM
		when the VC is being shut down or it no longer needs them.

DETAILS:
		This function can be called by FrameAL when it has received an event
		that causes it to begin shutting down.    FrameAL calls this function once
		for each Athena RX buffer that it has previously gotten passing a pointer
		to the Athena RX buffer descriptor.   As each Athena RX buffer is returned
		it is removed from the RXBfrInuseQ and put back onto the RXBfrEmptyQ.
		The buffer will be either reused or released depending upon whether the receiver
		is still running or the driver is shutting down.

		NOTE: Spinlocking is built into the skb queueing mechanism

*******************************************************************************/

GLOBAL void BufMgmtRxPutEmptyBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    IN	SK_BUFF_T	*BufDescPtr		// Buffer descriptor containing no received data
)
{
	FUNCTION_ENTRY

	// Get the VC Buffer Management context
	CDSL_VC_BUF_MGMT_T *VcBufMgmtCtx = VcCtx->BufMgmtMod;

	if ( skb_queue_empty(&VcBufMgmtCtx->RXBufInuseQ) )
	{
		// Somehow we were returned an empty buffer from FrameAL that we did not
		// have on our inuse queue.   This should not have occured so we need
		// to generate some kind of an error condition or log at this point.

		// !!! PROBABLY NEED TO LOG AN ERROR CONDITION HERE
	}
	else
	{
		SK_BUFF_T *Packet;

		// Remove the receive packet from the receive inuse queue list.
		// The packet removed from the queue and BufDescPtr should be the same.

		Packet = (SK_BUFF_T *)skb_dequeue(&VcBufMgmtCtx->RXBufInuseQ);

		// Put the packet back on to the ready queue so that it will
		// get released or reused later.
		skb_queue_tail(&VcBufMgmtCtx->RXBufRdyQ, Packet);

		// Verify that the buffer descriptors are the same.  Perform the needed
		// operation if they are otherwise log an error.
		if( BufDescPtr != Packet )
		{
			// The receive inuse queue list buffer descriptors and
			// those being returned to us are out of sync.
			// !!! PROBABLY NEED TO LOG AN ERROR CONDITION HERE
		}
	}
	return;
}


// *********************************
// TX/RX Buffer Management functions
// *********************************

/*******************************************************************************

FUNCTION NAME:
		BufMgmtCfgInit
		
ABSTRACT:
		Used to obtain registry information specific to Buffer Management

DETAILS:
		Allocates memory for a BM adapter context structure. If the memory
		allocation is unsuccessful, then return with whatever failure code
		NdisAllocateMemoryWithTag returned.  If successful	it attempts to read the
		BM specific parameters from the registry.

		If all registry parameter reads were successful the return with an
		STATUS_SUCCESS indication.
		
		If any of the registry parameter reads are unsuccessful, then deallocate
		the memory for the adapter context structure, zero the pointer to it
		and return with whatever failure code caused us to bail.
		
		NOTE:	This function makes the following assumptions: Some other module
			 	prior to this one being called has set the WindowsEnvironment
				variable in the device adapter structure.   Some other module
				has made the call to NDIS to open the registry configuration and
				obtained a registry subkey handle which is the ConfigHandle 
				parameter passed to this function.   Some other module will call
				the NDIS function to close the registry configuration.

*******************************************************************************/

GLOBAL NDIS_STATUS BufMgmtCfgInit
(
    IN	CDSL_ADAPTER_T	*AdprCtx,		// Driver supplied adapter context
    // (&ThisAdapter)  Required for events that
    // are not associated with a link
    IN PTIG_USER_PARAMS  pUserParams
)
{
	FUNCTION_ENTRY

	// Pointer to our allocated buffer management adapter context.
	CDSL_ADAPTER_BUF_MGMT_T			*BMAdprCtx;
	NDIS_STATUS						NdisStatus;

	// Allocate memory for the buffer management adapter context structure.
	NdisStatus = ALLOCATE_MEMORY
	             (
	                 (PVOID)&BMAdprCtx,
	                 sizeof(CDSL_ADAPTER_BUF_MGMT_T),
	                 MEMORY_TAG
	             );

	if (NdisStatus == STATUS_SUCCESS)
	{
		// Initialize the buffer management context structure values to NULL
		// and fill the Adapter Context structure with the pointer value
		// to the newly allocated BM adpater context structure.
		CLEAR_MEMORY (BMAdprCtx, sizeof(CDSL_ADAPTER_BUF_MGMT_T));
		AdprCtx->BufMgmtMod = BMAdprCtx;
	}
	else
	{
		// Buffer management could not allocate an adapter context
		// structure so bail out with an error status.
		return NdisStatus;
	}

	// Set the user specified data
	BMAdprCtx->RXBufAllocLimit = pUserParams->RXBufAllocLimit;
	BMAdprCtx->RXBufRatioLimit = pUserParams->RXBufRatioLimit;
	BMAdprCtx->RxMaxFrameSize  = pUserParams->RxMaxFrameSize;
	BMAdprCtx->TxMaxFrameSize  = pUserParams->TxMaxFrameSize;
	BMAdprCtx->MaxTransmit     = pUserParams->MaxTransmit;

	// Had to have been completely successful to have gotten here
	return NdisStatus;
}


/*******************************************************************************

FUNCTION NAME:
		BufMgmtAdpaterInit
		
ABSTRACT:
		To provide a point from which Buffer Management can initialize
		any adapter context specific data it may need.

DETAILS:
		This function currently does not do anything because there have not
		been any adapter specific items identified that buffer management 
		needs to operate.

*******************************************************************************/

GLOBAL NDIS_STATUS BufMgmtAdapterInit
(
    IN	CDSL_ADAPTER_T	*AdprCtx		// Driver supplied adapter context
    // (&ThisAdapter)
)
{
	// This function intentionally does nothing useful

	FUNCTION_ENTRY						//For debug purposes
	NOT_USED(AdprCtx);					//Prevent compiler warnings
	return STATUS_SUCCESS;
}


/*******************************************************************************

FUNCTION NAME:
		BufMgmtAdpaterShutdown
		
ABSTRACT:
		To provide a point from which Buffer Management can deinitialize
		any adapter context structure specific data it may have intialized.

DETAILS:
		Check to see if we had allocated an adapter context structure and
		free up the memory if we had. 

*******************************************************************************/

GLOBAL NDIS_STATUS BufMgmtAdapterShutdown
(
    IN	CDSL_ADAPTER_T	*AdprCtx		// Driver supplied adapter context
    // (&ThisAdapter)
)
{
	FUNCTION_ENTRY						//For debug purposes

	// Get the Buffer Management adapter context
	CDSL_ADAPTER_BUF_MGMT_T *BMAdprCtx = AdprCtx->BufMgmtMod;

	// If we had previously allocated an adapter context structure free it now.
	if(	BMAdprCtx != NULL )
	{
		FREE_MEMORY
		(
		    BMAdprCtx,
		    sizeof(CDSL_ADAPTER_BUF_MGMT_T),
		    0
		);
		AdprCtx->BufMgmtMod = NULL;
	}
	return STATUS_SUCCESS;
}


/*******************************************************************************

FUNCTION NAME:
		BufMgmtLinkInit
		
ABSTRACT:
		To provide a point from which Buffer Management can initialize
		any link context specific data it may need.

DETAILS:
		This function currently does not do anything because there have not
		been any link specific items identified that buffer management 
		needs to operate.

*******************************************************************************/

GLOBAL NDIS_STATUS BufMgmtLinkInit
(
    IN	CDSL_LINK_T		*LinkCtx		// Link context of data stream
)
{
	// This function intentionally does nothing useful

	FUNCTION_ENTRY						//For debug purposes
	NOT_USED(LinkCtx);					//Prevent compiler warnings
	return STATUS_SUCCESS;
}


/*******************************************************************************

FUNCTION NAME:
		BufMgmtLinkShutdown
		
ABSTRACT:
		Free up any resources used or being held by the BM for this link

DETAILS:
		This function currently does not do anything because there have not
		been any link specific items identified that buffer management 
		needs to operate.

*******************************************************************************/

GLOBAL NDIS_STATUS BufMgmtLinkShutdown
(
    IN	CDSL_LINK_T		*LinkCtx		// Link context of data stream
)
{
	// This function intentionally does nothing useful

	FUNCTION_ENTRY						//For debug purposes
	NOT_USED(LinkCtx);					//Prevent compiler warnings
	return STATUS_SUCCESS;
}


/*******************************************************************************

FUNCTION NAME:
		BufMgmtVcInit
		
ABSTRACT:
		Allocates and initializes Buffer Management VC context structure

DETAILS:
		This function is called by a Management module when a new VC has
		been established.    This function makes a call to an NDIS supplied memory
		allocation function to obtain a contiguous block of nonpaged system-space
		memory large enough to hold the buffer management VC context structure.
		After obtaining the memory for the structure, the function intializes all
		of the entries with the appropriate values and returns to the caller with
		a pointer to the buffer management VC context structure.    The presence
		of a non-NULL value in the return parameter indicates a successful
		initialization of the VC.   If memory cannot be allocated for the buffer
		management VC context structure then the function will return to the caller
		with a STATUS_FAILURE indication which would indicate an unsuccessful
		initialization of the VC.

		NOTE: Spinlocking is built into the skb queueing mechanism

*******************************************************************************/

GLOBAL NDIS_STATUS BufMgmtVcInit
(
    IN	CDSL_VC_T		*VcCtx			// Link context of data stream
)
{
	FUNCTION_ENTRY

	// Pointer for our newly allocated VC buffer management context.
	CDSL_VC_BUF_MGMT_T *VcBufMgmtCtx;

	// Get the Link context for this VC
	CDSL_LINK_T *LinkCtx = VcCtx->LinkHdl;

	// Get the pointer to Adapter context
	CDSL_ADAPTER_T *Adapter = LinkCtx->pThisAdapter;

	// Get the Adapter Buffer Management context
	CDSL_ADAPTER_BUF_MGMT_T *BMAdprCtx = Adapter->BufMgmtMod;

	// Holds the status result returned from an NDIS function call.
	NDIS_STATUS NdisStatus;

	// Allocate memory for the link buffer management context structure.
	NdisStatus = ALLOCATE_MEMORY
	             (
	                 (PVOID)&VcBufMgmtCtx,
	                 sizeof(CDSL_VC_BUF_MGMT_T),
	                 MEMORY_TAG
	             );
	if (NdisStatus == STATUS_SUCCESS)
	{
		// Initialize the buffer management context structure values to NULL.
		CLEAR_MEMORY (VcBufMgmtCtx, sizeof(CDSL_VC_BUF_MGMT_T));

		// Set Link parameter limits to those defined in the registry
		VcBufMgmtCtx->RXBufAllocLimit	= BMAdprCtx->RXBufAllocLimit;
		VcBufMgmtCtx->RXBufRatioLimit	= BMAdprCtx->RXBufRatioLimit;
		VcBufMgmtCtx->RXBufMaxSize 		= BMAdprCtx->RxMaxFrameSize + MAX_HDR_SIZE;
		VcBufMgmtCtx->TXBufMaxSize 		= BMAdprCtx->TxMaxFrameSize + MAX_HDR_SIZE;
		VcBufMgmtCtx->MaxTransmit		= BMAdprCtx->MaxTransmit;

		// Initialize the transmit and receive queue lists to empty.
		skb_queue_head_init(&VcBufMgmtCtx->TXBufRdyQ);
		skb_queue_head_init(&VcBufMgmtCtx->TXBufInuseQ);

		skb_queue_head_init(&VcBufMgmtCtx->RXBufRdyQ);
		skb_queue_head_init(&VcBufMgmtCtx->RXBufInuseQ);

		// Initialize the VC Context structure with the
		// BM context structure pointer value.
		VcCtx->BufMgmtMod = VcBufMgmtCtx;

		// save a pointer back to the vc structure for buffer release
		VcBufMgmtCtx->VcContext = VcCtx;
	}
	else
	{
		ErrPrt("<4>CnxADSL memory allocation failure\n");
	}

	return NdisStatus;
}


/*******************************************************************************

FUNCTION NAME:
		BufMgmtVcShutdown		

ABSTRACT:
		Free up any resources used or being held by the BM for this VC

DETAILS:
		This function is called by a Management module when a previously
		established VC needs to be removed.   When called this function will free
		up all remaining untransmitted buffers on the tx queue idle list and then
		deallocate all empty buffers remaining on RXBufRdyQ.   This function will
		also check to see that FrameAL has released any buffers that it may have had.
		The final operation	will be to deallocate the VC context structure for the
		buffer manager.  The return status value for this function will be dependent
		upon the success or failure of these operations.

		NOTE: Spinlocking is built into the skb queueing mechanism

*******************************************************************************/

GLOBAL NDIS_STATUS BufMgmtVcShutdown
(
    IN	CDSL_VC_T		*VcCtx			// Link context of data stream
)
{
	FUNCTION_ENTRY

	CDSL_VC_BUF_MGMT_T *VcBufMgmtCtx;
	CDSL_LINK_T *LinkCtx;
	CDSL_ADAPTER_T *Adapter;

	SK_BUFF_T *Packet;

	if( VcCtx != NULL )
	{
		// Get the Buffer Management VC context
		VcBufMgmtCtx = VcCtx->BufMgmtMod;

		// Get the Link context for this VC
		LinkCtx = VcCtx->LinkHdl;

		// Get the pointer to Adapter context
		Adapter = LinkCtx->pThisAdapter;
	}
	else
	{
		// The VC context pointer was NULL
		// !!! PROBABLY NEED TO LOG AN ERROR CONDITION HERE

		// No resources, if any were held, were freed
		return STATUS_FAILURE;
	}

	if
	(
	    VcBufMgmtCtx != NULL
	    &&	LinkCtx != NULL
	    &&	Adapter != NULL
	)
	{
		// Check for untransmitted tx buffers on the idle list and
		// return them back to NDIS with a failure status.

		while ( !skb_queue_empty(&VcBufMgmtCtx->TXBufRdyQ) )
		{
			// Remove a transmit packet from the FIFO transmit idle list.
			Packet = (SK_BUFF_T *)skb_dequeue(&VcBufMgmtCtx->TXBufRdyQ);

			// free the buffer
			RELEASE_SKB( VcBufMgmtCtx->VcContext->pVcc, Packet );
		}

		// Check for rx buffers on the ready list and
		// free up the memory if there are any.
		while ( !skb_queue_empty(&VcBufMgmtCtx->RXBufRdyQ) )
		{
			// Remove the receive packet from the receive ready queue list.
			Packet = (SK_BUFF_T *)skb_dequeue(&VcBufMgmtCtx->RXBufRdyQ);

			kfree_skb( Packet );
		}

		// Check to see that FrameAL released any buffers it may have had.
		if
		(
		    skb_queue_empty(&VcBufMgmtCtx->TXBufInuseQ)	&&
		    skb_queue_empty(&VcBufMgmtCtx->RXBufInuseQ)
		)
		{
			// FrameAL does not have any buffers
			// Deallocate the memory for the VC buffer management context structure.
			FREE_MEMORY
			(
			    VcBufMgmtCtx,
			    sizeof(CDSL_VC_BUF_MGMT_T),
			    0
			);

			// Zero the VC Context BM structure pointer since were through with it
			VcCtx->BufMgmtMod = NULL;

			return STATUS_SUCCESS;	// All resources freed
		}
		else
		{
			// FrameAL still has some buffers.  So do not
			// free up the BM context structure memory

			// !!! PROBABLY NEED TO LOG AN ERROR CONDITION HERE
			return STATUS_FAILURE;	// All resources not freed
		}
	}
	else
	{
		// The Buffer Management VC context pointer, the link
		// context pointer or the adapter context was NULL

		// !!! PROBABLY NEED TO LOG AN ERROR CONDITION HERE

		// No resources if any held were freed
		return STATUS_FAILURE;
	}
}


/******************************************************************************
	FUNCTION NAME:
		BMSetInformationHdlr

	ABSTRACT:
		This function is called by the Card Management module to set
		certain information in the miniport driver pertaining to 
		(encapsulated by) the Buffer Management module.

	RETURN:
		NDIS Status, status of information query

	DETAILS:
******************************************************************************/

GLOBAL NDIS_STATUS BMSetInformationHdlr
(
    IN CDSL_ADAPTER_T	*AdprCtx,
    IN VOID				*InfoBuffer,
    IN DWORD			InfoBufferLength,
    OUT DWORD			*BytesRead,
    OUT DWORD			*BytesNeeded
)
{
/* Not supported by Linux
		FUNCTION_ENTRY

		//	Assume the worst, and initialize to handle failure.
		NDIS_STATUS				Status = NDIS_STATUS_INVALID_LENGTH ;
		DWORD					NumBytesNeeded = 0 ;
		DWORD					NumBytesRead = 0 ;
		NDIS_WAN_SET_LINK_INFO	*LinkInfoBuf = InfoBuffer;
		CDSL_VC_T				*VcCtx;
		CDSL_VC_BUF_MGMT_T 		*VcBufMgmtCtx;

		NOT_USED(AdprCtx);

		switch (Oid)
		{
		case OID_WAN_SET_LINK_INFO:
			if ( InfoBufferLength == sizeof( NDIS_WAN_SET_LINK_INFO ) )
			{
				//	The first field in the info buffer is a MiniportVcContext
				//	which is really a pointer to an entry in our WanVcArray.

				// Pointer to the Vc context structure.
				VcCtx = (CDSL_VC_T *) (((NDIS_WAN_SET_LINK_INFO *)InfoBuffer)->NdisLinkHandle);
				if( VcCtx == NULL )
				{
					InfoBufferLength = 0;
					Status = NDIS_STATUS_INVALID_DATA;
					break;
				}

				// Get the VC Buffer Management context
				VcBufMgmtCtx = VcCtx->BufMgmtMod;

				if ( VcBufMgmtCtx == NULL )
				{
					InfoBufferLength = 0;
					Status = NDIS_STATUS_INVALID_DATA;
					break;
				}

				if( LinkInfoBuf->MaxSendFrameSize > 0 )
				{
					VcBufMgmtCtx->TXBufMaxSize = LinkInfoBuf->MaxSendFrameSize + MAX_HDR_SIZE;
				}

				if( LinkInfoBuf->MaxRecvFrameSize > 0 )
				{
					VcBufMgmtCtx->RXBufMaxSize = LinkInfoBuf->MaxRecvFrameSize + MAX_HDR_SIZE;
				}

				NumBytesRead += sizeof( LinkInfoBuf->MaxSendFrameSize ) + sizeof( LinkInfoBuf->MaxRecvFrameSize );
				NumBytesNeeded += NumBytesRead;
				Status = STATUS_SUCCESS;
				break;
			}
			else
			{
				Status = NDIS_STATUS_INVALID_LENGTH;
				NumBytesNeeded += sizeof( NDIS_WAN_SET_LINK_INFO );
				break;
			}

		default:
			//	OID either unknown or unprocessed by this module.
			Status = NDIS_STATUS_INVALID_OID;
			break;

		}

		//	Fill in the size fields before we leave.
		*BytesNeeded += NumBytesNeeded;
		*BytesRead += NumBytesRead;

		return Status ;
*/
	return STATUS_FAILURE;
}


/******************************************************************************
	FUNCTION NAME:
		BMDeviceSpecific

	ABSTRACT:
		This function coordinates all of the TAPI Device Specific (BACKDOOR)
		actions that are performed for BM (BufMgmt - Buffer Management) module.
		This function is called by the Card Management module to set
		certain information in the miniport driver pertaining to 
		(encapsulated by) the Buffer Management module.

	DETAILS:
		EVERY MODULE WHICH SUPPORTS *ANY* BACKDOOR ACTION
		(EVEN ONE) *MUST* IMPLEMENT THIS FUNCTION
		IN AT LEAST A SIMILAR MANNER OR FASHION!!!!!!!!

		BACKDOOR CODE/SUBCODE VALUES *WILL BE DESIGNED*
		TO **GUARANTEE** THAT EACH CODE/SUBCODE VALUE
		WILL BE DIRECTED TO *ONLY ONE* MODULE!!

		NOTE:	The BackDoor Status should ONLY BE SET TO FAILURE
				at the beginning of BackDoor parsing by the first
				OID parser (CardMgmt)!!!
				ALL OTHER successor OID parsing of BackDoor should
				only set the BackDoor Status when it is intended
				to be set to a specific Status (i.e., BAD_SIGNATURE,
				FAILURE, or SUCCESS)!!  The BackDoor Status should
				not be set at the beginning of BackDoor parsing
				intending to change it if needed!!!
				This is to keep other modules (BackDoor parsing)
				from overwriting the BackDoor status once a module
				has parsed the BackDoor structure and acted upon
				the data!!!
				If the BackDoor Status is not set to CODE_NOT_SUPPORTED,
				then parsing can be aborted/stopped since some other
				BackDoor parser (module) has parsed the BackDoor
				Code request and indicated an error condition or
				acted upon the request.
		Any values returned via the BackDoor scheme will have placed
		into the appropriate place in the BackDoor structure.
*******************************************************************************/
NDIS_STATUS BMDeviceSpecific(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN BACK_DOOR_T			* pBackDoorBuf,
    OUT PULONG				pNumBytesNeeded,
    OUT PULONG				pNumBytesWritten )
{
	NDIS_STATUS Status = STATUS_SUCCESS;

	// Get the Adapter Buffer Management context
	CDSL_ADAPTER_BUF_MGMT_T		* BMAdprCtx;


	//	Assume BACKDOOR ReqCode will be processed,
	//	set the BACKDOOR Status to Code Not Supported.
	//	NOTE:	The BackDoor Status should ONLY BE SET TO
	//			CODE_NOT_SUPPORTED at the beginning of BackDoor
	//			parsing by the first OID parser (CardMgmt)!!!
	//			ALL OTHER successor OID parsing of BackDoor should
	//			only set the BackDoor Status when it is intended
	//			to be set to a specific Status (i.e., BAD_SIGNATURE,
	//			FAILURE, or SUCCESS)!!  The BackDoor Status should
	//			not be set at the beginning of BackDoor parsing
	//			intending to change it if needed!!!

	if ( pBackDoorBuf->TotalSize < SIZEOF_DWORD_ALIGN( BACK_DOOR_T ) )
	{
		if ( pBackDoorBuf->TotalSize != SIZEOF_DWORD_ALIGN( MINI_BACK_DOOR_T ) )
		{
			pBackDoorBuf->ResultCode = ERROR_WARNING( RESULT_REQ_BUFF_TOO_SMALL );
			pBackDoorBuf->NeededSize = SIZEOF_DWORD_ALIGN( BACK_DOOR_T );
			*pNumBytesNeeded = SIZEOF_DWORD_ALIGN( BACK_DOOR_T );
			DBG_CDSL_DISPLAY(
			    DBG_LVL_WARNING,
			    pThisAdapter->DebugFlag,
			    (	"BMDeviceSpecific=BACK_DOOR:   Entry, Size Failure;  RC=0x%08lX;  NS=0x%08lX;  TS=0x%08lX.",
			      pBackDoorBuf->ReqCode,
			      pBackDoorBuf->NeededSize,
			      pBackDoorBuf->TotalSize) );
			return ERROR_WARNING( RESULT_REQ_BUFF_TOO_SMALL );
		}
	}

	//#if 0	//only during development or problems
	DBG_CDSL_DISPLAY(
	    DBG_LVL_MISC_INFO,
	    pThisAdapter->DebugFlag,
	    (	"BM=BACK_DOOR:  Entry; ReqCode=0x%08lX.",
	      pBackDoorBuf->ReqCode) );

	//#endif	//only during development or problems
	//xx//xxx *** ### djd temporary code
	//xxpBackDoorBuf->ResultCode = RESULT_SUCCESS;
	//xxpBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
	//xx*pNumBytesNeeded = SIZEOF_DWORD_ALIGN( BACK_DOOR_T );
	//xx*pNumBytesWritten =	sizeof( pBackDoorBuf->ResultCode )
	//xx					+ sizeof( pBackDoorBuf->NeededSize );
	//xxreturn;
	//xx//xxx *** ### djd temporary code
	switch (pBackDoorBuf->ReqCode)
	{

	case BD_BUFMGMT_GET_CONFIG:
		if ( pThisAdapter->BufMgmtMod == NULL )
		{
			pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_DATA_FAILURE );
		}
		else
		{
			BMAdprCtx = pThisAdapter->BufMgmtMod;
			pBackDoorBuf->Params.BdBufMgmtConfig.RxMaxFrameSize = BMAdprCtx->RxMaxFrameSize;
			pBackDoorBuf->Params.BdBufMgmtConfig.TxMaxFrameSize = BMAdprCtx->TxMaxFrameSize;
			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		}
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdBufMgmtConfig )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_BUFMGMT_SET_CONFIG:
		if ( pThisAdapter->BufMgmtMod == NULL )
		{
			pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_DATA_FAILURE );
		}
		else
		{
			BMAdprCtx = pThisAdapter->BufMgmtMod;
			BMAdprCtx->RxMaxFrameSize = pBackDoorBuf->Params.BdBufMgmtConfig.RxMaxFrameSize;
			BMAdprCtx->TxMaxFrameSize = pBackDoorBuf->Params.BdBufMgmtConfig.TxMaxFrameSize;
			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		}
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdBufMgmtConfig )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;


	default:
		Status = STATUS_UNSUPPORTED;
		break;
	}

	return Status;
}


// END OF BUFMGMT.C
