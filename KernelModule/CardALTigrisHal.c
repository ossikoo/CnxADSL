/********************************************************************************
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
**		CardALTigrisHal.c
**
**	ABSTRACT:
**		Tigris HAL Interface to ChipAl
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/KernelModule/CardALTigrisHal.c $
**	$Revision: 14 $
**	$Date: 7/05/01 12:06p $
*******************************************************************************
******************************************************************************/



#include "CardMgmt.h"
#include "ChipALCdslV.h"
#include "CardALTigrisHal.h"
#include "CardALTigrisHalV.h"
#include "CardALTigrisDp.h"
#include "CardALTigrisV.h"
#include "project.h"		//Pairgain DMT API wrapper customization file
#include "datapump.h"

#ifndef MEMORY_TAG
	#define	MEMORY_TAG		'HXNC'
#endif

	#define SSCAN			8	// GPIO8
	#define nFRST			6	// GPIO6
	#define AFEPOR			4	// GPIO4
	#define HYBSEL2			3	// GPIO3
	#define HYBSEL1			2	// GPIO2
	#define nDRVPWR			1	// GPIO1
	#define	FPWRDWN			0	// GPIO0
	#define SWITCHHOOK		13	// GPIN1


#if ! defined(CHIPAL_REMOTE)
	#define CHIPAL_REMOTE 0
#endif 

#if (CAL_P46_INSTALLED)||(CHIPAL_REMOTE ==1)
#define ADDRESS_BAR_AFE	ADDRESS_BAR_1
#endif		// #if CAL_P46_INSTALLED


#define ADSL_TIP_RING			0x01	// K2
#define ADSL_A_A1				0x02	// K2
#define ADSL_HOLD				0x03	// K2

#define MODEM_TIP_RING			0x04	// K3	DO NOT USE IN THIS DRIVER
#define MODEM_A_A1				0x08	// K3	DO NOT USE IN THIS DRIVER
#define MODEM_HOLD				0x0C	// K3

#define	ADSL_LED1_ON			0x00
#define	ADSL_LED1_OFF			0x10

#define	ADSL_LED2_ON			0x00
#define	ADSL_LED2_OFF			0x20

#define WIRING_RELAY_ADDRESS	(0x3C0)
#define WIRING_RELAY_DELAY		100


/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalDeviceRdyNotifyReq

ABSTRACT:
	Notifies us as to what routine to call when the device is ready.


RETURN:
	NA.

******************************************************************************/
MODULAR VOID ChipAlTigrisHalDeviceRdyNotifyReq
(
	MODULAR void ( *ParmDeviceRdyNotify )( IN CDSL_ADAPTER_T		* pThisAdapter ),
	IN CDSL_ADAPTER_T			* pThisAdapter
)
{
	CHIPAL_T					* pThisInterface ;
	TIGRIS_HAL_CONTEXT_T		* pContext ;

	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod ;
	pContext = ( TIGRIS_HAL_CONTEXT_T *) pThisInterface->ServiceContext[CAL_SERVICE_FALCON] ;

	// if notification routine has already run
	if ( pContext->DeviceRdyState )
	{
		// call this notification function here and now
		ParmDeviceRdyNotify ( pThisAdapter ) ;
	}
	else
	{
		// remember the notification function to call
		pContext->DeviceRdyNotify =  ParmDeviceRdyNotify ;

		// Double check in case notification routine preempted and ran just
		// after last if statement, but before we stored the function
		if ( pContext->DeviceRdyState && pContext->DeviceRdyNotify != NULL )
		{
			ParmDeviceRdyNotify ( pThisAdapter ) ;
		}
	}
}


/******************************************************************************
FUNCTION NAME:
	chipAlTigrisHalDeviceRdyNotify

ABSTRACT:
	This runs when last gpio command has been process and device is ready. We
	must notify other modules if we have been requested to.


RETURN:
	NA.

******************************************************************************/
LOCAL VOID chipAlTigrisHalDeviceRdyNotify
(
	IN void		* pCon
)
{
	TIGRIS_HAL_CONTEXT_T		* pContext=(TIGRIS_HAL_CONTEXT_T*)pCon;

	// If we have been given a routine to call
	if ( pContext->DeviceRdyNotify != NULL )
	{
		// call it
		pContext->DeviceRdyNotify ( ((CHIPAL_T *)pContext->pInterface)->pNdisAdapter ) ;

		// note that it has been called
		pContext->DeviceRdyNotify  = NULL ;
	}

	// remember that we are now ready
	pContext->DeviceRdyState = TRUE ;
}


