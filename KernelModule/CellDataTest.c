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
**		????
**
**	FILE NAME:
**		CellDataTest.c
**
**	ABSTRACT:
**		
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Header: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux 2.4 ATM Driver/KernelModule/CellDataTest.c 3     3/01/01 4:0 $
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux 2.4 ATM Driver/KernelModule/CellDataTest.c $
**	$Revision: 3 $
**	$Date: 3/01/01 4:00p $
*******************************************************************************
******************************************************************************/

#include "CardMgmt.h"
#include "CardMgmtLinkV.h"
#include "CellDataTest.h"

// ***************************************
//		DEBUG ITEMS
// ***************************************

#define ERROR_FATAL(error)		(LEVEL_FATAL   | MODULE_BUFFMGMT | INSTANCE_CELLDATATEST | error)
#define ERROR_WARNING(error)	(LEVEL_WARNING | MODULE_BUFFMGMT | INSTANCE_CELLDATATEST | error)
#define ERROR_NORMAL(error)		(LEVEL_NORMAL  | MODULE_BUFFMGMT | INSTANCE_CELLDATATEST | error)


#ifndef MEMORY_TAG
	#define	MEMORY_TAG	'TDC' 
#endif

#ifndef FUNCTION_ENTRY
	#define FUNCTION_ENTRY
#endif

#define PRBS_2_23_1_POLYNOMIAL  0x00420000UL    /* x^23 + x^18 + 1 */
#define PBRS_2_20_1_POLYNOMIAL	0x00090000UL	/* x^20+ x^17 + 1 */
#define PRBS_2_15_1_POLYNOMIAL  0x00006000UL    /* x^15 + x^14 + 1 */
#define ATM_BER_SYNC_THRESHOLD	20				/* number of errors in a block to cause a resync */

// Control the data in the data area of the test cell for Cell Verification test
#define INITIAL_TEST_PATTERN_BYTE 	0x10
#define TEST_PATTERN_UPDATE 		0x1
#define INITIAL_CELL_LOAD_COUNT		5

/* table of weights for 8-bit bytes */
static unsigned char Weight[256];
static PRBS_ARRAY PRBSTable;

LOCAL NDIS_STATUS CellDataTestNewCellForVerify
(
    IN	CDSL_VC_T		*VCHdl,			// VC Handle to data stream
    IN	SK_BUFF_T		*BufferDescPtr	// Pointer to a buffer descriptor
);

LOCAL NDIS_STATUS CellDataTestATMBERVCInit
(
    IN	CDSL_VC_T				*VCHdl,			// VC Handle to data stream
    IN  CDSL_LINK_T				*LinkHdl,
    IN 	BD_TESTMOD_ATM_BER_T	*Parameters
);

LOCAL NDIS_STATUS CellDataTestATMBerRx
(
    IN	CDSL_VC_T		*VCHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*BufferDescPtr	// Pointer to a buffer descriptor
);

LOCAL NDIS_STATUS CellDataTestNewCellForATMBER
(
    IN	CDSL_VC_T		*VCHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*BufferDescPtr	// Pointer to a buffer descriptor
);
/*******************************************************************************
FUNCTION NAME:
	 ParityCheck

ABSTRACT:	This function counts the mismatches between two 32bit word 

RETURNS:
	ParityCheck value 

DETAILS:
*******************************************************************************/
LOCAL unsigned long	ParityCheck (unsigned long PrbsAccum)
{
	unsigned long Mask=1UL, Parity=0UL;
	int i;
	for (i = 0;  i < 32;  ++i) {
		Parity ^= ((PrbsAccum & Mask) != 0UL);
		Mask <<= 1;
	}
	return Parity;
}

/*******************************************************************************
FUNCTION NAME:
	 UpdatePrbs

ABSTRACT:	This function updated the local copy of the prbs based 
            on the old prbs. 

RETURNS:
	ParityCheck value 

DETAILS:
*******************************************************************************/

LOCAL unsigned long	 UpdatePrbs (unsigned long prbs_accum)
{
	//PRBS_ARRAY PRBSTable;  // pointer to array

	unsigned char acc_lsb = 0;
	int i;

	//PRBSTable =  pPRBSTable;
	for (i = 0;  i < 4;  ++i ) {
		acc_lsb ^= PRBSTable [i] [ (prbs_accum >> (i * 8) ) & 0xff ];
	}
	return (prbs_accum << 8) ^ ((unsigned long)acc_lsb);
}


/*******************************************************************************
FUNCTION NAME:
	CellDataTestAdapterInit

ABSTRACT:	
	Initializes a new Adapter instance. 
	
RETURN:
	NDIS Status
	 
DETAILS:
	CellDataTest does not perform any initialization at adapter init time.
*******************************************************************************/

GLOBAL NDIS_STATUS CellDataTestAdapterInit
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
	CellDataTestAdapterShutdown

ABSTRACT:
	Shutdown a Adapter instance.  

RETURN:
	NDIS status

DETAILS:
	CellDataTest does not perform any initialization at adapter init time.

*******************************************************************************/

GLOBAL NDIS_STATUS CellDataTestAdapterShutdown
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
	CellDataTestLinkInit

ABSTRACT:	
	Initializes a new link instance.  

RETURN:
	NDIS status

DETAILS:
	This function allocates memory for the CellDataTest link context.
*******************************************************************************/

GLOBAL NDIS_STATUS CellDataTestLinkInit
(
    IN	CDSL_LINK_T	*LinkHdl,						// Link Handle to data stream
    IN  CELL_DATA_TEST_LINK_PARAMETERS * LinkParameters	//
)
{
	CELL_DATA_TEST_LINK_T  *pCellDataTestLinkT;
	NDIS_STATUS		Status;

	// Allocate NDIS block of memory
	Status = ALLOCATE_MEMORY ( (PVOID)&pCellDataTestLinkT, sizeof( * pCellDataTestLinkT), MEMORY_TAG );

	// if NDIS does not provide memory
	// This module will not function.
	if (Status != STATUS_SUCCESS)
	{
		return Status;
	}

	// clear CellDataTest Link structure
	CLEAR_MEMORY
	(
	    (PVOID *)pCellDataTestLinkT,
	    sizeof( * pCellDataTestLinkT)
	);

	// save Frame link structure pointer in link structure
	LinkHdl->CellDataTestMod = pCellDataTestLinkT;

	// save parameter block in CellDataTest link Stucture
	COPY_MEMORY
	(
	    (void *) &pCellDataTestLinkT->Parameters,
	    (void *) LinkParameters,
	    sizeof(CELL_DATA_TEST_LINK_PARAMETERS)
	);

	if (Status != STATUS_SUCCESS)
	{
		return Status;
	}

	return	STATUS_SUCCESS;
}



/*******************************************************************************
FUNCTION NAME:
	CellDataTestLinkShutdown

ABSTRACT:	
	Shutdowns a link instance.  

RETURN:
	NDIS Status

DETAILS:
	This function releases any buffers (TX or RX) allocated to CellDataTest by
	buffer management. After the buffers are released, the memory for 
	the CellDataTest link context is released..
*******************************************************************************/

