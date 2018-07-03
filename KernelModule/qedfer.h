/************************************************************************
*  $Workfile: qedfer.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/qedfer.h 17    8/16/01 5:58p Lauds $
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
      the QAM Encoder/Decoder and FEQ (QEDFER) block of the FALCON.
*************************************************************************/
#ifndef QEDFER_H
#define QEDFER_H

#include "types.h"
#include "sysinfo.h"


enum MSG_BANK_CHOICE{MSG_BANK0 ,MSG_BANK1};

/**************************************************************************
    QEDFER Control Latch   (Write Only)
**************************************************************************/



enum DATA_SYNC_CHOICE   {SYNC_ONLY, DATA_ONLY};
enum POLY_CHOICE        {TAP5_6GEN, TAP4_9GEN};
enum PWR_MEAS_CHOICE    {ERR_PWR_MEAS, RCVR_PWR_MEAS};
enum TPILOT_CHOICE      {TPILOT_17, TPILOT_16, TPILOT_64, TPILOT_96};
enum FMESS_CHOICE       {SUBCHNL_6_10,SUBCHNL_37_43,SUBCHNL_77_83,SUBCHNL_97_103};

typedef volatile union
{
    struct
    {
        UINT16 NTR             :2;
        UINT16 NRC             :2;
        UINT16 TPRBS_GEN       :1;
        UINT16 RPRBS_GEN       :1;
        UINT16 ALGO_SEL        :1;
        UINT16 DATA_SYNC       :1;
        UINT16 TPILOT_DIS      :1;
        UINT16 RPILOT_DIS      :1;
        UINT16 RATE_SWAP       :1;
        UINT16 TX_RAM_PAGE     :1;
        UINT16 RX_RAM_PAGE_H   :1;
        UINT16 RGC_SEL         :1;
        UINT16                 :2;
    }common;
    struct
    {
        UINT16                 :14;
        UINT16 NCO_DIS         :1;
        UINT16 PILOT_WB        :1;
    }trapper;
    struct
    {
        UINT16                 :14;
        UINT16 NCO_DIS         :1;
        UINT16 PPILOT_DUP      :1;
    }falcon2p;
    struct
    {
        UINT16                 :14;
        UINT16 RPILOT          :2;
    }falcon2;
}QEDCTLL_TYPE;


// This ONLY is common default
#define QEDCTLL_DEFAULT       \
        {ASIC_CONVERT_CHNL_SIZE(NUM_TX_CHNL),            \
         ASIC_CONVERT_CHNL_SIZE(QEDFER_RX_CHNL),         \
         TPRBS_GEN_POLYNOMIAL,           \
         RPRBS_GEN_POLYNOMIAL,           \
         RCVR_PWR_MEAS,        \
         SYNC_ONLY,           \
         FALSE,                \
         FALSE,                \
         0,  \
         0,  \
         0,  \
         0}

typedef volatile struct
{
   UINT16 TEQ_START      :8;
   UINT16 TEQ_END        :8;
}QEDCTLH_TYPE;

#define QEDCTLH_DEFAULT    {DEFAULT_VALID_RX_CHNL_START, DEFAULT_VALID_RX_CHNL_END}


/**************************************************************************
     Mode Control Register   (Write Only)
**************************************************************************/



typedef volatile struct
{
   UINT16 TX_OTHER        :1;
   UINT16 RX_OTHER        :1;
   UINT16 SFSEARCH        :1;
   // the following is not available in FALCON 1
   UINT16 OVERSAMPLE_EN   :1;   // 0: use NCO mode
                                // 1: developing oversampling DAC scheme
   UINT16 TEST_DEC_TXSYM  :1;   // if high decrement TXSYM_CNT once per MCLK
   UINT16 TEST_INC_TFRAME :1;   // if high increment TFRAME_CNT once per MCLK
   UINT16 FEQDYNR_MCR     :1;   // 0: FEQ coef update as in FALCON 1
                                // 1: disable DGC gain shift when FEQ coef overflow
   UINT16 TXO_NOW         :1;   // low to high transition forces BPQE to go
                                // immediately to the alternate state (test only)
   UINT16 TEST_DEC_RXSYM  :1;   // if high decrement RXSYM_CNT once per MCLK
   UINT16 TEST_INC_RFRAME :1;   // if high increment RFRAME_CNT once per MCLK
   UINT16 TEST_INC_NPOW   :1;   // if high increment NPOW_CNT once per MCLK
   UINT16 TEST_INC_TONE   :1;   // if high increment TONE_CNT once per MCLK
   UINT16 TEST_INC_PPOW   :1;   // if high increment PPOW_CNT once per MCLK
   UINT16 SAT_PE          :1;   // 0: no saturation will be performed
                                // 1: saturate Phase Error value (PE)
   UINT16 SEL_PE          :1;   // if high flip the sign of the phase error
                                // when the imag part of phase error is negative
                                // otherwise the phase error remain unchanged
   UINT16 RXO_NOW         :1;   // low to high transition forces QDBP to go
                                // immediately to the alternate state (test only)
}QAM_MCR_TYPE;


