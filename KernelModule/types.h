/************************************************************************
*  $Workfile: types.h $
*
*  $Header: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/KernelModule/types.h 11    7/05/01 4:39p L $
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
   Description:   This header file contains all commonly used data types
                  for the system.
****************************************************************************/

#ifndef TYPES_H
#define TYPES_H

#include "project.h"

#ifndef EUPHRATES
    #ifndef OS_LINUX
        #if defined(NDIS_MINIPORT_DRIVER)
            #include <ndis.h>
        #else
        #ifdef OS_NT_4
	        #include <ntddk.h>
        #else
	        #include <wdm.h>
        #endif
    #endif
#endif

#else 	//EUPHRATES
typedef unsigned char BOOLEAN;
typedef int LONGLONG;
#endif

#include "Common.h"
//enum {OFF = 0, ON};
//typedef unsigned char BOOLEAN;


enum {LITTLE_ENDIAN, BIG_ENDIAN};



#ifdef LITTLE_ENDIAN_MEMORY

#define BYTE_ORIENTATION  LITTLE_ENDIAN

#else

#define BYTE_ORIENTATION  BIG_ENDIAN

#endif

// NOTE:
// The following conditional compile switch were added since some OS such as
//  VX works already define them.  Originally they were 'typedef' or 'enum'
//  types.
#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif

#ifndef UINT8
#define UINT8   unsigned char
#endif
#ifndef UINT32
#define UINT32  unsigned long
#endif
// END NOTE

typedef signed char SINT8;
typedef signed long SINT32;

typedef void (* JOB_TYPE)(void *pDatapump);

// The following two types are defined such that generic int or unsigned int can be used
// when optimal data size of a data type can be used by the compiler (e.g. loop index)
typedef int          GEN_SINT;
typedef unsigned int GEN_UINT;


typedef UINT32 TICK_TYPE;


#ifndef NULL
#define NULL 0
#endif

#define uint32_ONE 0x80000000L
#define uint16_ONE 0x8000

#define sint32_MAX 0x7FFFFFFFL
#define sint16_MAX 0x7FFF

#define uint32_MAX 0xFFFFFFFFUL
#define uint16_MAX 0xFFFFU

#define sint32_MIN 0x00000001L
#define sint16_MIN 0x0001


typedef struct
{
   SINT16 real,imag;
}SINT16CMPLX;

typedef struct
{
   SINT32 real,imag;
}SINT32CMPLX;

/****************************************************************************
                      TIME/DATE DEFINITION
****************************************************************************/
enum DAYOFWEEK {SUNDAY,MONDAY,TUESDAY,WEDNESDAY,THURSDAY,FRIDAY,SATURDAY,DAYSINWEEK};

typedef struct
{
   UINT8 hr;
   UINT8 min;
   UINT8 sec;
} TIME_TYPE;

MAKE_RETURN_TYPE(TIME_TYPE)

#define   TIME_SIZE   (sizeof(TIME_TYPE))

#ifdef LITTLE_ENDIAN_MEMORY
// type definition for global data access via eoc or smart terminal
typedef struct
{
   SINT16 year       :16;
   UINT16 day        : 8;
   UINT16 dayofweek  : 4;
   UINT16 month      : 4;
} DATE_TYPE;

#else

typedef struct
{
   SINT16 year       :16;
   UINT16 month      : 4;
   UINT16 dayofweek  : 4;
   UINT16 day        : 8;
} DATE_TYPE;

#endif

MAKE_RETURN_TYPE(DATE_TYPE)

#define   DATE_SIZE   (sizeof(DATE_TYPE))

// This structure is defined for NMA to get time and date
typedef struct
{
   UINT16 year  ;
   UINT8 month  ;
   UINT8 day    ;
   UINT8 hr     ;
   UINT8 min    ;
   UINT8 sec    ;
}DATE_TIME_TYPE;

MAKE_RETURN_TYPE(DATE_TIME_TYPE)

