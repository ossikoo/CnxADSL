/************************************************************************
*  $Workfile: falcon.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/falcon.h 9     6/26/01 8:02p Lauds $
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
   Description: This header file includes all the header files that contain
      the register definitions in different blocks of the FALCON.
*************************************************************************/
#ifndef FALCON_H
#define FALCON_H

#include "sysinfo.h"
#include "micro.h"
#include "fftblk.h"
#include "ateblk.h"
#include "qedfer.h"
#include "dacintf.h"
#include "diblk.h"
#include "mifctl.h"
#include "seqblk.h"
#include "seqblk.h"
#include "atmecblk.h"


/**************************************************************************
     Internal RAM Block Addresses         (Read/Write)
**************************************************************************/

enum{Q_RCVR_SP_I = 0x300,             // 0x300
     Q_RCVR_SP_Q,                     // 0x301
     Q_TX_OUT,                        // 0x302
     Q_FEQC_I,                        // 0x303
     Q_FEQC_Q,                        // 0x304
     Q_TX_BIT_TONE,                   // 0x305
     Q_RX_BIT_TONE,                   // 0x306
     Q_RX_TONE_INFO,                  // 0x307
     Q_ERR_AVG,                       // 0x308
     Q_RX_GAIN,                       // 0x309
     Q_UEQ_AVG_I,                     // 0x30A
     Q_UEQ_AVG_Q,                     // 0x30B
     FFTA_SP_LI = 0x310,              // 0x310
     FFTA_SP_HI,                      // 0x311
     FFTA_SP_LQ,                      // 0x312
     FFTA_SP_HQ,                      // 0x313
     FFTA_SP_I,                       // 0x314
     FFTA_SP_Q,                       // 0x315
     FFTA_GAIN,                       // 0x316
     FFTA_ROM,                        // 0x317
     FFTB_SP_LI = 0x320,              // 0x320
     FFTB_SP_HI,                      // 0x321
     FFTB_SP_LQ,                      // 0x322
     FFTB_SP_HQ,                      // 0x323
     FFTB_SP_I,                       // 0x324
     FFTB_SP_Q,                       // 0x325
     FFTB_GAIN,                       // 0x326
     FFTB_ROM,                        // 0x327
     DAC_INTF_1 = 0x330,              // 0x330
     DAC_INTF_2,                      // 0x331
     DAC_INTF_3,                      // 0x332
     DAC_INTF_4,                      // 0x333
     DAC_INTF_5,                      // 0x334
     DAC_TX_COEF,                     // 0x335
     DAC_TX_DATA,                     // 0x336
     ATE_FIR_TEQ_COEF_NOT_USED =0x340,// 0x340  this address shouldn't be burst read/write directly
     ATE_FIR_DATA,                    // 0x341
     ATE_TEQ_DATA,                    // 0x342
     ATE_FIR_TEQ_SHCOEF,              // 0x343
     ATE_SYM_1,                       // 0x344
     ATE_SYM_2,                       // 0x345
     ATE_SYM_3,                       // 0x346
     ATE_SYM_4,                       // 0x347
     ATE_SYM_5,                       // 0x348
     ATE_PREFILTER_COEF,              // 0x349  available in FALCON 1A
     ATE_PREFILTER_DATA,              // 0x34A  available in FALCON 1A
     DI_OUT_FIFO_L = 0x350,           // 0x350
     DI_OUT_FIFO_H,                   // 0x351
     DI_IN_FIFO_L,                    // 0x352
     DI_IN_FIFO_H,                    // 0x353
     ASLS_FIFO_A_B,                   // 0x354
     ASLS_LSTXRX_FIFO,                // 0x355
     DI_INTF_FIFO_L,                  // 0x356
     DI_INTF_FIFO_H,                  // 0x357
     DI_TAOC_FIFO,                    // 0x358
     DI_RAOC_FIFO,                    // 0x359
     DI_FEC_DEC_IBUF_L,               // 0x35A
     DI_FEC_DEC_IBUF_H,               // 0x35B
     DI_FEC_DEC_FBUF,                 // 0x35C
     EC_COEF = 0x360,                 // 0x360
     EC_DATA,                         // 0x361
     VITERBI_TRACE_BACK = 0x370,      // 0x370
     VITERBI_DATA,                    // 0x371
     VITERBI_METRIC                   // 0x372
};


typedef struct
{
    QEDFER_TYPE          far *  QEDFER;
    FFTBLK_TYPE          far *  FFTA;
    FFTBLK_TYPE          far *  FFTB;
    DACINTF_TYPE         far *  DACINTF;
    ATEBLK_TYPE          far *  ATEBLK;
    DIBLK_TYPE           far *  DIBLK;
    SEQBLK_TYPE          far *  SEQBLK;
    MIFBLK_TYPE          far *  MIFBLK;
    EXTRA_REG_TYPE       far *  EXTRA_REG;
    ATMBLK_TYPE          far *  ATMBLK;
    ENHANCE_REG_TYPE     far *  ENHANCE_REG;
    EXTRA_SWAP_ADDR_TYPE far *  EXTRA_SWAP_ADDR;
    EXTRA_SWAPOP_TYPE    far *  EXTRA_SWAPOP;
    TIMERBLK_TYPE        far *  TIMERBLK;
}FALCON_REG_TYPE;


