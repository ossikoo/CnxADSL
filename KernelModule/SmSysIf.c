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
**		ChipAl
**
**	FILE NAME:
**		SmSysIf.c
**
**	ABSTRACT:
**		Interface providing mapping from various System Interfaces to
**		Driver Call back routines.
**
**	DETAILS:
**		NOTE  Only Common data types defined by Common.H should be included
**		in this file.  There should be no concept of NDIS, WDM, ADSL, etc...
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/SmSysIf.c $
**	$Revision: 8 $
**	$Date: 7/23/01 8:13a $
*******************************************************************************
******************************************************************************/
#define _SYSIF_C_
#include "CardMgmt.h"
#include "SmSysIfv.h"
#include "../CommonData.h"

#ifndef MEMORY_TAG
	#define	MEMORY_TAG		'SXNC'
#endif

LOCAL MSG_HANDLING_T * SmSysIfGetMsgStruct(
    IN struct CDSL_ADAPTER_S*      pAdapter
);

/******************************************************************************
FUNCTION NAME:
	SmSysIfInit

ABSTRACT:
	Map Channels to External Handlers


RETURN:
	NTSTATUS:
		-ENOMEM - Could not allocate memory for structures
		STATUS_SUCCESS	- All Ok

DETAILS:
******************************************************************************/
NTSTATUS SmSysIfInit(
    I_O struct CDSL_ADAPTER_S*      pAdapter
)
{
	DWORD			ChanLoop;
	DWORD			MaxMessages;
	DWORD			MaxChannels;
	NTSTATUS		Status;
	MSG_HANDLING_T* pMsg;

	// * Determine Max messages for all message queues
	MaxMessages = MESS_END_M & COMMAND_MASK;
	MaxChannels = MAX_CHANNELS;

	// allocate a message structure and point the adapter to it
	Status = ALLOCATE_MEMORY( (PVOID)&pMsg,
	                          sizeof( MSG_HANDLING_T),
	                          0);

	if (Status != STATUS_SUCCESS )
	{
		ErrPrt("<1> memory allocation failure\n");
		return -ENOMEM;
	}

	CLEAR_MEMORY( pMsg, sizeof( MSG_HANDLING_T) );

	pAdapter->pMsgHandleS = pMsg;
	pMsg->pThisAdapter = pAdapter;
	pMsg->MaxChannels = MaxChannels;
	pMsg->MaxMessages = MaxMessages;
	pMsg->MaxTypes = COMMAND_TYPE_MAX;

	// *
	// * Load Channel Handlers
	// *
	COPY_MEMORY( &(pMsg->pChanMethod[0]),
	             &pgChanMethods[0],
	             sizeof(pgChanMethods));

	// *
	// * Initial Channel and Message structures
	// *
	Status = STATUS_SUCCESS;
	for(ChanLoop = 0; ChanLoop < pMsg->MaxChannels && Status == STATUS_SUCCESS; ChanLoop++)
	{
		if ( pMsg->pChanMethod[ChanLoop].InitMessageChan )
		{
			// *
			// * Init Message Queue for each channel
			// *
			Status = (* pMsg->pChanMethod[ChanLoop].InitMessageChan)(
				&pMsg->pChanMethod[ChanLoop],
				&pMsg->pChannel[ChanLoop],
				pAdapter
				);
		}

		InitializeListHead(  &pMsg->pChannel[ChanLoop].PendingIo);
		INIT_SPIN_LOCK(&pMsg->pChannel[ChanLoop].IoLock);
	}

	if(Status != STATUS_SUCCESS)
	{
		SmSysIfShutdown(pAdapter);
	}
	return(Status);
}

NTSTATUS SmInitMessageHandlerMapped(
    CHAN_METHOD_T 		* pChanMethod,
    DEVICE_CHANNEL_T 	* pChannel,
    VOID				* ExternHandle
)
{
	DWORD MessLoop;
	NOT_USED(pChanMethod);
	NOT_USED(ExternHandle);

	pChannel->MaxMessages = MESS_END_M & COMMAND_MASK;

	for(MessLoop = 0; MessLoop < pChannel->MaxMessages; MessLoop++)
	{
		InitializeListHead( &(pChannel->MsgU.pMessageMapped[MessLoop].pMessage) );
	}
	return(STATUS_SUCCESS);
}

