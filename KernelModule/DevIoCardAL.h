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
**		DevIoCardAL.h
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
**		in the CardAL module.
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
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/DevIoCardAL.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
/**/	//This is a Page Eject character.
#ifndef _DEVIOCARDAL_H_
#define _DEVIOCARDAL_H_


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
//	CardAL Module BackDoor Definitions
//////////////////////////////////////////////


// ***************************************************
//	Parameters:	xxx->ReqCode = BD_CARDAL_GET_TEXT_LOG
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->NeededSize = number of bytes to log (number of DWORDs * 4)
//			The next section will be repeated for as many driver text log messages
//			that will fit into this BD request.
//				xxx->Params.BdCardALTextLog[0] = ZZZ1 (string reference number, of type BD_LOG_MSGS_T)
//				xxx->Params.BdCardALTextLog[1] = ZZZ2 (if one or more parameters to log)
//				...
//				xxx->Params.BdCardALTextLog[n] = ZZZn
//
//	This request response will return as many driver text log reference
//	numbers and parameters (that are queued in the driver) as it can fit
//	into the response.
//	Each driver text log will be of type "BD_DRIVER_LOG_T", as defined below.
//		xxx->FirstDword.Words.Size = ZZZ1 (Text Msg Size, in DWORDs, not including the
//			 								1st DWORD a combination of Size and Type)
//		xxx->FirstDword.Words.Type = ZZZ2 (string reference number, type BD_TEXT_MSGS_T)
//		xxx->DwordArray[0] = XYZ1 (if one or more parameters to log)
//		...
//		xxx->DwordArray[n] = XYZn (last parameter to log, if any)

#define NUM_PARAMS_0				0
#define NUM_PARAMS_1				1
#define NUM_PARAMS_2				2
#define NUM_PARAMS_3				3
#define NUM_PARAMS_4				4
#define NUM_PARAMS_6				6
#define MAX_NUM_PARAMS				(BD_MINI_DATA_DWORDS - 1)
#define MAX_DWORDS_IN_TEXT_LOG_MSG	MAX_NUM_PARAMS
#define MAX_NUM_LOG_PARAMS			(MAX_NUM_PARAMS - 2)	// subtract 2 for 64-bit timestamp value

#define BY_1_WORD					16

#define MASK_OFF_MSW				0x0000FFFF
#define MASK_OFF_LSW				0xFFFF0000


/**/	//This is a Page Eject character.
typedef enum BD_TEXT_MSGS_E
{
    //	TEXT_MSG  ==>  TM

    BD_TM_ZERO_NOT_USED			= 0,	// value of zero is not used!

    BD_TM_PACKED_STRING,				// NOT USED in string lookup table ("TextLogStrings")
    //	since there is no predefined string constant.

    BD_TM_ADSL_PSD_TEST_MODE,
    BD_TM_ADSL_RX_IDLE_CHAR,
    BD_TM_ADSL_RX_IDLE_TIMER,
    BD_TM_API_HW_IF_MODE,
    // ???? TO USE OR NOT TO USE, THAT IS THE QUESTION ????
    BD_TM_ATM_SERVER,					// ???? TO USE OR NOT TO USE, THAT IS THE QUESTION ????
    BD_TM_ATM_TX_CELL_A_FILE,
    BD_TM_ATM_TX_CELL_B_FILE,
    BD_TM_CELL_BURST_SIZE,
    BD_TM_DP_AUTO_DOWNLOAD,
    BD_TM_DP_BOOT_FILENAME,
    BD_TM_DP_BOOT_JUMPADDRESS,
    BD_TM_DP_FILENAME,
    BD_TM_DP_JUMP_ADDRESS,
    BD_TM_FPGA_PROGRAM_ENABLE,
    BD_TM_FPGA_PROGRAM_FILE,
    BD_TM_LINE_AUTO_ACTIVATION,
    BD_TM_LINE_PERSISTENCE_TIMER,
    BD_TM_LINE_PERSISTENT_ACTIVATION,
    BD_TM_LT_MODE,
    BD_TM_MAX_TRANSMIT,
    BD_TM_NUMBER_OF_LINES,
    BD_TM_RX_BUF_ALLOC_LIMIT,
    BD_TM_RX_BUF_RATIO_LIMIT,
    BD_TM_RX_FIFO_SIZE,
    BD_TM_RX_INTERRUPT_RATE,
    BD_TM_RX_MAX_LATENCY,
    BD_TM_RX_MIN_LATENCY,
    BD_TM_RX_SPEED,
    BD_TM_TX_FIFO_SIZE,
    BD_TM_TX_INTERRUPT_RATE,
    BD_TM_TX_MAX_LATENCY,
    BD_TM_TX_MIN_LATENCY,
    BD_TM_TX_SPEED,
    BD_TM_INVALID_MAC_ADDR,
    BD_TM_INVALID_STRING,

    BD_TM_END
} BD_TEXT_MSGS_T;


