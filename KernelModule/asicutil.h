/************************************************************************
*  $Workfile: asicutil.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/asicutil.h 20    7/10/01 12:55p Lauds $
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
    Description: This header module provides macros for all FALCON related
        functionalities.

*************************************************************************/

#ifndef ASICUTIL_H
#define ASICUTIL_H

#include "types.h"
#include "sysinfo.h"
#include "fftblk.h"
#include "qedfer.h"


//Structure for holding PRBS data for use with Echo Cancelling and peforming correlation
typedef struct
{
    UINT8 real;
    UINT8 imag;
}PRBS_TYPE;


// this macro will optimize single xcvr tremendously for the macros define in this module
#define FALCON_PTR(pDatapump)  (&(pDatapump)->falcon)

/**************************************************************************
     Constants for TEQ filter setup choice
**************************************************************************/

enum TEQ_SETUP_MODE {TEQ_TONE_DETECTION, TEQ_PRBS_DETECTION, TEQ_RX_FILTER, TEQ_CALCED_FILTER};


/**************************************************************************
     Constants for MIF block transfers definition
**************************************************************************/


#define ZERO_PAD (void *)0L

enum XFR_FORMAT {NORMAL_INT8=1, NORMAL_INT16=2, NORMAL_INT32=4,
                 PRIMARY_BYTE,REAL_OF_CMPLX32,IMAG_OF_CMPLX32,
                 REAL_OF_CMPLX16,IMAG_OF_CMPLX16,
                 BYTE_SPECIAL,EVEN_WORD,ODD_WORD,ODD_BYTE,BACKUP_BYTE,
                 TONE_DETECTION, FAST_SIG_DETECT, ACCUMULATE_INT32,
                 PILOT_DATA, GHS_TONE_DETECTION};

enum PWR_AVG_REQ_CHOICE {   UNUSED_AVG,
                            ERR_PWR_AVG,
                            ERR_PWR_AVG_UPDATE,
                            SIG_PWR_AVG,
                            UNWIN_FIR,
                            UNWIN_TEQ
                        };

/**************************************************************************
     Macro calls for tracing startup sequence
**************************************************************************/
#ifdef STARTUP_TRACE    // very useful for interoperability testing
#define STARTUP_TRACE_ROUTINE(pDatapump, sub_code)     \
{                                                       \
    DIAG_AddDiagnostic(pDatapump, TX_GetStateStr(TX_GetCurrentState(pDatapump)),sub_code); \
    DIAG_AddDiagnostic(pDatapump, RX_GetStateStr(RX_GetCurrentState(pDatapump)),sub_code); \
    DIAG_AddDiagnostic(pDatapump, BG_GetStateStr(BG_GetCurrentState(pDatapump)),sub_code); \
    DIAG_AddResetCount(pDatapump,sub_code); \
}
#else
#define STARTUP_TRACE_ROUTINE(pDatapump, sub_code)
#endif


/**************************************************************************
     Macro calls for FFTBLK related functionality
**************************************************************************/
// In the FALCON, number of subchannels are represented by 4 (2-bit) discrete
// representation (i.e 00 = CHNL_32, 01 = CHNL_64, 10 = CHNL_128, 11 = CHNL_256)
// this macro convert number of subchannels to the corresponding 2-bit
// representation (note that input should be a multiple of 32)
#define ASIC_CONVERT_CHNL_SIZE(x)    \
       (((x) == 32) ? 0 :            \
       (((x) == 64) ? 1 :            \
       (((x) == 128)? 2 : 3 )))      \

// refer to section 5.2 in the FALCON user guide
// Logical start address for fixed alignment
#define ASIC_DAC_START_ADDR(x)        \
       (((x) == 32) ? 0x46f :         \
       (((x) == 64) ? 0x45f :         \
       (((x) == 128)? 0x43f : 0x3ff )))

// note that in reality only 9-bit data in WIN_START and WIN_END are writable
// In other word, writing 512 (i.e. 0x200) is equivalent to writing 0.  So
// we don't have to worry about overflow when parameter end is 511, or
// underflowing when start is 0
#define ASIC_SET_TEQ_WINDOW(pDatapump,start,end)  \
{                                        \
    FALCON_PTR(pDatapump)->FFTB.WIN_START = (UINT16)((end)+1);        \
    FALCON_PTR(pDatapump)->FFTB.WIN_END = (UINT16)((start)-1);   \
    TRAPPER_FFT_REG_WRITE(pDatapump, FFTB, WIN_START); \
    TRAPPER_FFT_REG_WRITE(pDatapump, FFTB, WIN_END);   \
}

// However, we need to do a modulo of ADCSAMPLES for upstream to avoid values larger
// than ADCSAMPLES for FIR windown. (An unknown anomaly in FALCON)
#define ASIC_SET_FIR_WINDOW(pDatapump,start,end)  \
{                                                              \
    FALCON_PTR(pDatapump)->FFTB.WIN_START               \
        =(UINT16)(XCVR_MODE == ATU_C ? (end+1)&(ADCSAMPLES-1): end+1); \
    TRAPPER_FFT_REG_WRITE(pDatapump, FFTB, WIN_START);                 \
    FALCON_PTR(pDatapump)->FFTB.WIN_END                 \
        =(UINT16)((start)-1);                                            \
    TRAPPER_FFT_REG_WRITE(pDatapump, FFTB, WIN_END);                 \
}


#define ASIC_SET_FFTB_PWR_ADJUSTMENT(pDatapump, adjustment)     \
{                                                    \
   FALCON_PTR(pDatapump)->FFTB.POW_ADJ =(adjustment);   \
   TRAPPER_FFT_REG_WRITE(pDatapump, FFTB, POW_ADJ);           \
}

#define ASIC_UPLD_TX_GAIN_TBL(pDatapump, table)         \
{                                            \
   ASIC_BurstRead(pDatapump,               \
                  FFTA_GAIN,                 \
              (void *)(table),                         \
              NUM_TX_CHNL,                   \
              ODD_WORD);                     \
}

#define ASIC_DWLD_TX_GAIN_TBL(pDatapump, table)         \
{                                            \
   ASIC_BurstWrite(pDatapump,               \
                   FFTA_GAIN,                 \
              (void *)(table),                         \
              NUM_TX_CHNL,                   \
              ODD_WORD);                     \
}

