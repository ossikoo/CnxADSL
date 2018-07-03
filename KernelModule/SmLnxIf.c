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
**		SmLnxIf.c
**
**	ABSTRACT:
**		System Management WDM Interface
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/SmLnxIf.c $
**	$Revision: 1 $
**	$Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
#include "SmSysIfv.h"
#include "SmLnxIf.h"

#include "Product.h"
#include "DevIo.h"
#include "CardMgmt.h"


#define MAX_INSTANCE_COUNT	4		// Nax Instance Count + Null Term Char


#if !defined(P46ARB_DEBUG_MESSAGE)
	#define P46ARB_DEBUG_MESSAGE
#endif

/*******************************************************************************
* Custom Data Types
*******************************************************************************/


/*******************************************************************************
* PRIVATE Prototypes
*******************************************************************************/
NTSTATUS SmLnxSendSysIfComplete(
    IN PVOID 			Context
);

GLOBAL NTSTATUS SmLnxIfGetMessageHandle(
    IN	struct CDSL_ADAPTER_S	*fdo,
    OUT	MSG_HANDLING_T 			** ppDevExt
);

/*******************************************************************************
* Module Functions
*******************************************************************************/



/******************************************************************************
FUNCTION NAME:
	SmLnxIfLoadHandlers

ABSTRACT:
	Load Function handlers in  IRP response table.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS SmLnxIfLoadHandlers(
    //	PDRIVER_DISPATCH	* DispatchTable
																		void
)
{
	/*
		DWORD	Count;
		P46ARB_DEBUG_MESSAGE(("\n SmLnxIfLoadHandlers:  Loading IRP Handlers\n"));

		#if HOOK_DISPATCH_TABLE

		// * 
		// * We do not yet have context.  We must temporarily save dispatch
		// * table in global.  When AddDevice/Initialize Handler is called,
		// * we will copy the global entries to the context specific 
		// * variable. A global would probably be ok to use without the
		// * extra copy.  Driver Entry should only get called once for all 
		// * instances of the driver.
		// *
		NdisMoveMemory(gNextDispatch, DispatchTable, sizeof(gNextDispatch));
		#endif



		#if defined(NDIS_MINIPORT_DRIVER) 
		// *
		// * IRP_MJ_CREATE is called when a file object associated with the 
		// * device is created - CreateFile() in a user-mode program 
		// *
		DispatchTable[IRP_MJ_CREATE] = SmLnxIfRequestCreate;

		// *
		// * IRP_MJ_CLOSE file object opened on the driver is being removed from the 
		// * system; that is, all file object handles have been closed 
		// *
		DispatchTable[IRP_MJ_CLOSE] = SmLnxIfRequestClose;

		// *
		// * IRP_MJ_DEVICE_CONTROL internal device control requests 
		// *
		DispatchTable[IRP_MJ_DEVICE_CONTROL] = SmLnxIfRequestControl;

		// *
		// * IRP_MJ_WRITE Writes from COM port
		// *
		DispatchTable[IRP_MJ_WRITE] = SmLnxIfRequestWrite;
		#else	// NDIS_MINIPORT_DRIVER
		// *
		// * All Dispatch Entry points are processed by CardManagement to allow
		// * for IRPs that cannot be processed due to PNP state.
		// *
		for(Count = 0; Count <= IRP_MJ_MAXIMUM_FUNCTION; ++Count)
		{
			DispatchTable[Count] = CmgmtIrpDispatch; // * Main Dispatch Routine
		}
		#endif	//NDIS_MINIPORT_DRIVER
	*/ 	
	return(STATUS_SUCCESS);

}


#if HOOK_DISPATCH_TABLE
/******************************************************************************
FUNCTION NAME:
	SmLnxIfUnLoadHandlers

ABSTRACT:
	Restore Driver Dispatch Table to it's initial state.  This function is only
	used when HOOK_DISPATCH_TABLE is set.


RETURN:
	VOID


DETAILS:
******************************************************************************/

