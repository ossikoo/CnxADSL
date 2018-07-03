/************************************************************************
*  $Workfile: diblk.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/diblk.h 6     8/16/01 5:54p Lauds $
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
      the Digital Interface (DI) block of the FALCON.
*************************************************************************/

#ifndef DIBLK_H
#define DIBLK_H

#include "types.h"
#include "sysinfo.h"

/**************************************************************************
     DI Control Latch         (Write Only)
**************************************************************************/

enum BYPASS_CHOICE {NORMALMODE,BYPASSMODE};
enum INTER_SELCHOICE{INTER16_DEINT2, INTER2_DEINT16};

typedef volatile struct
{
   UINT16 FTCRC_BYPASS       :1;
   UINT16 FTSCR_BYPASS       :1;
   UINT16 FTFEC_BYPASS       :1;
   UINT16 ITCRC_BYPASS       :1;
   UINT16 ITSCR_BYPASS       :1;
   UINT16 ITFEC_BYPASS       :1;
   UINT16 INTER_BYPASS       :1;
   UINT16 FRCRC_BYPASS       :1;
   UINT16 FRSCR_BYPASS       :1;
   UINT16 FRFEC_BYPASS       :1;
   UINT16 IRCRC_BYPASS       :1;
   UINT16 IRSCR_BYPASS       :1;
   UINT16 IRFEC_BYPASS       :1;
   UINT16 DEINT_BYPASS       :1;
   UINT16 INTER_SELECT       :1;
   UINT16 ACC2CNT            :1;
}DICTLL_TYPE;

// This register will be rewritten later for the depending on RATES1 option
#define DICTLL_DEFAULT  {NORMALMODE, /* FTCRC_BYPASS */    \
                         NORMALMODE, /* FTSCR_BYPASS */    \
                         BYPASSMODE, /* FTFEC_BYPASS */    \
                         NORMALMODE, /* ITCRC_BYPASS */    \
                         NORMALMODE, /* ITSCR_BYPASS */    \
                         NORMALMODE, /* ITFEC_BYPASS */    \
                         BYPASSMODE, /* INTER_BYPASS */    \
                         NORMALMODE, /* FRCRC_BYPASS */    \
                         NORMALMODE, /* FRSCR_BYPASS */    \
                         BYPASSMODE, /* FRFEC_BYPASS */    \
                         NORMALMODE, /* IRCRC_BYPASS */    \
                         NORMALMODE, /* IRSCR_BYPASS */    \
                         NORMALMODE, /* IRFEC_BYPASS */    \
                         BYPASSMODE, /* DEINT_BYPASS */    \
                         NORMALMODE, /* INTER_SELECT */    \
                         NORMALMODE} /* ACC2CNT      */


enum TRI_SELCHOICE{RAM_16K, RAM_2K, INTDEC, INTENC};


typedef volatile struct
{
   UINT16 FGEI               :1;
   UINT16 WRITE_READB        :1;    // 0: reading from 16k and 2k RAM via dsp_bus[39:24]
                                    // 1: writing to 16k and 2k RAM
   UINT16 LB_1B              :1;
   UINT16 LB_DI              :1;
   UINT16 XMIT_RCV           :1;
   UINT16 DIS_SF             :1;
   UINT16 DIS_SYM            :1;
   UINT16 TRI_SEL            :2;
   UINT16 ALT_INPUT          :1;
   UINT16 ALT_INPUT1         :1;
   UINT16 RTST_EN1           :1;
   UINT16 DIS_BYTE           :1;
   UINT16                    :1;
   UINT16 CORRUPT_FAST       :1;
   UINT16 CORRUPT_INT        :1;
}DICTLH_TYPE;


// This register will be rewritten later for the depending on RATES1 option
#define DICTLH_DEFAULT  {      0,  /* FGEI          */    \
                               0,  /* READ_WRITE 1Z)*/    \
                               0,  /* LB_1B         */    \
                               0,  /* LB_DI         */    \
                               0,  /* XMIT_RCV      */    \
                               0,  /* DIS_SF        */    \
                               0,  /* DIS_SYM       */    \
                         RAM_16K,  /* TRI_SEL       */    \
                               0,  /* ALT_INPUT     */    \
                               0,  /* ALT_INPUT1    */    \
                               0,  /* RTST_EN1      */    \
                               0,  /* DIS_BYTE      */    \
                               0,  /* CORRUPT_FAST  */    \
                               0}  /* CORRUPT_INT   */

/**************************************************************************
     Transmit CRC Generator Polynomial Latch (Write Only)
**************************************************************************/

typedef volatile struct
{
   UINT16 tap_8  :1;
   UINT16 tap_7  :1;
   UINT16 tap_6  :1;
   UINT16 tap_5  :1;
   UINT16 tap_4  :1;
   UINT16 tap_3  :1;
   UINT16 tap_2  :1;
   UINT16 tap_1  :1;
   UINT16 tap_0  :1;
   UINT16        :7;    // not used
}TCRC_GEN_TYPE;

// This register will be rewritten later for the depending on RATES1 option
#define TCRC_GEN_DEFAULT  {1,0,0,0,1,1,1,0,1}


/**************************************************************************
     FEC Encoder Fast Path Control Latch (Write Only)
**************************************************************************/

enum {CHECK_2BYTES,CHECK_4BYTES,CHECK_6BYTES,CHECK_8BYTES,
      CHECK_10BYTES,CHECK_12BYTES,CHECK_14BYTES,CHECK_16BYTES,
      CHECK_18BYTES,CHECK_20BYTES,CHECK_0BYTE=0X0F};
