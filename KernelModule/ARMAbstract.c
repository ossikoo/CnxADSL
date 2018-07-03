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
**		ARMAbstract.c
**
**	ABSTRACT:
**		This file implements the ARM abstraction layer which is responsible
**		for the interface to the ARM processor.
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/KernelModule/ARMAbstract.c $
**	$Revision: 17 $
**	$Date: 7/03/01 4:39p $
*******************************************************************************
******************************************************************************/

#include "LnxTools.h"
#include "../CommonData.h"

#ifndef _DATAPUMP_H_
	#define _DATAPUMP_H_
	typedef struct DATAPUMP * PDATAPUMP_CONTEXT;
#endif

#include "CnxADSL.h"
#include "CardMgmt.h"
#include "ChipALCdsl.h"
#include "ChipALCdslV.h"
#include "ChipALInterf.h"
#include "ARMAbstract.h"

////////////////////////////////////
// local defines
////////////////////////////////////
#define 		JUMP_ADDRESS		0x00010480
#define			EXT_COMMAND_ADDRESS 0x00010504
// one second
#define			DOWNLOAD_TIMEOUT	1000

#define IS_CMD_LOADED	( 0x00000102 << NUM_ARM_RESP_BITS )

#define SSCAN			8	// GPIO8

/*******************************************************************************
Private Function Prototypes
*******************************************************************************/

typedef struct
{
	CSR_COMMAND_PARAM_T	Csr_Command_Param ;
	WORD				Size ;
} GPIO_INIT_T ;

typedef struct
{
	CDSL_ADAPTER_T			*pAdapter;
	DWORD					Resp2;
	EVENT_HNDL*		pEvent;
} ARM_ASYC_IO_T;


//////////////////////////////////////////////////////////////////////
// Local functions
//////////////////////////////////////////////////////////////////////
VOID ARMSyncRoutine(
    IN	void		    * pSync,
    IN	RESPONSE_T		* Response
);

DWORD atoh( IN CHAR* cInStr,
            IN DWORD uNumChars );

NTSTATUS
processDataRec(
    IN	CDSL_ADAPTER_T	*pThisAdapter,
    IN  char*			cInRec,
    DWORD				uExtAddr );

VOID
ARMALP46CompletionRoutine(
    IN PVOID Context
);

DWORD
parseData(	IN char* cInStr );

NTSTATUS
ARMALStartArm(
    IN	CDSL_ADAPTER_T	*pThisAdapter
);

NTSTATUS ARMALInitIo
(
    IN	CDSL_ADAPTER_T	*pThisAdapter
);


VOID
ARMALNotifyArmReady(
    IN CDSL_ADAPTER_T	*pThisAdapter
);


///////////////////////////////////////////////////////////////////////


/*++
    ARMALCfgInit

Routine Description:

    The card management module calls this function is called to 
    initialize any user configurable settings.  These settings 
    are passed into the driver via IOCTL commands and will be 
    accessible from the adapter structure. 

Arguments:

    None

Return Value:

    The status of the operation.

--*/
NDIS_STATUS
ARMALCfgInit (
    IN CDSL_ADAPTER_T	*pThisAdapter,
    IN PTIG_USER_PARAMS  pUserParams
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	//	ARM_ABSTRACT_T* pARMCtxt;

	DBG_CDSL_DISPLAY(
		DBG_LVL_MISC_INFO,
		ChipALDebugFlag,
		("CnxADSL=>ARMAbstract:ARMALCfgInit  %lx", (ULONG)pThisAdapter));

	pThisAdapter->ArmALMod = NULL;

	// Allocate memory for the buffer management adapter context structure.
	/* Not Used
		Status = ALLOCATE_MEMORY ((PVOID)&pARMCtxt,
		                          sizeof(CDSL_ADAPTER_T),
		                          'ArmL');

		if (Status == STATUS_SUCCESS )
		{
			memset( pARMCtxt, 0, sizeof( ARM_ABSTRACT_T ));
			pThisAdapter->ArmALMod = pARMCtxt;
		}
		else
			Status = -ENOMEM;
	*/

	return Status;
}


/*++
    ARMALInit

Routine Description:

    The card management module calls this function to initialize 
    the ARM abstraction module.  It will initialize any data required 
    by the module and prepare the module for operations. 

Arguments:

    None

Return Value:

    The status of the operation.

--*/
NDIS_STATUS
ARMALInit(
    IN CDSL_ADAPTER_T	*pThisAdapter
)
{
	NTSTATUS Status = STATUS_SUCCESS;

	DBG_CDSL_DISPLAY(
		DBG_LVL_MISC_INFO,
		ChipALDebugFlag,
		("CnxADSL=>ARMAbstract:ARMALInit  %lx", (ULONG)pThisAdapter));

	return Status;
}


/*++
    ARMALHalt

Routine Description:

    The card management module calls this function to terminate the 
    ARM abstraction module.  It will stop any active operations and 
    free any data allocated by the module.

Arguments:

    None

Return Value:

    The status of the operation.

--*/
NDIS_STATUS
ARMALHalt(
    IN CDSL_ADAPTER_T	*pThisAdapter
)
{
	NTSTATUS Status = STATUS_SUCCESS;

	DBG_CDSL_DISPLAY(
		DBG_LVL_MISC_INFO,
		ChipALDebugFlag,
		("CnxADSL=>ARMAbstract:ARMALHalt  %lx", (ULONG)pThisAdapter));

	// free the buffer management adapter context structure.
	// not used	FREE_MEMORY( pThisAdapter->ArmALMod, , 0);

	return Status;
}


LOG_ENTRY_T Log_Buffer [NUM_LOG_ENTRIES] ;
DWORD Log_Put_Index ;
BOOLEAN TraceAll = FALSE ;

void printl ( const char *pFormat, ... )
{
	va_list va_ptr;

	// init va
	va_start ( va_ptr, pFormat ) ;

	if ( ! TraceAll )
	{
		return ;
	}

	if ( Log_Put_Index < NUM_LOG_ENTRIES )
	{
		int Index ;

		Log_Buffer[Log_Put_Index].pFormat = (char *)pFormat ;

		for ( Index = 0  ; Index < NUM_LOG_PARMS ; Index++ )
		{
			Log_Buffer[Log_Put_Index].Data[Index] = va_arg( va_ptr, DWORD) ;
		}

		Log_Put_Index++ ;
	}

	// terminate va
	va_end ( va_ptr ) ;
}	 