#define QAM_MCR_DEFAULT             \
{                                   \
    0,      /* TX_OTHER       */    \
    0,      /* RX_OTHER       */    \
    0,      /* SFSEARCH       */    \
    0,      /* OVERSAMPLE_EN  */    \
    0,      /* TEST_DEC_TXSYM */    \
    0,      /* TEST_INC_TFRAME*/    \
    0,      /* FEQDYNR_MCR    */    \
    0,      /* TXO_NOW        */    \
    0,      /* TEST_DEC_RXSYM */    \
    0,      /* TEST_INC_RFRAME*/    \
    0,      /* TEST_INC_NPOW  */    \
    0,      /* TEST_INC_TONE  */    \
    0,      /* TEST_INC_PPOW  */    \
    0,      /* SAT_PE         */    \
    0,      /* SEL_PE         */    \
    0,      /* RXO_NOW        */    \
}



/**************************************************************************
     Interrupt Control Latch   (Write Only)
**************************************************************************/



enum ICR_CHOICE {FALCON_INTR_ENABLE, FALCON_INTR_DISABLE};

typedef volatile struct
{
   UINT16 COMP_RDY           :1;
   UINT16 TONE_RDY           :1;
   UINT16 TSTATE_INT         :1;
   UINT16 TSYNC_INT          :1;
   UINT16 RSTATE_INT         :1;
   UINT16 RSYNC_INT          :1;
   UINT16 TXCLK_INT          :1;
   UINT16 RXCLK_INT          :1;
   UINT16 TEOC_INT           :1;
   UINT16 TAOC_INT           :1;
   UINT16 BURSTDONE_INT      :1;
   UINT16 PRBSDET            :1;  /* FALCON 2 only */
   UINT16 FFTBCOMP_RDY       :1;
   UINT16 TXFIR_INT          :1;
   UINT16 EOC_INT            :1;
   UINT16 AOC_INT            :1;
}QEDFER_FALCON_TYPE;

typedef QEDFER_FALCON_TYPE QAM_ICR_TYPE;


#define QAM_ICR_DEFAULT {FALCON_INTR_DISABLE,     /* COMP_RDY      */ \
                         FALCON_INTR_DISABLE,     /* TONE_RDY      */ \
                         FALCON_INTR_DISABLE,     /* TSTATE_INT    */ \
                         FALCON_INTR_DISABLE,     /* TSYNC_INT     */ \
                         FALCON_INTR_DISABLE,     /* RSTATE_INT    */ \
                         FALCON_INTR_DISABLE,     /* RSYNC_INT     */ \
                         FALCON_INTR_DISABLE,     /* TXCLK_INT     */ \
                         FALCON_INTR_DISABLE,     /* RXCLK_INT     */ \
                         FALCON_INTR_DISABLE,     /* TEOC_INT or IFFTASAT_INT  */ \
                         FALCON_INTR_DISABLE,     /* TAOC_INT or FFTASAT_INT   */ \
                         FALCON_INTR_DISABLE,     /* BURSTDONE_INT */ \
                         FALCON_INTR_DISABLE,     /* PRBSDET (FALCON 2 only ) */ \
                         FALCON_INTR_DISABLE,     /* FFTBCOMP_RDY  */ \
                         FALCON_INTR_DISABLE,     /* TXFIR_INT     */ \
                         FALCON_INTR_DISABLE,     /* EOC_INT       */ \
                         FALCON_INTR_DISABLE}     /* AOC_INT       */

/****************************/

typedef volatile struct
{
   UINT16 PRBS_DET          :1;     /* Trapper */
   UINT16                   :15;
}QEDFER_TRAPPER_TYPE;

typedef QEDFER_TRAPPER_TYPE QAM_ICR2_TYPE;

