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
**		CardALMgmt.c
**
**	ABSTRACT:
**		Card level Abstraction Layer for CDSL.  This file/module abstracts
**		(hides the details of) card specific (CDSL) functions and presents
**		(hopefully) a constant interface to the upper level modules/functions
**		(CardMgmt) no matter what the actual card is (CDSL, ADSL, ISDN, etc.).
**		This file provides the functions that provide the interface from other
**		modules to the CardAL module (management of the CardAL module).
**
**	DETAILS:
**		Functions are arranged in top-down order.
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/CardALTigrisLC.c $
** $Revision: 3 $
** $Date: 7/24/01 8:40a $
*******************************************************************************
******************************************************************************/
#define		MODULE_ID			DBG_ENABLE_CARDAL


#include "CardMgmt.h"
#include "CardMgmtV.h"
#define		CARDAL_DECLARE_PUBLIC			1
#include "CardALV.h"
#undef		CARDAL_DECLARE_PUBLIC
#include "EEType.h"
#include "ChipALSEmw.h"

#define ERROR_FATAL(error)		(LEVEL_FATAL   | MODULE_CARDAL | INSTANCE_CARDAL | error)
#define ERROR_WARNING(error)	(LEVEL_WARNING | MODULE_CARDAL | INSTANCE_CARDAL | error)
#define ERROR_NORMAL(error)		(LEVEL_NORMAL  | MODULE_CARDAL | INSTANCE_CARDAL | error)

//	Unique file ID for error logging
#define		__FILEID__			6
#define MAC_BD_SIZE	( sizeof(DEVIO_HEADER_T) + sizeof(BD_CARDAL_REGISTER_T) + sizeof(pCardAl->MacAddress))




///////////////////////////////////////////////////////////////////////////////
//	Defines
///////////////////////////////////////////////////////////////////////////////

#ifndef MEMORY_TAG
	#define MEMORY_TAG			'MLAC'
#endif

#define DRIVER_LOG_Q_BODY_SIZE		4 * 1024

typedef struct DRIVER_LOG_TEXT_QUEUE_S
{
	DWORD		Head;
	DWORD		Tail;
	DWORD		Size;
	DWORD		Count;
	DWORD		Body [DRIVER_LOG_Q_BODY_SIZE];
} DRIVER_LOG_TEXT_QUEUE_T;




///////////////////////////////////////////////////////////////////////////////
//	Variable Declarations
///////////////////////////////////////////////////////////////////////////////

static DRIVER_LOG_TEXT_QUEUE_T		DriverLogTextQ;


//*
//* To Add a new EEProm Mac Address definition, create a define and add to the
//* gMacOffsetAddress table.  The table is processed based on the registry
//* entry that defines the Physical Driver Type.  In current drivers, 
//* this is a value stored in the registry key PhysicalDriverType.
//*
#define MAC_EE_ADDRESS_P46		0x0000007A
#define MAC_EE_ADDRESS_P51		0x000000FA

static DWORD		gMacOffsetAddress[DEVICE_TYPE_MAX] =
{
	MAC_EE_ADDRESS_P46,			/* Unused */
	MAC_EE_ADDRESS_P46,			/* P46 Based Devices */
	MAC_EE_ADDRESS_P51			/* P51 Based Devices */
};



///////////////////////////////////////////////////////////////////////////////
//	External Function Prototypes
///////////////////////////////////////////////////////////////////////////////

NDIS_STATUS cardALReadNetworkAddress(IN CDSL_ADAPTER_T	* pThisAdapter);



///////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
///////////////////////////////////////////////////////////////////////////////

NDIS_STATUS	CardAlWriteMacAddress(
    IN CARDAL_ADAPTER_T * pCardAl,
    IN BYTE			 	* pMacAddress,
    IN DWORD		 	MacAddrLength
);

NDIS_STATUS	CardAlReadMacAddress(
    IN CARDAL_ADAPTER_T * pCardAl,
    IN BYTE			 	* pMacAddress,
    IN DWORD		 	MacAddrLength
);

LOCAL	NDIS_STATUS	CardAlLoadMacAddress(
    CARDAL_ADAPTER_T * pCardAl
);

GLOBAL NTSTATUS CardAlPassiveWaitComplete(
    IN void					* pUserContext,
    IN NTSTATUS				Status,
    IN CHAR					* Buffer,
    IN DWORD				Length
);


///////////////////////////////////////////////////////////////////////////////
//	Functions
///////////////////////////////////////////////////////////////////////////////


