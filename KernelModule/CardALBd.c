/******************************************************************************
*******************************************************************************
****	Copyright (c) 1997, 1998, 1999
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
**		CardAL (Card Abstraction Layer)
**
**	FILE NAME:
**		CardALBd.c
**
**	ABSTRACT:
**		Card level Abstraction Layer for CDSL.  This file/module abstracts
**		(hides the details of) card specific (CDSL) functions and presents
**		(hopefully) a constant interface to the upper level modules/functions
**		(CardMgmt) no matter what the actual card is (CDSL, ADSL, ISDN, etc.).
**		This file provides the functions that process the BackDoor requests
**		from an application to interface to the CardAL module.
**
**	DETAILS:
**		Functions are arranged in top-down order.
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CardALBd.c $
** $Revision: 1 $
** $Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/
#define		MODULE_ID			DBG_ENABLE_CARDAL


#include "CardMgmt.h"
#include "CardMgmtV.h"
#define		CARDAL_DECLARE_PUBLIC			0
#include "CardALV.h"
#undef		CARDAL_DECLARE_PUBLIC
#include "EEType.h"

#if		PROJECTS_NAME == PROJECT_NAME_TIGRIS
 #include "CardALBdDp.h"
#endif

#define ERROR_FATAL(error)		(LEVEL_FATAL   | MODULE_CARDAL | INSTANCE_CARDAL | error)
#define ERROR_WARNING(error)	(LEVEL_WARNING | MODULE_CARDAL | INSTANCE_CARDAL | error)
#define ERROR_NORMAL(error)		(LEVEL_NORMAL  | MODULE_CARDAL | INSTANCE_CARDAL | error)

//	Unique file ID for error logging
#define		__FILEID__			7




///////////////////////////////////////////////////////////////////////////////
//	Defines
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//	Variable Declarations
///////////////////////////////////////////////////////////////////////////////

BD_DRIVER_LOG_T		BdAdslTextLog;




///////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
///////////////////////////////////////////////////////////////////////////////


//This function is also called by other files in the CardAL module.
//void cardALBdDeviceSpecific(
//	IN CDSL_ADAPTER_T				* pThisAdapter,
//	IN BACK_DOOR_T					* pBackDoorBuf,
//	OUT PULONG						pNumBytesNeeded,
//	OUT PULONG						pNumBytesWritten );

static DWORD cardALBdAdslGetDpVersions(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    I_O BD_ADSL_DP_VERSIONS_T		* pBdAdslDpVersStruc );

static DWORD cardALBdAdslDnldCodeToDP(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    I_O BD_ADSL_INIT_DP_T		* pBdAdslInitDPStruc );

static DWORD cardALBdAdslGetConnConfig(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    I_O BD_ADSL_CONN_CONFIG_T		* pBdAdslConnConfig );

static DWORD cardALBdAdslSetConnConfig(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    I_O BD_ADSL_CONN_CONFIG_T		* pBdAdslConnConfig );

static DWORD cardALBdAdslReadModemData(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    I_O BD_ADSL_MODEM_DATA_T	* pBdAdslModemData);

static DWORD cardALBdAdslGetPathSel(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    I_O BD_ADSL_DATA_PATH_SELECT_T		* pBdAdslPathSelect);

static DWORD cardALBdAdslSetPathSel(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    I_O BD_ADSL_DATA_PATH_SELECT_T		* pBdAdslPathSelect);

static DWORD cardALBdAdslGetLineStatus(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    I_O BD_ADSL_LINE_STATUS_T			* pBdAdslLineStatus);

static DWORD cardALBdAdslGetLineState(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    I_O BD_ADSL_LINE_STATE_T			* pBdAdslLineState);

static DWORD cardALBdPeekReg(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    I_O BD_CARDAL_REGISTER_T		* pBdCardALReg );

static DWORD cardALBdPokeReg(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    I_O BD_CARDAL_REGISTER_T		* pBdCardALReg );

static DWORD cardALBdAdslUserActivateLine(	IN CDSL_ADAPTER_T		* pThisAdapter );

static DWORD cardALBdAdslUserDeactivateLine(	IN CDSL_ADAPTER_T		* pThisAdapter );

static DWORD cardALBdGetAdslTransceiverStatus(
    IN CDSL_ADAPTER_T						* pThisAdapter,
    I_O BD_CARDAL_ADSL_TRANSCEIVER_STATUS_T	* pBdAdslTransceiverStatus );

static DWORD cardALBdAdslGetConfig(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    I_O BD_ADSL_CFG_T		* pBdAdslConfig );

static DWORD cardALBdAdslSetConfig(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    I_O BD_ADSL_CFG_T		* pBdAdslConfig );

static DWORD cardALBdAdslGetTextLogMsg(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    I_O BD_MINI_DATA_BLOCK_DWORDS_T		pBdAdslTextLogs );



///////////////////////////////////////////////////////////////////////////////
//	Functions
///////////////////////////////////////////////////////////////////////////////


/******************************************************************************
*	FUNCTION NAME:
*		cardALBdDeviceSpecific
*
*	ABSTRACT:
*		This function coordinates all of the TAPI Device Specific (BACKDOOR)
*		actions that are performed for CardAL (Card Abstraction Layer) module.
*
*	DETAILS:
*		EVERY MODULE WHICH SUPPORTS *ANY* BACKDOOR ACTION
*		(EVEN ONE) *MUST* IMPLEMENT THIS FUNCTION
*		IN AT LEAST A SIMILAR MANNER OR FASHION!!!!!!!!
*
*		BACKDOOR CODE/SUBCODE VALUES *WILL BE DESIGNED*
*		TO **GUARANTEE** THAT EACH CODE/SUBCODE VALUE
*		WILL BE DIRECTED TO *ONLY ONE* MODULE!!
*
*		NOTE:	The BackDoor Status should ONLY BE SET TO FAILURE
*				at the beginning of BackDoor parsing by the first
*				OID parser (CardMgmt)!!!
*				ALL OTHER successor OID parsing of BackDoor should
*				only set the BackDoor Status when it is intended
*				to be set to a specific Status (i.e., BAD_SIGNATURE,
*				FAILURE, or SUCCESS)!!  The BackDoor Status should
*				not be set at the beginning of BackDoor parsing
*				intending to change it if needed!!!
*				This is to keep other modules (BackDoor parsing)
*				from overwriting the BackDoor status once a module
*				has parsed the BackDoor structure and acted upon
*				the data!!!
*				If the BackDoor Status is not set to CODE_NOT_SUPPORTED,
*				then parsing can be aborted/stopped since some other
*				BackDoor parser (module) has parsed the BackDoor
*				Code request and indicated an error condition or
*				acted upon the request.
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
*******************************************************************************/
NDIS_STATUS CardALBdDeviceSpecific(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    IN BACK_DOOR_T					* pBackDoorBuf,
    OUT PULONG						pNumBytesNeeded,
    OUT PULONG						pNumBytesWritten )
{
	NDIS_STATUS				Status=STATUS_SUCCESS;
	DWORD					TmpNumBytesNeeded;
	DWORD					StrSize = 0;


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
			    (	"cardALDeviceSpecific=BACK_DOOR:   Entry, Size Failure;  RC=0x%08lX;  NS=0x%08lX;  TS=0x%08lX.",
			      pBackDoorBuf->ReqCode,
			      pBackDoorBuf->NeededSize,
			      pBackDoorBuf->TotalSize) );
			return ERROR_WARNING( RESULT_REQ_BUFF_TOO_SMALL );
		}
	}

	TmpNumBytesNeeded = pBackDoorBuf->TotalSize;

	//#if 0	//only during development or problems
	DBG_CDSL_DISPLAY(
	    DBG_LVL_MISC_INFO,
	    pThisAdapter->DebugFlag,
	    (	"CardAL=BACK_DOOR:  Entry; ReqCode=0x%08lX.",
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

#if		PROJECTS_MEDIUM == PROJECT_MEDIUM_ADSL
	case BD_ADSL_INIT_DATA_PUMP:
		pBackDoorBuf->ResultCode = cardALBdAdslDnldCodeToDP(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.BdAdslInitDataPump );
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_ADSL_GET_DP_VERSIONS:
		pBackDoorBuf->ResultCode = cardALBdAdslGetDpVersions(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.BdAdslDpVersions );
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdAdslDpVersions )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_ADSL_GET_CONN_CONFIG:
		pBackDoorBuf->ResultCode = cardALBdAdslGetConnConfig(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.BdAdslConnConfig );
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdAdslConnConfig )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_ADSL_SET_CONN_CONFIG:
		pBackDoorBuf->ResultCode = cardALBdAdslSetConnConfig(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.BdAdslConnConfig );
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdAdslConnConfig )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_ADSL_GET_MODEM_DATA:
		pBackDoorBuf->ResultCode = cardALBdAdslReadModemData(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.BdAdslModemBlock);
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdAdslModemBlock )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_ADSL_GET_DATA_PATH_SELECTION:
		pBackDoorBuf->ResultCode = cardALBdAdslGetPathSel(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.BdAdslDataPathSelect);
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdAdslDataPathSelect )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_ADSL_SET_DATA_PATH_SELECTION:
		pBackDoorBuf->ResultCode = cardALBdAdslSetPathSel(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.BdAdslDataPathSelect);
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdAdslDataPathSelect )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_ADSL_GET_LINE_STATUS:
		pBackDoorBuf->ResultCode = cardALBdAdslGetLineStatus(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.BdAdslLineStatus);
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdAdslLineStatus )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_ADSL_GET_LINE_STATE:
		pBackDoorBuf->ResultCode = cardALBdAdslGetLineState(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.BdAdslLineState);
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdAdslLineState )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

