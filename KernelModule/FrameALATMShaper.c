/******************************************************************************
*******************************************************************************
****	Copyright (c) 1998, 1999
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
**		Traffic Shaper for the ATM Layer of FrameAL
**
**	FILE NAME:
**		FrameALATMShaper.c
**
**	ABSTRACT:
**		This files contains functions to implement the FrameAL ATM Traffic  
**		shaper.	
**
**
**	DETAILS:
** 		The Traffic Shaper has three parts. 
**		The first part computes the bandwidth increments used track the 
**		bandwidth used by VCs. 
**
**		The second part is increments the available bandwidth of the the active
**		VCs. Every time the hobbs general time expires, all active VCs are given
**      their bandwidth allotment.
** 
**		The third part is selecting the next VC to produce a cell based on 
**		available bandwidth. Each time FrameALATM needs a cell, Shaper checks 
**		the available bandwidth of each active VC, when a VC is found with 
**		bandwidth to send, Shaper queries the VC for a cell. If the VC does not 
**		have a cell to tansmit, Shaper looks for another VC with bandwidth to 
**		send. 
**
** 		 
**			
**		
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/FrameALATMShaper.c $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/

//*****************************************************************************
//   Include Files
//*****************************************************************************
#ifdef FRAMEAL_SHAPER_SIM
	#include "shapersim\Framealsim.h"
#else
	#include "CardMgmt.h"
	#include "FrameAL.h"
#endif 

//*****************************************************************************
//   LOCAL defines
//*****************************************************************************


// convert the PCR rate from the control panel to kilobits per second
#define CONVERT_PCR_TO_SHAPER_RATE 			16

// MIN and MAX burst size, check bounds of Registy value
#define MIN_BURST_SIZE						2
#define MAX_BURST_SIZE						0xfff

// Enable Peak Cell Rate traffic shaping
#define PCR_ENABLED				1

// Max number of slots a VC operating at line rate will
#define MAX_TABLE_SLOTS			SLOTS_PER_VC

//*****************************************************************************
//   LOCAL functions prototypes.
//*****************************************************************************
LOCAL DWORD	FrATMShGetLineRate(  IN	CDSL_LINK_T	*LinkHdl);
LOCAL void FrATMShBuildSlotTable(  IN	CDSL_LINK_T	*LinkHdl);


/*******************************************************************************
FUNCTION NAME:
	FrATMShLinkInit

ABSTRACT:	
	Initializes a new link instance for Mux/Demux.  

RETURN:
	NDIS status

DETAILS:
	This function initialize traffic shaping counters.
	Insure that traffic shaping is inactive until a VC becomes active. 
		
	
*******************************************************************************/

GLOBAL NDIS_STATUS FrATMShLinkInit
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
)
{
	FRAMEAL_LINK_T * pFrameALLink;

	if (LinkHdl != NULL)
	{
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;

		if (pFrameALLink != NULL)
		{
			pFrameALLink->ATMLinkBlock.LinkShaper.LineRateCePS		= 0;
			pFrameALLink->ATMLinkBlock.LinkShaper.CurrentSlot		= 0;
			pFrameALLink->ATMLinkBlock.LinkShaper.LineCheckCounter 	= 0;
		}
	}

	return STATUS_SUCCESS;
}

/*******************************************************************************
FUNCTION NAME:
	FrATMShLinkShutdown

ABSTRACT:	
	Shutdowns an ATM layer's link Shaper instance.  

RETURN:
	NDIS Status

DETAILS:
	Shut down traffic shaping so if an timer happens before a complete 
	clean up a problem does not occur. 

*******************************************************************************/
GLOBAL NDIS_STATUS FrATMShLinkShutdown
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
)
{
	FRAMEAL_LINK_T * pFrameALLink;

	if (LinkHdl != NULL)
	{
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;

		if (pFrameALLink != NULL)
		{
			pFrameALLink->ATMLinkBlock.LinkShaper.LineRateCePS		= 0;
			pFrameALLink->ATMLinkBlock.LinkShaper.CurrentSlot		= 0;
			pFrameALLink->ATMLinkBlock.LinkShaper.LineCheckCounter 	= 0;
		}
	}

	return STATUS_SUCCESS;
}


