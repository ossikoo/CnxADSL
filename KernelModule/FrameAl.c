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
**		FrameAL
**
**	FILE NAME:
**		FrameAl.c
**
**	ABSTRACT:
**		Framer shim between NDIS miniport driver and HDLCOCT.
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux 2.4 ATM Driver/KernelModule/FrameAl.c $
**	$Revision: 2 $
**	$Date: 2/28/01 11:05a $
*******************************************************************************
******************************************************************************/

#include "CardMgmt.h"
#include "FrameAL.h"
#include "CardMgmtV.h"

#define NO_MGMT	0

#if CAL_BASIC2_INSTALLED	
// this is a Cheetah project

// max line rate at which traffic shaping can occur. Above this line rate Shaping is disabled
	#define CLIENTLIMIT							832000
#else
	#define CLIENTLIMIT							928000
#endif

// check for line rate change at this interval (based on Hobbs general timer)
#define LINE_CHECK_INTERVAL					0x1000

// Timer interval of 10ms
#define TIMER_INTERVAL_UPPER 				1

#define TIMER_INTERVAL_LOWER				100

// default bandwidth limit, used when Registy value is out of limits
#define DEFAULT_BURST_SIZE					500

#define ERROR_FATAL(error)		(LEVEL_FATAL   | MODULE_FRAMEAL | INSTANCE_ATM | error)
#define ERROR_WARNING(error)	(LEVEL_WARNING | MODULE_FRAMEAL | INSTANCE_ATM | error)
#define ERROR_NORMAL(error)		(LEVEL_NORMAL  | MODULE_FRAMEAL | INSTANCE_ATM | error)

#ifndef MEMORY_TAG
	#define	MEMORY_TAG		'LARF' 
#endif
// ***************************************
//		FILE STRUCTURES
// ***************************************

typedef struct
{
	// ******************************************
	// Adapter FrameAL Module Context structure
	// ******************************************

	// All the following are filled with values
	// obtained from reading the registry

	BD_FRAMEAL_ENCAPSULATION_T			RfcEncapsulationMode;

	DWORD								CellBurstSize;

} CDSL_ADAPTER_FRAMEAL_T;


// ***********************************************************************************
// ***********************************************************************************
// 									Data Types
// ***********************************************************************************



// ***********************************************************************************
// ***********************************************************************************
// ***********************************************************************************

/*******************************************************************************
	Private Prototypes
*******************************************************************************/
LOCAL DWORD FrameALUpdateATMStats(
    IN 	CDSL_VC_T * pVCHdl,
    IN 	BD_FRAMEAL_ATM_STATS_T * pATMStats
);


LOCAL DWORD FrameALUpdateAALStats(
    IN 	CDSL_VC_T * pVCHdl,
    IN 	BD_FRAMEAL_AAL_STATS_T * pAALStats
);

#define VC_STATS_OK 	0
#define VC_STATS_FAILED 1

LOCAL void FrameALTimerHandler
(
    IN ULONG 	LinkHdl
);

/*******************************************************************************
FUNCTION NAME:
	FrameALAdapterInit

ABSTRACT:	
	Initializes a new Adapter instance. 
	
RETURN:
	NDIS Status
	 
DETAILS:
	FrameAL does not perform any initialization at adapter init time.
*******************************************************************************/

GLOBAL NDIS_STATUS FrameALAdapterInit
(
    IN	CDSL_ADAPTER_T	*AdprHdl	// supplied Adapter Handle
    // (&ThisAdapter)
)
{
	// Nothing to do here
	// All initialization occurs at link init time.

	return STATUS_SUCCESS;
}

/*******************************************************************************
FUNCTION NAME:
	FrameALAdapterShutdown

ABSTRACT:
		To provide a point from which FrameAL module can deinitialize
		any adapter context structure specific data it may have intialized.

RETURN:
		NDIS status

DETAILS:
		Check to see if we had allocated an adapter context structure and
		free up the memory if we had. 

*******************************************************************************/

GLOBAL NDIS_STATUS FrameALAdapterShutdown
(
    IN	CDSL_ADAPTER_T	*AdprHdl	// supplied Adapter Handle
    // (&ThisAdapter)
)
{
	// Get the FrameAL adapter context
	CDSL_ADAPTER_FRAMEAL_T *FrameALAdprCtx = AdprHdl->FrameMod;

	// If we had previously allocated an adapter context structure free it now.
	if(	FrameALAdprCtx != NULL )
	{
		FREE_MEMORY
		(
		    FrameALAdprCtx,
		    sizeof(CDSL_ADAPTER_FRAMEAL_T),
		    0
		);
		AdprHdl->FrameMod = NULL;
	}

	return STATUS_SUCCESS;
}

/*******************************************************************************
FUNCTION NAME:
	FrameALLinkStartTX

ABSTRACT:
	BM calls this function when new data arrives on a VC.  

RETURN:
	NDIS status

DETAILS:
	FrameAL passes this on to Mux to call ChipAl at the right time.

*******************************************************************************/

GLOBAL NDIS_STATUS FrameALLinkStartTX
(
    IN CDSL_LINK_T 	*LinkHdle
)
{
	if (LinkHdle != NULL)
	{
		return FrameALATMStartTx (LinkHdle);
	}

	return STATUS_FAILURE;
}
/*******************************************************************************
FUNCTION NAME:
	FrameALVCStartTX

ABSTRACT:
	BM calls this function to when new data arrives on a VC.  

RETURN:
	NDIS status

DETAILS:
	FrameAL passes this on to Mux to call ChipAl at the right time.

*******************************************************************************/

GLOBAL NDIS_STATUS FrameALVCStartTX
(
    IN CDSL_VC_T 	*VCHdl
)
{
	if (VCHdl  !=  NULL)
	{
		return FrameALLinkStartTX (VCHdl->LinkHdl);
	}

	return STATUS_FAILURE;
}

/*******************************************************************************
FUNCTION NAME:
	FrameALLinkInit

ABSTRACT:	
	Initializes a new link instance.  

RETURN:
	NDIS status

DETAILS:
	This function allocates memory for the frameAL link context.
*******************************************************************************/

GLOBAL NDIS_STATUS FrameALLinkInit
(
    IN	CDSL_LINK_T	*LinkHdl,						// Link Handle to data stream
    IN  FRAMEAL_LINK_PARAMETERS * LinkParameters	//
)
{
	FRAMEAL_LINK_T  *pFrameALLinkT;

	NDIS_STATUS		Status;

	// Get the pointer to Adapter context
	CDSL_ADAPTER_T *Adapter = LinkHdl->pThisAdapter;

	// Get the Adapter FrameAL context
	CDSL_ADAPTER_FRAMEAL_T *FrameALAdprCtx = Adapter->FrameMod;

	// Allocate NDIS block of memory

	Status = ALLOCATE_MEMORY( (PVOID)&pFrameALLinkT,
	                          sizeof( FRAMEAL_LINK_T ),
	                          MEMORY_TAG );

	// if NDIS does not provide memory
	// This module will not function.
	if (Status != STATUS_SUCCESS)
	{
		return Status;
	}

	// clear FrameAL Link structure
	CLEAR_MEMORY
	(
	    (PVOID *)pFrameALLinkT,
	    sizeof( FRAMEAL_LINK_T )
	);

	// save Frame link structure pointer in link structure
	LinkHdl->FrameMod = pFrameALLinkT;

	// save parameter block in FrameAl link Stucture
	COPY_MEMORY
	(
	    (void *) &pFrameALLinkT->Parameters,
	    (void *) LinkParameters,
	    sizeof(FRAMEAL_LINK_PARAMETERS)
	);

	//
	// Copy into the FrameAL Link Structure any registry settings
	pFrameALLinkT->Parameters.CellBurstSize = FrameALAdprCtx->CellBurstSize;

	// set parameters for Traffic shaper.
	pFrameALLinkT->FrameALShaperInfo.ClientLimit 		=  CLIENTLIMIT;
	pFrameALLinkT->FrameALShaperInfo.LineCheckInterval 	=  LINE_CHECK_INTERVAL;
	pFrameALLinkT->FrameALShaperInfo.DefaultBurstSize   =  DEFAULT_BURST_SIZE;


	pFrameALLinkT->FrameALShaperInfo.TimerIntervalUpper	=  TIMER_INTERVAL_UPPER;
	pFrameALLinkT->FrameALShaperInfo.TimerIntervalLower 	=  TIMER_INTERVAL_LOWER;

	Status =  FrameALATMLinkInit( LinkHdl);

	if (Status != STATUS_SUCCESS)
	{
		return Status;
	}

	INIT_EVENT(&pFrameALLinkT->FrameALTimerStoppedEvent);

	INIT_TIMER(
	    &pFrameALLinkT->FrameALTimer,
	    FrameALTimerHandler,
	    (ULONG)LinkHdl );

	pFrameALLinkT->FrameALTimerState = TMR_TIMING;

	//	start back ground loop
	START_TIMER( &pFrameALLinkT->FrameALTimer,
	             FRAMEAL_TIMER_PERIOD );

	return	STATUS_SUCCESS;
}
/*******************************************************************************
FUNCTION NAME:
	FrameALVCInit

ABSTRACT:	
	Initializes a new VC instance.  

RETURN:
	NDIS status

DETAILS:
	This function allocates memory for the frameAL VC context.
*******************************************************************************/

