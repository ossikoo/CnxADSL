/******************************************************************************
****	Copyright (c) 1997, 1998, 1999
****	Conexant Systems Inc. (formerly Rockwell Semiconductor Systems)
****	Personal Computing Division
****	All Rights Reserved
****
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****
*******************************************************************************


MODULE NAME:
        ATM Cell Management Layer

FILE NAME:
        CellDataTestMgmt.c

ABSTRACT:
        Contains public data structures and prototypes for ATM Cell Management Layer

KEYWORDS:
        $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CellDataTestMgmt.c $
        $Revision: 1 $
        $Date: 1/09/01 10:53a $

******************************************************************************/

#include "CardMgmt.h"

// ***************************************
//		DEBUG ITEMS
// ***************************************

#ifndef MEMORY_TAG
	#define	MEMORY_TAG	'MTDC' 
#endif

#ifndef FUNCTION_ENTRY
	#define FUNCTION_ENTRY
#endif


// *********************************
// CellDataTestMgmt misc functions
// *********************************


/*******************************************************************************

FUNCTION NAME:
		CellDataTestMgmtVCInit

ABSTRACT:
		Called by Card Management to have Cell Management initialize any per
		VC specific parameters buffers etc required to manage TX/RX cells
		that will pass between the ATM and Test Module layers.

DETAILS:

*******************************************************************************/

GLOBAL NDIS_STATUS CellDataTestMgmtVCInit
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
)
{
	FUNCTION_ENTRY

	// Get the CellDataTest VC context
	CELL_DATA_TEST_VC_T *CellDataTestCtx = VcCtx->CellDataTestMod;

	// Get the VC Cell Management context
	CDSL_VC_CELL_MGMT_T *VcCellMgmtCtx = &CellDataTestCtx->CellMgmtCtx;

	CLEAR_MEMORY (VcCellMgmtCtx, sizeof(CDSL_VC_CELL_MGMT_T));

	VcCellMgmtCtx->TXCellAllocLimit	= TXCELLALLOCLIMIT;

	VcCellMgmtCtx->CellMgmtRxCell = NULL;
	VcCellMgmtCtx->CellMgmtRx48BytePtr = NULL;
	VcCellMgmtCtx->RXCellAllocLimit	= RXCELLALLOCLIMIT;

	skb_queue_head_init(&VcCellMgmtCtx->TXCellRdyQ);
	skb_queue_head_init(&VcCellMgmtCtx->TXCellInuseQ);

	skb_queue_head_init(&VcCellMgmtCtx->RXCellRdyQ);
	skb_queue_head_init(&VcCellMgmtCtx->RXCellInuseQ);

	return STATUS_SUCCESS;
}


/*******************************************************************************

FUNCTION NAME:
		CellDataTestMgmtVCShutdown

ABSTRACT:
		Called by Card Management to have Cell Management release any per VC
		specific parameters it acquired to manage TX/RX cells for the ATM and
		Test Module layers.
		 
DETAILS:

*******************************************************************************/