/******************************************************************************
FUNCTION NAME:
	SmInitMessageHandlerMappedMd

ABSTRACT:
	Allocate / Init the memory required to create a multi dimensional
	mapped (zero relative indexed) message Handler Table.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NTSTATUS SmInitMessageHandlerMappedMd(
    CHAN_METHOD_T 		* pChanMethod,
    DEVICE_CHANNEL_T 	* pChannel,
    VOID				* ExternHandle
)
{
	DWORD		* pSourceTable;
	DWORD		Rows;
	DWORD		Cols;
	NTSTATUS 	Status;
	DWORD		Index;

	if(pChanMethod->GetMessageTable == NULL)
	{
		return(STATUS_SUCCESS);
	}
	Status = (*	pChanMethod->GetMessageTable)(ExternHandle, &pSourceTable, &pChannel->MaxMessages);
	if (Status != STATUS_SUCCESS)
	{
		return(Status);
	}
	// *
	// * In the two dimensional case, MaxMessages is a pack set of two words.  The
	// * Upper Word is used as the max Row value (Major Function).  The Lower
	// * word is used as a column value (Minor Function). the pSourceTable paramter
	// * is unused.
	// *
	pChannel->MaxCols = pChannel->MaxMessages >> (sizeof(WORD)*8);
	pChannel->MaxMessages &= 0x0000FFFF;
	if(  !pChannel->MaxMessages || !pChannel->MaxCols)
	{
		// * We expect at minimum 1 row and 1 column.  If either
		// * is zero, the configuration is not valid.
		return(STATUS_SUCCESS);
	}

	Status = ALLOCATE_MEMORY(
	             (PVOID)&pChannel->MsgU.pMessageMappedTable,
	             sizeof(MESSAGE_MAPPED_TABLE_T) *  pChannel->MaxMessages * pChannel->MaxCols ,
	             MEMORY_TAG
	         );

	if(Status != STATUS_SUCCESS)
	{
		return(STATUS_FAILURE);
	}
	CLEAR_MEMORY(pChannel->MsgU.pMessageMappedTable, sizeof(MESSAGE_MAPPED_TABLE_T) * pChannel->MaxCols * pChannel->MaxMessages);

	// *
	// * Initialize Head List function pointers for all rows and Cols
	// *
	for(Rows = 0; Rows < pChannel->MaxMessages; ++Rows)
	{
		for(Cols = 0; Cols < pChannel->MaxCols; ++Cols)
		{
			Index =  Rows*pChannel->MaxCols + Cols;
			InitializeListHead( &(pChannel->MsgU.pMessageMappedTable[Index].pMessage) );
		}
	}
	return(Status);
}


/******************************************************************************
FUNCTION NAME:
	SmInitMessageHandlerExternTable

ABSTRACT:
	Process a group of external handlers


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NTSTATUS SmInitMessageHandlerExternTable(
    CHAN_METHOD_T 		* pChanMethod,
    DEVICE_CHANNEL_T 	* pChannel,
    VOID				* ExternHandle
)
{
	DWORD		* pSourceTable;
	DWORD		MessLoop;
	NTSTATUS 	Status;
	DWORD		LastKey;

	if(pChanMethod->GetMessageTable == NULL)
	{
		return(STATUS_SUCCESS);
	}
	Status = (*	pChanMethod->GetMessageTable)(ExternHandle, &pSourceTable, &pChannel->MaxMessages);
	if (Status != STATUS_SUCCESS)
	{
		return(Status);
	}

	Status = ALLOCATE_MEMORY(
	             (PVOID)&pChannel->MsgU.pMessageOrdered,
	             sizeof(MESSAGE_ORDERED_TABLE_T) * pChannel->MaxMessages,
	             MEMORY_TAG
	         );

	if(Status != STATUS_SUCCESS)
	{
		return(STATUS_FAILURE);
	}
	CLEAR_MEMORY(pChannel->MsgU.pMessageOrdered, sizeof(MESSAGE_ORDERED_TABLE_T) * pChannel->MaxMessages);

	// *
	// * Copy Each entry from Source. Being this is an ordered table, each entry must be greater
	// * than the previous Entry. Also, Initialize the head list pointer.
	// *

	LastKey = 0;
	for(MessLoop = 0; MessLoop < pChannel->MaxMessages; MessLoop++)
	{
		if(pSourceTable[MessLoop] < LastKey)
		{
			Status = STATUS_FAILURE;
			break;
		}
		LastKey = pSourceTable[MessLoop];
		pChannel->MsgU.pMessageOrdered[MessLoop].Key = LastKey;
		InitializeListHead( &(pChannel->MsgU.pMessageOrdered[MessLoop].pMessage) );
	}
	return(Status);
}

/******************************************************************************
FUNCTION NAME:
	SmSysIfShutdown

ABSTRACT:
	Map Channels to External Handlers


RETURN:
	NTSTATUS:
		-ENOMEM - Could not allocate memory for structures
		STATUS_SUCCESS	- All Ok

DETAILS:
******************************************************************************/
NTSTATUS SmSysIfShutdown(
    struct CDSL_ADAPTER_S*      pAdapter
)
{
	DWORD					ChanLoop;
	IO_COMPLETE_T			* pPendedIo;
	BOOL					ChanOpened;
	MSG_HANDLING_T*			pMsg = pAdapter->pMsgHandleS;
	DWORD					LockFlag;

	for(ChanLoop = 0; ChanLoop < pMsg->MaxChannels; ChanLoop++)
	{
		// *
		// * Release any remaining queued Items
		// *
		ChanOpened = FALSE;
		ACQUIRE_LOCK(&pMsg->pChannel[ChanLoop].IoLock, LockFlag);
		while( !IsListEmpty(&pMsg->pChannel[ChanLoop].PendingIo) )
		{
			ChanOpened = TRUE;
			pPendedIo = (IO_COMPLETE_T *) RemoveHeadList(&pMsg->pChannel[ChanLoop].PendingIo);
			if(pPendedIo->SendComplete)
			{
				(* pPendedIo->SendComplete) (pPendedIo->ExternMessId, EFAIL, pPendedIo->InfoBuffer, pPendedIo->Length);
			}
			FREE_MEMORY(pPendedIo, sizeof(IO_COMPLETE_T), 0);
		}
		RELEASE_LOCK(&pMsg->pChannel[ChanLoop].IoLock,LockFlag);
		FREE_SPIN_LOCK(&pMsg->pChannel[ChanLoop].IoLock);
		if(ChanOpened && pMsg->pChanMethod[ChanLoop].CloseChan)
		{
			( * pMsg->pChanMethod[ChanLoop].CloseChan)(
			    pMsg,
			    pMsg->pChannel[ChanLoop].pChanFileObject,
			    pMsg->pThisAdapter
			);
			pMsg->pChannel[ChanLoop].pChanFileObject = NULL;
		}


		// *
		// * Release any memory allocated in Message handler lists
		// *
		if ( pMsg->pChanMethod[ChanLoop].ShutdownMessageChan )
		{
			(* pMsg->pChanMethod[ChanLoop].ShutdownMessageChan)(
				&pMsg->pChanMethod[ChanLoop],
				&pMsg->pChannel[ChanLoop],
				pMsg->pThisAdapter
			);
		}
	}									// Channel Loop

	// release the message handling struct
	FREE_MEMORY( pMsg, sizeof( MSG_HANDLING_T ), 0 );

	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	SmShutdownHandlerMapped

ABSTRACT:
	Release all function tables that use Direct (Encoded) Mapping to hold a
	list of handlers


RETURN:
	VOID


DETAILS:
******************************************************************************/
NTSTATUS SmShutdownHandlerMapped(
    CHAN_METHOD_T 		* pChanMethod,
    DEVICE_CHANNEL_T 	* pChannel,
    VOID				* ExternHandle
)
{

	DWORD					MessLoop;
	DRIVER_MESSAGE_LIST_T 	* RelasedMessageHandler;

	NOT_USED(pChanMethod);
	NOT_USED(ExternHandle);

	for(MessLoop = 0; MessLoop < pChannel->MaxMessages; MessLoop++)
	{
		while (!IsListEmpty( &(pChannel->MsgU.pMessageMapped[MessLoop].pMessage) ))
		{
			RelasedMessageHandler = (DRIVER_MESSAGE_LIST_T *) RemoveHeadList (
			                            &(pChannel->MsgU.pMessageMapped[MessLoop].pMessage) );

			FREE_MEMORY
			(
			    RelasedMessageHandler,
			    sizeof(DRIVER_MESSAGE_LIST_T),
			    CONTEXT_MEM_FLAGS
			);
		}							// List Loop

	}								// Message Loop
	return(STATUS_SUCCESS);
}

/******************************************************************************
FUNCTION NAME:
	SmShutdownHandlerMapped

ABSTRACT:
	Release all function tables that use Direct (Encoded) Mapping to hold a
	list of handlers


RETURN:
	VOID


DETAILS:
******************************************************************************/
NTSTATUS SmShutdownHandlerMappedMd(
    CHAN_METHOD_T 		* pChanMethod,
    DEVICE_CHANNEL_T 	* pChannel,
    VOID				* ExternHandle
)
{

	DWORD					Rows;
	DWORD					Cols;
	DWORD					Index;

	DRIVER_MESSAGE_LIST_T 	* RelasedMessageHandler;

	NOT_USED(pChanMethod);
	NOT_USED(ExternHandle);

	for(Rows = 0; Rows < pChannel->MaxMessages; ++Rows)
	{
		for(Cols = 0; Cols < pChannel->MaxCols; ++Cols)
		{
			Index =  Rows*pChannel->MaxCols + Cols;

			while (!IsListEmpty( &(pChannel->MsgU.pMessageMappedTable[Index].pMessage) ))
			{
				RelasedMessageHandler = (DRIVER_MESSAGE_LIST_T *) RemoveHeadList (
				                            &(pChannel->MsgU.pMessageMappedTable[Index].pMessage) );

				FREE_MEMORY
				(
				    RelasedMessageHandler,
				    sizeof(DRIVER_MESSAGE_LIST_T),
				    CONTEXT_MEM_FLAGS
				);
			}							// List Loop
		}
	}								// Message Loop

	// * Free table of head list pointers
	FREE_MEMORY
	(
	    pChannel->MsgU.pMessageMappedTable,
	    sizeof(MESSAGE_MAPPED_TABLE_T)  * pChannel->MaxMessages * pChannel->MaxCols,
	    CONTEXT_MEM_FLAGS
	);

	return(STATUS_SUCCESS);
}

/******************************************************************************
FUNCTION NAME:
	SmShutdownExternTable

ABSTRACT:
	Release channel allocation that does not use direct mapping.  Instead,
	memory is allocated that holds the channel / function handler tables.
	Therefore, a second release of memory is required.


RETURN:
	Void


DETAILS:
******************************************************************************/
NTSTATUS SmShutdownExternTable(
    CHAN_METHOD_T 		* pChanMethod,
    DEVICE_CHANNEL_T 	* pChannel,
    VOID				* ExternHandle
)
{

	DWORD					MessLoop;
	DRIVER_MESSAGE_LIST_T 	* RelasedMessageHandler;

	NOT_USED(pChanMethod);
	NOT_USED(ExternHandle);

	if ( pChannel->MsgU.pMessageOrdered == NULL )
	{
		return(STATUS_FAILURE);
	}

	for(MessLoop = 0; MessLoop < pChannel->MaxMessages; MessLoop++)
	{
		while (!IsListEmpty( &(pChannel->MsgU.pMessageOrdered[MessLoop].pMessage) ))
		{
			if(pChannel->MsgU.pMessageOrdered[MessLoop].pMessage.pNext == NULL)
			{
				continue;
			}
			RelasedMessageHandler = (DRIVER_MESSAGE_LIST_T *) RemoveHeadList (
			                            &(pChannel->MsgU.pMessageOrdered[MessLoop].pMessage) );

			FREE_MEMORY
			(
			    RelasedMessageHandler,
			    sizeof(DRIVER_MESSAGE_LIST_T) * pChannel->MaxMessages,
			    CONTEXT_MEM_FLAGS
			);
		}							// List Loop
	}								// Message Loop

	FREE_MEMORY
	(
	    pChannel->MsgU.pMessageOrdered,
	    sizeof(MESSAGE_ORDERED_TABLE_T) * pChannel->MaxMessages,
	    CONTEXT_MEM_FLAGS
	);
	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	SmSysIfGetHandle

ABSTRACT:
	Get Handle to identify current Device Extension


RETURN:
	NTSTATUS
		STATUS_SUCCESS - pThisAdapter is not valid or initialized
		STATUS_SUCCESS - Good Handle Returned

DETAILS:
******************************************************************************/
GLOBAL NTSTATUS SmSysIfGetHandle(
    IN  VOID					* pThisAdapter,	// Ndis Adapter Context
    OUT VOID					** Handle		// Handle for Interface
)
{
	NTSTATUS	ReturnStatus = STATUS_SUCCESS;
	struct CDSL_ADAPTER_S*	pAdapter=(struct CDSL_ADAPTER_S*)pThisAdapter;

	if ( pAdapter && pAdapter->pMsgHandleS )
	{
		*Handle = pAdapter->pMsgHandleS;
	}
	else
	{
		*Handle = NULL;
		ReturnStatus = -EFAIL;
	}

	return(ReturnStatus);
}


/******************************************************************************
FUNCTION NAME:
	SmSysIfAddMessageHandler

ABSTRACT:
	Add handler for external message


RETURN:
	NTSTATUS:
		-EINVAL 		- Invalid Context Handle
		STATUS_SUCCESS 	- Invalid Message or Module ID
		-ENOMEM			- Could not allocate memory for new handler
		STATUS_SUCCESS				- All Ok

DETAILS:

******************************************************************************/
GLOBAL NTSTATUS SmSysIfAddMessageHandler(
    VOID					* pContext,		// User Context
    SYS_MESS_T				UserMessage,	// Unique to each channel
    DWORD					SubKey,			// Identifies embedded messages, if used.
    DWORD					Length,			// Bytes in Message affected
    FN_SYS_RECEIVE_HANDLER	ReceiveHandler,	// Handler to be called for new messages
    VOID					* Context,		// Context returned when Receive Handler is Called
    SYS_IF_MODULE_ID_T		ModuleId)		// Used to identify and order channel during callback
{
	NTSTATUS				ReturnStatus=STATUS_SUCCESS;
	MSG_HANDLING_T		* pCurrentDevice =( MSG_HANDLING_T *)pContext;
	CHAN_COMMAND_T			Message;

	if(pCurrentDevice == NULL)
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("\nSmIfAddMessageHandler: FAILURE - User Context is NULL \n"));

		return(-EFAIL);
	}
	Message.CommandU.Command = UserMessage;

	if( Message.CommandU.Element.Channel >= pCurrentDevice->MaxChannels)
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("\nSmIfAddMessageHandler:  Invalid Message ID\n"));
		return(STATUS_SUCCESS);
	}

	if ( pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].AddNewHandler )
	{
		ReturnStatus = (* pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].AddNewHandler)(
						&pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel],
						&pCurrentDevice->pChannel[Message.CommandU.Element.Channel],
						Message,
						SubKey,
						Length,
						ReceiveHandler,
						Context,
						ModuleId);

		if(ReturnStatus != STATUS_SUCCESS)
		{
			DBG_CDSL_DISPLAY(
				DBG_LVL_GENERIC,
				DBG_FLAG_DONT_CARE,
				("\nSmIfAddMessageHandler: Adapter Not Found \n"));
		}
	}

	return(ReturnStatus);

}

