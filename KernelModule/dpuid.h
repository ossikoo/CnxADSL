/****************************************************************************
*  $Workfile: dpuid.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/dpuid.h 13    6/26/01 7:56p Lauds $
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
*  Original Written By: Chi Tse                                             *
*  Original Date: 7/29/96                                                   *
*                                                                           *
*****************************************************************************
   Description:
	This is display processing unit (dpu). It contains typedefs and
   defines as well as data structure for the dumb terminal screens.

****************************************************************************/
#ifndef DPUID_H
#define DPUID_H

/****************************************************************************
                                  DEFINE
****************************************************************************/
/* List of screens ID */
enum
{
   LOGON_MENU_ID,                      // memus from dpu.c
   MAIN_MENU_ID,
   SYSTEM_MENU_ID,                     // menus from dpusys.c
   SYS_INFO_MENU_ID,
   SYS_CONFIG_MENU_ID,
   SYS_MISCCFG_MENU_ID,
   SYS_MISCCFG_SYS_MENU_ID,
   SYS_MISCCFG_OTHER_MENU_ID,
   SYS_TEST_MENU_ID,
   ADSL_MENU_ID,                       // menus from dpuadsl.c
   ADSL_STATUS_MENU_ID,
   ADSL_24HR_HIST_MENU_ID,
   ADSL_7DAY_HIST_MENU_ID,
   ADSL_ALARM_HIST_MENU_ID,
   REMOTE_LOGON_MENU_ID,               // menus from dpumisc.c
   DIAGNOSTIC_MENU_ID,
   TRACE_CFG_MENU_ID,
   TRACE_MENU_ID,
   PEEK_POKE_MEM_MENU_ID,
   PEEK_POKE_NVRAM_MENU_ID,
   BERT_SETUP_MENU_ID,                 // menus from dputest.c
   PRODUCTION_TEST_MENU_ID,
   ADSL_XCVR_MENU_ID,                  // menus from dpudmt.c
   ADSL_FALCON_STATUS_MENU_ID,
   ADSL_FALCON_BIT_ALLOC_MENU_ID,
   ADSL_FALCON_SNR_MENU_ID,
   ADSL_CONFIG_MENU_ID,
   ADSL_XCVR_TEST_MENU_ID,
   ADSL_XCVR_TX_TEST_MENU_ID,
   ADSL_XCVR_RX_TEST_MENU_ID,
   ADSL_XCVR_AFE_TEST_MENU_ID,
   ADSL_FALCON_VIEW_BUF_MENU_ID,
   PEEK_POKE_MENU_ID,
   PEEK_POKE_REG_MENU_ID,

#ifdef GHS_ENABLE
#ifndef NO_GHS_DISPLAY
   GHS_FR_ID,
#ifdef RT
   GHS_FR_LINEPROBE_ID,
#endif
   GHS_MENU_PHI_ID,
   GHS_MENU_PRO_ID,
   GHS_G9921A_ID,
   GHS_G9922AB_ID,
   GHS_COMMON_ID,
   ADSL_POWER_ID,                       // ADSL Power Level display menu
#endif
#endif
#ifdef EOCAOC_DISPLAY
   ADSL_EOC_AOC_ID,
   ADSL_EOC_ID,
   ADSL_AOC_ID,
#endif                                  // EOCAOC_DISPLAY
#ifdef TOMCAT
   EBM_MENU_ID,                        // menus from dpuebm.c
   RCB_PEEK_POKE_MENU_ID,              // Following are RCB related menu
   RCB_ERROR_CNT_MENU_ID,
   RCB_WAN_PORT_CTRL_MENU_ID,
   RCB_SRDB_MENU_ID,
   RCB_LOOKUP_TBL_MENU_ID,
   RCB_FRAME_BUF_MENU_ID,
#endif

   NUM_OF_SCR,         // The following menu is not used.
#ifndef TOMCAT
   EBM_MENU_ID,                        // menus from dpuebm.c
#endif
   EBM_SNMP_CONFIG_MENU_ID,           // This is used for dpu.c
   EBM_ROUTING_CFG_MENU_ID
};

