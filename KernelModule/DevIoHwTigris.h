/******************************************************************************
*******************************************************************************
****	Copyright (c) 1997, 1998, 1999
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
**		DevIo (though global and available to all files)
**
**	FILE NAME:
**		DevIoHwTigris.h
**
**	ABSTRACT:
**		This file contains defines for accessing the driver
**		through a device specific function/method (e.g., through
**		the "OID_TAPI_DEV_SPECIFIC" OID of the driver registered
**		SetInformationHandler function).
**		This is the device specific interface through which
**		applications may access certain driver specific information.
**		This device specific interface (DevIo) to the driver
**		has a legacy name of BackDoor (BD or Bd).
**		This file is for accessing device specific information
**		in the CardALHwTigris module.
**
**	DETAILS:
**		File Structure:
**			The structure of this file (and/or header files that
**			included this header file) is divided into three parts.
**			Part 1 - BackDoor enumeration and structure definitions
**					for elements of the BackDoor data structure union.
**			Part 2 - BackDoor data structure union definition.
**			Part 3 - BackDoor command enumeration definition.
**			The first two parts should be ordered in the manner
**			that the commands are (i.e., definitions for commands are
**			in the same order that the commands are).
**
**			The enumeration and structure definitions for elements
**			of the BackDoor data structure union (Part 1) are grouped
**			into sections for each of the BD commands.  Each of these
**			BD command sections is preceded by a comment block
**			defining the usage of the command (i.e., defining all
**			data passed to the driver and returned from the driver
**			via this command).
**
**			The definition of the BD data structure union (Part 2)
**			gives one union, overlay, for referencing data passed to
**			and returned from the driver in BD commands specifically
**			for that command.
**			Part 2 of the file is essentially in the Project header
**			file (e.g., "DevIoCheetah.h" or "DevIoTigris.h".
**
**			The definition of the BD command enumeration definition
**			(Part 3) enumerates the values for each BD command.  The
**			BD commands are separated into groups belonging to
**			modules or some logical collection.  The most significant
**			word of the BD command value is enumerated to the group
**			to which the command belongs.  The least significant
**			word of the BD command value is enumerated within that
**			group (starting with one).
**
**		Examples:
** |**|
**vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
**>PART 1 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
**>	// ***************************************************
**>	//	Parameters:	xxx->ReqCode = BD_XDSL_REINIT_DATA_PUMP	//	Execute ONLY!
**>	//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
**>	typedef struct BD_XDSL_REINIT_DP_S
**>	{
**>		DWORD						Unknown_Now_Need_More_Info;
**>	} BD_XDSL_REINIT_DP_T;
**>	
**>	
**>	// ****************************************************
**>	//	Parameters:	xxx->ReqCode = BD_XDSL_GET_CTRL_CONFIG
**>	//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
**>	//				xxx->Params.BdXdslCtrlConfig.NumOfVc = ZZZ1
**>	//				xxx->Params.BdXdslCtrlConfig.VcArray[n].Vpi = ZZZ3
**>	//				xxx->Params.BdXdslCtrlConfig.VcArray[n].Vci = ZZZ4
**>	//				xxx->Params.BdXdslCtrlConfig.VcArray[n].Pcr = ZZZ5
**>	//													where "n" is 0 to (ZZZ1-1)
**>	//
**>	// ****************************************************
**>	//	Parameters:	xxx->ReqCode = BD_XDSL_SET_CTRL_CONFIG
**>	//				xxx->Params.BdXdslCtrlConfig.NumOfVc = ZZZ1
**>	//				xxx->Params.BdXdslCtrlConfig.VcArray[n].Vpi = ZZZ3
**>	//				xxx->Params.BdXdslCtrlConfig.VcArray[n].Vci = ZZZ4
**>	//				xxx->Params.BdXdslCtrlConfig.VcArray[n].Pcr = ZZZ5
**>	//													where "n" is 0 to (ZZZ1-1)
**>	//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
**>	//
**>	// ****************************************************
**>	// *****************************************************************************
**>	
**>	typedef struct BD_XDSL_CTRL_VPI_VCI_S
**>	{
**>		BYTE						Vpi;		// Virtual Path Identifier
**>		WORD						Vci;		// Virtual Channel Identifier
**>		BD_XDSL_PCR_KBPS_T			Pcr;		// Peak Cell Rate
**>	} BD_XDSL_CTRL_VPI_VCI_T;
**>	
**>	typedef struct BD_XDSL_CTRL_CONFIG_S
**>	{
**>		DWORD						NumOfVc;
**>		BD_XDSL_CTRL_VPI_VCI_T		VcArray [DEFAULT_NUMBER_OF_LINES];
**>	} BD_XDSL_CTRL_CONFIG_T;
**>
**>PART 2 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
**>	typedef union BD_STRUCTURE_UNION_U
**>	{
**>		...
**>		BD_XDSL_REINIT_DP_T			BdXdslReInitDataPump;		//	Execute ONLY!
**>		BD_XDSL_CTRL_CONFIG_T		BdXdslCtrlConfig;
**>		...
**>	} BD_STRUCTURE_UNION_T;
**>
**>PART 3 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
**>		...
**>	#define	BD_XDSL_CODES		0x00060000
**>		...
**>	#define	BD_END_ALL_CODES	0x00FF0000
**>	
**>	typedef enum BACKDOOR_CODES_E
**>	{
**>		...
**>		BD_XDSL_START_CODE = BD_ADSL_CODES,
**>		...
**>		BD_XDSL_REINIT_DATA_PUMP,
**>		BD_XDSL_GET_CTRL_CONFIG,
**>		BD_XDSL_SET_CTRL_CONFIG,
**>		...
**>	} BACKDOOR_CODES_T;
**^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/DevIoHwTigris.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
/**/	//This is a Page Eject character.
#ifndef _DEVIOHWTIGRIS_H_
#define _DEVIOHWTIGRIS_H_


