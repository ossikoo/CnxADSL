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
**		CardMgmtV.h
**
**	ABSTRACT:
**		This files contains private defines for Card Management Module.
**      The private context structure is defined in this file.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4_ATM_Driver/KernelModule/CardMgmtV.h $
** $Revision: 2 $
** $Date: 5/22/01 10:37a $
*******************************************************************************
******************************************************************************/
#ifndef _CARDMGMTV_H_
#define _CARDMGMTV_H_


//
//	Include files necessary for Athena Driver Card Management module source files.
//
#include "CardMgmt.h"			//	Card Management -- module public declarations
#include "CardMgmtCcV.h"		//	Card Management -Call Control- public declarations within CardMgmt module
#include "CardMgmtLinkV.h"		//	Card Management -Link Management- public declarations within CardMgmt module
#include "CardMgmtVcV.h"		//	Card Management -VC Management- public declarations within CardMgmt module



typedef struct LINK_STATISTICS_S
{
	DWORD	NumRxFrameAborts;		//FE										ABT
	DWORD	NumRxOverRun;			//OE										BUFF
	DWORD	NumBadCRC;				//PE										FCS
	DWORD	NumSendReq;				//SendHandler
	DWORD	NumSendCompleteInd;		//sendcomplete handler
	DWORD	NumReceiveCompleteInd;	//ReceiveComplete Indications
	DWORD	NumReceiveComplete;		//ReceiveComplete , seen EOM
	DWORD	NumRxOverSized;			//frame too large to fit in our buffer		LNG
	DWORD	NumChangeOfFrameAlign;	//											COFA
	DWORD	NumOutOfFrame;			//											OOF
	DWORD	NumAlignError;			//											ALIGN
	DWORD	NumReceiveGood;			//OK from Line
	DWORD	NumVerifySuccess;
	DWORD	NumVerifyFail;
	DWORD	NumUnknownRxError;
	DWORD	NumBytesIn;
	DWORD   NumBytesOut;
} LINK_STATISTICS_T;




typedef	struct CDSL_LINK_CARD_MGMT_S
{

    // The speed provided by the link in bits per second.  This value is
    // passed up by the Miniport during the LINE_UP indication.
    DWORD					LinkSpeed;

#define 	LINK_STATE_IDLE 		0
#define 	LINK_STATE_CONNECTED 	1
    DWORD					State;

    //
    //used for statistics
    //
    LINK_STATISTICS_T		Stats;

} CDSL_LINK_CARD_MGMT_T;



typedef struct VC_STATISTICS_S
{
	DWORD	NumRxFrameAborts;		//FE										ABT
	DWORD	NumRxOverRun;			//OE										BUFF
	DWORD	NumBadCRC;				//PE										FCS
	DWORD	NumSendReq;				//SendHandler
	DWORD	NumSendCompleteInd;		//sendcomplete handler
	DWORD	NumReceiveCompleteInd;	//ReceiveComplete Indications
	DWORD	NumReceiveComplete;		//ReceiveComplete , seen EOM
	DWORD	NumRxOverSized;			//frame too large to fit in our buffer		LNG
	DWORD	NumChangeOfFrameAlign;	//											COFA
	DWORD	NumOutOfFrame;			//											OOF
	DWORD	NumAlignError;			//											ALIGN
	DWORD	NumReceiveGood;			//OK from Line
	DWORD	NumVerifySuccess;
	DWORD	NumVerifyFail;
	DWORD	NumUnknownRxError;
	DWORD	NumBytesIn;
	DWORD	NumBytesOut;
} VC_STATISTICS_T;


typedef enum CM_CFG_PARM_NAMES_E
{
    CM_CFG_START = 0,
    CM_CFG_ADDRESSLIST = CM_CFG_START,
    CM_CFG_APIHWIFMODE,
    CM_CFG_LINEAUTOACTIVATION,
    CM_CFG_VCLIST,
    CM_CFG_VENDORID,
    CM_CFG_VENDORDESCR,
    CM_CFG_WANADDRESS,
    CM_CFG_MAX
} CM_CFG_PARM_NAMES_T;

#endif //_CARDMGMTV_H_
