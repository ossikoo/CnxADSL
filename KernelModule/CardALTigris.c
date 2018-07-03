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
**		CardALTigris.c
**
**	ABSTRACT:
**		Card level Abstraction Layer for CDSL.  This file/module abstracts
**		(hides the details of) card specific (CDSL) functions and presents
**		(hopefully) a constant interface to the upper level modules/functions
**		(CardMgmt) no matter what the actual card is (CDSL, ADSL, ISDN, etc.).
**		This file provides the functions that require some amount of knowledge
**		of the card/chip (H/W) for the CardAL module.
**
**	DETAILS:
**		Functions are arranged in top-down order.
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.4.3_ATM_Driver/KernelModule/CardALTigris.c $
** $Revision: 25 $
** $Date: 8/09/01 1:47p $
*******************************************************************************
******************************************************************************/

#define		MODULE_ID			DBG_ENABLE_CARDAL
#define CARDAL_TIGRIS_OWNER


#include "types.h"
#include "datapump.h"
#include "LnxTools.h"
#include "CardMgmt.h"
#include "CardMgmtV.h"
#define		CARDAL_DECLARE_PUBLIC			0
#include "CardALV.h"
#undef		CARDAL_DECLARE_PUBLIC
#include "EEType.h"
#include "CardALTigrisDp.h"
#include "CardALTigrisV.h"

#include "types.h"
#include "sysinfo.h"
#include "project.h"
#include "ChipALCdsl.h"
#include "ChipALSEmw.h"
#include <linux/delay.h>
#include <linux/kthread.h>

#include "dmtintf.h"
#include "asicutil.h"

#ifndef MEMORY_TAG
	#define	MEMORY_TAG		'GXNC'
#endif

#define ERROR_FATAL(error)		(LEVEL_FATAL   | MODULE_CARDAL | INSTANCE_CARDAL | error)
#define ERROR_WARNING(error)	(LEVEL_WARNING | MODULE_CARDAL | INSTANCE_CARDAL | error)
#define ERROR_NORMAL(error)		(LEVEL_NORMAL  | MODULE_CARDAL | INSTANCE_CARDAL | error)

//	Unique file ID for error logging
#define		__FILEID__			21

#define DEBUG_OPEN_MESSAGE	"DSL Debug Channel Opened\n"


#define SI_THREAD_SLEEP_INTERVAL	1
#define BG_THREAD_SLEEP_INTERVAL	1
#define TM_THREAD_SLEEP_INTERVAL	10

#define BG_THREAD_BOOST				99
#define SI_THREAD_BOOST				98
#define TM_THREAD_BOOST				97

#define DEVICE_WAIT_TIMEOUT			(10*1000)		// MS (10 sec)
#define THREAD_SHUTDOWN_TIMEOUT		(10*1000)		// MS (10 sec)
#define BG_PAUSE_TIME				100				// MS (100 MS)
#define SI_PAUSE_TIME				100				// MS (100 MS)
#define RT_PAUSE_TIME 100
#define MAX_LOOP					(2)				//
#define ONE_SECOND					1000
#define DELTA_CALL					(ONE_SECOND / TICKS_PER_SEC)
#define DYING_GASP_TIMEOUT			20			// Time (MS) allowed for controller code to send Dying Gasp Command

///////////////////////////////////////////////////////////////////////////////
//	Defines
///////////////////////////////////////////////////////////////////////////////

#define CONVERT_TO_CHIP_PM_STATE(x) \
		((x==BD_PM_STATE_D0)? PM_STATE_D0:										\
		(x==BD_PM_STATE_D1)? PM_STATE_D1:										\
		(x==BD_PM_STATE_D2)? PM_STATE_D2:										\
		(x==BD_PM_STATE_D3_HOT)? PM_STATE_D3_HOT:								\
		(x==BD_PM_STATE_D3_COLD)? PM_STATE_D3_COLD:								\
		PM_STATE_END)

// *
// * Define Devices that will be Power Managed by ChipAL
// *
// * Format of the PM Config Struct is defined by PM_CONFIG_REG_T. Currently,
// * it is defined as:
// *		REGISTER,SIZE,BITMASK,RESERVED,CLEAR_VALUE,CLEAR_REGISTER
// *

#define TIGRIS_PM_CONFIG \
{																				\
		{0,							0,				{0},	{0},0,FALSE}				\
   /*	{BASIC2_IE,		sizeof(DWORD) , PM_INTERRUPTS,	0,0,FALSE},		   */	\
   /*	{BASIC2_STATUS,	sizeof(DWORD) , PM_INTERRUPTS, 	0,PM_INTERRUPTS,TRUE},*/\
}

///////////////////////////////////////////////////////////////////////////////
//	Variable Declarations
///////////////////////////////////////////////////////////////////////////////

DWORD				DpVerLoadBlock = 0;
const DWORD				DP_DNLD_BLOCK_SIZE		= 0x10;
const DWORD				DP_UPLD_BLOCK_SIZE		= 0x02;


///////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
** Dispatch Functions assiged to various handlers
******************************************************************************/
GLOBAL NTSTATUS cardALOpenDumbTerminal(
    VOID					* UserContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS cardALCloseDumbTerminal(
    void					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS cardAlRxComm (
    VOID					* UserContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS cardALOpenDebugTerminal(
    void					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS cardALOpenDebugEcho(
    void					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);


DWORD cardALChipShutdownAdslLine(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    CARDAL_ADAPTER_T		* pCardALAdapter );

static void cardALChipCnvrtAdslRegOpRslt(
    I_O DWORD		* pRegOpResultValue );

static void cardALChipCnvrtBdRegDataSize(
    IN DWORD		BdRegDataSize,
    I_O DWORD		* pAdslRegDataSize );

static void cardALChipCnvrtBdRegAccessType(
    IN DWORD		BdRegAccessType,
    I_O DWORD		* pAdslAccessType,
    I_O DWORD		* pRegLogicalDevice );

static BOOLEAN cardALChipCheckRegSizeAccessType(
    IN DWORD		AdslDataSize,
    IN DWORD		AdslAccessType );


///////////////////////////////////////////////////////////////////////////////
//	LOCAL FUNCTIONS
///////////////////////////////////////////////////////////////////////////////


/******************************************************************************
FUNCTION NAME:
	cardALDeviceTestFalcon

ABSTRACT:
	Writes and Reads to Falcon scratchpad RAM

RETURN:
	NA.

DETAILS:
******************************************************************************/
#include "falcon.h"		// ***test
#include "CardALTigrisHalV.h"		// ***test
#include "SmSysIf.h"

ULONG Stall = 0 ;		//Ms
char RdVerify = 1 ;
char TestF = 1 ;
char Once = 1 ;
ULONG Delay = 0 ; 		//10 //ms
#define MAX_FALCON_RAM 512


MODULAR VOID cardALDeviceTestFalcon
(
    IN CDSL_ADAPTER_T			* pThisAdapter
)
{
#if 1	// ***test
	FALCON_TYPE		* Falcon_Ptr ;
	SHORT			  Index ;
	WORD			  Error_Count ;

	TIGRIS_CONTEXT_T			* pController ;
	DATAPUMP_CONTEXT			* pDatapump ;

	//pThisInterface = (CARDAL_ADAPTER_T *) pThisAdapter->CardMod ;
	pController = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pController->pDatapump ;

	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_TIGRIS (pController) ;
	VERIFY_DATAPUMP (pDatapump) ;

	Falcon_Ptr = FALCON_PTR (pDatapump) ;
	Falcon_Ptr->SPRAM = (SPRAM_TYPE far *)
	    ((UINT16 far *) FALCON_START_ADDR ( pDatapump ) + 0x0100L ) ; // start addr SPRAM

	if ( Once == 1 )
	{
		Once = 0 ;
		DBG_CDSL_DISPLAY
		(
		    DBG_LVL_MISC_INFO,
		    pThisAdapter->DebugFlag,
		    ("CardAlTigris BGThread:  P46_BAR_ADSL_CSR= %lx", ((ULONG) pDatapump->Falcon_Space-MICRO_OFFSET)) ;
		) ;

		DBG_CDSL_DISPLAY
		(
		    DBG_LVL_MISC_INFO,
		    pThisAdapter->DebugFlag,
		    ("CardAlTigris BGThread:  FALCON_START_ADDR(i.e. Falcon_Space)= %lx", (ULONG)pDatapump->Falcon_Space) ;
		) ;

		DBG_CDSL_DISPLAY
		(
		    DBG_LVL_MISC_INFO,
		    pThisAdapter->DebugFlag,
		    ("CardAlTigris BGThread:  Falcon_Ptr->SPRAM= %lx", (ULONG)Falcon_Ptr->SPRAM) ;
		) ;
		DBG_CDSL_DISPLAY
		(
		    DBG_LVL_MISC_INFO,
		    pThisAdapter->DebugFlag,
		    ("CardAlTigris BGThread:  Falcon_Ptr->SPRAM->sint16= %lx", (ULONG)Falcon_Ptr->SPRAM->sint16) ;
		) ;
	}

	//			Pattern = (Index&0xF)<<12 | (Index&0xF)<<8 | (Index&0xF)<<4 | (Index&0xF) ;

	if ( TestF )
	{
		// bring Falcon out of reset
		ChipAlTigrisDpSetRstXcvr ( 1 ) ;

		// wait 100 ms for soft commands to be sent
		SLEEP ( 100 ) ;

		//-----------------------------------------------------------------------
		// write IIII
		Error_Count = 0 ;
		for ( Index = MAX_FALCON_RAM-1 ; Index >= 00 ; Index -- )
		{
			WORD Pattern ;
			WORD Value ;

			if ( Delay) SLEEP ( Delay);
			if ( Stall ) MICRO_DELAY ( Stall ) ;

			Pattern = (Index&0xFF)<<8 | (Index&0xFF) ;
			Falcon_Ptr->SPRAM->sint16[Index] = Pattern ;

			if ( RdVerify )
			{
				if ( Delay) SLEEP ( Delay);
				if ( Stall ) MICRO_DELAY ( Stall ) ;
				Value = Falcon_Ptr->SPRAM->sint16[Index] ;
				if ( Value != Pattern )
				{
					if ( Error_Count <= 10 )
					{
						Error_Count ++ ;
						DBG_CDSL_DISPLAY
						(
						    DBG_LVL_ERROR,
						    pThisAdapter->DebugFlag,
						    ("CardAlTigris Falcon Scratch Pad Write Error %3x=%4x", Index, Value) ;
						) ;
					}
				}
			}
		}

		//-----------------------------------------------------------------------
		// Verify IIII
		Error_Count = 0 ;
		for ( Index = 0 ; Index < MAX_FALCON_RAM ; Index ++ )
		{
			WORD Value ;
			WORD Pattern ;

			if ( Delay) SLEEP ( Delay);
			if ( Stall ) MICRO_DELAY ( Stall ) ;

			Pattern = (Index&0xFF)<<8 | (Index&0xFF) ;
			Value = Falcon_Ptr->SPRAM->sint16[Index] ;
			if ( Value != Pattern )
			{
				Error_Count ++ ;
				if ( Error_Count <= 10 )
				{
					DBG_CDSL_DISPLAY
					(
					    DBG_LVL_ERROR,
					    pThisAdapter->DebugFlag,
					    ("CardAlTigris Falcon Scratch Pad Read Error %3x=%4x", Index, Value) ;
					) ;
				}
			}
		}

		//-----------------------------------------------------------------------
		// Hammer one location
		Error_Count = 0 ;
		for ( Index = 0 ; Index < 50 ; Index ++ )
		{
			WORD Value ;

			if ( Delay) SLEEP ( Delay);
			if ( Stall ) MICRO_DELAY ( Stall ) ;

			Falcon_Ptr->SPRAM->sint16[55] =  Index ;

			if ( Delay) SLEEP ( Delay);
			if ( Stall ) MICRO_DELAY ( Stall ) ;

			Value = (volatile WORD) Falcon_Ptr->SPRAM->sint16[55] ;
			if ( Value != Index )
			{
				Error_Count ++ ;
				if ( Error_Count <= 10 )
				{
					DBG_CDSL_DISPLAY
					(
					    DBG_LVL_ERROR,
					    pThisAdapter->DebugFlag,
					    ("CardAlTigris BGThread:  Falcon Scratch Pad[55] Error %x", Index) ;
					) ;
				}
			}
		}

		//-----------------------------------------------------------------------
		DBG_CDSL_DISPLAY
		(
		    DBG_LVL_MISC_INFO,
		    pThisAdapter->DebugFlag,
		    ("CardAlTigris BGThread:  Falcon Scratch Pad test completed;")
		) ;
	}
#endif
}



/******************************************************************************
FUNCTION NAME:
	CardALDeviceRdyNotify

ABSTRACT:
	Notifies us that the device is ready


RETURN:
	None


DETAILS:
******************************************************************************/
GLOBAL void CardALDeviceRdyNotify
(
    IN CDSL_ADAPTER_T			* pThisAdapter
)
{
	TIGRIS_CONTEXT_T			* pContext ;

	//pThisInterface = (CARDAL_ADAPTER_T *) pThisAdapter->CardMod ;
	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_TIGRIS (pContext) ;

	// Tell BGThread, which is waiting on device initialization, that it can run
	SET_EVENT ( &pContext->BGThreadDeviceRdyEvent ) ;
}

/******************************************************************************
******************************************************************************/


UINT16 MaxLoop = MAX_LOOP ;
UINT16 Bg_Pause_Time = BG_PAUSE_TIME ;
UINT16 Si_Pause_Time = SI_PAUSE_TIME ;
UINT16 Device_Wait_Timeout = DEVICE_WAIT_TIMEOUT ;
BOOLEAN BG_USE_NDIS = FALSE ;



/******************************************************************************
FUNCTION NAME:
	cardALStartDevice

ABSTRACT:
	This function is called to start the device.


RETURN:
	NA.


DETAILS:
******************************************************************************/
LOCAL VOID cardALStartDevice
(
    IN CDSL_ADAPTER_T			* pThisAdapter
)
{
	TIGRIS_CONTEXT_T			* pContext ;
	DATAPUMP_CONTEXT			* pDatapump ;

	DBG_CDSL_DISPLAY
	(
	    DBG_LVL_MISC_INFO,
	    pThisAdapter->DebugFlag,
	    ("CardAlTigris cardALStartDevice:  Entry;")
	) ;


	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	if(pContext == NULL)
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			CardALDebugFlag,
			("cardALStartDevice: pContext == NULL"));

		return;
	}
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_TIGRIS (pContext) ;
	VERIFY_DATAPUMP (pDatapump) ;

	// Initialze pointers that Pairgain uses to directly access hardware
	pDatapump->Falcon_Space = ChipAlTigrisHalGetFalconSpace ( pThisAdapter ) ;
	if ( pDatapump->Falcon_Space == NULL )
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_ERROR,
		    pThisAdapter->DebugFlag,
			("cardALStartDevice: ERROR;  Failed to get hardware address!\n"));

		return ;
	}

	// Tell ChipAL who to notify when device is ready. This function we supply
	// will set the event when it runs.
	ChipAlTigrisHalDeviceRdyNotifyReq ( CardALDeviceRdyNotify, pThisAdapter ) ;

	// Wait until the device GPIO has been initialized (Timeout = 10 seconds)
	if ( ! WAIT_EVENT ( &pContext->BGThreadDeviceRdyEvent,  Device_Wait_Timeout  ) )
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_ERROR,
		    pThisAdapter->DebugFlag,
		    ("CardAlTigris cardALStartDevice: ERROR;  Wait on DeviceRdy timed out!\n") );
	}

	// Call Initializations if we are not being shutdown already
	if (!pThisAdapter->shutdown)
	{
		// Test Falcon
		cardALDeviceTestFalcon ( pThisAdapter ) ;	// ***test

		// Initialize Pairgain Code
		DMT_PwrUpInit ( pDatapump ) ;

		#ifdef FALCON_ATM_MODE
		XCVR_SetupAtmBlock ( pDatapump, TRUE, 0, 0 ) ;
		#endif

		// Enable Interrupts
		ChipAlTigrisDpEnableIRQ1_2 ( ) ;
	}
}

