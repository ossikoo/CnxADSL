/******************************************************************************
*******************************************************************************
****	Copyright (c) 1997, 1998
****	Rockwell Semiconductor Systems
****	Personal Computing Division
****	All Rights Reserved
****
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****
*******************************************************************************
**
**	MODULE NAME:
**		CardAL (Card Abstraction Layer)
**
**	FILE NAME:
**		CardALV.h
**
**	ABSTRACT:
**		Card level Abstraction Layer for CDSL.  This file/module abstracts
**		(hides the details of) card specific (CDSL) functions and presents
**		(hopefully) a constant interface to the upper level modules/functions
**		(CardMgmt) no matter what the actual card is (CDSL, ADSL, ISDN, etc.).
**		This files contains private defines for Card Abstraction Layer Module.
**		The private context structure is defined in this file.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/CardALV.h $
** $Revision: 2 $
** $Date: 7/23/01 8:02a $
*******************************************************************************
******************************************************************************/
#ifndef _CARDALV_H_
#define _CARDALV_H_
#include "Common.h"
#include "Product.h"
#include "CardAL.h"
#include "ChipALCdsl.h"

#ifdef _CARDAL_C_
	DWORD			CardALDebugFlag = 0L;
#else
	extern DWORD	CardALDebugFlag;
#endif

#if PROJECTS_NAME == PROJECT_NAME_TIGRIS
	#include "CardALTigrisDiag.h"
	#include "CardALTigrisV.h"
	#include "CardALADSLDiag.h"
#endif

#if PROJECTS_NAME != PROJECT_NAME_CHEETAH_1
/*******************************************************************************
Private Globals
*******************************************************************************/

//*
//* This table (gPersistData) defines all configuration key names and
//* defaults for CardAl.  The actual data is stored in an external source, as
//* determined by UtilReadPersistentData()
//*
#ifdef _CARDAL_C_
typedef enum CARDAL_CFG_PARM_NAMES_E
{
    CDAL_CFG_START = 0,
    CDAL_CFG_DEBUG_FLAG = CDAL_CFG_START,
    CDAL_CFG_MAX
}CARDAL_CFG_PARM_NAMES_T;

#endif	// Private Globals for _CARDAL_C_

/*******************************************************************************
Modular Definitions
*******************************************************************************/
typedef
VOID * (* FN_LINE_CARD_INIT)(
    IN	VOID 					* AdapterContext,
    IN	VOID 					* CardAlContext
);

typedef
VOID (* FN_LINE_CARD_SHUTDOWN)(
    IN	VOID 					* UserContext
);

typedef
NTSTATUS (* FN_LINE_CARD_RESET)(
    IN	VOID 					* UserContext
);

typedef
NTSTATUS (* FN_LINE_CARD_SET_LINE_STATE)(
    IN	VOID 					* UserContext,
    CARD_LINE_STATE_T			NewState
);

typedef
NTSTATUS (* FN_LINE_CARD_ENABLE)(
    IN	VOID 					* UserContext
);

typedef
NTSTATUS (* FN_LINE_CARD_DISABLE)(
    IN	VOID 					* UserContext
);

#define MAX_MODULE_DESCRIPTION			40

typedef struct MODULE_LINE_CARD_METHODS_S
{
	#ifdef DBG
	CHAR							Description[MAX_MODULE_DESCRIPTION];
	#endif
	FN_LINE_CARD_INIT				Init;			// Optional Init Card. Allocate resources
	FN_LINE_CARD_SHUTDOWN			Shutdown;		// Optional Shutdown Card and remove Allocations
	FN_LINE_CARD_ENABLE				Enable;			// Optional Enable Fully initialized Card
	FN_LINE_CARD_DISABLE			Disable;		// Disable Fully Initialized card, but maintain context
	FN_LINE_CARD_RESET				Reset;			// Optional Same as Init, but do not do Allocations
	FN_LINE_CARD_SET_LINE_STATE		SetLineState;	// Set New Line State

} MODULE_LINE_CARD_METHODS_T;

typedef struct MODULE_CARDAL_OPTIONAL_S
{
	#ifdef DBG
	CHAR							Description[MAX_MODULE_DESCRIPTION];
	#endif
	FN_LINE_CARD_INIT				Init;			// Optional Init Card. Allocate resources
	FN_LINE_CARD_SHUTDOWN			Shutdown;		// Optional Shutdown Card and remove Allocations
	FN_LINE_CARD_ENABLE				Enable;			// Optional Enable Fully initialized Card
	FN_LINE_CARD_DISABLE			Disable;		// Disable Fully Initialized card, but maintain context
} MODULE_CARDAL_OPTIONAL_T;