// c1,c2 are 32 bit value with sign bit in MSB. Both are LSB aligned.
// However C1, C2 are 20 bits in value. The 16LSB of C1,C2 are in C1,C2 reg.
// The 4MSB of C1, C2 are in store register at FALCON address 0x00b and 0x00c.
// Note: temp is created to ensure STORE field is not modified in FALCON_PTR(pDatapump)->QEDFER->STORE_Reg
#define ASIC_SET_LOWPASS_FILTER_COEF(pDatapump, c1,c2)     \
{                                                                  \
    FALCON_REG_READ(pDatapump, QEDFER, STORE_HI_Reg);                 \
    FALCON_PTR(pDatapump)->QEDFER.STORE_HI_Reg.C2=(UINT16)((c2)>>16);                               \
    FALCON_PTR(pDatapump)->QEDFER.STORE_HI_Reg.C1=(UINT16)((c1)>>16);                               \
    FALCON_PTR(pDatapump)->QEDFER.C1_Coef = (UINT16)(c1);    \
    FALCON_PTR(pDatapump)->QEDFER.C2_Coef = (UINT16)(c2);    \
    FALCON_REG_WRITE(pDatapump, QEDFER, C1_Coef);   \
    FALCON_REG_WRITE(pDatapump, QEDFER, C2_Coef);    \
    FALCON_REG_WRITE(pDatapump, QEDFER, STORE_HI_Reg);                 \
}

// c1,c2 are 32 bit value with sign bit in MSB. Both are LSB aligned.
// However C1, C2 are 20 bits in value. The 16LSB of C1,C2 are in C1,C2 reg.
// The 4MSB of C1, C2 are in store register at FALCON address 0x00b and 0x00c.
#define ASIC_GET_LOWPASS_FILTER_COEF(pDatapump, c1,c2)     \
{                                                                  \
    c1 = ((UINT32)(FALCON_PTR(pDatapump)->QEDFER.STORE_HI_Reg.C1)<<16) +            \
          (FALCON_PTR(pDatapump)->QEDFER.C1_Coef);                                  \
    c2 = ((UINT32)(FALCON_PTR(pDatapump)->QEDFER.STORE_HI_Reg.C2)<<16) +            \
          (FALCON_PTR(pDatapump)->QEDFER.C2_Coef);                                  \
}


#define ASIC_SET_LOWPASS_FILTER_PARAMETER(pDatapump, store_value)     \
{                                               \
    FALCON_PTR(pDatapump)->QEDFER.STORE_LO_Reg = (store_value);           \
    FALCON_PTR(pDatapump)->QEDFER.STORE_HI_Reg.HI_STORE=(store_value)>>16;       \
    FALCON_REG_WRITE(pDatapump, QEDFER, STORE_LO_Reg);                 \
    FALCON_REG_WRITE(pDatapump, QEDFER, STORE_HI_Reg);                 \
}

#define ASIC_GET_LOWPASS_FILTER_PARAMETER(pDatapump, store_value)     \
{                                                          \
    FALCON_REG_READ(pDatapump, QEDFER, STORE_HI_Reg);  \
    FALCON_REG_READ(pDatapump, QEDFER, STORE_LO_Reg);  \
    store_value = ((((UINT32)FALCON_PTR(pDatapump)->QEDFER.STORE_HI_Reg.HI_STORE<<16)+ \
        FALCON_PTR(pDatapump)->QEDFER.STORE_LO_Reg));  \
}



/**************************************************************************
     Macro calls for QEDFER related functionality
**************************************************************************/
// refer to section 12.1.1 in the FALCON user guide
#define ASIC_CONVERT_PILOT_CHNL(x)      \
       (((x) == 8 ) ? 0 :               \
       (((x) == 16) ? 1 :               \
       (((x) == 64) ? 2 : 3 )))         \

#ifdef TESTDMT
                          
#define ASIC_FORCE_TX_STATE_TRANSITION(pDatapump){           \
    if ( TX_GetCurrentState(pDatapump)!=TX_GetBreakPoint(pDatapump))  \
    {                                               \
           FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.TX_OTHER = 0;               \
           FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);                    \
           FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.TX_OTHER = 1;               \
           FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);                   \
    }                                               \
}

#else
#define ASIC_FORCE_TX_STATE_TRANSITION(pDatapump){        \
        FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.TX_OTHER = 0;               \
        FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);                  \
        FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.TX_OTHER = 1;               \
        FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);                    \
}

#endif

#define ASIC_FORCE_TX_STATE_NOW(pDatapump)      \
{                                                   \
    if (CHIP_ID_AFTER_F2PT)                         \
    {                                                \
        FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.TXO_NOW = 0;               \
        FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);                   \
        FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.TXO_NOW = 1;               \
        FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);                \
    }                                                                 \
}

#ifdef TESTDMT
#define ASIC_FORCE_RX_STATE_TRANSITION(pDatapump){   \
    if ( RX_GetCurrentState(pDatapump)!=RX_GetBreakPoint(pDatapump)) \
    {                                              \
           FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.RX_OTHER = 0;              \
           FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);                   \
           FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.RX_OTHER = 1;              \
           FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);                  \
    }                                               \
}
#else
#define ASIC_FORCE_RX_STATE_TRANSITION(pDatapump){        \
        FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.RX_OTHER = 0;               \
        FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);                   \
        FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.RX_OTHER = 1;               \
        FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);                 \
}
#endif

#define ASIC_FORCE_SF_SEARCH(pDatapump){             \
   FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.SFSEARCH = 0;               \
   FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);                  \
   FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.SFSEARCH = 1;               \
   FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);                  \
}

#define ASIC_CLR_INTR(pDatapump, x)                               \
{                                                      \
   FALCON_PTR(pDatapump)->QEDFER.QAM_ICR.x = FALCON_INTR_DISABLE;               \
   FALCON_REG_WRITE(pDatapump, QEDFER, QAM_ICR);                             \
   FALCON_PTR(pDatapump)->QEDFER.QAM_ICR.x = FALCON_INTR_ENABLE;                \
   FALCON_REG_WRITE(pDatapump, QEDFER, QAM_ICR);                             \
}

#define ASIC_IS_INTR_ACTIVE(pDatapump, x) FALCON_PTR(pDatapump)->QEDFER.MIF_ISR.x

