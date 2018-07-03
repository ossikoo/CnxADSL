/******************************************************************************
********************************************************************************
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
**		CardAL
**
**	FILE NAME:
**		CardALTigrisHalV.h
**
**	ABSTRACT:
**		CardALTigrisHal Private Header File
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/KernelModule/CardALTigrisHalV.h $
**	$Revision: 3 $
**	$Date: 7/05/01 12:06p $
*******************************************************************************
******************************************************************************/
#ifndef _CARDALTIGRISHALV_H				// File Wrapper
#define	_CARDALTIGRISHALV_H


#include "types.h"
#include "Common.h"
#include "CardMgmt.h"
#include "ChipALCdslV.h"
#include <asm/atomic.h>


/*******************************************************************************
Memory Map 
*******************************************************************************/
// We have multiple PCI Function DEVICES.
// Each PCI config space (V.90, ADSL, ARM/MadMax) has two base pointers
// (ABASE0 and ABASE1) found in BAR0 and BAR1 respectively in the PCI Configuration
// which point to regions containing blocks as specified below for MadMax and
// Crazy Ivan
// Within these 2 Regions (specified by a BAR) in each device
// we have several blocks which will be mapped to a DEVICE/Region combination
//
// MADMAX:
// 		V.90 (Not Implemented)
//				NA
// 		DSL
//				ABASE0 = ADSL_CSR space
//				ABASE1 = Micro/AFE space
// 		ARM/MadMax
//				ABASE0 = ARM_CSR spave
//				ABASE1 = FIFO space
//	CRAZY IVAN:
//		DSL
//				ABASE0 = DMA/ADSL_CSR/MICRO/AFE/GPIO/ACTEL
//
//
//	Here is the same information formatted differently (ABASE0=BAR0, ABASE1=BAR1)
//		BLOCK			P46 BAR/REGION	CRAZYIVAN BAR/REGION
//		-------			--------------	---------------------
//		ARM CSR			P46=ARM BAR0
//		DMA				P46=ARM BAR1	CRAZYIVAN=ADSL BAR0 (0)
//		ADSL CSR		P46=ADSL BAR0	CRAZYIVAN=ADSL BAR0 (B7000)
//		MICRO/AFE		P46=ADSL BAR1	CRAZYIVAN=ADSL BAR0 (B8000)
//		GPIO							CRAZYIVAN=ADSL BAR0 (B0200)
//		ACTEL							CRAZYIVAN=ADSL BAR0 (C0000)
//
// Although CrazyIvan only has one function and one region, it has numerous
// blocks within the region. We shall deal with each block as if it had its
// own distinct function/region so that it works the same as with MadMax
//
// Note: each BAR in a PCI Function appears as a separate "device" when talking
// to ChapAlWrite/Read function
//
// CRAZY IVAN NOTES
// The Crazy Ivan only supports one PCI config space as follows:
// 		ABASE0 = DMA controller & ARM Space (GPIO, Micro/AFE space)
// The DMA controller is at 0 and all space uses the ARM's memory map with an
// additional offsetof 0x80000 from ABASE0

#if CAL_P46_INSTALLED
//---------------------------------P46----------------------------------------

//...............................DEFINE THE BARS................................

// This macro is used in accessing the configuration information
// BAR0/1 are members of CAL_CONFIG_T

// We have two PCI functions (ADSL and ARM) and both have two BARs
// (EXCEPT Crazy Ivan does not duplicate this behavior - it has one BAR and one function)

#define	P46_BAR_ARM_CSR			BAR0
#define P46_BAR_DMA				BAR1
#define	P46_BAR_ADSL_CSR		BAR0
#define	P46_BAR_MICRO			BAR1
#define	P46_BAR_AFE				BAR1
//#define	P46_BAR_GPIO				// DOES NOT EXIST IN P46
//#define	P46_BAR_ACTEL				// DOES NOT EXIST IN P46
//...............................DEFINE THE DEVICES.............................

// This macro is used to identify Device when accessing memory
// (each appears as a separate "device")


//  Define them by service in terms of the function/bar combinations
#define	P46_DEVICE_ARM_CSR		P46_ARM_DSL
#define P46_DEVICE_DMA			P46_ARM_DSL
#define	P46_DEVICE_ADSL_CSR		P46_ARM_DSL
#define	P46_DEVICE_MICRO		P46_ARM_DSL
#define	P46_DEVICE_AFE			P46_ARM_DSL
//#define	P46_DEVICE_GPIO								// DOES NOT EXIST IN P46
//#define	P46_DEVICE_ACTEL							// DOES NOT EXIST IN P46