/*******************************************************************************
FUNCTION NAME:
	FrATMShVCInit

ABSTRACT:	
	Add a VC to the traffic shaper.  

RETURN:
	FRAMEAL_SHAPER_STATUS

DETAILS:
	This function initializes the FrameALATM Shapers VC data structure.
	The VCs cell rate is converted from 16k bps increments to cells per
	second, the cell rate is saved as both PCR and SCR. 
	This function also saves the burst size for both PCR and SCR
	and other fields are cleared. 
	Once the data structure is ready, the Step size update function is called
	and all VCs step sizes are updated. 
*******************************************************************************/

GLOBAL FRAMEAL_SHAPER_STATUS FrATMShVCInit
(
    IN	CDSL_VC_T	*VCHdl			// Link Handle to data stream
)
{
	FRAMEAL_VC_T * pFrameALVC;
	FRAMEAL_LINK_T * pFrameALLinkT=NULL;

	if (VCHdl != NULL)
	{
		pFrameALVC 		= (FRAMEAL_VC_T*)VCHdl->FrameALVCMod;

		if (pFrameALVC != NULL)
		{
			// update burst size if it is in limits
			if ((VCHdl->LinkHdl != NULL) && (VCHdl->LinkHdl->FrameMod != NULL))
			{
				pFrameALLinkT = (FRAMEAL_LINK_T*)VCHdl->LinkHdl->FrameMod;

				// save a default burst size for PCR and SCR
				// if the burst size in the registy
				pFrameALVC->ATMVCBlock.VCShaper.MaxPeakStepTotal =
				    FRACTIZE(pFrameALLinkT->FrameALShaperInfo.DefaultBurstSize);

				pFrameALVC->ATMVCBlock.VCShaper.MaxSubstainedStepTotal =
				    FRACTIZE(pFrameALLinkT->FrameALShaperInfo.DefaultBurstSize);

				if ((pFrameALLinkT->Parameters.CellBurstSize > MIN_BURST_SIZE) &&
				        (pFrameALLinkT->Parameters.CellBurstSize < MAX_BURST_SIZE))
				{
					pFrameALVC->ATMVCBlock.VCShaper.MaxPeakStepTotal =
					    FRACTIZE(pFrameALLinkT->Parameters.CellBurstSize);
					pFrameALVC->ATMVCBlock.VCShaper.MaxSubstainedStepTotal =
					    FRACTIZE(pFrameALLinkT->Parameters.CellBurstSize);
				}
			}

			//	get cell rate from entered cell rate value
			if ((pFrameALVC->Parameters.PeakCellRate != 0) &&
			        ((pFrameALVC->Parameters.PeakCellRate * 53*8) <	
						pFrameALLinkT->FrameALShaperInfo.ClientLimit))
			{
				pFrameALVC->ATMVCBlock.VCShaper.VCRequestPeakRateCePS =
				    pFrameALVC->Parameters.PeakCellRate ;
			}
			else
			{
				pFrameALVC->Parameters.PeakCellRate = SLOTS_PER_VC;
				pFrameALVC->ATMVCBlock.VCShaper.VCRequestPeakRateCePS =
				    pFrameALLinkT->FrameALShaperInfo.ClientLimit/(53*8);
			}

			pFrameALVC->ATMVCBlock.VCShaper.VCRequestSubstainedRateCePS	=
			    pFrameALVC->ATMVCBlock.VCShaper.VCRequestPeakRateCePS;
			pFrameALVC->ATMVCBlock.VCShaper.SubstainedStepTotal = FRACTIZE(1);
			pFrameALVC->ATMVCBlock.VCShaper.PeakStepTotal = FRACTIZE(1);
			pFrameALVC->ATMVCBlock.VCShaper.VCActive = TRUE;

			//	compute step size (bandwidth) for all VC
			return FrATMShUpStepSizes ( VCHdl->LinkHdl);
		}
	}

	return TRAFFIC_SHAPING_NOCHANGE;
}


