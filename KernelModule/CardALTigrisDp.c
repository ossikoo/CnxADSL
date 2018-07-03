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
**		CardAL
**
**	FILE NAME:
**		CardALTigrisDp.c
**
**	ABSTRACT:
**		Falcon HAL Interface to Pairgain Data Pump code
**
**	DETAILS:
**	
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.4.3_ATM_Driver/KernelModule/CardALTigrisDp.c $
**	$Revision: 11 $
**	$Date: 7/25/01 5:06p $
*******************************************************************************/

#include "Common.h"
#include "CardALTigrisV.h"
#include "../CommonData.h"
#include "CardALTigrisHal.h"
#include "CardALTigrisHalV.h"
#include "CardALTigrisDp.h"
#include "ChipALCdslV.h"
#include "CardALV.h"
#include "CardMgmt.h"
#include "sysinfo.h"
#include "linux/interrupt.h" 

// Tempory Globals since Pairgain does not pass handles to the wrapper routines,
// they must load handles from these globals for now

LOCAL CDSL_ADAPTER_T 		* pThisAdapter;

BOOLEAN ChipAlTigrisDPLinkUp
(
    void * Param
);

#define CHIPAL_CONNECT_TIMER_ID  "CHCT"
#define CARDAL_MAX_CONNECT_CRC    50


/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDPInit

ABSTRACT:
	Initialize


RETURN:
	.