/******************************************************************************
*	FUNCTION NAME:
*		CardALAdapterInit
*
*	ABSTRACT:
*		Requirement of the Architecture to initialize adapter structures,
*		parameters, etc. for the CardAL module.
*
*	RETURN:
*		NDIS_STATUS
******************************************************************************/
VOID * CDALTigrisInit(
    IN	VOID 					* AdapterContext,
    IN VOID 					* CardAlContext
)
{
	NDIS_STATUS				NdisStatus;
	CARDAL_ADAPTER_T		* pCardALAdapter = NULL;		// Card Al Management Context
	DWORD					ResultValue;
	CDSL_ADAPTER_T			* pThisAdapter = (CDSL_ADAPTER_T *)AdapterContext;


	// This "while( TRUE )" loop (only run once) allows the function/code to
	// 'break' out of the 'loop' at any point where an error occurs.
	//		(Instead of having to have error code that unallocates at every
	//		 error condition, all of the common error condition code is
	//		 at the end of the 'loop'!)
	while( TRUE )
	{
		NdisStatus = ALLOCATE_MEMORY( (PVOID)&(pCardALAdapter), sizeof( CARDAL_ADAPTER_T ), MEMORY_TAG );
		if ( NdisStatus != STATUS_SUCCESS )
		{
			NdisWriteErrorLogEntry(
			    pThisAdapter->MiniportAdapterHandle,
			    NDIS_ERROR_CODE_OUT_OF_RESOURCES,
			    4,
			    NdisStatus,
			    __FILEID__,
			    __LINE__,
			    RSS_INIT_ERROR_FAILED_ALLOCATE_CARDAL_ADPTR );
			DBG_CDSL_DISPLAY(
			    DBG_LVL_ERROR,
			    pThisAdapter->DebugFlag,
			    ("CardALAdapterInit: ERROR;  Could NOT Allocate CARDAL_ADAPTER_T!\n") );
			break;
		}

		CLEAR_MEMORY( pCardALAdapter, sizeof( CARDAL_ADAPTER_T ) );

		pThisAdapter->CardMod = pCardALAdapter;
		pCardALAdapter->pCdslAdapter = (CDSL_ADAPTER_T*)AdapterContext;
		
		// *
		// * set the rate mode from the registry
		// *
		pCardALAdapter->AdslDpRateMode = pThisAdapter->PersistData.AdslDpRateMode;
		
		//*
		//* Check registry for MAC address. If one exist load it instead of EEPROM.
		//*

		pCardALAdapter->MacAddressLocation = gMacOffsetAddress[pThisAdapter->PersistData.CnxtPhysicalDriverType];

		if(pThisAdapter->PersistData.OverrideMacAddress)
		{
			COPY_MEMORY(pThisAdapter->PermanentEthAddress,
			            pThisAdapter->PersistData.MACAddress,
			            ETH_ADDRESS_LENGTH );
		}
		else
		{
			ResultValue = CardAlLoadMacAddress(pCardALAdapter);

			if ( ResultValue == RESULT_SUCCESS )
			{
				COPY_MEMORY(pThisAdapter->PermanentEthAddress,
				            pCardALAdapter->MacAddress,
				            MIN_VALUE(sizeof(pCardALAdapter->MacAddress), ETH_ADDRESS_LENGTH)
				           );
			}
		}

		// save the mac address to the device for the protocols to use
		COPY_MEMORY(pThisAdapter->pAtmDevice->esi,
		            pThisAdapter->PermanentEthAddress,
		            ETH_ADDRESS_LENGTH );

		break;							// ONLY do this code ONE TIME
	}		// while( TRUE )


	// If an error condition occurred, check to see if memory needs to be
	// unallocated.
	if ( NdisStatus != STATUS_SUCCESS )
	{
		if ( pCardALAdapter != NULL )
		{
			FREE_MEMORY( pCardALAdapter, sizeof( CARDAL_ADAPTER_T ), 0 );
			pCardALAdapter = NULL;
			//pThisAdapter->CardMod = NULL;
		}
	}

	return pCardALAdapter;
}

/******************************************************************************
*	FUNCTION NAME:
*		CardALTigrisShutdown
*
*	ABSTRACT:
*		Requirement of the Architecture to halt/shutdown/stop/free allocated
*		memory or other required adapter items for the CardAL.
*
*	RETURN:
******************************************************************************/
VOID CDALTigrisShutdown(
	void		* pAdapter
)
{
	CARDAL_ADAPTER_T		* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	CDSL_ADAPTER_T		* pThisAdapter = pCardALAdapter->pCdslAdapter;

	DBG_CDSL_DISPLAY(
		DBG_LVL_MISC_INFO,
		pThisAdapter->DebugFlag,
		("CDALTigrisShutdown:  Entry.") );

	cardALChipShutdownAdslLine( pThisAdapter, pCardALAdapter );

	if ( pCardALAdapter != NULL )
	{
		FREE_MEMORY( pCardALAdapter, sizeof( CARDAL_ADAPTER_T ), 0 );
		pCardALAdapter = NULL;
	}
}

