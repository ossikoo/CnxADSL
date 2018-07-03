/******************************************************************************
********************************************************************************
****	Copyright (c) 1997, 1998, 1999, 2000, 2001
****	Conexant Systems Inc. (formerly Rockwell Semiconductor Systems)
****	Personal Computing Division
****	All Rights Reserved
****
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****
*******************************************************************************
**
**	MODULE NAME:
**		????
**
**	FILE NAME:
**		CnxLoad.h
**
**	ABSTRACT:
**		This is the download record include file.  It contains all of the
**		data definitions for the load process.
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux 2.4 ATM Driver/DownLoadApp/cnxadslload.h $
**	$Revision: 1 $
**	$Date: 3/29/01 8:24a $
*******************************************************************************
******************************************************************************/

/////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// hex record defines
#define REC_MARK_LEN		1
#define REC_DATA_LEN		2
#define REC_OFFSET_LEN		4
#define REC_TYPE_LEN		2
#define REC_CHECKSUM_LEN	2
#define EOL_LEN			2
#define MAX_RECORD_LENGTH	4096


// hex record format structure
typedef struct HEX_FORMAT_S
{
	CHAR	Mark[REC_MARK_LEN];
	CHAR	Length[REC_DATA_LEN];
	CHAR	Offset[REC_OFFSET_LEN];
	CHAR	Type[REC_TYPE_LEN];
}HEX_HEADER_T;


