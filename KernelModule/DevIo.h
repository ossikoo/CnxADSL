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
**		DevIo.h
**
**	ABSTRACT:
**		This file contains defines for accessing the driver
**		through a device specific function/method (e.g., through
**		the "OID_TAPI_DEV_SPECIFIC" OID of the driver registered
**		SetInformationHandler function).
**		This is the device specific interface through which
**		applications may access certain driver specific information.
**		This device specific interface (DevIo) to the driver
**		has a legacy name of BackDoor (BD or Bd) 
**
**	DETAILS:
**		File Structure:
**			The structure of this file (and/or header files included
**			by this file) is divided into three parts.
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
**>	***************************************************
**>	//	Parameters:	xxx->ReqCode = BD_XDSL_REINIT_DATA_PUMP	//	Execute ONLY!
**>	//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
**>	typedef struct BD_XDSL_REINIT_DP_S
**>	{
**>		DWORD						Unknown_Now_Need_More_Info;
**>	} BD_XDSL_REINIT_DP_T;
**>	
**>	
**>	****************************************************
**>	//	Parameters:	xxx->ReqCode = BD_XDSL_GET_CTRL_CONFIG
**>	//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
**>	//				xxx->Params.BdXdslCtrlConfig.NumOfVc = ZZZ1
**>	//				xxx->Params.BdXdslCtrlConfig.VcArray[n].Vpi = ZZZ3
**>	//				xxx->Params.BdXdslCtrlConfig.VcArray[n].Vci = ZZZ4
**>	//				xxx->Params.BdXdslCtrlConfig.VcArray[n].Pcr = ZZZ5
**>	//													where "n" is 0 to (ZZZ1-1)
**>	//
**>	****************************************************
**>	//	Parameters:	xxx->ReqCode = BD_XDSL_SET_CTRL_CONFIG
**>	//				xxx->Params.BdXdslCtrlConfig.NumOfVc = ZZZ1
**>	//				xxx->Params.BdXdslCtrlConfig.VcArray[n].Vpi = ZZZ3
**>	//				xxx->Params.BdXdslCtrlConfig.VcArray[n].Vci = ZZZ4
**>	//				xxx->Params.BdXdslCtrlConfig.VcArray[n].Pcr = ZZZ5
**>	//													where "n" is 0 to (ZZZ1-1)
**>	//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
**>	//
**>	****************************************************
**>	*****************************************************************************
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
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/DevIo.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
/**/	//This is a Page Eject character.
#ifndef _DevIo_H_
#define _DevIo_H_


// * Ioctl Format:
// *
// * CTL_CODE( DeviceType, Function, Method, Access ) (
// *   ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method)
// *
// * Note: Custom Functions must must have the MSB set i.e., 8xx)
// *
#define IOCTL_DEVICE_SPECIFIC		_IOW(TIGR_MAGIC,5,struct atmif_sioc)
#define IOCTL_GET_VERSION			_IOW(TIGR_MAGIC,6,struct atmif_sioc)
#define IOCTL_CNX_DEV_SPECIFIC		_IOW(TIGR_MAGIC,7,struct atmif_sioc)
#define IOCTL_CNX_NET_DEV_SPECIFIC	_IOW(TIGR_MAGIC,8,struct atmif_sioc)
#define IOCTL_CNX_NET_DATA			_IOW(TIGR_MAGIC,9,struct atmif_sioc)
#define IOCTL_CNX_NET_OPEN			_IOW(TIGR_MAGIC,10,struct atmif_sioc)
#define IOCTL_CNX_NET_CLOSE			_IOW(TIGR_MAGIC,11,struct atmif_sioc)
#define IOCTL_CNX_NET_ECHO_PARENT	_IOW(TIGR_MAGIC,12,struct atmif_sioc)

///////////////////////////////////////////////////////////////////////////////
//
//	General Definitions (defines)
//
///////////////////////////////////////////////////////////////////////////////