enum {TRSF1_FRAME, TRSF2_FRAMES,TRSF4_FRAMES=3,TRSF8_FRAMES=7,
      TRSF16_FRAMES=6};

typedef volatile struct
{
   UINT16 TKF    :9;
   UINT16 TRF    :4;
   UINT16 TRSF   :3;  // total num of frame tx fast codeword spreads over,
                      // not available in FALCON 1
}TFCTL_TYPE;

// This register will be rewritten later for the depending on RATES1 option
#define TFCTL_DEFAULT              \
{                                  \
    23,              /* TKF   */   \
    CHECK_4BYTES,    /* TRF   */   \
    TRSF1_FRAME      /* TRSF  */   \
}

/**************************************************************************
     FEC Encoder Interlelaved Path Control Latch (Write Only)
**************************************************************************/

enum {DEPTH_2, DEPTH_4, DEPTH_8, DEPTH_16,
      DEPTH_32,DEPTH_64,DEPTH_128,DEPTH_256};

typedef volatile struct
{
   UINT16 ICODE         :8;
   UINT16               :5;
   UINT16 TRSI          :3;
}TICTLL_TYPE;

typedef volatile struct
{
   UINT16 TKI           :9;
   UINT16 TRI           :4;
   UINT16 INTER_DEPTH   :3;
}TICTLH_TYPE;

// This register will be rewritten later for the depending on RATES1 option
#define TICTLL_DEFAULT {        0x47, /* ICODE        */ \
                                0x02} /* TRSI         */

#define TICTLH_DEFAULT {        0x43, /* TKI          */ \
                        CHECK_4BYTES, /* TRI          */ \
                             DEPTH_2} /* INTER_DEPTH  */

/**************************************************************************
     FEC Decoder Fast Path Control Latch (Write Only)
**************************************************************************/

typedef volatile struct
{
   UINT16 RKF           :9;
   UINT16 RRF           :4;
   UINT16 RNF_3LSB      :3;  // RNF is broken into 2 portions to prevent crossing
}RFCTLL_TYPE;

typedef volatile struct
{
   UINT16 RNF_5MSB      :5;  // word boundary which leads to bits padding
   UINT16 SD            :7;
   UINT16 DEINT_DEPTH   :3;
}RFCTLH_TYPE;

// this will be changed in run-time before the showtime. Initialized value is
// quite irrelevant.
#define RFCTLL_DEFAULT     {23, /* RKF         */ \
                  CHECK_4BYTES, /* RRF         */ \
                             0} /* RNF_3LSB    */

#define RFCTLH_DEFAULT      {0, /* RNF_5MSB    */ \
                             0, /* SD          */ \
                       DEPTH_2} /* DEINT_DEPTH */


/**************************************************************************
     FEC Decoder Interlelaved Path Control Latch (Write Only)
**************************************************************************/
enum {RS_1FRAME,RS_2FRAMES,RS_4FRAMES=3,RS_8FRAMES=7,RS_16FRAMES=6};

typedef volatile struct
{
   UINT16 DCODE         :8;
   UINT16               :8;
}RICTLL_TYPE;

typedef volatile struct
{
   UINT16 RKI           :9;
   UINT16 RRI           :4;
   UINT16 RS            :3; // total num of frame interleaved codeword spreads over,
                            // not available in FALCON 1
}RICTLH_TYPE;

// This register will be rewritten later for the depending on RATES1 option
#define RICTLL_DEFAULT {0x47} /* DCODE */

#define RICTLH_DEFAULT {0x43, /* RKI   */  \
                CHECK_4BYTES, /* RRI   */  \
                   RS_1FRAME} /* RSI    */

/**************************************************************************
     Receive CRC Generator Polynomial Latch (Write Only)
**************************************************************************/

typedef volatile struct
{
   UINT16 tap_8  :1;
   UINT16 tap_7  :1;
   UINT16 tap_6  :1;
   UINT16 tap_5  :1;
   UINT16 tap_4  :1;
   UINT16 tap_3  :1;
   UINT16 tap_2  :1;
   UINT16 tap_1  :1;
   UINT16 tap_0  :1;
   UINT16 RSF    :3;   // total num of frame fast codeword spreads over,
                       // not available in FALCON 1
}RCRC_GEN_TYPE;

// refer to ADSL spec for the CRC polynomial
#define RCRC_GEN_DEFAULT      \
{                             \
            1,  /* tap_8 */   \
            0,  /* tap_7 */   \
            0,  /* tap_6 */   \
            0,  /* tap_5 */   \
            1,  /* tap_4 */   \
            1,  /* tap_3 */   \
            1,  /* tap_2 */   \
            0,  /* tap_1 */   \
            1,  /* tap_0 */   \
    RS_1FRAME   /* RSF   */   \
}

/**************************************************************************
     RS Encoder ROM Start Address Latch (Write Only)
**************************************************************************/

typedef volatile struct
{
   UINT16 CF     :7;
   UINT16 CI     :7;
}CSTART_TYPE;

#define CSTART_DEFAULT  {5,5}

/**************************************************************************
     RS Decoder Chien Search Byte Skip Latch (Write Only)
**************************************************************************/

typedef volatile struct
{
   UINT16 FSKIP  :8;
   UINT16 ISKIP  :8;
}SKIP_TYPE;

// This register will be rewritten later for the depending on RATES1 option
#define SKIP_DEFAULT  {0x77,0x03}

