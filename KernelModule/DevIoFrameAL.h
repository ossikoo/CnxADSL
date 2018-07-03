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
**		DevIoFrameAL.h
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
**		in the FrameAL module.
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
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/DevIoFrameAL.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
/**/	//This is a Page Eject character.
#ifndef _DEVIOFRAMEAL_H_
#define _DEVIOFRAMEAL_H_


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
//	FrameAL Module BackDoor Definitions
//////////////////////////////////////////////

// *************************************************
//	Parameters:	xxx->ReqCode = BD_FRAMEAL_GET_STATS
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdFrameALStats.XXX = ZZZ
//	NOTE:
//		Also see the 'Mini' Backdoor command "MINI_BD_FRAMEAL_GET_STATS"
//		which is a small subset of this command.
//
typedef struct BD_FRAMEAL_STATS_S			//	Get/Read ONLY!
{
	DWORD	NumRxBytes;
	DWORD	NumRxFrames;
	DWORD	NumTxBytes;
	DWORD	NumTxFrames;

	DWORD	NumAbortErrs;
	DWORD	NumCrcErrs;
	DWORD	NumTxUnderrunErrs;

} BD_FRAMEAL_STATS_T;						//	Get/Read ONLY!


/**/	//This is a Page Eject character.
// *****************************************************************************
//	ATM statistics are gathered on a per VC basis and on a per Link (Line)
//	basis.
//	These statistics are requested through the BackDoor by specifying
//	which VC (or Link) statistics are to be retrieved and stored for
//	the response.
// *****************************************************************************

// *****************************************************
//	Parameters:	xxx->ReqCode = BD_FRAMEAL_ATM_GET_STATS
//				xxx->Params.BdFrameALAtmStats.VcIndex = XYZ
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdFrameALAtmStats.VcIndex = XYZ
//				xxx->Params.BdFrameALAtmStats.XXX = ZZZ
//							where:	XYZ is a VC index reference value or
//										the special Link index reference value.
//									XXX is one of the structure elements for a
//										statistic (e.g., NumTxBytes, etc.).
//									ZZZ is the statistic value to be
//										returned.
// *****************************************************

// *****************************************************************************

////	The following define specifies that instead of a particular VC index,
////	this index value specifies to return statistics for all VCs on the link.
//#define BD_ATM_STATS_INDEX_FOR_LINK			0xFFFF
//
typedef struct BD_FRAMEAL_ATM_STATS_S			//	Get/Read ONLY!
{
	DWORD				VcIndex;
	
	DWORD				Vpi;
	DWORD				Vci;

	DWORD				NumTxBytes;
	DWORD				NumTxCells;
	DWORD				NumTxMgmtCells;
	DWORD				NumTxClpEqual0Cells;
	DWORD				NumTxClpEqual1Cells;

	DWORD				NumRxBytes;
	DWORD				NumRxCells;
	DWORD				NumRxMgmtCells;
	DWORD				NumRxClpEqual0Cells;
	DWORD				NumRxClpEqual1Cells;

	DWORD				NumRxHecErrs;				//	for Link ONLY!
	DWORD				NumRxCellAlignErrs;			//	for Link ONLY!
	DWORD				NumRxUnroutCellErrs;		//	for Link ONLY!

} BD_FRAMEAL_ATM_STATS_T;						//	Get/Read ONLY!


/**/	//This is a Page Eject character.
// *****************************************************************************
//	AAL statistics are gathered on a per VC basis and on a per Link (Line)
//	basis.
//	These statistics are requested through the BackDoor by specifying
//	which VC (or Link) statistics are to be retrieved and stored for the
//	response.
// *****************************************************************************

// *****************************************************
//	Parameters:	xxx->ReqCode = BD_FRAMEAL_AAL_GET_STATS
//				xxx->Params.BdFrameALAalStats.VcIndex = XYZ
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdFrameALAalStats.VcIndex = XYZ
//				xxx->Params.BdFrameALAalStats.XXX = ZZZ
//							where:	XYZ is a VC index reference value or
//										the special Link index reference value.
//									XXX is one of the structure elements for a
//										statistic (e.g., NumTxGoodBytes, etc.).
//									ZZZ is the statistic value to be
//										returned.
// *****************************************************

// *****************************************************************************

////	The following define specifies that instead of a particular VC index,
////	this index value specifies to return statistics for all VCs on the link.
//#define BD_AAL_STATS_INDEX_FOR_LINK			0xFFFF
//
typedef enum BD_TESTMOD_AAL_TYPES_E
{
    BD_TESTMOD_AAL_TYPE_START	=	0,
    BD_TESTMOD_AAL_TYPE_0		=	0,	//	=> AAL_TYPE_AAL0	1
    BD_TESTMOD_AAL_TYPE_1,			// 1	=> AAL_TYPE_AAL1	2
    BD_TESTMOD_AAL_TYPE_34,			// 2	=> AAL_TYPE_AAL34	4
    BD_TESTMOD_AAL_TYPE_5,			// 3	=> AAL_TYPE_AAL5	8
    BD_TESTMOD_AAL_TYPE_END			// 4
} BD_TESTMOD_AAL_TYPES_T;