#define ASIC_DISABLE_ALL_INTR(pDatapump)                  \
{                                            \
    static const far QAM_ICR_TYPE disabled_icr =      \
    {                                                  \
        FALCON_INTR_DISABLE, FALCON_INTR_DISABLE, FALCON_INTR_DISABLE, FALCON_INTR_DISABLE,  \
        FALCON_INTR_DISABLE, FALCON_INTR_DISABLE, FALCON_INTR_DISABLE, FALCON_INTR_DISABLE,  \
        FALCON_INTR_DISABLE, FALCON_INTR_DISABLE, FALCON_INTR_DISABLE, FALCON_INTR_DISABLE,  \
        FALCON_INTR_DISABLE, FALCON_INTR_DISABLE, FALCON_INTR_DISABLE, FALCON_INTR_DISABLE   \
    };                                                                                       \
    FALCON_PTR(pDatapump)->QEDFER.QAM_ICR = disabled_icr;      \
    FALCON_REG_WRITE(pDatapump, QEDFER, QAM_ICR);              \
}



#define ASIC_ENABLE_INTR(pDatapump, x)                  \
{                                            \
   FALCON_PTR(pDatapump)->QEDFER.QAM_ICR.x = FALCON_INTR_ENABLE;      \
   FALCON_REG_WRITE(pDatapump, QEDFER, QAM_ICR);                  \
}

#define ASIC_DISABLE_INTR(pDatapump, x)                 \
{                                            \
   FALCON_PTR(pDatapump)->QEDFER.QAM_ICR.x = FALCON_INTR_DISABLE;     \
   FALCON_REG_WRITE(pDatapump, QEDFER, QAM_ICR);                   \
}

#define ASIC_SET_IRQ1(pDatapump, x)                     \
{                                            \
   FALCON_PTR(pDatapump)->QEDFER.INTR_PROG.x = IRQ1;                  \
   FALCON_REG_WRITE(pDatapump, QEDFER, INTR_PROG);                \
}

#define ASIC_SET_IRQ2(pDatapump, x)                     \
{                                            \
   FALCON_PTR(pDatapump)->QEDFER.INTR_PROG.x = IRQ2;                  \
   FALCON_REG_WRITE(pDatapump, QEDFER, INTR_PROG);                \
}

#define ASIC_GET_IRQ(pDatapump, x)    FALCON_PTR(pDatapump)->QEDFER.INTR_PROG.x

#define ASIC_DWLD_TX_BIT_TONE_TBL(pDatapump, table)     \
{                                            \
   ASIC_BurstWrite(pDatapump, Q_TX_BIT_TONE,            \
              (void *)(table),                         \
              NUM_TX_CHNL,                   \
              ODD_WORD);                     \
}

#define ASIC_DWLD_RX_BIT_TONE_TBL(pDatapump, table)     \
{                                            \
   ASIC_BurstWrite(pDatapump, Q_RX_BIT_TONE,            \
              (void *)(table),                         \
              QEDFER_RX_CHNL,                \
              EVEN_WORD);                    \
}

#define ASIC_UPLD_RX_TONE_INFO_TBL(pDatapump, table)    \
{                                            \
   ASIC_BurstRead(pDatapump,               \
                  Q_RX_TONE_INFO,            \
              (void *)(table),                         \
              QEDFER_RX_CHNL,                \
              EVEN_WORD);                    \
}

#define ASIC_DWLD_RX_TONE_INFO_TBL(pDatapump, table)    \
{                                            \
   ASIC_BurstWrite(pDatapump, Q_RX_TONE_INFO,           \
              (void *)(table),                         \
              QEDFER_RX_CHNL,                \
              EVEN_WORD);                    \
}

#define ASIC_UPLD_TEQ_COEF(pDatapump, table) \
{                                            \
   ASIC_BurstRead(                           \
              pDatapump,                    \
              ATE_FIR_TEQ_SHCOEF,            \
              (void *)(table),               \
              MAX_NUM_FIR_COEF+NUM_TEQ_COEF, \
              ODD_WORD);                     \
}

#define ASIC_DWLD_TEQ_COEF(pDatapump, table)  \
{                                            \
   ASIC_BurstWrite(                          \
              pDatapump,                    \
              ATE_FIR_TEQ_SHCOEF,            \
              (void *)(table),               \
              MAX_NUM_FIR_COEF+NUM_TEQ_COEF, \
              ODD_WORD);                     \
}

#define ASIC_DEFINE_ACTIVE_TX_CHNL(pDatapump, start,end)\
{                                            \
   FALCON_PTR(pDatapump)->QEDFER.TSUB.TSUB_START = (UINT16)(start);           \
   FALCON_PTR(pDatapump)->QEDFER.TSUB.TSUB_END   = (UINT16)(end);             \
   FALCON_REG_WRITE(pDatapump, QEDFER, TSUB);                          \
}


#define ASIC_UPLD_RX_GAIN_TBL(pDatapump, table)         \
{                                            \
   ASIC_BurstRead(pDatapump,               \
                  Q_RX_GAIN,                 \
                  (void *)(table),               \
                  NUM_RX_CHNL,                   \
                  ODD_WORD);                     \
}

#define ASIC_DWLD_RX_GAIN_TBL(pDatapump, table)         \
{                                            \
   ASIC_BurstWrite(pDatapump,              \
                  Q_RX_GAIN,                \
                  (void *)(table),               \
                  NUM_RX_CHNL,                   \
                  ODD_WORD);                     \
}


// this macro will disable DGC gain shift when FEQ coef overflow
#define ASIC_FREEZE_FEQ_DGC(pDatapump)      \
{                                            \
    FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.FEQDYNR_MCR = 1;                   \
    FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);   \
}

// this macro will allow DGC to update when FEQ coef overflows
#define ASIC_UNFREEZE_FEQ_DGC(pDatapump)      \
{                                            \
    FALCON_PTR(pDatapump)->QEDFER.QAM_MCR.FEQDYNR_MCR = 0;                   \
    FALCON_REG_WRITE(pDatapump, QEDFER, QAM_MCR);   \
}

/**************************************************************************
     Macro calls for ATEBLK related functionality
**************************************************************************/


#define ASIC_IS_ATE_PWR_AVG_RDY(pDatapump) (FALCON_PTR(pDatapump)->ATEBLK->ATECTL.POW_RDY)

// CAUTION : only ROB is working in FALCON 1
#define ASIC_STUFF(pDatapump, samples)      /* use in FALCON 1a only */ \
{                                            \
   FALCON_PTR(pDatapump)->ATEBLK.RFADJ.JUMP = (samples);       \
   FALCON_PTR(pDatapump)->ATEBLK.RFADJ.STUFF = 0;              \
   FALCON_REG_WRITE(pDatapump, ATEBLK, RFADJ);                 \
   FALCON_PTR(pDatapump)->ATEBLK.RFADJ.STUFF = 1;              \
   FALCON_REG_WRITE(pDatapump, ATEBLK, RFADJ);                  \
}

