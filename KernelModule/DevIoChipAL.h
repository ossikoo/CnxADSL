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
**		DevIoChipAL.h
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
**		in the ChipAL module.
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
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/DevIoChipAL.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
/**/	//This is a Page Eject character.
#ifndef _DEVIOCHIPAL_H_
#define _DEVIOCHIPAL_H_


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
//	ChipAL Module BackDoor Definitions
//////////////////////////////////////////////

// ***********************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_ATM_START_TX_PATTERN
//				xxx->Params.BdChipALAtmTxPattern = XXX
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//
typedef enum BD_ATM_TEST_CELL_PATTERNS_E
{
    BD_ATM_TEST_CELL_PATTERN_START		=	0,
    BD_ATM_TEST_CELL_PATTERN_A			=	0,
    BD_ATM_TEST_CELL_PATTERN_B,
    BD_ATM_TEST_CELL_PATTERN_C,
    BD_ATM_TEST_CELL_PATTERN_D,
    BD_ATM_TEST_CELL_PATTERN_E,
    BD_ATM_TEST_CELL_PATTERN_USER_A,
    BD_ATM_TEST_CELL_PATTERN_USER_B,
    BD_ATM_TEST_CELL_PATTERN_END
} BD_ATM_TEST_CELL_PATTERNS_T;
#define DEFAULT_BD_ATM_TEST_CELL_PATTERN	BD_ATM_TEST_CELL_PATTERN_A


// *****************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_START_LOOPBACK		//	Get/Read ONLY!
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//


// *******************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_START_1_LOOPBACK		//	Get/Read ONLY!
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//


// ****************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_STOP_LOOPBACK		//	Get/Read ONLY!
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//


// *******************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_GET_TEST_RESULTS		//	Get/Read ONLY!
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdChipALTestRslts = ZZZ
//


// ************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_GET_STATS
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdChipALStats.XXX = ZZZ
//
typedef struct BD_CHIPAL_STATS_S			//	Get/Read ONLY!
{
	DWORD	InterruptCountTx;
	DWORD	InterruptCountRx;
	DWORD	BytesTransmitted;
	DWORD	BytesReceived;
	DWORD	InterruptsMissedTx;
	DWORD	InterruptsMissedRx;
	DWORD	UnderRun;
	DWORD	OverRun;
} BD_CHIPAL_STATS_T;						//	Get/Read ONLY!


// **************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_CLEAR_STATS	//	Execute ONLY!
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//


/**/	//This is a Page Eject character.
// ********************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_ENQ_TEST_COMPLETE	//	Get/Read ONLY!
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdChipALTestEnq = ZZZ
//


// ***************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_INIT_ADAPTER	//	Execute ONLY!
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//


// ***********************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_GET_BUFF
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdChipALBuf96		// 96 byte array, filled in by driver
//
// ***********************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_SET_BUFF
//				xxx->Params.BdChipALBuf96		// 96 byte array, sent to driver
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//


// *************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_GET_CONFIG		//	Get/Read ONLY!
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//


// *************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_GET_CONFIG
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdChipALConfig.Status = XXX
//

typedef enum CHIPAL_STATUS_E
{
    CHIPAL_STATUS_START = 0,
    CHIPAL_STATUS_DOWN = 0,
    CHIPAL_STATUS_INITIALIZED,
    CHIPAL_STATUS_CONFIGURED,
    CHIPAL_STATUS_UP,
    CHIPAL_STATUS_END
} CHIPAL_STATUS_T;

typedef struct BD_CHIPAL_CONFIG_S
{
	DWORD	Status;
	DWORD	Irq;
	DWORD	BaseAddress;
	DWORD	SharedAddress;
} BD_CHIPAL_CONFIG_T;


/**/	//This is a Page Eject character.
// ********************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_GET_TUNING_PARAMS
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdChipALTuning.XXX = ZZZ
//
// ********************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_SET_TUNING_PARAMS
//				xxx->Params.BdChipALTuning.XXX = ZZZ
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//
typedef struct BD_CHIPAL_TUNING_S
{
	DWORD	RxMaxIrptLatency;
	DWORD	RxMinIrptLatency;
	DWORD	RxIrptPeriod;
	DWORD	RxMaxChanSpeed;
	DWORD	TxMaxIrptLatency;
	DWORD	TxMinIrptLatency;
	DWORD	TxIrptPeriod;
	DWORD	TxMaxChanSpeed;
} BD_CHIPAL_TUNING_T;