// .............................DEFINE THE OFFSETS..............................

#define DSL_AFE_OFFSET		0x800	// ADSL AFE Space
#define DSL_AFEC_OFFSET		0x800	// ADSL AFE Control Register
#define DSL_AFEA_OFFSET		0x804	// ADSL AFE ADC Register

//Host Micro is based on ABASE1
#define MICRO_OFFSET		0x0										// Micro (i.e. Falcon) address space

//Host ADSL CSRs is based on ABASE0

#define DSL_CTRL_OFFSET		0x0		// ADSL Control Register
#define DSL_ISR_OFFSET		0x4		// ADSL Interrupt Status Register
#define DSL_IER_OFFSET		0x8		// ADSL Interrupt Enable Register
#define DSL_CMD_OFFSET		0xC		// Command Register for ADSL Command Interface
#define DSL_RESP1_OFFSET	0x10	// Response Register 1 for ADSL Command Interface
#define DSL_RESP2_OFFSET	0x14	// Response Register 2 for ADSL Command Interface
#define DSL_PARM1_OFFSET	0x18	// Param Register 1 for ADSL Command Interface
#define DSL_PARM2_OFFSET	0x1C	// Param Register 2 for ADSL Command Interface
#define DSL_PARM3_OFFSET	0x20	// Param Register 3 for ADSL Command Interface
#define DSL_STAT_OFFSET		0x24	// Status Register for ADSL Command Interface
#define DSL_TCC_OFFSET		0x28	// ADSL Transmit Channel Current Count Register (16 bits)
#define DSL_RCC_OFFSET		0x2C	// ADSL Receive Channel Current Count Register (16 bits)
#define DSL_TIC_OFFSET		0x30	// ADSL Transmit Host Interrupt Count (16 bits)
#define DSL_RIC_OFFSET		0x34	// ADSL Receive Host Interrupt Count (16 bits)
#define DSL_ICIC_OFFSET		0x38	// ADSL ATM Idle Cell Insertion Count
#define DSL_OCDC_OFFSET		0x3C	// ADSL ATM Overrun Cell Discard Count






//ARM DMA
// ...TBD...

//ARM CSR
// ...TBD...

#endif	// END OF P46
//------------------------------------------------------------------------------






/*******************************************************************************
Structures 
*******************************************************************************/




// **************** AFE Registers ********************************************
typedef union	//lsb first
{
	struct
	{
DWORD	AFEDAT			:8 ;	// BITS0-7	AFE Data
		//			(Used when ADCREAD == 0 )

DWORD	AFEADD			:6 ;	// BITS8-13	AFE Register Number
		//			(Used when ADCREAD == 0 )

DWORD	AFERWN			:1 ;	// BIT14	Used when ADCREAD==0
		//			0 = Write AFEDAT
		//			1 = Read into AFEDAT

DWORD	ADCREAD			:1 ;	// BIT15
		//			0 = Operation controlled by AFERWN
		//			1 = Read the ADC tone detection data word into DSL_AFEA.ADCDAT

DWORD	AFESTART		:1 ;	// BIT16	Start Transaction bit(See bits14-15)
DWORD	Filler			:15 ;	// BITS17-31
	} Bit ;
	DWORD	DWord ;					// BITS0-31
} DSL_AFEC_T ;

typedef union	//lsb first
{
	struct
	{
DWORD	ADCDAT			:16 ;	// BITS0-15	ADC tone detection read value
		//			(used when ADCREAD==1)
DWORD	Filler			:16 ;	// BITS16-31
	} Bit ;
	DWORD	DWord ;					// BITS0-31
} DSL_AFEA_T ;
















/*******************************************************************************
Private  Module Data Types
*******************************************************************************/
/*******************************************************************************
Structures 
*******************************************************************************/

//*
//*	Development PCI Register Definitions
//*

//*
//* PCI DMA Control Register
//*
#define FPGA_DMA_CSR		0x00000000	// PCI DMA Control Register

//* Bit Masks in Register
#define FPGA_BIT_INT_ENA	0x00000001	// Master Interrupt Enable

