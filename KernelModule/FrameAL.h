/************************************************************************************
Module:		
File:		FrameAl.h
Function:
Description:
 
Copyright:
	Copyright 1997 Rockwell International
	Rockwell Semiconductor Systems
	All Rights Reserved
	CONFIDENTIAL AND PROPRIETARY
	No Dissemination or use without prior written permission.

Keywords:
$Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/FrameAL.h $
$Revision: 1 $
	
*************************************************************************************/
#ifndef _FRAMEAL_H_		//	File Wrapper,
#define _FRAMEAL_H_		//	prevents multiple inclusions


#include "FrameALOAM.h"
#include "FrameALAAL.h"
#include "FrameALATM.h"
// NOTE: The include for atm.h was put here because when it is

// time for cell credit interval in milliseconds
#define FRAMEAL_TIMER_PERIOD 	10

//
// Cheetah ATM Specific Media parameters
//
typedef enum
{
    VC_SAP_AAL = 0,
    VC_SAP_CELLDATATEST
}	FRAMEAL_VC_SAP;

typedef enum
{
    VC_MODE_NORMAL = 0,
    VC_MODE_INSERT_IDLE
} FRAMEAL_VC_MODE;

typedef enum
{
    FRAMEAL_VC_IDLE = 0,
    FRAMEAL_VC_ACTIVE,
    FRAMEAL_VC_DEACTIVE_PEND
} 	FRAMEAL_VC_STATE_E;

typedef struct _FRAMEAL_VC_PARAMETERS
{
	short				AALType;
	short				Vpi;
	int					Vci;
	DWORD				PeakCellRate;
	DWORD				DefaultCLP;
	BYTE				UserToUserIndication;
	BYTE				CommonPartIndicator;
	FRAMEAL_VC_SAP		VcSAP;
	FRAMEAL_VC_MODE		VCMode;
	BD_FRAMEAL_ENCAPSULATION_T	RfcEncapsulationMode;
} FRAMEAL_VC_PARAMETERS, *PFRAMEAL_VC_PARAMETERS;

// Cheetah link type covers all the VCs on a physical link
typedef enum
{
    LINK_TYPE_UBR_ONLY =0,
    LINK_TYPE_URB_ABR,
    LINK_TYPE_VBR_ABR_UBR,
}	FRAMEAL_LINK_TYPE;


#if 0
// Cheetah RFC encapsulation mode covers all the VCs on a physical link
typedef enum FRAMEAL_RFC_ENCAPSULATION_MODE_E
{
    RFC2364_NONE,
    RFC2364_VCMUX,
    RFC2364_LLC,
    RFC1483_NULL,
    RFC1483_VCMUX_BRIDGED,
    RFC1483_LLC_BRIDGED,
    RFC1483_VCMUX_ROUTED,
    RFC1483_LLC_ROUTED
} FRAMEAL_RFC_ENCAPSULATION_MODE_T;

#endif
//
// Cheetah ATM Specific Media parameters
//
typedef struct _FRAMEAL_LINK_PARAMETERS
{
	DWORD					TxCellsPerSecond;	// physical layer transmit link speed
	DWORD					RxCellsPerSecond;	// physical layer receive link speed
	FRAMEAL_LINK_TYPE		LinkType;			// need to support other services besided UBR/ABR

	DWORD					CellBurstSize;		// Sets the sustained cell rate burst limit

} FRAMEAL_LINK_PARAMETERS, *PFRAMEAL_LINK_PARAMETERS;