#define NUM_PRINTL_STRINGS 100
typedef struct 
{
	const char *pSrc ;
	char *pDest ;
} STRING_TABLE_T ;
STRING_TABLE_T		String_Table [NUM_PRINTL_STRINGS] ;


/******************************************************************************
FUNCTION NAME:
	Log_Copy_String

ABSTRACT:
	Copy the string if it is uniquely new.

RETURN:
	Returns a ptr to the destination buffer
	In case of overflow, it returns a ptr to first string (presumebly an error string)

DETAILS:
	Keep a table of ptrs to strings already copied and where the were copied to.
	When a new copy is requested, this table is searched and if the string
	has already been copied, a ptr to it is returned.

	If not, the string is copied to the string buffer and this is entered into
	the table if there is room. If there is no room, a ptr to the first entry
	in the table is returned.

	You should initially call this routine with an error string as this
	will be the string returned in case of buffer overflow.
******************************************************************************/
static	DWORD	String_Offset ;
static	int		String_Cnt ;
char * Log_Copy_String ( char *pString_Buffer, DWORD String_Buffer_Size, const char *pFormat )
{
	int		String_Index ;


	// search all existing copied string for a match
	for ( String_Index = 0 ; String_Index < String_Cnt ; String_Index ++ )
	{
		// is this a match
		if ( pFormat == String_Table [String_Index].pSrc )
		{
			// found
			break ;
		}
	}
	
	// did we NOT find the string?
	if ( String_Index == String_Cnt )
	{
		// is there room to add another string?
		if ( String_Cnt <= NUM_PRINTL_STRINGS )
		{
			// Add a new lookup table entry for this unique string
			String_Table [String_Cnt].pSrc =  pFormat ;
			String_Table [String_Cnt].pDest = &pString_Buffer[String_Offset] ;
			String_Cnt ++ ;

			// copy string without exceeding size of dest buffer
			strncat ( &pString_Buffer[String_Offset], pFormat, String_Buffer_Size - String_Offset ) ;

			// advance to next string in dest buffer
			String_Offset += strlen ( &pString_Buffer[String_Offset] ) + 1 ; // +1 for \0 terminator
		}
		else
		{
			printk ( "Log_Put String Table overflow\n" ) ;
			String_Index = 0 ;
		}
	}

	// return a ptr to the copied string
	return String_Table [String_Index].pDest ;
}

/******************************************************************************
FUNCTION NAME:
	Log_Scan_For_String

ABSTRACT:
	scan format for %[n]s and return which arg it is

RETURN:
	Arg index of match. -1 if string exhausted

DETAILS:
******************************************************************************/

int Log_Scan_For_String ( int Last_Arg_Found_Index, char *pFormat )
{
	int Char_Index ;
	int Next_Arg_Found_Index = -1 ;
	int Found_Match = 0 ; 

	typedef enum
	{
		LOOK_FOR_PERCENT,
		LOOK_FOR_CONV
	} STATE_T ;

	STATE_T State = LOOK_FOR_PERCENT ;

	for
	(
		Char_Index = 0 ;
		
		( ! Found_Match )
		&& ( pFormat [Char_Index] != 0 ) ;
		
		Char_Index ++
	)
	{
		// depending on state
		switch ( State )
		{
		case LOOK_FOR_PERCENT :
			if ( pFormat[Char_Index] == '%'  )
			{
				State = LOOK_FOR_CONV ;
			}
			break ;

		case LOOK_FOR_CONV :
			if ( pFormat[Char_Index] == '%'  )
			{
				// ignore the %% pair
				// return to looking for percent
				State = LOOK_FOR_PERCENT ;
			}



			else if ( ( pFormat[Char_Index] >= '0' )
			&&  ( pFormat[Char_Index] <= '9'  ) )
			{
				// this is a length modifier - ignore
			}
			
			
			
			else if ( pFormat[Char_Index] == 's'  )
			{
				// found a conversion specifier - count it
				Next_Arg_Found_Index ++ ;

				if ( Next_Arg_Found_Index > Last_Arg_Found_Index )
				{
					// Found next string conversion character
					// Done - exit loop
					Found_Match = 1 ;
				}
				else
				{
					// return to looking for percent
					State = LOOK_FOR_PERCENT ;
				}
			}


			// a specifier that wasn't %s 
			else
			{
				// found a conversion specifier - count it
				Next_Arg_Found_Index ++ ;

				// return to looking for percent
				State = LOOK_FOR_PERCENT ;
			}

			break ;
		}
	}

	// return the parm index of the match if one was found
	if ( ! Found_Match )
	{
		Next_Arg_Found_Index = -1 ;
	}
	return Next_Arg_Found_Index ;
}	



/******************************************************************************
FUNCTION NAME:
	Log_Copy

ABSTRACT:
	Copy the formats and data to the destination buffer

RETURN:
	NA

DETAILS:
	Copy a buffer of LOG_ENTRY types (ptr to format and N parms for printing)
	the the requested destination buffer. It also copies a buffer of format
	strings that the LOG_ENTRY will refer to.
	To save space, the buffer of format strings will re-use format strings
	rather than copy each reference to the same string.
******************************************************************************/
void Log_Copy ( TIG_PRINT_DESC *pDesc )
{
	static 	DWORD Cumulative_Start_Index = 0 ;
			DWORD	Src_Index ;
			DWORD	Dest_Index ;
			DWORD	Start ;

	if ( pDesc->Cumulative )
	{
		Start = 0 ;
	}
	else
	{
		Start = Cumulative_Start_Index ;
	}
	pDesc->pString_Buffer[0] = 0 ;
	String_Offset = 0 ;
	String_Cnt = 0 ;

	// add an error message string for printl to use when the table overflows
	Log_Copy_String ( pDesc->pString_Buffer, pDesc->String_Buffer_Size, "Log_Put Format Table Overflow\n" ) ;

	// do for each log entry - limited to size of smaller of source or dest table
	for ( Src_Index = Start, Dest_Index = 0 ; Src_Index < NUM_LOG_ENTRIES ; Src_Index ++, Dest_Index++ )
	{
		if ( Dest_Index >= pDesc->Buff_Size )
		{
			printk ( "Log_Copy Buffer Overflow\n" ) ;
			break ;
		}

		// copy entry (includes terminating null entry)
		pDesc->pBuff[Dest_Index] = Log_Buffer[Src_Index] ;

		// if terminating null entry
		if ( Log_Buffer[Src_Index].pFormat == NULL )
		{
			// done
			break ;
		}

		// add this format to string table and adjust ptr to string in table
		pDesc->pBuff[Dest_Index].pFormat = Log_Copy_String
		(
			pDesc->pString_Buffer,
			pDesc->String_Buffer_Size,
			Log_Buffer[Src_Index].pFormat
		) ;

		// scan	format string for %[n]s that indicates ptrs to strings that must be copied
		{
			int Last_Arg_Found_Index = -1 ;
			do
			{
				Last_Arg_Found_Index = Log_Scan_For_String ( Last_Arg_Found_Index, Log_Buffer[Src_Index].pFormat ) ;
				if ( Last_Arg_Found_Index >= 0 )
				{
					// add this string to string table and adjust ptr to string in table
					pDesc->pBuff[Dest_Index].Data[Last_Arg_Found_Index] = (DWORD) Log_Copy_String
					(
						pDesc->pString_Buffer,
						pDesc->String_Buffer_Size,
						(char *)Log_Buffer[Src_Index].Data[Last_Arg_Found_Index]
					) ;
				}
			}
			while ( Last_Arg_Found_Index >= 0 ) ;
		}
	}

	// remember how far we got so we can start here next time if desired
	Cumulative_Start_Index = Src_Index ;
}	

