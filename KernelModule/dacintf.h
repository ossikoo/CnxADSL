/************************************************************************
*  $Workfile: dacintf.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/dacintf.h 8     8/16/01 5:53p Lauds $
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
      the Digital to Analog Conversion Interface (DAC) block of the FALCON.
*************************************************************************/

#ifndef DACINTF_H
#define DACINTF_H

#include "sysinfo.h"
#include "types.h"

/**************************************************************************
     DAC Interface Control Latch         (Write Only)
**************************************************************************/
enum GAINCHOICE {NO_GAIN, GAINx4, GAINx16, GAINx64};
enum DACMODECHOICE {DAC_NORMAL, DAC_TEST1, DAC_TEST2, DAC_TEST3, DAC_TEST4};

typedef volatile struct
{
   UINT16 NTR                :2;
   UINT16 GAIN               :2;
   UINT16 DAC_PASS           :3;
   UINT16 OFFSET_DAC         :1;
   UINT16 SAT_TX             :1;
   UINT16 INV_DACCLK         :1;
   UINT16 INTERPOLATION      :2;    // Falcon 1Z
   UINT16 ROUNDING           :1;    // Falcon 1Z
   UINT16 SERIAL_EN          :1;    // Falcon 1Z
   UINT16 DA_BIT             :2;    // Falcon 1Z
}DACCTLL_TYPE;

typedef volatile union
{
   struct
   {
        UINT16 DAC_START        :11;
        UINT16 FCLK_FEQ         :1;
        UINT16 FCLK_PHB         :1;
        UINT16                  :1;
        UINT16 SHIFT_CNT        :2;
   }common;
   struct
   {
        UINT16                  :13;
        UINT16 SHIFT_CLK        :1; // Shift Clk Freq. (0-mclk, 1-mclk/2)
        UINT16                  :2;
   }falcon2p;
   struct
   {
        UINT16                  :13;
        UINT16 MAPPER_EN        :1;
        UINT16                  :2;
   }falcon2;
}DACCTLH_TYPE;
/* DAC_START is a table look-up value from pg 65 (sec 5.3) in FALCON SPEC */
#define DA_BIT  (AFE_NUM_OF_BIT<=12)  ? 0 :   \
                ((AFE_NUM_OF_BIT==13) ? 1 :   \
                ((AFE_NUM_OF_BIT==14) ? 2 : 3))

//Parallel or serial AFE interface
#ifdef CONEXANT_AFE
#define DAC_SERAIL_EN   1
#else
#define DAC_SERAIL_EN   0
#endif

#ifdef ECHO_TRAINING
#define INTERPOLATION       3
#else
#define INTERPOLATION       0
#endif

#ifdef CO
#define DACCTLL_DEFAULT       {ASIC_CONVERT_CHNL_SIZE(NIFFT_VALUE), /* NTR        */ \
                               GAINx4,                              /* GAIN       */ \
                               0,                                   /* DAC_PASS   */ \
                              (DAC_MODE == OFFSET_BINARY)? 1:0,     /* OFFSET_DAC */ \
                               0,                                   /* SAT_TX     */ \
                               1,                                   /* INV_DACCLK */ \
                               INTERPOLATION,                       /* 1Z Interpolation */ \
                               1,                                   /* Rounding   */ \
                               DAC_SERAIL_EN,                       /* serial_en  */ \
                               DA_BIT}                              /* DA_BIT     */
#else
#define DACCTLL_DEFAULT       {ASIC_CONVERT_CHNL_SIZE(NIFFT_VALUE), /* NTR        */ \
                               GAINx16,                             /* GAIN       */ \
                               0,                                   /* DAC_PASS   */ \
                              (DAC_MODE == OFFSET_BINARY)? 1:0,     /* OFFSET_DAC */ \
                               0,                                   /* SAT_TX     */ \
                               1,                                   /* INV_DACCLK */ \
                               INTERPOLATION,                       /* 1Z Interpolation */ \
                               1,                                   /* Rounding   */ \
                               DAC_SERAIL_EN,                       /* serial_en  */ \
                               DA_BIT}                              /* DA_BIT     */
