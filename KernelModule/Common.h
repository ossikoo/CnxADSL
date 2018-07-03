/******************************************************************************
*******************************************************************************
****	Copyright (c) 1997, 1998
****	Rockwell Semiconductor Systems
****	Personal Computing Division
****	All Rights Reserved
****
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****
*******************************************************************************
**
**	MODULE NAME:
**		Athena
**
**	FILE NAME:
**		Common.h
**
**	ABSTRACT:
**		Definitions common to all modules/files/functions.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/Common.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
#ifndef _COMMON_H_		//	File Wrapper,
#define _COMMON_H_		//	prevents multiple inclusions


/*******************************************************************************
**                              Keywords
*******************************************************************************/
#define LOCAL static					/* Used only in the current file */
#define MODULAR							/* Used only in the current module */
#define GLOBAL							/* Used inter module */

#define IN								/* Used to denote input arguments */
#define OUT								/* Used to denote output arguments */
#define I_O								/* Used to denote input/output arguments */


/*******************************************************************************
**                              Macros
*******************************************************************************/

#define NOT_USED(x)  (x) = (x)



////////////////////////////
//	
////////////////////////////

//define OFF						0
//define ON						1
#include "LnxTools.h"

#if 0
#ifndef __MODEM_H_
#if !defined( BOOL)
typedef BOOLEAN BOOL;
#endif

typedef enum 
{
OFF,
ON
} BINARY;


typedef unsigned char			BYTE;		/* 8-bit unsigned integer  */
typedef unsigned short			WORD;		/* 16-bit unsigned integer */
typedef unsigned long			DWORD;		/* 32-bit unsigned integer */

typedef signed char				CHAR;	    /* 8-bit signed integer  */
typedef signed short int		SHORT;	    /* 16-bit signed integer */
typedef signed long int			LONG;	    /* 32-bit signed integer */

#endif

//
// Bit mask
//
#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80

#else
typedef int BINARY;
#endif
/* 	The va_names type match the type name after promotion, These can be used with 
**	the va functions to get the correct size variable off the stack. 
*/

typedef BOOLEAN VA_BOOL;

typedef BINARY VA_BINARY;

typedef unsigned char			VA_BYTE;		  /* 8-bit unsigned integer  */
typedef unsigned short			VA_WORD;		  /* 16-bit unsigned integer */
typedef unsigned long			VA_DWORD;		  /* 32-bit unsigned integer */

typedef signed char				VA_CHAR;		  /* 8-bit signed integer  */
typedef signed short int		VA_SHORT;		  /* 16-bit signed integer */
typedef signed long int			VA_LONG;		  /* 32-bit signed integer */


//
// Bit clear mask
//
#define BIT0_CLEAR		0xFE
#define BIT1_CLEAR		0xFD
#define BIT2_CLEAR		0xFB
#define BIT3_CLEAR		0xF7
#define BIT4_CLEAR		0xEF
#define BIT5_CLEAR		0xDF
#define BIT6_CLEAR		0xBF
#define BIT7_CLEAR		0x7F



// counters to be used for statistics  
// each counter has a flag to track if the counter has overflowed.

typedef struct
{
	BOOL	Overflow ;
	BYTE	Cnt ;
} BYTE_CTR_T ;

typedef struct
{
	BOOL Overflow ;
	WORD Cnt ;
} WORD_CTR_T ;

typedef struct
{
	BOOL Overflow ;
	LONG Cnt ;
} LONG_CTR_T;

typedef struct
{
	BOOL Overflow ;
	LONGLONG Cnt ;
} LONGLONG_CTR_T;


// Macros used to update statistics counters
// These macors also check for overflow.

#define ADD_TO_CTR_TYPE(max, counter, delta) \
	do \
	{ \
		if ((counter)->Cnt > (max) - (delta)) \
			(counter)->Overflow = TRUE; \
		(counter)->Cnt += (delta); \
	} \
	while (0)

static inline void AddToCtrBYTE(BYTE_CTR_T *counter, BYTE delta) { ADD_TO_CTR_TYPE(BYTE_MAX, counter, delta); }
static inline void AddToCtrWORD(WORD_CTR_T *counter, WORD delta) { ADD_TO_CTR_TYPE(WORD_MAX, counter, delta); }
static inline void AddToCtrLONG(LONG_CTR_T *counter, LONG delta) { ADD_TO_CTR_TYPE(LONG_MAX, counter, delta); }
static inline void AddToCtrLONGLONG(LONGLONG_CTR_T *counter, LONGLONG delta) { ADD_TO_CTR_TYPE(LONGLONG_MAX, counter, delta); }

// Returns size in bytes rounded up to the nearest DWORD
#define SIZEOF_DWORD_ALIGN(n)	((sizeof(n) + 3) >> 2 << 2)
//#pragma intrinsic(memcmp)
#endif		//#ifndef _COMMON_H_