/*++
    ARMALIoctl

Routine Description:

    The card management module calls this function to process commands received 
    from the application.  Supported commands will include:

    · TIG_LOAD_START -    called to prepare the device for download.  No arguments.
    · TIG_LOAD_RECORD -   called to issue a firmware record to the device.  Argument 
                         will be a pointer to a firmware record.
    · TIG_LOAD_DONE -     called after the final record has been sent. No arguments.

Arguments:

    None

Return Value:

    The status of the operation.

--*/
NDIS_STATUS ARMALIoctl(IN CDSL_ADAPTER_T *pThisAdapter, IN unsigned int Command, IN PVOID CmdArg)
 {
 NTSTATUS Status = STATUS_SUCCESS;

 // switch on the command type
 switch ( Command )
  {
  case TIG_GET_PRINT_BUFF:
   {
   TIG_PRINT_DESC *pDesc=(TIG_PRINT_DESC *) CmdArg;
   Log_Copy (pDesc);
   break ;
   }
   
  case TIG_GET_DEBUG_DATA :
   {
   TIG_DEBUG_DATA_DESC *pDesc = (TIG_DEBUG_DATA_DESC *) CmdArg ;
   memset ( pDesc->Data, 0 , sizeof (pDesc->Data) );
   // load debug parms into pDesc->Data[0] to [9]
   break ;
   }

  case TIG_LOAD_START:
#ifdef PIM_DEBUG
printk(KERN_ALERT "CnxADSL: ARMALIoctl TIG_LOAD_START Prepare device for download command=%d\n", Command);
#endif
   pThisAdapter->Loading=TRUE;
   //*
   //* See if Extended Command set is already loaded.  This will reset the
   //* AutoDownload flag if the ARM responds to any extended commands.
   //*
   ARMIsCommandSetLoaded(pThisAdapter);
   if(!(pThisAdapter->AutoDownload))
    Status=-EALREADYLOADED;
   break;

  case TIG_LOAD_RECORD:
   {
   TIG_LOAD_RECORD_T* pRecord=(TIG_LOAD_RECORD_T*)CmdArg;

   // error out if I am not loading
   if (!pThisAdapter->Loading)
    {
    printk(KERN_ALERT "CnxADSL: ARMALIoctl TIG_LOAD_RECORD Load record while not in load\n");
    Status=-EBADFD;
    break;
    }
   else if (pRecord->RecordType==DATA_REC)
    {
    Status=processDataRec(pThisAdapter, pRecord->RecordData, pRecord->RecordAddress);
    }
   else if (pRecord->RecordType==EXT_GOTO_CMD_REC)
    {
    P46_IO_COMMAND_T ArmCommand;
    //*
    //* Start Segment Address (GOTO)
    //*
    ArmCommand.CsrParam.NotifyOnComplete = NULL;
    ArmCommand.CsrParam.Command	= ARM_GOTO;
    ArmCommand.CsrParam.Param.Goto.Address = pRecord->RecordAddress;
    Status=ChipALWrite(pThisAdapter, P46_ARM, ADDRESS_ARM_BAR_0, CSR_ARM_CMD, sizeof(P46_IO_COMMAND_T), (DWORD) &ArmCommand, 0);
    }
   else
    Status=-EINVALIDTYPE;
   break;
   }

  case TIG_LOAD_DONE:
#ifdef PIM_DEBUG
printk(KERN_ALERT "CnxADSL: ARMALIoctl TIG_LOAD_DONE Device finished download OK? command=%d\n", Command);
#endif
  // error out if I am not loading
   if (!pThisAdapter->Loading)
    {
    printk(KERN_ALERT "CnxADSL: ARMALIoctl TIG_LOAD_DONE Load record while not in load\n");
    Status=-EBADFD;
    break;
    }
   else
    {
    Status=ARMALStartArm(pThisAdapter);
    
    if (Status==STATUS_SUCCESS)
     {
     Status=ARMALInitIo(pThisAdapter);
     if (Status!=STATUS_SUCCESS)
      printk(KERN_ALERT "CnxADSL: ARMALIoctl TIG_LOAD_DONE ARMALInitIo failed with %lx\n", Status);
     }
    else
     printk(KERN_ALERT "CnxADSL: ARMALIoctl TIG_LOAD_DONE ARMALStartArm failed with %lx\n", Status);
    }
   break;
  }
 
 return Status;
 }