GLOBAL NDIS_STATUS CellDataTestMgmtVCShutdown
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
)
{
	FUNCTION_ENTRY

	SK_BUFF_T		*Packet;
	CELL_DATA_TEST_VC_T *CellDataTestCtx;
	CDSL_VC_CELL_MGMT_T *VcCellMgmtCtx;

	if( VcCtx != NULL )
	{
		// Get the CellDataTest VC context
		CellDataTestCtx = VcCtx->CellDataTestMod;

		// Get the VC Cell Management context
		VcCellMgmtCtx = &CellDataTestCtx->CellMgmtCtx;
	}
	else
	{
		// The VC context pointer was NULL
		// !!! PROBABLY NEED TO LOG AN ERROR CONDITION HERE

		// No resources, if any were held, were freed
		return STATUS_FAILURE;
	}

	if( CellDataTestCtx != NULL && VcCellMgmtCtx != NULL )
	{
		// Check for untransmitted tx cells on the idle list and return
		// them back to either the ready queue or the Test Module.

		// NOTE: It is expected that any cells that were on the TX inuse
		//		 queue were returned by the ATM layer prior to this point
		//		 and consequently have already been released.

		while ( !skb_queue_empty(&VcCellMgmtCtx->TXCellInuseQ) )
		{
			// Remove a transmit cell from the FIFO transmit idle list.
			Packet = (SK_BUFF_T *)skb_dequeue(&VcCellMgmtCtx->TXCellInuseQ);

			if ( OOD_CELLMGMT_CPY == (OOB_DATA( Packet ) & OOD_CELLMGMT_CPY ))
			{
				// If OOD_CELLMGMT_CPY was set then we own the packet
				// and it needs to be put onto the ready queue for deallocation.

				// Place the now empty packet back on the transmit ready queue.
				skb_queue_tail( &VcCellMgmtCtx->TXCellRdyQ, Packet );
			}
			else
			{
				// If OOD_CELLMGMT_CPY was not set then we do not own the packet
				// therefore return the empty cell back to the test module.

				CellDataTestTxComplete( VcCtx, Packet );
			}
		}

		// Check for TX cells on the ready list and
		// free up the memory if there are any.

		while ( !skb_queue_empty(&VcCellMgmtCtx->TXCellRdyQ) )
		{
			// Remove a transmit cell from the FIFO transmit ready queue list.
			Packet = (SK_BUFF_T *)skb_dequeue(&VcCellMgmtCtx->TXCellRdyQ);

			FREE_MEMORY
			(
			    Packet,
			    sizeof(SK_BUFF_T) + CELL_PAYLOAD_SIZE,
			    0
			);
		}

		// Check for RX cells on the ready list and
		// free up the memory if there are any.

		while ( !skb_queue_empty(&VcCellMgmtCtx->RXCellRdyQ) )
		{
			// Remove the receive packet from the receive ready queue list.
			Packet = (SK_BUFF_T *)skb_dequeue(&VcCellMgmtCtx->RXCellRdyQ);

			FREE_MEMORY
			(
			    Packet,
			    sizeof(SK_BUFF_T) + CELL_PAYLOAD_SIZE,
			    0
			);
		}

		return STATUS_SUCCESS;
	}
	else
	{
		// One of the pointers CellDataTestCtx or VcCellMgmtCtx was NULL.
		// Because of this NO resources, if any were held, were freed.

		return STATUS_FAILURE;
	}
}


// *********************************
// CellDataTestMgmt TX functions
// *********************************

/*******************************************************************************

FUNCTION NAME:
		CellDataTestMgmtTxGetEmptyBuf

ABSTRACT:

DETAILS:

*******************************************************************************/

GLOBAL SK_BUFF_T *CellDataTestMgmtTxGetEmptyBuf
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
)
{
	// Get the CellDataTest VC context
	CELL_DATA_TEST_VC_T *CellDataTestCtx = VcCtx->CellDataTestMod;

	// Get the VC Cell Management context
	CDSL_VC_CELL_MGMT_T *VcCellMgmtCtx = &CellDataTestCtx->CellMgmtCtx;

	// Temporary pointer to the Buffer Descriptor
	SK_BUFF_T *BufDescPtr;

	if ( skb_queue_empty( &VcCellMgmtCtx->TXCellRdyQ ) )
	{
		// TXCellRdyQ does not have any empty cells that can be used.  So we need
		// to allocate memory for the cell.  But first we check that we have not
		// exceeded the allowable number of cells.

		if( VcCellMgmtCtx->TXCellAllocCnt <= VcCellMgmtCtx->TXCellAllocLimit )
		{
			NDIS_STATUS			 	NdisStatus;

			// The buffer allocation limit has not been reached yet
			NdisStatus = ALLOCATE_MEMORY
			             (
			                 (PVOID)&BufDescPtr,
			                 ( sizeof(SK_BUFF_T) + CELL_PAYLOAD_SIZE ),
			                 MEMORY_TAG
			             );

			if (NdisStatus == STATUS_SUCCESS)
			{
				// Indicate one more cell allocated
				VcCellMgmtCtx->TXCellAllocCnt++;
			}
			else
			{
				// For some reason we could not get memory for a cell
				// !!! NEED TO LOG AN ERROR CONDITION HERE
				BufDescPtr = NULL;
			}
		}
		else
		{
			// The cell allocation limit has been reached.
			// !!! NEED TO LOG AN ERROR CONDITION HERE
			BufDescPtr = NULL;
		}
	}
	else
	{
		// TXCellRdyQ already contains one or more packets that are empty
		BufDescPtr = (SK_BUFF_T *)skb_dequeue(&VcCellMgmtCtx->TXCellRdyQ);
	}

	if (BufDescPtr != NULL)
	{
		//Initialize the buffer descriptor
		BufDescPtr->data = ((BYTE *) BufDescPtr) + sizeof(SK_BUFF_T);
		BufDescPtr->len = 0;

		// head and end are initialized such that the difference
		// between them indicates the maximum size of the allocated buffer in bytes.
		BufDescPtr->head = ((BYTE *) BufDescPtr) + sizeof(SK_BUFF_T);
		BufDescPtr->end  = 	((BYTE *) BufDescPtr)
		                    + 	sizeof(SK_BUFF_T)
		                    +	CELL_PAYLOAD_SIZE;
	}

	return BufDescPtr;
}


