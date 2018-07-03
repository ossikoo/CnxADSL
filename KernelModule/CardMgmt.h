/******************************************************************************
*******************************************************************************
****	Copyright (c) 1997, 1998, 1999, 2000, 2001
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
**		CardMgmt (Card Management)
**
**	FILE NAME:
**		CardMgmt.h
**
**	ABSTRACT:
**		This files contains public defines for Card Manager Module.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.4_ATM_Driver/KernelModule/CardMgmt.h $
** $Revision: 4 $
** $Date: 4/25/01 8:52a $
*******************************************************************************
******************************************************************************/
#ifndef _CARDMGMT_H_		//	File Wrapper,
#define _CARDMGMT_H_		//	prevents multiple inclusions

//
//	Include files available to all modules/files within the Athena Driver.
//

#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include "Version.h"
#include <linux/atm.h>
#define NET_IRDA_H // prevent redifinition of ASSERT via included linux/atmdev.h which included net/sock.h which included net/irda/irda.h which defined ASSERT
#include <linux/atmdev.h>
#include <linux/pci.h>
#include <linux/skbuff.h>

#include "Common.h"			//	Generic definitions/declarations common to all files/modules
#include "Product.h"		//	Product specific definitions/declarations

#include "UtilDbg.h"		//	Utility Debug public declarations
#include "UtilStr.h"		//	Utility String public declarations
#include "UtilTmr.h"		//	Utility Timer public declarations

#include "DevIo.h"			//	BACK_DOOR - public declarations
//	Defines = Control Panel Applet <> TAPI <> Miniport Driver
//	Device Specific ("LineDevSpecific") information.
#include "../CommonData.h"

///////////////////////////////////////////////////////////////////////////////

//
// NDIS MEMORY ALLOCATION GOODIES
// NOTE:	The following NDIS memory allocation was lifted from a Microsoft
//			NDIS example as the parameter to pass to indicate that any physical
//			address space was acceptable for memory.   It does not conform to the
//			Rockwell HSV coding standard but it is probably not worth the effort to
//			figure out what the Microsoft code is doing to remove the '-1' "magic numbers".
//
//	This constant is used for places where NdisAllocateMemory needs to be
//	called and the HighestAcceptableAddress does not matter.
#define HIGHEST_ACCEPTABLE_ADDRESS			NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);

//	This constant is used for places where NdisAllocateMemory needs to be
//	called and the highest acceptable address does not matter.
#define ANY_ADDRESS_ACCEPTABLE			NDIS_PHYSICAL_ADDRESS_CONST(-1,-1)

// Define so as not to have any "magic numbers" in our code
#define NON_PAGED_SYSTEM_SPACE_MEM		0

//
//	Memory tag name for use with calls to NdisAllocateMemoryWithTag
//	NOTE:	The tag name is in reverse order because when it is displayed
//			in dumps, etc. it is reversed by the dump utility (?).  This tag
//			therefore will come out looking like CNXT in the dumps.  Both
//			upper and lower case characters and numeric can be used in the
//			name.  Case sensitivity is significant.  For example "CNXT" is
//			not the same tag as "Cnxt".
//
//	It is recommended that you include in each 'C' file that makes calls to NdisAllocateMemoryWithTag the following conditional:
//		#ifndef MEMORY_TAG
//			#define	MEMORY_TAG		'xxxx'
//		#endif
//	where 'xxxx' is either the global driver memory tag name or one that
//	you can temporarily use to monitor the memory allocation/deallocation
//	of your particular module.
//
//#define MEMORY_TAG			'TXNC'


//STRUCTURE DEFINITIONS

// Defines to give usage names to the NDIS_WAN_PACKET
// structure MacReservedx fields
#define Ext_Buf_Des			MacReserved1	//Refers to the extended buffer descriptor struct
#define Adapter_Hdl			MacReserved2
#define Link_Context		MacReserved3

#define MAX_DESCRIPTION_LEN	80

struct CDSL_ADAPTER_S;
struct MSG_HANDLING_S;

// Structure for extended buffer descriptor information
typedef struct _BUFF_DES_EXT
{
	DWORD					Status;
	SK_BUFF_T				NextBufferDes;
} BUFF_DES_EXT;


typedef enum TIMER_STATES_E
{
    TMR_UNINITIALIZED,
    TMR_INITIALIZED,
    TMR_UNLOADING,
    TMR_TIMING,
    TMR_EXECUTING,
    TMR_COMPLETED,
    TMR_CANCELLED,
    TMR_INVALID_STATE
} TIMER_STATES_T;

