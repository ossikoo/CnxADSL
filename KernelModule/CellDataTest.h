/************************************************************************************
Module:		
File:		CellDataTest.h
Function:
Description:

Copyright:
	Copyright 1997 Rockwell International
	Rockwell Semiconductor Systems
	All Rights Reserved
	CONFIDENTIAL AND PROPRIETARY
	No Dissemination or use without prior written permission.

Keywords:
$Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CellDataTest.h 1     1/09/01 10:53a Lewisrc $
$Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CellDataTest.h $
$Revision: 1 $
	
*************************************************************************************/
#ifndef _CELL_DATA_TEST_H_
#define _CELL_DATA_TEST_H_

#include "CellDataTestMgmt.h"
typedef enum
{
    VC_MODE_IDLE = 0,
    VC_MODE_DIGITAL_LOOPBACK,
    VC_MODE_CELL_VERIFICATION_TEST,
    VC_MODE_ATM_BER
} CELL_DATA_TEST_VC_MODE;

typedef DWORD PRBS_ARRAY [4][256];

// place holder parameter block , not used for much initially
typedef struct _CELL_DATA_TEST_VC_PARAMETERS
{
	CELL_DATA_TEST_VC_MODE		VCMode;
	DWORD						MaxCellsInQueue;
	DWORD						CellVerificationPattern;

} CELL_DATA_TEST_VC_PARAMETERS, *PCELL_DATA_TEST_VC_PARAMETERS;


// place holder parameter block , not used for anything initially
typedef struct _CELL_DATA_TEST_LINK_PARAMETERS
{
	DWORD		linkType;			// have something in the parameter block

} CELL_DATA_TEST_LINK_PARAMETERS, *PCELL_DATA_TEST_LINK_PARAMETERS;




// Structure for Cell verification test stats, for both good and bad frames
typedef struct _CELL_DATA_TEST_VC_STATS
{
	// These statistics will move to VC structure someday
	LONG_CTR_T	TXATMCellsCLP0;		// number of cells sent with CLP=0
	LONG_CTR_T	TXATMCellsCLP1;		// number of cells sent with CLP=1

	// These statistics will move to VC structure someday
	LONG_CTR_T 	RXATMCellsGoodCLP0;	// number of good cells received with CLP=0
	LONG_CTR_T	RXATMCellsGoodCLP1;	// number of good cells received with CLP=1

	LONG_CTR_T 	RXATMCellsDataErrorsCLP0;
	LONG_CTR_T 	RXATMCellsDataErrorsCLP1;

	LONG_CTR_T	RXATMCellsSeqErrorsCLP0;
	LONG_CTR_T	RXATMCellsSeqErrorsCLP1;


	LONGLONG_CTR_T ATMBERRXErrors;	  // stats for BER test
	LONGLONG_CTR_T ATMBERRXBits;
	LONGLONG_CTR_T ATMBERRXResyncs;
	LONGLONG_CTR_T ATMBERTXBits;

	// These statistics will stay in the link (physical link) structure


}  CELL_DATA_TEST_VC_STATS_T;



typedef struct _TEST_PAYLOAD1_S
{
	DWORD	SeqNo;
	BYTE	Data[CELL_PAYLOAD_SIZE - sizeof(DWORD)];
} TEST_PAYLOAD1_T;

typedef struct _TEST_PAYLOAD_S
{
	BYTE	Data[CELL_PAYLOAD_SIZE];
} TEST_PAYLOAD_T;

// FrameAL VC structure for Cheetah-1 driver
// contains VC parameters, VC statistics and working pointers
typedef struct _CELL_DATA_TEST_VC_T
{
	DWORD							TXSize;
	DWORD							TxSeqNo;
	DWORD							RxSeqNo;
	BOOLEAN							FirstRxCell;
	TEST_PAYLOAD_T					TestPayload;
	CELL_DATA_TEST_VC_STATS_T		Stats;
	CELL_DATA_TEST_VC_PARAMETERS	Parameters;
	CDSL_VC_CELL_MGMT_T				CellMgmtCtx;
	BD_TESTMOD_ATM_BER_T			ATMBERParams;
	BD_TESTMOD_ATM_BER_INSERT_T     ATMBERInsertErrors; // state of insert error
	DWORD							TXPRBSAccum;   //
	DWORD							RXPRBSAccum;   //
	UCHAR 							TXPolarityMask;
	UCHAR							RXPolarityMask;
	long							Polynomial;  // for ATM BER PRBS
	BOOL							ATMBERInsync;
	DWORD							BitsSinceError;  // bits between inserted errors
	PRBS_ARRAY 						PRBSTable; // pointer to array
} CELL_DATA_TEST_VC_T ;


