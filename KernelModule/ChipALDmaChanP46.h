/**************************************************************************************
Module:		ChipAL
File:		ChipALDmaChanP46.h
Function:	Public Header for Internal ChipAl DMA module.
Description:                        	
			

			Copyright 1998 Rockwell International
***************************************************************************************
$Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/ChipALDmaChanP46.h $
$Revision: 1 $
$Date: 1/09/01 10:53a $
**************************************************************************************/

#ifndef _CHIPALDMACHANP46_H_				//	FILE WRAPPER,
#define _CHIPALDMACHANP46_H_				//	prevents multiple inclusions

#define MAX_HARDWARE_INTERRUPTS		32

//* Map Generic P46 definitions to DMA specific usage
#define INT_DMA_TX0		INT_ARMIRQ0
#define INT_DMA_RX0		INT_ARMIRQ1
#define HW_DMA_TX0		HW_ARMIRQ0
#define HW_DMA_RX0		HW_ARMIRQ1

#define DMA_CACHE_ENABLED 0

typedef enum _DMA_DEFINE_E
{
    HOST_TO_DEVICE = 0,
    NO_REVERSE_BIT_ORDER = 0,
    NON_FRAMED_MODE = 0,
    LOCAL_BUFFERS = 0,
    DEVICE_TO_HOST = 1,
    REVERSE_BIT_ORDER = 1,
    FRAME_MODE = 1,
    ADSL_DMA_PORT = 0,
    ADSL_DMA_CHAN_TX = 0,
    ADSL_DMA_CHAN_RX = 1


} DMA_DEFINES_T;

// This should describe number of available DMA Channels (ADSL_DMA_CHAN_RX + ADSL_DMA_CHAN_TX)
#define 	MAX_DATA_CHANNEL 	2

typedef struct _BUFF_STATUS_B2
{
DWORD	Ready:		1;				// True if Buffer is Ready for processing
DWORD	Error:		1;				// TRUE if An error occurred;
DWORD	Reserved1:	30;
} BUFF_STATUS_T;

typedef struct _MAC_RESERVED_T
{
	BUFF_STATUS_T	Status;
	UCHAR			* HeaderInfo;
	WORD			HeaderLength;
	WORD			Reserved1;
	DWORD			LinkHandle;
} MAC_RESERVED_T;

/*******************************************************************************
Private Module Functions - These functions are called by ChipAL as vector
table entries(via ChipALAddEventHandler(n).  They should not be called directly.
*******************************************************************************/

MODULAR void DmaChanP46RxDataHandler (
    HW_CH_ENTRY_T * pChanEntry,
    OUT RESPONSE_T	* Response
);

MODULAR void DmaChanP46TxDataHandler (
    HW_CH_ENTRY_T * pChanEntry,
    OUT RESPONSE_T	* Response
);


//*
//* DmaChanP46 is the Public service/vector table of type "Dma Channel Controller",
//* (See ChipAl.doc). All upper layer routines should use the vector table
//* functions.  This will allow all or part of this service class to replaced by
//* a new module.
//*
#if defined( _CHIPALDMACHANP46_C)
//*
//* Public Entry points via Vector table. If multiple DmaChan Class drivers
//* exist, move these functions to a private header file.  This will be
//* necessary to be able to re-use these functions in a new DmaChan Class module.
//*
MODULAR HW_CH_ENTRY_T *  ChipALDmaChanP46Open(
    VOID * 			UserHandle,
    HW_CH_ENTRY_T	* pChConfig);

MODULAR void			ChipALDmaChanP46Close(
    HW_CH_ENTRY_T * pChConfig
);

MODULAR NTSTATUS	ChipALDmaChanP46Enable(
    HW_CH_ENTRY_T * pCh
);

MODULAR NTSTATUS		ChipALDmaChanP46Disable(
    HW_CH_ENTRY_T * pCh
);

MODULAR void ChipALDmaChanP46GetLastBuffer (
    HW_CH_ENTRY_T * pChanEntry,
    SK_BUFF_T *UserPkt
);

#if DMA_STATS
MODULAR void ChipALDmaChanP46GetStats(
    HW_CH_ENTRY_T * pCh,
    BD_CHIPAL_STATS_T * Stats);

MODULAR void ChipALDmaChanP46ClearStats(
    HW_CH_ENTRY_T * pCh);


#endif

// Static Public Vector Table
DMA_CONTROL_T DmaChanP46 =
    {
        ChipALDmaChanP46Open,
        ChipALDmaChanP46Close,
        ChipALDmaChanP46Enable,
        ChipALDmaChanP46Disable,
        ChipALDmaChanP46GetLastBuffer

		#if DMA_STATS
        ,
        ChipALDmaChanP46GetStats,
        ChipALDmaChanP46ClearStats

		#else
,
        NULL,
        NULL
		#endif
    };

#else									// _CHIPALDMACHANP46_C
MODULAR extern DMA_CONTROL_T DmaChanP46;
	#define	DMA_CHAN_CONTROLLER	 DmaChanP46
#endif									// _CHIPALDMACHANP46_C

#endif									// _REGIOP46_H_
