/************************************************************************
*  $Workfile: project.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/project.h 44    6/26/01 8:21p Lauds $
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
*  Original Date: 6/29/98                                                   *
*                                                                           *
*****************************************************************************
   Description:   This header file contains all project specific parameters
                  for the system.
****************************************************************************/

#ifndef PROJECT_H
#define PROJECT_H

//  The following line is to be enabled for Euphrates project only !
//#define EUPHRATES

//#include <limits.h>
#include "c166.h"

#ifdef EUPHRATES
	typedef signed long int	LONG;	    /* 32-bit signed integer */
	#define KdPrint						// debug not supported in Euphrates
    #define PHYSICAL_COMM_PORT
#else
    #if defined (OS_LINUX)
        #include "LnxTools.h"
    #else
        #if		defined(OS_WIN_95)	// requires include=c:\program files\98ddk\inc\win98
            #include <ndis.h>
        #elif	defined(OS_NT_4)
            #include <ntddk.h>
        #else
            #include <wdm.h>
        #endif
    #endif
#endif

#define VCOMM			// Virtual Comm Port
#if ( (! defined ( RT ) ) && ( ! defined ( CO ) ) )
#define RT				// RT or CO should be defined in Sources - if not default to RT
#endif
#define CONEXANT_AFE 
#define CONEXANT_NIC

#ifdef RT   // this only applies to RT
#define CONEXANT_AGC
#endif
#define P46
//#define ECHO_TRAINING

#ifndef EUPHRATES
#define QEDFER_USES_WAIT
#endif
#define STARTUP_TRACE
#define EOCAOC_DISPLAY
#define FAST_PATH_FIX
//#define DATAPATH

#define AOC_RX_BUFFER 0			// Include support for loging of AOC rx (no output methods - use debugger to view)
#define DIAGNOS_WRITE_BUS 1		// Include support for writing diagnostic values to bus (e.g. PCI)
#define DIAGNOS_LOG_TRACE 1		// Include support for traceing of diagnostic log to debug port
#define DBG_PRINT_PORT_TRACE 1	// Include support for sending RAM Buffer to debug port
#define DMT_DBG_TRACE_RAM 1		// Include support for DMT_DBG_TRACE_PRINT to RAM buffer
#define AOC_TRACE 1				// Include support for tracing of AOC to RAM Buffer
#define EOC_TRACE 1				// Include support for tracing of EOC to RAM Buffer
#define AFE_WRITE_TRACE 1		// Include support for tracing of AFE Writes to RAM Buffer
#define DMT_FSM_TRACE 1			// Include support for tracing of DMT FSM to RAM Buffer
#define AUTOSENSE_TRACE 1		// Include support for tracing of Autosense to RAM Buffer

#define DMT_DBG_PRINT(arg) KdPrint(arg)

#define DBG_LINE_CNT 50			// Number of lines in debug RAM buffer
#define DBG_LINE_LEN 80			// Size of lines in debug RAM buffer

#ifndef EUPHRATES
#define DMT_ASSERTMSG(msg, expr) ASSERTMSG(msg,expr)
#else				// Euphrates not use debug messages
#define DMT_ASSERTMSG(msg, expr) //printf("%s  %d", msg, expr )
#define NDIS_STATUS int
#endif

#define FALCON_2
#define FALCON_2P	// To enable Falcon 2P support, the Falcon 2 compiler switch needs to be on also.
#define FALCON_TX_CLK_MODE

// enable G.994.1 code
// when compiling for C16x remember to reduce stack size (adsl.ilo)
#define GHS_ENABLE

#ifdef GHS_ENABLE
// if defined inhibits escape to fast retrain from G.HS
#define NO_ESC_TO_FR
// define this will set RDC and UC to Zero (Conexant require to aid debugging - non-standard mode of operation)
//#define NO_RT_FR_PRCUTBK
// if CO use micro-controller receiver
#ifdef CO
//#define GHS_CO_MICRO_RX
#endif
// if using _C16X processor we have limited near ROM memory for small data memory model
#ifdef _C166
// do not compile GHS display into code.
//#define NO_GHS_DISPLAY
#endif

// if has OnOffHook detector hardware present
#define OFFHOOK_DETECTOR


#endif// GHS_ENABLE

// the compiler switch to indicate this is a DMT system
#define DMT

// do not compile bridge display into code.
#define NO_BRIDGE_DISPLAY

//#define EOC_NAK_MODE

// macro to define where is the starting address of FALCON
#define FALCON_START_ADDR(pDatapump) ((pDatapump)->Falcon_Space)

// define if the processor is little or big endian orientation
#define LITTLE_ENDIAN_MEMORY


// The following type definitions are compiler dependent
typedef bit FLAG_TYPE;

typedef LONG LOCK_TYPE;


