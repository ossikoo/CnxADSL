/******************************************************************************
*******************************************************************************
****	Copyright (c)  1999
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
**		FrameAL	ATM OAM layer.
**
**	FILE NAME:
**		FrameALATMOAM.c
**
**	ABSTRACT:
**		This files contains functionss for FrameAL ATM OAM Layer. This includes
**		F4 and F5 Loop Back processint.  		  
**
**	DETAILS:
**		FrameALATM.c directs all OAM cells to this module, this module
**      generates reply cells and counts statistics. 
**
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/FrameALATMOAM.c $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/

#include "CardMgmt.h"
#include "FrameAL.h"
#include "FrameALOAM.h"
#include "FrameALHec.h"
#include "CellDataTestMgmt.h"
												
#define PAYLOAD_SIZE 48
#define POLYNOMIAL 0x633
#define INIT_VC_CELLS 1

static DWORD VCWithInitCellsToSend = 0; 

//*****************************************************************************
//   LOCAL functions prototypes.
//*****************************************************************************

LOCAL NDIS_STATUS FrameALATMOAMHandleLoopBack
(
	IN 	FRAMEAL_LINK_T * pFrameALLink,		// Link Handle reguired 
	IN	FRAMEAL_VC_T 	*pFrameALVC,			// VC Handle to data stream (optional)
	IN	FRAMEAL_ATM_OAM_CELL_T	*LoopbackCell	// Pointer to a buffer descriptor for FrameAL to use
);


LOCAL NDIS_STATUS FrameALATMOAMGenerateInitLoopBack
(
	IN 	FRAMEAL_LINK_T * 		pFrameALLink,		// Link Handle reguired 
	IN	DWORD 					CorrelationTag,			// VC Handle to data stream (optional)
	IN	OAM_MESSAGE_T	*       LoopbackCell	// Pointer to a buffer descriptor for FrameAL to use
);


void FrameALOAMGenCRC10Table
(
	IN FRAMEAL_ATM_OAM_LINK_T * OAMLinkHdl
)
/* generate the table of CRC-10 remainders for all possible bytes */
{
	
    DWORD RowIndex, BitIndex;
    WORD Crc10Accum;

    for ( RowIndex = 0;  RowIndex < 256;  RowIndex++ )
    {
        Crc10Accum = ((unsigned short) RowIndex << 2);
        for ( BitIndex = 0;  BitIndex < 8;  BitIndex++ )
        {
            if ((Crc10Accum <<= 1) & 0x400) Crc10Accum ^= POLYNOMIAL;
        }
        OAMLinkHdl->Crc10Table[RowIndex] = Crc10Accum;
    }
    return;
}

unsigned short FrameALOAMUpdateCrc10
(
	WORD crc10_accum, 
	BYTE *data_blk_ptr, 
	DWORD data_blk_size,
	FRAMEAL_ATM_OAM_LINK_T * OAMLinkHdl
)
 /* update the data block's CRC-10 remainder one byte at a time */
{
    DWORD Index;
    for ( Index = 0;  Index < data_blk_size;  Index++ )
    {
        crc10_accum = ((crc10_accum << 8) & 0x3ff)
                       ^ OAMLinkHdl->Crc10Table[( crc10_accum >> 2) & 0xff]
                                                      ^ *data_blk_ptr++;
    }
    return crc10_accum;
}

/*******************************************************************************
FUNCTION NAME:
	FrameALATMOAMLinkInit

ABSTRACT:	
	Initializes a new link instance for OAM.  

RETURN:
	NDIS status

DETAILS:
	This function initialize FrameALATM OAM layer 
	
*******************************************************************************/
GLOBAL NDIS_STATUS FrameALATMOAMLinkInit 
(
	IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
)
{

	FRAMEAL_LINK_T * pFrameALLink;

	FRAMEAL_ATM_OAM_LINK_T * pFrameALATMOAMLink;

	if ( (LinkHdl != NULL) && (LinkHdl->FrameMod != NULL))
	{
	 
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;

		
   		pFrameALATMOAMLink  =  &pFrameALLink->ATMLinkBlock.OAMLink;


		pFrameALATMOAMLink->CorrelationTag	= 1;

		FrameALOAMGenCRC10Table( pFrameALATMOAMLink);
	
		// mark link no cell to send
		pFrameALATMOAMLink->CellToSend = FALSE;
	 
		pFrameALATMOAMLink->LinkUpCellToSend  = FALSE;

		return STATUS_SUCCESS;
	}
	else
	{
		return STATUS_FAILURE;
	}

}