/******************************************************************************
FUNCTION NAME:
	SmSysIfAddMessageTable

ABSTRACT:
	Add handler for external message


RETURN:
	NTSTATUS:
		-EINVAL 		- Invalid Context Handle
		STATUS_SUCCESS 	- Invalid Message or Module ID
		-ENOMEM			- Could not allocate memory for new handler
		STATUS_SUCCESS				- All Ok

DETAILS:

******************************************************************************/
GLOBAL NTSTATUS SmSysIfAddMessageTable(
    VOID				* pContext,		// User Context
    MESSAGE_HANDLER_T	* pTable,
    VOID				* RecContext,		// Context returned when Receive Handler is Called
    DWORD				Entries
)
{
	NTSTATUS				ReturnStatus=STATUS_SUCCESS;
	MSG_HANDLING_T		* pCurrentDevice =( MSG_HANDLING_T *)pContext;
	CHAN_COMMAND_T			Message;
	DWORD					Count;

	if(pCurrentDevice == NULL)
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("\nSmIfAddMessageHandler: FAILURE - User Context is NULL \n"));

		return(-EINVAL);
	}
	for(Count = 0; Count < Entries; Count++)
	{
		Message.CommandU.Command = pTable[Count].UserMessage;

		if ( pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].AddNewHandler )
		{
			ReturnStatus = (* pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].AddNewHandler)(
							&pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel],
							&pCurrentDevice->pChannel[Message.CommandU.Element.Channel],
							Message,
							pTable[Count].SubKey,
							pTable[Count].Length,
							pTable[Count].ReceiveHandler,
							RecContext,
							pTable[Count].ModuleId);

			if(ReturnStatus != STATUS_SUCCESS)
			{
				DBG_CDSL_DISPLAY(
					DBG_LVL_GENERIC,
					DBG_FLAG_DONT_CARE,
					("\nSmIfAddMessageHandler: Adapter Not Found \n"));
				break;
			}
		}
	}

	return(ReturnStatus);
}