#endif	//	PROJECTS_MEDIUM == PROJECT_MEDIUM_ADSL
	case BD_CARDAL_GET_REGISTER:
		pBackDoorBuf->ResultCode = cardALBdPeekReg(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.BdCardALRegister );
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdCardALRegister )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CARDAL_SET_REGISTER:
		pBackDoorBuf->ResultCode = cardALBdPokeReg(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.BdCardALRegister );
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdCardALRegister )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_USER_ACTIVATE_LINE:
		pBackDoorBuf->ResultCode = cardALBdAdslUserActivateLine( pThisAdapter );
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_USER_DEACTIVATE_LINE:
		pBackDoorBuf->ResultCode = cardALBdAdslUserDeactivateLine( pThisAdapter );
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_ADSL_GET_BUS_CONTROLLER:
		// **## FIX "Unknown" ##**//
		//			StrSize = strlen( BD_BUS_CTRLR_BASIC_2p1 ) + 1;		// "BASIC 2.1"
		//			StrSize = strlen( BD_BUS_CTRLR_BASIC_2p15 ) + 1;	// "BASIC 2.15"
		StrSize = strlen( BD_BUS_CTRLR_UNKNOWN ) + 1;
		if ( StrSize > sizeof( BD_MINI_DATA_BLOCK_T ) )
		{
			StrSize = sizeof( BD_MINI_DATA_BLOCK_T ) - 1;
			pBackDoorBuf->Params.BdCardMgmtProdVer[StrSize] = 0;
			COPY_MEMORY(
			    ((PUCHAR) &pBackDoorBuf->Params.BdAdslBusController),
			    BD_BUS_CTRLR_UNKNOWN,
			    StrSize );
			pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_DATA_FAILURE );
		}
		else
		{
			COPY_MEMORY(
			    ((PUCHAR) &pBackDoorBuf->Params.BdAdslBusController),
			    BD_BUS_CTRLR_UNKNOWN,
			    StrSize );
			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		}
		//			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	StrSize
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_PM_SET_POWER_STATE:
		pBackDoorBuf->ResultCode = cardALChipSetPmState(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.PMPowerState);

		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.PMPowerState )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_PM_GET_POWER_STATE:
		pBackDoorBuf->ResultCode = cardALChipGetPmState(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.PMPowerState);

		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.PMPowerState )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CARDAL_GET_ADSL_TRANSCEIVER_STATUS:
		pBackDoorBuf->ResultCode = cardALBdGetAdslTransceiverStatus(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.AdslTransceiverStatus);

		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.AdslTransceiverStatus )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_ADSL_GET_CONFIG:
		pBackDoorBuf->ResultCode = cardALBdAdslGetConfig(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.BdAdslConfig );
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdAdslConfig )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_ADSL_SET_CONFIG:
		pBackDoorBuf->ResultCode = cardALBdAdslSetConfig(
		                               pThisAdapter,
		                               &pBackDoorBuf->Params.BdAdslConfig );
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdAdslConfig )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CARDAL_GET_TEXT_LOG:
		pBackDoorBuf->ResultCode = cardALBdAdslGetTextLogMsg(
		                               pThisAdapter,
		                               pBackDoorBuf->Params.BdCardALTextLog );
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;
		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdCardALTextLog )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	default:
		Status = STATUS_UNSUPPORTED;
		break;
	}

	return Status;
}