/**/	//This is a Page Eject character.
//
//	These enumeration values are to be typed as a DWORD.
//	Where the most significant word (upper 16 bits) defines how many
//	parameters (DWORDs) are to follow (to be printed/displayed after
//	the indicated text string) and the least significant word (lower
//	16 bits) defines the enumeration value used to indicate the text
//	string (to be printed/displayed).
//

typedef enum BD_LOG_MSGS_E
{
    //	LOG_MSG  ==>  LM

    BD_LM_ZERO_NOT_USED				= 0,	// value of zero is not used!

    // SPECIAL CASE (for BD_LM_PACKED_STRING):
    //		The most significant word will be the number of DWORDs that
    //		follow (a variable number, possibly different each time),
    //		containing a packed string (sz).
    //		This enumeration value is NOT USED in string lookup table
    //		("TextLogStrings") since there is no predefined string constant.
    BD_LM_PACKED_STRING				= BD_TM_PACKED_STRING,

    BD_LM_ADSL_PSD_TEST_MODE		= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_ADSL_PSD_TEST_MODE,
    BD_LM_ADSL_RX_IDLE_CHAR			= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_ADSL_RX_IDLE_CHAR,
    BD_LM_ADSL_RX_IDLE_TIMER		= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_ADSL_RX_IDLE_TIMER,
    BD_LM_API_HW_IF_MODE			= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_API_HW_IF_MODE,
    // ???? TO USE OR NOT TO USE, THAT IS THE QUESTION ????
    BD_LM_ATM_SERVER				= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_ATM_SERVER,	// ???? TO USE OR NOT TO USE, THAT IS THE QUESTION ????
    /**/BD_LM_ATM_TX_CELL_A_FILE		= (MAX_NUM_LOG_PARAMS << BY_1_WORD) | BD_TM_ATM_TX_CELL_A_FILE,
    /**/BD_LM_ATM_TX_CELL_B_FILE		= (MAX_NUM_LOG_PARAMS << BY_1_WORD) | BD_TM_ATM_TX_CELL_B_FILE,
    BD_LM_CELL_BURST_SIZE			= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_CELL_BURST_SIZE,
    BD_LM_DP_AUTO_DOWNLOAD			= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_DP_AUTO_DOWNLOAD,
    /**/BD_LM_DP_BOOT_FILENAME			= (MAX_NUM_LOG_PARAMS << BY_1_WORD) | BD_TM_DP_BOOT_FILENAME,
    BD_LM_DP_BOOT_JUMPADDRESS		= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_DP_BOOT_JUMPADDRESS,
    /**/BD_LM_DP_FILENAME				= (MAX_NUM_LOG_PARAMS << BY_1_WORD) | BD_TM_DP_FILENAME,
    BD_LM_DP_JUMP_ADDRESS			= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_DP_JUMP_ADDRESS,
    BD_LM_FPGA_PROGRAM_ENABLE		= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_FPGA_PROGRAM_ENABLE,
    /**/BD_LM_FPGA_PROGRAM_FILE			= (MAX_NUM_LOG_PARAMS << BY_1_WORD) | BD_TM_FPGA_PROGRAM_FILE,
    BD_LM_LINE_AUTO_ACTIVATION		= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_LINE_AUTO_ACTIVATION,
    BD_LM_LINE_PERSISTENCE_TIMER	= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_LINE_PERSISTENCE_TIMER,
    BD_LM_LINE_PERSISTENT_ACTIVATION= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_LINE_PERSISTENT_ACTIVATION,
    BD_LM_LT_MODE					= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_LT_MODE,
    BD_LM_MAX_TRANSMIT				= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_MAX_TRANSMIT,
    BD_LM_NUMBER_OF_LINES			= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_NUMBER_OF_LINES,
    BD_LM_RX_BUF_ALLOC_LIMIT		= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_RX_BUF_ALLOC_LIMIT,
    BD_LM_RX_BUF_RATIO_LIMIT		= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_RX_BUF_RATIO_LIMIT,
    BD_LM_RX_FIFO_SIZE				= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_RX_FIFO_SIZE,
    BD_LM_RX_INTERRUPT_RATE			= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_RX_INTERRUPT_RATE,
    BD_LM_RX_MAX_LATENCY			= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_RX_MAX_LATENCY,
    BD_LM_RX_MIN_LATENCY			= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_RX_MIN_LATENCY,
    BD_LM_RX_SPEED					= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_RX_SPEED,
    BD_LM_TX_FIFO_SIZE				= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_TX_FIFO_SIZE,
    BD_LM_TX_INTERRUPT_RATE			= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_TX_INTERRUPT_RATE,
    BD_LM_TX_MAX_LATENCY			= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_TX_MAX_LATENCY,
    BD_LM_TX_MIN_LATENCY			= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_TX_MIN_LATENCY,
    BD_LM_TX_SPEED					= (NUM_PARAMS_1 << BY_1_WORD) | BD_TM_TX_SPEED,
    BD_LM_INVALID_MAC_ADDR			= (NUM_PARAMS_6 << BY_1_WORD) | BD_TM_INVALID_MAC_ADDR,
    BD_LM_INVALID_STRING			= (NUM_PARAMS_0 << BY_1_WORD) | BD_TM_INVALID_STRING,

    BD_LM_END
} BD_LOG_MSGS_T;

