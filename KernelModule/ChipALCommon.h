/******************************************************************************
********************************************************************************
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
**		ChipAl
**
**	FILE NAME:
**		ChipalCommon.h
**
**	ABSTRACT:
**		Common ChipAl Data definitions.  These are not project 
**		dependent and are only created from common data types
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/ChipALCommon.h $
**	$Revision: 1 $
**	$Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/
#ifndef CHIPALCOMMON_H_					// File Wrapper
#define CHIPALCOMMON_H_

/*******************************************************************************
Data Stream (Link) common definitions.
******************************************************************************/

#if !defined(_CHIPALCDSL_H_)   && !defined(_CHIPALWDM_H_)

typedef DWORD HARDWARE_DEVICE_T;
typedef DWORD CHIPAL_EVENT_HANDLER_T;
typedef DWORD CHIPAL_EVENT_T;
typedef DWORD HW_EVENT_TYPE_T;

#endif									// _NDIS_


typedef SK_BUFF_T	* (* FN_DATA_HANDLER)
(
    IN VOID				* pUserLink,
    IN SK_BUFF_T		* BufferDescPtr
);

typedef VOID (* FN_PUT_TX_EMPTY)
(
    IN	VOID			* LinkHdl,		//Link Handle to data stream
    IN	SK_BUFF_T		* BufferDescPtr	//Pointer to a buffer descriptor
);


typedef VOID (* FN_RX_COMPLETE)
(
    IN	VOID			* LinkHdl		//Link Handle to data stream
);

typedef enum CAL_LINK_CONFIG_E
{
    LINK_CONFIG_START = 0,
    LINK_CONFIG_INTERNAL = LINK_CONFIG_START,
    LINK_CONFIG_EXTERNAL,
    LINK_CONFIG_MAX
} CAL_LINK_CONFIG_T;

typedef	struct CAL_LINK_CFG_S
{
    CAL_LINK_CONFIG_T 	Type;	// Type of Params structure supplied in this configuration

    union
    {
        //* I/O handlers and buffer requirements are project defined.
        struct
        {
            DWORD	Speed;		// Link Speed in BPS
            DWORD	Channel;	// 0 relative Channel Identifier (project specific)
            BYTE	Mode;		// Framed (1) or Non-Framed (0)
        } Local;

        //*
        //* I/O handlers and buffer requirements are defined dynamically.
        //*
        struct
        {
            DWORD				Channel; 	//  0 relative Channel Identifier (project specific)
            BYTE				Mode;		// Framed (1) or Non-Framed (0)
            FN_DATA_HANDLER		GetTxFull;	// User Function to Provide Full TX Buffers
            FN_PUT_TX_EMPTY		TxComplete;	// User function to indicate TX complete
            FN_DATA_HANDLER		PutRxFull;	// User function to Receive Full buffers
            FN_RX_COMPLETE		RxComplete ;// User function to indicate Rx Complete			WORD				RxSegmentSize; // Size in Bytes of buffer segments
            WORD				RxNumberOfSegments; // Number of segments
            WORD				RxSegmentSize; 		// Size in Bytes of buffer segments				WORD				TxNumberOfSegments; // Number of segments
            WORD				TxNumberOfSegments; // Number of segments
            WORD				TxSegmentSize; 		// Size in Bytes of buffer segments				WORD				TxNumberOfSegments; // Number of segments
        } Extern;

    } Params;

} CAL_LINK_CFG_T;

#endif 									// File Wrapper CHIPALCOMMON_H_