/******************************************************************************
FUNCTION NAME:
	cardALDeviceBGThread

ABSTRACT:
	Function scheduled for the background processing.


RETURN:
	NA.


DETAILS:
******************************************************************************/
MODULAR int cardALDeviceBGThread
(
	IN CDSL_ADAPTER_T *pThisAdapter
)
{
	TIGRIS_CONTEXT_T *pContext;

	if (BG_THREAD_BOOST != 0)
	{
		/* set thread priority to real time */
		current->policy = SCHED_FIFO;
		current->rt_priority = BG_THREAD_BOOST;
	}

	VERIFY_ADAPTER(pThisAdapter);

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	VERIFY_TIGRIS(pContext);

	// get out if no context.  This may happen during shutdown.
	if (pContext != NULL)
	{
		wait_queue_head_t queue;
		DATAPUMP_CONTEXT *pDatapump;
		//long timeout;
		long timeout = BG_PAUSE_TIME;
		DEFINE_WAIT(wait);
		init_waitqueue_head(&queue);		

		pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump;
		VERIFY_DATAPUMP( pDatapump );

///		// poll from BG rather than SI since this routine shuts down interrupts
///		// when changing wiring or handshake and if BG was polling an interrupt
///		// bit at this point, it would poll forever. Calling this from BG
///		// guarantees BG is here rather than polling an interrupt bit.
///		Auto_AutoSensePoll( pDatapump );

		for (;;)
		{
		  
			// fall asleep...
			//interruptible_sleep_on_timeout(&queue, BG_THREAD_SLEEP_INTERVAL);
			prepare_to_wait(&queue, &wait, TASK_INTERRUPTIBLE);
			timeout = schedule_timeout(timeout);
			finish_wait(&queue, &wait);

			if (kthread_should_stop())
			{
				break;
			}

			if ( ! pThisAdapter->shutdown )
			{
				// this is normal work to do
				DMT_ProcCurStartUpState( pDatapump, 0 );
			}
		}
	}

	return 0;
}


/******************************************************************************
FUNCTION NAME:
	cardALDeviceSIThread

ABSTRACT:
	Background Task to drive Pairgain Soft Interrupt "task"


RETURN:
	NA.


DETAILS:
******************************************************************************/
MODULAR int cardALDeviceSIThread
(
	CDSL_ADAPTER_T *pThisAdapter
)
{
	TIGRIS_CONTEXT_T *pContext;

	if (SI_THREAD_BOOST != 0)
	{
		/* set thread priority to real time */
		current->policy = SCHED_FIFO;
		current->rt_priority = SI_THREAD_BOOST;
	}

	VERIFY_ADAPTER(pThisAdapter);

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle( pThisAdapter, CARDAL_TIGRIS_CONTROLLER );
	VERIFY_TIGRIS(pContext);

	// get out if no context.  This normally happens during shutdown.
	if ( pContext != NULL )
	{
		wait_queue_head_t queue;
		DATAPUMP_CONTEXT *pDatapump;
		//long timeout;
		long timeout = SI_PAUSE_TIME;
		DEFINE_WAIT(wait);

		init_waitqueue_head(&queue);

		pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump;
		VERIFY_DATAPUMP( pDatapump );

		for (;;)
		{
			// fall asleep...
			//interruptible_sleep_on_timeout(&queue, SI_THREAD_SLEEP_INTERVAL);
			prepare_to_wait(&queue, &wait, TASK_INTERRUPTIBLE);
			timeout = schedule_timeout(timeout);
			finish_wait(&queue, &wait);			

			// here we are back from sleep, either due to the timeout, or because we caught a signal.
			if (kthread_should_stop())
			{
				// we received a request to terminate ourself
				break;
			}

			if ( ! pThisAdapter->shutdown )
			{
				// this is normal work to do

//				// Wait on software Trap, process trap if not timeout,
//				// timeout is set to wake up every 100 ms to check different flags
//				if ( WAIT_EVENT( &pContext->SITrapEvent, Si_Pause_Time ) ) 
//				{
//					// Reset event for next wait
//					RESET_EVENT( &pContext->SITrapEvent ) ;
//				}

//				// Trap handlers
				EOC_ProcessRxcMsg( pDatapump );

				Auto_AutoSensePoll( pDatapump );
			}
		}
	}

	return 0;
}


/******************************************************************************
FUNCTION NAME:
	cardALDeviceTmrThread

ABSTRACT:
	Timer Task to drive Pairgain Soft Low res"


RETURN:
	NA.


DETAILS:
******************************************************************************/
MODULAR int cardALDeviceTmrThread
(
	CDSL_ADAPTER_T *pThisAdapter
)
{
	TIGRIS_CONTEXT_T *pContext;
	ULONG CurrentTime;

	if (TM_THREAD_BOOST != 0)
	{
		/* set thread priority to real time */
		current->policy = SCHED_FIFO;
		current->rt_priority = TM_THREAD_BOOST;
	}

	VERIFY_ADAPTER(pThisAdapter);

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle( pThisAdapter, CARDAL_TIGRIS_CONTROLLER );
	VERIFY_TIGRIS(pContext);

	// get out if no context.  This normally happens during shutdown.
	if ( pContext != NULL )
	{
		wait_queue_head_t queue;
		DATAPUMP_CONTEXT *pDatapump;
		//long timeout;
		long timeout = RT_PAUSE_TIME;
		DEFINE_WAIT(wait);

		init_waitqueue_head(&queue);

		pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump;
		VERIFY_DATAPUMP( pDatapump );

		for (;;)
		{
			// fall asleep...
			//interruptible_sleep_on_timeout(&queue, TM_THREAD_SLEEP_INTERVAL);
			prepare_to_wait(&queue, &wait, TASK_INTERRUPTIBLE);
			timeout = schedule_timeout(timeout);
			finish_wait(&queue, &wait);

			// here we are back from sleep, either due to the timeout, or because we caught a signal.
			if (kthread_should_stop())
			{
				// we received a request to terminate ourself
				break;
			}

			if ( ! pThisAdapter->shutdown )
			{
				// this is normal work to do
				#ifdef VCOMM
				SER_ProcessRxcMsg( pDatapump );
				#endif

				// Call low resolution timer if enough time has passed
				// (Don't depend on resolution of this task or the timer handler - there
				// could be accumulated roundoff errors)
				CurrentTime = CardAlTigrisGetCurrentTick( pThisAdapter );

				if ( (CurrentTime - pContext->LastLoResCallTime) >= DELTA_CALL )
				{
					pContext->LastLoResCallTime += DELTA_CALL;
					TM_LoResTimerIntHandler( pDatapump );
				}
			}
		}
	}

	return 0;
}

/******************************************************************************
FUNCTION NAME:
	cardALDeviceIRQ1IsrHdlr

ABSTRACT:
	(DSL_ISR:FALIRQ1)
	ISR called function (from ChipAL module) for reading the ChipAL Falcon
	IRQ1.

RETURN:

DETAILS:
******************************************************************************/
void cardALDeviceIRQ1IsrHdlr
(
    IN void						* pAdapter,
    IN RESPONSE_T				* Data
)
{
	CDSL_ADAPTER_T				* pThisAdapter=(CDSL_ADAPTER_T*)pAdapter;
	TIGRIS_CONTEXT_T			* pContext ;
	DATAPUMP_CONTEXT			* pDatapump ;

	NOT_USED(Data);

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_TIGRIS (pContext) ;
	VERIFY_DATAPUMP (pDatapump) ;

	// Disable FIRQ1 and FIRQ2 until DPC has run
	// (Since this is already at interrupt level (DIRQL), it can call the
	// low level routine that involves a read/modify/write operation without
	// synchronizing with an interrupt i.e. it doesn't have to go through
	// ChipAlTigrisDpDisableIRQ1_2 like a background routine)
//	ChipAlTigrisHalDisableIRQ1_2_ISR ( pThisAdapter ) ;
	
	IRQ1_ISR ( pDatapump ) ;
}