#define ASIC_ROB(pDatapump, samples)                      \
{                                          \
   FALCON_PTR(pDatapump)->ATEBLK.RFADJ.JUMP = (samples);     \
   FALCON_PTR(pDatapump)->ATEBLK.RFADJ.ROB = 0;              \
   FALCON_REG_WRITE(pDatapump, ATEBLK, RFADJ);                \
   FALCON_PTR(pDatapump)->ATEBLK.RFADJ.ROB = 1;              \
   FALCON_REG_WRITE(pDatapump, ATEBLK, RFADJ);                \
}

#define ASIC_RESET_FRAME_FIFO(pDatapump)                  \
{                                          \
   FALCON_PTR(pDatapump)->ATEBLK.RFADJ.FIFO_RST = 0;         \
   FALCON_REG_WRITE(pDatapump, ATEBLK, RFADJ);                \
   FALCON_PTR(pDatapump)->ATEBLK.RFADJ.FIFO_RST = 1;         \
   FALCON_REG_WRITE(pDatapump, ATEBLK, RFADJ);                \
}


#define ASIC_ADJUST_ADC_SAMPLING_PHASE(pDatapump, phase)                                     \
{                                                                          \
   FALCON_PTR(pDatapump)->ATEBLK.ADCCTL.PHASE = (phase)*(ADCPHASE_PERIOD/NUM_SAMPLING_PHASE); \
   FALCON_REG_WRITE(pDatapump, ATEBLK, ADCCTL);                                    \
}

#define ASIC_DWLD_TX_FILTER(pDatapump, table)        \
{                                            \
   ASIC_BurstWrite(pDatapump, DAC_TX_COEF,                \
              (void *)(table),                         \
              DAC_TX_FILTER_SIZ,                 \
              NORMAL_INT16);                 \
}


/**************************************************************************
     Macro calls for SEQBLK related functionality
**************************************************************************/

// although this is a read/write register, image is used to work around
// the MIPS compiler bug

#define ASIC_START_FFT(pDatapump, fft_control)          \
{                                            \
   FALCON_PTR(pDatapump)->FFTB.FFTCTL = (fft_control);  \
   FALCON_REG_WRITE(pDatapump, FFTB, FFTCTL);   \
   FALCON_PTR(pDatapump)->SEQBLK.SEQCOMP.WIN_FFT = 0;                            \
   FALCON_REG_WRITE(pDatapump, SEQBLK, SEQCOMP);     \
   FALCON_PTR(pDatapump)->SEQBLK.SEQCOMP.WIN_FFT = 1;                            \
   FALCON_REG_WRITE(pDatapump, SEQBLK, SEQCOMP);     \
}

#define ASIC_START_IFFT(pDatapump, fft_control)          \
{                                             \
   FALCON_PTR(pDatapump)->FFTB.FFTCTL = (fft_control);  \
   FALCON_REG_WRITE(pDatapump, FFTB, FFTCTL);    \
   FALCON_PTR(pDatapump)->SEQBLK.SEQCOMP.WIN_IFFT = 0;                            \
   FALCON_REG_WRITE(pDatapump, SEQBLK, SEQCOMP);     \
   FALCON_PTR(pDatapump)->SEQBLK.SEQCOMP.WIN_IFFT = 1;                            \
   FALCON_REG_WRITE(pDatapump, SEQBLK, SEQCOMP);     \
}


#define ASIC_START_CMPLX_MULT(pDatapump, fft_control)     \
{                                              \
   FALCON_PTR(pDatapump)->FFTB.FFTCTL = (fft_control);  \
   FALCON_REG_WRITE(pDatapump, FFTB, FFTCTL);     \
   FALCON_PTR(pDatapump)->SEQBLK.SEQCOMP.FFTBCOMP_START = 0;                 \
   FALCON_REG_WRITE(pDatapump, SEQBLK, SEQCOMP);     \
   FALCON_PTR(pDatapump)->SEQBLK.SEQCOMP.FFTBCOMP_START = 1;                 \
   FALCON_REG_WRITE(pDatapump, SEQBLK, SEQCOMP);     \
}


#define ASIC_CLR_CMPLX_MULT_DONE(pDatapump) ASIC_CLR_INTR(pDatapump, FFTBCOMP_RDY)

#define ASIC_START_ATE_PWR_AVG(pDatapump)                \
{                                               \
   FALCON_PTR(pDatapump)->SEQBLK.SEQCOMP.ATEPOW_START = 0;                    \
   FALCON_REG_WRITE(pDatapump, SEQBLK, SEQCOMP);     \
   FALCON_PTR(pDatapump)->SEQBLK.SEQCOMP.ATEPOW_START = 1;                    \
   FALCON_REG_WRITE(pDatapump, SEQBLK, SEQCOMP);     \
}

// request FALCON to perform pilot tone pwr averaging, it will take 256 sym
// to finish and set QAM_RSR.POW_RDY accordingly.
#define ASIC_START_PILOT_PWR_AVG(pDatapump)              \
{                                               \
   FALCON_PTR(pDatapump)->SEQBLK.SEQCOMP.QEDPOW_START = 0;                    \
   FALCON_REG_WRITE(pDatapump, SEQBLK, SEQCOMP);     \
   FALCON_PTR(pDatapump)->SEQBLK.SEQCOMP.QEDPOW_START = 1;                    \
   FALCON_REG_WRITE(pDatapump, SEQBLK, SEQCOMP);     \
}


/**************************************************************************
     Macro calls for DAC Interface (DACINTF) related functionality
**************************************************************************/
// a low to high transition on one of the fields will move a corresponding
// pointer in DAC fifo RAM (refer to FALCON user guide for detail)
#define ASIC_ADJUST_FIFO(pDatapump, x)                 \
{                                      \
   FALCON_PTR(pDatapump)->DACINTF.FIFOCTL.common.x = 0;          \
   FALCON_REG_WRITE(pDatapump, DACINTF, FIFOCTL);        \
   FALCON_PTR(pDatapump)->DACINTF.FIFOCTL.common.x = 1;          \
   FALCON_REG_WRITE(pDatapump, DACINTF, FIFOCTL);        \
}

/**************************************************************************
     Macro calls for Digital Interface (DIBLK) related functionality
**************************************************************************/

