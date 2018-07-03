/************************************************************************
*  $Workfile: atmecblk.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/atmecblk.h 8     8/14/01 12:05p Lauds $
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
      Echo and ATM block of the FALCON.
*************************************************************************/

#ifndef ATM_EC_BLK_H
#define ATM_EC_BLK_H

#include "types.h"

/**************************************************************************
    Echo Canceller Control Latch  (Write Only)
    Ref: Sec 12.9.1 Falcon II
**************************************************************************/

typedef volatile struct
{
    UINT16 EC_FDM_   :1;  // 0: FDM mode
                          // 1: EC mode
    UINT16 RT_CO_    :1;  // 0: CO side, echo canceller is a decimating struct.
                          // 1: RT side, echo canceller is a interpolation struct.
    UINT16 BETA_EC   :3;  // 000: beta = 2^-5  (only valid for Falcon, but not Trapper !!!!)
                          // 001: beta = 2^-7
                          // 010: beta = 2^-9
                          // 011: beta = 2^-11
                          // 100: beta = 2^-16
                          // 101: beta = 2^-18
                          // 110: beta = 2^-20
                          // 111: beta = 2^-22

    UINT16 FREEZE_EC :1;  // 0: coefficients are allowed to update
                          // 1: coefficients are frozen
    UINT16 EC_ZERO_  :1;  // 0: zero out EC output
                          // 1:
    UINT16 SMP_RATIO :2;  // 00: 256:32 decimation/32:256 interpolation
                          // 01: 128:32 decimation/32:128 interpolation
                          // 10: 128:64 decimation/64:128 interpolation
                          // 11: 256:64 decimation/64:256 interpolation
    UINT16 GAIN_EC   :4;  //  0000: Gain = 2^0    (Added in Trapper)
                          //  0001: Gain = 2^1
                          //  0010: Gain = 2^2
                          //  0011: Gain = 2^4
                          //  0100: Gain = 2^6
                          //  0101: Gain = 2^8
                          //  0110: Gain = 2^10
                          //  0111: Gain = 2^12
                          //  1000: Gain = 2^-1
                          //  1001: Gain = 2^-2
                          //  1010: Gain = 2^-3
                          //  1011: Gain = 2^-4
                          //  1100: Gain = 2^-4
                          //  1101: Gain = 2^-4
                          //  1110: Gain = 2^-4
                          //  1111: Gain = 2^-4    
    UINT16 BETA_ECh  :1;  // Expanded in Trapper: High bit of BETA_EC 
    UINT16           :2;  // reserved
}ECCTL_TYPE;

#define EC_DIS_BLK  1
#define EC_ENA_BLK  0
#ifdef CO
#define EC_CO_RT   0
#else
#define EC_CO_RT   1
#endif
#define EC_BETA_DEFAULT 0      //2^-5 the biggest one
#define EC_BETA_TRAINING 6      //2^-20 small BETA used during initial training

#define ECCTL_DEFAULT {0,0,0,EC_DIS_BLK,0,0,0,0}   // disable EC
#define ECCTL_ENA_INIT {1,EC_CO_RT,EC_BETA_TRAINING,EC_ENA_BLK,0,0}  // init
#define ECCTL_TRAINING_BEGIN {1,EC_CO_RT,EC_BETA_DEFAULT,EC_ENA_BLK,0,0}  //start training


/**************************************************************************
     ATM Control Latch        (Write Only)
    Ref: 12.10.1 Falcon II

**************************************************************************/
enum {ATM_LS, ATM_AS};

typedef volatile struct
{
    UINT16 TC_RESET      :1; // 0:
                             // 1: reset TCx
    UINT16 TXTC_EN       :1; // 0:
                             // 1: enable TCx tx block
    UINT16 TX_AS_LS      :1; // 0: LSx as ATMx TX
                             // 1: ASx as ATMx TX
    UINT16 RXTC_EN       :1; // 0:
                             // 1: enable TCx rx block
    UINT16 RX_AS_LS      :1; // 0: LSx as ATMx RX
                             // 1: ASx as ATMx RX
    UINT16 DISC_IDLE     :1; // 0:
                             // 1: discard idle cells
    UINT16 EN_DESCR      :1; // 0:
                             // 1: enable the descrambler in TCx
    UINT16 EN_SCR        :1; // 0:
                             // 1: enable the crambler in TCx
    UINT16 LOOP_TEST     :1; // 0:
                             // 1: enable loop test
    UINT16 HEC_CLR       :1; // 0_to_1 transition, clear BADHEC and OVFL in
                             // BAD_HEC0 register
    UINT16 TXSOC_SEL     :1; // 0:connect ?XSOC0 to ATM0 and ?XSOC1 to ATM1 (FALCON 2 only)
    UINT16 RXSOC_SEL     :1; // 1:connect ?XSOC1 to ATM0 and ?XSOC0 to ATM1 (FALCON 2 only)
    UINT16 BYP_TXSOC     :1;
    UINT16               :3;
}ATMCTL_TYPE;