GLOBAL VOID SmLnxIfUnLoadHandlers(
    //	DEVICE_EXTENSION	* pDevExt
)
{

	if (pDevExt->pDriverObject == NULL)
	{
		return;
	}
	if (pDevExt->pDriverObject->MajorFunction == NULL)
	{
		return;
	}

	// *
	// * Restore the Driver Object to its previous state
	// *

	// * NOTE:
	// * For Win98, Driver Entry is only called once.  If the driver is unloaded, only initialize
	// * Handler is called.  Therefore, we will not restore the device object to it's original State.
	// *
	/*
	NdisMoveMemory(
		pDevExt->pDriverObject->MajorFunction, 
		pDevExt->NextDispatch, 
		sizeof(pDevExt->NextDispatch));
	*/
}
#endif



/******************************************************************************
FUNCTION NAME:
	SmLnxIfRequestControl

ABSTRACT:
	Process Control Message from Io Manager


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
MODULAR NTSTATUS SmLnxIfRequestControl(IN CDSL_ADAPTER_T* fdo,
                                       IN UINT			Command,
                                       INOUT PVOID		pArgument            )
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG InfoLength = 0;
	MSG_HANDLING_T		* pCurrentDevice;
	CHAN_COMMAND_T		Message;			// Misc Dword Value
	DEVIO_HEADER_T		* pDevIoHeader;
	CHAR				* UserBuffer;
	DWORD				InLength=1024;
	DWORD				BytesNeeded;

	// *
	// * Find Device Object
	// *
	status = SmLnxIfGetMessageHandle(fdo, &pCurrentDevice);

	if (status != STATUS_SUCCESS)
	{
		return -EINVAL;
	}

	switch (Command)
	{

	case IOCTL_DEVICE_SPECIFIC:
/*not supported for Linux	
		if (pCurrentDevice->QueryInformationHandler == NULL)
		{
			P46ARB_DEBUG_MESSAGE(("\nIoWDM RequestControl: Adapter Information Handler not defined\n"));
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}

		status = (* pCurrentDevice->QueryInformationHandler)(
			pCurrentDevice->pThisAdapter,
			OID_CUSTOM_DEVICE_IO,
			(PVOID) Irp->AssociatedIrp.SystemBuffer,
			outlength,
			&BytesWritten,
			&BytesNeeded);

		if ( status != STATUS_SUCCESS 
			|| BytesNeeded > outlength
			|| BytesWritten > outlength)
		{
			status = STATUS_INVALID_BUFFER_SIZE;
			break;
		}
		InfoLength = BytesNeeded;
*/		
		break;

	case IOCTL_GET_VERSION:
		*((DWORD *) pArgument) = VENDOR_DRIVER_VERSION;
		status = STATUS_SUCCESS;
		break;

	case IOCTL_CNX_NET_DATA:

		// * Set All bit fields for a generic COM data message
		Message.CommandU.Command = MESS_COMM0_DATA;

		// * Identify specific channel
		//Message.Element.Channel = stack->MinorFunction - pCurrentDevice->Instance;

		status = SmSysIfIndicateNewMessage(
		             fdo,
		             Message.CommandU.Command,
		             0l,
		             NULL,
		             pArgument,
		             &InLength);

		break;

	case IOCTL_CNX_NET_ECHO_PARENT:
		status = SmSysIfSendMessage(
		             pCurrentDevice->pThisAdapter,	// Ndis Adapter Context
		             MESS_COMM0_DATA,
		             (CHAR *) pArgument,		// Message Buffer
		             InLength,
		             NULL,
		             NULL);
		break;

	case IOCTL_CNX_DEV_SPECIFIC:

		UserBuffer = ((CHAR *)pArgument) +  sizeof(DEVIO_HEADER_T) ;

		pDevIoHeader = (DEVIO_HEADER_T	*) pArgument;
		BytesNeeded = 1024; //KLE temp
		status = SmSysIfIndicateNewMessage(
		             fdo,
		             MESS_DEV_SPECIFIC,
		             pDevIoHeader->ReqCode,
		             NULL,
		             UserBuffer,
		             &BytesNeeded);
		InfoLength = BytesNeeded;

		// * Complete IRP with Correct Status
		return( SmLnxIfCompleteRequestPeer(NULL, status, NULL, InfoLength));

		break;


	case IOCTL_CNX_NET_OPEN:
/* Not supported on Linux
		if(InLength < sizeof(DEVICE_INSTANCE_T))
		{
			status = !STATUS_SUCCESS;
			break;
		}
		pDevInstance = (DEVICE_INSTANCE_T *)Irp->AssociatedIrp.SystemBuffer;

		// *
		// * Set Device User Device Object
		// *
		Message.CommandU.Command = MESS_COMM0_DATA;
		
		//Message.Element.Channel = 3; //pDevInstance->Instance - pCurrentDevice->Instance; 
		status = SmSysIfSetDevice(
				fdo,
				Message,
				pDevInstance->pDeviceObj);
		if(status != STATUS_SUCCESS)
		{
			break;
		}

		// *
		// * Notify Interface we are ready to receive Data
		// *
		Message.CommandU.Command = MESS_COMM0_OPEN;
		//Message.Element.Channel = pDevInstance->Instance - pCurrentDevice->Instance; 
			 
		status = SmSysIfIndicateNewMessage(
						fdo,						// Adapter Device Object
						Message.CommandU.Command,			// Channel Commmand
						0l,							// Do not use embedded messages
						NULL,						// Async Call Back
						NULL,						// Buffer with embedded command
						&InLength);					// Length of buffer
*/
		break;

	case IOCTL_CNX_NET_CLOSE:
/* Not supported on Linux
		if(InLength < sizeof(DEVICE_INSTANCE_T))
		{
			status = !STATUS_SUCCESS;
			break;
		}
		pDevInstance = (DEVICE_INSTANCE_T *)Irp->AssociatedIrp.SystemBuffer;


		// *
		// * Notify Interface we are ready to receive Data
		// *
		Message.CommandU.Command = MESS_COMM0_CLOSE;
		Message.Element.Channel = pDevInstance->Instance - pCurrentDevice->Instance; 
	 
		status = SmSysIfIndicateNewMessage(
					fdo,						// Adapter Device Object
					Message.CommandU.Command,			// Channel Commmand
					0l,							// Do not use embedded messages
					NULL,						// Async Call Back
					NULL,						// Buffer with embedded command
					&InLength);							// Length of buffer

		if(status != STATUS_SUCCESS)
		{
			break;
		}

		// *
		// * Set Device User Device Object to NULL
		// *
		Message.CommandU.Command = MESS_COMM0_DATA;
		//Message.Element.Channel = pDevInstance->Instance - pCurrentDevice->Instance; 

		status = SmSysIfSetDevice(
			fdo,
			Message,
			NULL);
*/
		break;

	default:
		status = -EINVAL;
		break;
	}

	return SmLnxIfCompleteRequest(NULL, status, NULL, InfoLength);
}

/******************************************************************************
FUNCTION NAME:
	SmLnxIfRequestWrite

ABSTRACT:
	COM Port Write Operation


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
MODULAR NTSTATUS SmLnxIfRequestWrite(
    IN	struct CDSL_ADAPTER_S	*pAdapter
)
{
/* Not Supported on Linux
	NTSTATUS Status = -EINVAL;
	PIO_STACK_LOCATION 	Stack = IoGetCurrentIrpStackLocation(Irp);
	ULONG 				InfoLength = Stack->Parameters.Write.Length;
	CHAN_COMMAND_T		Message;
	PLIST_ENTRY 		pDeviceList;
	DEVICE_EXTENSION	* pCurrentDevice;


	Status = SmLnxIfGetMessageHandle(DeviceObject, &pCurrentDevice);

	#if HOOK_DISPATCH_TABLE
	if (Status != STATUS_SUCCESS)
	{
		if(gNextDispatch[IRP_MJ_WRITE])
		{
			// * 
			// * This IRP is not for one of our device objects.  Call next 
			// * Dispatch procedure and return.
			// *
			Status = (* gNextDispatch[IRP_MJ_WRITE])(DeviceObject,Irp);
			return(Status);
		}
		Status = STATUS_INVALID_DEVICE_REQUEST;
		SmLnxIfCompleteRequest(Irp, Status, NULL, InfoLength);
		return(Status);
	}
	#endif	//HOOK_DISPATCH_TABLE 


	// * Set All bit fields for a generic COM data message
	Message.CommandU.Command = MESS_COMM0_DATA;

	// * Identify specific channel
	Message.Element.Channel = Stack->MinorFunction - pCurrentDevice->Instance; 

	Status = SmSysIfIndicateNewMessage(
				DeviceObject,
				Message.CommandU.Command,
				0l,								// Do not use embedded messages
				NULL,						// Async Call Back
				Irp->AssociatedIrp.SystemBuffer,
				&InfoLength);

	Status = SmLnxIfCompleteRequest(Irp, Status, NULL, InfoLength);

	return(Status);
*/	
	return -EFAIL;
}