/*******************************************************************************
FUNCTION NAME:
	FrATMShVCShutdown

ABSTRACT:	
	Removes a VC from the list of active VCs that are being traffic shaped.

RETURN:
	FRAMEAL_SHAPER_STATUS

DETAILS:
	Clears the VC data structure and removes the VC from list VCs being 
	traffic shaped. 
*******************************************************************************/

GLOBAL FRAMEAL_SHAPER_STATUS FrATMShVCShutdown
(
    IN	CDSL_VC_T	*VCHdl			// Link Handle to data stream
)
{
	FRAMEAL_VC_T * pFrameALVC;

	if (VCHdl != NULL)
	{
		pFrameALVC 		= (FRAMEAL_VC_T*)VCHdl->FrameALVCMod;

		if (pFrameALVC != NULL)
		{
			// clear VC data structure
			pFrameALVC->ATMVCBlock.VCShaper.VCRequestPeakRateCePS =   0;
			pFrameALVC->ATMVCBlock.VCShaper.VCRequestSubstainedRateCePS =   0;
			pFrameALVC->ATMVCBlock.VCShaper.SubstainedStepTotal = 0;
			pFrameALVC->ATMVCBlock.VCShaper.PeakStepTotal = 0;
			pFrameALVC->ATMVCBlock.VCShaper.ExceededMaxTotalPeak	= 0;
			pFrameALVC->ATMVCBlock.VCShaper.ExceededMaxTotalSubstained	= 0;
			pFrameALVC->ATMVCBlock.VCShaper.VCActive = FALSE;

			return FrATMShUpStepSizes ( VCHdl->LinkHdl);
		}
	}

	return TRAFFIC_SHAPING_NOCHANGE;
}


/*******************************************************************************
FUNCTION NAME:
	FrATMShUpStepSizes

ABSTRACT:	
	Recomputes step sizes for Initializes a new link instance for Mux/Demux.  

RETURN:
	FRAMEAL_SHAPER_STATUS

DETAILS:
	This function gets the current line rate and computes the step size for 
	all active VCs. If traffic shaping is needed, this function also updates 
	the size of the slot interval, which is timer interval*cell interval.
	
******************************************************************************/