#define ATMCTL0_DEFAULT       \
     {0,   /* reset       */  \
      0,   /* tx enable   */  \
      0,   /* ATM->LS tx  */  \
      0,   /* rx enable   */  \
      0,   /* ATM->LS rx  */  \
      0,   /* idle cell deletion*/  \
      0,   /* descramble enable */  \
      0,   /* scramble enable   */  \
      0,   /* loop back (test)  */  \
      0,   /* keep at zero */       \
      0,   /* connect TXSOC0 to ATM0 and TXSOC1 to ATM1 (FALCON 2 only) */ \
      0,   /* connect RXSOC0 to ATM0 and RXSOC1 to ATM1 (FALCON 2 only) */ \
      0}   /* not bypass TXSOC */
#define ATMCTL1_DEFAULT   ATMCTL0_DEFAULT



/**************************************************************************
    ATM TC Block bad HEC counter   (Read Only)
    Ref: Sec 12.10.3 Falcon II
**************************************************************************/

typedef volatile struct
{
    UINT16 BADHEC     :8;     // bad HEC counter
    UINT16 OVFL       :1;     // FIFO overflow
    UINT16 SYNC       :1;     // SYNC is hunt
    UINT16            :6;
}BAD_HEC_TYPE;

/**************************************************************************
    Transmit Data Clock Control Latches       (Write Only)
    Ref: 12.10.5 - 12.10.7 Falcon II

**************************************************************************/
typedef volatile struct
{
    UINT16 C            :8;     // C parameter
    UINT16 DIV_COEF     :2;     // DIV_COEFF for generating ASx Transit
                                // 00 Divide by 1
                                // 01 Divide by 2
                                // 10 Divide by 4
                                // 11 Divide by 8
    UINT16 RST          :1;     // 0
                                // 1 reset the ASx transmit data clock circuitry
    UINT16 FLIP_I       :1;     // 0
                                // 1 flip the phase of ASx transmit data clk that drives ASLS block
    UINT16 FLIP_O       :1;     // 0
                                // 1 flip the phase of ASx transmit data clk that drives external source
    UINT16 TCK_AS_EN    :1;     // 0 disable ASx transmit data clk generator and use external clk source
                                // 1 enable ASx transmit data clk generator
    UINT16 TX_SEL       :1;     // 0 enable T_CLK_ASx input     (FALCON_2P)
                                // 1 disable T_CLK_ASx input
    UINT16              :1;     // Unused
}TXCLK_ASC_TYPE;


typedef volatile struct
{
    UINT16 C            :8;     // C parameter
    UINT16 DIV_COEF     :2;     // DIV_COEFF for generating LSx Transit
                                // 00 Divide by 1
                                // 01 Divide by 2
                                // 10 Divide by 4
                                // 11 Divide by 8
    UINT16 RST          :1;     // 0
                                // 1 reset the LSx transmit data clock circuitry
    UINT16 FLIP_I       :1;     // 0
                                // 1 flip the phase of LSx transmit data clk that drives ASLS block
    UINT16 FLIP_O       :1;     // 0
                                // 1 flip the phase of LSx transmit data clk that drives external source
    UINT16              :3;
}TXCLK_LSC_TYPE;


typedef volatile UINT16 TXCLK_TYPE;


#define TXCLK_AS0A_DEFAULT  0
#define TXCLK_AS0B_DEFAULT  0
#define TXCLK_AS0C_DEFAULT              \
     {0,   /* C Parameter           */  \
      0,   /* divide by 1           */  \
      0,   /* reset clk circuitry   */  \
      0,   /* don't flip phase      */  \
      0,   /* don't flip phase      */  \
      0,   /* disable internal clk */   \
      0}   /* enable disable external tx clk input */

#define TXCLK_AS1A_DEFAULT  0
#define TXCLK_AS1B_DEFAULT  0
#define TXCLK_AS1C_DEFAULT  TXCLK_AS0C_DEFAULT

#define TXCLK_LS0A_DEFAULT  0
#define TXCLK_LS0B_DEFAULT  0
#define TXCLK_LS0C_DEFAULT       \
     {0,   /* C Parameter           */  \
      0,   /* divide by 1           */  \
      0,   /* reset clk circuitry   */  \
      0,   /* don't flip phase      */  \
      0}   /* don't flip phase      */   

#define TXCLK_LS1A_DEFAULT  0            
#define TXCLK_LS1B_DEFAULT  0                                 
#define TXCLK_LS1C_DEFAULT  TXCLK_LS0C_DEFAULT                
                                                              
/**************************************************************************
    Transmit default byte content Register       (Write Only)
    Ref: 12.6.20.1 Trapper in Rushmore Spec
    Note: Only available in some version of Rushmore (only 8-bit is really writable)

**************************************************************************/

