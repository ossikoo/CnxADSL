/************************************************************************
*  $Workfile: util.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/util.h 12    6/26/01 8:47p Lauds $
*
*****************************************************************************
*  Copyright 1996 PairGain Technologies as unpublished work                 *
*  All Rights Reserved                                                      *
*                                                                           *
*  The information contained herein is the confidential property            *
*  of PairGain Technologies.  The use, copying, transfer or                 *
*  disclosure of such information is prohibited except by express           *
*  written agreement with PairGain Technologies.                            *
*                                                                           *
*                                                                           *
*  Original Written By:  David S. Lau                                       *
*  Original Date: 7/29/96                                                   *
*                                                                           *
*****************************************************************************
   Description:  This module provides some common used utilities routines
     Note that all routines in the file should be reentrant code
     (or pure code).  In other words, any of these routines can be invoked
     at the same time by different processes.
****************************************************************************/

#ifndef UTIL_H
#define UTIL_H

#include "types.h"

/****************************************************************************
                      ASCII CONVERSION
****************************************************************************/
//          union to access bytes of a word - used to access earch char 
//          returned in the word from UTIL_ByteToAscii

union WORDBYTE
{
    UINT16 Word ;
    struct
    {
        #ifdef LITTLE_ENDIAN_MEMORY
            UINT8 Lower ;
            UINT8 Upper ;
        #else
            UINT8 Upper ;
            UINT8 Lower ;
        #endif
    } Bytes ;
} ;
DPCCALL UINT16 UTIL_ByteToAscii (GEN_UINT Value) ;

/****************************************************************************
                      DISCRETE POINT MATH FUNCTION DEFINITION
****************************************************************************/
DPCCALL void    UTIL_Get_ext_int16_str(char *, SINT16);
DPCCALL void    UTIL_Get_ext_int16_normal_str(char *, SINT16);
DPCCALL UINT16  UTIL_int16_abs(SINT16 x);
DPCCALL UINT32  UTIL_int32_abs(SINT32 x);
DPCCALL SINT16  UTIL_sint16_mult(SINT16 n1, SINT16 n2);
DPCCALL UINT16  UTIL_uint16_mult(UINT16 n1, UINT16 n2);
DPCCALL SINT16  UTIL_suint16_mult(SINT16 n1, UINT16 n2);
DPCCALL UINT16  UTIL_ext_uint16_mult(UINT16 n1, UINT16 n2);
DPCCALL SINT16  UTIL_ext_sint16_mult(SINT16 n1, SINT16 n2);
DPCCALL SINT32  UTIL_sint32_mult(SINT32 n1, SINT32 n2);
DPCCALL SINT32  UTIL_ext_sint32_mult(SINT32 n1, SINT32 n2);
DPCCALL SINT16  UTIL_sint16_div(SINT16 dividend, SINT16 divisor);
DPCCALL UINT16  UTIL_uint16_div(UINT16 dividend, UINT16 divisor);
DPCCALL UINT16  UTIL_ext_uint16_div(UINT16 dividend, UINT16 divisor);
DPCCALL UINT16  UTIL_uint32_div(UINT32 dividend, UINT32 divisor );
DPCCALL SINT16  UTIL_sint32_div(SINT32 dividend, SINT32 divisor );
DPCCALL UINT16  UTIL_ext_sint16_sqr(SINT16 n1);
DPCCALL UINT16  UTIL_int16_sqr(SINT16 n1);
DPCCALL UINT32  UTIL_int32_sqr(SINT32 n1);
DPCCALL void    UTIL_sint16_cmplx_mult(SINT16CMPLX n1, SINT16CMPLX n2, SINT16CMPLX * result);
DPCCALL void    UTIL_sint32_cmplx_mult(SINT32CMPLX n1, SINT32CMPLX n2, SINT32CMPLX * result);
DPCCALL void    UTIL_sint16_cmplx_div(SINT16CMPLX n1, SINT16CMPLX n2, SINT16CMPLX * result);
DPCCALL UINT32  UTIL_int32_sqrt(UINT32 A);
DPCCALL UINT16  UTIL_int16_sqrt(UINT16 A);
DPCCALL UINT16  UTIL_ext_int16_sqrt(UINT16 x);
DPCCALL UINT16  UTIL_GetDecibel(UINT16, UINT8);
DPCCALL SINT16  UTIL_GetDecibel2(UINT16, UINT8);
DPCCALL SINT16  UTIL_GetDecibel3(UINT16 x, UINT8 flag);
DPCCALL GEN_UINT  UTIL_GetBCD(GEN_UINT);
DPCCALL UINT8   UTIL_GetLargestPwr2Number(UINT8);
DPCCALL void    UTIL_ByteSwapCpy(void * dest, const void * src, UINT16 length);
DPCCALL void    UTIL_WordSwapCpy(void * dest, const void * src, UINT16 length);

