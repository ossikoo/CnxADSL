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
**		ChipAL
**
**	FILE NAME:
**		SmSysIfv.h
**
**	ABSTRACT:
**		Shared Common Data types for the system interface
**
**	DETAILS:
**		NOTE:	Use only COMMON data types.  Do NOT use NDIS, WDM, or other 
**				OS specific types.
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.4_ATM_Driver/KernelModule/SmSysIfv.h $
**	$Revision: 2 $
**	$Date: 4/25/01 8:52a $
*******************************************************************************
******************************************************************************/
#ifndef _SMSYSIFV_H_					// File Wrapper
#define _SMSYSIFV_H_
#include "Product.h"

#if  defined(NDIS_MINIPORT_DRIVER) && !IRP_INTERFACE_NDIS
	#define		HOOK_DISPATCH_TABLE			1
#else
	#define		HOOK_DISPATCH_TABLE			0 
#endif

#include "SmLnxIf.h"

/******************************************************************************
 Data Types
******************************************************************************/

//*
//* Begin Internal Function Handler definitions
//*
#define MAX_CHAN_DESCRIPTION	40

typedef
NTSTATUS (* FN_MODULE_INIT)(
    IN	VOID 					* AdapterContext,
    OUT VOID 					** UserContext
);

typedef
NTSTATUS (* FN_MODULE_SHUTDOWN)(
    IN	VOID 					* UserContext
);

typedef struct MODULE_FUNCTION_S
{
	#ifdef DBG
	CHAR						Description[MAX_CHAN_DESCRIPTION];
	#endif
	FN_MODULE_INIT				Init;
	FN_MODULE_SHUTDOWN			Shutdown;
} MODULE_FUNCTION_T;


#include	"UtilDbg.h"
#include	"SmSysIf.h"
#include	"SmLnxIf.h"

//*
//* Driver Module Public Definitions
//*

//*
//* NOTE - The first two elements of MESSAGE_ORDERED_TABLE_S and
//* MESSAGE_MAPPED_TABLE_S must be the same. Both must appear
//* as the first elements in  DEVICE_CHANNEL_T
//*
typedef struct MESSAGE_ORDERED_TABLE_S
{
	LIST_ENTRY					pMessage;
	DWORD						Length;
	DWORD						Key;
} MESSAGE_ORDERED_TABLE_T;

typedef struct MESSAGE_MAPPED_TABLE_S
{
	LIST_ENTRY					pMessage;
	DWORD						Length;
} MESSAGE_MAPPED_TABLE_T;

typedef struct MAPPED_TABLE_SCALER_
{
	MESSAGE_MAPPED_TABLE_T		Index[1][1];
} MAPPED_TABLE_SCALER_T;

//*
//* DEVICE_CHANNEL_T represents the Dynamic portion of the Message channel. The
//* static Bindings or Configuration parameters are contained in the
//* CHAN_METHOD_T structure.
//*
typedef struct DEVICE_CHANNEL_S
{
	union
	{
		MESSAGE_MAPPED_TABLE_T		pMessageMapped[MAX_CHAN_MESSAGES];	/* Head Nodes - Mapped Storage*/
		MESSAGE_MAPPED_TABLE_T		* pMessageMappedTable;
		MESSAGE_ORDERED_TABLE_T		* pMessageOrdered;					/* Ordered List Storage Format */
	} MsgU;

	DWORD						MaxMessages;
	DWORD						MaxCols;
	struct CDSL_ADAPTER_S*      pAdapter;
	PFILE_OBJECT				pChanFileObject;						/* External File Object */
	SPIN_LOCK 					IoLock;
	LIST_ENTRY					PendingIo;								/* Pending Io Queue	*/
}DEVICE_CHANNEL_T;

//* PreDeclaration to allow function pointer to reference CHAN_METHOD_T
struct CHAN_METHOD_S;
struct MSG_HANDLING_S;

typedef
NTSTATUS (*FN_MESSAGE_CHAN_GEN)(
    struct CHAN_METHOD_S 		* pChanMethod,
    DEVICE_CHANNEL_T 	* pChannel,
    VOID				* ExternHandle
);