#define BD_TEXT_LOG_ATM_TX_CELL_A_FILE	"Default value for AtmTxCellAFile used "
#define BD_TEXT_LOG_ATM_TX_CELL_B_FILE	"Default value for AtmTxCellBFile used "
#define BD_TEXT_LOG_DP_BOOT_FILENAME	"Default value for DpBootFilename used "
#define BD_TEXT_LOG_DP_FILENAME			"Default value for DpFilename used "
#define BD_TEXT_LOG_FPGA_PROGRAM_FILE	"Default value for FpgaProgramFile used "

typedef struct BD_LOG_TYPE_SIZE_S
{
	WORD		Type;
	WORD		Size;
} BD_LOG_TYPE_SIZE_T;

typedef union BD_LOG_1ST_DWORD_U
{
	DWORD					Dword;
	BD_LOG_TYPE_SIZE_T		Words;
} BD_LOG_1ST_DWORD_T;

typedef struct BD_DRIVER_LOG_S
{
	BD_LOG_1ST_DWORD_T		FirstDword;
	DWORD					DwordArray [MAX_DWORDS_IN_TEXT_LOG_MSG];
} BD_DRIVER_LOG_T;

typedef struct BD_DRIVER_LOG_WITH_TIME_S
{
	BD_LOG_1ST_DWORD_T		FirstDword;
	LONGLONG				SystemTime;
	DWORD					DwordArray [MAX_NUM_LOG_PARAMS];
} BD_DRIVER_LOG_WITH_TIME_T;



