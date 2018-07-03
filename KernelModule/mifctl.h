/************************************************************************
*  $Workfile: mifctl.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/mifctl.h 9     8/16/01 5:58p Lauds $
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
      the Microcontroller Interface (MIF) block of the FALCON.
*************************************************************************/

#ifndef MIFCTL_H
#define MIFCTL_H

#include "types.h"
#include "sysinfo.h"


/**************************************************************************
     Micro-Interface Control Register       (Read/Write)
**************************************************************************/

typedef volatile struct
{
   UINT16 BURST_DONE         :1;
   UINT16 CLR_BD             :1;
   UINT16 T_SWAP             :1;
   UINT16 R_SWAP             :1;
   UINT16 REF_CTL            :1;
   UINT16 DIS_TRISTATE       :1;
   UINT16 SNOOP              :1;
   UINT16 SNP_WR             :4;
   UINT16 SNP_BUS            :1;
   UINT16 SNP_BATCH          :1;
   UINT16 BURST_OVERRIDE     :1;
   UINT16 SNP_BUS2           :1;
}MIFCTL_TYPE;

typedef volatile struct
{
   MIFCTL_TYPE          MIFCTL;    // 0x83
}MIFBLK_TYPE;

/**************************************************************************
     Micro-Interface Scratch Pad RAM       (Read/Write)
**************************************************************************/
// Variable(s) of the following typedef shall be used temporarily to store
// a copy of a block of data read from the Falcon scratch pad RAM. Note that
// the Falcon shall transfer data from its internal memory to the scratch pad
// ram for the uP to access. We shall copy data from the Falcon scratch pad
// ram into this type of variable for our own manipulations

typedef volatile union
{
   UINT8  uint8[1024];
   UINT16 uint16[512];
   UINT32 uint32[256];
   SINT8  sint8[1024];
   SINT16 sint16[512];
   SINT32 sint32[256];
   SINT16CMPLX sint16cmplx[256];
   SINT32CMPLX sint32cmplx[128];
} SPRAM_TYPE;

#define SPRAM_TYPE_SIZE     sizeof(SPRAM_TYPE)        // 1024 bytes




/*-----------------12-05-97 03:23pm-----------------
 Note that although the following register doesn't
 belong to the MIF Control Block. But since the address
 offsets locates after MIFCTL block, we just keep it
 in the file for simplicity.
--------------------------------------------------*/

/**************************************************************************
     Transmit Network Timing Reference Offset Register (Read Only)     (not available in FALCON 1)
     Receive Network Timing Reference Offset Register (Write Only)     (not available in FALCON 1)
**************************************************************************/
// This registers are DACCTL block register but locates at 0x84 and 0x85
typedef volatile struct
{
    UINT16 NTR_OFFSET        :9;
    UINT16                   :7;    // unused
}NTRTX_TYPE;

typedef volatile struct
{
    UINT16 NTR_STEP          :16;
}NTRRX_TYPE;

#define NTRRX_DEFAULT    {0}


/**************************************************************************
     Power Threshold Latch (Write Only)     (available in FALCON 2)
**************************************************************************/
// This registers are QEDFER block register but locates at 0x87
typedef volatile UINT16  POWER_THLD_TYPE;

/**************************************************************************
     Pilot-tone Latch (Read Only)     (not available in FALCON 1)
**************************************************************************/
// This registers are QEDFER block register but locates at 0x88,0x89 and 0x8A
#define PILOT_L_TYPE   F_PILOT_L_TYPE     // define in QEDFER.h
#define PILOT_M_TYPE   F_PILOT_M_TYPE     // define in QEDFER.h
#define PILOT_H_TYPE   F_PILOT_H_TYPE     // define in QEDFER.h


/**************************************************************************
     NCO_ACCM Register       (Read/Write)  (not available in FALCON 1)
**************************************************************************/
// This register is a QEDFER block register but locates at 0x8B
typedef volatile UINT16 NCO_ACCM_TYPE;

/**************************************************************************
     TFMESS Register       (Write Only)  (available in FALCON 2)
     RFMESS Register       (Write Only)  (available in FALCON 2)
**************************************************************************/
// This register is a QEDFER block register but locates at 0x8C and 0x8D
typedef volatile struct
{
    UINT16 hi_start_tone        : 8;
    UINT16 lo_start_tone        : 8;
}FMESS_TYPE;

// T_FMESS is also used to xmit multiple tone, we have to set it to zero
#define T_FMESS_DEFAULT    {0,0}

#ifdef CO
#define R_FMESS_DEFAULT    {UP_MESS_HI_START_TONE, UP_MESS_LO_START_TONE}

#else
#define R_FMESS_DEFAULT    {DWN_MESS_HI_START_TONE, DWN_MESS_LO_START_TONE}
#endif


/**************************************************************************
     TFBYTE Control Latch     (Write-Only) (not available in FALCON 1)
**************************************************************************/
// This register is DI block register but locates at 0x90

typedef volatile struct
{
   UINT16 F_CODE             :8;
   UINT16 DELAY_DIF          :4;
   UINT16 DELAY_ENC          :4;
}TFBYTE_TYPE;

// This register will be rewritten later for the depending on RATES1 option
#define TFBYTE_DEFAULT      \
{                           \
    0x0, /* F_CODE       */ \
    0x0, /* DELAY_DIF    */ \
    0x0, /* DELAY_ENC    */ \
}


/**************************************************************************
     Transmit FIFO Initial Offset Control Latch     (Write-Only)
**************************************************************************/