typedef
DWORD (*FN_GET_MESSAGE_TABLE)(
    VOID				* ExternHandle,
    DWORD				** MessageList,
    DWORD				* MaxMessages
);

typedef
NTSTATUS (*FN_EXTERNAL_SEND_HANDLER)(
    struct MSG_HANDLING_S		* pMsg,
    BYTE						* Buffer,
    DWORD						Length
);


typedef
NTSTATUS (*FN_EXTERNAL_OPEN_CHAN)(
    struct MSG_HANDLING_S			* pMsg,
    PFILE_OBJECT					* ppChanFileObject,
    struct CDSL_ADAPTER_S			** ppAdapter
);

typedef
VOID (*FN_EXTERNAL_CLOSE_CHAN)(
    struct MSG_HANDLING_S			* pMsg,
    PFILE_OBJECT					* ppChanFileObject,
    struct CDSL_ADAPTER_S			* pAdapter
);


typedef
NTSTATUS (*FN_ADD_HANDLER)(
    IN struct CHAN_METHOD_S			* pChanMethod,	// Functions for this Channel
    IN DEVICE_CHANNEL_T			* pChan,		// Message Channel
    IN CHAN_COMMAND_T			Message,		// Unique to each channel
    IN DWORD					SubKey,			// Subkey used to Identify Encoded messages
    IN DWORD					Length,			// Bytes in Message affected
    IN FN_SYS_RECEIVE_HANDLER	ReceiveHandler,	// Handler to be called for new messages
    IN VOID						* Context,		// Context returned when Receive Handler is Called
    IN SYS_IF_MODULE_ID_T		ModuleId		// Used to identify and order channel during callback
);

typedef
NTSTATUS (*FN_GET_HANDLER_LIST)(
    DEVICE_CHANNEL_T		* pChan,			// Message Channel
    CHAN_COMMAND_T			Message,			// Unique to each channel
    DWORD					SubKey,
    LIST_ENTRY				** ppMessageList	// Index of message in channel
);

typedef struct IO_COMPLETE_S
{
	LIST_ENTRY				NextIo;				// Link for building queue
	IN	VOID				* ExternMessId;		// Optional - External Handle used for Asynchronous Callbacks
	CHAR					* InfoBuffer;		// Optional - Internal Handle used for Asynchronous CallBacks
	FN_IO_COMPLETE_HANDLER_T SendComplete;		// Optional - Internal Message Handler
	VOID					* InternalContext;	// Optional - Context for internal message handler
	IN	DWORD				Length;				// Length of Message
}IO_COMPLETE_T;

typedef struct DRIVER_MESSAGE_LIST_S
{
	LIST_ENTRY					NextHandler;
	DWORD						ModuleId;
	DWORD						MessageType;
	DWORD						TotalLength;
	FN_SYS_RECEIVE_HANDLER		RxHandler;
	VOID						* UserContext;
} DRIVER_MESSAGE_LIST_T;

//*
//* CHAN_METHOD_CONFIG These are default configuration parmeters for the CHAN_METHOD_T
//* functions.  Also, they are used to control the operations of the channel
//* via a call to SmSysIfSetConfig
//*
typedef struct CHAN_METHOD_CONFIG_S
{
	BOOL						ActivateChannel;
	BOOL						DeActivateChannel;
}	CHAN_METHOD_CONFIG_T;