typedef volatile struct
{
    FALCON_REG_TYPE      falcon_reg;
    SPRAM_TYPE           far *  SPRAM;
    QEDFER_TYPE          QEDFER;
    FFTBLK_TYPE          FFTA;
    FFTBLK_TYPE          FFTB;
    DACINTF_TYPE         DACINTF;
    ATEBLK_TYPE          ATEBLK;
    DIBLK_TYPE           DIBLK;
    SEQBLK_TYPE          SEQBLK;
    MIFBLK_TYPE          MIFBLK;
    EXTRA_REG_TYPE       EXTRA_REG;
    ATMBLK_TYPE          ATMBLK;
    ENHANCE_REG_TYPE     ENHANCE_REG;
    EXTRA_SWAP_ADDR_TYPE EXTRA_SWAP_ADDR;
    EXTRA_SWAPOP_TYPE    EXTRA_SWAPOP;
    TIMERBLK_TYPE        TIMERBLK;
}FALCON_TYPE;


DPCCALL void FAL_MoveReg(UINT16 * dest, const UINT16 * src);

// since we don't need to support anything before FALCON2P. We can just check for 2P and 2PT
#define CHIP_ID_AFTER_F2PT \
   (pDatapump->falcon.QEDFER.QAM_RSR.CHIP_ID != FALCON2P && \
    pDatapump->falcon.QEDFER.QAM_RSR.CHIP_ID != FALCON2PT )


#ifdef EUPHRATES
    #define FALCON_REG_WRITE(pDatapump, subsys, reg) \
    {                                               \
        FAL_MoveReg((UINT16 *)&pDatapump->falcon.falcon_reg.subsys->reg,(UINT16 *)&pDatapump->falcon.subsys.reg); \
    }

    #define FALCON_REG_READ(pDatapump, subsys, reg)  \
    {                                               \
        FAL_MoveReg((UINT16 *)&pDatapump->falcon.subsys.reg, (UINT16 *)&pDatapump->falcon.falcon_reg.subsys->reg); \
    }

    // As needed in Trapper, this macro writes to a Trapper FFT register 2 times, with 0 and 1 values in FFTCTL.TOGGLE bit.
    #define TRAPPER_FFT_REG_WRITE(pDatapump, subsys, reg) \
    {                                               \
            FAL_MoveReg((UINT16 *)&pDatapump->falcon.falcon_reg.subsys->reg,(UINT16 *)&pDatapump->falcon.subsys.reg); \
            if (CHIP_ID_AFTER_F2PT)             \
            {       \
                pDatapump->falcon.subsys.FFTCTL.TOGGLE = ~(pDatapump->falcon.subsys.FFTCTL.TOGGLE); \
                FALCON_REG_WRITE(pDatapump, subsys, FFTCTL);    \
                FAL_MoveReg((UINT16 *)&pDatapump->falcon.falcon_reg.subsys->reg,(UINT16 *)&pDatapump->falcon.subsys.reg); \
                pDatapump->falcon.subsys.FFTCTL.TOGGLE = ~(pDatapump->falcon.subsys.FFTCTL.TOGGLE); \
                FALCON_REG_WRITE(pDatapump, subsys, FFTCTL);    \
            }       \
    }

#else // NOT EUPHRATES

    // P46 PCI Burst Fix
    #ifdef P46
        // P46 does not support PCI burst mode correctly. PC PCI chip may collect successive
        // incrementing accesses into a burst, exposing this bug. To prevent this, we must
        // insure there are no successive incrementing accesses. Reading any external location 
        // between each write accomplishes this. A semicolon is intentionally put at the end to
        // avoid someone calling this macro to do a write (e.g. P46_PCI_BURST_FIX(pDatapump) = x)
        #define P46_PCI_BURST_FIX(pDatapump)   pDatapump->falcon.falcon_reg.QEDFER->QAM_RSR;
    #else
        #define P46_PCI_BURST_FIX(pDatapump)
    #endif



        #define FALCON_REG_WRITE(pDatapump, subsys, reg)                            \
        {                                                                          \
            pDatapump->falcon.falcon_reg.subsys->reg = pDatapump->falcon.subsys.reg; \
            P46_PCI_BURST_FIX(pDatapump);                                           \
        }

        #define FALCON_REG_READ(pDatapump, subsys, reg)                             \
        {                                                                          \
            pDatapump->falcon.subsys.reg = pDatapump->falcon.falcon_reg.subsys->reg; \
        }

        #define TRAPPER_FFT_REG_WRITE(pDatapump, subsys, reg) \
        {            \
            pDatapump->falcon.falcon_reg.subsys->reg = pDatapump->falcon.subsys.reg; \
            P46_PCI_BURST_FIX(pDatapump);                                           \
            if (CHIP_ID_AFTER_F2PT)             \
            {       \
                pDatapump->falcon.subsys.FFTCTL.TOGGLE = ~(pDatapump->falcon.subsys.FFTCTL.TOGGLE); \
                FALCON_REG_WRITE(pDatapump, subsys, FFTCTL);    \
                pDatapump->falcon.falcon_reg.subsys->reg = pDatapump->falcon.subsys.reg; \
                pDatapump->falcon.subsys.FFTCTL.TOGGLE = ~(pDatapump->falcon.subsys.FFTCTL.TOGGLE); \
                FALCON_REG_WRITE(pDatapump, subsys, FFTCTL);    \
            }       \
        }



    #endif

