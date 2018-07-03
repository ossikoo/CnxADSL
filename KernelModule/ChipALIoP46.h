/******************************************************************************
********************************************************************************
****	Copyright (c) 1997, 1998, 1999, 2000, 2001
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
**		ChipAL
**
**	FILE NAME:
**		ChipAlIoP46.h
**
**	ABSTRACT:
**		Public header file for register I/O via the P46/P51 device.
**
**	DETAILS:
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux 2.4 ATM Driver/KernelModule/ChipALIoP46.h $
**	$Revision: 3 $
**	$Date: 2/26/01 3:08p $
*******************************************************************************
******************************************************************************/

#ifndef  _CHIPALIOP46_H
#define  _CHIPALIOP46_H				// File Wrapper
#undef	CAL_IOP46_INSTALLED
#define	CAL_IOP46_INSTALLED	1

#if PROJECTS_NAME ==   PROJECT_NAME_MADMAX
	#define ARM_ADDRESS_MAP 1
	#define DSL_ADDDRESS_MAP 0
	#define MODEM_ADDRESS_MAP 0
#elif PROJECTS_NAME ==  PROJECT_NAME_TIGRIS
	#define ARM_ADDRESS_MAP 0
	#define DSL_ADDDRESS_MAP 1
	#define MODEM_ADDRESS_MAP 0
#elif PROJECTS_NAME ==   PROJECT_NAME_WDM_FILTER
	#define ARM_ADDRESS_MAP 1
	#define DSL_ADDDRESS_MAP 0
	#define MODEM_ADDRESS_MAP 0
#else
	#define ARM_ADDRESS_MAP 0
	#define DSL_ADDDRESS_MAP 0
	#define MODEM_ADDRESS_MAP 0
#endif

#include "Common.h"

/*******************************************************************************
Module Definitions
*******************************************************************************/




/*******************************************************************************
Module Data Types
*******************************************************************************/

#define MAX_ADDRESS_ID 0xFFFFFFFF

typedef enum HW_DEVICE_TYPE_
{
	DEVICE_TYPE_P46  = 0x00000001,
	DEVICE_TYPE_P51,
	DEVICE_TYPE_MAX
} HW_DEVICE_TYPE_E;

#define P46_ARM_BASE								0x00000000
#define DSL_ARM_BASE								0x00000000

#if ARM_ADDRESS_MAP
	#define CSR_ARM_BASE							P46_ARM_BASE
#elif DSL_ADDDRESS_MAP
	#define CSR_ARM_BASE							DSL_ARM_BASE
#elif MODEM_ADDRESS_MAP
	#error MODEM ARM CSR Base Address NOT DEFINED!
#else
	#error Unknown Address Map!
#endif

#define MAX_CSR_PARAMS	3

//* Pci Address Space
#define PCI_CONFIG_PM_CAP							0x00000042

//* PCI_PM_CSR -
#define PCI_PM_CSR									0x00000044
#define	PM_CSR_STATUS								0x8000
#define	PM_CSR_PME_EN								0x0100
#define PM_CSR_POWER_STATE							0x0003

//* Values for bits 1:0 of Power Management CSR
#define PM_CSR_STATE_D0								0x0000
#define PM_CSR_STATE_D1								0x0001
#define PM_CSR_STATE_D2								0x0010
#define PM_CSR_STATE_D3								0x0011