//*
//* CHAN_METHOD_T Static bindings that define the functionality of a message
//* channel. To create a new channel, you can mix and match the the functions
//* and configurations that get defined globally by pgChanMethods.
//*
typedef struct CHAN_METHOD_S
{
	#ifdef DBG
	CHAR						Description[MAX_CHAN_DESCRIPTION];
	#endif
	// Function Pointers
	FN_MESSAGE_CHAN_GEN			InitMessageChan;		// Required - Setup Message handler Lists
	FN_MESSAGE_CHAN_GEN			ShutdownMessageChan;	// Required - Release All Event handlers and  Alloc'ed Memory
	FN_GET_MESSAGE_TABLE		GetMessageTable;		// Optional - Used when definition of Message is extern to SmSysif
	FN_EXTERNAL_SEND_HANDLER	ExtSendHandler;			// Optional - Sends a Message External Driver entity
	FN_ADD_HANDLER				AddNewHandler;			// Required - Adds a call back function to message
	FN_GET_HANDLER_LIST			GetMessageList;			// Required - Gets the Head of the CallBack list for a message
	FN_IO_COMPLETE_HANDLER_T	IndicateComplete;		// Optional - External Interface should be informed when Message is Async
	FN_EXTERNAL_OPEN_CHAN		OpenChan;				// Optional - Open on Demand External Channels
	FN_EXTERNAL_CLOSE_CHAN		CloseChan;				// Optional - Auto Close Channel (inverse of OpenChan)

	// Configuration Parameters controlling the channel Function Pointers.  Default
	// values are always 0.
	//CHAN_METHOD_CONFIG_T		Param;					// Close Channel When I/O Complete

} CHAN_METHOD_T;

typedef struct MSG_HANDLING_S
{
	LIST_ENTRY						List;
	struct CDSL_ADAPTER_S*			pThisAdapter;
	DWORD							DevObjType;			// Type of Device Object
	CHAN_METHOD_T					pChanMethod[MAX_CHANNELS];
	DEVICE_CHANNEL_T				pChannel[MAX_CHANNELS];
	DWORD							MaxChannels;
	DWORD							MaxTypes;
	DWORD							MaxMessages;
} MSG_HANDLING_T;

//*
//* Enumerator Definitions
//*
#define DEV_TYPE_FDO	0x00010000
#define DEV_TYPE_PDO	0x00020000
#define DEV_TYPE_MASK	0x0000FFFF

typedef enum EDEVOBJTYPE
{
    ArmFdo = DEV_TYPE_FDO,
    DslFdo,
    UnknownFdo,
    ModemChildPdo = DEV_TYPE_PDO + (UnknownFdo & DEV_TYPE_MASK) + 1,
    DslChildPdo
} DEV_OBJ_TYPE_T;

#ifndef DEVICE_TYPE
//* Temporary Defines from WDM.h.  Need to rearrange this file later.

	#define DEVICE_TYPE ULONG

typedef enum _SYSTEM_POWER_STATE {
    PowerSystemUnspecified = 0,
    PowerSystemWorking,
    PowerSystemSleeping1,
    PowerSystemSleeping2,
    PowerSystemSleeping3,
    PowerSystemHibernate,
    PowerSystemShutdown,
    PowerSystemMaximum
} SYSTEM_POWER_STATE, *PSYSTEM_POWER_STATE;

typedef enum {
    PowerActionNone,
    PowerActionReserved,
    PowerActionSleep,
    PowerActionHibernate,
    PowerActionShutdown,
    PowerActionShutdownReset,
    PowerActionShutdownOff
} POWER_ACTION, *PPOWER_ACTION;

typedef enum _DEVICE_POWER_STATE {
    PowerDeviceUnspecified = 0,
    PowerDeviceD0,
    PowerDeviceD1,
    PowerDeviceD2,
    PowerDeviceD3,
    PowerDeviceMaximum
} DEVICE_POWER_STATE, *PDEVICE_POWER_STATE;

typedef union _POWER_STATE {
	SYSTEM_POWER_STATE SystemState;
	DEVICE_POWER_STATE DeviceState;
} POWER_STATE, *PPOWER_STATE;

typedef enum _POWER_STATE_TYPE {
    SystemPowerState,
    DevicePowerState
} POWER_STATE_TYPE, *PPOWER_STATE_TYPE;

typedef struct _DEVICE_CAPABILITIES {
	USHORT Size;
	USHORT Version;  // the version documented here is version 1
ULONG Spare1:2;
ULONG LockSupported:1;
ULONG EjectSupported:1;
ULONG Removable:1;
ULONG DockDevice:1;
ULONG UniqueID:1;
ULONG SilentInstall:1;
ULONG RawDeviceOK:1;
ULONG SurpriseRemovalOK:1;
ULONG Reserved: 22;

	ULONG Address;
	ULONG UINumber;

	DEVICE_POWER_STATE DeviceState[PowerSystemMaximum];
	SYSTEM_POWER_STATE SystemWake;
	DEVICE_POWER_STATE DeviceWake;
	ULONG D1Latency;
	ULONG D2Latency;
	ULONG D3Latency;
} DEVICE_CAPABILITIES, *PDEVICE_CAPABILITIES;
#endif