/******************************************************************************
FUNCTION NAME:
	cardALDeviceIRQ2IsrHdlr

ABSTRACT:
	(DSL_ISR:FALIRQ2)
	ISR called function (from ChipAL module) for reading the ChipAL Falcon
	IRQ2.

RETURN:

DETAILS:
******************************************************************************/
void cardALDeviceIRQ2IsrHdlr
(
    IN void						* pAdapter,
    IN RESPONSE_T				* Data
)
{
	CDSL_ADAPTER_T				* pThisAdapter=(CDSL_ADAPTER_T*)pAdapter;
	TIGRIS_CONTEXT_T			* pContext ;
	DATAPUMP_CONTEXT			* pDatapump ;

	NOT_USED(Data);

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_TIGRIS (pContext) ;
	VERIFY_DATAPUMP (pDatapump) ;

	// Disable FIRQ1 and FIRQ2 until DPC has run
	// (Since this is already at interrupt level (DIRQL), it can call the
	// low level routine that involves a read/modify/write operation without
	// synchronizing with an interrupt i.e. it doesn't have to go through
	// ChipAlTigrisDpDisableIRQ1_2 like a background routine)
//	ChipAlTigrisHalDisableIRQ1_2_ISR ( pThisAdapter ) ;
	
	IRQ2_ISR ( pDatapump ) ;
}


/******************************************************************************
FUNCTION NAME:
	cardALDeviceIRQ2DpcHdlr

ABSTRACT:
	(DSL_ISR:FALIRQ2)
	DPC called function (from ChipAL module) for reading the ChipAL Falcon
	IRQ2.

RETURN:

DETAILS:
******************************************************************************/
//void cardALDeviceIRQ2DpcHdlr
//(
//    IN void				* pAdapter,
//    IN RESPONSE_T		* Data
//)
//{
//	CDSL_ADAPTER_T			* pThisAdapter=(CDSL_ADAPTER_T*)pAdapter;
//	TIGRIS_CONTEXT_T		* pContext ;
//	DATAPUMP_CONTEXT		* pDatapump ;
//
//	NOT_USED(Data);
//	printk("<1> \n\nError should not get here\n\n");
//
//	//pThisInterface = (CARDAL_ADAPTER_T *) pThisAdapter->CardMod ;
//	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
//	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;
//
//	VERIFY_ADAPTER (pThisAdapter) ;
//	VERIFY_TIGRIS (pContext) ;
//	VERIFY_DATAPUMP (pDatapump) ;
//
//	// Enable FIRQ1 and FIRQ2
//	ChipAlTigrisHalEnableIRQ1_2 ( pThisAdapter ) ;
//
//	// Call Pairgain irq handler
//	IRQ2_ISR ( pDatapump ) ;
//}


/******************************************************************************
FUNCTION NAME:
	cardALDeviceTimerExpiredDpcHdlr

ABSTRACT:
	(DSL_ISR:FALIRQ1)
	DPC called function (from ChipAL module) for reading the ChipAL Falcon
	IRQ1.

RETURN:

DETAILS:
******************************************************************************/

#if USE_DEVICE_TIMER_FOR_DELAY
/*
void cardALDeviceTimerExpiredDpcHdlr
(
    IN void						* pAdapter,
    IN RESPONSE_T				* Data
)
{
	CDSL_ADAPTER_T				* pThisAdapter=(CDSL_ADAPTER_T*)pAdapter;
	TIGRIS_CONTEXT_T			* pContext ;
	int							  TimerID ;

	TimerID = Data->Param[0] ;

	//pThisInterface = (CARDAL_ADAPTER_T *) pThisAdapter->CardMod ;
	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	if(pContext != NULL)
	{
		SET_EVENT ( &pContext->TimerExpiredEvent [TimerID] ) ;
	}
}
*/

void cardALDeviceTimer0Isr
(
    IN void						* pAdapter,
    IN RESPONSE_T				* Data
)
{
	CDSL_ADAPTER_T				* pThisAdapter=(CDSL_ADAPTER_T*)pAdapter;
	TIGRIS_CONTEXT_T			* pContext ;

	//pThisInterface = (CARDAL_ADAPTER_T *) pThisAdapter->CardMod ;
	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	if(pContext != NULL)
	{
		SET_EVENT_FROM_ISR ( &pContext->TimerExpiredEvent [0] ) ;
	}
}

void cardALDeviceTimer1Isr
(
    IN void						* pAdapter,
    IN RESPONSE_T				* Data
)
{
	CDSL_ADAPTER_T				* pThisAdapter=(CDSL_ADAPTER_T*)pAdapter;
	TIGRIS_CONTEXT_T			* pContext ;

	//pThisInterface = (CARDAL_ADAPTER_T *) pThisAdapter->CardMod ;
	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	if(pContext != NULL)
	{
		SET_EVENT_FROM_ISR ( &pContext->TimerExpiredEvent [1] ) ;
	}
}
#endif


///////////////////////////////////////////////////////////////////////////////
//	Pairgain Data Pump Support
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
FUNCTION NAME:
	CardAlTigrisGetCurrentTick

ABSTRACT:
	Return Timer's Tick Count


RETURN:
	Timer's Tick Count.

******************************************************************************/
TICK_TYPE CardAlTigrisGetCurrentTick
(
    IN CDSL_ADAPTER_T			* pThisAdapter
)
{
	ULONG						  Current_Time ;
	struct timeval				ctime;

	// get the current time
	do_gettimeofday( &ctime );

	// convert the current time from microseconds to milliseconds
	Current_Time = ctime.tv_sec * 1000 + ctime.tv_usec/1000;

	return (TICK_TYPE) Current_Time ;
}


///////////////////////////////////////////////////////////////////////////////
//	CardMgmt/CardALMgmt Support - Device specifics
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
FUNCTION NAME:
	cardALBdDebug

ABSTRACT:
	To be used for quick and dirty communication between control panel and the
	tranceiver core. Using the custom driver commands of the control panel
	and using the already defined BD_ADSL_DEBUG code, allows us to add sub-codes
	on the fly by only adding code to this subroutine.

RETURN:
	NTSTATUS


DETAILS:
	On the control panel, use [ALT A] to enable the full set of tabs
	Click the "Custom Driver Commands"
	Enter 60012 for the opcode
	Enter desired sub-code(s) in the data field.
	Add case(s) to this subroutine to handle your sub-code(s)

******************************************************************************/

typedef struct
{
	WORD	Buffer[10] ;
} BD_ADSL_DEBUG_T ;

LOCAL NTSTATUS cardALBdDebug
(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					  Length
)
{
	CARDAL_ADAPTER_T			* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					Status = STATUS_SUCCESS ;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter ;
	TIGRIS_CONTEXT_T			* pContext ;
	BD_ADSL_DEBUG_T				* pBdAdslDebug ;


	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle ( pThisAdapter, CARDAL_TIGRIS_CONTROLLER ) ;

	if (Buffer != NULL)
	{
		pBdAdslDebug = (BD_ADSL_DEBUG_T *) Buffer ;

		switch ( pBdAdslDebug->Buffer[0] )
		{
		default:
			ErrPrt ( "Illegal BD_ADSL_DEBUG sub-code received: %x\n", pBdAdslDebug->Buffer[0] ) ;
		}
	}

	return Status;
}


/******************************************************************************
FUNCTION NAME:
	cardALSetTrace

ABSTRACT:
	Sets the tranceiver core trace flags to disable/enable the traces.
	The message consists of a one WORD value that is bit mapped to control all traces

RETURN:
	NTSTATUS


DETAILS:
	On the control panel, use [ALT A] to enable the full set of tabs
	Click the "Custom Driver Commands"
	Enter 60011 for the opcode
	Enter desired bit mapped value in the data field to enable/disable tracing:
		AOC				1
		EOC				2
		DIAGNOS_LOG		4
		AFE_WRITE		8
		DIAGNOS_WRITE	10

******************************************************************************/

typedef enum
{
    BD_ADSL_AOC_TRACE			= 0x1,
    BD_ADSL_EOC_TRACE			= 0x2,
    BD_ADSL_DIAGNOS_LOG_TRACE	= 0x4,
    BD_ADSL_AFE_WRITE_TRACE		= 0x8,
	BD_ADSL_AFE_WRITE_PRINT		= 0x10,
    BD_ADSL_UNUSED_TRACE		= 0xFFE0
} BD_ADSL_TRACE_T ;
typedef struct
{
	BD_ADSL_TRACE_T	Trace_Type ;
} BD_ADSL_SET_TRACE_T ;

LOCAL NTSTATUS cardALSetTrace
(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					  Length
)
{
	CARDAL_ADAPTER_T		* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;

	NTSTATUS 					Status = STATUS_SUCCESS ;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter ;
	TIGRIS_CONTEXT_T			* pContext ;
	DATAPUMP_CONTEXT			* pDatapump ;
	BD_ADSL_SET_TRACE_T			* pBdAdslSetTrace ;


	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle ( pThisAdapter, CARDAL_TIGRIS_CONTROLLER ) ;
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_TIGRIS (pContext) ;
	VERIFY_DATAPUMP (pDatapump) ;

	if (Buffer != NULL)
	{

		pBdAdslSetTrace = (BD_ADSL_SET_TRACE_T *) Buffer ;

		#if AOC_TRACE
		pDatapump->Aoc_Trace = (pBdAdslSetTrace->Trace_Type & BD_ADSL_AOC_TRACE) ? 1 : 0 ;
		#endif

		#if EOC_TRACE
		pDatapump->Eoc_Trace = (pBdAdslSetTrace->Trace_Type & BD_ADSL_EOC_TRACE) ? 1 : 0 ;
		#endif

		#if DIAGNOS_LOG_TRACE
		pDatapump->Diagnos_Log_Trace = (pBdAdslSetTrace->Trace_Type & BD_ADSL_DIAGNOS_LOG_TRACE) ? 1 : 0 ;
		#endif

		#if AFE_WRITE_TRACE
		pDatapump->Afe_Write_Trace = (pBdAdslSetTrace->Trace_Type & BD_ADSL_AFE_WRITE_TRACE) ? 1 : 0 ;
		#endif

		ChipAlTigrisHalSetAfePrint ( pThisAdapter, (pBdAdslSetTrace->Trace_Type & BD_ADSL_AFE_WRITE_PRINT) ? 1 : 0 ) ;
		

		if ( pBdAdslSetTrace->Trace_Type & BD_ADSL_UNUSED_TRACE )
		{
			ErrPrt ( "Illegal BD_ADSL_SET_TRACE type received: %x\n", pBdAdslSetTrace->Trace_Type ) ;
		}
	}

	return Status;
}


/******************************************************************************
FUNCTION NAME:
	CardALTigrisSITrap

ABSTRACT:
  	Notifies us that it's ready to process software trap

RETURN:
	None


DETAILS:
******************************************************************************/
void CardALTigrisSITrap 
(
    IN CDSL_ADAPTER_T			* pThisAdapter
)
{
    TIGRIS_CONTEXT_T			* pContext ;

    //pThisInterface = (CARDAL_ADAPTER_T *) pThisAdapter->CardMod ;
    pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

    SET_EVENT ( &pContext->SITrapEvent ) ;
 }	