/**************************************************************************
     Interleave/Deinterleave Constant Latch (Write Only)
**************************************************************************/

typedef volatile struct
{
   UINT16 ICONSTANT       :14;
   UINT16 DCONSTANT_2LSB  :2;  // DCONSTANT is broken into 2 portions to prevent crossing
}CONSTANT_LO_TYPE;

typedef volatile struct
{
   UINT16 DCONSTANT_12MSB :12; // word boundary which leads to bits padding
}CONSTANT_HI_TYPE;

// This register will be rewritten later for the depending on RATES1 option
#define CONSTANT_LO_DEFAULT  {0x3FF4,0x3FF4}

#define CONSTANT_HI_DEFAULT  {0x3FF4>>2}

/**************************************************************************
     DI Fast FEC/CRC Status Latch       (Read Only)
**************************************************************************/

enum {NO_CRC_MISMATCH,CRC_MISMATCH};

typedef volatile struct
{
   UINT16 FCRC                :1;
   UINT16 FFEC                :7;
   UINT16 FFECC               :7;
}FAST_FEC_TYPE;

/**************************************************************************
     DI Interleaved FEC/CRC Status Latch       (Read Only)
**************************************************************************/

typedef volatile struct
{
   UINT16 ICRC                :1;
   UINT16 IFEC                :7;
   UINT16 IFECC               :7;
}INT_FEC_TYPE;


/**************************************************************************
     ASLS Interface Control Latch (Write Only)
**************************************************************************/

enum {SYM_CONFIG, ASYM_CONFIG};
enum {FAST_BUFFER, INTERLEAVED_BUFFER, DATA_PATH_NUM};
enum {AOC_FILTER_DISABLE, AOC_FILTER_ENABLE};
#ifdef CO
typedef volatile struct
{
   UINT16 ATU_MODE            :1;
   UINT16 ASYM_or_SYM         :1;
   UINT16 TPATH_AS0           :1;
   UINT16 TPATH_AS1           :1;
   UINT16 TPATH_LS1           :1;
   UINT16 TPATH_LS0           :1;
   UINT16 RPATH_LS1           :1;
   UINT16 RPATH_LS0           :1;
   UINT16 CLR_TEOC            :1;
   UINT16 CLR_TAOC            :1;
   UINT16 RAOC_IDLE           :1;
   UINT16 TREDUCE_FULLB       :1;    // 1: reduced overhead framing (tx)
   UINT16 TSINGLE_REDUCEB     :1;    // 1: single latency reduced overhead framing
   UINT16 RREDUCE_FULLB       :1;    // 1: reduced overhead framing (rx)
   UINT16 RSINGLE_REDUCEB     :1;    // 1: single latency reduced overhead framing
   UINT16                     :1;
}ASLSCTL_TYPE;
#else
typedef volatile struct
{
   UINT16 ATU_MODE            :1;
   UINT16 ASYM_or_SYM         :1;
   UINT16 RPATH_AS1           :1;
   UINT16 RPATH_AS0           :1;
   UINT16 TPATH_LS1           :1;
   UINT16 TPATH_LS0           :1;
   UINT16 RPATH_LS1           :1;
   UINT16 RPATH_LS0           :1;
   UINT16 CLR_TEOC            :1;
   UINT16 CLR_TAOC            :1;
   UINT16 RAOC_IDLE           :1;
   UINT16 TREDUCE_FULLB       :1;    // 1: reduced overhead framing (tx)
   UINT16 TSINGLE_REDUCEB     :1;    // 1: single latency reduced overhead framing
   UINT16 RREDUCE_FULLB       :1;    // 1: reduced overhead framing (rx)
   UINT16 RSINGLE_REDUCEB     :1;    // 1: single latency reduced overhead framing
   UINT16                     :1;
}ASLSCTL_TYPE;
#endif

// This register will be rewritten later for the depending on RATES1 option
#define ASLSCTL_DEFAULT {                                \
      (XCVR_MODE == ATU_R) ? 0 : 1,  /* ATU_MODE     */  \
                       ASYM_CONFIG,  /* ASYM_or_SYM  */  \
                INTERLEAVED_BUFFER,  /* TPATH_AS0    */  \
                INTERLEAVED_BUFFER,  /* TPATH_AS1    */  \
                INTERLEAVED_BUFFER,  /* TPATH_LS1    */  \
                INTERLEAVED_BUFFER,  /* TPATH_LS0    */  \
                INTERLEAVED_BUFFER,  /* RPATH_LS1    */  \
                INTERLEAVED_BUFFER,  /* RPATH_LS0    */  \
                                 1,  /* CLR_TEOC     */  \
                                 1,  /* CLR_TAOC     */  \
                 AOC_FILTER_ENABLE,  /* RAOC_IDLE    */  \
                                 0,  /* no tx reduced*/  \
                                 0,  /* no tx single */  \
                                 0,  /* no rx reduced*/  \
                                 0   /* no rx single */  \
}


/**************************************************************************
     Transmit AS0  (ATU-C)/
     Receive AS1   (ATU-R) Byte Allocation Latch (Write Only)
**************************************************************************/

enum C_CHNL_SRC{C_CHNL_LS0, C_CHNL_LEX};
#ifdef CO

typedef volatile struct
{
   UINT16 AS0         :9;
   UINT16 TC16        :1;
   UINT16 TLS0        :5;    // FALCON 2 only
}TRASX1_TYPE;