#endif


/***************************************************************************
 * $Log: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/dpuid.h $
 * 
 * 13    6/26/01 7:56p Lauds
 * multi-instance version integration
 * 
 * 12    11/30/00 2:44p Lewisrc
 * Added TRACE_CFG_MENU_ID
 * 
 * 11    11/27/00 4:55p Lewisrc
 * Added TRACE_MENU_ID
 * 
 * 10    2/02/00 8:42a Lewisrc
 * 2.02i Beta 3
 * Received new files from James Infusino on first day of HSV interop
 * Changes to these files were incorporated when Bob backed out his AGC
 * changes
 * 
 * 9     2/01/00 1:05p Lewisrc
 * 2.02i Beta (2) drop rxed from James Infusino to Rick Hill at interop on
 * first day prior to testing
 * 
 * 8     1/31/00 2:29p Lewisrc
 * 2.02i Beta - prior to HSV interop
 * 
 *    Rev 1.23   Jan 26 2000 11:47:22   yangl
 * -- Add STDEOC API
 * -- Support clear EOC
 * -- Add misc cfg menu
 *
 *    Rev 1.22   Dec 03 1999 11:12:26   yangl
 * -- Improve AOC channel
 * -- Add AOC menu
 * -- Add EOC menu for RT
 *
 *    Rev 1.21   Nov 24 1999 18:51:12   infusij
 * - EOC channel improve.
 *
 *    Rev 1.20   Sep 01 1999 17:15:20   yangl
 * Add G.lite power information menu
 *
 *    Rev 1.19   Aug 02 1999 17:32:12   InfusiJ
 * -ghs phy layer fixes
 * -fr startup mods
 * -fr dpu page added
 *
 *    Rev 1.18   May 13 1999 11:11:08   phang
 * Use common files but add compiler switch for TOMCAT routines.
 *
 *    Rev 1.17   Apr 27 1999 14:54:04   HEJ
 * Temporarily leave EBM menu id in this file
 * until dpu.c is split into two files in the future.
 *
 *    Rev 1.16   Apr 07 1999 18:22:34   laud
 * Get rid of SourceCom EBM code
 *
 *    Rev 1.15   Apr 01 1999 12:18:50   InfusiJ
 * g.hs update - co & rt code merged, still not operational
 *
 *    Rev 1.14   21 Nov 1997 17:13:26   HEJ
 * Write XCVR Cfg menu.
 *
 *    Rev 1.13   Mar 17 1997 15:39:46   HE
 * Add two more menu ID for Falcon tx test and rx test.
 *
 *    Rev 1.12   15 Mar 1997 14:29:04   TSE
 * remove two unused bridge test menus id.
 *
 *    Rev 1.11   15 Mar 1997 13:58:50   TSE
 * added production support test menu and remove two unused bridge test menus.
 *
 *    Rev 1.10   26 Feb 1997 20:41:26   DLAU
 * get rid of all message id's
 *
 *    Rev 1.9   26 Feb 1997 18:43:46   HE
 * check in by Dave
 *
 *    Rev 1.8   Feb 25 1997 18:41:10   HE
 * add new message ID
 *
 *    Rev 1.7   24 Feb 1997 07:43:00   TSE
 * Added bridge router menu ID.
 *
 *    Rev 1.6   21 Feb 1997 18:04:52   HE
 *
 * add new message ID
 *
 *    Rev 1.5   19 Feb 1997 14:26:38   HE
 * put new message id
 *
 *    Rev 1.4   10 Feb 1997 13:47:28   HE
 *
 *
 *    Rev 1.3   07 Feb 1997 09:48:20   TSE
 * group peek-poke memory id and peek-poke NVRAM id
 * with the dpumisc menus.
 *
 *    Rev 1.2   07 Feb 1997 08:58:06   HE
 *
 *    Rev 1.1   04 Feb 1997 10:22:52   HE
 *
 * add more menu ID
 *
 *    Rev 1.0   03 Feb 1997 20:58:26   DLAU
 * Initial revision.
****************************************************************************/
/***** end of file $Workfile: dpuid.h $ *****/
