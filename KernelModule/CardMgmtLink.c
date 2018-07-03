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
**		CardMgmt (Card Management)
**
**	FILE NAME:
**		CardMgmtLink.c
**
**	ABSTRACT:
**		WAN Miniport link management routines.
**		All information about the state of the link is stored in the
**		Link structure.
**
**	DETAILS:
**		Functions are arranged in top-down order.
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CardMgmtLink.c $
** $Revision: 1 $
** $Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/
#define		MODULE_ID			DBG_ENABLE_CARDMGMT


#include "CardMgmt.h"
#include "CardMgmtV.h"

#define		__FILEID__		4


///////////////////////////////////////////////////////////////////////////////
//	Defines
///////////////////////////////////////////////////////////////////////////////

#ifndef MEMORY_TAG
	#define MEMORY_TAG			'LgMC'
#endif


///////////////////////////////////////////////////////////////////////////////
//	Functions
///////////////////////////////////////////////////////////////////////////////


/******************************************************************************
*	FUNCTION NAME:
*		CMLInitAllLinks
*
*	ABSTRACT:
*		This routine initializes the driver level Link structures within
*		the Adapter structure.  We use the link structure to hold context
*		pointers for each layer in the driver. The layers use the link
*		structure to idenity a link and find the context for that link. 
******************************************************************************/
NDIS_STATUS CMLInitAllLinks(	IN CDSL_ADAPTER_T		* pThisAdapter )
{
	//	A pointer to our link information structure for the selected line device.
	CDSL_LINK_T					* pThisLink;
	CDSL_LINK_CARD_MGMT_T		* pThisCMContext;
	NDIS_STATUS					Status;
	NDIS_STATUS					AccumStatus;
	//	Index into the link array.

	AccumStatus = STATUS_SUCCESS;

	pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );

	//	All link structure fields must have been set to 0
	//	when the memory for the structure was allocated

	//	Setup the static features of the link.
	pThisLink->pThisAdapter = pThisAdapter;
	pThisLink->Allocated = FALSE;
	pThisLink->LinkEntryState = UNASSIGNED_LINK_ENTRY;

	// set all contexts to NULL
	pThisLink->CardMgmt = NULL;

	Status = ALLOCATE_MEMORY( (PVOID)&pThisCMContext, sizeof( CDSL_LINK_CARD_MGMT_T ), MEMORY_TAG );

	if ( Status != STATUS_SUCCESS )
	{
		//x			DBG_CDSL_BREAKPOINT( DBG_LVL_ERROR, pThisAdapter->DebugFlag );
		//
		// !!   do something   !!
		//
		AccumStatus = Status;
		pThisLink->CardMgmt = NULL;
	}
	else
	{
		// 	Save card management context address
		pThisLink->CardMgmt = pThisCMContext;
		CLEAR_MEMORY( (PVOID)((pThisCMContext)), sizeof( CDSL_LINK_CARD_MGMT_T ) );

		CLEAR_MEMORY( (PVOID)(&(pThisCMContext->Stats)), sizeof(LINK_STATISTICS_T) );

		pThisCMContext->State = LINK_STATE_IDLE;
	}

	return AccumStatus;
}


/******************************************************************************
*	FUNCTION NAME:
*		 CMLAllocateLink
*
*	ABSTRACT:
*		This routine allocates a specific Link structure and passes back a
*		pointer which can be used by the driver to access the link. 
*
*	RETURN:
*		A pointer to allocated link information structure, 
*		NULL if not allocated.
*
*	ARGUMENTS:
*		pThisAdapter _ A pointer ot our adapter information structure.
*
******************************************************************************/

CDSL_LINK_T * CMLAllocateLink(
    IN CDSL_ADAPTER_T		* pThisAdapter
)
{
	//	A pointer to our link information structure for the selected line device.
	CDSL_LINK_T					* pThisLink;
	CDSL_LINK_CARD_MGMT_T		* pThisCMContext;

	pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );
	pThisCMContext	= (CDSL_LINK_CARD_MGMT_T *)pThisLink->CardMgmt;

	if ( pThisLink->Allocated == FALSE )
	{
		//	Reset all the state information for this link.
		pThisLink->Allocated					= TRUE;
		pThisCMContext->LinkSpeed				= 1000000;
	}

	return (pThisLink);
}


/******************************************************************************
*	FUNCTION NAME:
*		CMLReleaseLink
*
*	ABSTRACT:
*		This routine releases a specific Link structure and makes it available
*		for future allocation.  It is assumed that the caller has closed any
*		associated connection and notified TAPI and WAN with a LINE_DOWN.
*
*
*	RETURN:
*		None.
*
*	ARGUMENTS:
*		Link _ A pointer to the link information structure to be released.
*
******************************************************************************/