GLOBAL NDIS_STATUS CellDataTestLinkShutdown
(
    IN	CDSL_LINK_T	*LinkHdl	// Link Handle to data stream
)
{
	CELL_DATA_TEST_LINK_T  *pCellDataTestLinkT = (CELL_DATA_TEST_LINK_T*)LinkHdl->CellDataTestMod;

	if (pCellDataTestLinkT  == NULL)
	{
		return STATUS_SUCCESS;
	}

	// release NDIS block of memory
	FREE_MEMORY
	(
	    (PVOID) LinkHdl->CellDataTestMod,
	    sizeof( CELL_DATA_TEST_LINK_T ),
	    0
	);

	LinkHdl->CellDataTestMod = NULL;

	return STATUS_SUCCESS;
}


/*******************************************************************************
FUNCTION NAME:
	CellDataTestVCInit

ABSTRACT:	
	Initializes a new VC instance.  

RETURN:
	NDIS status

DETAILS:
	This function allocates memory for the CellDataTest VC context.
*******************************************************************************/
GLOBAL NDIS_STATUS CellDataTestVCInit
(
    IN	CDSL_VC_T	*VCHdl,						// Link Handle to data stream
    IN  CELL_DATA_TEST_VC_PARAMETERS * VCParameters	//
)
{
	CELL_DATA_TEST_VC_T  		*pCellDataTestVC;
	NDIS_STATUS			Status;
	SK_BUFF_T	BufDescPtr;
	DWORD	Index;

	// Allocate NDIS block of memory
	Status = ALLOCATE_MEMORY( (PVOID)&pCellDataTestVC, sizeof( CELL_DATA_TEST_VC_T), MEMORY_TAG );

	// if NDIS does not provide memory
	// This module will not function.
	if (Status != STATUS_SUCCESS)
	{
		return Status;
	}

	// clear CellDataTest Link structure
	CLEAR_MEMORY
	(
	    (PVOID *)pCellDataTestVC,
	    sizeof( * pCellDataTestVC)
	);

	// save Frame link structure pointer in link structure
	VCHdl->CellDataTestMod = pCellDataTestVC;

	// save parameter block in CellDataTest link Stucture
	COPY_MEMORY
	(
	    (void *) &pCellDataTestVC->Parameters,
	    (void *) VCParameters,
	    sizeof(CELL_DATA_TEST_VC_PARAMETERS)
	);

	Status = CellDataTestMgmtVCInit(VCHdl);
	if (Status != STATUS_SUCCESS)
	{
		return Status;
	}

	if ( pCellDataTestVC->Parameters.VCMode == VC_MODE_CELL_VERIFICATION_TEST)
	{
		pCellDataTestVC->TxSeqNo = 0;
		pCellDataTestVC->RxSeqNo = 0;
		pCellDataTestVC->FirstRxCell = FALSE;

		{
			WORD LoopCounter;
			TEST_PAYLOAD1_T * pTestPayload = (TEST_PAYLOAD1_T*)&pCellDataTestVC->TestPayload.Data[0];

			for
			(
			    LoopCounter = 0;
			    LoopCounter < sizeof(pTestPayload->Data);
			    LoopCounter++

			)
			{
				// file data section of cell with a pattern
				pTestPayload->Data[LoopCounter] = INITIAL_TEST_PATTERN_BYTE + TEST_PATTERN_UPDATE* LoopCounter;
			}
		}

		// fill CellDataTestMgmt TX queue
		for (
		    Index = 0;
		    Index < INITIAL_CELL_LOAD_COUNT;
		    Index++
		)
		{
			CellDataTestNewCellForVerify( VCHdl, &BufDescPtr);
		}

		// start TX on chipal.
		FrameALVCStartTX( VCHdl);
	}

	return STATUS_SUCCESS;
}

/*******************************************************************************
FUNCTION NAME:
	CellDataTestATMBERVCInit

ABSTRACT:	
	Initializes a new VC instance for ATM BER.  

RETURN:
	NDIS status

DETAILS:
	This function allocates memory for the CellDataTest VC context.
*******************************************************************************/
LOCAL NDIS_STATUS CellDataTestATMBERVCInit
(
    IN	CDSL_VC_T				*VCHdl,			// VC Handle to data stream
    IN  CDSL_LINK_T				*LinkHdl,
    IN 	BD_TESTMOD_ATM_BER_T	* Parameters
)
{
	CELL_DATA_TEST_VC_T  		*pCellDataTestVC;
	NDIS_STATUS			Status;
	SK_BUFF_T	BufDescPtr;
	DWORD	Index, RowIndex, BitIndex;
	FRAMEAL_VC_PARAMETERS    	DefaultVCParameters;

	// check parameters for valid BER pattern
	switch (Parameters->Pattern)
	{
	case	BD_TESTMOD_15BIT:
	case 	BD_TESTMOD_20BIT:
	case	BD_TESTMOD_23BIT:
		break;
	case	BD_TESTMOD_USER_SUPPLIED:
		if ( Parameters->UserPatternMask == 0)
		{
			// zero user pattern is not valid
			return 	STATUS_FAILURE;
		}
		break;

	case	BD_TESTMOD_NO_PATTERN:
	default:
		return 	STATUS_FAILURE;
	}

	if (Parameters->Vci == 0)
	{
		return 	STATUS_FAILURE;
	}

	VCHdl->Allocated = TRUE;
	VCHdl->LinkHdl = LinkHdl;

	DefaultVCParameters.Vpi = Parameters->Vpi;
	DefaultVCParameters.Vci = Parameters->Vci;
	DefaultVCParameters.PeakCellRate = (ULONG) Parameters->Pcr;
	DefaultVCParameters.AALType = ATM_AAL5;
	DefaultVCParameters.VcSAP = VC_SAP_CELLDATATEST;
	DefaultVCParameters.VCMode = VC_MODE_NORMAL;

	// assume no vcs operating yet
	FrameALVCInit( VCHdl, &DefaultVCParameters);

	// Allocate NDIS block of memory
	Status = ALLOCATE_MEMORY( (PVOID)&pCellDataTestVC, sizeof( CELL_DATA_TEST_VC_T), MEMORY_TAG );

	// if NDIS does not provide memory
	// This module will not function.
	if (Status != STATUS_SUCCESS)
	{
		return Status;
	}

	// clear CellDataTest Link structure
	CLEAR_MEMORY
	(
	    (PVOID *)pCellDataTestVC,
	    sizeof( * pCellDataTestVC)
	);

	switch (Parameters->Pattern)
	{
	case	BD_TESTMOD_15BIT:
		pCellDataTestVC->Polynomial = PRBS_2_15_1_POLYNOMIAL;
		break;

	case 	BD_TESTMOD_20BIT:
		pCellDataTestVC->Polynomial	= PBRS_2_20_1_POLYNOMIAL;
		break;

	case	BD_TESTMOD_23BIT:
		pCellDataTestVC->Polynomial	= PRBS_2_23_1_POLYNOMIAL;
		break;

	case	BD_TESTMOD_USER_SUPPLIED:
		if ( Parameters->UserPatternMask != 0)
		{
			pCellDataTestVC->Polynomial	= Parameters->UserPatternMask;
		}
		else
		{
			// zero user pattern is not valid
			return 	STATUS_FAILURE;
		}
		break;

	case	BD_TESTMOD_NO_PATTERN:
	default:
		return 	STATUS_FAILURE;
	}

	// save Frame link structure pointer in link structure
	VCHdl->CellDataTestMod = pCellDataTestVC;

	// save parameter block in CellDataTest link Stucture
	COPY_MEMORY
	(
	    (void *) &pCellDataTestVC->ATMBERParams,
	    (void *) Parameters,
	    sizeof(BD_TESTMOD_ATM_BER_T)
	);

	Status = CellDataTestMgmtVCInit(VCHdl);
	if (Status != STATUS_SUCCESS)
	{
		return Status;
	}

	// mark VC as ATM BER
	pCellDataTestVC->Parameters.VCMode	 = VC_MODE_ATM_BER;

	// generate  table for 32 PRBS
	for (Index = 0;  Index < 4;  ++Index)
	{
		for (RowIndex = 0;  RowIndex < 256;  ++RowIndex)
		{
			unsigned long prbs_accum = ((unsigned long)RowIndex << (Index * 8));
			for (BitIndex = 0;  BitIndex < 8;  ++BitIndex)
			{
				prbs_accum = (prbs_accum << 1)
				             ^ ParityCheck(prbs_accum & pCellDataTestVC->Polynomial);
			}
			PRBSTable[Index][RowIndex] = (prbs_accum & 0xff);
		}
	}

	for (Index = 0;  Index < 256;  ++Index)
	{
		unsigned char mask=1U, ones_count = 0U;
		for (BitIndex = 0;  BitIndex < 8;  ++BitIndex)
		{
			ones_count += ((Index & mask) != 0U);
			mask = mask << 1;
		}
		Weight[Index] = ones_count;
	}

	pCellDataTestVC->ATMBERInsync = FALSE;

	if (Parameters->RXInvert == TRUE)
	{
		pCellDataTestVC->RXPolarityMask = 0xff;
	}

	if (Parameters->TXInvert == TRUE)
	{
		pCellDataTestVC->TXPolarityMask = 0xff;
	}

	// seed tx with something
	pCellDataTestVC->TXPRBSAccum = 0xebff0001;

	// if TX enabled fill channel with 5 cells of data
	if ( pCellDataTestVC->ATMBERParams.Status == BD_TESTMOD_ATM_BER_ACTIVE)
	{
		// fill CellDataTestMgmt TX queue
		for (
		    Index = 0;
		    Index < INITIAL_CELL_LOAD_COUNT;
		    Index++
		)
		{

			CellDataTestNewCellForATMBER( VCHdl, &BufDescPtr);
		}

		// start TX on chipal.
		FrameALVCStartTX( VCHdl);

	}

	return STATUS_SUCCESS;
}