GLOBAL FRAMEAL_SHAPER_STATUS FrATMShUpStepSizes
(
    IN	CDSL_LINK_T	*LinkHdl			// Link Handle to data stream
)
{
	DWORD				LineRateBps;
	FRAMEAL_LINK_T 	* 	pFrameALLink;
	FRAMEAL_VC_T 	* 	pFrameALVC;
	CDSL_VC_T		* 	VCHdl;
	DWORD				VCIndex;
	BOOLEAN				VCsNeedTrafficShaping = FALSE;

	if (LinkHdl != NULL)
	{
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;
		if (pFrameALLink != NULL)
		{
			LineRateBps = FrATMShGetLineRate(LinkHdl);
			pFrameALLink->ATMLinkBlock.LinkShaper.LineRateCePS	= LineRateBps/(53*8);

			//
			// Get Hobbs general timer value
			//
			pFrameALLink->ATMLinkBlock.LinkShaper.CellSlots  =
			    FRACTIZE(pFrameALLink->ATMLinkBlock.LinkShaper.LineRateCePS *
			             pFrameALLink->FrameALShaperInfo.TimerIntervalUpper)
			    / pFrameALLink->FrameALShaperInfo.TimerIntervalLower;

			// check that traffic shaping is needed and valid (not highspeed server)
			if (( LineRateBps > pFrameALLink->FrameALShaperInfo.ClientLimit)  || (LineRateBps == 0))
			{
				#ifndef FRAMEAL_SHAPER_SIM
				pFrameALLink->ATMLinkBlock.LinkShaper.CellSlots  = 0;
				return TRAFFIC_SHAPING_OFF;
				#endif
			}

			// Update VC scaling factores
			// each VC has two scaling factors, one for scaled bandwidth and one for max bandwidth
			//
			for
			(
			    VCIndex = 0;
			    VCIndex < MAX_VC_PER_LINK;
			    VCIndex++
			)
			{
				// check that this VC is active
				VCHdl = &LinkHdl->VC_Info[VCIndex];

				if (VCHdl != NULL)
				{
					pFrameALVC =  (FRAMEAL_VC_T *) VCHdl->FrameALVCMod;

					if ((pFrameALVC != NULL) && pFrameALVC->ATMVCBlock.VCShaper.VCActive)
					{
						// look for at least one VC which requires traffic shaping
						if (pFrameALVC->Parameters.PeakCellRate != 0)
						{
							VCsNeedTrafficShaping = TRUE;
						}

						// PCR step size (Bandwidth allocation)
						if (pFrameALVC->ATMVCBlock.VCShaper.VCRequestPeakRateCePS <
						        pFrameALLink->ATMLinkBlock.LinkShaper.LineRateCePS)
						{
							// this VC is active, set its Max bandwidth
							pFrameALVC->ATMVCBlock.VCShaper.PeakStepSize =
							    FRACTIZE( pFrameALVC->ATMVCBlock.VCShaper.VCRequestPeakRateCePS) /
							    pFrameALLink->ATMLinkBlock.LinkShaper.LineRateCePS;
						}
						else
						{
							// largest possible allocation is 1 cell per slot
							pFrameALVC->ATMVCBlock.VCShaper.PeakStepSize =
							    FRACTIZE(1);
						}

						// SCR step size (Bandwidth allocation)
						if (pFrameALVC->ATMVCBlock.VCShaper.VCRequestSubstainedRateCePS <
						        pFrameALLink->ATMLinkBlock.LinkShaper.LineRateCePS)
						{
							// this VC is active, set its Max bandwidth
							pFrameALVC->ATMVCBlock.VCShaper.SubstainedStepSize =
							    FRACTIZE( pFrameALVC->ATMVCBlock.VCShaper.VCRequestSubstainedRateCePS) /
							    pFrameALLink->ATMLinkBlock.LinkShaper.LineRateCePS;
						}
						else
						{
							// largest possible allocation is 1 cell per slot
							pFrameALVC->ATMVCBlock.VCShaper.SubstainedStepSize =
							    FRACTIZE(1);
						}
					}
				}
			}

			// if no VC needs traffic shaping stop traffic shaping
			if (VCsNeedTrafficShaping == FALSE)
			{
				pFrameALLink->ATMLinkBlock.LinkShaper.CellSlots  = 0;
				return TRAFFIC_SHAPING_OFF;
			}
			else
			{
				// atleast one VC needs traffic shaping
				FrATMShBuildSlotTable( LinkHdl);
			}
		}
	}

	return TRAFFIC_SHAPING_ACTIVE;
}


/*******************************************************************************
FUNCTION NAME:
	FrATMShGetCell

ABSTRACT:	
	Get a cell from one of the active VCs.   

RETURN:
	void

DETAILS:
	This function selects the next VC to send a cell. 
	First, the function gets the first VC candidate from the slot table. 
	If that VC candidate has enought bandwidth to send a Cell, 
	the VC is queried for a cell. 
	If the VC candidate does not have enough bandwidth or does not have a 
	cell available, the function checked the slot list for the next VC candidate.
	
*******************************************************************************/