// Structure for link stats, for both good and bad frames
typedef struct _FRAMEAL_LINK_STATS
{
	LONG_CTR_T 	TXFrames;  // Number of Good frames TX's
	LONG_CTR_T	TXBytes;   // Total Good Bytes TX's
	LONG_CTR_T	TXBytesEncoded; // bytes encoded by HDLC
	LONG_CTR_T	TXErrors;  // Number of frames that were not TX's

	// These statistics will move to VC structure someday
	LONG_CTR_T	TXATMCellsCLP0;		// number of cells sent with CLP=0
	LONG_CTR_T	TXATMCellsCLP1;		// number of cells sent with CLP=1
	LONG_CTR_T	TXATMCellsTShaped;  // number of times traffic shaping has blocked a vc or link
	LONG_CTR_T	TXATMMAXPeriod;		// How close did the driver get to blocking a VC

	LONG_CTR_T  TXATMCellsMgmCell;		// number of mgmt cells transmitted

	LONG_CTR_T	RXFrames;  // Number of Good frames Received
	LONG_CTR_T	RXBytes;   // Total Good Bytes Received
	LONG_CTR_T	RXErrors;  // Number of Bad frames Received (BAD CRC)
	LONG_CTR_T	RXAborts;  // Number of frames aborted.
	LONG_CTR_T	RXMissed;  // Number of frames dropped for lack of buffer

	// These statistics will move to VC structure someday
	LONG_CTR_T 	RXATMCellsGoodCLP0;	// number of good cells received with CLP=0
	LONG_CTR_T	RXATMCellsGoodCLP1;	// number of good cells received with CLP=1

	// These statistics will stay in the link (physical link) structure
	LONG_CTR_T	RXATMCellsMisrouted;	// number of cells received with invalid VPI/VCI
	LONG_CTR_T  RXATMCellsBadHEC;		// number of cells received with failed checked
	LONG_CTR_T  RXATMCellsMgmCell;		// number of mgmt cells received (OAM)

	// these counters are for 8byte CONDIS OIDS
	LONGLONG_CTR_T COTXBytes;			// Connection bytes transmitted
	LONGLONG_CTR_T CORXBytes;			// Connection bytes received

	LONGLONG_CTR_T COATMTXCells;		// ATM cells transmitted
	LONGLONG_CTR_T COATMRXCells;		// ATM cells received
	LONGLONG_CTR_T COATMRXCellsCRC;		// ATM cells with bad hec
	LONGLONG_CTR_T COATMRXCellsDrops; 	// ATM cells dropped


}  FRAMEAL_LINK_STATS_T;


// Structure for link stats, for both good and bad frames
typedef struct _FRAMEAL_VC_STATS
{
	LONG_CTR_T 	TXFrames;  // Number of Good frames TX's
	LONG_CTR_T	TXBytes;   // Total Good Bytes TX's

	// These statistics will move to VC structure someday
	LONG_CTR_T	TXATMCellsCLP0;		// number of cells sent with CLP=0
	LONG_CTR_T	TXATMCellsCLP1;		// number of cells sent with CLP=1

	LONG_CTR_T	RXFrames;  // Number of Good frames Received
	LONG_CTR_T	RXBytes;   // Total Good Bytes Received
	LONG_CTR_T	RXErrors;  // Number of Bad frames Received (BAD CRC)

	// These statistics will move to VC structure someday
	LONG_CTR_T 	RXATMCellsGoodCLP0;	// number of good cells received with CLP=0
	LONG_CTR_T	RXATMCellsGoodCLP1;	// number of good cells received with CLP=1

	// These statistics will stay in the link (physical link) structure
	LONG_CTR_T  RXATMCellsMgmCell;		// number of mgmt cells received (OAM)

	LONG_CTR_T  TXATMCellsMgmCell;		// number of mgmt cells transmitted

	// these counters are for 8byte CONDIS OIDS
	LONGLONG_CTR_T COTXBytes;			// Connection bytes transmitted
	LONGLONG_CTR_T CORXBytes;			// Connection bytes received

	LONGLONG_CTR_T COATMTXCells;		// ATM cells transmitted
	LONGLONG_CTR_T COATMRXCells;		// ATM cells received
	LONGLONG_CTR_T COATMRXCellsCRC;		// ATM cells with bad hec
	LONGLONG_CTR_T COATMRXCellsDrops; 	// ATM cells dropped


}  FRAMEAL_VC_STATS_T;

