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
**		DevIoADSLDiag.h
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
** $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4_ATM_Driver/KernelModule/DevIoADSLDiag.h $
** $Revision: 2 $
** $Date: 5/22/01 10:37a $
*******************************************************************************
******************************************************************************/
/**/	//This is a Page Eject character.
#ifndef _DEVIOADSLDIAG_H_
#define _DEVIOADSLDIAG_H_


/**/	//This is a Page Eject character.
// ***************************************************
//	Parameters:	xxx->ReqCode = BD_FALCON_GET_PERFORMANCE
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdADSLPerformanceStatus = ZZZ1
// *****************************************************************************

typedef enum BD_ADSL_ALARMS_ENUM_E
{
    BD_ADSL_ALARMS_NO_ALARM= 0, /* no alarm						  			*/
    BD_ADSL_ALARMS_LOF,       	/* lof_alarm                     			*/
    BD_ADSL_ALARMS_MAR,      	/* low_snr_margin_alarm           			*/
    BD_ADSL_ALARMS_ES,        	/* es_alarm                       			*/
    BD_ADSL_ALARMS_SES,       	/* severe error second            			*/
    BD_ADSL_ALARMS_LOS,       	/* los_alarm                      			*/
    BD_ADSL_ALARMS_LCD,       	/* lcd_alarm                      			*/
    BD_ADSL_ALARMS_SELF,      	/* self test                      			*/
    BD_ADSL_ALARMS_FR,			/* Fast Retrain					  			*/
    BD_ADSL_ALARMS_ELOF,      	/* excessive LOF within 15 minute 			*/
    BD_ADSL_ALARMS_ESES,      	/* excessive SES within 15 minute 			*/
    BD_ADSL_ALARMS_EUAS,      	/* excessive UAS within 15 minute 			*/
    BD_ADSL_ALARMS_ELOS,      	/* excessive LOS within 15 minute 			*/
    BD_ADSL_ALARMS_ELCD,       	/* excessive LCD within 15 minute			*/
    BD_ADSL_ALARMS_EFR,			/* excessive Fast Retrain within 15 minutes */
} BD_ADSL_ALARMS_ENUM_T;


// size of list of alarms
#define BD_ADSL_ALARMS_NUMBER 10

typedef struct BD_ADSL_ALARMS_S
{
	// 	if a slot has no alarm, that slot will contain
	// 	BD_ADSL_ALARMS_NO_ALARM= 0
	BD_ADSL_ALARMS_ENUM_T Alarms[BD_ADSL_ALARMS_NUMBER];

} BD_ADSL_ALARMS_T;

typedef struct BD_ADSL_ALARMS_STATUS_S
{
	BD_ADSL_ALARMS_T			UpStreamAlarms;
	BD_ADSL_ALARMS_T			DownStreamAlarms;
} BD_ADSL_ALARMS_STATUS_T;

/**/	//This is a Page Eject character.
// ***************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_GET_BIT_ALLOCATION_TABLE
//				xxx->Params.BdADSLBitAllocationTable.StartingBin = ZZZ
//				xxx->Params.BdADSLBitAllocationTable.EndingBin =  YYY
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdADSLBitAllocationTable.StartingBin = ZZZ1
//				xxx->Params.BdADSLBitAllocationTable.EndingBin =  YYY1
//				xxx->Params.BdADSLBitAllocationTable.Entry[] = XXX
// *****************************************************************************

typedef enum BD_ADSL_BIT_ALLOCATION_TYPE_ENUM
{
    UP_STREAM_BIN,
    DOWN_STREAM_BIN,
    NO_ASSIGNED_BIN
} BD_ADSL_BIT_ALLOCATION_TYPE_ENUM_T;

typedef struct BD_ADSL_BIT_ALLOCATION_ENTRY_S
{
	WORD									BinNumber;
	BD_ADSL_BIT_ALLOCATION_TYPE_ENUM_T   	BinStatus;
	WORD									BitsAssigned;
	WORD									BitCapacity;
} BD_ADSL_BIT_ALLOCATION_ENTRY_T;