GLOBAL NDIS_STATUS FrameALVCInit
(
    IN	CDSL_VC_T	*VCHdl,						// Link Handle to data stream
    IN  FRAMEAL_VC_PARAMETERS * VCParameters	//
)
{
	FRAMEAL_VC_T			*pFrameALVC;

	// Get the pointer to Adapter context
	CDSL_ADAPTER_T			*Adapter = VCHdl->LinkHdl->pThisAdapter;

	// Get the Adapter FrameAL context
	CDSL_ADAPTER_FRAMEAL_T	*FrameALAdprCtx = (CDSL_ADAPTER_FRAMEAL_T *) Adapter->FrameMod;

	NDIS_STATUS				Status;

	// Allocate NDIS block of memory

	Status = ALLOCATE_MEMORY( (PVOID)&pFrameALVC,
	                          sizeof( FRAMEAL_VC_T ),
	                          MEMORY_TAG );

	// if NDIS does not provide memory
	// This module will not function.
	if (Status != STATUS_SUCCESS)
	{
		return Status;
	}

	// clear FrameAL Link structure
	CLEAR_MEMORY
	(
	    (PVOID *)pFrameALVC,
	    sizeof( FRAMEAL_VC_T )
	);

	// save Frame link structure pointer in link structure
	VCHdl->FrameALVCMod = pFrameALVC;

	// save parameter block in FrameAl link Stucture
	COPY_MEMORY
	(
	    (void *) &pFrameALVC->Parameters,
	    (void *) VCParameters,
	    sizeof(FRAMEAL_VC_PARAMETERS)
	);

	pFrameALVC->Parameters.RfcEncapsulationMode = FrameALAdprCtx->RfcEncapsulationMode;

	//
	Status =  FrameALATMVCInit( VCHdl);

	if (Status != STATUS_SUCCESS)
	{
		return Status;
	}

	Status =  FrameALAALVCInit( VCHdl);

	if (Status != STATUS_SUCCESS)
	{
		return Status;
	}

	// mark this VC as in use
	pFrameALVC->State = FRAMEAL_VC_ACTIVE;

	return STATUS_SUCCESS;
}


/*******************************************************************************
FUNCTION NAME:
	FrameALLinkShutdown

ABSTRACT:	
	Shutdowns a link instance.  

RETURN:
	NDIS Status

DETAILS:
	This function releases any buffers (TX or RX) allocated to FrameAL by
	buffer management. After the buffers are released, the memory for 
	the FrameAL link context is released..
*******************************************************************************/

GLOBAL NDIS_STATUS FrameALLinkShutdown
(
    IN	CDSL_LINK_T	*LinkHdl	// Link Handle to data stream
)
{
	FRAMEAL_LINK_T  *pFrameALLinkT = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;
	BOOLEAN 		TimerCanceled;

	if (pFrameALLinkT == NULL)
	{
		return STATUS_SUCCESS;
	}

	// Let the ATM layer release its block of memory
	FrameALATMLinkShutdown ( LinkHdl);
	pFrameALLinkT->FrameALTimerState =TMR_CANCELLED;
	CANCEL_TIMER( &pFrameALLinkT->FrameALTimer,
	              &TimerCanceled);

	if (TimerCanceled == FALSE)
	{
		TimerCanceled = WAIT_EVENT(	&pFrameALLinkT->FrameALTimerStoppedEvent,
		                            FRAMEAL_TIMER_PERIOD );
	}

	// release NDIS block of memory
	FREE_MEMORY
	(
	    (PVOID) LinkHdl->FrameMod,
	    sizeof( FRAMEAL_LINK_T ),
	    0
	);

	//y	DBG_CDSL_DISPLAY(
	//y		DBG_LVL_MISC_INFO,
	//y		LinkHdl->pThisAdapter,
	//y		("FrameAL: Exit link shutdown") );

	LinkHdl->FrameMod = NULL;

	return STATUS_SUCCESS;
}



/*******************************************************************************
FUNCTION NAME:
	FrameALVCShutdown

ABSTRACT:	
	Shutdowns a VC instance.  

RETURN:
	NDIS Status

DETAILS:
   	First the sublayers are notified of the VC shutdown. Then FrameAL 
   	releases the memory for	the FrameAL link context is released.
*******************************************************************************/

GLOBAL NDIS_STATUS FrameALVCShutdown
(
    IN	CDSL_VC_T	*VCHdl	// Link Handle to data stream
)
{
	FRAMEAL_VC_T  *pFrameALVC = (FRAMEAL_VC_T*)VCHdl->FrameALVCMod;

	NDIS_STATUS Status;

	if (pFrameALVC == NULL)
	{
		// must already be shutdown
		return STATUS_SUCCESS;
	}

	// Let the ATM layer release its block of memory
	Status = (FrameALATMVCShutdown ( VCHdl));
	if (Status == STATUS_SUCCESS)
	{
		// Let the ATM layer release its block of memory
		Status = FrameALAALVCShutdown ( VCHdl);
	}

	if ( Status == COMMAND_PENDING)
	{
		pFrameALVC->State = FRAMEAL_VC_DEACTIVE_PEND;
		return Status;
	}

	VCHdl->FrameALVCMod = NULL;

	// release NDIS block of memory
	FREE_MEMORY
	(
	    (PVOID) pFrameALVC,
	    sizeof( FRAMEAL_VC_T ),
	    0
	);

	//y	DBG_CDSL_DISPLAY(
	//y		DBG_LVL_MISC_INFO,
	//y		LinkHdl->pThisAdapter,
	//y		("FrameAL: Exit link shutdown") );

	return STATUS_SUCCESS;
}

/*******************************************************************************
FUNCTION NAME:
	FrameALEvent

ABSTRACT:	
	Processes an event. 
	
RETURNS:
	None
		  
DETAILS:
	No events are currently defined
*******************************************************************************/

GLOBAL void FrameALEvent
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	WORD			 Event,			// Event enumeration
    IN	void			*Data			// Pointer to further data dependent on event
)
{
	// Nothing to do here
	return;
}