// place holder definition of CHIP AL interface	(Forward Reference).
struct CHIPAL_INF_S;
//typedef struct CHIPAL_REMOTE_INF *PCHIPAL_REMOTE_INF;



// structure for Virtual Circuit info
typedef struct CDSL_VC_S
{
	// Pointer back to the Link (line) data structure so we can get at it
	// when passed a pointer to the VC structure.
	struct CDSL_LINK_S		* LinkHdl;

	// This is a zero based index used to assign TAPI device ID to the
	// VC based on the TAPI DeviceIdBase assigned to the driver.
	WORD					VcDeviceId;
	DWORD					VcAddressId;

	// handle supplied by linux for identifying connection
	struct atm_vcc*			pVcc;

	short					Vpi;		// Virtual Path Identifier
	int						Vci;		// Virtual Channel Identifier
	BD_ADSL_PCR_KBPS_T		Pcr;		// Peak Cell Rate

	BOOLEAN					Allocated;

	BOOLEAN					Connected;

	// Vectors to the respective module structure VC context instantiations
	void					* FrameALVCMod;
	void					* CellDataTestMod;
	void					* BufMgmtMod;

} CDSL_VC_T;


//structure for link info
typedef struct CDSL_LINK_S
{

	// Pointer back to the Adapter data structure so we can get at it
	// when passed a pointer to this structure in NdisMWanSend().
	struct CDSL_ADAPTER_S	* pThisAdapter;

#define UNASSIGNED_LINK_ENTRY	0
	//
	WORD					LinkEntryState;

	//
	BOOLEAN					Allocated;
	// Vectors to the respective module structure context instantiations
	void					* CardMgmt;
	void					* FrameMod;
	void					* CellDataTestMod;

	CDSL_VC_T				VC_Info [MAX_VC_PER_LINK];	// structure for VC info

} CDSL_LINK_T;

//===================================================================================
//	The following function pointers are defined to provide a generic interface to
//	each module. The Card Management module will call these functions.
//===================================================================================
typedef
NDIS_STATUS (* FN_MODULE_CFG_INIT)
(
    IN struct CDSL_ADAPTER_S	*pThisAdapter,
    IN PTIG_USER_PARAMS             pParams
);

typedef
NDIS_STATUS (* FN_MODULE_ADAPTER_INIT)
(
    IN struct CDSL_ADAPTER_S 	*pThisAdapter
);

typedef
NDIS_STATUS (* FN_MODULE_HALT)
(
    IN struct CDSL_ADAPTER_S 	*pThisAdapter
);

typedef
NDIS_STATUS (* FN_MODULE_IOCTL_HDLR)
(
    IN struct CDSL_ADAPTER_S	* pThisAdapter,
    IN BACK_DOOR_T				* pBackDoorBuf,
    OUT PULONG					pNumBytesNeeded,
    OUT PULONG					pNumBytesWritten
);

/************************************************************************************
**	This is the Card Management vector table. These are the the generic
**	functions that will be called from within the Card Management module. 
**	The function prototypes are defined in CardMgmt.h. If a module requires one 
**	of these predefined functions then its name is placed into the #define
**	otherwise NULL is entered. In this way a consistent interface is maintained
**	with the Card Management module who will call these functions.
*************************************************************************************/
#define CARDMGMT_DESCRIPTION	"Card Management Module"

#define CARDMGMT_FNS {												\
	CARDMGMT_DESCRIPTION,		/*Module Description			*/	\
	cmCtlCfgInit,				/*Module CfgInit Function		*/	\
	cmCtlAdapterInitialize,		/*Module AdapterInit Function 	*/	\
	cmCtlAdapterHalt			/*Module Shutdown Function		*/	\
}

/******************************************************************************
**	This is the vector table structure that Card Management will use to call each
** 	modules CfgInit(), AdapterInit(), and Shutdown() functions as defined in
**	CardMgmt.h.
******************************************************************************/
typedef struct MODULE_FUNCTIONS_S
{
	CHAR 						Description[MAX_DESCRIPTION_LEN];
	FN_MODULE_CFG_INIT			CfgInit;
	FN_MODULE_ADAPTER_INIT		AdapterInit;
	FN_MODULE_HALT				Shutdown;
	FN_MODULE_IOCTL_HDLR		IoctlHdlr;
} MODULE_FUNCTIONS_T;