/******************************************************************************
FUNCTION NAME:
	CardALDeviceAdapterInit

ABSTRACT:
	Allocate Necessary Resources


RETURN:
	Context Block for This instance or NULL if failure.

******************************************************************************/
MODULAR VOID * CardALDeviceAdapterInit
(
    IN void		* pAdapterContext,
    IN VOID		* pCardAlContext
)
{
	// Use temporary global - This will initialize this variable
	// This variable is global and used in ChipAlTigrisDp cause it is called by
	// Pairgain Data pump code which does not currently use pThisAdapter and
	// therefore does not pass it the ChipAlTigrisDP.

	TIGRIS_CONTEXT_T			* pContext ;
	DATAPUMP_CONTEXT			* pDatapump ;
	NDIS_STATUS					  Status;

	DBG_CDSL_DISPLAY
	(
	    DBG_LVL_MISC_INFO,
	    ((CDSL_ADAPTER_T * ) pAdapterContext)->DebugFlag,
	    ("CardALDeviceAdapterInit:  Entry;")
	) ;

	// *
	// * Allocate and clear Context Structue
	// *
	Status =	ALLOCATE_MEMORY
	         (
	             (PVOID)&pContext,
	             sizeof(TIGRIS_CONTEXT_T),
	             MEMORY_TAG
	         );

	if (Status != STATUS_SUCCESS)
	{
		return NULL ;
	}
	CLEAR_MEMORY(pContext, sizeof(TIGRIS_CONTEXT_T));

	// *
	// * Allocate and clear Datapump Context Structue
	// *
	Status =	ALLOCATE_MEMORY
	         (
	             &pContext->pDatapump,
	             sizeof(DATAPUMP_CONTEXT),
	             MEMORY_TAG
	         );

	if (Status != STATUS_SUCCESS)
	{
		return NULL ;
	}
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	CLEAR_MEMORY ( pContext->pDatapump, sizeof(DATAPUMP_CONTEXT) );

	InitDatapumpStructure( pDatapump, pContext);

	#if VERIFY_CONTEXT
		pAdapterStatic = pAdapterContext ;
		cardALChkAdptrStrucsOk( pAdapterContext, (CARDAL_ADAPTER_T **) &pCardAlStatic ) ;
		pTigrisStatic = pContext ;
		pDatapumpStatic = pDatapump ;
	#endif

	// * Save Global Handle for this instance of the Adapter
	pContext->pThisAdapter = (CDSL_ADAPTER_T *) pAdapterContext;

	// * Save Handle to parent module.
	pContext->pCardAl = pCardAlContext;

	// Initialize BG disable flag 
	pContext->BGThreadDisableFlag = TRUE ;

	// Initialize SI disable flag 
	pContext->SIThreadDisableFlag = TRUE ;

	// Initialize Tmr disable flag 
	pContext->TmrThreadDisableFlag = TRUE ;


	// *
	// * Initialize low resolution elapsed time counts
	// *
	pContext->LastLoResCallTime =  CardAlTigrisGetCurrentTick( pAdapterContext );

	// *
	// * initialize thread sync events
	// *
	INIT_EVENT ( &pContext->BGThreadDeviceRdyEvent ) ;

	// *
	// * initialize SI Trap
	// *
	INIT_EVENT ( &pContext->SITrapEvent ) ;

	// *
	// * initialize qedfer flag/event
	// *
	INIT_EVENT ( &pContext->Qedfer_Event ) ;
	*(UINT16 *)&pContext->Qedfer_Wait_Flag = 0 ;

	// *
	// * Hook in ISRs and DPCs
	// *

	// Hook in ISR for FIRQ1
	Status = ChipALAddEventHandler
	         (
	             pContext->pThisAdapter,
	             DSL_FUNCTION,
	             HW_FAL_IRQ_1,
	             HW_ISR_EVENT,
	             cardALDeviceIRQ1IsrHdlr,
	             pContext->pThisAdapter
	         );

	// Hook in ISR for FIRQ2
	Status |= ChipALAddEventHandler
	          (
	              pContext->pThisAdapter,
	              DSL_FUNCTION,
	              HW_FAL_IRQ_2,
	              HW_ISR_EVENT,
	              cardALDeviceIRQ2IsrHdlr,
	              pContext->pThisAdapter
	          );

	if ( Status != STATUS_SUCCESS )
	{
		NdisWriteErrorLogEntry
		(
		    pContext->pThisAdapter->MiniportAdapterHandle,
		    NDIS_ERROR_CODE_DRIVER_FAILURE,
		    4,
		    Status,
		    __FILEID__,
		    __LINE__,
		    RSS_INIT_ERROR_CARDAL_ADAPTER_INIT_FAILURE
		);

		DBG_CDSL_DISPLAY(
		    DBG_LVL_ERROR,
		    pContext->pThisAdapter->DebugFlag,
		    ("CardALDeviceAdapterInit: ERROR;  Could NOT Add ISR to ChipAL module!\n") );

		FREE_MEMORY( pContext, sizeof(TIGRIS_CONTEXT_T), 0 );

		return NULL ;
	}

	// *
	// * All is good, now do soft bindings to external interfaces
	// *
	Status = SmSysIfGetHandle(pContext->pThisAdapter, &pContext->SysIfHandle);

	if(Status != STATUS_SUCCESS)
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_GENERIC,
		    pContext->pThisAdapter->DebugFlag,
		    ("CardALDeviceAdapterInit: Could not get SystemIf Handler for ThisAdapter: (%8lx) \n",Status));
	}
	else
	{

		// * Add Comm Port Rx Handler (From Protocol External Protocol Stack, not the device
		MESSAGE_HANDLER_T		DbgHandlerTable[] =
		    {
		        /*	Message			Key		Len		Handler					Module Id  */
		#ifdef VCOMM
		        {MESS_COMM0_OPEN,	0,		0l,		cardALOpenDumbTerminal,	IF_CARDAL_ID},
		        {MESS_COMM0_CLOSE,	0,		0l,		cardALCloseDumbTerminal,IF_CARDAL_ID},
		        {MESS_COMM0_DATA,	0,		0l,		cardAlRxComm,			IF_CARDAL_ID},
		        {MESS_COMM2_OPEN,	0,		0l,		cardALOpenDebugTerminal,IF_CARDAL_ID},
		        {MESS_COMM2_DATA,	0,		0l,		cardALOpenDebugEcho,	IF_CARDAL_ID},
		#endif	//VCOMM

		        {MESS_API_COMMAND,  BD_ADSL_SET_TRACE,	sizeof(BD_ADSL_SET_TRACE_T),	cardALSetTrace,		IF_CARDAL_ID},
		        {MESS_API_COMMAND,  BD_ADSL_DEBUG,		sizeof(BD_ADSL_DEBUG_T),		cardALBdDebug,		IF_CARDAL_ID},


		    };

		// * Pass Function Table to System Interface
		Status = SmSysIfAddMessageTable(pContext->SysIfHandle,
		                                DbgHandlerTable,
		                                pContext,
		                                sizeof(DbgHandlerTable) / sizeof(MESSAGE_HANDLER_T));

		if(Status != STATUS_SUCCESS)
		{
			DBG_CDSL_DISPLAY(
			    DBG_LVL_GENERIC,
			    pContext->pThisAdapter->DebugFlag,
			    ("CardALDeviceAdapterInit: SmSysIfAddMessageHandler Failed (%8lx) \n",Status));
		}
	}

	#ifdef VCOMM
	SER_InitMsgQ ( pDatapump ) ;
	TRM_TerminalInit ( pDatapump ) ;
	SER_ClearRemoteLogon ( pDatapump ) ;         // logoff remote if logged in
	#endif

	#if USE_DEVICE_TIMER_FOR_DELAY
	// Initialize Device Timer lock
	INIT_SPIN_LOCK ( &pContext->TimerStackLock ) ;

	// initialzie device timer event, allocation stack and stack ptr
	for (pContext->TimerStackPtr = 0 ;
	        pContext->TimerStackPtr < NUM_VIRTUAL_DEVICE_TIMERS ;
	        pContext->TimerStackPtr ++ )
	{
		pContext->TimerStackArray[pContext->TimerStackPtr] = NUM_VIRTUAL_DEVICE_TIMERS - pContext->TimerStackPtr -1 ;
		INIT_EVENT ( &pContext->TimerExpiredEvent [pContext->TimerStackPtr] ) ;
	}

	// Hook in handler for timer expired device response
	Status = ChipALAddEventHandler
	         (
	             pContext->pThisAdapter,
	             DSL_FUNCTION,
	             HW_TIMER_0,
	             HW_ISR_EVENT,
	             cardALDeviceTimer0Isr,
	             pContext->pThisAdapter
	         );

	// Hook in handler for timer expired device response
	Status = ChipALAddEventHandler
	         (
	             pContext->pThisAdapter,
	             DSL_FUNCTION,
	             HW_TIMER_1,
	             HW_ISR_EVENT,
	             cardALDeviceTimer1Isr,
	             pContext->pThisAdapter
	         );

	#endif

	return pContext ;
}



/******************************************************************************
FUNCTION NAME:
	CardALDeviceAdapterHalt

ABSTRACT:
	Release All Resources


RETURN:
	None


DETAILS:
******************************************************************************/
MODULAR void CardALDeviceAdapterHalt
(
    IN void			* pCon
)
{
	TIGRIS_CONTEXT_T			* pContext=(TIGRIS_CONTEXT_T*)pCon;
	DBG_CDSL_DISPLAY
	(
	    DBG_LVL_MISC_INFO,
	    pContext->pThisAdapter->DebugFlag,
	    ("CardALDeviceAdapterHalt:  Entry;")
	) ;

	// AdapterInit must be followd by paired Enable/Disable calls leaving the threads disabled
	if ( ! pContext->BGThreadDisableFlag || ! pContext->SIThreadDisableFlag || ! pContext->TmrThreadDisableFlag )
	{
		DBG_CDSL_DISPLAY
		(
			DBG_LVL_ERROR,
			pContext->pThisAdapter->DebugFlag,
			("CardALDeviceAdapterHalt:  Device halt while device is still enabled!\n")
		) ;

		// Disable it now and hope for the best
		CardALDeviceDisable ( pContext ) ;
	}


	// *
	// * Remove the ISRs and DPCs
	// *
	ChipALRemoveEventHandler
	(
	    pContext->pThisAdapter,
	    DSL_FUNCTION,
	    HW_FAL_IRQ_1,
	    HW_ISR_EVENT,
	    cardALDeviceIRQ1IsrHdlr
	) ;

	ChipALRemoveEventHandler
	(
	    pContext->pThisAdapter,
	    DSL_FUNCTION,
	    HW_FAL_IRQ_2,
	    HW_ISR_EVENT,
	    cardALDeviceIRQ2IsrHdlr
	) ;

#if USE_DEVICE_TIMER_FOR_DELAY
	ChipALRemoveEventHandler
	(
	    pContext->pThisAdapter,
	    DSL_FUNCTION,
	    HW_TIMER_0,
	    HW_ISR_EVENT,
	    cardALDeviceTimer0Isr
	);

	ChipALRemoveEventHandler
	(
	    pContext->pThisAdapter,
	    DSL_FUNCTION,
	    HW_TIMER_1,
	    HW_ISR_EVENT,
	    cardALDeviceTimer1Isr
	);

	FREE_SPIN_LOCK ( &pContext->TimerStackLock ) ;
#endif

	// *
	// * Release Context Structure
	// *
	FREE_MEMORY( pContext->pDatapump, sizeof(DATAPUMP_CONTEXT), 0 );

	FREE_MEMORY( pContext, sizeof(TIGRIS_CONTEXT_T), 0 );
}