// note that the floating multiplication in done in compile time
// 0.5 is added to round up the given positive number and
// 0.5 is subtracted to round up the given negative number

#define FLT2EXT_INT16(x) (SINT16)((x) * 0x100 + ((x > 0) ? 0.5 : -0.5))    // bit 0-7 decimal, bit 8-15 integer part

#define FLT2EXT_INT32(x) (SINT32)((x) * 0x10000L + ((x > 0) ? 0.5 : -0.5)) // bit 0-15 decimal, bit 16-31 integer part

#define FLT2INT16(x) (SINT16)((x) * 0x8000L + ((x > 0) ? 0.5 : -0.5))      // bit 0-14 decimal, bit 15 is signed bit or integer part

#define FLT2INT32(x) (SINT32)((x) * 0x80000000L + ((x > 0) ? 0.5 : -0.5))  // bit 0-30 decimal, bit 31 is signed bit or integer part


#define SINT32_CMPLX_MAG(n1)   \
   (UTIL_int32_sqrt(UTIL_int32_sqr((n1).real)+ UTIL_int32_sqr((n1).imag),8))
#define SINT16_CMPLX_MAG(n1)   \
   (UTIL_int16_sqrt(UTIL_int16_sqr((n1).real)+ UTIL_int16_sqr((n1).imag),4))
#define TWO_POW(x)  (2L<<(x))

#define UTIL_toupper(c) (c>='a' ? c-'a'+'A' : c)

enum
{
   SIGNAL_DB,
   POWER_DB
};

/****************************************************************************
                      COMMONLY USED MATH FUNCTION DEFINITION
****************************************************************************/
DPCCALL GEN_UINT   UTIL_Modulo(GEN_SINT a, GEN_SINT b);
DPCCALL GEN_SINT   UTIL_BinarySearch (const UINT32 *list, UINT32 item, GEN_SINT top, GEN_SINT bottom);
DPCCALL void     UTIL_ShellSortL (const SINT32 *list, UINT8 *order, UINT16 size);
DPCCALL void     UTIL_ShellSort (const SINT16 *list, UINT8 *order, UINT16 size);
DPCCALL void     UTIL_ShellSort2(UINT16 *list, UINT16 size);
DPCCALL GEN_SINT   UTIL_Max(GEN_SINT value1, GEN_SINT value2);


/****************************************************************************
                      STATIC FIFO MANAGER DEFINITION
****************************************************************************/

typedef struct
{
   UINT8    *buffer;        // Where the storage starts
   UINT16   InsertInd,      // Where to store next
            RemoveInd,      // Where to remove next
            ItemCnt,        // Number of entry in the storage
            MaxQlen;        // Total size
   UINT8    ItemSize;       // Size in bytes of each entry
}FIFO_TYPE, *pFIFO_TYPE;


#define FIFO_TYPE_SIZE  sizeof(FIFO_TYPE)

DPCCALL void     UTIL_NewQ(pFIFO_TYPE,void *,UINT16, UINT8 );

DPCCALL BOOLEAN  UTIL_IsQFull(const pFIFO_TYPE);

DPCCALL BOOLEAN  UTIL_IsQEmpty(const pFIFO_TYPE);

DPCCALL void     UTIL_ResetQ(pFIFO_TYPE);

DPCCALL BOOLEAN  UTIL_InsertQ(pFIFO_TYPE, const void *);

DPCCALL void *   UTIL_RemoveQ(pFIFO_TYPE);

/****************************************************************************
                     DYNAMIC FIFO MANAGER DEFINITION
****************************************************************************/
typedef struct
{
   UINT8    *buffer;
   UINT16   InsertInd,
            RemoveInd,
            MaxQlen,
            ItemCnt;
}DYN_FIFO_TYPE, *pDYN_FIFO_TYPE;


DPCCALL void    UTIL_NewDynQ(pDYN_FIFO_TYPE newfifo,void *buffer,UINT16 fifosize);

DPCCALL BOOLEAN UTIL_IsDynQFull(const pDYN_FIFO_TYPE, UINT8 );


DPCCALL BOOLEAN UTIL_IsDynQEmpty(const pDYN_FIFO_TYPE);
DPCCALL UINT16  UTIL_GetDynQItemCnt(const pDYN_FIFO_TYPE fifo);


DPCCALL void    UTIL_ResetDynQ(pDYN_FIFO_TYPE );

DPCCALL BOOLEAN UTIL_InsertDynQ(pDYN_FIFO_TYPE, const void *, UINT8);

DPCCALL void *  UTIL_RemoveDynQ2(pDYN_FIFO_TYPE fifo, UINT8 * size);

DPCCALL void *  UTIL_RemoveDynQ(pDYN_FIFO_TYPE);

DPCCALL UINT8    UTIL_UpdateCRC( UINT8, UINT8);