/******************************************************************************
FUNCTION NAME:
	CDALTigrisReset

ABSTRACT:
	Card Reset - TBD


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NDIS_STATUS CDALTigrisReset(
    void			* pAdapter
)
{
	//	CARDAL_ADAPTER_T			* pCardALAdapter=()pAdapter;
	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	CDALTigrisSetLineState

ABSTRACT:
	Set ADSL Line State


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NDIS_STATUS CDALTigrisSetLineState(
    void					* pAdapter,
    CARD_LINE_STATE_T		NewState
)
{
	CARDAL_ADAPTER_T		* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NDIS_STATUS				Status;

	if ( pCardALAdapter == NULL )
	{
		return(-EINVAL);
	}

	switch(NewState)
	{

	case CDAL_LINE_STATE_INT:
		//* Do Shutdown Code
		Status = cardALChipShutdownAdslLine( pCardALAdapter->pCdslAdapter, pCardALAdapter );
		break;

	case CDAL_LINE_STATE_INITIALIZE:
	case CDAL_LINE_STATE_PERSIST_INITIALIZE:
		//* Startup Line
		Status = cardALChipStartAdslLine( pCardALAdapter->pCdslAdapter, pCardALAdapter );
		break;

	default:
		Status = -EINVAL;
		break;
	}
	return(Status);
}



/******************************************************************************
*	FUNCTION NAME:
*		CardALGetLineStats
*
*	ABSTRACT:
*		Function called to return certain line stats (i.e., line status
*		and downstream rate/speed).
*
*	RETURN:
*		NDIS_STATUS
*
*	DETAILS:
******************************************************************************/
GLOBAL NDIS_STATUS CardALGetLineStats(
    IN CDSL_ADAPTER_T	* pThisAdapter,
    OUT BYTE			* LineStatus,
    OUT WORD			* LineDownstreamRate,
    OUT WORD			* LineUpstreamRate )
{
#if	PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
	BD_ADSL_PERFORMANCE_T		BdAdslPerformStruc;
#endif	//PROJECTS_NAME == PROJECT_NAME_CHEETAH_1

	*LineStatus = (BYTE) BD_MODEM_UNKNOWN;
	*LineDownstreamRate = (WORD) UNKNOWN_16BIT_VALUE;
	*LineUpstreamRate = (WORD) UNKNOWN_16BIT_VALUE;
#if	PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
	cardALBdAdslGetPerform( pThisAdapter, &BdAdslPerformStruc );
	*LineStatus = (BYTE) BdAdslPerformStruc.M_ModemStatus;
	*LineDownstreamRate = (WORD) BdAdslPerformStruc.R_ChanDataFastNear;
	*LineUpstreamRate = (WORD) BdAdslPerformStruc.R_ChanDataFastFar;
#endif	//PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
#if	PROJECTS_NAME == PROJECT_NAME_TIGRIS
	//@$&%~*******######  NEEDS TIGRIS SPECIFIC CODE HERE  ######*******~%&$@//
	*LineStatus = (BYTE) BD_MODEM_UNKNOWN;			// ######*******~%&$@//
	*LineDownstreamRate = (WORD) UNKNOWN_16BIT_VALUE;//######*******~%&$@//
	*LineUpstreamRate = (WORD) UNKNOWN_16BIT_VALUE;//  ######*******~%&$@//
	//@$&%~*******######  NEEDS TIGRIS SPECIFIC CODE HERE  ######*******~%&$@//
#endif	//PROJECTS_NAME == PROJECT_NAME_TIGRIS

	return STATUS_SUCCESS;
}




/******************************************************************************
*	FUNCTION NAME:
*		CardALGetAdslLineSpeed
*
*	ABSTRACT:
*		Function called to return the ADSL line speed.
*
*	RETURN:
*		NDIS_STATUS
*
*	DETAILS:
*		The ADSL line speed is determined from the upstream (far) data rate.
*		The rate is further determined from the interleaved ("...Int...") and
*		fast ("...Fast...") values, whichever is nonzero.
******************************************************************************/
GLOBAL NDIS_STATUS CardALGetAdslLineSpeed(
    IN CDSL_ADAPTER_T	* pThisAdapter,
    OUT WORD			* pAdslLineSpeedUpOrFar,
    OUT WORD			* pAdslLineSpeedDownOrNear )
{
	DWORD					ResultValue;
	NDIS_STATUS				Status;
	DWORD					AdptrChk;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	*pAdslLineSpeedUpOrFar = (WORD) 0;
	*pAdslLineSpeedDownOrNear = (WORD) 0;
	Status = -EINVAL;
	AdptrChk = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );
	if ( AdptrChk == RESULT_SUCCESS )
	{
		if ( pCardALAdapter->AdslLineUp )
		{
			ResultValue = cardALChipGetLineSpeed(
			                  pThisAdapter,
			                  pCardALAdapter,
			                  pAdslLineSpeedUpOrFar,
			                  pAdslLineSpeedDownOrNear );
			if ( ResultValue == RESULT_SUCCESS )
			{
				Status = STATUS_SUCCESS;
			}
		}
	}
	else
	{
	}

	return Status;
}




