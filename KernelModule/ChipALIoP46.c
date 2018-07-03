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
**		ChipAL
**
**	FILE NAME:
**		ChipAlIoP46.c
**
**	ABSTRACT:
**		Public  file for  Register I/O via the P46 Device
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/KernelModule/ChipALIoP46.c $
**	$Revision: 9 $
**	$Date: 5/14/01 11:28a $
*******************************************************************************
******************************************************************************/

#define _CHIPALIOP46_C_

#include "Product.h"

#include "CardMgmt.h"
#include "ChipALCdslV.h"
#include "../CommonData.h"

/*******************************************************************************
Module Definitions
*******************************************************************************/
#ifndef MEMORY_TAG
	#define	MEMORY_TAG		'PXNC'
#endif

#ifdef DBG
	#define	VALIDATION_COMPLETE	1
#else
	#define VALIDATION_COMPLETE	0
#endif

#ifdef _CHIPALCDSLV_H_
	#define QUEUE_CSR_WRITES	1		// Writes to CSR can use FIFO Queue
#else
	#define	QUEUE_CSR_WRITES	1
#endif

#if ! defined(P46ARB_DEBUG_MESSAGE)
	#define P46ARB_DEBUG_MESSAGE( x )
#endif


#if VALIDATION_COMPLETE
	#define MAX_CSR_TRACE						32
	#define INC_CSR_TRACE						( WriteTraceQueueIndex = (WriteTraceQueueIndex + 1) % MAX_CSR_TRACE )
	#define INC_RESP_TRACE						( ReadTraceQueueIndex = (ReadTraceQueueIndex + 1) % MAX_CSR_TRACE )


static CSR_COMMAND_PARAM_T	WriteTraceQueue[MAX_CSR_TRACE];
static BYTE				    WriteTraceQueueIndex = 0;
static CSR_RESPONSE_T	    ReadTraceQueue[MAX_CSR_TRACE];
static BYTE				    ReadTraceQueueIndex = 0;

#endif

#define WRITE_LOCK                      spinlock_t
#define INIT_WRITE_LOCK( pLock )        spin_lock_init(pLock)
#define GET_WRITE_LOCK(Id,pLock,flag) 	spin_lock_irqsave( pLock, flag )
#define GET_WRITE_LOCK_AT_ISR(Id,pLock)	spin_lock( pLock )
#define PUT_WRITE_LOCK(Id,pLock,flag) 	spin_unlock_irqrestore( pLock, flag)
#define PUT_WRITE_LOCK_AT_ISR(Id,pLock)	spin_unlock( pLock )

/*******************************************************************************
Module Private Data Types
*******************************************************************************/
#if ! defined(P46ARB_ASSERT_TRACE)
	#define P46ARB_ASSERT_TRACE		CHIPAL_ASSERT_TRACE
#endif

// Arbitrary value unique to this file
#define IS_ARM_ALIVE	( 0x00000101 << NUM_ARM_RESP_BITS )

/*******************************************************************************
Module Public  Functions
*******************************************************************************/

//*
//* Define Structure and Static Definitions of ALL CSRs
//*
typedef struct CSR_ADDRESS_S
{
	DWORD	CommandBase;
	DWORD	MaxCommands;
	DWORD	ParamAddress[ARM_MAX_COMMAND][MAX_CSR_PARAMS+1];

}CSR_ADDRESS_T;

//*
//* Static Lookup table for ARM Commands
//*

//*
//* Per Ron, Don't use lookup of address.  He would rather write every parameter every time....
//* Therefore, the below Lookup Table is no longer used.  Only the first row of each function
//* is used for a default param address!
//*
#define COMMAND_DEFAULT				0x00000000		

#define DISABLE_COMMAND_CHECK		0x0FFFFFFF