VOID CMLReleaseLink(	IN CDSL_LINK_T		* pThisLink )
{
	CDSL_LINK_CARD_MGMT_T		* pThisCMContext;

	pThisCMContext	= (CDSL_LINK_CARD_MGMT_T *)pThisLink->CardMgmt;

	pThisLink->Allocated		= FALSE;
}


/******************************************************************************
*	FUNCTION NAME:
*		CMLCallAllModuleLinkInits
*
*	ABSTRACT:
*		This routine calls each (all) module's 'LinkInit' function so that each
*		module having a Link context may initialize anything necessary
*		associated with it's Link context.
*
*	RETURN:
*		status of initialization and setup
*
*	DETAILS:
*		NOTE!!
*			The other modules' Link Init functions *MUST* be called in a
*			specific order!!!
*				BufMgmtLinkInit
*				FrameALLinkInit
*				ChipALLinkInit
*			The other modules' Link Shut Down functions *MUST* be called in a 
*			specific order!!!
*				ChipALLinkHalt
*				FrameALLinkShutdown
*				BufMgmtLinkShutdown
******************************************************************************/
NDIS_STATUS CMLCallAllModuleLinkInits(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN CDSL_LINK_T			* pThisLink )
{
	NDIS_STATUS					Status=STATUS_SUCCESS;
	NDIS_STATUS					Status2;
	CDSL_LINK_CARD_MGMT_T		* pThisCMContext;
	FRAMEAL_LINK_PARAMETERS		DefaultLinkParameters;

	//	We can't allow indications to NULL...
	if ( pThisLink->CardMgmt != NULL )
	{
		pThisCMContext = (CDSL_LINK_CARD_MGMT_T *)pThisLink->CardMgmt;
		pThisCMContext->State = LINK_STATE_CONNECTED;

		//*********************************************************************
		//****	SEE NOTE!!! in DETAILS section of function title block above!
		//*********************************************************************
		DBG_CDSL_DISPLAY(
		    DBG_LVL_MISC_INFO,
		    pThisLink->pThisAdapter->DebugFlag,
		    ("CMLIndicateLinkLineUp(): Calling BufMgmtLinkInit") );

		Status = BufMgmtLinkInit( pThisLink );
		if ( Status == STATUS_SUCCESS )
		{
			DBG_CDSL_DISPLAY(
			    DBG_LVL_MISC_INFO,
			    pThisLink->pThisAdapter->DebugFlag,
			    ("CMLIndicateLinkLineUp(): Calling FrameALLinkInit") );
			DefaultLinkParameters.TxCellsPerSecond = 1000;
			DefaultLinkParameters.RxCellsPerSecond = 1000;
			DefaultLinkParameters.LinkType = LINK_TYPE_UBR_ONLY;
			Status = FrameALLinkInit( pThisLink, &DefaultLinkParameters );
			if ( Status == STATUS_SUCCESS )
			{
				DBG_CDSL_DISPLAY(
				    DBG_LVL_MISC_INFO,
				    pThisLink->pThisAdapter->DebugFlag,
				    ("CMLIndicateLinkLineUp(): Calling ChipALLinkInit") );
				Status = ChipALLinkInit(
				             pThisLink->pThisAdapter,
				             pThisLink,
				             0,				/*Speed*/
				             ADSL_CHANNEL,	/*Channel*/
				             0 );			/*Mode = Non-Framed*/
				if ( Status != STATUS_SUCCESS )
				{
					//x					DBG_CDSL_BREAKPOINT( DBG_LVL_ERROR, pThisLink->pThisAdapter->DebugFlag );
					//	Log error message and exit.
					NdisWriteErrorLogEntry(
					    pThisAdapter->MiniportAdapterHandle,
					    NDIS_ERROR_CODE_DRIVER_FAILURE,
					    4,
					    Status,
					    __FILEID__,
					    __LINE__,
					    RSS_INIT_ERROR_CHIPAL_LINK_INIT_FAILURE );
					DBG_CDSL_DISPLAY(
					    DBG_LVL_GENERIC,
					    DBG_FLAG_DONT_CARE,
					    ("CMLCallAllModuleLinkInits: ERROR;  ChipALLinkInit Failure!\n") );
					DBG_CDSL_DISPLAY(
					    DBG_LVL_MISC_INFO,
					    pThisLink->pThisAdapter->DebugFlag,
					    ("CMLIndicateLinkLineUp(): Calling ChipALLinkShutdown") );
					Status2 = ChipALLinkShutdown( pThisLink );
					DBG_CDSL_DISPLAY(
					    DBG_LVL_MISC_INFO,
					    pThisLink->pThisAdapter->DebugFlag,
					    ("CMLIndicateLinkLineUp(): Calling FrameALLinkShutdown") );
					Status2 = FrameALLinkShutdown( pThisLink );
					DBG_CDSL_DISPLAY(
					    DBG_LVL_MISC_INFO,
					    pThisLink->pThisAdapter->DebugFlag,
					    ("CMLIndicateLinkLineUp(): Calling BufMgmtLinkShutdown") );
					Status2 = BufMgmtLinkShutdown( pThisLink );
				}
			}
			else
			{
				//x				DBG_CDSL_BREAKPOINT( DBG_LVL_ERROR, pThisLink->pThisAdapter->DebugFlag );
				//	Log error message and exit.
				NdisWriteErrorLogEntry(
				    pThisAdapter->MiniportAdapterHandle,
				    NDIS_ERROR_CODE_DRIVER_FAILURE,
				    4,
				    Status,
				    __FILEID__,
				    __LINE__,
				    RSS_INIT_ERROR_FRAMEAL_LINK_INIT_FAILURE );
				DBG_CDSL_DISPLAY(
				    DBG_LVL_GENERIC,
				    DBG_FLAG_DONT_CARE,
				    ("CMLCallAllModuleLinkInits: ERROR;  FrameALLinkInit Failure!\n") );
				DBG_CDSL_DISPLAY(
				    DBG_LVL_MISC_INFO,
				    pThisLink->pThisAdapter->DebugFlag,
				    ("CMLIndicateLinkLineUp(): Calling FrameALLinkShutdown") );
				Status2 = FrameALLinkShutdown( pThisLink );
				DBG_CDSL_DISPLAY(
				    DBG_LVL_MISC_INFO,
				    pThisLink->pThisAdapter->DebugFlag,
				    ("CMLIndicateLinkLineUp(): Calling BufMgmtLinkShutdown") );
				Status2 = BufMgmtLinkShutdown( pThisLink );
			}
		}
		else
		{
			//x			DBG_CDSL_BREAKPOINT( DBG_LVL_ERROR, pThisLink->pThisAdapter->DebugFlag );
			//	Log error message and exit.
			NdisWriteErrorLogEntry(
			    pThisAdapter->MiniportAdapterHandle,
			    NDIS_ERROR_CODE_DRIVER_FAILURE,
			    4,
			    Status,
			    __FILEID__,
			    __LINE__,
			    RSS_INIT_ERROR_BUFMGMT_LINK_INIT_FAILURE );
			DBG_CDSL_DISPLAY(
			    DBG_LVL_GENERIC,
			    DBG_FLAG_DONT_CARE,
			    ("CMLCallAllModuleLinkInits: ERROR;  BufMgmtLinkInit Failure!\n") );
			DBG_CDSL_DISPLAY(
			    DBG_LVL_MISC_INFO,
			    pThisLink->pThisAdapter->DebugFlag,
			    ("CMLIndicateLinkLineUp(): Calling BufMgmtLinkShutdown") );
			Status2 = BufMgmtLinkShutdown( pThisLink );
		}
	}
	return Status;
}


