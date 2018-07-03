/**************************************************************************************
Module:		ChipAL
File:		ChipALCdslv.h
Function:	Chip Abstraction Layer Private Header file
Description:                        	
			

			Copyright 1998 Rockwell International
***************************************************************************************
$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/KernelModule/ChipALCdslV.h $
$Revision: 4 $
$Date: 4/18/01 7:59a $
**************************************************************************************/
#ifndef _CHIPALCDSLV_H_					//	File Wrapper,
#define _CHIPALCDSLV_H_					//	prevents multiple inclusions

#include "ChipALCdsl.h"
#include "ChipALSEmw.h"

/*******************************************************************************
Module Constants
*******************************************************************************/
#if CAL_BASIC2_INSTALLED
	#define BUS_BURST_NOT_ALLOWED	TRUE
#else
	#define BUS_BURST_NOT_ALLOWED	FALSE
#endif

#define REPORT_UNHADLED_EVENTS	FALSE	// Call Unknown Event Handler if DPC event was not handled

#ifdef DBG								// Performance Only. Can disable during DBG if needed
	#define VERIFY_EVENTS 1				// Range Check ChipALSetEvent() events 
#else
	#define VERIFY_EVENTS 0				// Disable Range Check
#endif

#define 	GET_ADDRESS(Base,Offset)	( (volatile DWORD *) ((BYTE *)Base + Offset))


/*******************************************************************************
Private Module Functions - Used in User Defined Static structures
*******************************************************************************/

MODULAR INLINE CHIPAL_FAILCODE_T chipALReadMemory (
    void				* RegIoContext,
    PVOID				BaseAddress,
    DWORD				OffSetAddress,
    DWORD				RegisterLength,
    void 				* pReturnValue );

MODULAR INLINE CHIPAL_FAILCODE_T chipALWriteMemory (
    void				*RegIoContext,
    PVOID				BaseAddress,
    DWORD				OffSetAddress,
    DWORD				RegisterLength,
    DWORD				MaskValue,
    void *		 	Value );


/*******************************************************************************
Shared Module Defined Data Types
*******************************************************************************/


//*
//* From miniport.h or 	ntddk.h.  Due to redefinitions caused by directly
//* including these files, this structue is copied to this local file.  Future
//* versions ofthe  DDK may correct this problem.
//*
//* NOTE:  Starting with the NT 5.0 DDK, the defines were added to NDIS.H.  Now
//* they are only defined if they are not defined in the DDK path (NDIS.H).
//*
#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)	((LONG)&(((type *)0)->field))
#endif
#ifndef PCI_TYPE0_ADDRESSES
typedef struct _PCI_SLOT_NUMBER
{
	union
	{
		struct
		{
ULONG   DeviceNumber:5;
ULONG   FunctionNumber:3;
ULONG   Reserved:24;
		} bits;

		ULONG   AsULONG;
	} u;
} PCI_SLOT_NUMBER, *PPCI_SLOT_NUMBER;
#endif


//*
//* Shared Read ONLY  Bus Control Data
//*
//*
//* PCI Configuration Space 0.	  64 of 256 bytes defined.  The remaining
//* 192 bytes are user configuration space
//*
typedef struct tPCI_CONFIG_SPACE0 {
	USHORT	VendorId;
	USHORT	DeviceId;

	USHORT	Command;
	USHORT	Status;

DWORD	RevisionId:		8;
DWORD	ClassCode:		24;

DWORD	RSVD_01:		8;
DWORD	LatencyTimer:	8;
DWORD	HeaderType:		8;
DWORD	RSVD_00:		8;

DWORD	BaseAddress:	32;
DWORD	BaseAddress1:	32;

DWORD	RSVD_06:		32;
DWORD	RSVD_07:		32;
DWORD	RSVD_08:		32;
DWORD	RSVD_09:		32;
DWORD	RSVD_10:		32;

	USHORT	SubVendorId;
	USHORT	SubSystemId;

DWORD	RSVD_12:		32;
DWORD	RSVD_13:		32;
DWORD	RSVD_14:		32;

DWORD	InterruptLine:	8;
DWORD	InterruptPin:	8;
DWORD	MinGrant:		8;
DWORD	MaxLatency:		8;
}PCI_CONFIG_SPACE0, *PPCI_CONFIG_SPACE0;

