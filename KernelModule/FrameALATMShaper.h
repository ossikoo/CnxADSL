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
**		Traffic Shaper for the ATM Layer of FrameAL
**
**	FILE NAME:
**		FrameALATMShaper.h
**
**	ABSTRACT:
**		This files contains interface definitions for FrameAL ATM Traffic Shaper 
**		layer function.	This interface consists of link and VC structures 
**		and function prototypes.
**
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/FrameALATMShaper.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/

#ifndef _FRAMEALATMSHAPER_H_
#define _FRAMEALATMSHAPER_H_

/*******************************************************************************
** FRACTOIDS
*******************************************************************************/
// Fractoids
typedef DWORD FRACTOID ;
#define FRACTIZE(I) ((FRACTOID)I<<16)
#define DEFRACTIZE(F) (F>>16)




typedef enum
{
    TRAFFIC_SHAPING_OFF = 0,
    TRAFFIC_SHAPING_ACTIVE,
    TRAFFIC_SHAPING_NOCHANGE
}FRAMEAL_SHAPER_STATUS;


typedef struct	_FRAMEAL_ATM_VC_SHAPER_S
{

	FRACTOID		PeakStepTotal;	  // used for allocated bandwidth
	FRACTOID		PeakStepSize;
	FRACTOID		MaxPeakStepTotal;

	FRACTOID		SubstainedStepTotal;	 // used for unallocated bandwidth
	FRACTOID		SubstainedStepSize;
	FRACTOID		MaxSubstainedStepTotal;

	DWORD			VCRequestSubstainedRateCePS;

	DWORD			VCRequestPeakRateCePS;


	BOOLEAN			VCActive;
	BOOLEAN			UnScaleable; 	// TRUE = CBR

	DWORD			ExceededMaxTotalPeak;
	DWORD			ExceededMaxTotalSubstained;

	FRACTOID		SlotTableBandwidth;
	FRACTOID		SlotTableBandwidthTotal;

	DWORD			VCSlotsUsed;


} FRAMEAL_ATM_VC_SHAPER_T;

#define SLOTS_PER_VC 52  // 832k/16k
#define SLOT_TABLE_SIZE (SLOTS_PER_VC * MAX_VC_PER_LINK)

typedef struct VCSLOT_ENTRY_S
{
	#define VCSLOT_EMPTY 0xff
	WORD	VC;  		// VC assigned to this slot
	WORD	NextVC; 	// next slot to try to find a VC with data to send
} VCSLOT_ENTRY_T;

typedef struct _FRAMEAL_ATM_LINK_SHAPER_PARMS_S
{
	DWORD 		TXClientLimit;
	DWORD		TimerValueUpper;
	DWORD		TImerValueLower;

} FRAMEAL_ATM_LINK_SHAPER_PARMS_S;


typedef struct	_FRAMEAL_ATM_LINK_SHAPER_S
{

	DWORD			LineRateCePS;
	DWORD			VCsActive;
	DWORD			CurrentSlot;
	DWORD			VCHighestBandwidth;
	DWORD			LineCheckCounter;
	FRACTOID		CellSlots;
	VCSLOT_ENTRY_T	VCSlotTable[SLOT_TABLE_SIZE];
	DWORD			VCSlotTableEnd;


} FRAMEAL_ATM_LINK_SHAPER_T;



GLOBAL NDIS_STATUS FrATMShLinkInit
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
);


GLOBAL NDIS_STATUS FrATMShLinkShutdown
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
);


GLOBAL FRAMEAL_SHAPER_STATUS FrATMShVCInit
(
    IN	CDSL_VC_T	*VCHdl			// Link Handle to data stream
);


GLOBAL FRAMEAL_SHAPER_STATUS FrATMShVCShutdown
(
    IN	CDSL_VC_T	*VCHdl			// Link Handle to data stream
);


GLOBAL void FrATMShGetCell
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
);


GLOBAL FRAMEAL_SHAPER_STATUS  FrATMShUpStepSizes
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
);

GLOBAL FRAMEAL_SHAPER_STATUS FrATMShTimerHandler
(
    IN	CDSL_LINK_T		*LinkHdl, 	// Link Handle to data stream
    OUT	BOOLEAN			*NeedStartTx
);

GLOBAL FRAMEAL_SHAPER_STATUS FrATMShIncrStepSize
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    OUT	BOOLEAN			*NeedStartTx

);

#endif //  _FRAMEALATMSHAPER_H_