#define UNKNOWN_STRING			"Unknown!"
#define UNKNOWN_4BIT_VALUE		0x0000000F
#define UNKNOWN_8BIT_VALUE		0x000000FF
#define UNKNOWN_16BIT_VALUE		0x0000FFFF
#define UNKNOWN_32BIT_VALUE		0xFFFFFFFF


//	The following define specifies that instead of a particular VC index,
//	this index value specifies to return statistics for all VCs on the link.
#define BD_ATM_STATS_INDEX_FOR_LINK			0xFFFF

//	The following define specifies that instead of a particular VC index,
//	this index value specifies to return statistics for all VCs on the link.
#define BD_AAL_STATS_INDEX_FOR_LINK			0xFFFF


// Mask for determining Message bits
#define DEBUG_MESSAGE_LEVEL			0x0000001D
#define DEBUG_MODULE_ID_MASK		0xffff0000
#define DBG_DISABLED				0x00000000
#define DBG_ENA_MESSAGES			0x00000001
#define DBG_ENA_BREAKPOINTS			0x00000002
#define DBG_ENA_MISC_MSG			0x00000004
#define DBG_ENA_WARNING_MSG			0x00000008
#define DBG_ENA_ERROR_MSG			0x00000010
#define DBG_ENA_MISC_BRK			0x00000020
#define DBG_ENA_WARNING_BRK			0x00000040
#define DBG_ENA_ERROR_BRK			0x00000080
#define DBG_ENA_DRVR_ENTRY_BRK		0x00000100
#define DBG_ENABLE_BIT_9			0x00000200
#define DBG_ENABLE_BIT_10			0x00000400
#define DBG_ENABLE_BIT_11			0x00000800
#define DBG_ENABLE_BIT_12			0x00001000
#define DBG_ENABLE_BIT_13			0x00002000
#define DBG_ENABLE_BIT_14			0x00004000
#define DBG_ENABLE_BIT_15			0x00008000
#define DBG_ENABLE_CHIPAL			0x00010000
#define DBG_ENABLE_CARDAL			0x00020000
#define DBG_ENABLE_CARDMGMT			0x00040000
#define DBG_ENABLE_FRAMEAL			0x00080000
#define DBG_ENABLE_BUFMGMT			0x00100000
#define DBG_ENABLE_CELLDATATEST		0x00200000
#define DBG_ENABLE_CONDIS 			0x00400000
#define DBG_ENABLE_BIT_23			0x00800000
#define DBG_ENABLE_BIT_24			0x01000000
#define DBG_ENABLE_BIT_25			0x02000000
#define DBG_ENABLE_BIT_26			0x04000000
#define DBG_ENABLE_BIT_27			0x08000000
#define DBG_ENABLE_BD_MSG_DUMP		0x10000000
#define DBG_ENABLE_BIT_29			0x20000000
#define DBG_ENABLE_BIT_30			0x40000000
#define DBG_ENABLE_BIT_31			0x80000000
// The following are defined for use but are defined here for Appy purposes.
#define DBG_ENABLE_BIT_0			0x00000001
#define DBG_ENABLE_BIT_1			0x00000002
#define DBG_ENABLE_BIT_2			0x00000004
#define DBG_ENABLE_BIT_3			0x00000008
#define DBG_ENABLE_BIT_4			0x00000010
#define DBG_ENABLE_BIT_5			0x00000020
#define DBG_ENABLE_BIT_6			0x00000040
#define DBG_ENABLE_BIT_7			0x00000080
#define DBG_ENABLE_BIT_8			0x00000100
#define DBG_ENABLE_BIT_16			0x00010000
#define DBG_ENABLE_BIT_17			0x00020000
#define DBG_ENABLE_BIT_18			0x00040000
#define DBG_ENABLE_BIT_19			0x00080000
#define DBG_ENABLE_BIT_20			0x00100000
#define DBG_ENABLE_BIT_21			0x00200000
#define DBG_ENABLE_BIT_22			0x00400000
#define DBG_ENABLE_BIT_28			0x10000000


//	The following define gives the sizeof the TAPI Dev Specific header
//	structure, excluding the "Params" element which is overlayed by the
//	BackDoor structure/data.
#define TAPI_DEV_SPECIFIC_SIZE	(sizeof( NDIS_TAPI_DEV_SPECIFIC ) - sizeof( UCHAR * ))