// ***********************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_ATM_START_TX_PATTERN	//	Execute ONLY!
//				xxx->Params.BdChipALAtmTxPattern = ZZZ
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//


// **********************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_ATM_STOP_TX_PATTERN	//	Execute ONLY!
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//


// ******************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_ATM_GET_TX_CELL	//	Get/Read ONLY!
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdChipALAtmCell		// 128 byte array, last Tx ATM cell, filled in by driver
//


// ******************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_ATM_GET_RX_CELL	//	Get/Read ONLY!
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdChipALAtmCell		// 128 byte array, last Rx ATM cell, filled in by driver
//


// ****************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_ATM_GET_STATS
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdChipALAtmStats.XXX = ZZZ
//
typedef struct BD_CHIPAL_ATM_STATS_S		//	Get/Read ONLY!
{
	DWORD	NumTxCells;
	DWORD	NumRxCells;
} BD_CHIPAL_ATM_STATS_T;					//	Get/Read ONLY!


// ******************************************************
//	Parameters:	xxx->ReqCode = BD_CHIPAL_ATM_CLEAR_STATS	//	Execute ONLY!
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Union of Structures Definitions
//////////////////////////////////////////////


#define DEVIO_CHIPAL_STRUC_UNION												\
	DWORD						BdChipALTestRslts;		/* Get/Read ONLY! */	\
	BD_CHIPAL_STATS_T			BdChipALStats;			/* Get/Read ONLY! */	\
	DWORD						BdChipALTestEnq;								\
	BD_GEN_BUFFER_96_T			BdChipALBuf96;									\
	BD_CHIPAL_CONFIG_T			BdChipALConfig;			/* Get/Read ONLY! */	\
	BD_CHIPAL_TUNING_T			BdChipALTuning;									\
	BD_ATM_TEST_CELL_PATTERNS_T	BdChipALAtmTxPattern;	/* Start (Set) ONLY! */	\
	BD_GEN_BUFFER_96_T			BdChipALAtmCell;								\
	BD_CHIPAL_ATM_STATS_T		BdChipALAtmStats;		/* Get/Read ONLY! */




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Code Definitions
//////////////////////////////////////////////


#define DEVIO_CHIPAL_REQ_CODES											\
	BD_CHIPAL_START_CODE = BD_CHIPAL_CODES,								\
	BD_CHIPAL_START_LOOPBACK,						/* 0x00010001 */	\
	BD_CHIPAL_START_1_LOOPBACK,											\
	BD_CHIPAL_STOP_LOOPBACK,											\
	BD_CHIPAL_GET_TEST_RESULTS,											\
	BD_CHIPAL_GET_STATS,							/* 0x00010005 */	\
	BD_CHIPAL_CLEAR_STATS,												\
	BD_CHIPAL_ENQ_TEST_COMPLETE,										\
	BD_CHIPAL_INIT_ADAPTER,												\
	BD_CHIPAL_GET_BUFF,													\
	BD_CHIPAL_SET_BUFF,								/* 0x0001000A */	\
	BD_CHIPAL_GET_CONFIG,												\
	BD_CHIPAL_GET_TUNING_PARAMS,										\
	BD_CHIPAL_SET_TUNING_PARAMS,										\
	BD_CHIPAL_ATM_START_TX_PATTERN,										\
	BD_CHIPAL_ATM_STOP_TX_PATTERN,					/* 0x0001000F */	\
	BD_CHIPAL_ATM_GET_TX_CELL,											\
	BD_CHIPAL_ATM_GET_RX_CELL,											\
	BD_CHIPAL_ATM_GET_STATS,											\
	BD_CHIPAL_ATM_CLEAR_STATS,						/* 0x00010013 */	\
	BD_CHIPAL_END_CODE,								/* 0x00010014 */




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




#endif		//#ifndef _DEVIOCHIPAL_H_