/*******************************************************************************
FUNCTION NAME:
	CellDataTestATMBERVCShutdown

ABSTRACT:	
	Shutdown a VC instance for ATM BER.  

RETURN:
	NDIS status

DETAILS:
	This function frees memory for the CellDataTest VC context.
*******************************************************************************/
LOCAL NDIS_STATUS CellDataTestATMBERVCShutdown
(
    IN	CDSL_VC_T	*VCHdl	// Link Handle to data stream
)
{
	CELL_DATA_TEST_VC_T  *pCellDataTestVC = (CELL_DATA_TEST_VC_T*)VCHdl->CellDataTestMod;
	NDIS_STATUS Status;

	if (pCellDataTestVC == NULL)
	{

		return STATUS_SUCCESS;
	}

	Status = CellDataTestMgmtVCShutdown(VCHdl);

	// release NDIS block of memory
	FREE_MEMORY
	(
	    (PVOID) VCHdl->CellDataTestMod,
	    sizeof( CELL_DATA_TEST_VC_T ),
	    0
	);

	//y	DBG_CDSL_DISPLAY(
	//y		DBG_LVL_MISC_INFO,
	//y		LinkHdl->pThisAdapter,
	//y		("CellDataTest: Exit link shutdown") );

	VCHdl->CellDataTestMod = NULL;

	return Status;
}

/*******************************************************************************
FUNCTION NAME:
	CellDataTestVCShutdown

ABSTRACT:	
	Shutdowns a VC instance.  

RETURN:
	NDIS Status

DETAILS:
   	First the sublayers are notified of the VC shutdown. Then CellDataTest 
   	releases the memory for	the CellDataTest link context is released.
*******************************************************************************/

GLOBAL NDIS_STATUS CellDataTestVCShutdown
(
    IN	CDSL_VC_T	*VCHdl	// Link Handle to data stream
)
{
	CELL_DATA_TEST_VC_T  *pCellDataTestVC = (CELL_DATA_TEST_VC_T*)VCHdl->CellDataTestMod;
	NDIS_STATUS Status;

	if (pCellDataTestVC == NULL)
	{

		return STATUS_SUCCESS;
	}

	Status = CellDataTestMgmtVCShutdown(VCHdl);

	// release NDIS block of memory
	FREE_MEMORY
	(
	    (PVOID) VCHdl->CellDataTestMod,
	    sizeof( CELL_DATA_TEST_VC_T ),
	    0
	);

	//y	DBG_CDSL_DISPLAY(
	//y		DBG_LVL_MISC_INFO,
	//y		LinkHdl->pThisAdapter,
	//y		("CellDataTest: Exit link shutdown") );

	VCHdl->CellDataTestMod = NULL;

	return Status;
}


//***********************************************************************************
GLOBAL NDIS_STATUS CellDataTestCfgInit
(
    IN	CDSL_ADAPTER_T	*AdprCtx,			// Driver supplied adapter context
    // (&ThisAdapter)  Required for events that
    // are not associated with a link
    IN PTIG_USER_PARAMS  pUserParams
)
{
	FUNCTION_ENTRY
	NOT_USED( *AdprCtx );
	NOT_USED( pUserParams );
	return( STATUS_SUCCESS );
};



/******************************************************************************
FUNCTION NAME:
	CellDataTestDeviceSpecific

ABSTRACT:
	Process Device Specific OIDs

RETURN:
	Ndis Status
 
DETAILS:
	Only OID is for frame AL statistics. 
******************************************************************************/
NDIS_STATUS CellDataTestDeviceSpecific(
    CDSL_ADAPTER_T				* pThisAdapter,
    BACK_DOOR_T					* pBackDoorBuf,
    PULONG						pNumBytesNeeded,
    PULONG						pNumBytesWritten )