//structure for adapter info
typedef struct CDSL_ADAPTER_S
{
	// linux atm device pointer
	struct     atm_dev *			pAtmDevice;
	struct CDSL_ADAPTER_S *			pNext;

	////////////////////////////
	//	Registery Parameters
	////////////////////////////
	ULONG					TimerResolution;

	DWORD					ApiHwIfMode;
	DWORD					DebugFlag;
	CHAR					VcList [VCLIST_LENGTH];

	//	Returned from an OID_GEN_VENDOR_ID QueryInformation request.
	//	Again, the vendor's assigned ethernet vendor code should be used if possible.
	CHAR					VendorId [MAX_VENDOR_ID_LENGTH];

	//	Returned from an OID_GEN_VENDOR_DESCRIPTION QueryInformation request.
	//	This is an arbitrary string which may be used by upper layers to present
	//	a user friendly description of the adapter.
	CHAR					VendorDescription [MAX_VENDOR_DESCRIPTION_LENGTH];

	/////////////////////////////////////////
	//	end of Registery Parameters block
	/////////////////////////////////////////
	DWORD					ProductVersion;

	//	The length should be six since ethernet addressess are six bytes long.
	CHAR					PermanentEthAddress [MAX_ETH_ADDRESS_LENGTH];

	CHAR					AddrTerminator;

	//Flag to indicate when Adapter needs to be reset.
	BOOLEAN					NeedReset;

	//Flag to indicate when Adapter is in load process.
	BOOLEAN					Loading;
	BOOLEAN					AutoDownload;

	//	Used to maintain a media connect status for LAN driver
	DWORD					MediaConnectStatus;
	BOOLEAN					Showtime;
	//
	//Flag to indicate when interrupts are ready
	//
	BOOLEAN					InterruptsInitialized;

	/////////////////////////////
	//wrapper parameters
	//instance number
	UCHAR					InstanceNumber;

	// Spinlock to protect fields in this structure..
	SPIN_LOCK				Lock;

	//NDIS timer used by the adapter
	LINUX_TIMER			MainTimer;
	TIMER_STATES_T		MainTimerState;
	EVENT_HNDL			MainTimerStoppedEvent;

	//Adapter Initialization was complete.  Used to determine if it is
	//necessary to call NDISFreexxx funcitons for Timers, Spinlocks, etc...
	BOOL					AdapterInitComplete;
	BOOL					DownloadInitComplete;

	//Structure for timers maintained by the driver code.
	TIMER_INFO_BLOCK		TimerBlock;

	BD_CDSL_PROD_INFO_T		CdslProdInfo;
	BD_CDSL_OPERATIONAL_T	CdslOperational;

	//
	//Communication link structure
	//
	CDSL_LINK_T				AdapterLink;

	VOID					* pModuleContext[MAX_DRIVER_MODULES];
	BOOLEAN					shutdown;

	// user configurable data
	TIG_USER_PARAMS			PersistData;

	// Vectors to the respective module structure context instantiations
	void					* CardMgmt;
	void					* FrameMod;
	void					* ChipMod;
	void					* CardMod;
	void					* CellDataTestMod;
	void					* ArmALMod;
	void					* BufMgmtMod;

	struct CHIPAL_INF_S*	pChipALRemoteInf;  // chip al interface table

	// message handling section
	struct MSG_HANDLING_S*	pMsgHandleS;

	// interrupt structure
	struct tasklet_struct DSLInterruptQueStr;
	struct tasklet_struct ARMInterruptQueStr;

	// Linux queue structures
	struct work_struct		PGBkgndQueStr;
	LINUX_TIMER				PGBkgndTimerStr;
	struct work_struct		SITaskQueStr;
        struct file_operations *dbg_fops;
} CDSL_ADAPTER_T;

#include "CardAL.h"		//	Card Abstraction Layer public declarations
#include "CardMgmtCC.h"		//	Call Control public declarations
#include "FrameAL.h"		//	Frame Abstraction Layer public declarations
#include "CellDataTest.h"	//  Cell data test module public declaration
#include "ChipALCdsl.h"		//	Chip Abstraction Layer public declarations
#include "BufMgmt.h"		//	Buffer Management public declarations

///////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
///////////////////////////////////////////////////////////////////////////////
DWORD CmgmtGetOidList(
    VOID				* ExternHandle,
    DWORD				** MessageList,
    DWORD				* MaxMessages
);

DWORD CmgmtGetApiList(
    VOID				* ExternHandle,
    DWORD				** MessageList,
    DWORD				* MaxMessages
);

void CmgmtIndicateLinkUp(
    IN CDSL_ADAPTER_T		* pThisAdapter
);

void CmgmtIndicateLinkDown(
    IN CDSL_ADAPTER_T		* pThisAdapter
);

#endif		//#ifndef _CARDMGMT_H_