/******************************************************************************
FUNCTION NAME:
	ARMIsCommandSetLoaded

ABSTRACT:
	Determine if ARM is already Downloaded


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NTSTATUS
ARMIsCommandSetLoaded(
    IN	CDSL_ADAPTER_T	*pThisAdapter 	)
{
	NTSTATUS						Status;
	P46_IO_COMMAND_T				ArmCommand;
	signed long 					NsecDelay;
	int								TimeRemain;
	EVENT_HNDL						CompletionEvent;
	static volatile ARM_ASYC_IO_T	ArmSync;

	DBG_CDSL_DISPLAY(
		DBG_LVL_MISC_INFO,
		ChipALDebugFlag,
		("CnxADSL=>ARMAbstract:ARMIsCommandSetLoaded"));

	// prepare the wait entry
	INIT_EVENT(&CompletionEvent);

	NsecDelay = DOWNLOAD_TIMEOUT;
	ArmSync.pAdapter = pThisAdapter;
	ArmSync.pEvent = &CompletionEvent;

	// add a DPC handler to get the response
	//	DBG_CDSL_DISPLAY(
	//		DBG_LVL_MISC_INFO,
	//		ChipALDebugFlag,
	//		("CnxADSL=>ARMAbstract:ARMIsCommandSetLoaded  add event handler"));
	Status = ChipALAddEventHandler( pThisAdapter,
	                                ARM_FUNCTION,
	                                HW_COMMAND_SYNC,
	                                HW_DPC_EVENT,
	                                ARMSyncRoutine,
	                                (VOID *) &ArmSync );

	ArmSync.Resp2 = 0;
	ArmCommand.CsrParam.Command = IS_CMD_LOADED | ARM_COMMAND_SYNC;
	ArmCommand.CsrParam.Param.CommandSync.Response = RESPONSE_YES;

	ArmCommand.CsrParam.NotifyOnComplete = NULL;

	// issu the command
	//	DBG_CDSL_DISPLAY(
	//		DBG_LVL_MISC_INFO,
	//		ChipALDebugFlag,
	//		("CnxADSL=>ARMAbstract:ARMIsCommandSetLoaded  issue a write IS_CMD_LOADED"));

	// don't do this as it causes an interrupt on the DSL function before the ARM function
	// completes all operations and the Qube doesn't like interrupts on both functions simultaneously.
	//			calP46WriteCsr(pContext, BaseAddress, &Command);
	//Status = ChipALWrite(	pThisAdapter,
	//                      P46_ARM,
	//                      ADDRESS_ARM_BAR_0,
	//                      CSR_ARM_CMD,
	//                      sizeof(ArmCommand),
	//                      (DWORD) &ArmCommand,
	//                      0
	//                    );

	//	DBG_CDSL_DISPLAY(
	//		DBG_LVL_MISC_INFO,
	//		ChipALDebugFlag,
	//		("CnxADSL=>ARMAbstract:ARMIsCommandSetLoaded  wait for complete"));
	
	// speed up timeout that we know will happen since we commented out the write above
	TimeRemain = WAIT_EVENT (&CompletionEvent, 1);
	//TimeRemain = WAIT_EVENT (&CompletionEvent, NsecDelay);

	//* Remove Sync Handler
	//	DBG_CDSL_DISPLAY(
	//		DBG_LVL_MISC_INFO,
	//		ChipALDebugFlag,
	//		("CnxADSL=>ARMAbstract:ARMIsCommandSetLoaded  remove event handler"));
	ChipALRemoveEventHandler( pThisAdapter,
	                          ARM_FUNCTION,
	                          HW_COMMAND_SYNC,
	                          HW_DPC_EVENT,
	                          ARMSyncRoutine);

	// Make sure we did not timeout and the ARM sent a command sync
	// response.  0 indicates timeout.  It really doesn't matter if it was
	// our command sync response or if it was one from some other module.
	// We only want to know if the ARM is responding.
	if (TimeRemain != 0)
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			ChipALDebugFlag,
			("CnxADSL=>ARMAbstract:ARMIsCommandSetLoaded  no need to download"));
		pThisAdapter->AutoDownload =  FALSE;
		Status = STATUS_SUCCESS;
	}
	else
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			ChipALDebugFlag,
			("CnxADSL=>ARMAbstract:ARMIsCommandSetLoaded  download required"));
		pThisAdapter->AutoDownload =  TRUE;
		Status = STATUS_FAILURE;
	}

	return(Status);
}


/******************************************************************************
FUNCTION NAME:
	ARMSyncRoutine

ABSTRACT:
	Used as	a  general purpose completion routine so it	can	signal an ARM 
	response to a specific command.

RETURN:
	VOID

DETAILS:
******************************************************************************/
VOID ARMSyncRoutine(IN void *pSync, IN RESPONSE_T *Response)
 {
#ifdef PIM_DEBUG
printk(KERN_ALERT "CnxADSL: ARMSyncRoutine Resp2=0x%lx\n", *((DWORD *)(&Response->Param[0])));
#endif

 // Store Response
 ((ARM_ASYC_IO_T*)pSync)->Resp2 = *((DWORD *)(&Response->Param[0]));
 // Signal Waiting Thread
 SET_EVENT(((ARM_ASYC_IO_T*)pSync)->pEvent);
 
 return;
 }