{
	CDSL_LINK_T		* LinkHdl;
	NDIS_STATUS       Status=STATUS_SUCCESS;

	CELL_DATA_TEST_VC_T		*pCellDataTestVC;
	FRAMEAL_VC_PARAMETERS    	DefaultVCParameters;
	CELL_DATA_TEST_VC_PARAMETERS		CellDataTestVcParameters;
	CELL_DATA_TEST_LINK_PARAMETERS 		CellDataTestLinkParamters;

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
			*pNumBytesNeeded = pBackDoorBuf->NeededSize;
			return ERROR_WARNING( RESULT_REQ_BUFF_TOO_SMALL );
		}
	}

	// only process request codes for CellDataTest
	switch (pBackDoorBuf->ReqCode)
	{
	case 	 BD_TEST_MOD_START_DIGITAL_LOOPBACK:
		// *********************************************************************
		//	set the BACKDOOR Status to SUCCESS.
		pBackDoorBuf->ResultCode = RESULT_SUCCESS;

		// get the first link assume no links up yet
		LinkHdl = GET_LINK_FROM_ADAPTER(pThisAdapter);

		//x			DefaultLinkParameters.TxCellsPerSecond = 1000;
		//x			DefaultLinkParameters.RxCellsPerSecond = 1000;
		//x
		//x			DefaultLinkParameters.LinkType = LINK_TYPE_UBR_ONLY;
		//x
		//x			Status = FrameALLinkInit( LinkHdl, &DefaultLinkParameters);
		//x			if ( Status == STATUS_SUCCESS )
		{
			LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK].Allocated = TRUE;
			LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK].LinkHdl = LinkHdl;

			DefaultVCParameters.Vpi = pBackDoorBuf->Params.BdTestModDigitalLoopBack.TestDigLbVpi;
			DefaultVCParameters.Vci = pBackDoorBuf->Params.BdTestModDigitalLoopBack.TestDigLbVci;

			DefaultVCParameters.PeakCellRate =
			    (ULONG) pBackDoorBuf->Params.BdTestModDigitalLoopBack.TestDigLbPcr;

			DefaultVCParameters.AALType = ATM_AAL5;
			DefaultVCParameters.VcSAP = VC_SAP_CELLDATATEST;
			DefaultVCParameters.VCMode = VC_MODE_NORMAL; //VC_MODE_INSERT_IDLE;

			// assume no vcs operating yet
			FrameALVCInit( &LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK], &DefaultVCParameters);

			CellDataTestLinkInit( LinkHdl ,  &CellDataTestLinkParamters);
			CellDataTestVcParameters.VCMode 			= VC_MODE_DIGITAL_LOOPBACK;
			CellDataTestVcParameters.MaxCellsInQueue = 20;
			CellDataTestVCInit( &LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK], &CellDataTestVcParameters);

			//x						// Enable Transmit and Receive Channels
			//x				Status = ChipALLinkInit (
			//x					pThisAdapter,		// Adapter Instance
			//x					LinkHdl,            // Arbitrary Link
			//x					0,					// Link Speed in BPS
			//x					ADSL_CHANNEL,	    // Communications Channel
			//x					0);	            // Framed or NonFramed
			//x
			//x
			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
			*pNumBytesNeeded = pBackDoorBuf->TotalSize;
			*pNumBytesWritten = sizeof( pBackDoorBuf->ResultCode )
			                    + sizeof( pBackDoorBuf->NeededSize )
			                    + sizeof( pBackDoorBuf->ReqCode) ;
		}
		break;

	case 	 BD_TEST_MOD_STOP_DIGITAL_LOOPBACK:

		//	set the BACKDOOR Status to SUCCESS.
		pBackDoorBuf->ResultCode = RESULT_SUCCESS;

		// get the first link assume no links up yet
		LinkHdl = GET_LINK_FROM_ADAPTER(pThisAdapter );
		FrameALVCShutdown(&LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK]);
		CellDataTestVCShutdown(&LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK]);

		LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK].Allocated = FALSE;

		CellDataTestLinkShutdown( LinkHdl);
		//x			FrameALLinkShutdown( LinkHdl);
		//x
		//x			ChipALLinkShutdown( LinkHdl );

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten = sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize )
		                    + sizeof( pBackDoorBuf->ReqCode) ;

		break;

	case 	 BD_TEST_MOD_START_CELL_VERIFY:

		// *********************************************************************
		//	set the BACKDOOR Status to SUCCESS.
		pBackDoorBuf->ResultCode = RESULT_SUCCESS;

		// get the first link assume no links up yet
		LinkHdl = GET_LINK_FROM_ADAPTER(pThisAdapter );

		//x			DefaultLinkParameters.TxCellsPerSecond = 1000;
		//x			DefaultLinkParameters.RxCellsPerSecond = 1000;
		//x
		//x			DefaultLinkParameters.LinkType = LINK_TYPE_UBR_ONLY;
		//x
		//x			Status = FrameALLinkInit( LinkHdl, &DefaultLinkParameters);
		//x			if ( Status == STATUS_SUCCESS )
		{
			LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK].Allocated = TRUE;
			LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK].LinkHdl = LinkHdl;

			DefaultVCParameters.Vpi = pBackDoorBuf->Params.BdTestModCellVerify.Vpi;
			DefaultVCParameters.Vci = pBackDoorBuf->Params.BdTestModCellVerify.Vci;

			DefaultVCParameters.PeakCellRate =
			    (ULONG) pBackDoorBuf->Params.BdTestModCellVerify.Pcr;

			DefaultVCParameters.AALType = ATM_AAL5;

			DefaultVCParameters.VcSAP = VC_SAP_CELLDATATEST;

			DefaultVCParameters.VCMode = VC_MODE_NORMAL; //VC_MODE_INSERT_IDLE;

			// assume no vcs operating yet
			FrameALVCInit( &LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK], &DefaultVCParameters);


			CellDataTestLinkInit( LinkHdl ,  &CellDataTestLinkParamters);

			CellDataTestVcParameters.VCMode 			= VC_MODE_CELL_VERIFICATION_TEST;
			CellDataTestVcParameters.MaxCellsInQueue = 20;
			CellDataTestVCInit( &LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK], &CellDataTestVcParameters);


			//x						// Enable Transmit and Receive Channels
			//x				Status = ChipALLinkInit (
			//x					pThisAdapter,		// Adapter Instance
			//x					LinkHdl,            // Arbitrary Link
			//x					0,					// Link Speed in BPS
			//x					ADSL_CHANNEL,	    // Communications Channel
			//x					0);	            // Framed or NonFramed
			//x
			//x
			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
			*pNumBytesNeeded = pBackDoorBuf->TotalSize;
			*pNumBytesWritten = sizeof( pBackDoorBuf->ResultCode )
			                    + sizeof( pBackDoorBuf->NeededSize )
			                    + sizeof( pBackDoorBuf->ReqCode) ;

		}
		break;

	case 	 BD_TEST_MOD_STOP_CELL_VERIFY:

		//	set the BACKDOOR Status to SUCCESS.
		pBackDoorBuf->ResultCode = RESULT_SUCCESS;

		// get the first link assume no links up yet
		LinkHdl = GET_LINK_FROM_ADAPTER(pThisAdapter );
		FrameALVCShutdown(&LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK]);
		CellDataTestVCShutdown(&LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK]);

		LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK].Allocated = FALSE;

		CellDataTestLinkShutdown( LinkHdl);
		//x			FrameALLinkShutdown( LinkHdl);
		//x
		//x			ChipALLinkShutdown( LinkHdl );

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten = sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize )
		                    + sizeof( pBackDoorBuf->ReqCode) ;

		break;

	case BD_TEST_MOD_GET_CELL_VERIFY_STATS:

		pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_DATA_UNAVAILABLE );

		// only will work for one link
		LinkHdl = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if (LinkHdl != NULL)
		{
			pCellDataTestVC = (CELL_DATA_TEST_VC_T*)LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK].CellDataTestMod;
			if (pCellDataTestVC != NULL)
			{
				pBackDoorBuf->Params.BdTestModVerifyStats.NumTestFramesTxd =
				    (pCellDataTestVC->Stats.TXATMCellsCLP0.Cnt + pCellDataTestVC->Stats.TXATMCellsCLP1.Cnt);

				pBackDoorBuf->Params.BdTestModVerifyStats.NumGoodTestFramesRxd =
				    (pCellDataTestVC->Stats.RXATMCellsGoodCLP0.Cnt + pCellDataTestVC->Stats.RXATMCellsGoodCLP1.Cnt);

				pBackDoorBuf->Params.BdTestModVerifyStats.NumOutOfOrderTestFramesRxd =
				    (pCellDataTestVC->Stats.RXATMCellsDataErrorsCLP0.Cnt + pCellDataTestVC->Stats.RXATMCellsDataErrorsCLP1.Cnt) +
				    (pCellDataTestVC->Stats.RXATMCellsSeqErrorsCLP0.Cnt + pCellDataTestVC->Stats.RXATMCellsSeqErrorsCLP1.Cnt) ;
				pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			}
		}

		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdTestModVerifyStats )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );

		break;

	case 	BD_TEST_MOD_START_ATM_BER:
		//	set the BACKDOOR Status to SUCCESS.
		pBackDoorBuf->ResultCode = RESULT_SUCCESS;

		// get the first link assume no links up yet
		LinkHdl = GET_LINK_FROM_ADAPTER(pThisAdapter );

		CellDataTestLinkInit( LinkHdl ,  &CellDataTestLinkParamters);

		Status = CellDataTestATMBERVCInit
		         (
		             &LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK],
		             LinkHdl,
		             &pBackDoorBuf->Params.BdTestModATMBER
		         );

		// get the first link assume no links up yet
		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten = sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize )
		                    + sizeof( pBackDoorBuf->ReqCode) ;
		break;

	case	BD_TEST_MOD_STOP_ATM_BER:

		//	set the BACKDOOR Status to SUCCESS.
		pBackDoorBuf->ResultCode = RESULT_SUCCESS;

		// get the first link assume no links up yet
		LinkHdl = GET_LINK_FROM_ADAPTER(pThisAdapter );
		FrameALVCShutdown(&LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK]);

		CellDataTestATMBERVCShutdown(&LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK]);

		LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK].Allocated = FALSE;

		CellDataTestLinkShutdown( LinkHdl);

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten = sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize )
		                    + sizeof( pBackDoorBuf->ReqCode) ;
		break;

	case	BD_TEST_MOD_ATM_BER_STATUS:

		pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_DATA_UNAVAILABLE );

		// only will work for one link
		LinkHdl = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if (LinkHdl != NULL)
		{
			pCellDataTestVC = (CELL_DATA_TEST_VC_T*)LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK].CellDataTestMod;
			if (pCellDataTestVC != NULL)
			{
				COPY_MEMORY
				(
				    &pBackDoorBuf->Params.BdTestModATMBER,
				    &pCellDataTestVC->ATMBERParams,
				    sizeof(BD_TESTMOD_ATM_BER_T)
				);

				pBackDoorBuf->Params.BdTestModATMBER.ErrInsertion =
				    pCellDataTestVC->ATMBERInsertErrors;

				pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			}
		}

		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdTestModATMBER )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case	BD_TEST_MOD_ATM_BER_STATS:

		pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_DATA_UNAVAILABLE );

		// only will work for one link
		LinkHdl = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if (LinkHdl != NULL)
		{
			pCellDataTestVC = (CELL_DATA_TEST_VC_T*)LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK].CellDataTestMod;
			if (pCellDataTestVC != NULL)
			{
				pBackDoorBuf->Params.BdTestModATMBERSTAT.RXBits =
				    ((DWORD)pCellDataTestVC->Stats.ATMBERRXBits.Cnt / 10000);

				pBackDoorBuf->Params.BdTestModATMBERSTAT.TXBits =
				    ((DWORD)pCellDataTestVC->Stats.ATMBERTXBits.Cnt / 10000);

				pBackDoorBuf->Params.BdTestModATMBERSTAT.RXErrorBits =
				    (DWORD) pCellDataTestVC->Stats.ATMBERRXErrors.Cnt ;

				pBackDoorBuf->Params.BdTestModATMBERSTAT.RXResyncs =
				    (DWORD) pCellDataTestVC->Stats.ATMBERRXResyncs.Cnt ;

				pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			}
		}

		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdTestModATMBERSTAT )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case	BD_TEST_MOD_ATM_BER_STATS_64BITS:

		pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_DATA_UNAVAILABLE );

		// only will work for one link
		LinkHdl = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if (LinkHdl != NULL)
		{
			pCellDataTestVC = (CELL_DATA_TEST_VC_T*)LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK].CellDataTestMod;
			if (pCellDataTestVC != NULL)
			{
				BD_TESTMOD_ATM_BER_STATS64_T *pBdTestModATMBERSTAT64;
				pBdTestModATMBERSTAT64 =  (BD_TESTMOD_ATM_BER_STATS64_T *)&pBackDoorBuf->Params;
				pBdTestModATMBERSTAT64->RXBits = 	pCellDataTestVC->Stats.ATMBERRXBits.Cnt;
				pBdTestModATMBERSTAT64->TXBits = 	pCellDataTestVC->Stats.ATMBERTXBits.Cnt;
				pBdTestModATMBERSTAT64->RXErrorBits =pCellDataTestVC->Stats.ATMBERRXErrors.Cnt ;
				pBdTestModATMBERSTAT64->RXResyncs = 	pCellDataTestVC->Stats.ATMBERRXResyncs.Cnt ;
				pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			}
		}

		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( BD_TESTMOD_ATM_BER_STATS64_T )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case	BD_TEST_MOD_ATM_BER_STATS_CLEAR:
		pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_DATA_UNAVAILABLE );

		// only will work for one link
		LinkHdl = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if (LinkHdl != NULL)
		{
			pCellDataTestVC = (CELL_DATA_TEST_VC_T*)LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK].CellDataTestMod;
			if (pCellDataTestVC != NULL)
			{
				pCellDataTestVC->Stats.ATMBERRXBits.Cnt  	= 0;
				pCellDataTestVC->Stats.ATMBERTXBits.Cnt  	= 0;
				pCellDataTestVC->Stats.ATMBERRXErrors.Cnt	= 0;
				pCellDataTestVC->Stats.ATMBERRXResyncs.Cnt	= 0;
				pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			}
		}

		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	 sizeof( pBackDoorBuf->ResultCode )
		                     + sizeof( pBackDoorBuf->NeededSize )
		                     + sizeof( pBackDoorBuf->ReqCode) ;
		break;

	case	BD_TEST_MOD_ATM_BER_INJECT_ERRORS:
		pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_DATA_UNAVAILABLE );

		// only will work for one link
		LinkHdl = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if (LinkHdl != NULL)
		{
			pCellDataTestVC = (CELL_DATA_TEST_VC_T*)LinkHdl->VC_Info[TAPI_ADDRESSID_LOOPBACK].CellDataTestMod;
			if (pCellDataTestVC != NULL)
			{

				pCellDataTestVC->ATMBERInsertErrors =
				    pBackDoorBuf->Params.BdTestModATMBERInject;

				pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			}
		}

		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	 sizeof( pBackDoorBuf->ResultCode )
		                     + sizeof( pBackDoorBuf->NeededSize )
		                     + sizeof( pBackDoorBuf->ReqCode) ;
		break;

	default:
		Status = STATUS_UNSUPPORTED;
		break;

	}  // end switch

	return Status;

}	// end function

