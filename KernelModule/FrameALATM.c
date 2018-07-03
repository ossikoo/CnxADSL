/******************************************************************************
*******************************************************************************
****	Copyright (c) 1998, 1999, 2000, 2001
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
**		FrameAL	ATM layer.
**
**	FILE NAME:
**		FrameALATM.c
**
**	ABSTRACT:
**		This files contains functionss for FrameAL ATM Layer. This includes
**		Traffic Shaping, Mux/DeMux and CellAlign functions.
**
**	DETAILS:
**		In the initial implementation only one VC is allowed. The link
**		structure will be used to manage the VC. Since only one VC is allowed,
**		MUX and DeMUX functions become passthru functions.  The CellAlign
**		functions is to fix a problem with the Basic2 Hardware and CellAlign
**		is not part of generic ATM.
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux 2.4 ATM Driver/KernelModule/FrameALATM.c $
** $Revision: 2 $
** $Date: 2/28/01 11:05a $
*******************************************************************************
******************************************************************************/

#include "CardMgmt.h"
#include "FrameAL.h"
#include "FrameALHec.h"
#include "CellDataTestMgmt.h"
#include "FrameALOAM.h"

// tigris
#define  FRAMEALATM_TRAFFIC_SHAPING 1
#define  FRAMEALATM_CELL_RCV_INTERFACE 1

#define OAM_VCI 51
#define OAM_VPI 8

// Cell align states
typedef enum  _CELL_ALIGN_STATES
{
    // initialize to start of cell because first data out of the fifo is a cell.
    START_OF_CELL = 0,
    INCOMPLETE_HEADER,
    IN_CELL,
    INCOMPLETE_CELL,
    LOST_ALIGN,
    FLAG_1,
    FLAG_2,
    FLAG_3,
    FLAG_4,
    FLAG_5,
    FLAG_6,
    FLAG_7,
    FLAG_8,
    FLAG_9,
    FLAG_10,
    FLAG_LAST
} CELL_ALIGN_STATES;

typedef enum
{
    ATM_LINK_STATE_IDLE = 0,
    ATM_LINK_STATE_ACTIVE
}	ATM_LINK_STATE;

typedef enum
{
    ATM_RX_CELL_COMPLETE = 0,
    ATM_RX_CELL_PARTIAL
}
RX_BUFFER_TYPE;

//*****************************************************************************
//   LOCAL functions prototypes.
//*****************************************************************************
LOCAL  CDSL_VC_T *	FrameALATMDeMUX
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
);


LOCAL void FrameALATMGetCell
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
);


/*******************************************************************************
FUNCTION NAME:
	FrameALATMLinkInit

ABSTRACT:	
	Initializes a new link instance for Mux/Demux.  

RETURN:
	NDIS status

DETAILS:
	This function initialize FrameALATM layer 
	
*******************************************************************************/
GLOBAL NDIS_STATUS FrameALATMLinkInit
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
)
{

	FRAMEAL_LINK_T * pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;
	P_ATM_HEADER_T TempHeader;

	// set link state to idle so start tx calls will be passed thru
	pFrameALLink->ATMLinkBlock.LinkState = ATM_LINK_STATE_IDLE;

	// make sure HEC has generated its encode/decode table
	FrameALHECInit();

	// Initailize the Traffic Shaper
#if FRAMEALATM_TRAFFIC_SHAPING
	FrATMShLinkInit( LinkHdl);
#endif 

	// init idle cell data
	CLEAR_MEMORY
	(
	    &pFrameALLink->ATMLinkBlock.IdleHeader,
	    CELL_HEADER_SIZE
	);

	pFrameALLink->ATMLinkBlock.IdleHeader.CLP = 1;

	MEMORY_SET
	(
	    &pFrameALLink->ATMLinkBlock.IdlePayload[0],
	    IDLE_PAYLOAD_CHAR,
	    CELL_PAYLOAD_SIZE
	);

	FrameALHecEncode((BYTE *) &pFrameALLink->ATMLinkBlock.IdleHeader);

	TempHeader = &pFrameALLink->ATMLinkBlock.LinkOAMHeader;

	SET_VCI( TempHeader, OAM_VCI);

	SET_VPI( TempHeader, OAM_VPI) ;

	// set the PTI field
	pFrameALLink->ATMLinkBlock.LinkOAMHeader.PTI  =  PTI_MGM_BIT | PTI_USER_BIT;

	// set the other fields to default values
	pFrameALLink->ATMLinkBlock.LinkOAMHeader.CLP = 1;
	pFrameALLink->ATMLinkBlock.LinkOAMHeader.GFC = 0;

	// calculate the HEC for this cell header
	FrameALHecEncode( (BYTE *) &pFrameALLink->ATMLinkBlock.LinkOAMHeader)	;

	// setup for OAM for this connection
	FrameALATMOAMLinkInit( LinkHdl);

	return STATUS_SUCCESS;
}

