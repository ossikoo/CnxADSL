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
**		CardMgmtVc.c
**
**	ABSTRACT:
**		WAN Miniport VC management routines.
**		All information about the state of the VC is stored in the
**		VC structure.
**
**	DETAILS:
**		Functions are arranged in top-down order.
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CardMgmtVc.c $
** $Revision: 1 $
** $Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/
#define		MODULE_ID			DBG_ENABLE_CARDMGMT


#include "CardMgmt.h"
#include "CardMgmtV.h"

#define		__FILEID__		5



///////////////////////////////////////////////////////////////////////////////
//	Defines
///////////////////////////////////////////////////////////////////////////////

#ifndef MEMORY_TAG
	#define MEMORY_TAG			'CVMC'
#endif


/******************************************************************************
*	FUNCTION NAME:
*		CMVCInitAllVcs
*
*	ABSTRACT:
*		This routine initializes the driver level VC structures within
*		the Adapter structure.  We use the VC structure to hold context
*		pointers for each layer in the driver. The layers use the VC
*		structure to idenity a VC and find the context for that VC. 
******************************************************************************/
NDIS_STATUS CMVCInitAllVcs(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN CDSL_LINK_T			* pThisLink )
{
	//	A pointer to our VC information structure for the selected line device.
	CDSL_VC_T					* pThisVc;
	NDIS_STATUS					Status=STATUS_SUCCESS;

	//	Index into the VC array.
	USHORT						VcIndex;

	//	Go through and initialize each VC.
	for ( VcIndex = 0;   VcIndex < MAX_VC_PER_LINK;   VcIndex++)
	{
		pThisVc = GET_VC_FROM_VCINDEX( pThisLink, VcIndex );

		//	All VC structure fields must have been set to 0
		//	when the memory for the structure was allocated

		//	Setup the static features of the VC.
		pThisVc->LinkHdl = pThisLink;
		pThisVc->VcDeviceId = VcIndex;
		pThisVc->VcAddressId = VcIndex;
		pThisVc->Allocated = FALSE;
		pThisVc->Connected = FALSE;
	}
	return Status;
}



/******************************************************************************
FUNCTION NAME:
	CMVCShutdownAllVcs

ABSTRACT:
	Allocate all memory allocated by CMVCInitAllVcs


RETURN:
	VOID


DETAILS:
******************************************************************************/
VOID CMVCShutdownAllVcs(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN CDSL_LINK_T			* pThisLink )
{
	DWORD					VcIndex;
	CDSL_VC_T				* pThisVc;

	for ( VcIndex = 0;   VcIndex < MAX_VC_PER_LINK;   VcIndex++)
	{
		pThisVc = GET_VC_FROM_VCINDEX( pThisLink, VcIndex );
		if(pThisVc == NULL)
		{
			continue;
		}
	}
}


/******************************************************************************
*	FUNCTION NAME:
*		 CMVCAllocateVc
*
*	ABSTRACT:
*		This routine allocates a VC structure and passes back a
*		pointer which can be used by the driver to access the VC. 
*
*	RETURN:
*		A pointer to allocated VC information structure, 
*		NULL if not allocated.
*
*	ARGUMENTS:
*		pThisAdapter _ A pointer ot our adapter information structure.
*		pThisLink    - identifies the link
*		vci,vpi      - connection identifiers
*		pVcc         - Linux protocol link identifier
*
******************************************************************************/

CDSL_VC_T * CMVCAllocateVc(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN CDSL_LINK_T			* pThisLink,
    IN short				vpi,
    IN int					vci,
    IN struct atm_vcc*		pVcc
)
{
	//	A pointer to our VC information structure for the selected line device.
	CDSL_VC_T				* pThisVc;
	int						Status;

	pThisVc = GetUnusedVC( pThisLink );
	if ( !pThisVc )
	{
		ErrPrt("<1> CnxADSL no more VCs available\n");
		return NULL;
	}

	else
	{
		FRAMEAL_VC_PARAMETERS		DefaultVCParameters;

		// initialize this VCs information
		pThisVc->Allocated	= TRUE;
		pThisVc->Connected	= TRUE;
		pThisVc->Vpi		= vpi;
		pThisVc->Vci		= vci;
		pThisVc->pVcc		= pVcc;
		pThisVc->Pcr		= pVcc->qos.txtp.max_pcr;
		
		if ( pThisAdapter->PersistData.PeakCellRate != 0 )
		{
			pThisVc->Pcr = pThisAdapter->PersistData.PeakCellRate ;
		}

		Status = BufMgmtVcInit( pThisVc );
		if ( Status == STATUS_SUCCESS )
		{
			DefaultVCParameters.Vci = pThisVc->Vci;
			DefaultVCParameters.Vpi = pThisVc->Vpi;
			DefaultVCParameters.PeakCellRate = pThisVc->Pcr;
			DefaultVCParameters.AALType = ATM_AAL5;
			DefaultVCParameters.VcSAP = VC_SAP_AAL;
			DefaultVCParameters.VCMode = VC_MODE_NORMAL; //VC_MODE_INSERT_IDLE;

			Status = FrameALVCInit( pThisVc, &DefaultVCParameters );

			if ( Status != STATUS_SUCCESS )
			{
				Status = BufMgmtVcShutdown( pThisVc );
			}
		}
	}

	// if anything failed then mark the VC as not active
	if ( Status != STATUS_SUCCESS )
	{
		pThisVc->Allocated	= FALSE;
		pThisVc->Connected	= FALSE;
		pThisVc = NULL;
	}

	return (pThisVc);
}

/******************************************************************************
*	FUNCTION NAME:
*		CMVCReleaseVc
*
*	ABSTRACT:
*		This routine releases a specific VC structure and makes it available
*		for future allocation.  
*
*
*	RETURN:
*		None.
*
*	ARGUMENTS:
*		Vc _ A pointer to the VC information structure to be released.
*
******************************************************************************/

VOID CMVCReleaseVc(	IN CDSL_VC_T		* pThisVc )
{
	FrameALVCShutdown( pThisVc );
	BufMgmtVcShutdown( pThisVc );
	pThisVc->Allocated			= FALSE;
	pThisVc->Connected	= FALSE;
}