/******************************************************************************
FUNCTION NAME:
	CellDataTestSetInformationHdlr

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
NDIS_STATUS CellDataTestSetInformationHdlr(
    IN CDSL_ADAPTER_T	* pThisAdapter,
    //    IN NDIS_OID			Oid,
    IN PVOID			InfoBuffer,
    IN ULONG			InfoBufferLength,
    OUT PULONG			pBytesRead,
    OUT PULONG			pBytesNeeded )
{
	NDIS_STATUS				Status=STATUS_FAILURE;
	/* Not Supported on Linux
		ULONG					NumBytesNeeded = 0;
		ULONG					NumBytesRead = 0;

		//	DBG_CDSL_DISPLAY(
		//		DBG_LVL_WARNING,
		//		pThisAdapter,
		//		(	"    Function Entry 'CellDataTestSetInformationHdlr':   OID=0x%08lX.\n", Oid) );
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
			//				(	"    Function 'CellDataTestSetInformationHdlr':   Unknown OID  -  0x%08lX.\n", Oid) );
			Status = NDIS_STATUS_INVALID_OID;
			break;
		}

		//	Fill in the size fields before we leave.
		*pBytesNeeded += NumBytesNeeded;
		*pBytesRead += NumBytesRead;
	*/
	return (Status);
}