// This register will be rewritten later for the depending on RATES1 option
#define TRASX1_DEFAULT  {0x40, C_CHNL_LS0, 0}

#else

typedef volatile struct
{
   UINT16 AS1         :9;
   UINT16 TC16        :1;
   UINT16 TLS0        :5;    // FALCON 2 only
}TRASX1_TYPE;

// This register will be rewritten later for the depending on RATES1 option
#define TRASX1_DEFAULT  {0, C_CHNL_LS0, 0}
#endif


/**************************************************************************
     Transmit AS1 (ATU-C)/
     Receive AS0  (ATU-R) Byte Allocation Latch (Write Only)
**************************************************************************/

#ifdef CO
typedef volatile struct
{
   UINT16 AS1         :9;
   UINT16 RC16        :1;
   UINT16 RLS0        :5;    // FALCON 2 only
}TRASX2_TYPE;

// This register will be rewritten later for the depending on RATES1 option
#define TRASX2_DEFAULT  {0x40, C_CHNL_LS0, 0}


#else

typedef volatile struct
{
   UINT16 AS0         :9;
   UINT16 RC16        :1;
   UINT16 RLS0        :5;    // FALCON 2 only
}TRASX2_TYPE;

// This register will be rewritten later for the depending on RATES1 option
#define TRASX2_DEFAULT  {0, C_CHNL_LS0, 0}

#endif


/**************************************************************************
     Transmit/Receive LSX Byte Allocation Latch (Write Only)
**************************************************************************/

typedef volatile struct
{
   UINT16 TLS1       :5;
   UINT16            :3;
   UINT16 RLS1       :5;
   UINT16            :3;
}TRLSX_TYPE;

// This register will be rewritten later for the depending on RATES1 option

#define TRLSX_DEFAULT  {0x12,0x12}

/**************************************************************************
     Transmit Indicator Bits (High and Middle Byte) Latch (Write Only)
**************************************************************************/

typedef volatile struct
{
    UINT16 crc_i      :1;     // near_end CRC error, interleave
                              //  (Reported back to other unit as febe_i)
    UINT16 fec_i      :1;     // near_end forward error correction, interleave
    UINT16 crc_f      :1;     // near_end CRC error, fast
                              //  (Reported back to other unit as febe_ni)
    UINT16 fec_f      :1;     // near_end forward error correction, fast
    UINT16 los        :1;     // near_end Los of Signal
    UINT16 sef        :1;     // near_end Severely Errored Frame
                              //  (Reported back to other unit as rdi)
    UINT16 ncd_i      :1;     // near_end No cell delineation, interleave
    UINT16 ncd_f      :1;     // near_end No cell delineation, fast
    UINT16 hec_i      :1;     // near_end Header Error Check, interleave
    UINT16 hec_f      :1;     // near_end Header Error Check, fast
    UINT16 NTR        :6;     // set to 1 for issue 2
}TIB_HM_TYPE;


// This register will be rewritten later for the depending on RATES1 option
#define TIB_HM_DEFAULT  {1,1,1,1,1,1,1,1,1,1,0x3F}



/**************************************************************************
     Transmit Indicator Bits (Low Byte) Latch (Read Only)
**************************************************************************/

typedef volatile UINT16  IBL_TYPE;

// This register will be rewritten later for the depending on RATES1 option
// write  0xFF to frame #1

#define TIBL_DEFAULT     0xFF

/**************************************************************************
     Receive Indicator Bits (High and Middle Byte) Latch (Read Only)
**************************************************************************/

typedef volatile struct
{
    UINT16 febe_i     :1;     // far_end block error, interleave
                              //  (crc_i anomaly detected at far end)
    UINT16 ffec_i     :1;     // far_end forward error correction, interleave
    UINT16 febe_f     :1;     // far_end block error, fast
                              //  (crc_ni anomaly detected at far end)
    UINT16 ffec_f     :1;     // far_end forward error correction, fast
    UINT16 los        :1;     // far_end Los of Signal defect
    UINT16 rdi        :1;     // far_end Remote defect Indicator
                              //  (sef defect detected at far end)
    UINT16 ncd_i      :1;     // near_end No cell delineation, interleave
    UINT16 ncd_f      :1;     // near_end No cell delineation, fast
    UINT16 hec_i      :1;     // near_end Header Error Check, interleave
    UINT16 hec_f      :1;     // near_end Header Error Check, fast
    UINT16            :6;
}RIB_HM_TYPE;



/**************************************************************************
     Receive Indicator Bits (Low Byte) Latch (Read Only)
**************************************************************************/


/**************************************************************************
     Transmit EOC Message Latch (Write Only)
     Receive EOC Message Latch (Read Only)
**************************************************************************/

enum EOC_MODE_CHOICE{EOC_DATA, EOC_OPCODE};

// Refer eoc word format in T1E1.4/95-007R2 section 6.2.1.1
typedef volatile struct
{
   UINT16 one_always          :1;    // lsb of even frame is always 1
   UINT16 reserved            :1;    // reserved for future use (set to 1)
   UINT16 address             :2;    // destination addr (00=RT,11=CO)
   UINT16 mode                :1;    // 0=Data, 1=Opcode
   UINT16 parity              :1;    // 0=even parity, 1=odd parity
   UINT16 rt_response_bit     :1;    // not used in ATU-C,
                                     // set to 0 by ATU-R for autonomous msg
                                     // set to 1 for response to ATU_C
   UINT16 data_bit0           :1;    // bit 6 of one of the 58 opcodes or
                                     // the 8-bit data
   UINT16 one_always_too      :1;    // lsb of odd frame is always 1
   UINT16 data_bit1_7         :7;    // bit 7 to 13 of one of the 58 opcodes
                                     // or the 8-bit data
}EOC_TYPE;

