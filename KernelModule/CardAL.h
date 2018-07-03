/******************************************************************************
*******************************************************************************
****	Copyright (c) 1997, 1998, 1999, 2000, 2001
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
**		CardAL (Card Abstraction Layer)
**
**	FILE NAME:
**		CardAL.h
**
**	ABSTRACT:
**		Card level Abstraction Layer for CDSL.  This file/module abstracts
**		(hides the details of) card specific (CDSL) functions and presents
**		(hopefully) a constant interface to the upper level modules/functions
**		(CardMgmt) no matter what the actual card is (CDSL, ADSL, ISDN, etc.).
**		This file contains the Public/Global prototypes for the CardAL module.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/KernelModule/CardAL.h $
** $Revision: 3 $
** $Date: 7/03/01 4:40p $
*******************************************************************************
******************************************************************************/
#ifndef _CardAL_H_		//	File Wrapper,
#define _CardAL_H_		//	prevents multiple inclusions

/*******************************************************************************
Module Public Datatypes
*******************************************************************************/
#define		CARDAL_ASSERT_TRACE_ALWAYS 	1	// Unconditionally Display Assert Message
#define		CARDAL_ASSERT_TRACE_FULL 	1	// Use Expanded Details for Assert Message Trace
#define		CARDAL_BREAK_ON_ASSERT 		1	// Issue Software Break on Assertion Failure



typedef enum CARD_LINE_STATE_E
{
    CDAL_LINE_STATE_START = 0,
    CDAL_LINE_STATE_INT = CDAL_LINE_STATE_START,
    CDAL_LINE_STATE_INITIALIZE,
    CDAL_LINE_STATE_PERSIST_INITIALIZE,
    CDAL_LINE_STATE_ACTIVE,
    CDAL_LINE_STATE_ERROR,
    CDAL_LINE_STATE_END
} CARD_LINE_STATE_T;


//*
//* Define Macros to Set and Get user Context values from a public data structure
//*
#ifdef DBG
	#define VALIDATE_GET_CONTEXT(DestinContext, Adapter, ModuleId)	\
		if(ModuleId >= MAX_DRIVER_MODULES || Adapter == NULL)					\
		{																		\
			DestinContext = NULL;												\
		}																		\
		else

	#define VALIDATE_SET_CONTEXT(ModuleId)	\
		if(ModuleId < MAX_DRIVER_MODULES)
#else
	#define VALIDATE_GET_CONTEXT(DestinContext, Adapter, ModuleId)
	#define VALIDATE_SET_CONTEXT(ModuleId)
#endif
//GET_MODULE_CONTEXT(pThisAdapter, pCardAl, IF_CARDAL_ID);
#define GET_MODULE_CONTEXT(DestinContext, Adapter, ModuleId)				\
		VALIDATE_GET_CONTEXT(DestinContext, Adapter, ModuleId)			\
		DestinContext = ((CDSL_ADAPTER_T *)Adapter)->pModuleContext[ModuleId];						

#define SET_MODULE_CONTEXT(Adapter, UserContext, ModuleId)					\
	VALIDATE_SET_CONTEXT(ModuleId)											\
	{																		\
		((CDSL_ADAPTER_T *)Adapter)->pModuleContext[ModuleId] = UserContext;\
	}																		




//*
//* Note: This init returns the CardAl Context.  If the failure case, NULL is
//* returned.
//*
VOID *	CardAlInit(
    CDSL_ADAPTER_T		* pThisAdapter	 //* Will be used for backwards compatiblility only!
);

//*
//* Note: The remaining functions accept the CardAl Context, not the psuedo global
//* CDSL_ADAPTER_T type.
//*
VOID		CardAlShutdown(
    void				* pThisCardAl			// Context argument returned by CardAlInit
);

VOID		CardAlReset(
    void				* pThisCardAl			// Context argument returned by CardAlInit
);

NTSTATUS	CardalSetLineState(
    void				* pThisCardAl,			// Context argument returned by CardAlInit
    CARD_LINE_STATE_T	NewState				// Next Line State - Activate, Persistent Activation, Disable
);

//* Begin Legacy Card Al
typedef enum CARDAL_EVENT_HDLR_RSLTS_E
{
    CARDAL_EVENT_RSLT_SUCCESS,
    CARDAL_EVENT_RSLT_PENDING,
    CARDAL_EVENT_RSLT_REFUSED,
    CARDAL_EVENT_RSLT_NULL_ADAPTERS,
    CARDAL_EVENT_RSLT_UNKNOWN_EVENT,
    CARDAL_EVENT_RSLT_FAILED
} CARDAL_EVENT_HDLR_RSLTS_T;


typedef enum CARDAL_EVENT_HDLR_CODES_E
{
    CARDAL_EVENT_FIRST_EVENT_CODE			= 0,

    //	Unsolicited events from lower level
    //	CARDAL_EVENT_CONNECTION_LOST			= 0x100,
    //	CARDAL_EVENT_SERIAL_IF_FRAMING_ERROR,
    CARDAL_EVENT_ADSL_RESPONSE_INTERRUPT	= 0x100,

    //	Commands received from upper level to be sent to lower level
    CARDAL_EVENT_INITIATE_SELFTEST_LONG		= 0x200,
    CARDAL_EVENT_INITIATE_SELFTEST_SHORT,
    CARDAL_EVENT_OPEN_CONNECTION,
    CARDAL_EVENT_CLOSE_CONNECTION,
    CARDAL_EVENT_SEND_DYING_GASP,
    CARDAL_EVENT_DOWNLOAD_DATA_PUMP,

    CARDAL_EVENT_LAST_EVENT_CODE
} CARDAL_EVENT_HDLR_CODES_T;


