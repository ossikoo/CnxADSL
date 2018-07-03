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
**		Version.h
**
**	ABSTRACT:
**		This module contains the Revision level for the Tigris project.
**
**	DETAILS:
**		All modules that report a Revision or Version should use the Revision
**		defined in this file.
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.4.3_ATM_Driver/KernelModule/Version.h $
**	$Revision: 13 $
**	$Date: 9/10/01 11:52a $
*******************************************************************************
******************************************************************************/

#ifndef _VERSION_H_
#define _VERSION_H_
#include <linux/version.h>
#include <linux/sched.h>

//	The product revision level is in the following format:
//	p.h.MM.mmm (wwyy)  OR
//	p.h.MM.mmm+ (wwyy)
//
//	Defined as:
//	PRODUCT.HARDWARE.MAJOR_REVISION.MINOR_REVISION (DATE_CODE)
//
//  NOTE:  All numbers are in decimal format.

//  See VersionDef.h for details on constructing the REVISION_LEVEL string.

// --PIM--
#define REVISION_LEVEL		"6.1.2.007-PIM-2.6-2.8 (26122012)"
// --/PIM--

#ifndef INLINE
#define INLINE
#endif

// --/PIM--

#endif