///////////////////////////////////////////////////////////////////////////////
//
//	General Definitions (defines)
//
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
//
//	General Buffer Definitions (typedefs)
//
///////////////////////////////////////////////////////////////////////////////




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	ADSL Tigris (CardALHwTigris) BackDoor Definitions
//////////////////////////////////////////////

// ***********************************************************
//	Parameters:	xxx->ReqCode = BD_CARDAL_GET_ADSL_TRANSCEIVER_STATUS
// ** the previous line should be changed to the following line!!!!!!!!!!!!!!
//	Parameters:	xxx->ReqCode = BD_ADSL_GET_TRANSCEIVER_STATUS
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.AdslTransceiverStatus = ZZZ1
//

typedef struct			// Matches DATE_TYPE in types.h
{
SHORT	year		:16;
WORD	day			: 8;
WORD	dayofweek	: 4;
WORD	month		: 4;
} DATE_T;

typedef struct			// Matches TIME_TYPE in types.h
{
	BYTE		hr;
	BYTE		min;
	BYTE		sec;
} TIME_T;

typedef struct
{						// See DPUUTIL.C: UTIL_Get_ext_int16_str for how to print.
	SHORT	Mar_Cur;	// Stored 8.8 (8 bits integer, 8 bits fraction)
	SHORT	Mar_Min;	// Stored 8.8 (8 bits integer, 8 bits fraction)
	WORD	Mar_Min_Bin;
} SNR_MARGIN_T;

//typedef struct BD_ADSL_TRANSCEIVER_STATUS_S
typedef struct BD_CARDAL_ADSL_TRANSCEIVER_STATUS_S
{
	DATE_T			Date;
	TIME_T			Time;
	BYTE			Transmit_State;
	BYTE			Receive_State;
	BYTE			Process_State;
	SNR_MARGIN_T	Up_SNR_Margin;
	SNR_MARGIN_T	Down_SNR_Margin;
	SHORT			Up_Attn;
	SHORT			Down_Attn;
	CHAR			Tx_Power;
	WORD			Up_Bits_Per_Frame;
	WORD			Down_Bits_Per_Frame;
	WORD			Startup_Attempts;
} BD_CARDAL_ADSL_TRANSCEIVER_STATUS_T;
//} BD_ADSL_TRANSCEIVER_STATUS_T;



/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Union of Structures Definitions
//////////////////////////////////////////////


