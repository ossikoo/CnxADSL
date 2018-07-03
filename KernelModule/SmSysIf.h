/******************************************************************************
********************************************************************************
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
**		ChipAl
**
**	FILE NAME:
**		SmSysIf.h
**
**	ABSTRACT:
**		Public header for nterface providing mapping from various System  
**		Interfaces to Driver Call back routines.
**
**	DETAILS:
**		NOTE  Only Common data types defined by Common.H should be included
**		in this file.  There should be no concept of NDIS, WDM, ADSL, etc...
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/SmSysIf.h $
**	$Revision: 1 $
**	$Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
#ifndef 	_SMSYSIF_H_						// File Wrapper
#define		_SMSYSIF_H_
#include "Filenames.h"
#include "Version.h"

#if ! defined(CHIPAL_REMOTE)
	#define CHIPAL_REMOTE 0
#endif 
#if PROJECTS_NAME == PROJECT_NAME_MADMAX

	#define DRIVER_HAL_INSTALLED 	1
	#define DBG_TERMINAL_INSTALLED	1
	#define COM_CHANNEL_INSTALLED 	1
	#define API_CHAN_INSTALLED		0
	#define IRP_CHAN_INSTALLED		1
	#define PEER_CHAN_INSTALLED		1
	#define MGMT_CHAN_INSTALLED		1
	#define CHIPAL_REMOTE_CHAN_INSTALLED 0
	#define	MAX_CHANNELS			0x00000008			// Max Active Command Channels for build

	#define PEER_CHAN_NAME			TIGRIS_NDIS_EXT_DRIVER_NAME_WC // Name of Ndis Driver


#elif PROJECTS_NAME == PROJECT_NAME_NILE_TEST

	#define DRIVER_HAL_INSTALLED 	1
	#define DBG_TERMINAL_INSTALLED	1
	#define COM_CHANNEL_INSTALLED 	1
	#define API_CHAN_INSTALLED		0
	#define IRP_CHAN_INSTALLED		0
	#define PEER_CHAN_INSTALLED		0
	#define MGMT_CHAN_INSTALLED		0
	#define CHIPAL_REMOTE_CHAN_INSTALLED 0
	#define PEER_CHAN_NAME			TIGRIS_NDIS_EXT_DRIVER_NAME_WC // Name of Ndis Driver
	#define	MAX_CHANNELS			0x00000005			// Max Active Command Channels for build

#elif PROJECTS_NAME == PROJECT_NAME_TIGRIS
	#if ! defined(EXTERNAL_DRIVER_NAME_WC)
	#define EXTERNAL_DRIVER_NAME_WC	NDIS_EXTERNAL_DRIVER_NAME_WC
	#endif
	#define DRIVER_HAL_INSTALLED 	0
	#define DBG_TERMINAL_INSTALLED	0
	#define COM_CHANNEL_INSTALLED 	1
	#define API_CHAN_INSTALLED		1
	#define IRP_CHAN_INSTALLED		0
	#define PEER_CHAN_INSTALLED		0
	#define MGMT_CHAN_INSTALLED		0
	#define CHIPAL_REMOTE_CHAN_INSTALLED 1
	#define		MAX_CHANNELS		0x0000000A			// Max Active Command Channels for build	
	#define MGMT_CHAN_INSTALLED		0
	#define PEER_CHAN_NAME			TIGRIS_P46ARB_DRIVER_NAME_WC	// Name of P46 Arb Driver (Parent)
#elif PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
	#if ! defined(EXTERNAL_DRIVER_NAME_WC)
	#define EXTERNAL_DRIVER_NAME_WC	NDIS_EXTERNAL_DRIVER_NAME_WC
	#endif
	#define DRIVER_HAL_INSTALLED 	0
	#define DBG_TERMINAL_INSTALLED	0
	#define COM_CHANNEL_INSTALLED 	1
	#define PEER_CHAN_NAME			L""
	#define API_CHAN_INSTALLED		0
	#define IRP_CHAN_INSTALLED		0
	#define PEER_CHAN_INSTALLED		0
	#define MGMT_CHAN_INSTALLED 	0
	#define CHIPAL_REMOTE_CHAN_INSTALLED 0
	#define		MAX_CHANNELS		0x00000005			// Max Active Command Channels for build	
#elif PROJECTS_NAME == PROJECT_NAME_WDM_FILTER

	#define DRIVER_HAL_INSTALLED 	1
	#define DBG_TERMINAL_INSTALLED	1
	#define COM_CHANNEL_INSTALLED 	1
	#define API_CHAN_INSTALLED		0
	#define IRP_CHAN_INSTALLED		1
	#define PEER_CHAN_INSTALLED		1
	#define MGMT_CHAN_INSTALLED		1
	#define CHIPAL_REMOTE_CHAN_INSTALLED 0
	#define	MAX_CHANNELS			0x00000008			// Max Active Command Channels for build
	#define PEER_CHAN_NAME			TIGRIS_NDIS_EXT_DRIVER_NAME_WC // Name of Ndis Driver
#else

	#if ! defined(EXTERNAL_DRIVER_NAME_WC)
	#define EXTERNAL_DRIVER_NAME_WC	NDIS_EXTERNAL_DRIVER_NAME_WC
	#endif
	#define DRIVER_HAL_INSTALLED 	0
	#define DBG_TERMINAL_INSTALLED	0
	#define COM_CHANNEL_INSTALLED 	1
	#define API_CHAN_INSTALLED		0
	#define IRP_CHAN_INSTALLED		0
	#define MGMT_CHAN_INSTALLED		0
	#define CHIPAL_REMOTE_CHAN_INSTALLED 0
#endif

#ifdef DBG
	#define EXTENDED_VALIDATION	  	1
#else
	#define EXTENDED_VALIDATION	  0
#endif

#ifndef MAX_VALUE
	#define MAX_VALUE(_v1,_v2)	( ((_v1) > (_v2)) ? (_v1) : (_v2) ) 
	#define MIN_VALUE(_v1,_v2)	( ((_v1) < (_v2)) ? (_v1) : (_v2) ) 
#endif

#if DRIVER_HAL_INSTALLED
	#if PROJECTS_NAME == PROJECT_NAME_MADMAX
		#include "ChipAlBusCtlWdm.h"
	#elif  PROJECTS_NAME == PROJECT_NAME_NILE_TEST
		#include "ChipAlBusCtlNile.h"
	#endif
#endif

#if DBG_TERMINAL_INSTALLED
	#include "DbgTerminal.h"
#endif

//*
//* Include any module public header files that can contain message definitons
//*

// #include HERE
#define DRIVER_DEVICE_PATH_WC	L"\\Device\\"
#define DRIVER_SYMBOLIC_PATH_WC L"\\DosDevices\\"
#define DRIVER_DEVICE_PATH		"\\Device\\"
#define DRIVER_SYMBOLIC_PATH 	"\\DosDevices\\"


/******************************************************************************
 Data Types
******************************************************************************/
#define CONTEXT_MEM_FLAGS	NON_PAGED_SYSTEM_SPACE_MEM