//*
//* FPGA_DSL_CSR
//*
#define FPGA_DSL_CSR		0x000B7000	// PCI DMA Control Register

//* Bit Masks in Register

//Host ADSL CSRs is based on ABASE0
#define FPGA_ISR_OFFSET		FPGA_DSL_CSR +0x4	// ADSL Interrupt Status Register
#define FPGA_IER_OFFSET		FPGA_DSL_CSR +0x8	// ADSL Interrupt Enable Register


//----------------------------------------------------------------------------



// ADSL Control (ADSL Control Register)
typedef union
{
	struct
	{
DWORD	SRESET			:1 ;	// BIT0		Global DSL PC SOfware Initiated Reset
		//			Must be set and cleared by host
DWORD	IDLE_INSERT		:1 ;	// BIT1		Enables idle cell insertion for TX serial channel
DWORD	STREAM			:1 ;	// BIT2		Tells DMA whether DMA will be operatin in AATM cell or STERAM mode
		//			0=ATM Cell Data
		//			1=STREAM Data
DWORD	AIE				:1 ;	// BIT3		Global ADSL host interrupt enable
DWORD	AFEMUX			:1 ;	// BIT4		Controls muxing of AFE interface
		//			0=ARM has access to AFE Interface
		//			1=Host has access to AFE Interface
DWORD	TXFIFOMUX		:1 ;	// BIT5		Control muxing of the transmit FIFOF datapath
		//			0=Host datapath selected
		//			1=ARM datapath selected
DWORD	RXFIFOMUX		:1 ;	// BIT6		Control muxing of the receive FIFOF datapath
		//			0=Host datapath selected
		//			1=ARM datapath selected
DWORD	IDLECLRD		:1 ;	// BIT7		Controls the clear function for DSL_ICIC
		//			0=ICIC register clears on rollover
		//			1=ICIC register clears immediately after a ICIC read
DWORD	OVRNCLRD		:1 ;	// BIT8		Controls the clear function for DSL_OCDC
		//			0=OCDC register clears on rollover
		//			1=OCDC register clears immediately after a OCDC read
DWORD	TESTCOUNT		:1 ;	// BIT9		Puts all ADSL counters in test mode (TIC, RIC, ICIC, OCDC)
DWORD	DMAHEADER		:1 ;	// BIT10	Puts ADSL block into DMA header mode
		//			(STREAM must be 0 )
DWORD	DIS_TXSOC0		:1 ;	// BIT11	Disable TXSOC0 pulsing on ADSL interface
		//			(STREAM must be 0 )
DWORD	DSL_LPBK		:1 ;	// BIT12	ADSL block loopback enable
DWORD	Filler			:3 ;	// BITS13-15
DWORD	TXUBYTE			:8 ;	// BITS16-23Transmit Underrun Byte
DWORD	Filler2			:8 ;	// BITS24-31
	} Bit ;
	DWORD	DWord ;					// BITS0-31
} DSL_CTRL_T ;

//These following equates are from an aborted method of accessing the bits - via
//bit mask rather than bit fields. They are retained for now because ChipAlBusCtlP46
//is already using them.
//*
//*	DSL_CTRL_OFFSET Bits - DSL prefix not used for bits commmon to all Functions
//*
#define CSR_BIT_AIE			0x00000008	// AIE: Global host Interrupt Enable
#define CSR_BIT_AIE_DISABLE	0x00000000	// AIE: Host Interrupts Disabled