/*******************************************************************************
FUNCTION NAME:
	FrameALATMVCInit

ABSTRACT:	
	Initializes a new VC instance.  

RETURN:
	NDIS status

DETAILS:
	This function initialize VC instance which includes 
	ATM cell headers.
*******************************************************************************/
GLOBAL NDIS_STATUS FrameALATMVCInit
(
    IN	CDSL_VC_T	*VCHdl			// Link Handle to data stream
)
{
	// initialize cell headers
	WORD 				LoopCounter;
	FRAMEAL_VC_T  		*pFrameALVC;
	P_ATM_HEADER_T TempHeader;
	FRAMEAL_SHAPER_STATUS	TempTrafficShapingFlag;
	FRAMEAL_SHAPER_STATUS	OldTrafficShapingFlag=FALSE;
	FRAMEAL_LINK_T * pFrameALLink=NULL;

	if (VCHdl != NULL)
	{
		pFrameALVC 		= (FRAMEAL_VC_T*)VCHdl->FrameALVCMod;

#if FRAMEALATM_TRAFFIC_SHAPING
		if (( VCHdl->LinkHdl != NULL) && (VCHdl->LinkHdl->FrameMod != NULL))
		{
			// Update traffic shaper status
			pFrameALLink = (FRAMEAL_LINK_T*) VCHdl->LinkHdl->FrameMod;

			// turn off traffic shaping while updating the ATM layer
			OldTrafficShapingFlag = pFrameALLink->ATMLinkBlock.TrafficShapingActive;
			pFrameALLink->ATMLinkBlock.TrafficShapingActive = TRAFFIC_SHAPING_OFF;
		}
#endif 
		//
		// Fill in all possible Cell Headers
		//
		for
		(
		    LoopCounter = 0;
		    LoopCounter < PRECOMPUTED_HEADER_NUM;
		    LoopCounter++
		)
		{
			TempHeader = &pFrameALVC->ATMVCBlock.PreCompHeaders[LoopCounter];

			// fill in VCI and VPI fields of the header
			SET_VCI( TempHeader, pFrameALVC->Parameters.Vci);

			SET_VPI( TempHeader, pFrameALVC->Parameters.Vpi) ;

			// set the PTI field
			TempHeader->PTI = (LoopCounter>>1);

			// set the other fields to default values
			TempHeader->CLP = (LoopCounter & 1);
			TempHeader->GFC = 0;

			// calculate the HEC for this cell header
			FrameALHecEncode( (BYTE *) TempHeader)	;
		}

		// fill in SAP service routine addresses based on SAP info
		switch ( pFrameALVC->Parameters.VcSAP)
		{
		case VC_SAP_AAL:
			// set the addresses of AAL's server routines into the VC entry
			pFrameALVC->ATMVCBlock.TxGetFullBuf  		= FrameALAALTxGetFullBuf;
			pFrameALVC->ATMVCBlock.TxPutEmptyBuf		= FrameALAALTxPutEmptyBuf;
			pFrameALVC->ATMVCBlock.RxPutFullBuf			= FrameALAALRxPutFullBuf;

			break;

		case VC_SAP_CELLDATATEST:
			// set the addresses of AAL's server routines into the VC entry
			pFrameALVC->ATMVCBlock.TxGetFullBuf  		= CellDataTestMgmtTxGetFullBuf;
			pFrameALVC->ATMVCBlock.TxPutEmptyBuf		= CellDataTestMgmtTxPutEmptyBuf;
			pFrameALVC->ATMVCBlock.RxPutFullBuf			= CellDataTestMgmtRxPutFullBuf;

			break;
		default:

			pFrameALLink->ATMLinkBlock.TrafficShapingActive = OldTrafficShapingFlag;
			return STATUS_FAILURE;

		}

#if FRAMEALATM_TRAFFIC_SHAPING		
		if (( VCHdl->LinkHdl != NULL) && (VCHdl->LinkHdl->FrameMod != NULL))
		{
			// Add this VC to the traffic shaper
			TempTrafficShapingFlag = FrATMShVCInit( VCHdl);
			if (TempTrafficShapingFlag !=  TRAFFIC_SHAPING_NOCHANGE)
			{
				pFrameALLink->ATMLinkBlock.TrafficShapingActive = TempTrafficShapingFlag;
			}
			else
			{
				pFrameALLink->ATMLinkBlock.TrafficShapingActive = OldTrafficShapingFlag;
			}
		}
#endif 
		// set up OAM for this VC
		FrameALATMOAMVCInit (VCHdl);

		return STATUS_SUCCESS;
	}
	else
	{
		return STATUS_FAILURE;
	}
}


/*******************************************************************************
FUNCTION NAME:
	FrameALATMLinkshutdown

ABSTRACT:	
	Shutdowns an ATM layer's link instance.  

RETURN:
	NDIS Status

DETAILS:
	The memory for the FrameAL ATM layer link context is released.
	Also, any cells in the loopback queue are released. 

*******************************************************************************/
GLOBAL NDIS_STATUS FrameALATMLinkShutdown
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
)
{
	// shutdown Link traffic shaper

	FrATMShLinkShutdown (LinkHdl);

	// shutdown Link OAM
	FrameALATMOAMLinkShutdown (LinkHdl);
	
	return STATUS_SUCCESS;
}

/*******************************************************************************
FUNCTION NAME:
	FrameALATMVChutdown

ABSTRACT:	
	Shutdowns an ATM layer's VC instance.  

RETURN:
	NDIS Status

DETAILS:
	The memory for the FrameAL ATM layer link context is released.
*******************************************************************************/
GLOBAL NDIS_STATUS FrameALATMVCShutdown
(
    IN	CDSL_VC_T	*VCHdl			// Link Handle to data stream
)
{
	FRAMEAL_SHAPER_STATUS	TempTrafficShapingFlag;
	FRAMEAL_SHAPER_STATUS	OldTrafficShapingFlag;
	FRAMEAL_LINK_T * pFrameALLink;
	FRAMEAL_VC_T		*	pFrameALVC;
	DWORD 	Index;

	pFrameALVC =  (FRAMEAL_VC_T *) VCHdl->FrameALVCMod;

	if (( VCHdl->LinkHdl != NULL) && (VCHdl->LinkHdl->FrameMod != NULL))
	{
		//turn off data for this VC
		if (pFrameALVC != NULL)
		{
			pFrameALVC->ATMVCBlock.TxGetFullBuf	= NULL;
			pFrameALVC->ATMVCBlock.RxPutFullBuf = NULL;

			// check for any calls to upper layers still active
			for (Index= 0;  Index < FRAMEALVC_CNXT_NO ;Index++)
			{
				if (pFrameALVC->ATMVCBlock.InVCALCNXT[Index] == TRUE)
				{
					return COMMAND_PENDING;
				}
			}
		}

#if FRAMEALATM_TRAFFIC_SHAPING
		// Update traffic shaper status
		pFrameALLink = (FRAMEAL_LINK_T*) VCHdl->LinkHdl->FrameMod;

		// turn off traffic shaping while updating the ATM layer
		OldTrafficShapingFlag = pFrameALLink->ATMLinkBlock.TrafficShapingActive;
		pFrameALLink->ATMLinkBlock.TrafficShapingActive = TRAFFIC_SHAPING_OFF;

		// remove this VC from the traffic shaper
		TempTrafficShapingFlag = FrATMShVCShutdown( VCHdl);
		if (TempTrafficShapingFlag !=  TRAFFIC_SHAPING_NOCHANGE)
		{
			pFrameALLink->ATMLinkBlock.TrafficShapingActive = TempTrafficShapingFlag;
		}
		else
		{
			pFrameALLink->ATMLinkBlock.TrafficShapingActive = OldTrafficShapingFlag;
		}
#endif 	

		FrameALATMOAMVCShutdown (VCHdl);
		
		return STATUS_SUCCESS;
	}

	return STATUS_SUCCESS;
}

/*******************************************************************************
FUNCTION NAME:
	FrameALMUXGetTxFullBuffer

ABSTRACT:	
	Get a cell from one of the active VCs.   

RETURN:
	Pointer to NDIS WAN Packet descriptor

DETAILS:
	This function gets a buffer containing one ATM cell from SAR.
	Initially only one VC is allowed, the function justs calls SAR 
	with the link handle. When multiple VCs are supported, this function 
	will chose which VC to get the next Cell. Also, this functio will 
	do traffic shaping.  
	
*******************************************************************************/

