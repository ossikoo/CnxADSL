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
**		CardAlTigrisDiag.c
**
**	ABSTRACT:
**		Optional Card Al Diagnostics.  Anything in this file is related to
** 		functions required for Applications (Control Panel, user.exe's, etc...).
** 		If an API interface is not required, this file is not necessary.
**		There should be not direct calls to this file.  It uses a vector 
**		table init for installation.
**
**		NOTE:  This file is Project / Hardware Specific
**
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.4.3_ATM_Driver/KernelModule/CardALTigrisDiag.c $
**	$Revision: 5 $
**	$Date: 7/23/01 5:24p $
*******************************************************************************
******************************************************************************/
#include "CardALTigrisV.h"
#include "CardALTigrisDiag.h"
#include "CardALBdDp.h"
#include "dmtapi.h"
#include "autosense.h"



#include "sysinfo.h"    // system information
#ifdef GHS_ENABLE
#include "ghsutil.h"
#endif


#define ERROR_FATAL(error)		(LEVEL_FATAL   | MODULE_CARDAL | INSTANCE_CARDAL | error)
#define ERROR_WARNING(error)	(LEVEL_WARNING | MODULE_CARDAL | INSTANCE_CARDAL | error)
#define ERROR_NORMAL(error)		(LEVEL_NORMAL  | MODULE_CARDAL | INSTANCE_CARDAL | error)

BD_DRIVER_LOG_T		BdAdslTextLog2; 		// * Legacy Stuff. Needs to be removed!!!!!!



/*******************************************************************************
Private Function Prototypes
*******************************************************************************/
GLOBAL NTSTATUS CdalDiagInitDataPump(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagGetDataPumpVer(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagGetConnConfig(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagSetConnConfig(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagGetModemData(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagGetDataPath(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagSetDataPath(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagGetLineStatus(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);


GLOBAL NTSTATUS CdalDiagGetLineState(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagGetRegister(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagSetRegister(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagActivateLine(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagDeactivateLine(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagGetBusCtrl(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagSetPmState(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagGetPmState(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagGetTrnStatus(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagGetAdslConfig(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalDiagSetAdslConfig(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);


GLOBAL NTSTATUS CdalDiagGetTxtLog(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);


GLOBAL NTSTATUS CdalDiagGetADSLPerf(
    VOID					* pContext,
    CHAR					* Buffer,
    DWORD					Length
);

/*******************************************************************************
Module Public Functions
*******************************************************************************/

/******************************************************************************
FUNCTION NAME:
	CdalDiagInit

ABSTRACT:
	Associate all CardAl optional Diagnostic routines to API requests


RETURN:
	!NULL if Success 
	
	NULL for failure

DETAILS:
******************************************************************************/
MODULAR VOID * CdalDiagInit(
    IN void		* pAdapterContext,
    IN VOID		* pCardAlContext
)
{
	NDIS_STATUS						Status;
	static VOID						* SysIfHandle;

	// *
	// * This table relates all API requests to a CardAl Function.  To process
	// * a new request, add an entry to this table and write the
	// * appropriate function.
	// *
	MESSAGE_HANDLER_T		HandlerTable[] =
	    {
	        /*	Message			Key						Len								Handler					Module Id  */
	        {MESS_API_COMMAND,	BD_ADSL_INIT_DATA_PUMP,	0l,								CdalDiagInitDataPump,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_GET_DP_VERSIONS,sizeof(BD_ADSL_DP_VERSIONS_T),	CdalDiagGetDataPumpVer,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_GET_CONN_CONFIG,sizeof(BD_ADSL_CONN_CONFIG_T),	CdalDiagGetConnConfig,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_GET_MODEM_DATA,	sizeof(BD_ADSL_MODEM_DATA_T),	CdalDiagGetModemData,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_GET_DATA_PATH_SELECTION, sizeof(BD_ADSL_DATA_PATH_SELECT_T),CdalDiagGetDataPath,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_SET_DATA_PATH_SELECTION, sizeof(BD_ADSL_DATA_PATH_SELECT_T),CdalDiagSetDataPath,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_GET_LINE_STATUS,sizeof(BD_ADSL_LINE_STATUS_T),	CdalDiagGetLineStatus,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_GET_LINE_STATE,	sizeof(BD_ADSL_LINE_STATE_T),	CdalDiagGetLineState,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_CARDAL_GET_REGISTER,	sizeof(BD_CARDAL_REGISTER_T),	CdalDiagGetRegister,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_CARDAL_SET_REGISTER,	sizeof(BD_CARDAL_REGISTER_T),	CdalDiagSetRegister,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_USER_ACTIVATE_LINE, 		0L,	CdalDiagActivateLine,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_USER_DEACTIVATE_LINE, 	0L,	CdalDiagDeactivateLine,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_GET_BUS_CONTROLLER, sizeof(BD_MINI_DATA_BLOCK_T), CdalDiagGetBusCtrl,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_PM_SET_POWER_STATE,	sizeof(BD_PM_POWER_STATE_T), CdalDiagSetPmState,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_PM_GET_POWER_STATE,	sizeof(BD_PM_POWER_STATE_T), CdalDiagGetPmState,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_CARDAL_GET_ADSL_TRANSCEIVER_STATUS, sizeof(BD_CARDAL_ADSL_TRANSCEIVER_STATUS_T), CdalDiagGetTrnStatus,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_GET_CONFIG, 	sizeof(BD_ADSL_CFG_T), CdalDiagGetAdslConfig,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_SET_CONFIG, 	sizeof(BD_ADSL_CFG_T), CdalDiagSetAdslConfig,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_CARDAL_GET_TEXT_LOG, sizeof(BD_MINI_DATA_BLOCK_DWORDS_T), CdalDiagGetTxtLog,	IF_CARDAL_ID},

	        {MESS_API_COMMAND,	BD_ADSL_SET_CONN_CONFIG, sizeof(BD_ADSL_CONN_CONFIG_T),	CdalDiagSetConnConfig,	IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_GET_PERFORMANCE, sizeof(BD_ADSL_PERFORMANCE_T), CdalDiagGetADSLPerf,	IF_CARDAL_ID}

	    };


	Status = CardALTextLogMsgQInit();
	if (Status != STATUS_SUCCESS)
	{
		return(NULL);
	}

	Status = SmSysIfGetHandle(pAdapterContext, &SysIfHandle);
	if (Status != STATUS_SUCCESS)
	{
		return(NULL);
	}

	// * Pass Function Table to System Interface
	Status = SmSysIfAddMessageTable(SysIfHandle, HandlerTable, pCardAlContext, sizeof(HandlerTable) / sizeof(MESSAGE_HANDLER_T));

	if (Status != STATUS_SUCCESS)
	{
		return(NULL);
	}

	return((void*)!NULL);
}

/*******************************************************************************
Private Functions
*******************************************************************************/

/******************************************************************************
FUNCTION NAME:
	CdalDiagInitDataPump

ABSTRACT:
	Initialize Data Pump from Cold state


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagInitDataPump(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS Status;
	CDSL_ADAPTER_T		* pThisAdapter = pCardALAdapter->pCdslAdapter;

	Status = CDALSetLineState(pThisAdapter,CDAL_LINE_STATE_PERSIST_INITIALIZE);

	return (Status);
}


/******************************************************************************
FUNCTION NAME:
	CdalDiagGetDataPumpVer

ABSTRACT:
	Get Data Pump Version Information


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagGetDataPumpVer(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					Status;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	BD_ADSL_DP_VERSIONS_T		* pBdAdslDpVersStruc;

	pBdAdslDpVersStruc = (BD_ADSL_DP_VERSIONS_T	*) Buffer;
	pBdAdslDpVersStruc->AdslVersionNear = UNKNOWN_8BIT_VALUE;
	pBdAdslDpVersStruc->AdslVersionFar = UNKNOWN_8BIT_VALUE;
	pBdAdslDpVersStruc->AdslVendorNear = UNKNOWN_16BIT_VALUE;
	pBdAdslDpVersStruc->AdslVendorFar = UNKNOWN_16BIT_VALUE;
	pBdAdslDpVersStruc->AdslDpSwVerMajor = UNKNOWN_4BIT_VALUE;
	pBdAdslDpVersStruc->AdslDpSwVerMinor = UNKNOWN_4BIT_VALUE;
	pBdAdslDpVersStruc->AdslDpSwVerSubMinor = UNKNOWN_8BIT_VALUE;

	Status = cardALChipGetDpVersions(
	             pThisAdapter,
	             pCardALAdapter,
	             pBdAdslDpVersStruc );

	return Status;
}


/******************************************************************************
FUNCTION NAME:
	CdalDiagGetConnConfig

ABSTRACT:
	Get Data Pump Connection Configuration


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagGetConnConfig(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					Status = RESULT_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	BD_ADSL_CONN_CONFIG_T		* pBdAdslConnConfig = (BD_ADSL_CONN_CONFIG_T *)Buffer;
	DWORD						VcIndex;


	if ( DEFAULT_NUMBER_OF_LINES <= MAX_VC_PER_LINK )
	{
		pBdAdslConnConfig->NumOfVc = DEFAULT_NUMBER_OF_LINES;
		if ( _MAX_LINKS == 1 )
		{
			for
			(
			    VcIndex = 0;
			    (
			        (VcIndex < DEFAULT_NUMBER_OF_LINES)
			        &&
			        (Status == RESULT_SUCCESS)
			    );
			    VcIndex++
			)
			{
				pBdAdslConnConfig->VcArray[VcIndex].Vpi =
				    pThisAdapter->AdapterLink.VC_Info[VcIndex].Vpi;
				pBdAdslConnConfig->VcArray[VcIndex].Vci =
				    pThisAdapter->AdapterLink.VC_Info[VcIndex].Vci;
				pBdAdslConnConfig->VcArray[VcIndex].Pcr =
				    pThisAdapter->AdapterLink.VC_Info[VcIndex].Pcr;
			}
		}
		else
		{
			Status = ERROR_WARNING( RESULT_DATA_FAILURE );
		}
	}
	else
	{
		Status = ERROR_WARNING( RESULT_DATA_FAILURE );
	}

	return Status;

}

/******************************************************************************
FUNCTION NAME:
	CdalDiagSetConnConfig

ABSTRACT:
	Set ADSL Connection Parameters


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagSetConnConfig(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					RtnResultValue = RESULT_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	BD_ADSL_CONN_CONFIG_T		* pBdAdslConnConfig = (BD_ADSL_CONN_CONFIG_T *)Buffer;
	DWORD						VcIndex;

	RtnResultValue = RESULT_SUCCESS;
	if ( pBdAdslConnConfig->NumOfVc <= DEFAULT_NUMBER_OF_LINES )
	{
		if ( _MAX_LINKS == 1 )
		{
			for
			(
			    VcIndex = 0;
			    (
			        (VcIndex < pBdAdslConnConfig->NumOfVc)
			        &&
			        (RtnResultValue == RESULT_SUCCESS)
			    );
			    VcIndex++
			)
			{
				pThisAdapter->AdapterLink.VC_Info[VcIndex].Vpi =
				    pBdAdslConnConfig->VcArray[VcIndex].Vpi;
				pThisAdapter->AdapterLink.VC_Info[VcIndex].Vci =
				    pBdAdslConnConfig->VcArray[VcIndex].Vci;
				pThisAdapter->AdapterLink.VC_Info[VcIndex].Pcr =
				    pBdAdslConnConfig->VcArray[VcIndex].Pcr;
			}
		}
		else
		{
			RtnResultValue = ERROR_WARNING( RESULT_DATA_FAILURE );
		}
	}
	else
	{
		RtnResultValue = ERROR_WARNING( RESULT_DATA_FAILURE );
	}

	return RtnResultValue;
}


/******************************************************************************
FUNCTION NAME:
	CdalDiagGetModemData

ABSTRACT:
	Read ram from Data Pump Memory


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagGetModemData(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					RtnResultValue = RESULT_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	BD_ADSL_MODEM_DATA_T		* pBdAdslModemData = (BD_ADSL_MODEM_DATA_T *)Buffer;


	RtnResultValue = cardALChipUpldCodeFromDP(
	                     pThisAdapter,
	                     pCardALAdapter,
	                     pBdAdslModemData->ModemBlockStartAddr,
	                     &pBdAdslModemData->ModemBlockLength,
	                     &pBdAdslModemData->ModemBlockData[0] );

	return RtnResultValue;

}


/******************************************************************************
FUNCTION NAME:
	CdalDiagGetDataPath

ABSTRACT:
	Determine ADSL Data Path Configuation Parameters


RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagGetDataPath(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					RtnResultValue = RESULT_SUCCESS;
	BD_ADSL_DATA_PATH_SELECT_T	* pBdAdslPathSelect = (BD_ADSL_DATA_PATH_SELECT_T *)Buffer;

	switch (pCardALAdapter->AdslDataPathSelected)
	{
	case ADSL_PATH_SELECT_AUTO:
		pBdAdslPathSelect->AdslDataPathSelection = BD_ADSL_PATH_SELECT_AUTOMATIC;
		break;
	case ADSL_PATH_SELECT_FAST:
		pBdAdslPathSelect->AdslDataPathSelection = BD_ADSL_PATH_SELECT_FAST;
		break;
	case ADSL_PATH_SELECT_INTERLEAVED:
		pBdAdslPathSelect->AdslDataPathSelection = BD_ADSL_PATH_SELECT_INTERLEAVED;
		break;
	default:
		pBdAdslPathSelect->AdslDataPathSelection = BD_ADSL_PATH_SELECT_UNKNOWN;
		break;
	}

	return RtnResultValue;

}

/******************************************************************************
FUNCTION NAME:
	CdalDiagSetDataPath

ABSTRACT:
	Set ADSL data path configuration parameters


RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagSetDataPath(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					RtnResultValue = RESULT_SUCCESS;
	BD_ADSL_DATA_PATH_SELECT_T	* pBdAdslPathSelect = (BD_ADSL_DATA_PATH_SELECT_T *)Buffer;

	switch (pBdAdslPathSelect->AdslDataPathSelection)
	{
	case BD_ADSL_PATH_SELECT_AUTOMATIC:
		pCardALAdapter->AdslDataPathSelected = ADSL_PATH_SELECT_AUTO;
		break;
	case BD_ADSL_PATH_SELECT_FAST:
		pCardALAdapter->AdslDataPathSelected = ADSL_PATH_SELECT_FAST;
		break;
	case BD_ADSL_PATH_SELECT_INTERLEAVED:
		pCardALAdapter->AdslDataPathSelected = ADSL_PATH_SELECT_INTERLEAVED;
		break;
	default:
		pCardALAdapter->AdslDataPathSelected = ADSL_PATH_SELECT_UNKNOWN;
		break;
	}

	return(RtnResultValue);
}

/******************************************************************************
FUNCTION NAME:
	CdalDiagGetLineStatus

ABSTRACT:
	Get Current ADSL Data Speed and Line Status


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagGetLineStatus(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					RtnResultValue = RESULT_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	BD_ADSL_LINE_STATUS_T		* pBdAdslLineStatus = (BD_ADSL_LINE_STATUS_T *)Buffer;

	RtnResultValue = CardALGetAdslLineSpeed(
	                     pThisAdapter,
	                     (WORD *) &pBdAdslLineStatus->LineSpeedUpOrFar,
	                     (WORD *) &pBdAdslLineStatus->LineSpeedDownOrNear );
	RtnResultValue |= CardALGetAdslLineState(
	                      pThisAdapter,
	                      &pBdAdslLineStatus->LineState );

	if ( RtnResultValue == STATUS_SUCCESS)
	{
		RtnResultValue = RESULT_SUCCESS;
	}
	else
	{
		RtnResultValue = ERROR_NORMAL( RESULT_DEVICE_FAILURE );
	}
	return(RtnResultValue);

}


/******************************************************************************
FUNCTION NAME:
	CdalDiagGetLineState

ABSTRACT:
	Get Current ADSL Line State


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagGetLineState(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					RtnResultValue = RESULT_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	BD_ADSL_LINE_STATE_T		* pBdAdslLineState = (BD_ADSL_LINE_STATE_T *)Buffer;

	RtnResultValue = CardALGetAdslLineState(
	                     pThisAdapter,
	                     &pBdAdslLineState->LineState );
	if ( RtnResultValue == STATUS_SUCCESS )
	{
		RtnResultValue = RESULT_SUCCESS;
	}
	else
	{
		RtnResultValue = ERROR_NORMAL( RESULT_DEVICE_FAILURE );
	}

	return(RtnResultValue);
}


/******************************************************************************
FUNCTION NAME:
	CdalDiagGetRegister

ABSTRACT:
	Read Hardware Register


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagGetRegister(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					RtnResultValue = RESULT_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	BD_CARDAL_REGISTER_T		* pBdCardALReg = (BD_CARDAL_REGISTER_T *)Buffer;


	RtnResultValue = cardALChipPeekReg( pThisAdapter, pCardALAdapter, pBdCardALReg );

	return RtnResultValue;

}


/******************************************************************************
FUNCTION NAME:
	CdalDiagSetRegister

ABSTRACT:
	Write Hardware Register


RETURN:
	Ntstatus


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagSetRegister(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					RtnResultValue = RESULT_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	BD_CARDAL_REGISTER_T		* pBdCardALReg = (BD_CARDAL_REGISTER_T *)Buffer;


	RtnResultValue = cardALChipPokeReg( pThisAdapter, pCardALAdapter, pBdCardALReg );


	return RtnResultValue;

}


/******************************************************************************
FUNCTION NAME:
	CdalDiagActivateLine

ABSTRACT:
	Set ADSL Line to Activated state


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagActivateLine(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					RtnResultValue;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;

	RtnResultValue = CDALSetLineState(pThisAdapter,CDAL_LINE_STATE_PERSIST_INITIALIZE);

	return RtnResultValue;

}


/******************************************************************************
FUNCTION NAME:
	CdalDiagDeactivateLine

ABSTRACT:
	Deactivate ADSL Line


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagDeactivateLine(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					RtnResultValue = RESULT_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;


	RtnResultValue = CDALSetLineState(pThisAdapter,	CDAL_LINE_STATE_INT);
	if ( RtnResultValue == RESULT_SUCCESS )
	{
		pThisAdapter->PersistData.LineAutoActivation = FALSE;
		pThisAdapter->PersistData.LinePersistentAct  = FALSE;
	}

	return RtnResultValue;

}


/******************************************************************************
FUNCTION NAME:
	CdalDiagGetBusCtrl

ABSTRACT:
	Determine the type of PCI / Bus Interface installed on Product


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagGetBusCtrl(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	NTSTATUS 					RtnResultValue = RESULT_SUCCESS;


	COPY_MEMORY(
	    Buffer,
	    BD_BUS_CTRLR_UNKNOWN,
	    sizeof(BD_BUS_CTRLR_UNKNOWN) );

	return RtnResultValue;

}


/******************************************************************************
FUNCTION NAME:
	CdalDiagSetPmState

ABSTRACT:
	Set Card Power Level


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagSetPmState(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					RtnResultValue;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	BD_PM_POWER_STATE_T			* pBdAdslPowerState = (BD_PM_POWER_STATE_T *)Buffer;


	RtnResultValue  = cardALChipSetPmState(
	                      pThisAdapter,
	                      pBdAdslPowerState);

	return(RtnResultValue);

}

/******************************************************************************
FUNCTION NAME:
	CdalDiagGetPmState

ABSTRACT:
	Get the Current Power Level.  This function should be moved to CardMgmt.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagGetPmState(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					RtnResultValue;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;

	RtnResultValue  = cardALChipGetPmState(
	                      pThisAdapter,
	                      (BD_PM_POWER_STATE_T *)Buffer );

	return(RtnResultValue);

}


/******************************************************************************
FUNCTION NAME:
	CdalDiagGetTrnStatus
	
ABSTRACT:
	Get The current Line Transceiver status

RETURN:
	NTSTATUS:


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagGetTrnStatus(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 				RtnResultValue;
	CDSL_ADAPTER_T			* pThisAdapter = pCardALAdapter->pCdslAdapter;


	RtnResultValue = cardALBdDpGetAdslTransceiverStatus(
	                     pThisAdapter,
	                     pCardALAdapter,
	                     (BD_CARDAL_ADSL_TRANSCEIVER_STATUS_T *)Buffer
	                 );

	return(RtnResultValue);

}


/******************************************************************************
FUNCTION NAME:
	CdalDiagGetAdslConfig

ABSTRACT:
	Get Line Modulation Rate
	Use Pairgain data to determine line modulation. w
RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagGetAdslConfig(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	NTSTATUS 					RtnResultValue = RESULT_SUCCESS;
	BD_ADSL_CFG_T				* pBdAdslConfig = (BD_ADSL_CFG_T *)Buffer;
	TIGRIS_CONTEXT_T			* pContext ;
	CDSL_ADAPTER_T				* pThisAdapter = ( CDSL_ADAPTER_T* )pAdapter;
	DATAPUMP_CONTEXT			* pDatapump ;
	SYS_HANDSHAKE_TYPE			Handshake;
	SYS_WIRING_SELECTION_TYPE	Wiring;
	SYS_AUTO_SENSING_STATE_TYPE	Autosense_State;
	XCVR_OTHER_CFG_TYPE   		GhsCfg;
	GHS_DB_TYPE 				*pghs ;
	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;
	pghs = &pDatapump->ghs_db;


	//printk(KERN_ALERT "CnxADSL:  CdalDiagGetAdslConfig START %8.8x %8.8x %8.8x\n", pAdapter, Buffer, Length);

	SYS_GetSysNvram(pDatapump, NEAR_END,NVRAM_XCVR_OTHER_CFG,(void *)&GhsCfg);

	Auto_GetAutoSenseStatus(	pDatapump,
								&Handshake,
								&Wiring,
								&Autosense_State);

	if (Handshake == G_TONE_MODE)
	{
		pBdAdslConfig->AdslDpRateMode = BD_ADSL_DP_RATE_T1_DOT_413;
	}
	else
	{
		pBdAdslConfig->AdslDpRateMode =  BD_ADSL_DP_RATE_G_DOT_DMT;
		// test bits 922.1 annex a = true and 922.2 annexa/b = false
		// for g.dmt


		// test bits 922.1 annex a = false and 922.2 annexa/b = true
		// for g.lite
		if ((!(pghs->local_capabilities.standards & 0x1)) && (pghs->local_capabilities.standards & 0x8))
		{
			pBdAdslConfig->AdslDpRateMode =  BD_ADSL_DP_RATE_G_DOT_LITE;
		}
		else
			// test bits 922.1 annex a = true and 922.2 annexa/b = true
			// look at negotiated
			if ((pghs->local_capabilities.standards & 0x1) && (pghs->local_capabilities.standards & 0x8))
			{
				if ((!(pghs->negotiated_capabilities.standards & 0x1)) && (pghs->negotiated_capabilities.standards & 0x8))
				{
					pBdAdslConfig->AdslDpRateMode =  BD_ADSL_DP_RATE_G_DOT_LITE;
				}
			}
	}

	//printk(KERN_ALERT "CnxADSL:  CdalDiagGetAdslConfig END\n", pAdapter, Buffer, Length);

	return(RtnResultValue);
}

/******************************************************************************
FUNCTION NAME:
	CdalDiagSetAdslConfig

ABSTRACT:
	Set Line Modulation Rate

	NOTE:  This Needs to be implemented.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagSetAdslConfig(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	//	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 				RtnResultValue = ERROR_NORMAL(RESULT_PARAM_OUT_OF_RANGE);
	//	CDSL_ADAPTER_T			* pThisAdapter = pCardALAdapter->pCdslAdapter;
	//	BD_ADSL_CFG_T			* pBdAdslConfig = (BD_ADSL_CFG_T *)Buffer;


	return(RtnResultValue);
}

/******************************************************************************
FUNCTION NAME:
	CdalDiagGetTxtLog

ABSTRACT:
	Get Auto Logger Data.  This needs to be moved to CardMgmt.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
GLOBAL NTSTATUS CdalDiagGetTxtLog(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	NTSTATUS 					RtnResultValue = RESULT_SUCCESS;
	DWORD						* pBdAdslTextLogs = (DWORD *)Buffer;
	DWORD						LogMsgSizeInDwords;
	DWORD						BdAdslTextLogIndex;
	DWORD						* pdwArray;
	DWORD						Ndx;

	for ( Ndx = 0;   Ndx < BD_MINI_DATA_DWORDS;   Ndx++ )
	{
		pBdAdslTextLogs[Ndx] = 0;
	}
	BdAdslTextLogIndex = 0;
	RtnResultValue = CardALTextLogMsgQEnqMsg( &BdAdslTextLog2 );
	while (		(RtnResultValue == RESULT_SUCCESS)
	         &&	(BdAdslTextLog2.FirstDword.Words.Size > 0)
	         &&	((BdAdslTextLogIndex + BdAdslTextLog2.FirstDword.Words.Size) < BD_MINI_DATA_DWORDS) )
	{
		RtnResultValue = CardALTextLogMsgQGetMsg( &BdAdslTextLog2 );
		if ( RtnResultValue == RESULT_SUCCESS )
		{
			LogMsgSizeInDwords = BdAdslTextLog2.FirstDword.Words.Size + 1;
			pdwArray = (DWORD *) &BdAdslTextLog2;
			for ( Ndx = 0;   Ndx < LogMsgSizeInDwords;   Ndx++ )
			{
				pBdAdslTextLogs[BdAdslTextLogIndex++] = pdwArray[Ndx];
			}
		}
		RtnResultValue = CardALTextLogMsgQEnqMsg( &BdAdslTextLog2 );
	}
	RtnResultValue = RESULT_SUCCESS;

	return RtnResultValue;


}

/******************************************************************************
FUNCTION NAME:
	CdalDiagGetADSLPerf

ABSTRACT:
	Get ADSL Perf Data.  This needs to be moved to CardMgmt.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/

GLOBAL NTSTATUS CdalDiagGetADSLPerf(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T	* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;

	TIGRIS_CONTEXT_T			* pContext ;
	XCVR_DATABASE_TYPE			* xcvr_db ;
	BD_ADSL_PERFORMANCE_T       * pBdADSLPerfStatus;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	DATAPUMP_CONTEXT			* pDatapump ;

	DWORD TempOccup ;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	pBdADSLPerfStatus = (BD_ADSL_PERFORMANCE_T *) Buffer;

	xcvr_db = &pDatapump->XCVR_database ;


	TempOccup = XCVR_GetDataRate(pDatapump, UP_STREAM) * 100;

	if (xcvr_db->bit_capacity_info.upstream_attainable_data_rate != 0)
	{
		TempOccup = TempOccup/ xcvr_db->bit_capacity_info.upstream_attainable_data_rate;
	}
	else
	{
		TempOccup=0;
	}
	if (TempOccup > 100)
	{
		TempOccup =100;
	}

	pBdADSLPerfStatus->R_relCapacityOccupationUpstr = TempOccup;

	TempOccup = XCVR_GetDataRate(pDatapump, DWN_STREAM) * 100;

	if (xcvr_db->bit_capacity_info.downstream_attainable_data_rate != 0)
	{
		TempOccup = TempOccup/ xcvr_db->bit_capacity_info.downstream_attainable_data_rate;
	}
	else
	{
		TempOccup=0;
	}

	if (TempOccup > 100)
	{
		TempOccup =100;
	}

	pBdADSLPerfStatus->R_relCapacityOccupationDnstr = TempOccup;

	pBdADSLPerfStatus->R_noiseMarginDnstr   =
	    (-1 * pDatapump->SYS_measurement[DWN_STREAM].mar_cur >>7);
	pBdADSLPerfStatus->R_outputPowerUpstr = 0; // (AFE_GetTxGain(pContext->Xcvr_Num)>>7) ;

	TempOccup =	(pDatapump->SYS_measurement[DWN_STREAM].line_atn >>8) ;
	if (TempOccup > 100)
	{
		TempOccup =100;
	}

	pBdADSLPerfStatus->R_attenuationDnstr  = TempOccup;

	pBdADSLPerfStatus->R_noiseMarginUpstr =
	    (-1 * pDatapump->SYS_measurement[UP_STREAM].mar_cur >>7);
	pBdADSLPerfStatus->R_outputPowerDnstr= 0;

	TempOccup =	(pDatapump->SYS_measurement[UP_STREAM].line_atn >> 8);
	if (TempOccup > 100)
	{
		TempOccup =100;
	}

	pBdADSLPerfStatus->R_attenuationUpstr =	TempOccup;
	pBdADSLPerfStatus->R_ChanDataIntNear = 0;
	pBdADSLPerfStatus->R_ChanDataFastNear = 0;
	pBdADSLPerfStatus->R_ChanDataIntFar= 0;
	pBdADSLPerfStatus->R_ChanDataFastFar= 0;
	pBdADSLPerfStatus->M_ModemStatus =BD_MODEM_UNKNOWN;
	pBdADSLPerfStatus->M_NsecValidBerFast = 0;
	pBdADSLPerfStatus->M_AccBerFast = 0;


	// Up and Down Bits/Frame
	//pBdAdslTransceiverStatus->Up_Bits_Per_Frame = xcvr_db->C_MSGS2.bits_per_sym ;
	//	pBdAdslTransceiverStatus->Down_Bits_Per_Frame = xcvr_db->R_MSGS2.bits_per_sym ;



	return RESULT_SUCCESS;



}

