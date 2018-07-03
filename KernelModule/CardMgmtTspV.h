/******************************************************************************
*******************************************************************************
****	Copyright (c) 1997, 1998
****	Rockwell Semiconductor Systems
****	Personal Computing Division
****	All Rights Reserved
****
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****
*******************************************************************************
**
**	MODULE NAME:
**		CardMgmt (Card Management)
**
**	FILE NAME:
**		CardMgmtTspV.h
**
**	ABSTRACT:
**		WAN Miniport NDIS TAPI requests.
**		This module contains the declarations for CardMgmtTsp.c.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CardMgmtTspV.h $
** $Revision: 1 $
** $Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/
#ifndef _CARDMGMTTSPV_H_		//	File Wrapper,
#define _CARDMGMTTSPV_H_		//	prevents multiple inclusions




#define NO_ANSWER_TIMEOUT			70000		//	msec
#define NO_ACCEPT_TIMEOUT			5000		//	msec
#define NO_CONNECT_TIMEOUT			70000		//	msec


///////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
///////////////////////////////////////////////////////////////////////////////

void CMTSPTapiIndicateLinkUp(
    IN CDSL_ADAPTER_T		* pThisAdapter );

void CMTSPTapiIndicateLinkDown(
    IN CDSL_ADAPTER_T		* pThisAdapter );

VOID CMTSPTapiLinkTimerHandler(
    IN PVOID			SystemSpecific1,
    IN CDSL_LINK_T		* pThisLink,
    IN PVOID			SystemSpecific2,
    IN PVOID			SystemSpecific3 );

VOID CMTSPTapiCallTimerHandler(
    IN PVOID			SystemSpecific1,
    IN CDSL_VC_T		* pThisVc,
    IN PVOID			SystemSpecific2,
    IN PVOID			SystemSpecific3 );

VOID CMTSPTapiResetHandler(	IN CDSL_ADAPTER_T				* pThisAdapter );

///////////////////////////////////
//	TAPI Query functions
///////////////////////////////////

void CMTSPDeviceSpecific(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    IN BACK_DOOR_T					* pBackDoorBuf,
    OUT PULONG						pNumBytesNeeded,
    OUT PULONG						pNumBytesWritten );

#if		PROJECTS_NDIS_IF == PROJECT_NDIS_IF_WAN
NDIS_STATUS CMTSPTapiNegotiateExtVersion(
    CDSL_ADAPTER_T						* pThisAdapter,
    PNDIS_TAPI_NEGOTIATE_EXT_VERSION	Request );

NDIS_STATUS CMTSPTapiGetAddressCaps(
    CDSL_ADAPTER_T						* pThisAdapter,
    PNDIS_TAPI_GET_ADDRESS_CAPS			Request );

NDIS_STATUS CMTSPTapiGetAddressID(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_GET_ADDRESS_ID		Request );

NDIS_STATUS CMTSPTapiGetAddressStatus(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_GET_ADDRESS_STATUS	Request );

NDIS_STATUS CMTSPTapiGetCallAddressID(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_GET_CALL_ADDRESS_ID	Request );

NDIS_STATUS CMTSPTapiGetCallInfo(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_GET_CALL_INFO			Request );

NDIS_STATUS CMTSPTapiGetCallStatus(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_GET_CALL_STATUS		Request );

NDIS_STATUS CMTSPTapiGetDevCaps(
    CDSL_ADAPTER_T						* pThisAdapter,
    PNDIS_TAPI_GET_DEV_CAPS				Request );

NDIS_STATUS CMTSPTapiGetExtensionID(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_GET_EXTENSION_ID		Request );

NDIS_STATUS CMTSPTapiGetID(
    CDSL_ADAPTER_T						* pThisAdapter,
    PNDIS_TAPI_GET_ID					Request );

NDIS_STATUS CMTSPTapiGetLineDevStatus(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_GET_LINE_DEV_STATUS	Request );

NDIS_STATUS CMTSPTapiMakeCall(
    CDSL_ADAPTER_T						* pThisAdapter,
    PNDIS_TAPI_MAKE_CALL				Request );

NDIS_STATUS CMTSPTapiOpen(
    CDSL_ADAPTER_T						* pThisAdapter,
    PNDIS_TAPI_OPEN						Request );

NDIS_STATUS CMTSPTapiProviderInitialize(
    CDSL_ADAPTER_T					* pThisAdapter,
    PNDIS_TAPI_PROVIDER_INITIALIZE	InfoBuffer );

 #ifdef NDISTAPI_EXTENSIONS
NDIS_STATUS CMTSPTapiProviderEnumDevices(
    CDSL_ADAPTER_T						* pThisAdapter,
    PNDIS_TAPI_PROVIDER_ENUM_DEVICES	InfoBuffer );

NDIS_STATUS CMTSPTapiLineNegotiateTspiVersion(
    CDSL_ADAPTER_T					* pThisAdapter,
    PNDIS_TAPI_NEGOTIATE_TSPI_VER	Request );

 #endif


///////////////////////////////////
//	TAPI Set functions
///////////////////////////////////

NDIS_STATUS CMTSPTapiAnswer(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_ANSWER				Request );

NDIS_STATUS CMTSPTapiClose(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_CLOSE					Request );

NDIS_STATUS CMTSPTapiCloseCall(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_CLOSE_CALL			Request );

NDIS_STATUS CMTSPTapiConditionalMediaDetection(
    IN CDSL_ADAPTER_T							* pAdapter,
    IN PNDIS_TAPI_CONDITIONAL_MEDIA_DETECTION	Request );

NDIS_STATUS CMTSPTapiDrop(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_DROP					Request );

NDIS_STATUS CMTSPTapiProviderShutdown(
    CDSL_ADAPTER_T					* pThisAdapter,
    PNDIS_TAPI_PROVIDER_SHUTDOWN	Request );

NDIS_STATUS CMTSPTapiSetAppSpecific(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_SET_APP_SPECIFIC		Request );

NDIS_STATUS CMTSPTapiSetCallParams(
    IN CDSL_ADAPTER_T					* pAdapter,
    IN PNDIS_TAPI_SET_CALL_PARAMS		Request );

NDIS_STATUS CMTSPTapiSetDefaultMediaDetection(
    IN CDSL_ADAPTER_T							* pThisAdapter,
    IN PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION	Request );

NDIS_STATUS CMTSPTapiSetMediaMode(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_SET_MEDIA_MODE		Request );

NDIS_STATUS CMTSPTapiSetStatusMessages(
    IN CDSL_ADAPTER_T					* pThisAdapter,
    IN PNDIS_TAPI_SET_STATUS_MESSAGES	Request );

#endif		//	PROJECTS_NDIS_IF == PROJECT_NDIS_IF_WAN
///////////////////////////////////
//	other functions
///////////////////////////////////

VOID CMTSPTapiLineDevStateHandler(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN CDSL_LINK_T			* pThisLink,
    IN CDSL_VC_T			* pThisVc,
    IN ULONG				LineDevState );

VOID CMTSPTapiCallStateHandler(
    IN CDSL_VC_T			* pThisVc,
    IN ULONG				CallState,
    IN ULONG				StateParam );


#endif		//	#ifndef _CARDMGMTTSPV_H_