/******************************************************************************
FUNCTION NAME:
	processDataRec

ABSTRACT:
	Process Data Record


RETURN:
	NTSTATUS:


DETAILS:
	This Function was orginally based on the RipTide processDataRec function.
	Essentially, it is the same.  The major difference is how writes to the
	hardware are performed.  

	In this implementation, the ARM CSR is interrupt driven.  Waits are used
	to ensure we have one command ready to go as soon as the ARM finishes 
	processing the current command.  In other words, the first command
	we send down gets written immediately.  The second command will be queued
	by the lower layer.  The second command is written automatically by the
	lower layer DPC when the ARM interrupts to indicate command complete.  
	When wirtten, it signals the local thread to continue
	processing.
******************************************************************************/
NTSTATUS	processDataRec(
    IN	CDSL_ADAPTER_T	*pThisAdapter,
    IN  char*			cInRec,
    IN  DWORD			uExtAddr )
{
	DWORD 						uDataAddr;	// Data	address
	DWORD 						uData;		// Program data	to download
	DWORD 						uByteCnt;	// Number of data bytes	in this	record
	char						* pInPtr;	// Points into the input data record
	NTSTATUS					Status;		// Misc completion Status
	signed long 				NsecDelay;
	int							TimeRemain;
	EVENT_HNDL					CompletionEvent;
	static P46_IO_COMMAND_T		ArmCommand;		// Command written to Hardware

//	DBG_CDSL_DISPLAY(
//		DBG_LVL_MISC_INFO,
//		ChipALDebugFlag,
//		("CnxADSL=>ARMAbstract:processDataRec"));

	NsecDelay = DOWNLOAD_TIMEOUT;

	// prepare the wait entry
	INIT_EVENT(&CompletionEvent);

	// Parse the record	byte count
	uByteCnt = atoh( &cInRec[ 1	], 2 );

	// Calculate the data load address
	uDataAddr =	( uExtAddr + atoh( &cInRec[	3 ], 4 ) );

	ArmCommand.CsrParam.NotifyOnComplete = ARMALP46CompletionRoutine;
	ArmCommand.CsrParam.UserContext = &CompletionEvent;
	ArmCommand.CsrParam.Command	= ARM_SET_MEMORY_WRITE_ADDRESS;
	ArmCommand.CsrParam.Param.SetWriteAddress.Address =	uDataAddr;
	ArmCommand.CsrParam.Param.SetWriteAddress.AutoIncrement	= ARM_AUTO_INCREMENT_YES;

	//	DBG_CDSL_DISPLAY(
	//		DBG_LVL_MISC_INFO,
	//		ChipALDebugFlag,
	//		("CnxADSL=>ARMAbstract:processDataRec  Write Address cmd"));

	Status = ChipALWrite(	pThisAdapter,
	                      P46_ARM,
	                      ADDRESS_ARM_BAR_0,
	                      CSR_ARM_CMD,
	                      sizeof(P46_IO_COMMAND_T), // Length is unused for ARM.
	                      (DWORD) &ArmCommand,
	                      0 );	// Mask	Valus is unused	by ARM

	TimeRemain = WAIT_EVENT (&CompletionEvent, NsecDelay);
	if (TimeRemain == 0)
	{
		Status = -ETIMEOUT;
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("CnxADSL=>ARMAbstract:processDataRec  Write Address cmd timed out"));
		return (Status);
	}

	// Point to	the	start of the data in the input record
	pInPtr = cInRec	+ 9;

	//*	Set	Arm	Command	for	Dword writes
	ArmCommand.CsrParam.Command	= ARM_WRITE_MEMORY;
	ArmCommand.CsrParam.Param.WriteMemory.AccessType = ARM_ACCESS_32;

	//	DBG_CDSL_DISPLAY(
	//		DBG_LVL_MISC_INFO,
	//		ChipALDebugFlag,
	//		("CnxADSL=>ARMAbstract:processDataRec  send record data"));

	// loop through the data and send each word
	while( uByteCnt	)
	{
		// prepare the event
		RESET_EVENT( &CompletionEvent );

		// Read	a DWORD	of data	from the record
		uData =	parseData( pInPtr );

		ArmCommand.CsrParam.Param.WriteMemory.Value	= uData;
		Status = ChipALWrite(	pThisAdapter,
		                      P46_ARM,
		                      ADDRESS_ARM_BAR_0,
		                      CSR_ARM_CMD,
		                      sizeof(P46_IO_COMMAND_T), // Length is unused for ARM.
		                      (DWORD) &ArmCommand,
		                      0 	);

		TimeRemain = WAIT_EVENT (&CompletionEvent, NsecDelay);
		if (TimeRemain == 0)
		{
			DBG_CDSL_DISPLAY(
				DBG_LVL_GENERIC,
				DBG_FLAG_DONT_CARE,
				("CnxADSL=>ARMAbstract:processDataRec  Write data cmd timed out"));
			Status = -ETIMEOUT;
			return (Status);
		}

		// Adjust the byte count, input	pointer, and data address
		uByteCnt -=	4;
		pInPtr += 8;
	}

	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	ARMALP46CompletionRoutine

ABSTRACT:
	Used as	a  general purpose completion routine so it	can	signal an event,
	passed as the Context


RETURN:
	VOID

DETAILS:
******************************************************************************/
VOID ARMALP46CompletionRoutine(
    IN PVOID Context
)
{
	EVENT_HNDL*	Event =	Context;

	// Set the input event
	// Signal Waiting Thread
	SET_EVENT(Event);

	return;
}


/****************************************************************************
 *
 *	Name:			static DWORD atoh( char* cInStr, DWORD uNumChars )
 *
 *	Description:	Converts ASCII hex to DWORD
 *
 *	Inputs:			char *cInStr = pointer to string
 *					DWORD uNumChars	= number of	characters in the string
 *
 *	Outputs:		none
 *
 ***************************************************************************/
DWORD atoh( CHAR* cInStr, DWORD uNumChars )
{
	DWORD uSum = 0;
	DWORD uMult	= 1;

	while( uNumChars )
	{
		if ( cInStr[ uNumChars - 1 ] >=	'0'	&& cInStr[ uNumChars - 1 ] <= '9' )
		{
			uSum +=	uMult *	( cInStr[ uNumChars	- 1	] -	'0'	);
		}

		else if	( cInStr[ uNumChars	- 1	] >= 'A' &&	cInStr[	uNumChars -	1 ]	<= 'F' )
		{
			uSum +=	uMult *	( cInStr[ uNumChars	- 1	] -	'A'	+ 10 );
		}

		else if	( cInStr[ uNumChars	- 1	] >= 'a' &&	cInStr[	uNumChars -	1 ]	<= 'f' )
		{
			uSum +=	uMult *	( cInStr[ uNumChars	- 1	] -	'a'	+ 10 );
		}

		uMult *= 16;
		--uNumChars;
	}

	return uSum;
}


/****************************************************************************
 *
 *	Name:			static DWORD parse Data( char* cInStr )
 *
 *	Description:	Converts ASCII hex to a	DWORD of data
 *
 *	Inputs:			char *cInStr = pointer to string
 *
 *	Outputs:		none
 *
 ***************************************************************************/
DWORD parseData(	char* cInStr )
{
	DWORD uCharNum;
	DWORD uSum = 0;
	static DWORD uShift[] =	{ 4, 0,	12,	8, 20, 16, 28, 24 };

	for( uCharNum =	0; uCharNum	< 8; ++uCharNum	)
	{
		if ( cInStr[ uCharNum ]	>= '0' && cInStr[ uCharNum ] <=	'9'	)
		{
			uSum +=	( DWORD)( cInStr[ uCharNum ] - '0' ) <<	uShift[	uCharNum ];
		}

		else if	( cInStr[ uCharNum ] >=	'A'	&& cInStr[ uCharNum	] <= 'F' )
		{
			uSum +=	( DWORD	)( cInStr[ uCharNum	] -	'A'	+ 10 ) << uShift[ uCharNum ];
		}

		else if	( cInStr[ uCharNum ] >=	'a'	&& cInStr[ uCharNum	] <= 'f' )
		{
			uSum +=	( DWORD	)( cInStr[ uCharNum	] -	'a'	+ 10 ) << uShift[ uCharNum ];
		}
	}

	return uSum;
}