//
//* PCI Configuration Space 1.	  64 of 256 bytes defined.  The remaining
//* 192 bytes are user configuration space
//
typedef struct tPCI_CONFIG_SPACE1 {
	USHORT	VendorId;
	USHORT	DeviceId;

	USHORT	Command;
	USHORT	Status;

DWORD	RevisionId:		8;
DWORD	ClassCode:		24;


DWORD	RSVD_00:		8;
DWORD	RSVD_01:		8;
DWORD	HeaderType:		8;
DWORD	RSVD_02:		8;

DWORD	BaseAddress:	32;

DWORD	RSVD_05:		32;
DWORD	RSVD_06:		32;
DWORD	RSVD_07:		32;
DWORD	RSVD_08:		32;
DWORD	RSVD_09:		32;
DWORD	RSVD_10:		32;
DWORD	RSVD_11:		32;
DWORD	RSVD_12:		32;
DWORD	RSVD_13:		32;
DWORD	RSVD_14:		32;



DWORD	InterruptLine:	8;
DWORD	InterruptPin:	8;
DWORD	RSVD_15:		16;

}PCI_CONFIG_SPACE1, *PPCI_CONFIG_SPACE1;

//*
//* Tuning parameters.  These are dynamic parameters that are loaded from an
//* external source (Registery, Disk File, etc...). This is a global data structure
//* that is allocated before the ChipAl Context/Data structure or any other
//* lower level Module context structure.
//*

typedef struct _CHIPAL_TUNING_T
{
	DWORD 					RxMaxLatency;
	DWORD					RxMinLatency;
	DWORD					RxInterruptRate;
	DWORD					RxSpeed;
	DWORD					RxFifoSize;
	WORD					RxChannelSize;
	WORD					RxSegments;
	DWORD					TxMaxLatency;
	DWORD					TxMinLatency;
	DWORD					TxInterruptRate;
	DWORD					TxSpeed;
	DWORD					TxFifoSize;
	WORD					TxChannelSize;
	WORD					TxSegments;
	DWORD					TotalSharedMemory;
	ULONG					PhysicalBase;
	ULONG					IRQ;
} CHIPAL_TUNING_T;

typedef struct _BUS_RESOURCE_T
{
	struct pci_dev*                 pPciARMDevice;
	struct pci_dev*                 pPciADSLDevice;

	PCI_CONFIG_SPACE0		PciFunctionZero;
	PCI_CONFIG_SPACE1		PciFunctionOne;

	PVOID					VirtualAddressBAR0;
	DWORD					LenBar0;
	PVOID					VirtualAddressBAR1;
	DWORD					LenBar1;
	PVOID					VirtualAddressARMBAR0;
	DWORD					LenARMBar0;

	//
	//Shared Memory
	//
	PVOID					VirtualAddressSharedMem;
	PVOID					PhysicalAddressSharedMem;

	//
	//IRQ Info
	//
	ULONG					PriIntVector;
	ULONG					PriIntLevel;
	BOOLEAN					PriIntRegistered;

	ULONG					SecIntVector;
	ULONG					SecIntLevel;
	BOOLEAN					SecIntRegistered;

	ULONG					InterruptReg;
	ULONG					MasterInterruptBit;

	DWORD					OwnedInterrupts;
	DWORD					SharedInterrupts;
	DWORD					DisableCount;
	BOOLEAN					InterruptsInitialized;
	BOOLEAN					MapRegAllocated;
	ULONG					RequiredSharedMemory;
	USHORT					InstanceNumber;

	BYTE					* Controller;		// NOTE:  This must be the Last Element
} BUS_RESOURCE_T;

#define NULL_PCI_RECOURCES ((BUS_RESOURCE_T *) 0)


//*
//* Bus Control Vector Table and Function Definition Definition
//*
typedef BUS_RESOURCE_T * (* PF_BUS_INIT)(
    I_O	CDSL_ADAPTER_T	* NdisAdapter,
    IN	CHIPAL_TUNING_T	* Tune,
    I_O	USHORT 			* MyAdapterCount
);