GLOBAL SK_BUFF_T	*FrameALATMGetTxFullBuffer
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
)
{
	FRAMEAL_LINK_T  *pFrameALLink;
	P_ATM_HEADER_T pATMHeader;

	// make sure we have a link handle
	if ( LinkHdl != NULL)
	{
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;

		if (pFrameALLink != NULL)
		{
			// call sar interface function this link
			FrameALATMGetCell( LinkHdl, InBufDescPtr);

			if (InBufDescPtr->len > 0)
			{
				if (OOD_IDLE != (OOB_DATA(InBufDescPtr) & OOD_IDLE ))
				{
					pATMHeader = CELLHEAD_DATA(InBufDescPtr);

					if (pATMHeader->CLP == 0)
					{
						AddToCtrLONG(&pFrameALLink->Stats.TXATMCellsCLP0, 1);
					}
					else
					{
						AddToCtrLONG(&pFrameALLink->Stats.TXATMCellsCLP1, 1);
					}

					AddToCtrLONG(&pFrameALLink->Stats.TXFrames, 1);
					AddToCtrLONG(&pFrameALLink->Stats.TXBytes,
					            (LONG)(InBufDescPtr->len) + CELL_LTH_DATA(InBufDescPtr));
					AddToCtrLONGLONG(&pFrameALLink->Stats.COATMTXCells, 1);
					AddToCtrLONGLONG(&pFrameALLink->Stats.COTXBytes,
					            (LONG)(InBufDescPtr->len) + CELL_LTH_DATA(InBufDescPtr));
				}
				pFrameALLink->ATMLinkBlock.LinkState = ATM_LINK_STATE_ACTIVE;
			}
			else
			{
				// set link state to idle so start tx calls will be passed thru
				pFrameALLink->ATMLinkBlock.LinkState = ATM_LINK_STATE_IDLE;
			}
		}
	}

	return InBufDescPtr;
}

/*******************************************************************************
FUNCTION NAME:
	FrameALMUXTxPutEmptyBuf

ABSTRACT:	
	Handles buffer for a cell that has just been transmitted. 

RETURN:
	Pointer to NDIS WAN Packet descriptor

DETAILS:
	This function checks to see if the buffer is for a loopback cell or 
	a cell from the AAL layer. If the buffer is for a loopback cell, release 
	the memory used for the buffer. If the buffer is for an AAL layer cell
	call AAL layer and let it update its pointers.  
	
*******************************************************************************/

GLOBAL SK_BUFF_T	*FrameALATMTxPutEmptyBuf
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
)
{
	CDSL_VC_T				*   pVChdl;
	FRAMEAL_VC_T 			* 	pFrameALVC;

	if
	(
	    InBufDescPtr != NULL
	    &&	InBufDescPtr->len > 0
	    &&	OOD_IDLE != (OOB_DATA(InBufDescPtr) & OOD_IDLE )
	    &&	OOD_OAM_CELL != (OOB_DATA(InBufDescPtr) & OOD_OAM_CELL )
	)
	{
		// make sure we have a link handle
		pVChdl = VC_HNDL_DATA(InBufDescPtr);
		if (pVChdl != NULL)
		{
			pFrameALVC = (FRAMEAL_VC_T * )pVChdl->FrameALVCMod;
			if ( pFrameALVC != NULL)
			{
				if (pFrameALVC->ATMVCBlock.TxPutEmptyBuf != NULL)
				{
					pFrameALVC->ATMVCBlock.InVCALCNXT[FRAMEALVC_CNXT_PUTTX] = TRUE;
					(* pFrameALVC->ATMVCBlock.TxPutEmptyBuf)( VC_HNDL_DATA(InBufDescPtr), InBufDescPtr);
					pFrameALVC->ATMVCBlock.InVCALCNXT[FRAMEALVC_CNXT_PUTTX] = FALSE;
				}
			}
		}
	}

	return InBufDescPtr;
}