/*******************************************************************************

FUNCTION NAME:
		CellDataTestMgmtSend

ABSTRACT:
		Called by the Test Module layer to deliver to Cell Management
		buffer descriptors that contain cell data to be queued and
		subsequently transmitted by the physical layer.

DETAILS:

*******************************************************************************/

GLOBAL NDIS_STATUS CellDataTestMgmtSend
(
    IN CDSL_VC_T		*VcCtx,			// VC context of data stream
    IN SK_BUFF_T	*BufDescPtr		// Buffer descriptor to be sent
)
{
	FUNCTION_ENTRY

	BOOL			 TXCellRdyQEmpty;
	SK_BUFF_T *Packet;
	NDIS_STATUS		 NdisStatus;


	// Get the CellDataTest VC context
	CELL_DATA_TEST_VC_T *CellDataTestCtx = VcCtx->CellDataTestMod;

	// Get the VC Cell Management context
	CDSL_VC_CELL_MGMT_T *VcCellMgmtCtx = &CellDataTestCtx->CellMgmtCtx;

	// Determine if OOD_CELLMGMT_CPY is set.  If it is then we
	// have to copy the data out of the buffer descriptor into
	// one of our own packet descriptors.

	if ( OOD_CELLMGMT_CPY == (OOB_DATA( BufDescPtr ) & OOD_CELLMGMT_CPY ) )
	{
		// We have to copy the data out of the buffer from the cell
		// data test layer so go obtain an empty packet to put it in.

		Packet = CellDataTestMgmtTxGetEmptyBuf( VcCtx );

		if ( Packet != NULL )
		{
			COPY_MEMORY
			(
			    // Destination
			    Packet->data,
			    // Source
			    BufDescPtr->data,
			    // Length
			    BufDescPtr->len
			);

			Packet->len = BufDescPtr->len;
			OOB_DATA(Packet) = OOB_DATA(BufDescPtr);
			NdisStatus = STATUS_SUCCESS;
		}
		else
		{
			// We could not obtain a packet to put the data into so
			// indicate this to cell data test layer with a failure	status.

			return STATUS_FAILURE;
		}
	}
	else
	{
		Packet = BufDescPtr;
		NdisStatus = COMMAND_PENDING;
	}

	// Note if the transmit ready queue is empty to begin with.
	TXCellRdyQEmpty = skb_queue_empty( &VcCellMgmtCtx->TXCellInuseQ );

	// Place the packet on the transmit Inuse queue.
	skb_queue_tail( &VcCellMgmtCtx->TXCellInuseQ, Packet );

	// Indicate one more NDIS xmit packet in process
	VcCellMgmtCtx->TXCellInprocCnt++;


	// If the cell copy bit is set then do not call the transmit start
	// function when the queue was empty.   The Cell Data Test layer
	// will start the transmitter after it fills the queue with some
	// number of cells.

	if ( OOD_CELLMGMT_CPY != (OOB_DATA(BufDescPtr) & OOD_CELLMGMT_CPY) )
	{
		if ( TXCellRdyQEmpty )
		{
			// The transmit buffer idle queue was empty so the transmitter
			// must be "tickled" to get it started.   Once the transmitter
			// is started and as long as we have buffers on the idle queue,
			// the interrupt handler DPC will keep it going.

			FrameALVCStartTX( VcCtx );
		}
	}

	return NdisStatus;
}