//*
//* Device Object Configuration
//*
typedef struct DEVICE_OBJECT_CONFIG_S
{
	IN ULONG			DeviceExtensionSize;
	IN char*			DeviceName;
	IN DEVICE_TYPE		DeviceType;
	IN ULONG			DeviceCharacteristics;
	IN BOOLEAN			Exclusive;
} DEVICE_OBJECT_CONFIG_T;

typedef struct DRIVER_EXTENSION_PRIV_S
{

	struct CDSL_ADAPTER_S*      pAdapter;
	char*		RegistryPath;					// Pointer to registry path passed to DriverEntry

	//* Enumerator Functions
	DWORD				ModemChildren;
	DWORD				DslChildren;

	//* Embedded Firmware Section
	PVOID				CodeSectionHandle;				// Used for locking down the pageable code
	PVOID				DataSectionHandle;				// Used for locking down the pageable data
	PVOID				FirmwareAddr;					// Address of firmware
	ULONG				FirmwareSize;					// Size of firmware

} DRIVER_EXTENSION_PRIV_T;

extern	LIST_ENTRY 	gHeadIfList;
extern	SPIN_LOCK	gIoLock;


/******************************************************************************
 Modular Functions
******************************************************************************/

NTSTATUS SmSysIfInit(
    struct CDSL_ADAPTER_S*		pAdapter
);

NTSTATUS SmInitMessageHandlerMapped(
    CHAN_METHOD_T 		* pChanMethod,
    DEVICE_CHANNEL_T 	* pChannel,
    VOID				* ExternHandle
);

NTSTATUS SmInitMessageHandlerMappedMd(
    CHAN_METHOD_T 		* pChanMethod,
    DEVICE_CHANNEL_T 	* pChannel,
    VOID				* ExternHandle
);

NTSTATUS SmInitMessageHandlerExternTable(
    CHAN_METHOD_T 		* pChanMethod,
    DEVICE_CHANNEL_T 	* pChannel,
    VOID				* ExternHandle
);

NTSTATUS SmSysIfShutdown(
    I_O struct CDSL_ADAPTER_S*		pAdapter
)	;

NTSTATUS SmShutdownHandlerMapped(
    CHAN_METHOD_T 		* pChanMethod,
    DEVICE_CHANNEL_T 	* pChannel,
    VOID				* ExternHandle
);

NTSTATUS SmShutdownHandlerMappedMd(
    CHAN_METHOD_T 		* pChanMethod,
    DEVICE_CHANNEL_T 	* pChannel,
    VOID				* ExternHandle
);

NTSTATUS SmShutdownExternTable(
    CHAN_METHOD_T 		* pChanMethod,
    DEVICE_CHANNEL_T 	* pChannel,
    VOID				* ExternHandle
);

MODULAR NTSTATUS SmSysIfSetDevice(
    IN struct CDSL_ADAPTER_S*		pAdapter,
    IN	CHAN_COMMAND_T		Message,			// External Message.  This does not contain an Event
    I_O	struct CDSL_ADAPTER_S*		pUserAdapter);		// User Device Object

MODULAR NTSTATUS SmSysIfIndicateNewMessage(
    IN struct CDSL_ADAPTER_S*		pAdapter,
    IN	DWORD				ExternMessage,		// External Message.  This does not contain an Event
    IN	DWORD				SubKey,				// Optional - Embedded Message Key
    IN	VOID				* ExternMessId,		// Optional - Handle used for Asynchronous Callbacks
    I_O	CHAR				* pBuffer,			// Message Buffer
    IN	DWORD				* Length);			// Length of Message