/**/	//This is a Page Eject character.
// ****************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_GET_CONN_CONFIG
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdAdslConnConfig.NumOfVc = ZZZ1
//				xxx->Params.BdAdslConnConfig.VcArray[n].Vpi = ZZZ3
//				xxx->Params.BdAdslConnConfig.VcArray[n].Vci = ZZZ4
//				xxx->Params.BdAdslConnConfig.VcArray[n].Pcr = ZZZ5
//													where "n" is 0 to (ZZZ1-1)
//
// ****************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_SET_CONN_CONFIG
//				xxx->Params.BdAdslConnConfig.NumOfVc = ZZZ1
//				xxx->Params.BdAdslConnConfig.VcArray[n].Vpi = ZZZ3
//				xxx->Params.BdAdslConnConfig.VcArray[n].Vci = ZZZ4
//				xxx->Params.BdAdslConnConfig.VcArray[n].Pcr = ZZZ5
//													where "n" is 0 to (ZZZ1-1)
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//
// ****************************************************
// *****************************************************************************

//				xxx->Params.BdAdslConnConfig.VcArray[n].Pcr = ZZZ5
typedef enum BD_ADSL_PCR_KBPS_E
{
    BD_PCR_START					= 0,
    BD_PCR_SAME_AS_LINE				= 0,
    BD_PCR_16_KBPS,					// 1
    BD_PCR_32_KBPS,
    BD_PCR_48_KBPS,
    BD_PCR_64_KBPS,					// 4
    BD_PCR_80_KBPS,
    BD_PCR_96_KBPS,
    BD_PCR_112_KBPS,
    BD_PCR_128_KBPS,				// 8
    BD_PCR_144_KBPS,
    BD_PCR_160_KBPS,
    BD_PCR_176_KBPS,
    BD_PCR_192_KBPS,				// 12
    BD_PCR_208_KBPS,
    BD_PCR_224_KBPS,
    BD_PCR_240_KBPS,
    BD_PCR_256_KBPS,				// 16
    BD_PCR_272_KBPS,
    BD_PCR_288_KBPS,
    BD_PCR_304_KBPS,
    BD_PCR_320_KBPS,				// 20
    BD_PCR_336_KBPS,
    BD_PCR_352_KBPS,
    BD_PCR_368_KBPS,
    BD_PCR_384_KBPS,				// 24
    BD_PCR_400_KBPS,
    BD_PCR_416_KBPS,
    BD_PCR_432_KBPS,
    BD_PCR_448_KBPS,				// 28
    BD_PCR_464_KBPS,
    BD_PCR_480_KBPS,
    BD_PCR_496_KBPS,
    BD_PCR_512_KBPS,				// 32
    BD_PCR_528_KBPS,
    BD_PCR_544_KBPS,
    BD_PCR_560_KBPS,
    BD_PCR_576_KBPS,				// 36
    BD_PCR_592_KBPS,
    BD_PCR_608_KBPS,
    BD_PCR_624_KBPS,
    BD_PCR_640_KBPS,				// 40
    BD_PCR_656_KBPS,
    BD_PCR_672_KBPS,
    BD_PCR_688_KBPS,
    BD_PCR_704_KBPS,				// 44
    BD_PCR_720_KBPS,
    BD_PCR_736_KBPS,
    BD_PCR_752_KBPS,
    BD_PCR_768_KBPS,				// 48
    BD_PCR_784_KBPS,
    BD_PCR_800_KBPS,
    BD_PCR_816_KBPS,
    BD_PCR_832_KBPS,				// 52
    BD_PCR_END
} BD_ADSL_PCR_KBPS_T;

typedef struct BD_ADSL_CONN_VPI_VCI_S
{
	BYTE						Vpi;		// Virtual Path Identifier
	WORD						Vci;		// Virtual Channel Identifier
	BD_ADSL_PCR_KBPS_T			Pcr;		// Peak Cell Rate
} BD_ADSL_CONN_VPI_VCI_T;

