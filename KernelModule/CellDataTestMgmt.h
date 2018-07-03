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
        ATM Cell Management Layer

FILE NAME:
        CellDataTestMgmt.h

ABSTRACT:
        Contains public data structures and prototypes for ATM Cell Management Layer

KEYWORDS:
        $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CellDataTestMgmt.h $
        $Revision: 1 $
        $Date: 1/09/01 10:53a $

******************************************************************************/

#ifndef _CELLDATATESTMGMT_H_
#define _CELLDATATESTMGMT_H_

#include "Common.h"


#define		RXCELLALLOCLIMIT	256
#define		TXCELLALLOCLIMIT	20

// ***************************************
//		FILE STRUCTURES
// ***************************************

typedef struct
{
	// ******************************************
	// Cell Management Module VC Context structure
	// ******************************************

	//********************************
	// TX Cell Management Parameters
	//********************************

	// Empty Cells waiting to be filled
	SK_BUFF_QUEUE_T	 	TXCellRdyQ;

	// Cells currently submitted to the controller for transmission.
	SK_BUFF_QUEUE_T		TXCellInuseQ;

	// The limit on the number of transmit cell buffers that can be allocated
	DWORD 			TXCellAllocLimit;

	// The number of transmit cell buffers that have been allocated
	DWORD 			TXCellAllocCnt;

	SK_BUFF_T		*CellMgmtTxCell;

	// Transmit cells currently in our process
	DWORD 			TXCellInprocCnt;

	//********************************
	// RX Cell Management Parameters
	//********************************

	// Empty Receive Cells that can be filled when data is received.
	SK_BUFF_QUEUE_T 		RXCellRdyQ;

	// Receive Cells given to the controller to be filled with data
	SK_BUFF_QUEUE_T 		RXCellInuseQ;

	SK_BUFF_T		*CellMgmtRxCell;

	BYTE			*CellMgmtRx48BytePtr;

	// The limit on the number of receive cell buffers that can be allocated
	DWORD 			RXCellAllocLimit;

	// The number of receive cell buffers that have been allocated
	DWORD 			RXCellAllocCnt;

} CDSL_VC_CELL_MGMT_T;

// *********************************
// CellDataTestMgmt misc functions
// *********************************


GLOBAL NDIS_STATUS CellDataTestMgmtVCInit
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
);

GLOBAL NDIS_STATUS CellDataTestMgmtVCShutdown
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
);


// *********************************
// CellDataTestMgmt TX functions
// *********************************
GLOBAL NDIS_STATUS CellDataTestMgmtSend
(
    IN CDSL_VC_T		*VcCtx,			// VC context of data stream
    IN SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be sent
);

GLOBAL SK_BUFF_T	 *CellDataTestMgmtTxGetFullBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be processed
);

GLOBAL void CellDataTestMgmtTxPutEmptyBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be processed
);

// *********************************
// CellDataTestMgmt RX functions
// *********************************

GLOBAL void CellDataTestMgmtRxPutFullBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be processed
);

GLOBAL void CellDataTestMgmtRxPutEmptyBuf
(
    IN	CDSL_VC_T 		*VcCtx,			// VC context of data stream
    I_O	SK_BUFF_T		*BufDescPtr		// Buffer descriptor to be processed
);

GLOBAL void CellDataTestMgmtRxPutBufComplete
(
    IN	CDSL_VC_T 		*VcCtx			// VC context of data stream
);


#endif	/* prevent nested inclusions */