/******************************************************************************
FUNCTION NAME:
	SmSysIfSendMessage

ABSTRACT:
	Sends a message from an internal module to an external source.  Currently,
	this only supports Mapped handlers.  It may work with other handlers.

RETURN:
	NTSTATUS:
		-EINVAL 		- Invalid Adapter Handle
		STATUS_SUCCESS 	- Invalid Channel, Message or Module ID
		STATUS_DEVICE_NOT_CONNECTED - Comm Channel is not open
		STATUS_SUCCESS				- All Ok

DETAILS:

******************************************************************************/
GLOBAL NTSTATUS SmSysIfSendMessage(
    VOID						* Context,	// Ndis Adapter Context
    SYS_MESS_T					UserMessage,	// Unique to each channel
    CHAR						* Buffer,		// Message Buffer
    DWORD						Length,			// Bytes in Message affected
    FN_IO_COMPLETE_HANDLER_T	SendComplete,	// Optional - Called when an Async Message is finished processing
    IN void						* pUserContext	// Optional - Callback context for SendComplete
)
{
	NTSTATUS				ReturnStatus = -EINVAL;
	MSG_HANDLING_T		* pCurrentDevice = (MSG_HANDLING_T *)Context;
	CHAN_COMMAND_T			Message;
	IO_COMPLETE_T			* pPendedIo;
	DWORD					LockFlag;

	Message.CommandU.Command = (DWORD) UserMessage;

	if(pCurrentDevice == NULL)
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_GENERIC,
		    DBG_FLAG_DONT_CARE,
		    ("\nSmSysIfSendMessage: Adapter Not Found \n"));

		return(ReturnStatus);
	}

	// *
	// * Make sure channel is open.  If not and channel supports open on demand, open
	// * the channel and continue. Open on demand channes are automatically closed when
	// * all messages are completely transmitted.
	// *
	while (pCurrentDevice->pChannel[Message.CommandU.Element.Channel].pAdapter == NULL)
	{
		if(pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].OpenChan)
		{
			(pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].OpenChan)(
			    pCurrentDevice,
			    &(pCurrentDevice->pChannel[Message.CommandU.Element.Channel].pChanFileObject),
			    &(pCurrentDevice->pChannel[Message.CommandU.Element.Channel].pAdapter));
			if (pCurrentDevice->pChannel[Message.CommandU.Element.Channel].pAdapter != NULL)
			{
				break;
			}

			DBG_CDSL_DISPLAY(
			    DBG_LVL_GENERIC,
			    DBG_FLAG_DONT_CARE,
		    	("\nSmSysIfSendMessage: Interface Channel Not Open! \n"));
		}
		return(-ENXIO);
	}

	#if EXTENDED_VALIDATION
	// *
	// * Validate Message
	// *
	if( Message.CommandU.Element.Channel >= pCurrentDevice->MaxChannels
		|| Message.CommandU.Element.Type >= pCurrentDevice->MaxTypes
		|| Message.CommandU.Element.Event >= pCurrentDevice->MaxMessages )
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_GENERIC,
		    DBG_FLAG_DONT_CARE,
		    ("\nSmSysIfSendMessage:  Invalid Message ID\n"));
		return(STATUS_SUCCESS);
	}
	#endif								// EXTENDED_VALIDATION

	if( pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].ExtSendHandler != NULL)
	{
		ReturnStatus = (* pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].ExtSendHandler)(
				   pCurrentDevice,
				   Buffer,
				   Length);
	}
	else
	{
		return(-ESVCINVAL);
	}



	if(SendComplete)
	{
		// *
		// * We may need to inform the Caller when an Async message is complete.
		// *  If it was pended by the external handler, queue message and return. Otherwise,
		// * don't care.
		// *
		if(ReturnStatus == STATUS_PENDING)
		{
			ReturnStatus = ALLOCATE_MEMORY(
					   (PVOID)&pPendedIo,
					   sizeof(IO_COMPLETE_T),
					   MEMORY_TAG
				       );
			if(ReturnStatus != STATUS_SUCCESS)
			{
				return(STATUS_FAILURE);
			}
			CLEAR_MEMORY(pPendedIo, sizeof(IO_COMPLETE_T));
			pPendedIo->SendComplete = SendComplete;
			pPendedIo->ExternMessId = pUserContext;
			pPendedIo->Length = Length;
			pPendedIo->InfoBuffer=Buffer;
			ACQUIRE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock, LockFlag);
			InsertHeadList (&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].PendingIo, &pPendedIo->NextIo);
			RELEASE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock, LockFlag);
			ReturnStatus = STATUS_PENDING;
		}
		else
		{
			(* SendComplete)(pUserContext, ReturnStatus, Buffer, Length);
		}

	}

	// *
	// * Close Channel if Auto Close Supported and we Do not have Pending Io
	// *
	ACQUIRE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock, LockFlag);
	if(IsListEmpty(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].PendingIo))
	{
		RELEASE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
		if(pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].CloseChan)
		{
			( * pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].CloseChan)(
			    pCurrentDevice,
			    pCurrentDevice->pChannel[Message.CommandU.Element.Channel].pChanFileObject,
			    pCurrentDevice->pChannel[Message.CommandU.Element.Channel].pAdapter
			);
			pCurrentDevice->pChannel[Message.CommandU.Element.Channel].pChanFileObject = NULL;
			pCurrentDevice->pChannel[Message.CommandU.Element.Channel].pAdapter = NULL;
		}
	}
	else
	{
		RELEASE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
	}

	return(ReturnStatus);
}


