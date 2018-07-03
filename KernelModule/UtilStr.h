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
**		Util (Utilities)
**
**	FILE NAME:
**		UtilStr.h
**
**	ABSTRACT:
**		Utility string manipulation functions.
**		This module contains the declarations for UtilStr.c.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/UtilStr.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
#ifndef _UTILSTR_H_		//	File Wrapper,
#define _UTILSTR_H_		//	prevents multiple inclusions



#define ASCII_HEX_BYTE_MAX_DIGIT_LEN		2
#define ASCII_HEX_WORD_MAX_DIGIT_LEN		4
#define ASCII_HEX_DWORD_DIGIT_LEN			8

#define DECIMAL_BASE_VALUE					10
#define HEXADECIMAL_BASE_VALUE				0x10

#if !defined(toupper)
#define toupper(ch)		(((ch >= 'a') && (ch <= 'z')) ? (ch-'a'+'A'):ch)
#endif

/******************************************************************************
*	MACRO NAME:
*		STR_TO_NDIS_STR
*
*	ABSTRACT:
*		This macro initializes an NDIS string (_Dest) to the zero terminated
*		wide string (_Source).
*
*	DETAILS:
*		The zero terminated wide string (_Source) should be defined like the
*		example below.
*			Ex:	#define NDIS_REG_PARAM_STRING_NAME	L"RegistryParameterName"
*		The "L" in front of the string constant is NECESSARY to make the
*		string constant a wide string!
*		Also, be sure and terminate any call to this macro with a semicolon!
******************************************************************************/
#define STR_TO_NDIS_STR(_Dest,_Source)		\
	_Dest.Length = sizeof(_Source)-2;		\
	_Dest.MaximumLength = sizeof(_Source);	\
	_Dest.Buffer = (WORD *) (_Source)



///////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
///////////////////////////////////////////////////////////////////////////////
INT UtilStrIntToAscii(
    IN int			i,
    OUT PUCHAR		szBuf,
    IN int			maxlength );

LONG UtilStrStrCmpI(
    IN PUCHAR		str1,
    IN PUCHAR		str2 );

INT UtilStrAsciiToInt(	IN PUCHAR		szAsciiNumStr );

INT UtilStrAsciiHexToByte(
    IN PUCHAR		szAsciiHexNumStr,
    IN OUT BYTE		* ByteVal );

INT UtilStrAsciiHexToWord(
    IN PUCHAR		szsciiHexNumStr,
    IN OUT WORD		* WordVal );

BOOLEAN UtilASCIItoHex(
    IN CHAR a,
    OUT PUCHAR ah );

#endif		//	#ifndef _UTILSTR_H_