/*******************************************************************************
FUNCTION NAME:
	FrameALATMCellAlign

ABSTRACT:	
	Search the receive buffer for good cells.   

RETURN:
	Pointer to NDIS WAN Packet descriptor

DETAILS:
	This function processes raw receive buffers. The function extracts cells 
	from the receive buffer. Cells are identified by a header with a valid HEC
	and Headers are preceeded by the 11 byte file pattern (except for the first 
	cell in the buffer). Since the actual buffer is a circular buffer ChipAl
	handles raps by splitting the buffer in two pieces (first piece last position to end 
	of buffer and second piece start of buffer to current position). Since Chipal 
	passes two pieces, this function must consume all data in the buffer before 
	returning it to ChipAl. 

	When all the data in the buffe is consumed, this function will notify AAL and 
	Bufmgmt of end of buffer. They will call NDIS complete functions.  

*******************************************************************************/
GLOBAL SK_BUFF_T	*FrameALATMCellAlign
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*BufferDescPtr	// Pointer to a buffer descriptor
)
{
	FRAMEAL_LINK_T  	*	pFrameALLink;
	FRAMEAL_VC_T		*   pFrameALVC;
	CDSL_VC_T 			* pCellVC;

	// make sure chipAL passed FrameAL a good link handle
	if ( LinkHdl != NULL)
	{
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;
	}
	else
	{
		// return the buffer and let ChipAL finish processing the buffer
		return 	BufferDescPtr;
	}

	//
	// Get the Idle character from the hardware via ChipAL hal
	//

#if 	FRAMEALATM_CELL_RCV_INTERFACE
	// clear upper bits used by frameAL
	OOB_DATA(BufferDescPtr) &= ((1<< OOD_OFFSET) -1);
	OOB_DATA(BufferDescPtr) |= OOD_END_CELL;

	CELL_LTH_DATA(BufferDescPtr) = ( CELL_LTH_DATA(BufferDescPtr) & 0xffff) ;

	pCellVC = FrameALATMDeMUX
	          (
	              LinkHdl,
	              BufferDescPtr
	          );

	if (pCellVC != NULL)
	{
		pFrameALVC =  (FRAMEAL_VC_T *) pCellVC->FrameALVCMod;

		if (pFrameALVC != NULL)
		{
			pFrameALVC->ATMVCBlock.CellsToSap = 0;
		}
	}

	return BufferDescPtr;

#else
{
	BYTE				*	pBufferPointer;
	SK_BUFF_T				TempDesc;
	DWORD					CurrentIndex;
	DWORD					VCIndex;
	BOOLEAN					SearchLoopControl;
	CDSL_VC_T			*	pVCHdl;
	DWORD				NewCellCount;
	DWORD				NewPayloadCount;
	DWORD				TempIdleCell;
	DWORD				ReadStatus;

	ReadStatus = ChipALRead
	             (
	                 LinkHdl->pThisAdapter, 		// adapter handle
	                 HOBBES_FPGA, 				// Hardware device
	                 HOBBES_RX_IDLE_CHAR,
	                 sizeof( BYTE ), 			// size of data to be read
	                 &TempIdleCell
	             );
	pFrameALLink->ATMLinkBlock.IdleCharacter = (BYTE) TempIdleCell;

	if (ReadStatus != CHIPAL_SUCCESS)
	{
		//
	}

	// if a HARDWARE loopback is needed set the loopback bit in the Hobbs control register
#if 0
	ReadStatus = ChipALWrite
	             (
	                 LinkHdl->pThisAdapter, 		// adapter handle
	                 HOBBES_FPGA, 				// Hardware device
	                 HOBBES_CONTROL,
	                 sizeof( BYTE ), 			// size of data to be read
	                 LOOP1, // value to be written
	                 LOOP1 // mask value
	             );
	if (ReadStatus != CHIPAL_SUCCESS)
	{
		//

	}
#endif

	// Set up the loop to process the buffer of raw data
	CurrentIndex = 0;
	pBufferPointer = BufferDescPtr->CurrentBuffer;
	SearchLoopControl = TRUE;

	// Loop thru the raw receive buffer finding and processing cells
	while  (SearchLoopControl)
	{

		switch ( pFrameALLink->ATMLinkBlock.CellAlignState)
		{
		case START_OF_CELL:

			// first check for a complete cell in the buffer
			if (( BufferDescPtr->len - CurrentIndex) <
			        ( CELL_HEADER_SIZE + CELL_PAYLOAD_SIZE ))
			{

				// reset payload count
				pFrameALLink->ATMLinkBlock.RxPayloadCount = 0;

				// First. check that the complete header is in the buffer
				if (( BufferDescPtr->len - CurrentIndex) <
				        ( CELL_HEADER_SIZE ))

				{
					// copy first portion of the cell to the tempory header buffer in the
					// the link structure
					memcpy
					(
					    (void *) &pFrameALLink->ATMLinkBlock.RxCellHeader,
					    (void *) pBufferPointer,
					    BufferDescPtr->len - CurrentIndex
					);

					pFrameALLink->ATMLinkBlock.RxHeaderCount =
					    BufferDescPtr->len - CurrentIndex;

					CurrentIndex =  BufferDescPtr->len;

					pFrameALLink->ATMLinkBlock.CellAlignState = INCOMPLETE_CELL;
					SearchLoopControl = FALSE;
				}
				else
				{
					// check HEC if good goto in cell state
					// and process the cell
					// NOTE: the incell state could be moved to this
					// if statement block

					if ( FrameALHecDecode(pBufferPointer) == HEC_GOOD )
					{
						// the link structure
						memcpy
						(
						    (void *) &pFrameALLink->ATMLinkBlock.RxCellHeader,
						    (void *) pBufferPointer,
						    CELL_HEADER_SIZE
						);

						pFrameALLink->ATMLinkBlock.RxHeaderCount =  CELL_HEADER_SIZE;

						// set header pointer in wan packet descriptor
						// to point to cell header
						CELLHEAD_DATA(&TempDesc) = pBufferPointer;
						CELL_LTH_DATA(&TempDesc) = CELL_HEADER_SIZE;

						// move buffer pointer to cell payload
						pBufferPointer +=  CELL_HEADER_SIZE;
						CurrentIndex += CELL_HEADER_SIZE;

						// set current buffer pointer of WAN packet descriptor
						// to cell payload
						TempDesc.CurrentBuffer = pBufferPointer;
						TempDesc.len =  BufferDescPtr->len - CurrentIndex;

						// OOD_END_CELL bit is already clear

						// clear upper bits used by frameAL
						OOB_DATA(&TempDesc) = 0;

						// move buffer pointer past cell payload
						pBufferPointer +=  BufferDescPtr->len - CurrentIndex;
						CurrentIndex += TempDesc.len;

						pFrameALLink->ATMLinkBlock.CellAlignState =  INCOMPLETE_CELL;
						SearchLoopControl = FALSE;

						// send the cell to demux which will send the cell
						// on to SAR
						pFrameALLink->ATMLinkBlock.RxPayloadCount = TempDesc.len;

						FrameALATMDeMUX
						(
						    LinkHdl,
						    &TempDesc
						);
						CurrentIndex =  BufferDescPtr->len;

						pFrameALLink->ATMLinkBlock.CellAlignState = INCOMPLETE_CELL;
						SearchLoopControl = FALSE;
						break;
					}
					else
					{
						// Not a good HEC, drop in to search mode
						//
						if ( *pBufferPointer == pFrameALLink->ATMLinkBlock.IdleCharacter)
						{
							pFrameALLink->ATMLinkBlock.CellAlignState = FLAG_1;
						}
						else
						{
							pFrameALLink->ATMLinkBlock.CellAlignState = LOST_ALIGN;
						}

						pBufferPointer += 1;
						CurrentIndex +=1;
					}
				}
			}
			else
			{
				// check HEC if good goto in cell state
				// and process the cell
				// NOTE: the incell state could be moved to this
				// if statement block

				if ( FrameALHecDecode(pBufferPointer) == HEC_GOOD )
				{
					pFrameALLink->ATMLinkBlock.CellAlignState = IN_CELL;
				}
				else
				{
					// Not a good HEC, drop in to search mode
					//
					if ( *pBufferPointer == pFrameALLink->ATMLinkBlock.IdleCharacter)
					{
						pFrameALLink->ATMLinkBlock.CellAlignState = FLAG_1;
					}
					else
					{
						pFrameALLink->ATMLinkBlock.CellAlignState = LOST_ALIGN;
					}

					pBufferPointer += 1;
					CurrentIndex +=1;
				}
			}
			break;

		case INCOMPLETE_CELL:

			NewCellCount = 0;

			// first check for a complete header in temp header buffer
			if (pFrameALLink->ATMLinkBlock.RxHeaderCount < CELL_HEADER_SIZE)
			{

				NewCellCount = 	CELL_HEADER_SIZE - pFrameALLink->ATMLinkBlock.RxHeaderCount;

				if (( BufferDescPtr->len - CurrentIndex) < ( NewCellCount ))
				{
					NewCellCount = BufferDescPtr->len - CurrentIndex;
				}

				// copy first portion of the cell to the tempory header buffer in the
				// the link structure
				memcpy
				(
				    (void *) (((BYTE*)&pFrameALLink->ATMLinkBlock.RxCellHeader)+
				              pFrameALLink->ATMLinkBlock.RxHeaderCount),
				    (void *) pBufferPointer,
				    NewCellCount
				);

				pBufferPointer += NewCellCount;
				CurrentIndex +=  NewCellCount;

				if ((pFrameALLink->ATMLinkBlock.RxHeaderCount + NewCellCount )< CELL_HEADER_SIZE)
				{
					pFrameALLink->ATMLinkBlock.RxHeaderCount += NewCellCount;
					SearchLoopControl = FALSE;
					// check if if now have a complete cell
					break;
				}

				// Determine if the HEC needs to tested.
				if ((pFrameALLink->ATMLinkBlock.RxHeaderCount + NewCellCount )< CELL_HEADER_SIZE)
				{
					if ( FrameALHecDecode((void *) (&pFrameALLink->ATMLinkBlock.RxCellHeader)) != HEC_GOOD )
					{
						if ( *pBufferPointer == pFrameALLink->ATMLinkBlock.IdleCharacter)
						{
							pFrameALLink->ATMLinkBlock.CellAlignState = FLAG_1;
						}
						else
						{
							pFrameALLink->ATMLinkBlock.CellAlignState = LOST_ALIGN;
						}

						pBufferPointer += 1;
						CurrentIndex +=1;
						break;

					}
				}
			}

			// set header pointer in wan packet descriptor
			// to point to cell header
			CELLHEAD_DATA(&TempDesc) = (PVOID) (&pFrameALLink->ATMLinkBlock.RxCellHeader);
			CELL_LTH_DATA(&TempDesc) = CELL_HEADER_SIZE;

			// set current buffer pointer of WAN packet descriptor
			// to cell payload
			TempDesc.CurrentBuffer = pBufferPointer;
			NewPayloadCount = CELL_PAYLOAD_SIZE - pFrameALLink->ATMLinkBlock.RxPayloadCount;

			if 	( NewPayloadCount > (BufferDescPtr->len - CurrentIndex ))
			{

				NewPayloadCount =  BufferDescPtr->len - CurrentIndex;
			}

			TempDesc.len =  NewPayloadCount;

			// OOD_END_CELL bit is already clear

			// move buffer pointer past cell payload
			pBufferPointer +=  NewPayloadCount;
			CurrentIndex += NewPayloadCount;

			// clear upper bits used by frameAL
			OOB_DATA(&TempDesc)  = 0;

			if (( NewPayloadCount + pFrameALLink->ATMLinkBlock.RxPayloadCount )
			        <  CELL_PAYLOAD_SIZE)
			{
				pFrameALLink->ATMLinkBlock.CellAlignState =  INCOMPLETE_CELL;
				SearchLoopControl = FALSE;
				pFrameALLink->ATMLinkBlock.RxPayloadCount +=  NewPayloadCount;
			}
			else
			{
				pFrameALLink->ATMLinkBlock.CellAlignState =  FLAG_1;
				OOB_DATA(&TempDesc) |= OOD_END_CELL;
			}

			// send the cell to demux which will send the cell
			// on to SAR
			FrameALATMDeMUX
			(
			    LinkHdl,
			    &TempDesc
			);

			break;

		case IN_CELL:
			// Found a cell (good HEC)

			// first check for a complete cell in the buffer
			if (( BufferDescPtr->len - CurrentIndex) <
			        ( CELL_HEADER_SIZE + CELL_PAYLOAD_SIZE ))
			{
				// complete cell is not in the buffer
				// exit and wait for next buffer
				SearchLoopControl = FALSE;
				break;
			}

			// set header pointer in wan packet descriptor
			// to point to cell header
			CELLHEAD_DATA(&TempDesc) = pBufferPointer;
			CELL_LTH_DATA(&TempDesc) = CELL_HEADER_SIZE;

			// move buffer pointer to cell payload
			pBufferPointer +=  CELL_HEADER_SIZE;
			CurrentIndex += CELL_HEADER_SIZE;

			// set current buffer pointer of WAN packet descriptor
			// to cell payload
			TempDesc.CurrentBuffer = pBufferPointer;
			TempDesc.len = CELL_PAYLOAD_SIZE;

			// move buffer pointer past cell payload
			pBufferPointer += CELL_PAYLOAD_SIZE;
			CurrentIndex += CELL_PAYLOAD_SIZE;

			pFrameALLink->ATMLinkBlock.CellAlignState =  FLAG_1;
			// send the cell to demux which will send the cell
			// on to SAR

			// mark this cell as complete

			OOB_DATA(&TempDesc) = OOD_END_CELL;

			FrameALATMDeMUX
			(
			    LinkHdl,
			    &TempDesc
			);
			break;

		case LOST_ALIGN:

			// search for first idle character
			if ( *pBufferPointer == pFrameALLink->ATMLinkBlock.IdleCharacter)
			{
				pFrameALLink->ATMLinkBlock.CellAlignState = FLAG_1;
				// in ATM, use rx error counter to report lost align errors
				AddToCtrLONG(&pFrameALLink->Stats.RXATMCellsBadHEC, 1);
				AddToCtrLONG(&pFrameALLink->Stats.COATMRXCellsDrops, 1);

			}

			pBufferPointer += 1;
			CurrentIndex +=1;
			break;

		case FLAG_1:
		case FLAG_2:
		case FLAG_3:
		case FLAG_4:
		case FLAG_5:
		case FLAG_6:
		case FLAG_7:
		case FLAG_8:
		case FLAG_9:
		case FLAG_10:
			// Check for first idle character
			if ( *pBufferPointer == pFrameALLink->ATMLinkBlock.IdleCharacter)
			{
				pFrameALLink->ATMLinkBlock.CellAlignState++;
				pBufferPointer += 1;
				CurrentIndex +=1;

			}
			else
			{
				pFrameALLink->ATMLinkBlock.CellAlignState = START_OF_CELL;
			}

			break;

		case FLAG_LAST:
			// Check for idle character
			// If it is idle character, it should be the last one before a header
			if ( *pBufferPointer == pFrameALLink->ATMLinkBlock.IdleCharacter)
			{
				pBufferPointer += 1;
				CurrentIndex +=1;
			}

			pFrameALLink->ATMLinkBlock.CellAlignState = START_OF_CELL;

			break;

		default:
			// should never get a default case
			ASSERT( FALSE);
			break;
		}

		// Check for end of buffer
		if ( CurrentIndex == BufferDescPtr->len)
		{
			SearchLoopControl = FALSE;
		}
	}

	// Determine if AAL Rx complete needs to called
	for
	(
	    VCIndex = 0;
	    VCIndex < MAX_VC_PER_LINK;
	    VCIndex++
	)
	{
		pVCHdl = &LinkHdl->VC_Info[VCIndex];

		if (pVCHdl != NULL)
		{
			pFrameALVC =  (FRAMEAL_VC_T *) pVCHdl->FrameALVCMod;

			if (pFrameALVC != NULL)
			{
				pFrameALVC->ATMVCBlock.CellsToSap = 0;
			}
		}
	}

	// Inform ChipAL how much of the buffer was consumed
	BufferDescPtr->len = CurrentIndex;

	// Return the WAN packet descriptor to ChipAL
	return BufferDescPtr;
}
#endif 
}