typedef enum CDAL_MODULES_E
{
    CDAL_MODULES_START = 0,
    CDAL_LINE_CONTROLLER = CDAL_MODULES_START,
    CDAL_LINE_END,
    CDAL_OPTIONAL_START = CDAL_LINE_END,
    CDAL_OPTIONAL_UTIL = CDAL_OPTIONAL_START,
    CDAL_DIAGS,
    CDAL_ADSL_DIAGS,
    /*
    CDAL_OPTIONAL_STATUS,
    CDAL_OPTIONAL_CELL_VERIFY,
    */
    CDAL_OPTIONAL_END,
    CDAL_MODULES_END = CDAL_OPTIONAL_END,
} CDAL_MODULES_T;



//*
//*	Begin Private CardAl Data definitions
//*
#ifdef _CARDAL_C_

//*
//* Load array of all possible Line Cards Controllers
//*
#if ! defined CARDAL_LC_FNS
#pragma message(__WARNING__"CARDAL: No LineCards Defined") 
MODULE_LINE_CARD_METHODS_T gLineCards[1] = {NULL};

#else

MODULE_LINE_CARD_METHODS_T gLineCards[] =
{
	CARDAL_LC_FNS
};

#endif
#define	MAX_LINE_CARDS	(sizeof(gLineCards) / sizeof(MODULE_LINE_CARD_METHODS_T))

//*
//* Load array of all Optional Card Controllers
//*
#if ! defined CARDAL_DIAG_MOD
	#define CARDAL_DIAG_MOD
#endif

#if ! defined CARDAL_ADSL_DIAG_MOD
	#define CARDAL_ADSL_DIAG_MOD
#endif 

#if ! defined CARDAL_OPT_MOD
MODULE_CARDAL_OPTIONAL_T gOptCardModules[1] = {NULL};
#else
MODULE_CARDAL_OPTIONAL_T gOptCardModules[] =
{
	CARDAL_OPT_MOD
	CARDAL_DIAG_MOD
	CARDAL_ADSL_DIAG_MOD
};

#endif
#define	MAX_OPT_CARD_MODS	(sizeof(gOptCardModules) / sizeof(MODULE_CARDAL_OPTIONAL_T))

#endif // _CARDAL_C_

//*
//* Modular Functions
//*
VOID * CDALGetModuleHandle(
    CDSL_ADAPTER_T * pThisAdapter,
    CDAL_MODULES_T CDALModule
);

DPCCALL NDIS_STATUS CardAlRegStoreHeadEnd( DWORD VendorId );

#endif	// != PROJECT_NAME_CHEETAH_1

//*
//* Begin Legacy definitions of CardAl
//*
///////////////////////////////////////////////////////////////////////////////
//	Defines
///////////////////////////////////////////////////////////////////////////////


#define		BIT_MASK_NOT_USED					0

#define		MAX_DP_FILE_SIZE_ALLOCATED			350 * 1024
#define		MAX_DP_BOOT_FILE_SIZE_ALLOCATED		16 * 1024
#define		MAX_DP_MODEM_DATA_SIZE				0x400

#define		DP_DOWNLOAD_START_ADDRESS			0

#define		ADSL_NO_CMD_IN_PROG					0

//	The value of 52,000,000 is based on running
//	through the loop this equate is used in and
//	timing how many times the loop executed in
//	250ms then extrapolating out to 1000ms.  The
//	value was rounded up.  The test was run on
//	a 450MHz P-II.
#define		GEN_TMR_HARD_LOOP_COUNT				52000000
#define		GEN_TMR_WAIT_EVENT_RECEIVED			0
#define		GEN_TMR_WAIT_TIMOUT_OCCURRED		1
#define		GEN_TMR_HW_INTERRUPT_FAILURE		2




///////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
///////////////////////////////////////////////////////////////////////////////

//
//	CardALMgmt.c
//

DWORD cardALChkAdptrStrucsOkDpDownloaded(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN OUT CARDAL_ADAPTER_T		** ppCardALAdapter );

DWORD cardALChkAdptrStrucsOkNotDnldgDP(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN OUT CARDAL_ADAPTER_T		** ppCardALAdapter );

DWORD cardALChkAdptrStrucsOk(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN OUT CARDAL_ADAPTER_T		** ppCardALAdapter );

//
//	CardALBd.c
//
DWORD cardALChipDnldCodeToDP(
    IN CDSL_ADAPTER_T		* pThisAdapter );


#if	PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
DWORD cardALBdAdslGetPerform(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    I_O BD_ADSL_PERFORMANCE_T		* pBdAdslPerfStruc );

#endif	//PROJECTS_NAME == PROJECT_NAME_CHEETAH_1

//
//	Chip Specific CardALHw??? file.
//	e.g.,
//		CardAL20136.c		for Cheetah1
//		CardALTigris.c		for Tigris
//

