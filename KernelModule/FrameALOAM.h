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
**		ATM OAM Layer of FrameAL
**
**	FILE NAME:
**		FrameALATM.h
**
**	ABSTRACT:
**		This files contains functions for FrameAL ATM OAM Layer. This includes
**		F4 and F5 Loop Back processint.  		  
**
**	DETAILS:
**		FrameALATM.c directs all OAM cells to this module, this module
**      generates reply cells and counts statistics. 
**

*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/FrameALOAM.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/

#ifndef _FRAMEALATMOAM_H_
#define _FRAMEALATMOAM_H_

typedef enum OAM_TYPE_E
{
    OAM_TYPE_FAULT_MANAGEMENT =1,
    OAM_TYPE_PERFORM_MANAGEMENT=2,
    OAM_TYPE_ACT_DEACT_MANAGEMENT=8,
    OAM_TYPE_SYSTEM_MANAGEMENT=0xF
}OAM_TYPE_T;

typedef enum OAM_FUNCTION_FAULT_E
{
    OAM_FUNCTION_FAULT_AIS = 0,
    OAM_FUNCTION_FAULT_RDI,
    OAM_FUNCTION_FAULT_CONTINUITY_CHECK,
    OAM_FUNCITON_FAULT_LOOPBACK =8
} OAM_FUNCTION_FAULT_T;


//
// Force misalignment of the following structures
//
// #include <pshpack1.h>


typedef struct _OAM_LOOPBACK_S
{
BYTE 		LoopBackInd:8;
DWORD 		CorrelationTag:32;
	BYTE 		LocationID[16];
	BYTE	    SoureID[16];
	BYTE		Unused[8];
} OAM_LOOPBACK_T;


typedef struct _OAM_MESSAGE_S
{
	BYTE		FunctNType;

	//OAM_FUNCTION_FAULT_T	FunctionType:4;
	//OAM_TYPE_T 				OAMType:4;

	//union
	//{
	//	OAM_LOOPBACK_T	OAMLoopBack;
	//	BYTE			Payload[45];
	//} Message;
BYTE 		LoopBackInd:8;
	BYTE 		CorrelationTag[4];
	BYTE 		LocationID[16];
	BYTE	    SoureID[16];
	BYTE		Unused[8];

	// 	OAMReserved:6;
	BYTE	OAMCRC10[2]; //:10;

} OAM_MESSAGE_T;

// #include <poppack.h>

typedef struct 	_FRAMEAL_ATM_OAM_CELL_S
{
	CDSL_VC_T * 			VCHdl;
	BYTE 					LoopBackHeader[5];
	OAM_MESSAGE_T			Data;
	DWORD					DataCount;
	BYTE					*BufferPointer;

} FRAMEAL_ATM_OAM_CELL_T;



typedef struct	_FRAMEAL_ATM_OAM_VC_T
{
	BOOLEAN					CellToSend;

	FRAMEAL_ATM_OAM_CELL_T	OAMCell;

	DWORD					InitCellsToSend;
	FRAMEAL_ATM_OAM_CELL_T	InitVCOAMCell;
} FRAMEAL_ATM_OAM_VC_T;


typedef struct	_FRAMEAL_ATM_OAM_LINK_T
{
	BOOLEAN					CellToSend;
	WORD 					Crc10Table [256];
	BOOLEAN					LinkUpCellToSend;

	DWORD 					CorrelationTag;
	FRAMEAL_ATM_OAM_CELL_T	OAMCell;
	LONG_CTR_T  			OAMTXOverrun;
	LONG_CTR_T  			OAMUnhandledCells;
	LONG_CTR_T  			OAMBadCRC10Cells;
	LONG_CTR_T  			OAMInvalidLoopback;
} FRAMEAL_ATM_OAM_LINK_T;


GLOBAL SK_BUFF_T	*FrameALOAMLinkGetTxFullBuffer
(
	IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
	IN 	SK_BUFF_T		*pPacket
);


GLOBAL VOID FrameALOAMRxBuffer
(
	IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
	IN	CDSL_VC_T		*VCHdl,			// VC Handle (Optional) for F5 cells
	IN	SK_BUFF_T		*InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
);


GLOBAL NDIS_STATUS FrameALATMOAMLinkInit
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
);


GLOBAL NDIS_STATUS FrameALATMOAMLinkShutdown
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
);


GLOBAL NDIS_STATUS FrameALATMOAMVCInit
(
    IN	CDSL_VC_T	*VCHdl				// VC Handle to data stream
);


GLOBAL NDIS_STATUS FrameALATMOAMVCShutdown
(
    IN	CDSL_VC_T	*VCHdl				// VC Handle to data stream
);

GLOBAL void FrameALATMOAMLinkUp
(
    IN	CDSL_LINK_T		*LinkHdl		// send out OAM cell when showtime happens

);

#endif //  _FRAMEALATMOAM_H_