#define QAM_ICR2_DEFAULT {FALCON_INTR_DISABLE,     /* CLR_PRBSDET      */ \
                          FALCON_INTR_DISABLE}


/**************************************************************************
     Transmit Control Register   (Write Only)
**************************************************************************/


enum TX_MODE_CHOICE {TX_NORMAL = 0,
                     TX_ZERO_QAM,
                     TX_TONE,
                     TX_4QAM,
                     TX_8BIT_ENC,
                     TX_TEST_SYNC,
                     TX_TEST_TCM,
                     TX_NORMAL_TCM};
#define TX_SYM_HOLD            0

enum {PRBS_RESET=0, PRBS_NORESET};  // for periodic or non-periodic
                                    // pseudo-random signal

enum {QAM_USE_FINE_GAIN_RAM,QAM_USE_TADJUST1,QAM_USE_TADJUST2=3};

typedef volatile union
{
   struct
   {
      UINT16 TX_MODE           :3;
      UINT16 TCP_EN            :1;
      UINT16 TONE_ADDR         :8;
      UINT16 TMSG_BANK         :1;
      UINT16 TADJUST           :2;
   }bvalue1;  // This is used when TX_MODE = TX_TONE
   struct
   {
      UINT16 TX_MODE           :3;
      UINT16 TCP_EN            :1;
      UINT16 RESET_PRBS        :1;
      UINT16 CNT_EN            :1;
      UINT16 INV_ENC           :1;
      UINT16 REV_XY            :1;
      UINT16 /* not used */    :4;
      UINT16 TMSG_BANK         :1;
      UINT16 TADJUST           :2;
   }bvalue2;  // This is used when TX_MODE = TX_NORMAL, TX_ZERO_QAM, TX_4QAM,
              // TX_8BIT_ENC, and TX_TEST_SYNC
}QAM_TCRL_TYPE;

// note: The FFT block will use the QAM_TCRL.TADJUST to decide to use TADJUST1,
// TADJUST2, or FFTA GAIN RAM for tx gain (except TX_MODE = TX_NORMAL) if
// FFTCTL.TGAIN is set to USE_2BIT_STATUS.  This is the recommended method
// to control tx gain factor


#define QAM_TCRL_DEFAULT      {{TX_TONE,0,TX_PILOT_TONE,0, QAM_USE_TADJUST1}}




typedef volatile struct
{
   UINT16 SYMBOLS            :12;
   UINT16 SHOWTIME           :1;
   UINT16 TX_UPD_W_RX        :1;   // Trapper only
}QAM_TCRH_TYPE;

#define QAM_TCRH_DEFAULT        {TX_SYM_HOLD, 0, 0}





/**************************************************************************
     Receive Control Register   (Write Only)
**************************************************************************/


enum {RX_NORMAL, RX_4QAM, RX_8BIT_DEC, RX_NORMAL_TCM};

#define RX_SYM_HOLD            0


typedef volatile struct
{
   UINT16 RX_MODE           :2;
   UINT16 RCP_EN            :1;
   UINT16 RESET_PRBS        :1;
   UINT16 THRESHOLD         :8;
   UINT16 START_PRBS_DET    :1;
   UINT16 CNT_EN            :1;
   UINT16 INV_DEC           :1;
   UINT16 REV_XY            :1;
}QAM_RCRL_TYPE;

#define QAM_RCRL_DEFAULT      {RX_4QAM,0,0,NUM_RX_CHNL/2,0,0,0,0}





typedef volatile struct
{
   UINT16 WAIT              :1;
   UINT16 REFERENCE         :1;
   UINT16 SHOWTIME          :1;
   UINT16 RMSG_BANK         :1;
   UINT16 SYMBOLS           :12;
}QAM_RCRH_TYPE;

#define QAM_RCRH_DEFAULT      {0,1,0,0,RX_SYM_HOLD}



/**************************************************************************
     Receive Status Latch   (Read Only)
**************************************************************************/


enum CHIP_ID_CHOICE 
{
    FALCON1=0, 
    FALCON1Z, 
    FALCON2, 
    FALCON2P, 
    TRAPPER, 
    RUSHMORE, 
    GEORGE, 
    FALCON2PT,
    RUSHMORE_UMC = 0xD,    // Trapper core in Rushmore for UMC shuttle
    NUM_FALCON_IDS 
};

typedef volatile struct
{
   UINT16 MATCH             :9;
   UINT16 FDET              :1;
   UINT16 POW_RDY           :1;
   UINT16                   :1;
   UINT16 CHIP_ID           :4;
}QAM_RSR_TYPE;