#define DEVIO_HWTIGRIS_STRUC_UNION													\
	BD_CARDAL_ADSL_TRANSCEIVER_STATUS_T	AdslTransceiverStatus;						\
	BD_ADSL_INIT_DP_T			BdAdslInitDataPump;		/* Init DP (Set) ONLY! */	\
	BD_ADSL_CONN_CONFIG_T		BdAdslConnConfig;									\
	BD_ADSL_MODEM_DATA_T		BdAdslModemBlock;		/* Get/Read ONLY! */		\
	BD_ADSL_DATA_PATH_SELECT_T	BdAdslDataPathSelect;								\
	BD_ADSL_LINE_STATUS_T		BdAdslLineStatus;		/* Get/Read ONLY! */		\
	BD_ADSL_LINE_STATE_T		BdAdslLineState;		/* Get/Read ONLY! */		\
	BD_ADSL_DP_VERSIONS_T		BdAdslDpVersions;		/* Get/Read ONLY! */		\
	BD_MINI_DATA_BLOCK_T		BdAdslBusController;	/* Get/Read ONLY! */




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Code Definitions
//////////////////////////////////////////////


#define DEVIO_HWTIGRIS_REQ_CODES											\
	BD_DMTAPI_START_CODE = BD_DMTAPI_CODES,									\
	BD_CARDAL_GET_ADSL_TRANSCEIVER_STATUS,									\
/*	BD_ADSL_GET_TRANSCEIVER_STATUS, */										\
/*	BD_ADSL_INIT_DATA_PUMP,			*/										\
/*	BD_ADSL_GET_CONN_CONFIG,		*/										\
/*	BD_ADSL_SET_CONN_CONFIG,		*/										\
/*	BD_ADSL_GET_MODEM_DATA,			*/				/* 0x000A0005 */		\
/*	BD_ADSL_GET_DATA_PATH_SELECTION,*/										\
/*	BD_ADSL_SET_DATA_PATH_SELECTION,*/										\
/*	BD_ADSL_GET_LINE_STATUS,		*/										\
/*	BD_ADSL_GET_LINE_STATE,			*/										\
/*	BD_ADSL_GET_DP_VERSIONS,		*/				/* 0x000A000A */		\
/*	BD_ADSL_GET_BUS_CONTROLLER,		*/										\
	BD_DMTAPI_END_CODE,								/* 0x000A000C */		\
																			\
	BD_ADSL_START_CODE = BD_ADSL_CODES,				/* 0x00060000 */			\
	BD_ADSL_DO_CMD,																\
	BD_ADSL_GET_CMD_RESP,														\
	BD_ADSL_GET_PARAM,	/* *** no longer used, left to keep #s the same *** */	\
	BD_ADSL_SET_PARAM,	/* *** no longer used, left to keep #s the same *** */	\
	BD_ADSL_GET_PERFORMANCE,						/* 0x00060005 */			\
	BD_ADSL_INIT_DATA_PUMP,														\
	BD_ADSL_GET_CONN_CONFIG,													\
	BD_ADSL_SET_CONN_CONFIG,													\
	BD_ADSL_GET_MODEM_DATA,														\
	BD_ADSL_GET_DATA_PATH_SELECTION,				/* 0x0006000A */			\
	BD_ADSL_SET_DATA_PATH_SELECTION,											\
	BD_ADSL_GET_LINE_STATUS,													\
	BD_ADSL_GET_LINE_STATE,														\
	BD_ADSL_GET_DP_VERSIONS,													\
	BD_ADSL_GET_SPECIFIC_CMD_RESP,					/* 0x0006000F */			\
	BD_ADSL_GET_BUS_CONTROLLER,													\
	BD_ADSL_SET_TRACE,															\
	BD_ADSL_DEBUG,																\
	BD_ADSL_END_CODE,								/* 0x00060013 */			\




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Return Codes Definitions
//////////////////////////////////////////////

//
//	The BackDoor Return Code is defined using a 32-bit value.
//	This 32-bit value is broken/divided into four parts.
//	The four individual parts are:
//
//		Field Name			Bit Positions			# of Bits
//		===============		===================		=========
//		Error Code			bits  0 through 11		12 bits
//		Module Instance		bits 12 through 15		 4 bits
//		Module				bits 16 through 27		12 bits
//		Error Level			bits 28 through 31		 4 bits
//
//	Each individual part named above has its own enumeration/definition
//	of possible values defined below.
//




#endif		//#ifndef _DEVIOHWTIGRIS_H_