typedef CHIPAL_FAILCODE_T (* PF_BUS_GET_CONFIG)(
    IN	 CDSL_ADAPTER_T 		* CdslAdapter,
    I_O	CAL_CONFIG_PARAMS_T		* Params
);

typedef CHIPAL_FAILCODE_T (* PF_BUS_SET_CONFIG)(
    IN	 CDSL_ADAPTER_T 		* CdslAdapter,
    I_O	CAL_CONFIG_PARAMS_T		* Params
);

typedef struct _BUS_CONTROL
{
	PF_BUS_INIT Init;
	void (*Release) (CDSL_ADAPTER_T*,BUS_RESOURCE_T*);
	void (*Enable) (BUS_RESOURCE_T*,DWORD,DWORD);
	void (*Disable) (BUS_RESOURCE_T*,DWORD,DWORD);
	void (*GetStatus)(BUS_RESOURCE_T*,DWORD,DWORD*);
	void (*ClearStatus)(BUS_RESOURCE_T*,ULONG,DWORD);
	DWORD (*GetError)(BUS_RESOURCE_T*,DWORD);
	DWORD (*GetVersion)(BUS_RESOURCE_T*,CDSL_ADAPTER_T*);
	PF_BUS_GET_CONFIG GetConfig;
	PF_BUS_GET_CONFIG SetConfig;
} BUS_CONTROL_T;


typedef
CHIPAL_FAILCODE_T (*FN_REGIO_GET_STATUS)(
    void					* RegIoContext,
    void					* vBaseAddress,
    CHIPAL_DEVICE_STATUS_T	* DeviceStatus
);


//*
//* Register IO Control
//*
#define MAX_DEVICE_DESCRIPTION	25
struct _CHIPAL;

typedef struct _REG_IO_CONTROL
{
	#ifdef DBG
	CHAR				Description[MAX_DEVICE_DESCRIPTION];
	#endif
	CHIPAL_DEV_TYPE_T	DeviceType;
	DWORD				FunctionSelect;
	void 				*(*RegIoContInit) (void*,DWORD);
	void 				(*RegIoShutdown) (void*,DWORD);
	CHIPAL_FAILCODE_T	(*RegIoRead) (void*,PVOID,DWORD,DWORD,void*);
	CHIPAL_FAILCODE_T	(*RegIoWrite)(void*,PVOID,DWORD,DWORD,DWORD,void*);
	FN_REGIO_GET_STATUS RegIoGetStatus;

} REG_IO_CONTROL_T;
#define NULL_REG_IO_CONTEXT (REG_IO_CONTEXT_T *) 0

#define MAX_SERVICE_DESCRIPTION	MAX_DEVICE_DESCRIPTION

typedef struct CHIPAL_SERVICE_TABLE_S
{
	#ifdef DBG
	CHAR				Description[MAX_SERVICE_DESCRIPTION];
	#endif
	void 				*(*ServiceInit) (const void *); //APW CAL_SVC_TIGRIS
	void 				(*ServiceShutdown) (const void *);
	CHIPAL_FAILCODE_T	(*ServiceCommand) (CAL_MODULE_PARAMS_T *);
	CHIPAL_FAILCODE_T	(*ServiceConfigure)(CAL_MODULE_PARAMS_T *);
} CHIPAL_SERVICE_TABLE_T;



typedef struct _LONG_VALUE
{
	DWORD	HighPart;
	DWORD	LowPart;
} LONG_VALUE_T;



#if CAL_BASIC2_INSTALLED
	#include "ChipALBusCtlB2.h"
	#include "ChipALDmaChanB2.h"
#elif CAL_P46_INSTALLED
	#include "ChipALBusCtlP46V.h"
	#include "CardALTigrisHalV.h"
#endif


// Reverse Bit Ordering of a Byte
#define REVERSE_BYTE(n) \
			((n & 0x80) >> 7) | \
			((n & 0x40) >> 5) | \
			((n & 0x20) >> 3) | \
			((n & 0x10) >> 1) | \
			((n & 0x08) << 1) | \
			((n & 0x04) << 3) | \
			((n & 0x02) << 5) | \
			((n & 0x01) << 7) 


