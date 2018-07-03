/******************************************************************************
*******************************************************************************
****	Copyright (c) 1997, 1998, 1999
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
**		CardAL (Card Abstraction Layer)
**
**	FILE NAME:
**		CardALBdDp.H
**
**	ABSTRACT:
**		Card level Abstraction Layer for CDSL.  This file/module abstracts
**		(hides the details of) card specific (CDSL) functions and presents
**		(hopefully) a constant interface to the upper level modules/functions
**		(CardMgmt) no matter what the actual card is (CDSL, ADSL, ISDN, etc.).
**		This file provides the functions that process the BackDoor requests
**		from an application to interface to the CardAL module that relate to the
**		data pump code (Pairgain DMT).
**
**	DETAILS:
**		Functions are arranged in top-down order.
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CardALBdDp.h $
** $Revision: 1 $
** $Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/
#ifndef _CARDALBDDP_H_		//	File Wrapper,
#define _CARDALBDDP_H_		//	prevents multiple inclusions


MODULAR DWORD cardALBdDpGetAdslTransceiverStatus(
    IN CDSL_ADAPTER_T						* pThisAdapter,
    CARDAL_ADAPTER_T						* pCardALAdapter,
    I_O BD_CARDAL_ADSL_TRANSCEIVER_STATUS_T	* pBdAdslTransceiverStatus ) ;


#endif	// File Wrapper