/******************************************************************************
*	FUNCTION NAME:
*		CMLCallAllModuleLinkShutdowns
*
*	ABSTRACT:
*		This routine calls each (all) module's 'LinkShutdown' function so that
*		each module having a Link context may shutdown anything necessary
*		associated with it's Link context.
*
*	RETURN:
*		None.
*
*	DETAILS:
******************************************************************************/
VOID CMLCallAllModuleLinkShutdowns(	IN CDSL_LINK_T		* pThisLink )
{
	NDIS_STATUS					Status;
	CDSL_LINK_CARD_MGMT_T		* pThisCMContext;

	//	We can't allow indications to NULL...
	if ( pThisLink->CardMgmt != NULL )
	{
		pThisCMContext = (CDSL_LINK_CARD_MGMT_T *)pThisLink->CardMgmt;
		pThisCMContext->State = LINK_STATE_IDLE;

		DBG_CDSL_DISPLAY(
		    DBG_LVL_MISC_INFO,
		    pThisLink->pThisAdapter->DebugFlag,
		    ("CMLIndicateLinkLineDown(): Calling ChipALLinkShutdown") );

		Status = ChipALLinkShutdown( pThisLink );
		DBG_CDSL_DISPLAY(
		    DBG_LVL_MISC_INFO,
		    pThisLink->pThisAdapter->DebugFlag,
		    ("CMLIndicateLinkLineDown(): Calling FrameALLinkShutdown") );

		Status = FrameALLinkShutdown( pThisLink );
		DBG_CDSL_DISPLAY(
		    DBG_LVL_MISC_INFO,
		    pThisLink->pThisAdapter->DebugFlag,
		    ("CMLIndicateLinkLineDown(): Calling BufMgmtLinkShutdown") );

		Status = BufMgmtLinkShutdown( pThisLink );
		DBG_CDSL_DISPLAY(
		    DBG_LVL_MISC_INFO,
		    pThisLink->pThisAdapter->DebugFlag,
		    ("CMLIndicateLinkLineDown(): Link Shutdowns almost complete") );

		FREE_MEMORY( pThisLink->CardMgmt, sizeof( CDSL_LINK_CARD_MGMT_T ), 0 );
		pThisLink->CardMgmt = NULL;
	}
}