typedef DWORD (* CARDALEVENTHDLR_CALL_T)
(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    IN CARDAL_EVENT_HDLR_CODES_T	EventCode
);


typedef CARDAL_EVENT_HDLR_RSLTS_T (* CARDAL_EVENT_HDLR_FN_T)
(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    IN CARDAL_EVENT_HDLR_CODES_T	EventCode
);


typedef void (* CARDAL_INTRPT_EVENT_HDLR_FN_T)
(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN DWORD				IntrptMaskBit
);



/*******************************************************************************
Public CardAL  Functions
*******************************************************************************/
NDIS_STATUS CDALSetLineState(
    CDSL_ADAPTER_T				* pThisAdapter,
    CARD_LINE_STATE_T			NewState
);

///////////////////////////////////////////////////////////////////////////////
//	LEGACY Function Prototypes
///////////////////////////////////////////////////////////////////////////////

#if		PROJECTS_MEDIUM == PROJECT_MEDIUM_ADSL
NDIS_STATUS CardALCfgInit(
    IN	CDSL_ADAPTER_T		* pThisAdapter,
    IN	PTIG_USER_PARAMS pParams                // parameter struct from the download app
);

NDIS_STATUS CardALAdapterInit( CDSL_ADAPTER_T		* pThisAdapter );

NDIS_STATUS CardALAdapterHalt( CDSL_ADAPTER_T		* pThisAdapter );

NDIS_STATUS CardALBdDeviceSpecific(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    IN BACK_DOOR_T					* pBackDoorBuf,
    OUT PULONG						pNumBytesNeeded,
    OUT PULONG						pNumBytesWritten );

NDIS_STATUS CardALTextLogMsgPackStr(
    IN char					* szString,
    I_O BD_DRIVER_LOG_T		* pLogMsg );

NDIS_STATUS CardALTextLogMsgQPutMsg(
    IN BD_DRIVER_LOG_T		* pLogMsg );

NDIS_STATUS CardALTextLogMsgQGetMsg(
    I_O BD_DRIVER_LOG_T		* pLogMsg );

NDIS_STATUS CardALTextLogMsgQInit( void );

NDIS_STATUS CardALTextLogMsgQRelease( void );

NDIS_STATUS CardALTextLogMsgQEnqMsg(
    IN BD_DRIVER_LOG_T		* pLogMsg );

void CardALChipUnknownIntrptEventHdlr(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN DWORD				IntrptMaskBit );

GLOBAL NDIS_STATUS CardALGetLineStats(
    IN CDSL_ADAPTER_T	* pThisAdapter,
    OUT BYTE			* LineStatus,
    OUT WORD			* LineDownstreamRate,
    OUT	WORD			* LineUpstreamRate);

GLOBAL NDIS_STATUS CardALGetAdslLineSpeed(
    IN CDSL_ADAPTER_T	* pThisAdapter,
    OUT WORD			* pAdslLineSpeedUpOrFar,
    OUT WORD			* pAdslLineSpeedDownOrNear );

GLOBAL NDIS_STATUS CardALGetAdslLineState(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    OUT BD_ADSL_MODEM_STATUS_T		* pAdslLineStatus );

GLOBAL NDIS_STATUS CardALSetInformation(
    IN CDSL_ADAPTER_T	* pThisAdapter,
    IN PVOID			InfoBuffer,
    IN ULONG			InfoBufferLength,
    OUT PULONG			BytesWritten,
    OUT PULONG			BytesNeeded );

GLOBAL DWORD CardALChipGetMacAddr(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    OUT BYTE				* pBdMacAddr,
    IN DWORD				MacAddrLength );

GLOBAL DWORD CardALChipSetMacAddr(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN BYTE					* pBdMacAddr,
    IN DWORD				MacAddrLength );

GLOBAL void CardALDelayMsec
(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN WORD 				Duration		// In milliseconds
) ;

//===================================================================================
//	This is the Card Abstraction Layer vector table. These are the the generic
//	functions that will be called from within the Card Management module.
//	The function prototypes are defined in CardMgmt.h. If a module requires one
//	of these predefined functions then its name is placed into the #define
//	otherwise NULL is entered. In this way a consistent interface is maintained
//	with the Card Management module.
//===================================================================================
	#define CARDAL_DESCRIPTION	"Card Abstraction Layer Module"

	#define CARDAL_FNS {  										\
		CARDAL_DESCRIPTION,	/*Module Description			*/	\
		CardALCfgInit,		/*Module CfgInit Function	 	*/	\
		CardALAdapterInit,	/*Module AdapterInit Function	*/	\
		CardALAdapterHalt, 	/*Module Shutdown Function		*/ 	\
		CardALBdDeviceSpecific /*Ioctl handler				*/	\
	}

#endif	// PROJECTS_MEDIUM == PROJECT_MEDIUM_ADSL

DWORD CmgmtGetPeerList(
    VOID				* ExternHandle,
    DWORD				** MessageList,
    DWORD				* MaxMessages
);

#endif		//	#ifndef _CardAL_H_