//* ARM Address Map
typedef enum _CSR_ARM_E
{
    CSR_ARM_START =					CSR_ARM_BASE,
    CSR_ARM_CTRL =					CSR_ARM_BASE +	0x00000000,
    CSR_ARM_ISR =					CSR_ARM_BASE +	0x00000004,
    CSR_ARM_IER =					CSR_ARM_BASE +	0x00000008,
    CSR_ARM_CMD =					CSR_ARM_BASE +	0x0000000C,
    CSR_ARM_RESP1 =					CSR_ARM_BASE +	0x00000010,
    CSR_ARM_RESP2 =					CSR_ARM_BASE +	0x00000014,
    CSR_ARM_PARAM1 =				CSR_ARM_BASE +	0x00000018,
    CSR_ARM_PARAM2 =				CSR_ARM_BASE +	0x0000001C,
    CSR_ARM_PARAM3 =				CSR_ARM_BASE +	0x00000020,
    CSR_ARM_STAT =					CSR_ARM_BASE +	0x00000024,
	#if ARM_ADDRESS_MAP
    CSR_ARM_AAIR=					CSR_ARM_BASE +	0x00000028,
    CSR_ARM_EPRM =					CSR_ARM_BASE +	0x0000002C,
    CSR_ARM_PBLC =					CSR_ARM_BASE +	0x00000030,
    CSR_ARM_BRR = 					CSR_ARM_BASE +	0x00000034,
    CSR_ARM_END = 					CSR_ARM_BRR
	#else
CSR_ARM_END = 					CSR_ARM_STAT
	#endif
                       ,CSR_ARM_DEBUG_BASE = 			CSR_ARM_BASE +	0x00000040
} CSR_ARM_T;


#ifdef _CHIPALIOP46_C_
volatile DWORD	NoBurstValue;
#else
extern volatile DWORD NoBurstValue;
#endif

#define			NO_BURST_ACCESS		CSR_ARM_CTRL
#define			NO_BURST_WRITE_VALUE		0xffffffff

#define 		NO_PCI_BURST(Bar)	\
	NoBurstValue = *(DWORD *)(Bar.pByte + NO_BURST_ACCESS)


#if DSL_ADDDRESS_MAP
//* DSL Address Map
typedef enum CSR_DSL_E
{
    CSR_DSL_TCC=					CSR_ARM_BASE +	0x00000028,
    CSR_DSL_RCC =					CSR_ARM_BASE +	0x0000002C,
    CSR_DSL_TIC =					CSR_ARM_BASE +	0x00000030,
    CSR_DSL_RIC =					CSR_ARM_BASE +	0x00000034,
    CSR_DSL_IIC =					CSR_ARM_BASE +	0x00000038,
    CSR_DSL_OCDC =					CSR_ARM_BASE +	0x0000003C,
    CSR_DSL_END = 					CSR_ARM_BASE
} CSR_DSL_T;
#endif  // DSL_ADDDRESS_MAP


//*
//*	CSR_ARM_CTRL
//*
#define NONE_CSR_CTRL_SWRESET			0x000000001	/* Global Device Software Initiated Reset */
#define NONE_CSR_CTRL_TIM0_MUX			0x000000006	/* Determines where Timer0 Interrupt will be routed */
#define NONE_CSR_CTRL_TIM1_MUX			0x000000018	/* Determines where Timer1 Interrupt will be routed */
#define NONE_CSR_CTRL_AIE				0x000000020	/* Global Arm host interrupt enable*/
#define NONE_CSR_CTRL_FIRMGO			0x000000040	/* Firmware Go*/
#define NONE_CSR_CTRL_FIFODEBUG			0x000000080	/* Fifo Debug Enable */
#define NONE_CSR_CTRL_DIS0_D3RST		0x000002000	/* Disable ARM function D3 to D0 Reset */
#define NONE_CSR_CTRL_DIS1_D3RST		0x000004000	/* Disable Modem Function D3 to D0 Reset */
#define NONE_CSR_CTRL_DIS2_D3RST		0x000008000	/* Disable ADSL Function D3 to D0 Reset */

#define TIGRIS_CSR_CTRL_SWRESET			0x000000001	/* Global Device Software Initiated Reset */
#define TIGRIS_CSR_CTRL_TIM0_MUX		0x000000006	/* Determines where Timer0 Interrupt will be routed */
#define TIGRIS_CSR_CTRL_TIM1_MUX		0x000000018	/* Determines where Timer1 Interrupt will be routed */
#define TIGRIS_CSR_CTRL_AIE				0x000000020	/* Global Arm host interrupt enable*/
#define TIGRIS_CSR_CTRL_FIRMGO			0x000000040	/* Firmware Go*/
#define TIGRIS_CSR_CTRL_FIFODEBUG		0x000000080	/* Fifo Debug Enable */
#define TIGRIS_CSR_CTRL_DIS0_D3RST		0x000002000	/* Disable ARM function D3 to D0 Reset */
#define TIGRIS_CSR_CTRL_DIS1_D3RST		0x000004000	/* Disable Modem Function D3 to D0 Reset */
#define TIGRIS_CSR_CTRL_DIS2_D3RST		0x000008000	/* Disable ADSL Function D3 to D0 Reset */

