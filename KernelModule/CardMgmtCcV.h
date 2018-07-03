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
**		CardMgmtCcV.h
**
**	ABSTRACT:
**		WAN Miniport Call Control functions.
**		This module contains the declarations for CardMgmtCc.c.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CardMgmtCcV.h $
** $Revision: 1 $
** $Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/
#ifndef _CARDMGMTCCV_H_		//	File Wrapper,
#define _CARDMGMTCCV_H_		//	prevents multiple inclusions


///////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
///////////////////////////////////////////////////////////////////////////////

BOOLEAN CMCCSimulateIncoming(	IN CDSL_VC_T		* pThisVc );

BOOLEAN CMCCSimulateRemoteDisc(	IN CDSL_VC_T		* pThisVc );

void CMCCConnectHdlr(	IN CDSL_VC_T		* pThisVc );

void CMCCIdleHdlr(	IN CDSL_VC_T		* pThisVc );

BOOLEAN CMCCMakeCall(	IN CDSL_VC_T		* pThisVc );

BOOLEAN CMCCDropCall(	IN CDSL_VC_T		* pThisVc );

BOOLEAN CMCCAnswer(	IN CDSL_VC_T		* pThisVc );

BOOLEAN CMCCGoToDialToneState(	IN CDSL_VC_T		* pThisVc );

BOOLEAN CMCCGoToDialingState(	IN CDSL_VC_T		* pThisVc );

BOOLEAN CMCCGoToConnectedState(	IN CDSL_VC_T		* pThisVc );

void CMCCGoToIdleState(	IN CDSL_VC_T		* pThisVc );



#endif		//	#ifndef _CARDMGMTCCV_H_
