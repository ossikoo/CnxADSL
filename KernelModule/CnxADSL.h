/******************************************************************************
********************************************************************************
****	Copyright (c) 1997, 1998, 1999, 2000, 2001
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
**		????
**
**	FILE NAME:
**		CnxADSL.h
**
**	ABSTRACT:
**		
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux 2.4 ATM Driver/KernelModule/CnxADSL.h $
**	$Revision: 5 $
**	$Date: 2/28/01 11:10a $
*******************************************************************************
******************************************************************************/

#include <linux/kernel.h>
#include "Version.h"
#include <linux/atm.h>
#define NET_IRDA_H // prevent redifinition of ASSERT via included linux/atmdev.h which included net/sock.h which included net/irda/irda.h which defined ASSERT
#include <linux/atmdev.h>
#include <linux/pci.h>
#include <linux/skbuff.h>

#include "CardMgmt.h"

///////////////////////////////////
// Common defines
///////////////////////////////////
#define IN
#define OUT
#define INOUT
#define UCHAR unsigned char
#define USHORT unsigned short
#define UINT unsigned int
#define ULONG unsigned long
#define PVOID void*

///////////////////////////////////////
// head of the device structure linked list
///////////////////////////////////////
extern CDSL_ADAPTER_T* gpAdapterList;

/////////////////////////////////////////
// device identifiers
/////////////////////////////////////////

#define CNX_DEV_LABEL				"CnxAdsl"

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
typedef int OptionLength_t;
#else
typedef unsigned OptionLength_t;
#endif

//////////////////////////////////////////////
// Linux ATM interface function prototypes
//////////////////////////////////////////////
void
CmgmtDeviceClose(
    IN struct atm_dev*	pDev
);

int CmgmtOpen(IN struct atm_vcc* pVcc);

void
CmgmtClose(
    IN struct atm_vcc*	pVcc);

int
CmgmtProcIOCTL(
    IN struct atm_dev*	pDev,
    IN UINT				Command,
    INOUT PVOID			pArgument );


int
CmgmtGetSocketOption(
    IN struct atm_vcc*	pVcc,
    IN int				OptionLevel,
    IN int				OptionName,
    OUT PVOID			OptionValue,
    IN int				OptionLength );

int
CmgmtSetSocketOption(
    IN struct atm_vcc*	pVcc,
    IN int				OptionLevel,
    IN int				OptionName,
    IN PVOID			OptionValue,
    IN OptionLength_t	OptionLength);

int
CmgmtSendBuffer(
    IN struct atm_vcc*	pVcc,
    IN SK_BUFF_T*	pSkb );

int
CmgmtScatGatSendQuery(
    IN struct atm_vcc*	pVcc,
    IN ULONG			BufferStart,
    IN ULONG			BufferSize );

int
CmgmtSendOAMCell(
    IN struct atm_vcc*	pVcc,
    IN PVOID			pCell,
    IN int				flags);

void
CmgmtPhyPut(
    IN struct atm_dev*	pDev,
    IN UCHAR			value,
    IN ULONG			addr);

UCHAR
CmgmtPhyGet(
    IN struct atm_dev*	pDev,
    OUT ULONG			addr);

void
CmgmtFeedback(
    IN struct atm_vcc*	pVcc,
    IN SK_BUFF_T*	pSkb,
    IN ULONG			start,
    IN ULONG			dest,
    IN int				len);

int
CmgmtChangeQOS(
    IN struct atm_vcc*	pVcc,
    IN struct atm_qos*	pQOS,
    IN int				flags);

void
CmgmtFreeRxSkb(
    IN struct atm_vcc*	pVcc,
    IN SK_BUFF_T*	pSkb);

int
CmgmtProcessRead(
    IN struct atm_dev*	pDev,
    IN loff_t*			pPOS,
    IN char*			pPage);


int
CmgmtPhyStart(
    IN struct atm_dev*	pDev
);

int
CmgmtPhyIOCTL(
    IN struct atm_dev*	pDev,
    IN unsigned int		Command,
    INOUT PVOID			pArg);

void
CmgmtPhyInterrupt(
    IN struct atm_dev*	pDev
);

//////////////////////////////////////////////
// module  defines
//////////////////////////////////////////////
// keep the following enum and the method table
// in the same order
typedef enum
{
    MODULE_TYPE_ARMAL=0,
    MODULE_TYPE_CARDMGMT,
    MODULE_TYPE_CARDAL,
    MODULE_TYPE_CoNDIS,
    MODULE_TYPE_BUFMGMT,
    MODULE_TYPE_FRAMEAL,
    MODULE_TYPE_CHIPAL
} MODULE_IDENTS;

