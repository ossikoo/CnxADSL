/************************************************************************
*  $Workfile: fftblk.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/fftblk.h 13    8/16/01 5:55p Lauds $
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
      the Fast Fourier Transform (FFT) block of the FALCON.
*************************************************************************/

#ifndef FFTBLK_H
#define FFTBLK_H

#include "types.h"
#include "sysinfo.h"


/**************************************************************************
     FFT Control Latch         (Write Only)
**************************************************************************/


// Ref: "FFT Control Latch", Sec. 12.2.1, Falcon User's guide
// Define the MODE in which the fine-gain operand source is determined:
// This 2 bit field is TGAIN field, specified in FFTCTL register

enum TGAIN_SOURCE {FFT_USE_2BIT_STATUS, // note: other option should not be used for FFTA
                   FFT_USE_GAIN_RAM,    // Use Fine_Gain RAM only
                   FFT_USE_TADJUST1,    // Use TADJUST1 control reg only
                   FFT_USE_TADJUST2};   // Use TADJUST2 control reg only

enum {USE_NIFFT_VALUE, USE_NFFT_VALUE};

// Define data structure for FFT Control Latch.

typedef volatile struct
{
   UINT16 NIFFT         :2; // # of subchannels for IFFT operation
                            // 00: 32, 01: 64, 10: 128, 11: 256
   UINT16 NFFT          :2; // # of subchannels for FFT operation
                            // 00: 32, 01: 64, 10: 128, 11: 256
   UINT16 TGAIN         :2; // TGAIN field should be always set to
                            // USE_2BIT_STATUS for FFTA and which uses
                            // the TADJUST in QAM_TCR register to
                            // select the gain source. Enumeration for
                            // the choice here is declared in QEDFER.h
   UINT16 SCALE_IFFT    :1; // 0: No scaling, 1: scaling: right shift 1 bit
   UINT16 SAT_IFFT      :1; // 0: No saturation perform, 1: saturation perform
   UINT16 SCALE_FFT     :1; // 0: No scaling, 1: scaling: right shift 1 bit
   UINT16 SAT_FFT       :1; // 0: No saturation perform, 1: saturation perform
   UINT16 SQ_SPECIAL    :1; // 0: No Squaring of burst output data
   UINT16 BR_SPECIAL    :1; // 0: No bit reversal of the burst output data
   UINT16 NBURST        :1; //
   UINT16 WIN_OVERRIDE  :1;
   UINT16 TOGGLE        :1;          
   UINT16 TEQ_AUTO      :1; // Trapper only! It's used for NYQ_OUT before (kinda useless)
}FFTCTL_TYPE;

// note: The FFTA block will use the QAM_TCRL.TADJUST to decide to use TADJUST1,
// TADJUST2, or FFTA GAIN RAM for tx gain (except TX_MODE = TX_NORMAL) if
// FFTCTL.TGAIN is set to USE_2BIT_STATUS.  This is the recommended method
// to control tx gain factor

#define  FFTCTL_DEFAULT                                                      \
{                                                                            \
    ASIC_CONVERT_CHNL_SIZE(NIFFT_VALUE),              /*   NIFFT         */  \
    ASIC_CONVERT_CHNL_SIZE(NFFT_VALUE),               /*   NFFT          */  \
    FFT_USE_2BIT_STATUS,                              /*   TGAIN         */  \
    1,                                                /*   SCALE_IFFT    */  \
    1,                                                /*   SAT_IFFT      */  \
    0,                                                /*   SCALE_FFT     */  \
    1,                                                /*   SAT_FFT       */  \
    0,                                                /*   SQ_SPECIAL    */  \
    0,                                                /*   BR_SPECIAL    */  \
    0,                                                /*   NBURST        */  \
    0,                                                /*   WIN_OVERRIDE  */  \
    0,                                                /*   TOGGLE        */  \
    0                                                 /* TEQ_AUTO/NYQ_OUT*/  \
}