#define BYTE_SIZE 256
#define CHIPAL_CONTEXT_MEM_FLAGS 0
#define CHIPAL_ANY_ADDRESS_ACCEPTABLE NDIS_PHYSICAL_ADDRESS_CONST(-1,-1)

//*
//* DMA structures are now defined by ChipAl, not the lower level modules.
//* The following conditional allows for backwards compatibility.
//*
#if ! CAL_BASIC2_INSTALLED


#define 	FIFO_SIZE_TO_MASK(REG,VALUE)  VALUE

//*
//* Dma Configuration and Context Structure
//*
typedef struct _CHAN_DESCRIPT_T
{
	union DMA_CHAN_U
	{
		DWORD Value;					// Complete BitMapped Value

		struct
		{
DWORD Direction:		1;		// 0 Host->Device,  1 Device->Host
DWORD LocalBuffers:		1;		// 0 ChipAl Buffers, 1 User Buffers
DWORD FrameMode:		1;		// 0 Non-Framed Mode, 1 Framed Mode
DWORD ReverseBitOrder:	1;		// 1 Reverse Order of Byte (Receive)
DWORD Reserved1:		4;		// Reserved Field 1
DWORD Port:				4;		// DMA Port;
DWORD Channel:			6;		// DMA Channel;
DWORD Event:			8;
DWORD Reserved2:		6;		// Reserved Field 2
		} Element;
	} ElUnion;
} CHAN_DESCRIPT_T;

typedef struct _HW_CH_ENTRY
{
	// Public Config Data
	SK_BUFF_T			* (*ProcessData)(void*,SK_BUFF_T*);
	void				(*TxComplete)(void*,SK_BUFF_T*);
	CDSL_LINK_T 		* UserLink;
	WORD				SegmentSize;
	WORD				NumberOfSegments;
	CHAN_DESCRIPT_T		Description;
	BUS_RESOURCE_T 		* BusResource;
	DWORD  				PhysicalAddress;
	DWORD  				VirtualAddress;
	DWORD				TotalBufferSize;
	DWORD				FifoSize;
	CHIPAL_TUNING_T		* RegisteryDefault;
	SPIN_LOCK			ChannelLock;


	//*
	//* The last process union is used to determine how much data was processed
	//* during the last DPC.  In the case of cicrular buffers, this the last
	//* address processed is compared against the current address pointer of
	//* the device.  In a PIO mode, this is simply a count of the number of
	//* bytes last written to a fifo.
	//*
		union //LAST_PROCESSED_U
	{
		struct
		{
			DWORD		LastPhysical;
			DWORD		LastVirtual;
		} CirBuff;

		struct
		{
			DWORD		LastProcessed;
		} PIO;
	};


	#if DMA_STATS
	ULONGLONG			BytesProcessed;
	#endif

	BYTE				Busy;

	//*
	//* Extension is a place holder for device specific information
	//* The size and contents are defined by the specific Channel Controller.
	//* Basically, it is the Private context information.  A DWORD pointer is
	//* used to ensure DWORD alignment in the Extension.
	//*
	DWORD				Extension[1];

} HW_CH_ENTRY_T ;

#define NULL_HW_CH_ENTRY ((HW_CH_ENTRY_T *) 0)

//*
//* Definition of DMA Controller Functions (Vector Table)
//*
typedef HW_CH_ENTRY_T * (* FN_DMA_OPEN_HANDLER_T)
(
    VOID * 			UserHandle,
    HW_CH_ENTRY_T	* pChConfig
);

typedef VOID (* FN_DMA_CLOSE_HANDLER_T)
(
    HW_CH_ENTRY_T * pChConfig
);
typedef NTSTATUS (* FN_DMA_ENABLE_HANDLER_T)
(
    HW_CH_ENTRY_T * pChConfig
);
typedef NTSTATUS (* FN_DMA_DISABLE_HANDLER_T)
(
    HW_CH_ENTRY_T * pChConfig
);