/*******************************************************************************
FUNCTION NAME:
	FrameALCfgInit

ABSTRACT:
	Processes an configuration information.   

RETURN:
	NDIS Status

DETAILS:
	
*******************************************************************************/

GLOBAL NDIS_STATUS FrameALCfgInit
(
    IN	CDSL_ADAPTER_T	*AdprCtx,			// Driver supplied adapter context
    // (&ThisAdapter)  Required for events that
    // are not associated with a link
    IN PTIG_USER_PARAMS  pUserParams
)
{
	int Status;
	CDSL_ADAPTER_FRAMEAL_T	*FrameALAdprCtx;

	// Allocate memory for the FrameAL adapter context structure.
	Status = ALLOCATE_MEMORY( (PVOID)&FrameALAdprCtx,
	                          sizeof(CDSL_ADAPTER_FRAMEAL_T),
	                          MEMORY_TAG );

	if (Status == STATUS_SUCCESS)
	{
		// Initialize the FrameAL context structure values to NULL
		// and fill the Adapter Context structure with the pointer value
		// to the newly allocated FrameAL adpater context structure.
		CLEAR_MEMORY (FrameALAdprCtx, sizeof(CDSL_ADAPTER_FRAMEAL_T));
		AdprCtx->FrameMod = FrameALAdprCtx;
	}
	else
	{
		// FrameAL could not allocate an adapter context
		// structure so bail out with an error status.
		return Status;
	}

	FrameALAdprCtx->RfcEncapsulationMode = pUserParams->RfcEncapsulationMode;
	FrameALAdprCtx->CellBurstSize		 = pUserParams->CellBurstSize ;

	return STATUS_SUCCESS;
}


/*******************************************************************************
FUNCTION NAME:
	FrameALStatsRequest

ABSTRACT:
	Return the statistics block to the caller..  

RETURNS:
	Pointer to Link Statistics

DETAILS:
    Return the a pointer to link statistics of 
    a given link.		    
*******************************************************************************/

GLOBAL FRAMEAL_LINK_STATS_T * FrameALStatsRequest
(
    IN	CDSL_LINK_T		*LinkHdl
)
{
	// Get pointer to statistics structure for this link
	FRAMEAL_LINK_T  *pFrameALLinkT = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;
	return  (&pFrameALLinkT->Stats);
}


/******************************************************************************
FUNCTION NAME:
	FrameALDeviceSpecific

ABSTRACT:
	Process Device Specific OIDs

RETURN:
	Ndis Status

DETAILS:
	Only OID is for frame AL statistics. 
******************************************************************************/
GLOBAL NDIS_STATUS FrameALDeviceSpecific(
    CDSL_ADAPTER_T				* pThisAdapter,
    BACK_DOOR_T					* pBackDoorBuf,
    PULONG						pNumBytesNeeded,
    PULONG						pNumBytesWritten )