typedef struct BD_ADSL_CONN_CONFIG_S
{
	DWORD						NumOfVc;
	BD_ADSL_CONN_VPI_VCI_T		VcArray [DEFAULT_NUMBER_OF_LINES];
	// leave these out for now (maybe forever?)!
	//	DWORD						MaxRateDownstream;
	//	DWORD						MaxRateUpstream;
} BD_ADSL_CONN_CONFIG_T;


/**/	//This is a Page Eject character.
// ***************************************************
//	Parameters:	xxx->ReqCode = BD_CARDAL_GET_REGISTER
//				xxx->Params.BdCardALRegister.RegOffset = ZZZ
//				xxx->Params.BdCardALRegister.RegMask = ZZZ
//				xxx->Params.BdCardALRegister.RegDataSize = XXX1
//				xxx->Params.BdCardALRegister.RegAccess = XXX2
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdCardALRegister.RegValue = ZZZ
//
// ***************************************************
//	Parameters:	xxx->ReqCode = BD_CARDAL_SET_REGISTER
//				xxx->Params.BdCardALRegister.RegOffset = ZZZ
//				xxx->Params.BdCardALRegister.RegMask = ZZZ
//				xxx->Params.BdCardALRegister.RegValue = ZZZ
//				xxx->Params.BdCardALRegister.RegDataSize = XXX1
//				xxx->Params.BdCardALRegister.RegAccess = XXX2
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//
// ***************************************************
// *****************************************************************************

//				xxx->Params.BdCardALRegister.RegDataSize = XXX1
typedef enum BD_CARDAL_REG_DATA_SIZE_E
{
    BD_ADSL_REG_DATA_SIZE_START			=	0,
    BD_ADSL_REG_DATA_SIZE_DWORD_4		=	0,
    BD_ADSL_REG_DATA_SIZE_WORD_2,		// 1
    BD_ADSL_REG_DATA_SIZE_BYTE_1,		// 2
    BD_ADSL_REG_DATA_SIZE_END			// 3
} BD_ADSL_REG_DATA_SIZE_T;
#define DEFAULT_BD_CARDAL_REG_DATA_SIZE		BD_ADSL_REG_DATA_SIZE_DWORD_4


//				xxx->Params.BdCardALRegister.RegAccess = XXX2
typedef enum BD_CARDAL_REG_ACCESS_E
{
    BD_ADSL_REG_ACCESS_START			=	0,
    BD_ADSL_REG_ACCESS_MEMORY			=	0,
    BD_ADSL_REG_ACCESS_HOBBES,			// 1
    BD_ADSL_REG_ACCESS_ALCATEL_ADSL,	// 2
    BD_ADSL_REG_ACCESS_BASIC2,			// 3
    BD_ADSL_REG_ACCESS_EEPROM,			// 4
    BD_ADSL_REG_ACCESS_ARM,				// 5
    BD_ADSL_REG_ACCESS_MICROIF,			// 6
    BD_ADSL_REG_ACCESS_END				// 7
} BD_ADSL_REG_ACCESS_T;
#define DEFAULT_BD_CARDAL_REG_ACCESS		BD_ADSL_REG_ACCESS_MEMORY


typedef struct BD_CARDAL_REGISTER_S
{
	DWORD						RegOffset;
	DWORD						RegMask;
	DWORD						RegValue;
	BD_ADSL_REG_DATA_SIZE_T		RegDataSize;
	BD_ADSL_REG_ACCESS_T		RegAccess;
	DWORD						VarLength;
	char						VarBuff[4];
} BD_CARDAL_REGISTER_T;