/*******************************************************************************
FUNCTION NAME:
	FrameALATMDeMux

ABSTRACT:	
	Sent a cell to SAR with the proper VC handle.   

RETURN:
	None 

DETAILS:
	This function sends a buffer containing one ATM cell from SAR.
	Initially only one VC is allowed, the function justs calls SAR 
	with the link handle. When multiple VCs are supported, this function 
	will chose which VC handle for the Cell.  
	
*******************************************************************************/

LOCAL CDSL_VC_T	* FrameALATMDeMUX
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
)
{

	P_ATM_HEADER_T 		pCellHeader = CELLHEAD_DATA(InBufDescPtr);
	FRAMEAL_LINK_T 		*pFrameALLink;
	WORD				VCIndex;
	CDSL_VC_T			*pVCHdl = NULL;
	FRAMEAL_VC_T  		*pFrameALVC = NULL;
	WORD 				TempVci;
	WORD 				TempVpi;
	DWORD 				TempOOD;

	// make sure chipAL passed FrameAL a good link handle
	if ( LinkHdl != NULL)
	{
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;
	}
	else
	{
		// return the buffer and let ChipAL finish processing the buffer
		return NULL;
	}

	TempOOD = OOB_DATA(InBufDescPtr);
	TempVci = GET_VCI(pCellHeader);
	TempVpi = GET_VPI(pCellHeader);

	// check for idle cell
	if ((TempVci == 0) &&  (TempVpi == 0))
	{
		return NULL;
	}

	for
	(
	    VCIndex = 0;
	    VCIndex < MAX_VC_PER_LINK;
	    VCIndex++
	)
	{
		pVCHdl = &LinkHdl->VC_Info[VCIndex];
		if ( pVCHdl != NULL)
		{
			pFrameALVC =  (FRAMEAL_VC_T *) pVCHdl->FrameALVCMod;
			if (pFrameALVC != NULL)
			{
				// check VCI/VPI for correct match
				if (( pFrameALVC->Parameters.Vci == TempVci) &&
				        ( pFrameALVC->Parameters.Vpi == TempVpi))
				{
					if ((OOB_DATA(InBufDescPtr) & OOD_END_CELL) == OOD_END_CELL)
					{
						if (pCellHeader->CLP == 0)
						{
							AddToCtrLONG(&pFrameALLink->Stats.RXATMCellsGoodCLP0, 1);
							AddToCtrLONG(&pFrameALVC->Stats.RXATMCellsGoodCLP0, 1);
						}
						else
						{
							AddToCtrLONG(&pFrameALLink->Stats.RXATMCellsGoodCLP1, 1);
							AddToCtrLONG(&pFrameALVC->Stats.RXATMCellsGoodCLP1, 1);
						}

						AddToCtrLONGLONG(&pFrameALLink->Stats.COATMRXCells, 1);
						AddToCtrLONGLONG(&pFrameALVC->Stats.COATMRXCells, 1);
						AddToCtrLONG(&pFrameALLink->Stats.RXFrames, 1);
						AddToCtrLONG(&pFrameALLink->Stats.RXBytes, CELL_PAYLOAD_SIZE + CELL_HEADER_SIZE);
						AddToCtrLONG(&pFrameALVC->Stats.RXBytes, CELL_PAYLOAD_SIZE + CELL_HEADER_SIZE);
						AddToCtrLONGLONG(&pFrameALLink->Stats.CORXBytes, CELL_PAYLOAD_SIZE + CELL_HEADER_SIZE);
						AddToCtrLONGLONG(&pFrameALVC->Stats.CORXBytes, CELL_PAYLOAD_SIZE + CELL_HEADER_SIZE);
					}
					break;
				}
			}
		}
	}

	// check for mgmt cells
	if ((TempVci == 3) || (TempVci == 4))
	{
		FrameALOAMRxBuffer
		(
		    LinkHdl,
		    NULL,
		    InBufDescPtr
		);

		return NULL;
	}

	if (((pCellHeader->PTI & PTI_MGM_BIT) == PTI_MGM_BIT) && (pFrameALVC != NULL))
	{
		FrameALOAMRxBuffer
		(
		    LinkHdl,
		    pVCHdl,
		    InBufDescPtr
		);

		return NULL;
	}


	if (pFrameALVC == NULL)
	{

		if ((OOB_DATA(InBufDescPtr) & OOD_END_CELL) == OOD_END_CELL)
		{
			AddToCtrLONG(&pFrameALLink->Stats.RXATMCellsMisrouted, 1);
			AddToCtrLONGLONG(&pFrameALLink->Stats.COATMRXCellsDrops, 1);
		}
		return NULL;
	}

	if (!(( pFrameALVC->Parameters.Vci == TempVci) &&
	        ( pFrameALVC->Parameters.Vpi == TempVpi)))
	{
		if ((OOB_DATA(InBufDescPtr) & OOD_END_CELL) == OOD_END_CELL)
		{
			AddToCtrLONG(&pFrameALLink->Stats.RXATMCellsMisrouted, 1);
			AddToCtrLONGLONG(&pFrameALLink->Stats.COATMRXCellsDrops, 1);

		}
		return NULL;
	}

	TempOOD = OOB_DATA(InBufDescPtr);
	TempOOD |= ((((( PTI_USER_BIT | PTI_CI_BIT ) & pCellHeader->PTI) << 1) | pCellHeader->CLP) << OOD_OFFSET);
	OOB_DATA(InBufDescPtr) = TempOOD;

	// mark this vc that it delivered a cell to aal.
	pFrameALVC->ATMVCBlock.CellsToSap++;

	if (pFrameALVC->ATMVCBlock.RxPutFullBuf != NULL)
	{
		pFrameALVC->ATMVCBlock.InVCALCNXT[FRAMEALVC_CNXT_PUTRX] = TRUE;

		(*pFrameALVC->ATMVCBlock.RxPutFullBuf)(pVCHdl, InBufDescPtr);

		pFrameALVC->ATMVCBlock.InVCALCNXT[FRAMEALVC_CNXT_PUTRX] = FALSE;

	}

	// call SAR with the cell and VC handle (Link Handle)
	return pVCHdl;

}

