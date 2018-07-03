/******************************************************************************
*******************************************************************************
****	Copyright (c) 1998
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
**		ATM Layer of FrameAL
**
**	FILE NAME:
**		FrameALATM.h
**
**	ABSTRACT:
**		This files contains interface definitions for FrameAL ATM layer function.
**		Those functions Traffic Shaping, Mux/DeMux and CellAlign functions. 
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/FrameALATM.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/

#ifndef _FRAMEALATM_H_
#define _FRAMEALATM_H_

#if PROJECTS_NDIS_IF == PROJECT_NDIS_IF_CO
	#include "FrameALATMCoShaper.h"
#else
	#include "FrameALATMShaper.h"
#endif

#define PRECOMPUTED_HEADER_NUM 16

// ATM header Structure


//
// Force misalignment of the following structures
//
// #include <pshpack1.h>

typedef struct _ATM_HEADER_T
{
unsigned int	VPI_UPPER: 4;
unsigned int    GFC:4;
unsigned int	VCI_UPPER: 4;
unsigned int	VPI_LOWER: 4;
unsigned int	VCI_MIDDLE: 8;

unsigned int 	CLP:1;
unsigned int	PTI:3;
#define PTI_USER_BIT 		1
#define PTI_CI_BIT 			2
#define PTI_MGM_BIT			4

unsigned int	VCI_LOWER: 4;


unsigned int	HEC:8;
} ATM_HEADER_T, *P_ATM_HEADER_T;



// #include <poppack.h>

#define	CELL_PAYLOAD_SIZE	48

#define IDLE_PAYLOAD_CHAR	0x6a

#define	CELL_HEADER_SIZE	5


#define SET_VPI(HeaderPtr, VPI)  HeaderPtr->VPI_UPPER = (( VPI>>4 ) & 0xf); \
							  HeaderPtr->VPI_LOWER = ( VPI & 0xf); 

#define GET_VPI(HeaderPtr)   (HeaderPtr->VPI_UPPER << 4)  + (HeaderPtr->VPI_LOWER)

#define SET_VCI(HeaderPtr, VCI) HeaderPtr->VCI_UPPER = ((VCI>>12) & 0xf);  \
								HeaderPtr->VCI_MIDDLE = (( VCI>>4 ) & 0xff); \
								HeaderPtr->VCI_LOWER = ( VCI & 0xf) ;

#define GET_VCI(HeaderPtr)  ((HeaderPtr->VCI_UPPER << 12) \
							+ (HeaderPtr->VCI_MIDDLE << 4) \
							+ (HeaderPtr->VCI_LOWER))



#define ATM_HEADER_F5END_TO_END 5*2



typedef struct	_FRAMEAL_ATM_VC_T
{

	DWORD					CellPerBucket;		// cells sent in this time interval

	DWORD					CellPerBucketThreshold;  // Maxs cells sent in an interval


	ATM_HEADER_T			PreCompHeaders[PRECOMPUTED_HEADER_NUM];
	// fill in the four possible headers
	// differnce user bit and Congestion bit
	// same VCI/VPI and GFC

	WORD					CellsToSap;

#define FRAMEALVC_CNXT_NO 		4
#define FRAMEALVC_CNXT_PUTTX 	0
#define FRAMEALVC_CNXT_GETTX	1
#define FRAMEALVC_CNXT_PUTRX	2
#define FRAMEALVC_CNXT_RXCOMP	3

	BOOLEAN					InVCALCNXT[FRAMEALVC_CNXT_NO];


	FRAMEAL_ATM_VC_SHAPER_T	VCShaper;

	FRAMEAL_ATM_OAM_VC_T	OAMVc;
	// function pointer for SAP service routines

	SK_BUFF_T*				(*TxGetFullBuf) (CDSL_VC_T*,SK_BUFF_T*);
	VOID					(*TxPutEmptyBuf)(CDSL_VC_T*,SK_BUFF_T*);
	VOID					(*RxPutFullBuf) (CDSL_VC_T*,SK_BUFF_T*);
	VOID					(*RxPutBufComplete)(CDSL_VC_T*,SK_BUFF_T*);


} FRAMEAL_ATM_VC_T, *pFRAMEAL_ATM_VC_T;

typedef struct	_FRAMEAL_ATM_LINK_T
{
	BOOLEAN					BlockedForTrafficShaping;

	BYTE					IdleCharacter;

	WORD					CellAlignState;

	// link receive data
	ATM_HEADER_T			RxCellHeader;
	DWORD					RxHeaderCount;

	DWORD					RxPayloadCount;



	WORD					CurrentVC;

	WORD					LinkState;	// does the link need to send a start tx or not

	FRAMEAL_SHAPER_STATUS	TrafficShapingActive;

	FRAMEAL_ATM_LINK_SHAPER_T LinkShaper;

	ATM_HEADER_T			IdleHeader;

	BYTE					IdlePayload[CELL_PAYLOAD_SIZE];

	ATM_HEADER_T			LinkOAMHeader;

	BYTE					LinkOAMPayload[CELL_PAYLOAD_SIZE];


	FRAMEAL_ATM_OAM_LINK_T  OAMLink;
} FRAMEAL_ATM_LINK_T;


GLOBAL SK_BUFF_T	*FrameALATMGetTxFullBuffer
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
);


GLOBAL SK_BUFF_T	*FrameALATMTxPutEmptyBuf
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
);


GLOBAL SK_BUFF_T	*FrameALATMCellAlign
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T	*BufferDescPtr	// Pointer to a buffer descriptor
);


GLOBAL NDIS_STATUS FrameALATMLinkInit
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
);


GLOBAL NDIS_STATUS FrameALATMLinkShutdown
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
);


GLOBAL NDIS_STATUS FrameALATMVCInit
(
    IN	CDSL_VC_T	*VCHdl				// VC Handle to data stream
);


GLOBAL NDIS_STATUS FrameALATMVCShutdown
(
    IN	CDSL_VC_T	*VCHdl				// VC Handle to data stream
);

GLOBAL NDIS_STATUS FrameALATMStartTx
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
);

GLOBAL void FrameALATMGetTXCell
(
    IN	CDSL_VC_T	*VCHdl,				// VC Handle to data stream
    IN	SK_BUFF_T	*BufferDescPtr		// Pointer to a buffer descriptor
);

GLOBAL void FrameALATMTimerHandler
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
);

GLOBAL void FrameALATMLinkUp
(
    IN	CDSL_LINK_T		*LinkHdl
);

GLOBAL void FrameALATMLinkDown
(
    IN	CDSL_LINK_T		*LinkHdl
);

#endif //  _FRAMEALATM_H_
