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
**		DevIoPm.h
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
**		in the Power Management module.
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
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/DevIoPm.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
/**/	//This is a Page Eject character.
#ifndef _DEVIOPM_H_
#define _DEVIOPM_H_


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
// **************************************************
//	Parameters:	xxx->ReqCode = BD_PM_GET_POWER_STATE
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.PMPowerState = ZZZ1
// **************************************************
//	Parameters:	xxx->ReqCode = BD_PM_SET_POWER_STATE
//				xxx->Params.PMPowerState = ZZZ1
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//
typedef enum BD_POWER_STATE_E
{
    BD_PM_STATE_START	= 0,
    BD_PM_STATE_D0		= BD_PM_STATE_START,	/* Fully Operational */
    BD_PM_STATE_D1,								/* PCI Clock Running, Process PME Event*/
    BD_PM_STATE_D2,								/* PCI Clock Off, Process PME Event */
    BD_PM_STATE_D3_HOT,							/* Power Off to Devices, Context Maintained*/
    BD_PM_STATE_D3_COLD,						/* Power Off to Devices, Context lost */
    BD_PM_STATE_END
} BD_PM_POWER_STATE_T;




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Union of Structures Definitions
//////////////////////////////////////////////


#define DEVIO_PM_STRUC_UNION												\
	BD_PM_POWER_STATE_T			PMPowerState;




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Code Definitions
//////////////////////////////////////////////


#define DEVIO_PM_REQ_CODES												\
	BD_PM_START_CODE = BD_PM_CODES,										\
	BD_PM_GET_POWER_STATE,							/* 0x00090001 */	\
	BD_PM_SET_POWER_STATE,												\
	BD_PM_END_CODE,									/* 0x00090003 */




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




#endif		//#ifndef _DEVIOPM_H_

