/******************************************************************************
*******************************************************************************
****	Copyright (c) 2000
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
**		dmtdbg (Debug support for DMT core code)
**
**	FILE NAME:
**		dmtdbg.h
**
**	ABSTRACT:
**		.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/dmtdbg.h $
** $Revision: 6 $
** $Date: 6/26/01 7:51p $
*******************************************************************************
******************************************************************************/


/*****                      INCLUDE SECTION                           ******/
#include "project.h"
#include "xcvrinfo.h"


#ifndef DMT_DBG_H
#define DMT_DBG_H

typedef struct
{
	TIME_TYPE Time ;
	char Text [DBG_LINE_LEN] ;
} DMT_DBG_TRACE_TYPE ;

DPCCALL void PutDebugTraceLine ( PDATAPUMP_CONTEXT pDatapump, const char * Format, ... ) ;
DPCCALL DMT_DBG_TRACE_TYPE *GetDebugTraceLine ( PDATAPUMP_CONTEXT pDatapump, UINT16 Index ) ;
DPCCALL void InitDebugTrace ( PDATAPUMP_CONTEXT pDatapump ) ;

#if DMT_DBG_TRACE_RAM
	#define DMT_DBG_TRACE_PRINT( ArgList ) \
		PutDebugTraceLine ArgList ;
#else //DMT_DBG_TRACE_RAM
	#define DMT_DBG_TRACE_PRINT( ArgList )
#endif //DMT_DBG_TRACE_RAM

#endif	// DMT_DBG_H