{
	NDIS_STATUS				Status=STATUS_SUCCESS;
	DWORD 					VCIndex;
	FRAMEAL_LINK_T			* pFrameALLink;
	CDSL_LINK_T				* LinkHdl;
	FRAMEAL_LINK_T			* pFrameALLinkT;
	CDSL_ADAPTER_FRAMEAL_T	* FrameALAdprCtx;


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

	ASSERT
	(
	    (pThisAdapter != (CDSL_ADAPTER_T *) 0  )
	);

	if ( pBackDoorBuf->TotalSize < SIZEOF_DWORD_ALIGN( BACK_DOOR_T ) )
	{
		if ( pBackDoorBuf->TotalSize != SIZEOF_DWORD_ALIGN( MINI_BACK_DOOR_T ) )
		{
			pBackDoorBuf->ResultCode = ERROR_WARNING( RESULT_REQ_BUFF_TOO_SMALL );
			pBackDoorBuf->NeededSize = SIZEOF_DWORD_ALIGN( BACK_DOOR_T );
			*pNumBytesNeeded = pBackDoorBuf->NeededSize;
			return ERROR_WARNING( RESULT_REQ_BUFF_TOO_SMALL );
		}
	}

	// only process request codes for FRAMEAL
	switch (pBackDoorBuf->ReqCode)
	{

	case  BD_FRAMEAL_GET_STATS:
		// Get statistics for display on Control Panel

		pBackDoorBuf->Params.BdFrameALStats.NumTxBytes			= 0;
		pBackDoorBuf->Params.BdFrameALStats.NumTxFrames			= 0;
		pBackDoorBuf->Params.BdFrameALStats.NumRxBytes			= 0;
		pBackDoorBuf->Params.BdFrameALStats.NumRxFrames 		= 0;
		pBackDoorBuf->Params.BdFrameALStats.NumCrcErrs			= 0;
		pBackDoorBuf->Params.BdFrameALStats.NumAbortErrs		= 0;
		pBackDoorBuf->Params.BdFrameALStats.NumTxUnderrunErrs	= 0;
		pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_DATA_UNAVAILABLE );

		//	get stats for link .
		LinkHdl = GET_LINK_FROM_ADAPTER( pThisAdapter  );
		if ((LinkHdl->Allocated == TRUE) &&	 (LinkHdl->FrameMod != NULL))
		{
			pFrameALLinkT = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;
			pBackDoorBuf->Params.BdFrameALStats.NumTxBytes			= 	pFrameALLinkT->Stats.TXBytes.Cnt;
			pBackDoorBuf->Params.BdFrameALStats.NumTxFrames			=	pFrameALLinkT->Stats.TXFrames.Cnt;
			pBackDoorBuf->Params.BdFrameALStats.NumRxBytes			=	pFrameALLinkT->Stats.RXBytes.Cnt;
			pBackDoorBuf->Params.BdFrameALStats.NumRxFrames 		=	pFrameALLinkT->Stats.RXFrames.Cnt;
			pBackDoorBuf->Params.BdFrameALStats.NumCrcErrs			=	pFrameALLinkT->Stats.RXErrors.Cnt;
			pBackDoorBuf->Params.BdFrameALStats.NumAbortErrs		=	pFrameALLinkT->Stats.RXAborts.Cnt;
			pBackDoorBuf->Params.BdFrameALStats.NumTxUnderrunErrs	=	pFrameALLinkT->Stats.TXErrors.Cnt;

			// mark OID as successful
			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		}

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdFrameALStats )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;


	case	BD_FRAMEAL_ATM_GET_STATS:

		pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_DATA_UNAVAILABLE );
		pBackDoorBuf->Params.BdFrameALAtmStats.NumTxBytes = 0;
		pBackDoorBuf->Params.BdFrameALAtmStats.NumTxCells = 0;
		pBackDoorBuf->Params.BdFrameALAtmStats.NumTxMgmtCells = 0;
		pBackDoorBuf->Params.BdFrameALAtmStats.NumTxClpEqual0Cells = 0;
		pBackDoorBuf->Params.BdFrameALAtmStats.NumTxClpEqual1Cells = 0;
		pBackDoorBuf->Params.BdFrameALAtmStats.NumRxBytes = 0;
		pBackDoorBuf->Params.BdFrameALAtmStats.NumRxCells = 0;
		pBackDoorBuf->Params.BdFrameALAtmStats.NumRxMgmtCells = 0;
		pBackDoorBuf->Params.BdFrameALAtmStats.NumRxClpEqual0Cells = 0;
		pBackDoorBuf->Params.BdFrameALAtmStats.NumRxClpEqual1Cells = 0;
		pBackDoorBuf->Params.BdFrameALAtmStats.NumRxHecErrs = 0;
		pBackDoorBuf->Params.BdFrameALAtmStats.NumRxCellAlignErrs = 0;
		pBackDoorBuf->Params.BdFrameALAtmStats.NumRxUnroutCellErrs = 0;

		//
		//	Return ATM layer stats to the control panel
		//  Control panel sends down an array of VC Indexs of VCs the control panel wants stats for.
		//  The driver checks each index to see if there is a match to a valid VC.
		// 	If the Index is valid, the stats for that VC are placed in next available entry of the VCArray
		//  Link Stats are marked using a special value for the VC index (0xffff).
		//  If the driver finds that value in the list of requested VCs. The links stats are placed
		//  in the next available VCArray entry.

		// only will work for one link
		LinkHdl = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if ((LinkHdl->Allocated == TRUE) &&	 (LinkHdl->FrameMod != NULL))
		{

			// check for a request of the links stats.
			if (pBackDoorBuf->Params.BdFrameALAtmStats.VcIndex == BD_ATM_STATS_INDEX_FOR_LINK)
			{
				// get a ptr to structure where the links stats are stored
				pFrameALLink = LinkHdl->FrameMod;

				if (pFrameALLink != NULL)
				{
					// move the stats to the VcArray entry
					pBackDoorBuf->Params.BdFrameALAtmStats.NumTxBytes =
					    pFrameALLink->Stats.TXBytes.Cnt;
					pBackDoorBuf->Params.BdFrameALAtmStats.NumTxCells =
					    (pFrameALLink->Stats.TXATMCellsCLP0.Cnt + pFrameALLink->Stats.TXATMCellsCLP1.Cnt);

					pBackDoorBuf->Params.BdFrameALAtmStats.NumTxMgmtCells =
					    pFrameALLink->Stats.TXATMCellsMgmCell.Cnt;

					pBackDoorBuf->Params.BdFrameALAtmStats.NumTxClpEqual0Cells =
					    pFrameALLink->Stats.TXATMCellsCLP0.Cnt;

					pBackDoorBuf->Params.BdFrameALAtmStats.NumTxClpEqual1Cells =
					    pFrameALLink->Stats.TXATMCellsCLP1.Cnt;

					pBackDoorBuf->Params.BdFrameALAtmStats.NumRxBytes =
					    pFrameALLink->Stats.RXBytes.Cnt;
					pBackDoorBuf->Params.BdFrameALAtmStats.NumRxCells =
					    (pFrameALLink->Stats.RXATMCellsGoodCLP0.Cnt + pFrameALLink->Stats.RXATMCellsGoodCLP1.Cnt);

					pBackDoorBuf->Params.BdFrameALAtmStats.NumRxMgmtCells =
					    pFrameALLink->Stats.RXATMCellsMgmCell.Cnt;
					pBackDoorBuf->Params.BdFrameALAtmStats.NumRxClpEqual0Cells =
					    pFrameALLink->Stats.RXATMCellsGoodCLP0.Cnt;
					pBackDoorBuf->Params.BdFrameALAtmStats.NumRxClpEqual1Cells =
					    pFrameALLink->Stats.RXATMCellsGoodCLP1.Cnt;

					pBackDoorBuf->Params.BdFrameALAtmStats.NumRxHecErrs =
					    pFrameALLink->Stats.RXATMCellsBadHEC.Cnt;
					pBackDoorBuf->Params.BdFrameALAtmStats.NumRxCellAlignErrs = 0;
					pBackDoorBuf->Params.BdFrameALAtmStats.NumRxUnroutCellErrs =
					    pFrameALLink->Stats.RXATMCellsMisrouted.Cnt;

					pBackDoorBuf->Params.BdFrameALAtmStats.VcIndex = BD_ATM_STATS_INDEX_FOR_LINK;

					pBackDoorBuf->ResultCode = RESULT_SUCCESS;
				}
				else
				{
					// A link structure is not available
					break;
				}
			}
			else
			{
				VCIndex = pBackDoorBuf->Params.BdFrameALAtmStats.VcIndex;
				if ( VCIndex < MAX_VC_PER_LINK)
				{
					// get the stats for the requested VC
					Status = 	FrameALUpdateATMStats
					          (
					              &LinkHdl->VC_Info [VCIndex],
					              &pBackDoorBuf->Params.BdFrameALAtmStats
					          );
					if (Status == VC_STATS_OK)
					{
						// update number of VCarray entries in use
						//							pBackDoorBuf->Params.BdFrameALAtmStats.VcIndex = VCIndex;
						pBackDoorBuf->ResultCode = RESULT_SUCCESS;
					}
				}
			}
		}

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdFrameALAtmStats )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;


	case	BD_FRAMEAL_AAL_GET_STATS:

		pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_DATA_UNAVAILABLE );
		pBackDoorBuf->Params.BdFrameALAalStats.AalType = 0;
		pBackDoorBuf->Params.BdFrameALAalStats.NumTxGoodBytes = 0;
		pBackDoorBuf->Params.BdFrameALAalStats.NumTxGoodFrames = 0;
		pBackDoorBuf->Params.BdFrameALAalStats.NumTxDiscardedBytes = 0;
		pBackDoorBuf->Params.BdFrameALAalStats.NumTxDiscardedFrames = 0;
		pBackDoorBuf->Params.BdFrameALAalStats.NumRxGoodBytes = 0;
		pBackDoorBuf->Params.BdFrameALAalStats.NumRxGoodFrames = 0;
		pBackDoorBuf->Params.BdFrameALAalStats.NumRxDiscardedBytes = 0;
		pBackDoorBuf->Params.BdFrameALAalStats.NumRxDiscardedFrames = 0;
		pBackDoorBuf->Params.BdFrameALAalStats.NumRxCrcErrs = 0;
		pBackDoorBuf->Params.BdFrameALAalStats.NumRxInvalidLenErrs = 0;
		pBackDoorBuf->Params.BdFrameALAalStats.NumRxTimeoutErrs = 0;

		//
		//	Return AAL layer stats to the control panel
		//  Control panel sends down an array of VC Indexs of VCs the control panel wants stats for.
		//  The driver checks each index to see if there is a match to a valid VC.
		// 	If the Index is valid, the stats for that VC are placed in next available entry of the VCArray
		//  Link Stats are marked using a special value for the VC index (0xffff).
		//  If the driver finds that value in the list of requested VCs. The links stats are placed
		//  in the next available VCArray entry.

		// only will work for one link
		LinkHdl = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if ((LinkHdl->Allocated == TRUE) &&	 (LinkHdl->FrameMod != NULL))
		{

			// check for a request of the links stats.
			if (pBackDoorBuf->Params.BdFrameALAalStats.VcIndex == BD_AAL_STATS_INDEX_FOR_LINK)
			{

				// get a ptr to structure where the links stats are stored/

				pFrameALLink = LinkHdl->FrameMod;
				if (pFrameALLink != NULL)
				{
					pBackDoorBuf->Params.BdFrameALAalStats.NumTxGoodBytes +=
					    pFrameALLink->AALLinkBlock.AALTxGoodByte.Cnt;
					pBackDoorBuf->Params.BdFrameALAalStats.NumTxGoodFrames +=
					    pFrameALLink->AALLinkBlock.AALTxGood.Cnt;
					pBackDoorBuf->Params.BdFrameALAalStats.NumTxDiscardedBytes +=
					    pFrameALLink->AALLinkBlock.AALTxDiscarded.Cnt;
					pBackDoorBuf->Params.BdFrameALAalStats.NumTxDiscardedFrames +=
					    pFrameALLink->AALLinkBlock.AALTxDiscardedByte.Cnt;
					pBackDoorBuf->Params.BdFrameALAalStats.NumRxGoodBytes +=
					    pFrameALLink->AALLinkBlock.AALRxGoodByte.Cnt;
					pBackDoorBuf->Params.BdFrameALAalStats.NumRxGoodFrames +=
					    pFrameALLink->AALLinkBlock.AALRxGood.Cnt;
					pBackDoorBuf->Params.BdFrameALAalStats.NumRxDiscardedBytes +=
					    pFrameALLink->AALLinkBlock.AALRxDiscardedByte.Cnt;
					pBackDoorBuf->Params.BdFrameALAalStats.NumRxDiscardedFrames +=
					    pFrameALLink->AALLinkBlock.AALRxDiscarded.Cnt;
					pBackDoorBuf->Params.BdFrameALAalStats.NumRxCrcErrs +=
					    pFrameALLink->AALLinkBlock.AALRxInvalidCRC.Cnt;
					pBackDoorBuf->Params.BdFrameALAalStats.NumRxInvalidLenErrs +=
					    pFrameALLink->AALLinkBlock.AALRxInvalidLen.Cnt;
					pBackDoorBuf->Params.BdFrameALAalStats.NumRxTimeoutErrs +=
					    pFrameALLink->AALLinkBlock.AALRxReassemblyTimeout.Cnt;

					//						pBackDoorBuf->Params.BdFrameALAalStats.VcIndex = BD_AAL_STATS_INDEX_FOR_LINK;

					pBackDoorBuf->ResultCode = RESULT_SUCCESS;
				}
				else
				{
					// A link structure is not available
					break;
				}
			}
			else
			{
				VCIndex = pBackDoorBuf->Params.BdFrameALAalStats.VcIndex;
				if ( VCIndex < MAX_VC_PER_LINK)
				{
					// get the stats for the requested VC
					Status = 	FrameALUpdateAALStats
					          (
					              &LinkHdl->VC_Info [VCIndex],
					              &pBackDoorBuf->Params.BdFrameALAalStats
					          );

					if (Status == VC_STATS_OK)
					{
						//							pBackDoorBuf->Params.BdFrameALAalStats.VcIndex = VCIndex;
						pBackDoorBuf->ResultCode = RESULT_SUCCESS;
					}
				}
			}
		}

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdFrameALAalStats )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;


	case	BD_FRAMEAL_GET_CONFIG:
		// Get FrameAL configuration for display on Control Panel

		FrameALAdprCtx = (CDSL_ADAPTER_FRAMEAL_T *) pThisAdapter->FrameMod;

		switch (FrameALAdprCtx->RfcEncapsulationMode)
		{
		case BD_ENCAPSULATION_PPPOA_VCMUX:	// PPP over ATM VXMUX
		case BD_ENCAPSULATION_PPPOA_LLC:	// PPP over ATM LLC
		case BD_ENCAPSULATION_BIPOA_LLC:	// Bridged IP over ATM LLCSNAP
		case BD_ENCAPSULATION_RIPOA_LLC:	// Routed IP over ATM LLCSNAP
		case BD_ENCAPSULATION_BIPOA_VCMUX:	// Bridged IP over ATM VCMUX
		case BD_ENCAPSULATION_RIPOA_VCMUX:	// Routed IP over ATM VCMUX
			// The next three enumerations are added for CNXT-defined encapsulations,
			// they are not found in the ADSL Forum documentation.
		case BD_ENCAPSULATION_PPPOA_NONE:	// PPP over ATM no encapsulation
		case BD_ENCAPSULATION_BIPOA_NONE:	// Bridged IP over ATM no encapsulation
		case BD_ENCAPSULATION_RIPOA_NONE:	// Routed IP over ATM no encapsulation
			pBackDoorBuf->Params.BdFrameALConfig.RfcEncapsulationMode =
			    FrameALAdprCtx->RfcEncapsulationMode;
			// mark OID as successful
			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			break;

		case BD_ENCAPSULATION_IPOA:			// Classical IP over ATM
		case BD_ENCAPSULATION_NATIVE_ATM:	// Native ATM
		case BD_ENCAPSULATION_PROPRIETARY:	// Proprietary
		default:
			pBackDoorBuf->Params.BdFrameALConfig.RfcEncapsulationMode =
			    FrameALAdprCtx->RfcEncapsulationMode;
			pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_PARAM_OUT_OF_RANGE );
			break;
		}

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdFrameALConfig )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;


	case	BD_FRAMEAL_SET_CONFIG:
		// Get FrameAL configuration for display on Control Panel

		FrameALAdprCtx = (CDSL_ADAPTER_FRAMEAL_T *) pThisAdapter->FrameMod;

		switch (pBackDoorBuf->Params.BdFrameALConfig.RfcEncapsulationMode)
		{
		case BD_ENCAPSULATION_PPPOA_VCMUX:	// PPP over ATM VXMUX
		case BD_ENCAPSULATION_PPPOA_LLC:	// PPP over ATM LLC
		case BD_ENCAPSULATION_BIPOA_LLC:	// Bridged IP over ATM LLCSNAP
		case BD_ENCAPSULATION_RIPOA_LLC:	// Routed IP over ATM LLCSNAP
		case BD_ENCAPSULATION_BIPOA_VCMUX:	// Bridged IP over ATM VCMUX
		case BD_ENCAPSULATION_RIPOA_VCMUX:	// Routed IP over ATM VCMUX
			// The next three enumerations are added for CNXT-defined encapsulations,
			// they are not found in the ADSL Forum documentation.
		case BD_ENCAPSULATION_PPPOA_NONE:	// PPP over ATM no encapsulation
		case BD_ENCAPSULATION_BIPOA_NONE:	// Bridged IP over ATM no encapsulation
		case BD_ENCAPSULATION_RIPOA_NONE:	// Routed IP over ATM no encapsulation
			FrameALAdprCtx->RfcEncapsulationMode =
			    pBackDoorBuf->Params.BdFrameALConfig.RfcEncapsulationMode;
			// mark OID as successful
			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			break;

		case BD_ENCAPSULATION_IPOA:			// Classical IP over ATM
		case BD_ENCAPSULATION_NATIVE_ATM:	// Native ATM
		case BD_ENCAPSULATION_PROPRIETARY:	// Proprietary
		default:
			pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_PARAM_OUT_OF_RANGE );
			break;
		}

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdFrameALConfig )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;


	default:
		Status = STATUS_UNSUPPORTED;
		break;


	}  // end switch

	return Status;
}	// end function