MODULAR NTSTATUS SmSysIfAddHandlerEncoded(
    IN CHAN_METHOD_T			* pChanMethod,
    IN DEVICE_CHANNEL_T			* pChan,		// Message Channel
    IN CHAN_COMMAND_T			Message,		// Unique to each channel
    IN DWORD					SubKey,
    IN DWORD					Length,			// Bytes in Message affected
    IN FN_SYS_RECEIVE_HANDLER	ReceiveHandler,	// Handler to be called for new messages
    IN VOID						* Context,		// Context returned when Receive Handler is Called
    IN SYS_IF_MODULE_ID_T		ModuleId		// Used to identify and order channel during callback
);

MODULAR NTSTATUS SmSysIfGetHandlerList(
    IN  DEVICE_CHANNEL_T		* pChan,		// Message Channel
    IN  CHAN_COMMAND_T			Message,		// Unique to each channel
    IN	DWORD					SubKey,			// Identifies Encoded Messages
    OUT LIST_ENTRY				** ppHeadEvent	// User Pointer to Load Message List Head Pointer
);

MODULAR NTSTATUS SmSysIfGetHandlerListMd(
    IN  DEVICE_CHANNEL_T		* pChan,			// Message Channel
    IN  CHAN_COMMAND_T			Message,			// Identifies Minor Function
    IN	DWORD					SubKey,				// Identifies Major Function
    OUT LIST_ENTRY				** ppHeadEvent		// User Pointer to Load Message List Head Pointer
);

MODULAR NTSTATUS SmSysIfGetHandlerListBinary(
    IN  DEVICE_CHANNEL_T		* pChan,			// Message Channel
    IN  CHAN_COMMAND_T			Message,			// Unique to each channel
    IN	DWORD					SubKey,
    OUT LIST_ENTRY				** ppHeadEvent		// User Pointer to Load Message List Head Pointer
);

GLOBAL NTSTATUS SmSysIfSendComplete(
    IN struct CDSL_ADAPTER_S*		pAdapter,
    IN	DWORD				ExternMessage,	// External Message.  This does not contain an Event
    CHAR					* pBuffer,		// Message Buffer
    DWORD					Length,			// Bytes in Message affected
    NTSTATUS				Status);			// Final Status

//* Need to be in different file.  Temporary
GLOBAL NTSTATUS SmLnxIfInitDevice(
    IN	struct CDSL_ADAPTER_S*		pAdapter
);

GLOBAL NTSTATUS SmLnxIfRemoveDevice(
    struct CDSL_ADAPTER_S*		pAdapter
);

//============================================================================
//	These definitions are used provide a channel identifier while debugging...
//============================================================================
#ifdef DBG
	#define COMM_CHANNEL0_DESCRIPTION {"Virtual Comm Channel 0"},
	#define COMM_CHANNEL1_DESCRIPTION {"Virtual Comm Channel 1"},
	#define COMM_CHANNEL2_DESCRIPTION {"Virtual Comm Channel 2"},
	#define COMM_CHANNEL3_DESCRIPTION {"Virtual Comm Channel 3"},
	#define COMM_CHANNEL4_DESCRIPTION {"Internal Comm Channel 4"},
#else
	#define COMM_CHANNEL0_DESCRIPTION
	#define COMM_CHANNEL1_DESCRIPTION
	#define COMM_CHANNEL2_DESCRIPTION
	#define COMM_CHANNEL3_DESCRIPTION
	#define COMM_CHANNEL4_DESCRIPTION
#endif


