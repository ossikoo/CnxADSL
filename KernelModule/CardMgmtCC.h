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
**		CardMgmtCc.h
**
**	ABSTRACT:
**		WAN Miniport Call Control functions.
**		This module contains the declarations for CardMgmtCc.c.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CardMgmtCC.h $
** $Revision: 1 $
** $Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/
#ifndef _CARDMGMTCC_H_		//	File Wrapper,
#define _CARDMGMTCC_H_		//	prevents multiple inclusions


///////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
///////////////////////////////////////////////////////////////////////////////

void CMCCIncomingCallIndHdlr(	IN CDSL_VC_T		* pThisVc );

void CMCCDisconnectHdlr(	IN CDSL_VC_T		* pThisVc );



#endif		//	#ifndef _CARDMGMTCC_H_