/**************************************************************************
     Digital Lowpass Linear Coefficient Register    (Write Only)
     Digital Lowpass Integral Coefficient Register  (Write Only)
**************************************************************************/




typedef volatile UINT16 C_Coef_TYPE;

#define C1_Coef_DEFAULT       0   //0xF9
#define C2_Coef_DEFAULT       0   //0x26



/**************************************************************************
     Digital Lowpass Frequency Register    (Read/Write)
**************************************************************************/


typedef volatile UINT16 STORE_LO_Reg_TYPE;

typedef volatile struct
{
   UINT16 HI_STORE           :4;
   UINT16 C1                 :4;
   UINT16 C2                 :4;
   UINT16                    :4;      // unused
}STORE_HI_Reg_TYPE;

#define STORE_REG_C1_DEFAULT       0
#define STORE_REG_C2_DEFAULT       0

/**************************************************************************
     Average Pilot-Tone Signal Power Register    (Read Only)
**************************************************************************/



typedef volatile UINT16 PT_POW_TYPE;


/**************************************************************************
     FEQCTL/Miscellaneous Control Latch    (Write Only)
    Ref: sec. 12.1.11 FEQ/Miscellaneous Control Latch:

**************************************************************************/

//  Beta coefficient is 2-bit representation of FEQ coefficient LMS adaptation
//  factor

//  For FEQ operation:
//          00: beta = 2**(-5)
//          01: beta = 2**(-8)
//          10: beta = 2**(-12)
//          11: beta = 2**(-20)

enum FEQ_BETA_CHOICE{FEQ_2NEG5,FEQ_2NEG8,FEQ_2NEG12,FEQ_2NEG20};

//  For Unwindow TEQ operation:
//          00: beta = 2**(-0)
//          01: beta = 2**(-2)
//          10: beta = 2**(-4)
//          11: beta = 2**(-6)

enum TEQ_BETA_CHOICE{TEQ_2NEG0,TEQ_2NEG2,TEQ_2NEG4,TEQ_2NEG6};

// AVERAGE is a 4-bit value representing the number of symbols in which to
//  perform averaging. which has value ranging from
//  2**(n) where n = 0 to 9 ( 1 to 512)

typedef enum {AVG_1SYM, AVG_2SYM, AVG_4SYM, AVG_8SYM, AVG_16SYM, AVG_32SYM,
                    AVG_64SYM, AVG_128SYM, AVG_256SYM, AVG_512SYM} AVERAGE_CHOICE ;



typedef volatile struct
{
   UINT16 FREEZE_FEQ    :1; // FALCON 1  0: allowed to updated, 1: all FEQ coef. are frozen
                            // FALCON 1A 0: allowed to updated in data sym only, 1: all FEQ coef. are frozen
   UINT16 FREEZE_SYNC   :1; // 0: allowed to updated in sync sym, 1: all FEQ coef. are frozen
   UINT16 BETA          :2; // This beta field is used for TEQ and FEQ training
   UINT16 AVERAGE       :4; // Representing number of symbols in which to perform
                            //  averaging
   UINT16 MONITOR       :8; // Address which specifies location of sub. chan
                            // to be monitored
}FEQCTL_TYPE;

#ifdef CO
#define FEQCTL_DEFAULT     {1, 1, 0, AVG_2SYM, RX_PILOT_TONE-1}
#else
#define RT_DEFAULT_CONST_MONITOR_CHNL  (ANNEX_MODE == ANNEX_A ? 56 : DEFAULT_VALID_RX_CHNL_START+ 10)   // Generally used for both T1.413 and G.HS
#define FEQCTL_DEFAULT     {1, 1, 0, AVG_2SYM, RT_DEFAULT_CONST_MONITOR_CHNL}
#endif




/**************************************************************************
     Transmit Subchannel Address Latch  (Write Only)
**************************************************************************/



typedef volatile struct
{
   UINT16 TSUB_START      :8;
   UINT16 TSUB_END        :8;
}TSUB_TYPE;


#define TSUB_DEFAULT   {DEFAULT_VALID_TX_CHNL_START, DEFAULT_VALID_TX_CHNL_END}



/**************************************************************************
     QEDFER Bit Swap Operands Latch  (Write Only)
**************************************************************************/


typedef volatile UINT16 RSWAP_TYPE;