#ifdef CO
#define TEOC_DEFAULT   {1,1,0,0,1,0,0,1,0}
#else
#define TEOC_DEFAULT   {1,1,3,0,1,0,0,1,0}
#endif


/**************************************************************************
     Transmit AOC Message Latch (Write Only)
**************************************************************************/

typedef volatile UINT16 AOC_TYPE;


#define TAOC_DEFAULT    0

/**************************************************************************
     Receive AOC Message Latch (Read Only)
**************************************************************************/

typedef volatile UINT16 RAOC_TYPE;


/**************************************************************************
     ASLS Status Register      (Read Only)
**************************************************************************/

typedef volatile struct
{
   UINT16 TAS0_OFF            :1;
   UINT16 TAS1_OFF            :1;
   UINT16 RAS0_OFF            :1;
   UINT16 RAS1_OFF            :1;
   UINT16 TLS1_OFF            :1;
   UINT16 TLS0_OFF            :1;
   UINT16 RLS1_OFF            :1;
   UINT16 RLS0_OFF            :1;
   UINT16 RAOC_EMPTY          :1;
   UINT16 REOC_EMPTY          :1;
   UINT16 TAOC_FULL           :1;
   UINT16 TAOC_EMPTY          :1;
   UINT16 TEOC_EMPTY          :1;
   UINT16                     :3;
}DISR_TYPE;

/**************************************************************************
     AS? Digital Lowpass Filter Coefficient Register (Write Only)
     AS? Digital Lowpass Frequency Storage Register (Read/Write)
**************************************************************************/

typedef volatile UINT16 C_TYPE;
typedef volatile UINT16 STORE_LO_TYPE;
typedef volatile UINT16 STORE_HI_TYPE;

// refer to user manual for initialization values
// STORE value for AS0 = 0x01000000L
#define STORE_AS0_LO_DEFAULT  0x0000L
#define STORE_AS0_HI_DEFAULT  0x0100L

// STORE value for AS1 = 0x01000000L
#define STORE_AS1_LO_DEFAULT  0x0000L
#define STORE_AS1_HI_DEFAULT  0x0100L

// STORE value for LS1 = 0x01000000L
#define STORE_LS1_LO_DEFAULT  0x0000L
#define STORE_LS1_HI_DEFAULT  0x0100L

// STORE value for LS0 = 0x00408102L
#define STORE_LS0_LO_DEFAULT  0x8102L
#define STORE_LS0_HI_DEFAULT  0x0040L


#define C1_AS0_DEFAULT  0
#define C2_AS0_DEFAULT  0
#define C1_AS1_DEFAULT  0
#define C2_AS1_DEFAULT  0
#define C1_LS1_DEFAULT  0
#define C2_LS1_DEFAULT  0
#define C1_LS0_DEFAULT  0
#define C2_LS0_DEFAULT  0


/**************************************************************************
     Transmit AS0 /Receive AS1 Reference Latch (Write Only)
**************************************************************************/

/**************************************************************************
     Transmit AS1 /Receive AS0 Reference Latch (Write Only)
**************************************************************************/
#ifdef CO
typedef volatile UINT16 REF_TAS_TYPE;
#define REF_TAS0_DEFAULT  0x34
#define REF_TAS1_DEFAULT  0x34
#else
typedef volatile UINT16 REF_RAS_TYPE;
#define REF_RAS1_DEFAULT  0
#define REF_RAS0_DEFAULT  0
#endif

/**************************************************************************
      LS1 Reference Latch (Write Only)
      LS0 Reference Latch (Write Only)
**************************************************************************/

typedef volatile struct
{
   UINT16 transmit           :9;
   UINT16 receive            :7;
}REF_TRLSX_TYPE;

/**************************************************************************
     Receive Phase Comparator Thresholds (Write Only)
**************************************************************************/

typedef volatile struct
{
   UINT16 AS0                  :3;
   UINT16 AS1                  :3;
   UINT16 LS1                  :3;
   UINT16 LS0                  :3;
}PH_ALSX_TYPE;

#define PH_ALSX_DEFAULT  {7,   /* AS0  */    \
                          7,   /* AS1  */    \
                          7,   /* LS1  */    \
                          7}   /* LS0  */

/**************************************************************************
     Transmit ASX  (ATU-C)/
     Receive ASX   (ATU-R) Step Size (Write Only)
**************************************************************************/

#ifdef CO
typedef volatile UINT16  STEP_TAS_TYPE;
#define STEP_TAS0_DEFAULT  0x1D
#define STEP_TAS1_DEFAULT  0x1D
#else
typedef volatile UINT16  STEP_RAS_TYPE;
#define STEP_RAS0_DEFAULT  0x1D
#define STEP_RAS1_DEFAULT  0x1D
#endif

/**************************************************************************
     Transmit LSX  /  Receive LSX  Step Size (Write Only)
**************************************************************************/

typedef volatile UINT16  STEP_LS_TYPE;
#define STEP_TLS1_DEFAULT  0x07
#define STEP_TLS0_DEFAULT  0x07
#define STEP_RLS1_DEFAULT  0x00
#define STEP_RLS0_DEFAULT  0x07

/**************************************************************************
     FIFO Bank Reset Control Register (Write Only)
**************************************************************************/


