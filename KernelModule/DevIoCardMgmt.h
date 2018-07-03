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
**		DevIoCardMgmt.h
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
**		in the CardMgmt module.
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
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/DevIoCardMgmt.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
/**/	//This is a Page Eject character.
#ifndef _DEVIOCARDMGMT_H_
#define _DEVIOCARDMGMT_H_


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
//	CardMgmt Module BackDoor Definitions
//////////////////////////////////////////////

// *******************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_GET_DEBUG_FLAG
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdCardMgmtDebugFlag = XXX
//
// *******************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_SET_DEBUG_FLAG
//				xxx->Params.BdCardMgmtDebugFlag = XXX
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//


// ***********************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_GET_CONNECT_STATUS
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdCardMgmtConnStatus = XXX
//
typedef enum BACKDOOR_STATUS_E
{
    BACKDOOR_STATUS_START =					0,
    BACKDOOR_STATUS_IDLE =					0,
    BACKDOOR_STATUS_HANDSHAKE,				//1
    BACKDOOR_STATUS_TRAINING,				//2
    BACKDOOR_STATUS_DATAMODE,				//3
    BACKDOOR_STATUS_RETRAIN,				//4
    BACKDOOR_STATUS_TESTMODE,				//5
    BACKDOOR_STATUS_POWERDOWN,				//6
    BACKDOOR_STATUS_FAILURECODE,			//7
    BACKDOOR_STATUS_END						//8
} BACKDOOR_STATUS_T;


/**/	//This is a Page Eject character.
// **********************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_SIMULATE_INCOMING	//	Execute ONLY!
//				xxx->Params.BdCardMgmtSimInCallVc = n   (0 to NumberOfVCs-1)
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//


// *************************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_SIMULATE_REMOTE_DISC	//	Execute ONLY!
//				xxx->Params.BdCardMgmtSimRemDiscVc = n   (0 to NumberOfVCs-1)
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//


// ***********************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_SIMULATE_CONNECTED	//	Execute ONLY!
//				xxx->Params.BdCardMgmtSimConnectVc = n   (0 to NumberOfVCs-1)
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//


// *********************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_GET_PRODUCT_INFO
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdCardMgmtProdInfo.XXX = ZZZ
//
typedef struct BD_CARDMGMT_PROD_INFO_S	//	Get/Read ONLY!
{
	DWORD	DrvrSwVer;
	DWORD	DrvrTapiVer;
	DWORD	DrvrNdisVer;
} BD_CARDMGMT_PROD_INFO_T;				//	Get/Read ONLY!


// **************************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_GET_VC_CONNECT_STATE
//				xxx->Params.BdCardMgmtVcConnState.VcSelectIndex = n   (0 to NumberOfVCs-1)
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdCardMgmtVcConnState.VcConnectState = XXX
//

typedef enum BD_VC_CONNECT_STATE_E
{
    BD_VC_STATE_START =				0,
    BD_VC_STATE_IDLE =				0,
    BD_VC_STATE_DIALTONE,			//1
    BD_VC_STATE_DIALING,			//2
    BD_VC_STATE_PROCEEDING,			//3
    BD_VC_STATE_RINGBACK,			//4
    BD_VC_STATE_OFFERING,			//5
    BD_VC_STATE_ACCEPTED,			//6
    BD_VC_STATE_CONNECTED,			//7
    BD_VC_STATE_DISCONNECTED,		//8
    BD_VC_STATE_BUSY,				//9
    BD_VC_STATE_END					//10
} BD_VC_CONNECT_STATE_T;

typedef struct BD_CARDMGMT_VC_CONN_STATE_S	//	Get/Read ONLY!
{
	DWORD						VcSelectIndex;
	BD_VC_CONNECT_STATE_T		VcConnectState;
} BD_CARDMGMT_VC_CONN_STATE_T;				//	Get/Read ONLY!


/**/	//This is a Page Eject character.
// *********************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_OPEN_BD_INSTANCE
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->InstanceId = XXX1
//				xxx->Params.BdCardMgmtInstanceStatus = XXX2
// **********************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_CLOSE_BD_INSTANCE
//				xxx->InstanceId = XXX1
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdCardMgmtInstanceStatus = XXX2
// *****************************************************************************