//*
//* Primary Bitmap used to map enumerated commands to external interfaces.
//*
typedef struct CHAN_COMMAND_B
{
	union COMMAND_U
	{
		DWORD	Command;

		struct
		{
DWORD		Event:		16;
DWORD		Channel:	8;
DWORD		Type:		8;
		} Element;
	}CommandU;
} CHAN_COMMAND_T;


//============================================================================
// 	COMMAND_CHAN_xxx macros are used to create values that can be assigned to
//	the Command value of the CHAN_COMMAND_T data type.  Also, values are used
//	to construct SYS_MESS_T definitions. Max Values are zero relative.
//	NOTE:
//	All COMMAND_CHAN_xxx macros should consider a max value of MAX_CHANNELS.
//	Anything exceed MAX_CHANNELS is not reachable.
//============================================================================

// Command Channels for all drivers
#define		COMMAND_CHAN_ALL		0x00FF0000
#define 	COMMAND_CHAN_COM0		0x00000000
#define 	COMMAND_CHAN_COM1		0x00010000
#define 	COMMAND_CHAN_COM2		0x00020000
#define 	COMMAND_CHAN_COM3		0x00030000
#define 	COMMAND_CHAN_INTERNAL	0x00040000

// Project Specific Channels
#if PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
	#define 	COMMAND_CHAN_NDIS_QUERY	0x00050000
	#define 	COMMAND_CHAN_API		0x00060000
	#define 	COMMAND_CHAN_PEER		0x00070000			// CHIPAL of some other Driver
	#define 	COMMAND_CHAN_NDIS_SET	0x00000000			// Unused
	#define 	COMMAND_CHAN_CHIPAL		0x00000000          // Unused 
	#define 	COMMAND_CHAN_IRP		0x00000000			// System IRP channel
	#define 	COMMAND_CHAN_MANAGEMENT	0x00000000

