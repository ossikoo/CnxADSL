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
**		UtilStr.c
**
**	ABSTRACT:
**		Utility string manipulation functions.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/UtilStr.c $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/



#include "CardMgmtV.h"


/******************************************************************************
*	FUNCTION NAME:
*		UtilStrIntToAscii
*
*	ABSTRACT:
*		This function converts the specified source string
*		into an 16 bit signed value.
*
*	RETURN:
*		The 16 bit value
******************************************************************************/
INT UtilStrIntToAscii(
    IN int			i,
    OUT PUCHAR		szBuf,
    IN int			maxlength )
{
	int		Tmp,
	length;

	*szBuf = '\0';

	//	We don't do negatives...
	if ( i < 0 )
	{
		return 0;
	}

	//	we want padding to fill three digits.
	length = 3;

	if ( length > (maxlength - 1) )
	{
		return 0;
	}

	szBuf += length;
	*szBuf-- = '\0';
	for ( Tmp = length;   Tmp > 0;   Tmp-- )
	{
		*szBuf-- = (i % DECIMAL_BASE_VALUE) + '0';
		i = i / DECIMAL_BASE_VALUE;
	}

	return length;
}



/******************************************************************************
*	FUNCTION NAME:
*		UtilStrStrCmpI
*
*	ABSTRACT:
*		This function compares two strings ignoring case sensitivity.
*
*	RETURN:
*		The 32 bit value
*			0 = false, strings are different
*			1 = true, stings are identical
******************************************************************************/
LONG UtilStrStrCmpI(
    IN PUCHAR		str1,
    IN PUCHAR		str2 )
{
	ULONG		len = 0;

	if ( str1 == str2 )
	{
		return(0);
	}
	if ( (str1 == NULL)  ||  (str2 == NULL) )
	{
		return(-1);
	}

	while
	(
	    (toupper( str1[len] ) == toupper( str2[len] ))
	    &&
	    (str1[len] != '\0')
	    &&
	    (str2[len] != '\0')
	)
	{
		len++;
	}

	if ( toupper( str1[len] )  ==  toupper( str2[len] ) )
	{
		return(0);
	}
	else if ( toupper( str1[len] )  <  toupper( str2[len] ) )
	{
		return(-1);
	}
	else
	{
		return(1);
	}

}	// end of __strcmpi



/******************************************************************************
*	FUNCTION NAME:
*		UtilStrAsciiToInt
*
*	ABSTRACT:
*		This function converts an ASCII string to an integer value.
*
*	RETURN:
*		The integer value represented by the ASCII string.
******************************************************************************/
INT UtilStrAsciiToInt(	IN PUCHAR		szAsciiNumStr )
{
	int		RetVal = 0;

	while ( (*szAsciiNumStr >= '0')  &&  (*szAsciiNumStr <= '9') )
	{
		RetVal = (RetVal * DECIMAL_BASE_VALUE) + (int) (*szAsciiNumStr - '0');
		szAsciiNumStr++;
	}

	return RetVal;
}



/******************************************************************************
*	FUNCTION NAME:
*		UtilStrAsciiHexToByte
*
*	ABSTRACT:
*		This function converts an ASCII hexadecimal string to a byte (unsigned
*		8-bit value.
*
*	RETURN:
*		function return -	The number of ASCII hexadecimal digits converted.
*		parameter ByteVal -	The byte value represented by the ASCII hexadecimal
*							string.
******************************************************************************/
INT UtilStrAsciiHexToByte(
    IN PUCHAR		szAsciiHexNumStr,
    IN OUT BYTE		* ByteVal )
{
	BYTE		AsciiOffset;
	BYTE		HexBaseAdder;
	BYTE		TmpByteVal =			0;
	INT			AsciiHexDigitCnt =		0;
	BOOLEAN		ContConv =				TRUE;
	BOOLEAN		ConvComplete =			FALSE;


	while ( ContConv == TRUE )
	{
		if ( (*szAsciiHexNumStr >= '0')  &&  (*szAsciiHexNumStr <= '9') )
		{
			AsciiOffset = '0';
			HexBaseAdder = 0;
		}
		else if ( (*szAsciiHexNumStr >= 'A')  &&  (*szAsciiHexNumStr <= 'F') )
		{
			AsciiOffset = 'A';
			HexBaseAdder = 0xA;
		}
		else if ( (*szAsciiHexNumStr >= 'a')  &&  (*szAsciiHexNumStr <= 'f') )
		{
			AsciiOffset = 'a';
			HexBaseAdder = 0xA;
		}
		else
		{
			ContConv = FALSE;
			//x			if ( AsciiHexDigitCnt == ASCII_HEX_BYTE_MAX_DIGIT_LEN )
			//x			{
			//x				ConvComplete = TRUE;
			//x			}
			ConvComplete = TRUE;
			break;
		}
		AsciiHexDigitCnt++;
		if ( AsciiHexDigitCnt <= ASCII_HEX_BYTE_MAX_DIGIT_LEN )
		{
			TmpByteVal =
			    (TmpByteVal * HEXADECIMAL_BASE_VALUE)
			    + (BYTE) ((BYTE) (*szAsciiHexNumStr) - AsciiOffset + HexBaseAdder);
			szAsciiHexNumStr++;
		}
		else
		{
			ContConv = FALSE;
			break;
		}
	}

	if ( ConvComplete == TRUE )
	{
		*ByteVal = TmpByteVal;
	}
	else
	{
		*ByteVal = 0;
	}

	return AsciiHexDigitCnt;
}