/*******************************************************************************
FUNCTION NAME:
	CellDataTestTxComplete

ABSTRACT:
	This function returns a buffer descriptor to CellDataTest..  

RETURNS:
	None

DETAILS:
	This returns a buffer descriptor and its associated buffer back to 
	CellDataTest after ChipAL no longer needs the data. If CellDataTest is 
	using buffers that ChipAL provides when calling CellDataTestGetTxFullBuffer, 
	then this function is not used.
*******************************************************************************/

GLOBAL void CellDataTestTxComplete
(
    IN	CDSL_VC_T		*VCHdl,			//Link Handle to data stream
    IN	SK_BUFF_T	*BufferDescPtr	//Pointer to a buffer descriptor
)
{

	CELL_DATA_TEST_VC_MODE		VCMode;

	// Get the test module VC context
	CELL_DATA_TEST_VC_T  * pCellDataTestVC = (CELL_DATA_TEST_VC_T*)VCHdl->CellDataTestMod;
	SK_BUFF_T	NewBufferDescPtr;

	// Guard against receiving an uninitialized pointer
	if ( pCellDataTestVC != NULL )
	{
		VCMode = pCellDataTestVC->Parameters.VCMode;
	}
	else
	{
		VCMode = VC_MODE_IDLE;
	}

	// Determine which test mode we are operating in
	switch ( VCMode )
	{
	case VC_MODE_IDLE:
		// Do nothing??
		break;

	case VC_MODE_DIGITAL_LOOPBACK:
		// In digital loopback return the transmitted cell
		// back to cell management so that it may return it
		// to the ready queue.
		CellDataTestMgmtRxPutEmptyBuf( VCHdl, BufferDescPtr);
		break;

	case VC_MODE_CELL_VERIFICATION_TEST:
		CellDataTestNewCellForVerify( VCHdl, &NewBufferDescPtr);
		break;

	case VC_MODE_ATM_BER:
		CellDataTestNewCellForATMBER(VCHdl, &NewBufferDescPtr);
		break;

	default:
		// Do nothing??
		break;
	}
	return;
}


/*******************************************************************************
FUNCTION NAME:
	 CellDataTestPutRxFullBuffer

ABSTRACT:	This function delivers received raw data to HDLC,	  

RETURNS:
	NDISWAN packet pointe

DETAILS:
	This function gives a received buffer descriptor and its associated 
	buffer to CellDataTest. This will be a buffer previously provided 
	by CellDataTestGetRxEmptybuffer or a ChipAL provided buffer.
*******************************************************************************/