/******************************************************************************
FUNCTION NAME:
	CardALDeviceEnable

ABSTRACT:
	Enable Fully Configured Device


RETURN:
	STATUS_SUCCESS	- All Ok
	STATUS_FAILURE	- Null context parameter was received.


DETAILS:
******************************************************************************/
MODULAR NTSTATUS CardALDeviceEnable
(
    IN void		* pCon
)
{
	TIGRIS_CONTEXT_T		* pContext=(TIGRIS_CONTEXT_T*)pCon;

	if(pContext == NULL)
	{
		return(STATUS_FAILURE);
	}

	DBG_CDSL_DISPLAY(
	    DBG_LVL_MISC_INFO,
	    pContext->pThisAdapter->DebugFlag,
	    ("CardALDeviceEnable:") );

	VERIFY_TIGRIS (pContext) ;
	VERIFY_ADAPTER (pContext->pThisAdapter) ;

	// Note threads enabled
	pContext->BGThreadDisableFlag = FALSE ;
	pContext->SIThreadDisableFlag = FALSE ;
	pContext->TmrThreadDisableFlag = FALSE ;

	cardALStartDevice( pContext->pThisAdapter ) ;

	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	CardALDeviceDisable

ABSTRACT:
	Enable Fully Configured Device


RETURN:
	STATUS_SUCCESS	- All Ok
	STATUS_FAILURE	- Null context parameter was received or Timed out.


DETAILS:
******************************************************************************/
MODULAR NTSTATUS CardALDeviceDisable
(
    IN void		* pCon
)
{
	TIGRIS_CONTEXT_T		* pContext = (TIGRIS_CONTEXT_T*)pCon ;
	CARDAL_ADAPTER_T		* pCardALAdapter ;

	if ( pContext == NULL )
	{
		return STATUS_FAILURE ;
	}

	DBG_CDSL_DISPLAY
	(
		DBG_LVL_MISC_INFO,
		pContext->pThisAdapter->DebugFlag,
		("CardALDeviceDisable:")
	);

	if ( cardALChkAdptrStrucsOk( pContext->pThisAdapter, &pCardALAdapter ) != RESULT_SUCCESS )
	{
		DBG_CDSL_DISPLAY
		(
			DBG_LVL_WARNING,
			pContext->pThisAdapter->DebugFlag,
			("CardALChipUnknownIntrptEventHdlr:  Null CardAL Adapter or Adapter Pointer.")
		);
		return STATUS_FAILURE;
	}

	VERIFY_ADAPTER (pContext->pThisAdapter) ;
	VERIFY_CARDAL (pCardALAdapter) ;
	VERIFY_TIGRIS (pContext) ;

	// Shutdown Line
	cardALChipShutdownAdslLine ( pContext->pThisAdapter, pCardALAdapter ) ;

	// let threads run and process this before we return and threads then quit
	// running and nothing will be processed then

	// Using SLEEP is a hack. We should wait on events and have each thread
	// set the events after running one time.
	SLEEP ( 500 ) ;

	ChipAlTigrisDpDisableIRQ1_2 ( ) ;


	DBG_CDSL_DISPLAY
	(
		DBG_LVL_MISC_INFO,
		pContext->pThisAdapter->DebugFlag,
		("CardALDeviceDisable: Exit")
	);


	pContext->BGThreadDisableFlag = TRUE ;
	pContext->SIThreadDisableFlag = TRUE ;
	pContext->TmrThreadDisableFlag = TRUE ;

	return STATUS_SUCCESS ;
}


///////////////////////////////////////////////////////////////////////////////
//	CardALMgmt Support - Device specifics
///////////////////////////////////////////////////////////////////////////////


/******************************************************************************
*	FUNCTION NAME:
*		CardALChipUnknownIntrptEventHdlr
*
*	ABSTRACT:
*		This is the external entry point for the lower level chip module
*		to call the CardAL module to handle reading of chip registers
*		for an chip module unknown/unhandled interrupt.
*		This Vector is setup by the call to ChipALAdapterInit
*
*	RETURN:
*
*	DETAILS:
*		Any values read are placed into the appropriate place in CardAL's
*		adapter structure.
******************************************************************************/
void CardALChipUnknownIntrptEventHdlr
(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    IN DWORD						  IntrptMaskBit
)
{
	CARDAL_ADAPTER_T				* pCardALAdapter;

	if ( cardALChkAdptrStrucsOk( pThisAdapter, &pCardALAdapter ) != RESULT_SUCCESS )
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_WARNING,
		    pThisAdapter->DebugFlag,
		    ("CardALChipUnknownIntrptEventHdlr:  Null CardAL Adapter or Adapter Pointer.") );
		return;
	}
}

/******************************************************************************
*	FUNCTION NAME:
*		CardALChipGetMacAddr
*
*	ABSTRACT:
*		Retrieves the MAC address (six bytes) stored in EEPROM on the PCI
*		adapter card.
*
*	RETURN:
*		BackDoor Result Code
*
*	DETAILS:
******************************************************************************/
GLOBAL DWORD CardALChipGetMacAddr
(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    OUT BYTE				* pBdMacAddr,
    IN DWORD				MacAddrLength )
{
	DWORD					RtnResultValue;
	CARDAL_ADAPTER_T		* pCardALAdapter;

	RtnResultValue = cardALChkAdptrStrucsOk( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		RtnResultValue = CardAlReadMacAddress(pCardALAdapter, pBdMacAddr, MacAddrLength);
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_MISC_INFO,
		    pThisAdapter->DebugFlag,
		    ("CardALChipGetMacAddr: MISC.;  NULL Structures!\n") );
		return RtnResultValue;
	}
	return RtnResultValue;
}

/******************************************************************************
*	FUNCTION NAME:
*		CardALChipSetMacAddr
*
*	ABSTRACT:
*		Programs/set the MAC address (six bytes) stored in EEPROM on the PCI
*		adapter card.
*
*	RETURN:
*		BackDoor Result Code
*
*	DETAILS:
******************************************************************************/
GLOBAL DWORD CardALChipSetMacAddr
(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN BYTE						* pBdMacAddr,
    IN DWORD					  MacAddrLength
)
{
	DWORD						  RtnResultValue;
	CARDAL_ADAPTER_T			* pCardALAdapter;

	RtnResultValue = cardALChkAdptrStrucsOk( pThisAdapter, &pCardALAdapter );
	if ( RtnResultValue == RESULT_SUCCESS )
	{

		RtnResultValue = CardAlWriteMacAddress(pCardALAdapter, pBdMacAddr, MacAddrLength);
	}
	else
	{
		DBG_CDSL_DISPLAY(
		    DBG_LVL_MISC_INFO,
		    pThisAdapter->DebugFlag,
		    ("CardALChipSetMacAddr: MISC.;  NULL Structures!\n") );
		return RtnResultValue;
	}
	return RtnResultValue;
}