/*********  complex multiplication configuration control *************/
#define CMPLX_MULT_CFG_CTL                                                   \
{                                                                            \
    0,                                       /* NIFFT        (irrelevant) */ \
    0,                                       /* NFFT         (irrelevant) */ \
    FFT_USE_TADJUST1,                        /* TGAIN        (irrelevant) */ \
    0,                                       /* SCALE_IFFT   (irrelevant) */ \
    0,                                       /* SAT_IFFT     (irrelevant) */ \
    0,                                       /* SCALE_FFT    (irrelevant) */ \
    0,                                       /* SAT_FFT      (irrelevant) */ \
    0,                                       /* SQ_SPECIAL                */ \
    0,                                       /* BR_SPECIAL                */ \
    USE_NIFFT_VALUE,                         /* NBURST       (irrelevant) */ \
    1,                                       /* WIN_OVERRIDE (irrelevant) */ \
    0,                                       /* TOGGLE       (irrelevant) */ \
    0,                                       /* TEQ_AUTO/NYQ_OUT          */ \
}

/*********  conventional IFFT configuration control *************/
#define IFFT_CFG_CTL                                                         \
{                                                                            \
    ASIC_CONVERT_CHNL_SIZE(QEDFER_RX_CHNL),  /* NIFFT                     */ \
    ASIC_CONVERT_CHNL_SIZE(QEDFER_RX_CHNL),  /* NFFT         (irrelevant) */ \
    FFT_USE_TADJUST1,                        /* TGAIN                     */ \
    1,                                       /* SCALE_IFFT                */ \
    1,                                       /* SAT_IFFT                  */ \
    0,                                       /* SCALE_FFT    (irrelevant) */ \
    1,                                       /* SAT_FFT      (irrelevant) */ \
    0,                                       /* SQ_SPECIAL                */ \
    1,                                       /* BR_SPECIAL                */ \
    USE_NIFFT_VALUE,                         /* NBURST                    */ \
    1,                                       /* WIN_OVERRIDE              */ \
    0,                                       /* TOGGLE       (irrelevant) */ \
    0,                                       /* TEQ_AUTO/NYQ_OUT          */ \
}

/*********  conventional FFT configuration control *************/
#define FFT_CFG_CTL                                                          \
{                                                                            \
    ASIC_CONVERT_CHNL_SIZE(QEDFER_RX_CHNL),  /* NIFFT        (irrelevant) */ \
    ASIC_CONVERT_CHNL_SIZE(QEDFER_RX_CHNL),  /* NFFT                      */ \
    FFT_USE_TADJUST1,                        /* TGAIN                     */ \
    1,                                       /* SCALE_IFFT   (irrelevant) */ \
    1,                                       /* SAT_IFFT     (irrelevant) */ \
    0,                                       /* SCALE_FFT                 */ \
    1,                                       /* SAT_FFT                   */ \
    0,                                       /* SQ_SPECIAL                */ \
    1,                                       /* BR_SPECIAL                */ \
    USE_NFFT_VALUE,                          /* NBURST                    */ \
    1,                                       /* WIN_OVERRIDE              */ \
    0,                                       /* TOGGLE       (irrelevant) */ \
    0,                                       /* TEQ_AUTO/NYQ_OUT          */ \
}

/*********  window FFT configuration control *************/
#define WIN_FFT_CFG_CTL                                                      \
{                                                                            \
    ASIC_CONVERT_CHNL_SIZE(QEDFER_RX_CHNL),  /* NIFFT        (irrelevant) */ \
    ASIC_CONVERT_CHNL_SIZE(QEDFER_RX_CHNL),  /* NFFT                      */ \
    FFT_USE_TADJUST1,                        /* TGAIN                     */ \
    1,                                       /* SCALE_IFFT   (irrelevant) */ \
    1,                                       /* SAT_IFFT     (irrelevant) */ \
    0,                                       /* SCALE_FFT                 */ \
    1,                                       /* SAT_FFT                   */ \
    0,                                       /* SQ_SPECIAL                */ \
    1,                                       /* BR_SPECIAL                */ \
    USE_NFFT_VALUE,                          /* NBURST                    */ \
    0,                                       /* WIN_OVERRIDE              */ \
    0,                                       /* TOGGLE       (irrelevant) */ \
    0,                                       /* TEQ_AUTO/NYQ_OUT          */ \
}