#if	PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
DWORD cardALChipRequestNegotiatedModulation(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    CARDAL_ADAPTER_T		* pCardALAdapter );

DWORD cardALChipDoSelftest1Cmd(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    CARDAL_ADAPTER_T		* pCardALAdapter );

DWORD cardALChipDoSelftest2Cmd(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    CARDAL_ADAPTER_T		* pCardALAdapter );

DWORD cardALChipDoSendDyingGaspCmd(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    CARDAL_ADAPTER_T		* pCardALAdapter );

DWORD cardALChipGetPerformData(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    CARDAL_ADAPTER_T				* pCardALAdapter,
    I_O BD_ADSL_PERFORMANCE_T		* pBdAdslPerfStruc );

BOOLEAN cardALChipCheckForRespToCmd(
    I_O BD_ADSL_CMD_RESPONSES_T		* pBdAdslRespStrucRsltValue,
    I_O BD_ADSL_CMD_RESPONSES_T		* pBdAdslRespStrucRsltParam,
    I_O DWORD						* pBdResultValue,
    I_O ADSL_REG_CMD_RESP_T			* pAdslCmdResp );

#endif	//PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
DWORD cardALChipUpldCodeFromDP(
    IN CDSL_ADAPTER_T			* pThisAdapter,
    IN CARDAL_ADAPTER_T			* pCardALAdapter,
    IN DWORD					BlockStartAddress,
    I_O DWORD					* BlockLength,
    OUT BYTE					* BlockData );

DWORD cardALChipAdslLineHalt(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    CARDAL_ADAPTER_T		* pCardALAdapter );

DWORD cardALChipStartAdslLine(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    CARDAL_ADAPTER_T		* pCardALAdapter );

DWORD cardALChipShutdownAdslLine(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    CARDAL_ADAPTER_T		* pCardALAdapter );

DWORD cardALChipPeekReg(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    CARDAL_ADAPTER_T				* pCardALAdapter,
    I_O BD_CARDAL_REGISTER_T		* pBdCardALReg );

DWORD cardALChipPokeReg(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    CARDAL_ADAPTER_T				* pCardALAdapter,
    I_O BD_CARDAL_REGISTER_T		* pBdCardALReg );

DWORD cardALChipGetLineStatus(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    CARDAL_ADAPTER_T				* pCardALAdapter,
    OUT BYTE						* pLineStatus );

DWORD cardALChipGetLineSpeed(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    CARDAL_ADAPTER_T				* pCardALAdapter,
    OUT WORD						* pAdslLineSpeedUpOrFar,
    OUT WORD						* pAdslLineSpeedDownOrNear );

DWORD cardALChipGetDpVersions(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    CARDAL_ADAPTER_T				* pCardALAdapter,
    I_O BD_ADSL_DP_VERSIONS_T		* pBdAdslDpVersStruc );

DWORD cardALChipGetPmState(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    OUT BD_PM_POWER_STATE_T	* pPMPowerState);

DWORD cardALChipSetPmState(
    IN CDSL_ADAPTER_T		* pThisAdapter,
    IN BD_PM_POWER_STATE_T	* pPMPowerState);

NDIS_STATUS	CardAlWriteMacAddress(
    IN CARDAL_ADAPTER_T * pCardAl,
    IN BYTE			 	* MacAddress,
    IN DWORD		 	MacAddrLength
);

NDIS_STATUS	CardAlReadMacAddress(
    IN CARDAL_ADAPTER_T * pCardAl,
    IN BYTE			 	* MacAddress,
    IN DWORD		 	MacAddrLength
);

////////////////////////////////////////////////////////
// diag support routines and data
////////////////////////////////////////////////////////
typedef struct BD_ADSL_PERFORMANCE_S
{
	DWORD						R_relCapacityOccupationDnstr;
	DWORD						R_noiseMarginDnstr;
	DWORD						R_outputPowerUpstr;
	DWORD						R_attenuationDnstr;
	DWORD						R_relCapacityOccupationUpstr;
	DWORD						R_noiseMarginUpstr;
	DWORD						R_outputPowerDnstr;
	DWORD						R_attenuationUpstr;
	DWORD						R_ChanDataIntNear;
	DWORD						R_ChanDataFastNear;
	DWORD						R_ChanDataIntFar;
	DWORD						R_ChanDataFastFar;
	BD_ADSL_MODEM_STATUS_T		M_ModemStatus;
	DWORD						M_NsecValidBerFast;
	DWORD						M_AccBerFast;
} BD_ADSL_PERFORMANCE_T;

GLOBAL NTSTATUS CdalDiagGetADSLPerf(
    void					* pAdapter,
    CHAR					* Buffer,
    DWORD					Length
);

#endif		//#ifndef _CARDMGMTV_H_