// ADSL Interrupt Status Register
typedef union
{
	// Interrupts are cleared by writing ONE to the ISR except where noted
	// Many interrupts are routable and must be routined to host (as oppossed to the ARM0
	// before they are used.


	struct
	{
DWORD	CMDE			:1 ;	// BIT0		Command Empty Interrupt Status
DWORD	DATAF			:1 ;	// BIT1		Data Full Interrupt Status
DWORD	ARMIRQ0			:1 ;	// BIT2		Generic ARM Interrupt 0
DWORD	ARMIRQ1			:1 ;	// BIT3		Generic ARM Interrupt 1
DWORD	ARMIRQ2			:1 ;	// BIT4		Generic ARM Interrupt 1
DWORD	ARMIRQ3			:1 ;	// BIT5		Generic ARM Interrupt 1
DWORD	ARMIRQ4			:1 ;	// BIT6		Generic ARM Interrupt 1
DWORD	ARMIRQ5			:1 ;	// BIT7		Generic ARM Interrupt 1
DWORD	ARMIRQ6			:1 ;	// BIT8		Generic ARM Interrupt 1
DWORD	MABRTIRQ		:1 ;	// BIT9		Master Abort Interrupt Status
		//			Set if ARM sets either‘Received Target Abort’
		//			or ‘Received Master Abort’ bit in ADSL PCI
		//			config space status register
DWORD	TX_COUNT		:1 ;	// BIT10	Set when ADSL Transmit Channel Current Count
		//			matches the ADSL Transmit Interrupt Count.
		//			*Used only if ARM_AAIR:RTE_TX_COUNT=1
		//			In data stream mode the count is in dwords.
		//			In ATM mode the count is in ATM cells.
DWORD	TX_UNDERRUN		:1 ;	// BIT11	Transmit FIFO has underrun
		//			*Used only if ARM_AAIR:RTE_TX_UNDERRUN=1
DWORD	RX_COUNT		:1 ;	// BIT12	Set when ADSL Receive Channel Current Count
		//			matches the ADSL Receive Interrupt Count/
		//			*Used only if ARM_AAIR:RTE_RX_COUNT=1
		//			In data stream mode the count is in dwords.
		//			In ATM mode the count is in ATM cells.
DWORD	RX_OVERRUN		:1 ;	// BIT13	Receive FIFO has overrun
		//			*Used only if ARM_AAIR:RTE_RX_OVERRUN=1
DWORD	IDLE_CNTOV		:1 ;	// BIT14	Idle cell insertion counter overflow
		//			*Used only if ARM_AAIR:RTE_IDLE_CNTOV=1
DWORD	DISCARD_CNTOV	:1 ;	// BIT15	Discard cell counter overflow
		//			*Used only if ARM_AAIR:RTE_DISCARD_CNTOV=1
DWORD	TIMER0			:1 ;	// BIT16	General purpose timer 0 interrupt
		//			*Used only if ARM_AAIR:RTE_TIMER0=1
DWORD	TIMER1			:1 ;	// BIT17	General purpose timer 1 interrupt
		//			*Used only if ARM_AAIR:RTE_TIMER1=1
DWORD	FALIRQ1			:1 ;	// BIT18	Falcon IRQ1 interrupt
		//			*Used only if ARM_AAIR:RTE_FALIRQ1=1
		//			***The host must clear the Falcon IRQ1 interrupt
		//			in order for this interrupt bit to be cleared
DWORD	FALIRQ2			:1 ;	// BIT19	Falcon IRQ2 interrupt
		//			*Used only if ARM_AAIR:RTE_FALIRQ2=1
		//			***The host must clear the Falcon IRQ2 interrupt
		//			in order for this interrupt bit to be cleared
DWORD	GPIO_INT		:1 ;	// BIT20	General purpose input/output interrupt
		//			Generic interrupt - Requires host to
		//			poll ARM via command to retrieve specifics
DWORD	AFE_INT			:1 ;	// BIT21	Analog Front End interrupt
		//			*Used only if ARM_AAIR:RTE_AFEINT=1
		//          Indicates DSL_AFEC AFESTART bit has
		//			transitioned from one to zero.
	} Bit ;
	DWORD	DWord ;					// BITS0-31
} DSL_ISR_T ;