/**/	//This is a Page Eject character.
typedef enum BD_ADSL_MODEM_STATUS_E
{
    BD_MODEM_START					= 0,
    BD_MODEM_DOWN					= 0,
    BD_MODEM_ACTIVATION,			// 1
    BD_MODEM_TRANSCEIVER_TRAINING,
    BD_MODEM_CHANNEL_ANALYSIS,
    BD_MODEM_EXCHANGE,
    BD_MODEM_ACTIVATED,				// 5
    BD_MODEM_WAITING_INIT,
    BD_MODEM_INITIALIZING,
    BD_MODEM_UNKNOWN,
    BD_MODEM_END					// 9
} BD_ADSL_MODEM_STATUS_T;


/**/	//This is a Page Eject character.
// ***************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_INIT_DATA_PUMP	//	Execute ONLY!
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
typedef struct BD_ADSL_INIT_DP_S
{
	DWORD						Unknown_Need_More_Info;
} BD_ADSL_INIT_DP_T;


// ***************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_GET_MODEM_DATA
//				xxx->Params.BdAdslModemBlock.ModemBlockStartAddr = xyz
//				xxx->Params.BdAdslModemBlock.ModemBlockLength = xx
//				xxx->Params.BdAdslModemBlock.pModemBlockData = where to store block
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdAdslModemBlock.ModemBlockLength = xx
//				xxx->Params.BdAdslModemBlock.pModemBlockData[0..xx-1] = data block read
//
//	Command to do CTRL-E (DP) Modem Load Block command.

//	WARNING:	BD_MINI_DATA_DWORDS should be AT LEAST 3!!!
//	The following definition gives the size (in DWORDS) of the 'Mini' BackDoor
//	'Param' space left after the elements "ModemBlockStartAddr" and
//	"ModemBlockLength" below.
#define	BD_MODEM_BLOCK_DATA_IN_DWORDS	BD_MINI_DATA_DWORDS - 2

typedef BYTE	BD_MODEM_BLOCK_DATA_T [BD_MODEM_BLOCK_DATA_IN_DWORDS * sizeof( DWORD )];

//	The following definition gives the size (in BYTES) of the 'Mini' BackDoor
//	'Param' space left after the elements "ModemBlockStartAddr" and
//	"ModemBlockLength" below.
#define BD_MODEM_BLOCK_DATA_SIZE		sizeof( BD_MODEM_BLOCK_DATA_T )

typedef struct BD_ADSL_MODEM_DATA_S
{
	DWORD						ModemBlockStartAddr;
	DWORD						ModemBlockLength;
	BD_MODEM_BLOCK_DATA_T		ModemBlockData;
} BD_ADSL_MODEM_DATA_T;


/**/	//This is a Page Eject character.
// ************************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_GET_DATA_PATH_SELECTION
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdAdslDataPathSelect.AdslDataPathSelection = xx
//
// ************************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_SET_DATA_PATH_SELECTION
//				xxx->Params.BdAdslDataPathSelect.AdslDataPathSelection = xx
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//

typedef enum BD_ADSL_DATA_PATH_SELECTS_E
{
    BD_ADSL_PATH_SELECT_START			=	0,
    BD_ADSL_PATH_SELECT_AUTOMATIC		=	0,
    BD_ADSL_PATH_SELECT_FAST,			// 1
    BD_ADSL_PATH_SELECT_INTERLEAVED,	// 2
    BD_ADSL_PATH_SELECT_UNKNOWN,		// 3
    BD_ADSL_PATH_SELECT_END				// 4
} BD_ADSL_DATA_PATH_SELECTS_T;
#define DEFAULT_BD_ADSL_DATA_PATH_SELECTION		BD_ADSL_PATH_SELECT_FAST

typedef struct BD_ADSL_DATA_PATH_SELECT_S
{
	BD_ADSL_DATA_PATH_SELECTS_T		AdslDataPathSelection;
} BD_ADSL_DATA_PATH_SELECT_T;


/**/	//This is a Page Eject character.
// ****************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_GET_LINE_STATUS
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdAdslLineStatus.XXX = ZZZ