#define ASIC_ENABLE_DI(pDatapump, x)                         \
{                                            \
   FALCON_PTR(pDatapump)->DIBLK.DICCTLL.x = NORMALMODE;       \
   FALCON_REG_WRITE(pDatapump, DIBLK, DICCTLL);              \
}

#define ASIC_BYPASS_DI(pDatapump, x)                         \
{                                            \
   FALCON_PTR(pDatapump)->DIBLK.DICCTLL.x = BYPASSMODE;       \
   FALCON_REG_WRITE(pDatapump, DIBLK, DICCTLL);              \
}

#define ASIC_SET_16K_INTER_2K_DEINTER(pDatapump)           \
{                                            \
   FALCON_PTR(pDatapump)->DIBLK.DICCTLL.INTER_SELECT = 0;     \
   FALCON_REG_WRITE(pDatapump, DIBLK, DICCTLL);              \
}

#define ASIC_SET_2K_INTER_16K_DEINTER(pDatapump)           \
{                                            \
   FALCON_PTR(pDatapump)->DIBLK.DICCTLL.INTER_SELECT = 1;     \
   FALCON_REG_WRITE(pDatapump, DIBLK, DICCTLL);              \
}

#define ASIC_SET_TCRC_GEN(pDatapump, tap)                    \
{                                            \
   FALCON_PTR(pDatapump)->DIBLK.TCRC_GEN.tap = 1;             \
   FALCON_REG_WRITE(pDatapump, DIBLK, TCRC_GEN);            \
}

#define ASIC_CLR_TCRC_GEN(pDatapump, tap)                    \
{                                            \
   FALCON_PTR(pDatapump)->DIBLK.TCRC_GEN.tap = 0;             \
   FALCON_REG_WRITE(pDatapump, DIBLK, TCRC_GEN);            \
}



#define ASIC_SET_RCRC_GEN(pDatapump, tap)                    \
{                                            \
   FALCON_PTR(pDatapump)->DIBLK.RCRC_GEN.tap = 1;             \
   FALCON_REG_WRITE(pDatapump, DIBLK, RCRC_GEN);            \
}

#define ASIC_CLR_RCRC_GEN(pDatapump, tap)                    \
{                                            \
   FALCON_PTR(pDatapump)->DIBLK.RCRC_GEN.tap = 0;             \
   FALCON_REG_WRITE(pDatapump, DIBLK, RCRC_GEN);            \
}



#define ASIC_READ_AOC(pDatapump, read_aoc)      \
{                                                \
    FALCON_REG_READ(pDatapump, DIBLK, RAOC); \
    read_aoc = (UINT8)(FALCON_PTR(pDatapump)->DIBLK.RAOC); \
}

#define ASIC_WRITE_AOC(pDatapump, x)      \
{                                          \
   FALCON_PTR(pDatapump)->DIBLK.TAOC = x;  \
   FALCON_REG_WRITE(pDatapump, DIBLK, TAOC); \
}



#define ASIC_RESET_FIFO_BANK(pDatapump, x)                  \
{                                          \
   FALCON_PTR(pDatapump)->DIBLK.RESET_CTL.x = 0;            \
   FALCON_REG_WRITE(pDatapump, DIBLK, RESET_CTL);        \
   FALCON_PTR(pDatapump)->DIBLK.RESET_CTL.x = 1;            \
   FALCON_REG_WRITE(pDatapump, DIBLK, RESET_CTL);        \
}


#define ASIC_ENABLE_ASLS_LOOPBACK(pDatapump, x)                \
{                                             \
   FALCON_PTR(pDatapump)->DIBLK->LOOP.x = 1;                         \
   FALCON_REG_WRITE(pDatapump, DIBLK, LOOP);                     \
}

#define ASIC_ENABLE_AOC_IDLE_FILTER(pDatapump)  \
{                                   \
   FALCON_PTR(pDatapump)->DIBLK.ASLSCTL.RAOC_IDLE = 1;      \
   FALCON_REG_WRITE(pDatapump, DIBLK, ASLSCTL);            \
}

#define ASIC_DISABLE_AOC_IDLE_FILTER(pDatapump) \
{                                   \
   FALCON_PTR(pDatapump)->DIBLK.ASLSCTL.RAOC_IDLE = 0;      \
   FALCON_REG_WRITE(pDatapump, DIBLK, ASLSCTL);           \
}



#define ASIC_UPDATE_FEQC(pDatapump, x)                       \
{                                            \
   FALCON_PTR(pDatapump)->QEDFER.FEQCTL.DATA_SYNC = SYNC_ONLY; \
   FALCON_PTR(pDatapump)->QEDFER.FEQCTL.AVERAGE = (x);         \
   FALCON_REG_WRITE(pDatapump, QEDFER, FEQCTL);                \
}

/*  look out : no more sync sym update on normal operation */
#define ASIC_ENABLE_GLOBAL_FEQ_ADAPTATION(pDatapump)         \
{                                            \
   FALCON_PTR(pDatapump)->QEDFER.FEQCTL.FREEZE_FEQ = 0;        \
   FALCON_PTR(pDatapump)->QEDFER.FEQCTL.FREEZE_SYNC = 1;       \
   FALCON_REG_WRITE(pDatapump, QEDFER, FEQCTL);               \
}

// This macro is for both setting TEQ and FEQ adaptation
#define ASIC_SET_ADAPTATION_BETA(pDatapump, beta)              \
{                                            \
   FALCON_PTR(pDatapump)->QEDFER.FEQCTL.BETA = (beta);         \
   FALCON_REG_WRITE(pDatapump, QEDFER, FEQCTL);                \
}

#define ASIC_DISABLE_GLOBAL_FEQ_ADAPTATION(pDatapump)        \
{                                            \
   FALCON_PTR(pDatapump)->QEDFER.FEQCTL.FREEZE_FEQ = 1;        \
   FALCON_PTR(pDatapump)->QEDFER.FEQCTL.FREEZE_SYNC = 1;       \
   FALCON_REG_WRITE(pDatapump, QEDFER, FEQCTL);                \
}

#define ASIC_START_TONE_DETECTION(pDatapump, addr)       ASIC_GetAvgSigPwr(pDatapump, addr, AVG_16SYM, TONE_DETECTION)

#define ASIC_START_GHS_TONE_DETECTION(pDatapump, addr)       ASIC_GetAvgSigPwr(pDatapump, addr, AVG_32SYM, GHS_TONE_DETECTION)