/******************************************************************************
FUNCTION NAME:
	DeSendBufferNext

ABSTRACT:
	Send buffer to next lower layer driver


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NTSTATUS SmLnxSendBufferComm
(
    struct MSG_HANDLING_S		* pMsg,
    I_O BYTE 					* Buffer,
    IN	DWORD					Length)
{
/* Not supported on Linux
	NTSTATUS			Status=STATUS_SUCCESS;
	KEVENT				event;
	PIRP				irp;
	IO_STATUS_BLOCK		ioStatus;
	PIO_STACK_LOCATION	pNextStack;
	DWORD				Version;

	KeInitializeEvent(&event, NotificationEvent, FALSE);

	irp = IoBuildDeviceIoControlRequest (
		IOCTL_CNX_NET_DATA,
		pDeviceObj,
		Buffer,
		Length,
		NULL,
		0,
		FALSE,
		&event,
		&ioStatus);

	if (!irp)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pNextStack = IoGetNextIrpStackLocation(irp);
	pNextStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
	pNextStack->DeviceObject = pDeviceObj;
	//pNextStack->FileObject = pExtension->pNextFileObject;
	//pNextStack->Parameters = CurrentStack->Parameters;

	Status = IoCallDriver(pDeviceObj, irp);

	if (!NT_SUCCESS(Status))
	{
		// * We failed. We return the reason we failed.
		P46ARB_DEBUG_MESSAGE(("SmLnxSendBufferComm: IoCallDriver Failure (%8x)\n",Status));
	}

	return(Status) ;
*/
	return -EFAIL;	
}


/******************************************************************************
FUNCTION NAME:
	SmLnxIfGetMessageHandle

ABSTRACT:
	Search Global link list for for Device Extension that contains a given
	fdo (Functional Device Object).  This is used in the case where we cannot
	use the Device Extension field to keep a pointer to the our local 
	context (NDIS).

RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS SmLnxIfGetMessageHandle(
    IN	struct CDSL_ADAPTER_S	*fdo,
    OUT	MSG_HANDLING_T 			** ppDevExt
)
{

	if ( fdo->pMsgHandleS )
	{
		*ppDevExt = fdo->pMsgHandleS;
		return STATUS_SUCCESS;
	}
	else
	{
		return -EFAIL;
	}
}

#if HOOK_DISPATCH_TABLE
/******************************************************************************
FUNCTION NAME:
	SmLnxifLoadGlobalDispatch

ABSTRACT:
	Load Dispatch Functions reported in Driver Entry into local context
	structure. This is probably not necessary, being there is only one dispatch
	table per driver, but is just good practice to Not have globals.


RETURN:
	VOID


DETAILS:
******************************************************************************/
GLOBAL VOID SmLnxifLoadGlobalDispatch(
    DEVICE_EXTENSION	* pDevExt
)
{
	COPY_MEMORY(pDevExt->NextDispatch, gNextDispatch, sizeof(gNextDispatch));
}
#endif