typedef struct BD_ADSL_LINE_STATUS_S
{
	DWORD						LineSpeedUpOrFar;
	DWORD						LineSpeedDownOrNear;
	BD_ADSL_MODEM_STATUS_T		LineState;
} BD_ADSL_LINE_STATUS_T;


// ****************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_GET_LINE_STATE
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdAdslLineState.XXX = ZZZ

typedef struct BD_ADSL_LINE_STATE_S
{
	BD_ADSL_MODEM_STATUS_T		LineState;
} BD_ADSL_LINE_STATE_T;


/**/	//This is a Page Eject character.
// ****************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_GET_DP_VERSIONS
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdAdslDpVersions.XXX = ZZZ

typedef struct BD_ADSL_DP_VERSIONS_S
{
	DWORD						AdslVersionNear;		// 0 to 0xFF
	DWORD						AdslVersionFar;			// 0 to 0xFF
	DWORD						AdslVendorNear;			// 0 to 0xFFFF
	DWORD						AdslVendorFar;			// 0 to 0xFFFF
	DWORD						AdslDpSwVerMajor;		// 0 to 0xF
	DWORD						AdslDpSwVerMinor;		// 0 to 0xF
	DWORD						AdslDpSwVerSubMinor;	// 0 to 0xFF
} BD_ADSL_DP_VERSIONS_T;


// *******************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_GET_BUS_CONTROLLER
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdAdslBusController = XYZ
//											(ASCII string, zero terminated)
// ***************************************************

#define	BD_BUS_CTRLR_BASIC_2p1		"BASIC 2.1"
#define	BD_BUS_CTRLR_BASIC_2p15		"BASIC 2.15"

#define	BD_BUS_CTRLR_UNKNOWN		"*Unknown!*"


/**/	//This is a Page Eject character.
// ***********************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_GET_CONFIG
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdAdslConfig.AdslDpRateMode = ZZ1
//
// ***********************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_SET_CONFIG
//				xxx->Params.BdAdslConfig.AdslDpRateMode = ZZ1
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//
// ***********************************************
// *****************************************************************************

typedef enum BD_ADSL_DP_RATE_MODE_E			// AdslDpRateMode values:
{
    BD_ADSL_DP_RATE_G_DOT_DMT		=	0,		// G.DMT, Rate Unlimited  /  Full Rate
    BD_ADSL_DP_RATE_G_DOT_LITE,					// G.Lite, Rate Limited (1.5 Mbps)
    BD_ADSL_DP_RATE_T1_DOT_413,					// T1.413, ANSI
    BD_ADSL_DP_RATE_NOT_AVAIL,					// None/NotAvailable/etc., not in Showtime
    BD_ADSL_DP_RATE_END
} BD_ADSL_DP_RATE_MODE_T;

typedef struct BD_ADSL_CFG_S
{
	DWORD	AdslDpRateMode;

} BD_ADSL_CFG_T;




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Union of Structures Definitions
//////////////////////////////////////////////


#define DEVIO_CARDAL_STRUC_UNION											\
	BD_CARDAL_REGISTER_T			BdCardALRegister;						\
	BD_MINI_DATA_BLOCK_DWORDS_T		BdCardALTextLog;						\
	BD_ADSL_CFG_T					BdAdslConfig;							\


/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Code Definitions
//////////////////////////////////////////////


#define DEVIO_CARDAL_REQ_CODES											\
	BD_CARDAL_START_CODE = BD_CARDAL_CODES,			/* 0x00070000 */	\
	BD_CARDAL_GET_REGISTER,												\
	BD_CARDAL_SET_REGISTER,												\
	BD_USER_ACTIVATE_LINE,												\
	BD_USER_DEACTIVATE_LINE,											\
	BD_CARDAL_GET_TEXT_LOG,												\
	BD_ADSL_GET_CONFIG,													\
	BD_ADSL_SET_CONFIG,													\
	BD_CARDAL_END_CODE,




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




#endif		//#ifndef _DEVIOCARDAL_H_

