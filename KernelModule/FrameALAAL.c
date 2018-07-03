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
**		ATM Abstraction Layer
**
**	FILE NAME:
**		FrameALAAL.c
**
**	ABSTRACT:
**		Contains the functions to support AAL (SAR & CS) processing of the
**		driver data path interface with NDIS.
**
**	DETAILS:
**		The functions that are contained within this file provide the
**		functionality of the Segmentation and Re-assembly and Convergence
**		Sublayer processing as required by the AALx types listed in ITU I.363.
**		Collectively these functions make up the FrameALAAL module.
**
**		Together the FrameALAAL module and the ATM module make up the FrameAL
**		module.  Within the FrameAL module the heirarchy of the AAL and ATM
**		modules has the AAL module above the ATM module.  Within the data path
**		stack heirarchy the FrameAL module is below the Buffer Management module
**		and above the ChipAL module.
**
**		Both the transmit and receive paths of the FrameAL module are driven
**		from below by ChipAL.  That is to say when ChipAL needs data to
**		transmit or has data that it has received, it makes calls that work
**		their way UP the data path stack.
**
**		The functions are grouped as follows and in this order, AAL Timer and
**		other misc. functions, transmit, and receive.
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux 2.4 ATM Driver/KernelModule/FrameALAAL.c $
**	$Revision: 3 $
**	$Date: 2/28/01 11:05a $
*******************************************************************************
******************************************************************************/

#include "FrameALAAL.h"

// ***************************************
//		DEFINES THAT REALLY SHOULD
//		IN PRODUCT.H OR THE REGISTRY
// ***************************************

#define DEFAULT_CPCS_UU			0
#define	DEFAULT_CPI				0
#define	DEFAULT_RASCELLCNT		0

#define CELL_LOSS		0
#define CONGESTION_IND	0
#define	RAS_TIMEOUT		10				// time in milliseconds

// ***************************************
//		DEBUG ITEMS
// ***************************************

#ifndef FUNCTION_ENTRY
	#define FUNCTION_ENTRY
#endif

#define	INIT_AAL5_PADDING	0
#define	AAL5_PADDING_VALUE	0
#define RAS_TMR_ACTIVE		0
#define PPP_OVER_ATM		1

// ***************************************
//		FILE CONSTANTS
// ***************************************

#define INITIAL_CRC_VAL		0xffffffff  // negative one
#define CRC_COMPLEMENT_MASK	0xffffffff  // negative one
#define GOODCRC32			0xc704dd7b  // Good final CRC value
#define	MAX_PADDING			47			// Maximum number of padding bytes
#define AAL5_TRLR_SIZE		8			// AAL5 trailer size in bytes

#define NDISPktPtr			ProtocolReserved1

#define OOBDATA_CLP( x )	(*(DWORD*)&((x)->cb[              0]))
#define OOBDATA_CPI( x )	(*(DWORD*)&((x)->cb[  sizeof(DWORD)]))
#define OOBDATA_UUI( x )	(*(DWORD*)&((x)->cb[2*sizeof(DWORD)]))
#define OOBDATA_PTI( x )	(*(DWORD*)&((x)->cb[3*sizeof(DWORD)]))


// ***************************************
//		FILE STRUCTURES
// ***************************************

typedef struct
{
	BYTE	UserToUserIndication;
	BYTE	CommonPartIndicator;
	WORD	Payload_Len;
	DWORD	Crc32;

} AAL5_TRLR;

// ***************************************
//		FILE TABLES
// ***************************************

// CRC32 Lookup Table generated from Charles Michael
//  Heard's CRC-32 code
static DWORD crc32_tbl[256] =
    {
        0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
        0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
        0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
        0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
        0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
        0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
        0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
        0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
        0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
        0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
        0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
        0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
        0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
        0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
        0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
        0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
        0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
        0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
        0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
        0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
        0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
        0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
        0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
        0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
        0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
        0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
        0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
        0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
        0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
        0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
        0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
        0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
        0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
        0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
        0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
        0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
        0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
        0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
        0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
        0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
        0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
        0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
        0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
        0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
        0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
        0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
        0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
        0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
        0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
        0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
        0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
        0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
        0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
        0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
        0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
        0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
        0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
        0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
        0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
        0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
        0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
        0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
        0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
        0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
    };

// *********************************
// Local function definitions
// *********************************
LOCAL void FrameALAALStartRASTimer
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
);

LOCAL void FrameALAALStopRASTimer
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
);

LOCAL DWORD FrameALAALCalcCRC32
(
    IN	BYTE         *pData,
    IN  DWORD        Length,
    IN  DWORD        InitialValue
);

LOCAL NDIS_STATUS FrameALAALTxProcessBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be processed
);

LOCAL NDIS_STATUS FrameALAALRxProcessBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be processed
);

// *********************************
// AAL Timer and other Misc functions
// *********************************

/*******************************************************************************

FUNCTION NAME:
		FrameALAALRASTimerExpiry

ABSTRACT:
		Cleans house for the VC whose receiver inter-cell reassembly timer expired.

DETAILS:
		When one of the per VC re-assembly timers expire NDIS calls this function
		passing to it the context of VC for which the timer expired.   This
		function will remove the RAS timer from the system and call the function
		FrameALAALRxProcessBuf to see if the frame that has been reassembled to this point
		is valid or not.   If it is valid then the RX reassembly buffer variables are
		reset to NULL and the function returns to the caller.
		
		If the frame is not valid then the 48 byte pointer is reset to point
		back to the beginning of the buffer.  The appropriate error statistics are
		logged and the function returns to the caller.

*******************************************************************************/

GLOBAL BOOLEAN FrameALAALRASTimerExpiry
(
    IN	void 		*pCtx			// VC context of data stream
)
{
	FUNCTION_ENTRY

	CDSL_VC_T 		*VcCtx = (CDSL_VC_T*)pCtx;

	NDIS_STATUS NdisStatus;

	// Get the FrameAL VC context
	FRAMEAL_VC_T *FrameALVcCtx = VcCtx->FrameALVCMod;

	// Get the Frame Link context
	FRAMEAL_LINK_T *FrameALLinkCtx = ((CDSL_LINK_T *) (VcCtx->LinkHdl))->FrameMod;

	// Get the AAL Link context
	FRAMEAL_AAL_LINK_T *AALLinkCtx = &FrameALLinkCtx->AALLinkBlock;

	// Locate the AAL VC Context
	FRAMEAL_AAL_VC_T *AALVcCtx = &FrameALVcCtx->AALVcCtx;

	// Even though the RAS timer has expired go ahead and call the function
	// FrameALAALStopRASTimer which will remove it from the system.  If a RAS timer
	// is required again, a new one can/will be started.
	FrameALAALStopRASTimer( VcCtx );

	// Now apply the AALx processing to the RX frame
	NdisStatus	=	FrameALAALRxProcessBuf
	             (
	                 // VC context of data stream
	                 VcCtx,
	                 // Buffer descriptor to be processed
	                 AALVcCtx->AALxRxBfr
	             );

	// Was the RX frame successfully processed?
	if ( NdisStatus == STATUS_SUCCESS )
	{
		// The reassembly buffer has been successfully processed and returned to
		// Buffer Management so indicate that we no longer have an reassembly
		// buffer to use

		AALVcCtx->AALxRxBfr = NULL;
		AALVcCtx->AALxRx48BytePtr = NULL;
	}
	else
	{
		// The RX frame WAS NOT successfully processed by the AALx RX
		// processing function so flush the current reassembled frame

		AALVcCtx->AALxRx48BytePtr = AALVcCtx->AALxRxBfr->data;
		AALVcCtx->AALxRxBfr->len = 0;

		// Indicate a bad reassembly frame
		// Update the AAL per VC stats
		AddToCtrLONG(&AALVcCtx->AALRxReassemblyTimeout, 1);

		// Update the AAL per Link stats
		AddToCtrLONG(&AALLinkCtx->AALRxReassemblyTimeout, 1);

		// Update Linux ATM stats
		atomic_inc(&VcCtx->pVcc->dev->stats.aal5.rx_err);
	}
	return TRUE;
}