#define ASIC_START_SIG_DETECTION(pDatapump, sig_pwr)      ASIC_GetAvgSigPwr(pDatapump, &(sig_pwr), AVG_1SYM, FAST_SIG_DETECT)

#define ASIC_IS_TONE_DETECT_DONE(pDatapump) ASIC_IsPowerAverageDone(pDatapump)

#define ASIC_IS_SIG_DETECT_DONE(pDatapump)  ASIC_IsPowerAverageDone(pDatapump)

#define ASIC_WAIT_TONE_DETECT_DONE(pDatapump) ASIC_WaitPowerAverageDone(pDatapump)

#define ASIC_WAIT_SIG_DETECT_DONE(pDatapump)  ASIC_WaitPowerAverageDone(pDatapump)

// Perform warm restart on the DI block
// Note: for some FALCON problems, we need to toggle the bit many times to guarantee warm reset
#define    ASIC_FALCON_WARM_RESET(pDatapump)                                   \
{                                                                               \
    UINT8 index;                 \
    for (index = 0 ; index < 10 ; ++index)                 \
    {                                                      \
        FALCON_PTR(pDatapump)->DIBLK.RESET_CTL.W_RESET = 0;                          \
        FALCON_REG_WRITE(pDatapump, DIBLK, RESET_CTL);   \
        FALCON_PTR(pDatapump)->DIBLK.RESET_CTL.W_RESET = 1;                          \
        FALCON_REG_WRITE(pDatapump, DIBLK, RESET_CTL);   \
    }                                                             \
    FALCON_PTR(pDatapump)->DIBLK.RESET_CTL.W_RESET = 0;                          \
    FALCON_REG_WRITE(pDatapump, DIBLK, RESET_CTL);   \
}

/**************************************************************************
     Macro calls for Timer block Interface (TIMERBLK) related functionality
**************************************************************************/

// low to high transition on the TIMER_START bit will start the timer
#define ASIC_START_TIMER(xcvr_index, which_timer)                  \
{                                             \
    FALCON_PTR(xcvr_index)->TIMERBLK.which_timer.TIMER_START = 0;   \
    FALCON_REG_WRITE(xcvr_index, TIMERBLK, which_timer);   \
    FALCON_PTR(xcvr_index)->TIMERBLK.which_timer.TIMER_START = 1;   \
    FALCON_REG_WRITE(xcvr_index, TIMERBLK, which_timer);   \
}


/**************************************************************************
     Macro calls for Micro-Controller Interface (MIFBLK) related functionality
**************************************************************************/

// By writing and reading to the MIF address will initiate the burst write and
// read correspondingly.
#define ASIC_START_BURST_WRITE(pDatapump, x)                  \
{                                             \
    FALCON_OFFSET(pDatapump, x)=0;\
}

#define ASIC_START_BURST_READ(pDatapump, x)                   \
{                                             \
    FALCON_OFFSET(pDatapump, x);  \
}

#define MAX_QLEN           10


typedef enum BIT_SWAP_MODE {NULL_BIT_SWAP, TX_BIT_SWAP, RX_BIT_SWAP, TXRX_BIT_SWAP} BIT_SWAP_MODE;

typedef struct
{
    BIT_SWAP_MSG_TYPE bit_swap_req[6];   // maximum of 6 only for ext bit swap req
    UINT8 size;
}BIT_SWAP_REQ_TYPE;

typedef struct
{
    FFTCTL_TYPE  control_setting;   // One of element in fft_control[]
    UINT16       length;
    void         *buffer;
    UINT8        operation;         // one of FFTBREQ_OPERATION
}FFTB_REQ_TYPE;

typedef struct
{
    UINT8    current_operation;
    UINT8    MIF_dest_format;
    void     *dest_addr;
}AVG_PWR_REQ_TYPE;

typedef struct
{
    void     *dest_addr;
    BOOLEAN  is_tone_detection;
}AVG_SIG_REQ_TYPE;

typedef struct
{
    GEN_UINT   xfrmode;               // Defined in XFR_MODE_TYPE
    GEN_UINT   format ;
    GEN_UINT   MIF_src_addr;
    GEN_UINT   MIF_dest_addr;
    GEN_UINT   xfrlen;
    void     *buffaddr;
}MIF_XFER_REQ_TYPE;


typedef struct
{
    GEN_UINT       teq_win_start, fir_win_start;
    GEN_UINT       teq_win_size, fir_win_size;
    UINT32       ref_norm;
    SINT16CMPLX  pilot_phase;
    POW_ADJ_TYPE fir_pow_adj_factor;
}FTEQ_SETUP_TYPE;

typedef struct
{
    #ifdef ECHO_TRAINING
    UINT8 echo_cancel_Beta[2];//={6,7};
    #endif
    volatile LOCK_TYPE mif_lock;

    volatile LOCK_TYPE fftb_lock;

    volatile LOCK_TYPE qedfer_algo_lock;

    volatile LOCK_TYPE tone_avg_lock;
    FTEQ_SETUP_TYPE fteq_setup;
    FIFO_TYPE xfr_reqq;
    MIF_XFER_REQ_TYPE xfr_req_buffer[MAX_QLEN], current_req;
    BIT_SWAP_MODE bit_swap_mode;
    AVG_PWR_REQ_TYPE pwravg_req;
    AVG_SIG_REQ_TYPE sigavg_req;
    FFTB_REQ_TYPE current_fftb_req;
    #ifdef FAST_PATH_FIX
    UINT8 data_mode;
    #endif
    BIT_SWAP_REQ_TYPE tx_bit_swap_req, rx_bit_swap_req;
    BOOLEAN globespan_eoc_mode;
}LOCAL_ASICUTIL_TYPE;


DPCCALL BOOLEAN ASIC_IsFDET(PDATAPUMP_CONTEXT pDatapump);
DPCCALL BOOLEAN ASIC_IsSignalAverageDone(PDATAPUMP_CONTEXT);
DPCCALL BOOLEAN ASIC_IsPowerAverageDone(PDATAPUMP_CONTEXT);
DPCCALL BOOLEAN ASIC_IsErrPwrAverageDone(PDATAPUMP_CONTEXT);
DPCCALL BOOLEAN ASIC_IsBurstDone(PDATAPUMP_CONTEXT);
DPCCALL BOOLEAN ASIC_IsFFTB_Done(PDATAPUMP_CONTEXT);
DPCCALL BOOLEAN ASIC_WaitSignalAverageDone(PDATAPUMP_CONTEXT);
DPCCALL BOOLEAN ASIC_WaitPowerAverageDone(PDATAPUMP_CONTEXT);
DPCCALL BOOLEAN ASIC_WaitErrPwrAverageDone(PDATAPUMP_CONTEXT);
DPCCALL BOOLEAN ASIC_WaitBurstDone(PDATAPUMP_CONTEXT);
DPCCALL BOOLEAN ASIC_WaitFFTB_Done(PDATAPUMP_CONTEXT);