#elif PROJECTS_NAME == PROJECT_NAME_TIGRIS
	#define 	COMMAND_CHAN_NDIS_QUERY	0x00050000
	#define 	COMMAND_CHAN_API		0x00060000
	#define 	COMMAND_CHAN_PEER		0x00070000			// CHIPAL of some other Driver
	#define 	COMMAND_CHAN_NDIS_SET	0x00080000
	#define 	COMMAND_CHAN_CHIPAL		0x00090000          // remote chipal
	#define 	COMMAND_CHAN_IRP		0x00000000			// System IRP channel
	#define 	COMMAND_CHAN_MANAGEMENT	0x00000000

#elif PROJECTS_NAME == PROJECT_NAME_MADMAX
	#define 	COMMAND_CHAN_NDIS_QUERY	0x00000000			// Unused
	#define 	COMMAND_CHAN_API		0x00000000			// Unused
	#define 	COMMAND_CHAN_PEER		0x00050000			// CHIPAL of some other Driver
	#define 	COMMAND_CHAN_IRP		0x00060000			// System IRP channel
	#define 	COMMAND_CHAN_MANAGEMENT	0x00070000
	#define 	COMMAND_CHAN_NDIS_SET	0x00000000			// Unused
	#define 	COMMAND_CHAN_CHIPAL		0x00000000          // Unused 

#elif PROJECTS_NAME == PROJECT_NAME_NILE_TEST
	#define 	COMMAND_CHAN_NDIS_QUERY	0x00000000			// Unused
	#define 	COMMAND_CHAN_API		0x00000000			// Unused
	#define 	COMMAND_CHAN_PEER		0x00050000			// CHIPAL of some other Driver
	#define 	COMMAND_CHAN_IRP		0x00000000			// System IRP channel
	#define 	COMMAND_CHAN_MANAGEMENT	0x00000000
	#define 	COMMAND_CHAN_NDIS_SET	0x00000000			// Unused
	#define 	COMMAND_CHAN_CHIPAL		0x00000000          // Unused 
#elif PROJECTS_NAME == PROJECT_NAME_WDM_FILTER
	#define 	COMMAND_CHAN_NDIS_QUERY	0x00000000			// Unused
	#define 	COMMAND_CHAN_API		0x00000000			// Unused
	#define 	COMMAND_CHAN_PEER		0x00050000			// CHIPAL of some other Driver
	#define 	COMMAND_CHAN_IRP		0x00060000			// System IRP channel
	#define 	COMMAND_CHAN_MANAGEMENT	0x00070000
	#define 	COMMAND_CHAN_NDIS_SET	0x00000000			// Unused
	#define 	COMMAND_CHAN_CHIPAL		0x00000000          // Unused 
#else
	#define 	COMMAND_CHAN_NDIS_QUERY	0x00000000			// Unused
	#define 	COMMAND_CHAN_API		0x00000000			// Unused
	#define 	COMMAND_CHAN_PEER		0x00000000			// CHIPAL of some other Driver
	#define 	COMMAND_CHAN_IRP		0x00000000			// System IRP channel
	#define 	COMMAND_CHAN_MANAGEMENT	0x00000000
	#define 	COMMAND_CHAN_NDIS_SET	0x00000000			// Unused
	#define 	COMMAND_CHAN_CHIPAL		0x00000000          // Unused 

