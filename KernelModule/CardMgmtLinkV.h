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
**		CardMgmtLinkV.h
**
**	ABSTRACT:
**		WAN Miniport link management routines.
**		All information about the state of the link is stored in the
**		Link structure.
**		This module contains the declarations for CardMgmtLink.c.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CardMgmtLinkV.h $
** $Revision: 1 $
** $Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/
#ifndef _CARDMGMTLINKV_H_		//	File Wrapper,
#define _CARDMGMTLINKV_H_		//	prevents multiple inclusions

#define RATE_64000					64000
#define RATE_56000					56000

//	Use this macro to get a pointer to the link structure.
#define GET_LINK_FROM_ADAPTER(ThisAdapter) \
	&(ThisAdapter->AdapterLink)

//	Use this macro to determine if a link structure pointer is really valid.
#define IS_VALID_LINK(ThisAdapter, ThisLink) \
		(ThisLink && ThisLink->ThisAdapter == ThisAdapter)

///////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
///////////////////////////////////////////////////////////////////////////////

NDIS_STATUS CMLInitAllLinks(	IN CDSL_ADAPTER_T		* pThisAdapter );

CDSL_LINK_T * CMLAllocateLink(
    IN CDSL_ADAPTER_T		* ThisAdapter
);

VOID CMLReleaseLink(	IN CDSL_LINK_T			* ThisLink );

NDIS_STATUS CMLCallAllModuleLinkInits(
    IN CDSL_ADAPTER_T		* ThisAdapter,
    IN CDSL_LINK_T			* ThisLink );

VOID CMLCallAllModuleLinkShutdowns(	IN CDSL_LINK_T		* ThisLink );

#endif		//	#ifndef _CARDMGMTLINKV_H_