/******************************************************************************
*	FUNCTION NAME:
*		cardALBdAdslGetDpVersions
*
*	ABSTRACT:
*		Function for performing the BackDoor ADSL Read Data Pump Versions
*		parameters.  This is accomplished by reading the specific
*		register(s) for each ADSL DP version parameter.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/
static DWORD cardALBdAdslGetDpVersions(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    I_O BD_ADSL_DP_VERSIONS_T		* pBdAdslDpVersStruc )
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	pBdAdslDpVersStruc->AdslVersionNear = UNKNOWN_8BIT_VALUE;
	pBdAdslDpVersStruc->AdslVersionFar = UNKNOWN_8BIT_VALUE;
	pBdAdslDpVersStruc->AdslVendorNear = UNKNOWN_16BIT_VALUE;
	pBdAdslDpVersStruc->AdslVendorFar = UNKNOWN_16BIT_VALUE;
	pBdAdslDpVersStruc->AdslDpSwVerMajor = UNKNOWN_4BIT_VALUE;
	pBdAdslDpVersStruc->AdslDpSwVerMinor = UNKNOWN_4BIT_VALUE;
	pBdAdslDpVersStruc->AdslDpSwVerSubMinor = UNKNOWN_8BIT_VALUE;

	RtnResultValue = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		RtnResultValue = cardALChipGetDpVersions(
		                     pThisAdapter,
		                     pCardALAdapter,
		                     pBdAdslDpVersStruc );
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_ERROR,
		    pThisAdapter->DebugFlag,
		    ("cardALBdAdslGetDpVersions: ERROR;  NULL Structures!\n") );
	}

	return RtnResultValue;
}





/******************************************************************************
*	FUNCTION NAME:
*		cardALBdAdslDnldCodeToDP
*
*	ABSTRACT:
*		Function for downloading the Data Pump (DP) code from memory to the DP.
*
*	RETURN:
*
*	DETAILS:
******************************************************************************/
static DWORD cardALBdAdslDnldCodeToDP(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    I_O BD_ADSL_INIT_DP_T		* pBdAdslInitDPStruc )
{
	DWORD					RtnResultValue=STATUS_FAILURE;

	UtilTmrClearTmr(
	    &(pThisAdapter->TimerBlock),
	    pThisAdapter,
	    thPersistentActivation );
	/* not supported on Linux
		RtnResultValue = cardALChipDnldCodeToDP( pThisAdapter );
	*/
	return RtnResultValue;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALBdAdslGetConnConfig
*
*	ABSTRACT:
*		Function for performing the BackDoor ADSL Read Connection Configuration
*		parameters.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/
static DWORD cardALBdAdslGetConnConfig(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    I_O BD_ADSL_CONN_CONFIG_T		* pBdAdslConnConfig )
{
	DWORD							RtnResultValue;
	DWORD							VcIndex;

	RtnResultValue = RESULT_SUCCESS;
	if ( DEFAULT_NUMBER_OF_LINES <= MAX_VC_PER_LINK )
	{
		pBdAdslConnConfig->NumOfVc = DEFAULT_NUMBER_OF_LINES;
		if ( _MAX_LINKS == 1 )
		{
			for
			(
			    VcIndex = 0;
			    (
			        (VcIndex < DEFAULT_NUMBER_OF_LINES)
			        &&
			        (RtnResultValue == RESULT_SUCCESS)
			    );
			    VcIndex++
			)
			{
				pBdAdslConnConfig->VcArray[VcIndex].Vpi =
				    pThisAdapter->AdapterLink.VC_Info[VcIndex].Vpi;
				pBdAdslConnConfig->VcArray[VcIndex].Vci =
				    pThisAdapter->AdapterLink.VC_Info[VcIndex].Vci;
				pBdAdslConnConfig->VcArray[VcIndex].Pcr =
				    pThisAdapter->AdapterLink.VC_Info[VcIndex].Pcr;
			}
		}
		else
		{
			RtnResultValue = ERROR_WARNING( RESULT_DATA_FAILURE );
		}
	}
	else
	{
		RtnResultValue = ERROR_WARNING( RESULT_DATA_FAILURE );
	}

	return RtnResultValue;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALBdAdslSetConnConfig
*
*	ABSTRACT:
*		Function for performing the BackDoor ADSL Write Connection Configuration
*		parameters.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/
static DWORD cardALBdAdslSetConnConfig(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    I_O BD_ADSL_CONN_CONFIG_T		* pBdAdslConnConfig )
{
	DWORD							RtnResultValue;
	DWORD							VcIndex;

	RtnResultValue = RESULT_SUCCESS;
	if ( pBdAdslConnConfig->NumOfVc <= DEFAULT_NUMBER_OF_LINES )
	{
		if ( _MAX_LINKS == 1 )
		{
			for
			(
			    VcIndex = 0;
			    (
			        (VcIndex < pBdAdslConnConfig->NumOfVc)
			        &&
			        (RtnResultValue == RESULT_SUCCESS)
			    );
			    VcIndex++
			)
			{
				pThisAdapter->AdapterLink.VC_Info[VcIndex].Vpi =
				    pBdAdslConnConfig->VcArray[VcIndex].Vpi;
				pThisAdapter->AdapterLink.VC_Info[VcIndex].Vci =
				    pBdAdslConnConfig->VcArray[VcIndex].Vci;
				pThisAdapter->AdapterLink.VC_Info[VcIndex].Pcr =
				    pBdAdslConnConfig->VcArray[VcIndex].Pcr;
			}
		}
		else
		{
			RtnResultValue = ERROR_WARNING( RESULT_DATA_FAILURE );
		}
	}
	else
	{
		RtnResultValue = ERROR_WARNING( RESULT_DATA_FAILURE );
	}
	;
	return RtnResultValue;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALReadModemBlockFromDP
*
*	ABSTRACT:
*		Function for reading (uploading) memory from the Data Pump (DP).
*
*	RETURN:
*
*	DETAILS:
*		This function reads a block of modem memory one byte at time.
*
******************************************************************************/
static DWORD cardALBdAdslReadModemData(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    I_O BD_ADSL_MODEM_DATA_T	* pBdAdslModemData)
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	DBG_CDSL_DISPLAY(
	    DBG_LVL_MISC_INFO,
	    pThisAdapter->DebugFlag,
	    ("cardALBdAdslReadModemData:  Entry.") );

	if ( pBdAdslModemData->ModemBlockLength == 0 )
	{
		return RESULT_SUCCESS;
	}

	if ( pBdAdslModemData->ModemBlockLength > BD_MODEM_BLOCK_DATA_SIZE )
	{
		return ERROR_WARNING( RESULT_PARAM_OUT_OF_RANGE );
	}

	RtnResultValue = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		RtnResultValue = cardALChipUpldCodeFromDP(
		                     pThisAdapter,
		                     pCardALAdapter,
		                     pBdAdslModemData->ModemBlockStartAddr,
		                     &pBdAdslModemData->ModemBlockLength,
		                     &pBdAdslModemData->ModemBlockData[0] );
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_ERROR,
		    pThisAdapter->DebugFlag,
		    ("cardALBdAdslReadModemData: ERROR;  NULL Structures!\n") );
		return RtnResultValue;
	}

	return RtnResultValue;
}