/******************************************************************************
*	FUNCTION NAME:
*		cardALChipUpldCodeFromDP
*
*	ABSTRACT:
*		Function for uploading the Data Pump (DP) code from the DP to memory.
*
*	RETURN:
*
*	DETAILS:
******************************************************************************/
DWORD cardALChipUpldCodeFromDP(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN CARDAL_ADAPTER_T			* pCardALAdapter,
    IN DWORD					BlockStartAddress,
    I_O DWORD					* BlockLength,
    OUT BYTE					* BlockData )
{
	DWORD					RtnResultValue;

	DBG_CDSL_DISPLAY(
	    DBG_LVL_MISC_INFO,
	    pThisAdapter->DebugFlag,
	    ("cardALChipUpldCodeFromDP:  Entry.") );

	RtnResultValue = RESULT_SUCCESS;

	return RtnResultValue;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALChipStartAdslLine
*
*	ABSTRACT:
*		Function called to set the ADSL DP into normal power mode and then
*		to perform an 'Open Line' on the ADSL DP.
*
*	RETURN:
*		BackDoor Result Code
*
*	DETAILS:
******************************************************************************/
#include "xcvrinfo.h"

DWORD cardALChipStartAdslLine(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    CARDAL_ADAPTER_T		* pCardALAdapter )
{
	TIGRIS_CONTEXT_T			* pContext ;
	DATAPUMP_CONTEXT			* pDatapump ;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_CARDAL (pCardALAdapter) ;
	VERIFY_TIGRIS (pContext) ;
	VERIFY_DATAPUMP (pDatapump) ;

	Auto_StartAutoSense ( pDatapump ) ;

	return RESULT_SUCCESS;
}



/******************************************************************************
*	FUNCTION NAME:
*		cardALChipShutdownAdslLine
*
*	ABSTRACT:
*		Function called to perform a 'Close Line' on the ADSL DP and then.
*		to set the ADSL DP into low power mode.
*
*	RETURN:
*		BackDoor Result Code
*
*	DETAILS:
******************************************************************************/
//#include "qedfer.h"	  
DWORD cardALChipShutdownAdslLine(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    CARDAL_ADAPTER_T		* pCardALAdapter )
{
	DWORD					  RtnResultValue;
	TIGRIS_CONTEXT_T		* pContext ;
	DATAPUMP_CONTEXT		* pDatapump ;
	BYTE					  LineStatus ;


	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_CARDAL (pCardALAdapter) ;

	if ( ! pThisAdapter->shutdown )
	{
		if ( ( pThisAdapter != NULL ) && ( pCardALAdapter != NULL ) )
		{
			pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;
			VERIFY_TIGRIS (pContext) ;
			VERIFY_DATAPUMP (pDatapump) ;

			// *** should next line be here or not? *** TTD / TBD (to be done)
			Auto_StartAutoSense ( pDatapump ) ;

			// If the line is active, we must send a dying Gasp command.
			cardALChipGetLineStatus ( pThisAdapter, pCardALAdapter, &LineStatus ) ;
			switch ( LineStatus )
			{
				case BD_MODEM_TRANSCEIVER_TRAINING:
				case BD_MODEM_CHANNEL_ANALYSIS:
				case BD_MODEM_EXCHANGE:
				case BD_MODEM_ACTIVATED:

					// Send dying gasp command
					STDEOC_DyingGasp ( pDatapump ) ;

					// The dying gasp command may be queued by the controller
					// code, we must allow time for the command to be processed.
					SLEEP ( DYING_GASP_TIMEOUT ) ;
					
				default:
					break ;
			}
			Auto_StopAutoSense ( pDatapump ) ;
		}
	}

	RtnResultValue = RESULT_SUCCESS;
	return RtnResultValue;
}



/******************************************************************************
*	FUNCTION NAME:
*		cardALChipPeekReg
*
*	ABSTRACT:
*		Function called to peek (read) registers/memory/etc. from the ADSL DP.
*
*	RETURN:
*		BackDoor Result Code
*
*	DETAILS:
******************************************************************************/
DWORD cardALChipPeekReg(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    CARDAL_ADAPTER_T				* pCardALAdapter,
    I_O BD_CARDAL_REGISTER_T		* pBdCardALReg )
{
	DWORD					RtnResultValue;
	DWORD					RegDataSize;
	DWORD					RegAccessType;
	DWORD					RegLogicalDevice;
	BOOLEAN					RegChkDataSizeAccessTypeOkay;
	REG_IO_EXT_T			RegIoExt = {{0}};

	cardALChipCnvrtBdRegDataSize( pBdCardALReg->RegDataSize, &RegDataSize );
	cardALChipCnvrtBdRegAccessType( pBdCardALReg->RegAccess, &RegAccessType, &RegLogicalDevice );
	RegChkDataSizeAccessTypeOkay = cardALChipCheckRegSizeAccessType(
	                                   RegDataSize,
	                                   RegAccessType );
	if ( RegChkDataSizeAccessTypeOkay )
	{
		switch (RegAccessType)
		{
		case MEMORY_MAPPED:
			RtnResultValue = CHIPAL_FAILURE;
			break;

			RtnResultValue = ChipALRead(
			                     pThisAdapter,
			                     MEMORY_MAPPED,
			                     RegLogicalDevice,
			                     pBdCardALReg->RegOffset,
			                     RegDataSize,
			                     &pBdCardALReg->RegValue );
			break;

		case BD_ADSL_REG_ACCESS_EEPROM:
			RegIoExt.SrcDestU.Destination = (BYTE*)&pBdCardALReg->RegValue;
			RegIoExt.IoType = REG_IO_READ;

			RtnResultValue = ChipALRead(
			                     pThisAdapter,
			                     EE_PROM,
			                     REMOTE_ADDRESS_BAR_2,
			                     pBdCardALReg->RegOffset,
			                     RegDataSize,
			                     (DWORD *)&RegIoExt
			                 );

			break;

		default:
			RtnResultValue = CHIPAL_FAILURE;
			break;
		}
		if ( RtnResultValue == CHIPAL_SUCCESS )
		{
			if ( pBdCardALReg->RegMask != BIT_MASK_NOT_USED )
			{
				pBdCardALReg->RegValue &= pBdCardALReg->RegMask;
			}
		}
	}
	else
	{
		RtnResultValue = CHIPAL_FAILURE;
	}
	cardALChipCnvrtAdslRegOpRslt( &RtnResultValue );

	return RtnResultValue;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALChipPokeReg
*
*	ABSTRACT:
*		Function called to poke (write) registers/memory/etc. to the ADSL DP.
*
*	RETURN:
*		BackDoor Result Code
*
*	DETAILS:
******************************************************************************/
DWORD cardALChipPokeReg(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    CARDAL_ADAPTER_T				* pCardALAdapter,
    I_O BD_CARDAL_REGISTER_T		* pBdCardALReg )
{
	DWORD					RtnResultValue;
	DWORD					RegDataSize;
	DWORD					RegAccessType;
	DWORD					RegLogicalDevice = 0; /* keep compiler happy */
	BOOLEAN					RegChkDataSizeAccessTypeOkay;
	REG_IO_EXT_T			RegIoExt = {{0}};

	cardALChipCnvrtBdRegDataSize( pBdCardALReg->RegDataSize, &RegDataSize );
	cardALChipCnvrtBdRegAccessType( pBdCardALReg->RegAccess, &RegAccessType, &RegLogicalDevice );
	RegChkDataSizeAccessTypeOkay = cardALChipCheckRegSizeAccessType(
	                                   RegDataSize,
	                                   RegAccessType );
	if ( RegChkDataSizeAccessTypeOkay )
	{
		switch (RegAccessType)
		{
		case MEMORY_MAPPED:
			RtnResultValue = ChipALWrite(
			                     pThisAdapter,
			                     MEMORY_MAPPED,
			                     RegLogicalDevice,
			                     pBdCardALReg->RegOffset,
			                     RegDataSize,
			                     pBdCardALReg->RegValue,
			                     pBdCardALReg->RegMask );
			break;

		case BD_ADSL_REG_ACCESS_EEPROM:
			RegIoExt.SrcDestU.Destination = (BYTE*)&pBdCardALReg->RegValue;
			RegIoExt.IoType = REG_IO_WRITE;

			RtnResultValue = ChipALWrite(
			                     pThisAdapter,
			                     EE_PROM,
			                     REMOTE_ADDRESS_BAR_2,
			                     pBdCardALReg->RegOffset,
			                     RegDataSize,
			                     (DWORD)&RegIoExt,
			                     pBdCardALReg->RegMask
			                 );

			break;

		default:
			RtnResultValue = CHIPAL_FAILURE;
			break;
		}
	}
	else
	{
		RtnResultValue = CHIPAL_FAILURE;
	}
	cardALChipCnvrtAdslRegOpRslt( &RtnResultValue );

	return RtnResultValue;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALChipGetLineStatus
*
*	ABSTRACT:
*		Function called to get the ADSL line status from the ADSL DP.
*
*	RETURN:
*		BackDoor Result Code
*
*	DETAILS:
******************************************************************************/
//#include "xcvrinfo.h"

DWORD cardALChipGetLineStatus(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    CARDAL_ADAPTER_T				* pCardALAdapter,
    OUT BYTE						* pLineStatus )
{
	DWORD					RtnResultValue;
	XCVR_MAIN_SUB_STATE_TYPE main_sub_state;
	TIGRIS_CONTEXT_T			* pContext ;
	DATAPUMP_CONTEXT			* pDatapump ;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_CARDAL (pCardALAdapter) ;
	VERIFY_TIGRIS (pContext) ;
	VERIFY_DATAPUMP (pDatapump) ;

	XCVR_GetXCVRState ( pDatapump, &main_sub_state, NEAR_END ) ;


	switch ( main_sub_state.main_st )
	{
		// Initialization
	case INIT:
	case DEBUG:
	case XCVR_TEST:
		// sub_state == SUB_INIT
		*pLineStatus = BD_MODEM_DOWN ;
		break ;

	case START_UP:
		switch ( main_sub_state.sub_st )
		{
			//Activate and Acknowledge cycle
		case SUB_ACK:
			*pLineStatus = BD_MODEM_ACTIVATION ;
			break ;

			//Training cycle
		case SUB_TRAIN:
			*pLineStatus = BD_MODEM_TRANSCEIVER_TRAINING ;
			break ;

			//Channel Analysys cyle
		case SUB_CHANA:
			*pLineStatus = BD_MODEM_CHANNEL_ANALYSIS ;
			break ;

			//Exchange cycle
		case SUB_EXCH:
			*pLineStatus = BD_MODEM_EXCHANGE ;
			break ;
		}
		break ;

		// Data mode (Active mode)
	case DATA:
		// sub_state == SUB_DATA
		*pLineStatus = BD_MODEM_ACTIVATED ;
		break ;

	case RT_FAST_RETRAIN:
		*pLineStatus = BD_MODEM_TRANSCEIVER_TRAINING ;
		break ;

	case CSTART:
	case WSTART:
		*pLineStatus = BD_MODEM_INITIALIZING ;
		break ;

	case XCVR_PWR_DWN:
		*pLineStatus = BD_MODEM_WAITING_INIT ;
		break ;

	default:
		*pLineStatus = UNKNOWN_8BIT_VALUE ;
		break ;
	}




	RtnResultValue = RESULT_SUCCESS;

	return RtnResultValue;
}






/******************************************************************************
*	FUNCTION NAME:
*		cardALChipGetLineSpeed
*
*	ABSTRACT:
*		Function called to get the ADSL line speed from the ADSL DP.
*
*	RETURN:
*		BackDoor Result Code
*
*	DETAILS:
******************************************************************************/
DWORD cardALChipGetLineSpeed(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    CARDAL_ADAPTER_T				* pCardALAdapter,
    OUT WORD						* pAdslLineSpeedUpOrFar,
    OUT WORD						* pAdslLineSpeedDownOrNear )
{
	DWORD					RtnResultValue;
	TIGRIS_CONTEXT_T			* pContext ;
	DATAPUMP_CONTEXT			* pDatapump ;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_CARDAL (pCardALAdapter) ;
	VERIFY_TIGRIS (pContext) ;
	VERIFY_DATAPUMP (pDatapump) ;

	//
	// Read Upstream Near and Far Rates, whichever is nonzero is the
	// Upstream Rate (Line Speed).
	//

	*pAdslLineSpeedUpOrFar = XCVR_GetDataRate ( pDatapump, UP_STREAM ) ;
	*pAdslLineSpeedDownOrNear = XCVR_GetDataRate ( pDatapump, DWN_STREAM ) ;

	RtnResultValue = RESULT_SUCCESS;

	return RtnResultValue;
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALChipGetDpVersions
*
*	ABSTRACT:
*		Function called to get the ADSL line status from the ADSL DP.
*
*	RETURN:
*		BackDoor Result Code
*
*	DETAILS:
******************************************************************************/
DWORD cardALChipGetDpVersions(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    CARDAL_ADAPTER_T				* pCardALAdapter,
    I_O BD_ADSL_DP_VERSIONS_T		* pBdAdslDpVersStruc )
{

	TIGRIS_CONTEXT_T			* pContext ;
	DATAPUMP_CONTEXT			* pDatapump ;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_CARDAL (pCardALAdapter) ;
	VERIFY_TIGRIS (pContext) ;
	VERIFY_DATAPUMP (pDatapump) ;

	XCVR_GetDatabaseInfo
	(
		pDatapump,
		(UINT16 *)&pBdAdslDpVersStruc->AdslVersionFar,
		(UINT16 *)&pBdAdslDpVersStruc->AdslVendorFar,
		(UINT16 *)&pBdAdslDpVersStruc->AdslVersionNear,
		(UINT16 *)&pBdAdslDpVersStruc->AdslVendorNear
	);


	return RESULT_SUCCESS;
}


/******************************************************************************
*	FUNCTION NAME:
*		cardALChipGetPmState
*
*	ABSTRACT:
*		Return the Current CardAl Power Management State
*
*	RETURN:
*		BackDoor Result Code
*
*	DETAILS:
******************************************************************************/
DWORD cardALChipGetPmState(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    OUT BD_PM_POWER_STATE_T	* pPMPowerState)
{
	CARDAL_ADAPTER_T		* pCardAdapter;

	// *
	// * Range Check
	// *
	pCardAdapter = (CARDAL_ADAPTER_T *) CDALGetModuleHandle(pThisAdapter, CDAL_LINE_CONTROLLER);
	if (pCardAdapter == NULL)
	{
		return ERROR_WARNING(RESULT_MODULE_UNINITIALIZED);
	}

	#ifdef DSI
	if (pCardAdapter->PMDeviceState <  PM_STATE_START
	        || pCardAdapter->PMDeviceState >=  PM_STATE_END)
	{
		return ERROR_WARNING(RESULT_DATA_FAILURE);
	}
	#endif

	*pPMPowerState = pCardAdapter->PMDeviceState;

	return(RESULT_SUCCESS);
}





/******************************************************************************
*	FUNCTION NAME:
*		cardALChipSetPmState
*
*	ABSTRACT:
*		Set New Power Management State
*
*	RETURN:
*		BackDoor Result Code
*
*	DETAILS:
*		It is not allowable to switch between low power states.  The only valid
*		switch is from D0 - Fully On to any Low power state or from a Low
*		power state to D0.
******************************************************************************/
DWORD cardALChipSetPmState(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN BD_PM_POWER_STATE_T	* pPMPowerState)
{
	// *
	// * Disable This function until the power management resources are defined.
	// * Remove #if 0 to enable.
	// *
	return(RESULT_SUCCESS);

#if 0
	CAL_MODULE_PARAMS_T 	ServiceCmd;
	CHIPAL_PM_STATE_T		NewState = CONVERT_TO_CHIP_PM_STATE(*pPMPowerState);
	CARDAL_ADAPTER_T		* pCardAdapter;
	CHIPAL_FAILCODE_T		ChipSuccess;

	// *
	// * Range Check
	// *
	pCardAdapter = (CARDAL_ADAPTER_T *) CDALGetModuleHandle(pThisAdapter, CDAL_LINE_CONTROLLER);
	if (pCardAdapter == NULL)
	{
		ERROR_WARNING(RESULT_MODULE_UNINITIALIZED);
	}
	if (NewState <  PM_STATE_START
	        || NewState >=  PM_STATE_END)
	{
		return ERROR_WARNING(RESULT_PARAM_OUT_OF_RANGE);
	}


	// *
	// * Validate New State.  Prevent switching between low power states
	// *
	switch (pCardAdapter->PMDeviceState)
	{
	case PM_STATE_D0:
		if (NewState == PM_STATE_D0)
		{
			return(ERROR_NORMAL(RESULT_PARAM_OUT_OF_RANGE));
		}
		UtilTmrClearTmr(
		    &(pThisAdapter->TimerBlock),
		    pThisAdapter,
		    thPersistentActivation );
		ChipALWrite(
		    pThisAdapter,
		    ADSL_ALCATEL,			//	hHwDevice
		    TX_COMMAND,				//	hRegAddress
		    sizeof( BYTE ),			//	RegisterLength
		    ADSL_SEND_DYING_GASP,	//	value to write
		    BIT_MASK_NOT_USED );

		pCardAdapter->PMDeviceState = NewState;
		ServiceCmd.PmSetState.PmState = NewState;

		ChipSuccess =  ChipALService (
		                   pThisAdapter,
		                   CAL_SERVICE_PM,
		                   SVC_PM_SET_STATE,
		                   &ServiceCmd);
		if (ChipSuccess != RESULT_SUCCESS)
		{
			ChipSuccess = RESULT_DEVICE_FAILURE;
			break;
		};

		ChipALDisableInterruptHandler(pThisAdapter);
		break;

	case PM_STATE_D1:
	case PM_STATE_D2:
	case PM_STATE_D3_HOT:
	case PM_STATE_D3_COLD:
		if (NewState != PM_STATE_D0)
		{
			return(ERROR_NORMAL(RESULT_PARAM_OUT_OF_RANGE));
		}
		ServiceCmd.PmSetState.PmState = NewState;
		ChipSuccess =  ChipALService (
		                   pThisAdapter,
		                   CAL_SERVICE_PM,
		                   SVC_PM_SET_STATE,
		                   &ServiceCmd);

		if (ChipSuccess != RESULT_SUCCESS)
		{
			ChipSuccess = RESULT_DEVICE_FAILURE;
			break;
		};
		pCardAdapter->PMDeviceState = NewState;

		if (ChipSuccess != RESULT_SUCCESS)
		{
			break;
		};

		ChipSuccess = cardALChipStartAdslLine(pThisAdapter,pCardAdapter);
		break;

	default:
		return ERROR_WARNING(RESULT_FAILURE);
		break;

	}

	return(ChipSuccess);
#endif	
}



/******************************************************************************
*	FUNCTION NAME:
*		cardALChipCnvrtAdslRegOpRslt
*
*	ABSTRACT:
*		Function for converting the ChipAL Alcatel register operation
*		result to the BackDoor Response Result Value (result of register
*		operation; failed, successful, etc.) Parameter.
*
*	RETURN:
*
*	DETAILS:
*		The parameter pointed to by the function's call parameter (the
*		ChipAL Alcatel ADSL register operation result value) will be set
*		to the converted value (the BackDoor register operation result value).
******************************************************************************/
static void cardALChipCnvrtAdslRegOpRslt(
    I_O DWORD		* pRegOpResultValue )
{
	switch (*pRegOpResultValue)
	{
	case CHIPAL_SUCCESS:
		*pRegOpResultValue = RESULT_SUCCESS;
		break;
	case CHIPAL_BUSY:
		*pRegOpResultValue = ERROR_NORMAL( RESULT_DEVICE_BUSY );
		break;
	case CHIPAL_FAILURE:
		*pRegOpResultValue = ERROR_NORMAL( RESULT_DEVICE_FAILURE );
		break;
	case CHIPAL_RESOURCE_ALLOCATION_FAILURE:
		*pRegOpResultValue = ERROR_FATAL( RESULT_RESOURCE_ALLOC_FAILURE );
		break;
	case CHIPAL_RESOURCE_CONFLICT:
		*pRegOpResultValue = ERROR_FATAL( RESULT_RESOURCE_CONFLICT );
		break;
	case CHIPAL_PARAM_OUT_OF_RANGE:
		*pRegOpResultValue = ERROR_WARNING( RESULT_PARAM_OUT_OF_RANGE );
		break;
	case CHIPAL_ADAPTER_NOT_FOUND:
		*pRegOpResultValue = ERROR_FATAL( RESULT_ADAPTER_NOT_FOUND );
		break;
	case CHIPAL_INVALID_PARAM:
		*pRegOpResultValue = ERROR_WARNING( RESULT_PARAM_INVALID );
		break;
	case CHIPAL_MODULE_NOT_INITIALIZED:
		*pRegOpResultValue = ERROR_WARNING( RESULT_MODULE_UNINITIALIZED );
		break;
	default:
		*pRegOpResultValue = ERROR_WARNING( RESULT_DEVICE_FAILURE );
		break;
	}
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALChipCnvrtBdRegSize
*
*	ABSTRACT:
*		Function for converting the BackDoor register data size enumeration
*		to the ChipAL Alcatel register data size specification (in bytes).
*
*	RETURN:
*
*	DETAILS:
*		The parameter pointed to by the function's call parameter (the
*		BackDoor register data size enumeration) will be set to the
*		converted value (the ChipAL Alcatel ADSL register data size in bytes).
******************************************************************************/
static void cardALChipCnvrtBdRegDataSize(
    IN DWORD		BdRegDataSize,
    I_O DWORD		* pAdslRegDataSize )
{
	switch (BdRegDataSize)
	{
	case BD_ADSL_REG_DATA_SIZE_DWORD_4:
		*pAdslRegDataSize = sizeof( DWORD );
		break;
	case BD_ADSL_REG_DATA_SIZE_WORD_2:
		*pAdslRegDataSize = sizeof( WORD );
		break;
	case BD_ADSL_REG_DATA_SIZE_BYTE_1:
		*pAdslRegDataSize = sizeof( BYTE );
		break;
	default:
		*pAdslRegDataSize = 0;
		break;
	}
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALChipCnvrtBdAccessType
*
*	ABSTRACT:
*		Function for converting the BackDoor register access type enumeration
*		(memory mapped, Alcatel, etc.) to the ChipAL Alcatel register access
*		type specification (in bytes).
*
*	RETURN:
*
*	DETAILS:
*		The parameter pointed to by the function's call parameter (the
*		BackDoor register access type enumeration) will be set to the
*		converted value (the ChipAL Alcatel ADSL register access type
*		enumeration).
******************************************************************************/
static void cardALChipCnvrtBdRegAccessType(
    IN DWORD		BdRegAccessType,
    I_O DWORD		* pAdslAccessType,
    I_O DWORD		* pRegLogicalDevice )
{
	switch (BdRegAccessType)
	{
	case BD_ADSL_REG_ACCESS_MICROIF:
		*pAdslAccessType = MEMORY_MAPPED;
		*pRegLogicalDevice = ADDRESS_BAR_1;
		break;
	case BD_ADSL_REG_ACCESS_MEMORY:
		*pAdslAccessType = MEMORY_MAPPED;
		*pRegLogicalDevice = ADDRESS_BAR_0;
		break;
	case BD_ADSL_REG_ACCESS_EEPROM:
		*pAdslAccessType = BD_ADSL_REG_ACCESS_EEPROM;
		break;
	default:
		*pAdslAccessType = MAX_HARDWARE_DEVICES;
		break;
	}
}




/******************************************************************************
*	FUNCTION NAME:
*		cardALChipCheckRegSizeAccessType
*
*	ABSTRACT:
*		Function for checking the ChipAL Alcatel register data size and
*		register access type to verify a valid register read or write will
*		be performed.
*
*	RETURN:
*		BOOLEAN
*			TRUE	- Register Data Size and Register Access Type check okay.
*			FALSE	- Register Data Size or Register Access Type is invalid.
*
*	DETAILS:
******************************************************************************/
static BOOLEAN cardALChipCheckRegSizeAccessType(
    IN DWORD		AdslDataSize,
    IN DWORD		AdslAccessType )
{
	switch (AdslAccessType)
	{
	case MEMORY_MAPPED:
		switch (AdslDataSize)
		{
		case sizeof( DWORD ):
					case sizeof( WORD ):
						case sizeof( BYTE ):
								return TRUE;
			break;
		default:
			return FALSE;
			break;
		}
		break;

	case BD_ADSL_REG_ACCESS_EEPROM:
		switch (AdslDataSize)
{
		case sizeof( DWORD ):
					case sizeof( WORD ):
						case sizeof( BYTE ):
								return TRUE;
			break;
		default:
			return FALSE;
			break;
		}
		break;

	default:
		return FALSE;
		break;
	}
}

/******************************************************************************
The following functions are external interfaces provided by Pairgain.  The
function headers are taken directly from the file(s):

	Serial.c

******************************************************************************/

#ifdef VCOMM
/****************************************************************************
    Function       : void SER_XmitMsg(const char *msg, UINT16 size)
    Parameters     : msg = pointer to the serial message
                     size = size of the serial message
    Return         : none
    Description    : This is the global routine provided to outside
                     to transmit any serial message
    Note           : Message will be sent out as is. Overhead such as header,
                     checksum are assumed to be in the message itself already.
                     This is a recursive function to break down a long message
                     into smaller messages.
                     size is declared as UINT16 is to accomodate large
                     message like a terminal display message.
****************************************************************************/
void cardAL_SER_XmitMsg
(
	PDATAPUMP_CONTEXT	 pDatapump, 
	const char			*msg,
	UINT16				 size
)
{
	NTSTATUS 					Status;
	TIGRIS_CONTEXT_T            * pContext = pDatapump->pExternalContext ;

	// Pargain is unaware of context.  Must use global SINGLE instance context!
	if (pContext == NULL)
	{
		return;
	}

	VERIFY_TIGRIS (pContext) ;
	VERIFY_DATAPUMP (pDatapump) ;

	// *
	// * Pass Message to System Interface.  Don't care about return Status
	// *
	Status = SmSysIfSendMessage(
	             pContext->SysIfHandle,		// System Interface Handle
	             MESS_COMM0_DATA,			// Event
	             (CHAR *)msg,				// Message Buffer
	             (DWORD) size,				// Length
	             NULL,						// SendComplete
	             NULL						// SendComplete Context
	         );
}

/******************************************************************************
 Begin Serial Port Debug Routines
******************************************************************************/

/******************************************************************************
FUNCTION NAME:
	cardALOpenDumbTerminal

ABSTRACT:
	Monitor Opens from Comm Apps.  Open appropriate Pairgain serial channel


RETURN:
	STATUS_SUCCESS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS cardALOpenDumbTerminal(
    void					* pCon,
    CHAR					* Buffer,
    DWORD					Length
)
{
	TIGRIS_CONTEXT_T		* pContext;
	DATAPUMP_CONTEXT		* pDatapump ;

	pContext= (TIGRIS_CONTEXT_T *) pCon;
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	VERIFY_TIGRIS (pContext) ;
	VERIFY_DATAPUMP (pDatapump) ;

	SER_DetectTerminalConnected ( pDatapump ) ;

	return (STATUS_SUCCESS);
}



/******************************************************************************
FUNCTION NAME:
	cardALCloseDumbTerminal

ABSTRACT:
	Close Pairgain serial channel


RETURN:
	STATUS_SUCCESS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS cardALCloseDumbTerminal(
    void					* pCon,
    CHAR					* Buffer,
    DWORD					Length
)
{
	TIGRIS_CONTEXT_T		* pContext;
	DATAPUMP_CONTEXT		* pDatapump ;

	pContext= (TIGRIS_CONTEXT_T *) pCon;
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	VERIFY_TIGRIS (pContext) ;
	VERIFY_DATAPUMP (pDatapump) ;

	DBG_CDSL_DISPLAY
	(
	    DBG_LVL_MISC_INFO,
	    pContext->pThisAdapter->DebugFlag,
	    ("ShutdownTerminal:  Entry;")
	) ;

	// *
	// * Disable Auto update and log off
	// *
	TRM_AutoUpdateDisable ( pDatapump, LOCAL_LOGON, TRUE ) ;
	SER_ClearRemoteLogon ( pDatapump ) ;

	return (STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	cardAlRxComm

ABSTRACT:
	This function is used to take a new buffer from the host, which is then
	sent to the Pairgain serial port


RETURN:
	NTSTATUS:
		STATUS_SUCCESS: No errors are returned from Pairgain.  Therefore,
		we hope it works.


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS cardAlRxComm (
	VOID					* pCon,
	CHAR					* Buffer,
	DWORD					  Length
)
{
	DWORD					  RxLoop;
	TIGRIS_CONTEXT_T		* pContext;
	DATAPUMP_CONTEXT		* pDatapump ;

	pContext= (TIGRIS_CONTEXT_T *) pCon;
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	VERIFY_TIGRIS (pContext) ;
	VERIFY_DATAPUMP (pDatapump) ;

	if(!Buffer || !Length)
	{
		return(-EINVAL);
	}
	// *
	// * Transmit Buffer from RX Stack to Serial Port Device
	// *
	for(RxLoop = 0; RxLoop < Length; RxLoop++)
	{
		SER_RcvIntrHandler ( pDatapump, Buffer[RxLoop] );
	}
	return STATUS_SUCCESS;
}


/******************************************************************************
FUNCTION NAME:
	cardALOpenDebugTerminal

ABSTRACT:
	Debug Routine to indicate we have an open Comm Channel. A hello world 
	message is sent to the Comm port.


RETURN:
	NTSTATUS: STATUS_SUCCESS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS cardALOpenDebugTerminal(
    void					* pCon,
    CHAR					* Buffer,
    DWORD					Length
)
{
	TIGRIS_CONTEXT_T		* pContext=(TIGRIS_CONTEXT_T*)pCon;

	// *
	// * Pass Message to System Interface.  Don't care about return Status
	// *
	SmSysIfSendMessage(
	    pContext->SysIfHandle,		// System Interface Handle
	    MESS_COMM2_DATA,			// Event
	    DEBUG_OPEN_MESSAGE,			// Message Buffer
	    sizeof(DEBUG_OPEN_MESSAGE),	// Length
	    NULL,						// SendComplete
	    NULL						// SendComplete Context
	);

	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	cardALOpenDebugEcho

ABSTRACT:
	Echo characters back to debug port


RETURN:
	NTSTATUS: STATUS_SUCCESS


DETAILS:
******************************************************************************/

GLOBAL NTSTATUS cardALOpenDebugEcho(
    void					* pCon,
    CHAR					* Buffer,
    DWORD					Length
)
{
	TIGRIS_CONTEXT_T		* pContext=(TIGRIS_CONTEXT_T*)pCon;

	// *
	// * Pass Message to System Interface.  Don't care about return Status
	// *
	SmSysIfSendMessage(
	    pContext->SysIfHandle,		// System Interface Handle
	    MESS_COMM2_DATA,			// Event
	    Buffer,						// Message Buffer
	    Length,					// Length
	    NULL,						// SendComplete
	    NULL						// SendComplete Context
	);

	return(STATUS_SUCCESS);
}

#endif									// VCOMM



/******************************************************************************
FUNCTION NAME:
	CardAlTigrisWaitQedfer

ABSTRACT:
	Waits on qedfer flags to be set. 

RETURN:
	NONE

DETAILS:
	
******************************************************************************/
BYTE WaitQedferUsesDelay = 0 ;
void CardAlTigrisWaitQedfer
(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN WORD						  Flags,
    IN WORD						  Resolution,	//milliseconds
    IN WORD						  Timeout		//milliseconds
)
{
	TIGRIS_CONTEXT_T			* pContext ;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_TIGRIS (pContext) ;

	if ( WaitQedferUsesDelay )
	{
		CardALDelayMsec ( pThisAdapter, Resolution ) ;
		return ;
	}


	// clear event we will wait on
	RESET_EVENT ( &pContext->Qedfer_Event ) ;

	pContext->Qedfer_Wait_Flag = Flags ;

	// Timeout = 0 is infinite - that is not what is desired
	if ( Timeout == 0 )
	{
		Timeout = 1 ;
	}

	// Wait on interrupt handler (Pairgain handler run at DPC/Dispatch level)
	// to handle all requested flags and set event
	WAIT_EVENT ( &pContext->Qedfer_Event, Timeout ) ;
}



/******************************************************************************
FUNCTION NAME:
	CardAlTigrisSignalQedfer

ABSTRACT:
	sets qedfer flags 

RETURN:
	NONE

DETAILS:
	
******************************************************************************/
void CardAlTigrisSignalQedfer
(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN WORD						  Flags
)
{
	TIGRIS_CONTEXT_T			* pContext ;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_TIGRIS (pContext) ;

	if ( WaitQedferUsesDelay )
	{
		return ;
	}

	pContext->Qedfer_Wait_Flag &= ~Flags ;
	if ( pContext->Qedfer_Wait_Flag == 0 )
	{
		SET_EVENT ( &pContext->Qedfer_Event ) ;
	}
}


/******************************************************************************
FUNCTION NAME:
	CardALDelayMsec

ABSTRACT:
	Provides a delay function (in milliseconds). 

RETURN:
	NONE

DETAILS:
	Depending on the setting of USE_DEVICE_TIMER_FOR_DELAY, this function can
	use the device's loogical timers or NDIS functions. Note that under NT, the
	NDIS functions only provide 10ms of resolution and under Win98 only 2.5 ms.
	
	The DPCs and the work items are actually running at the same level. Under NT
	with its pre-emption everything works fine. But under Win98, since the work
	item is often polling a flag that is set by a DPC, if the work item pauses
	0 milliseconds, the DPC never gets to run and set the flag so that we are
	deadlocked. Therefor this function provides 1ms of delay under Win98 when
	the caller asks for 0. 
******************************************************************************/

GLOBAL void CardALDelayMsec
(
	IN CDSL_ADAPTER_T				* pThisAdapter,
	IN WORD 						  Duration		// In milliseconds
)
{
	TIGRIS_CONTEXT_T				* pContext ;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	VERIFY_ADAPTER (pThisAdapter) ;
	VERIFY_TIGRIS (pContext) ;

// just use sleep if not using the device timer. Use
#if	!USE_DEVICE_TIMER_FOR_DELAY
	// Use a spin for short delays 
	if (Duration <= 3)
		MICRO_DELAY( Duration*1000);
	else
		SLEEP ( Duration ) ;
/*
	{
		if ( pThisAdapter->Showtime )
			SLEEP ( Duration ) ;
		else
		{
			ULONG DeltaTime;
			ULONG StartTime    = CardAlTigrisGetCurrentTick((CDSL_ADAPTER_T*)pThisAdapter);
			do
			{
				MICRO_DELAY( 500);
				DeltaTime = 
					CardAlTigrisGetCurrentTick((CDSL_ADAPTER_T*)pThisAdapter) -
					StartTime;
			} while (DeltaTime < Duration);
		}
	}
*/
	return;

// otherwise use the device timer
#else
	// if shutting down
	if ( pThisAdapter->shutdown )
	{
		// end all outstanding delays
		BYTE index;

		for ( index = 0 ; index < NUM_VIRTUAL_DEVICE_TIMERS ; index++ )
		{
			if(pContext != NULL)
			{
				SET_EVENT ( &pContext->TimerExpiredEvent [index] ) ;
			} 
			else 
			{
				break;
			}
		}
		return;
	}

	// Do the delay
	// If I am training up to showtime then use the device timer for
	// extra timing accuracy and then once in showtime use sleeps to
	// reduce overhead (the constant interrupt handling affects system
	// system performance.
	if ( pThisAdapter->Showtime )
	{
		if ( !pThisAdapter->shutdown )
		{
			if (Duration <= 1)
			{
				MICRO_DELAY( Duration*1000);
			}
			else
			{
				SLEEP ( Duration ) ;
			}

			return;
		}
	}
	else
	{
		CSR_COMMAND_PARAM_T	Csr_Command_Param ;
		BYTE TimerID ;
		DWORD LockFlag;

		// if its a 0 duration then check the scheduler and return
		if (Duration <= 0)
		{
			schedule();
			return;
		}

		// allocate one of the two timers (one for each thread/work item)
		// These must be allocated since we cant hard associate a timer with each
		// work item as we don't know which work item is calling us.
		ACQUIRE_LOCK ( &pContext->TimerStackLock, LockFlag ) ;
		if (pContext->TimerStackPtr == 0 )
		{
			DBG_CDSL_DISPLAY
			(
			    DBG_LVL_ERROR,
			    pThisAdapter->DebugFlag,
			    ("CardAlTigris :  Timer stack allocation underflow")
			) ;
		}

		TimerID = pContext->TimerStackArray [ --(pContext->TimerStackPtr) ] ;
		RELEASE_LOCK ( &pContext->TimerStackLock, LockFlag ) ;

		// Reset the event before we issue the command that will set it.
		RESET_EVENT ( &pContext->TimerExpiredEvent [TimerID] ) ;

		Csr_Command_Param.Param.StartTimer.LogicalTimer = TimerID ;
		Csr_Command_Param.Param.StartTimer.MicroSeconds = Duration * 1000 ;
		Csr_Command_Param.Param.StartTimer.Mode = TIMER_MODE_ONE_SHOT_W_IND ;
		Csr_Command_Param.NotifyOnComplete = NULL ;
		Csr_Command_Param.UserContext = 0 ;
		Csr_Command_Param.Command = DSL_START_LOGICAL_TIMER ;

		// Send start timer command to device
		ChipALWrite(
		    pThisAdapter,
		    P46_ARM_DSL,
		    ADDRESS_BAR_0,
		    CSR_ARM_CMD,
		    sizeof (DWORD) + sizeof ( COMMAND_START_LOGICAL_TIMER_T ),	// Not used at this time
		    (DWORD)&Csr_Command_Param,
		    0 					// Mask	Value is unused	by CSR_ARM_CMD
		);

		// Wait on device to send timer expired response
		if ( !WAIT_EVENT ( &pContext->TimerExpiredEvent[TimerID], Device_Wait_Timeout ) )
		{
			// Timed out rather than device sent event - stop timer
			CSR_COMMAND_PARAM_T	Csr_Command_Param ;

			// build stop timer command
			Csr_Command_Param.Param.StopTimer.LogicalTimer = 0 ;
			Csr_Command_Param.NotifyOnComplete = NULL ;
			Csr_Command_Param.UserContext = 0 ;
			Csr_Command_Param.Command = DSL_STOP_LOGICAL_TIMER ;

			// Send stop timer command to device
			ChipALWrite
			(
			    pThisAdapter,
			    P46_ARM_DSL,
			    ADDRESS_BAR_0,
			    CSR_ARM_CMD,
			    sizeof (DWORD) + sizeof ( COMMAND_STOP_LOGICAL_TIMER_T ),	// Not used at this time
			    (DWORD)&Csr_Command_Param,
			    0 					// Mask	Value is unused	by CSR_ARM_CMD
			);
		}

		// Release the allocated timer ID
		ACQUIRE_LOCK ( &pContext->TimerStackLock, LockFlag ) ;
		pContext->TimerStackArray [ (pContext->TimerStackPtr)++ ] = TimerID ;
		if (pContext->TimerStackPtr > NUM_VIRTUAL_DEVICE_TIMERS )
		{
			DBG_CDSL_DISPLAY
			(
			    DBG_LVL_ERROR,
			    pThisAdapter->DebugFlag,
			    ("CardAlTigris :  Timer stack allocation underflow") ;
			) ;
		}
		RELEASE_LOCK ( &pContext->TimerStackLock, LockFlag ) ;
	}
#endif
}




#if 0
#else
	extern int						CnxtDslPhysicalDriverType;
#endif
/******************************************************************************
FUNCTION NAME:
	cardALTigrisAFEEEPROM

ABSTRACT:
	Return AFE eeprom value recorded in context.
	AFE eeprom is read during BG thread initialization

RETURN:
	AFE eeprom value recorded in context

DETAILS:
******************************************************************************/ 
DPCCALL GLOBAL DWORD cardALTigrisAFEEEPROM
(
	PVOID *		pContext
)
{
	TIGRIS_CONTEXT_T *		pController;
	NTSTATUS				Status;


	pController = (TIGRIS_CONTEXT_T *) pContext;
	Status = NON_TIGRIS_EEPROM_AFE_ID;

//***
//*** Reading the AFE Id and AFE Id Check from EEPROM needs to be added
//*** at some point in the future!
//***
#if 0
	if ( pController != NULL)
	{
		Status = pController->AFETypeFromEEPROM;
	}
#else
	switch (CnxtDslPhysicalDriverType)
	{
		case DRIVER_TYPE_TIGRIS:
			Status = TIGRIS_EEPROM_AFE2B_AFE_ID;
			break;
		case DRIVER_TYPE_YUKON:
		default:
			Status = NON_TIGRIS_EEPROM_AFE_ID;
			break;
	}
#endif

	return Status;
}
