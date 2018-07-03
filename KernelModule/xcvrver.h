/****************************************************************************
*  $Workfile: xcvrver.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/xcvrver.h 35    8/31/01 2:47p Lauds $
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
*  Original Written By:  David Lau                                          *
*  Original Date:  5/25/99                                                  *
*                                                                           *
*****************************************************************************
    Description:

        This is transceiver information(sysinfo) data. It contains typedefs and
        defines as well as data structure for all statistical, errored,
        alarm information. Each data structure shall have its own
        define to define the structure size using sizeof().
****************************************************************************/
#ifndef XCVRVER_H
#define XCVRVER_H


/****************************************************************************
                           DEFINE
****************************************************************************/

                                             
#define    UNLIMIT_DOWNSTREAM
#define    XCVR_SW_VERSION_NUM    "3.00 beta"
#define    XCVR_SW_VERSION     ANNEX_MODE == ANNEX_A ? XCVR_SW_VERSION_NUM" A":XCVR_SW_VERSION_NUM" B"


#endif       // End if for 'ifndef XCVRVER_H'
/***************************************************************************
 * $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/xcvrver.h $
 * 
 * 35    8/31/01 2:47p Lauds
 * first 3.00 beta version
 * 
 * 34    7/10/01 1:08p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 * 
 * 33    6/26/01 8:51p Lauds
 * multi-instance version integration
 * 
 * 32    6/06/01 3:01p Lauds
 * Annex Mode suffix in xcvr version
 * 
 * 31    6/04/01 2:54p Lauds
 * bump up to 2.08 and add beta
 * 
 * 30    5/31/01 6:13p Lauds
 * take out "beta"
 * 
 * 29    5/29/01 11:28a Lauds
 * bump up to 2.07 and add beta
 * 
 * 28    5/24/01 6:26p Lauds
 *  take out beta for 099 release build
 * 
 * 27    5/18/01 11:22a Lauds
 * bump up to 2.06 and add "beta"
 * 
 * 26    5/17/01 11:47p Lauds
 * take out beta for 099 build
 * 
 * 25    5/11/01 10:52a Lauds
 * bump up to 2.05 and add "beta"
 * 
 * 24    5/02/01 2:30p Lauds
 * take out beta for 2.04
 * 
 * 23    4/19/01 11:06a Lauds
 * add "beta" in sw version
 * 
 * 22    11/30/00 4:54p Lauds
 * shorten XCVR_SW_VERSION and change 2.03b beta to 2.03c avoid confusion
 * 
 * 21    8/11/00 5:10p Lauds
 * Trellis coding enable based on chip id
 * 
 * 20    7/19/00 9:35p Lauds
 * 
 * 19    5/10/00 4:14p Lauds
 * estimate SNR before deciding if Trellis should be turn on to achieve 8+
 * Mbps with some vendors
 * 
 * 18    3/20/00 11:30a Lewisrc
 * 2.03a
 * 
 *    Rev 1.14   06 Mar 2000 10:46:46   nordees
 * version 2.03a
 *
 *    Rev 1.13   Feb 25 2000 15:43:38   nordees
 * Version 2.03
 *
 *    Rev 1.12   Dec 07 1999 11:21:22   nordees
 * version 2.02i beta.
 *
 *    Rev 1.11   Dec 07 1999 11:20:46   nordees
 * version 2.02h
 *
 *    Rev 1.11   Dec 03 1999 18:16:54   nordees
 * version 2.02h beta
 *
 *    Rev 1.10   Dec 03 1999 18:16:24   nordees
 * version 2.02g
 *
 *    Rev 1.9   Nov 04 1999 18:58:24   nordees
 * version 2.02f
 *
 *    Rev 1.8   Oct 18 1999 14:03:16   nordees
 * version 2.02e
 *
 *    Rev 1.7   Oct 14 1999 16:40:00   nordees
 * version 2.02d.
 *
 *    Rev 1.6   Sep 17 1999 10:04:14   nordees
 * version 2.02c.
 *
 *    Rev 1.5   Sep 01 1999 10:50:38   nordees
 * version 2.02b.
 *
 *    Rev 1.4   Aug 04 1999 15:31:30   nordees
 * version 2.02a.
 *
 *    Rev 1.3   Jul 29 1999 17:36:10   nordees
 * version 2.01g
 *
 *    Rev 1.2   Jun 22 1999 15:08:48   nordees
 * version 2.01f
 *
 *    Rev 1.1   Jun 02 1999 15:11:50   laud
 * version 2.01e
 *
 *    Rev 1.0   May 25 1999 12:06:30   laud
 * put xcvr version in xcvrver.h
****************************************************************************/
/**********                end of file $Workfile: xcvrver.h $          *****************/