GLOBAL NDIS_STATUS	CellDataTestPutRxFullBuffer
(
    IN	CDSL_VC_T		*VCHdl,			// Link Handle to data stream
    IN	SK_BUFF_T *BufferDescPtr	// Pointer to a buffer descriptor
)
{

	FUNCTION_ENTRY

	NDIS_STATUS		Status;
	CELL_DATA_TEST_VC_MODE	VCMode;

	// Get the test module VC context
	CELL_DATA_TEST_VC_T  *pCellDataTestVC = (CELL_DATA_TEST_VC_T*)VCHdl->CellDataTestMod;

	TEST_PAYLOAD1_T 	  *		pTestCell;
	TEST_PAYLOAD1_T 	  *		pReceivedCell;
	LONG			SeqError;

	// Guard against receiving an uninitialized pointer
	if ( pCellDataTestVC != NULL )
	{
		VCMode = pCellDataTestVC->Parameters.VCMode;
	}
	else
	{
		VCMode = VC_MODE_IDLE;
	}

	// Determine which test mode we are operating in
	switch ( VCMode )
	{
	case VC_MODE_IDLE:
		// Returning a successful status causes cell management
		// to return the cell buffer back to the ready queue.
		Status = STATUS_SUCCESS;
		break;

	case VC_MODE_DIGITAL_LOOPBACK:
		// make sure cell management passed non-null buffer to this function
		if (BufferDescPtr != NULL)
		{
			Status = CellDataTestMgmtSend( VCHdl, BufferDescPtr);
		}
		else
		{
			Status = STATUS_SUCCESS;
		}
		break;

	case VC_MODE_CELL_VERIFICATION_TEST:

		// Check incoming cell payload for correct seq. no and data area.
		if (BufferDescPtr != NULL)
		{
			pReceivedCell = (TEST_PAYLOAD1_T*) BufferDescPtr->data;
			pTestCell = (TEST_PAYLOAD1_T*) &pCellDataTestVC->TestPayload;

			if ( pReceivedCell->SeqNo == (pCellDataTestVC->RxSeqNo+1))
			{
				if (memcmp(pReceivedCell->Data, pTestCell->Data, sizeof(pTestCell->Data)) == 0)
				{
					if ((OOB_DATA(BufferDescPtr) | OOD_CLP)  == 0)
					{
						AddToCtrLONG(&pCellDataTestVC->Stats.RXATMCellsGoodCLP0, 1);
					}
					else
					{
						AddToCtrLONG(&pCellDataTestVC->Stats.RXATMCellsGoodCLP1, 1);
					}
				}
				else
				{
					KdPrint((" Cell verify failed fill area compare  at %lx \r\n",
					         ( pCellDataTestVC->Stats.RXATMCellsGoodCLP0.Cnt +
					           pCellDataTestVC->Stats.RXATMCellsGoodCLP1.Cnt)));

					if ((OOB_DATA(BufferDescPtr) | OOD_CLP)  == 0)
					{
						AddToCtrLONG(&pCellDataTestVC->Stats.RXATMCellsDataErrorsCLP0, 1);
					}
					else
					{
						AddToCtrLONG(&pCellDataTestVC->Stats.RXATMCellsDataErrorsCLP1, 1);
					}
				}
			}
			else
			{
				KdPrint((" Cell verify sequence Nu. should %lx is %lx at %lx \r\n",
				         (pCellDataTestVC->RxSeqNo+1),
				         pReceivedCell->SeqNo,
				         (pCellDataTestVC->Stats.RXATMCellsGoodCLP0.Cnt+
				          pCellDataTestVC->Stats.RXATMCellsGoodCLP1.Cnt)));

				if ( pReceivedCell->SeqNo < pCellDataTestVC->RxSeqNo)
				{
					// Tx had rapped
					SeqError = (LONG)pCellDataTestVC->RxSeqNo + 0xffff -  (LONG)pReceivedCell->SeqNo;


				}else if ( pReceivedCell->SeqNo > pCellDataTestVC->RxSeqNo)
				{
					SeqError = (LONG)pReceivedCell->SeqNo - ((LONG)pCellDataTestVC->RxSeqNo+1);
					if (SeqError <= 0)
					{
						SeqError = 1;
					}
				}
				else
				{
					// they are equal
					SeqError = 1;
				}

				if (pCellDataTestVC->FirstRxCell == TRUE)
				{
					if ((OOB_DATA(BufferDescPtr) | OOD_CLP)  == 0)
					{
						AddToCtrLONG(&pCellDataTestVC->Stats.RXATMCellsSeqErrorsCLP0, 1); //(LONG)SeqError);
					}
					else
					{
						AddToCtrLONG(&pCellDataTestVC->Stats.RXATMCellsSeqErrorsCLP1, 1); // (LONG)SeqError);
					}
				}
			}
			// update sequence number
			pCellDataTestVC->FirstRxCell = TRUE;
			pCellDataTestVC->RxSeqNo = pReceivedCell->SeqNo;
			// In digital loopback return the transmitted cell
			// back to cell management so that it may return it
			// to the ready queue.
		}
		Status = STATUS_SUCCESS;

		break;

	case VC_MODE_ATM_BER:

		CellDataTestATMBerRx
		(
		    VCHdl,
		    BufferDescPtr
		);


	default:
		Status = STATUS_SUCCESS;
		break;
	}
	return Status;
}

/*******************************************************************************
FUNCTION NAME:
	 CellDataTestRxComplete

ABSTRACT:	This function delivers received raw data to HDLC,	  

RETURNS:
	NDISWAN packet pointe

DETAILS:
	This function gives a received buffer descriptor and its associated 
	buffer to CellDataTest. This will be a buffer previously provided 
	by CellDataTestGetRxEmptybuffer or a ChipAL provided buffer.
*******************************************************************************/

GLOBAL void CellDataTestRxComplete
(
    IN	CDSL_VC_T		*VCHdl		// Link Handle to data stream
)
{
	FUNCTION_ENTRY
	NOT_USED( VCHdl );
	return;
}

/*******************************************************************************
FUNCTION NAME:
	 CellDataTestNewCellForAtmVer

ABSTRACT:	This function generates a new test cell for Cell Verification Test

RETURNS:
	NDISSTATUS 

DETAILS:
*******************************************************************************/
LOCAL NDIS_STATUS CellDataTestNewCellForATMBER
(
    IN	CDSL_VC_T		*VCHdl,		// Link Handle to data stream
    IN	SK_BUFF_T *BufferDescPtr	// Pointer to a buffer descriptor
)
{
	FUNCTION_ENTRY

	CELL_DATA_TEST_VC_T  *		pCellDataTestVC;
	NDIS_STATUS					Status;
	BYTE 				 		Payload [CELL_PAYLOAD_SIZE];
	DWORD						Index;
	DWORD 						BitsSinceErrorTable [] =
	    {
	        0,			 //NO_ERROR
	        0,			 //SINGLE_ERROR
	        0,
	        1000,		 //ERRORM3,
	        10000,		 //ERRORM4,
	        100000,		 //ERRORM5,
	        1000000,	 //ERRORM6,
	        10000000	 //ERRORM7
	    };


	if (( VCHdl != NULL)  && ( VCHdl->CellDataTestMod != NULL))
	{
		pCellDataTestVC = (CELL_DATA_TEST_VC_T *) VCHdl->CellDataTestMod;

		if (BufferDescPtr != NULL)
		{
			for (Index = 0; Index < (CELL_PAYLOAD_SIZE ); Index++)
			{
				Payload[Index] = (BYTE) (((pCellDataTestVC->TXPRBSAccum >> 24) ^
				                          (pCellDataTestVC->RXPolarityMask)) & 0xff);

				pCellDataTestVC->TXPRBSAccum = UpdatePrbs(pCellDataTestVC->TXPRBSAccum);

			}

			switch (pCellDataTestVC->ATMBERInsertErrors)
			{
			case INJECT_NO_ERROR:
				break;

			case INJECT_SINGLE_ERROR:
				Payload[0] ^= 0x00000001;
				pCellDataTestVC->ATMBERInsertErrors = INJECT_NO_ERROR;
				break;

			case ERRORM3:

			case ERRORM4:
			case ERRORM5:
			case ERRORM6:
			case ERRORM7:

				if (pCellDataTestVC->BitsSinceError > BitsSinceErrorTable[pCellDataTestVC->ATMBERInsertErrors])
				{
					Payload[0] ^= 0x00000001;
					pCellDataTestVC->BitsSinceError  = 0;
				}
				else
				{
					pCellDataTestVC->BitsSinceError += (CELL_PAYLOAD_SIZE * 8);
				}


				break;

			}

			BufferDescPtr->data = (PVOID) Payload;
			BufferDescPtr->len  = CELL_PAYLOAD_SIZE;
			OOB_DATA(BufferDescPtr) = OOD_CELLMGMT_CPY;

			// record number of bits sent
			AddToCtrLONGLONG(&pCellDataTestVC->Stats.ATMBERTXBits, CELL_PAYLOAD_SIZE * 8);

			if ((OOB_DATA(BufferDescPtr) | OOD_CLP)  == 0)
			{
				AddToCtrLONG(&pCellDataTestVC->Stats.TXATMCellsCLP0, 1);
			}
			else
			{
				AddToCtrLONG(&pCellDataTestVC->Stats.TXATMCellsCLP1, 1);
			}
			// CellDataTestMgmt will copy the payload, TestCell can be on the stack.
			Status = CellDataTestMgmtSend( VCHdl, BufferDescPtr);
			if (Status != STATUS_SUCCESS)
			{
				// if statement is to allow the setting of a breakpoint if this error happens.
				ASSERT(Status == STATUS_SUCCESS);
			}
		}
		else
		{
			return STATUS_FAILURE;
		}
	}
	else
	{
		return STATUS_SUCCESS;
	}

	return STATUS_SUCCESS;
}