/*********** UNWIN TEQ FFT and IFFT configuration control *************/
#define UNWIN_TEQ_CFG_CTL                                                    \
{                                                                            \
    ASIC_CONVERT_CHNL_SIZE(QEDFER_RX_CHNL),  /* NIFFT                     */ \
    ASIC_CONVERT_CHNL_SIZE(QEDFER_RX_CHNL),  /* NFFT                      */ \
    FFT_USE_TADJUST1,                        /* TGAIN                     */ \
    1,                                       /* SCALE_IFFT                */ \
    1,                                       /* SAT_IFFT                  */ \
    0,                                       /* SCALE_FFT                 */ \
    1,                                       /* SAT_FFT                   */ \
    0,                                       /* SQ_SPECIAL                */ \
    1,                                       /* BR_SPECIAL                */ \
    USE_NIFFT_VALUE,                         /* NBURST                    */ \
    0,                                       /* WIN_OVERRIDE              */ \
    0,                                       /* TOGGLE                    */ \
    0,                                       /* TEQ_AUTO/NYQ_OUT          */ \
}

/*********** UNWIN FIR FFT and IFFT configuration control *************/
#define UNWIN_FIR_CFG_CTL                                                    \
{                                                                            \
    ASIC_CONVERT_CHNL_SIZE(QEDFER_RX_CHNL),  /* NIFFT                     */ \
    ASIC_CONVERT_CHNL_SIZE(QEDFER_RX_CHNL),  /* NFFT                      */ \
    FFT_USE_TADJUST1,                        /* TGAIN                     */ \
    1,                                       /* SCALE_IFFT                */ \
    1,                                       /* SAT_IFFT                  */ \
    0,                                       /* SCALE_FFT                 */ \
    1,                                       /* SAT_FFT                   */ \
    0,                                       /* SQ_SPECIAL                */ \
    1,                                       /* BR_SPECIAL                */ \
    USE_NIFFT_VALUE,                         /* NBURST                    */ \
    0,                                       /* WIN_OVERRIDE              */ \
    1,                                       /* TOGGLE                    */ \
    0,                                       /* TEQ_AUTO/NYQ_OUT          */ \
}




/**************************************************************************
    IFFT Alternate Fine-Gain Adjustment Latch   (Write Only)
    IFFT Dynamic Range Control Latch            (Write Only)
    FFT Dynamic Range Control Latch             (Write Only)
    FFT Bit Swap Operand Latch                  (Write Only)
**************************************************************************/

typedef volatile UINT16 TADJUST_TYPE;

typedef volatile UINT16 IFFTDYN_TYPE;
typedef volatile UINT16 FFTDYN_TYPE;
typedef volatile UINT16 SWAP_TYPE;



// To resolve a mysterious upstream FECs problem with Hyundai ADI DSLAMs
// we reduce the TADJUST from 0.56 to 0.48
#define  TADJUST1_A_DEFAULT     FLT2INT16(0.48f)

#define  TADJUST2_A_DEFAULT     FLT2INT16(0.08875f)    /* 0.56*1e(-0.8) */
#define  IFFTDYN_B_DEFAULT      0x0800