#define RSWAP1_DEFAULT        0
#define RSWAP2_DEFAULT        0
#define RSWAP3_DEFAULT        0




/**************************************************************************
     QEDFER Bit Swap Address Latch  (Write Only)
**************************************************************************/



typedef volatile struct
{
   UINT16 RADDR1        :8;
   UINT16 RADDR2        :8;
}RADDRL_TYPE;

typedef volatile struct
{
   UINT16 RADDR3        :8;
   UINT16               :4;
   UINT16 D_PILOT       :4;  // alternate dynamic range gain value for
                             // rcvd pilot tone. This provides a gain (or
                             // attenuation) of 2^(D_PILOT-4) (similar to DGC
                             // rx bit tone RAM)
}RADDRH_TYPE;



#define RADDRL_DEFAULT        {0,0}    //???? testing

#define RADDRH_DEFAULT        {0,2}    //???? testing




/**************************************************************************
     QEDFER Alternate Receive Gain Adjustment Latch  (Write Only)
**************************************************************************/



typedef volatile UINT16 RADJUST_TYPE;

#define DEFAULT_RADJUST1_GAIN        0.3f
#define RADJUST1_DEFAULT     (FLT2INT16(DEFAULT_RADJUST1_GAIN)<<1)
#define RADJUST2_DEFAULT     (FLT2INT16(0.5)<<1)



/**************************************************************************
     Interrupt Source Status Latch       (Read Only)
**************************************************************************/

typedef QEDFER_FALCON_TYPE MIF_ISR_TYPE;
typedef QEDFER_TRAPPER_TYPE MIF_ISR2_TYPE;


/**************************************************************************
     Pilot-tone FEQ Coefficient Latch (Write Only)
**************************************************************************/



typedef volatile struct
{
   UINT16 F_IMAG_LO       :16;
}F_PILOT_L_TYPE;

typedef volatile struct
{
   UINT16 F_IMAG_HI       :4;
   UINT16 F_REAL_LO       :12;
}F_PILOT_M_TYPE;

typedef volatile union 
{
   struct
   {
        UINT16 F_REAL_HI       :8;
        UINT16                 :8;
   }common;
   struct
   {
        UINT16                 :8;
        UINT16 RPILOT          :8;     // Receive Pilot Tone Address
   }falcon2p;
}F_PILOT_H_TYPE;

// initialize F_PILOT to 0.5 + 0.0j 
// for simplicity, we are only going to set the 16 msb of the real and imag part
// note that there are several places that the sw is assuming the imaginary
// portion is zero to be more efficient.  If for reason a non zero imag value
// is desired, make sure make corresponding changes on those code.
#define F_PILOT_MAG  (FLT2INT16(0.5))
#define F_PILOT_L_DEFAULT      {FLT2INT16(0.000)<<4}
#define F_PILOT_M_DEFAULT      {FLT2INT16(0.000)>>12, (UINT16)( F_PILOT_MAG <<4)}
#define F_PILOT_H_DEFAULT      {{F_PILOT_MAG>>8}} // common field



/**************************************************************************
     Interrupt Control Latch       (Write Only)
**************************************************************************/

enum IRQ_CHOICE{ IRQ1, IRQ2};


typedef QEDFER_FALCON_TYPE INTR_PROG_TYPE;

#define INTR_PROG_DEFAULT      {IRQ2,   /* COMP_RDY     */    \
                                IRQ2,   /* TONE_RDY     */    \
                                IRQ2,   /* TSTATE_INT   */    \
                                IRQ2,   /* TSYNC_INT    */    \
                                IRQ2,   /* RSTATE_INT   */    \
                                IRQ2,   /* RSYNC_INT    */    \
                                IRQ2,   /* TXCLK_INT    */    \
                                IRQ2,   /* RXCLK_INT    */    \
                                IRQ2,   /* TEOC_INT or IFFTASAT_INT */    \
                                IRQ2,   /* TAOC_INT or FFTASAT_INT  */    \
                                IRQ2,   /* BURSTDONE_INT*/    \
                                IRQ2,   /* PRBSDET (FALCON 2 only ) */    \
                                IRQ2,   /* FFTBCOMP_RDY */    \
                                IRQ2,   /* TXFIR_INT    */    \
                                IRQ2,   /* EOC_INT      */    \
                                IRQ2}   /* AOC_INT      */

typedef QEDFER_TRAPPER_TYPE INTR_PROG2_TYPE;