//  BitCapacity contains an implied decimal point between the upper and lower bytes.

#define BD_ADSL_BIT_ALLOCATION_NUMBER 16
typedef struct BD_ADSL_BIT_ALLOCATION_S
{
	WORD StartingBin;
	WORD EndingBin;

	BD_ADSL_BIT_ALLOCATION_ENTRY_T Entry[BD_ADSL_BIT_ALLOCATION_NUMBER];
} BD_ADSL_BIT_ALLOCATION_T;


/**/	//This is a Page Eject character.
// ***************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_GET_SNR_TABLE
//				xxx->Params.BdADSLSnrTable.StartingBin = ZZZ
//				xxx->Params.BdADSLSnrTable.EndingBin =  YYY
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdADSLSnrTable.StartingBin = ZZZ1
//				xxx->Params.BdADSLSnrTable.EndingBin =  YYY1
//				xxx->Params.BdADSLSnrTable.Entry[] = XXX
// *****************************************************************************


typedef struct BD_ADSL_SNR_ENTRY_S
{
	WORD 					BinNumber;
	WORD					BinSNR;
} BD_ADSL_SNR_ENTRY_T;


 #define BD_ADSL_SNR_NUMBER 	16
typedef struct BD_ADSL_SNR_S
{
	WORD 	StartingBin;
	WORD	EndingBin;
	BD_ADSL_SNR_ENTRY_T Entry[BD_ADSL_SNR_NUMBER];
}BD_ADSL_SNR_T;


/**/	//This is a Page Eject character.
// ***************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_CONTROLLER_LOG_CONTROL
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.EnableLogsToAutolog 	= ZZZ1
// *****************************************************************************


typedef struct BD_ADSL_CONTROLLER_LOG_CLT_S
{
	BOOLEAN  EnableLogsToAutolog;
} BD_ADSL_CONTROLLER_LOG_CLT_T;


/**/	//This is a Page Eject character.
// ***************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_GET_G922_CAP
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//				xxx->Params.BdADSLG922Cap 	= ZZZ1
// *****************************************************************************


typedef struct BD_ADSL_GHS_CAP_ENTRY_S
{
	BOOLEAN		LocalEndpoint;
	BOOLEAN		RemoteEndpoint;
	BOOLEAN		Negiotiated;
} BD_ADSL_GHS_CAP_ENTRY_T;


typedef struct BD_ADSL_GHS_CAP_S
{
	BD_ADSL_GHS_CAP_ENTRY_T 		V8;
	BD_ADSL_GHS_CAP_ENTRY_T   	V8bis;
	BD_ADSL_GHS_CAP_ENTRY_T   	SilentPeriod;
	BD_ADSL_GHS_CAP_ENTRY_T   	GPLOAM;
	BD_ADSL_GHS_CAP_ENTRY_T   	G9221AnnexA;
	BD_ADSL_GHS_CAP_ENTRY_T	  	G9221AnnexB;
	BD_ADSL_GHS_CAP_ENTRY_T	  	G9221AnnexC;
	BD_ADSL_GHS_CAP_ENTRY_T   	G9222AnnexAB;
	BD_ADSL_GHS_CAP_ENTRY_T   	G9222AnnexC;
	BYTE							RemoteCountry;
	BYTE							RemoteVendorID[4];
	BYTE							RemoteSpecInfo[2];
} BD_ADSL_GHS_CAP_T;

/**/	//This is a Page Eject character.
// ***************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_LOCAL_G922_CAP
//              xxx->Params.BdADSLLocalG922Cap		= XXXX
//				xxx->Params.BdADSLLocalG922AnnexCap = YYYY
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//              xxx->Params.BdADSLLocalG922Cap		= XXXX1
//				xxx->Params.BdADSLLocalG922AnnexCap = YYYY1
// *****************************************************************************