/*******************************************************************************
FUNCTION NAME:
	FrameALATMGetCell

ABSTRACT:	
	Sent a cell to SAR with the proper VC handle.   

RETURN:
	None 

DETAILS:
	This function gets a buffer containing one ATM cell from SAR.
	Initially only one VC is allowed, the function justs calls SAR 
	with the link handle. When multiple VCs are supported, this function 
	will chose which VC handle for the Cell.  
	
*******************************************************************************/

LOCAL void	FrameALATMGetCell
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
)
{

	FRAMEAL_LINK_T  	*	pFrameALLink;
	DWORD 					CellIndex;
	CDSL_VC_T			*	pVCHdl;
	FRAMEAL_VC_T		*	pFrameALVC;
	ATM_HEADER_T		*   TempCellHeaderPtr;
	DWORD					TempOOD;

	// make sure buffer is marked as empty
	InBufDescPtr->len = 0;

	// clear upper bits used by frameAL
	OOB_DATA(InBufDescPtr) &= ((1<< OOD_OFFSET) -1);
	// make sure chipAL passed FrameAL a good link handle
	if ( LinkHdl != NULL)
	{
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;
	}
	else
	{
		// return the buffer and let ChipAL finish processing the buffer
		return;
	}

	FrameALOAMLinkGetTxFullBuffer( LinkHdl,InBufDescPtr);
	if (InBufDescPtr->len !=  0)
	{
		if (VC_HNDL_DATA(InBufDescPtr) != NULL)
		{
			pFrameALVC =   (FRAMEAL_VC_T *) (VC_HNDL_DATA(InBufDescPtr))->FrameALVCMod;
			AddToCtrLONG(&pFrameALVC->Stats.TXATMCellsMgmCell, 1);
			pFrameALVC = NULL;
		}

		OOB_DATA(InBufDescPtr) |= OOD_OAM_CELL;
		VC_HNDL_DATA(InBufDescPtr) = (CDSL_VC_T*)NULL;
		AddToCtrLONG(&pFrameALLink->Stats.TXATMCellsMgmCell, 1);
	}
	else
	{

	#if FRAMEALATM_TRAFFIC_SHAPING
		if (pFrameALLink->ATMLinkBlock.TrafficShapingActive == TRAFFIC_SHAPING_OFF)
	#endif 
		{
			for
			(
			    CellIndex = 0;
			    CellIndex <  MAX_VC_PER_LINK;
			    CellIndex ++
			)
			{
				pVCHdl = &LinkHdl->VC_Info[pFrameALLink->ATMLinkBlock.CurrentVC];

				if (pFrameALLink->ATMLinkBlock.CurrentVC < (MAX_VC_PER_LINK - 1))
				{
					pFrameALLink->ATMLinkBlock.CurrentVC++;
				}
				else
				{
					pFrameALLink->ATMLinkBlock.CurrentVC =  0;
				}

				pFrameALVC =  (FRAMEAL_VC_T *) pVCHdl->FrameALVCMod;

				if ((pFrameALVC != NULL) && (pFrameALVC->ATMVCBlock.TxGetFullBuf != NULL))
				{
					pFrameALVC->ATMVCBlock.InVCALCNXT[FRAMEALVC_CNXT_GETTX] = TRUE;

					// 	 TxGetFullBuf returns a pointer but it is the same as InBufDescPtr
					(*pFrameALVC->ATMVCBlock.TxGetFullBuf) (pVCHdl, InBufDescPtr);
					pFrameALVC->ATMVCBlock.InVCALCNXT[FRAMEALVC_CNXT_GETTX] = FALSE;
				}

				if ( InBufDescPtr->len > 0)
				{
					break;
				}
			}

			if ( InBufDescPtr->len != 0)
			{
				TempOOD = OOB_DATA(InBufDescPtr) >> OOD_OFFSET;
				TempOOD &= (PRECOMPUTED_HEADER_NUM-1);

				CELLHEAD_DATA(InBufDescPtr) = &(pFrameALVC->ATMVCBlock.PreCompHeaders[TempOOD]);
				CELL_LTH_DATA(InBufDescPtr) = CELL_HEADER_SIZE;
				VC_HNDL_DATA(InBufDescPtr)  = pVCHdl;
				AddToCtrLONG(&pFrameALVC->Stats.TXBytes,
				            InBufDescPtr->len + CELL_LTH_DATA(InBufDescPtr));
				AddToCtrLONGLONG(&pFrameALVC->Stats.COTXBytes,
				            InBufDescPtr->len + CELL_LTH_DATA(InBufDescPtr));

				// update VC stats
				TempCellHeaderPtr = CELLHEAD_DATA(InBufDescPtr);

				if (TempCellHeaderPtr->CLP == 0)
				{
					AddToCtrLONG(&pFrameALVC->Stats.TXATMCellsCLP0, 1);
				}
				else
				{
					AddToCtrLONG(&pFrameALVC->Stats.TXATMCellsCLP1, 1);
				}

				AddToCtrLONGLONG(&pFrameALVC->Stats.COATMTXCells, 1);
			}
		}
		else
		{
			FrATMShGetCell( LinkHdl, InBufDescPtr);
			if ( InBufDescPtr->len == 0)
			{
				#if 0
				CELLHEAD_DATA(InBufDescPtr) = &pFrameALLink->ATMLinkBlock.IdleHeader;
				CELL_LTH_DATA(InBufDescPtr) = CELL_HEADER_SIZE;
				InBufDescPtr->len = CELL_PAYLOAD_SIZE;
				InBufDescPtr->CurrentBuffer = &pFrameALLink->ATMLinkBlock.IdlePayload[0];
				OOB_DATA(InBufDescPtr) = (PVOID) OOD_IDLE;
				VC_HNDL_DATA(InBufDescPtr) =   NULL;
			    #endif 
			}
			else
			{
				pVCHdl =  VC_HNDL_DATA(InBufDescPtr);

				pFrameALVC =  (FRAMEAL_VC_T *) pVCHdl->FrameALVCMod;

				TempOOD = OOB_DATA(InBufDescPtr) >> OOD_OFFSET;
				TempOOD &= (PRECOMPUTED_HEADER_NUM-1);

				CELLHEAD_DATA(InBufDescPtr) = &(pFrameALVC->ATMVCBlock.PreCompHeaders[TempOOD]);
				CELL_LTH_DATA(InBufDescPtr) = CELL_HEADER_SIZE;
				//VC_HNDL_DATA(InBufDescPtr) = pVCHdl;
				AddToCtrLONG(&pFrameALVC->Stats.TXBytes, InBufDescPtr->len + CELL_LTH_DATA(InBufDescPtr));
				AddToCtrLONGLONG(&pFrameALVC->Stats.COTXBytes, InBufDescPtr->len + CELL_LTH_DATA(InBufDescPtr));

				// update VC stats
				TempCellHeaderPtr = CELLHEAD_DATA(InBufDescPtr);

				if (TempCellHeaderPtr->CLP == 0)
				{
					AddToCtrLONG(&pFrameALVC->Stats.TXATMCellsCLP0, 1);
				}
				else
				{
					AddToCtrLONG(&pFrameALVC->Stats.TXATMCellsCLP1, 1);
				}

				AddToCtrLONGLONG(&pFrameALVC->Stats.COATMTXCells, 1);
			}
		}
	}

	return;
}