/******************************************************************************
FUNCTION NAME:
	FrameALUpdateATMStats
	
ABSTRACT:

RETURN:
	ATM Stats Status

DETAILS:
	Get the stats for VC.
******************************************************************************/
LOCAL DWORD FrameALUpdateATMStats(
    IN 	CDSL_VC_T * pVCHdl,
    IN 	BD_FRAMEAL_ATM_STATS_T * pATMStats)

{
	FRAMEAL_VC_T  		*pFrameALVC = NULL;
	DWORD Status = VC_STATS_OK;

	if ( pVCHdl != NULL)
	{
		pFrameALVC =  (FRAMEAL_VC_T *) pVCHdl->FrameALVCMod;
		if (pFrameALVC != NULL)
		{
			pATMStats->Vpi					= pFrameALVC->Parameters.Vpi;
			pATMStats->Vci					= pFrameALVC->Parameters.Vci;
			pATMStats->NumTxBytes 			= pFrameALVC->Stats.TXBytes.Cnt;
			pATMStats->NumTxCells			= (pFrameALVC->Stats.TXATMCellsCLP0.Cnt
			                           + pFrameALVC->Stats.TXATMCellsCLP1.Cnt);

			pATMStats->NumTxMgmtCells		= pFrameALVC->Stats.TXATMCellsMgmCell.Cnt;

			pATMStats->NumTxClpEqual0Cells	= pFrameALVC->Stats.TXATMCellsCLP0.Cnt;
			pATMStats->NumTxClpEqual1Cells	= pFrameALVC->Stats.TXATMCellsCLP1.Cnt;
			pATMStats->NumRxBytes			= pFrameALVC->Stats.RXBytes.Cnt;
			pATMStats->NumRxCells			= (pFrameALVC->Stats.RXATMCellsGoodCLP0.Cnt
			                           + pFrameALVC->Stats.RXATMCellsGoodCLP1.Cnt);

			pATMStats->NumRxMgmtCells		= pFrameALVC->Stats.RXATMCellsMgmCell.Cnt;
			pATMStats->NumRxClpEqual0Cells	= pFrameALVC->Stats.RXATMCellsGoodCLP0.Cnt;
			pATMStats->NumRxClpEqual1Cells	= pFrameALVC->Stats.RXATMCellsGoodCLP1.Cnt;
		}
		else
		{
			Status = VC_STATS_FAILED;
		}

	}
	else
	{
		Status = VC_STATS_FAILED;
	}


	return Status;
}