typedef struct BD_ADSL_LOCAL_GHS_CAP_S
{
	WORD BdADSLLocalG922Cap;
#define ADSL_GHS_CAPABILITIES_V8 				1
#define ADSL_GHS_CAPABILITIES_V8bis 			2
#define ADSL_GHS_CAPABILITIES_SilentPeriod 		4
#define ADSL_GHS_CAPABILITIES_GPLOAM			8

	WORD BdADSLLocalG922AnnexCap;
#define ADSL_GHS_CAPABILITIES_922_1A			1
#define ADSL_GHS_CAPABILITIES_922_1B			2
#define ADSL_GHS_CAPABILITIES_922_1C		    4
#define ADSL_GHS_CAPABILITIES_922_2AB			8
#define ADSL_GHS_CAPABILITIES_922_2C			0x10

} BD_ADSL_LOCAL_GHS_CAP_T;


/**/	//This is a Page Eject character.
// *****************************************************************************
//	Parameters:	xxx->ReqCode = BD_ADSL_SET_AUTO_SENSE
//              xxx->Params.BdADSL_sys_autohandshake		= XXXX
//				xxx->Params.BdADSL_sys_autowiring 			= YYYY
//				xxx->Params.BdADSL_sys_autosense_wiresfirst = zzzz
//	Response:	xxx->ResultCode = BD_RESULT_SUCCESS, etc.
//              xxx->Params.BdADSL_sys_autohandshake		= XXXX1
//				xxx->Params.BdADSL_sys_autowiring 			= YYYY1
//				xxx->Params.BdADSL_sys_autosense_wiresfirst = zzzz1
// *****************************************************************************



typedef struct BD_ADSL_SET_AUTO_SENSE_S
{
	// Use DWORD for compatability with control panel Custom Driver Commands tab
	DWORD BdADSL_sys_autohandshake;
#define ADSL_GHS_TONE_MODE   	0
#define ADSL_TONE_GHS_MODE   	1
#define ADSL_G_HANDSHAKE_MODE	2
#define ADSL_G_TONE_MODE     	3

	DWORD BdADSL_sys_autowiring;
#define ADSL_WIRING_3_4_2_5		0
#define ADSL_WIRING_2_5_3_4		1
#define ADSL_WIRING_2_5		    2
#define ADSL_WIRING_3_4			3

	DWORD BdADSL_sys_autosense_wiresfirst;
} BD_ADSL_SET_AUTO_SENSE_T;



/**/	//This is a Page Eject character.
#define DEVIO_ADSL_DIAG_STRUC_UNION 								\
	BD_ADSL_ALARMS_STATUS_T			BdADSLAlaramsStatus; 			\
	BD_ADSL_BIT_ALLOCATION_T 		BdADSLBitAllocationTable;		\
	BD_ADSL_SNR_T					BdADSLSnrTable;					\
	BD_ADSL_CONTROLLER_LOG_CLT_T   	BdADSLControllerLogClt;		 	\
	BD_ADSL_GHS_CAP_T				BdADSLG_HSCapabilities;			\
	BD_ADSL_LOCAL_GHS_CAP_T			BdADSLocalG_HSCapabilities; 		\
	BD_ADSL_SET_AUTO_SENSE_T		BdADSLAutoSense;				\


#define DEVIO_ADSL_DIAG_REQ_CODES 							\
		BD_ADSL_DIAG_START_CODE = BD_ADSL_DIAG_CODES,		\
		BD_ADSL_GET_ALARMS,									\
		BD_ADSL_GET_SNR_TABLE,								\
		BD_ADSL_GET_CONTROLLER_LOG_CLT,						\
		BD_ADSL_SET_CONTROLLER_LOG_CLT,						\
		BD_ADSL_GET_BIT_ALLOCATION_TABLE,					\
		BD_ADSL_GET_GHS_CAP,								\
		BD_ADSL_GET_GHS_LOCAL_CAP,							\
		BD_ADSL_SET_GHS_LOCAL_CAP,							\
		BD_ADSL_SET_AUTO_SENSE,								\




#endif //_DEVIOADSDIAG_H_