#define	BD_MINI_DATA_DWORDS		25


///////////////////////////////////////
//	BackDoor Group Code Definitions
///////////////////////////////////////

#define	BD_START_ALL_CODES	0x0000FFFF
#define	BD_CHIPAL_CODES		0x00010000
#define	BD_FRAMEAL_CODES	0x00020000
#define	BD_BUFMGMT_CODES	0x00030000
#define	BD_CARDMGMT_CODES	0x00040000
#define	BD_CDSL_CODES		0x00050000
#define	BD_ADSL_CODES		0x00060000
#define	BD_CARDAL_CODES		0x00070000
#define	BD_TEST_MOD_CODES	0x00080000
#define	BD_PM_CODES			0x00090000
#define	BD_DMTAPI_CODES		0x000A0000
#define BD_CONDIS_CODES		0x000B0000
#define BD_ADSL_DIAG_CODES  0x000C0000
#define	BD_END_ALL_CODES	0x00FF0000




///////////////////////////////////////////////////////////////////////////////
//
//	General Buffer Definitions (typedefs)
//
///////////////////////////////////////////////////////////////////////////////


typedef DWORD	BD_GEN_BUFFER_LENGTH_T;

typedef BYTE	BD_MINI_DATA_BLOCK_T [BD_MINI_DATA_DWORDS * sizeof( DWORD )];

typedef DWORD	BD_MINI_DATA_BLOCK_DWORDS_T [BD_MINI_DATA_DWORDS];

typedef BYTE	BD_GEN_BUFFER_DATA_96_T [96];

/*
******* NOT available with MINI BD buffer size *******
typedef BYTE	BD_GEN_BUFFER_DATA_128_T [128];

typedef BYTE	BD_GEN_BUFFER_DATA_256_T [256];

typedef BYTE	BD_GEN_BUFFER_DATA_512_T [512];

typedef BYTE	BD_GEN_BUFFER_DATA_1024_T [1024];

typedef BYTE	BD_GEN_BUFFER_DATA_1024_T [1024];
******* NOT available with MINI BD buffer size *******
*/

typedef struct BD_GEN_BUFFER_96_S
{
	BD_GEN_BUFFER_LENGTH_T		Length;
	BD_GEN_BUFFER_DATA_96_T		DataBytes;
} BD_GEN_BUFFER_96_T;

/*
******* NOT available with MINI BD buffer size *******
typedef struct BD_GEN_BUFFER_128_S
{
	BD_GEN_BUFFER_LENGTH_T		Length;
	BD_GEN_BUFFER_DATA_128_T	DataBytes;
} BD_GEN_BUFFER_128_T;

typedef struct BD_GEN_BUFFER_256_S
{
	BD_GEN_BUFFER_LENGTH_T		Length;
	BD_GEN_BUFFER_DATA_256_T	DataBytes;
} BD_GEN_BUFFER_256_T;

typedef struct BD_GEN_BUFFER_512_S
{
	BD_GEN_BUFFER_LENGTH_T		Length;
	BD_GEN_BUFFER_DATA_512_T	DataBytes;
} BD_GEN_BUFFER_512_T;

typedef struct BD_GEN_BUFFER_1024_S
{
	BD_GEN_BUFFER_LENGTH_T		Length;
	BD_GEN_BUFFER_DATA_1024_T	DataBytes;
} BD_GEN_BUFFER_1024_T;
******* NOT available with MINI BD buffer size *******
*/




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	Module header file Include Section
//////////////////////////////////////////////


#if	PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
	#include "DevIoCheetah.h"
#else
	#define DEVIO_CHEETAH_STRUC_UNION
	#define DEVIO_CHEETAH_REQ_CODES
#endif


#if	PROJECTS_NAME == PROJECT_NAME_TIGRIS
	#include "DevIoTigris.h"
#else
	#define DEVIO_TIGRIS_STRUC_UNION
	#define DEVIO_TIGRIS_REQ_CODES