/******************************************************************************
*	FUNCTION NAME:
*		cardALBdAdslGetPathSel
*
*	ABSTRACT:
*		Function for getting the ADSL path selection (Fast, Interleaved, Auto).
*
*	RETURN:
*
*	DETAILS:
******************************************************************************/
static DWORD cardALBdAdslGetPathSel(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    I_O BD_ADSL_DATA_PATH_SELECT_T		* pBdAdslPathSelect)
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	RtnResultValue = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		switch (pCardALAdapter->AdslDataPathSelected)
		{
		case ADSL_PATH_SELECT_AUTO:
			pBdAdslPathSelect->AdslDataPathSelection = BD_ADSL_PATH_SELECT_AUTOMATIC;
			break;
		case ADSL_PATH_SELECT_FAST:
			pBdAdslPathSelect->AdslDataPathSelection = BD_ADSL_PATH_SELECT_FAST;
			break;
		case ADSL_PATH_SELECT_INTERLEAVED:
			pBdAdslPathSelect->AdslDataPathSelection = BD_ADSL_PATH_SELECT_INTERLEAVED;
			break;
		default:
			pBdAdslPathSelect->AdslDataPathSelection = BD_ADSL_PATH_SELECT_UNKNOWN;
			break;
		}
		//		RtnResultValue = RESULT_SUCCESS;
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_ERROR,
		    pThisAdapter->DebugFlag,
		    ("cardALBdAdslGetPathSel: ERROR;  NULL Structures!\n") );
		return RtnResultValue;
	}

	return RtnResultValue;
}