#define YUKON_CSR_CTRL_SWRESET			0x000000001	/* Global Device Software Initiated Reset */
#define YUKON_CSR_CTRL_TIM0_MUX			0x00000000E	/* Determines where Timer0 Interrupt will be routed */
#define YUKON_CSR_CTRL_TIM1_MUX			0x000000070	/* Determines where Timer1 Interrupt will be routed */
#define YUKON_CSR_CTRL_AIE				0x000000080	/* Global Arm host interrupt enable*/
#define YUKON_CSR_CTRL_FIRMGO			0x000000100	/* Firmware Go*/
#define YUKON_CSR_CTRL_FIFODEBUG		0x000000200	/* Fifo Debug Enable */
#define YUKON_CSR_CTRL_DIS0_D3RST		0x000002000	/* Disable ARM function D3 to D0 Reset */
#define YUKON_CSR_CTRL_DIS1_D3RST		0x000004000	/* Disable Modem Function D3 to D0 Reset */
#define YUKON_CSR_CTRL_DIS2_D3RST		0x000008000	/* Disable ADSL Function D3 to D0 Reset */

typedef struct ARM_CONTROL_REGISTER_S
{
	DWORD		CsrCtrlSwReset;
	DWORD		CsrCtrlTim0Mux;
	DWORD		CsrCtrlTim1Mux;
	DWORD		CsrCtrlAie;
	DWORD		CsrCtrlFirmGo;
	DWORD		CsrCtrlFifoDebug;
	DWORD		CsrCtrlDis0D3Rst;
	DWORD		CsrCtrlDis1D3Rst;
	DWORD		CsrCtrlDis2D3Rst;
} ARM_CONTROL_REGISTER_T;

//*
//*	CSR_ARM_ISR
//*
#define CSR_ISR_CMDE					0x000000001	/* Status of the CMD Empty Event; Set When Arm Reads Command Reg*/
#define CSR_ISR_RESPFIRQ				0x000000002	/* Status of the Response Full Event; Set When Arm Writes Data to RESP1*/
#define CSR_ISR_DEVREADY				0x000000004	/* Set if ARM is ready to commence normal operations */
#define CSR_ISR_DLREADY					0x000000008	/* Set if ARM is ready to Download */

//*
//* CSR_ARM_STAT
//*
#define CSR_STAT_CMDE					0x000000001	/* Command Empty */
#define CSR_STAT_DATAF					0x000000002	/* Response Reg 1 Valid */


//*
//*	CSR_DSL_CTRL
//*
#define CSR_DSL_SWRESET					0x00000001	/* Global Device Software Initiated Reset */
#define CSR_DSL_IDLE_INSERT				0x00000002	/* Enable Insertion of Idle Cells*/
#define CSR_DSL_STREAM					0x00000004	/* Stream Mode */
#define CSR_DSL_AIE						0x00000008	/* Host Interrupt Enable */
#define CSR_DSL_AFE_MUX					0x00000010	/* Host has access to AFE Interface */
#define CSR_DSL_TX_FIFO_MUX				0x00000020	/* Fifo is Mux set for ARM datapath */
#define CSR_DSL_RX_FIFO_MUX				0x00000040	/* Fifo is Mux set for ARM datapath*/
#define CSR_DSL_IDLE_CLEARD				0x00000080	/* ICIC is cleared after a ICIC Read*/
#define CSR_DSL_OVRN_CLEARD				0x00000100	/* OCDC register cleareas after a OCDC read*/
#define CSR_DSL_TEST_COUNT				0x00000200	/* Test TIC RIC ICIC OCDC */
#define CSR_DSL_DMA_HEADER				0x00000400	/* DMA Header Mode (vs Normal mode) */
#define CSR_DSL_DIS_TXSOC0				0x00000800	/* Sets TSCOC0 pin low */
#define CSR_DSL_DSL_LPBK				0x00001000	/* Loops Transmit to Receive*/
#define CSR_DSL_TX_BYTE_SWP				0x00002000	/* Sets byte swapping capability of Transmit Channel*/
#define CSR_DSL_RX_BYTE_SWP				0x00004000	/* Sets byte swapping capability of Receive Channel*/
#define CSR_DSL_RESERVED1				0x00008000	/* */
#define CSR_DSL_TXUBYTE					0x00FF0000	/* Sets Tx Underrun byte when in Stream Mode */