/******************************************************************************
FUNCTION NAME:
	SmSysIfSendComplete

ABSTRACT:
	Called by External Interface to indicate a previously pended message is
	now complete.  A message is automatically pended if the External interface
	returns STATUS_PENDING.


RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS SmSysIfSendComplete(
    IN struct CDSL_ADAPTER_S*	pAdapter,
    IN	DWORD				ExternMessage,	// External Message.  This does not contain an Event
    CHAR					* pBuffer,		// Message Buffer
    DWORD					Length,			// Bytes in Message affected
    NTSTATUS				Status)			// Final Status
{
	NTSTATUS				ReturnStatus = -EINVAL;
	PLIST_ENTRY 			pThisList;
	PLIST_ENTRY				pHeadList;
	MSG_HANDLING_T		* pCurrentDevice;
	CHAN_COMMAND_T			Message;
	BOOL					Found;
	IO_COMPLETE_T			* pIoComplete;
	DWORD					LockFlag;

	Message.CommandU.Command = ExternMessage;

	pCurrentDevice = SmSysIfGetMsgStruct(pAdapter);
	if(!pCurrentDevice)
	{
		return(-EINVAL);
	}

   	#if EXTENDED_VALIDATION

	// *
	// * Validate Message
	// *
	if( Message.CommandU.Element.Channel >= pCurrentDevice->MaxChannels)
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_GENERIC,
		    DBG_FLAG_DONT_CARE,
		    ("\nSmSysIfBroadcastMessage:  Invalid Message ID\n"));
		return(STATUS_SUCCESS);
	}
	#endif	// EXTENDED_VALIDATION

	// *
	// * See if an Internal handler requested a completion event
	// *
	ACQUIRE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
	pHeadList = &pCurrentDevice->pChannel[Message.CommandU.Element.Channel].PendingIo;
	pThisList= pHeadList;
	Found = FALSE;
	while(pThisList->pNext != pHeadList)
	{
		if( ((IO_COMPLETE_T *)(pThisList->pNext))->InfoBuffer == pBuffer)
		{
			Found = TRUE;
			break;
		}
		pThisList = pThisList->pNext;
	}

	// *
	// * Make sure buffer was pended
	// *
	if(!Found)
	{
		RELEASE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
		return(STATUS_SUCCESS);
	}

	// *
	// * Call Handler and free Memory
	// *
	pIoComplete = (IO_COMPLETE_T *) RemoveHeadList(pThisList);
	if(pIoComplete->SendComplete)
	{
		ReturnStatus = (* pIoComplete->SendComplete) (pIoComplete->ExternMessId, Status, pBuffer, Length);
	}
	FREE_MEMORY(pIoComplete, sizeof(IO_COMPLETE_T), 0);

	if(IsListEmpty(pHeadList))
	{
		if(pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].CloseChan)
		{
			( * pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].CloseChan)(
			    pCurrentDevice,
			    pCurrentDevice->pChannel[Message.CommandU.Element.Channel].pChanFileObject,
			    pCurrentDevice->pChannel[Message.CommandU.Element.Channel].pAdapter
			);
			pCurrentDevice->pChannel[Message.CommandU.Element.Channel].pChanFileObject = NULL;
			pCurrentDevice->pChannel[Message.CommandU.Element.Channel].pAdapter = NULL;
		}
	}
	RELEASE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);

	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	SmSysIfIndicateNewMessage

ABSTRACT:
	Indicate a new message from an external source.  If the called Handler
	returns a status of STATUS_PENDING, the message will automatically
	queued for further processing.  The pended message is released by
	a later call to SmSysIfIndicateComplete.  The address of the
	message buffer is used as a key for storing the message.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
MODULAR NTSTATUS SmSysIfIndicateNewMessage(
    IN struct CDSL_ADAPTER_S*		pAdapter,
    IN	DWORD				ExternMessage,	// External Message.  This does not contain an Event
    IN	DWORD				SubKey,			// Optional - Embedded Message Key
    IN	VOID				* ExternMessId,	// Optional - Handle used for Asynchronous Callbacks
    I_O	CHAR				* pBuffer,		// Message Buffer
    IN	DWORD				* Length)		// Length of Message
{
	MSG_HANDLING_T		* pCurrentDevice;
	NTSTATUS				ReturnStatus = -EINVAL;
	CHAN_COMMAND_T			Message;
	PLIST_ENTRY				pHeadList;
	DRIVER_MESSAGE_LIST_T	* pHandler;
	IO_COMPLETE_T			* pPendedIo;
	DWORD					LockFlag;

	Message.CommandU.Command = ExternMessage;
	pCurrentDevice = SmSysIfGetMsgStruct(pAdapter);

	if(!pCurrentDevice)
	{
		return(-EINVAL);
	}


	#if EXTENDED_VALIDATION
	// *
	// * Validate Message
	// *
	if( Message.CommandU.Element.Channel >= pCurrentDevice->MaxChannels
	        || Message.CommandU.Element.Type >= pCurrentDevice->MaxTypes)
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_GENERIC,
		    DBG_FLAG_DONT_CARE,
		    ("\nSmIfAddMessageHandler:  Invalid Message ID\n"));
		return(STATUS_SUCCESS);
	}
	#endif								// EXTENDED_VALIDATION

	// Set Head Pointer
	ACQUIRE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock, LockFlag);
	if ( pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].GetMessageList )
	{
		ReturnStatus = (* pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].GetMessageList)(
						&pCurrentDevice->pChannel[Message.CommandU.Element.Channel],	// Message Channel Context
						Message,		//Message
						SubKey,
						&pHeadList);	//Returned List
	}

	if(ReturnStatus != STATUS_SUCCESS)
	{
		RELEASE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
		return(STATUS_SUCCESS);
	}

	if(IsListEmpty(pHeadList))
	{
		RELEASE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
		return(STATUS_SUCCESS);
	}

	if( *Length  < ((DRIVER_MESSAGE_LIST_T *)pHeadList->pNext)->TotalLength)
	{
		*Length = ((DRIVER_MESSAGE_LIST_T *)pHeadList->pNext)->TotalLength;
		RELEASE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
		return(-EINVAL);
	}

	pHandler = (DRIVER_MESSAGE_LIST_T *) pHeadList;
	ReturnStatus = !STATUS_SUCCESS;

	while(pHeadList != ( (PLIST_ENTRY)pHandler)->pNext)
	{
		pHandler = (DRIVER_MESSAGE_LIST_T *) pHandler->NextHandler.pNext;
		if( pHandler->RxHandler != NULL)
		{
			RELEASE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
			ReturnStatus = (* pHandler->RxHandler)(pHandler->UserContext, pBuffer, *Length);

			if(ReturnStatus == STATUS_SUCCESS)
			{
				// * Abort Processing without touching any data structures.
				return(STATUS_SUCCESS);
			}

			ACQUIRE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
			if(ReturnStatus != STATUS_SUCCESS)
			{
				break;
			}
		}
	}

	// * Update Length Field if anything was Written.  Otherwise, use the default
	// * input value.
	if(((DRIVER_MESSAGE_LIST_T *)pHeadList->pNext)->TotalLength > *Length)
	{
		*Length = ((DRIVER_MESSAGE_LIST_T *)pHeadList->pNext)->TotalLength;
	}

	if(ReturnStatus == STATUS_PENDING)
	{
		// *
		// * For pended messages, we may need to inform the external handler
		// * when the message is complete. If so, queue message and return. Otherwise,
		// * don't care.
		// *
		if(pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].IndicateComplete)
		{
			ReturnStatus = ALLOCATE_MEMORY(
			                   (PVOID)&pPendedIo,
			                   sizeof(IO_COMPLETE_T),
			                   MEMORY_TAG
			               );
			if(ReturnStatus != STATUS_SUCCESS)
			{
				RELEASE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
				return(STATUS_FAILURE);
			}
			CLEAR_MEMORY(pPendedIo, sizeof(IO_COMPLETE_T));
			pPendedIo->ExternMessId = ExternMessId;
			pPendedIo->Length = *Length;
			pPendedIo->InfoBuffer=pBuffer;
			InsertHeadList (&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].PendingIo, &pPendedIo->NextIo);
			ReturnStatus = STATUS_PENDING;
		}
	}

	RELEASE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
	return(ReturnStatus);
}


/******************************************************************************
FUNCTION NAME:
	SmSysIfGetDeviceExtension

ABSTRACT:
	Locate Device Extension based on Device Object.  In NDIS miniports, the
	DeviceExtension element of the DeviceObject is private to NDIS.  Therefore,
	we keep a global linked list of all possible DeviceExtensions.  We match
	these based on the DeviceObject as the key element.


RETURN:
	Pointer to Device Extension, or null if failure


DETAILS:
******************************************************************************/
LOCAL MSG_HANDLING_T * SmSysIfGetMsgStruct(
    IN struct CDSL_ADAPTER_S*      pAdapter
)
{
	return pAdapter->pMsgHandleS;
}