/*******************************************************************************
FUNCTION NAME:
	FrameALATMOAMVCInit

ABSTRACT:	
	Initializes a new VC instance.  

RETURN:
	NDIS status

DETAILS:
	This function initialize VC instance. 
*******************************************************************************/
GLOBAL NDIS_STATUS FrameALATMOAMVCInit 
(
	IN	CDSL_VC_T	*VCHdl			// Link Handle to data stream
)
{
	FRAMEAL_VC_T  		*pFrameALVC;


	FRAMEAL_ATM_OAM_VC_T 	*pFrameALATMOAMVC;

	if (VCHdl != NULL)
	{
		pFrameALVC 		= (FRAMEAL_VC_T*)VCHdl->FrameALVCMod;

		pFrameALATMOAMVC = &pFrameALVC->ATMVCBlock.OAMVc;

		pFrameALATMOAMVC->CellToSend = FALSE;


		pFrameALATMOAMVC->InitCellsToSend = INIT_VC_CELLS;

		pFrameALATMOAMVC->InitVCOAMCell.Data.FunctNType  = 
			(OAM_TYPE_FAULT_MANAGEMENT <<4) | OAM_FUNCITON_FAULT_LOOPBACK ;

		pFrameALATMOAMVC->InitVCOAMCell.Data.LoopBackInd = 1;

		VCWithInitCellsToSend++;


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
GLOBAL NDIS_STATUS FrameALATMOAMLinkShutdown 
(
	IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
)
{
	FRAMEAL_LINK_T * pFrameALLink;

	FRAMEAL_ATM_OAM_LINK_T * pFrameALATMOAMLink;


	if ( (LinkHdl != NULL) && (LinkHdl->FrameMod != NULL))
	{
	 
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;

		
   		pFrameALATMOAMLink  =  &pFrameALLink->ATMLinkBlock.OAMLink;


	
		// mark link no cell to send
		pFrameALATMOAMLink->CellToSend = FALSE;
	 

		return STATUS_SUCCESS;
	}
	else
	{
		return STATUS_FAILURE;
	}

 
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
GLOBAL NDIS_STATUS FrameALATMOAMVCShutdown  
(				   
	IN	CDSL_VC_T	*VCHdl			// Link Handle to data stream
)
{
	FRAMEAL_VC_T  		*pFrameALVC;


	FRAMEAL_ATM_OAM_VC_T 	*pFrameALATMOAMVC;

	if ((VCHdl != NULL) 	&& (VCHdl->FrameALVCMod != NULL))
	{
		pFrameALVC 		= (FRAMEAL_VC_T*)VCHdl->FrameALVCMod;

		pFrameALATMOAMVC = &pFrameALVC->ATMVCBlock.OAMVc;

		pFrameALATMOAMVC->CellToSend = FALSE;

		return STATUS_SUCCESS;

	}
	else
	{
		return STATUS_FAILURE;
	} 



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

GLOBAL SK_BUFF_T	*FrameALOAMLinkGetTxFullBuffer
(
	IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
	IN 	SK_BUFF_T * pBuffDesc
)
{

	FRAMEAL_LINK_T * pFrameALLink;

	FRAMEAL_ATM_OAM_LINK_T * pFrameALATMOAMLink;
	NDIS_STATUS Status;


	if ( (LinkHdl != NULL) && (LinkHdl->FrameMod != NULL))
	{
	 
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;
   		pFrameALATMOAMLink  =  &pFrameALLink->ATMLinkBlock.OAMLink;

		if (pFrameALATMOAMLink->CellToSend == TRUE)
		{
			CELLHEAD_DATA(pBuffDesc) =  (ATM_HEADER_T*)&(pFrameALATMOAMLink->OAMCell.LoopBackHeader);
			CELL_LTH_DATA(pBuffDesc) = CELL_HEADER_SIZE;
			pBuffDesc->data =  (BYTE*)&pFrameALATMOAMLink->OAMCell.Data;

			pBuffDesc->len = CELL_PAYLOAD_SIZE;
			VC_HNDL_DATA(pBuffDesc) = pFrameALATMOAMLink->OAMCell.VCHdl;

			// mark link no cell to send
			pFrameALATMOAMLink->CellToSend = FALSE;
	 		pFrameALATMOAMLink->OAMCell.DataCount = 0;
	
			return pBuffDesc; 
		}
		else if ( VCWithInitCellsToSend	!= 0)
		{
			DWORD VCIndex;
			CDSL_VC_T			*	pVCHdl;
			FRAMEAL_VC_T		*   pFrameALVC;



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
		
		
   						pFrameALATMOAMLink  =  &pFrameALLink->ATMLinkBlock.OAMLink;


 	
						Status =  STATUS_FAILURE;

						if (pFrameALVC->ATMVCBlock.OAMVc.InitCellsToSend != 0)
						{
							pFrameALVC->ATMVCBlock.OAMVc.InitCellsToSend--; 
							if (pFrameALVC->ATMVCBlock.OAMVc.InitCellsToSend == 0)
							{
								VCWithInitCellsToSend--;
							}

							Status = FrameALATMOAMGenerateInitLoopBack( pFrameALLink,
																		pFrameALVC->ATMVCBlock.OAMVc.InitCellsToSend,
																		&pFrameALVC->ATMVCBlock.OAMVc.InitVCOAMCell.Data);
						}
				

						if (Status == STATUS_SUCCESS)
						{
							CELLHEAD_DATA(pBuffDesc) =  (ATM_HEADER_T*)&pFrameALVC->ATMVCBlock.PreCompHeaders[ATM_HEADER_F5END_TO_END];
							CELL_LTH_DATA(pBuffDesc) = CELL_HEADER_SIZE;
							pBuffDesc->data =  (BYTE*)&pFrameALVC->ATMVCBlock.OAMVc.InitVCOAMCell.Data;

							pBuffDesc->len = CELL_PAYLOAD_SIZE;
			
							VC_HNDL_DATA(pBuffDesc) = pVCHdl;
							// mark link no cell to send
							return pBuffDesc; 
					   	}
					}
				}
			}
		
			// all init OAM have been sent
	   		VCWithInitCellsToSend = 0;
	   		return NULL;
	   		
		}
		else if (pFrameALATMOAMLink->LinkUpCellToSend)
		{
		
			CELLHEAD_DATA(pBuffDesc) =  (ATM_HEADER_T*)&pFrameALLink->ATMLinkBlock.LinkOAMHeader;
			CELL_LTH_DATA(pBuffDesc) = CELL_HEADER_SIZE;
			pBuffDesc->data =  pFrameALLink->ATMLinkBlock.LinkOAMPayload;

			pBuffDesc->len = CELL_PAYLOAD_SIZE;
			
			VC_HNDL_DATA(pBuffDesc) = NULL; 

			// mark link no cell to send
			pFrameALATMOAMLink->LinkUpCellToSend = FALSE;

			return pBuffDesc; 

			
		}

		else
		{
			return NULL;
		}
		
	}

	return 0;
}


/*******************************************************************************
FUNCTION NAME:
	FrameALOAMVCRxBuffer

ABSTRACT:	
	Receives OAM cells from FRAMEALATM.  

RETURN:
	NDIS Status

DETAILS:
	The memory for the FrameAL ATM layer link context is released.
*******************************************************************************/

GLOBAL VOID FrameALOAMRxBuffer
(
	IN 	CDSL_LINK_T		*LinkHdl,		// Link Handle reguired 
	IN	CDSL_VC_T		*VCHdl,			// VC Handle to data stream (optional)
	IN	SK_BUFF_T	*InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
)
{
		
	FRAMEAL_LINK_T * pFrameALLink;

	FRAMEAL_ATM_OAM_LINK_T * pFrameALATMOAMLink;

	FRAMEAL_VC_T  		*pFrameALVC=NULL;

	NDIS_STATUS Status; 

	// find VC pointer is this happens to F5 cell
  	if ((VCHdl != NULL)	&& (VCHdl->FrameALVCMod != NULL))
	{
		// used later in call to OAM loopback
		pFrameALVC 		= (FRAMEAL_VC_T*)VCHdl->FrameALVCMod;

	}


	// link pointer and FrameAL link pointer
	if ( (LinkHdl != NULL) && (LinkHdl->FrameMod != NULL))
	{
	 
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;

		
   		pFrameALATMOAMLink  =  &pFrameALLink->ATMLinkBlock.OAMLink;

		// check for a cell still in the TX slot
		// The cell should be gone by now
		if (pFrameALATMOAMLink->CellToSend == FALSE)
		{
			


			// if this is first piece of the cell 
			// set up the pointers for a new cell
			// this is because payloads can come in pieces.
			if (pFrameALATMOAMLink->OAMCell.DataCount == 0)
			{
			
				pFrameALATMOAMLink->OAMCell.BufferPointer = 
					(BYTE*)&pFrameALATMOAMLink->OAMCell.Data;

				// ATM Cell header has to complete or 
				// Demux can not deliver the pieces of payload

				COPY_MEMORY
				(
					(PVOID)pFrameALATMOAMLink->OAMCell.LoopBackHeader,	// Destination
					(PVOID)CELLHEAD_DATA(InBufDescPtr),					// Source
					(CELL_LTH_DATA(InBufDescPtr) & 0xffff)				// Length
				);
	
			
			}
				
			// copy the payload 
			// do not run of the end of buffer.
			if ((pFrameALATMOAMLink->OAMCell.DataCount 
				+ InBufDescPtr->len) <= CELL_PAYLOAD_SIZE)
			{
				
				COPY_MEMORY
				(
					pFrameALATMOAMLink->OAMCell.BufferPointer,	// Destination
					InBufDescPtr->data,				// Source
					InBufDescPtr->len					// Length
				);
			 		


				// update pointers and counters
			   pFrameALATMOAMLink->OAMCell.BufferPointer += 
					InBufDescPtr->len;
		 
			   pFrameALATMOAMLink->OAMCell.DataCount +=
					InBufDescPtr->len;
			 }


			// if this piece of a payload is the last 
			// piece, process the payload
			if ((DWORD) OOB_DATA(InBufDescPtr) & OOD_END_CELL)
			{
				// if have a complete cell process
				// check for OAM loopback cell
				WORD LocalCRC10;

				// save VCHdl for use by tx

				pFrameALATMOAMLink->OAMCell.VCHdl = VCHdl;

			  	// check for valid cell (crc10)
			   	LocalCRC10 = FrameALOAMUpdateCrc10
								(
									0, 
									(BYTE *)&pFrameALATMOAMLink->OAMCell.Data,
									pFrameALATMOAMLink->OAMCell.DataCount,
									pFrameALATMOAMLink
								);

				// if good CRC10 parse the payload 
				if ( LocalCRC10 == 0)
				{

					
					//switch (pFrameALATMOAMLink->OAMCell.Data.OAMType & 0xf)
					switch ((pFrameALATMOAMLink->OAMCell.Data.FunctNType >>4) &0xf)
					{
						case OAM_TYPE_FAULT_MANAGEMENT:
						
							switch (pFrameALATMOAMLink->OAMCell.Data.FunctNType&0xf)
							//switch (pFrameALATMOAMLink->OAMCell.Data.FunctionType & 0xf)
							{
								
								case	 OAM_FUNCITON_FAULT_LOOPBACK:
									Status = FrameALATMOAMHandleLoopBack
												(
													pFrameALLink,
													pFrameALVC,
									 				&pFrameALATMOAMLink->OAMCell	
												);
									if (Status == STATUS_SUCCESS)
									{
										// good cell send it 
										pFrameALATMOAMLink->CellToSend = TRUE;
										FrameALATMStartTx(LinkHdl);

									}
									else
									{
								
										// bum cell discard it
									   	pFrameALATMOAMLink->OAMCell.DataCount = 0;
										AddToCtrLONG(&pFrameALATMOAMLink->OAMUnhandledCells, 1);
							
									}

									break;
								case 	OAM_FUNCTION_FAULT_AIS:
								case 	OAM_FUNCTION_FAULT_RDI:
								case	OAM_FUNCTION_FAULT_CONTINUITY_CHECK:
								default:
										// bum cell discard it
									   	pFrameALATMOAMLink->OAMCell.DataCount = 0;
										break;						
						 
								}
							break;
						case OAM_TYPE_PERFORM_MANAGEMENT:			
						case OAM_TYPE_ACT_DEACT_MANAGEMENT:
						case OAM_TYPE_SYSTEM_MANAGEMENT:
						default:
							// bum cell discard it
						   	pFrameALATMOAMLink->OAMCell.DataCount = 0;
							AddToCtrLONG(&pFrameALATMOAMLink->OAMUnhandledCells, 1);
							break;
					}
				}
				else
				{
					// payload did not pass CRC10 
				   	pFrameALATMOAMLink->OAMCell.DataCount = 0;
					AddToCtrLONG(&pFrameALATMOAMLink->OAMBadCRC10Cells, 1);
	
				}


			}
 		}
		else
		{
			// overrun in tx.
			AddToCtrLONG(&pFrameALATMOAMLink->OAMTXOverrun, 1);
				
		}
	}
 }

/*******************************************************************************
FUNCTION NAME:
	FrameALATMOAMHandleLoopBack

ABSTRACT:	
	Process Received loopback cells from FRAMEALATM.  

RETURN:
	NDIS Status

DETAILS:
	
*******************************************************************************/

LOCAL NDIS_STATUS FrameALATMOAMHandleLoopBack
(
	IN 	FRAMEAL_LINK_T 			* pFrameALLink,	// Link Handle reguired 
	IN	FRAMEAL_VC_T 			*pFrameALVC,	// VC Handle to data stream (optional)
	IN	FRAMEAL_ATM_OAM_CELL_T	*LoopbackCell	// Pointer to a buffer descriptor for FrameAL to use
)
{  	

	FRAMEAL_ATM_OAM_LINK_T * pFrameALATMOAMLink;


	WORD TempCrc10; 

   	pFrameALATMOAMLink  =  &pFrameALLink->ATMLinkBlock.OAMLink;

	// for the cell is not the response to a loopback
	// that the cell was loopbacked to use
	if ((LoopbackCell->Data.LoopBackInd & 1) ==  0)
	{
		// count statistics for this cell 
		AddToCtrLONG(&pFrameALLink->Stats.RXATMCellsMgmCell, 1);
		if (pFrameALVC != NULL)
		{
			AddToCtrLONG(&pFrameALVC->Stats.RXATMCellsMgmCell, 1);
		}

		// clear loopback indicator (meaning cell has been looped back)

		LoopbackCell->Data.LoopBackInd = 0;	
		// clear old crc first, otherwise the new crc will be zero
		TempCrc10 = FrameALOAMUpdateCrc10
		(
			0, 
			(BYTE *)&LoopbackCell->Data,
			LoopbackCell->DataCount,
			pFrameALATMOAMLink
		);


 		LoopbackCell->Data.OAMCRC10[0] ^= (TempCrc10 >> 8);
		LoopbackCell->Data.OAMCRC10[1] ^= TempCrc10 &0xff;
		return STATUS_SUCCESS;

	}
	else
	{
		AddToCtrLONG(&pFrameALATMOAMLink->OAMInvalidLoopback, 1);
			
		return STATUS_FAILURE;
	}
}


/*******************************************************************************
FUNCTION NAME:
	FrameALATMOAMGenerateInitLoopBack

ABSTRACT:	
	Generate the first OAM cells for a VC that is just opening 

RETURN:
	NDIS Status

DETAILS:
	
*******************************************************************************/



LOCAL NDIS_STATUS FrameALATMOAMGenerateInitLoopBack
(
	IN 	FRAMEAL_LINK_T * 		pFrameALLink,		// Link Handle reguired 
	IN	DWORD 					CorrelationTag,			// VC Handle to data stream (optional)
	IN	OAM_MESSAGE_T	*       LoopbackCell	// Pointer to a buffer descriptor for FrameAL to use
)
{
	WORD TempCrc10; 
 	FRAMEAL_ATM_OAM_LINK_T * pFrameALATMOAMLink;


   	pFrameALATMOAMLink  =  &pFrameALLink->ATMLinkBlock.OAMLink;



	LoopbackCell->LoopBackInd = 0;	

	LoopbackCell->CorrelationTag[0] = CorrelationTag;

	// clear old crc first, otherwise the new crc will be zero
	TempCrc10 = FrameALOAMUpdateCrc10
	(
		0, 
		(BYTE *)&LoopbackCell,
		CELL_PAYLOAD_SIZE,
		pFrameALATMOAMLink
	);


 	LoopbackCell->OAMCRC10[0] ^= (TempCrc10 >> 8);
	LoopbackCell->OAMCRC10[1] ^= TempCrc10 &0xff;
	return STATUS_SUCCESS;

}

	
/*******************************************************************************
FUNCTION NAME:
	FrameALATMOAMLinkUp

ABSTRACT:
	This function is called when the line enters showtime
	It queues one cell for transmission
RETURNS:
	None

DETAILS:
*******************************************************************************/

GLOBAL void FrameALATMOAMLinkUp
(
	IN	CDSL_LINK_T		*LinkHdl		// Link Handle to data stream
)
{
	FRAMEAL_LINK_T * pFrameALLink;
	FRAMEAL_ATM_OAM_LINK_T * pFrameALATMOAMLink;

	if ( (LinkHdl != NULL) && (LinkHdl->FrameMod != NULL))
	{
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;
   		pFrameALATMOAMLink  =  &pFrameALLink->ATMLinkBlock.OAMLink;
		FrameALATMOAMGenerateInitLoopBack( pFrameALLink,
										   	pFrameALATMOAMLink->CorrelationTag++,
										   	(OAM_MESSAGE_T*)&pFrameALLink->ATMLinkBlock.LinkOAMPayload);
			
		
		pFrameALATMOAMLink->LinkUpCellToSend = TRUE;

	}
	

}
