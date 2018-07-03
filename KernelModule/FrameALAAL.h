/******************************************************************************
****	Copyright (c) 1997, 1998, 1999
****	Conexant Systems Inc. (formerly Rockwell Semiconductor Systems)
****	Personal Computing Division
****	All Rights Reserved
****
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****
*******************************************************************************


MODULE NAME:
        ATM Abstraction Layer

FILE NAME:
        AAL.h

ABSTRACT:
        Contains public data structures and prototypes for ATM Abstraction Layer

KEYWORDS:
        $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/FrameALAAL.h $
        $Revision: 1 $
        $Date: 1/09/01 10:54a $

******************************************************************************/

#ifndef _FRAMEALAAL_H_
#define _FRAMEALAAL_H_

// #include <ndis.h>	// Already included by cardmgmt.h
// #include <ndiswan.h>	// Already included by cardmgmt.h
#include "CardMgmt.h"
#include "Common.h"

// *********************************
// AAL Timer and other misc functions
// *********************************

GLOBAL BOOLEAN FrameALAALRASTimerExpiry
(
    IN	void	 		*VcCtx			// VC context of data stream
);

GLOBAL NDIS_STATUS FrameALAALVCInit
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
);

GLOBAL NDIS_STATUS FrameALAALVCShutdown
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
);


// *********************************
// AAL TX functions
// *********************************
GLOBAL SK_BUFF_T	 *FrameALAALTxGetFullBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be processed
);

GLOBAL void FrameALAALTxPutEmptyBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be processed
);

// *********************************
// AAL RX functions
// *********************************
GLOBAL void FrameALAALRxPutFullBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be processed
);

#endif	/* prevent nested inclusions */