/******************************************************************************
*	FUNCTION NAME:
*		cardALBdAdslSetPathSel
*
*	ABSTRACT:
*		Function for getting the ADSL path selection (Fast, Interleaved, Auto).
*
*	RETURN:
*
*	DETAILS:
******************************************************************************/
static DWORD cardALBdAdslSetPathSel(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    I_O BD_ADSL_DATA_PATH_SELECT_T		* pBdAdslPathSelect)
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	RtnResultValue = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		switch (pBdAdslPathSelect->AdslDataPathSelection)
		{
		case BD_ADSL_PATH_SELECT_AUTOMATIC:
			pCardALAdapter->AdslDataPathSelected = ADSL_PATH_SELECT_AUTO;
			break;
		case BD_ADSL_PATH_SELECT_FAST:
			pCardALAdapter->AdslDataPathSelected = ADSL_PATH_SELECT_FAST;
			break;
		case BD_ADSL_PATH_SELECT_INTERLEAVED:
			pCardALAdapter->AdslDataPathSelected = ADSL_PATH_SELECT_INTERLEAVED;
			break;
		default:
			pCardALAdapter->AdslDataPathSelected = ADSL_PATH_SELECT_UNKNOWN;
			break;
		}
		//		RtnResultValue = RESULT_SUCCESS;
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_ERROR,
		    pThisAdapter->DebugFlag,
		    ("cardALBdAdslSetPathSel: ERROR;  NULL Structures!\n") );
		return RtnResultValue;
	}

	return RtnResultValue;
}