DPCCALL void    UTIL_BigEndianMemcpy(void *dest, const void *src, UINT16 n);
DPCCALL SINT32  UTIL_ext_sint32_mult(SINT32 n1, SINT32 n2);
DPCCALL GEN_UINT   UTIL_Modulo(GEN_SINT a, GEN_SINT b);
DPCCALL GEN_SINT  UTIL_BinarySearch (const UINT32 *list, UINT32 item, GEN_SINT top, GEN_SINT bottom);
DPCCALL void    UTIL_ShellSortL (const SINT32 *list, UINT8 *order, UINT16 size);
DPCCALL void    UTIL_ShellSort (const SINT16 *list, UINT8 *order, UINT16 size);
DPCCALL void    UTIL_ShellSort2(UINT16 *list, UINT16 size);
DPCCALL GEN_SINT  UTIL_Max(GEN_SINT value1, GEN_SINT value2);
DPCCALL size_t  UTIL_strncpy(UINT8 *out_data, UINT8 *in_data, size_t max_len);

DPCCALL void    UTIL_NewQ(pFIFO_TYPE,void *,UINT16, UINT8 );
DPCCALL BOOLEAN UTIL_IsQFull(const pFIFO_TYPE);
DPCCALL BOOLEAN UTIL_IsQEmpty(const pFIFO_TYPE);
DPCCALL void    UTIL_ResetQ(pFIFO_TYPE);
DPCCALL BOOLEAN UTIL_InsertQ(pFIFO_TYPE, const void *);
DPCCALL void *  UTIL_RemoveQ(pFIFO_TYPE);
DPCCALL GEN_SINT UTIL_strncmp( const char  * s1, const char  * s2, GEN_UINT n );


#endif
/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/util.h $
 * 
 * 12    6/26/01 8:47p Lauds
 * multi-instance version integration
 * 
 * 3     6/25/01 9:59a Lauds
 * merged changes
 * 
 * 2     6/22/01 3:15p Lauds
 * minor cleanup and bug fix in miscutil.c and terminal.c
 * 
 * 1     6/21/01 3:12p Lauds
 * Multi-instance initial version
 * 
 * 9     4/27/01 5:57p Lauds
 * minor bugs and warning cleanup for Euphrates
 * 
 * 8     1/19/01 1:27p Lauhk
 * Cleanup code to reduce Euphrates warnings.
 * 
 * 7     1/09/01 10:15a Lewisrc
 * Wirespeed/Redhat had to make minor changes to DMT files to compile
 * under GNU
 * 
 * Wrap toupper
 * 
 * 6     11/22/00 9:01a Lewisrc
 * Add UTIL_ByteToAscii to replace UTIL_num_const_str.
 * Add WORDBYTE definition
 * 
 * 5     9/27/00 7:57p Lauds
 * more cleanup, reduce warning messages
 * 
 * 4     9/18/00 7:03p Lauhk
 * Added compiler switch to help porting to Euphrates.
 * 
 * 3     3/02/00 11:22a Palazzjd
 * Added MACRO toupper.
 * 
 * 2     2/01/00 8:34a Lewisrc
 * 2.02i Beta (Pre HSV Interop drop)
 * 
 * 3     1/31/00 2:29p Lewisrc
 * 2.02i Beta - prior to HSV interop
 * 
 *    Rev 1.37   Jan 27 2000 19:58:04   infusij
 * - RT fast retrain update
 *
 *    Rev 1.36   Jul 07 1998 19:00:50   laud
 * big endian support for global data access
 *
 *    Rev 1.35   May 28 1998 19:13:44   laud
 *
 *
 *    Rev 1.34   May 27 1998 19:04:58   laud
 * DMT API cleanup
 *
 *    Rev 1.33   17 Feb 1998 18:03:28   laud
 * add new function to get the largest 2 power
 *
 *    Rev 1.32   29 Jan 1998 15:15:16   laud
 * add ShellSortL for sorting SINT32
 *
 *    Rev 1.31   27 Oct 1997 15:54:26   phamt
 * Port to DSLAM design
 *
 *    Rev 1.29   17 Jul 1997 11:46:32   DLAU
 * create ext_int16_sqrt
 *
 *    Rev 1.28   18 Jun 1997 14:08:48   DLAU
 * fix spelling mistake for dividend and add sint32_div
 *
 *    Rev 1.27   09 Jun 1997 11:40:44   DLAU
 * add new function for int16 division
 *
 *    Rev 1.26   22 Apr 1997 17:20:32   DLAU
 * use the new GetDecibel3 routine for gain adjustment
 *
 *    Rev 1.25   17 Apr 1997 15:18:50   DLAU
 * provide unbias rounding on all FLT2INT macros
 *
 *    Rev 1.24   04 Apr 1997 20:30:36   DLAU
 * move windowing routine to away
 * Initial revision.
*
*****************************************************************************/
/**********                end of file $Workfile: util.h $          *****************/