typedef volatile struct
{
    UINT16 SC0          :1;     // 0  perform sync control as indicated by the sc7 to sc1
                                // 1  Fast buffer:this byte is part of an eoc frame
                                //    Intv buffer:LEX byte carries AOC; sync control is allowed for 
                                //    "add AEX" or "delete" as indicated by sc7-sc4
    UINT16 LS_BEARER_SC :1;     // 0  add LEX byte to designated ASx beare chnl
                                // 1  delete last byte from designated ASx beare chnl
    UINT16 LS_BEARER    :2;     // 00 LS0
                                // 01 LS1
                                // 10 LS2
                                // 11 no sync action
    UINT16 AS_BEARER_SC :2;     // 00 no sync action
                                // 00 add AEX byte to designated ASx beare chnl
                                // 01 add LEX byte to designated ASx beare chnl
                                // 10 delete last byte from designated ASx beare chnl
    UINT16 AS_BEARER    :2;     // 00 AS0
                                // 01 AS1
                                // 10 AS2
                                // 11 AS3
    UINT16              :8;     // not used
}SYNC_CONT_TYPE;
#define SYNC_CONT_DEFAULT  {0, 0, 3, 0, 0}                                 

                                                              

typedef volatile struct                                    
{                                                          
    ECCTL_TYPE            ECCTL;          //                0x92  
    ATMCTL_TYPE           ATMCTL0;        //                0x93  
    ATMCTL_TYPE           ATMCTL1;        //                0x94  
    BAD_HEC_TYPE          BAD_HEC0;       //                0x95  
    BAD_HEC_TYPE          BAD_HEC1;       //                0x96  
    TXCLK_TYPE            TXCLK_AS0A;     // FALCON 2 only  0x97  
    TXCLK_TYPE            TXCLK_AS0B;     // FALCON 2 only  0x98  
    TXCLK_ASC_TYPE        TXCLK_AS0C;     // FALCON 2 only  0x99  
    TXCLK_TYPE            TXCLK_AS1A;     // FALCON 2 only  0x9A  
    TXCLK_TYPE            TXCLK_AS1B;     // FALCON 2 only  0x9B  
    TXCLK_ASC_TYPE        TXCLK_AS1C;     // FALCON 2 only  0x9C  
    TXCLK_TYPE            TXCLK_LS0A;     // FALCON 2 only  0x9D  
    TXCLK_TYPE            TXCLK_LS0B;     // FALCON 2 only  0x9E  
    TXCLK_LSC_TYPE        TXCLK_LS0C;     // FALCON 2 only  0x9F  
    TXCLK_TYPE            TXCLK_LS1A;     // FALCON 2 only  0xA0  
    TXCLK_TYPE            TXCLK_LS1B;     // FALCON 2 only  0xA1  
    TXCLK_LSC_TYPE        TXCLK_LS1C;     // FALCON 2 only  0xA2  
    SYNC_CONT_TYPE        SYNC_CONT;      // Rushmore UMC   0xA3  
}ATMBLK_TYPE;                                               
                                                              
                                                              
                                                              
#endif
/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/atmecblk.h $
 * 
 * 8     8/14/01 12:05p Lauds
 * accomodate UMC Rushmore 
 * 
 * 7     6/26/01 7:41p Lauds
 * multi-instance version integration
 * 
 * 6     2/05/01 7:49p Lauds
 * Integrate Trapper changes from Harry Lau
 * 
 * 5     9/07/99 4:15p Lewisrc
 * 09/01/99 2.01B release changes
 * 
 *    Rev 1.12   Aug 17 1999 16:48:30   nordees
 * Enable T_CLK_ASx input on Falcon 2P.
 *
 *    Rev 1.11   Jul 29 1999 16:55:52   nordees
 * Falcon 2P support.
 *
 *    Rev 1.10   Mar 15 1999 12:31:30   nordees
 * Removed embedded comment.
 *
 *    Rev 1.9   Feb 09 1999 18:29:26   laud
 * LCD anomaly, defect, and failure evaluation
 *
 *    Rev 1.8   Jan 28 1999 15:44:18   laud
 * get rid of default reset for tx clk
 *
 *    Rev 1.7   Jan 22 1999 09:39:34   nordees
 * Echo Cancelling
 *
 *    Rev 1.6   Jan 21 1999 17:30:36   laud
 * Trellis coding support
 *
 *    Rev 1.5   Jan 18 1999 17:15:02   laud
 * big and small compatibility change for FALCON 2
 *
 *    Rev 1.4   Jan 12 1999 18:43:04   nordees
 * Falcon 2 support.
 *
 *    Rev 1.3   Oct 21 1998 15:23:22   laud
 * new default value of ATMCTL defaults
 *
 *    Rev 1.2   Sep 02 1998 11:44:46   laud
 * FALCON 1Z fixes
 *
 *    Rev 1.1   01 Sep 1998 14:28:56   tsaurl
 * Falcon 1Z default value
 *
 *    Rev 1.0   Aug 04 1998 17:12:44   laud
 * add ATM and EC block for 1Z
 **********   end of file $Workfile: atmecblk.h $       *****************/