******************************************************************************/
void ChipAlTigrisDPInit
(
    IN CDSL_ADAPTER_T 			* pThisAdapterParm,
    IN TIGRIS_HAL_CONTEXT_T		* pContext

)
{
	// Set global variable that we use since Pairgain code does not pass this to
	// us at this time
	pThisAdapter = pThisAdapterParm ;

	// initialize to 1 because interrupts are initially off and first enable/disable
	// call is to enable and this should enable interrupts.
	pContext->DisableInterruptCount = 1 ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpGetCurrentTick

ABSTRACT:
	Return the Timer's Tick Count


RETURN:
	.

******************************************************************************/
DPCCALL TICK_TYPE ChipAlTigrisDpGetCurrentTick
(
	IN DATAPUMP_CONTEXT		  *pDatapump
) 
{
	TIGRIS_CONTEXT_T			* pControllerContext ;
	CDSL_ADAPTER_T				* pThisAdapter ;

	pControllerContext = (TIGRIS_CONTEXT_T *) pDatapump->pExternalContext ;
	pThisAdapter = pControllerContext->pThisAdapter ;

	VERIFY_ADAPTER(pThisAdapter) ;
	VERIFY_TIGRIS(pControllerContext) ;
	VERIFY_DATAPUMP(pDatapump) ;

	return CardAlTigrisGetCurrentTick ( pThisAdapter ) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpWaitQedfer

ABSTRACT:
	Waits on qedfer flags to be set. 


RETURN:
	.

******************************************************************************/
DPCCALL void ChipAlTigrisDpWaitQedfer
(
    IN WORD Flags,
    IN WORD Resolution,
    IN WORD Timeout
)
{
	CardAlTigrisWaitQedfer ( pThisAdapter, Flags, Resolution, Timeout ) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDPGetCurrentTick

ABSTRACT:
	sets qedfer flags 


RETURN:
	.

******************************************************************************/
DPCCALL void ChipAlTigrisDpSignalQedfer
(
    IN WORD Flags
)
{
	CardAlTigrisSignalQedfer ( pThisAdapter, Flags ) ;
}


/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDPLinkUp

ABSTRACT:
	Indicate Link is up


RETURN:
	.

******************************************************************************/
BOOLEAN ChipAlTigrisDPLinkUp
(
    void * Param
)
{
//	CDSL_ADAPTER_T			* pThisAdapter  ;	//use the global since Param==NULL and we can't derive it.
	CARDAL_ADAPTER_T		* pCardALAdapter  ;
	TIGRIS_CONTEXT_T		* pControllerContext ;
	DATAPUMP_CONTEXT		* pDatapump ;
	DWORD					AdptrChk;

//	pThisAdapter = pCardALAdapter->pCdslAdapter ;	// we cant derive this - ignore for now
	AdptrChk = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );

	pControllerContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pControllerContext->pDatapump ;

	VERIFY_ADAPTER(pThisAdapter) ;
	VERIFY_CARDAL(pCardALAdapter) ;
	VERIFY_TIGRIS(pControllerContext) ;
	VERIFY_DATAPUMP(pDatapump) ;

	if ( AdptrChk == RESULT_SUCCESS )
	{
		if ((pCardALAdapter->CurrentCRCErrors <= pDatapump->sys_err_cnt.crc) &&
		        ((pCardALAdapter->CurrentCRCErrors + CARDAL_MAX_CONNECT_CRC ) > pDatapump->sys_err_cnt.crc))
		{
			pThisAdapter->MediaConnectStatus = MediaConnected;

			CmgmtIndicateLinkUp ( pThisAdapter ) ;

			pCardALAdapter->AdslLineUp = TRUE ;
		}
		else
		{
			// reschedule timer and try later
			pCardALAdapter->CurrentCRCErrors  = pDatapump->sys_err_cnt.crc;

			UtilTmrStartTmr
			(
			    &(pThisAdapter->TimerBlock),
			    pCardALAdapter,
			    (DWORD)CHIPAL_CONNECT_TIMER_ID,
			    TIMER_RESOLUTION_2_SEC,
			    &ChipAlTigrisDPLinkUp,
			    NULL
			);
		}
	}

	return TRUE; // only one shot timer
}


/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDPIndicateLinkUp

ABSTRACT:
	Indicate Link is up


RETURN:
	.

******************************************************************************/
DPCCALL void ChipAlTigrisDPIndicateLinkUp
(
	TIGRIS_CONTEXT_S		* pControllerContextS
)
{
	TIGRIS_CONTEXT_T		* pControllerContext ;
	CDSL_ADAPTER_T			* pThisAdapter ;
	CARDAL_ADAPTER_T		* pCardALAdapter;
	DATAPUMP_CONTEXT		* pDatapump ;
	DWORD					AdptrChk;

	// convert from un-populated datatype to populated data type 
	// these are equivilent - the unpopulated is available for function prototyping
	// whereas the populated one is not due to chicken/egg problem with include files.
	pControllerContext = ( TIGRIS_CONTEXT_T * ) pControllerContextS ;

	pDatapump = (DATAPUMP_CONTEXT *) pControllerContext->pDatapump ;
	pThisAdapter = pControllerContext->pThisAdapter ;

	AdptrChk = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );

	VERIFY_ADAPTER(pThisAdapter) ;
	VERIFY_CARDAL(pCardALAdapter) ;
	VERIFY_TIGRIS(pControllerContext) ;
	VERIFY_DATAPUMP(pDatapump) ;

	if ( AdptrChk == RESULT_SUCCESS )
	{
		UtilTmrClearTmr
		(
		    &(pThisAdapter->TimerBlock),
		    pCardALAdapter,
		    (DWORD)CHIPAL_CONNECT_TIMER_ID
		);

		pCardALAdapter->CurrentCRCErrors  = pDatapump->sys_err_cnt.crc;

		UtilTmrStartTmr
		(
		    &(pThisAdapter->TimerBlock),
		    pCardALAdapter,
		    (DWORD)CHIPAL_CONNECT_TIMER_ID,
		    TIMER_RESOLUTION_2_SEC,
		    &ChipAlTigrisDPLinkUp,
		    NULL
		);
	}

}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDPIndicateLinkDown

ABSTRACT:
	Indicate Link is down


RETURN:
	.

******************************************************************************/
DPCCALL void ChipAlTigrisDPIndicateLinkDown
(
	TIGRIS_CONTEXT_S		* pControllerContextS
)
{
	CDSL_ADAPTER_T			* pThisAdapter ;
	CARDAL_ADAPTER_T		* pCardALAdapter;
	DWORD					AdptrChk;
	TIGRIS_CONTEXT_T		* pControllerContext ;

	// convert from un-populated datatype to populated data type 
	// these are equivilent - the unpopulated is available for function prototyping
	// whereas the populated one is not due to chicken/egg problem with include files.
	pControllerContext = ( TIGRIS_CONTEXT_T * ) pControllerContextS ;

	pThisAdapter = pControllerContext->pThisAdapter ;
	AdptrChk = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );

	VERIFY_ADAPTER(pThisAdapter) ;
	VERIFY_CARDAL(pCardALAdapter) ;
	VERIFY_TIGRIS(pControllerContext) ;

	// Clear any outstanding Connect timer
	UtilTmrClearTmr
	(
	    &(pThisAdapter->TimerBlock),
	    pCardALAdapter,
	    (DWORD)CHIPAL_CONNECT_TIMER_ID
	);

	pThisAdapter->MediaConnectStatus = MediaNotConnected;

	CmgmtIndicateLinkDown ( pThisAdapter ) ;

	AdptrChk = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );
	if ( AdptrChk == RESULT_SUCCESS )
	{
		pCardALAdapter->AdslLineUp = FALSE ;
	}

}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpDisableIRQ1_2