/******************************************************************************
FUNCTION NAME:
	ChipAlSwitchHookStateEnq

ABSTRACT:

RETURN:
	OffHook = 0
	OnHook = 1

DETAILS:
******************************************************************************/
GLOBAL BOOLEAN ChipAlSwitchHookStateEnq
(
	IN CDSL_ADAPTER_T			* pThisAdapter
)
{
	CHIPAL_T					* pThisInterface ;
	TIGRIS_HAL_CONTEXT_T		* pContext ;

	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod ;
	pContext = ( TIGRIS_HAL_CONTEXT_T *) pThisInterface->ServiceContext[CAL_SERVICE_FALCON] ;

	return pContext->SwitchHookState;
}



/******************************************************************************
FUNCTION NAME:
	chipAlIoChangeIndDpcHdlr

ABSTRACT:
	(DSL_ISR:FALIRQ1)
	DPC called function (from ChipAL module) for reading the ChipAL Falcon
	IRQ1.

RETURN:

DETAILS:
******************************************************************************/
LOCAL void chipAlIoChangeIndIsrHdlr
(
	IN void						* pAdapter,
	I_O RESPONSE_T				* Response
)
{
	CDSL_ADAPTER_T				* pThisAdapter=(CDSL_ADAPTER_T*)pAdapter;
	CHIPAL_T					* pThisInterface ;
	TIGRIS_HAL_CONTEXT_T		* pContext ;
	WORD						  GPIO_Number ;
	WORD						  GPIO_Value ;
	DWORD						  Param ;


	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod ;
	pContext = ( TIGRIS_HAL_CONTEXT_T *) pThisInterface->ServiceContext[CAL_SERVICE_FALCON] ;


	// Copy any parameters to user space
	Param = *(DWORD *)Response[HW_IO_CHANGE_IND].Param ;
	GPIO_Number = (WORD) Param >> 16 ;
	GPIO_Value = (WORD) Param & 0xFFFF ;


	switch (GPIO_Number)
	{
	case SWITCHHOOK:
		pContext->SwitchHookState = (BOOLEAN) GPIO_Value ;
		break;
	}
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalInit

ABSTRACT:
	Initialize MadMax registers


RETURN:
	NA.

******************************************************************************/
MODULAR VOID *ChipAlTigrisHalInit
(
	IN const VOID				* UserHandle
)
{
	TIGRIS_HAL_CONTEXT_T		* pContext ;
	CDSL_ADAPTER_T				* pThisAdapter ;
	CHIPAL_T					* pThisInterface ;
	NDIS_STATUS					  Status ;

	pThisInterface = (CHIPAL_T *) UserHandle ;
	pThisAdapter = pThisInterface->pNdisAdapter ;

	DBG_CDSL_DISPLAY
	(
		DBG_LVL_MISC_INFO,
		pThisAdapter->DebugFlag,
		("ChipAlTigrisHalInit:  Entry;")
	) ;

	//*
	//* Allocate and clear Context Structue
	//*
	Status =	ALLOCATE_MEMORY
				(
					(PVOID)&pContext,
					sizeof(TIGRIS_HAL_CONTEXT_T),
					MEMORY_TAG
				);

	if (Status != STATUS_SUCCESS)
	{
		return NULL ;
	}
	CLEAR_MEMORY(pContext, sizeof(TIGRIS_HAL_CONTEXT_T));

	pContext->pInterface = pThisInterface ;

	pContext->SwitchHookState = TRUE  ;	// TRUE = Onhook


	pContext->ADSL_Relay_Set_Value = ADSL_TIP_RING ;
	pContext->Modem_Relay_Set_Value = MODEM_TIP_RING ;
	pContext->ADSL_LED1_Value = ADSL_LED1_OFF ;
	pContext->ADSL_LED2_Value = ADSL_LED2_OFF ;

#if CAL_P46_INSTALLED

	{
		typedef struct
		{
			CSR_COMMAND_PARAM_T	Csr_Command_Param ;
			WORD				Size ;
		} GPIO_INIT_T ;

		GPIO_INIT_T GPIO_Init_Table [] =
		{
			//GP IO Outputs	 (DSL_SET_IO)
			//
			//  Command		Number	 Value		Call  Parm		Size
			//----------------------------
			{ { DSL_SET_IO, {{SSCAN,   0 }},		NULL, NULL },	sizeof (DWORD) + sizeof ( COMMAND_SET_IO_T ) },
			{ { DSL_SET_IO, {{FPWRDWN, 1 }},		NULL, NULL },	sizeof (DWORD) + sizeof ( COMMAND_SET_IO_T ) },
			{ { DSL_SET_IO, {{FPWRDWN, 0 }},		NULL, NULL },	sizeof (DWORD) + sizeof ( COMMAND_SET_IO_T ) },
			{ { DSL_SET_IO, {{nDRVPWR, 1 }},		NULL, NULL },	sizeof (DWORD) + sizeof ( COMMAND_SET_IO_T ) },
			{ { DSL_SET_IO, {{AFEPOR,  1 }},		NULL, NULL },	sizeof (DWORD) + sizeof ( COMMAND_SET_IO_T ) },
			{ { DSL_SET_IO, {{nFRST,   1 }},		NULL, NULL },	sizeof (DWORD) + sizeof ( COMMAND_SET_IO_T ) },

			// Repeat last command with the call back to the CardAL completion
			// routine. The callback is called after the cmd is read by device,
			// but we do not know if the device has completed processing except by a
			// response (which does not exist for this command) or when it reads
			// the next command. Therefor once this last repeated command is
			// read, we are guaranteed that the previous original is processed.
			{ { DSL_SET_IO, {{nFRST, 1 }},		chipAlTigrisHalDeviceRdyNotify,
			    pContext },
			  sizeof (DWORD) + sizeof ( COMMAND_SET_IO_T ) }

			//GP IO Interrupt Enable Register (DSL_SET_IO_CHANGE_IND)
			//(Valid only when line is configured as input via GP_DIR)
			//(0 = Disable    1 = Enable)
			//(We don't use interrupts - leave to defaults of disabled)

		};


		WORD Tbl_Index ;

		for
		(
			Tbl_Index =  0 ;
			Tbl_Index < sizeof (GPIO_Init_Table) / sizeof (GPIO_INIT_T) ;
			Tbl_Index++
		)
		{
			P46_IO_COMMAND_T			Command;
			NTSTATUS					Status;

			Command.CsrParam = GPIO_Init_Table[Tbl_Index].Csr_Command_Param ;

			Status =	ChipALWrite(
							pThisAdapter,
							P46_ARM_DSL,
							ADDRESS_BAR_0,
							CSR_ARM_CMD,
							GPIO_Init_Table[Tbl_Index].Size,	// Not used at this time
							(DWORD)&Command,
							0 					// Mask	Value is unused	by CSR_ARM_CMD
						);

			//if(!NT_SUCCESS(Status))
			//{
			//	return pContext;
			//}
		}
	}


	// Setup to get indications of when SwitchHook Detect changes
	// Hook in handler for GPIO change response
	Status =	ChipALAddEventHandler
				(
					pThisAdapter,
					DSL_FUNCTION,
					HW_IO_CHANGE_IND,
					HW_ISR_EVENT,
					chipAlIoChangeIndIsrHdlr,
					pThisAdapter
				);


	{
		P46_IO_COMMAND_T			Command;
		NTSTATUS					Status;

		CLEAR_MEMORY ( &Command.CsrParam, sizeof ( CSR_COMMAND_PARAM_T ) ) ;
		Command.CsrParam.Command = DSL_SET_IO_CHANGE_IND ;
		Command.CsrParam.Param.IoChange.Number	= SWITCHHOOK ;
		Command.CsrParam.Param.IoChange.ChangeMode	= GPIO_CHANGE_ON ;

		Status =	ChipALWrite(
						pThisAdapter,
						P46_ARM_DSL,
						ADDRESS_BAR_0,
						CSR_ARM_CMD,
						sizeof (GPIO_CHANGE_INDICATION_T),	// Not used at this time
						(DWORD) &Command,
						0 							// Mask	Value is unused	by CSR_ARM_CMD
					);
	}

#endif		// #if CAL_P46_INSTALLED

	// Initialize the interface between Pairgain Data Pump and this
	ChipAlTigrisDPInit ( pThisAdapter, pContext ) ;


	return pContext ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalShutdown

ABSTRACT:
	Free allocated resources


RETURN:
	NA.

******************************************************************************/

MODULAR VOID ChipAlTigrisHalShutdown
(
	IN const VOID				* UserHandle
)
{
	TIGRIS_HAL_CONTEXT_T		* pContext ;
	CDSL_ADAPTER_T				* pThisAdapter ;
	CHIPAL_T					* pThisInterface ;

	pContext = (TIGRIS_HAL_CONTEXT_T *) UserHandle ;

	// if context is null then I was never initialized so nothing to do
	if ( !pContext )
	{
		return;
	}

	pThisInterface = pContext->pInterface ;
	pThisAdapter = pThisInterface->pNdisAdapter ;

	DBG_CDSL_DISPLAY
	(
		DBG_LVL_MISC_INFO,
		pThisAdapter->DebugFlag,
		("ChipAlTigrisHalShutdown:  Entry;")
	) ;

	ChipALRemoveEventHandler
	(
		pThisAdapter,
		DSL_FUNCTION,
		HW_IO_CHANGE_IND,
		HW_ISR_EVENT,
		chipAlIoChangeIndIsrHdlr
	);


	//*
	//* Free Context Structue
	//*
	FREE_MEMORY( pContext, sizeof(TIGRIS_HAL_CONTEXT_T), 0 );
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalSetRstXcvr

ABSTRACT:
	Sets or Resets the transceiver


RETURN:
	NA.


DETAILS:
******************************************************************************/
MODULAR VOID ChipAlTigrisHalSetRstXcvr
(
	IN CDSL_ADAPTER_T			* pThisAdapter,
	IN BOOLEAN					  state
)
{
#if CAL_P46_INSTALLED

	P46_IO_COMMAND_T			Command;
	NTSTATUS					Status;

	CLEAR_MEMORY ( &Command.CsrParam, sizeof ( CSR_COMMAND_PARAM_T ) ) ;
	Command.CsrParam.Command			= DSL_SET_IO;
	Command.CsrParam.Param.SetIo.Number	= nFRST;
	Command.CsrParam.Param.SetIo.Value	= state;

	Status =	ChipALWrite(
					pThisAdapter,
					P46_ARM_DSL,
					ADDRESS_BAR_0,
					CSR_ARM_CMD,
					sizeof (COMMAND_SET_IO_T),	// Not used at this time
					(DWORD) &Command,
					0 							// Mask	Value is unused	by CSR_ARM_CMD
				);

	if(!NT_SUCCESS(Status))
	{
		return;
	}

#endif		// #if CAL_P46_INSTALLED
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisSetPwrDwnXcvr

ABSTRACT:
	Sets or Resets the Falcon PowerDown pin


RETURN:
	NA.


DETAILS:
******************************************************************************/
MODULAR VOID ChipAlTigrisSetPwrDwnXcvr
(
	IN CDSL_ADAPTER_T			* pThisAdapter,
	IN BOOLEAN					  state
)
{
#if CAL_P46_INSTALLED

	P46_IO_COMMAND_T			Command;
	NTSTATUS					Status;

	CLEAR_MEMORY ( &Command.CsrParam, sizeof ( CSR_COMMAND_PARAM_T ) ) ;
	Command.CsrParam.Command			= DSL_SET_IO;
	Command.CsrParam.Param.SetIo.Number	= FPWRDWN;
	Command.CsrParam.Param.SetIo.Value	= state;

	Status =	ChipALWrite(
					pThisAdapter,
					P46_ARM_DSL,
					ADDRESS_BAR_0,
					CSR_ARM_CMD,
					sizeof (COMMAND_SET_IO_T),	// Not used at this time
					(DWORD) &Command,
					0 							// Mask	Value is unused	by CSR_ARM_CMD
				);

	if(!NT_SUCCESS(Status))
	{
		return;
	}

#endif		// #if CAL_P46_INSTALLED
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisSetPwrDwnXcvr

ABSTRACT:
	Sets or Resets the Falcon PowerDown pin


RETURN:
	NA.


DETAILS:
******************************************************************************/
MODULAR VOID ChipAlTigrisSetGpio
(
	IN CDSL_ADAPTER_T			* pThisAdapter,
	IN BYTE						  Pin,
	IN BOOLEAN					  state
)
{
#if CAL_P46_INSTALLED

	P46_IO_COMMAND_T			Command;
	NTSTATUS					Status;

	CLEAR_MEMORY ( &Command.CsrParam, sizeof ( CSR_COMMAND_PARAM_T ) ) ;
	Command.CsrParam.Command			= DSL_SET_IO;
	Command.CsrParam.Param.SetIo.Number	= Pin;
	Command.CsrParam.Param.SetIo.Value	= state;

	Status =	ChipALWrite(
					pThisAdapter,
					P46_ARM_DSL,
					ADDRESS_BAR_0,
					CSR_ARM_CMD,
					sizeof (COMMAND_SET_IO_T),	// Not used at this time
					(DWORD) &Command,
					0 							// Mask	Value is unused	by CSR_ARM_CMD
				);

	if(!NT_SUCCESS(Status))
	{
		return;
	}

#endif		// #if CAL_P46_INSTALLED
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalSetAFEHybridSelect

ABSTRACT:
	Sets or Resets the AFE Hybrid Select lines


RETURN:
	NA.


DETAILS:
******************************************************************************/
MODULAR VOID ChipAlTigrisHalSetAFEHybridSelect
(
	IN CDSL_ADAPTER_T			* pThisAdapter,
	IN UINT8					  Select_Line,
	IN BOOLEAN					  State
)
{
#if CAL_P46_INSTALLED

	P46_IO_COMMAND_T			Command;
	NTSTATUS					Status;

	CLEAR_MEMORY ( &Command.CsrParam, sizeof ( CSR_COMMAND_PARAM_T ) ) ;
	Command.CsrParam.Command			= DSL_SET_IO;
	switch (Select_Line)
	{
	case 1:
		Command.CsrParam.Param.SetIo.Number	= HYBSEL1;
		break;
	case 2:
		Command.CsrParam.Param.SetIo.Number	= HYBSEL2;
		break;
	}
	Command.CsrParam.Param.SetIo.Value	= State;


	Status =	ChipALWrite(
					pThisAdapter,
					P46_ARM_DSL,
					ADDRESS_BAR_0,
					CSR_ARM_CMD,
					sizeof (COMMAND_SET_IO_T),	// Not used at this time
					(DWORD) &Command,
					0 							// Mask	Value is unused	by CSR_ARM_CMD
				);

	if(!NT_SUCCESS(Status))
	{
		return;
	}

#endif		// #if CAL_P46_INSTALLED
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalEnableIRQ1_2

ABSTRACT:
	Enables IRQ1 and IRQ2


RETURN:
	NA.


DETAILS:
******************************************************************************/
MODULAR VOID ChipAlTigrisHalEnableIRQ1_2
(
	IN CDSL_ADAPTER_T			* pThisAdapter
)
{
	CHIPAL_T					* pThisInterface ;
	TIGRIS_HAL_CONTEXT_T		* pContext ;

	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod ;
	pContext = ( TIGRIS_HAL_CONTEXT_T *) pThisInterface->ServiceContext[CAL_SERVICE_FALCON] ;

	if (pThisInterface->BusControl != NULL)
	{
		(* pThisInterface->BusControl->Enable)
		(
			pThisInterface->BusResources,
			DSL_FUNCTION,
			INT_FAL_IRQ_1 | INT_FAL_IRQ_2
		) ;
	}
	else
	{
		(*pThisAdapter->pChipALRemoteInf->CALIntf.pCalEnableInterrupt)
		(pThisAdapter->pChipALRemoteInf->CALIntf.iface.Context,
		 INT_FAL_IRQ_1 | INT_FAL_IRQ_2);
	}
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalDisableIRQ1_2

ABSTRACT:
	Disables IRQ1 and IRQ2


RETURN:
	NA.


DETAILS:
	Called from ChipAlTigrisHalDisableIRQ1_2 via NdisMSynchronizeWithInterrupt
	so that this runs at DIRQL
******************************************************************************/

GLOBAL VOID ChipAlTigrisHalDisableIRQ1_2_ISR
(
	IN CDSL_ADAPTER_T			* pThisAdapter
)
{
	CHIPAL_T					* pThisInterface ;
	TIGRIS_HAL_CONTEXT_T		* pContext ;

	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod ;
	pContext = ( TIGRIS_HAL_CONTEXT_T *) pThisInterface->ServiceContext[CAL_SERVICE_FALCON] ;



	if (pThisInterface->BusControl != NULL)
	{

		(* pThisInterface->BusControl->Disable)
		(
			pThisInterface->BusResources,
			DSL_FUNCTION,
			INT_FAL_IRQ_1 | INT_FAL_IRQ_2
		) ;
	}
	else
	{
		(*pThisAdapter->pChipALRemoteInf->CALIntf.pCalDisableInterrupt)
		(pThisAdapter->pChipALRemoteInf->CALIntf.iface.Context,
		 INT_FAL_IRQ_1 | INT_FAL_IRQ_2);
	}
}




/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalDisableIRQ1_2

ABSTRACT:
	Disables IRQ1 and IRQ2

RETURN:
	NA.

DETAILS:
******************************************************************************/

MODULAR VOID ChipAlTigrisHalDisableIRQ1_2
(
	CDSL_ADAPTER_T				* pThisAdapter
)
{
	CHIPAL_T					* pThisInterface ;

	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod ;



	// Twiddle the bits at interrupt level (DIRQL) cause this involves a read/modify/write operation
	/*
	NdisMSynchronizeWithInterrupt
	(
		&pThisInterface->BusResources->PriIntCtrl,
		ChipAlTigrisHalDisableIRQ1_2_ISR,
		pThisAdapter
	) ;
	*/

	ChipAlTigrisHalDisableIRQ1_2_ISR(pThisAdapter);

}


/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalSetAfePrint

ABSTRACT:
	Turn On/Off printing of all AFE writes


RETURN:
	NA.


DETAILS:
******************************************************************************/

GLOBAL VOID ChipAlTigrisHalSetAfePrint
(
	IN CDSL_ADAPTER_T			* pThisAdapter,
	IN BOOLEAN					  Setting
)
{
	CHIPAL_T					* pThisInterface ;
	TIGRIS_HAL_CONTEXT_T		* pContext ;

	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod ;
	pContext = ( TIGRIS_HAL_CONTEXT_T *) pThisInterface->ServiceContext[CAL_SERVICE_FALCON] ;

	pContext->AFE_Print = Setting ;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalWriteAfe

ABSTRACT:
	Write data to AFE at specified location


RETURN:
	NA.


DETAILS:
******************************************************************************/

MODULAR VOID ChipAlTigrisHalWriteAfe
(
	IN CDSL_ADAPTER_T			* pThisAdapter,
	IN BYTE						  Address,
	IN BYTE						  Data
)
{
	DSL_AFEC_T					  DSL_AFEC_Value ;
	CHIPAL_T					* pThisInterface ;
	TIGRIS_HAL_CONTEXT_T		* pContext ;

	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod ;
	pContext = ( TIGRIS_HAL_CONTEXT_T *) pThisInterface->ServiceContext[CAL_SERVICE_FALCON] ;




	// build the control register value
	DSL_AFEC_Value.DWord = 0 ;

	DSL_AFEC_Value.Bit.AFESTART = 1 ;
	DSL_AFEC_Value.Bit.ADCREAD = 0 ;
	DSL_AFEC_Value.Bit.AFERWN = 0 ;
	DSL_AFEC_Value.Bit.AFEADD = Address ;
	DSL_AFEC_Value.Bit.AFEDAT = Data ;

	// write the control register
	ChipALWrite
	(
		pThisAdapter,
		P46_DEVICE_AFE,
		ADDRESS_BAR_AFE,
		DSL_AFEC_OFFSET,
		sizeof ( DSL_AFEC_T ),
		DSL_AFEC_Value.DWord,
		0
	) ;

	if ( pContext->AFE_Print )
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			ChipALDebugFlag,
			("AFE Write: Reg %d = 0x%x", Address, Data));
	}
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalPendingReadAfe

ABSTRACT:
	Send a read data command to AFE at specified location


RETURN:
	NA.


DETAILS:
******************************************************************************/
MODULAR VOID ChipAlTigrisHalPendingReadAfe
(
	IN CDSL_ADAPTER_T			* pThisAdapter,
	IN BYTE						  Address
)
{
	DSL_AFEC_T					  DSL_AFEC_Value ;


	// build the control register value
	DSL_AFEC_Value.DWord = 0 ;

	DSL_AFEC_Value.Bit.AFESTART = 1 ;
#ifdef CONEXANT_AFE
	DSL_AFEC_Value.Bit.ADCREAD = 0 ;
	DSL_AFEC_Value.Bit.AFERWN = 1 ;			// read request
#endif
	DSL_AFEC_Value.Bit.AFEADD = Address ;
	DSL_AFEC_Value.Bit.AFEDAT = 0 ;

	// write the control register
	ChipALWrite
	(
		pThisAdapter,
		P46_DEVICE_AFE,
		ADDRESS_BAR_AFE,
		DSL_AFEC_OFFSET,
		sizeof ( DSL_AFEC_T ),
		DSL_AFEC_Value.DWord,
		0
	) ;

}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalIsAfeDone

ABSTRACT:
	Return whether previous write/read has completed


RETURN:
	NA.


DETAILS:
******************************************************************************/
MODULAR BOOLEAN ChipAlTigrisHalIsAfeDone
(
	IN CDSL_ADAPTER_T			* pThisAdapter
)
{
	DSL_AFEC_T					  DSL_AFEC_Value ;
	CHIPAL_T					* pThisInterface ;
	TIGRIS_HAL_CONTEXT_T		* pContext ;

	pThisInterface = (CHIPAL_T *) pThisAdapter->ChipMod ;
	pContext = ( TIGRIS_HAL_CONTEXT_T *) pThisInterface->ServiceContext[CAL_SERVICE_FALCON] ;



	ChipALRead
	(
		pThisAdapter,
		P46_DEVICE_AFE,
		ADDRESS_BAR_AFE,
		DSL_AFEC_OFFSET,
		sizeof ( DSL_AFEC_T ),
		&DSL_AFEC_Value.DWord
	) ;

	if ( DSL_AFEC_Value.Bit.AFESTART == 0 )
	{
		return TRUE ;
	}
	else
	{
		return FALSE ;
	}

}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalReadAfe

ABSTRACT:
	Reads AFE data at address specified in ChipAlTigrisHalPendingReadAfe()


RETURN:
	NA.


DETAILS:
******************************************************************************/
MODULAR BYTE ChipAlTigrisHalReadAfe
(
	IN CDSL_ADAPTER_T			* pThisAdapter
)
{
	DSL_AFEC_T					  DSL_AFEC_Value ;

	ChipALRead
	(
		pThisAdapter,
		P46_DEVICE_AFE,
		ADDRESS_BAR_AFE,
		DSL_AFEC_OFFSET,
		sizeof ( DSL_AFEC_T ),
		&DSL_AFEC_Value.DWord
	) ;

	return (BYTE)DSL_AFEC_Value.Bit.AFEDAT;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalGetFalconSpace

ABSTRACT:
	write to an Actel Register


RETURN:
	NA.


DETAILS:
******************************************************************************/
MODULAR void * ChipAlTigrisHalGetFalconSpace
(
	IN CDSL_ADAPTER_T			* pThisAdapter
)
{
	CAL_CONFIG_T				  ChipAlConfig;
	NDIS_STATUS					  Status ;


	ChipAlConfig.ModuleType = MODULE_TYPE_BUS_CONTROLLER;
	ChipAlConfig.Id = 0;				// Only one Bus Controller available
	Status =	ChipAlGetConfig
				(
					pThisAdapter,
					&ChipAlConfig
				);
	if (Status != STATUS_SUCCESS)
	{
		return NULL ;
	}

	// Init Falcon_Space
	// P46_DEVICE_ADSL_CSR is defined as BAR0 or BAR1 (virtual address) in ChipALFalconHalv.h
	return (BYTE *)ChipAlConfig.Params.BusConfig.P46_BAR_MICRO + MICRO_OFFSET;
}



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalTraceWrite

ABSTRACT:
	Write to Debug Trace Location


RETURN:
	NA.


DETAILS:
******************************************************************************/
MODULAR VOID ChipAlTigrisHalTraceWrite
(
	IN CDSL_ADAPTER_T			* pThisAdapter,
	IN BYTE						Offset,
	IN DWORD					Value
)
{
#if CAL_P46_INSTALLED

	NTSTATUS					Status;

	if (Offset <= 15)
	{
		Status =	ChipALWrite(
						pThisAdapter,
						P46_ARM_DSL,
						ADDRESS_BAR_0,
						(CSR_ARM_DEBUG_BASE + (Offset<<2)),
						sizeof (DWORD),				// Not used at this time
						(DWORD) Value,
						0 							// Mask	Value is unused	by CSR_ARM_CMD
					);

		if(!NT_SUCCESS(Status))
		{
			return;
		}
	}

#endif		// #if CAL_P46_INSTALLED
}



/******************************************************************************
FUNCTION NAME:
	chipAlTigrisHalSetWiringCombo

ABSTRACT:
	Write to all ADSL wiring relays in the list
NOTES:
	This routine only energizes or de-energizes the relays as provided for in
	the Relay_Value parm.
RETURN:
	.

******************************************************************************/
static void chipAlTigrisHalSetWiringCombo
(
	IN CDSL_ADAPTER_T				* pThisAdapter,
	UINT8							* pGpio_List,
	BOOLEAN							  Relay_Value
)
{
	UINT8 Last_Gpio ;
	UINT8 Index ;

	// set each hardware GPIO
	Last_Gpio = WIRING_COMBO_AUTO ;	//this value chosen so as to NEVER match on first loop
	for ( Index = 0 ; Index < AUTOSENSE_WIRING_COMBO_MAX_NUM_GPIO ; Index ++ )
	{
		UINT8						  Relay_Gpio ;
		P46_IO_COMMAND_T			  Command ;
		NTSTATUS					  Status ;
	
		// pick off next GPIO to be set
		Relay_Gpio = pGpio_List[Index] ;

		//Stop if we are repeating ourselves
		if ( Relay_Gpio == Last_Gpio )
		{
			break ;
		}

		// build command to Energize the relay to the desired wiring pair
		CLEAR_MEMORY ( &Command.CsrParam, sizeof ( CSR_COMMAND_PARAM_T ) ) ;
		Command.CsrParam.Command			= DSL_SET_IO ;
		Command.CsrParam.Param.SetIo.Number	= Relay_Gpio ;
		Command.CsrParam.Param.SetIo.Value	= Relay_Value ;
		 
		// Send command to Energize the relay to the desired wiring pair
		Status = ChipALWrite
		(
			pThisAdapter,
			P46_ARM_DSL,
			ADDRESS_BAR_0,
			CSR_ARM_CMD,
			sizeof (COMMAND_SET_IO_T),	// Not used at this time
			(DWORD) &Command,
			0 							// Mask	Value is unused	by CSR_ARM_CMD
		) ;
	
		DBG_CDSL_DISPLAY
		(
			DBG_LVL_MISC_INFO,
			pThisAdapter->DebugFlag,
			("chipAlTigrisHalSetWiringCombo:  Set GPIO %d = %d", Relay_Gpio, Relay_Value )
		) ;

		if ( ! NT_SUCCESS (Status) && Status != STATUS_PENDING )
		{
			DBG_CDSL_DISPLAY
			(
				DBG_LVL_ERROR,
				pThisAdapter->DebugFlag,
				("chipAlTigrisHalSetWiringCombo:  ChipALWrite Failure!")
			) ;
			return ;
		}

		// remember this gpio to use for the comparison next time through loop
		Last_Gpio = Relay_Gpio ; 
	}
}	



/******************************************************************************
FUNCTION NAME:
	ChipAlTigrisHalSetWiring

ABSTRACT:
	Set the ADSL wiring relay to the desired pair
NOTES:
	The wiring selection is an enumerated list. The enumerated values are
	AUTO, COMBO1, COMBO2...  The value AUTO should not be passed to this routine.
	When AUTO is being used, pass the current automatic selection (COMBO1, COMBO2 etc).
	When a manual setting is being used, pass this setting and set the parm
	Automatic to FALSE. When you set the parm Automatic to TRUE, we use the
	wiring selection as an index in to a ordered list of combinations.

	For example, assuming COMBO1 is setting A and COMBO2 is setting B, etc. when
	we see Automatic = FALSE and Wiring_Selection = COMBO1 we use setting A.
	Now assume that the ordered list of combinations is "C, B, A". If Automatic
	= TRUE and Wiring_Selection = COMBO1, we take COMBO1 to mean the first 
	setting in the ordered list (i.e. "A").
RETURN:
	.

******************************************************************************/
#include "dmtdbg.h"

GLOBAL void ChipAlTigrisHalSetWiring
(
	IN CDSL_ADAPTER_T				* pThisAdapter,
	IN BOOLEAN						  Automatic,
	IN SYS_WIRING_SELECTION_TYPE	  Wiring_Selection
)
{
#if CAL_P46_INSTALLED

	CARDAL_ADAPTER_T				* pCardALAdapter ;
	UINT8							* Gpio_List ;
	BOOLEAN							  Relay_Value ;
	WORD							  Relay_Delay ;
	WORD							  Num_Combos ;

	// *
	// * Range Check
	// *
	if ( pThisAdapter == NULL )
	{
		DBG_CDSL_DISPLAY
		(
			DBG_LVL_ERROR,
			pThisAdapter->DebugFlag,
			("ChipAlTigrisHalSetWiring:  NULL Pointer")
		) ;
		return ;
	}

	pCardALAdapter  = (CARDAL_ADAPTER_T *) pThisAdapter->CardMod;
	if ( pCardALAdapter == NULL )
	{
		DBG_CDSL_DISPLAY
		(
			DBG_LVL_ERROR,
			pThisAdapter->DebugFlag,
			("ChipAlTigrisHalSetWiring:  NULL Pointer")
		) ;
		return ;
	}



	// get args together into simple variables
	#if defined(NDIS_MINIPORT_DRIVER) //Remove me...
		if ( Automatic )
		{
			// Selection = Order List [ Selection - COMBO1]
			Wiring_Selection = pThisAdapter->PersistData.AutoWiringOrder[Wiring_Selection-WIRING_COMBO1] ;
		}
		Gpio_List = pThisAdapter->PersistData.AutoWiringComboGPIO[Wiring_Selection-WIRING_COMBO1],
		Relay_Value = pThisAdapter->PersistData.AutoWiringRelayEnrg ;
		Relay_Delay = pThisAdapter->PersistData.AutoWiringRelayDelay ;
		Num_Combos = pThisAdapter->PersistData.AutoWiringNumCombos ;
	#else
		if ( Automatic )
		{
			// Selection = Order List [ Selection - COMBO1]
			Wiring_Selection = ((AUTOWIRING_ORDER_T *)	pCardALAdapter->Cfg[LC_AUTO_SENSE_WIRING_ORDER].pInfo)->Array[Wiring_Selection-WIRING_COMBO1] ;
		}
		Gpio_List = (UINT8 *) &((AUTOWIRING_COMBO_GPIO_T *) pCardALAdapter->Cfg[LC_AUTO_SENSE_WIRING_COMBO_GPIO].pInfo)->Array[Wiring_Selection-WIRING_COMBO1][0],
		Relay_Value = * (BOOLEAN *) pCardALAdapter->Cfg[LC_AUTO_SENSE_WIRING_RELAY_ENRG].pInfo ;
		Relay_Delay =  * (WORD *)	pCardALAdapter->Cfg[LC_AUTO_SENSE_WIRING_RELAY_DELAY].pInfo ;
		Num_Combos =  * (WORD *)	pCardALAdapter->Cfg[LC_AUTO_SENSE_WIRING_NUM_COMBOS].pInfo ;
	#endif

	// range check GPIO list value
	if ( ( Wiring_Selection < WIRING_COMBO1 )
	  || ( Wiring_Selection >= ( WIRING_COMBO1 + Num_Combos ) ) )
	{
		DBG_CDSL_DISPLAY
		(
			DBG_LVL_ERROR,
			pThisAdapter->DebugFlag,
			("chipAlTigrisHalSetWiring:  Invalid Wiring Selection! %d", Wiring_Selection )
		) ;
	}

	// Energize all relays in  ComboList[Selection-COMBO1] 
	chipAlTigrisHalSetWiringCombo
	(
		pThisAdapter,
		Gpio_List,
		Relay_Value
	) ;

	// Delay to give all relays time to switch
	CardALDelayMsec ( pThisAdapter, Relay_Delay ) ;
	
	// De-Energize all relays in  ComboList[Selection-COMBO1] 
	Relay_Value = ! Relay_Value ;
	chipAlTigrisHalSetWiringCombo
	(
		pThisAdapter,
		Gpio_List,
		Relay_Value
	) ;

#endif		// #if CAL_P46_INSTALLED
}