/******************************************************************************
*	FUNCTION NAME:
*		UtilStrAsciiHexToWord
*
*	ABSTRACT:
*		This function converts an ASCII hexadecimal string to a word (unsigned
*		16-bit value.
*
*	RETURN:
*		function return -	The number of ASCII hexadecimal digits converted.
*		parameter WordVal -	The word value represented by the ASCII hexadecimal
*							string.
******************************************************************************/
INT UtilStrAsciiHexToWord(
    IN PUCHAR		szAsciiHexNumStr,
    IN OUT WORD		* WordVal )
{
	BYTE		AsciiOffset;
	BYTE		HexBaseAdder;
	WORD		TmpWordVal =			0;
	INT			AsciiHexDigitCnt =		0;
	BOOLEAN		ContConv =				TRUE;
	BOOLEAN		ConvComplete =			FALSE;


	while ( ContConv == TRUE )
	{
		if ( (*szAsciiHexNumStr >= '0')  &&  (*szAsciiHexNumStr <= '9') )
		{
			AsciiOffset = '0';
			HexBaseAdder = 0;
		}
		else if ( (*szAsciiHexNumStr >= 'A')  &&  (*szAsciiHexNumStr <= 'F') )
		{
			AsciiOffset = 'A';
			HexBaseAdder = 0xA;
		}
		else if ( (*szAsciiHexNumStr >= 'a')  &&  (*szAsciiHexNumStr <= 'f') )
		{
			AsciiOffset = 'a';
			HexBaseAdder = 0xA;
		}
		else
		{
			ContConv = FALSE;
			//x			if ( AsciiHexDigitCnt == ASCII_HEX_WORD_MAX_DIGIT_LEN )
			//x			{
			//x				ConvComplete = TRUE;
			//x			}
			ConvComplete = TRUE;
			break;
		}
		AsciiHexDigitCnt++;
		if ( AsciiHexDigitCnt <= ASCII_HEX_WORD_MAX_DIGIT_LEN )
		{
			TmpWordVal =
			    (TmpWordVal * HEXADECIMAL_BASE_VALUE)
			    + (WORD) ((BYTE) (*szAsciiHexNumStr) - AsciiOffset + HexBaseAdder);
			szAsciiHexNumStr++;
		}
		else
		{
			ContConv = FALSE;
			break;
		}
	}

	if ( ConvComplete == TRUE )
	{
		*WordVal = TmpWordVal;
	}
	else
	{
		*WordVal = 0;
	}

	return AsciiHexDigitCnt;
}

/******************************************************************************
*	FUNCTION NAME:
*		UtilASCIItoHex
*
*	ABSTRACT:
*		This function checks the validity of 'a' to see if it is a legal
*		character representation of a hexidecimal number.
*
*	RETURN:
*		BOOLEAN:
*			TRUE -	Is a valid hexidecimal character.
*					
*			FALSE -	Not valid.
*
*		OUT *ah:
*			Returns hex value of the converted char.
*
*	DETAILS:
******************************************************************************/
BOOLEAN UtilASCIItoHex(
    IN CHAR a,
    OUT PUCHAR ah )
{
	BOOLEAN isHexAlpha = FALSE;

	if(a >= 'a' && a <= 'f') {
		*ah = a - 87;
		isHexAlpha = TRUE;

	} else if (a >= 'A' && a <= 'F') {
		*ah = a - 55;
		isHexAlpha = TRUE;

	} else if (a >= '0' && a <= '9') {
		*ah = a - 48;
		isHexAlpha = TRUE;

	} else if (a == ' ') {
		isHexAlpha = TRUE;

	} else {
		isHexAlpha = FALSE;
	}

	return isHexAlpha;
}


