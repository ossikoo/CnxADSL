/************************************************************************
*  $Workfile: microxcv.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/microxcv.h 2     6/26/01 8:15p Lauds $
*
*****************************************************************************
*  Copyright 1996 PairGain Technologies as unpublished work                 *
*  All Rights Reserved                                                      *
*                                                                           *
*  The information contained herein is the confidential property            *
*  of PairGain Technologies.  The use, copying, transfer or                 *
*  disclosure of such information is prohibited except by express           *
*  written agreement with PairGain Technologies.                            *
*                                                                           *
*                                                                           *
*  Original Written By:  David S. Lau                                       *
*  Original Date: 1/27/97                                                   *
*                                                                           *
*****************************************************************************
    Description:  This module should contain all transceiver dependent hardware
     definition.
****************************************************************************/
#ifndef MICROXCV_H
#define MICROXCV_H

#define MIC_DISTXEOC(x)     ASIC_DISABLE_INTR(x, TEOC_INT)

#define MIC_ENATXEOC(x)     ASIC_ENABLE_INTR(x, TEOC_INT)

#define MIC_DISTXAOC(x)     ASIC_DISABLE_INTR(x, TAOC_INT)

#define MIC_ENATXAOC(x)     ASIC_ENABLE_INTR(x, TAOC_INT)
/*****                     Global functions                           ******/
DPCCALL void MIC2_EnableIRQ1_2(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MIC2_DisableIRQ1_2(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MIC_SetRstXcvr(PDATAPUMP_CONTEXT pDatapump, BOOLEAN);
DPCCALL void MIC_SetPwrDwnXcvr(PDATAPUMP_CONTEXT pDatapump, BOOLEAN state);

#endif

/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/microxcv.h $
 * 
 * 2     6/26/01 8:15p Lauds
 * multi-instance version integration
 * 
 * 2     6/22/01 3:11p Lauds
 * minor cleanup and bug fix in miscutil.c and terminal.c
 * 
 * 1     6/21/01 3:09p Lauds
 * Multi-instance initial version
 * 
 * 1     2/09/99 4:32p Lewisrc
 * Moved the following macors from micro.h to newly created microxcv.h to
 * more closely match pairgain.
 * 	MIC_DISTXEOC
 * 	MIC_ENATXEOC
 * 	MIC_DISTXAOC
 * 	MIC_ENATXAOC
 * 
 *    Rev 1.12   Aug 07 1998 17:08:44   laud
 * FALCON 1Z support
 *
 *    Rev 1.11   21 May 1998 19:58:44   laud
 * add xcvr_num parameter to MIC routines and macros
 *
 *    Rev 1.10   18 May 1998 19:29:32   laud
 * DSLAM integration
 *
 *    Rev 1.9   31 Oct 1997 18:12:30   laud
 * DSLAM
 *
 *    Rev 1.8   Aug 19 1997 14:54:54   TPHAM
 * lots of clean up + more NMA cmd + enable tx/rx interrupts
 * for EOC/AOC/RSYN/TSYNC + Added logic to initiate
 * AOC transfer for testing
 *
 *    Rev 1.7   02 Apr 1997 17:24:28   DLAU
 *
 *    Rev 1.6   10 Mar 1997 12:22:06   DLAU
 *
 * cleanup
 *
 *    Rev 1.5   05 Mar 1997 20:12:18   DLAU
 * change FALCON start address
 *
 *    Rev 1.4   11 Feb 1997 15:04:52   DLAU
 *
 * move FALCON start address for testing purpose
 *
 *    Rev 1.3   10 Feb 1997 13:47:16   DLAU
 *
 *    Rev 1.2   29 Jan 1997 18:29:22   DLAU
 *
 *    Rev 1.1   29 Jan 1997 14:39:06   DLAU
 *
 *    Rev 1.0   27 Jan 1997 14:47:16   DLAU
 * Initial revision.
*
*****************************************************************************/
/**********                end of file $Workfile: microxcv.h $          *****************/