#endif

#ifdef CONEXANT_AFE
#ifdef CO
#define DACCTLH_DEFAULT       {{ASIC_DAC_START_ADDR(NIFFT_VALUE),0,1,2}}
#else
#define DACCTLH_DEFAULT       {{ASIC_DAC_START_ADDR(NIFFT_VALUE),0,1,3}}
#endif
#else
#define DACCTLH_DEFAULT       {{ASIC_DAC_START_ADDR(NIFFT_VALUE),0,1,0}}
#endif  // End ifdef CONEXANT_AFE


/**************************************************************************
     DAC FIFO Control Latch         (Write Only)
**************************************************************************/

typedef volatile union
{
   struct
   {
       UINT16 COARSE_INCR4       :1;
       UINT16 COARSE_DECR4       :1;
       UINT16 COARSE_INCR1       :1;
       UINT16 COARSE_DECR1       :1;
       UINT16 FINE_INCR          :1;
       UINT16 FINE_DECR          :1;
       UINT16 FIFO_RST           :1;
       UINT16                    :2;
       UINT16 EC_DELAY           :7;
   }common;
   struct
   {
       UINT16                    :7;
       UINT16 WCLK_RATE          :1;
       UINT16                    :8;
   }falcon2p;
   struct
   {
       UINT16                    :7;
       UINT16 HALF_RATE          :1;
       UINT16                    :8;
   }falcon2;
}FIFOCTL_TYPE;

#define FIFOCTL_DEFAULT       {{0,0,0,0,0,0,0,0}} // common field


typedef volatile struct
{
    DACCTLL_TYPE  DACCTLL;           // 0x3B 
    DACCTLH_TYPE  DACCTLH;           // 0x3C 
    FIFOCTL_TYPE  FIFOCTL;           // 0x3D 
}DACINTF_TYPE;


#endif
/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/dacintf.h $
 * 
 * 8     8/16/01 5:53p Lauds
 * put physical addresses for each register as comments next to each
 * register definition
 * 
 * 7     6/26/01 7:48p Lauds
 * multi-instance version integration
 * 
 * 6     11/09/99 9:30a Lewisrc
 * 2.02f code release from Pairgain
 * 
 *    Rev 1.27   Oct 26 1999 16:38:28   yangl
 * -- Stop Support Falcon 1Z
 * -- Dynamic config with Falcon2 & 2P
 * 
 * 
 *    Rev 1.26   Jul 29 1999 16:56:16   nordees
 * Falcon 2P support.
 *
 *    Rev 1.25   Apr 30 1999 16:34:14   nordees
 * Added Echo Cancellation code (disabled).
 *
 *    Rev 1.24   Apr 23 1999 17:09:42   nordees
 * Conexant AFE support.
 *
 *    Rev 1.23   Jan 07 1999 15:08:04   laud
 * FALCON_2 fixes
 *
 *    Rev 1.22   Sep 02 1998 11:44:50   laud
 * FALCON 1Z fixes
 *
 *    Rev 1.21   01 Sep 1998 12:47:30   tsaurl
 * Put Falcon 1Z default value
 *
 *    Rev 1.20   Jul 29 1998 14:15:50   laud
 * OFFSET_DAC based on project.h
 *
 *    Rev 1.19   28 Jul 1998 16:36:24   tsaurl
 * update for falcon 1z
 *
 *    Rev 1.21   27 Jul 1998 16:42:52   laud
 *
 *
 *    Rev 1.20   27 Jul 1998 16:37:32   laud
 *
 *
 *    Rev 1.19   27 Jul 1998 16:16:52   laud
 *
 *
 *    Rev 1.18   15 May 1998 17:05:56   laud
 * DSLAM integration again
 *
 *    Rev 1.17   15 Oct 1997 19:02:56   laud
 * DSLAM portability
 *
 *    Rev 1.16   09 Oct 1997 12:52:24   laud
 *
 *
*
*****************************************************************************/