// ADSL Interrupt Enable Register
typedef union
{
	// Many interrupts are routable and must be routined to host (as oppossed to the ARM0
	// before they are used.


	struct
	{
DWORD	ECMDE			:1 ;	// BIT0		Command Empty Interrupt Enable
DWORD	EDATAF			:1 ;	// BIT1		Data Full Interrupt Enable
DWORD	EARMIRQ0		:1 ;	// BIT2		Generic ARM Enable Interrupt 0
DWORD	EARMIRQ1		:1 ;	// BIT3		Generic ARM Enable Interrupt 1
DWORD	EARMIRQ2		:1 ;	// BIT4		Generic ARM Enable Interrupt 1
DWORD	EARMIRQ3		:1 ;	// BIT5		Generic ARM Enable Interrupt 1
DWORD	EARMIRQ4		:1 ;	// BIT6		Generic ARM Enable Interrupt 1
DWORD	EARMIRQ5		:1 ;	// BIT7		Generic ARM Enable Interrupt 1
DWORD	EARMIRQ6		:1 ;	// BIT8		Generic ARM Enable Interrupt 1
DWORD	EMABRTIRQ		:1 ;	// BIT9		Master Abort Interrupt Enable
		//			Set if ARM sets either‘Received Target Abort’
		//			or ‘Received Master Abort’ bit in ADSL PCI
		//			config space status register
DWORD	ETX_COUNT		:1 ;	// BIT10	Set when ADSL Transmit Channel Current Count
		//			matches the ADSL Transmit Interrupt Count.
		//			*Used only if ARM_AAIR:RTE_TX_COUNT=1
		//			In data stream mode the count is in dwords.
		//			In ATM mode the count is in ATM cells.
DWORD	ETX_UNDERRUN	:1 ;	// BIT11	Transmit FIFO has underrun
		//			*Used only if ARM_AAIR:RTE_TX_UNDERRUN=1
DWORD	ERX_COUNT		:1 ;	// BIT12	Set when ADSL Receive Channel Current Count
		//			matches the ADSL Receive Interrupt Count/
		//			*Used only if ARM_AAIR:RTE_RX_COUNT=1
		//			In data stream mode the count is in dwords.
		//			In ATM mode the count is in ATM cells.
DWORD	ERX_OVERRUN		:1 ;	// BIT13	Receive FIFO has overrun
		//			*Used only if ARM_AAIR:RTE_RX_OVERRUN=1
DWORD	EIDLE_CNTOV		:1 ;	// BIT14	Idle cell insertion counter overflow
		//			*Used only if ARM_AAIR:RTE_IDLE_CNTOV=1
DWORD	EDISCARD_CNTOV	:1 ;	// BIT15	Discard cell counter overflow
		//			*Used only if ARM_AAIR:RTE_DISCARD_CNTOV=1
DWORD	ETIMER0			:1 ;	// BIT16	General purpose timer 0 interrupt
		//			*Used only if ARM_AAIR:RTE_TIMER0=1
DWORD	ETIMER1			:1 ;	// BIT17	General purpose timer 1 interrupt
		//			*Used only if ARM_AAIR:RTE_TIMER1=1
DWORD	EFALIRQ1		:1 ;	// BIT18	Falcon IRQ1 interrupt
		//			*Used only if ARM_AAIR:RTE_FALIRQ1=1
DWORD	EFALIRQ2		:1 ;	// BIT19	Falcon IRQ2 interrupt
		//			*Used only if ARM_AAIR:RTE_FALIRQ2=1
DWORD	EGPIO_INT		:1 ;	// BIT20	General purpose input/output interrupt
		//			Generic interrupt - Requires host to
		//			poll ARM via command to retrieve specifics
DWORD	EAFE_INT		:1 ;	// BIT21	Analog Front End interrupt
		//			*Used only if ARM_AAIR:RTE_AFEINT=1
		//          Indicates DSL_AFEC AFESTART bit has
		//			transitioned from one to zero.
	} Bit ;
	DWORD	DWord ;					// BITS0-31
} DSL_IER_T ;


// ADSL Status Register
typedef union
{
	// Many interrupts are routable and must be routed to host (as oppossed to the ARM0
	// before they are used.


	struct
	{
DWORD	DATAF			:1 ;	// BIT0		Set when ARM writes to RESP1
		//			Cleared when host reads RESP1
DWORD	CMDE			:1 ;	// BIT1		Cleared when host writes to CMD
		//			Set when ARM reads CMD
DWORD	Filler			:30 ;	// BITS2-31
	} Bit ;
	DWORD	DWord ;					// BITS0-31
} DSL_STAT_T ;






//*
//* Primary Context structure for this Module
//*
typedef struct
{
	void *pInterface ;

	// Counts nested calls to DisableInterrupt
	int  DisableInterruptCount ;
	volatile BOOL  DeviceRdyState ;
	MODULAR void ( *DeviceRdyNotify )
	(
	    IN CDSL_ADAPTER_T			* pThisAdapter
	) ;

	BOOLEAN	SwitchHookState ;

	WORD	ADSL_Relay_Set_Value ;
	WORD	Modem_Relay_Set_Value ;
	WORD	ADSL_LED1_Value ;
	WORD	ADSL_LED2_Value ;

	BOOLEAN	AFE_Print ;


} TIGRIS_HAL_CONTEXT_T;



#endif	// File Wrapper