// This register is DI block register but locates at 0x91
typedef volatile struct
{
    UINT16 STADDR_TAS0             :3;    // ASO FIFO ptr offset
    UINT16 STADDR_TAS1             :3;    // AS1 FIFO ptr offset
    UINT16 STADDR_TLS0             :2;    // LS0 FIFO ptr offset
    UINT16 STADDR_TLS1             :2;    // LS1 FIFO ptr offset
    UINT16                         :6;
} TXFIFO_START_CONTROL_TYPE;

#define TXFIFO_START_CONTROL_DEFAULT   {4,4,2,3}   // reset FIFO ptr


typedef volatile struct
{
    NTRTX_TYPE                NTRTX;       // 0x84       DACCTL register
    NTRRX_TYPE                NTRRX;       // 0x85       DACCTL register
    UINT16                    not_used1;   // 0x86
    POWER_THLD_TYPE           POWER_THLD;  // 0x87       FALCON 2 only QEDFER register
    PILOT_L_TYPE              PILOT_L;     // 0x88       QEDFER register
    PILOT_M_TYPE              PILOT_M;     // 0x89       QEDFER register
    PILOT_H_TYPE              PILOT_H;     // 0x8A       QEDFER register
    NCO_ACCM_TYPE             NCO_ACCM;    // 0x8B       QEDFER register
    FMESS_TYPE                T_FMESS;     // 0x8C       FALCON 2   QEDFER register
    FMESS_TYPE                R_FMESS;     // 0x8D       FALCON 2   QEDFER register
    QAM_ICR2_TYPE             QAM_ICR2;    // 0x8E       Trapper
    INTR_PROG2_TYPE           INTR_PROG2;  // 0x8F       Trapper
    TFBYTE_TYPE               TFBYTE;      // 0x90       DI register
    TXFIFO_START_CONTROL_TYPE TXFIFO_START_CONTROL;    // 0x91       DI register
}EXTRA_REG_TYPE;

typedef volatile struct
{
    UINT16          RADDR4 : 8;
    UINT16          RADDR5 : 8;
}RADDR5_TYPE;

typedef volatile struct
{
    UINT16          RADDR6 : 8;
    UINT16                 : 8;
}RADDR7_TYPE;

typedef volatile struct
{
    UINT16                  LAT_SEL;        // 0xAA  r/w
    UINT16                  not_used;       // 0xAB
    MIF_ISR2_TYPE           MIF_ISR2;       // 0xAC  r
    RSWAP_TYPE              RSWAP4;         // 0xAD  w
    RSWAP_TYPE              RSWAP5;         // 0xAE  w
    RADDR5_TYPE             RADDRL;         // 0xAF  w
    UINT16                  QAM_THLD;       // 0xB0  w
    UINT16                  QAM_SUBCH;      // 0xB1  w
    UINT16                  QAM_BER;        // 0xB2  r/w
    UINT16                  QAM_BERCTL;     // 0xB3  w
    RSWAP_TYPE              RSWAP6;         // 0xB4  w
    RADDR7_TYPE             RADDRH;         // 0xB5  w
}ENHANCE_REG_TYPE;

typedef volatile struct
{
    UINT16          ADDR5 : 8;
    UINT16          ADDR4 : 8;
}EXTRA_TADDRL_TYPE;


typedef volatile struct
{
    UINT16                 : 8;
    UINT16          ADDR6  : 8;
}EXTRA_TADDRH_TYPE;


typedef volatile struct
{
    EXTRA_TADDRL_TYPE           TADDRL;         // 0xE0  w
    EXTRA_TADDRH_TYPE           TADDRH;         // 0xE1  w
}EXTRA_SWAP_ADDR_TYPE;

typedef volatile struct
{
    SWAP_TYPE                  SWAP4;         // 0xEC  w
    SWAP_TYPE                  SWAP5;         // 0xED  w
    SWAP_TYPE                  SWAP6;         // 0xEE  w
}EXTRA_SWAPOP_TYPE;


// Trapper TIMER Block 
enum RESOLUTION_OPTION {EVERY_16CLK, EVERY_1SYM, EVERY_4SYM};
typedef volatile struct
{
    UINT16          TIMER_START  : 1;
    UINT16          RESOLUTION   : 2;
    UINT16                       : 1;  // not used
    UINT16          RESERVED     : 12; // reserved, need to set to all 1's 
}HWTIMER_TYPE;

#define HWTIMER_DEFAULT {0, EVERY_1SYM, 0xFFF}      // default timer resolution to 1 symbol

typedef volatile struct
{
    HWTIMER_TYPE            TIMER1;         // 0xFC  r/w
    HWTIMER_TYPE            TIMER2;         // 0xFD  r/w
    UINT16                  COUNTER1;       // 0xFE  r
    UINT16                  COUNTER2;       // 0xFF  r
}TIMERBLK_TYPE;
#endif
/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/mifctl.h $
 * 
 * 9     8/16/01 5:58p Lauds
 * put physical addresses for each register as comments next to each
 * register definition
 * 
 * 8     6/26/01 8:16p Lauds
 * multi-instance version integration
 * 
 * 7     4/16/01 7:58p Lauds
 * Use of Trapper timer for MIC_GetCurrentTick and MIC_ElapsedMsec
 * 
 * 6     2/23/01 3:50p Lauds
 * add volatile keyword to SPRAM type
 * 
 * 5     2/08/01 5:28p Lauds
 * New Trapper features:   We further take advantage of this queued TCR
 * write to alleviate interrupt  response requirement in tx state machine.
 * Support more than 3 channel tx and rx bit swap.  
 * 
 * 4     2/05/01 7:52p Lauds
 * Integrate Trapper changes from Harry Lau
 * 
 * 3     4/03/00 6:48p Lauds
 * New Falcon Access Wrapper for ARM compiler
 * 
 * 2     3/31/99 4:16p Lewisrc
 * 03/22/99 Pairgain Release (2.01a)
 * 
*
*****************************************************************************/