// AAL link structure for Cheetah-1 driver
// contains VC parameters and VC statistics
typedef struct _FRAMEAL_AAL_VC_T
{
	// General parameters
	WORD				RASCellCnt;

	// TX parameters
	BYTE				CPCS_UU;
	BYTE				CPI;
	SK_BUFF_T		 	*AALxTxBfr;
	BYTE				*AALxTx48BytePtr;
	BYTE				*AALxTxBfrPtr;
	ULONG				AALxTxBfrLength;
	LONG_CTR_T  		AALTxGood;
	LONG_CTR_T  		AALTxGoodByte;
	LONG_CTR_T  		AALTxDiscarded;
	LONG_CTR_T  		AALTxDiscardedByte;
	ULONG				RemainingBufferLength;
	char				ExtraPad[2*CELL_PAYLOAD_SIZE];
	ULONG				RemainingPadLength;

	// RX parameters
	SK_BUFF_T		 	*AALxRxBfr;
	BYTE				*AALxRx48BytePtr;
	LONG_CTR_T  		AALRxGood;
	LONG_CTR_T  		AALRxGoodByte;
	LONG_CTR_T  		AALRxDiscarded;
	LONG_CTR_T  		AALRxDiscardedByte;
	LONG_CTR_T  		AALRxInvalidLen;
	LONG_CTR_T  		AALRxInvalidCRC;
	LONG_CTR_T  		AALRxReassemblyTimeout;

} FRAMEAL_AAL_VC_T ;


// AAL Link structure
// Currently only used to maintain statistic info
typedef struct _FRAMEAL_AAL_LINK_T
{
	// TX parameters
	LONG_CTR_T  		AALTxGood;
	LONG_CTR_T  		AALTxGoodByte;
	LONG_CTR_T  		AALTxDiscarded;
	LONG_CTR_T  		AALTxDiscardedByte;

	// RX parameters
	LONG_CTR_T  		AALRxGood;
	LONG_CTR_T  		AALRxGoodByte;
	LONG_CTR_T  		AALRxDiscarded;
	LONG_CTR_T  		AALRxDiscardedByte;
	LONG_CTR_T  		AALRxInvalidLen;
	LONG_CTR_T  		AALRxInvalidCRC;
	LONG_CTR_T  		AALRxReassemblyTimeout;
	LONG_CTR_T			AALRxNoBufferAvail;


} FRAMEAL_AAL_LINK_T ;


// FrameAL VC structure for Cheetah-1 driver
// contains VC parameters, VC statistics and working pointers
typedef struct _FRAMEAL_VC_T
{
	DWORD					TXSize;
	FRAMEAL_VC_STATS_T		Stats;
	FRAMEAL_VC_STATE_E		State;

	FRAMEAL_VC_PARAMETERS	Parameters;



	FRAMEAL_ATM_VC_T		ATMVCBlock;

	FRAMEAL_AAL_VC_T		AALVcCtx;

} FRAMEAL_VC_T ;

typedef struct 	FRAMEAL_SHAPER_INFO_S
{
	DWORD		TimerIntervalUpper;
	DWORD		TimerIntervalLower;
	DWORD		LineCheckInterval;
	DWORD		ClientLimit;
	DWORD		DefaultBurstSize;

} FRAMEAL_SHAPER_INFO_T;
// FrameAL link structure for Cheetah-1 driver
// contains link parameters, link statistics and working pointers
typedef struct _FRAMEAL_LINK_T
{

	DWORD					TXSize;

	FRAMEAL_LINK_STATS_T	Stats;

	FRAMEAL_LINK_PARAMETERS	Parameters;

	FRAMEAL_ATM_LINK_T		ATMLinkBlock;

	FRAMEAL_AAL_LINK_T		AALLinkBlock;

	//NDIS timer used by the adapter
	LINUX_TIMER				FrameALTimer;

	TIMER_STATES_T			FrameALTimerState;
	EVENT_HNDL				FrameALTimerStoppedEvent;


	FRAMEAL_SHAPER_INFO_T	FrameALShaperInfo;

} FRAMEAL_LINK_T ;