#if  defined(NDIS_MINIPORT_DRIVER)	&& API_CHAN_INSTALLED
	#ifdef DBG
		#define OID_QUERY_CHAN_DESCRIPTION {"NDIS Query Channel"},
	#else
		#define OID_QUERY_CHAN_DESCRIPTION
	#endif

	#define OID_QUERY_CHAN \
	,																					\
	{																					\
	OID_QUERY_CHAN_DESCRIPTION			/* Description of Channel- DEBUG ONLY !!!	*/	\
	SmInitMessageHandlerExternTable,	/* Init Message Handler Lists				*/	\
	SmShutdownExternTable,			/* Release all Message Handler memory		*/	\
	NULL,								/* Get External Message Table				*/	\
	NULL,								/* External Send Handler - REQUIRED			*/	\
	SmSysIfAddHandlerEncoded,			/* Add Event Handler - REQUIRED				*/	\
	SmSysIfGetHandlerListBinary,		/* Get Event Handler List - REQUIRED		*/	\
	NULL,								/* Optional - External Interface should be informed when Message is Async */ \
	NULL,								/* Optional - Open on Demand External Channels	 */ \
	NULL								/* Optional - Auto Close Channel (inverse of OpenChan) */ \
	}

	#ifdef DBG
		#define OID_SET_CHAN_DESCRIPTION {"NDIS Set Channel"},
	#else
		#define OID_SET_CHAN_DESCRIPTION
	#endif

	#define OID_SET_CHAN \
	,																					\
	{																					\
	OID_SET_CHAN_DESCRIPTION			/* Description of Channel- DEBUG ONLY !!!	*/	\
	SmInitMessageHandlerExternTable,	/* Init Message Handler Lists				*/	\
	SmShutdownExternTable,			/* Release all Message Handler memory		*/	\
	NULL,								/* Get External Message Table				*/	\
	NULL,								/* External Send Handler - REQUIRED			*/	\
	SmSysIfAddHandlerEncoded,			/* Add Event Handler - REQUIRED				*/	\
	SmSysIfGetHandlerListBinary,		/* Get Event Handler List - REQUIRED		*/	\
	NULL,								/* Optional - External Interface should be informed when Message is Async */ \
	NULL,								/* Optional - Open on Demand External Channels	 */ \
	NULL								/* Optional - Auto Close Channel (inverse of OpenChan) */ \
	}

	#ifdef DBG
		#define OID_API_CHAN_DESCRIPTION {"User API Channel"},
	#else
		#define OID_API_CHAN_DESCRIPTION
	#endif

	#define OID_API_CHAN \
	,																					\
	{																					\
	OID_QUERY_CHAN_DESCRIPTION			/* Description of Channel- DEBUG ONLY !!!	*/	\
	SmInitMessageHandlerExternTable,	/* Init Message Handler Lists				*/	\
	SmShutdownExternTable,			/* Release all Message Handler memory		*/	\
	CmgmtGetApiList,					/* Get External Message Table				*/	\
	NULL,								/* External Send Handler - REQUIRED			*/	\
	SmSysIfAddHandlerEncoded,			/* Add Event Handler - REQUIRED				*/	\
	SmSysIfGetHandlerListBinary,		/* Get Event Handler List - REQUIRED		*/	\
	NULL,								/* Optional - External Interface should be informed when Message is Async */ \
	NULL,								/* Optional - Open on Demand External Channels	 */ \
	NULL								/* Optional - Auto Close Channel (inverse of OpenChan) */ \
	}

#else
	#define OID_QUERY_CHAN
	#define OID_API_CHAN
	#define	OID_SET_CHAN
#endif


#if PEER_CHAN_INSTALLED

	#ifdef DBG
		#define PEER_CHAN_DESCRIPTION {"Peer Driver Channel"},
	#else
		#define PEER_CHAN_DESCRIPTION
	#endif

	#define PEER_CHAN \
	,																					\
	{																					\
	PEER_CHAN_DESCRIPTION				/* Description of Channel- DEBUG ONLY !!!	*/	\
	SmInitMessageHandlerExternTable,	/* Init Message Handler Lists				*/	\
	SmShutdownExternTable,			/* Release all Message Handler memory		*/	\
	CmgmtGetPeerList,					/* Get External Message Table				*/	\
	SmLnxSendBufferPeer,				/* External Send Handler - REQUIRED			*/	\
	SmSysIfAddHandlerEncoded,			/* Add Event Handler - REQUIRED				*/	\
	SmSysIfGetHandlerListBinary,		/* Get Event Handler List - REQUIRED		*/	\
	SmLnxIfCompleteRequestPeer,		/* Optional - External Interface should be informed when Message is Async */ \
	SmLnxOpenPeerChan,				/* Optional - Open on Demand External Channels	 */ \
	SmLnxClosePeerChan				/* Optional - Auto Close Channel (inverse of OpenChan) */ \
	}