/******************************************************************************
FUNCTION NAME:
	SmLnxSendBufferPeer

ABSTRACT:
	Send buffer to peer driver.  This is an asynchronous Send, so a completion routine is 
	attached to the IRP.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NTSTATUS SmLnxSendBufferPeer(
    IN struct MSG_HANDLING_S	* pMsg,
    I_O BYTE 					* Buffer,
    IN	DWORD					Length
)
{
/* Not supported on Linux
	NTSTATUS			Status = STATUS_SUCCESS;
	KIRQL 					OldIrql = 0x00;

	KEVENT				event;
	PIRP				irp;
	IO_STATUS_BLOCK		ioStatus;
	PIO_STACK_LOCATION	Stack;
	DWORD				Version;
	KSPIN_LOCK	SpinLock;
	ULONG 				OutLength; 	

	OldIrql = KeGetCurrentIrql();				

	KeInitializeEvent(&event, NotificationEvent, FALSE);

	irp = IoBuildDeviceIoControlRequest (
		IOCTL_CNX_DEV_SPECIFIC,
		pDeviceObj,
		Buffer,
		Length,
		Buffer,
		Length,
		FALSE,
		&event,
		&ioStatus);

	if (!irp)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	irp->UserBuffer = Buffer; 

	IoSetNextIrpStackLocation(irp);
	Stack = IoGetCurrentIrpStackLocation(irp);
	Stack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
	IoCopyCurrentIrpStackLocationToNext(irp);
	Stack->DeviceObject = pDevExt->pDeviceObject;

	Stack = IoGetNextIrpStackLocation(irp);
	Stack->DeviceObject = pDeviceObj;
	if(KeGetCurrentIrql() != PASSIVE_LEVEL )
	{
		IoSetCompletionRoutine(
			irp,							// IRP Affected
			SmLnxSendSysIfComplete,			// CompletionRoutine,
			(VOID *)MESS_DEV_SPECIFIC,		// Context - Signals which Channel the completion routine applies to
			TRUE,							// InvokeOnSuccess,
			TRUE,							// InvokeOnError,
			TRUE							// InvokeOnCancel
			);
	}

	Status = IoCallDriver(pDeviceObj, irp);

	if (!NT_SUCCESS(Status) && Status != STATUS_PENDING)
	{
		// * We failed. We return the reason we failed.
		P46ARB_DEBUG_MESSAGE(("SmLnxSendBufferComm: IoCallDriver Failure (%8x)\n",Status));
	}
	
	return(Status) ;
*/
	return -EFAIL;	
}



/******************************************************************************
FUNCTION NAME:
	SmLnxOpenPeerChan

ABSTRACT:
	Open Peer Channel


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NTSTATUS SmLnxOpenPeerChan(
    struct MSG_HANDLING_S			* pMsg,
    PFILE_OBJECT					* ppChanFileObject,
    struct CDSL_ADAPTER_S			** ppAdapter
)
{
/* Not supported on Linux
	NTSTATUS		Status=STATUS_SUCCESS;
	UNICODE_STRING	PeerChanName;
	UNICODE_STRING	InstanceName;
	WCHAR 			CountBuff[MAX_INSTANCE_COUNT];
	WCHAR			PeerNameBuff[sizeof(DRIVER_DEVICE_PATH_WC) + sizeof(PEER_CHAN_NAME) + MAX_INSTANCE_COUNT + sizeof(WCHAR) ];
	
	// * Build Name String with enough room for Instance
	PeerChanName.Buffer = PeerNameBuff;
	PeerChanName.MaximumLength = sizeof(DRIVER_DEVICE_PATH_WC) + sizeof(PEER_CHAN_NAME) + MAX_INSTANCE_COUNT + sizeof(WCHAR);
	PeerChanName.Length = 0;

	RtlAppendUnicodeToString(
		&PeerChanName,
		DRIVER_DEVICE_PATH_WC );

	RtlAppendUnicodeToString(
		&PeerChanName,
		PEER_CHAN_NAME );

	// * Build Instance String
	InstanceName.Length = 0;
	InstanceName.MaximumLength = MAX_INSTANCE_COUNT;
	InstanceName.Buffer = CountBuff;
	RtlIntegerToUnicodeString( 
		pDevExt->Instance,	// Number to Convert
		10,					// Base used for conversion (Decimal, Binary, Octal, or Hex)
		&InstanceName ); 	// Target Unicode String

	// * Concat Name and Instance Strings
	RtlAppendUnicodeStringToString( 
		&PeerChanName,
		&InstanceName);	    

	// * Attempt Open.  Don't Care about failures (Caller must do something)
	Status = IoGetDeviceObjectPointer (
		&PeerChanName,
		FILE_READ_ATTRIBUTES,
		ppChanFileObject,
		ppChanDeviceObject);
	
	return Status;
*/
	return -EFAIL;	
}