//				xxx->Params.BdCardMgmtInstanceStatus = XXX2
typedef enum BD_INSTANCE_STATUS_E
{
    BD_STATUS_INSTANCE_START =				0,
    BD_STATUS_INSTANCE_SUCCESS =			0,
    BD_STATUS_INSTANCE_EMPTY,				//1
    BD_STATUS_INSTANCE_UNAVAILABLE,			//2
    BD_STATUS_INSTANCE_FAILURE,				//3
    BD_STATUS_INSTANCE_END					//4
} BD_INSTANCE_STATUS_T;


// ************************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_GET_PRODUCT_VERSION
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdCardMgmtProdVer = "p.h.MM.mmm+ (wwyy)"
//											(ASCII string, zero terminated)
// ************************************************************


// *************************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_GET_CURRENT_MAC_ADDR
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdCardMgmtMacAddr = 'the six bytes
//												of the MAC address
// *************************************************************


// ************************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_GET_EEPROM_MAC_ADDR
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdCardMgmtMacAddr = 'the six bytes
//												of the MAC address
// ************************************************************
// ************************************************************
//	Parameters:	xxx->ReqCode = BD_CARDMGMT_SET_EEPROM_MAC_ADDR
//				xxx->Params.BdCardMgmtMacAddr = 'the six bytes
//												of the MAC address
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
// ************************************************************

typedef BYTE	BD_CARDMGMT_MAC_ADDR_T [6];




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	CDSL ?Module? BackDoor Definitions
//////////////////////////////////////////////

// ***********************************************************
//	Parameters:	xxx->ReqCode = BD_CDSL_GET_OPERATIONAL_PARAMS
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdCdslOperational.TxSymRate = XXX
//												.
//												.
//												.
//
// ***********************************************************
//	Parameters:	xxx->ReqCode = BD_CDSL_SET_OPERATIONAL_PARAMS
//				xxx->Params.BdCdslOperational.TxSymRate = XXX
//												.
//												.
//												.
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//

//				xxx->Params.BdCdslOperational.TxSymRate = XXX
typedef enum BACKDOOR_SYM_TX_RATE_VALUE_E
{
    BACKDOOR_SYM_TX_RATE_VALUE_START =		0,
    BACKDOOR_SYM_TX_RATE_VALUE_2K =			0,
    BACKDOOR_SYM_TX_RATE_VALUE_4K,			//1
    BACKDOOR_SYM_TX_RATE_VALUE_8K,			//2
    BACKDOOR_SYM_TX_RATE_VALUE_16K,			//3
    BACKDOOR_SYM_TX_RATE_VALUE_32K,			//4
    BACKDOOR_SYM_TX_RATE_VALUE_64K,			//5
    BACKDOOR_SYM_TX_RATE_VALUE_128K,		//6
    BACKDOOR_SYM_TX_RATE_VALUE_256K,		//7
    BACKDOOR_SYM_TX_RATE_VALUE_END			//8
} BACKDOOR_SYM_TX_RATE_VALUE_T;
#define DEFAULT_SYMBOL_TX_RATE					BACKDOOR_SYM_TX_RATE_VALUE_2K

//				xxx->Params.BdCdslOperational.RxSymRate = XXX
typedef enum BACKDOOR_SYM_RX_RATE_VALUE_E
{
    BACKDOOR_SYM_RX_RATE_VALUE_START =		0,
    BACKDOOR_SYM_RX_RATE_VALUE_2K =			0,
    BACKDOOR_SYM_RX_RATE_VALUE_4K,			//1
    BACKDOOR_SYM_RX_RATE_VALUE_8K,			//2
    BACKDOOR_SYM_RX_RATE_VALUE_16K,			//3
    BACKDOOR_SYM_RX_RATE_VALUE_32K,			//4
    BACKDOOR_SYM_RX_RATE_VALUE_64K,			//5
    BACKDOOR_SYM_RX_RATE_VALUE_128K,		//6
    BACKDOOR_SYM_RX_RATE_VALUE_256K,		//7
    BACKDOOR_SYM_RX_RATE_VALUE_END			//8
} BACKDOOR_SYM_RX_RATE_VALUE_T;
#define DEFAULT_SYMBOL_RX_RATE					BACKDOOR_SYM_RX_RATE_VALUE_2K

//				xxx->Params.BdCdslOperational.TxRate = XXX
typedef enum BACKDOOR_TX_RATE_VALUE_E
{
    BACKDOOR_TX_RATE_VALUE_START =			0,
    BACKDOOR_TX_RATE_VALUE_34K =			0,
    BACKDOOR_TX_RATE_VALUE_68K,				//1
    BACKDOOR_TX_RATE_VALUE_END				//2
} BACKDOOR_TX_RATE_VALUE_T;
#define DEFAULT_TX_RATE							BACKDOOR_TX_RATE_VALUE_68K