typedef struct BD_FRAMEAL_AAL_STATS_S			//	Get/Read ONLY!
{
	DWORD					VcIndex;

	BD_TESTMOD_AAL_TYPES_T	AalType;				//	for VC (not Link) ONLY!
	DWORD					Vpi;
	DWORD					Vci;

	DWORD					NumTxGoodBytes;
	DWORD					NumTxGoodFrames;
	DWORD					NumTxDiscardedBytes;
	DWORD					NumTxDiscardedFrames;

	DWORD					NumRxGoodBytes;
	DWORD					NumRxGoodFrames;
	DWORD					NumRxDiscardedBytes;
	DWORD					NumRxDiscardedFrames;

	DWORD					NumRxCrcErrs;
	DWORD					NumRxInvalidLenErrs;
	DWORD					NumRxTimeoutErrs;

} BD_FRAMEAL_AAL_STATS_T;						//	Get/Read ONLY!


/**/	//This is a Page Eject character.
// **************************************************
//	Parameters:	xxx->ReqCode = BD_FRAMEAL_GET_CONFIG
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdFrameALConfig.RfcEncapsulationMode = ZZ1
//
// **************************************************
//	Parameters:	xxx->ReqCode = BD_FRAMEAL_SET_CONFIG
//				xxx->Params.BdFrameALConfig.RfcEncapsulationMode = ZZ1
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//
// **************************************************
// *****************************************************************************

typedef enum BD_FRAMEAL_ENCAPSULATION_E		// RfcEncapsulationMode values:
{
    BD_ENCAPSULATION_START			= 0,
    // NOTE:
    //		These values are from the
    //		ADSL Forum 99.102.0 spec.
    //	These values MUST MATCH the values
    //	from this document!!
    BD_ENCAPSULATION_PPPOA_VCMUX	= 0,	// PPP over ATM VXMUX					// was RFC2364_VCMUX
    BD_ENCAPSULATION_PPPOA_LLC,				// PPP over ATM LLC						// was RFC2364_LLC
    BD_ENCAPSULATION_BIPOA_LLC,				// Bridged IP over ATM LLCSNAP			// was RFC1483_LLC_BRIDGED
    BD_ENCAPSULATION_RIPOA_LLC,				// Routed IP over ATM LLCSNAP			// was RFC1483_LLC_ROUTED
    BD_ENCAPSULATION_BIPOA_VCMUX,			// Bridged IP over ATM VCMUX			// was RFC1483_VCMUX_BRIDGED
    BD_ENCAPSULATION_RIPOA_VCMUX,			// Routed IP over ATM VCMUX				// was RFC1483_VCMUX_ROUTED
    BD_ENCAPSULATION_IPOA,					// Classical IP over ATM				// was n/a
    BD_ENCAPSULATION_NATIVE_ATM,			// Native ATM							// was n/a
    BD_ENCAPSULATION_PROPRIETARY	= -1,	// Proprietary							// was n/a
    // The next three enumerations are added for CNXT-defined encapsulations,
    // they are not found in the ADSL Forum documentation.
    BD_ENCAPSULATION_PPPOA_NONE		= 0x80,	// PPP over ATM no encapsulation		// was RFC2364_NONE
    BD_ENCAPSULATION_BIPOA_NONE,			// Bridged IP over ATM no encapsulation	// was RFC1483_NULL
    BD_ENCAPSULATION_RIPOA_NONE,			// Routed IP over ATM no encapsulation	// was RFC1483_NULL
    BD_ENCAPSULATION_NONE			= 0xF8,	// Encapsulation performed above driver	// was n/a
    BD_ENCAPSULATION_END
} BD_FRAMEAL_ENCAPSULATION_T;

typedef struct BD_FRAMEAL_CFG_S
{
	DWORD	RfcEncapsulationMode;

} BD_FRAMEAL_CFG_T;




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Union of Structures Definitions
//////////////////////////////////////////////

#define DEVIO_FRAMEAL_STRUC_UNION												\
	BD_FRAMEAL_STATS_T			BdFrameALStats;			/* Get/Read ONLY! */	\
	BD_FRAMEAL_ATM_STATS_T		BdFrameALAtmStats;		/* Get/Read ONLY! */	\
	BD_FRAMEAL_AAL_STATS_T		BdFrameALAalStats;		/* Get/Read ONLY! */	\
	BD_FRAMEAL_CFG_T			BdFrameALConfig;								\




/**/	//This is a Page Eject character.
//////////////////////////////////////////////
//	BackDoor Code Definitions
//////////////////////////////////////////////


#define DEVIO_FRAMEAL_REQ_CODES											\
	BD_FRAMEAL_START_CODE = BD_FRAMEAL_CODES,							\
	BD_FRAMEAL_GET_STATS,							/* 0x00020001 */	\
	BD_FRAMEAL_ATM_GET_STATS,											\
	BD_FRAMEAL_AAL_GET_STATS,											\
	BD_FRAMEAL_GET_CONFIG,												\
	BD_FRAMEAL_SET_CONFIG,							/* 0x00020005 */	\
	BD_FRAMEAL_END_CODE,




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




#endif		//#ifndef _DEVIOFRAMEAL_H_