/******************************************************************************
FUNCTION NAME:
	SmLnxClosePeerChan

ABSTRACT:
	Close Peer Channel


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
void SmLnxClosePeerChan(
    struct MSG_HANDLING_S			* pMsg,
    PFILE_OBJECT					* ppChanFileObject,
    struct CDSL_ADAPTER_S			* pAdapter
)
{
/* Not supported on Linux
	if(pChanFileObject)
	{
		ObDereferenceObject(pChanFileObject);
	}

#if 0
	if(pChanDeviceObject)
	{
		ObDereferenceObject(pChanDeviceObject);
	}
#endif
	return STATUS_SUCCESS;
*/
	return;
}


/******************************************************************************
FUNCTION NAME:
	SmLnxSendSysIfComplete

ABSTRACT:
	Asynch Io Completion Routine for IRPs to the Peer to Peer Channel


RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
NTSTATUS SmLnxSendSysIfComplete(
    IN PVOID 			Context
)
{
/* Not supported on Linux

	NTSTATUS			Status=STATUS_SUCCESS;
	PIO_STACK_LOCATION 	Stack = IoGetCurrentIrpStackLocation(Irp);
	ULONG 				OutLength = MIN_VALUE(Irp->IoStatus.Information, Stack->Parameters.DeviceIoControl.InputBufferLength);
	
	if(OutLength && Irp->UserBuffer)
	{
		//NdisMoveMemory(Irp->UserBuffer,Irp->AssociatedIrp.SystemBuffer,OutLength);
	}

	Status = SmSysIfSendComplete(
		DeviceObject,						// Device Object for This Adapter
		(DWORD) Context,					// External Message.  This does not contain an Event
		Irp->UserBuffer,					// Message Buffer
		OutLength,							// Bytes in Message affected - Unused
		Irp->IoStatus.Status				// Final Status
		);		
	
	return(Status);
*/	
	return -EFAIL;
}


/******************************************************************************
FUNCTION NAME:
	SmLnxIfCompleteRequest

ABSTRACT:
	Mark IRP as complete


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NTSTATUS SmLnxIfCompleteRequest(
    IN void* 	Irp,
    IN NTSTATUS Status,
    IN CHAR		* Buffer,
    IN ULONG 	InfoLength)
{
/* not supported by Linux
	Status = STATUS_SUCCESS;
	if (Status == STATUS_PENDING)
	{
		IoMarkIrpPending(Irp);
		Irp->IoStatus.Information = InfoLength;
	}
	else
	{
		Irp->IoStatus.Status = Status;
		Irp->IoStatus.Information = InfoLength;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
	}
	
	return(Status);
*/
	return -EFAIL;	
}							// CompleteRequest


/******************************************************************************
FUNCTION NAME:
	SmLnxIfCompleteRequestPeer

ABSTRACT:
	Mark IRP as complete in a Data dependent way. (Peer to Peer channel)


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NTSTATUS SmLnxIfCompleteRequestPeer(
    IN PVOID 	Irp,
    IN NTSTATUS Status,
    IN CHAR		* Buffer,
    IN ULONG 	InfoLength)
{
/* not supported by Linux
	Status = STATUS_SUCCESS;
	NOT_USED(Buffer);

	if (Status == STATUS_PENDING)
	{
		IoMarkIrpPending(Irp);
	}
	else
	{
		if (Status == STATUS_INVALID_BUFFER_SIZE)
		{
			Irp->IoStatus.Information = InfoLength;
		}
		else
		{
			Irp->IoStatus.Information = InfoLength + sizeof(DEVIO_HEADER_T);
		}
		Irp->IoStatus.Status = Status;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
	}
	
	return(Status);
*/	
	return -EFAIL;
}