// casts for the resverved fields
#define OOB_DATA( x )		(*(DWORD*)         &((x)->cb[0              ]))
#define CELLHEAD_DATA( x )	(*(P_ATM_HEADER_T*)&((x)->cb[  sizeof(DWORD)]))
#define CELL_LTH_DATA( x )	(*(DWORD*)         &((x)->cb[2*sizeof(DWORD)]))
#define VC_HNDL_DATA( x )	(*(CDSL_VC_T**)    &((x)->cb[3*sizeof(DWORD)]))

// bit definitations for Out of Band Data field
#define OOD_CLP				0x010000
#define OOD_AUU				0x020000
#define OOD_CI				0x040000
#define OOD_IDLE			0x080000 
#define OOD_END_CELL		0x100000

// This bit indicates to CellDataTestMgmt to copy the payload and do not use the Wan Desciptor
// passed.
#define OOD_CELLMGMT_CPY	0x200000
#define OOD_OAM_CELL		0x400000 
#define OOD_OFFSET			16

// Debug macro to trace data access through the driver
#if 0   
#define DEBUG_VC(LinkHdlx) 	  	{ 													\
			FRAMEAL_LINK_T  *pFrameALLink1;   																	\
			pFrameALLink1 = (FRAMEAL_LINK_T*)(LinkHdlx)->FrameMod;													\
			if (pFrameALLink1 != NULL)																			\
				DbgPrint( "file %s line %d VcCount %x \r\n",__FILE__, __LINE__,pFrameALLink1->ATMLinkBlock.CurrentVC); 	\
			else																								\
					DbgPrint( " FrameALATM line %d \r\n",__LINE__);												\
		};
#else
	#define DEBUG_VC(LinkHdlx) 
#endif
// ***********************************************************************************

GLOBAL NDIS_STATUS FrameALAdapterInit
(
    IN	CDSL_ADAPTER_T	*AdprHdl	// supplied Adapter Handle
    // (&ThisAdapter)
);

// ***********************************************************************************

GLOBAL NDIS_STATUS FrameALAdapterShutdown
(
    IN	CDSL_ADAPTER_T	*AdprHdl	// supplied Adapter Handle
    // (&ThisAdapter)
);

// ***********************************************************************************

GLOBAL NDIS_STATUS FrameALDeviceSpecific
(
    CDSL_ADAPTER_T				* pThisAdapter,		// Adapter context
    BACK_DOOR_T					* pBackDoorBuf,		// NDIS supplied buffer
    PULONG						pNumBytesNeeded,	// Bytes needed to return item
    PULONG						pNumBytesWritten 	// Bytes actually placed in the infobuffer
);

// ***********************************************************************************

// Start TX function
// return failure if link handle invalid or link not in "opened" state.

GLOBAL NDIS_STATUS FrameALLinkStartTX
(
    IN CDSL_LINK_T 	*LinkHdl
);

// ***********************************************************************************

// Start TX function
// version accepting a VC handle as a parameters.

GLOBAL NDIS_STATUS FrameALVCStartTX
(
    IN CDSL_VC_T 	*VCHdl
);






// ***********************************************************************************

GLOBAL NDIS_STATUS FrameALLinkInit
(
    IN	CDSL_LINK_T	*LinkHdl,						// Link Handle to data stream
    IN  FRAMEAL_LINK_PARAMETERS  *LinkParameters		// LINK parameters (link speed and link mode)
);

// ***********************************************************************************

GLOBAL NDIS_STATUS FrameALLinkShutdown
(
    IN	CDSL_LINK_T	*LinkHdl	// Link Handle to data stream
);

// ***********************************************************************************

GLOBAL NDIS_STATUS FrameALVCInit
(
    IN	CDSL_VC_T	*VCHdl,			// Link Handle to data stream
    IN  FRAMEAL_VC_PARAMETERS * VCParameters	//

);

// ***********************************************************************************

GLOBAL NDIS_STATUS FrameALVCShutdown
(
    IN	CDSL_VC_T	*VCHdl			// Link Handle to data stream
);

// ***********************************************************************************

