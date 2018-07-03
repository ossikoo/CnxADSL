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
**		DevIoTestMod.h
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
**		in the TestMod module.
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
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/DevIoTestMod.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
/**/	//This is a Page Eject character.
#ifndef _DEVIOTESTMOD_H_
#define _DEVIOTESTMOD_H_

#ifndef INCLUDE_IN_DEVIO_UNION 
#define INCLUDE_IN_DEVIO_UNION 	 0
#endif 


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
// ***************************************************************
//	Parameters:	xxx->ReqCode = BD_TEST_MOD_START_DIGITAL_LOOPBACK
//				xxx->Params.BdTestModDigitalLoopBack.TestDigLbVpi = YYY
//				xxx->Params.BdTestModDigitalLoopBack.TestDigLbVci = ZZZ
//				xxx->Params.BdTestModDigitalLoopBack.TestDigLbPcr = XXX
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//
// **************************************************************
//	Parameters:	xxx->ReqCode = BD_TEST_MOD_STOP_DIGITAL_LOOPBACK
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//
typedef struct	BD_TESTMOD_INFO_S
{
	DWORD					TestDigLbVpi;		// Virtual Path Identifier
	DWORD					TestDigLbVci;		// Virtual Channel Identifier
	BD_ADSL_PCR_KBPS_T		TestDigLbPcr;		// Peak Cell Rate
} BD_TESTMOD_INFO_T;								//	Start (Set/Write) ONLY!


// **********************************************************
//	Parameters:	xxx->ReqCode = BD_TEST_MOD_START_CELL_VERIFY
//				xxx->Params.BdTestModCellVerify.Vpi = YYY1
//				xxx->Params.BdTestModCellVerify.Vci = YYY2
//				xxx->Params.BdTestModCellVerify.Pcr = YYY4
//				xxx->Params.BdTestModCellVerify.PatternIndex = YYY3
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
// *********************************************************
//	Parameters:	xxx->ReqCode = BD_TEST_MOD_STOP_CELL_VERIFY
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
// *********************************************************
//

typedef enum BD_TESTMOD_CELL_PATTERNS_E
{
    BD_TESTMOD_CELL_PATTERN_START	=	0,
    BD_TESTMOD_CELL_PATTERN_1		=	0,
    BD_TESTMOD_CELL_PATTERN_END			// 1
} BD_TESTMOD_CELL_PATTERNS_T;

typedef struct	BD_TESTMOD_CELL_VERIFY_S
{
	DWORD						Vpi;		// Virtual Path Identifier
	DWORD						Vci;		// Virtual Channel Identifier
	BD_TESTMOD_CELL_PATTERNS_T	PatternIndex;
	BD_ADSL_PCR_KBPS_T			Pcr;		// Peak Cell Rate
} BD_TESTMOD_CELL_VERIFY_T;							//	Start (Set/Write) ONLY!


// **************************************************************
//	Parameters:	xxx->ReqCode = BD_TEST_MOD_GET_CELL_VERIFY_STATS
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdTestModVerifyStats.NumTestFramesTxd = ZZZ1
//				xxx->Params.BdTestModVerifyStats.NumGoodTestFramesRxd = ZZZ2
//				xxx->Params.BdTestModVerifyStats.NumOutOfOrderTestFramesRxd = ZZZ3
//
typedef struct	BD_TESTMOD_VERIFY_STATS_S
{
	DWORD						NumTestFramesTxd;
	DWORD						NumGoodTestFramesRxd;
	DWORD						NumOutOfOrderTestFramesRxd;

} BD_TESTMOD_VERIFY_STATS_T;					//	Get/Read ONLY!

typedef enum BD_TESTMOD_ATM_BER_PATTERN_E
{
    BD_TESTMOD_NO_PATTERN =0,
    BD_TESTMOD_15BIT,
    BD_TESTMOD_20BIT,
    BD_TESTMOD_23BIT,
    BD_TESTMOD_USER_SUPPLIED
} BD_TESTMOD_ATM_BER_PATTERN_T;


typedef enum BD_TESTMOD_ATM_BER_STATUS_E
{
    BD_TESTMOD_ATM_BER_IDLE = 0,
    BD_TESTMOD_ATM_BER_ACTIVE,
    BD_TESTMOD_ATM_BER_RXONLY
}BD_TESTMOD_ATM_BER_STATUS_T;

typedef enum BD_TESTMOD_ATM_BER_INSERT_E
{
    INJECT_NO_ERROR =0,
    INJECT_SINGLE_ERROR,	// insert one error
    ERRORM3=3,	 	// insert one error every 10^3 bits
    ERRORM4,	 	// insert one error every 10^4 bits
    ERRORM5,	 	// insert one error every 10^5 bits
    ERRORM6,	 	// insert one error every 10^6 bits
    ERRORM7			// insert one error every 10^7 bits
} BD_TESTMOD_ATM_BER_INSERT_T;


