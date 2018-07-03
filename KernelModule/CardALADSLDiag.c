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
**		CardAlADSLDiag.c
**
**	ABSTRACT:
**		This file contain ADSL Diagnostics Control Panel inteface.
**		The interface is a set of backdoor commands that allow 
**      the control panel to retreive detailed ADSL status 
**      information.   	  
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/KernelModule/CardALADSLDiag.c $
**	$Revision: 7 $
**	$Date: 7/03/01 4:40p $
*******************************************************************************
******************************************************************************/
#include "CardALTigrisV.h"
#include "CardALADSLDiag.h"
#include "dmtapi.h"
#include "xcvrinfo.h"

#define ERROR_FATAL(error)		(LEVEL_FATAL   | MODULE_CARDAL | INSTANCE_CARDAL | error)
#define ERROR_WARNING(error)	(LEVEL_WARNING | MODULE_CARDAL | INSTANCE_CARDAL | error)
#define ERROR_NORMAL(error)		(LEVEL_NORMAL  | MODULE_CARDAL | INSTANCE_CARDAL | error)


/*******************************************************************************
Private Function Prototypes
*******************************************************************************/

GLOBAL NTSTATUS CdalADSLAlarms(
    void					* pCardALAdapter,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalADSLDiagBitTable(
    void					* pCardALAdapter,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalADSLDiagSNR(
    void					* pCardALAdapter,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalADSLDiagGetLog(
    void					* pCardALAdapter,
    CHAR					* Buffer,
    DWORD					Length
);


GLOBAL NTSTATUS CdalADSLDiagSetLog(
    void					* pCardALAdapter,
    CHAR					* Buffer,
    DWORD					Length
);


GLOBAL NTSTATUS CdalADSLGHSCap(
    void					* pCardALAdapter,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalADSLGetLocalGHSCap(
    void					* pCardALAdapter,
    CHAR					* Buffer,
    DWORD					Length
);

GLOBAL NTSTATUS CdalADSLSetLocalGHSCap(
    void					* pCardALAdapter,
    CHAR					* Buffer,
    DWORD					Length
);

LOCAL NTSTATUS CdalADSLSetAutoSense
(
    void					* pCardALAdapter,
    CHAR					* Buffer,
    DWORD					  Length
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
MODULAR VOID * CdalADSLDiagInit(
    IN void		* pAdapterContext,
    IN VOID		* pCardAlContext
)
{
	NDIS_STATUS						Status;
	static VOID						* SysIfHandle;
	CDSL_ADAPTER_T					* pThisAdapter;
	CARDAL_ADAPTER_T				* pCardALAdapter;
	TIGRIS_CONTEXT_T				* pContext ;
	DATAPUMP_CONTEXT				* pDatapump ;

	// *
	// * This table relates all API requests to a CardAl Function.  To process
	// * a new request, add an entry to this table and write the
	// * appropriate function.
	// *
	MESSAGE_HANDLER_T		HandlerTable[] =
	    {
	        {MESS_API_COMMAND,	BD_ADSL_GET_ALARMS, 				sizeof(BD_ADSL_ALARMS_STATUS_T),	CdalADSLAlarms,				IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_GET_BIT_ALLOCATION_TABLE,	sizeof(BD_ADSL_BIT_ALLOCATION_T), 	CdalADSLDiagBitTable,		IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_GET_SNR_TABLE,				sizeof(BD_ADSL_SNR_T),				CdalADSLDiagSNR,			IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_GET_CONTROLLER_LOG_CLT,		sizeof(BD_ADSL_CONTROLLER_LOG_CLT_T),CdalADSLDiagGetLog,		IF_CARDAL_ID},
	        {MESS_API_COMMAND,	BD_ADSL_SET_CONTROLLER_LOG_CLT, 	sizeof(BD_ADSL_CONTROLLER_LOG_CLT_T),CdalADSLDiagSetLog,		IF_CARDAL_ID},
	        {MESS_API_COMMAND,  BD_ADSL_GET_GHS_CAP,				sizeof(BD_ADSL_GHS_CAP_T),			CdalADSLGHSCap,				IF_CARDAL_ID},
	        {MESS_API_COMMAND,  BD_ADSL_GET_GHS_LOCAL_CAP, 			sizeof(BD_ADSL_LOCAL_GHS_CAP_T),	CdalADSLGetLocalGHSCap,		IF_CARDAL_ID},
	        {MESS_API_COMMAND,  BD_ADSL_SET_GHS_LOCAL_CAP, 			sizeof(BD_ADSL_LOCAL_GHS_CAP_T),	CdalADSLSetLocalGHSCap,		IF_CARDAL_ID},
	        {MESS_API_COMMAND,  BD_ADSL_SET_AUTO_SENSE, 			sizeof(BD_ADSL_SET_AUTO_SENSE_T),	CdalADSLSetAutoSense,		IF_CARDAL_ID},
	    };


	pThisAdapter = (CDSL_ADAPTER_T*) pAdapterContext;
	pCardALAdapter  = (CARDAL_ADAPTER_T *) pThisAdapter->CardMod;
	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	Auto_SetAutoSense
	(
	    pDatapump,
	    (SYS_HANDSHAKE_TYPE)pThisAdapter->PersistData.AutoSenseHandshake,
	    (SYS_WIRING_SELECTION_TYPE)	pThisAdapter->PersistData.AutoWiringSelection,
	    (BOOLEAN)			pThisAdapter->PersistData.AutoSenseWiresFirst
	);
	pDatapump->CdALDiagControllerLogEnable	= (BOOLEAN) pThisAdapter->PersistData.CdALDiagControllerLogEnable;
	pDatapump->AdslLocalG922Cap				= (WORD)pThisAdapter->PersistData.BdADSLLocalG922Cap;
	pDatapump->AdslLocalG922AnnexCap		= (WORD)pThisAdapter->PersistData.BdADSLLocalG922AnnexCap;

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

	return((PVOID)!NULL);
}

/*******************************************************************************
Private Functions
*******************************************************************************/

/******************************************************************************
FUNCTION NAME:
	CdalADSLAlarms

ABSTRACT:
	Retreives current G.HS Capabilities, including local endpoint, remote endpoint 
	and negiti ubset of the ADSL SNR table (one SNR per BIN)


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/

GLOBAL NTSTATUS CdalADSLAlarms(
    VOID					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T			* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					Status = STATUS_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	TIGRIS_CONTEXT_T			* pContext ;
	BD_ADSL_ALARMS_STATUS_T		* pBdADSLAlaramsStatus;
	DATAPUMP_CONTEXT			* pDatapump ;
	DWORD Index;
	DWORD AlarmIndex;


	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	if (Buffer != NULL)
	{

		pBdADSLAlaramsStatus = (BD_ADSL_ALARMS_STATUS_T	*)Buffer;

		AlarmIndex = 0;
		for (Index=0; Index<NUM_OF_DUMB_DISP_ALARM; Index++)
		{
			if (pDatapump->MON_alarm[UP_STREAM][Index].active)
			{
				pBdADSLAlaramsStatus->UpStreamAlarms.Alarms[AlarmIndex] =
				    Index+1;

				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					CardALDebugFlag,
					("pBdADSLAlaramsStatus->UpStreamAlarms.Alarms[%ld] = %x", AlarmIndex, pBdADSLAlaramsStatus->UpStreamAlarms.Alarms[AlarmIndex]));

				AlarmIndex++;

			}
			if (AlarmIndex == BD_ADSL_ALARMS_NUMBER)
			{
				break;
			}
		}

		AlarmIndex = 0;
		for (Index=0; Index<NUM_OF_DUMB_DISP_ALARM; Index++)
		{
			if (pDatapump->MON_alarm[DWN_STREAM][Index].active)
			{
				pBdADSLAlaramsStatus->DownStreamAlarms.Alarms[AlarmIndex] =
				    Index+1;

				DBG_CDSL_DISPLAY(
					DBG_LVL_MISC_INFO,
					CardALDebugFlag,
					(	"pBdADSLAlaramsStatus->DownStreamAlarms.Alarms[%ld] = %x",
						AlarmIndex,
						pBdADSLAlaramsStatus->DownStreamAlarms.Alarms[AlarmIndex]));

				AlarmIndex++;
			}
			if (AlarmIndex == BD_ADSL_ALARMS_NUMBER)
			{
				break;
			}
		}
	}

	return Status;
}

/******************************************************************************
FUNCTION NAME:
	CdalADSLDiagBitTable

ABSTRACT:
	Retreive a set of ADSL Bit allocation table entries.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/

GLOBAL NTSTATUS CdalADSLDiagBitTable(
    VOID					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T			* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					Status = STATUS_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	BD_ADSL_BIT_ALLOCATION_T 	*	pBdADSLBitAllocationTable;


	UINT8 BinIndex;
	DWORD TableIndex = 0;

	TIGRIS_CONTEXT_T			* pContext ;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	if (Buffer != NULL)
	{
		pBdADSLBitAllocationTable =  (BD_ADSL_BIT_ALLOCATION_T *) Buffer;

		// check requested bins
		if ((pBdADSLBitAllocationTable->StartingBin > pBdADSLBitAllocationTable->EndingBin)	 ||
		        ((pBdADSLBitAllocationTable->EndingBin - pBdADSLBitAllocationTable->StartingBin) > BD_ADSL_SNR_NUMBER)	||
		        (pBdADSLBitAllocationTable->EndingBin > 255))
		{
			return STATUS_FAILURE;
		}

		for
		(
		    BinIndex = (UINT8) pBdADSLBitAllocationTable->StartingBin;
		    BinIndex < (UINT8)pBdADSLBitAllocationTable->EndingBin;
		    BinIndex++
		)
		{
			CHNL_INFO_TYPE ChnlInfo;

			ChnlInfo=MISC_GetChnlInfo(pContext->pDatapump, BinIndex).return_value;
			pBdADSLBitAllocationTable->Entry[TableIndex].BinNumber 	= BinIndex;
			pBdADSLBitAllocationTable->Entry[TableIndex].BinStatus	= ChnlInfo.status;
			pBdADSLBitAllocationTable->Entry[TableIndex].BitsAssigned = ChnlInfo.bits;
			pBdADSLBitAllocationTable->Entry[TableIndex].BitCapacity 	= ChnlInfo.bit_capacity;

			DBG_CDSL_DISPLAY(
				DBG_LVL_MISC_INFO,
				CardALDebugFlag,
				("pBdADSLBitAllocationTable->Entry[%ld].BinNumber 	= %x",
					TableIndex,
					pBdADSLBitAllocationTable->Entry[TableIndex].BinNumber));
			DBG_CDSL_DISPLAY(
				DBG_LVL_MISC_INFO,
				CardALDebugFlag,
				("pBdADSLBitAllocationTable->Entry[%ld].BinStatus 	= %x",
					TableIndex,
					pBdADSLBitAllocationTable->Entry[TableIndex].BinStatus));
			DBG_CDSL_DISPLAY(
				DBG_LVL_MISC_INFO,
				CardALDebugFlag,
				("pBdADSLBitAllocationTable->Entry[%ld].BitsAssigned	= %x",
					TableIndex,
					pBdADSLBitAllocationTable->Entry[TableIndex].BitsAssigned));
			DBG_CDSL_DISPLAY(
				DBG_LVL_MISC_INFO,
				CardALDebugFlag,
				("pBdADSLBitAllocationTable->Entry[%ld].BitCapacity 	= %x",
					TableIndex,
					pBdADSLBitAllocationTable->Entry[TableIndex].BitCapacity));


			TableIndex++;
		}


	}


	return Status;
}

/******************************************************************************
FUNCTION NAME:
	CdalADSLDiagSNR

ABSTRACT:
	Retreivea a subset of the ADSL SNR table (one SNR per BIN)


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/

GLOBAL NTSTATUS CdalADSLDiagSNR(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T			* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					Status = STATUS_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	BD_ADSL_SNR_T				* pBdADSLSnrTable;
	UINT8 BinIndex;
	DWORD TableIndex = 0;
	SINT16 ADSLSNR;


	TIGRIS_CONTEXT_T			* pContext ;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	if (Buffer != NULL)
	{
		pBdADSLSnrTable = (BD_ADSL_SNR_T *) Buffer;

		// check requested bins
		if ((pBdADSLSnrTable->StartingBin > pBdADSLSnrTable->EndingBin)	 ||
		        ((pBdADSLSnrTable->EndingBin - pBdADSLSnrTable->StartingBin) > BD_ADSL_SNR_NUMBER)	||
		        (pBdADSLSnrTable->EndingBin > 255))
		{
			return STATUS_FAILURE;
		}

		for
		(
		    BinIndex = (UINT8)pBdADSLSnrTable->StartingBin;
		    BinIndex < (UINT8)pBdADSLSnrTable->EndingBin;
		    BinIndex++
		)
		{

			ADSLSNR=BG_GetSnrMargin(pContext->pDatapump, BinIndex);
			pBdADSLSnrTable->Entry[TableIndex].BinNumber = BinIndex;
			pBdADSLSnrTable->Entry[TableIndex].BinSNR    = ADSLSNR;

			DBG_CDSL_DISPLAY(
				DBG_LVL_MISC_INFO,
				CardALDebugFlag,
				("pBdADSLSnrTable->Entry[%ld].BinNumber 	= %x",
					TableIndex,
					pBdADSLSnrTable->Entry[TableIndex].BinNumber));
			DBG_CDSL_DISPLAY(
				DBG_LVL_MISC_INFO,
				CardALDebugFlag,
				("pBdADSLSnrTable->Entry[%ld].BinSNR 	= %x",
					TableIndex,
					pBdADSLSnrTable->Entry[TableIndex].BinSNR));

			TableIndex++;
		}


	}


	return Status;
}

/******************************************************************************
FUNCTION NAME:
	CdalADSLDiagGetLog

ABSTRACT:
	Retreive ADSL Controller log control flag.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/

GLOBAL NTSTATUS CdalADSLDiagGetLog(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	NTSTATUS 					Status = STATUS_SUCCESS;

	BD_ADSL_CONTROLLER_LOG_CLT_T	* pBdADSLDiagnosticLog;
	DATAPUMP_CONTEXT				* pDatapump ;
	TIGRIS_CONTEXT_T				* pContext ;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;


	if (Buffer != NULL)
	{
		pBdADSLDiagnosticLog = (BD_ADSL_CONTROLLER_LOG_CLT_T *) Buffer;
		pBdADSLDiagnosticLog->EnableLogsToAutolog = pDatapump->CdALDiagControllerLogEnable;
		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			CardALDebugFlag,
			(	"pBdADSLDiagnosticLog->EnableLogsToAutolog	= %x",
				pBdADSLDiagnosticLog->EnableLogsToAutolog));
	}
	return Status;
}

/******************************************************************************
FUNCTION NAME:
	CdalADSLDiagSetLog

ABSTRACT:
	Sets ADSL Controller log control flag.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/


GLOBAL NTSTATUS CdalADSLDiagSetLog(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	NTSTATUS 					Status = STATUS_SUCCESS;

	BD_ADSL_CONTROLLER_LOG_CLT_T	* pBdADSLDiagnosticLog;
	DATAPUMP_CONTEXT				* pDatapump ;
	TIGRIS_CONTEXT_T				* pContext ;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;



	if (Buffer != NULL)
	{
		pBdADSLDiagnosticLog = (BD_ADSL_CONTROLLER_LOG_CLT_T *) Buffer;
		pDatapump->CdALDiagControllerLogEnable = pBdADSLDiagnosticLog->EnableLogsToAutolog;
		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			CardALDebugFlag,
			(	"pBdADSLDiagnosticLog->EnableLogsToAutolog	= %x",
				pBdADSLDiagnosticLog->EnableLogsToAutolog));
	}
	return Status;
}



/******************************************************************************
FUNCTION NAME:
	CdalADSLGHSCap

ABSTRACT:
	Retreives current G.HS Capabilities, including local endpoint, remote endpoint 
	and negiti ubset of the ADSL SNR table (one SNR per BIN)


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/

GLOBAL NTSTATUS CdalADSLGHSCap(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T			* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					Status = STATUS_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	TIGRIS_CONTEXT_T			* pContext ;
	DATAPUMP_CONTEXT			* pDatapump ;

	GHS_DB_TYPE *pghs;

	XCVR_OTHER_CFG_TYPE   ghs_cfg_ptr;

	BD_ADSL_GHS_CAP_T * pBdADSLG_HSCapabilities;
	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	if (Buffer != NULL)
	{

		pBdADSLG_HSCapabilities = (BD_ADSL_GHS_CAP_T *)Buffer;

		SYS_GetSysNvram((PDATAPUMP_CONTEXT)pContext->pDatapump,NEAR_END,NVRAM_XCVR_OTHER_CFG,(void *)&ghs_cfg_ptr);
		pghs=&pDatapump->ghs_db;


		// set V8
		pBdADSLG_HSCapabilities->V8.LocalEndpoint 	= (((ghs_cfg_ptr.CAP_common&0x1)==0)?1:0);
		pBdADSLG_HSCapabilities->V8.RemoteEndpoint 	= (((pghs->remote_capabilities.common&0x1)==0)?1:0);
		pBdADSLG_HSCapabilities->V8.Negiotiated 	= (((pghs->negotiated_capabilities.common&0x1)==0)?1:0);

		pBdADSLG_HSCapabilities->V8bis.LocalEndpoint 	= (((ghs_cfg_ptr.CAP_common&0x2)==0)?1:0);
		pBdADSLG_HSCapabilities->V8bis.RemoteEndpoint 	= (((pghs->remote_capabilities.common&0x2)==0)?1:0);
		pBdADSLG_HSCapabilities->V8bis.Negiotiated 		= (((pghs->negotiated_capabilities.common&0x2)==0)?1:0);

		pBdADSLG_HSCapabilities->SilentPeriod.LocalEndpoint 	= (((ghs_cfg_ptr.CAP_common&0x4)==0)?1:0);
		pBdADSLG_HSCapabilities->SilentPeriod.RemoteEndpoint 	= (((pghs->remote_capabilities.common&0x4)==0)?1:0);
		pBdADSLG_HSCapabilities->SilentPeriod.Negiotiated 		= (((pghs->negotiated_capabilities.common&0x4)==0)?1:0);

		pBdADSLG_HSCapabilities->GPLOAM.LocalEndpoint 	= (((ghs_cfg_ptr.CAP_common&0x10)==0)?1:0);
		pBdADSLG_HSCapabilities->GPLOAM.RemoteEndpoint 	= (((pghs->remote_capabilities.common&0x10)==0)?1:0);
		pBdADSLG_HSCapabilities->GPLOAM.Negiotiated 	= (((pghs->negotiated_capabilities.common&0x10)==0)?1:0);

		pBdADSLG_HSCapabilities->G9221AnnexA.LocalEndpoint 	= (((ghs_cfg_ptr.CAP_standards&0x1)==0)?1:0);
		pBdADSLG_HSCapabilities->G9221AnnexA.RemoteEndpoint = (((pghs->remote_capabilities.standards&0x1)==0)?1:0);
		pBdADSLG_HSCapabilities->G9221AnnexA.Negiotiated 	= (((pghs->negotiated_capabilities.standards&0x1)==0)?1:0);

		pBdADSLG_HSCapabilities->G9221AnnexB.LocalEndpoint 	= (((ghs_cfg_ptr.CAP_standards&0x2)==0)?1:0);
		pBdADSLG_HSCapabilities->G9221AnnexB.RemoteEndpoint = (((pghs->remote_capabilities.standards&0x2)==0)?1:0);
		pBdADSLG_HSCapabilities->G9221AnnexB.Negiotiated 	= (((pghs->negotiated_capabilities.standards&0x2)==0)?1:0);

		pBdADSLG_HSCapabilities->G9221AnnexC.LocalEndpoint 	= (((ghs_cfg_ptr.CAP_standards&0x4)==0)?1:0);
		pBdADSLG_HSCapabilities->G9221AnnexC.RemoteEndpoint = (((pghs->remote_capabilities.standards&0x4)==0)?1:0);
		pBdADSLG_HSCapabilities->G9221AnnexC.Negiotiated 	= (((pghs->negotiated_capabilities.standards&0x4)==0)?1:0);

		pBdADSLG_HSCapabilities->G9222AnnexAB.LocalEndpoint	= (((ghs_cfg_ptr.CAP_standards&0x8)==0)?1:0);
		pBdADSLG_HSCapabilities->G9222AnnexAB.RemoteEndpoint= (((pghs->remote_capabilities.standards&0x8)==0)?1:0);
		pBdADSLG_HSCapabilities->G9222AnnexAB.Negiotiated 	= (((pghs->negotiated_capabilities.standards&0x8)==0)?1:0);


		pBdADSLG_HSCapabilities->G9222AnnexC.LocalEndpoint 	= (((ghs_cfg_ptr.CAP_standards&0x10)==0)?1:0);
		pBdADSLG_HSCapabilities->G9222AnnexC.RemoteEndpoint = (((pghs->remote_capabilities.standards&0x10)==0)?1:0);
		pBdADSLG_HSCapabilities->G9222AnnexC.Negiotiated 	= (((pghs->negotiated_capabilities.standards&0x10)==0)?1:0);


		pBdADSLG_HSCapabilities->RemoteCountry = pghs->remote_vendor_id.country_code;
		pBdADSLG_HSCapabilities->RemoteVendorID[0] =  pghs->remote_vendor_id.vendor_spec_info[0];
		pBdADSLG_HSCapabilities->RemoteVendorID[1] =  pghs->remote_vendor_id.vendor_spec_info[1];
		pBdADSLG_HSCapabilities->RemoteVendorID[2] =  pghs->remote_vendor_id.vendor_spec_info[2];
		pBdADSLG_HSCapabilities->RemoteVendorID[3] =  pghs->remote_vendor_id.vendor_spec_info[3];

		pBdADSLG_HSCapabilities->RemoteSpecInfo[0] =  pghs->remote_vendor_id.vendor_spec_info[0];
		pBdADSLG_HSCapabilities->RemoteSpecInfo[1] =  pghs->remote_vendor_id.vendor_spec_info[1];

	}


	return Status;
}
/******************************************************************************
FUNCTION NAME:
	CdalADSLGetLocalGHSCap
	
ABSTRACT:
	Retreives current G.HS Capabilities, including local endpoint, remote endpoint 
	and negiti ubset of the ADSL SNR table (one SNR per BIN)


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/

GLOBAL NTSTATUS CdalADSLGetLocalGHSCap(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T			* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					Status = STATUS_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	TIGRIS_CONTEXT_T			* pContext ;

	XCVR_OTHER_CFG_TYPE   ghs_cfg_ptr;

	BD_ADSL_LOCAL_GHS_CAP_T	 * pBdADSLocalG_HSCapabilities;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	if (Buffer != NULL)
	{
		pBdADSLocalG_HSCapabilities = (BD_ADSL_LOCAL_GHS_CAP_T	 *) Buffer;

		SYS_GetSysNvram((PDATAPUMP_CONTEXT)pContext->pDatapump,NEAR_END,NVRAM_XCVR_OTHER_CFG,(void *)&ghs_cfg_ptr);

		pBdADSLocalG_HSCapabilities->BdADSLLocalG922Cap 		= ghs_cfg_ptr.CAP_common;
		pBdADSLocalG_HSCapabilities->BdADSLLocalG922AnnexCap 	= ghs_cfg_ptr.CAP_standards;

		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			CardALDebugFlag,
			(	"pBdADSLocalG_HSCapabilities->BdADSLLocalG922Cap	= %x",
				pBdADSLocalG_HSCapabilities->BdADSLLocalG922Cap));
		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			CardALDebugFlag,
			(	"pBdADSLocalG_HSCapabilities->BdADSLLocalG922AnnexCap	= %x",
				pBdADSLocalG_HSCapabilities->BdADSLLocalG922AnnexCap));
	}


	return Status;
}

/******************************************************************************
FUNCTION NAME:
	CdalADSLSetLocalGHSCap

ABSTRACT:
	Retreives current G.HS Capabilities, including local endpoint, remote endpoint 
	and negiti ubset of the ADSL SNR table (one SNR per BIN)


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/

GLOBAL NTSTATUS CdalADSLSetLocalGHSCap(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
)
{
	CARDAL_ADAPTER_T			* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 					Status = STATUS_SUCCESS;
	CDSL_ADAPTER_T				* pThisAdapter = pCardALAdapter->pCdslAdapter;
	TIGRIS_CONTEXT_T			* pContext ;
	DATAPUMP_CONTEXT			* pDatapump ;

	XCVR_OTHER_CFG_TYPE   ghs_cfg_ptr;

	BD_ADSL_LOCAL_GHS_CAP_T	 * pBdADSLocalG_HSCapabilities;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	if (Buffer != NULL)
	{
		pBdADSLocalG_HSCapabilities = (BD_ADSL_LOCAL_GHS_CAP_T	 *) Buffer;

		SYS_GetSysNvram((PDATAPUMP_CONTEXT)pContext->pDatapump,NEAR_END,NVRAM_XCVR_OTHER_CFG,(void *)&ghs_cfg_ptr);

		ghs_cfg_ptr.CAP_common = 	pBdADSLocalG_HSCapabilities->BdADSLLocalG922Cap;
		ghs_cfg_ptr.CAP_standards = pBdADSLocalG_HSCapabilities->BdADSLLocalG922AnnexCap;

		// save local copy for NVRAM restore
		pDatapump->AdslLocalG922Cap			=	pBdADSLocalG_HSCapabilities->BdADSLLocalG922Cap;
		pDatapump->AdslLocalG922AnnexCap	=	pBdADSLocalG_HSCapabilities->BdADSLLocalG922AnnexCap;

		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			CardALDebugFlag,
			(	"pBdADSLocalG_HSCapabilities->BdADSLLocalG922Cap	= %x",
				pBdADSLocalG_HSCapabilities->BdADSLLocalG922Cap));
		DBG_CDSL_DISPLAY(
			DBG_LVL_MISC_INFO,
			CardALDebugFlag,
			(	"pBdADSLocalG_HSCapabilities->BdADSLLocalG922AnnexCap	= %x",
				pBdADSLocalG_HSCapabilities->BdADSLLocalG922AnnexCap));

		SYS_SetSysNvram ( pDatapump, NEAR_END, NVRAM_XCVR_OTHER_CFG, (void *)&ghs_cfg_ptr ) ;

	}

	return Status;
}


/******************************************************************************
FUNCTION NAME:
	CdalADSLRestoreGHSCap

ABSTRACT:
	Returns the local G.HS Capabilities.
	This function is called when NVRAM is restored. 

RETURN:
	void 


DETAILS:
******************************************************************************/

GLOBAL void CdalADSLRestoreGHSCap(
    IN TIGRIS_CONTEXT_T     * pContext,
    WORD 	* CAP_common,
    WORD    * CAP_standards
)
{
	DATAPUMP_CONTEXT			* pDatapump ;
	pDatapump = (DATAPUMP_CONTEXT *) pContext->pDatapump ;

	* CAP_common 	= pDatapump->AdslLocalG922Cap;
	* CAP_standards = pDatapump->AdslLocalG922AnnexCap;
}
/******************************************************************************
FUNCTION NAME:
	CardALADSLLogPairgain

ABSTRACT:


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/

DPCCALL GLOBAL void CardALADSLLogPairgain
(
    PDATAPUMP_CONTEXT	  pDatapump, 			
    const char			* String,
    DIAG_SUBCODE_TYPE	  Subcode
)
{


	BD_DRIVER_LOG_T		LogMsg;
	char				LogStr[BD_MINI_DATA_DWORDS*sizeof(DWORD)];
	NDIS_STATUS			Status;

	if (pDatapump->CdALDiagControllerLogEnable == TRUE)
	{
		sprintf(LogStr,"%s %d\n",
		        String,
		        Subcode
		       );

		Status = CardALTextLogMsgPackStr( LogStr, &LogMsg );
		Status = CardALTextLogMsgQPutMsg( &LogMsg );
	}

};



/******************************************************************************
FUNCTION NAME:
	CdalADSLGetVendorId

ABSTRACT:


RETURN:
	Vendor ID


DETAILS:
******************************************************************************/

GLOBAL UINT16 CdalADSLGetVendorId
(
    CARDAL_ADAPTER_T		* pCardALAdapter
)
{

	if ( pCardALAdapter != NULL)
	{
		CDSL_ADAPTER_T			* pThisAdapter	= pCardALAdapter->pCdslAdapter;

		return	(WORD)pThisAdapter->PersistData.VendorNearId;
	}
	else
	{
		return DEFAULT_VENDOR_NEAR_ID;
	}
}





/******************************************************************************
FUNCTION NAME:
	CdalADSLGetAutoWiring

ABSTRACT:


RETURN:
	Vendor ID


DETAILS:
******************************************************************************/

GLOBAL UINT16 CdalADSLGetAutoWiring
(
    CARDAL_ADAPTER_T		* pCardALAdapter
)
{
	if ( pCardALAdapter != NULL)
	{
		CDSL_ADAPTER_T			* pThisAdapter	= pCardALAdapter->pCdslAdapter;

		return	(WORD)pThisAdapter->PersistData.AutoWiringSelection;
	}
	else
	{
		return DEFAULT_AUTO_SENSE_WIRING;
	}
}



/******************************************************************************
FUNCTION NAME:
	CdalADSLGetAutoHandshake

ABSTRACT:


RETURN:
	Vendor ID


DETAILS:
******************************************************************************/

GLOBAL UINT16 CdalADSLGetAutoHandshake
(
    CARDAL_ADAPTER_T		* pCardALAdapter
)
{
	if ( pCardALAdapter != NULL)
	{
		CDSL_ADAPTER_T			* pThisAdapter	= pCardALAdapter->pCdslAdapter;

		return	(WORD)pThisAdapter->PersistData.AutoSenseHandshake;
	}
	else
	{
		return DEFAULT_AUTO_SENSE_HANDSHAKE;
	}
}



/******************************************************************************
FUNCTION NAME:
	CdalADSLGetAutoSenseWiresFirst

ABSTRACT:


RETURN:
	Vendor ID


DETAILS:
******************************************************************************/

GLOBAL UINT16 CdalADSLGetAutoSenseWiresFirst
(
    CARDAL_ADAPTER_T		* pCardALAdapter
)
{
	if ( pCardALAdapter != NULL)
	{
		CDSL_ADAPTER_T			* pThisAdapter	= pCardALAdapter->pCdslAdapter;

		return	(WORD)pThisAdapter->PersistData.AutoSenseWiresFirst;
	}
	else
	{
		return DEFAULT_AUTO_SENSE_WIRES_FIRST;
	}
}



/******************************************************************************
FUNCTION NAME:
	CdalADSLSetAutoSense

ABSTRACT:
	Set the autosensing options in data pump code.


RETURN:
	status


DETAILS:
******************************************************************************/
LOCAL NTSTATUS CdalADSLSetAutoSense
(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					  Length
)
{
	CARDAL_ADAPTER_T		* pCardALAdapter=(CARDAL_ADAPTER_T*)pAdapter;
	NTSTATUS 				  Status		= STATUS_SUCCESS;
	CDSL_ADAPTER_T			* pThisAdapter	= pCardALAdapter->pCdslAdapter;
	TIGRIS_CONTEXT_T		* pContext ;


	BD_ADSL_SET_AUTO_SENSE_T* pBdADSAutoSense;

	pContext = (TIGRIS_CONTEXT_T *) CDALGetModuleHandle(pThisAdapter, CARDAL_TIGRIS_CONTROLLER);

	if (Buffer != NULL)
	{
		pBdADSAutoSense = (BD_ADSL_SET_AUTO_SENSE_T *) Buffer;

		Auto_SetAutoSense
		(
			(PDATAPUMP_CONTEXT)	pContext->pDatapump,
								pBdADSAutoSense->BdADSL_sys_autohandshake,
								pBdADSAutoSense->BdADSL_sys_autowiring,
								pBdADSAutoSense->BdADSL_sys_autosense_wiresfirst
		) ;
	}
	return Status;
}