/*******************************************************************************

FUNCTION NAME:
		CellDataTestMgmtTxGetFullBuf

ABSTRACT:
		Called by ATM layer to obtain buffer descriptors that contain
		cell data that will be transmitted by the physical layer.

DETAILS:

*******************************************************************************/

GLOBAL SK_BUFF_T *CellDataTestMgmtTxGetFullBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T	*BufDescPtr		// Buffer descriptor to be processed
)
{
	FUNCTION_ENTRY

	// Get the CellDataTest VC context
	CELL_DATA_TEST_VC_T *CellDataTestCtx = VcCtx->CellDataTestMod;

	// Get the VC Cell Management context
	CDSL_VC_CELL_MGMT_T *VcCellMgmtCtx = &CellDataTestCtx->CellMgmtCtx;

	if ( skb_queue_empty( &VcCellMgmtCtx->TXCellInuseQ ) )
	{
		// VcCellMgmtCtx does not have any cells waiting to be transmitted.
		BufDescPtr->data = NULL;
		BufDescPtr->len = 0;
	}
	else
	{
		// VcCellMgmtCtx contains one or more cells waiting to be transmitted.
		VcCellMgmtCtx->CellMgmtTxCell = (SK_BUFF_T *)skb_dequeue(&VcCellMgmtCtx->TXCellInuseQ);

		//Initialize the buffer descriptor to be returned to ATM
		BufDescPtr->data = VcCellMgmtCtx->CellMgmtTxCell->data;
		BufDescPtr->len  = CELL_PAYLOAD_SIZE;
		BufDescPtr->head = VcCellMgmtCtx->CellMgmtTxCell->data;
		BufDescPtr->end  = VcCellMgmtCtx->CellMgmtTxCell->data + CELL_PAYLOAD_SIZE;
		OOB_DATA(BufDescPtr) = OOB_DATA( VcCellMgmtCtx->CellMgmtTxCell );
	}

	return BufDescPtr;
}


/*******************************************************************************

FUNCTION NAME:
		CellDataTestMgmtTxPutEmptyBuf

ABSTRACT:
		Called by ATM layer to return buffer descriptors containing
		cell data that has been transmitted by the physical layer.

DETAILS:

*******************************************************************************/

GLOBAL void CellDataTestMgmtTxPutEmptyBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T	*BufDescPtr		// Buffer descriptor to be processed
)
{
	FUNCTION_ENTRY

	// Get the CellDataTest VC context
	CELL_DATA_TEST_VC_T *CellDataTestCtx = VcCtx->CellDataTestMod;

	// Get the VC Cell Management context
	CDSL_VC_CELL_MGMT_T *VcCellMgmtCtx = &CellDataTestCtx->CellMgmtCtx;

	// If OOD_CELLMGMT_CPY is set then we allocated the buffer
	// descriptor so return it back to our transmit ready queue.

	if ( OOD_CELLMGMT_CPY == (OOB_DATA(VcCellMgmtCtx->CellMgmtTxCell) & OOD_CELLMGMT_CPY) )
	{
		// Place the now empty packet back on the transmit ready queue.
		skb_queue_tail( &VcCellMgmtCtx->TXCellRdyQ, VcCellMgmtCtx->CellMgmtTxCell );
	}

	// Indicate one less xmit cell in process
	VcCellMgmtCtx->TXCellInprocCnt--;

	// Indicate completion of transmission of the cell back to the test module
	CellDataTestTxComplete( VcCtx, VcCellMgmtCtx->CellMgmtTxCell );

	return;
}


// *********************************
// CellDataTestMgmt RX functions
// *********************************


/*******************************************************************************

FUNCTION NAME:
		CellDataTestMgmtRxGetEmptyBuf

ABSTRACT:

DETAILS:

*******************************************************************************/