#endif


#define		COMMAND_TYPE_DATA		0x00000000			// Data Buffer contains no embedded commands
#define		COMMAND_TYPE_COMMAND	0x01000000			// Data Buffer contains embedded commands
#define 	COMMAND_TYPE_MAX		0x00000002			// Max Channel Types

#define		COMMAND_MASK			0x0000FFFF			// Used to Mask off embedded Channel and Type info
#define 	COMMAND_TYPE_VALUE(c)	((c & 0xff000000) >> 24 )

#define		COMMAND_CHAN_VALUE(c)	((c & COMMAND_CHAN_ALL) >> 16 )
#define		COMMAND_CHAN_SET(v,c)	((v & ~COMMAND_CHAN_ALL) | (c << 16) )
//*
//* Messages visible to driver public modules.  These modules may define
//* messages to be inserted into this enum.  The general structure of the definiton follows
//* the CHAN_COMMAND_T bit map. The first 16 bits are enumerated over all message definitons.
//* The high order 16 are used by the SmSysIf module to map events to various external
//* edges.  Any module defining events Must follow the below example, including incrementing the new
//* event value.
//*

//* These message Events are platform / driver independent.  If used in SYS_MESS_T, make
//* sure not to exceed MESS_END_M or MAX_CHAN_MESSAGES

#define EVENT_READ		0x0000
#define EVENT_WRITE		0x0001

#if COM_CHANNEL_INSTALLED
typedef enum SYS_MESS_E
{
    MESS_START = 0,

    /* Begin Messages that are Data Only */
    MESS_DATA = MESS_START,
    MESS_COMM0_DATA	= COMMAND_CHAN_COM0 | COMMAND_TYPE_DATA 	| MESS_DATA,				/* 0xnnnn0000 */
    MESS_COMM1_DATA	= COMMAND_CHAN_COM1 | COMMAND_TYPE_DATA 	| MESS_DATA,				/* 0xnnnn0000 */
    MESS_COMM2_DATA	= COMMAND_CHAN_COM2 | COMMAND_TYPE_DATA 	| MESS_DATA,				/* 0xnnnn0000 */
    MESS_COMM3_DATA	= COMMAND_CHAN_COM3 | COMMAND_TYPE_DATA 	| MESS_DATA,				/* 0xnnnn0000 */

    /* Begin messages that contain embedded Commands */
    MESS_COMMAND_BEGIN = (COMMAND_MASK & MESS_COMM2_DATA)	+ 1,
    MESS_COMM0_OPEN	= COMMAND_CHAN_COM0 | COMMAND_TYPE_COMMAND	| MESS_COMMAND_BEGIN,		/* 0xnnnn0001 */
    MESS_COMM1_OPEN	= COMMAND_CHAN_COM1 | COMMAND_TYPE_COMMAND	| MESS_COMMAND_BEGIN,		/* 0xnnnn0001 */
    MESS_COMM2_OPEN	= COMMAND_CHAN_COM2 | COMMAND_TYPE_COMMAND	| MESS_COMMAND_BEGIN,		/* 0xnnnn0001 */
    MESS_COMM3_OPEN	= COMMAND_CHAN_COM3 | COMMAND_TYPE_COMMAND	| MESS_COMMAND_BEGIN,		/* 0xnnnn0001 */
    MESS_COMM0_CLOSE= (COMMAND_CHAN_COM0 | COMMAND_TYPE_COMMAND	| MESS_COMMAND_BEGIN) + 1,	/* 0xnnnn0002 */
    MESS_COMM1_CLOSE= (COMMAND_CHAN_COM1 | COMMAND_TYPE_COMMAND	| MESS_COMMAND_BEGIN) + 1,	/* 0xnnnn0002 */
    MESS_COMM2_CLOSE= (COMMAND_CHAN_COM2 | COMMAND_TYPE_COMMAND	| MESS_COMMAND_BEGIN) + 1,	/* 0xnnnn0002 */
    MESS_COMM3_CLOSE= (COMMAND_CHAN_COM3 | COMMAND_TYPE_COMMAND	| MESS_COMMAND_BEGIN) + 1,	/* 0xnnnn0002 */

    //* Ndis Query Information Handler
    MESS_NDIS_QUERY_COMMAND = COMMAND_CHAN_NDIS_QUERY | COMMAND_TYPE_COMMAND	| MESS_START,				/* 0xnnnn0000 */

    //* Ndis Set Information Handler
    MESS_NDIS_SET_COMMAND = COMMAND_CHAN_NDIS_SET | COMMAND_TYPE_COMMAND	| MESS_START,				/* 0xnnnn0000 */

    //* Used to communicate to private USER API commands
    MESS_API_COMMAND = COMMAND_CHAN_API | COMMAND_TYPE_COMMAND	| MESS_START,				/* 0xnnnn0000 */

    //* Remote Device Driver Private Functions
    MESS_DEV_SPECIFIC = COMMAND_CHAN_PEER | COMMAND_TYPE_COMMAND | MESS_START,				/* 0xnnnn0000 */

    //* System IRP Channel
    MESS_IRP = COMMAND_CHAN_IRP | COMMAND_TYPE_COMMAND | MESS_START,

    MESS_INTERNAL_START 	= MESS_START,
    MESS_INTERNAL_TEST 		= COMMAND_CHAN_INTERNAL | COMMAND_TYPE_COMMAND | MESS_START,		/* 0xnnnn0000 */

    //* Internal Management Message Channel
    MESS_MGMT = COMMAND_CHAN_MANAGEMENT | COMMAND_TYPE_COMMAND | MESS_START,


    // Message to lower driver chipal

    MESS_CHIPAL_INTF = COMMAND_CHAN_CHIPAL	 | COMMAND_TYPE_COMMAND | MESS_START,
    MESS_END_M = MESS_COMM3_CLOSE + 1

} SYS_MESS_T;
#endif