/*******************************************************************************
FUNCTION NAME:
	FrameALATMGetTXCell

ABSTRACT:	
	Traffic Shaper calls this function to a Cell from a VC.    

RETURN:
	None 

DETAILS:
	This function gets a buffer containing one ATM cell from SAR.
	
*******************************************************************************/
GLOBAL void FrameALATMGetTXCell
(
    IN	CDSL_VC_T	*VCHdl,				// VC Handle to data stream
    IN	SK_BUFF_T	*BufferDescPtr	// Pointer to a buffer descriptor
)
{
	FRAMEAL_VC_T		*	pFrameALVC;

	pFrameALVC =  (FRAMEAL_VC_T *) VCHdl->FrameALVCMod;

	if ((pFrameALVC != NULL) && (pFrameALVC->ATMVCBlock.TxGetFullBuf != NULL))
	{
		pFrameALVC->ATMVCBlock.InVCALCNXT[FRAMEALVC_CNXT_GETTX] = TRUE;

		// 	 TxGetFullBuf returns a pointer but it is the same as InBufDescPtr
		(*pFrameALVC->ATMVCBlock.TxGetFullBuf) (VCHdl, BufferDescPtr );

		if (BufferDescPtr->len > 0)
		{
			VC_HNDL_DATA(BufferDescPtr) = VCHdl;
		}

		pFrameALVC->ATMVCBlock.InVCALCNXT[FRAMEALVC_CNXT_GETTX] = FALSE;
	}
}