typedef VOID (* FN_DMA_CLEAR_STATS_T)
(
    HW_CH_ENTRY_T * pChConfig
);

typedef VOID (* FN_DMA_GET_STATS_T)
(
    HW_CH_ENTRY_T * pCh,
    BD_CHIPAL_STATS_T * Stats
);

typedef VOID (* FN_DMA_GET_LAST_BUFF_T)
(
    HW_CH_ENTRY_T * pChanEntry,
    SK_BUFF_T *UserPkt
);

typedef struct _DMA_CONTROL
{
	FN_DMA_OPEN_HANDLER_T		ChanOpen;
	FN_DMA_CLOSE_HANDLER_T 		ChanClose;
	FN_DMA_ENABLE_HANDLER_T 	ChanEnable;
	FN_DMA_DISABLE_HANDLER_T 	ChanDisable;
	FN_DMA_GET_LAST_BUFF_T		GetLastBuffer;

	FN_DMA_GET_STATS_T			ChanGetStats;
	FN_DMA_CLEAR_STATS_T		ChanClearStats;

} DMA_CONTROL_T;

#if CAL_P46_INSTALLED
	#include "ChipALDmaChanP46.h"
#endif

#endif 									// ! CAL_BASIC2_INSTALLED

/*******************************************************************************
User Defined Data Types
*******************************************************************************/


//*
//* List entry for Chipal Callback Routines
//*
typedef struct _CAL_EVENT_LIST
{
	LIST_ENTRY					HandlerList;
	CHIPAL_EVENT_HANDLER_T		EventHandler;
	void						* UserContext;
} CAL_EVENT_LIST_T;


typedef struct _INTERRUPT_STATS
{
	DWORD					InterruptsMissed;
	DWORD					MaxLatency;
	DWORD					InterruptCount;
	DWORD					DeviceErrors;
} INTERRUPT_STATS_T;

typedef struct _CHIPAL
{
	struct _CHIPAL					* Next;
	CDSL_ADAPTER_T					* pNdisAdapter;
	BUS_CONTROL_T					* BusControl;
	BUS_RESOURCE_T					* BusResources;
	DMA_CONTROL_T					* DmaControl;

	#if (CHIPAL_REMOTE ==0)
	HW_CH_ENTRY_T					* DmaChannel[MAX_DATA_CHANNEL];
	#else
HW_CH_ENTRY_T					* DmaChannel[1];
	#endif 

	REG_IO_CONTROL_T				RegIoControl[MAX_HARDWARE_DEVICES];
	void							* RegIoContext[MAX_HARDWARE_DEVICES];
	SPIN_LOCK						RegIoLock[MAX_HARDWARE_DEVICES];
	CHIPAL_SERVICE_TABLE_T			Service[MAX_CHIPAL_SERVICES];
	void							* ServiceContext[MAX_CHIPAL_SERVICES];
	CHIPAL_TUNING_T					* Tune;
	INTERRUPT_STATS_T				IntStats[HW_EVENT_END];
	CARDAL_INTRPT_EVENT_HDLR_FN_T	MgmtEventHandler;
	LIST_ENTRY						EventHandler[NUM_FUNCTIONS][HW_MAX_EVENT_TYPES][HW_MAX_EVENTS];
	DWORD							InterruptStatus[NUM_FUNCTIONS][HW_MAX_EVENT_REGISTERS];
	DWORD							IsrEvent[NUM_FUNCTIONS][HW_MAX_EVENT_REGISTERS];
	SPIN_LOCK						IsrLock;

	#if	MAX_RESPONSE_SIZE			// ChipAl supplies Response Param(s) from ISR to DPC Events
	RESPONSE_T						ResponseParams[HW_MAX_EVENTS];
	#endif

	#if CAL_SOFT_TIMER
	struct timer_list				CalTimer;
	TIMER_STATES_T					CalTimerState;
	EVENT_HNDL						CalTimerStoppedEvent;
	#endif

} CHIPAL_T;

#define NULL_CHIPAL_T ((CHIPAL_T *) 0)

#define HARDWARE_INTERRUPT_REGISTER	0