GLOBAL void FrATMShGetCell
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
)
{
	FRAMEAL_LINK_T 	* pFrameALLink;
	FRAMEAL_VC_T 	* pFrameALVC;
	CDSL_VC_T		* VCHdl;
	DWORD			  VCIndex;
	DWORD 			  NumberOFVcTried = 0;

	if (LinkHdl != NULL)
	{
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;
		if (pFrameALLink != NULL)
		{
			// get the first VC candidate
			VCIndex =  pFrameALLink->ATMLinkBlock.LinkShaper.CurrentSlot;

			// update the next slot table for next entry into this function
			if ((pFrameALLink->ATMLinkBlock.LinkShaper.CurrentSlot+1) <
			        pFrameALLink->ATMLinkBlock.LinkShaper.VCSlotTableEnd)
			{
				pFrameALLink->ATMLinkBlock.LinkShaper.CurrentSlot++;
			}
			else
			{
				pFrameALLink->ATMLinkBlock.LinkShaper.CurrentSlot =  0;
			}

			// Search for a VC over the threshold.
			// Stop when a VC produces cell or all VCs have been checked.
			do
			{
				VCHdl = &LinkHdl->VC_Info[pFrameALLink->ATMLinkBlock.LinkShaper.VCSlotTable[VCIndex].VC];

				if (VCHdl != NULL)
				{
					pFrameALVC =  (FRAMEAL_VC_T *) VCHdl->FrameALVCMod;

					if ((pFrameALVC != NULL) && pFrameALVC->ATMVCBlock.VCShaper.VCActive)
					{
						// does the current VC candidate have enough bandwidth to send a cell
						if (pFrameALVC->ATMVCBlock.VCShaper.SubstainedStepTotal > FRACTIZE(1))
							#if PCR_ENABLED
							if (pFrameALVC->ATMVCBlock.VCShaper.PeakStepTotal > FRACTIZE(1))
							#endif 
							{
								// attempt to get a cell from this VC
								FrameALATMGetTXCell( VCHdl, InBufDescPtr);

								if ( InBufDescPtr->len > 0)
								{
									// the VC provided a cell, update total and exit
									pFrameALVC->ATMVCBlock.VCShaper.SubstainedStepTotal -= FRACTIZE(1);
									if ((long)(pFrameALVC->ATMVCBlock.VCShaper.SubstainedStepTotal)  < 0)
									{
										pFrameALVC->ATMVCBlock.VCShaper.SubstainedStepTotal =0;
									}

								#if PCR_ENABLED
									pFrameALVC->ATMVCBlock.VCShaper.PeakStepTotal -= FRACTIZE(1);

									if ((long)(pFrameALVC->ATMVCBlock.VCShaper.PeakStepTotal)  < 0)
									{
										pFrameALVC->ATMVCBlock.VCShaper.PeakStepTotal =0;
									}
								#endif 

									return;
								}
							}
					}
				}

				// last candidate did not produce a cell, try the next VC in the list
				// the slot table contains pointer to the next VC so the same VC candidate is
				// checked more than once.

				VCIndex = pFrameALLink->ATMLinkBlock.LinkShaper.VCSlotTable[VCIndex].NextVC;
				NumberOFVcTried++;

				// if all active VCs have been checked, exit.
			}  	while (NumberOFVcTried < pFrameALLink->ATMLinkBlock.LinkShaper.VCsActive);
		}
	}
}



/*******************************************************************************
FUNCTION NAME:
	FrATMShGetLineRate

ABSTRACT:	
	Get ADSL upstream line rate from cardalcdsl.c .    

RETURN:
	Line rate in BPS

DETAILS:
	Call CardAl module and recover the line rate in Kilobits per second. 
*******************************************************************************/
LOCAL DWORD	FrATMShGetLineRate(  IN	CDSL_LINK_T	*LinkHdl)

{
	WORD			LineSpeed;
	WORD			UpStreamSpeed;
	NDIS_STATUS		TmpStatus;

	TmpStatus =  CardALGetAdslLineSpeed(
	                 LinkHdl->pThisAdapter,
	                 &UpStreamSpeed,
	                 &LineSpeed );

	return (UpStreamSpeed*1000);
}


/*******************************************************************************
FUNCTION NAME:
	FrATMShTimerHandler

ABSTRACT:	
	Called when Hobbs timer expires.
	This function updates credit/bandwidth counters.     

RETURN:
	None

DETAILS:
	
*******************************************************************************/

GLOBAL FRAMEAL_SHAPER_STATUS FrATMShTimerHandler
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    OUT	BOOLEAN			*NeedStartTx
)
{
	return (FrATMShIncrStepSize(LinkHdl, NeedStartTx ));
};


/*******************************************************************************
FUNCTION NAME:
	FrATMShIncrStepSize

ABSTRACT:	
    Update step size totals for each Active VC.   

RETURN:
	FRAMEAL_SHAPER_STATUS

DETAILS:
	Called by timer to add one time intervals worth of credit to each active VC.
	Also, if a VC went over its theshold, this function calls start tx, to 
	get the transmitter running again. 
	Since thie function is called by the timer, this function contains a counter 
	to divide the timer rate down to a time inteval to poll the modem for line 
	rate changes. If the line rate changes, than all VCs bandwidth usage is 
	recalculated. 

*******************************************************************************/