/*******************************************************************************

FUNCTION NAME:
		FrameALAALStartRASTimer

ABSTRACT:
		Starts or resets the receiver inter-cell reassembly timer.
DETAILS:
		When called by the function FrameALAALRxPutFullBuf it it uses the VC context to
		find the link context associated with the VC.   It then calls the function
		UtilTmrIsTmrActive passing the VC and associated Link contexts to it to
		determine if there is already an active timer for this VC.   If there is
		not then it calls to the function UtilTmrStartTmr passing to it the VC and
		associated Link contexts to be used as identifiers of the timer.   It also
		passes to this function a time value that is calculated using the RASCellCnt
		and current speed of the VC for the RAS time.   A pointer to the function
		FrameALAALRASTimerExpiry and a pointer to the VC context are also passed as the
		function and parameter respectively that are to be used if the timer expires.

		If there is already a running timer, then the function UtilTmrReStartTmr is
		called passing to it the VC and associated Link context pointers as timer
		identifiers and a time value that is calculated using the same variables as
		in the call to UtilTmrStartTmr. FrameALAALStartRASTimer returns to the caller
		immediately upon returning from either of the calls to UtilTmrStartTmr or
		UtilTmrReStartTmr.

		NOTE:	The current implementation of this function does not use RASCellCnt
				and the current speed of the VC to calculate a RAS time value.  It
				is currently hard coded to a value of 10MS for expediency sake.

*******************************************************************************/

LOCAL void FrameALAALStartRASTimer
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
)
{
	FUNCTION_ENTRY

	BOOL	TmrActive;
	BOOL	TmrResult;

	if ( RAS_TMR_ACTIVE )
	{
		// Check for a running RAS timer for this Link/VC
		TmrActive =	UtilTmrIsTmrActive
		            (
		                &(VcCtx->LinkHdl->pThisAdapter->TimerBlock),
		                VcCtx->LinkHdl,		// RefOne
		                (DWORD) VcCtx		// RefTwo
		            );

		// Is there an running RAS timer for this Link/VC
		if ( TmrActive )
		{
			// A RAS timer DOES exist so reset it
			TmrResult =	UtilTmrReStartTmr
			            (
			                &(VcCtx->LinkHdl->pThisAdapter->TimerBlock),
			                VcCtx->LinkHdl,			// RefOne
			                (DWORD) VcCtx,			// RefTwo
			                RAS_TIMEOUT				// Time interval in millisec
			            );

			// Did the timer reset properly?
			if ( !TmrResult )
			{
				// The RAS timer for this Link/VC did not reset properly
				// !!! AN INDICATION FOR THIS CONDITION SHOULD PROBABLY BE LOGGED
			}
		}
		else
		{
			// A RAS timer DOES NOT exist or is not running so start one
			TmrResult =	UtilTmrStartTmr
			            (
			                &(VcCtx->LinkHdl->pThisAdapter->TimerBlock),
			                VcCtx->LinkHdl,			// RefOne
			                (DWORD) VcCtx,			// RefTwo
			                RAS_TIMEOUT,			// Time interval in millisec
			                &FrameALAALRASTimerExpiry,// Function to call upon expiry
			                (void *) VcCtx			// Parameter to pass expiry function
			            );

			// Did the timer start properly?
			if ( !TmrResult )
			{
				// The RAS timer for this Link/VC did not start properly
				// !!! AN INDICATION FOR THIS CONDITION SHOULD PROBABLY BE LOGGED
			}
		}
	}
	return;
}


/*******************************************************************************

FUNCTION NAME:
		FrameALAALStopRASTimer

ABSTRACT:
		Stops the receiver inter-cell reassembly timer.

DETAILS:
		When called by the function FrameALAALRxPutFullBuf it uses the VC context to find
		the link context associated with the VC.   It then calls the function
		UtilTmrClearTmr passing the VC and associated Link context to it to stop/remove
		the re-assembly timer.  It basically returns to the caller immediately upon
		returning from the call to UtilTmrClearTmr.

*******************************************************************************/

LOCAL void FrameALAALStopRASTimer
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
)
{
	FUNCTION_ENTRY

	BOOL	TmrResult;

	if ( RAS_TMR_ACTIVE )
	{
		// Stop and remove from the system any RAS timer for
		// this particular Link/VC combination
		TmrResult = UtilTmrClearTmr
		            (
		                &(VcCtx->LinkHdl->pThisAdapter->TimerBlock),
		                VcCtx->LinkHdl,			// RefOne
		                (DWORD) VcCtx			// RefTwo
		            );

		// Did the timer stop properly?
		if ( !TmrResult )
		{
			// The RAS timer for this Link/VC did not stop properly or did not exist
			// !!! AN INDICATION FOR THIS CONDITION SHOULD PROBABLY BE LOGGED
		}
	}
	return;
}

/*******************************************************************************

FUNCTION NAME:
		FrameALAALVCInit

ABSTRACT:
		Initializes the AAL VC specific data structure.

DETAILS:
		This function is called from within FrameALVCInit when a new VC is
		established.  This function intializes all of the entries of the AAL VC
		context structure contained within the FrameAL VC context structure with
		the appropriate values.   The return status value for this function will
		be dependent upon the success or failure of this operation.

		NOTE:	In the current implementation of this function it will always
				return a successful status.

*******************************************************************************/

GLOBAL NDIS_STATUS FrameALAALVCInit
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
)
{
	FUNCTION_ENTRY

	// Get the FrameAL VC context
	FRAMEAL_VC_T *FrameALVcCtx = VcCtx->FrameALVCMod;

	// Locate the AAL VC Context
	FRAMEAL_AAL_VC_T *AALVcCtx = &FrameALVcCtx->AALVcCtx;

	// General parameters
	AALVcCtx->RASCellCnt = DEFAULT_RASCELLCNT;

	// TX parameters
	AALVcCtx->CPCS_UU = DEFAULT_CPCS_UU;
	AALVcCtx->CPI = DEFAULT_CPI;

	AALVcCtx->AALxTxBfr = NULL;
	AALVcCtx->AALxTx48BytePtr = NULL;

	AALVcCtx->AALTxGood.Cnt = 0;
	AALVcCtx->AALTxGoodByte.Cnt = 0;
	AALVcCtx->AALTxDiscarded.Cnt = 0;
	AALVcCtx->AALTxDiscardedByte.Cnt = 0;

	// RX parameters
	AALVcCtx->AALxRxBfr = NULL;
	AALVcCtx->AALxRx48BytePtr = NULL;

	AALVcCtx->AALRxGood.Cnt = 0;
	AALVcCtx->AALRxGoodByte.Cnt = 0;
	AALVcCtx->AALRxDiscarded.Cnt = 0;
	AALVcCtx->AALRxDiscardedByte.Cnt = 0;
	AALVcCtx->AALRxInvalidLen.Cnt = 0;
	AALVcCtx->AALRxInvalidCRC.Cnt = 0;
	AALVcCtx->AALRxReassemblyTimeout.Cnt = 0;

	return STATUS_SUCCESS;
}

/*******************************************************************************

FUNCTION NAME:
		FrameALAALVCShutdown

ABSTRACT:
		Performs any AAL module cleanup for a VC when it is shutdown.

DETAILS:
		This function is called from within FrameALVCShutdown when a previously
		established VC needs to be removed.   When called this function will
		return any transmit or receive buffers that it has "ownership" of to
		Buffer Management and stop and remove from the system any RAS timers
		that may have been running.  The return status value for this function
		will be dependent upon the success or failure of these operations.

		NOTE:	In the current implementation of this function it will always
				return a successful status.

*******************************************************************************/