DPCCALL void    ASIC_TSYNC_IntrHandler(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    ASIC_RSYNC_IntrHandler(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    ASIC_BurstDoneIntrHandler(PDATAPUMP_CONTEXT);
DPCCALL void    ASIC_BurstIntXfr(PDATAPUMP_CONTEXT, GEN_UINT, GEN_UINT);
DPCCALL void    ASIC_BurstWrite(PDATAPUMP_CONTEXT, GEN_UINT, const void *, GEN_UINT, GEN_UINT);
DPCCALL void    ASIC_BurstRead(PDATAPUMP_CONTEXT, GEN_UINT MIF_src_addr, void *dest_addr, GEN_UINT length, GEN_UINT xfrformat);

DPCCALL UINT8   ASIC_ScheduleTxBitSwap(PDATAPUMP_CONTEXT);
DPCCALL void    ASIC_ScheduleRxBitSwap(PDATAPUMP_CONTEXT, UINT8);
DPCCALL void    ASIC_RequestTxBitSwap(PDATAPUMP_CONTEXT, const AOC_MSG_TYPE *, UINT8);
DPCCALL void    ASIC_RequestRxBitSwap(PDATAPUMP_CONTEXT, const AOC_MSG_TYPE *, UINT8);


DPCCALL void    ASIC_TxClkIntrHandler(PDATAPUMP_CONTEXT);
DPCCALL void    ASIC_RxClkIntrHandler(PDATAPUMP_CONTEXT);
DPCCALL void    ASIC_FFTB_CompRdyIntrHandler(PDATAPUMP_CONTEXT);
DPCCALL void    ASIC_CompRdyIntrHandler(PDATAPUMP_CONTEXT);
DPCCALL void    ASIC_ToneRdyIntrHandler(PDATAPUMP_CONTEXT);
DPCCALL void    ASIC_GetAvgSig(PDATAPUMP_CONTEXT, void *, AVERAGE_CHOICE symbols, BOOLEAN);
DPCCALL void    ASIC_GetAvgErrPwr(PDATAPUMP_CONTEXT, void *, AVERAGE_CHOICE symbols, enum PWR_AVG_REQ_CHOICE mode);
DPCCALL void    ASIC_GetAvgSigPwr(PDATAPUMP_CONTEXT, void *, AVERAGE_CHOICE symbols, enum XFR_FORMAT mif_format);
DPCCALL void    ASIC_CmplxMult(PDATAPUMP_CONTEXT, const SINT32CMPLX *,const SINT16CMPLX *, SINT16CMPLX *, UINT16, BOOLEAN);
DPCCALL void    ASIC_FastFourierTransform(PDATAPUMP_CONTEXT, SINT16 *, SINT16CMPLX *, FFTCTL_TYPE);
DPCCALL void    ASIC_InvFastFourierTransform(PDATAPUMP_CONTEXT, SINT16CMPLX *, SINT16 *, FFTCTL_TYPE);
DPCCALL void    ASIC_ResetSysConfig(PDATAPUMP_CONTEXT, void *);
DPCCALL BOOLEAN ASIC_FteqTrainingStart(PDATAPUMP_CONTEXT, SINT16 *);
DPCCALL BOOLEAN ASIC_FteqTrainingStop(PDATAPUMP_CONTEXT, SINT16 *);
DPCCALL BOOLEAN ASIC_FteqTrainingProcess(PDATAPUMP_CONTEXT, SINT16 *, UINT8, UINT8, UINT8, BOOLEAN);
DPCCALL BOOLEAN ASIC_Config_ASX_LSX(PDATAPUMP_CONTEXT, const C_RATES_OPTION_TYPE *, C_RATES_OPTION_TYPE *, UINT8);
DPCCALL void    ASIC_GenerateR_RATES(PDATAPUMP_CONTEXT, R_RATES_OPTION_TYPE *, C_RATES_OPTION_TYPE *);
DPCCALL void    ASIC_SetEocProtocol(PDATAPUMP_CONTEXT, BOOLEAN);
DPCCALL BOOLEAN ASIC_GetEocProtocol(PDATAPUMP_CONTEXT);
DPCCALL void    ASIC_TxEocIntrHandler(PDATAPUMP_CONTEXT);
DPCCALL void    ASIC_RxEocIntrHandler(PDATAPUMP_CONTEXT);
DPCCALL void    ASIC_InitXfrReqQ(PDATAPUMP_CONTEXT);
DPCCALL void    ASIC_SetupTeqShadowRam(PDATAPUMP_CONTEXT, UINT8, SINT16 *);
DPCCALL void    ASIC_CancelAveragingOperation(PDATAPUMP_CONTEXT);
DPCCALL void    ASIC_SendEOCByte(PDATAPUMP_CONTEXT pDatapump, UINT8 outbyte);
DPCCALL GEN_UINT ASIC_GetDataMode(PDATAPUMP_CONTEXT pDatapump, GEN_UINT direction, GEN_UINT data_path);
DPCCALL GEN_UINT ASIC_GetInterleaveDepth(PDATAPUMP_CONTEXT pDatapump, GEN_UINT direction, GEN_UINT data_path);

DPCCALL GEN_UINT  ASIC_Windowing(const SINT16 *buffer, GEN_UINT size, GEN_UINT windowsize);

DPCCALL SINT16  ASIC_MeasureISI(PDATAPUMP_CONTEXT, SINT16 *);

DPCCALL UINT16  ASIC_ComputeCodeWord(
                         GEN_UINT AS_payload,
                         GEN_UINT LS_payload,
                         GEN_UINT overhead_mode);

DPCCALL void ASIC_SetGlobespanEocMode(PDATAPUMP_CONTEXT pDatapump, BOOLEAN mode);
DPCCALL void ASIC_RxEocIntrHandler(PDATAPUMP_CONTEXT pDatapump);


#endif
/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/asicutil.h $
 * 
 * 20    7/10/01 12:55p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 * 
 * 19    7/03/01 4:39p Lewisrc
 * Adapting Multi-instance Datapump code to Linux
 * 
 * Change "struct AVERAGE_CHOICE" to "AVERAGE_CHOICE" since GNU C doesn't
 * allow this.
 * 
 * 18    6/26/01 7:40p Lauds
 * multi-instance version integration
 * 
 * 2     6/22/01 3:06p Lauds
 * minor cleanup and bug fix in miscutil.c and terminal.c
 * 
 * 17    5/30/01 5:27p Lauds
 * add some explicit casting in some of the macros to get rid some
 * warnings.
 * 
 * 16    4/27/01 7:41p Lauds
 * add macro to read loop timing low pass coefficients
 * 
 * 15    4/23/01 7:15p Lauds
 * deleted one line by mistake
 * 
 * 14    4/16/01 7:53p Lauds
 * Use of Trapper timer for MIC_GetCurrentTick and MIC_ElapsedMsec
 * 
 * 13    3/19/01 5:08p Lauds
 * check for after Falcon 2PT before FORCE_TX_STATE_NOW
 * 
 * 12    2/06/01 6:39p Lauds
 * Accomodate behavior of TCR queued write in Trapper
 * 
 * 11    2/05/01 7:48p Lauds
 * Integrate Trapper changes from Harry Lau
 * 
 * 10    1/09/01 10:08a Lewisrc
 * Wirespeed/Redhat had to make minor changes to DMT files to compile
 * under GNU
 * 
 * Fix Unbalanced comment on each FALCON_REG_WRITE
 * 
 * 9     1/05/01 7:40p Lauds
 * move showtime snr margin measurement from ISR to background task
 * 
 * 8     9/26/00 6:34p Lauds
 * Code cleanup
 * 1. change local var to GEN_UINT and GEN_SINT when possible
 * 2. change xcvr_num to XCVR_INDEX_TYPE
 * 3. Get rid of all Falcon reg access // #lcon
 * 
 * 7     4/03/00 5:49p Lauds
 * New Falcon Access Wrapper for ARM compiler
 * 
 * 6     3/08/00 2:03p Lewisrc
 * modifiy STARTUP_TRACE_ROUTINE to call new DIAG_AddResetCount
 * 
 * 5     11/09/99 9:30a Lewisrc
 * 2.02f code release from Pairgain
 * 
 *    Rev 1.84   Oct 26 1999 16:30:24   yangl
 * -- Stop Support Falcon 1Z
 * -- Dynamic config with Falcon2 & 2P
 * 
 * 
 *    Rev 1.83   Oct 26 1999 16:12:28   yangl
 *  
 *
 *    Rev 1.82   Jul 29 1999 15:53:32   InfusiJ
 * Initial Fast Retrain merge.
 * FR is enabled ONLY with G.hs
 * FR is in a Debug state
 * Escape to FR is not functional
 *
 *    Rev 1.81   Apr 30 1999 16:31:58   nordees
 * Moved PRBS_TYPE from miscutil.c for Echo Cancellation.
 *
 *    Rev 1.80   Apr 21 1999 18:22:34   InfusiJ
 * -added G.hs -> G.lite(full retrain)
 * -improved G.hs -> G.dmt support
 * -ghs tone detectors are now 32 bit
 * -ghs states tested for Belgium interop.
 *
 *    Rev 1.79   Mar 16 1999 11:00:26   nordees
 * DSLAM cleanup.
 *
 *    Rev 1.78   Mar 10 1999 16:19:10   laud
 * allow ATU-C to talk to either G.Lite and G.DMT modem based on multiple tone detection (i.e. R_ACK3)
 *
 *    Rev 1.77   Jan 28 1999 19:17:58   laud
 * get rid of 1Z compiler switch
 *
 *    Rev 1.76   Jan 27 1999 20:32:10   laud
 * check both primary and backup tones in 8 bit/sym
 *
 *    Rev 1.75   Dec 08 1998 15:41:56   laud
 * changes and bugs fixed according to UAWG in Dec
 *
 *    Rev 1.74   Nov 09 1998 17:33:54   laud
 * code space optimization for single xcvr
 *
 *    Rev 1.73   Nov 06 1998 18:17:18   laud
 * support rx down no oversampling
 *
 *    Rev 1.72   Oct 30 1998 15:48:18   laud
 * optimize all macros for single xcvr
 *
 *    Rev 1.71   Sep 18 1998 18:56:10   laud
 * limit WIN_START within 128 for upstream (CO)
 *
 *    Rev 1.70   Sep 02 1998 19:29:28   laud
 * FALCON 1Z fixes
 *
 *    Rev 1.69   Aug 28 1998 14:47:18   laud
 * ATEBLK support for FALCON 1Z
 *
 *    Rev 1.68   Jul 01 1998 15:26:04   laud
 * change ASIC_IS_FDET to function call
 *
 *    Rev 1.67   Jun 30 1998 14:27:26   laud
 * avoid byte access for FALCON reg to workaround MIPS compiler bug
 *
 *    Rev 1.66   Jun 26 1998 19:11:50   laud
 * no more access FALCON more than 16-bit at a time
 *
 *    Rev 1.65   15 May 1998 17:05:46   laud
 * DSLAM integration again
 *
 *    Rev 1.64   30 Apr 1998 14:10:02   laud
 * remove blank line that will cause problem later
 *
 *    Rev 1.63   27 Apr 1998 12:38:44   laud
 * DSLAM integration
 *
 *    Rev 1.62   27 Mar 1998 16:29:14   laud
 * toggle warm reset bit many times for warm-reset
 *
 *    Rev 1.61   16 Mar 1998 17:18:16   laud
 * warm start changes
 *
 *    Rev 1.60   18 Feb 1998 18:29:08   laud
 * use FFTA ram for gain source after R_QUIET3
 *
 *    Rev 1.59   03 Feb 1998 19:13:04   laud
 * tone detection with sig avg
 *
 *    Rev 1.58   29 Jan 1998 19:03:04   laud
 * new tone detection algorithm
 *
 *    Rev 1.57   23 Jan 1998 18:19:58   laud
 * modify FEQ adaptation updates to include sync symbol
 *
 *    Rev 1.56   23 Jan 1998 14:41:34   laud
 * get rid FALCON 1A reference
 *
 *    Rev 1.55   03 Dec 1997 16:56:14   laud
 * falcon 1a changes
 *
 *    Rev 1.0   24 Jan 1996 11:47:06   DLAU
 * Initial revision.
*
*****************************************************************************/