/******************************************************************************
FUNCTION NAME:
	SmSysIfIndicateComplete

ABSTRACT:
	Called by internal Modules to indicate a previously pended message is
	now complete.  A message is automatically pended if the Module Callback
	module returns a status of STATUS_PENDING.


RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS SmSysIfIndicateComplete(
    VOID					* Context,		// NDIS (CDSL Adapter) Context
    DWORD					UserChan,		// Unique to each channel
    CHAR					* pBuffer,		// Message Buffer
    DWORD					Length,			// Bytes in Message affected
    NTSTATUS				Status)			// Final Status
{
	PLIST_ENTRY 			pThisList;
	PLIST_ENTRY				pHeadList;
	MSG_HANDLING_T		* pCurrentDevice = (MSG_HANDLING_T *)Context;
	NTSTATUS				ReturnStatus = -EINVAL;
	CHAN_COMMAND_T			Message;
	//	DRIVER_MESSAGE_LIST_T	* pHandler;
	//	DWORD					DataOffset;
	BOOL					Found;
	IO_COMPLETE_T			* pIoComplete;
	//	KIRQL					OldIrql;
	DWORD					LockFlag;

	Message.CommandU.Command = UserChan;

	if(pCurrentDevice == NULL)
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_GENERIC,
		    DBG_FLAG_DONT_CARE,
		    ("\nSmSysIfBroadcastMessage: Adapter Not Found \n"));
		return(ReturnStatus);
	}

   	#if EXTENDED_VALIDATION
	// *
	// * Validate Message
	// *
	if( Message.CommandU.Element.Channel >= pCurrentDevice->MaxChannels)
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_GENERIC,
		    DBG_FLAG_DONT_CARE,
		    ("\nSmSysIfBroadcastMessage:  Invalid Message ID\n"));
		return(STATUS_SUCCESS);
	}
	#endif								// EXTENDED_VALIDATION

	if(pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].IndicateComplete == NULL)
	{
		// * External channel does not care about completion messages. These messages
		// * were never queued.
		return(STATUS_SUCCESS);
	}
	ACQUIRE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
	pHeadList = &pCurrentDevice->pChannel[Message.CommandU.Element.Channel].PendingIo;
	pThisList= pHeadList;
	Found = FALSE;
	while(pThisList->pNext != pHeadList)
	{
		if( ((IO_COMPLETE_T *)(pThisList->pNext))->InfoBuffer == pBuffer)
		{
			Found = TRUE;
			break;
		}
		pThisList = pThisList->pNext;
	}

	// *
	// * Make sure buffer was pended
	// *
	if(!Found)
	{
		RELEASE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
		return(STATUS_SUCCESS);
	}

	// *
	// * Call Handler and free Memory
	// *
	pIoComplete = (IO_COMPLETE_T *) RemoveHeadList(pThisList);
	RELEASE_LOCK(&pCurrentDevice->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
		(* pCurrentDevice->pChanMethod[Message.CommandU.Element.Channel].IndicateComplete)(
		    pIoComplete->ExternMessId,
		    Status,
		    pBuffer,
		    Length);
	FREE_MEMORY(pIoComplete, sizeof(IO_COMPLETE_T), 0);

	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	SmSysIfBroadcastMessage

ABSTRACT:
	This function will broadcast a message to all driver modules who have
	registered to receive a message of type 'UserMessage'. To register
	for a specific message a module must use SmSysIfAddMessageHandler()
	or SmSysIfAddMessageTable() defined in this file.

RETURN:
	NTSTATUS:
		-EINVAL 		- Invalid Adapter Handle
		STATUS_SUCCESS 	- Invalid Channel, Message or Module ID
		STATUS_DEVICE_NOT_CONNECTED - Comm Channel is not open
		STATUS_SUCCESS				- All Ok

DETAILS:

******************************************************************************/
GLOBAL NTSTATUS SmSysIfBroadcastMessage(
    VOID				* Context,		// NDIS (CDSL Adapter) Context
    SYS_MESS_T				UserMessage,	// Unique to each channel
    DWORD				SubKey,			// Embedded Message key
    CHAR				* pBuffer,		// Message Buffer
    DWORD				* Length)			// Bytes in Message affected
{
	PLIST_ENTRY		pHeadList;
	CDSL_ADAPTER_T*		pAdapter = (CDSL_ADAPTER_T *)Context;
	NTSTATUS		ReturnStatus = -EINVAL;
	CHAN_COMMAND_T		Message;
	DRIVER_MESSAGE_LIST_T	* pHandler;
	DWORD			LockFlag;
	//	DWORD					DataOffset;

	Message.CommandU.Command = (DWORD) UserMessage;

	if(pAdapter == NULL)
	{
		DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("\nSmSysIfBroadcastMessage: Adapter Not Found \n"));

		return(ReturnStatus);
	}

#if EXTENDED_VALIDATION
	// *
	// * Validate Message
	// *
	if( Message.CommandU.Element.Channel >= pAdapter->pMsgHandleS->MaxChannels
		|| Message.CommandU.Element.Type >= pAdapter->pMsgHandleS->MaxTypes
	)
	{
		DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("\nSmSysIfBroadcastMessage:  Invalid Message ID\n"));
		return(STATUS_SUCCESS);
	}