//*
//* EEPROM Register
//*
#define REG_EEPROM				0x0000002c

//*
//* EEPROM Masks
//*
#define SROMCS					0x00000001
#define SROMSK					0x00000002
#define SROMDO					0x00000004
#define SROMDI					0x00000008
#define SROENABLE				0x00000000

#define SROMCS_BIT_POSITION		0x0
#define SROMSK_BIT_POSITION		0x1
#define SROMDO_BIT_POSITION		0x2
#define SROMDI_BIT_POSITION		0x3
#define SROMEN_BIT_POSITION		0x0


//* Bootloader ARM Commands (ARM Function Only)
#define 	ARM_GET_VERSION									0x0001
#define 	ARM_READ_MEMORY									0x0002
#define 	ARM_SET_MEMORY_WRITE_ADDRESS					0x0003
#define 	ARM_WRITE_MEMORY								0x0004
#define 	ARM_CALL										0x0005
#define 	ARM_GOTO										0x0006
#define 	ARM_GET_EXCEPTION_TABLE_ADDRESS					0x0007
#define 	ARM_GET_RAM_DOWNLOAD_ADDRESS					0x0008
#define 	ARM_SET_COMMAND_EXTENSION_ROUTINE_ADDRESS		0x0009
#define 	ARM_GET_IDLE_EXTENSION_ROUTINE_ADDRESS			0x000A

//* Command Extensions (ARM Function Only)
#define 	ARM_SET_IO_DIRECTION							0x0100
#define 	ARM_SET_IO_EDGE									0x0101
#define 	ARM_SET_IO_ACCESS_PRIV							0x0102
#define 	ARM_SET_IO										0x0103
#define 	ARM_READ_IO										0x0104
#define 	ARM_SET_IO_CHANGE_IND							0x0105
#define 	ARM_START_PHYSICAL_TIMER						0x0120
#define 	ARM_STOP_PHYSICAL_TIMER							0x0121
#define 	ARM_READ_PHYSICAL_TIMER							0x0122
#define 	ARM_POWER_MANAGEMENT_EVENT						0x0180
#define 	ARM_GET_ERROR_INFO								0x01C0
#define 	ARM_CONFIGURATION_COMPLETE						0x016F
#define 	ARM_COMMAND_SYNC								0x01E0



//* Command Extensions (ADSL Function Only)
#define 	DSL_SET_IO										ARM_SET_IO
#define 	DSL_READ_IO										ARM_READ_IO
#define 	DSL_SET_IO_CHANGE_IND							ARM_SET_IO_CHANGE_IND
#define 	DSL_START_LOGICAL_TIMER							0x0128
#define 	DSL_STOP_LOGICAL_TIMER							0x0129
#define 	DSL_READ_LOGICAL_TIMER							0x012A
#define 	DSL_SET_ADSL_MODE								0x0160
#define 	DSL_SET_RX_IDLE_TIMEOUT							0x016F
#define 	DSL_START_TX_DMA								0x0140
#define 	DSL_START_RX_DMA								0x0141
#define 	DSL_STOP_DMA									0x0142
#define 	DSL_READ_COUNTER								0x01C1
#define 	DSL_RESET_COUNTER								0x01C2
#define 	DSL_QUEUE_FALCON_TX								0x01A0
#define 	DSL_QUEUE_FALCON_RX								0x01A1
#define 	DSL_CLEAR_FALCON_CMD_QUEUES						0x01A2
#define 	DSL_COMMAND_SYNC								ARM_COMMAND_SYNC