#define CEDIT_UPDATE(SlotNo,StepSize) (StepSize * DEFRACTIZE(SlotNo) + ((StepSize * (SlotNo &0xffff)) >> 16))
GLOBAL FRAMEAL_SHAPER_STATUS FrATMShIncrStepSize
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    OUT	BOOLEAN			*NeedStartTx
)
{
	FRAMEAL_LINK_T 	* pFrameALLink;
	FRAMEAL_VC_T 	* pFrameALVC;
	CDSL_VC_T		* VCHdl;
	DWORD			VCIndex;
	FRACTOID		NewCredit;

	FRAMEAL_SHAPER_STATUS TempShaperStatus =  TRAFFIC_SHAPING_NOCHANGE;
	*NeedStartTx = FALSE;

	if (LinkHdl != NULL)
	{
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;

		if (pFrameALLink != NULL)
		{
			// poll Modem for line rate change
			pFrameALLink->ATMLinkBlock.LinkShaper.LineCheckCounter++;
			if ( pFrameALLink->ATMLinkBlock.LinkShaper.LineCheckCounter++ >  pFrameALLink->FrameALShaperInfo.LineCheckInterval)
			{
				pFrameALLink->ATMLinkBlock.LinkShaper.LineCheckCounter = 0;
				TempShaperStatus =  FrATMShUpStepSizes( LinkHdl);

			}

			if (pFrameALLink->ATMLinkBlock.LinkShaper.CellSlots > 0)
			{
				// Update each active VCs Step Total (bandwidth available)
				for
				(
				    VCIndex = 0;
				    VCIndex < MAX_VC_PER_LINK;
				    VCIndex++
				)
				{
					VCHdl = &LinkHdl->VC_Info[VCIndex];

					if (VCHdl != NULL)
					{
						pFrameALVC =  (FRAMEAL_VC_T *) VCHdl->FrameALVCMod;

						if ((pFrameALVC != NULL) && pFrameALVC->ATMVCBlock.VCShaper.VCActive)
						{
							NewCredit = CEDIT_UPDATE(	pFrameALLink->ATMLinkBlock.LinkShaper.CellSlots,
							                          pFrameALVC->ATMVCBlock.VCShaper.SubstainedStepSize);

							if ( pFrameALVC->ATMVCBlock.VCShaper.SubstainedStepTotal <
							        pFrameALVC->ATMVCBlock.VCShaper.MaxSubstainedStepTotal)
							{
								if (( pFrameALVC->ATMVCBlock.VCShaper.SubstainedStepTotal < FRACTIZE(1))
								        &&
								        ( pFrameALVC->ATMVCBlock.VCShaper.SubstainedStepTotal + NewCredit >	FRACTIZE(1)))
								{
									*NeedStartTx = TRUE;
								}

								pFrameALVC->ATMVCBlock.VCShaper.SubstainedStepTotal += NewCredit;
							}
							else
							{
								pFrameALVC->ATMVCBlock.VCShaper.ExceededMaxTotalSubstained += NewCredit;
							}

							#if PCR_ENABLED
							NewCredit = CEDIT_UPDATE(	pFrameALLink->ATMLinkBlock.LinkShaper.CellSlots,
							                          pFrameALVC->ATMVCBlock.VCShaper.PeakStepSize);

							if ( pFrameALVC->ATMVCBlock.VCShaper.PeakStepTotal <
							        pFrameALVC->ATMVCBlock.VCShaper.MaxPeakStepTotal)
							{
								if (( pFrameALVC->ATMVCBlock.VCShaper.PeakStepTotal < FRACTIZE(1)) &&
								        ( pFrameALVC->ATMVCBlock.VCShaper.PeakStepTotal + NewCredit >	FRACTIZE(1)))
								{
									*NeedStartTx = TRUE;
								}

								pFrameALVC->ATMVCBlock.VCShaper.PeakStepTotal +=  NewCredit;
							}
							else
							{
								pFrameALVC->ATMVCBlock.VCShaper.ExceededMaxTotalPeak += NewCredit;
							}
							#endif 
						}
					}
				}
			}
		}
	}

	// return state of traffic shaper
	// if traffic shaping is no longer allowed (because of line rate)
	// inform FrameALATM.
	return TempShaperStatus;
}