#ifdef CO
typedef volatile struct
{
   UINT16 TAS0                    :1;
   UINT16 TAS1                    :1;
   UINT16 TLS1                    :1;
   UINT16 TLS0                    :1;
   UINT16 RLS1                    :1;
   UINT16 RLS0                    :1;
   UINT16 RTST_EN                 :1;
   UINT16 START_DI                :1;
   UINT16 FRESET                  :1;   // not available in FALCON 1
   UINT16 IRESET                  :1;   // not available in FALCON 1
   UINT16 TFSUP_GEN               :1;   // not available in FALCON 1
   UINT16 TFSYM_GEN               :1;   // not available in FALCON 1
   UINT16 TISUP_GEN               :1;   // not available in FALCON 1
   UINT16 TISYM_GEN               :1;   // not available in FALCON 1
   UINT16                         :1;   // not available in FALCON 1
   UINT16 W_RESET                 :1;   // not available in FALCON 1
}RESET_CTL_TYPE;
#else
typedef volatile struct
{
   UINT16 RAS1                    :1;
   UINT16 RAS0                    :1;
   UINT16 TLS1                    :1;
   UINT16 TLS0                    :1;
   UINT16 RLS1                    :1;
   UINT16 RLS0                    :1;
   UINT16 RTST_EN                 :1;
   UINT16 START_DI                :1;
   UINT16 FRESET                  :1;   // not available in FALCON 1
   UINT16 IRESET                  :1;   // not available in FALCON 1
   UINT16 TFSUP_GEN               :1;   // not available in FALCON 1
   UINT16 TFSYM_GEN               :1;   // not available in FALCON 1
   UINT16 TISUP_GEN               :1;   // not available in FALCON 1
   UINT16 TISYM_GEN               :1;   // not available in FALCON 1
   UINT16                         :1;   // not available in FALCON 1
   UINT16 W_RESET                 :1;   // not available in FALCON 1
}RESET_CTL_TYPE;
#endif

// This register will be rewritten later for the depending on RATES1 option
#define RESET_CTL_DEFAULT                                \
{                                                        \
    0,    /* RAS1                                    */  \
    0,    /* RAS0                                    */  \
    0,    /* TLS1                                    */  \
    0,    /* TLS0                                    */  \
    0,    /* RLS1                                    */  \
    0,    /* RLS0                                    */  \
    0,    /* RTST_EN                                 */  \
    0,    /* START_DI                                */  \
    0,    /* FRESET     (not available in FALCON 1)  */  \
    0,    /* IRESET     (not available in FALCON 1)  */  \
    0,    /* TFSUP_GEN  (not available in FALCON 1)  */  \
    0,    /* TFSYM_GEN  (not available in FALCON 1)  */  \
    0,    /* TISUP_GEN  (not available in FALCON 1)  */  \
    0,    /* TISYM_GEN  (not available in FALCON 1)  */  \
    0     /* W_RESET    (not available in FALCON 1)  */  \
}

/**************************************************************************
     ASLS Miscellaneous Configuration Latch  (Write Only)
**************************************************************************/

enum {USE_MCLK, USE_DOUBLE_CLK};
enum {CARRYOUT_BIT,MSB_SUM_BIT};
enum {SYNC_CTL_ALLOWED, SYNC_CTL_DISALLOWED};

typedef volatile struct
{
   UINT16 REF_CLK                 :1;
   UINT16 RASO_CLK                :1;
   UINT16 RAS1_CLK                :1;
   UINT16 RLS1_CLK                :1;
   UINT16 RLS0_CLK                :1;
   UINT16 TAS0_SYNCOFF            :1;
   UINT16 TAS1_SYNCOFF            :1;
   UINT16 TLS1_SYNCOFF            :1;
   UINT16 TLS0_SYNCOFF            :1;
   UINT16 RAS0_SYNCOFF            :1;   // not available in FALCON 1
   UINT16 RAS1_SYNCOFF            :1;   // not available in FALCON 1
   UINT16 RLS0_SYNCOFF            :1;   // not available in FALCON 1
   UINT16 RLS1_SYNCOFF            :1;   // not available in FALCON 1
}MISC_CONFIG_TYPE;

// This register will be rewritten later for the depending on RATES1 option
// refer to software functional spec for register initialization
#define MISC_CONFIG_DEFAULT                     \
{                                               \
    USE_MCLK,              /* REF_CLK      */   \
    MSB_SUM_BIT,           /* RASO_CLK     */   \
    MSB_SUM_BIT,           /* RAS1_CLK     */   \
    MSB_SUM_BIT,           /* RLS1_CLK     */   \
    MSB_SUM_BIT,           /* RLS0_CLK     */   \
    SYNC_CTL_DISALLOWED,   /* TAS0_SYNCOFF */   \
    SYNC_CTL_DISALLOWED,   /* TAS1_SYNCOFF */   \
    SYNC_CTL_DISALLOWED,   /* TLS1_SYNCOFF */   \
    SYNC_CTL_DISALLOWED,   /* TLS0_SYNCOFF */   \
    SYNC_CTL_DISALLOWED,   /* RAS0_SYNCOFF (not available in FALCON 1)*/   \
    SYNC_CTL_DISALLOWED,   /* RAS1_SYNCOFF (not available in FALCON 1)*/   \
    SYNC_CTL_DISALLOWED,   /* RLS1_SYNCOFF (not available in FALCON 1)*/   \
    SYNC_CTL_DISALLOWED,   /* RLS0_SYNCOFF (not available in FALCON 1)*/   \
}