/*******************************************************************************
Private Module Functions and Optional Services
*******************************************************************************/


/*******************************************************************************
Private Module Shared Data
*******************************************************************************/
typedef struct CAL_CRITICAL_CONTEXT_S
{
	CHIPAL_T			* pChipAl;
	VOID				* pVoid1;
	VOID				* pVoid2;
	VOID				* pVoid3;
	PLIST_ENTRY			pHeadList;
	PLIST_ENTRY			pEntry;
	BOOL				AddEvent;
} CAL_CRITICAL_CONTEXT_S;

typedef
BOOLEAN
(*PKSYNCHRONIZE_ROUTINE) (
    IN PVOID SynchronizeContext
);

#ifdef _CHIPALCDSL_C_

USHORT		MyAdapterCount = 0;
CHIPAL_T	ChipALHead = {  (struct _CHIPAL *) 0 };
CHIPAL_T	* pChipALHead = NULL_CHIPAL_T;
DWORD		ChipALDebugFlag;



//*
//* Static Vector Table for Register I/O Control Modules. The entries must
//* Be in sync with the enumeration HARDWARE_DEVICE_T, which is defined in
//* ChipAlCdsl.h.
//*

#ifdef DBG
#define MAPPED_MODULE_DESCRIPTION {"Memory Mapped Device    "},
#else
#define MAPPED_MODULE_DESCRIPTION
#endif

REG_IO_CONTROL_T IoDevice[MAX_HARDWARE_DEVICES] =
    {
        {									// Default Memory Read/Write
            MAPPED_MODULE_DESCRIPTION			// Description of Device
            DEV_TYPE_MEM_MAPPED,				// Type of Device (Mapped, Linear, etc..)
            0,									// no function for this device
            NULL,								// RegIoContInit - (Optional)
            NULL,								// RegIoShutdown - (Optional)
            chipALReadMemory,					// RegIoRead - (Required)
            chipALWriteMemory,					// RegIoWrite - (Required)
            NULL								// Status
        },

P46_ARM_DSL_IO_CONTROL,					//	DSL function ARM IO control

P46_ARM_IO_CONTROL,						// ARM function ARM IO control

IO_CONTROL_SEMW
    };

//*
//* Load Optional ChipAl Extension modules.  These are similar to the
//* REG_IO_CONTROL_T modules.  The primary difference being the names of the
//* member functions.  However, future modules may need additional entry points
//* that will not apply to the REG_IO_CONTROL_T module.
//*
//*	Any module that will likely not be used in future ChipAl versions should use
//* this vector table.
//*

CHIPAL_SERVICE_TABLE_T OptionalServices[MAX_CHIPAL_SERVICES] =
    {
        { /*CLN!!!*/
#ifdef DBG
	  "",
#endif
	  NULL,
	  NULL,
	  NULL,
	  NULL},
        CAL_SVC_TIGRIS
    };



#else
extern UCHAR	ReverseByteTable[BYTE_SIZE];
extern DWORD	ChipALDebugFlag;
#endif

#define DISPATCH_LEVEL	2

//*
//* The xxx_LONG_VALUE macros are used when a very Large UINT value is
//* needed. In the case of a 32 bit DWORD, LONG_VALUE_T is a 64 bit value,
//* which provides a max count of 1.844674407371e+19
//*
#define MAX_LONG_VALUE_PART 	0xffffffff

//*
//* ADD_LONG_VALUE - Adds a DWORD (_Amount)to a LONG_VALUE (_Dest).
//* Results stored in _Dest.
//*
#define ADD_LONG_VALUE(_Dest,_Amount)										\
	if (MAX_LONG_VALUE_PART - _Dest.LowPart < _Amount)						\
	{																		\
		_Dest.LowPart = _Amount - (MAX_LONG_VALUE_PART - _Dest.LowPart);	\
		_Dest.HighPart++;													\
	}																		\
	else																	\
	{																		\
		_Dest.LowPart += _Amount;											\
	}