// refer XCVR Software Spec 6.5.2.4
#ifdef CO
// drop the tx gain on FALCON 2P to work around unknow issue
//#define  IFFTDYN_A_DEFAULT_F2P  FLT2INT16(0.516)
#define  IFFTDYN_A_DEFAULT_F2P  FLT2INT16(0.410)
// drop the tx gain on FALCON 2 to work around the DAC saturation problem
// drop the GAIN in DACCTLL for ATU_R and drop IFFTDYN for ATU_C
#define  IFFTDYN_A_DEFAULT_F2   FLT2INT16(0.516/2)
#else
#define  IFFTDYN_A_DEFAULT_F2      FLT2INT16(0.25)
// to compensate for the TADJUST reduction, we are increasing IFFTDYN from
// 0.2 to 2.3333
#define  IFFTDYN_A_DEFAULT_F2P     FLT2INT16(0.2333f)
#endif

// TADJUST1_B set to 0.99997 to minimize attenuation during TEQ training

#define  TADJUST1_B_DEFAULT     0x7FFF
#define  TADJUST2_B_DEFAULT     0


#define  FFTDYN_A_DEFAULT      FLT2INT16(FFTDYN_GAIN)

// FFTDYN_B is initialized to 1/16 will provide no gain during TEQ training
// (i.e. there is a automatic left shift of 4 in FFT block)

#define  FFTDYN_B_DEFAULT      FLT2INT16(0.0625)

#define  SWAP1_A_DEFAULT        0
#define  SWAP2_A_DEFAULT        0
#define  SWAP3_A_DEFAULT        0
#define  SWAP1_B_DEFAULT        0
#define  SWAP2_B_DEFAULT        0
#define  SWAP3_B_DEFAULT        0


/**************************************************************************
     FFT Bit Swap Address Latch  (Write Only)
**************************************************************************/

typedef volatile struct
{
   UINT16 ADDR2        :8;
   UINT16 ADDR1        :8;
}FFT_ADDRL_TYPE;


typedef volatile struct
{
   UINT16 ADDR3         :8;
   UINT16 WR_PILOT      :1;
   UINT16 NYQ_OUT       :1;  // Trapper only
   UINT16               :4;  // unused
   UINT16 GR_SYSNC_EN   :1;  // Falcon IZ
   UINT16 GAIN_RAM_PAGE :1;  // Falcon IZ
}FFT_ADDRH_TYPE;

#define  FFTA_ADDRL_DEFAULT     {0,0}
#define  FFTA_ADDRH_DEFAULT     {0,0,0,0,0}
#define  FFTB_ADDRL_DEFAULT     {0,0}
#define  FFTB_ADDRH_DEFAULT     {0,0,0,0,0}

/**************************************************************************
     FFT Window Start Latch         (Write Only)
     FFT Window End Latch           (Write Only)
**************************************************************************/

// note that in reality only 9-bit data in WIN_START and WIN_END are writable

typedef volatile UINT16 WIN_START_TYPE;
typedef volatile UINT16 WIN_END_TYPE;

#define  WIN_START_DEFAULT     NUM_RX_CHNL

#define  WIN_END_DEFAULT       0


/**************************************************************************
     FFT Power Adjust Latch         (Write Only)
**************************************************************************/

typedef volatile UINT16 POW_ADJ_TYPE;

#define  POW_ADJ_A_DEFAULT     0                 // not used

// default to be 1/16 due to the internal left shift of 4 in POW_ADJ multiplication

#define  POW_ADJ_B_DEFAULT     FLT2INT16(0.0625f)

/**************************************************************************
     FFT Nyquist Data Register TEQ  (Write Only)
     FFT Nyquist Data Register FIR  (Write Only)
**************************************************************************/

typedef volatile UINT16 NYQUIST_TYPE;

#define  FFTA_TEQ_NYQUIST_DEFAULT     0
#define  FFTA_FIR_NYQUIST_DEFAULT     0
#define  FFTB_TEQ_NYQUIST_DEFAULT     0
#define  FFTB_FIR_NYQUIST_DEFAULT     0