/******************************************************************************
FUNCTION NAME:
	FrameALUpdateAALStats
	
ABSTRACT:

RETURN:
	AAL Stats Status

DETAILS:  
	Get the stats for VC.
******************************************************************************/
LOCAL DWORD FrameALUpdateAALStats(
    IN 	CDSL_VC_T * pVCHdl,
    IN 	BD_FRAMEAL_AAL_STATS_T * pAALStats)

{
	FRAMEAL_VC_T  		*pFrameALVC = NULL;
	DWORD Status = VC_STATS_OK;

	if ( pVCHdl != NULL)
	{
		pFrameALVC =  (FRAMEAL_VC_T *) pVCHdl->FrameALVCMod;
		if (pFrameALVC != NULL)
		{
			pAALStats->Vpi					= pFrameALVC->Parameters.Vpi;
			pAALStats->Vci					= pFrameALVC->Parameters.Vci;
			pAALStats->NumTxGoodBytes	  	= pFrameALVC->AALVcCtx.AALTxGoodByte.Cnt;
			pAALStats->NumTxGoodFrames	  	= pFrameALVC->AALVcCtx.AALTxGood.Cnt;
			pAALStats->NumTxDiscardedBytes	= pFrameALVC->AALVcCtx.AALTxDiscarded.Cnt;
			pAALStats->NumTxDiscardedFrames	= pFrameALVC->AALVcCtx.AALTxDiscardedByte.Cnt;
			pAALStats->NumRxGoodBytes	  	= pFrameALVC->AALVcCtx.AALRxGoodByte.Cnt;
			pAALStats->NumRxGoodFrames	  	= pFrameALVC->AALVcCtx.AALRxGood.Cnt;
			pAALStats->NumRxDiscardedBytes	= pFrameALVC->AALVcCtx.AALRxDiscardedByte.Cnt;
			pAALStats->NumRxDiscardedFrames	= pFrameALVC->AALVcCtx.AALRxDiscarded.Cnt;
			pAALStats->NumRxCrcErrs		  	= pFrameALVC->AALVcCtx.AALRxInvalidCRC.Cnt;
			pAALStats->NumRxInvalidLenErrs	= pFrameALVC->AALVcCtx.AALRxInvalidLen.Cnt;
			pAALStats->NumRxTimeoutErrs	  	= pFrameALVC->AALVcCtx.AALRxReassemblyTimeout.Cnt;
		}
		else
		{
			Status = VC_STATS_FAILED;
		}

	}
	else
	{
		Status = VC_STATS_FAILED;
	}


	return Status;
}


/******************************************************************************
FUNCTION NAME:
	FrameALSetInformationHdlr

ABSTRACT:
	This function is called from the CDSLSetInformationHdlr function
	(the function called by NDIS to handle this drivers
	SetInformationHandler calls) to handle the Information Handler
	Sets that come through NDIS via WAN.

RETURN:
	NDIS Status

DETAILS:
	Frame AL does not support any Set OIDs at this time.
******************************************************************************/
/* not supported by Linux
NDIS_STATUS FrameALSetInformationHdlr(
	IN CDSL_ADAPTER_T	* pThisAdapter,
	IN NDIS_OID			Oid,
	IN PVOID			InfoBuffer,
	IN ULONG			InfoBufferLength,
	OUT PULONG			pBytesRead,
	OUT PULONG			pBytesNeeded )
{
	NDIS_STATUS				Status;
	ULONG					NumBytesNeeded;
	ULONG					NumBytesRead;


//	DBG_CDSL_DISPLAY(
//		DBG_LVL_WARNING,
//		pThisAdapter,
//		(	"    Function Entry 'FrameALSetInformationHdlr':   OID=0x%08lX.\n", Oid) );
//

	NumBytesNeeded = 0;
	NumBytesRead = 0;

	//	Determine which OID is being requested and do the right thing.
	switch (Oid)
	{
		default:
			//	Unknown OID
  //			DBG_CDSL_DISPLAY(
  //				DBG_LVL_WARNING,
  //				pThisAdapter,
  //				(	"    Function 'FrameALSetInformationHdlr':   Unknown OID  -  0x%08lX.\n", Oid) );
			Status = NDIS_STATUS_INVALID_OID;
			break;
	}

	//	Fill in the size fields before we leave.
	*pBytesNeeded += NumBytesNeeded;
	*pBytesRead += NumBytesRead;

	return (Status);
}
*/

#if PROJECTS_NDIS_IF == PROJECT_NDIS_IF_CO
/******************************************************************************
FUNCTION NAME:
	FrameAlStatsRequestHdlr	
ABSTRACT:
	This function is called from the CoNDIS request handler to 
	get stats.

RETURN:
	NDIS Status

DETAILS:
******************************************************************************/