#include "ChipALInterf.h"							// CompleteRequest
#include "ChipALRemoteI.h"


/******************************************************************************
FUNCTION NAME:
	SmLnxOpenChipAlInterfQuery

ABSTRACT:
	Open Peer Channel


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NTSTATUS SmLnxOpenChipAlInterfQuery(
    struct MSG_HANDLING_S			* pMsg,
    PFILE_OBJECT					* ppChanFileObject,
    struct CDSL_ADAPTER_S			** ppAdapter
)
{
	NTSTATUS		Status;

	*ppAdapter =  (struct CDSL_ADAPTER_S*)0x1234;
	Status = STATUS_SUCCESS;

	return Status;
}





/******************************************************************************
FUNCTION NAME:
	SmLnxSendChipALInterfQuery

ABSTRACT:


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NTSTATUS SmLnxSendChipALInterfQuery(
    IN struct MSG_HANDLING_S	* pMsg,
    I_O BYTE 					* Buffer,
    IN	DWORD					Length
)
{
/* not supported by Linux
	NTSTATUS					Status = -EINVAL;
	PIRP Irp;
    KEVENT Event;
    PIO_STACK_LOCATION IrpSp;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG ReturnLength;
	PCHIPAL_REMOTE_INF  pChipInterf = (PCHIPAL_REMOTE_INF)Buffer;

    static const GUID CHIPAL_CONFIG_INT_GUID = 	CHIPAL_CONFIG_GUID

    static const GUID CHIPAL_LINK_INT_GUID  =  CHIPAL_LINK_GUID
										  
	static const GUID CHIPAL_INT_GUID =	 CHIPAL_INTERFACE_GUID
	

    KeInitializeEvent( &Event, NotificationEvent, FALSE );
    Irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
					pChipInterf->StackDeviceObject,
					NULL,
					0,
					NULL,
					&Event,
					&IoStatusBlock );
    if (Irp == NULL) {
	return(STATUS_INSUFFICIENT_RESOURCES);
    }

	 
    IrpSp = IoGetNextIrpStackLocation( Irp );
    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
    IrpSp->MajorFunction = IRP_MJ_PNP;
    IrpSp->MinorFunction = IRP_MN_QUERY_INTERFACE;

	switch (pChipInterf->InterfaceType) 
	{

		case CHIPAL_BASIC_INTERFACE:
	
		IrpSp->Parameters.QueryInterface.InterfaceType = (LPGUID)&CHIPAL_INT_GUID;
			break;
	
		case CHIPAL_CONFIG_INTERFACE:
			IrpSp->Parameters.QueryInterface.InterfaceType = (LPGUID)&CHIPAL_CONFIG_INT_GUID;
			break;

		case CHIPAL_LINK_INTERFACE:
			IrpSp->Parameters.QueryInterface.InterfaceType = (LPGUID)&CHIPAL_LINK_INT_GUID;
			break;

		default:
			return (STATUS_UNSUCCESSFUL);
			break;
	}

    IrpSp->Parameters.QueryInterface.Size = pChipInterf->SizeofInterface;
    IrpSp->Parameters.QueryInterface.Version = 1;
    IrpSp->Parameters.QueryInterface.Interface = (INTERFACE*)&pChipInterf->CALIntf; 
    IrpSp->Parameters.QueryInterface.InterfaceSpecificData = NULL;

    Status = IoCallDriver(pChipInterf->StackDeviceObject, Irp);
    if (Status == STATUS_PENDING) {
	KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );
	Status = Irp->IoStatus.Status;
    }

 
	// get user conext 
	if (Status == STATUS_SUCCESS)
	{
		if (pChipInterf->CALIntf.InterfaceReference != NULL)
		{
			(*pChipInterf->CALIntf.InterfaceReference)(pChipInterf->CALIntf.Context);
		}
	}
	
	return Status;
*/
	return -EFAIL;	
}