GLOBAL NDIS_STATUS FrameALAALVCShutdown
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
)
{
	FUNCTION_ENTRY

	FRAMEAL_VC_T	 *FrameALVcCtx;
	FRAMEAL_AAL_VC_T *AALVcCtx;

	if( VcCtx != NULL )
	{
		// Get the FrameAL VC context
		FrameALVcCtx = VcCtx->FrameALVCMod;

		// Locate the AAL VC Context
		AALVcCtx = &FrameALVcCtx->AALVcCtx;
	}
	else
	{
		// The VC context pointer was NULL
		// !!! PROBABLY NEED TO LOG AN ERROR CONDITION HERE

		// No resources, if any were held, were freed
		return STATUS_FAILURE;
	}

	if(	FrameALVcCtx != NULL && AALVcCtx != NULL )
	{
		// Stop/remove from the system any RAS timer that may have been running
		FrameALAALStopRASTimer( VcCtx );

		// Check for a TX frame we may have in our possession
		if ( AALVcCtx->AALxTxBfr != NULL )
		{
			// Return the TX frame back to Buffer Management
			// since we cannot do anything more with it

			AALVcCtx->AALxTxBfr->data = AALVcCtx->AALxTxBfrPtr;
			AALVcCtx->AALxTxBfr->len = AALVcCtx->AALxTxBfrLength;

			BufMgmtTxPutEmptyBuf
			(
			    // VC context of data stream
			    VcCtx,
			    // TX frame that we will not finish sending
			    AALVcCtx->AALxTxBfr
			);
		}

		// Do we have an RX buffer in our possession?
		if ( AALVcCtx->AALxRxBfr != NULL )
		{
			// Return the RX frame back to Buffer Management
			// since we cannot do anything more with it

			BufMgmtRxPutEmptyBuf
			(
			    // VC context of data stream
			    VcCtx,
			    // RX frame we no longer need
			    AALVcCtx->AALxRxBfr
			);
		}
		return STATUS_SUCCESS;
	}
	else
	{
		// One of the pointers FrameALVcCtx or AALVcCtx was NULL.
		// Because of this No resources, if any were held, were freed.

		return STATUS_FAILURE;
	}
}


/*******************************************************************************

FUNCTION NAME:
		FrameALAALCalcCRC32

ABSTRACT:
		Calculates a CRC32 value on the data referenced by the Buffer Descriptor

DETAILS:
		This function is called by both the FrameALAALRxPutFullBuf and FrameALAALTxGetFullBuf
		to calculate a CRC32 value on the data pointed to by the Athena buffer
		descriptor passed to this function.  The CRC32 value is calculated using
		an algorithm developed by Charles Michael Heard.   The original source
		code for this algorithm was found at the following URL:
		
		http://cell-relay.indiana.edu/cell-relay/publications/software/CRC/32bitCRC.c.html

		The data from the buffer that will be included in the calculation extends
		from currentbuffer to ( currentbuffer + length).   If the CRC bytes in the
		PDU trailer structure are not to be included in the calculation, then the
		length field of the Athena buffer descriptor being passed to the function
		needs to be reduced	by 4 bytes.    After calculating the CRC, the function
		returns a DWORD	containing the CRC32 value.

*******************************************************************************/

LOCAL DWORD FrameALAALCalcCRC32
(
    IN	BYTE         *pData,
    IN  DWORD        Length,
    IN  DWORD        InitialValue
)
{
	FUNCTION_ENTRY

	DWORD	crc32_val = InitialValue;
	BYTE	*buffer_data_ptr = pData;
	DWORD	buffer_data_len = Length;

	// Calculate a CRC32 value
	while (buffer_data_len--)
	{
		crc32_val = ( crc32_val << 8 ) ^ crc32_tbl[(( crc32_val >> 24) ^ *buffer_data_ptr++ ) & 0xff];
	}

	return (crc32_val);
}


// *********************************
// AAL TX functions
// *********************************

/*******************************************************************************

FUNCTION NAME:
		FrameALAALTxProcessBuf

ABSTRACT:
		Applies AALx processing to the transmit data frames from NDIS  

DETAILS:
		Even though the function name implies multiple AAL type processing, the
		initial implementation of this function will only support AAL5.  If the link
		is configured for any other	AAL type the function will return an
		STATUS_FAILURE response.

		AAL5 PROCESSING
		This function will be called by the SAR module when there is a transmit
		data buffer to process.   SAR functionality will pass to this function the
		current	VC context pointer and a non-NULL pointer to an Athena buffer descriptor
		containing the transmit data to be encapsulated in AAL5 format.  The first
		step in encapsulating the frame will be to determine the amount of padding
		required to insure that the CPCS-PDU frame is an integer multiple of 48 bytes.
		If a debug flag is set the padding area will be initialized to a "known" value
		to facilitate the identification of the padding area.   If the debug flag is
		not set this initialization of the padding area will not take place.   The
		calculated quantity of padding is then added to a temporary pointer which
		contains a copy of the value found in the length field of the Athena buffer
		descriptor.   This temporary pointer is then used and incremented appropriately
		to copy the CPCS-UU, CPI and length field value of the Athena buffer descriptor
		into the transmit data buffer.   After performing these copies, the length field
		of the Athena buffer descriptor is updated to reflect the addition of the padding
		bytes, CPCS-UU, CPI and Length fields.    The CPCS-UU and CPI fields are set with
		the values found in the CPCS-UU and CPI fields in the current link context.   The
		function FrameALAALCalcCRC32 is then called passing to it a pointer to the Athena buffer
		descriptor being operated upon.   The return value from FrameALAALCalcCRC32 is put into
		the CRC field of the AAL5 trailer in the data buffer, the Athena buffer descriptor
		length field is updated to reflect the addition of the CRC32.   The appropriate
		statistic counters located in the VC context are updated and an NDIS status
		indicating the success/failure of this operation is returned to the SAR function.
		
*******************************************************************************/