CSR_ADDRESS_T	CsrLookup[PCI_FN_END] =
    {
        {									/* ARM (Mad Max) Function Table */
            0,								/* Command Base */
            DISABLE_COMMAND_CHECK,						/* Number of Commands Available */
            {
                /*	Param1				Param2				Param3				Param Terminator */
                { CSR_ARM_PARAM1,		CSR_ARM_PARAM2,		CSR_ARM_PARAM3,		SENTINAL_ADDRESS },
                { CSR_ARM_PARAM1,		CSR_ARM_PARAM2,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
                { CSR_ARM_PARAM1,		CSR_ARM_PARAM2,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
                { CSR_ARM_PARAM1,		CSR_ARM_PARAM2,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
                { CSR_ARM_PARAM1,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
                { CSR_ARM_PARAM1,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
                { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
                { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
                { CSR_ARM_PARAM1,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
                { CSR_ARM_PARAM1,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS }
            }

        },
{									/* DSL Function Table */
    0,								/* Command Base */
    DISABLE_COMMAND_CHECK,								/* Number of ARM Commands Available to this Function */
    {
        /*	Param1				Param2				Param3				Param Terminator */
        { CSR_ARM_PARAM1,		CSR_ARM_PARAM2,		CSR_ARM_PARAM3,		SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS }
    }
},
{									/* Modem Function Table */
    0,								/* Command Base			*/
    DISABLE_COMMAND_CHECK,								/* Number of ARM Commands Available to this Function */
    {
        /*	Param1				Param2				Param3				Param Terminator */
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS },
        { SENTINAL_ADDRESS,		SENTINAL_ADDRESS,	SENTINAL_ADDRESS,	SENTINAL_ADDRESS }
    }
}
    };

typedef struct COMMAND_FIFO_S
{
	LIST_ENTRY			NextEntry;
	P46_IO_COMMAND_T 	Command;
	IO_ADDRESS_T		BaseAddress;


}COMMAND_FIFO_T;

//*
//* Primary Context Structure for this IO Controller
//*
typedef struct IO_P46_S
{
	CSR_ADDRESS_T		* CsrCommand;
	DWORD				FunctionSelect;
	BOOLEAN				ArmReady;

	#if QUEUE_CSR_WRITES		//* Module Uses ChipAl
	LIST_ENTRY			WriteFifo;
	WRITE_LOCK			WriteLock;
	CHIPAL_T			* pChipAl;
	#endif

} IO_P46_T;



/*******************************************************************************
Module Private  Functions
*******************************************************************************/
LOCAL VOID calP46WriteCsr(
    IN IO_P46_T 		* pContext,
    IO_ADDRESS_T		BaseAddress,
    IN P46_IO_COMMAND_T	* pCommand
);

LOCAL BOOL calP46WriteCsrCritical(
    IN PVOID SynchronizeContext
);

GLOBAL NTSTATUS calP46StartIo(
    IO_P46_T				* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

#if QUEUE_CSR_WRITES // Module Uses ChipAl

LOCAL NTSTATUS calP46AttachInterrupt(
    IN	IO_P46_T 	* pContext,
    IN  DWORD	      FunctSel
);

MODULAR VOID calP46IsArmAlive(
    IN IO_P46_T			* pContext,
    IN DWORD			  FunctSel
);

LOCAL NTSTATUS calP46ReleaseInterrupt(
    IN	IO_P46_T 	* pContext,
    IN  DWORD         FunctSel
);

MODULAR VOID calP46DPCWriteNextCommand(
    IN	void 	* pContext,
    OUT RESPONSE_T	* Response
);

MODULAR VOID calP46ISRProcessCSRResp(
    IN	void 	* pContext,
    OUT RESPONSE_T	* Response
);

MODULAR VOID calP46ISRArmTimer(
    IN	void 	* pContext,
    OUT RESPONSE_T	* Response
);

MODULAR VOID calP46ISRGpioChangeInd(
    IN	IO_P46_T 	* pContext,
    OUT RESPONSE_T	* Response
);

MODULAR VOID calP46ISRCommandSync(
    IN	void 	* pContext,
    OUT RESPONSE_T	* Response
);


MODULAR VOID calP46DPCArmReady(
    IN	void 	* pContext,
    OUT RESPONSE_T	* Response
);

#ifdef DBG
MODULAR VOID calP46IsrErrorLog(
    IN	void 	* pContext,
    OUT RESPONSE_T	* Response
);
MODULAR VOID calP46IsrErrorInd(
    IN	void 	* pContext,
    OUT RESPONSE_T	* Response
);
#endif


#endif		// QUEUE_CSR_WRITES 	Module Uses ChipAl


/******************************************************************************
FUNCTION NAME:
	CalP46Init

ABSTRACT:
	Init Function for P46 Controller


RETURN:
	Context for Controller


DETAILS:
******************************************************************************/
MODULAR VOID *	CalP46Init(
    IN VOID	*	pUserContext,
    IN DWORD    FunctSel
)
{
	NTSTATUS			Status;
	IO_P46_T			* pContext;

	P46ARB_ASSERT_TRACE
	(
	    "CalP46Init() ",
	    (pUserContext != NULL)
	)

	//* Allocate Context
	Status = ALLOCATE_MEMORY( (PVOID)&(pContext), sizeof(IO_P46_T), MEMORY_TAG );
	if (Status != STATUS_SUCCESS)
	{
		return(NULL);
	}
	CLEAR_MEMORY(pContext, sizeof(IO_P46_T) );

	// set the function select in the context (mostly for debug)
	pContext->FunctionSelect = FunctSel;

	//* Locate Correct CSR Command Table
	pContext->CsrCommand = &CsrLookup[COMMAND_DEFAULT];

	INIT_WRITE_LOCK(&pContext->WriteLock);
	InitializeListHead(&pContext->WriteFifo);
	pContext->pChipAl = (CHIPAL_T *) pUserContext;

	calP46AttachInterrupt(pContext,FunctSel);

	//*
	//* See if the ARM is ready
	//*
	pContext->ArmReady = FALSE;
	calP46IsArmAlive(pContext,FunctSel);

	return(pContext);
}


/******************************************************************************
FUNCTION NAME:
	CalP46Shutdown

ABSTRACT:
	Release all allocated Resources


RETURN:
	void


DETAILS:
******************************************************************************/
MODULAR VOID CalP46Shutdown(
    IN PVOID		pCon,
    IN DWORD        FunctSel
)
{
	IO_P46_T			* pContext = (IO_P46_T*)pCon ;
	COMMAND_FIFO_T		* pQueuedCommand;
	DWORD                   LockFlag;

	P46ARB_ASSERT_TRACE
	(
	    "CalP46Shutdown() ",
	    (pContext != NULL)
	)
	if (pContext)
	{
		//*
		//* Dump Write Queue
		//*
		#if QUEUE_CSR_WRITES

		GET_WRITE_LOCK(1,&pContext->WriteLock, LockFlag);
		calP46ReleaseInterrupt(pContext,FunctSel);
		while(!IsListEmpty(&pContext->WriteFifo))
		{
			pQueuedCommand = (COMMAND_FIFO_T *) RemoveHeadList(&pContext->WriteFifo);
			FREE_MEMORY
			(
			    pQueuedCommand,
			    sizeof(COMMAND_FIFO_T),
			    0
			);
		}
		PUT_WRITE_LOCK(1,&pContext->WriteLock, LockFlag);
		#endif

		//*
		//* Release Primary Context
		//*
		FREE_MEMORY
		(
		    pContext,
		    sizeof(IO_P46_T),
		    0
		);
	}
}


/******************************************************************************
FUNCTION NAME:
	CalP46ArmRead

ABSTRACT:
	Read IO Space


RETURN:
	NTSTATUS:
		-EINVAL - Invalid input parameter
		-ENOTREADY	- Response is not available in Response Register
		STATUS_SUCCESS - Read was valid

DETAILS:
******************************************************************************/
MODULAR CHIPAL_FAILCODE_T CalP46ArmRead (
    IN void				* RegIoContext,
    IN PVOID			vBaseAddress,
    IN DWORD			OffSetAddress,
    IN DWORD			RegisterLength,
    OUT void 			* pReturnValue )
{
	IO_ADDRESS_T 	pSourceRegister;
	IO_ADDRESS_T	BaseAddress;
	CSR_RESPONSE_T	* pCsrResponse;
	IO_P46_T		* pContext = (IO_P46_T *)RegIoContext;
	DEV_RESPONSE_T	ResponseType;

	if(vBaseAddress == NULL || RegIoContext == NULL)
	{
		return(-EINVAL);
	}

	BaseAddress.pVoid = vBaseAddress;


	pSourceRegister.pByte = BaseAddress.pByte + OffSetAddress;

	//*
	switch (OffSetAddress)
	{
	case CSR_ARM_RESP1:
	case CSR_ARM_RESP2:

		if(RegisterLength < sizeof(CSR_RESPONSE_T))
		{
			return(-EINVAL);
		}

		pCsrResponse = (CSR_RESPONSE_T	*) pReturnValue;

		if( ! (*(DWORD *)(BaseAddress.pByte + CSR_ARM_STAT) & CSR_STAT_DATAF) )
		{
			// Command Empty Bit was not set
			return(-ENOTREADY);
		}


			#if VALIDATION_COMPLETE

		//* Read Hardware Directly to Trace Queue.  Copy from Trace Queue back to destination
		ReadTraceQueue[ReadTraceQueueIndex].Resp2 = *((DWORD *)(BaseAddress.pByte + CSR_ARM_RESP2));
		ReadTraceQueue[ReadTraceQueueIndex].Resp1 = *((DWORD *)(BaseAddress.pByte + CSR_ARM_RESP1));
		*pCsrResponse = ReadTraceQueue[ReadTraceQueueIndex];
		INC_RESP_TRACE;

			#else

		pCsrResponse->Resp2 = *((DWORD *)(BaseAddress.pByte + CSR_ARM_RESP2));
		pCsrResponse->Resp1 = *((DWORD *)(BaseAddress.pByte + CSR_ARM_RESP1));

			#endif

		ResponseType.DevResponse =  pCsrResponse->Resp1;

		if( IS_BOOT_LOADER_CSR( ResponseType ) )
		{
			(* pContext->pChipAl->BusControl->Enable)(pContext->pChipAl->BusResources, ARM_FUNCTION, INT_CMDE );
			ChipALSetEvent (pContext->pChipAl->pNdisAdapter, ARM_FUNCTION, HW_CMDE);
			{
				RESPONSE_T	Response ;
				calP46DPCWriteNextCommand ( pContext, &Response ) ;
			}
		}
		break;

	default:

		//*
		//* Unconditional Read based on Register Length
		//*
		switch (RegisterLength)
		{
		case sizeof(BYTE):
						*(BYTE *)pReturnValue = *pSourceRegister.pByte;
			break;
		case sizeof(WORD):
						*(WORD *)pReturnValue = *pSourceRegister.pWord;
			break;
		case sizeof(DWORD):
						*(DWORD *)pReturnValue = *pSourceRegister.pDword;
			break;

		default:
			return(-EINVAL);
			break;
		}
		break;
	}

	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	CalP46ArmWrite

ABSTRACT:
	Write IO Space


RETURN:
	NTSTATUS:
		-EINVAL - Invalid input parameter
		-EBUSY	- ARM Command register was not available
		STATUS_SUCCESS - Write was valid

DETAILS:
******************************************************************************/
MODULAR CHIPAL_FAILCODE_T CalP46ArmWrite (
    IN void				* RegIoContext,
    IN PVOID			vBaseAddress,
    IN DWORD			OffSetAddress,
    IN DWORD			RegisterLength,
    IN DWORD			MaskValue,
    VOID				* UserValue )
{
	DWORD				RegisterValue;
	DWORD				WriteValue = (DWORD)UserValue;

	IO_ADDRESS_T		BaseAddress;
	IO_ADDRESS_T		pRegister;
	IO_P46_T			* pContext = (IO_P46_T *)RegIoContext;
	P46_IO_COMMAND_T	* pCommand;
	COMMAND_FIFO_T		* pQueuedCommand;
	NTSTATUS			Status;
	DWORD				CommandEmpty;
	DWORD               LockFlag;
	DWORD 				IsrStat;

	if(vBaseAddress == NULL || RegIoContext == NULL)
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			ChipALDebugFlag,
			("CnxADSL no base address!"));
		return(-EINVAL);
	}

	BaseAddress.pVoid = vBaseAddress;

	BaseAddress.pVoid = vBaseAddress;
	pRegister.pByte = BaseAddress.pByte + OffSetAddress;

	GET_WRITE_LOCK(2,&pContext->WriteLock,LockFlag);

	//* Apply Rules to Addresses
	switch (OffSetAddress)
	{
	case CSR_ARM_CMD:

		pCommand = (P46_IO_COMMAND_T *) UserValue;
		TRACE_CRITICAL(0x0801EEEE);
		//*
		//* Start Atomic Operation on CSR and Param registers
		//*

		// Protect against other DPCs
//		CommandEmpty = (*(DWORD *)(BaseAddress.pByte + CSR_ARM_STAT)) & CSR_STAT_CMDE;
		IsrStat = *(DWORD *)(BaseAddress.pByte + CSR_ARM_STAT);
		CommandEmpty = IsrStat & CSR_STAT_CMDE;

		if ( !CommandEmpty ||
		     !IsListEmpty(&pContext->WriteFifo)  )
		{
//			DBG_CDSL_DISPLAY(
//				DBG_LVL_MISC_INFO,
//				ChipALDebugFlag,
//				("CnxADSL CalP46ArmWrite queue cmd on CSR queue"));
			//*
			//* We already have commands in the CSR queue, Command is in process, or ARM is not ready
			//* Add to the List and return.
			//*
			Status = ALLOCATE_MEMORY((PVOID)&(pQueuedCommand), sizeof(COMMAND_FIFO_T), MEMORY_TAG );
			if (Status != STATUS_SUCCESS)
			{
				Status = -ENOMEM;
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("CnxADSL CalP46ArmWrite failed to allocate memory for CSR queue"));
				goto UNLOCK_EXIT;
			}

			//*
			//* Copy data from user space and place at head of list.
			//*
			pQueuedCommand->Command = *pCommand;
			pQueuedCommand->BaseAddress = BaseAddress;
			
			InsertHeadList(
			    &pContext->WriteFifo,
			    &pQueuedCommand->NextEntry
			);

			TRACE_CRITICAL(0x0804EEEE);
			Status = STATUS_PENDING;
			goto UNLOCK_EXIT;
		}

		//*
		//* Write CSR Parameters
		//*
		calP46WriteCsr(pContext,BaseAddress, pCommand);

		TRACE_CRITICAL(0x0802EEEE);
		Status = STATUS_SUCCESS;
		goto UNLOCK_EXIT;

		break;

	default:
		//*
		//* Unconditional Write.  Continue Processing...
		//*
		break;
	}

	if(vBaseAddress == NULL)
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			ChipALDebugFlag,
			("CnxADSL CalP46ArmWrite vBaseAddress=NULL, return Status=busy"));
		Status = -EBUSY;
		goto UNLOCK_EXIT;
	}

	//*
	//* Determine if Write is Bitmapped or if complete value should be written.
	//* For BitMapped, we must do a Read + Modify + Write cycle.
	//*
	if (MaskValue)
	{
		//*
		//* Read IO Register
		//*
		switch (RegisterLength)
		{
			case sizeof(BYTE):
//				DBG_CDSL_DISPLAY(
//					DBG_LVL_MISC_INFO,
//					ChipALDebugFlag,
//					("CnxADSL byte cmd"));
				//(BYTE) RegisterValue = *pRegister.pByte;		// Read Register
				RegisterValue = *pRegister.pByte;		// Read Register
				break;

			case sizeof(WORD):
//				DBG_CDSL_DISPLAY(
//					DBG_LVL_MISC_INFO,
//					ChipALDebugFlag,
//					("CnxADSL word cmd"));
				//(WORD) RegisterValue = *pRegister.pWord;		// Read Register
				RegisterValue = *pRegister.pWord;		// Read Register
				break;

			case sizeof(DWORD):
//				DBG_CDSL_DISPLAY(
//					DBG_LVL_MISC_INFO,
//					ChipALDebugFlag,
//					("CnxADSL dword cmd"));
				RegisterValue = *pRegister.pDword;				// Read Register
				break;

			default:
				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					ChipALDebugFlag,
					("CnxADSL invalid MaskValue"));
				Status = -EINVAL;
				goto UNLOCK_EXIT;
				break;
		}

		RegisterValue &= ~MaskValue;			// Clear Bits indicated by Mask

		// Set Bit(s). WriteValue is expected to only affect bits indicated by the Mask
		RegisterValue |= (WriteValue & MaskValue);

		WriteValue = RegisterValue;
	}

	switch (RegisterLength)
	{
	case sizeof(BYTE):
					*pRegister.pByte= (BYTE) WriteValue;
		break;

	case sizeof(WORD):
					*pRegister.pWord = (WORD) WriteValue;
		break;

	case sizeof(DWORD):
					*pRegister.pDword = WriteValue;
		break;

	default:
		Status = -EINVAL;
		goto UNLOCK_EXIT;
		break;
	}

	NO_PCI_BURST(BaseAddress);
	Status = STATUS_SUCCESS;

UNLOCK_EXIT:

	PUT_WRITE_LOCK(2,&pContext->WriteLock,LockFlag);

	return(Status);
}


/******************************************************************************
FUNCTION NAME:
	CalP46ArmGetStatus

ABSTRACT:
	Get Device Status (Ability to process a new command)


RETURN:
	NTSTATUS:
		STATUS_SUCCESS - Always returns success

	DeviceStatus (Input Param 3):
		CHIPAL_DEVICE_READY - ARM CSR can accept a new command
		CHIPAL_DEVICE_BUSY - ARM CSR is busy (processing a command)

DETAILS:
******************************************************************************/
MODULAR CHIPAL_FAILCODE_T CalP46ArmGetStatus (
    IN void					* RegIoContext,
    IN void					* vBaseAddress,
    OUT CHIPAL_DEVICE_STATUS_T	* DeviceStatus
)
{
	IO_ADDRESS_T		BaseAddress;

	BaseAddress.pVoid = vBaseAddress;

	//* DeviceStatus STATUS_SUCCESS indicates device is ready
	*DeviceStatus = ((*(DWORD *)(BaseAddress.pByte + CSR_ARM_STAT)) & CSR_STAT_CMDE) ? CHIPAL_DEVICE_READY : CHIPAL_DEVICE_BUSY;

	return(STATUS_SUCCESS);
}



/******************************************************************************
FUNCTION NAME:
	calP46WriteCsr

ABSTRACT:
	Write of Command and All Param registers.  This routine assumes the 
	appropriate synchronization actions have already been taken.


RETURN:
	VOID


DETAILS:
	The Param Registers are written in reverse order to preven PCI burst
	mode.

******************************************************************************/
LOCAL VOID calP46WriteCsr(
    IN IO_P46_T 				* pContext,
    IO_ADDRESS_T				BaseAddress,
    IN P46_IO_COMMAND_T			* pCommand)
{
	CAL_CRITICAL_CONTEXT_S		CritContext;

	CritContext.pChipAl =	pContext->pChipAl;
	CritContext.pVoid1 = 	pContext;
	CritContext.pVoid2 = 	BaseAddress.pVoid;
	CritContext.pVoid3 = 	pCommand;

	// Protect Against ISRs.  WriteCsrCritical accesses BAR0+8.  ISRs
	// access BAR0+4.  In the P46 device, sequential
	// accesss can put the PCI bus in Burst mode.  This is
	// only a problem in MultiProcessor machines.
	// Should be inside a lock when I get here.
	calP46WriteCsrCritical(&CritContext);

	//* If user has a callback function and we are not an ISR, notify
	//* user.  We do not allow callbacks at DIRQL.  There is too much potential
	//* for error (user routines calling functions like KeSetEvent from the call
	//* back handler).
	if(pCommand->CsrParam.NotifyOnComplete)
	{
		(* pCommand->CsrParam.NotifyOnComplete)(pCommand->CsrParam.UserContext);
	}
}


/******************************************************************************
FUNCTION NAME:
	calP46WriteCsrCritical

ABSTRACT:
	Write CSR register while holding the ISR spinlock


RETURN:
	BOOL - TRUE - This function always succeeds


DETAILS:
******************************************************************************/
LOCAL BOOL calP46WriteCsrCritical(
    IN PVOID SynchronizeContext
)
{

	BYTE 					ParamIndex;
	DEV_RESPONSE_T			ResponseType;
	CAL_CRITICAL_CONTEXT_S 	* pCrit =	(CAL_CRITICAL_CONTEXT_S *) SynchronizeContext;
	IO_ADDRESS_T			BaseAddress;
	P46_IO_COMMAND_T		* pCommand =  (P46_IO_COMMAND_T* ) pCrit->pVoid3;
	IO_P46_T 				* pContext =  (IO_P46_T * ) (pCrit->pVoid1);

	BaseAddress.pVoid = pCrit->pVoid2;

	TRACE_CRITICAL(0x0C01eeee);


	#if VALIDATION_COMPLETE
	WriteTraceQueue[WriteTraceQueueIndex].Command = pCommand->CsrParam.Command;
	WriteTraceQueue[WriteTraceQueueIndex].Param.Raw[0] = pCommand->CsrParam.Param.Raw[0];
	WriteTraceQueue[WriteTraceQueueIndex].Param.Raw[1] = pCommand->CsrParam.Param.Raw[1];
	INC_CSR_TRACE;
	#endif

	ResponseType.DevResponse =  pCommand->CsrParam.Command;

	if
	(
	    IS_BOOT_LOADER_CSR( ResponseType )
	)
	{
		// The bootloader does not queue responses.  We must disable the command
		// interrupt until the response is received.
		(* pContext->pChipAl->BusControl->Disable)(pContext->pChipAl->BusResources, ARM_FUNCTION, INT_CMDE );
	}

	//* Zero Relative processing.
	for(ParamIndex = MAX_CSR_PARAMS; ParamIndex != 0; ParamIndex --)
	{
		*(DWORD *)(BaseAddress.pByte + pContext->CsrCommand->ParamAddress[COMMAND_DEFAULT][ParamIndex-1] ) =
		    pCommand->CsrParam.Param.Raw[ParamIndex-1];
	}
	*(DWORD *)(BaseAddress.pByte+CSR_ARM_CMD) = pCommand->CsrParam.Command;
	NO_PCI_BURST(BaseAddress);

	TRACE_CRITICAL(0x0C02eeee);
	return(TRUE);
}


/*******************************************************************************
Module Private  Functions using ChipAl
*******************************************************************************/

#if PROJECTS_NAME == PROJECT_NAME_TIGRIS
/******************************************************************************
FUNCTION NAME:
	calP46AttachInterrupt

ABSTRACT:
	Attach event handlers for normal processing.


RETURN:
	NTSTATUS:
		The results of all ChipALAddEventHandler calls.  This should never 
		fail so the combined results are returned.


DETAILS:
******************************************************************************/
LOCAL NTSTATUS calP46AttachInterrupt(
    IN	IO_P46_T 	* pContext,
    IN  DWORD         FunctSel
)
{
	NTSTATUS		Status;

	//*
	//* Hardware Event Handlers
	//*

	//* Handler for ARM Command Empty event
	Status = ChipALAddEventHandler(
	             pContext->pChipAl->pNdisAdapter,
	             FunctSel,
	             HW_CMDE,
	             HW_ISR_EVENT,
	             calP46DPCWriteNextCommand,
	             pContext );

	//* Handler to Decode ARM Responses
	Status |= ChipALAddEventHandler(
	              pContext->pChipAl->pNdisAdapter,
	              FunctSel,
	              HW_EDATAF,
	              HW_ISR_EVENT,
	              calP46ISRProcessCSRResp,
	              pContext );

	//* Handler to Decode Timer Responses
	Status |= ChipALAddEventHandler(
	              pContext->pChipAl->pNdisAdapter,
	              FunctSel,
	              HW_LOGICAL_TIMER_EXPIRY,
	              HW_ISR_EVENT,
	              calP46ISRArmTimer,
	              pContext );

	//*
	//* Unsolicited Response Handlers
	//*

	//* Handler for Arm Ready Indication
	Status |= ChipALAddEventHandler(
	              pContext->pChipAl->pNdisAdapter,
	              FunctSel,
	              HW_INTERFACE_READY,
                  HW_ISR_EVENT,
	              calP46DPCArmReady,
	              pContext );

	//*
	//* Handler for Sync commands.  Currently this is only used to
	//* test for ARM Ready.  This is necessary if the P46ARB driver
	//* finishes downloading and configuration before this module is
	//* initialized.
	//*
	Status |= ChipALAddEventHandler(
	              pContext->pChipAl->pNdisAdapter,
	              FunctSel,
	              HW_COMMAND_SYNC,
	              HW_ISR_EVENT,
	              calP46ISRCommandSync,
	              pContext );

	#ifdef DBG
	Status |= ChipALAddEventHandler(
	              pContext->pChipAl->pNdisAdapter,
	              FunctSel,
	              HW_ARM_COMMAND_PROCESSING_ERROR,
	              HW_ISR_EVENT,
	              calP46IsrErrorLog,
	              pContext );

	Status |= ChipALAddEventHandler(
	              pContext->pChipAl->pNdisAdapter,
	              FunctSel,
	              HW_ERROR_IND,
	              HW_ISR_EVENT,
	              calP46IsrErrorInd,
	              pContext );
 	#endif


	//* Enable Interrupts for Command Empty and Data Full
	(* pContext->pChipAl->BusControl->Enable)(pContext->pChipAl->BusResources, FunctSel, (INT_CMDE|INT_DATAF) );

	return(Status);
}

/******************************************************************************
FUNCTION NAME:
	calP46ReleaseInterrupt

ABSTRACT:
	Remove event handlers for normal processing.


RETURN:
	NTSTATUS:
		The results of all ChipALAddEventHandler calls.  This should never 
		fail so the combined results are returned.


DETAILS:
******************************************************************************/
LOCAL NTSTATUS calP46ReleaseInterrupt(
    IN	IO_P46_T 	* pContext,
    IN  DWORD         FunctSel
)
{
	NTSTATUS		Status;

	//*
	//* Hardware Event Handlers
	//*

	//* Handler for ARM Command Empty event
	Status = ChipALRemoveEventHandler(
	             pContext->pChipAl->pNdisAdapter,
	             FunctSel,
	             HW_CMDE,
	             HW_ISR_EVENT,
	             calP46DPCWriteNextCommand
	         );

	//* Handler to Decode ARM Responses
	Status |= ChipALRemoveEventHandler(
	              pContext->pChipAl->pNdisAdapter,
	              FunctSel,
	              HW_EDATAF,
	              HW_ISR_EVENT,
	              calP46ISRProcessCSRResp
	          );

	//*
	//* Unsolicited Response Handlers
	//*

	//* Handler for Arm Ready Indication
	Status |= ChipALRemoveEventHandler(
	              pContext->pChipAl->pNdisAdapter,
	              FunctSel,
	              HW_INTERFACE_READY,
                  HW_ISR_EVENT,
	              calP46DPCArmReady
	          );

	Status |= ChipALRemoveEventHandler(
	              pContext->pChipAl->pNdisAdapter,
	              FunctSel,
	              HW_COMMAND_SYNC,
	              HW_ISR_EVENT ,
	              calP46ISRCommandSync
	          );

	#ifdef DBG
	Status |= ChipALRemoveEventHandler(
	              pContext->pChipAl->pNdisAdapter,
	              FunctSel,
	              HW_ARM_COMMAND_PROCESSING_ERROR,
	              HW_ISR_EVENT ,
	              calP46IsrErrorLog
	          );
	Status |= ChipALRemoveEventHandler(
	              pContext->pChipAl->pNdisAdapter,
	              FunctSel,
	              HW_ERROR_IND,
	              HW_ISR_EVENT ,
	              calP46IsrErrorInd
	          );

	#endif

	//* Disable Interrupts for Command Empty and Data Full
	(* pContext->pChipAl->BusControl->Disable)(pContext->pChipAl->BusResources,
	        FunctSel,
	        (INT_CMDE|INT_DATAF) );

	return(Status);
}
#endif

/******************************************************************************
FUNCTION NAME:
	calP46ISRArmTimer

ABSTRACT:
	Determine which Timer Interrupted and set event accordingly.

RETURN:
	VOID


DETAILS:
******************************************************************************/
MODULAR VOID calP46ISRArmTimer(
    IN	void	 	* pCon,
    OUT RESPONSE_T	* Response
)
{
	IO_P46_T 			* pContext = (IO_P46_T*)pCon;
	CHIPAL_EVENT_T		SoftEvent;
	DWORD               FunctSel=pContext->FunctionSelect;

	// Set ChipAl Event
	SoftEvent = HW_LOGICAL_TIMER_EXPIRY + 1 + ( Response[HW_LOGICAL_TIMER_EXPIRY].Param[0] & 0x00000001 );

	if(!(SoftEvent >= HW_EVENT_BEGIN  && SoftEvent < HW_EVENT_END))
	{
		//* Error / undefined response
		P46ARB_DEBUG_MESSAGE( ("calP46ISRProcessResp: Unknown Arm Timer: %x\n   \n",Response[HW_LOGICAL_TIMER_EXPIRY].Param));
		return;
	}

	// Copy any parameters to user space
	COPY_MEMORY(&Response[SoftEvent].Param, &Response[HW_LOGICAL_TIMER_EXPIRY], sizeof(Response->Param));

	ChipALSetEvent (pContext->pChipAl->pNdisAdapter, FunctSel, SoftEvent );
}



/******************************************************************************
FUNCTION NAME:
	chipALP46IsArmAlive

ABSTRACT:
	Send Extended command to ARM.  If a response is received, the extended
	command set is Loaded.  Otherwise, the Config complete message will start
	the ARM queue processing.


RETURN:
	void


DETAILS:
******************************************************************************/
MODULAR VOID calP46IsArmAlive(
    IN IO_P46_T			* pContext,
    IN DWORD			  FunctSel
)
{
	IO_ADDRESS_T			BaseAddress;
	DWORD					CommandEmpty;
	P46_IO_COMMAND_T		Command;
	DWORD                   LockFlag;

	// for the DSL function, use an interrupt to see if the
	// ARM is ready
	if ( FunctSel == DSL_FUNCTION )
	{
		BaseAddress.pVoid = pContext->pChipAl->BusResources->VirtualAddressBAR0;

		if(BaseAddress.pVoid == NULL)
		{
			return;
		}

		GET_WRITE_LOCK(4,&pContext->WriteLock,LockFlag);
		CommandEmpty = (*(DWORD *)(BaseAddress.pByte + CSR_ARM_STAT)) & CSR_STAT_CMDE;

		if(CommandEmpty )
		{
			//* The upper Command word us used to uniquely identify this Sync command
			Command.CsrParam.Command = IS_ARM_ALIVE | DSL_COMMAND_SYNC;
			Command.CsrParam.Param.CommandSync.Response = RESPONSE_YES;
			Command.CsrParam.NotifyOnComplete = NULL;

// don't do this as it causes an interrupt on the DSL function before the ARM function
// completes all operations and the Qube doesn't like interrupts on both functions simultaneously.
// instead just mark it as ready
//			calP46WriteCsr(pContext, BaseAddress, &Command);
			pContext->ArmReady = TRUE;

		}
		else
		{
			//* Being this is an Init routine and the first time the Arm has been
			//* written, this should never happen.
			P46ARB_DEBUG_MESSAGE( ("chipALP46IsArmAlive: Command Register Unavailable\n"));
		}
		
		PUT_WRITE_LOCK(4,&pContext->WriteLock,LockFlag);
	}

	// for the ARM function there is an ARM ready bit to read
	else
	{
		BOOLEAN DeviceReady;

		// We have a hardware bit that we can examine to determine if ARM is ready.
		BaseAddress.pVoid = pContext->pChipAl->BusResources->VirtualAddressARMBAR0;

		if(BaseAddress.pVoid == NULL)
		{
			return;
		}

		// See if we are ready to access ARM
		DeviceReady = (*(DWORD *)(BaseAddress.pByte + CSR_ARM_ISR)) & (CSR_ISR_DLREADY | CSR_ISR_DEVREADY);

		if(DeviceReady)
		{
			pContext->ArmReady = TRUE;
			P46ARB_DEBUG_MESSAGE( ("calP46StartIo: Enabling ARM CSR Queue\n"));
		}
	}
}


/******************************************************************************
FUNCTION NAME:
	calP46ISRProcessCSRResp

ABSTRACT:
	ISR Level routine to process a response from the ARM.  Response Register 1
	is used to map a response code to an internal ChipAl Event.  Response
	Register 2 is read and stored in the ChipAl context structure.  The soft
	event is posted back to chipal to be further processed by either a ISR or
	DPC level routines.


RETURN:
	VOID


DETAILS:
******************************************************************************/
MODULAR VOID calP46ISRProcessCSRResp(
    IN	void 		* pCon,
    OUT RESPONSE_T	* Response
)
{
	NTSTATUS			Status;
	IO_P46_T 			* pContext = (IO_P46_T*)pCon;
	CSR_RESPONSE_T	 	CsrResponse;
	CHIPAL_EVENT_T		SoftEvent;
	DEV_RESPONSE_T		RawEvent;
	DWORD				* pPrivateResp;
	void       			* pAddressReg;
	volatile DWORD		* pRespReg;
	DWORD               FunctSel=pContext->FunctionSelect;

#ifdef PIM_DEBUG
if (irqs_disabled())
  printk(KERN_ALERT "CnxADSL: calP46ISRProcessCSRResp IRQs disabled?\n");
#endif
	// select the address based on the function
	if ( FunctSel == DSL_FUNCTION )
	{
		pAddressReg = pContext->pChipAl->BusResources->VirtualAddressBAR0;
		pRespReg = (volatile DWORD*)( (char*)pAddressReg + DSL_STAT_OFFSET );
	}
	else
	{
		pAddressReg = pContext->pChipAl->BusResources->VirtualAddressARMBAR0;
		pRespReg = (volatile DWORD*)( (char*)pAddressReg + CSR_ARM_STAT );
	}

	//* Read All Response Registers
	Status = CalP46ArmRead (
	    		         pContext,
	        		     pAddressReg,
	            		 CSR_ARM_RESP1,
		            	 sizeof(CsrResponse),
			             &CsrResponse);

	if(Status == STATUS_SUCCESS)
	{
		// Translate device code to ChipAl soft event
		RawEvent.DevResponse = CsrResponse.Resp1 & ARM_RESP_BITS;
		HASH_SOFT_HW_EVENT(SoftEvent,RawEvent);

		if(!(SoftEvent >= HW_EVENT_BEGIN  && SoftEvent < HW_EVENT_END))
		{
			//* Error / undefined response
			P46ARB_DEBUG_MESSAGE( ("calP46ISRProcessCSRResp: Unknown Arm Response:\n"
		    	                   "    Response1 (%8x) Response2 (%8x)\n",
		        	               CsrResponse.Resp1,CsrResponse.Resp2));

			return;
		}
		switch (SoftEvent)
		{
			case HW_COMMAND_SYNC:
				pPrivateResp = (DWORD *) &Response[HW_COMMAND_SYNC];
				*pPrivateResp = CsrResponse.Resp1 & ~ARM_RESP_BITS;
				break;

			default:
				// Copy any parameters to user space
				COPY_MEMORY(&Response[SoftEvent].Param, &CsrResponse.Resp2, sizeof(Response->Param));
				break;
		}
			
		// Set ChipAl Event
		ChipALSetEvent (pContext->pChipAl->pNdisAdapter,  FunctSel, SoftEvent);
	}
}

/******************************************************************************
FUNCTION NAME:
	calP46ISRCommandSync

ABSTRACT:
	DPC Level routine to indicate the ARM is now ready for operation.


RETURN:
	VOID


DETAILS:
******************************************************************************/
MODULAR VOID calP46ISRCommandSync(
    IN	void	 	* pCon,
    OUT RESPONSE_T	* Response
)
{
	DWORD SyncCode = *((DWORD *)&Response[HW_COMMAND_SYNC]);
	IO_P46_T 	* pContext = (IO_P46_T*)pCon;

	switch(SyncCode)
	{
	case IS_ARM_ALIVE:
		P46ARB_DEBUG_MESSAGE( ("calP46ISRCommandSync: Enabling ARM CSR Queue\n"));
		pContext->ArmReady = TRUE;
		ChipALSetEvent (pContext->pChipAl->pNdisAdapter, ARM_FUNCTION, HW_CMDE);
		break;

	default:
		// * Not Our Sync Command
		break;
	}
}


/******************************************************************************
FUNCTION NAME:
	calP46DPCArmReady

ABSTRACT:
	DPC Level routine to indicate the ARM is now ready for operation.


RETURN:
	VOID


DETAILS:
******************************************************************************/
MODULAR VOID calP46DPCArmReady(
    IN	void	 	* pCon,
    OUT RESPONSE_T	* Response
)
{
	IO_P46_T 	* pContext = (IO_P46_T*)pCon;
	P46ARB_DEBUG_MESSAGE( ("calP46DPCArmReady: Enabling ARM CSR Queue\n"));
	pContext->ArmReady = TRUE;

	//* Jump Start Write Queue.  After this point, it will be interrupt driven.
	calP46DPCWriteNextCommand(pContext, &pContext->pChipAl->ResponseParams[HW_CMDE]);
}


/******************************************************************************
FUNCTION NAME:
	calP46DPCWriteNextCommand

ABSTRACT:
	DPC Level routine to process queued CSR commands.  The queue will only be 
	processed if ArmReady is set to TRUE.  This is based on receiving the
	HW_INTERFACE_READY response from the ARM.


RETURN:
	VOID


DETAILS:
******************************************************************************/
MODULAR VOID calP46DPCWriteNextCommand(
    IN	void 		* pCon,
    OUT RESPONSE_T	* Response
)
{
	IO_P46_T 	* pContext = (IO_P46_T*)pCon;
	DWORD					CommandEmpty;
	COMMAND_FIFO_T			* pQueuedCommand;
	DWORD					LockFlag ;

	GET_WRITE_LOCK(6,&pContext->WriteLock, LockFlag);
	
	if(!IsListEmpty(&pContext->WriteFifo) )
	{
		//*
		//* Dequeue Command
		//*
		pQueuedCommand = (COMMAND_FIFO_T* ) RemoveTailList(&pContext->WriteFifo);
		CommandEmpty = (*(DWORD *)(pQueuedCommand->BaseAddress.pByte + CSR_ARM_STAT)) & CSR_STAT_CMDE;

		if(!CommandEmpty)
		{
			//*
			//* Arm is busy.  Requeue and Process on Next Interrupt. This should not happen.
			//*
			InsertTailList(&pContext->WriteFifo, &pQueuedCommand->NextEntry);
		}
		
		//*
		//* Write CSR Parameters
		//*
		else
		{
			calP46WriteCsr(pContext, pQueuedCommand->BaseAddress, &pQueuedCommand->Command );

			FREE_MEMORY
			(
			    pQueuedCommand,
			    sizeof(COMMAND_FIFO_T),
			    0
			);
		}
	}

	PUT_WRITE_LOCK(6,&pContext->WriteLock, LockFlag);

	//*
	//* Cleanup and Exit
	//*

	return;
}


#ifdef DBG
/******************************************************************************
FUNCTION NAME:
	calP46IsrErrorLog

ABSTRACT:
	Display Synchronous ARM Error indication to debugger. This is a "Special
	Response Code"

RETURN:
	VOID


DETAILS:
	All Arm error messages will contain the text "ARM Error".

******************************************************************************/
MODULAR VOID calP46IsrErrorLog(
    IN	void	 	* pCon,
    OUT RESPONSE_T	* Response
)
{
	P46ARB_DEBUG_MESSAGE( ("calP46IsrErrorLog: ARM Error: %8x\n",*((DWORD *)&Response[HW_ARM_COMMAND_PROCESSING_ERROR])));
}


/******************************************************************************
FUNCTION NAME:
	calP46IsrErrorInd

ABSTRACT:
	Display Asynchronous ARM Error indication to debugger.	This is a "Unsolicited
	Indication".


RETURN:
	VOID


DETAILS:
******************************************************************************/
MODULAR VOID calP46IsrErrorInd(
    IN	void	 	* pCon,
    OUT RESPONSE_T	* Response
)
{
	P46ARB_DEBUG_MESSAGE( ("calP46IsrErrorInd: ARM Error Indication: %8x\n",*((DWORD *)&Response[HW_ERROR_IND])));
}
#endif		 // DBG