typedef volatile struct
{
    FFTCTL_TYPE          FFTCTL;          // 0x1D for FFTA and 0x2D for FFTB
    TADJUST_TYPE         TADJUST1;        // 0x1E for FFTA and 0x2D for FFTB
    TADJUST_TYPE         TADJUST2;        // 0x1F for FFTA and 0x2E for FFTB
    IFFTDYN_TYPE         IFFTDYN;         // 0x20 for FFTA and 0x2F for FFTB
    FFTDYN_TYPE          FFTDYN;          // 0x21 for FFTA and 0x30 for FFTB
    SWAP_TYPE            SWAP1;           // 0x22 for FFTA and 0x31 for FFTB
    SWAP_TYPE            SWAP2;           // 0x23 for FFTA and 0x32 for FFTB
    SWAP_TYPE            SWAP3;           // 0x24 for FFTA and 0x33 for FFTB
    FFT_ADDRL_TYPE       FFT_ADDRL;       // 0x25 for FFTA and 0x34 for FFTB
    FFT_ADDRH_TYPE       FFT_ADDRH;       // 0x26 for FFTA and 0x35 for FFTB
    WIN_START_TYPE       WIN_START;       // 0x27 for FFTA and 0x36 for FFTB
    WIN_END_TYPE         WIN_END;         // 0x28 for FFTA and 0x37 for FFTB
    POW_ADJ_TYPE         POW_ADJ;         // 0x29 for FFTA and 0x38 for FFTB
    NYQUIST_TYPE         TEQ_NYQUIST;     // 0x2A for FFTA and 0x39 for FFTB
    NYQUIST_TYPE         FIR_NYQUIST;     // 0x2B for FFTA and 0x3A for FFTB
}FFTBLK_TYPE;

#endif
/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/fftblk.h $
 * 
 * 13    8/16/01 5:55p Lauds
 * put physical addresses for each register as comments next to each
 * register definition
 * 
 * 12    6/26/01 8:03p Lauds
 * multi-instance version integration
 * 
 * 11    4/24/01 11:38a Wangm2
 * Changed TADJUST1_A_DEFAULT back to its original value due to the 0.5
 * boost caused problems for ADI DSLAM, as the filters for ADI DSLAM were
 * designed based on the TADJUST1_A_DEFAULT original value. 
 * 
 * 10    4/23/01 4:12p Wangm2
 * Increased TADJUST1_A_DEFAULT by 0.5dB, and this increase is compnesated
 * by 0.5dB in Tx filter gain. No reason for this other than that most of
 * the tests in upstream tx power boost were done in this distribution.
 * 
 * 9     11/22/00 3:31p Lauds
 * add registers for Trapper
 * 
 * 8     11/08/00 12:05p Lauds
 * raise IFFTDYN to compensate TADJUST reduction
 * 
 * 7     10/30/00 9:11a Lauds
 * Reduce TADJUST value from 0.56 to 0.48 to fix upstream FEC error with
 * Hyundai ADI and TI DSLAMs
 * 
 * 6     2/28/00 10:48a Lewisrc
 * 2.03 Pairgain release for San Jose Code Interop
 * 
 *    Rev 1.41   Feb 25 2000 16:52:56   yangl
 * -- Change default IFFTDYN value for F2P
 * 
 *    Rev 1.40   Oct 26 1999 16:38:28   yangl
 * -- Stop Support Falcon 1Z
 * -- Dynamic config with Falcon2 & 2P
 * 
 * 
 *    Rev 1.39   Sep 02 1999 14:44:56   nordees
 * Removed gain drop fix (needed on Falcon 2 only) for Falcon 2P.
 *
 *    Rev 1.38   Feb 09 1999 15:26:38   laud
 * drop gain for FALCON 2 DAC saturation bug
 *
 *    Rev 1.37   28 Jul 1998 16:34:38   tsaurl
 * update for falcon 1z
 *
 *    Rev 1.39   27 Jul 1998 16:44:10   laud
 *
 *
 *    Rev 1.38   27 Jul 1998 16:38:22   laud
 *
 *
 *    Rev 1.37   27 Jul 1998 16:18:34   laud
 *
*
*****************************************************************************/