/******************************************************************************
FUNCTION NAME:
	CmgmtStartArm

ABSTRACT:
	Start Arm Extended Command Set


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NTSTATUS ARMALStartArm(IN CDSL_ADAPTER_T *pThisAdapter)
 {
 P46_IO_COMMAND_T ArmCommand;
 NTSTATUS Status;
 int TimeRemain;
 signed long NsecDelay;

 //* The Completion and Sync routines are synchronous to allow for
 //* case when Interrupts are not working correctly or the system
 //* is extremely slow.  In such a case we will Timeout and not download
 //* the code.
 EVENT_HNDL CompletionEvent;
 static volatile ARM_ASYC_IO_T ArmSync;

 INIT_EVENT(&CompletionEvent);
 NsecDelay=DOWNLOAD_TIMEOUT;
 ArmSync.pAdapter=pThisAdapter;
 ArmSync.pEvent=&CompletionEvent;

 do
  {
  ArmCommand.CsrParam.Command=ARM_GOTO;
  ArmCommand.CsrParam.Param.Goto.Address=JUMP_ADDRESS;
  ArmCommand.CsrParam.NotifyOnComplete=ARMALP46CompletionRoutine;
  ArmCommand.CsrParam.UserContext=&CompletionEvent;
  Status=ChipALWrite(pThisAdapter,
		     P46_ARM,
		     ADDRESS_ARM_BAR_0,
		     CSR_ARM_CMD,
		     sizeof(ArmCommand),
		     (DWORD) &ArmCommand,
		     0 						// Mask	Valus is unused	by ARM
		     );
  if (!NT_SUCCESS(Status))
   printk(KERN_ALERT "CnxADSL: ARMALStartArm ARM GoTo cmd failed, Status=%lx\n", Status);

  TimeRemain=WAIT_EVENT(&CompletionEvent, NsecDelay);
  if (TimeRemain==0)
   {
   Status=-ETIMEOUT;
   printk(KERN_ALERT "CnxADSL: ARMALStartArm Write Timeout waiting on Jump Address\n");
   break;
   }

  RESET_EVENT(&CompletionEvent);
  // The Response is Interrupt Driven.  We need a sync handler to catch the
  // response from ChipAl
  ArmCommand.CsrParam.NotifyOnComplete=NULL;
  ArmCommand.CsrParam.UserContext=NULL;
  Status=ChipALAddEventHandler(pThisAdapter,
			       ARM_FUNCTION,
			       HW_BOOT_READ_MEMORY,
			       HW_DPC_EVENT,
			       ARMSyncRoutine,
			       (VOID *) &ArmSync);
  ArmSync.Resp2=0;
  ArmCommand.CsrParam.Command=ARM_READ_MEMORY;
  ArmCommand.CsrParam.Param.ReadMemory.Address=EXT_COMMAND_ADDRESS;
  ArmCommand.CsrParam.Param.ReadMemory.AccessType=ARM_ACCESS_32;
  if (!NT_SUCCESS(Status))
   printk(KERN_ALERT "CnxADSL: ARMALStartArm ChipALAddEventHandler failed, Status=%lx\n", Status);

  Status=ChipALWrite(pThisAdapter,
		     P46_ARM,
		     ADDRESS_ARM_BAR_0,
		     CSR_ARM_CMD,
		     sizeof(ArmCommand),
		     (DWORD) &ArmCommand,
		     0 					// Mask	Values is unused	by ARM
		     );
  if (!NT_SUCCESS(Status))
   printk(KERN_ALERT "CnxADSL: ARMALStartArm ARM read address failed, Status=%lx\n", Status);
  TimeRemain=WAIT_EVENT(&CompletionEvent, NsecDelay);

  //* Remove Sync Handler
  ChipALRemoveEventHandler(pThisAdapter,
			   ARM_FUNCTION,
			   HW_BOOT_READ_MEMORY,
			   HW_DPC_EVENT,
			   ARMSyncRoutine);

  if(TimeRemain==0 || ArmSync.Resp2 == 0)
   {
   Status=-ETIMEOUT;
   printk(KERN_ALERT "CnxADSL: ARMALStartArm Read Ext Command Address Failed, TimeRemain=%d ArmSync.Resp2=%lx\n", TimeRemain, ArmSync.Resp2);
   break;
   }

  RESET_EVENT(&CompletionEvent);
  ArmCommand.CsrParam.Command=ARM_SET_COMMAND_EXTENSION_ROUTINE_ADDRESS;
  ArmCommand.CsrParam.Param.CommandExtAddress.Address=ArmSync.Resp2;
  ArmCommand.CsrParam.NotifyOnComplete=ARMALP46CompletionRoutine;
  ArmCommand.CsrParam.UserContext =&CompletionEvent;
  Status = ChipALWrite(pThisAdapter,
		       P46_ARM,
		       ADDRESS_ARM_BAR_0,
		       CSR_ARM_CMD,
		       sizeof(ArmCommand), // Length is unused for ARM.
		       (DWORD) &ArmCommand,
		       0			// Mask	Values is unused by ARM
		       );
  if (!NT_SUCCESS(Status))
   printk(KERN_ALERT "CnxADSL: ARMALStartArm ARM set extension cmd failed, Status=%lx\n", Status);
  TimeRemain=WAIT_EVENT (&CompletionEvent, NsecDelay);
  if (TimeRemain==0)
   {
   Status=-ETIMEOUT;
   printk(KERN_ALERT "CnxADSL: ARMALStart Arm Timeout waiting on Write of Ext. Address.\n");
   break;
   }
  Status=STATUS_SUCCESS;
  } while(FALSE);

 if(Status!=STATUS_SUCCESS )
  printk(KERN_ALERT "CnxADSL: ARMALStart Arm Was Never Ready for Operation!\n");

 return Status;
 }


#define MAX_NUM_GPIO_PINS 	( sizeof(DWORD) * 8 )

/****************************************************************************
 *
 *	Name:			LOCAL GPIO_INIT_T ARMALSetEdge(WORD pin)
 *
 *	Description:	Builds an ARM Set IO Edge command.	The ARM
 *					initializes all GPIO's to trigger on the falling edge.
 *					This command is used to override the default.
 *
 *	Inputs:			GPIO pin number.
 *
 *	Outputs:		GPIO command data structure.
 *
 ***************************************************************************/