/**************************************************************************
     Loopback Configuration Latch  (Write Only)
**************************************************************************/
enum ASLSorGENchoice {GENERIC_INTF, ASLS_INTF};
enum LB1choice {ASLS_DEMUX, ASLS_MUX};
enum LB2choice {ASLS_GENERIC, ASLS_LBMUX};
enum LBchoice {NO_ASLS_LOOPBACK, ASLS_LOOPBACK};
enum CHAN_SELchoice {AS1_CHAN, AS0_CHAN, LS1_CHAN, LS0_CHAN};

typedef volatile struct
{
   UINT16 FARLP_LS0               :1;
   UINT16 FARLP_LS1               :1;
   UINT16 FARLP_AS0               :1;
   UINT16 TASLSorGEN              :1;
   UINT16 RASLSorGEN              :1;
   UINT16 ASLS_LB1                :1;
   UINT16 ASLS_LB2                :1;
   UINT16 INT_SEL                 :2;
   UINT16 CHAN_SEL                :2;
   UINT16 FASTorINT               :1;
   UINT16 ASLS_TEST               :1;
   UINT16 TATM_ASLS_              :1;    // 0: ASLS
                                         // 1: Tx TAM
   UINT16 RATM_ASLS_              :1;    // 0: ASLS
                                         // 1: Rx ATM
   UINT16                         :1;
}LOOP_TYPE;

#define LOOP_DEFAULT             \
        {NO_ASLS_LOOPBACK, NO_ASLS_LOOPBACK, NO_ASLS_LOOPBACK,  \
         ASLS_INTF, ASLS_INTF,             \
         ASLS_DEMUX, ASLS_GENERIC,               \
         0, AS1_CHAN, 0, 0, 0, 0 }


#ifdef CO
typedef volatile struct
{
    DICTLL_TYPE        DICTLL;            // 0x43
    DICTLH_TYPE        DICTLH;            // 0x44
    TCRC_GEN_TYPE      TCRC_GEN;          // 0x45
    TFCTL_TYPE         TFCTL;             // 0x46
    TICTLL_TYPE        TICTLL;            // 0x47
    TICTLH_TYPE        TICTLH;            // 0x48
    RFCTLL_TYPE        RFCTLL;            // 0x49
    RFCTLH_TYPE        RFCTLH;            // 0x4A
    RICTLL_TYPE        RICTLL;            // 0x4B
    RICTLH_TYPE        RICTLH;            // 0x4C
    RCRC_GEN_TYPE      RCRC_GEN;          // 0x4D
    CSTART_TYPE        CSTART;            // 0x4E
    SKIP_TYPE          SKIP;              // 0x4F
    CONSTANT_LO_TYPE   CONSTANT_LO;       // 0x50
    CONSTANT_HI_TYPE   CONSTANT_HI;       // 0x51
    FAST_FEC_TYPE      FAST_FEC;          // 0x52
    INT_FEC_TYPE       INT_FEC;           // 0x53
    ASLSCTL_TYPE       ASLSCTL;           // 0x54
    TRASX1_TYPE        TRASX1;            // 0x55
    TRASX2_TYPE        TRASX2;            // 0x56
    TRLSX_TYPE         TRLSX;             // 0x57
    TIB_HM_TYPE        TIB_HM;            // 0x58
    IBL_TYPE           TIBL;              // 0x59
    RIB_HM_TYPE        RIB_HM;            // 0x5A
    IBL_TYPE           RIBL;              // 0x5B
    EOC_TYPE           TEOC;              // 0x5C
    EOC_TYPE           REOC;              // 0x5D
    AOC_TYPE           TAOC;              // 0x5E
    AOC_TYPE           RAOC;              // 0x5F
    DISR_TYPE          DISR;              // 0x60
    C_TYPE             C1_AS0;            // 0x61
    C_TYPE             C2_AS0;            // 0x62
    STORE_LO_TYPE      STORE_AS0_LO;      // 0x63
    STORE_HI_TYPE      STORE_AS0_HI;      // 0x64
    C_TYPE             C1_AS1;            // 0x65
    C_TYPE             C2_AS1;            // 0x66
    STORE_LO_TYPE      STORE_AS1_LO;      // 0x67
    STORE_HI_TYPE      STORE_AS1_HI;      // 0x68
    C_TYPE             C1_LS1;            // 0x69
    C_TYPE             C2_LS1;            // 0x6A
    STORE_LO_TYPE      STORE_LS1_LO;      // 0x6B
    STORE_HI_TYPE      STORE_LS1_HI;      // 0x6C
    C_TYPE             C1_LS0;            // 0x6D
    C_TYPE             C2_LS0;            // 0x6E
    STORE_LO_TYPE      STORE_LS0_LO;      // 0x6F
    STORE_HI_TYPE      STORE_LS0_HI;      // 0x70
    REF_TAS_TYPE       REF_TAS0;          // 0x71
    REF_TAS_TYPE       REF_TAS1;          // 0x72
    REF_TRLSX_TYPE     REF_LS1;           // 0x73
    REF_TRLSX_TYPE     REF_LS0;           // 0x74
    PH_ALSX_TYPE       PH_ALSX;           // 0x75
    STEP_TAS_TYPE      STEP_TAS0;         // 0x76
    STEP_TAS_TYPE      STEP_TAS1;         // 0x77
    STEP_LS_TYPE       STEP_TLS1;         // 0x78
    STEP_LS_TYPE       STEP_TLS0;         // 0x79
    STEP_LS_TYPE       STEP_RLS1;         // 0x7A
    STEP_LS_TYPE       STEP_RLS0;         // 0x7B
    RESET_CTL_TYPE     RESET_CTL;         // 0x7C
    MISC_CONFIG_TYPE   MISC_CONFIG;       // 0x7D
    LOOP_TYPE          LOOP;              // 0x7E
}DIBLK_TYPE;