GLOBAL void FrameALEvent
(
    IN	CDSL_LINK_T		*LinkHdl,		// Link Handle to data stream
    IN	WORD			 Event,			// Event enumeration
    IN	void			*Data			// Pointer to further data dependent on event
);

// ***********************************************************************************
GLOBAL NDIS_STATUS FrameALCfgInit
(
    IN	CDSL_ADAPTER_T	*AdprCtx,		// Driver supplied adapter context
    // (&ThisAdapter)  Required for events that
    // are not associated with a link
    IN PTIG_USER_PARAMS  pUserParams
);
// ***********************************************************************************
// get the stats structure for a link
GLOBAL FRAMEAL_LINK_STATS_T *FrameALLinkStatsRequest
(
    IN	CDSL_LINK_T		*LinkHdl
);

#if PROJECTS_NDIS_IF == PROJECT_NDIS_IF_CO
// ***********************************************************************************

// get the stats structure for a link
GLOBAL FRAMEAL_VC_STATS_T  FrameALVCStatsRequest
(
    IN	CDSL_VC_T		*VCHdl
);
#endif 

// ***********************************************************************************
/*not implemented for Linux

NDIS_STATUS FrameALSetInformationHdlr(
    IN CDSL_ADAPTER_T	* pThisAdapter,
    IN PVOID			InfoBuffer,
    IN ULONG			InfoBufferLength,
    OUT PULONG			BytesRead,
    OUT PULONG			BytesNeeded );
*/


// ***********************************************************************************

GLOBAL SK_BUFF_T	*FrameALGetTxFullBuffer
(
    IN	void			*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*InBufDescPtr	// Pointer to a buffer descriptor for FrameAL to use
);


// ***********************************************************************************

GLOBAL void FrameALPutTxEmptyBuffer
(
    IN	void			*LinkHdl,		//Link Handle to data stream
    IN	SK_BUFF_T	    *BufferDescPtr	//Pointer to a buffer descriptor
);

// ***********************************************************************************

GLOBAL void FrameALGetRxEmptyBuffer
(
    IN	void			*LinkHdl,		// Link Handle to data stream
    OUT	SK_BUFF_T	    *BufferDescPtr	//Pointer to a buffer descriptor
);

// ***********************************************************************************

GLOBAL SK_BUFF_T	*FrameALPutRxFullBuffer
(
    IN	void			*LinkHdl,		// Link Handle to data stream
    IN	SK_BUFF_T		*BufferDescPtr	// Pointer to a buffer descriptor
);

// ***********************************************************************************

GLOBAL void FrameALTxQNotEmpty
(
    IN	CDSL_LINK_T		*LinkHdl		// Link Handle to data stream

);

GLOBAL void FrameALClearStats
(
    IN	CDSL_LINK_T		*LinkHdl		// Link Handle to data stream

);

GLOBAL void FrameALLinkUp
(
    IN	CDSL_LINK_T		*LinkHdl		// start ATM operations when showtime is entered
);

GLOBAL void FrameALLinkDown
(
    IN	CDSL_LINK_T		*LinkHdl

);

#endif	//#ifndef _FRAMEAL_H_

//===================================================================================
//	This is the Frame Abstraction Layer vector table. These are the the generic
//	functions that will be called from within the Card Management module.
//	The function prototypes are defined in CardMgmt.h. If a module requires one
//	of these predefined functions then its name is placed into the #define
//	otherwise NULL is entered. In this way a consistent interface is maintained
//	with the Card Management module.
//===================================================================================
#define FRAMEAL_DESCRIPTION	"Frame Abstraction Layer Module"
#define FRAMEAL_FNS	{										   	\
	FRAMEAL_DESCRIPTION,	/*Module Description			*/	\
	FrameALCfgInit,			/*Module CfgInit Function		*/	\
	FrameALAdapterInit,		/*Module AdapterInit Function	*/	\
	FrameALAdapterShutdown,	/*Module Shutdown Function		*/	\
	FrameALDeviceSpecific	/*Ioctl handler					*/  \
}