#else
	#define PEER_CHAN
#endif


#if IRP_CHAN_INSTALLED

	#ifdef DBG
		#define IRP_CHAN_DESCRIPTION {"System IRP Channel"},
	#else
		#define IRP_CHAN_DESCRIPTION
	#endif

	#define IRP_CHAN \
	,																					\
	{																					\
	IRP_CHAN_DESCRIPTION				/* Description of Channel- DEBUG ONLY !!!	*/	\
	SmInitMessageHandlerMappedMd,		/* Init Message Handler Lists				*/	\
	SmShutdownHandlerMappedMd,		/* Release all Message Handler memory		*/	\
	CmgmtGetIrpList,					/* Get External Message Table				*/	\
	NULL,								/* External Send Handler 					*/	\
	SmSysIfAddHandlerEncoded,			/* Add Event Handler - REQUIRED				*/	\
	SmSysIfGetHandlerListMd,			/* Get Event Handler List - REQUIRED		*/	\
	NULL,								/* Optional - External Interface should be informed when Message is Async */ \
	NULL,								/* Optional - Open on Demand External Channels	 */ \
	NULL								/* Optional - Auto Close Channel (inverse of OpenChan) */ \
	}

#else
	#define IRP_CHAN
#endif



#if COMMAND_CHAN_MANAGEMENT

	#ifdef DBG
		#define MGMT_CHAN_DESCRIPTION {"Management Message Channel"},
	#else
		#define MGMT_CHAN_DESCRIPTION
	#endif

	#define MGMT_CHAN \
	,																					\
	{																					\
	MGMT_CHAN_DESCRIPTION				/* Description of Channel- DEBUG ONLY !!!	*/	\
	SmInitMessageHandlerMappedMd,		/* Init Message Handler Lists				*/	\
	SmShutdownHandlerMappedMd,		/* Release all Message Handler memory		*/	\
	CmgmtGetMgmtList,					/* Get External Message Table				*/	\
	NULL,								/* External Send Handler 					*/	\
	SmSysIfAddHandlerEncoded,			/* Add Event Handler - REQUIRED				*/	\
	SmSysIfGetHandlerListMd,			/* Get Event Handler List - REQUIRED		*/	\
	NULL,								/* Optional - External Interface should be informed when Message is Async */ \
	NULL,								/* Optional - Open on Demand External Channels	 */ \
	NULL								/* Optional - Auto Close Channel (inverse of OpenChan) */ \
	}

#else
	#define MGMT_CHAN
#endif

#if CHIPAL_REMOTE_CHAN_INSTALLED

	#ifdef DBG
		#define CHIPAL_REMOTE_CHAN_DESCRIPTION {"ChipAL Remote Message Channel"},
	#else
		#define CHIPAL_REMOTE_CHAN_DESCRIPTION
	#endif

	#define CHIPAL_REMOTE_CHAN \
	, \
	{ \
	CHIPAL_REMOTE_CHAN_DESCRIPTION		/* Description of Channel- DEBUG ONLY !!!		*/		\
	SmInitMessageHandlerMapped,		/* Init Message Handler Lists 				 	*/	\
	SmShutdownHandlerMapped,			/* Release all Message Handler memory  			*/	\
	NULL,								/* Get External Message Table	 				*/	\
	SmLnxSendChipALInterfQuery,	    /* External Send Handler 						*/	\
	SmSysIfAddHandlerEncoded,			/* Add Event Handler - REQUIRED					*/	\
	SmSysIfGetHandlerList,			/* Get Event Handler List - REQUIRED			*/	\
	NULL,								/*Optional - External Interface should be informed when Message is Async */	\
	SmLnxOpenChipAlInterfQuery,   	/* NDIS part of driver gets device pointer	 	*/ \
	NULL								/* Optional - Auto Close Channel (inverse of OpenChan) */ \
	}

#else
	#define CHIPAL_REMOTE_CHAN
#endif