LOCAL NDIS_STATUS FrameALAALTxProcessBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be processed
)
{
	FUNCTION_ENTRY

	DWORD	crc32_val;
	WORD	padding_len;
	WORD	payload_len_mod;
	WORD	payload_len;
	UCHAR*  pData;

	// Get the FrameAL VC context
	FRAMEAL_VC_T *FrameALVcCtx = VcCtx->FrameALVCMod;

	// Get the Frame Link context
	FRAMEAL_LINK_T *FrameALLinkCtx = ((CDSL_LINK_T *) (VcCtx->LinkHdl))->FrameMod;

	// Get the AAL Link context
	FRAMEAL_AAL_LINK_T *AALLinkCtx = &FrameALLinkCtx->AALLinkBlock;

	// Get the VC Parameters
	FRAMEAL_VC_PARAMETERS FrameALVcParms = FrameALVcCtx->Parameters;

	// Locate the AAL VC Context
	FRAMEAL_AAL_VC_T *AALVcCtx = &FrameALVcCtx->AALVcCtx;

	// Determine what AAL type we are
	switch( FrameALVcParms.AALType )
	{
	case ATM_AAL5:

		// Do we really have a buffer to process?
		if( BufDescPtr->data == NULL )
		{
			return(STATUS_FAILURE);
		}

		// AAL5 PADDING CALCULATION

		// The following operation produces a value that is the
		// remainder (modulus) of the division of the payload length by 48
		payload_len = (WORD)BufDescPtr->len;
		payload_len_mod = payload_len % CELL_PAYLOAD_SIZE;

		if ( payload_len_mod <= (CELL_PAYLOAD_SIZE - AAL5_TRLR_SIZE) )
		{
			// Intra-cell padding
			// If the frame length modulus is less than or equal to
			// 40 bytes then the AAL5 trailer of 8 bytes plus padding
			// will fit within the last 48 byte cell payload.


			// NOTE: padding_len in this case should range from 0 to 40.
			// NOTE: payload_len_mod + padding_len + AAL5 trailer = 48

			padding_len = CELL_PAYLOAD_SIZE - payload_len_mod - AAL5_TRLR_SIZE;
		}
		else
		{
			// Inter-cell padding
			// If the frame length modulus is greater than 40 then the
			// AAL5 trailer of 8 bytes will not fit within the last 48
			// byte cell payload.  So an additional 48 byte cell will
			// have to be added to the buffer and padded accordingly to
			// place the AAL5 trailer in the last 8 bytes of the new cell.

			// NOTE: padding_len in this case should range from 41 to 47.
			// NOTE: payload_len_mod + padding_len + AAL5 trailer = 96

			//				padding length in next to last cell		padding length in last cell
			padding_len = (CELL_PAYLOAD_SIZE - payload_len_mod) + (CELL_PAYLOAD_SIZE - AAL5_TRLR_SIZE);
		}

		// Test to make sure that our addition of the AAL5 padding and trailer
		// will not cause us to exceed the size of the buffer given to us.
		// If it is exceeded (as it generally will be) we will put the extra
		// data in a padding store area

		// I did have room so just set the pointers to put the data directly in
		// the buffer
		if( (BufDescPtr->data + BufDescPtr->len + padding_len + AAL5_TRLR_SIZE) < BufDescPtr->end )
		{
			pData   = &BufDescPtr->data[BufDescPtr->len] ;
			BufDescPtr->len += padding_len + AAL5_TRLR_SIZE;
			AALVcCtx->RemainingPadLength    = 0;
			AALVcCtx->RemainingBufferLength = BufDescPtr->len;
		}

		// I did not have room, so I will have to extend the buffer into a 
		// scratch region
		else
		{
			UCHAR* EndBlock;

			// only need one extra block so copy the overhanging data
			// and set the pointers and lengths
			if ( padding_len <= 40 )
			{
				DWORD  ExtraLth;
				UCHAR* pBufData;

				ExtraLth = BufDescPtr->len % CELL_PAYLOAD_SIZE;

				BufDescPtr->len -= ExtraLth;
				pBufData = &BufDescPtr->data[BufDescPtr->len] ;
				pData    = AALVcCtx->ExtraPad;

				while( ExtraLth-- )
				{
					*pData++ = *pBufData++;
				}

				AALVcCtx->RemainingPadLength = CELL_PAYLOAD_SIZE;
				AALVcCtx->RemainingBufferLength = BufDescPtr->len;
			}

			// if 2 additional blocks are required then
			//   see if I have room in the buffer to extend the buffer up
			//   up to the end of the first block
			else
			{
				EndBlock = BufDescPtr->data + BufDescPtr->len + padding_len - 40;

				// I have enough extra room for one cell, so take what I need out
				// of the base buffer and then put it in the extra padding block.
				if ( EndBlock <= BufDescPtr->end )
				{
					if( INIT_AAL5_PADDING )
					{
						pData        = &BufDescPtr->data[BufDescPtr->len] ;
						padding_len -= 40;

						// Padding area is to be initialized to a "known" value
						// up to the beginning of the AAL5 trailer.
						while( padding_len-- )
						{
							*pData++ = AAL5_PADDING_VALUE;
						}
					}

					pData = &AALVcCtx->ExtraPad[0];
					BufDescPtr->len += padding_len + AAL5_TRLR_SIZE - CELL_PAYLOAD_SIZE;
					padding_len = CELL_PAYLOAD_SIZE - AAL5_TRLR_SIZE;
					AALVcCtx->RemainingPadLength = CELL_PAYLOAD_SIZE;
					AALVcCtx->RemainingBufferLength = BufDescPtr->len;
				}

				// I need both extra blocks so determine how much buffer is
				// off the end of the last cell.  Copy it to the extra padding
				// buffer and then adjust the lengths and pointers.
				else
				{
					DWORD  ExtraLth;
					UCHAR* pBufData;

					ExtraLth = BufDescPtr->len % CELL_PAYLOAD_SIZE;

					BufDescPtr->len -= ExtraLth;
					pBufData = &BufDescPtr->data[BufDescPtr->len] ;
					pData    = AALVcCtx->ExtraPad;

					while( ExtraLth-- )
					{
						*pData++ = *pBufData++;
					}

					AALVcCtx->RemainingPadLength = 2*CELL_PAYLOAD_SIZE;
					AALVcCtx->RemainingBufferLength = BufDescPtr->len;
				}
			}
		}

		// Do we need to initialize the padding area?
		if( INIT_AAL5_PADDING )
		{
			// Padding area is to be initialized to a "known" value
			// up to the beginning of the AAL5 trailer.
			while( padding_len-- )
			{
				*pData++ = AAL5_PADDING_VALUE;
			}
		}
		else
		{
			// Just add the padding offset to the length to put us
			// up to the beginning of the AAL5 trailer.
			pData += (DWORD)padding_len;
		}

		// AAL5 TRAILER APPENDING

		// NDIS gives us OOB data in the CO driver
		*pData++ = (BYTE) OOBDATA_UUI(BufDescPtr);
		*pData++ = (BYTE) OOBDATA_CPI(BufDescPtr);

		// Fill in the Frame Payload Length field
		// NOTE: Length field value is stored MSB first
		*pData++ = (BYTE)((payload_len >> 8) & 0x00ff);
		*pData++ = (BYTE)(payload_len & 0x00ff);

		// Calculate the CRC32 field value
		if ( AALVcCtx->RemainingPadLength )
		{
			crc32_val = FrameALAALCalcCRC32( BufDescPtr->data,
			                                 BufDescPtr->len,
			                                 INITIAL_CRC_VAL );

			crc32_val = FrameALAALCalcCRC32( AALVcCtx->ExtraPad,
			                                 AALVcCtx->RemainingPadLength-4,
			                                 crc32_val );
		}
		else
		{
			crc32_val = FrameALAALCalcCRC32( BufDescPtr->data,
			                                 BufDescPtr->len-4,
			                                 INITIAL_CRC_VAL );
		}

		// Add CRC32 to transmit buffer
		// NOTE:The CRC32 value must first be ones complemented and
		// 		then stored Most significant byte first in the CRC32 field
		// NOTE:The following method of storing the CRC32 value in the buffer
		//		is independent of the endian'ness of the machine.

		crc32_val ^= CRC_COMPLEMENT_MASK;
		pData[ 3 ] = (BYTE)(crc32_val & 0x00ff);
		pData[ 2 ] = (BYTE)((crc32_val >>= 8) & 0x00ff);
		pData[ 1 ] = (BYTE)((crc32_val >>= 8) & 0x00ff);
		pData[ 0 ] = (BYTE)((crc32_val >> 8) & 0x00ff);

		// Update the AAL per VC stats
		AddToCtrLONG(&AALVcCtx->AALTxGood, 1);
		AddToCtrLONG(&AALVcCtx->AALTxGoodByte, (LONG)BufDescPtr->len);

		// Update Linux ATM stats
		atomic_inc(&VcCtx->pVcc->dev->stats.aal5.tx);

		// Update the AAL per Link stats
		AddToCtrLONG(&AALLinkCtx->AALTxGood, 1);
		AddToCtrLONG(&AALLinkCtx->AALTxGoodByte, (LONG)BufDescPtr->len);

		return(STATUS_SUCCESS);
		break;

	default:
		// Any other AAL type is currently invalid

		// Update the AAL per VC stats
		AddToCtrLONG(&AALVcCtx->AALTxDiscarded, 1);
		AddToCtrLONG(&AALVcCtx->AALTxDiscardedByte, (LONG)BufDescPtr->len);

		// Update the AAL per Link stats
		AddToCtrLONG(&AALLinkCtx->AALTxDiscarded, 1);
		AddToCtrLONG(&AALLinkCtx->AALTxDiscardedByte, (LONG)BufDescPtr->len);

		return(STATUS_FAILURE);
		break;
	}

	return(STATUS_FAILURE);
}


