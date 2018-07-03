/******************************************************************************
****    
****    Copyright (c) 1997, 1998
****    Rockwell International
****    All Rights Reserved
****    
****    CONFIDENTIAL and PROPRIETARY --
****            No Dissemination or use without prior written permission.
****    
****    Technical Contact:
****            Rockwell Semiconductor Systems
****            Personal Computing Division
****            Huntsville Design Center
****            6703 Odyssey Drive, Suite 303
****            Huntsville, AL   35806
****   
*******************************************************************************


MODULE NAME:
        Buffer Management

FILE NAME:
        BufMgmt.h

ABSTRACT:
        Contains public data structures and prototypes for buffer management

KEYWORDS:
        $Archive:: /Projects/Tigris and Derivatives/Reference Designs/T $
        $Revision: 1 $
        $Date: 1/09/01 10:53a $

******************************************************************************/

#ifndef _BUFMGMT_H_
#define _BUFMGMT_H_

// #include <ndis.h>	// Already included by cardmgmt.h
// #include <ndiswan.h>	// Already included by cardmgmt.h
#include "CardMgmt.h"
#include "FrameAL.h"
#include "Common.h"

// *********************************
// TX buffer functions
// *********************************
GLOBAL NDIS_STATUS BufMgmtSend
(
    IN CDSL_VC_T*		pVCStr,		// identifies the connection
    IN SK_BUFF_T*		BufDescPtr	// Buffer descriptor that can be filled
);

GLOBAL SK_BUFF_T *BufMgmtTxGetFullBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    IN	SK_BUFF_T		*BufDescPtr		// Buffer descriptor that can be filled
);

GLOBAL void BufMgmtTxPutEmptyBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    IN	SK_BUFF_T		*BufDescPtr		// Buffer descriptor that has been transmitted
);


// *********************************
// RX buffer functions
// *********************************

GLOBAL SK_BUFF_T *BufMgmtRxGetEmptyBuf
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
);

GLOBAL void BufMgmtRxPutFullBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    IN	SK_BUFF_T	*BufDescPtr		// Buffer descriptor containing a received frame
);

GLOBAL void BufMgmtRxPutEmptyBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    IN	SK_BUFF_T	*BufDescPtr		// Buffer descriptor containing no received data
);

// *********************************
// TX/RX Buffer Management functions
// *********************************

GLOBAL NDIS_STATUS BufMgmtCfgInit
(
    IN	CDSL_ADAPTER_T	*AdprCtx,		// Driver supplied adapter context
    // (&ThisAdapter)  Required for events that
    // are not associated with a link
    IN PTIG_USER_PARAMS  pUserParams
);

GLOBAL NDIS_STATUS BufMgmtAdapterInit
(
    IN	CDSL_ADAPTER_T	*AdprCtx		// Driver supplied adapter context
    // (&ThisAdapter)
);

GLOBAL NDIS_STATUS BufMgmtAdapterShutdown
(
    IN	CDSL_ADAPTER_T	*AdprCtx		// Driver supplied adapter context
    // (&ThisAdapter)
);

GLOBAL NDIS_STATUS BMDeviceSpecific(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN BACK_DOOR_T			* pBackDoorBuf,
    OUT PULONG				pNumBytesNeeded,
    OUT PULONG				pNumBytesWritten );

GLOBAL NDIS_STATUS BufMgmtLinkInit
(
    IN	CDSL_LINK_T		*LinkCtx		// Link context of data stream
);

GLOBAL NDIS_STATUS BufMgmtLinkShutdown
(
    IN	CDSL_LINK_T		*LinkCtx		// Link context of data stream
);

GLOBAL NDIS_STATUS BufMgmtVcInit
(
    IN	CDSL_VC_T		*VcCtx			// VC context of data stream
);

GLOBAL NDIS_STATUS BufMgmtVcShutdown
(
    IN	CDSL_VC_T		*VcCtx			// VC context of data stream
);

GLOBAL NDIS_STATUS BMSetInformationHdlr
(
    IN CDSL_ADAPTER_T	*AdprCtx,
    IN VOID				*InfoBuffer,
    IN DWORD			InfoBufferLength,
    OUT DWORD			*BytesRead,
    OUT DWORD			*BytesNeeded
);

#endif	/* prevent nested inclusions */

//===================================================================================
//	This is the Buffer Management vector table. These are the the generic
//	functions that will be called from within the Card Management module.
//	The function prototypes are defined in CardMgmt.h. If a module requires one
//	of these predefined functions then its name is placed into the #define
//	otherwise NULL is entered. In this way a consistent interface is maintained
//	with the Card Management module who will call these functions.
//===================================================================================
#define BUFMGMT_DESCRIPTION	"Buffer Management Module"
#define BUFMGMT_FNS	{											\
	BUFMGMT_DESCRIPTION,	/*Module Description			*/	\
	BufMgmtCfgInit,			/*Module CfgInit Function		*/	\
	NULL,					/*Module AdapterInit Function	*/	\
	BufMgmtAdapterShutdown,	/*Module shutdown function		*/	\
	BMDeviceSpecific        /* Ioctl handler */                 \
}