LOCAL GPIO_INIT_T	ARMALSetEdge(WORD pin)
{
	GPIO_INIT_T	GPIO_Init = { { ARM_SET_IO_EDGE, {{0}}, NULL, NULL}, 
							  sizeof ( COMMAND_SET_IO_EDGE_T ) 
	};

	DBG_CDSL_DISPLAY(
		DBG_LVL_MISC_INFO,
		ChipALDebugFlag,
		("CnxADSL=>ARMAbstract:ARMALSetEdge"));

	//
	//Parameter 1 - GPIO Pin number
	//
	GPIO_Init.Csr_Command_Param.Param.IoEdge.Number  = pin;

	//
	//Parameter 2 - Edge Trigger, Falling = 1
	//
	GPIO_Init.Csr_Command_Param.Param.IoEdge.Trigger = 1;
	
	return GPIO_Init;
}

/****************************************************************************
 *
 *	Name:			LOCAL GPIO_INIT_T ARMALSetIoDirection(WORD pin)
 *
 *	Description:	Builds an ARM Set IO Direction command.	The ARM
 *					initializes all GPIO's as input.
 *					This command is used to override the default.
 *
 *	Inputs:			GPIO pin number.
 *
 *	Outputs:		GPIO command data structure.
 *
 ***************************************************************************/
LOCAL GPIO_INIT_T ARMALSetIoDirection(DWORD GpioDirectionMask, WORD pin)
{
	WORD 		direction;
	GPIO_INIT_T	GPIO_Init = { { ARM_SET_IO_DIRECTION, {{0}}, NULL, NULL}, 
							  sizeof ( COMMAND_SET_IO_DIRECTION_T ) 
	};

	DBG_CDSL_DISPLAY(
		DBG_LVL_MISC_INFO,
		ChipALDebugFlag,
		("CnxADSL=>ARMAbstract:ARMALSetIoDirection"));

	//
	//Parameter 1 - GPIO Pin Number
	//
	GPIO_Init.Csr_Command_Param.Param.IoDirection.Number  = pin;
	
	//
	//Parameter 2 - Direction
	//
	direction = ( GpioDirectionMask & ( 1 << pin )) ? 1 : 0;
	GPIO_Init.Csr_Command_Param.Param.IoDirection.Direction = direction;
	
	return	GPIO_Init;
}
/****************************************************************************
 *
 *	Name:			LOCAL GPIO_INIT_T ARMALSetIoAccessPrivileges( 
 *											WORD function, 
 *											UDWORD GpioMask 
 *					);
 *
 *	Description:	Builds an ARM Set IO Access Privileges command.	
 *
 *	Inputs:			GPIO pin number.
 *					GPIO Mask of bits affected.
 *
 *	Outputs:		GPIO command data structure.
 *
 ***************************************************************************/
LOCAL GPIO_INIT_T ARMALSetIoAccessPrivileges( WORD function, DWORD GpioMask )
{
	GPIO_INIT_T	GPIO_Init = { { ARM_SET_IO_ACCESS_PRIV, {{0}}, NULL, NULL}, 
							  sizeof ( CSR_COMMAND_PARAM_T ) 
	};

	DBG_CDSL_DISPLAY(
		DBG_LVL_MISC_INFO,
		ChipALDebugFlag,
		("CnxADSL=>ARMAbstract:ARMALSetIoAccessPrivileges"));

   	//
   	//Parameter 1 
	//
	GPIO_Init.Csr_Command_Param.Param.IoAccess.Function = function;

	//
	//Parameter 2 
   	//
   	GPIO_Init.Csr_Command_Param.Param.IoAccess.IoAccess = GpioMask;

	return	GPIO_Init;	
}
/****************************************************************************
 *
 *	Name:			LOCAL GPIO_INIT_T ARMALSetIo(WORD pin, DWORD value)
 *
 *	Description:	Builds an ARM Set IO command.	
 *
 *	Inputs:			GPIO pin number.
 *					value 1 = ON, 0 = OFF
 *
 *	Outputs:		GPIO command data structure.
 *
 ***************************************************************************/
LOCAL GPIO_INIT_T ARMALSetIo(WORD pin, WORD value)
{
  	GPIO_INIT_T	GPIO_Init = { { ARM_SET_IO, {{0}}, NULL, NULL}, 
							  sizeof (DWORD) + sizeof ( COMMAND_SET_IO_T )
	};

	DBG_CDSL_DISPLAY(
		DBG_LVL_MISC_INFO,
		ChipALDebugFlag,
		("CnxADSL=>ARMAbstract:ARMALSetIo"));

   	//
   	//Parameter 1
	//
	GPIO_Init.Csr_Command_Param.Param.SetIo.Number = pin;
	
	//
	//Parameter 2
	//
	GPIO_Init.Csr_Command_Param.Param.SetIo.Value = value;
	
	return	GPIO_Init;
}
/****************************************************************************
 *
 *	Name:			LOCAL void initIO( CDSL_ADAPTER_T * pThisAdapter )
 *
 *	Description:	Initializes GPIO access priviledges and directions
 *
 *	Inputs:			CDSL_ADAPTER_T * pThisAdapter = pointer to adapter context
 *
 *	Outputs:		none
 *
 ***************************************************************************/
static NTSTATUS ARMWrite(CDSL_ADAPTER_T *pThisAdapter, GPIO_INIT_T GpioInit)
{
	NTSTATUS					Status=STATUS_SUCCESS;
	static P46_IO_COMMAND_T		Command;
	Command.CsrParam = GpioInit.Csr_Command_Param;
	if (!NT_SUCCESS(Status = ChipALWrite(pThisAdapter, P46_ARM,
		ADDRESS_ARM_BAR_0, CSR_ARM_CMD, sizeof (Command),
		(DWORD)&Command, 0)))
	{
		DBG_CDSL_DISPLAY(DBG_LVL_GENERIC, DBG_FLAG_DONT_CARE,
			("CnxADSL=>ARMAbstract:ARMALInitIo  Write command failure on write #%d.", Tbl_Index));
	}
	return Status;
}