/*******************************************************************************
FUNCTION NAME:
	FrameALATMStartTx

ABSTRACT:	
	Calls ChipAL to start sending Cells on a Link.   

RETURN:
	NDIS status

DETAILS:
	
*******************************************************************************/

GLOBAL NDIS_STATUS FrameALATMStartTx
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
)
{
	FRAMEAL_LINK_T  	*	pFrameALLink;
	// make sure chipAL passed FrameAL a good link handle
	if ( LinkHdl != NULL)
	{
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;

		if (pFrameALLink != NULL)
		{
			// if link state is idle that start the ChipAL
			if (pFrameALLink->ATMLinkBlock.LinkState == ATM_LINK_STATE_IDLE)
			{
				// make sure chipallink init has occurred first by checking for context
				if ( LinkHdl->pThisAdapter->pChipALRemoteInf->LinkInterface.iface.Context != NULL )
				{
					ChipALStartTx( LinkHdl);
				}
			}
		}
		else
		{
			// return failed status
			return STATUS_FAILURE;
		}
	}
	else
	{
		// return failed status
		return STATUS_FAILURE;
	}

	return STATUS_SUCCESS;
}

/*******************************************************************************
FUNCTION NAME:
	FrameALATMTimerHandler

ABSTRACT:	
	Calls when Hobbs timer expires.    

RETURN:
	None

DETAILS:
	
*******************************************************************************/
GLOBAL void FrameALATMTimerHandler
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
)
{
	FRAMEAL_SHAPER_STATUS	TempTrafficShapingFlag;
	FRAMEAL_SHAPER_STATUS	OldTrafficShapingFlag;

	FRAMEAL_LINK_T * pFrameALLink;
	BOOLEAN                 NeedStartTx = FALSE;

#if FRAMEALATM_TRAFFIC_SHAPING	

	if (( LinkHdl != NULL) && (LinkHdl->FrameMod != NULL))
	{
		// Update traffic shaper status
		pFrameALLink = (FRAMEAL_LINK_T*) LinkHdl->FrameMod;

		// turn off traffic shaping while updating the ATM layer
		OldTrafficShapingFlag = pFrameALLink->ATMLinkBlock.TrafficShapingActive;
		pFrameALLink->ATMLinkBlock.TrafficShapingActive = TRAFFIC_SHAPING_OFF;

		// Add this VC to the traffic shaper
		TempTrafficShapingFlag = FrATMShTimerHandler( LinkHdl, &NeedStartTx);
		if (TempTrafficShapingFlag !=  TRAFFIC_SHAPING_NOCHANGE)
		{

			pFrameALLink->ATMLinkBlock.TrafficShapingActive = TempTrafficShapingFlag;
		}
		else
		{
			pFrameALLink->ATMLinkBlock.TrafficShapingActive = OldTrafficShapingFlag;
		}
	}
#endif
	FrameALLinkStartTX (LinkHdl);
}

/*******************************************************************************
FUNCTION NAME:
	FrameALATMLinkUp

ABSTRACT:
	This function is called when the line enters showtime

RETURNS:
	None

DETAILS:
*******************************************************************************/

GLOBAL void FrameALATMLinkUp
(
    IN	CDSL_LINK_T		*LinkHdl		// Link Handle to data stream
)
{
	FrameALATMOAMLinkUp(LinkHdl);
}

/*******************************************************************************
FUNCTION NAME:
	FrameALLinkDown

ABSTRACT:
	This function is called when the line leaves showtime

RETURNS:
	None

DETAILS:
*******************************************************************************/

GLOBAL void FrameALATMLinkDown
(
    IN	CDSL_LINK_T		*LinkHdl		// Link Handle to data stream
)
{
	return;
}