DPCCALL void            FAL_ResetFalconReg(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void            FAL_InitFalcon(PDATAPUMP_CONTEXT pDatapump);
DPCCALL SYS_ST_RET_TYPE FAL_FalconTest(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void            FAL_GotoPwrDwnMode(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void            Auto_AutoSenseExtendNoToneTimeout (PDATAPUMP_CONTEXT pDatapump, BOOLEAN autosense_extension ) ;
DPCCALL void FAL_PokeFalconReg(PDATAPUMP_CONTEXT pDatapump, UINT16 address, UINT16 data);
DPCCALL UINT16 FAL_PeekFalconReg(PDATAPUMP_CONTEXT pDatapump, UINT16 address);
DPCCALL void    InitATEBLKreg(PDATAPUMP_CONTEXT pDatapump);
extern const far QEDFER_TYPE     QEDFER_reg_default;
extern const char * far falcon_id_str[8];

#endif  // file wrapper
/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/falcon.h $
 * 
 * 9     6/26/01 8:02p Lauds
 * multi-instance version integration
 * 
 * 2     6/22/01 3:10p Lauds
 * minor cleanup and bug fix in miscutil.c and terminal.c
 * 
 * 1     6/21/01 3:08p Lauds
 * Multi-instance initial version
 * 
 * 8     4/16/01 7:55p Lauds
 * Use of Trapper timer for MIC_GetCurrentTick and MIC_ElapsedMsec
 * 
 * 7     2/23/01 3:46p Lauds
 * redo P46_PCI_BURST_FIX by reading somewhere other than MIF SPRAM
 * 
 * 6     2/12/01 3:08p Lauds
 * change >= to != in macro CHIP_ID_AFTER_F2PT
 * 
 * 5     2/05/01 7:50p Lauds
 * Integrate Trapper changes from Harry Lau
 * 
 * 4     7/14/00 12:11p Lewisrc
 * FALCON_REG_WRITE should always read a falcon location as well as doing
 * the requested write so as to prevent two sucessive writes that might
 * be consecutive from triggering the P46 PCI burst write bug.
 * 
 * 3     5/01/00 12:49p Lauds
 * Euphrates need a routine for FALCON registers
 * 
 * 2     4/03/00 5:54p Lauds
 * New Falcon Access Wrapper for ARM compiler
 * 
 * 1     2/04/99 11:52a Lewisrc
 * Pairgain DMT API core files
 * 
 *    Rev 1.13   Dec 21 1998 19:44:40   laud
 * support peeking and poking new FALCON2 reg
 *
 *    Rev 1.12   Sep 21 1998 18:48:24   laud
 * EC coef data update capability
 *
 *    Rev 1.11   Aug 07 1998 17:08:44   laud
 * FALCON 1Z support
 *
 *    Rev 1.10   Aug 04 1998 17:12:46   laud
 *
 *
 *    Rev 1.9   Jun 30 1998 14:27:28   laud
 * avoid byte access for FALCON reg to workaround MIPS compiler bug
 *
 *    Rev 1.8   Jun 25 1998 23:51:36   laud
 * move FALCON_START_ADDR macro from falcon.h to project.h
 *
 *    Rev 1.7   18 May 1998 19:29:20   laud
 * DSLAM integration
 *
 *    Rev 1.6   15 May 1998 17:06:04   laud
 * DSLAM integration again
 *
 *    Rev 1.5   10 Feb 1997 13:46:44   DLAU
 *
 *    Rev 1.4   31 Jan 1997 17:35:34   DLAU
 *
 *
 *    Rev 1.3   29 Jan 1997 14:39:10   DLAU
 *
 *    Rev 1.2   29 Jan 1997 14:25:04   HE
 * No change.
 *
 *    Rev 1.1   01 Dec 1996 17:56:10   DLAU
 *
 *
 *    Rev 1.0   20 Nov 1996 18:24:08   DLAU
 * Initial revision.
*
*****************************************************************************/