#define 	ARM_MAX_COMMAND									0x000A

#define		SENTINAL_ADDRESS								0xFFFF

//*
//* ARM CSR Param Definitions.
//*
typedef enum GPIO_DIRECTION_E
{
    GPIO_DIR_INPUT = 0,
    GPIO_DIR_OUTPUT
} GPIO_DIRECTION_T;

typedef enum GPIO_TRIGGER_E
{
    GPIO_TRIGGER_FALLING = 0,
    GPIO_TRIGGER_RISING
} GPIO_TRIGGER_T;

typedef enum GPIO_CHANGE_INDICATION_E
{
    GPIO_CHANGE_OFF = 0,
    GPIO_CHANGE_ON
} GPIO_CHANGE_INDICATION_T;

//*
//* ARM Command definitions. The order of the structure implies the order of the Params
//* that will be written.
//*
typedef struct COMMAND_SET_IO_S
{
	DWORD			Number;
	DWORD			Value;
} COMMAND_SET_IO_T;

typedef struct COMMAND_READ_IO_S
{
	DWORD			Number;
} COMMAND_READ_IO_T;

typedef struct COMMAND_SET_IO_CHANGE_S
{
	DWORD						Number;
	GPIO_CHANGE_INDICATION_T	ChangeMode;
} COMMAND_SET_IO_CHANGE_T;

typedef struct COMMAND_SET_IO_DIRECTION_S
{
	DWORD						Number;
	GPIO_DIRECTION_T			Direction;
} COMMAND_SET_IO_DIRECTION_T;

typedef struct COMMAND_SET_IO_EDGE_S
{
	DWORD						Number;
	GPIO_TRIGGER_T				Trigger;
} COMMAND_SET_IO_EDGE_T;

typedef struct COMMAND_SET_IO_ACCESS_S
{
	PCI_FN_NAME_T				Function;
	GPIO_TRIGGER_T				IoAccess;
} COMMAND_SET_IO_ACCESS_T;

typedef enum ARM_ACCESS_E
{
    ARM_ACCESS_8	= 										0x00000000,
    ARM_ACCESS_16	= 										0x00000001,
    ARM_ACCESS_32	= 										0x00000002
} ARM_ACCESS_T;

typedef enum ARM_INCREMENT_E
{
    ARM_AUTO_INCRMENT_NO =									0x00000000,
    ARM_AUTO_INCREMENT_YES =								0x00000001
} ARM_INCREMENT_T;

//*
//* Begin Definition of Arm Commands
//*
typedef struct COMMAND_READ_MEMORY_S
{
	DWORD			Address;
	ARM_ACCESS_T	AccessType;
} COMMAND_READ_MEMORY_T;

typedef struct COMMAND_SET_MEMORY_WRITE_ADD_S
{
	DWORD			Address;
	ARM_INCREMENT_T	AutoIncrement;
} COMMAND_SET_MEMORY_WRITE_ADD_T;

typedef struct COMMAND_WRITE_MEMORY_S
{
	DWORD			Value;
	ARM_ACCESS_T	AccessType;
} COMMAND_WRITE_MEMORY_T;

typedef struct COMMAND_CALL_S
{
	DWORD			Address;
} COMMAND_CALL_T;

typedef enum TIMER_MODE_E
{
    TIMER_MODE_ONE_SHOT_WO_IND = 0,
    TIMER_MODE_REPEAT_WO_IND,
    TIMER_MODE_ONE_SHOT_W_IND,
    TIMER_MODE_REPEAT_W_IND

} TIMER_MODE_T;

typedef struct COMMAND_START_LOGICAL_TIMER_S
{
	DWORD			LogicalTimer;
	TIMER_MODE_T	Mode;
	DWORD			MicroSeconds;
} COMMAND_START_LOGICAL_TIMER_T;

typedef struct COMMAND_STOP_LOGICAL_TIMER_S
{
	DWORD			LogicalTimer;
} COMMAND_STOP_LOGICAL_TIMER_T;