/******************************************************************************
*	FUNCTION NAME:
*		CardALGetAdslLineState
*
*	ABSTRACT:
*		Function called to return the ADSL line state.
*
*	RETURN:
*		NDIS_STATUS
*
*	DETAILS:
******************************************************************************/
GLOBAL NDIS_STATUS CardALGetAdslLineState(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    OUT BD_ADSL_MODEM_STATUS_T		* pAdslLineStatus )
{
	NDIS_STATUS				Status;
	DWORD					ResultValue;
	DWORD					AdptrChk;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	*pAdslLineStatus = BD_MODEM_DOWN;
	Status = -EINVAL;
	AdptrChk = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );
	if ( AdptrChk == RESULT_SUCCESS )
	{
		ResultValue = cardALChipGetLineStatus(
		                  pThisAdapter,
		                  pCardALAdapter,
		                  (BYTE *) pAdslLineStatus );
		if ( ResultValue == RESULT_SUCCESS )
		{
			Status = STATUS_SUCCESS;
		}
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_ERROR,
		    pThisAdapter->DebugFlag,
		    ("CardALGetAdslLineState: ERROR;  NULL Structures!\n") );
	}

	return Status;
}

/******************************************************************************
FUNCTION NAME:
	CmgmtGetPeerList

ABSTRACT:
	Return List of commands for user API. The BdSupportedList must be an
	INORDER list.


RETURN:
	STATUS_SUCCESS


DETAILS:
	The storage for BdSupportedList cannot be released until after
	Driver Adapter Inits are complete.  In this case, it is a static so it is
	not necessary to release.
******************************************************************************/
DWORD CmgmtGetPeerList(
    VOID				* ExternHandle,
    DWORD				** MessageList,
    DWORD				* MaxMessages
)
{
	static DWORD	PeerSupportedList[] =
	    {
	        BD_CARDAL_GET_REGISTER,	 					/* 0x00070001 */
	        BD_CARDAL_SET_REGISTER
	    };

	*MaxMessages = sizeof(PeerSupportedList)/sizeof(PeerSupportedList[0]);
	*MessageList = (DWORD *)PeerSupportedList;
	return(STATUS_SUCCESS);
}

/******************************************************************************
*	FUNCTION NAME:
*		CardALTextLogMsgPackStr
*
*	ABSTRACT:
*		.
*
*	RETURN:
*		NDIS_STATUS
*
*	DETAILS:
*		The ASCII string passed in will be packed into the DWORD array passed
*		in starting with the second DWORD array element.  If the ASCII string
*		is larger (including the zero string terminator) than the DWORD array
*		size in bytes (starting with second DWORD), then the string will be
*		truncated to fill the DWORD array.
*
*		The number of bytes that may be packed into the DWORD array is
*			(maximum number of DWORDs ("MAX_NUM_LOG_PARAMS") * 4) - 1.
*		There are 4 BYTEs per DWORD and 1 byte must be reserved for the zero
*		string terminator.
*
*		Msg length in DWORDs is
*			(
*				the ASCII string length
*				+ 1 (to get the zero terminator)
*				+ 3 (to get partially filled DWORDs to report as a whole DWORD)
*			)
*			/ 4.
*
*		The first DWORD of the DWORD array will have the message length (in DWORDs,
*		excuding the first DWORD) as the most significant WORD and the Log Msg
*		type of "BD_LM_PACKED_STRING" as the least significant WORD.
******************************************************************************/
GLOBAL NDIS_STATUS CardALTextLogMsgPackStr(
    IN char					* szString,
    I_O BD_DRIVER_LOG_T		* pLogMsg )
{
	DWORD		StrLenInBytes;
	BYTE		* pByteArray;

	pByteArray = (BYTE *) pLogMsg->DwordArray;
	StrLenInBytes = strlen( szString );
	if ( StrLenInBytes > ((MAX_NUM_LOG_PARAMS * 4) - 1) )
	{
		StrLenInBytes = (MAX_NUM_LOG_PARAMS * 4) - 1;
	}
	COPY_MEMORY( ((PUCHAR) pByteArray), szString, StrLenInBytes );
	pByteArray[StrLenInBytes] = 0;		// terminate ASCII string

	pLogMsg->FirstDword.Words.Size = (WORD) (StrLenInBytes + 1 + 3) / 4;	// Msg length in DWORDs

	pLogMsg->FirstDword.Words.Type = BD_LM_PACKED_STRING;

	return STATUS_SUCCESS;
}