#define INTR_PROG2_DEFAULT      {IRQ2}   /* PRBS_DET     */     


/**************************************************************************
    DI Start Delay         (Write Only)
**************************************************************************/

typedef volatile struct
{
   UINT16 DELAY           :14;
   UINT16 TPILOT          :2;
}TXSYM_TYPE;

#define TXSYM_DEFAULT         \
        {0,                   \
         ASIC_CONVERT_PILOT_CHNL(TX_PILOT_TONE)}


typedef volatile struct
{
    QEDCTLL_TYPE      QEDCTLL;              // 0x00  
    QEDCTLH_TYPE      QEDCTLH;              // 0x01
    QAM_MCR_TYPE      QAM_MCR;              // 0x02
    QAM_ICR_TYPE      QAM_ICR;              // 0x03
    QAM_TCRL_TYPE     QAM_TCRL;             // 0x04
    QAM_TCRH_TYPE     QAM_TCRH;             // 0x05
    QAM_RCRL_TYPE     QAM_RCRL;             // 0x06
    QAM_RCRH_TYPE     QAM_RCRH;             // 0x07
    QAM_RSR_TYPE      QAM_RSR;              // 0x08
    C_Coef_TYPE       C1_Coef;              // 0x09
    C_Coef_TYPE       C2_Coef;              // 0x0A
    STORE_LO_Reg_TYPE STORE_LO_Reg;         // 0x0B
    STORE_HI_Reg_TYPE STORE_HI_Reg;         // 0x0C
    PT_POW_TYPE       PT_POW;               // 0x0D
    FEQCTL_TYPE       FEQCTL;               // 0x0E
    TSUB_TYPE         TSUB;                 // 0x0F
    RSWAP_TYPE        RSWAP1;               // 0x10
    RSWAP_TYPE        RSWAP2;               // 0x11
    RSWAP_TYPE        RSWAP3;               // 0x12
    RADDRL_TYPE       RADDRL;               // 0x13
    RADDRH_TYPE       RADDRH;               // 0x14
    RADJUST_TYPE      RADJUST1;             // 0x15
    RADJUST_TYPE      RADJUST2;             // 0x16
    MIF_ISR_TYPE      MIF_ISR;              // 0x17
    F_PILOT_L_TYPE    F_PILOT_L;            // 0x18
    F_PILOT_M_TYPE    F_PILOT_M;            // 0x19
    F_PILOT_H_TYPE    F_PILOT_H;            // 0x1A
    INTR_PROG_TYPE    INTR_PROG;            // 0x1B
    TXSYM_TYPE        TXSYM;                // 0x1C
}QEDFER_TYPE;


#endif
/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/qedfer.h $
 * 
 * 17    8/16/01 5:58p Lauds
 * put physical addresses for each register as comments next to each
 * register definition
 * 
 * 16    8/14/01 12:08p Lauds
 * accomodate UMC Rushmore 
 * 
 * 15    8/02/01 7:40p Lauds
 * modify default monitor channel for Annex B compatible
 * 
 * 14    7/03/01 4:46p Lewisrc
 * Adapting Multi-instance Datapump code to Linux
 * 
 * Change AVERAGE_CHOICE from simple enum to typedef
 * 
 * 13    6/26/01 8:22p Lauds
 * multi-instance version integration
 * 
 * 12    6/19/01 4:53p Lauds
 * Rushmore Support
 * 
 * 11    4/12/01 10:29a Lauds
 * Annex B support
 * 
 * 10    2/05/01 7:52p Lauds
 * Integrate Trapper changes from Harry Lau
 * 
 * 9     11/22/00 3:32p Lauds
 * add registers for Trapper
 * 
 * 8     8/04/00 1:43p Lewisrc
 * Make all events in INTR_PROG_DEFAULT to IRQ2 to prevent multi-processor
 * problems.
 * Added FALCON_2PT in CHIP_ID_CHOICE for David Lau.
 * 
 * 7     3/30/00 4:50p Vivianjj
 * Changed default constellation monitor channel to 56, which is used for
 * both T1.413 and G.HS.
 * 
 * 6     2/15/00 1:15p Vivianjj
 * Added #define for default RT constellation monitor channel.  Also
 * changed the default channel from 41, which seldom is used, to 55 which
 * is almost always used.  Jerry Viviano 
 * 
 * 5     11/09/99 9:30a Lewisrc
 * 2.02f code release from Pairgain
 * 
*
*****************************************************************************/