//				xxx->Params.BdCdslOperational.RxRate = XXX
typedef enum BACKDOOR_RX_RATE_VALUE_E
{
    BACKDOOR_RX_RATE_VALUE_START =			0,
    BACKDOOR_RX_RATE_VALUE_136K =			0,
    BACKDOOR_RX_RATE_VALUE_204K,			//1
    BACKDOOR_RX_RATE_VALUE_272K,			//2
    BACKDOOR_RX_RATE_VALUE_340K,			//3
    BACKDOOR_RX_RATE_VALUE_408K,			//4
    BACKDOOR_RX_RATE_VALUE_476K,			//5
    BACKDOOR_RX_RATE_VALUE_544K,			//6
    BACKDOOR_RX_RATE_VALUE_1088K,			//7
    BACKDOOR_RX_RATE_VALUE_END				//8
} BACKDOOR_RX_RATE_VALUE_T;
#define DEFAULT_RX_RATE							BACKDOOR_RX_RATE_VALUE_1088K

//				xxx->Params.BdCdslOperational.RateMode = XXX
typedef enum BACKDOOR_TX_RATE_MODE_E
{
    BACKDOOR_TX_RATE_MODE_START =			0,
    BACKDOOR_TX_RATE_MODE_FIXED =			0,
    BACKDOOR_TX_RATE_MODE_ADAPTIVE,			//1
    BACKDOOR_TX_RATE_MODE_END				//2
} BACKDOOR_TX_RATE_MODE_T;
#define DEFAULT_RATE_MODE						BACKDOOR_TX_RATE_MODE_FIXED

//				xxx->Params.BdCdslOperational.TxPwrMode = XXX
typedef enum BACKDOOR_TX_POWER_MODE_E
{
    BACKDOOR_TX_POWER_MODE_START =			0,
    BACKDOOR_TX_POWER_MODE_FIXED =			0,
    BACKDOOR_TX_POWER_MODE_ADAPTIVE,		//1
    BACKDOOR_TX_POWER_MODE_END				//2
} BACKDOOR_TX_POWER_MODE_T;
#define DEFAULT_POWER_MODE						BACKDOOR_TX_POWER_MODE_FIXED

//				xxx->Params.BdCdslOperational.TxPwrVal = XXX
typedef enum BACKDOOR_TX_POWER_VALUE_E
{
    BACKDOOR_TX_POWER_START =				0,
    BACKDOOR_TX_POWER_VALUENEG9 =			0,
    BACKDOOR_TX_POWER_VALUENEG8,			//1
    BACKDOOR_TX_POWER_VALUENEG7,			//2
    BACKDOOR_TX_POWER_VALUENEG6,			//3
    BACKDOOR_TX_POWER_VALUENEG5,			//4
    BACKDOOR_TX_POWER_VALUENEG4,			//5
    BACKDOOR_TX_POWER_VALUENEG3,			//6
    BACKDOOR_TX_POWER_VALUENEG2,			//7
    BACKDOOR_TX_POWER_VALUENEG1,			//8
    BACKDOOR_TX_POWER_VALUE0,				//9
    BACKDOOR_TX_POWER_VALUE1,				//10
    BACKDOOR_TX_POWER_VALUE2,				//11
    BACKDOOR_TX_POWER_VALUE3,				//12
    BACKDOOR_TX_POWER_VALUE4,				//13
    BACKDOOR_TX_POWER_VALUE5,				//14
    BACKDOOR_TX_POWER_VALUE6,				//15
    BACKDOOR_TX_POWER_VALUE7,				//16
    BACKDOOR_TX_POWER_VALUE8,				//17
    BACKDOOR_TX_POWER_VALUE9,				//18
    BACKDOOR_TX_POWER_VALUE10,				//19
    BACKDOOR_TX_POWER_VALUE11,				//20
    BACKDOOR_TX_POWER_VALUE12,				//21
    BACKDOOR_TX_POWER_END					//22
} BACKDOOR_TX_POWER_VALUE_T;
#define DEFAULT_POWER_LEVEL						BACKDOOR_TX_POWER_VALUENEG3