typedef struct	BD_TESTMOD_ATM_BER_S
{
	DWORD							Vpi;		// Virtual Path Identifier
	DWORD							Vci;		// Virtual Channel Identifier
	BD_ADSL_PCR_KBPS_T				Pcr;		// Peak Cell Rate
	BD_TESTMOD_ATM_BER_PATTERN_T 	Pattern;
	DWORD							UserPatternMask;
	BOOLEAN							RXInvert;   // expect receive data to be inverted
	BOOLEAN							TXInvert;	// Invert transmit data
	BD_TESTMOD_ATM_BER_STATUS_T  	Status;		// On start sets status
	BD_TESTMOD_ATM_BER_INSERT_T		ErrInsertion;

} BD_TESTMOD_ATM_BER_T;							//	Start (Set/Write) ONLY!


typedef struct BD_TESTMOD_ATM_BER_STATS_S
{


	DWORD					RXBits;	 // multiply by 10000
	DWORD					RXErrorBits;
	DWORD					RXResyncs;
	DWORD					TXBits;	// multiply by 10000
} 	BD_TESTMOD_ATM_BER_STATS_T;


typedef struct BD_TESTMOD_ATM_BER_STATS64_S
{


	LONGLONG				RXBits;
	LONGLONG				RXErrorBits;
	LONGLONG				RXResyncs;
	LONGLONG				TXBits;
} 	BD_TESTMOD_ATM_BER_STATS64_T;




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Union of Structures Definitions
//////////////////////////////////////////////


#if INCLUDE_IN_DEVIO_UNION 
#define DEVIO_TESTMOD_STRUC_UNION													\
	BD_TESTMOD_INFO_T			BdTestModDigitalLoopBack;	/* Start (Set) ONLY! */	\
	BD_TESTMOD_CELL_VERIFY_T	BdTestModCellVerify;		/* Start (Set) ONLY! */	\
	BD_TESTMOD_VERIFY_STATS_T	BdTestModVerifyStats;		/* Get/Read ONLY! */	\
	BD_TESTMOD_ATM_BER_T		BdTestModATMBER;  			\
	BD_TESTMOD_ATM_BER_STATS_T  BdTestModATMBERSTAT;		\
	BD_TESTMOD_ATM_BER_INSERT_T BdTestModATMBERInject;		\
	BD_TESTMOD_ATM_BER_STATS64_T BdTestModATMBERSTAT64;		\

#else
#define DEVIO_TESTMOD_STRUC_UNION													\
	BD_TESTMOD_INFO_T			BdTestModDigitalLoopBack;	/* Start (Set) ONLY! */	\
	BD_TESTMOD_CELL_VERIFY_T	BdTestModCellVerify;		/* Start (Set) ONLY! */	\
	BD_TESTMOD_VERIFY_STATS_T	BdTestModVerifyStats;		/* Get/Read ONLY! */	\
	BD_TESTMOD_ATM_BER_T		BdTestModATMBER;  			\
	BD_TESTMOD_ATM_BER_STATS_T  BdTestModATMBERSTAT;		\
	BD_TESTMOD_ATM_BER_INSERT_T BdTestModATMBERInject;		\

#endif 

/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Code Definitions
//////////////////////////////////////////////


#define DEVIO_TESTMOD_REQ_CODES											\
	BD_TEST_MOD_START_CODE = BD_TEST_MOD_CODES,							\
	BD_TEST_MOD_START_DIGITAL_LOOPBACK,				/* 0x00080001 */	\
	BD_TEST_MOD_STOP_DIGITAL_LOOPBACK,									\
	BD_TEST_MOD_START_CELL_VERIFY,										\
	BD_TEST_MOD_STOP_CELL_VERIFY,										\
	BD_TEST_MOD_GET_CELL_VERIFY_STATS,				/* 0x00080005 */	\
	BD_TEST_MOD_START_ATM_BER,											\
	BD_TEST_MOD_STOP_ATM_BER,											\
	BD_TEST_MOD_ATM_BER_STATUS,											\
	BD_TEST_MOD_ATM_BER_STATS,											\
	BD_TEST_MOD_ATM_BER_STATS_CLEAR,									\
	BD_TEST_MOD_ATM_BER_INJECT_ERRORS,									\
	BD_TEST_MOD_ATM_BER_STATS_64BITS,									\
	BD_TEST_MOD_END_CODE,												\




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




#endif		//#ifndef _DEVIOTESTMOD_H_