#ifdef _SYSIF_C_
CHAN_METHOD_T	pgChanMethods[MAX_CHANNELS] =
    {
        {
            COMM_CHANNEL0_DESCRIPTION			// Description of Channel- DEBUG ONLY !!!
            SmInitMessageHandlerMapped,		// Init Message Handler Lists
            SmShutdownHandlerMapped,			// Release all Message Handler memory
            NULL,								// Get External Message Table
            SmLnxSendBufferComm,				// External Send Handler - REQUIRED
            SmSysIfAddHandlerEncoded,			// Add Event Handler - REQUIRED
            SmSysIfGetHandlerList,			// Get Event Handler List - REQUIRED
            NULL,								// Optional - External Interface should be informed when Message is Async
            NULL,								// Optional - Open on Demand External Channels
            NULL								// Optional - Auto Close Channel (inverse of OpenChan)
        },
{
    COMM_CHANNEL1_DESCRIPTION			// Description of Channel- DEBUG ONLY !!!
    SmInitMessageHandlerMapped,		// Init Message Handler Lists
    SmShutdownHandlerMapped,			// Release all Message Handler memory
    NULL,								// Get External Message Table
    SmLnxSendBufferComm,				// External Send Handler - REQUIRED
    SmSysIfAddHandlerEncoded,			// Add Event Handler - REQUIRED
    SmSysIfGetHandlerList,			// Get Event Handler List - REQUIRED
    NULL,								// Optional - External Interface should be informed when Message is Async
    NULL,								// Optional - Open on Demand External Channels
    NULL								// Optional - Auto Close Channel (inverse of OpenChan)
},
{
    COMM_CHANNEL2_DESCRIPTION			// Description of Channel- DEBUG ONLY !!!
    SmInitMessageHandlerMapped,		// Init Message Handler Lists
    SmShutdownHandlerMapped,			// Release all Message Handler memory
    NULL,								// Get External Message Table
    SmLnxSendBufferComm,				// External Send Handler - REQUIRED
    SmSysIfAddHandlerEncoded,			// Add Event Handler - REQUIRED
    SmSysIfGetHandlerList,			// Get Event Handler List - REQUIRED
    NULL,								// Optional - External Interface should be informed when Message is Async
    NULL,								// Optional - Open on Demand External Channels
    NULL								// Optional - Auto Close Channel (inverse of OpenChan)
},
{
    COMM_CHANNEL3_DESCRIPTION			// Description of Channel- DEBUG ONLY !!!
    SmInitMessageHandlerMapped,		// Init Message Handler Lists
    SmShutdownHandlerMapped,			// Release all Message Handler memory
    NULL,								// Get External Message Table
    SmLnxSendBufferComm,				// External Send Handler - REQUIRED
    SmSysIfAddHandlerEncoded,			// Add Event Handler - REQUIRED
    SmSysIfGetHandlerList,			// Get Event Handler List - REQUIRED
    NULL,								// Optional - External Interface should be informed when Message is Async
    NULL,								// Optional - Open on Demand External Channels
    NULL								// Optional - Auto Close Channel (inverse of OpenChan)
},
//============================================================================
//	These are the functions (methods) that support the communication channel
//	used by the driver modules for internal message passing. Currenly there
//	is no external send handler defined...
//============================================================================
{
    COMM_CHANNEL4_DESCRIPTION			// Description of Channel- DEBUG ONLY !!!
    SmInitMessageHandlerMapped,		// Init Message Handler Lists
    SmShutdownHandlerMapped,			// Release all Message Handler memory
    NULL,								// Get External Message Table
    NULL,								// External Send Handler
    SmSysIfAddHandlerEncoded,			// Add Event Handler - REQUIRED
    SmSysIfGetHandlerList,			// Get Event Handler List - REQUIRED
    NULL,								// Optional - External Interface should be informed when Message is Async
    NULL,								// Optional - Open on Demand External Channels
    NULL								// Optional - Auto Close Channel (inverse of OpenChan)
}

//*
//* Begin Optional Channels
//*
OID_QUERY_CHAN
OID_API_CHAN
PEER_CHAN
OID_SET_CHAN
IRP_CHAN
MGMT_CHAN
CHIPAL_REMOTE_CHAN
    };

#endif									//_SYSIF_C_



#endif									// File Wrapper