/*******************************************************************************
FUNCTION NAME:
	FrATMShBuildSlotTable

ABSTRACT:	
    Update slot table. Slot table is used to distribute extra bandwidth fairly 
    to the VCs. The bandwidth is distributed proportional to subscribed bandwidth. 
     

RETURN:
	void

DETAILS:
	
*******************************************************************************/

LOCAL void FrATMShBuildSlotTable(  IN	CDSL_LINK_T	*LinkHdl)
{
	FRAMEAL_LINK_T 	* pFrameALLink;
	FRAMEAL_VC_T 	* pFrameALVC;
	CDSL_VC_T		* VCHdl;
	DWORD			VCIndex;
	DWORD			SlotIndex;
	DWORD 			TotalRequestBandWidthCePS = 0;

	if (LinkHdl != NULL)
	{
		pFrameALLink = (FRAMEAL_LINK_T*)LinkHdl->FrameMod;

		if (pFrameALLink != NULL)
		{
			// Total all active bandwidth used by all VC
			for
			(
			    VCIndex = 0;
			    VCIndex < MAX_VC_PER_LINK;
			    VCIndex++
			)
			{
				VCHdl = &LinkHdl->VC_Info[VCIndex];

				if (VCHdl != NULL)
				{
					pFrameALVC =  (FRAMEAL_VC_T *) VCHdl->FrameALVCMod;

					if ((pFrameALVC != NULL) && pFrameALVC->ATMVCBlock.VCShaper.VCActive)
					{
						TotalRequestBandWidthCePS += pFrameALVC->ATMVCBlock.VCShaper.VCRequestSubstainedRateCePS;
					}
				}
			}

			/*
			*
			*	Find bandwidth value for filling the slot table.
			* 	
			*/

			pFrameALLink->ATMLinkBlock.LinkShaper.VCsActive  = 0;
			pFrameALLink->ATMLinkBlock.LinkShaper.VCSlotTableEnd =0;
			for
			(
			    VCIndex = 0;
			    VCIndex < MAX_VC_PER_LINK;
			    VCIndex++
			)
			{
				VCHdl = &LinkHdl->VC_Info[VCIndex];

				if (VCHdl != NULL)
				{
					pFrameALVC =  (FRAMEAL_VC_T *) VCHdl->FrameALVCMod;
					if ((pFrameALVC != NULL) && pFrameALVC->ATMVCBlock.VCShaper.VCActive)
					{
						if (pFrameALVC->Parameters.PeakCellRate != 0)
						{
							pFrameALLink->ATMLinkBlock.LinkShaper.VCSlotTableEnd  +=
							    pFrameALVC->Parameters.PeakCellRate;

							pFrameALVC->ATMVCBlock.VCShaper.VCSlotsUsed =
							    pFrameALVC->Parameters.PeakCellRate;
						}
						else
						{
							pFrameALLink->ATMLinkBlock.LinkShaper.VCSlotTableEnd  += MAX_TABLE_SLOTS;
							pFrameALVC->ATMVCBlock.VCShaper.VCSlotsUsed =  MAX_TABLE_SLOTS;
						}

						pFrameALLink->ATMLinkBlock.LinkShaper.VCsActive++;
						pFrameALVC->ATMVCBlock.VCShaper.SlotTableBandwidthTotal =0;

						pFrameALVC->ATMVCBlock.VCShaper.SlotTableBandwidth =
						    FRACTIZE( pFrameALVC->ATMVCBlock.VCShaper.VCRequestSubstainedRateCePS) /
						    TotalRequestBandWidthCePS;
					}
				}
			}

			//
			//
			//  Build Slot table by running the bandwidth alg. on the slot table
			//	Allocate slots to VCs based on their requested bandwdith
			//
			//
			for
			(
			    SlotIndex = 0;
			    SlotIndex < pFrameALLink->ATMLinkBlock.LinkShaper.VCSlotTableEnd;
			    SlotIndex++
			)
			{
				DWORD				VCMax;
				FRACTOID			VCMaxBandwidth;
				FRAMEAL_VC_T 	* 	pVCSlotFrameALVC=NULL;

				//
				// find a vc for current slot
				// keep incrementing until atleast on VC has reached the threshold
				//
				do
				{
					VCMax = MAX_VC_PER_LINK;
					VCMaxBandwidth  = 0;

					for
					(
					    VCIndex = 0;
					    VCIndex < MAX_VC_PER_LINK;
					    VCIndex++
					)
					{
						VCHdl = &LinkHdl->VC_Info[VCIndex];

						if (VCHdl != NULL)
						{
							pFrameALVC =  (FRAMEAL_VC_T *) VCHdl->FrameALVCMod;
							if ((pFrameALVC != NULL) && pFrameALVC->ATMVCBlock.VCShaper.VCActive)
							{
								pFrameALVC->ATMVCBlock.VCShaper.SlotTableBandwidthTotal +=
								    pFrameALVC->ATMVCBlock.VCShaper.SlotTableBandwidth;

								if ((  VCMaxBandwidth < pFrameALVC->ATMVCBlock.VCShaper.SlotTableBandwidthTotal) &&
								        ( pFrameALVC->ATMVCBlock.VCShaper.VCSlotsUsed > 0))
								{
									pVCSlotFrameALVC = pFrameALVC;
									VCMaxBandwidth = pFrameALVC->ATMVCBlock.VCShaper.SlotTableBandwidthTotal;
									VCMax = VCIndex;
								}
							}
						}
					}

					if (VCMaxBandwidth >= FRACTIZE(1))
					{
						pFrameALLink->ATMLinkBlock.LinkShaper.VCSlotTable[SlotIndex].VC = (WORD)VCMax;
						pVCSlotFrameALVC->ATMVCBlock.VCShaper.SlotTableBandwidthTotal -= FRACTIZE(1);
						pVCSlotFrameALVC->ATMVCBlock.VCShaper.VCSlotsUsed--;
						break;
					}
				} while (1);
			}

			//
			// Update next VC pointer in Slot table
			//
			for
			(
			    SlotIndex = 0;
			    SlotIndex < pFrameALLink->ATMLinkBlock.LinkShaper.VCSlotTableEnd;
			    SlotIndex++
			)
			{
				DWORD 	NextVCSlotIndex = SlotIndex;
				DWORD 	CurrentVC = pFrameALLink->ATMLinkBlock.LinkShaper.VCSlotTable[SlotIndex].VC;
				DWORD	NextVC;

				// find next active VC
				// get firt VC candidate
				NextVC = ((CurrentVC+1) < MAX_VC_PER_LINK) ? CurrentVC+1 : 0;

				// look through VC list for next active VC
				for
				(
				    VCIndex = 0;
				    VCIndex < MAX_VC_PER_LINK;
				    VCIndex++
				)
				{
					VCHdl = &LinkHdl->VC_Info[NextVC];

					if (VCHdl != NULL)
					{
						pFrameALVC =  (FRAMEAL_VC_T *) VCHdl->FrameALVCMod;
						if ((pFrameALVC != NULL) && pFrameALVC->ATMVCBlock.VCShaper.VCActive)
						{
							// found a next active VC
							break;
						}
					}

					// try next VC  in list
					NextVC = ((NextVC+1) < MAX_VC_PER_LINK) ? NextVC+1 : 0;
				}

				//
				// find the next Actice VC which is different from the current VC
				//
				do
				{
					NextVCSlotIndex = 	((NextVCSlotIndex +1) < pFrameALLink->ATMLinkBlock.LinkShaper.VCSlotTableEnd)?
					                   (NextVCSlotIndex + 1) : 0 ;

				} while ( (NextVC != pFrameALLink->ATMLinkBlock.LinkShaper.VCSlotTable[NextVCSlotIndex].VC) &&
				          ( NextVCSlotIndex != SlotIndex));

				// 	update Next VC pointer in the Slot list
				pFrameALLink->ATMLinkBlock.LinkShaper.VCSlotTable[SlotIndex].NextVC = (WORD)NextVCSlotIndex;
			}
		}
	}
}

