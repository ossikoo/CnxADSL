/******************************************************************************
*******************************************************************************
****	Copyright (c) 1998
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
**		HEC (Header Error Control)
**
**	FILE NAME:
**		HEC.c
**
**	ABSTRACT:
**		This files contains functions to implement HEC.
**
**	DETAILS:
**  	Code was obtained from an internet web site, orginal auther was Charles Micheal Heard.
**      The code did not seem to contain any copyright notices.
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/FrameALHec.c $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/


// include files
#include "CardMgmt.h"
#include "FrameALHec.h"

// local data and defines
/********************************************************************/
#define HEC_GENERATOR   0x107               /* x^8 + x^2 +  x  + 1  */
#define COSET_LEADER    0x055               /* x^6 + x^4 + x^2 + 1  */


#define NO_ERROR_DETECTED   -128
#define UNCORRECTIBLE_ERROR  128

static int err_posn_table[256];
static unsigned char syndrome_table[256];


GLOBAL void  FrameALHECInit
(
    void
)
/* generate a table of CRC-8 syndromes for all possible input bytes */
{

	WORD i, j, k, syndrome;
	BYTE hec_accum;

	for ( i = 0;  i < 256;  i++ )
	{
		syndrome = i;
		for ( j = 0;  j < 8;  j++ )        {
			if ( syndrome & 0x80 )
				syndrome = ( syndrome << 1 ) ^ HEC_GENERATOR;
			else
				syndrome = ( syndrome << 1 );        }
		syndrome_table[i] = (unsigned char) syndrome;
	}

	/* Generate the error position table as a function of the syndrome. */
	/* A negative value indicates that there is no error to correct.    */
	/* A value of 40 or greater indicates that an uncorrectible error   */
	/* has occurred, i.e., that the syndrome either indicates a double  */
	/* error pattern or points to a single-bit error position that is   */
	/* outside the header.  Finally, a value between 0 and 39 indicates */
	/* that a single-bit error occurred in that position (bit 0 being   */
	/* the header bit which was transmitted first).                     */

	/* mark the NO_ERROR_DETECTED position */
	err_posn_table[0] = NO_ERROR_DETECTED;

	/* default remaining positions to UNCORRECTIBLE */
	for ( i = 1;  i < 256;  i++ )
		err_posn_table[i] = UNCORRECTIBLE_ERROR;
	/* override w/err posn for syndromes indicating other single-bit errors */
	for ( i = 0;  i < 5;  i++ )
	{
		for ( j = 0;  j < 8;  j++ )
		{
			if ( i < 4)
			{
				hec_accum = 0;
				for ( k = 0;  k < 4;  k++ )
					if ( k == i)
						hec_accum = syndrome_table [ hec_accum ^ (0x80 >> j) ];
					else
						hec_accum = syndrome_table [ hec_accum ];
			}
			else
			{
				hec_accum = (0x80 >> j);
			}

			err_posn_table [ hec_accum ] = (i * 8) + j;
		}
	}

	return;
}


GLOBAL void FrameALHecEncode
(
    BYTE *Header
)
/* calculate CRC-8 remainder over first four bytes of cell header   */
/* exclusive-or with coset leader & insert into fifth header byte   */
{
	BYTE hec_accum = 0;
	WORD i;

	for ( i = 0;  i < 4;  i++ )
		hec_accum = syndrome_table [ hec_accum ^ *Header++ ];
	*Header = hec_accum ^ COSET_LEADER;

	return;
}


GLOBAL WORD	FrameALHecDecode
(
    BYTE *Header
)

/* return HEC value calculated over first four bytes of cell header */
{
	BYTE syndrome;
	int i, err_posn;
	BYTE *cell_header = Header;
	syndrome = 0;


	for ( i = 0;  i < 4;  i++ )
		syndrome = syndrome_table [ syndrome ^ cell_header[i] ];

	syndrome ^= cell_header[4] ^ COSET_LEADER;

	err_posn = err_posn_table [ syndrome ];
	if ( err_posn < 0)
	{
		return HEC_GOOD;
	}
	else
		if ( err_posn < 40)
		{
			// do not correct errors because we may not be pointing to a cell header
			// Cell Align may be in a search mode.
			/* cell_header [ err_posn / 8 ] ^= (0x80 >> (err_posn % 8)); */
			return HEC_ONE_BIT_ERROR;
		}
		else
		{
			return HEC_BAD;
		}
}



