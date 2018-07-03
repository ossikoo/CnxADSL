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
**		SmLnxIf.h
**
**	ABSTRACT:
**		Public Header file for System Management WDM Interface
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/SmLnxIf.h $
**	$Revision: 1 $
**	$Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/

#ifndef _SMLNXIF_H_
#define _SMLNXIF_H_						// File Wrapper
#include "Common.h"

//
// Put in an empty definition for the DEVICE_EXTENSION so that the
// routines can reference it before it is declared.
//
struct CDSL_ADAPTER_S;
struct MSG_HANDLING_S;

/*******************************************************************************
* Custom Data Types
*******************************************************************************/

GLOBAL VOID SmLnxIfUnLoadHandlers(
    IN struct CDSL_ADAPTER_S* pAdapter
);

#if HOOK_DISPATCH_TABLE
GLOBAL VOID SmLnxifLoadGlobalDispatch(
    IN struct CDSL_ADAPTER_S* pAdapter
);
#endif

NTSTATUS SmLnxSendBufferComm
(
    struct MSG_HANDLING_S		* pMsg,
    I_O BYTE 					* Buffer,
    IN	DWORD					Length);

NTSTATUS SmLnxOpenPeerChan(
    struct MSG_HANDLING_S			* pMsg,
    PFILE_OBJECT					* ppChanFileObject,
    struct CDSL_ADAPTER_S			** ppAdapter
);

VOID SmLnxClosePeerChan(
    struct MSG_HANDLING_S			* pMsg,
    PFILE_OBJECT					* ppChanFileObject,
    struct CDSL_ADAPTER_S			* pAdapter
);

NTSTATUS SmLnxSendBufferPeer(
    IN struct MSG_HANDLING_S	* pMsg,
    I_O BYTE 					* Buffer,
    IN	DWORD					Length
);

NTSTATUS SmLnxIfCompleteRequest(
    IN VOID	*	pRequest,
    IN NTSTATUS status,
    IN CHAR	* 	Buffer,
    IN ULONG info);

NTSTATUS SmLnxIfCompleteRequestPeer(
    IN VOID	*	pRequest,
    IN NTSTATUS status,
    IN CHAR	* 	Buffer,
    IN ULONG info);

NTSTATUS SmLnxOpenChipAlInterfQuery(
    struct MSG_HANDLING_S			* pMsg,
    PFILE_OBJECT					* ppChanFileObject,
    struct CDSL_ADAPTER_S			** ppAdapter
);

NTSTATUS SmLnxSendChipALInterfQuery
(
    IN struct MSG_HANDLING_S	* pMsg,
    I_O BYTE 					* Buffer,
    IN	DWORD					Length
);

#endif									// File Wrapper