// FrameAL link structure for Cheetah-1 driver
// contains link parameters, link statistics and working pointers
typedef struct _CELL_DATA_TEST_LINK_T
{
	DWORD					TXSize;

	CELL_DATA_TEST_LINK_PARAMETERS	Parameters;

} CELL_DATA_TEST_LINK_T ;


// ***********************************************************************************

GLOBAL NDIS_STATUS CellDataTestAdapterInit
(
    IN	CDSL_ADAPTER_T	*AdprHdl	// supplied Adapter Handle
    // (&ThisAdapter)
);

// ***********************************************************************************

GLOBAL NDIS_STATUS CellDataTestAdapterShutdown
(
    IN	CDSL_ADAPTER_T	*AdprHdl	// supplied Adapter Handle
    // (&ThisAdapter)
);


// ***********************************************************************************

GLOBAL NDIS_STATUS CellDataTestLinkInit
(
    IN	CDSL_LINK_T					*LinkHdl,  		// Link Handle to data stream
    IN  CELL_DATA_TEST_LINK_PARAMETERS  	*LinkParameters	// LINK parameters (link speed and link mode)
);

// ***********************************************************************************

GLOBAL NDIS_STATUS CellDataTestLinkShutdown
(
    IN	CDSL_LINK_T	*LinkHdl	// Link Handle to data stream
);

// ***********************************************************************************

GLOBAL NDIS_STATUS CellDataTestVCInit
(
    IN	CDSL_VC_T	*VCHdl,			// Link Handle to data stream
    IN  CELL_DATA_TEST_VC_PARAMETERS * VCParameters	//

);

// ***********************************************************************************

GLOBAL NDIS_STATUS CellDataTestVCShutdown
(
    IN	CDSL_VC_T	*VCHdl			// Link Handle to data stream
);

// ***********************************************************************************
GLOBAL NDIS_STATUS CellDataTestCfgInit
(
    IN	CDSL_ADAPTER_T	*AdprCtx,			// Driver supplied adapter context
    // (&ThisAdapter)  Required for events that
    // are not associated with a link
    IN  PTIG_USER_PARAMS  pUserParams
);

// ***********************************************************************************
NDIS_STATUS CellDataTestDeviceSpecific(
    CDSL_ADAPTER_T				* pThisAdapter,
    BACK_DOOR_T					* pBackDoorBuf,
    PULONG						pNumBytesNeeded,
    PULONG						pNumBytesWritten );

// ***********************************************************************************

NDIS_STATUS CellDataTestSetInformationHdlr(
    IN CDSL_ADAPTER_T	* pThisAdapter,
    IN PVOID			InfoBuffer,
    IN ULONG			InfoBufferLength,
    OUT PULONG			BytesRead,
    OUT PULONG			BytesNeeded );



// ***********************************************************************************

GLOBAL void CellDataTestTxComplete
(
    IN	CDSL_VC_T		*VCHdl,		//Link Handle to data stream
    IN	SK_BUFF_T		*BufferDescPtr	//Pointer to a buffer descriptor
);

// ***********************************************************************************

GLOBAL void CellDataTestRxComplete
(
    IN	CDSL_VC_T		*VCHdl		// Link Handle to data stream
);

// ***********************************************************************************

GLOBAL NDIS_STATUS	CellDataTestPutRxFullBuffer
(
    IN	CDSL_VC_T		*VCHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*BufferDescPtr	// Pointer to a buffer descriptor
);

//===================================================================================
//	This is the Cell data text vector table. These are the the generic
//	functions that will be called from within the Card Management module.
//	The function prototypes are defined in CardMgmt.h. If a module requires one
//	of these predefined functions then its name is placed into the #define
//	otherwise NULL is entered. In this way a consistent interface is maintained
//	with the Card Management module who will call these functions.
//===================================================================================
#define CELLTEST_DESCRIPTION	"Cell Data Test Module"
#define CELLTEST_FNS	{											\
	CELLTEST_DESCRIPTION,		/*Module Description			*/	\
	CellDataTestCfgInit,		/*Module CfgInit Function		*/	\
	NULL,						/*Module AdapterInit Function	*/	\
	NULL,						/*Module shutdown function		*/	\
	CellDataTestDeviceSpecific	/* Ioctl handler */                 \
}


#endif // _CELL_DATA_TEST_H_