typedef enum ATM_MODE_E
{
    ATM_MODE_STREAM =										0x00000000,
    ATM_MODE_MANUAL_ATM =									0x00000001,
    ATM_MODE_IDLE_INS_ATM =									0x00000002
} ATM_MODE_T;

typedef struct COMMAND_SET_ADSL_MODE_S
{
	ATM_MODE_T		Mode;
	DWORD			BlockSize;
} COMMAND_SET_ADSL_MODE_T;

typedef enum RX_IDLE_MODE_E
{
    IDLE_DISABLE =											0x00000000,
    IDLE_ENABLE =											0x00000001
} RX_IDLE_MODE_T;


typedef struct COMMAND_SET_RX_IDLE_S
{
	RX_IDLE_MODE_T	Mode;
	DWORD			MicroSeconds;
} COMMAND_SET_RX_IDLE_T;

typedef enum CHANNEL_DIRECTION_E
{
    CHANNEL_RX =											0x00000000,
    CHANNEL_TX =											0x00000001,
    CHANNEL_RX_TX =											0x00000002
} CHANNEL_DIRECTION_T;

typedef struct COMMAND_STOP_DMA_S
{
	CHANNEL_DIRECTION_T	Direction;
} COMMAND_STOP_DMA_T;

typedef struct COMMAND_READ_COUNTER_S
{
	DWORD	Counter;
} COMMAND_READ_COUNTER_T;

typedef COMMAND_READ_COUNTER_T COMMAND_RESET_COUNTER_T;

typedef struct COMMAND_CLEAR_FALCON_QUEUES
{
	CHANNEL_DIRECTION_T	Direction;
} COMMAND_CLEAR_FALCON_QUEUES_T;

typedef enum RESPONSE_REQ_E
{
    RESPONSE_NO =											0x00000000,
    RESPONSE_YES =											0x00000001
} RESPONSE_REQ_T;

typedef struct COMMAND_SYNC
{
	RESPONSE_REQ_T	Response;
} COMMAND_SYNC_T;

typedef struct DMA_BUFF_CONTROL
{
DWORD	Count		:8;
DWORD	Handle		:8;
DWORD	Reserved	:16;
} DMA_BUFF_CONTROL_T;

typedef struct COMMAND_START_TX_DMA
{
	DWORD				Address;
	DMA_BUFF_CONTROL_T	Control;

}COMMAND_START_TX_DMA_T;

typedef COMMAND_START_TX_DMA_T COMMAND_START_RX_DMA_T;

typedef COMMAND_STOP_LOGICAL_TIMER_T COMMAND_READ_LOGICAL_TIMER_T;
typedef COMMAND_CALL_T COMMAND_GOTO_T;
typedef COMMAND_CALL_T COMMAND_SET_COMMAND_EXT_ADDRESS_T;
typedef COMMAND_CALL_T COMMAND_SET_IDLE_EXTENSION_T;
typedef COMMAND_CALL_T COMMAND_START_RX_DMA;


typedef void (* CSR_COMPLETEION_HANDLER_T)
(
    IN void			* pUserContext
);

typedef struct	CSR_COMMAND_PARAM_S
{
	DWORD									Command;				// CSR Command to Write

	union CSR_COMMAND_PARAM_U
	{
		DWORD								Raw[MAX_CSR_PARAMS];
		COMMAND_READ_MEMORY_T				ReadMemory;
		COMMAND_SET_MEMORY_WRITE_ADD_T		SetWriteAddress;
		COMMAND_WRITE_MEMORY_T				WriteMemory;
		COMMAND_CALL_T						Call;
		COMMAND_GOTO_T						Goto;
		COMMAND_SET_COMMAND_EXT_ADDRESS_T	CommandExtAddress;
		COMMAND_SET_IDLE_EXTENSION_T		IdleExtensionAddress;
		COMMAND_SET_IO_T					SetIo;
		COMMAND_READ_IO_T					ReadIo;
		COMMAND_SET_IO_CHANGE_T				IoChange;
		COMMAND_SET_IO_DIRECTION_T			IoDirection;
		COMMAND_SET_IO_EDGE_T				IoEdge;
		COMMAND_SET_IO_ACCESS_T				IoAccess;

		COMMAND_START_LOGICAL_TIMER_T		StartTimer;
		COMMAND_STOP_LOGICAL_TIMER_T		StopTimer;
		COMMAND_READ_LOGICAL_TIMER_T		ReadTimer;
		COMMAND_SET_ADSL_MODE_T				SetAdslMode;

		COMMAND_SET_RX_IDLE_T				SetRxIdle;
		COMMAND_START_TX_DMA_T				StartTxDma;
		COMMAND_START_RX_DMA_T				StartRxDma;
		COMMAND_STOP_DMA_T					StopDma;

		COMMAND_READ_COUNTER_T				ReadCounter;
		COMMAND_RESET_COUNTER_T				ResetCounter;

		COMMAND_CLEAR_FALCON_QUEUES_T		ClearFalconQueues;
		COMMAND_SYNC_T						CommandSync;
	}Param;

	CSR_COMPLETEION_HANDLER_T				NotifyOnComplete;		// Call Back Function
	VOID									* UserContext;			// Call Back Context

}CSR_COMMAND_PARAM_T;