static GPIO_INIT_T SetNotify(GPIO_INIT_T GpioInit, EVENT_HNDL *pCompletionEvent)
{
	GpioInit.Csr_Command_Param.UserContext = pCompletionEvent;
	GpioInit.Csr_Command_Param.NotifyOnComplete = ARMALP46CompletionRoutine;
	return GpioInit;
}

static NTSTATUS InitIo
(
	CDSL_ADAPTER_T *pThisAdapter,
	EVENT_HNDL *pCompletionEvent
)
{
	WORD 					   	pin;
	NTSTATUS					Status=STATUS_SUCCESS;

	//Setup ADSL Access Privileges
	if (!NT_SUCCESS(Status = ARMWrite(pThisAdapter,
		ARMALSetIoAccessPrivileges(1,
		pThisAdapter->PersistData.GpioADSLFunctionMask))))
		return Status;

	//Setup Arbitrator Access Privileges
	if (!NT_SUCCESS(Status = ARMWrite(pThisAdapter,
		ARMALSetIoAccessPrivileges(0,
		pThisAdapter->PersistData.GpioARMFunctionMask))))
		return Status;

	//Build-up GPIO commands
	for (pin = 0; pin < MAX_NUM_GPIO_PINS; pin++)
	{
		//Build Set IO Direction commands
		if (pThisAdapter->PersistData.GpioDirectionMask & (1<<pin))
		{
			if (!NT_SUCCESS(Status = ARMWrite(pThisAdapter,
				ARMALSetIoDirection(pThisAdapter->PersistData.
				GpioDirectionMask, pin))))
				return Status;
		}

		//Build Set Edge trigger commands
		if (pThisAdapter->PersistData.GpioEdgeMask & (1<<pin))
		{
			if (!NT_SUCCESS(Status = ARMWrite(pThisAdapter,
				ARMALSetEdge(pin))))
				return Status;
		}
	}

	//Set initial GPIO bits ON (SSCAN).
	if (!NT_SUCCESS(Status = ARMWrite(pThisAdapter, ARMALSetIo(SSCAN,
		1))))
		return Status;

	//Set SSCAN I/O Direction
	return ARMWrite(pThisAdapter, SetNotify(ARMALSetIoDirection(
		pThisAdapter->PersistData.GpioDirectionMask, SSCAN),
		pCompletionEvent));
}

NTSTATUS ARMALInitIo(CDSL_ADAPTER_T *pThisAdapter)
{
	EVENT_HNDL CompletionEvent;
	NTSTATUS Status=STATUS_SUCCESS;
	signed long NsecDelay;

	DBG_CDSL_DISPLAY(
		DBG_LVL_MISC_INFO,
		ChipALDebugFlag,
		("CnxADSL=>ARMAbstract:ARMALInitIo"));

	NsecDelay = DOWNLOAD_TIMEOUT;
	INIT_EVENT(&CompletionEvent);

	InitIo(pThisAdapter, &CompletionEvent);

	if (!WAIT_EVENT(&CompletionEvent, NsecDelay))
	{
		Status = -ETIMEOUT ;
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("CnxADSL=>ARMAbstract:ARMALInitIo  Write Timout waiting on GPIO."));
	}
	else
	{
		Status = STATUS_SUCCESS;

		ARMALNotifyArmReady(pThisAdapter);
	}

	return Status;
}

/******************************************************************************
FUNCTION NAME:
	ARMALNotifyArmReady

ABSTRACT:
	Notify other functions ARM is now ready to receive commands

	Writes ARM_CONFIGURATION_COMPLETE and then waits on the corresponding response.
	This waiting on the response is in order to determine when the last
	ARM response has been received. This is to work around a problem with 
	Qube Linux in that if an interrupt is pending on both functions simultaneously,
	interrupts are no longer serviced.

RETURN:
	VOID


DETAILS:
******************************************************************************/
VOID ARMALNotifyArmReady(
    IN CDSL_ADAPTER_T	*pThisAdapter
)
{
	NTSTATUS						Status;
	P46_IO_COMMAND_T				Command;
	int								TimeRemain;
	EVENT_HNDL						CompletionEvent;
	static volatile ARM_ASYC_IO_T	ArmSync;

	DBG_CDSL_DISPLAY(
		DBG_LVL_MISC_INFO,
		ChipALDebugFlag,
		("CnxADSL=>ARMAbstract:ARMALNotifyArmReady"));

	// prepare the wait entry
	INIT_EVENT(&CompletionEvent);

	ArmSync.pAdapter = pThisAdapter;
	ArmSync.pEvent = &CompletionEvent;

	Status = ChipALAddEventHandler( pThisAdapter,
	                                ARM_FUNCTION,
	                                HW_CONFIGURATION_COMPLETE,
	                                HW_DPC_EVENT,
	                                ARMSyncRoutine,
	                                (VOID *) &ArmSync );


	Command.CsrParam.Command = ARM_CONFIGURATION_COMPLETE;
	Command.CsrParam.NotifyOnComplete = NULL;

	Status = ChipALWrite(
	             pThisAdapter,
	             P46_ARM,
	             ADDRESS_ARM_BAR_0,
	             CSR_ARM_CMD,
	             sizeof(Command),	// unused
	             (DWORD) &Command,
	             0 			// Mask Valus is unused	by ARM
	         );

	// wait on response
	TimeRemain = WAIT_EVENT (&CompletionEvent, DOWNLOAD_TIMEOUT);

	// Make sure we did not timeout and the ARM sent a response
	if (TimeRemain != 0)
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			ChipALDebugFlag,
			("CnxADSL=>ARMAbstract:ARMALNotifyArmReady  ARM function completed"));
		Status = STATUS_SUCCESS;

	}
	else
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			ChipALDebugFlag,
			("CnxADSL=>ARMAbstract:ARMALNotifyArmReady  ARM_CONFIGURATION_COMPLETE timed out!"));
		Status = STATUS_FAILURE;
	}

	// write the bit that tells the firmware it is now safe to begin DSL responses
	// (and therefor the first DSL interrupt) since the last ARM interrupt
	// (the ARM_CONFIGURATION_COMPLETE response) has completed.
	if ( Status == STATUS_SUCCESS )
	{
		ChipALSetFirmGo (pThisAdapter) ;
	}
}