#endif

#if PROJECTS_NAME == PROJECT_NAME_MADMAX || PROJECTS_NAME == PROJECT_NAME_NILE_TEST || PROJECTS_NAME == PROJECT_NAME_WDM_FILTER
	#include "DevIoChipAl.h"
	#include "DevIoP46Arb.h"
#else
	#define DEVIO_P46ARB_STRUC_UNION
	#define DEVIO_P46ARB_REQ_CODES
#endif

/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Union of Structures Definitions
//////////////////////////////////////////////

typedef union BD_STRUCTURE_UNION_U
{

	DEVIO_CHEETAH_STRUC_UNION

	DEVIO_TIGRIS_STRUC_UNION

	DEVIO_P46ARB_STRUC_UNION

} BD_STRUCTURE_UNION_T;


typedef union MINI_BD_STRUC_UNION_U
{

	DEVIO_CHEETAH_STRUC_UNION

	DEVIO_TIGRIS_STRUC_UNION

	DWORD						DwordReserved [BD_MINI_DATA_DWORDS];
	//	Reserved area used to establish size of
	//	'Mini' BackDoor data area so that  its
	//	size will match the WMI expected size.
	//
	//	*** WARNING ***
	//
	//		The size of the "MINI_BACK_DOOR_T" structure MUST match
	//		the size of the parameters defined in the "Cheetah1.MOF"
	//		file for the WMI interface and the driver to work
	//		together properly!!!!!
	//
	//		The union element "DwordReserved" is the method used to
	//		ensure that the "MINI_BACK_DOOR_T" size matches the size
	//		defined in the file "Cheetah1.MOF".
	//
} MINI_BD_STRUC_UNION_T;




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Structure Definition
//////////////////////////////////////////////
typedef struct DEVIO_HEADER_s
{
	DWORD						InstanceId;
	DWORD						ReqCode;
	DWORD						TotalSize;
	DWORD						NeededSize;
	DWORD						ResultCode;
} DEVIO_HEADER_T;


typedef struct BACK_DOOR_S
{
	DWORD						InstanceId;
	DWORD						ReqCode;
	DWORD						TotalSize;
	DWORD						NeededSize;
	DWORD						ResultCode;
	BD_STRUCTURE_UNION_T		Params;
} BACK_DOOR_T;


typedef struct MINI_BACK_DOOR_S
{
	DWORD						InstanceId;
	DWORD						ReqCode;
	DWORD						TotalSize;
	DWORD						NeededSize;
	DWORD						ResultCode;
	MINI_BD_STRUC_UNION_T		Params;
} MINI_BACK_DOOR_T;




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Code Definitions
//////////////////////////////////////////////


typedef enum BACKDOOR_CODES_E
{

    BD_START_CODE					= BD_START_ALL_CODES,

    DEVIO_CHEETAH_REQ_CODES

    DEVIO_TIGRIS_REQ_CODES

    DEVIO_P46ARB_REQ_CODES

    BD_END_CODE						= BD_END_ALL_CODES
} BACKDOOR_CODES_T;




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



typedef enum RESULT_CODES_E
{
    RESULT_START						= 0,
    RESULT_SUCCESS						= 0,
    RESULT_FAILURE,
    RESULT_REQ_NOT_SUPPORTED,
    RESULT_REQ_BUFF_TOO_SMALL,
    RESULT_BD_INSTANCE_UNAVAIL,				//		0x004
    RESULT_BD_INSTANCE_EMPTY,				// 5
    RESULT_DATA_UNAVAILABLE,
    RESULT_DATA_UNINITIALIZED,
    RESULT_DATA_FAILURE,					//		0x008
    RESULT_DEVICE_BUSY,
    RESULT_DEVICE_UNAVAILABLE,				// 10
    RESULT_DEVICE_UNINITIALIZED,
    RESULT_DEVICE_DOWNLOADING,				//		0x00C
    RESULT_DEVICE_CMD_IN_PROG,
    RESULT_DEVICE_FAILURE,
    RESULT_DEVICE_HW_TIMER_FAIL,			// 15
    RESULT_CALL_UNINITIALIZED,				//		0x010
    RESULT_RESOURCE_ALLOC_FAILURE,
    RESULT_RESOURCE_CONFLICT,
    RESULT_PARAM_OUT_OF_RANGE,
    RESULT_PARAM_INVALID,					// 20	0x014
    RESULT_MODULE_UNINITIALIZED,
    RESULT_ADAPTER_NOT_FOUND,
    RESULT_ADAPTER_UNINITIALIZED,
    RESULT_END								// 24	0x018
} RESULT_CODES_T;

