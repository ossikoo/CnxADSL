/************************************************************************
*  $Workfile: ateblk.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/ateblk.h 4     8/16/01 5:52p Lauds $
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
      the ADC Interface and TEQ (ATE) block of the FALCON.
*************************************************************************/

#ifndef ATEBLK_H
#define ATEBLK_H

#include "types.h"
#include "sysinfo.h"


/**************************************************************************
     ATE Control Latch         (Write Only) (only one bit is readable)
    Ref: 12.5.1 Falcon

**************************************************************************/
enum ADC_PASS_CHOICE{NORMAL_ADC, TEST_ADC, WR_23_8, WR_19_4, WR_15_0};
enum CLIP_CHOICE    {CLIP_12_BIT, CLIP_13_BIT, CLIP_14_BIT, CLIP_16_BIT};
enum TEQ_ALL_CHOICE {FIR_TEQ,TEQ_62};
enum LOOPBACK_CHOICE{NO_ATE_LOOPBACK,ATE_LOOPBACK};


typedef volatile struct
{
   UINT16 NRC           :2; // # of rcv subchan to be decoded
                            // 00(32),01 (64), 10(128), 11(256)
   UINT16 ADC_PASS      :3; // Selection of ADC interface output destination
   UINT16 CLIP          :2; // Select the digital values for which the ATE
                            //  block will notify uP thru the CLIP-interrupt
   UINT16               :1; // UNUSED
   UINT16 POW_RDY       :1; // 1: ATE block finishes a signasl power average
                            //  computation and the result in AD_POW
   UINT16 OFFSET_ADC    :1; //
   UINT16 OFFSET_DAC    :1;
   UINT16 LOOPBACK      :1;
   UINT16 OVER_SAMPLING :1; // 0: plain FIR filter (FALCON 1a only)
                            // 1: prefilter will be take input at 2X rate
   UINT16 FIR_LONG      :1; // 0: 30 taps of receiver block FIR (FALCON 1z)
                            // 1: 62 taps
   UINT16 TEQ_LONG      :1; // 0: 32 taps of TEQ (FALCON 1z only)
                            // 1: 64 taps
   UINT16 TEQ_BYPASS    :1; // 0: output of TEQ sent to FIFO
                            // 1: bypass TEQ block
}ATECTL_TYPE;

// note if number of FIR coef is zero, that's mean we use all 62 taps as TEQ

// *** need to be defined later on.
#define ATECTL_DEFAULT                                         \
     {ASIC_CONVERT_CHNL_SIZE(NFFT_VALUE),   /*  NRC        */  \
      NORMAL_ADC,                           /*  ADC_PASS   */  \
      CLIP_14_BIT,                          /*  CLIP       */  \
      0,                                    /*  POW_RDY    */  \
      (ADC_MODE == OFFSET_BINARY)? 1:0,     /*  OFFSET_ADC */  \
      (DAC_MODE == OFFSET_BINARY)? 1:0,     /*  OFFSET_DAC */  \
      NO_ATE_LOOPBACK,                      /*  LOOPBACK   */  \
      0,                                    /*  OVER_SAMPLING*/\
      (NUM_FIR_COEF == 30) ? 0:1,           /*  30 or 62 tap FIR */\
      (NUM_TEQ_COEF == 32) ? 0:1,           /*  32 or 64 tap TEQ */\
      0}                                    /*  no Bypass TEQ   */

/**************************************************************************
     Average Signal Power of A/D Interface Output Register (Read Only)
    Ref: Sec. 12.5.2 Falcon
**************************************************************************/


typedef volatile UINT16 AD_POW_TYPE;

/**************************************************************************
     Receive Frame Alignment Control Register (Write Only)
    Ref: Sec 12.5.3 Falcon
**************************************************************************/

typedef volatile struct
{
   UINT16 STUFF     :1;     // 0-1 transition causes samples to be stuffed
   UINT16 ROB       :1;     // 0-1 transition causes samples to be added
   UINT16 FIFO_RST  :1;     // 0-1 transition reset Frame Buffer FIFO
   UINT16           :1;     // Unused
   UINT16 JUMP      :10;    // Number of samples in which the read addr ptr of
                            // the Frame buf should incr (stuffed) or
                            //  decrement (robbed).
}RFADJ_TYPE;

#define RFADJ_DEFAULT   {0, 0, 0, 0}



/**************************************************************************
     Frame Buffer FIFO Control Latch        (Write Only)
    Ref: Sec 12.5.4 Falcon
**************************************************************************/

typedef volatile struct
{
   UINT16 FR_START  :11;
   UINT16 HALF_RATE :1;
   UINT16 SHIFT_CNT :2;
   UINT16           :2;
} FR_START_TYPE;

#define FR_START_DEFAULT {0,0,0}

// the default value depends on the ATU (see initialization routine)


/**************************************************************************
     ADC Timing Control Latch         (Write Only)
    Ref: Sec.   12.5.5 Falcon
                "ADC operation", Section 6.1, Falcon
**************************************************************************/


typedef volatile struct
{
   UINT16 SWEET     :7;
   UINT16           :1; // UNUSED
   UINT16 PHASE     :7;
}ADCCTL_TYPE;

#define ADCCTL_DEFAULT {DEFAULT_ADC_SWEET_VALUE,0}


typedef volatile struct
{
    ATECTL_TYPE            ATECTL;        // 0x3E 
    AD_POW_TYPE            AD_POW;        // 0x3F 
    RFADJ_TYPE             RFADJ;         // 0x40 
    FR_START_TYPE          FR_START;      // 0x41 
    ADCCTL_TYPE            ADCCTL;        // 0x42 
}ATEBLK_TYPE;                                     



#endif
/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/ateblk.h $
 * 
 * 4     8/16/01 5:52p Lauds
 * put physical addresses for each register as comments next to each
 * register definition
 * 
 * 3     6/26/01 7:41p Lauds
 * multi-instance version integration
 * 
 * 2     3/31/99 4:16p Lewisrc
 * 03/22/99 Pairgain Release (2.01a)
 * 
 *    Rev 1.27   Jan 28 1999 19:17:52   laud
 * get rid of 1Z compiler switch
 *
 *    Rev 1.26   Sep 15 1998 16:37:28   laud
 * change shift_cnt field to 2 bits
 *
 *    Rev 1.25   01 Sep 1998 15:24:16   tsaurl
 * modify fr_start reg and default value
 *
 *    Rev 1.24   Aug 28 1998 14:47:20   laud
 * ATEBLK support for FALCON 1Z
 *
 *    Rev 1.23   28 Jul 1998 16:32:12   tsaurl
 * update for falcon 1Z
 *
*
*****************************************************************************/