#endif								// EXTENDED_VALIDATION
	// --PIM-- Check if the adapter has a message handler
	if (pAdapter->pMsgHandleS==NULL)
	 {
         printk(KERN_ALERT "CnxADSL: SmSysIfBroadcastMessage No message handler?\n");
	 return -EINVAL;
         }
	// --/PIM--

	// * Set Head Pointer

	ACQUIRE_LOCK(&pAdapter->pMsgHandleS->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
	if ( pAdapter->pMsgHandleS->pChanMethod[Message.CommandU.Element.Channel].GetMessageList )
	{
		ReturnStatus = (* pAdapter->pMsgHandleS->pChanMethod[Message.CommandU.Element.Channel].GetMessageList)(
				&pAdapter->pMsgHandleS->pChannel[Message.CommandU.Element.Channel],	//Message Channel Context
				Message,      //Message
				SubKey,
				&pHeadList);	// Returned List

		if(ReturnStatus != STATUS_SUCCESS)
		{
			RELEASE_LOCK(&pAdapter->pMsgHandleS->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
			return(STATUS_SUCCESS);
		}
	}
	else
	{
		RELEASE_LOCK(&pAdapter->pMsgHandleS->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
		return(STATUS_SUCCESS);
	}

	if(IsListEmpty(pHeadList))
	{
		RELEASE_LOCK(&pAdapter->pMsgHandleS->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
		return(STATUS_SUCCESS);
	}


	if( *Length  < ((MESSAGE_ORDERED_TABLE_T *)pHeadList)->Length )
	{
		RELEASE_LOCK(&pAdapter->pMsgHandleS->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
		*Length = ((MESSAGE_ORDERED_TABLE_T *)pHeadList)->Length;
		return(-EINVAL);
	}

	pHandler = (DRIVER_MESSAGE_LIST_T *) pHeadList;
	ReturnStatus = STATUS_SUCCESS;

	while(pHeadList != ( (PLIST_ENTRY)pHandler)->pNext)
	{
		pHandler = (DRIVER_MESSAGE_LIST_T *) pHandler->NextHandler.pNext;
		if( pHandler->RxHandler != NULL)
		{
			RELEASE_LOCK(&pAdapter->pMsgHandleS->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
			ReturnStatus = (* pHandler->RxHandler)(pHandler->UserContext, pBuffer, *Length);
			ACQUIRE_LOCK(&pAdapter->pMsgHandleS->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);

			if(ReturnStatus != STATUS_SUCCESS)
			{
				break;
			}
		}
	}

	// * Update Length Field if anything was Written.  Otherwise, use the default
	// * input value.
	if(((DRIVER_MESSAGE_LIST_T *)pHeadList->pNext)->TotalLength)
	{
		*Length = ((DRIVER_MESSAGE_LIST_T *)pHeadList->pNext)->TotalLength;
	}

	RELEASE_LOCK(&pAdapter->pMsgHandleS->pChannel[Message.CommandU.Element.Channel].IoLock,LockFlag);
	return(ReturnStatus);
}




/******************************************************************************
FUNCTION NAME:
	SmSysIfSetDevice

ABSTRACT:
	Sets an external (API) handle to be associated with a
	particular DeviceExtension.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
MODULAR NTSTATUS SmSysIfSetDevice(
    IN struct CDSL_ADAPTER_S*		pAdapter,
    IN	CHAN_COMMAND_T				Message,		// External Message.  This does not contain an Event
    I_O	struct CDSL_ADAPTER_S*		pUserAdapter)	// User Device Object
{
/* not implemented for Linux
	PLIST_ENTRY 		pDeviceList;
	MSG_HANDLING_T		* pCurrentDevice=pAdapter->pMsgHandleS;
	NTSTATUS			ReturnStatus = -EINVAL;
	DWORD				LockFlag;

	ACQUIRE_LOCK(&gIoLock,LockFlag);
	pDeviceList = gHeadIfList.pNext;

	while (pDeviceList != &gHeadIfList)
	{
		pCurrentDevice = (MSG_HANDLING_T *) pDeviceList;

		if (pCurrentDevice->pThisAdapter == pAdapter)
		{
			ReturnStatus = STATUS_SUCCESS;
			break;
		}
		pDeviceList = pDeviceList->pNext;
	}
	RELEASE_LOCK(&gIoLock,LockFlag);

	if(ReturnStatus != STATUS_SUCCESS)
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("\nSmSysIfSetDevice: Incorrect Device Object \n"));

		return(ReturnStatus);
	}
	#if EXTENDED_VALIDATION

	// *
	// * Validate Message
	// *
	if( Message.CommandU.Element.Channel >= pCurrentDevice->MaxChannels
			|| Message.CommandU.Element.Type >= pCurrentDevice->MaxTypes)
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("\nSmSysIfSetDevice:  Invalid Message ID\n"));
		return(STATUS_SUCCESS);
	}
	#endif								// EXTENDED_VALIDATION

	pCurrentDevice->pMsgHandleS->pChannel[Message.CommandU.Element.Channel].pAdapter = pUserAdapter;
*/
	return(STATUS_SUCCESS);
}

/******************************************************************************
FUNCTION NAME:
	SmSysIfAddHandlerEncoded

ABSTRACT:
	Add a new handler to the given Message channel. The Handler List is a
	0 relative linear list of message ID's.  The is can be used as a default
	handler for any messaging module.


RETURN:
	NTSTATUS
		STATUS_SUCCESS 	- Invalid Message or Module ID
		-ENOMEM			- Could not allocate memory for new handler
		STATUS_SUCCESS				- All Ok

DETAILS:
******************************************************************************/
MODULAR NTSTATUS SmSysIfAddHandlerEncoded(
    IN CHAN_METHOD_T			* pChanMethod,
    IN DEVICE_CHANNEL_T			* pChan,		// Message Channel
    IN CHAN_COMMAND_T			Message,		// Unique to each channel
    IN DWORD					SubKey,
    IN DWORD					Length,			// Bytes in Message affected
    IN FN_SYS_RECEIVE_HANDLER	ReceiveHandler,	// Handler to be called for new messages
    IN VOID						* Context,		// Context returned when Receive Handler is Called
    IN SYS_IF_MODULE_ID_T		ModuleId		// Used to identify and order channel during callback
)
{
	NTSTATUS				ReturnStatus;
	DRIVER_MESSAGE_LIST_T	* pNewHandler;
	PLIST_ENTRY				pHeadEvent;
	PLIST_ENTRY				pFirstEvent;
	DWORD					LockFlag;

	#if EXTENDED_VALIDATION
	// *
	// * Validate Message
	// *
	if( Message.CommandU.Element.Event >= pChan->MaxMessages )
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("\nSmIfAddMessageHandler:  Invalid Message Number\n"));
		return(STATUS_SUCCESS);
	}

	if( ReceiveHandler == NULL )
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("\nSmIfAddMessageHandler:  Invalid Parameter \n"));
		return(STATUS_SUCCESS);
	}
	#endif								// EXTENDED_VALIDATION

	ACQUIRE_LOCK(&pChan->IoLock,LockFlag);
	ReturnStatus = (* pChanMethod->GetMessageList)(
					   pChan,							// Message Channel
					   Message,						// Unique to each channel
					   SubKey,							// Identifies Encoded Messages.  Not used for Mapped.
					   &pHeadEvent						// User Pointer to Load Message List Head Pointer
				   );

	if (ReturnStatus != STATUS_SUCCESS)
	{
		RELEASE_LOCK(&pChan->IoLock,LockFlag);
		return(STATUS_SUCCESS);
	}

	// *
	// * We have a valid Message.  Allocate memory for this handler.
	// *

	ReturnStatus = 	ALLOCATE_MEMORY
					(
						(PVOID)&pNewHandler,
						sizeof(DRIVER_MESSAGE_LIST_T),
						MEMORY_TAG
					);

	if (ReturnStatus != STATUS_SUCCESS)
	{
		RELEASE_LOCK(&pChan->IoLock,LockFlag);
		return(-ENOMEM);
	}
	CLEAR_MEMORY(pNewHandler, sizeof(DRIVER_MESSAGE_LIST_T));

	// *
	// * Copy Data from User Space
	// *
	pNewHandler->ModuleId = ModuleId;
	pNewHandler->RxHandler = ReceiveHandler;
	pNewHandler->UserContext = Context;
	pNewHandler->MessageType = Message.CommandU.Element.Type;
	pNewHandler->TotalLength = Length;

	pFirstEvent	= pHeadEvent;
	// *
	// * Insert in Ordered List
	// *
	while(pHeadEvent->pNext != pFirstEvent)
	{
		if( ((DRIVER_MESSAGE_LIST_T *)pHeadEvent->pNext)->ModuleId >= pNewHandler->ModuleId)
		{
			// * Found Insert Location
			break;
		}
		else
		{
			pHeadEvent = pHeadEvent->pNext;
		}
	}

	InsertHeadList (
		pHeadEvent,
		&pNewHandler->NextHandler);

	RELEASE_LOCK(&pChan->IoLock,LockFlag);

	// * Sum all lengths.  Store in first handler.
	((MESSAGE_ORDERED_TABLE_T *)pHeadEvent)->Length += Length;

	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	SmSysIfGetHandlerList

ABSTRACT:
	Return head pointer to list of message handlers


RETURN:
	NTSTATUS


DETAILS: 
******************************************************************************/
MODULAR NTSTATUS SmSysIfGetHandlerList(
    IN  DEVICE_CHANNEL_T		* pChan,			// Message Channel
    IN  CHAN_COMMAND_T			Message,			// Unique to each channel
    IN	DWORD					SubKey,				// Identifies Encoded messages
    OUT LIST_ENTRY				** ppHeadEvent		// User Pointer to Load Message List Head Pointer
)
{
	NOT_USED(SubKey);

	#if EXTENDED_VALIDATION
	// *
	// * Validate Message
	// *
	if( Message.CommandU.Element.Event >= pChan->MaxMessages )
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("\nSmSysIfGetHandlerList:  Invalid Message ID\n"));
		return(STATUS_SUCCESS);
	}
	#endif

	*ppHeadEvent = &pChan->MsgU.pMessageMapped[Message.CommandU.Element.Event].pMessage;

	return(STATUS_SUCCESS);
}