//*
//* Preprocessor similar to MESS_END_M.  This indicates the max number of Channel messages. To set,
//* add 1 last MESS_xxx value (first 16 bits only).
//*
#define MAX_CHAN_MESSAGES		3

//============================================================================
//	Module ID is used to indicate the callback priority of a specific Module.
//	Modules	are called in the order listed. If a module is to provide a
//	a callback function it must add its 'MODULE_ID' to this list. The highest
//	priority is MODULE_ID = 0; next is 1 and so on...
//============================================================================
typedef enum _SYS_IF_MODULE_ID
{
    IF_MODULE_ID_START = 0,
    IF_SYS_MGMT_ID = IF_MODULE_ID_START,	//highest priority module
    IF_CARDMGMT_ID = 0x1,					// Leave space for other modules
    IF_CARDAL_ID = 0x2,					//
    IF_CHIPAL_ID = 0x3,					//
    IF_DBG_TERM_ID = 0x4,					//
    IF_MODULE_ID_END						//lowest priority module
} SYS_IF_MODULE_ID_T;


typedef
NTSTATUS (* FN_SYS_RECEIVE_HANDLER)(
    VOID					* UserContext,
    CHAR					* Buffer,
    DWORD					Length
);

typedef
NTSTATUS (* FN_IO_COMPLETE_HANDLER_T)(
    IN void					* pUserContext,
    IN NTSTATUS				Status,
    IN CHAR					* Buffer,
    IN DWORD				Length
);

GLOBAL NTSTATUS SmSysIfAddMessageHandler(
    VOID					* pThisAdapter,	// Ndis Adapter Context
    SYS_MESS_T				Message,		// Unique to each channel
    DWORD					SubKey,			// Identifies embedded message, if used.
    DWORD					Length,			// Bytes in Message affected
    FN_SYS_RECEIVE_HANDLER	ReceiveHandler,	// Handler to be called for new messages
    VOID					* Context,		// Context returned when Receive Handler is Called
    SYS_IF_MODULE_ID_T		ModuleId);		// Used to identify and order channel during callback