typedef struct CDAL_IO_SYNC_S
{
	NDIS_STATUS				Status;
	DWORD					Length;
	EVENT_HNDL				EventComplete;
}CDAL_IO_SYNC_T;


/******************************************************************************
FUNCTION NAME:
	CardAlLoadMacAddress

ABSTRACT:
	Read Mac Address from Card


RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
LOCAL	NDIS_STATUS	CardAlLoadMacAddress(
    CARDAL_ADAPTER_T * pCardAl
)
{

	NDIS_STATUS				Status;
	CHAR					BdBuff[MAC_BD_SIZE] = {0};
	BD_CARDAL_REGISTER_T	* pRegIo = (BD_CARDAL_REGISTER_T *) ( BdBuff + sizeof(DEVIO_HEADER_T)  );
	REG_IO_EXT_T			RegIoExt = {{0}};

	pRegIo->RegOffset 	= pCardAl->MacAddressLocation;
	pRegIo->RegMask		= 0x00000000;
	pRegIo->RegValue	= 0x0L;
	pRegIo->RegDataSize = sizeof(BYTE);
	pRegIo->RegAccess 	= BD_ADSL_REG_ACCESS_EEPROM;
	pRegIo->VarLength 	= sizeof(pCardAl->MacAddress);

	RegIoExt.SrcDestU.Destination = pRegIo->VarBuff;
	RegIoExt.IoType = REG_IO_READ;

	Status = ChipALRead(pCardAl->pCdslAdapter,
	                    EE_PROM,
	                    REMOTE_ADDRESS_BAR_2,
	                    pRegIo->RegOffset,
	                    pRegIo->VarLength,
	                    (DWORD *)pRegIo->VarBuff );

	if(Status == STATUS_SUCCESS)
	{
		// Copy Results from Temp Buffr to final destination
		COPY_MEMORY(pCardAl->MacAddress,
		            pRegIo->VarBuff,
		            sizeof(pCardAl->MacAddress)
		           );
	}

	return(Status);

}

/******************************************************************************
FUNCTION NAME:
	CardAlReadMacAddress

ABSTRACT:
	Read Mac Address from Card


RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
NDIS_STATUS	CardAlReadMacAddress(
    IN CARDAL_ADAPTER_T * pCardAl,
    IN BYTE			 	* pMacAddress,
    IN DWORD		 	MacAddrLength
)
{

	NDIS_STATUS				Status;
	CHAR					BdBuff[MAC_BD_SIZE] = {0};
	BD_CARDAL_REGISTER_T	* pRegIo = (BD_CARDAL_REGISTER_T *) ( BdBuff + sizeof(DEVIO_HEADER_T)  );
	REG_IO_EXT_T			RegIoExt = {{0}};

	pRegIo->RegOffset 	= pCardAl->MacAddressLocation;
	pRegIo->RegMask		= 0x00000000;
	pRegIo->RegValue	= 0x0L;
	pRegIo->RegDataSize = sizeof(BYTE);
	pRegIo->RegAccess 	= BD_ADSL_REG_ACCESS_EEPROM;
	pRegIo->VarLength 	= MacAddrLength;

	// Don't update the address in cardal's local context. Just return it to the
	// caller...
	pRegIo->RegOffset 	= pCardAl->MacAddressLocation;
	pRegIo->RegMask		= 0x00000000;
	pRegIo->RegValue	= 0x0L;
	pRegIo->RegDataSize = sizeof(BYTE);
	pRegIo->RegAccess 	= BD_ADSL_REG_ACCESS_EEPROM;
	pRegIo->VarLength 	= sizeof(pCardAl->MacAddress);

	RegIoExt.SrcDestU.Destination = pRegIo->VarBuff;
	RegIoExt.IoType = REG_IO_READ;

	Status = ChipALRead(
	             pCardAl->pCdslAdapter,
	             EE_PROM,
	             REMOTE_ADDRESS_BAR_2,
	             pCardAl->MacAddressLocation,
	             MacAddrLength,
	             (DWORD *)BdBuff
	         );


	if(Status == STATUS_SUCCESS)
	{
		// Copy Results from Temp Buffr to final destination
		COPY_MEMORY(pMacAddress,
		            BdBuff,
		            MacAddrLength
		           );
	}

	return(Status);

}

/******************************************************************************
FUNCTION NAME:
	CardAlWriteMacAddress

ABSTRACT:
	Write the Mac Address to the Card. The MAC address is read
	when the driverr is loaded, thus the new address will not
	(should not) take effect until then.


RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
NDIS_STATUS	CardAlWriteMacAddress(
    IN CARDAL_ADAPTER_T * pCardAl,
    IN BYTE			 	* pMacAddress,
    IN DWORD		 	MacAddrLength
)
{

	CDAL_IO_SYNC_T			Sync;
	NDIS_STATUS				Status;
	CHAR					BdBuff[MAC_BD_SIZE] = {0};
	DEVIO_HEADER_T			* pDevIoHeader = (DEVIO_HEADER_T *)BdBuff;
	BD_CARDAL_REGISTER_T	* pRegIo = (BD_CARDAL_REGISTER_T *) ( BdBuff + sizeof(DEVIO_HEADER_T)  );
	VOID					* SysIfHandle;

	Sync.Status = -EINVAL;
	Sync.Length = 0;

	INIT_EVENT(&Sync.EventComplete);

	pDevIoHeader->InstanceId = 0;
	pDevIoHeader->ReqCode = BD_CARDAL_SET_REGISTER;
	pDevIoHeader->TotalSize = MAC_BD_SIZE;
	pDevIoHeader->NeededSize = MAC_BD_SIZE;
	pDevIoHeader->ResultCode = STATUS_SUCCESS;


	pRegIo->RegOffset 	= pCardAl->MacAddressLocation;
	pRegIo->RegMask		= 0x00000000;
	pRegIo->RegValue	= 0x0L;
	pRegIo->RegDataSize = sizeof(BYTE);
	pRegIo->RegAccess 	= BD_ADSL_REG_ACCESS_EEPROM;
	pRegIo->VarLength 	= MacAddrLength;
	COPY_MEMORY(pRegIo->VarBuff, pMacAddress, MacAddrLength);

	Status = SmSysIfGetHandle(pCardAl->pCdslAdapter, &SysIfHandle);

	Status = SmSysIfSendMessage(
	             SysIfHandle,				// System Interface Handle
	             MESS_DEV_SPECIFIC,			// Event
	             (CHAR *)pDevIoHeader,		// Message Buffer
	             MAC_BD_SIZE,				// Length
	             CardAlPassiveWaitComplete,	// SendComplete
	             &Sync						// SendComplete Context
	         );
	return(Status);
}

/******************************************************************************
FUNCTION NAME:
	CardAlPassiveWaitComplete

ABSTRACT:
	In the case we are not at DISPATCH level, or request will be pended.
	This routine will signal the completion


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CardAlPassiveWaitComplete(
    IN void					* pUserContext,
    IN NTSTATUS				Status,
    IN CHAR					* Buffer,
    IN DWORD				Length
)
{

	CDAL_IO_SYNC_T * Sync = (CDAL_IO_SYNC_T *) pUserContext;
	Sync->Status = Status;
	Sync->Length = Length;
	SET_EVENT(&Sync->EventComplete);

	return(STATUS_SUCCESS);
}


/******************************************************************************
*	FUNCTION NAME:
*		CardALTextLogMsgQPutMsg
*
*	ABSTRACT:
*		.
*
*	RETURN:
*		NDIS_STATUS
*
*	DETAILS:
******************************************************************************/
GLOBAL NDIS_STATUS CardALTextLogMsgQPutMsg(
    IN BD_DRIVER_LOG_T				* pLogMsg )
{
	NDIS_STATUS					Status;
	DWORD						TmpMsgLenInDwords;
	DWORD						TmpDword;
	DWORD						Ndx;
	DWORD						InMsgSizeInDwords;
	DWORD						OutMsgSizeInDwords;
	DWORD						TotalMsgSizeInDwords;
	DWORD						* pdwArray;
	union
	{
		DWORD			Dwords [2];
		LARGE_INTEGER	Ddword;
	}							unSystemTime;


	unSystemTime.Ddword = jiffies;

	InMsgSizeInDwords = pLogMsg->FirstDword.Words.Size;
	if ( InMsgSizeInDwords > MAX_NUM_LOG_PARAMS )
	{
		Status = -EINVAL;
	}
	else
	{
		// Adjust the stored (Out) msg size to include the system time stamp.
		OutMsgSizeInDwords = InMsgSizeInDwords + (sizeof( LARGE_INTEGER ) / sizeof( DWORD ));
		pLogMsg->FirstDword.Words.Size = (WORD) OutMsgSizeInDwords;
		// Set the total (Total) msg size to include Size/Type and the system time stamp.
		TotalMsgSizeInDwords = OutMsgSizeInDwords + 1;

		//
		//	If queue is too full, remove oldest log msgs until there is room in the queue.
		//
		while ( TotalMsgSizeInDwords > (DriverLogTextQ.Size - DriverLogTextQ.Count) )
		{
			//	Remove Size/Type DWORD (of oldest log msg) from the queue.
			pLogMsg->FirstDword.Dword = DriverLogTextQ.Body[DriverLogTextQ.Tail];
			DriverLogTextQ.Tail++;
			if ( DriverLogTextQ.Tail >= DriverLogTextQ.Size )
			{
				DriverLogTextQ.Tail = 0;
			}
			DriverLogTextQ.Count--;

			//	Remove remainder (parameters) (of oldest log msg) from the queue.
			TmpMsgLenInDwords = pLogMsg->FirstDword.Words.Size;
			while ( TmpMsgLenInDwords > 0 )
			{
				TmpDword = DriverLogTextQ.Body[DriverLogTextQ.Tail];
				DriverLogTextQ.Tail++;
				if ( DriverLogTextQ.Tail >= DriverLogTextQ.Size )
				{
					DriverLogTextQ.Tail = 0;
				}
				DriverLogTextQ.Count--;
				TmpMsgLenInDwords--;
			}
		}

		//	Place the log msg Size/Type DWORD in the queue.
		DriverLogTextQ.Body[DriverLogTextQ.Head] = pLogMsg->FirstDword.Dword;
		DriverLogTextQ.Head++;
		if ( DriverLogTextQ.Head >= DriverLogTextQ.Size )
		{
			DriverLogTextQ.Head = 0;
		}
		DriverLogTextQ.Count++;

		//	Place the least significant DWORD of the system time in the queue.
		DriverLogTextQ.Body[DriverLogTextQ.Head] = unSystemTime.Dwords[0];
		DriverLogTextQ.Head++;
		if ( DriverLogTextQ.Head >= DriverLogTextQ.Size )
		{
			DriverLogTextQ.Head = 0;
		}
		DriverLogTextQ.Count++;

		//	Place the most significant DWORD of the system time in the queue.
		DriverLogTextQ.Body[DriverLogTextQ.Head] = unSystemTime.Dwords[1];
		DriverLogTextQ.Head++;
		if ( DriverLogTextQ.Head >= DriverLogTextQ.Size )
		{
			DriverLogTextQ.Head = 0;
		}
		DriverLogTextQ.Count++;

		//	Place the remainder (parameters) of the log msg in the queue.
		pdwArray = (DWORD *) pLogMsg->DwordArray;
		for ( Ndx = 0;   Ndx < InMsgSizeInDwords;   Ndx++ )
		{
			DriverLogTextQ.Body[DriverLogTextQ.Head] = pdwArray[Ndx];
			DriverLogTextQ.Head++;
			if ( DriverLogTextQ.Head >= DriverLogTextQ.Size )
			{
				DriverLogTextQ.Head = 0;
			}
			DriverLogTextQ.Count++;
		}

		Status = STATUS_SUCCESS;
	}

	return Status;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALTextLogMsgQGetMsg
*
*	ABSTRACT:
*		.
*
*	RETURN:
*		NDIS_STATUS
*
*	DETAILS:
******************************************************************************/
GLOBAL NDIS_STATUS CardALTextLogMsgQGetMsg(
    I_O BD_DRIVER_LOG_T		* pLogMsg )
{
	NDIS_STATUS					Status;
	DWORD						TmpMsgLenInDwords;
	DWORD						Ndx;

	if ( DriverLogTextQ.Count > 0 )
	{
		pLogMsg->FirstDword.Dword = DriverLogTextQ.Body[DriverLogTextQ.Tail];
		DriverLogTextQ.Tail++;
		if ( DriverLogTextQ.Tail >= DriverLogTextQ.Size )
		{
			DriverLogTextQ.Tail = 0;
		}
		DriverLogTextQ.Count--;
		TmpMsgLenInDwords = pLogMsg->FirstDword.Words.Size;
		Ndx = 0;
		while ( TmpMsgLenInDwords > 0 )
		{
			pLogMsg->DwordArray[Ndx] = DriverLogTextQ.Body[DriverLogTextQ.Tail];
			Ndx++;
			DriverLogTextQ.Tail++;
			if ( DriverLogTextQ.Tail >= DriverLogTextQ.Size )
			{
				DriverLogTextQ.Tail = 0;
			}
			DriverLogTextQ.Count--;
			TmpMsgLenInDwords--;
		}
		Status = STATUS_SUCCESS;
	}
	else
	{
		Status = -EINVAL;
	}

	return Status;
}




/******************************************************************************
*	FUNCTION NAME:
*		CardALTextLogMsgQEnqMsg
*
*	ABSTRACT:
*		.
*
*	RETURN:
*		NDIS_STATUS
*
*	DETAILS:
******************************************************************************/
GLOBAL NDIS_STATUS CardALTextLogMsgQEnqMsg(
    IN BD_DRIVER_LOG_T		* pLogMsg )
{
	NDIS_STATUS					Status;

	if ( DriverLogTextQ.Count > 0 )
	{
		pLogMsg->FirstDword.Dword = DriverLogTextQ.Body[DriverLogTextQ.Tail];
		Status = STATUS_SUCCESS;
	}
	else
	{
		pLogMsg->FirstDword.Dword = 0;
		Status = -EINVAL;
	}

	return Status;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALTextLogMsgQInit
*
*	ABSTRACT:
*		.
*
*	RETURN:
*		NDIS_STATUS
*
*	DETAILS:
******************************************************************************/
GLOBAL NDIS_STATUS CardALTextLogMsgQInit( void )
{
	NDIS_STATUS					Status;

	DriverLogTextQ.Head = 0;
	DriverLogTextQ.Tail = 0;
	DriverLogTextQ.Size = DRIVER_LOG_Q_BODY_SIZE;
	DriverLogTextQ.Count = 0;
	Status = STATUS_SUCCESS;

	return Status;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALTextLogMsgQRelease
*
*	ABSTRACT:
*		.
*
*	RETURN:
*		NDIS_STATUS
*
*	DETAILS:
******************************************************************************/
GLOBAL NDIS_STATUS CardALTextLogMsgQRelease( void )
{
	NDIS_STATUS					Status;

	DriverLogTextQ.Head = 0;
	DriverLogTextQ.Tail = 0;
	DriverLogTextQ.Size = 0;
	DriverLogTextQ.Count = 0;
	Status = STATUS_SUCCESS;

	return Status;
}



/******************************************************************************
*	FUNCTION NAME:
*		cardALChkAdptrStrucsOkNotDnldgDP
*
*	ABSTRACT:
*		Function for checking the CDSL Adapter structure and the CardAL
*		Adapter structure to make sure they are not NULL pointers (make
*		sure they are allocated and valid to a small extent) and that
*		a download of the DP is not in progress.
*
*	RETURN:
*		BOOLEAN:
*			TRUE -	Adapter structures appear valid and DP download is not
*					in progress.
*			FALSE -	One or both of the structures are NULL pointers or the
*					the DP is in process of being downloaded.
*		OUT ppCardALAdapter:
*			Where this parameter points to will be set to point to
*			the CardAL Adapter structure if both Adapter structures
*			are valid or NULL otherwise.
*
*	DETAILS:
******************************************************************************/
DWORD cardALChkAdptrStrucsOkNotDnldgDP(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN OUT CARDAL_ADAPTER_T		** ppCardALAdapter )
{
	DWORD			RtnResult;

	RtnResult = cardALChkAdptrStrucsOk( pThisAdapter, ppCardALAdapter );

	return RtnResult;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALChkAdptrStrucsOk
*
*	ABSTRACT:
*		Function for checking the CDSL Adapter structure and the CardAL
*		Adapter structure to make sure they are not NULL pointers (make
*		sure they are allocated and valid to a small extent).
*
*	RETURN:
*		BOOLEAN:
*			TRUE -	Adapter structures appear valid.
*			FALSE -	One or both of the structures are NULL pointers.
*		OUT ppCardALAdapter:
*			Where this parameter points to will be set to point to
*			the CardAL Adapter structure if both Adapter structures
*			are valid or NULL otherwise.
*
*	DETAILS:
******************************************************************************/
DWORD cardALChkAdptrStrucsOk(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN OUT CARDAL_ADAPTER_T		** ppCardALAdapter )
{
	if ( pThisAdapter == NULL )
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_WARNING,
		    pThisAdapter->DebugFlag,
		    ("cardALChkAdptrStrucsOk:  Null Adapter Pointer.") );
		*ppCardALAdapter = NULL;
		return ERROR_FATAL( RESULT_ADAPTER_NOT_FOUND );
	}

	*ppCardALAdapter = (CARDAL_ADAPTER_T *) CDALGetModuleHandle(pThisAdapter,CDAL_LINE_CONTROLLER);
	if(*ppCardALAdapter == NULL)
	{
		return ERROR_FATAL( RESULT_ADAPTER_NOT_FOUND );
	}
	else
	{
		return RESULT_SUCCESS;
	}
}