ABSTRACT:
	Disable ADSL interrupts


RETURN:
	NA.


DETAILS:
******************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpDisableIRQ1_2
(
    void
)
{
	CHIPAL_T					* pThisInterface ;
	TIGRIS_HAL_CONTEXT_T		* pContext ;

	// if I haven't been initialized then just ignore the request
	if (!pThisAdapter)
		return;

	// get the hal contxt
	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod ;
	pContext = ( TIGRIS_HAL_CONTEXT_T *) pThisInterface->ServiceContext[CAL_SERVICE_FALCON] ;

	// Keep a count to support nested calls
	pContext->DisableInterruptCount++;

	// Disable interrupts together
	// (This runs at interrupt level (DIRQL) cause this involves a read/modify/write operation)
	ChipAlTigrisHalDisableIRQ1_2 ( pThisAdapter ) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpEnableIRQ1_2

ABSTRACT:
	Enable ADSL interrupts


RETURN:
	NA.


DETAILS:
******************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpEnableIRQ1_2
(
    void
)
{
	LONG						  DisableInterruptCount ;
	CHIPAL_T					* pThisInterface ;
	TIGRIS_HAL_CONTEXT_T		* pContext ;

	// if I haven't been initialized then just ignore the request
	if (!pThisAdapter)
		return;

	// get the hal contxt
	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod ;
	pContext = ( TIGRIS_HAL_CONTEXT_T *) pThisInterface->ServiceContext[CAL_SERVICE_FALCON] ;

	// if this is the bottom call of nested calls.
	DisableInterruptCount = --pContext->DisableInterruptCount;

	if ( DisableInterruptCount == 0 )
	{
		// We can Twiddle the bits at any level since intterupts are off
		ChipAlTigrisHalEnableIRQ1_2 ( pThisAdapter ) ;
	}
}


/***************************************************************************
   Macro         : ChipAlTigrisDpAtomicStart
   Parameters    : none.
   Returns       : none
   Description   : This macro will disable interrupts until the next atomic
   				   end call.
****************************************************************************/
DWORD AtomicFlag;
DWORD AtomicCnt=0;
DPCCALL MODULAR VOID ChipAlTigrisDpAtomicStart
(
	void
)
{
	// if this is the first start then save
	// the interrupt enable state and disable
	// interrupts
	if ( AtomicCnt == 0)
	{ // --PIM-- SMP unsafe?
        	local_irq_save(AtomicFlag);
//disable_irq(11);
	} // --/PIM--
	
	AtomicCnt++;
}

/***************************************************************************
   Macro         : ChipAlTigrisDpAtomicEnd
   Parameters    : none.
   Returns       : none
   Description   : This macro will re-enable interrupts after an atomic
   				   start call.
****************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpAtomicEnd
(
	void
)
{
	// validate the call
	if (AtomicCnt)
	{
		AtomicCnt--;
	
		// if this is the last end then restore the
		// interrupt enable state
		if (!AtomicCnt)
		{ // --PIM-- SMP unsafe?
		  local_irq_restore(AtomicFlag);
// enable_irq(11);
		} // --/PIM--
	}
	
#ifdef DBG
	else
		printk("<1>CnxADSL invalid atomic end\n");
#endif 
}



/******************************************************************************
FUNCTION NAME:
    CardALTigrisSITrap 

ABSTRACT:
	Wrapper routine for controller code to notifies us that it's ready to 
	process software trap

RETURN:
	NA.


DETAILS:

******************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpSITrap ( void )
{
	// set event
	CardALTigrisSITrap ( pThisAdapter ) ;
}



/***************************************************************************
   Macro         : ChipAlTigrisDpIsrStart
   Parameters    : none.
   Returns       : none
   Description   : This function is here to satisfy the PairGain code.  For
                   Linux it has nothing to do.
****************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpIsrStart
(
	void
)
{
}


/***************************************************************************
   Macro         : ChipAlTigrisDpIsrEnd
   Parameters    : none.
   Returns       : none
   Description   : This function is here to satisfy the PairGain code.  For
                   Linux it has nothing to do.
****************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpIsrEnd
(
	void
)
{
}

/***************************************************************************
   Macro         : ChipAlTigrisDpDelayMsec
   Parameters    : duration - delay amount in milliseconds.
   Returns       : none
   Description   : This macro will call the cardAL function to pause for the
                   requested amount of time.
****************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpDelayMsec
( 
	TIGRIS_CONTEXT_S		* pControllerContextS,
	DWORD					  duration 
) 
{
	CDSL_ADAPTER_T				* pThisAdapter ;
	TIGRIS_CONTEXT_T			* pControllerContext ;

	// convert from un-populated datatype to populated data type 
	// these are equivilent - the unpopulated is available for function prototyping
	// whereas the populated one is not due to chicken/egg problem with include files.
	pControllerContext = ( TIGRIS_CONTEXT_T * ) pControllerContextS ;

	pThisAdapter = pControllerContext->pThisAdapter ;

	CardALDelayMsec ( pThisAdapter, duration );
}


/***************************************************************************
   Macro         : ChipAlTigrisDpGetNonVolatileParms
   Parameters    : pointers to the return values read from the registry   
   Returns       : none
   Description   : This function returns the nonvolatile params read from
                   the system registry
****************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpGetNonVolatileParms
(
	IN PDATAPUMP_CONTEXT		  pDatapump,
	IN XCVR_OTHER_CFG_TYPE		* pXcvr
) 
{
	TIGRIS_CONTEXT_T			* pControllerContext ;
	CDSL_ADAPTER_T				* pThisAdapter ;

	pControllerContext = (TIGRIS_CONTEXT_T *) pDatapump->pExternalContext ;
	pThisAdapter = pControllerContext->pThisAdapter ;

	// if I haven't been initialized then just ignore the request
	if (!pThisAdapter)
	{
		return;
	}

	VERIFY_ADAPTER(pThisAdapter) ;
	VERIFY_TIGRIS(pControllerContext) ;
	VERIFY_DATAPUMP(pDatapump) ;

	pXcvr->sys_autohandshake		= pThisAdapter->PersistData.AutoSenseHandshake;
	pXcvr->sys_autosense_wiresfirst	= pThisAdapter->PersistData.AutoSenseWiresFirst;
	pXcvr->sys_autowiring_selection	= pThisAdapter->PersistData.AutoWiringSelection;
	pXcvr->sys_autowiring_num_combos= pThisAdapter->PersistData.AutoWiringNumCombos;
	pXcvr->AdslHeadEnd				= pThisAdapter->PersistData.AdslHeadEnd;
	pXcvr->AdslHeadEndEnvironment	= pThisAdapter->PersistData.AdslHeadEndEnvironment;
}


/***************************************************************************
   Macro         : ChipAlTigrisDpTestAndSet
   Parameters    : flag = the flag we want to set, we will set it if it's
                   not already set.
   Returns       : return TRUE when flag is not set
                   and FALSE if already set
   Description   : This macro will perform atomic testing and setting
                   of the flag.
   Note          : BSO provides a library macro for C16x to perform this
                   operation.
****************************************************************************/
DPCCALL MODULAR BOOLEAN ChipAlTigrisDpTestAndSet
(
    IN volatile LOCK_TYPE		* x
)
{
	BOOLEAN RetVal;
	
	RetVal = !test_and_set_bit( 0, x );
	
	return RetVal;
}


/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpSetRstXcvr

ABSTRACT:
	Sets or Resets the transceiver


RETURN:
	NA.


DETAILS:
******************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpSetRstXcvr
(
    IN BOOLEAN					  State
)
{
	ChipAlTigrisHalSetRstXcvr	 ( pThisAdapter, State ) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpSetGpio

ABSTRACT:
	Sets or Resets the GPIO pin


RETURN:
	NA.


DETAILS:
******************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpSetGpio
(
    IN BYTE						  Pin,
    IN BOOLEAN					  State
)
{
	ChipAlTigrisSetGpio	 ( pThisAdapter, Pin, State ) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpSetAFEHybridSelect

ABSTRACT:
	Sets or Resets the AFE Hybrid Select lines


RETURN:
	NA.


DETAILS:
******************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpSetAFEHybridSelect
(
    IN UINT8					  Select_Line,
    IN BOOLEAN					  State
)
{
	ChipAlTigrisHalSetAFEHybridSelect ( pThisAdapter, Select_Line, State ) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpSetPwrDwnXcvr

ABSTRACT:
	Sets or Resets the Falcon Power Down pin


RETURN:
	NA.


DETAILS:
******************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpSetPwrDwnXcvr
(
    IN BOOLEAN					  State
)
{
	ChipAlTigrisSetPwrDwnXcvr	 ( pThisAdapter, State ) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpWriteAFE

ABSTRACT:
	Write data to AFE at specified location


RETURN:
	NA.


DETAILS:
******************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpWriteAFE
(
    IN BYTE						  Address,
    IN BYTE						  Data
)
{
	ChipAlTigrisHalWriteAfe ( pThisAdapter, Address, Data ) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpPendingReadAFE

ABSTRACT:
	Send a read data command to AFE at specified location


RETURN:
	NA.


DETAILS:
******************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpPendingReadAFE
(
	IN BYTE						  Address
)
{
	ChipAlTigrisHalPendingReadAfe ( pThisAdapter, Address ) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpReadAFE

ABSTRACT:
	Read data from AFE at location defined in ChipAlTigrisDpPendingReadAFE()


RETURN:
	NA.


DETAILS:
******************************************************************************/
DPCCALL MODULAR BYTE ChipAlTigrisDpReadAFE
(
	void
)
{
	return ChipAlTigrisHalReadAfe ( pThisAdapter ) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpIsAFEDone

ABSTRACT:
	Return whether previous write/read has completed


RETURN:
	NA.


DETAILS:
******************************************************************************/
DPCCALL MODULAR BOOLEAN ChipAlTigrisDpIsAFEDone
(
    void
)
{
	return ChipAlTigrisHalIsAfeDone ( pThisAdapter ) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlDpSwitchHookStateEnq

ABSTRACT:

RETURN:
	OffHook = 0
	OnHook = 1

DETAILS:
******************************************************************************/
DPCCALL GLOBAL BOOLEAN ChipAlDpSwitchHookStateEnq
(
    void
)
{
	return ChipAlSwitchHookStateEnq ( pThisAdapter ) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpTraceWrite

ABSTRACT:
	Write Value to PCI Trace Location


RETURN:
	NA.


DETAILS:
******************************************************************************/
DPCCALL MODULAR VOID ChipAlTigrisDpTraceWrite
(
    IN BYTE	Offset,
    IN DWORD Value
)
{
	ChipAlTigrisHalTraceWrite	 ( pThisAdapter, Offset, Value ) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpGetVendorId

ABSTRACT:
	Return Vendor ID


RETURN:
	.

******************************************************************************/
DPCCALL GLOBAL UINT16 ChipAlTigrisDpGetVendorId
(
    void
)
{
	CARDAL_ADAPTER_T		* pCardALAdapter;
	DWORD					AdptrChk;

	AdptrChk = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );

	return CdalADSLGetVendorId (pCardALAdapter) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpGetAutoWiring

ABSTRACT:
	Return Auto Wiring


RETURN:
	.

******************************************************************************/
GLOBAL UINT16 ChipAlTigrisDpGetAutoWiring
(
    void
)
{
	CARDAL_ADAPTER_T		* pCardALAdapter;
	DWORD					AdptrChk;

	AdptrChk = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );

	return CdalADSLGetAutoWiring (pCardALAdapter) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpGetAutoHandshake

ABSTRACT:
	Return Auto Wiring


RETURN:
	.

******************************************************************************/
GLOBAL UINT16 ChipAlTigrisDpGetAutoHandshake
(
    void
)
{
	CARDAL_ADAPTER_T		* pCardALAdapter;
	DWORD					AdptrChk;

	AdptrChk = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );

	return CdalADSLGetAutoHandshake (pCardALAdapter) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpGetAutoSenseWiresFirst

ABSTRACT:
	Return Auto Wiring


RETURN:
	.

******************************************************************************/
GLOBAL UINT16 ChipAlTigrisDpGetAutoSenseWiresFirst
(
    void
)
{
	CARDAL_ADAPTER_T		* pCardALAdapter;
	DWORD					AdptrChk;

	AdptrChk = cardALChkAdptrStrucsOkNotDnldgDP( pThisAdapter, &pCardALAdapter );

	return CdalADSLGetAutoSenseWiresFirst (pCardALAdapter) ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpSetWiring

ABSTRACT:
	Set the wiring relays to the desired pair


RETURN:
	.

******************************************************************************/
DPCCALL GLOBAL void ChipAlTigrisDpSetWiring
(
	BOOLEAN						AutomaticDetection,
    SYS_WIRING_SELECTION_TYPE	WiringSelection
)
{
	ChipAlTigrisHalSetWiring (	pThisAdapter,
								AutomaticDetection,
								WiringSelection ) ;
}


/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpSER_InitMsgQ

ABSTRACT:
	


RETURN:
	.

******************************************************************************/
GLOBAL void ChipAlTigrisDpSER_InitMsgQ
(
	IN CDSL_ADAPTER_T				* pThisAdapter
)
{
	TIGRIS_CONTEXT_T				* pControllerContext ;

	pControllerContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	VERIFY_ADAPTER(pThisAdapter) ;
	VERIFY_TIGRIS(pControllerContext) ;
	VERIFY_DATAPUMP(pControllerContext->pDatapump) ;

	SER_InitMsgQ ( pControllerContext->pDatapump ) ;
}

/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpTRM_TerminalInit

ABSTRACT:
	


RETURN:
	.

******************************************************************************/
GLOBAL void ChipAlTigrisDpTRM_TerminalInit
(
	IN CDSL_ADAPTER_T				* pThisAdapter
)
{
	TIGRIS_CONTEXT_T				* pControllerContext ;

	pControllerContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	VERIFY_ADAPTER(pThisAdapter) ;
	VERIFY_TIGRIS(pControllerContext) ;
	VERIFY_DATAPUMP(pControllerContext->pDatapump) ;

	TRM_TerminalInit ( pControllerContext->pDatapump ) ;
}

/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpSER_DetectTerminalConnected

ABSTRACT:
	


RETURN:
	.

******************************************************************************/
GLOBAL void ChipAlTigrisDpSER_DetectTerminalConnected
(
	IN CDSL_ADAPTER_T				* pThisAdapter
)
{
	TIGRIS_CONTEXT_T				* pControllerContext ;

	pControllerContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	VERIFY_ADAPTER(pThisAdapter) ;
	VERIFY_TIGRIS(pControllerContext) ;
	VERIFY_DATAPUMP(pControllerContext->pDatapump) ;

	SER_DetectTerminalConnected ( pControllerContext->pDatapump ) ;
}

/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpTRM_AutoUpdateDisable

ABSTRACT:
	


RETURN:
	.

******************************************************************************/
GLOBAL void ChipAlTigrisDpTRM_AutoUpdateDisable
(
	IN CDSL_ADAPTER_T				* pThisAdapter,
	SYS_LOGON_MODE_TYPE				  mode,
	BOOLEAN							  disable
)
{
	TIGRIS_CONTEXT_T				* pControllerContext ;

	pControllerContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	VERIFY_ADAPTER(pThisAdapter) ;
	VERIFY_TIGRIS(pControllerContext) ;
	VERIFY_DATAPUMP(pControllerContext->pDatapump) ;

	TRM_AutoUpdateDisable ( pControllerContext->pDatapump, mode, disable ) ;
}

/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpSER_ClearRemoteLogon

ABSTRACT:
	


RETURN:
	.

******************************************************************************/
GLOBAL void ChipAlTigrisDpSER_ClearRemoteLogon
(
	IN CDSL_ADAPTER_T				* pThisAdapter
)
{
	TIGRIS_CONTEXT_T				* pControllerContext ;

	pControllerContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	VERIFY_ADAPTER(pThisAdapter) ;
	VERIFY_TIGRIS(pControllerContext) ;
	VERIFY_DATAPUMP(pControllerContext->pDatapump) ;

	SER_ClearRemoteLogon ( pControllerContext->pDatapump ) ;
}


/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisDpSER_RcvIntrHandler

ABSTRACT:
	


RETURN:
	.

******************************************************************************/
GLOBAL void ChipAlTigrisDpSER_RcvIntrHandler
(
	IN CDSL_ADAPTER_T				* pThisAdapter,
	IN char							  input_char
)
{
	TIGRIS_CONTEXT_T				* pControllerContext ;

	pControllerContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	VERIFY_ADAPTER(pThisAdapter) ;
	VERIFY_TIGRIS(pControllerContext) ;
	VERIFY_DATAPUMP(pControllerContext->pDatapump) ;

	SER_RcvIntrHandler ( pControllerContext->pDatapump, input_char ) ;
}