/******************************************************************************
FUNCTION NAME:
	SmSysIfGetHandlerListMd

ABSTRACT:
	Return head pointer to list of message handlers


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
MODULAR NTSTATUS SmSysIfGetHandlerListMd(
    IN  DEVICE_CHANNEL_T		* pChan,			// Message Channel
    IN  CHAN_COMMAND_T			Message,			// Identifies Minor Function
    IN	DWORD					SubKey,				// Identifies Major Function
    OUT LIST_ENTRY				** ppHeadEvent		// User Pointer to Load Message List Head Pointer
)
{
	#if EXTENDED_VALIDATION
	// *
	// * Validate Message
	// *
	if(  Message.CommandU.Element.Event >= pChan->MaxMessages || SubKey >= pChan->MaxCols )
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_GENERIC,
		    DBG_FLAG_DONT_CARE,
		    ("\nSmSysIfGetHandlerList:  Invalid Message ID %lx\n", Message.CommandU.Command));
		return(STATUS_SUCCESS);
	}
	#endif
	*ppHeadEvent = &(pChan->MsgU.pMessageMappedTable[Message.CommandU.Element.Event*pChan->MaxCols + SubKey].pMessage);

	return(STATUS_SUCCESS);
}

/******************************************************************************
FUNCTION NAME:
	SmSysIfGetHandlerListBinary

ABSTRACT:
	Return head pointer to list of message handlers.  In this case, the
	message table is a orderd list based on keys.  The table is
	searched using a standard Binary Search algorithm.  For best
	efficiency, this method should only be used on tables with more than 5 or
	10 entries.  Otherwise, a direct Mapped table or a
	Case statement would be more efficient.

RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
MODULAR NTSTATUS SmSysIfGetHandlerListBinary(
    IN  DEVICE_CHANNEL_T		* pChan,			// Message Channel
    IN  CHAN_COMMAND_T			Message,			// Unique to each channel
    IN	DWORD					SubKey,
    OUT LIST_ENTRY				** ppHeadEvent		// User Pointer to Load Message List Head Pointer
)
{

	DWORD	First;
	DWORD	Mid=0;
	DWORD	Last;
	BOOL	Found;

	NOT_USED(Message);					// This is only used for Mapped / Encoded messages

	#if EXTENDED_VALIDATION
	// *
	// * Validate Message
	// *
	if( SubKey > pChan->MsgU.pMessageOrdered[ pChan->MaxMessages - 1].Key )
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_GENERIC,
		    DBG_FLAG_DONT_CARE,
		    ("\nSmSysIfGetHandlerList:  Invalid Message ID\n"));
		return(STATUS_SUCCESS);
	}
	#endif
	First = 0L;
	Last = 	pChan->MaxMessages - 1;

	Found = FALSE;
	while(!Found && First <= Last && Last < pChan->MaxMessages)
	{
		Mid = (First + Last) / 2;
		if( SubKey  < pChan->MsgU.pMessageOrdered[Mid].Key )
		{
			Last = --Mid;
		}
		else if ( SubKey > pChan->MsgU.pMessageOrdered[Mid].Key  )
		{
			First = ++Mid;
		}
		else
		{
			Found = TRUE;
		}
	}

	if(Found)
	{
		*ppHeadEvent = &pChan->MsgU.pMessageOrdered[Mid].pMessage;
		return(STATUS_SUCCESS);
	}
	else
	{
		return(STATUS_UNSUCCESSFUL);
	}

}

/******************************************************************************
FUNCTION NAME:
	SmSysIfTrace

ABSTRACT:
	Debug Routine for verifying a new System Interface Channel


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS SmSysIfTrace(
    VOID		* UserContext,
    CHAR		* Buffer,
    DWORD		Length
)
{
	if(Length >= sizeof(CHAR))
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_GENERIC,
		    DBG_FLAG_DONT_CARE,
		    ("\nSmSysIfTrace:  Received Message Length (%4ld) Value (%2x) Ascii(%c) \n",Length, *((BYTE *)Buffer), (*(CHAR *)Buffer)));
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_GENERIC,
		    DBG_FLAG_DONT_CARE,
		    ("\nSmSysIfTrace:  Received Message Length (%4ld) Value (\?\?\?\?) \n",Length));

	}

	return(STATUS_SUCCESS);

}