GLOBAL NDIS_STATUS FrameAlRequestHdlr
(
    IN		CDSL_ADAPTER_T 	* 	pAdapter,
    IN		CDSL_VC_T 		* 	pVCHdl,
    IN OUT	PNDIS_REQUEST	 pRequest
)
{
	NDIS_STATUS				Status;
	PCHAR	   				DataToMove;
	ULONG					Data = 0;
	ULONG	   				DataLength;
	ULONG					Statistics;

	CDSL_LINK_T		*		LinkHdl;
	FRAMEAL_VC_T  	* 		pFrameALVC;
	FRAMEAL_LINK_T	*		pFrameALLink;

	//
	//	Default the status to success for below.
	//
	Status = STATUS_SUCCESS;

	//
	//	Default length.
	//
	DataToMove = (PUCHAR)&Data;
	DataLength = 0;

	if (pVCHdl == NULL)
	{
		if (pAdapter != NULL)
		{
			// get link handle
			LinkHdl = GET_LINK_FROM_ADAPTER(pAdapter->AdapterLink);

			pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;

			switch (pRequest->DATA.QUERY_INFORMATION.Oid)
			{
			case    OID_GEN_CO_BYTES_XMIT:

				DataToMove = (PUCHAR)&pFrameALLink->Stats.COTXBytes.Cnt;
				DataLength = sizeof(pFrameALLink->Stats.COTXBytes.Cnt);

				break;

			case    OID_GEN_CO_BYTES_RCV:

				DataToMove = (PUCHAR)&pFrameALLink->Stats.CORXBytes.Cnt;
				DataLength = sizeof(pFrameALLink->Stats.CORXBytes.Cnt);

				break;
			case	OID_GEN_CO_RCV_CRC_ERROR:
				Data = pFrameALLink->AALLinkBlock.AALRxInvalidCRC.Cnt;
				DataLength = sizeof(pFrameALLink->Stats.RXBytes.Cnt);
				break;

			case 	OID_GEN_CO_XMIT_PDUS_OK:
				Data = pFrameALLink->AALLinkBlock.AALTxGood.Cnt;
				DataLength = sizeof(ULONG);
				break;
			case	OID_GEN_CO_RCV_PDUS_OK:
				Data = pFrameALLink->AALLinkBlock.AALRxGood.Cnt;
				DataLength = sizeof(ULONG);
				break;
			case	OID_GEN_CO_XMIT_PDUS_ERROR:
				Data = pFrameALLink->AALLinkBlock.AALTxDiscarded.Cnt;
				DataLength = sizeof(ULONG);
				break;
			case	OID_GEN_CO_RCV_PDUS_ERROR:
				Data = 	pFrameALLink->AALLinkBlock.AALRxInvalidCRC.Cnt	+
				        pFrameALLink->AALLinkBlock.AALRxInvalidLen.Cnt 	+
				        pFrameALLink->AALLinkBlock.AALRxReassemblyTimeout.Cnt;
				DataLength = sizeof(ULONG);
				break;
			case	OID_GEN_CO_RCV_PDUS_NO_BUFFER:

				break;


			case 	OID_ATM_RCV_CELLS_OK:
				DataToMove = (PUCHAR)&pFrameALLink->Stats.COATMRXCells.Cnt;
				DataLength = sizeof(pFrameALLink->Stats.COATMRXCells.Cnt);

				break;

			case 	OID_ATM_XMIT_CELLS_OK:
				DataToMove = (PUCHAR)&pFrameALLink->Stats.COATMTXCells.Cnt;
				DataLength = sizeof(ULONG);

				break;

			case 	OID_ATM_RCV_CELLS_DROPPED:

				break;
			case 	OID_ATM_CELLS_HEC_ERROR:
				DataToMove = (PUCHAR)&pFrameALLink->Stats.COATMRXCellsDrops.Cnt;
				DataLength = sizeof(pFrameALLink->Stats.COATMRXCellsDrops.Cnt);
				break;

			case 	OID_ATM_RCV_REASSEMBLY_ERROR:

				break;

			case   	OID_ATM_RCV_INVALID_VPI_VCI:

				Data= pFrameALLink->Stats.RXATMCellsMisrouted.Cnt;
				DataLength = sizeof(pFrameALLink->Stats.RXATMCellsMisrouted.Cnt);
				break;

			default:
				Status = NDIS_STATUS_NOT_SUPPORTED;

				break;

			}
		}
	}
	else
	{

		pFrameALVC =  (FRAMEAL_VC_T *) pVCHdl->FrameALVCMod;

		if (pFrameALVC != NULL)
		{
			switch (pRequest->DATA.QUERY_INFORMATION.Oid)
			{

			case    OID_GEN_CO_BYTES_XMIT:

				DataToMove = (PUCHAR)&pFrameALVC->Stats.COTXBytes.Cnt;
				DataLength = sizeof(pFrameALVC->Stats.COTXBytes.Cnt);

				break;

			case    OID_GEN_CO_BYTES_RCV:

				DataToMove = (PUCHAR)&pFrameALVC->Stats.CORXBytes.Cnt;
				DataLength = sizeof(pFrameALVC->Stats.CORXBytes.Cnt);

				break;


			case 	OID_GEN_CO_XMIT_PDUS_OK:
				Data = pFrameALVC->AALVcCtx.AALTxGood.Cnt;
				DataLength = sizeof(ULONG);
				break;

			case	OID_GEN_CO_RCV_PDUS_OK:
				Data = pFrameALVC->AALVcCtx.AALRxGood.Cnt;
				DataLength = sizeof(ULONG);
				break;

			case	OID_GEN_CO_XMIT_PDUS_ERROR:
				Data = pFrameALVC->AALVcCtx.AALTxDiscarded.Cnt;
				DataLength = sizeof(ULONG);
				break;

			case	OID_GEN_CO_RCV_PDUS_ERROR:

			case	OID_GEN_CO_RCV_PDUS_NO_BUFFER:

			case 	OID_ATM_RCV_CELLS_OK:
				DataToMove = (PUCHAR)&pFrameALVC->Stats.COATMRXCells.Cnt;
				DataLength = sizeof(pFrameALVC->Stats.COATMRXCells.Cnt);
				break;

			case 	OID_ATM_XMIT_CELLS_OK:
				DataToMove = (PUCHAR)&pFrameALVC->Stats.COATMRXCells.Cnt;
				DataLength = sizeof(pFrameALVC->Stats.COATMRXCells.Cnt);
				break;
			case 	OID_ATM_RCV_CELLS_DROPPED:


			case 	OID_ATM_CELLS_HEC_ERROR:
			case 	OID_ATM_RCV_REASSEMBLY_ERROR:
				break;
			default:
				Status = NDIS_STATUS_NOT_SUPPORTED;

				break;
			}
		}

	}

	if (STATUS_SUCCESS == Status)
	{
		if (pRequest->DATA.QUERY_INFORMATION.InformationBufferLength < DataLength)
		{
			pRequest->DATA.QUERY_INFORMATION.BytesNeeded = DataLength;
			Status = NDIS_STATUS_INVALID_LENGTH;
		}
		else
		{
			if (DataLength > 0)
			{
				NdisMoveMemory(
				    pRequest->DATA.QUERY_INFORMATION.InformationBuffer,
				    DataToMove,
				    DataLength);

				pRequest->DATA.QUERY_INFORMATION.BytesWritten = DataLength;
			}
		}
	}

	return(Status);
}

#endif //PROJECTS_NDIS_IF == PROJECT_NDIS_IF_CO

/*******************************************************************************
FUNCTION NAME:
	FrameALGetTxFullBuffer

ABSTRACT:
	Provides a buffer of data that is to be transmitted.  

RETURNS: 
	NDISWAN packet pointer. 

DETAILS:
	The caller may optionally provide a buffer descriptor and associated 
	buffer that FrameAL must use. If FrameAL is transforming the data, 
	it has to do a copy anyway. In this mode, FrameAL must fill the 
	buffer exactly. Otherwise FrameAL will return a different buffer 
	descriptor, possibly avoiding any copying. In either case, 
	FrameAL returns a pointer to the buffer descriptor used
*******************************************************************************/

GLOBAL SK_BUFF_T	*FrameALGetTxFullBuffer
(
    IN	void			*pLink,			// Link Handle to data stream
    IN	SK_BUFF_T	    *InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
)
{
	CDSL_LINK_T		*LinkHdl = (CDSL_LINK_T*)pLink;
	SK_BUFF_T	*BufDescPtr;

	BufDescPtr =  InBufDescPtr;

	// make sure CHIP AL gave Frame AL a buffer.
	if (InBufDescPtr != NULL)
	{
		BufDescPtr = FrameALATMGetTxFullBuffer
		             (
		                 LinkHdl,
		                 InBufDescPtr
		             );
	}

	return ( BufDescPtr);
}


/*******************************************************************************
FUNCTION NAME:
	FrameALPutTxEmptyBuffer

ABSTRACT:
	This function returns a buffer descriptor to FrameAL..  

RETURNS:
	None

DETAILS:
	This returns a buffer descriptor and its associated buffer back to 
	FrameAL after ChipAL no longer needs the data. If FrameAL is 
	using buffers that ChipAL provides when calling FrameALGetTxFullBuffer, 
	then this function is not used.
*******************************************************************************/

GLOBAL void FrameALPutTxEmptyBuffer
(
    IN	void			*pLink,			//Link Handle to data stream
    IN	SK_BUFF_T		*BufferDescPtr	//Pointer to a buffer descriptor
)
{
	CDSL_LINK_T		*LinkHdl=(CDSL_LINK_T*)pLink;

	// let AAL sublayer release buffer
	FrameALATMTxPutEmptyBuf
	(
	    LinkHdl,
	    BufferDescPtr
	);

	return;
}