typedef union P46_IO_U
{
	DWORD					Dword;
	CSR_COMMAND_PARAM_T		CsrParam;


} P46_IO_COMMAND_T;

typedef struct CSR_RESPONSE_S
{
	DWORD	Resp1;
	DWORD	Resp2;

} CSR_RESPONSE_T;


/*******************************************************************************
Module Public  Functions
*******************************************************************************/
MODULAR VOID *	CalP46Init(
    IN VOID				* pUserContext,
    IN DWORD              FunctSel
);


MODULAR VOID CalP46Shutdown(
    IN VOID				* pContext,
    IN DWORD			  FunctSel
);

MODULAR CHIPAL_FAILCODE_T CalP46ArmRead (
    IN void				* RegIoContext,
    IN PVOID			BaseAddress,
    IN DWORD			OffSetAddress,
    IN DWORD			RegisterLength,
    OUT void 			* pReturnValue );

MODULAR CHIPAL_FAILCODE_T CalP46ArmWrite (
    IN void				* RegIoContext,
    IN PVOID			BaseAddress,
    IN DWORD			OffSetAddress,
    IN DWORD			RegisterLength,
    IN DWORD			MaskValue,
    VOID				* Value );

MODULAR CHIPAL_FAILCODE_T CalP46ArmGetStatus (
    IN void					* RegIoContext,
    IN void					* vBaseAddress,
    OUT CHIPAL_DEVICE_STATUS_T	* DeviceStatus
);

#ifdef DBG
#define P46_ARM_DESCRIPTION {"ARM I/O Control"},
#else
#define P46_ARM_DESCRIPTION
#endif

#if PROJECTS_NAME == PROJECT_NAME_MADMAX
	#define IO_P46_GET_STATUS ,{CalP46ArmGetStatus}
#else
	#define IO_P46_GET_STATUS 
#endif

#define P46_ARM_DSL_IO_CONTROL 												\
	{																	\
		P46_ARM_DESCRIPTION												\
		DEV_TYPE_MEM_MAPPED,											\
		DSL_FUNCTION,       /* DSL function number */                   \
		CalP46Init,			/* RegIoContInit	*/						\
		CalP46Shutdown,		/* RegIoShutdown*/							\
		CalP46ArmRead,			/* RegIoRead	*/						\
		CalP46ArmWrite			/* RegIoWrite	*/						\
		IO_P46_GET_STATUS			/* RegIoGetStatus */				\
	}

#define P46_ARM_IO_CONTROL 												\
	{																	\
		P46_ARM_DESCRIPTION												\
		DEV_TYPE_MEM_MAPPED,											\
		ARM_FUNCTION,       /* DSL function number */                   \
		CalP46Init,			/* RegIoContInit	*/						\
		CalP46Shutdown,		/* RegIoShutdown*/							\
		CalP46ArmRead,			/* RegIoRead	*/						\
		CalP46ArmWrite			/* RegIoWrite	*/						\
		IO_P46_GET_STATUS			/* RegIoGetStatus */				\
	}

#endif 									// File Wrapper _CHIPALIOP46_H
