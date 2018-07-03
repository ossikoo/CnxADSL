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
**		HEC (Header Error Control)
**
**	FILE NAME:
**		FrameALHEC.h
**
**	ABSTRACT:
**		This files contains interface definitions for HEC.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4_ATM_Driver/KernelModule/FrameALHec.h $
** $Revision: 2 $
** $Date: 5/22/01 10:38a $
*******************************************************************************
******************************************************************************/

#ifndef _FRAMEAL_HEC_H_
#define _FRAMEAL_HEC_H_


// function to create HEC tables
GLOBAL void  FrameALHECInit (void);

// generate the HEC for a four byte ATM header; insert the HEC in the fifth byte
GLOBAL void FrameALHecEncode
(
    BYTE *Header
);

// Return value is 	0 = good header
//					1 -
typedef enum _HEC_RX_RETURN_VALUES
{
    HEC_GOOD			= 0,
    HEC_ONE_BIT_ERROR,
    HEC_BAD
}  HEC_RX_RETURN_VALUES;


// Check the Hec of an incoming ATM cell
// If header only has one bit error this function will modify the header to correct it

GLOBAL WORD	FrameALHecDecode
(
    BYTE *Header
);




#endif //_FRAMEAL_HEC_H_