//*
//* Divides a LONG_VALUE (_Source) by a DWORD (_Divisor).
//* Results are returned in _Rslts,
//*
#define DIV_LONG_VALUE(_Rslts,_Source,_Divisor)								\
	{																		\
		LONG_VALUE_T TmpRslts = _Source;									\
																			\
		_Rslts = 0;															\
		while (TmpRslts.HighPart != 0)										\
		{																	\
				_Rslts += TmpRslts.LowPart / _Divisor;						\
				TmpRslts.LowPart = TmpRslts.LowPart - ((TmpRslts.LowPart / _Divisor) * _Divisor);\
				TmpRslts.HighPart--;										\
				_Rslts += MAX_LONG_VALUE_PART / _Divisor;					\
				TmpRslts.LowPart += MAX_LONG_VALUE_PART - (MAX_LONG_VALUE_PART / _Divisor * _Divisor); \
		}																	\
		_Rslts += TmpRslts.LowPart / _Divisor;								\
	}


//* Move to Card Management

//*
//* Format of the PM Config Struct is defined by PM_CONFIG_REG_T. Currently,
//* it is defined as:
//*		REGISTER,SIZE,BITMASK,RESERVED,CLEAR_VALUE,CLEAR_REGISTER
//*

#define PM_CONTROL	VOICE_RX_DATA_2X| MASTER_CLOCK_FREQ|						\
					VOICE_INT_SENSE | ISDN_INT_SENSE | L85_CLK_HOLD |			\
					MODE_10490 | OSC_GATE | OSC_EN | USE_SIO485

#define PM_CONTROL2	B2_20410_VOICE_TX_FIFO | B2_20410_VOICE_RX_FIFO

#define PM_INTERRUPTS 	(ISDN_INTR|VOICE_INTR|DMA2_EN|DMA3_EN)

#define PM_GPIO_PINS		(GPIO_3|GPIO_4|GPIO_5|GPIO_6|GPIO_7)
#define ALL_BITS 0
#define PM_HOBBES_STATUS (TX_EMPTY|TX_AVAIL|TX_FULL|TX_THRESH|RX_IDLE|TIMER)

#define CHEETAH_PM_CONFIG_B2_ELEMENTS	14
#define CHEETAH_PM_CONFIG_B2 \
{																				\
	{BASIC2_IE,		sizeof(DWORD) , PM_INTERRUPTS,	0,DISABLE_INTERRUPT,TRUE},	\
	{BASIC2_STATUS,	sizeof(DWORD) , PM_INTERRUPTS, 	0,PM_INTERRUPTS,TRUE},		\
	{BASIC2_CONTROL,sizeof(DWORD) , PM_CONTROL,		0,0,FALSE},					\
	{BASIC2_CONTROL2,sizeof(DWORD), PM_CONTROL2, 	0,0,FALSE},					\
	{DMA_CONTROL,	sizeof(DWORD) , DMA3_EN|DMA2_EN|VOICE_TX_EN,0,0,FALSE},		\
	{GPDIR,			sizeof(DWORD) , PM_GPIO_PINS,	0,0,FALSE},					\
	{GPOUT, 		sizeof(DWORD) , PM_GPIO_PINS, 	0,0,FALSE},					\
	{HOBBES_CTL_REG,sizeof(BYTE), TX_SYNC | RX_SYNC | IDLE_RST,0,0,FALSE},		\
	{HOBBES_INT_EN, sizeof(BYTE), RX_IDLE | TX_THRESH | TIMER ,0,DISABLE_INTERRUPT,TRUE},	\
	{HOBBES_STATUS_REG, sizeof(BYTE), PM_HOBBES_STATUS,	0,PM_HOBBES_STATUS,TRUE},\
	{HOBBES_THRESHOLD, sizeof(BYTE),ALL_BITS,0,0,FALSE},						\
	{HOBBES_RX_IDLE_CHAR, sizeof(BYTE), ALL_BITS, 	0,0,FALSE},					\
	{HOBBES_RX_IDLE_TIMER, sizeof(BYTE), ALL_BITS, 	0,0,FALSE},					\
	{HOBBES_TIMER, 	sizeof(BYTE), ALL_BITS,0,0,FALSE}					\
}
// End Card Management

#endif									// _CHIPALCDSLV_H_


