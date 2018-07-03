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
**		CardMgmtVcV.h
**
**	ABSTRACT:
**		WAN Miniport VC management routines.
**		All information about the state of the VC is stored in the
**		VC structure.
**		This module contains the declarations for CardMgmtVC.c.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CardMgmtVcV.h $
** $Revision: 1 $
** $Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/
#ifndef _CARDMGMTVCV_H_		//	File Wrapper,
#define _CARDMGMTVCV_H_		//	prevents multiple inclusions


//	This value indicates how many point to point connections are allowed per WAN link VC.
#define RATE_64000					64000
#define RATE_56000					56000


//	Use this macro to determine if a VC structure pointer is really valid.
#define IS_VALID_VC(ThisAdapter, ThisVc) \
		(ThisVc && ThisVc->LinkHdl && (ThisVc->LinkHdl->ThisAdapter == ThisAdapter))

//	Use this macro to get a pointer to a VC structure from a VcIndex.
#define GET_VC_FROM_VCINDEX(pThisLink, VcIndex) \
		&(pThisLink->VC_Info[VcIndex])

//  Use this inline to get an unused VC
static __inline CDSL_VC_T* GetUnusedVC ( CDSL_LINK_T* pLink )
{
	int cnt;
	CDSL_VC_T* pRetVC=NULL;

	for ( cnt=0; cnt<MAX_VC_PER_LINK; cnt++)
	{
		if ( !pLink->VC_Info[cnt].Allocated )
		{
			pRetVC = &pLink->VC_Info[cnt];
			break;
		}
	}

	return pRetVC;
}

///////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
///////////////////////////////////////////////////////////////////////////////

NDIS_STATUS CMVCInitAllVcs(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN CDSL_LINK_T			* pThisLink );

VOID CMVCShutdownAllVcs(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN CDSL_LINK_T			* pThisLink );

CDSL_VC_T * CMVCAllocateVc(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN CDSL_LINK_T			* pThisLink,
    IN short				vpi,
    IN int					vci,
    IN struct atm_vcc*		pVcc );

VOID CMVCReleaseVc(	IN CDSL_VC_T			* ThisVc );

#endif		//	#ifndef _CARDMGMTVCV_H_