GLOBAL SK_BUFF_T *CellDataTestMgmtRxGetEmptyBuf
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
)
{
	// Get the CellDataTest VC context
	CELL_DATA_TEST_VC_T *CellDataTestCtx = VcCtx->CellDataTestMod;

	// Get the VC Cell Management context
	CDSL_VC_CELL_MGMT_T *VcCellMgmtCtx = &CellDataTestCtx->CellMgmtCtx;

	// Temporary pointer to the Buffer Descriptor
	SK_BUFF_T *BufDescPtr;

	NDIS_STATUS 	NdisStatus;

	if ( skb_queue_empty( &VcCellMgmtCtx->RXCellRdyQ ) )
	{
		// RXCellRdyQ does not have any empty cells that can be used.  So we need
		// to allocate memory for the cell.  But first we check that we have not
		// exceeded the allowable number of cells.

		if( VcCellMgmtCtx->RXCellAllocCnt <= VcCellMgmtCtx->RXCellAllocLimit )
		{
			// The buffer allocation limit has not been reached yet
			NdisStatus = ALLOCATE_MEMORY
			             (
			                 (PVOID)&BufDescPtr,
			                 ( sizeof(SK_BUFF_T) + CELL_PAYLOAD_SIZE ),
			                 MEMORY_TAG
			             );

			if (NdisStatus == STATUS_SUCCESS)
			{
				// Indicate one more cell allocated
				VcCellMgmtCtx->RXCellAllocCnt++;
			}
			else
			{
				// For some reason we could not get memory for a cell
				// !!! NEED TO LOG AN ERROR CONDITION HERE
				BufDescPtr = NULL;
			}
		}
		else
		{
			// The cell allocation limit has been reached.
			// !!! NEED TO LOG AN ERROR CONDITION HERE
			BufDescPtr = NULL;
		}
	}
	else
	{
		// RXCellRdyQ already contains one or more packets that are empty
		BufDescPtr = (SK_BUFF_T *)skb_dequeue(&VcCellMgmtCtx->RXCellRdyQ);
	}

	if (BufDescPtr != NULL)
	{
		//Initialize the buffer descriptor
		BufDescPtr->data = ((BYTE *) BufDescPtr) + sizeof(SK_BUFF_T);
		BufDescPtr->len = 0;

		// head and end are initialized such that the difference
		// between them indicates the maximum size of the allocated buffer in bytes.
		BufDescPtr->head = ((BYTE *) BufDescPtr) + sizeof(SK_BUFF_T);
		BufDescPtr->end  = 	((BYTE *) BufDescPtr)
		                    + 	sizeof(SK_BUFF_T)
		                    +	CELL_PAYLOAD_SIZE;
	}

	return BufDescPtr;
}


/*******************************************************************************

FUNCTION NAME:
		CellDataTestMgmtRxPutEmptyBuf

ABSTRACT:
		Called by the Test Module layer to return empty	cell buffers to Cell Managment

DETAILS:

*******************************************************************************/

GLOBAL void CellDataTestMgmtRxPutEmptyBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T	*BufDescPtr		// Buffer descriptor to be processed
)
{
	// Get the CellDataTest VC context
	CELL_DATA_TEST_VC_T *CellDataTestCtx = VcCtx->CellDataTestMod;

	// Get the VC Cell Management context
	CDSL_VC_CELL_MGMT_T *VcCellMgmtCtx = &CellDataTestCtx->CellMgmtCtx;

	if ( BufDescPtr != NULL )
	{
		// Put the cell back on to the ready queue so that it will
		// get released or reused later.
		skb_queue_tail(&VcCellMgmtCtx->RXCellRdyQ, BufDescPtr);
	}

	return;
}


/*******************************************************************************

FUNCTION NAME:
		CellDataTestMgmtRxPutFullBuf

ABSTRACT:
		Used by ATM to deliver 48 byte cells payload and header bits to Cell Management

DETAILS:

*******************************************************************************/