/*******************************************************************************
FUNCTION NAME:
	 CellDataTestNewCellForVerify

ABSTRACT:	This function generates a new test cell for Cell Verification Test

RETURNS:
	NDISSTATUS 

DETAILS:
*******************************************************************************/

LOCAL NDIS_STATUS CellDataTestNewCellForVerify
(
    IN	CDSL_VC_T		*VCHdl,		// Link Handle to data stream
    IN	SK_BUFF_T *BufferDescPtr	// Pointer to a buffer descriptor
)
{
	FUNCTION_ENTRY

	CELL_DATA_TEST_VC_T  *		pCellDataTestVC;
	TEST_PAYLOAD1_T 	 * 		pTestCell;
	NDIS_STATUS					Status;
	if ( VCHdl != NULL)
	{
		pCellDataTestVC = (CELL_DATA_TEST_VC_T*)VCHdl->CellDataTestMod;

		if ((pCellDataTestVC != NULL) && (BufferDescPtr != NULL))
		{
			pTestCell = (TEST_PAYLOAD1_T*)&pCellDataTestVC->TestPayload;
			pTestCell->SeqNo = (DWORD)pCellDataTestVC->TxSeqNo++;

			BufferDescPtr->data = (PVOID) pTestCell;
			BufferDescPtr->len  = CELL_PAYLOAD_SIZE;

			OOB_DATA(BufferDescPtr) = OOD_CELLMGMT_CPY;
			if ((OOB_DATA(BufferDescPtr) | OOD_CLP)  == 0)
			{
				AddToCtrLONG(&pCellDataTestVC->Stats.TXATMCellsCLP0, 1);
			}
			else
			{
				AddToCtrLONG(&pCellDataTestVC->Stats.TXATMCellsCLP1, 1);
			}
			// CellDataTestMgmt will copy the payload, TestCell can be on the stack.
			Status = CellDataTestMgmtSend( VCHdl, BufferDescPtr);
			if (Status != STATUS_SUCCESS)
			{
				// if statement is to allow the setting of a breakpoint if this error happens.
				ASSERT(Status == STATUS_SUCCESS);
			}
		}
		else
		{
			return STATUS_FAILURE;
		}
	}
	else
	{
		return STATUS_SUCCESS;
	}

	return STATUS_SUCCESS;
}

/*******************************************************************************
FUNCTION NAME:
	 CellDataTestATMBerRx

ABSTRACT:	This function checks received cells for ATM BER errors. 

RETURNS:
	NDISSTATUS 

DETAILS:
*******************************************************************************/

LOCAL NDIS_STATUS CellDataTestATMBerRx
(
    IN	CDSL_VC_T		*VCHdl,		// Link Handle to data stream
    IN	SK_BUFF_T *BufferDescPtr	// Pointer to a buffer descriptor
)
{
	unsigned char *rx_data;
	unsigned long error_count = 0U;
	int i,rx_data_length=48;

	// Get the test module VC context
	CELL_DATA_TEST_VC_T  *pCellDataTestVC = (CELL_DATA_TEST_VC_T*)VCHdl->CellDataTestMod;
	//rx_data = received_data;

	rx_data =  (unsigned char *)BufferDescPtr->data;

	if (pCellDataTestVC->ATMBERInsync == FALSE)
	{
		/* seed the PRBS accumulator */
		for (i = 0;  i < 4;  ++i) {
			pCellDataTestVC->RXPRBSAccum =
			    (pCellDataTestVC->RXPRBSAccum << 8) ^
			    (rx_data[i] ^ pCellDataTestVC->RXPolarityMask);
		}
	}

	/* check the received data */
	for (i = 0;  i < rx_data_length;  ++i)
	{
		unsigned long error_pattern =
		    (pCellDataTestVC->RXPRBSAccum >> 24) ^
		    (rx_data[i] ^ pCellDataTestVC->RXPolarityMask);
		if (error_pattern != 0U)
		{
			error_count += Weight[error_pattern & 0xff];
		}
		pCellDataTestVC->RXPRBSAccum = UpdatePrbs
		                               (
		                                   pCellDataTestVC->RXPRBSAccum
		                               );
	}

	AddToCtrLONGLONG(&pCellDataTestVC->Stats.ATMBERRXBits, rx_data_length * 8);

	if (error_count == 0)
	{
		pCellDataTestVC->ATMBERInsync = TRUE;
	}

	if (error_count > ATM_BER_SYNC_THRESHOLD)
	{
		// check for lost cell
		AddToCtrLONGLONG(&pCellDataTestVC->Stats.ATMBERRXResyncs, 1);

		/* seed the PRBS accumulator */
		for (i = 0;  i < 4;  ++i)
		{
			pCellDataTestVC->RXPRBSAccum =
			    (pCellDataTestVC->RXPRBSAccum << 8) ^
			    (rx_data[i] ^ pCellDataTestVC->RXPolarityMask);
		}

		/* check the received data */
		for (i = 0;  i < rx_data_length;  ++i)
		{
			unsigned long error_pattern =
			    (pCellDataTestVC->RXPRBSAccum >> 24) ^
			    (rx_data[i] ^ pCellDataTestVC->RXPolarityMask);
			if (error_pattern != 0U)
			{
				error_count += Weight[error_pattern & 0xff ];
			}
			pCellDataTestVC->RXPRBSAccum = 	UpdatePrbs
			                                (
			                                    pCellDataTestVC->RXPRBSAccum
			                                );
		}

		if (error_count > ATM_BER_SYNC_THRESHOLD)
		{
			pCellDataTestVC->ATMBERInsync = FALSE;
		}
	}

	AddToCtrLONGLONG(&pCellDataTestVC->Stats.ATMBERRXErrors, error_count);

	return STATUS_SUCCESS;
}