#else     // ATU_R
typedef volatile struct
{
    DICTLL_TYPE        DICTLL;            // 0x43
    DICTLH_TYPE        DICTLH;            // 0x44
    TCRC_GEN_TYPE      TCRC_GEN;          // 0x45
    TFCTL_TYPE         TFCTL;             // 0x46
    TICTLL_TYPE        TICTLL;            // 0x47
    TICTLH_TYPE        TICTLH;            // 0x48
    RFCTLL_TYPE        RFCTLL;            // 0x49
    RFCTLH_TYPE        RFCTLH;            // 0x4A
    RICTLL_TYPE        RICTLL;            // 0x4B
    RICTLH_TYPE        RICTLH;            // 0x4C
    RCRC_GEN_TYPE      RCRC_GEN;          // 0x4D
    CSTART_TYPE        CSTART;            // 0x4E
    SKIP_TYPE          SKIP;              // 0x4F
    CONSTANT_LO_TYPE   CONSTANT_LO;       // 0x50
    CONSTANT_HI_TYPE   CONSTANT_HI;       // 0x51
    FAST_FEC_TYPE      FAST_FEC;          // 0x52
    INT_FEC_TYPE       INT_FEC;           // 0x53
    ASLSCTL_TYPE       ASLSCTL;           // 0x54
    TRASX1_TYPE        TRASX1;            // 0x55
    TRASX2_TYPE        TRASX2;            // 0x56
    TRLSX_TYPE         TRLSX;             // 0x57
    TIB_HM_TYPE        TIB_HM;            // 0x58
    IBL_TYPE           TIBL;              // 0x59
    RIB_HM_TYPE        RIB_HM;            // 0x5A
    IBL_TYPE           RIBL;              // 0x5B
    EOC_TYPE           TEOC;              // 0x5C
    EOC_TYPE           REOC;              // 0x5D
    AOC_TYPE           TAOC;              // 0x5E
    AOC_TYPE           RAOC;              // 0x5F
    DISR_TYPE          DISR;              // 0x60
    C_TYPE             C1_AS0;            // 0x61
    C_TYPE             C2_AS0;            // 0x62
    STORE_LO_TYPE      STORE_AS0_LO;      // 0x63
    STORE_HI_TYPE      STORE_AS0_HI;      // 0x64
    C_TYPE             C1_AS1;            // 0x65
    C_TYPE             C2_AS1;            // 0x66
    STORE_LO_TYPE      STORE_AS1_LO;      // 0x67
    STORE_HI_TYPE      STORE_AS1_HI;      // 0x68
    C_TYPE             C1_LS1;            // 0x69
    C_TYPE             C2_LS1;            // 0x6A
    STORE_LO_TYPE      STORE_LS1_LO;      // 0x6B
    STORE_HI_TYPE      STORE_LS1_HI;      // 0x6C
    C_TYPE             C1_LS0;            // 0x6D
    C_TYPE             C2_LS0;            // 0x6E
    STORE_LO_TYPE      STORE_LS0_LO;      // 0x6F
    STORE_HI_TYPE      STORE_LS0_HI;      // 0x70
    REF_RAS_TYPE       REF_RAS1;          // 0x71
    REF_RAS_TYPE       REF_RAS0;          // 0x72
    REF_TRLSX_TYPE     REF_LS1;           // 0x73
    REF_TRLSX_TYPE     REF_LS0;           // 0x74
    PH_ALSX_TYPE       PH_ALSX;           // 0x75
    STEP_RAS_TYPE      STEP_RAS1;         // 0x76
    STEP_RAS_TYPE      STEP_RAS0;         // 0x77
    STEP_LS_TYPE       STEP_TLS1;         // 0x78
    STEP_LS_TYPE       STEP_TLS0;         // 0x79
    STEP_LS_TYPE       STEP_RLS1;         // 0x7A
    STEP_LS_TYPE       STEP_RLS0;         // 0x7B
    RESET_CTL_TYPE     RESET_CTL;         // 0x7C
    MISC_CONFIG_TYPE   MISC_CONFIG;       // 0x7D
    LOOP_TYPE          LOOP;              // 0x7E
}DIBLK_TYPE;                              
                                          
                                          
#endif                                    
                                          
                                          
                                          
#endif                                    
/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/diblk.h $
 * 
 * 6     8/16/01 5:54p Lauds
 * put physical addresses for each register as comments next to each
 * register definition
 * 
 * 5     6/26/01 7:49p Lauds
 * multi-instance version integration
 * 
 * 4     12/07/99 10:23a Lewisrc
 * 2.02g
 * 
 *    Rev 1.48   Dec 03 1999 17:47:18   nordees
 * Added support for datapath selection API.
 * 
 *    Rev 1.47   Oct 26 1999 16:38:28   yangl
 * -- Stop Support Falcon 1Z
 * -- Dynamic config with Falcon2 & 2P
 * 
 * 
 *    Rev 1.46   Feb 09 1999 18:29:12   laud
 * LCD anomaly, defect, and failure evaluation
 * 
*
*****************************************************************************/