//*
//*  The following Defines may need to have seperate .h file
//*

#define	STATUS_SUCCESS_ABORT
//*
//* Enumeration for all Embedded Eessages
//*
typedef enum SYS_IF_COMMAND_E
{
    SYS_IF_COMMAND_START = 0,
    SYS_IF_OPEN_CHANNEL = SYS_IF_COMMAND_START,
    SYS_IF_CLOSE_CHANNEL,
    SYS_IF_COMMAND_END
} SYS_IF_COMMAND_T;

//*
//* Data Definitions for all Embedded Messages
//*

//*
//* Name:
//*		COM_CHANNEL_OPERATION_T
//* Commands:
//*		SYS_IF_OPEN_CHANNEL
//*		SYS_IF_CLOSE_CHANNEL
typedef struct COM_CHANNEL_OPERATION_S
{
	DWORD	Channel;
} COM_CHANNEL_OPERATION_T;

//*
//* Structure combining Embedded Commands and Data
//*
typedef struct SYS_IF_COMMAND_BUFF_S
{
	SYS_IF_COMMAND_T Command;

	//union SYS_IF_COMMAND_BUFF_U
	//{
	//	COM_CHANNEL_OPERATION_T Operation;
	//};
} SYS_IF_COMMAND_BUFF_T;

typedef struct MESSAGE_HANDLER_S
{
	SYS_MESS_T				UserMessage;	// Unique to each channel
	DWORD					SubKey;			// Identifies embedded messages, if used.
	DWORD					Length;			// Bytes in Message affected
	FN_SYS_RECEIVE_HANDLER	ReceiveHandler;	// Handler to be called for new messages
	SYS_IF_MODULE_ID_T		ModuleId;		// Used to identify and order channel during callback
}MESSAGE_HANDLER_T;

//*
//* End Embedded Command/Data Definitions
//*



GLOBAL NTSTATUS SmSysIfSendMessage(
    VOID					* pThisAdapter,	// Ndis Adapter Context
    SYS_MESS_T				UserMessage,	// Unique to each channel
    CHAR					* Buffer,		// Message Buffer
    DWORD					Length,			// Bytes in Message affected
    FN_IO_COMPLETE_HANDLER_T SendComplete,	// Optional - Called when an Async Message is finished processing
    IN void					* pUserContext	// Optional - Callback context for SendComplete
);

GLOBAL NTSTATUS SmSysIfIndicateComplete(
    VOID					* Context,		// NDIS (CDSL Adapter) Context
    DWORD					UserChan,		// Unique to each channel
    CHAR					* pBuffer,		// Message Buffer
    DWORD					Length,			// Bytes in Message affected
    NTSTATUS				Status);		// Final Status

GLOBAL NTSTATUS SmSysIfTrace(
    VOID		* UserContext,
    CHAR		* Buffer,
    DWORD		Length
);

GLOBAL NTSTATUS SmSysIfGetHandle(
    VOID					* pThisAdapter,	// Ndis Adapter Context
    VOID					** Handle		// Handle for Interface
);

//============================================================================
//	This function will broadcast a message to all driver modules who have
//	registered to receive a message of type 'UserMessage'...
//============================================================================
GLOBAL NTSTATUS SmSysIfBroadcastMessage(
    VOID					* Context,		// NDIS (CDSL Adapter) Context
    SYS_MESS_T				UserMessage,	// Unique to each channel
    DWORD					SubKey,			// Embedded Message key
    CHAR					* pBuffer,		// Message Buffer
    DWORD					* Length);		// Bytes in Message affected

GLOBAL NTSTATUS SmSysIfAddMessageTable(
    VOID				* pContext,			// User Context
    MESSAGE_HANDLER_T	* pTable,
    VOID				* RecContext,		// Context returned when Receive Handler is Called
    DWORD				Entries
);

extern BOOLEAN win98;

#endif									// File Wrapper 