GLOBAL void CellDataTestMgmtRxPutFullBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T	*BufDescPtr		// Buffer descriptor to be processed
)
{
	FUNCTION_ENTRY

	NDIS_STATUS NdisStatus;
	DWORD		OutOfBandData;

	// Get the CellDataTest VC context
	CELL_DATA_TEST_VC_T *CellDataTestCtx = VcCtx->CellDataTestMod;

	// Get the VC Cell Management context
	CDSL_VC_CELL_MGMT_T *VcCellMgmtCtx = &CellDataTestCtx->CellMgmtCtx;

	// Check for a reassembly RX buffer currently in use
	if ( VcCellMgmtCtx->CellMgmtRxCell == NULL )
	{
		// There IS NOT a RX cell currently available for
		// reassembly so try to get one from Cell Management

		VcCellMgmtCtx->CellMgmtRxCell = CellDataTestMgmtRxGetEmptyBuf
		                                (
		                                    VcCtx
		                                );

		// Did Cell Management have an RX cell for us?
		if ( VcCellMgmtCtx->CellMgmtRxCell == NULL )
		{
			// Cell Management DID NOT have an empty cell for us to use

			// so reset the RX buffer pointers

			VcCellMgmtCtx->CellMgmtRx48BytePtr = NULL;

			// Return to ATM without doing anything with the cell data
			// !!! PROBABLY NEED TO LOG THIS AS AN ERROR CONDITION
			return;
		}
		else
		{
			// Cell Management DID have an empty cell for us to use
			// so initialize the RX cell pointers appropriately.

			VcCellMgmtCtx->CellMgmtRx48BytePtr = VcCellMgmtCtx->CellMgmtRxCell->data;
		}
	}

	// We have an RX cell buffer that is currently available for our use
	// so copy the data from the ATM cell into our reassembly cell

	// NOTE: ATM can give us buffer descriptors that have payload data
	// lengths that are less than 48 bytes.  ATM will do this when the
	// physical layer DMA buffer wraps.  In this situation ATM will give
	// us the 48 byte payload data in two separate pieces.   ATM will use
	// the bit OOD_END_CELL in the out of band data field of the buffer
	// descriptor to indicate when it contains a complete cell or the
	// last partial piece of a cell.

	COPY_MEMORY
	(
	    // Destination
	    VcCellMgmtCtx->CellMgmtRx48BytePtr,

	    // Source
	    BufDescPtr->data,

	    // Length
	    BufDescPtr->len
	);

	// Update the RX cell 48 byte segment pointer and
	// RX cell length with the value of however much data
	// we were given by ATM in the buffer descriptor.

	VcCellMgmtCtx->CellMgmtRx48BytePtr += BufDescPtr->len;
	VcCellMgmtCtx->CellMgmtRxCell->len += BufDescPtr->len;

	OutOfBandData = OOB_DATA(BufDescPtr);

	// Do we have a complete cell to deliver?
	// Or have we reached the end of the RX cell?
	if
	(
	    OOD_END_CELL == (OutOfBandData & OOD_END_CELL)
	    ||	VcCellMgmtCtx->CellMgmtRx48BytePtr >= VcCellMgmtCtx->CellMgmtRxCell->end
	)
	{
		// Update the out of band data field of the cell
		// buffer for the completely reassembled cell.

		OOB_DATA(VcCellMgmtCtx->CellMgmtRxCell) = OutOfBandData;

		//	Deliver the reassembled cell to the Test Module
		NdisStatus = CellDataTestPutRxFullBuffer( VcCtx, VcCellMgmtCtx->CellMgmtRxCell );

		if ( NdisStatus != COMMAND_PENDING )
		{
			// Any Ndis status other than COMMAND_PENDING indicates that
			// the Test Module has returned ownership of the cell to us so
			// return the cell back to cell management.

			CellDataTestMgmtRxPutEmptyBuf( VcCtx, VcCellMgmtCtx->CellMgmtRxCell );
		}

		// Null out the Rx Cell pointers now that we have
		// finished with them.

		VcCellMgmtCtx->CellMgmtRxCell = NULL;
		VcCellMgmtCtx->CellMgmtRx48BytePtr = NULL;
	}
	return;
}


/*******************************************************************************

FUNCTION NAME:
		CellDataTestMgmtRxPutBufComplete

ABSTRACT:
		Used by ATM to indicate the last available cell from the raw buffer 
		has been processed.

DETAILS:

*******************************************************************************/

GLOBAL void CellDataTestMgmtRxPutBufComplete
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
)
{
	FUNCTION_ENTRY

	// This is not important to cell management but it may
	// be to the Test Module so let it know.
	CellDataTestRxComplete( VcCtx );

	return;
}