/*******************************************************************************

FUNCTION NAME:
		FrameALAALTxGetFullBuf

ABSTRACT:
		The functionality of the AAL SAR is basically performed in this function.

DETAILS:
		This function is called by the ATM layer when it requires a 48 byte data
		fragment for an ATM cell.   The first phase upon entering this function
		will be to check if there is currently a frame from Buffer Management that
		in process of being segmented.   It does this by checking the TX path buffer
		descriptor pointer of the AAL VC context structure for a non-NULL value.
		If the buffer descriptor pointer is NULL, then a a do-while() loop is entered
		which executes until it either successfully processes a buffer from Buffer
		Management or there are no more buffers from Buffer Management to transmit.

		The first step of this loop is a call to BufMgmtTxGetFullBuf to obtain a new
		frame to process.   If Buffer Management does not have a frame to process it
		will return a NULL pointer in which case FrameALAALTxGetFullBuf will set it's VC context
		structure buffer descriptor variables to NULL, set the Ndis status to success
		which will cause execution to break out of the do-while and return to the ATM
		layer with the cell buffer descriptor data pointer and data length set to NULL.
		
		If the call to BufMgmtTxGetFullBuf returns a frame for processing, then the AAL
		TX path buffer descriptor pointer is updated with the Athena Buffer Descriptor
		information returned from Buffer Management.   The function FrameALAALTxProcessBuf is
		then called to process the PPP encapsulated TCP/IP data of the frame into an
		AAL5 frame.   If it successfully returns from this call, FrameALAALTxGetFullBuf initializes
		the fragment pointer it uses to keep track of the segmenting of the frame into
		48 byte fragments to the beginning of the data buffer.  The Ndis status indicates
		success which causes execution to break out of the do-while loop.

		If the call to process the TX frame returns unsuccessfully, then return the frame
		back to Buffer Management by calling BufMgmtTxPutEmptyBuf.  After returning from
		this call, set the AAL VC context structure buffer descriptor variables and 48 byte
		cell pointer to NULL.   Set ATMs buffer descriptor variables to indicate no data and
		because the Ndis status indicated failure the do-while loop execution returns back
		to the top of the loop with the call to BufMgmtTxGetFullBuf again.

		If you get to this point there is a frame either newly obtained or still in process
		that is	available to segment into 48 byte fragments.  FrameALAALTxGetFullBuf now sets the
		data pointer of the ATM layer buffer descriptor to the current value of the fragment
		pointer.   The ATM layer buffer descriptor length field is set to 48 and the
		fragment pointer incremented to point to the beginning of the next 48 byte fragment.
		
		If after incrementing the fragment pointer by 48 it now points to the end of the
		data buffer the AUU bit of the Out-of-Band field is set, otherwise it is cleared.
		The Out-of-Band field parameters, Cell Loss and Congestion Indication are then set
		appropriately in the ATM layer buffer descriptor.  Upon completion of this
		FrameALAALTxGetFullBuf returns to the ATM layer.

*******************************************************************************/

GLOBAL SK_BUFF_T *FrameALAALTxGetFullBuf
(
    IN	CDSL_VC_T 	*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T	*BufDescPtr		// Buffer descriptor to be processed
)
{
	FUNCTION_ENTRY

	NDIS_STATUS NdisStatus;

	// Get the FrameAL VC context
	FRAMEAL_VC_T *FrameALVcCtx = VcCtx->FrameALVCMod;

	// Locate the AAL VC Context
	FRAMEAL_AAL_VC_T *AALVcCtx = &FrameALVcCtx->AALVcCtx;

	// Check for a TX frame currently in process
	// and try to acquire one if not

	if ( AALVcCtx->AALxTxBfr == NULL )
	{
		// There IS NOT a TX frame currently in process
		// so check with Buffer Management to see if it
		// has any frames available for processing

		// NOTE: The following do-while() loop is used to obtain a buffer
		// to transmit.   The do-while() loop executes until it either
		// successfully processes a buffer from Buffer Management or there
		// are no more buffers from Buffer Management to transmit.

		// NOTE: It is important to keep trying to successfully obtain a
		// a buffer to transmit so that we can keep ATM coming back for more
		// data or to drain Buffer Management's xmit queue so that it will
		// notify us when it's xmit queue goes from empty to non-empty.   If
		// we do not do either of these, then potentially the transmitter can
		// get into a condition whereby we have buffers queued up in Buffer
		// Management that ATM/ChipAL will never come to get.

		do
		{
			AALVcCtx->AALxTxBfr = 	BufMgmtTxGetFullBuf
			                       (
			                           // VC context of data stream
			                           VcCtx,
			                           // Void pointer to fill the function parameter
			                           NULL
			                       );

			// Did Buffer Management have a TX frame for us?
			if ( AALVcCtx->AALxTxBfr == NULL )
			{
				// It DID NOT have a TX frame for us
				// so NULL ours and ATMs pointers so
				// it will know there is no data.

				AALVcCtx->AALxTxBfr = NULL;
				AALVcCtx->AALxTx48BytePtr = NULL;

				BufDescPtr->data = NULL;
				BufDescPtr->len = 0;

				// Used to break us out of the do-while loop
				NdisStatus = STATUS_SUCCESS;
			}
			else
			{
				// Buffer Management DID have a TX frame for us

				// SAVE the current buffer pointer and length to restore them in
				// the buffer descriptor before we return it to buffer management.
				AALVcCtx->AALxTxBfrPtr =  AALVcCtx->AALxTxBfr->data;
				AALVcCtx->AALxTxBfrLength =	AALVcCtx->AALxTxBfr->len;

				// Initialize the 48 byte cell pointer to the beginning of the
				// newly acquired frame
				AALVcCtx->AALxTx48BytePtr = AALVcCtx->AALxTxBfr->data;

				// Now apply the AALx processing to the TX frame
				NdisStatus	=	FrameALAALTxProcessBuf
				             (
				                 // VC context of data stream
				                 VcCtx,
				                 // Buffer descriptor to be processed
				                 AALVcCtx->AALxTxBfr
				             );

				// Was the TX frame successfully processed?
				if ( NdisStatus != STATUS_SUCCESS )
				{
					// The TX frame could not be processed by the
					// AALx processing function for some reason

					// Because NdisStatus indicated AAL frame processing was
					// unsuccessful, the while condition at the bottom of this loop
					// will cause us to go back to Buffer Management again to get
					// another buffer to try and process.

					// Return the TX frame back to Buffer Management
					// since we cannot do anything more with it

					AALVcCtx->AALxTxBfr->data = AALVcCtx->AALxTxBfrPtr;
					AALVcCtx->AALxTxBfr->len = AALVcCtx->AALxTxBfrLength;

					BufMgmtTxPutEmptyBuf
					(
					    // VC context of data stream
					    VcCtx,
					    // TX frame we could not process
					    AALVcCtx->AALxTxBfr
					);

				}
				else
				{
					if ( AALVcCtx->RemainingBufferLength )
						AALVcCtx->AALxTx48BytePtr = AALVcCtx->AALxTxBfr->data;
					else
						AALVcCtx->AALxTx48BytePtr = AALVcCtx->ExtraPad;

					// The TX frame was successfully processed by the AALx
					// process so the Ndis status success indication will
					// break us out of the do-while loop.
				}
			}
		}while( NdisStatus != STATUS_SUCCESS );
	}

	// Do we have a TX frame to segment?
	// AND we have not reached the end of the buffer?

	// NOTE: The second condition is present to keep from
	// going past the end of the buffer if for some reason
	// the FrameALAALTxPutEmptyBuf function is never called.

	if(	AALVcCtx->AALxTxBfr != NULL	)
	{
		// There IS a TX frame currently in the segmenting process
		//Initialize the buffer descriptor to be returned to ATM
		BufDescPtr->data = AALVcCtx->AALxTx48BytePtr;
		BufDescPtr->len  = CELL_PAYLOAD_SIZE;
		BufDescPtr->head = AALVcCtx->AALxTx48BytePtr;
		BufDescPtr->end  = AALVcCtx->AALxTx48BytePtr + CELL_PAYLOAD_SIZE;

		// Increment the 48 byte cell pointer to
		// the beginning of the next 48 byte cell

		AALVcCtx->AALxTx48BytePtr += CELL_PAYLOAD_SIZE;

		if ( AALVcCtx->RemainingBufferLength )
		{
			AALVcCtx->RemainingBufferLength -= CELL_PAYLOAD_SIZE;

			// if at the end of the buffer see if we have extra pad blocks to send
			if ( !AALVcCtx->RemainingBufferLength )
			{
				AALVcCtx->AALxTx48BytePtr = AALVcCtx->ExtraPad;
			}
		}
		else if ( AALVcCtx->RemainingPadLength )
		{
			AALVcCtx->RemainingPadLength -= CELL_PAYLOAD_SIZE;
		}
		else
		{
			printk("<1>CnxADSL trying to send when I don't have buffer length \n");
		}

		// Check to see if we are at the end of the TX frame
		// If so then this was the last 48 byte cell of the TX frame
		// and the AUU bit field information needs to be set accordingly

		if ( AALVcCtx->RemainingBufferLength || AALVcCtx->RemainingPadLength )
		{
			// This IS NOT the last 48 byte cell of the
			// frame so clear the AUU bit
			OOB_DATA(BufDescPtr) = OOB_DATA(BufDescPtr) & ~OOD_AUU ;
		}
		else
		{
			// This IS the last 48 byte cell of the frame
			// frame so set the AUU bit
			OOB_DATA(BufDescPtr) = OOB_DATA(BufDescPtr) | OOD_AUU ;
		}

		// NOTE: The following methods of setting/clearing the CI
		//		 bit field is not a preferred embodiment, but for the sake
		//		 of expidiency it will suffice for now.

		// In the CO driver we get CLP data from NDIS
		if ( OOBDATA_CLP(AALVcCtx->AALxTxBfr) != 0 )
		{
			OOB_DATA(BufDescPtr) = OOB_DATA(BufDescPtr) |  OOD_CLP ;
		}
		else
		{
			OOB_DATA(BufDescPtr) = OOB_DATA(BufDescPtr) & ~OOD_CLP ;
		}

		// Set the Congestion Indication bit field
		if ( CONGESTION_IND )
		{
			OOB_DATA(BufDescPtr) = OOB_DATA(BufDescPtr) |  OOD_CI ;
		}
		else
		{
			OOB_DATA(BufDescPtr) = OOB_DATA(BufDescPtr) & ~OOD_CI ;
		}

	}
	return BufDescPtr;
}

