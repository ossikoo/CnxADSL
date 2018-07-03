/************************************************************************
*  $Workfile: seqblk.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/seqblk.h 9     8/16/01 5:59p Lauds $
*
*************************************************************************
*       Copyright 1996 PairGain Technologies as unpublished work        *
*       All Rights Reserved                                             *
*                                                                       *
*       The information contained herein is the confidential property   *
*       of PairGain Technologies.  The use, copying, transfer or        *
*       disclosure of such information is prohibited except by express  *
*       written agreement with PairGain Technologies.                   *
*                                                                       *
*************************************************************************
   Description: This header file includes all the register definitions in
      the DSP Sequencer block of the FALCON.
*************************************************************************/

#ifndef SEQBLK_H
#define SEQBLK_H

#include "types.h"
#include "sysinfo.h"

/**************************************************************************
     Sequencer Download Register    (Read/Write)
**************************************************************************/

typedef volatile UINT16 SEQWR_TYPE;


/**************************************************************************
     Sequencer RAM Address Latch    (Write Only)
**************************************************************************/

typedef volatile UINT16 SADDR_TYPE;


/**************************************************************************
     Sequencer Data and Control Latch (Write Only)
**************************************************************************/

enum DLD_CHOICE {DLD_DISABLED, DLD_ENABLED};

typedef volatile union
{
    UINT16 DLD_MODE           :1;
    UINT16 TEQ_CNT            :10;  // Trapper only 
    UINT16                    :1;
    UINT16 DIS_ECCLK          :1;   // Enable/Disable MCLK for EC block (falcon 2P)
    UINT16 DIS_FFTBCLK        :1;   // Enable/Disable MCLK for FFTB block (falcon 2P)
    UINT16 DIS_VDECCLK        :1;   // Enable/Disable MCLK for VDEC block (falcon 2P)
    UINT16                    :1;
}SEQCTL_TYPE;

#define SEQCTL_DEFAULT   {DLD_DISABLED} // clear every fields to zero


/**************************************************************************
     Sequencer FFTB Control Register (Read/Write)
**************************************************************************/

typedef volatile struct
{
    UINT16 SUP_OVERRIDE       :1;
    UINT16 WIN_FFT            :1;
    UINT16 WIN_IFFT           :1;
    UINT16 WR_OVERRIDE        :1;
    UINT16 FFT_WR             :1;
    UINT16 IFFT_WR            :1;
    UINT16 QEDCOMP_START      :1;
    UINT16 QEDTONE_START      :1;
    UINT16 QEDPOW_START       :1;
    UINT16 QEDFIR_START       :1;
    UINT16 QEDTEQ_START       :1;
    UINT16 FFTBCOMP_START     :1;
    UINT16 ATEPOW_START       :1;
    UINT16 TEQ_AUTO_START     :1;   // available for Trapper or after
}SEQCOMP_TYPE;


typedef volatile struct
{
    SEQWR_TYPE        SEQWR;       // 0x7F
    SADDR_TYPE        SADDR;       // 0x80
    SEQCTL_TYPE       SEQCTL;      // 0x81
    SEQCOMP_TYPE      SEQCOMP;     // 0x82
}SEQBLK_TYPE;



#endif
/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/seqblk.h $
 * 
 * 9     8/16/01 5:59p Lauds
 * put physical addresses for each register as comments next to each
 * register definition
 * 
 * 8     6/26/01 8:29p Lauds
 * multi-instance version integration
 * 
 * 2     6/22/01 3:13p Lauds
 * minor cleanup and bug fix in miscutil.c and terminal.c
 * 
 * 1     6/21/01 3:10p Lauds
 * Multi-instance initial version
 * 
 * 7     5/21/01 6:22p Lauds
 * cleanup warning messages
 * 
 * 6     1/19/01 1:24p Lauhk
 * Cleanup code to reduce Euphrates warnings.
 * 
 * 5     12/01/00 2:49p Lauds
 * new trapper registers
 * 
 * 4     11/09/99 9:30a Lewisrc
 * 2.02f code release from Pairgain
 * 
 *    Rev 1.17   Oct 26 1999 16:38:30   yangl
 * -- Stop Support Falcon 1Z
 * -- Dynamic config with Falcon2 & 2P
 * 
 * 
 *    Rev 1.16   Jul 29 1999 16:57:54   nordees
 * Falcon 2P support.
 *
*
*****************************************************************************/