#endif
/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/KernelModule/types.h $
 * 
 * 11    7/05/01 4:39p Lewisrc
 * change JOB_TYPE arg1 from ptr to DATAPUMP_CONTEXT to void to avoid
 * problem with struct not being defined and type not either due to
 * chicken/egg problem of order of includes.
 * 
 * 10    6/26/01 8:44p Lauds
 * multi-instance version integration
 * 
 * 2     6/22/01 3:14p Lauds
 * minor cleanup and bug fix in miscutil.c and terminal.c
 * 
 * 1     6/21/01 3:12p Lauds
 * Multi-instance initial version
 * 
 * 9     1/19/01 1:25p Lauhk
 * Cleanup code to reduce Euphrates warnings.
 * 
 * 8     1/09/01 10:12a Lewisrc
 * Wirespeed/Redhat had to make minor changes to DMT files to compile
 * under GNU
 * 
 * Case of includes
 * wrap include of <ndis.h>
 * TICK_TYPE 16>>>32
 * 
 * 7     9/25/00 3:58p Lauds
 * define GEN_SINT and GEN_UINT for generic signed and unsigned integer
 * types
 * 
 * 6     9/18/00 7:02p Lauhk
 * Added compiler switch to help porting to Euphrates.
 * 
 * 5     7/31/00 12:46p Boikedc
 * Added OS_NT_4 flag to include either ntddk.h or wdm.h
 * 
 * 4     5/04/00 1:15p Lewisrc
 * include wdm.h rather than ndis.h if wdm filter driver
 * 
 * 3     4/20/00 5:06p Palazzjd
 * Phase 1A changes in support of WDM filter driver.
 * 
 * 2     2/09/99 10:27a Lewisrc
 * Remove OFF,ON enum and BOOLEAN typedef since it is duplicated by
 * common.h
 * 
 * Include ndis.h and common.h for Pairgain files which do not include
 * common.h directly. We still have to remove these duplicate definitions
 * since some of the new Pairgain DMT API wrapper files include both
 * common.h and types.h.
 * 
 * 1     2/04/99 11:54a Lewisrc
 * Pairgain DMT API core files
 * 
 *    Rev 1.21   Sep 30 1998 16:01:14   laud
 * fix DATE_TYPE for big endian
 *
 *    Rev 1.20   Sep 01 1998 17:56:52   laud
 * fix minor problem in Big endian DATE_TYPE
 *
 *    Rev 1.19   Jul 07 1998 19:00:50   laud
 * big endian support for global data access
 *
 *    Rev 1.18   Jun 25 1998 22:40:08   laud
 * use of project.h
 *
 *    Rev 1.17   Jun 19 1998 19:11:48   laud
 * endian problem with CRC
 *
 *    Rev 1.16   Jun 03 1998 16:03:36   laud
 * provide big_endian capability
 *
 *    Rev 1.15   Jun 02 1998 12:15:16   laud
 * Endian dependent structure to types.h
 *
 *    Rev 1.14   May 28 1998 19:13:42   laud
 *
 *
 *    Rev 1.13   May 27 1998 19:04:56   laud
 * DMT API cleanup
 *
 *    Rev 1.12   17 Dec 1997 17:09:42   laud
 * change TICKS_PER_SEC to 10 (i.e. 100 msec period)
 *
 *    Rev 1.11   15 Dec 1997 16:06:24   phamt
 * Move defines XCVR_NUM_ADSL and XCVR_NUM_0 from xcvrinfo.h
 * to types.h
 *
 *    Rev 1.10   11 Dec 1997 10:58:58   HEJ
 * define a structure to include both date and time.
 *
 *    Rev 1.9   13 Nov 1997 10:00:36   phamt
 * DSLAM
 *
 *    Rev 1.8   06 Nov 1997 17:11:48   phamt
 * DSLAM
 *
 *    Rev 1.7   27 Oct 1997 15:54:20   phamt
 * Port to DSLAM design
 *
 *    Rev 1.6   03 Feb 1997 20:58:30   DLAU
 *
 *
 *    Rev 1.0   24 Jan 1996 11:47:28   DLAU
 * Initial revision.
*
*****************************************************************************/
/**********                end of file $Workfile: types.h $          *****************/