/*******************************************************************************

FUNCTION NAME:
		FrameALAALTxPutEmptyBuf

ABSTRACT:
		Checks for the last 48 byte cell of the frame and returns it to Buffer
		Management if it is. 

DETAILS:
		This function is called by the ATM layer when it has completed transmission
		of the last 48 byte data fragment it obtained.   Upon entering this function
		the frame fragment pointer is checked to see if it points to the end of the
		frame data buffer.   If it does not, the function immediately returns.   If
		it does then the function BufMgmtTxPutEmptyBuf is called to return to Buffer
		Management the frame that has just been transmitted.   After returning from
		this call to Buffer Management, the buffer descriptor context for AAL TX path
		is initialized to NULL to indicate that there is no frame in process.   The
		function then returns to the ATM layer.

*******************************************************************************/

GLOBAL void FrameALAALTxPutEmptyBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be processed
)
{
	FUNCTION_ENTRY

	// Get the FrameAL VC context
	FRAMEAL_VC_T *FrameALVcCtx = VcCtx->FrameALVCMod;

	// Locate the AAL VC Context
	FRAMEAL_AAL_VC_T *AALVcCtx = &FrameALVcCtx->AALVcCtx;

	// Was this the last 48 byte cell of the TX frame?
	if
	(
	    AALVcCtx->AALxTxBfr != NULL &&
	    !AALVcCtx->RemainingBufferLength &&
	    !AALVcCtx->RemainingPadLength
	)
	{
		// This WAS the last 48 byte cell of the frame so
		// return the transmitted frame to Buffer Management

		// Restore the buffer descriptor current buffer and length back
		// to it's original values before returning it to buffer management

		AALVcCtx->AALxTxBfr->data = AALVcCtx->AALxTxBfrPtr;
		AALVcCtx->AALxTxBfr->len = AALVcCtx->AALxTxBfrLength;

		BufMgmtTxPutEmptyBuf
		(
		    // VC context of data stream
		    VcCtx,
		    // TX frame we finished processing
		    AALVcCtx->AALxTxBfr
		);

		// Indicate that we do not have a TX frame
		// to segment anymore
		AALVcCtx->AALxTxBfr = NULL;
		AALVcCtx->AALxTx48BytePtr = NULL;

		BufDescPtr->data = NULL;
		BufDescPtr->len = 0;
	}
	return;
}

/*******************************************************************************

FUNCTION NAME:
		FrameALStartTx

ABSTRACT:
		"Kick starts" the transmitter 
DETAILS:
		This function is called by Buffer Management when the transmit queues
		have gone from empty to non-empty.   AAL in turn calls the function
		ATMStartTx to inform it of this occurrence.   Upon returning from the
		call to ATM this function returns to Buffer Management.

*******************************************************************************/

GLOBAL void FrameALStartTx
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
)
{
	FUNCTION_ENTRY
	FrameALVCStartTX( VcCtx );
	return;
}

// *********************************
// AAL RX functions
// *********************************


/*******************************************************************************

FUNCTION NAME:
		FrameALAALRxProcessBuf

ABSTRACT:
		Applies AALx processing to the received data frames from SAR  

DETAILS:
		Even though the function name implies multiple AAL type processing, the
		initial implementation of this function will only support AAL5.  If the link
		is configured for any other	AAL type the function will return an
		STATUS_FAILURE response.

		AAL5 PROCESSING
		This function is called by the SAR module when it has completed reassembly
		on an AAL frame.   SAR will pass to this function the current link context
		pointer and a non-NULL pointer to an Athena buffer descriptor containing the
		AAL5 format encapsulated frame to be processed.  The first processing step
		to be performed is verification of the length of the AAL frame.   To accomplish
		this the length of the data payload of the AAL5 frame, as indicated by the
		length field of the AAL5 PDU trailer, will be subtracted from the length of the
		received AAL5 frame, as indicated in the length field of the Athena buffer
		descriptor.    If the difference between them is between 8 and 55 bytes inclusive,
		the frame length is presumed to be valid and a CRC32 value will be calculated on
		the frame. The limits of 8 and 55 come from there being at least a minimum of 8
		additional bytes (which is the AAL5 trailer) up to a maximum of 55 bytes (which
		is the AAL5 trailer	plus 47 bytes padding) that can be added to the original length
		of the payload.  To calculate the CRC the function FrameALAALCalcCRC32 is called passing
		to it a pointer	to the Athena buffer descriptor containing the AAL5 frame being
		operated upon.  The return value from FrameALAALCalcCRC32 function is then compared with
		the "constant" CRC32 value of 0xdebb20e3.   This "constant" value will always be
		the result of the CRC32 calculation when the CRC32 field of the frame is included
		in the CRC32 calculation on a frame that had no bit errors.

		If the 8/55 byte difference check, CPI field validation and the CRC32 check pass,
		then the length	field of the Athena buffer descriptor is set to the length value
		indicated in the length field of the AAL5 PDU trailer.  The function
		BufMgmtRxPutFullBuf is then	called to deliver the reassembled AAL5 frame to
		Buffer Management. The appropriate good frame counters located	in the link context
		are updated and FrameALAALRxProcessBuf	returns to the SAR module with an NDIS status
		indicating success.

		If the 8/55 byte difference, CPI field validation or the CRC32 checks fail,
		AALRxPutFullBuf	updates the appropriate bad frame counters located in the link
		context and returns	to the SAR module with an NDIS status indicating failure.

*******************************************************************************/