#ifndef UINT16
#define UINT16  unsigned short int
#endif

typedef signed short int  SINT16;

// this is to specify the duration of T6 in sec

#define TICKS_PER_SEC   10         //  Timer has 100 msec period


// Define number of xcvr channels:  Was origiannly in xcvrinfo.h, we moved
//  them here so that DSLAM can redefine them.  For ADSL number of adsl channel
//  is always 1

// The following specifies the DAC and ADC settings of the AFE
enum {TWOS_COMPLEMENT, OFFSET_BINARY};

#ifdef DATAPATH
#define ADC_MODE   OFFSET_BINARY
#define DAC_MODE   OFFSET_BINARY
#define AFE_NUM_OF_BIT  16
#else
#define ADC_MODE   TWOS_COMPLEMENT
#define DAC_MODE   TWOS_COMPLEMENT

#ifdef CONEXANT_AFE
#define AFE_NUM_OF_BIT  15
#else
#define AFE_NUM_OF_BIT  14
#endif
#endif

// Below Define if has ATM mode or not
// define only if cell based system
#define FALCON_ATM_MODE

#endif
/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/project.h $
 * 
 * 44    6/26/01 8:21p Lauds
 * multi-instance version integration
 * 
 * 2     6/22/01 3:12p Lauds
 * minor cleanup and bug fix in miscutil.c and terminal.c
 * 
 * 1     6/21/01 3:10p Lauds
 * Multi-instance initial version
 * 
 * 43    1/10/01 8:42a Lewisrc
 * include of LinuxTools.h needs <>
 * 
 * 42    1/09/01 10:07a Lewisrc
 * Wirespeed/Redhat had to make minor changes to DMT files to compile
 * under GNU
 * 
 * Change include of wdm.h to LnxTools.h
 * 
 * 41    12/06/00 11:22a Lauds
 * define CONEXANT_AGC only for RT and fix corresponding compiling errors.
 * update CO AFE startup sequence table
 * 
 * 40    12/05/00 2:12p Lewisrc
 * ONly define RT if it or CO not already defined in Sources
 * 
 * 2     12/05/00 2:07p Lewisrc
 * ONly define RT if it or CO not already defined in Sources
 * 
 * 1     12/05/00 1:50p Lewisrc
 * 
 * 39    11/30/00 2:41p Lewisrc
 * add/remove/rename TRACE defines
 * 
 * 38    11/27/00 6:15p Lauhk
 * Define NDIS_STATUS for Euphrates.
 * 
 * 37    11/27/00 4:55p Lewisrc
 * Added settings for debug RAM print buffer (e.g. DMT_DBG_TRACE_RAM)
 * 
 * 36    9/18/00 7:01p Lauhk
 * Added compiler switch to help porting to Euphrates.
 * 
 * 35    9/05/00 1:52p D1builder
 * PROBLEM:
 * Need to be able to build NDIS drivers for Windows95 NDIS 4.0.
 * 
 * SOLUTION:
 * Added conditional compile statements to remove and/or change defines,
 * code, etc. not available in NDIS 4.0 for Window95.
 * 
 * 34    8/04/00 5:36p Lewisrc
 * Change LOCK_TYPE from bit to LONG so we can use
 * InterlockedCompareExchange
 * 
 * 33    8/01/00 6:23a Boikedc
 * Added OS_NT_4 flag to include either ntddk.h or wdm.h 
 * 
 * 32    7/21/00 10:03a Lewisrc
 * Add DMT_DBG_PRINT & DMT_ASSERTMSG
 * 
 * 31    7/14/00 12:11p Lewisrc
 * Add flag for P46 for PCI Burst Workaround
 * Add QEDFER_USES_WAIT
 * 
 * 30    3/20/00 11:00a Lewisrc
 * #define  NO_BRIDGE_DISPLAY such that no more bridge menu is displayed. 
 * This avoid crashing the system when hitting the wrong key
 * 
 * 29    2/11/00 10:37a Lewisrc
 * Change DIAGNOS_TRACE to DIAGNOS_LOG_TRACE
 * Add DIAGNOS_WRITE_TRACE
 * 
 * 28    2/11/00 9:45a Lewisrc
 * Enable 
 * 	AOC_TRACE
 * 	EOC_TRACE
 * 	DIAGNOS_TRACE
 * 	AFE_WRITE_TRACE
 * 
 * 27    2/10/00 1:37p Lewisrc
 * Added:
 * 	AFE_WRITE_TRACE
 * 	DIAGNOS_TRACE
 * 
 * 26    2/09/00 3:08p Bradforw
 * Addition of AGC and Software SwitchHook
 * 
 * 25    12/11/99 12:29p Lewisrc
 * Update from Sean for Fast Path problems with Alcatel, etc
 * 
 * 24    12/07/99 10:22a Lewisrc
 * added  EOCAOC_DISPLAY
 * 
 * 23    11/09/99 9:30a Lewisrc
 * 2.02f code release from Pairgain
 * 
 * 22    10/15/99 2:45p Lewisrc
 * 2.02D 10/15/99 Release
 * 
 * 21    10/13/99 2:52p Lewisrc
 * Added EOC_NAK_MODE
 * 
 * 20    10/11/99 3:45p Lewisrc
 * Enable G.HS
 * 
 *    Rev 1.31   Sep 28 1999 12:42:42   yangl
 * Add NO_RT_FR_PRCUTBK compiler switch
 * to disable power cutback in G.lite.
 *
 *    Rev 1.30   Sep 02 1999 16:05:18   InfusiJ
 * -removed unused fast retrain compile define
 *
 *    Rev 1.29   Aug 27 1999 15:56:36   InfusiJ
 * - T.413 and G.HS now runtime selectable
 * - ITeX interop results merged
 *
 *
 *    Rev 1.28   Aug 05 1999 15:11:38   yangl
 * Add OFFHOOK_DETECTOR compiler switch
 * for hardware w/o detector.
 *
 *    Rev 1.27   Aug 04 1999 15:57:52   nordees
 * Falcon 2P compiler switch added.
 *
 *    Rev 1.26   Jul 29 1999 15:53:42   InfusiJ
 * Initial Fast Retrain merge.
 * FR is enabled ONLY with G.hs
 * FR is in a Debug state
 * Escape to FR is not functional
 *
 *    Rev 1.25   Jun 23 1999 17:48:28   laud
 * move FIR_OVERSAMPLING_ENABLE to xcvrinfo.h
 *
 *    Rev 1.24   Jun 22 1999 13:55:46   nordees
 * Enable oversampling for phase selection (inc. new rx filter) on RT.
 *
 *    Rev 1.23   May 20 1999 20:57:14   laud
 * move XCVR_SW_VERSION to xcvrinfo.h
 *
 *    Rev 1.22   May 19 1999 16:46:10   laud
 * get rid of global G_LITE_VERSION compiler switch
 *
 *    Rev 1.21   Apr 23 1999 17:25:04   nordees
 * Conexant AFE support.
 *
 *    Rev 1.20   Apr 19 1999 17:54:56   laud
 * version 2.01b
 *
 *    Rev 1.19   Apr 14 1999 16:23:38   InfusiJ
 *
 *
 *    Rev 1.18   Apr 14 1999 16:21:26   InfusiJ
 * g.hs update. 3com interop
 *
 *    Rev 1.17   Apr 01 1999 12:18:50   InfusiJ
 * g.hs update - co & rt code merged, still not operational
 *
 *    Rev 1.16   Mar 22 1999 15:56:28   nordees
 * Support for G.HS
 *
 *    Rev 1.15   Mar 18 1999 18:50:48   laud
 *
 *
 *    Rev 1.14   Jan 19 1999 16:10:32   laud
 * release for 2.01
 *
 *    Rev 1.13   Jan 07 1999 15:08:22   laud
 * FALCON_2 fixes
 *
 *    Rev 1.12   05 Jan 1999 14:09:06   tsaurl
 * Lih-feng: Add FAST_RETRAIN,
 * and TEST_FAST_RETRAIN  compiler switch
 * default is undefined
 *
 *    Rev 1.11   Dec 21 1998 13:56:26   laud
 * DATAPATH stuff
 *
 *    Rev 1.10   Dec 08 1998 17:56:06   laud
 * AFE modification for DATAPATH and usable for DSLAM and TIGER
 *
 *    Rev 1.9   Nov 11 1998 18:00:50   laud
 * FALCON_ATM_MODE disabled for non cell base system
 *
 *    Rev 1.8   Oct 21 1998 15:21:22   laud
 * FALCON_ATM_MODE define
 *
 *    Rev 1.7   Oct 08 1998 19:18:56   laud
 * change default back to 1A for release purpose
 *
 *    Rev 1.6   Oct 02 1998 11:51:24   laud
 * default to be 1Z
 *
 *    Rev 1.5   Sep 30 1998 17:49:36   laud
 * eoc support for multiple xcvr and sw ver API
 *
 *    Rev 1.4   01 Sep 1998 15:26:32   tsaurl
 * add flag for 1z default value
 *
 *    Rev 1.3   Aug 10 1998 15:27:40   laud
 * support DataPath AFE
 *
 *    Rev 1.2   Jul 24 1998 15:00:16   laud
 * changes for UAWG
 *
 *    Rev 1.1   Jun 25 1998 23:51:36   laud
 *
 *
 *    Rev 1.0   Jun 25 1998 22:40:08   laud
 * use of project.h
*
*****************************************************************************/
/**********                end of file $Workfile: project.h $          *****************/