/*******************************************************************************
FUNCTION NAME:
	FrameALGetRxEmptyBuffer

ABSTRACT:
	This function returns a recevie buffer descriptor to ChipAL.  

RETURNS:
	None

DETAILS:
	This returns a buffer descriptor and its associated buffer for ChipAL 
	to fill up with received data. If ChipAL is providing its own buffers, 
	this function is not used.
*******************************************************************************/

GLOBAL void FrameALGetRxEmptyBuffer
(
    IN	void			*pLink,			// Link Handle to data stream
    OUT	SK_BUFF_T		*BufferDescPtr	//Pointer to a buffer descriptor
)
{
	// Not implemented for Linux
	//	CDSL_LINK_T		*LinkHdl = (CDSL_LINK_T*)pLink;
	//NOT USED
	return;
}



/*******************************************************************************
FUNCTION NAME:
	 FrameALPutRxFullBuffer

ABSTRACT:	This function delivers received raw data to HDLC,	  

RETURNS:
	NDISWAN packet pointe

DETAILS:
	This function gives a received buffer descriptor and its associated 
	buffer to FrameAL. This will be a buffer previously provided 
	by FrameALGetRxEmptybuffer or a ChipAL provided buffer.
*******************************************************************************/

GLOBAL SK_BUFF_T	*FrameALPutRxFullBuffer
(
    IN	void			*pLink,			// Link Handle to data stream
    IN	SK_BUFF_T		*BufferDescPtr	// Pointer to a buffer descriptor
)
{
	CDSL_LINK_T		*LinkHdl=(CDSL_LINK_T*)pLink;

	// make sure ChipAL passed a buffer to this function
	if (BufferDescPtr != NULL)
	{
		return FrameALATMCellAlign(LinkHdl,BufferDescPtr);
	}

	return ((SK_BUFF_T *)0 );
}


/*******************************************************************************
FUNCTION NAME:
	FrameALTxQNotEmpty

ABSTRACT:
	This function wakes up Frame AL TX when new data is ready.  

RETURNS:
	None

DETAILS:
	This function is not used in this implementation
*******************************************************************************/

GLOBAL void FrameALTxQNotEmpty
(
    IN	CDSL_LINK_T		*LinkHdl		// Link Handle to data stream
)
{
	#if 0
	NDIS_WAN_PACKET	BufferDescPtr ;
	WORD Count;
	for (Count = 0; Count < 50; Count++)
	{

		FrameALGetTxFullBuffer( LinkHdl, &BufferDescPtr);
		if (BufferDescPtr.CurrentLength > 0)
		{
			FrameALPutTxEmptyBuffer( LinkHdl, &BufferDescPtr);
		}
	}
	#endif 

	FrameALLinkStartTX( LinkHdl);
	return;
}

/*******************************************************************************
FUNCTION NAME:
	FrameALTimerHandler

ABSTRACT:
	This function is called by ChipAL when the Hobbs timer expires

RETURNS:
	None

DETAILS:
*******************************************************************************/
LOCAL void FrameALTimerHandler
(
    IN ULONG 	pLinkH
)
{
	IN CDSL_LINK_T		* 	LinkHdl = (CDSL_LINK_T*)pLinkH;

	FRAMEAL_LINK_T  *pFrameALLinkT  = (FRAMEAL_LINK_T *) LinkHdl->FrameMod ;
	if 	(pFrameALLinkT != NULL)
	{
		if (pFrameALLinkT->FrameALTimerState == TMR_TIMING)
		{
			FrameALATMTimerHandler( LinkHdl);
			START_TIMER( &pFrameALLinkT->FrameALTimer,
			             FRAMEAL_TIMER_PERIOD );
		}
		else
		{
			// Timer is being shutdown or stopped
			SET_EVENT( &pFrameALLinkT->FrameALTimerStoppedEvent );
		}

	}
}


/*******************************************************************************
FUNCTION NAME:
	FrameALClearStats

ABSTRACT:
	This function is called to clear the link stats

RETURNS:
	None

DETAILS:
*******************************************************************************/

GLOBAL void FrameALClearStats
(
    IN	CDSL_LINK_T		*LinkHdl		// Link Handle to data stream

)
{

	FRAMEAL_LINK_T  *pFrameALLink;


	// make sure we have a link handle
	if ( LinkHdl != NULL)
	{
		int cnt;
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;

		if (pFrameALLink != NULL)
		{
			CLEAR_MEMORY
			(
			    (PVOID *)&pFrameALLink->Stats,
			    sizeof(FRAMEAL_LINK_STATS_T)
			);

			CLEAR_MEMORY
			(
			    (PVOID *)&pFrameALLink->AALLinkBlock,
			    sizeof(FRAMEAL_AAL_LINK_T)
			);
		}
		
		// clear the per vc stats
		for (cnt=0; cnt<MAX_VC_PER_LINK ; cnt++)
		{
			FRAMEAL_VC_T *pFrameALVC =  (FRAMEAL_VC_T *) LinkHdl->VC_Info[cnt].FrameALVCMod;
			if ( pFrameALVC )
			{
				// clear the AAL stats
				pFrameALVC->AALVcCtx.AALTxGoodByte.Cnt			= 0;
				pFrameALVC->AALVcCtx.AALTxGood.Cnt				= 0;
				pFrameALVC->AALVcCtx.AALTxDiscarded.Cnt			= 0;
				pFrameALVC->AALVcCtx.AALTxDiscardedByte.Cnt		= 0;
				pFrameALVC->AALVcCtx.AALRxGoodByte.Cnt			= 0;
				pFrameALVC->AALVcCtx.AALRxGood.Cnt				= 0;
				pFrameALVC->AALVcCtx.AALRxDiscardedByte.Cnt		= 0;
				pFrameALVC->AALVcCtx.AALRxDiscarded.Cnt			= 0;
				pFrameALVC->AALVcCtx.AALRxInvalidCRC.Cnt		= 0;
				pFrameALVC->AALVcCtx.AALRxInvalidLen.Cnt		= 0;
				pFrameALVC->AALVcCtx.AALRxReassemblyTimeout.Cnt	= 0;
				// clear the atm stats
				pFrameALVC->Stats.TXBytes.Cnt = 0;
				pFrameALVC->Stats.TXATMCellsCLP0.Cnt=0;
				pFrameALVC->Stats.TXATMCellsCLP1.Cnt=0;
				pFrameALVC->Stats.TXATMCellsMgmCell.Cnt=0;
				pFrameALVC->Stats.TXATMCellsCLP0.Cnt=0;
				pFrameALVC->Stats.TXATMCellsCLP1.Cnt=0;
				pFrameALVC->Stats.RXBytes.Cnt=0;
				pFrameALVC->Stats.RXATMCellsGoodCLP0.Cnt=0;
				pFrameALVC->Stats.RXATMCellsGoodCLP1.Cnt=0;
				pFrameALVC->Stats.RXATMCellsMgmCell.Cnt=0;
				pFrameALVC->Stats.RXATMCellsGoodCLP0.Cnt=0;
				pFrameALVC->Stats.RXATMCellsGoodCLP1.Cnt=0;
			}
		}
	}

}


/*******************************************************************************
FUNCTION NAME:
	FrameALLinkUp

ABSTRACT:
	This function is called when the line enters showtime

RETURNS:
	None

DETAILS:
*******************************************************************************/

GLOBAL void FrameALLinkUp
(
    IN	CDSL_LINK_T		*LinkHdl		// Link Handle to data stream
)
{

	FrameALATMLinkUp(LinkHdl);

}


/*******************************************************************************
FUNCTION NAME:
	FrameALLinkDown

ABSTRACT:
	This function is called when the line enters showtime

RETURNS:
	None

DETAILS:
*******************************************************************************/

GLOBAL void FrameALLinkDown
(
    IN	CDSL_LINK_T		*LinkHdl		// Link Handle to data stream
)
{

	FrameALATMLinkDown(LinkHdl);

}