//				xxx->Params.BdCdslOperational.RxLevel = XXXX

//				xxx->Params.BdCdslOperational.SnMargin = XXXX

typedef struct BD_CDSL_OPERATIONAL_S
{
	DWORD	RateMode;
	DWORD	RxRate;
	DWORD	RxSymRate;
	DWORD	TxRate;
	DWORD	TxSymRate;
	DWORD	TxPwrMode;
	DWORD	TxPwrVal;
	DWORD	RxLevel;			//	Get/Read ONLY!
	DWORD	SnMargin;			//	Get/Read ONLY!
} BD_CDSL_OPERATIONAL_T;
#define DEFAULT_RX_LEVEL			UNKNOWN_16BIT_VALUE		//	Unknown
#define DEFAULT_SN_MARGIN			UNKNOWN_8BIT_VALUE		//	Unknown


/**/	//This is a Page Eject character.
// *****************************************************
//	Parameters:	xxx->ReqCode = BD_CDSL_GET_PRODUCT_INFO
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdCdslProdInfo.XXX = ZZZ
//
typedef struct BD_CDSL_PROD_INFO_S		//	Get/Read ONLY!
{
	CHAR	RemoteManufacturer [MAX_MANUFACTURER_LENGTH];
	DWORD	CdslHwRevision;
	DWORD	RemoteVendorId;
} BD_CDSL_PROD_INFO_T;					//	Get/Read ONLY!
#define DEFAULT_REMOTE_MANUF		UNKNOWN_STRING				//	Unknown
#define DEFAULT_CDSL_HW_REV			UNKNOWN_32BIT_VALUE			//	Unknown
#define DEFAULT_REM_VENDOR_ID		UNKNOWN_16BIT_VALUE			//	Unknown




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Union of Structures Definitions
//////////////////////////////////////////////

#define DEVIO_CARDMGMT_STRUC_UNION												\
	DWORD						BdCardMgmtDebugFlag;							\
	DWORD						BdCardMgmtConnStatus;	/* Get/Read ONLY! */	\
	DWORD						BdCardMgmtSimInCallVc;	/* Execute ONLY! */		\
	DWORD						BdCardMgmtSimRemDiscVc;	/* Execute ONLY! */		\
	DWORD						BdCardMgmtSimConnectVc;	/* Execute ONLY! */		\
	BD_CARDMGMT_PROD_INFO_T		BdCardMgmtProdInfo;		/* Get/Read ONLY! */	\
	BD_CARDMGMT_VC_CONN_STATE_T	BdCardMgmtVcConnState;	/* Get/Read ONLY! */	\
	DWORD						BdCardMgmtInstanceStatus;						\
	BD_MINI_DATA_BLOCK_T		BdCardMgmtProdVer;		/* Get/Read ONLY! */	\
	BD_CARDMGMT_MAC_ADDR_T		BdCardMgmtMacAddr;




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Code Definitions
//////////////////////////////////////////////


#define DEVIO_CARDMGMT_REQ_CODES												\
	BD_CARDMGMT_START_CODE = BD_CARDMGMT_CODES,		/* 0x00040000 */			\
	BD_CARDMGMT_GET_DEBUG_FLAG,													\
	BD_CARDMGMT_SET_DEBUG_FLAG,													\
	BD_CARDMGMT_GET_CONNECT_STATUS,	/* Line (physical i/f) Connect Status */	\
	BD_CARDMGMT_SIMULATE_INCOMING,												\
	BD_CARDMGMT_SIMULATE_REMOTE_DISC,				/* 0x00040005 */			\
	BD_CARDMGMT_SIMULATE_CONNECTED,												\
	BD_CARDMGMT_GET_PRODUCT_INFO,												\
	BD_CARDMGMT_GET_VC_CONNECT_STATE,											\
	BD_CARDMGMT_OPEN_BD_INSTANCE,												\
	BD_CARDMGMT_CLOSE_BD_INSTANCE,					/* 0x0004000A */			\
	BD_CARDMGMT_GET_PRODUCT_VERSION,											\
	BD_CARDMGMT_GET_CURRENT_MAC_ADDR,											\
	BD_CARDMGMT_GET_EEPROM_MAC_ADDR,											\
	BD_CARDMGMT_SET_EEPROM_MAC_ADDR,											\
	BD_CARDMGMT_END_CODE,							/* 0x0004000F */




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




#endif		//#ifndef _DEVIOCARDMGMT_H_