#define	RESULT_MASK			0x00000FFF



//	NOTE:	For the 'Instance' field, each module has its own set of
//			values and the starting value is the same for every module!
//			(i.e., the values are overlayed for each module)
//	NOTE:	The "INSTANCE_END" value MUST be set to be one greater
//			than the largest value!
typedef enum INSTANCE_E
{
    INSTANCE_SUCCESS				= 0,
    INSTANCE_START					= 0x1000,
    INSTANCE_UNKNOWN				= 0x1000,
    INSTANCE_CARDAL					= 0x2000,		//	CardAL Instances
    INSTANCE_CARDMGMT				= 0x2000,		//	CardMgmt Instances
    INSTANCE_BASIC2					= 0x2000,		//	ChipAL Instances
    INSTANCE_P64					= 0x3000,
    INSTANCE_UTILITY				= 0x2000,		//	Utility Instances
    INSTANCE_ATM					= 0x2000,		//	FrameAL Instances
    INSTANCE_HDLC					= 0x3000,
    INSTANCE_WAN					= 0x2000,		//	BuffMgmt Instances
    INSTANCE_LAN					= 0x3000,
    INSTANCE_CO						= 0x4000,
    INSTANCE_CELLDATATEST			= 0x5000,
    INSTANCE_END					= 0x6000,		// CAUTION:	MUST BE one greater
    //			than largest instance!!
} INSTANCE_T;

#define	INSTANCE_MASK		0x0000F000


typedef enum MODULE_NAME_E
{
    MODULE_SUCCESS					= 0,
    MODULE_START					= 0x00010000,
    MODULE_UNKNOWN					= 0x00010000,
    MODULE_BUFFMGMT					= 0x00020000,
    MODULE_CARDAL					= 0x00030000,
    MODULE_CARDMGMT					= 0x00040000,
    MODULE_CHIPAL					= 0x00050000,
    MODULE_FRAMEAL					= 0x00060000,
    MODULE_UTILITY					= 0x00070000,
    MODULE_END						= 0x00080000
} MODULE_NAME_T;

#define	MODULE_MASK			0x0FFF0000



typedef enum ERROR_LEVEL_E
{
    LEVEL_SUCCESS					= 0,
    LEVEL_START						= 0x10000000,
    LEVEL_NORMAL					= 0x10000000,
    LEVEL_WARNING					= 0x20000000,
    LEVEL_FATAL						= 0x30000000,
    LEVEL_END						= 0x40000000
} ERROR_LEVEL_T;

#define	LEVEL_MASK			0xF0000000



typedef struct ERROR_FIELDS_S
{
RESULT_CODES_T			Results	:12;		// Error Code			(Enum)
BYTE					Instance:4;			// Instance of Module	(Enum)
WORD					Module	:12;		// Module Type			(Enum)
BYTE					Level	:4;			// Severity Level		(Enum)
} ERROR_FIELDS_T;

typedef union ERROR_CODE_U
{
	DWORD				dwValue;		// Complete Error Code In a DWORD Value
	ERROR_FIELDS_T		Fields;			// Error Code Broken Into 4 Fields
} ERROR_CODE_T;
/*
**
**	Notes:
**
**	To check the complete value
**
**	ERROR_CODE_T 	Ec;
**	...
**	if (Ec.Value == Success)
**	..
**
**	To check an individual Element:
**
**	if (Ec.Error.Results == SpecificResult)
**
**	...
**
*/



#endif		//#ifndef _DevIo_H_