LOCAL NDIS_STATUS FrameALAALRxProcessBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be processed
)
{

	FUNCTION_ENTRY

	DWORD		TrailerIndex;
	BYTE		UserToUserIndication;
	BYTE		CommonPartIndicator;
	WORD		Payload_Len = 0;
	DWORD		crc32_val;

	// Get the FrameAL VC context
	FRAMEAL_VC_T *FrameALVcCtx = VcCtx->FrameALVCMod;

	// Get the VC Parameters
	FRAMEAL_VC_PARAMETERS FrameALVcParms = FrameALVcCtx->Parameters;

	// Get the Frame Link context
	FRAMEAL_LINK_T *FrameALLinkCtx = ((CDSL_LINK_T *) (VcCtx->LinkHdl))->FrameMod;

	// Get the AAL Link context
	FRAMEAL_AAL_LINK_T *AALLinkCtx = &FrameALLinkCtx->AALLinkBlock;

	// Locate the AAL VC Context
	FRAMEAL_AAL_VC_T *AALVcCtx = &FrameALVcCtx->AALVcCtx;

	// Determine what AAL type we are
	switch( FrameALVcParms.AALType )
	{
	case ATM_AAL5:

		// AAL5 FRAME VERIFICATION

		// Verify the AAL5 frame CRC
		crc32_val = FrameALAALCalcCRC32( BufDescPtr->data,
		                                 BufDescPtr->len,
		                                 INITIAL_CRC_VAL );
		if ( crc32_val != GOODCRC32 )
		{
			// AAL5 frame CRC was invalid

			// Update the AAL per VC stats
			AddToCtrLONG(&AALVcCtx->AALRxInvalidCRC, 1);
			AddToCtrLONG(&AALVcCtx->AALRxDiscarded, 1);
			AddToCtrLONG(&AALVcCtx->AALRxDiscardedByte, (LONG)BufDescPtr->len);

			// Update the AAL per Link stats
			AddToCtrLONG(&AALLinkCtx->AALRxInvalidCRC, 1);
			AddToCtrLONG(&AALLinkCtx->AALRxDiscarded, 1);
			AddToCtrLONG(&AALLinkCtx->AALRxDiscardedByte, (LONG)BufDescPtr->len);

			// Update Linux ATM stats
			atomic_inc(&VcCtx->pVcc->dev->stats.aal5.rx_err);
			return(STATUS_FAILURE);
		}

		// Temp index variable to step us through the AAL5 trailer
		TrailerIndex = BufDescPtr->len - AAL5_TRLR_SIZE;

		// Extract the CPCS-UU field
		UserToUserIndication = BufDescPtr->data[ TrailerIndex++ ];

		// Extract the CPI field
		CommonPartIndicator = BufDescPtr->data[ TrailerIndex++ ];

		// Verify the CPI field
		if ( AALVcCtx->CPI != CommonPartIndicator )
		{
			// Update the AAL per VC stats
			AddToCtrLONG(&AALVcCtx->AALRxDiscarded, 1);
			AddToCtrLONG(&AALVcCtx->AALRxDiscardedByte, (LONG)BufDescPtr->len);

			// Update the AAL per Link stats
			AddToCtrLONG(&AALLinkCtx->AALRxDiscarded, 1);
			AddToCtrLONG(&AALLinkCtx->AALRxDiscardedByte, (LONG)BufDescPtr->len);

			return(STATUS_FAILURE);
		}

		// Extract the payload length field in a manner that is
		// independent of the endian'ness of the machine.

		// NOTE: The payload length field of the AAL5 trailer is
		// stored MSB first.

		Payload_Len = Payload_Len | ((( (WORD) BufDescPtr->data[ TrailerIndex++ ]) << 8 ) & 0xff00 );
		Payload_Len = Payload_Len | (( (WORD) BufDescPtr->data[ TrailerIndex++ ]) & 0x00ff );

		// Verify that the payload length is not zero which indicates
		//  a user aborted frame.
		// AND
		// Verify the AAL5 frame length / payload field length
		//  difference to be between 8 and 55 bytes inclusive.
		if
		(
		    Payload_Len == 0
		    ||	(BufDescPtr->len - Payload_Len) < AAL5_TRLR_SIZE
		    ||	(BufDescPtr->len - Payload_Len) > AAL5_TRLR_SIZE + MAX_PADDING
		)
		{
			// AAL5 frame length was invalid

			// Update the AAL per VC stats
			AddToCtrLONG(&AALVcCtx->AALRxInvalidLen, 1);
			AddToCtrLONG(&AALVcCtx->AALRxDiscarded, 1);
			AddToCtrLONG(&AALVcCtx->AALRxDiscardedByte, (LONG)BufDescPtr->len);

			// Update the AAL per Link stats
			AddToCtrLONG(&AALLinkCtx->AALRxInvalidLen, 1);
			AddToCtrLONG(&AALLinkCtx->AALRxDiscarded, 1);
			AddToCtrLONG(&AALLinkCtx->AALRxDiscardedByte, (LONG)BufDescPtr->len);

			// Update Linux ATM stats
			atomic_inc(&VcCtx->pVcc->dev->stats.aal5.rx_err);

			return(STATUS_FAILURE);
		}

		if ( AALVcCtx->AALxRx48BytePtr >= AALVcCtx->AALxRxBfr->end )
		{
			// Reassembled AAL5 frame length was greater than Maximum reassembly frame size

			// Update the AAL per VC stats
			AddToCtrLONG(&AALVcCtx->AALRxInvalidLen, 1);
			AddToCtrLONG(&AALVcCtx->AALRxDiscarded, 1);
			AddToCtrLONG(&AALVcCtx->AALRxDiscardedByte, (LONG)BufDescPtr->len);

			// Update the AAL per Link stats
			AddToCtrLONG(&AALLinkCtx->AALRxInvalidLen, 1);
			AddToCtrLONG(&AALLinkCtx->AALRxDiscarded, 1);
			AddToCtrLONG(&AALLinkCtx->AALRxDiscardedByte, (LONG)BufDescPtr->len);

			// Update Linux ATM stats
			atomic_inc(&VcCtx->pVcc->dev->stats.aal5.rx_err);

			return(STATUS_FAILURE);
		}

		// AAL5 frame was valid

		// Put the UUI and CPI fields where BufMgmt for CO can get the data

		// NOTE: The UUI and CPI data put into the buffer descriptor fields
		// will only be used by a CO driver.

		OOBDATA_UUI(BufDescPtr)	= UserToUserIndication;
		OOBDATA_CPI(BufDescPtr) = CommonPartIndicator;

		// Update the AAL per VC stats
		AddToCtrLONG(&AALVcCtx->AALRxGood, 1);
		AddToCtrLONG(&AALVcCtx->AALRxGoodByte, (LONG)BufDescPtr->len);

		// Update the AAL per Link stats
		AddToCtrLONG(&AALLinkCtx->AALRxGood, 1);
		AddToCtrLONG(&AALLinkCtx->AALRxGoodByte, (LONG)BufDescPtr->len);

		// Update Linux ATM stats
		atomic_inc(&VcCtx->pVcc->dev->stats.aal5.rx);

		// Update the buffer descriptor pointer	to
		// indicate only the length of the payload
		BufDescPtr->len = Payload_Len;

		// Always return success for the CO driver
		return STATUS_SUCCESS;

		break;

	default:
		// Any other AAL type is currently invalid

		// Update the AAL per VC stats
		AddToCtrLONG(&AALVcCtx->AALRxDiscarded, 1);
		AddToCtrLONG(&AALVcCtx->AALRxDiscardedByte, (LONG)BufDescPtr->len);

		// Update the AAL per Link stats
		AddToCtrLONG(&AALLinkCtx->AALRxDiscarded, 1);
		AddToCtrLONG(&AALLinkCtx->AALRxDiscardedByte, (LONG)BufDescPtr->len);

		return STATUS_FAILURE;
		break;
	}
}

/*******************************************************************************

FUNCTION NAME:
		FrameALAALRxPutFullBuf

ABSTRACT:
		Reassembles the 48 byte cells from ATM into AAL frames

DETAILS:
		This function is called by the ATM layer when it has a 48 byte cell for
		AAL to reassemble into an AAL frame. The first phase upon entering this
		function will be to check if there is currently a frame from Buffer
		Management that an AAL frame can be reassembled in.   It does this by
		checking the RX path buffer descriptor of the AAL VC context structure
		for a non-NULL value.   If the buffer descriptor is NULL a call to
		BufMgmtRxGetEmptyBuf is made to obtain an empty buffer for re-assembly
		purposes.   If Buffer Management does not have an empty buffer will return
		a NULL pointer in which case FrameALAALRxPutFullBuf will set it's VC context
		structure buffer descriptor variables to NULL and return to the ATM layer.
		If the call to BufMgmtRxGetEmptyBuf returns a buffer, the AAL RX path buffer
		descriptor is updated with the Athena Buffer Descriptor information returned
		from Buffer Management and the 48 byte fragment pointer is intialized to the
		value contained in the element data.

		At this point the function has an Athena Buffer Descriptor available to it
		for re-assembly of 48 byte cells into AAL frames.   For AAL5 frames, the 48
		byte cell data from ATM is copied into the re-assembly frame data buffer using
		the 48 byte fragment pointer, the 48 byte fragment pointer updated to the
		new value and the buffer length field updated with another 48 byte increment.
		The Cell Loss and Congestion Indication bit fields are extracted from the
		Out-of-Band field and stored temporarily in local variables for future use.

		The AUU bit field for the cell is extracted from the Out-of-Band field and checked
		for this to be the last cell of the frame.  If it is not the last cell (AUU=0) then
		the function FrameALAALStartRASTimer is called to start/reset the re-assembly timer and
		the function returns to ATM.

		If either the last cell of the frame has been received (AUU=1) or the size limit
		of the RX buffer has been reached the function FrameALAALStopRASTimer is called to stop the
		re-assembly timer and the function FrameALAALRxProcessBuf is called to validate the frame.

		If the status returned from FrameALAALRxProcessBuf indicates failure then the 48 byte
		fragment pointer is reset back to the beginning of the data buffer, the appropriate
		error statistics logged and the function returns to ATM.
		
		If the status returned from FrameALAALRxProcessBuf indicates successful RX frame processing
		and delivery to Buffer Management then VC context structure buffer descriptor
		variables are reset to NULL, the 48 byte fragment pointer is set to NULL and the
		function returns to the ATM layer.

*******************************************************************************/

GLOBAL void FrameALAALRxPutFullBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be processed
)
{
	FUNCTION_ENTRY

	NDIS_STATUS NdisStatus;
	BOOL		cell_loss = FALSE;
	BOOL		congestion_indication = FALSE;
	DWORD		OutOfBandData;

	// Get the FrameAL VC context
	FRAMEAL_VC_T *FrameALVcCtx = VcCtx->FrameALVCMod;

	// Get the AAL VC Context
	FRAMEAL_AAL_VC_T *AALVcCtx = &FrameALVcCtx->AALVcCtx;

	// Get the Frame Link context
	FRAMEAL_LINK_T *FrameALLinkCtx = ((CDSL_LINK_T *) (VcCtx->LinkHdl))->FrameMod;

	// Get the AAL Link context
	FRAMEAL_AAL_LINK_T *AALLinkCtx = &FrameALLinkCtx->AALLinkBlock;

	// Check for a reassembly RX buffer currently in use
	if ( AALVcCtx->AALxRxBfr == NULL )
	{
		// There IS NOT a RX buffer currently available for
		// reassembly so try to get one from Buffer Management
		AALVcCtx->AALxRxBfr = 	BufMgmtRxGetEmptyBuf
		                       (
		                           // VC context of data stream
		                           VcCtx
		                       );

		// Did Buffer Management have an RX buffer for us?
		if ( AALVcCtx->AALxRxBfr == NULL )
		{
			// Buffer Management DID NOT have an empty buffer for us to use

			// so reset the RX buffer pointers

			AALVcCtx->AALxRx48BytePtr = NULL;

			// Count number of times for NDIS General OID
			AddToCtrLONG(&AALLinkCtx->AALRxNoBufferAvail, 1);

			// Update Linux ATM stats
			atomic_inc(&VcCtx->pVcc->dev->stats.aal5.rx_drop);

			// Return to ATM without doing anything with the buffer data
			// !!! PROBABLY NEED TO LOG THIS AS AN ERROR CONDITION
			return;
		}
		else
		{
			// Buffer Management DID have an empty buffer for us to use
			// so initialize the RX buffer pointers appropriately.

			AALVcCtx->AALxRx48BytePtr = AALVcCtx->AALxRxBfr->data;
			AALVcCtx->AALxRxBfr->len = 0;
		}
	}

	// We have an RX buffer that is currently available for our use
	// so copy the data from the ATM cell into our reassembly frame

	// NOTE: ATM can give us buffer descriptors that have payload data
	// lengths that are less than 48 bytes.  ATM will do this when the
	// physical layer DMA buffer wraps.  In this situation ATM will give
	// us the 48 byte payload data in two separate pieces.   ATM will use
	// the bit OOD_END_CELL in the out of band data field of the buffer
	// descriptor to indicate when it contains a complete cell or the
	// last partial piece of a cell.

	// Will the new 48 byte segment fit within the remaining buffer space?
	if ( AALVcCtx->AALxRx48BytePtr + BufDescPtr->len <= AALVcCtx->AALxRxBfr->end )
	{
		// The new 48 byte data segment will fit within the remaining buffer space.

		COPY_MEMORY
		(
		    AALVcCtx->AALxRx48BytePtr,		// Destination
		    BufDescPtr->data,		// Source
		    BufDescPtr->len		// Length
		);

		// Update the RX buffer 48 byte cell segment pointer and
		// RX buffer length with the value of however much data
		// we were given by ATM in the buffer descriptor.

		AALVcCtx->AALxRx48BytePtr += BufDescPtr->len;
		AALVcCtx->AALxRxBfr->len += BufDescPtr->len;

		OutOfBandData = OOB_DATA(BufDescPtr);

		// 	  Do we have a complete cell to examine?
		// OR have we reached the end of the RX data buffer?

		if(	OOD_END_CELL == (OutOfBandData & OOD_END_CELL))
		{
			// Extract the Cell Loss and Congestion Indication bit fields

			// NOTE: The Cell Loss and Congestion Indication bit field values
			// 		 retrieved from the ATM header are currently not used in any
			//		 useful manner.   But in case the information in them is ever
			// 		 determined to be of some use, here is where you would hook
			// 		 into the code to get them.

			// NOTE: The following methods of retrieving/checking the CLP, CI and AUU
			//		 bit fields is not necessarily a preferred embodiment, but for the
			//		 sake of expidiency it will suffice for now.

			if ( OOD_CLP == (OutOfBandData & OOD_CLP) )
			{
				cell_loss = TRUE;
			}

			if ( OOD_CI == (OutOfBandData & OOD_CI) )
			{
				congestion_indication = TRUE;
			}

			// Check if this was either the last 48 byte cell of the frame
			// or the RX buffer size limit has been reached.  If so then
			// go process the reassembled AAL frame

			if
			(
			    OOD_AUU == ( OutOfBandData & OOD_AUU )
			    ||	AALVcCtx->AALxRx48BytePtr >= AALVcCtx->AALxRxBfr->end
			)
			{
				// This IS the last 48 byte cell of the RX frame AUU bit = 1
				// or the maximum size of the reassembly buffer HAS been reached

				// Stop the inter-cell reassembly timer
				FrameALAALStopRASTimer( VcCtx );

				// Now apply the AALx processing to the RX frame
				NdisStatus	=	FrameALAALRxProcessBuf
				             (
				                 // VC context of data stream
				                 VcCtx,
				                 // Buffer descriptor to be processed
				                 AALVcCtx->AALxRxBfr
				             );

				// Was the RX frame successfully processed?
				if ( NdisStatus == STATUS_SUCCESS )
				{
					// The reassembly buffer has been successfully processed so return it to
					// Buffer Management and indicate that we no longer have a reassembly
					// buffer to use
					BufMgmtRxPutFullBuf
					(
					    // VC context of data stream
					    VcCtx,
					    // RX frame we finished reassembling
					    AALVcCtx->AALxRxBfr
					);

					AALVcCtx->AALxRxBfr = NULL;
					AALVcCtx->AALxRx48BytePtr = NULL;
				}
				else
				{
					// The RX frame WAS NOT successfully processed by the AALx RX
					// processing function so flush the current reassembled frame

					AALVcCtx->AALxRx48BytePtr = AALVcCtx->AALxRxBfr->data;
					AALVcCtx->AALxRxBfr->len = 0;
				}
			}
			else
			{
				// This IS NOT the last 48 byte cell of the frame
				// AUU bit = 0

				// Reset the inter-cell reassembly timer
				FrameALAALStartRASTimer( VcCtx );
			}
		}
	}
	else
	{
		// The RX frame has exceeded the length of the reassembly
		//  buffer so flush the current reassembled frame.
		AALVcCtx->AALxRx48BytePtr = AALVcCtx->AALxRxBfr->data;
		AALVcCtx->AALxRxBfr->len = 0;

		// AAL5 frame length was invalid

		// Update the AAL per VC stats
		AddToCtrLONG(&AALVcCtx->AALRxInvalidLen, 1);
		AddToCtrLONG(&AALVcCtx->AALRxDiscarded, 1);
		AddToCtrLONG(&AALVcCtx->AALRxDiscardedByte, (LONG)BufDescPtr->len);

		// Update the AAL per Link stats
		AddToCtrLONG(&AALLinkCtx->AALRxInvalidLen, 1);
		AddToCtrLONG(&AALLinkCtx->AALRxDiscarded, 1);
		AddToCtrLONG(&AALLinkCtx->AALRxDiscardedByte, (LONG)BufDescPtr->len);
	}
	return;
}

// END OF FrameALAAL.C