/******************************************************************************
*	FUNCTION NAME:
*		cardALBdAdslGetLineStatus
*
*	ABSTRACT:
*		Function for performing the BackDoor ADSL Read of the line status
*		(speed/rate and state).  This is accomplished by reading the specific
*		register(s) for each parameter.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/
static DWORD cardALBdAdslGetLineStatus(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    I_O BD_ADSL_LINE_STATUS_T			* pBdAdslLineStatus)
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;
	NDIS_STATUS				Status1;
	NDIS_STATUS				Status2;

	pBdAdslLineStatus->LineSpeedUpOrFar = 0;
	pBdAdslLineStatus->LineSpeedDownOrNear = 0;
	pBdAdslLineStatus->LineState = BD_MODEM_DOWN;
	RtnResultValue = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		Status1 = CardALGetAdslLineSpeed(
		              pThisAdapter,
		              (WORD *) &pBdAdslLineStatus->LineSpeedUpOrFar,
		              (WORD *) &pBdAdslLineStatus->LineSpeedDownOrNear );
		Status2 = CardALGetAdslLineState(
		              pThisAdapter,
		              &pBdAdslLineStatus->LineState );
		if ( (Status1 == STATUS_SUCCESS)  &&  (Status2 == STATUS_SUCCESS) )
		{
			RtnResultValue = RESULT_SUCCESS;
		}
		else
		{
			RtnResultValue = ERROR_NORMAL( RESULT_DEVICE_FAILURE );
		}
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_ERROR,
		    pThisAdapter->DebugFlag,
		    ("cardALBdAdslGetLineStatus: ERROR;  NULL Structures!\n") );
		return RtnResultValue;
	}

	return RtnResultValue;
}



/******************************************************************************
*	FUNCTION NAME:
*		cardALBdAdslGetLineState
*
*	ABSTRACT:
*		Function for performing the BackDoor ADSL Read of the line state.
*		This is accomplished by reading the specific
*		register(s) for each parameter.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/
static DWORD cardALBdAdslGetLineState(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    I_O BD_ADSL_LINE_STATE_T			* pBdAdslLineState)
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;
	NDIS_STATUS				Status;

	pBdAdslLineState->LineState = BD_MODEM_DOWN;
	RtnResultValue = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		Status = CardALGetAdslLineState(
		             pThisAdapter,
		             &pBdAdslLineState->LineState );
		if ( Status == STATUS_SUCCESS )
		{
			RtnResultValue = RESULT_SUCCESS;
		}
		else
		{
			RtnResultValue = ERROR_NORMAL( RESULT_DEVICE_FAILURE );
		}
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_ERROR,
		    pThisAdapter->DebugFlag,
		    ("cardALBdAdslGetLineState: ERROR;  NULL Structures!\n") );
		return RtnResultValue;
	}

	return RtnResultValue;
}



/******************************************************************************
*	FUNCTION NAME:
*		cardALBdPeekReg
*
*	ABSTRACT:
*		Function for performing the BackDoor ADSL Peek Register (Read Register).
*		This is accomplished by reading the specific register.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/
static DWORD cardALBdPeekReg(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    I_O BD_CARDAL_REGISTER_T		* pBdCardALReg )
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	RtnResultValue = cardALChkAdptrStrucsOk( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		RtnResultValue = cardALChipPeekReg( pThisAdapter, pCardALAdapter, pBdCardALReg );
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_MISC_INFO,
		    pThisAdapter->DebugFlag,
		    ("cardALBdPeekReg: MISC.;  NULL Structures!\n") );
	}

	return RtnResultValue;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALBdPokeReg
*
*	ABSTRACT:
*		Function for performing the BackDoor ADSL Poke Register (Write
*		Register).  This is accomplished by writing the specific register.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/
static DWORD cardALBdPokeReg(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    I_O BD_CARDAL_REGISTER_T		* pBdCardALReg )
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	RtnResultValue = cardALChkAdptrStrucsOk( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		RtnResultValue = cardALChipPokeReg( pThisAdapter, pCardALAdapter, pBdCardALReg );
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_MISC_INFO,
		    pThisAdapter->DebugFlag,
		    ("cardALBdPokeReg: MISC.;  NULL Structures!\n") );
	}

	return RtnResultValue;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALBdAdslUserActivateLine
*
*	ABSTRACT:
*		Function for performing the BackDoor User Manual Activate ADSL Line.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/
static DWORD cardALBdAdslUserActivateLine(	IN CDSL_ADAPTER_T		* pThisAdapter )
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	RtnResultValue = cardALChkAdptrStrucsOk( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		pCardALAdapter->LineAutoActivation = TRUE;
		pCardALAdapter->LinePersistentAct = TRUE;
		RtnResultValue = cardALChipStartAdslLine( pThisAdapter, pCardALAdapter );
		if ( RtnResultValue == RESULT_SUCCESS )
		{
		}
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_MISC_INFO,
		    pThisAdapter->DebugFlag,
		    ("cardALBdAdslUserDeactivateLine: MISC.;  NULL Structures!\n") );
	}

	return RtnResultValue;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALBdAdslUserDeactivateLine
*
*	ABSTRACT:
*		Function for performing the BackDoor User Manual Deactivate ADSL Line.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/
static DWORD cardALBdAdslUserDeactivateLine(	IN CDSL_ADAPTER_T		* pThisAdapter )
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	RtnResultValue = cardALChkAdptrStrucsOk( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		pCardALAdapter->LineAutoActivation = FALSE;
		pCardALAdapter->LinePersistentAct = FALSE;
		RtnResultValue = cardALChipShutdownAdslLine( pThisAdapter, pCardALAdapter );
		if ( RtnResultValue == RESULT_SUCCESS )
		{
		}
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_MISC_INFO,
		    pThisAdapter->DebugFlag,
		    ("cardALBdAdslUserDeactivateLine: MISC.;  NULL Structures!\n") );
	}

	return RtnResultValue;
}




#if	PROJECTS_NAME == PROJECT_NAME_TIGRIS
/******************************************************************************
*	FUNCTION NAME:
*		cardALBdGetAdslTransceiverStatus
*
*	ABSTRACT:
*		Function for performing the BackDoor ADSL Get ADSL Transceiver Status.
*		This is accomplished by calling pairgain DMT API core routines.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/
static DWORD cardALBdGetAdslTransceiverStatus(
    IN CDSL_ADAPTER_T							* pThisAdapter,
    I_O BD_CARDAL_ADSL_TRANSCEIVER_STATUS_T		* pBdAdslTransceiverStatus )
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	RtnResultValue = cardALChkAdptrStrucsOk( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		RtnResultValue = cardALBdDpGetAdslTransceiverStatus( pThisAdapter, pCardALAdapter, pBdAdslTransceiverStatus );
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_MISC_INFO,
		    pThisAdapter->DebugFlag,
		    ("cardALBdGetAdslTransceiverStatus: MISC.;  NULL Structures!\n") );
	}

	return RtnResultValue;
}




#endif	//PROJECTS_NAME == PROJECT_NAME_TIGRIS
/******************************************************************************
*	FUNCTION NAME:
*		cardALBdAdslGetConfig
*
*	ABSTRACT:
*		Function for performing the BackDoor ADSL Read Configuration
*		parameters.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/
static DWORD cardALBdAdslGetConfig(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    I_O BD_ADSL_CFG_T		* pBdAdslConfig )
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	RtnResultValue = cardALChkAdptrStrucsOk( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		switch (pCardALAdapter->AdslDpRateMode)
		{
#if	PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
		case DP_OPERATIONAL_MODE_NOT_AVAIL:
			pBdAdslConfig->AdslDpRateMode = BD_ADSL_DP_RATE_NOT_AVAIL;
			break;
		case DP_OPERATIONAL_MODE_ANSI:
			pBdAdslConfig->AdslDpRateMode = BD_ADSL_DP_RATE_T1_DOT_413;
			break;
		case DP_OPERATIONAL_MODE_G_DOT_DMT:
			pBdAdslConfig->AdslDpRateMode = BD_ADSL_DP_RATE_G_DOT_DMT;
			break;
		case DP_OPERATIONAL_MODE_G_DOT_LITE:
			pBdAdslConfig->AdslDpRateMode = BD_ADSL_DP_RATE_G_DOT_LITE;
			break;
		default:
			RtnResultValue = ERROR_NORMAL(RESULT_DATA_FAILURE);
			break;
#endif
#if		PROJECTS_NAME == PROJECT_NAME_TIGRIS
		default:
			pBdAdslConfig->AdslDpRateMode = BD_ADSL_DP_RATE_G_DOT_DMT;
			break;
#endif
		}
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_WARNING,
		    pThisAdapter->DebugFlag,
		    ("cardALBdAdslGetConfig:  Failed adapter chk!") );
		// !!!! if the adapter check failed, then there is no guarantee that
		// !!!! the pCardALAdapter structure pointer was set properly!!!!!!!!!!!!
	}

	return RtnResultValue;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALBdAdslSetConfig
*
*	ABSTRACT:
*		Function for performing the BackDoor ADSL Write Configuration
*		parameters.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/
static DWORD cardALBdAdslSetConfig(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    I_O BD_ADSL_CFG_T		* pBdAdslConfig )
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	RtnResultValue = cardALChkAdptrStrucsOk( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
#if	PROJECTS_NAME == PROJECT_NAME_TIGRIS
		RtnResultValue = ERROR_NORMAL(RESULT_PARAM_OUT_OF_RANGE);
#endif
#if	PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
		RtnResultValue = ERROR_NORMAL(RESULT_PARAM_OUT_OF_RANGE);
#endif
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_WARNING,
		    pThisAdapter->DebugFlag,
		    ("cardALBdAdslSetConfig:  Failed adapter chk!") );
		// !!!! if the adapter check failed, then there is no guarantee that
		// !!!! the pCardALAdapter structure pointer was set properly!!!!!!!!!!!!
	}

	return RtnResultValue;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALBdAdslGetTextLogMsg
*
*	ABSTRACT:
*		Function for performing the BackDoor ADSL Read Configuration
*		parameters.
*
*	RETURN:
*
*	DETAILS:
*		Any values returned via the BackDoor scheme will have placed
*		into the appropriate place in the BackDoor structure.
******************************************************************************/
static DWORD cardALBdAdslGetTextLogMsg(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    I_O BD_MINI_DATA_BLOCK_DWORDS_T		pBdAdslTextLogs )
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;
	DWORD					LogMsgSizeInDwords;
	DWORD					BdAdslTextLogIndex;
	DWORD					* pdwArray;
	DWORD					Ndx;

	RtnResultValue = cardALChkAdptrStrucsOk( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		for ( Ndx = 0;   Ndx < BD_MINI_DATA_DWORDS;   Ndx++ )
		{
			pBdAdslTextLogs[Ndx] = 0;
		}
		BdAdslTextLogIndex = 0;
		RtnResultValue = CardALTextLogMsgQEnqMsg( &BdAdslTextLog );
		while (		(RtnResultValue == RESULT_SUCCESS)
		         &&	(BdAdslTextLog.FirstDword.Words.Size > 0)
		         &&	((BdAdslTextLogIndex + BdAdslTextLog.FirstDword.Words.Size) < BD_MINI_DATA_DWORDS) )
		{
			RtnResultValue = CardALTextLogMsgQGetMsg( &BdAdslTextLog );
			if ( RtnResultValue == RESULT_SUCCESS )
			{
				LogMsgSizeInDwords = BdAdslTextLog.FirstDword.Words.Size + 1;
				pdwArray = (DWORD *) &BdAdslTextLog;
				for ( Ndx = 0;   Ndx < LogMsgSizeInDwords;   Ndx++ )
				{
					pBdAdslTextLogs[BdAdslTextLogIndex++] = pdwArray[Ndx];
				}
			}
			RtnResultValue = CardALTextLogMsgQEnqMsg( &BdAdslTextLog );
		}
		RtnResultValue = RESULT_SUCCESS;
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_WARNING,
		    pThisAdapter->DebugFlag,
		    ("cardALBdAdslGetTextLogMsg:  Failed adapter chk!") );
		// !!!! if the adapter check failed, then there is no guarantee that
		// !!!! the pCardALAdapter structure pointer was set properly!!!!!!!!!!!!
	}

	return RtnResultValue;
}




