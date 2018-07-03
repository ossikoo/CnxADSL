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
**		ChipALBusCtlP46.c
**
**	ABSTRACT:
**		Bus Control Service for ChipAL Module
**
**	DETAILS:
**		This module is responsible for the following:
**
**		Configuring the Bus Controller Device
**		Obtaining system allocated resources (IRQ, Base Memory Address, etc...)
**		Allocating additional System Hardware Resources
**		Hooking Interrupts
**		Disabling/Enabling Interrupts
**		Getting Interrupt Status
**		Clearing Interrupts - Bus Controller Only.  Other modules may clear Device
**			specific conditions that resulted in a interrupt condition to the 
**			Bus Controller.
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux 2.4 ATM Driver/KernelModule/ChipALBusCtlP46.c $
**	$Revision: 6 $
**	$Date: 2/26/01 3:10p $
*******************************************************************************
******************************************************************************/
#define CHIPALBUSCTLP46_C				// File ID Flag

/*******************************************************************************
Includes
*******************************************************************************/
#include "Version.h"
#include "CardMgmt.h"
#include "ChipALCdslV.h"
#include "ChipALBusCtlP46V.h"
#include "CnxADSL.h"

/*******************************************************************************
External Data
*******************************************************************************/
extern int		CnxtDslVendorId;
extern int		CnxtDslArmDeviceId;
extern int		CnxtDslAdslDeviceId;
extern int		CnxtDslPhysicalDriverType;

/*******************************************************************************
Private Bus Module Definitions
*******************************************************************************/
#define BUS_ERROR_FATAL(error)		(CHIPAL_FATAL | CHIPAL_MODULE  | CHIPAL_SYSTEM_SERVICE | CHIPAL_P46 | error)
#define BUS_ERROR_WARNING(error)	(CHIPAL_WARNING | CHIPAL_MODULE  | CHIPAL_SYSTEM_SERVICE | CHIPAL_P46 | error)

#define AVAILABLE_INTERRUPTS	0

#if ! defined(MEMORY_TAG)
	#define MEMORY_TAG			'4PLC'
#endif

#define MAX_INTERRUPT_BITS		0x003FFFFF

/*******************************************************************************
Private Module Data	Types
*******************************************************************************/
// *
// * Bus Control Local Context Structure
// *
typedef struct _BUS_CONTEXT_T
{
	DWORD	DeviceId;					// Pci Device Id
	DWORD	VendorId;					// Pci Vendor Id
} BUS_CONTEXT_T;

#define NULL_BUS_CONTEXT ((BUS_CONTEXT_T *) 0)

/*******************************************************************************
Private Module Data - Bus Control Basic 2 PCI (calBusCtlP46)
*******************************************************************************/
ARM_CONTROL_REGISTER_T			ArmControlRegister [DEVICE_TYPE_MAX] =
{
	{
		NONE_CSR_CTRL_SWRESET,
		NONE_CSR_CTRL_TIM0_MUX,
		NONE_CSR_CTRL_TIM1_MUX,
		NONE_CSR_CTRL_AIE,
		NONE_CSR_CTRL_FIRMGO,
		NONE_CSR_CTRL_FIFODEBUG,
		NONE_CSR_CTRL_DIS0_D3RST,
		NONE_CSR_CTRL_DIS1_D3RST,
		NONE_CSR_CTRL_DIS2_D3RST
	},
	{
		TIGRIS_CSR_CTRL_SWRESET,
		TIGRIS_CSR_CTRL_TIM0_MUX,
		TIGRIS_CSR_CTRL_TIM1_MUX,
		TIGRIS_CSR_CTRL_AIE,
		TIGRIS_CSR_CTRL_FIRMGO,
		TIGRIS_CSR_CTRL_FIFODEBUG,
		TIGRIS_CSR_CTRL_DIS0_D3RST,
		TIGRIS_CSR_CTRL_DIS1_D3RST,
		TIGRIS_CSR_CTRL_DIS2_D3RST
	},
	{
		YUKON_CSR_CTRL_SWRESET,
		YUKON_CSR_CTRL_TIM0_MUX,
		YUKON_CSR_CTRL_TIM1_MUX,
		YUKON_CSR_CTRL_AIE,
		YUKON_CSR_CTRL_FIRMGO,
		YUKON_CSR_CTRL_FIFODEBUG,
		YUKON_CSR_CTRL_DIS0_D3RST,
		YUKON_CSR_CTRL_DIS1_D3RST,
		YUKON_CSR_CTRL_DIS2_D3RST
	}
};


/*******************************************************************************
Private Module Prototypes - Bus Control Basic 2 PCI (calBusCtlP46)
*******************************************************************************/
INLINE LOCAL DWORD calBusCtlP46AllocSystemResource (
    BUS_RESOURCE_T * 	Resource );

INLINE LOCAL DWORD calBusCtlP46ConfigBusController (
    struct pci_dev * 	pPciDevice );

INLINE LOCAL DWORD calBusCtlP46RegisterInterrupt (
    CDSL_ADAPTER_T	*       pCnxAdapter,
    BUS_RESOURCE_T * 	Resource );


/*******************************************************************************
 Module Functions
*******************************************************************************/

/******************************************************************************
FUNCTION NAME:
	CalBusCtlP46Init

ABSTRACT:
	Init Bus Controller module.  Allocate any necessary system resources.


RETURN:
	Pointer to Bus Controller Context or NULL_PCI_RECOURCES if failure.


DETAILS:
	This function is called from the public vector table BusControlP46 -
	element CalBusCtlP46Init.
******************************************************************************/
MODULAR BUS_RESOURCE_T * CalBusCtlP46Init (
    I_O	CDSL_ADAPTER_T	* pCnxAdapter,
    IN	CHIPAL_TUNING_T	* Tune,
    I_O	USHORT 		* MyAdapterCount )
{
	NDIS_STATUS 	Status = STATUS_FAILURE ;
	DWORD 			Results = CHIPAL_FAILURE;
	BUS_RESOURCE_T 	*pBusResources = NULL_PCI_RECOURCES;
	struct pci_dev  *pci_dev=NULL;
	DWORD			*pArmCSR;

	// *
	// * Allocate Bus Interface Context Block
	// *
	Status = ALLOCATE_MEMORY
	         (
	             (VOID*)&pBusResources,
	             sizeof(BUS_RESOURCE_T) + sizeof(BUS_CONTEXT_T),
	             MEMORY_TAG
	         );

	if (Status != STATUS_SUCCESS)
	{
		return(NULL_PCI_RECOURCES);
	}

	CLEAR_MEMORY(pBusResources, sizeof(BUS_RESOURCE_T) + sizeof(BUS_CONTEXT_T));

	pBusResources->RequiredSharedMemory = Tune->TotalSharedMemory;

	//
	// *
	// * Find the ARM device on pci bus
	// *
	pci_dev = pci_get_device( CnxtDslVendorId,
	                           CnxtDslArmDeviceId,
	                           pci_dev );

	if (pci_dev == NULL)
	{
		FREE_MEMORY
		(
		    pBusResources,
		    sizeof(BUS_RESOURCE_T),
		    CHIPAL_CONTEXT_MEM_FLAGS
		);
		return(NULL_PCI_RECOURCES);
	}
        // enable the device
	Status = pci_enable_device(pci_dev);
	if (Status != 0)
	{
		FREE_MEMORY(pBusResources, sizeof (BUS_RESOURCE_T), CHIPAL_CONTEXT_MEM_FLAGS);
		return NULL_PCI_RECOURCES;
	}

	// save the device resource info
	pBusResources->pPciARMDevice = pci_dev;
#if 0
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciARMDevice devfn=%x\n",pBusResources->pPciARMDevice->devfn));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciARMDevice vendor=%x\n",pBusResources->pPciARMDevice->vendor));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciARMDevice device=%x\n",pBusResources->pPciARMDevice->device));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciARMDevice subsystem_vendor=%x\n",pBusResources->pPciARMDevice->subsystem_vendor));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciARMDevice subsystem_device=%x\n",pBusResources->pPciARMDevice->subsystem_device));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciARMDevice class=%x\n",pBusResources->pPciARMDevice->class));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciARMDevice hdr_type=%x\n",pBusResources->pPciARMDevice->hdr_type));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciARMDevice rom_base_reg=%x\n",pBusResources->pPciARMDevice->rom_base_reg));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciARMDevice irq=%x\n",pBusResources->pPciARMDevice->irq));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciARMDevice name=%s\n",pBusResources->pPciARMDevice->name));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciARMDevice slot_name=%s\n",pBusResources->pPciARMDevice->slot_name));
#endif

	//
	// *
	// * Find the ADSL device on pci bus
	// *
	pci_dev = NULL;
	pci_dev = pci_get_device( CnxtDslVendorId,
	                           CnxtDslAdslDeviceId,
	                           pci_dev );

	if (pci_dev == NULL)
	{
		FREE_MEMORY
		(
		    pBusResources,
		    sizeof(BUS_RESOURCE_T),
		    CHIPAL_CONTEXT_MEM_FLAGS
		);
		return(NULL_PCI_RECOURCES);
	}

	Status = pci_enable_device(pci_dev);
	if (Status != 0)
	{
		FREE_MEMORY(pBusResources, sizeof (BUS_RESOURCE_T), CHIPAL_CONTEXT_MEM_FLAGS);
		return NULL_PCI_RECOURCES;
	}
	
	// save the device resource info, do not enable this is enabled in CardMgmt.c
	pBusResources->pPciADSLDevice = pci_dev;
#if 0
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciADSLDevice devfn=%x\n",pBusResources->pPciADSLDevice->devfn));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciADSLDevice vendor=%x\n",pBusResources->pPciADSLDevice->vendor));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciADSLDevice device=%x\n",pBusResources->pPciADSLDevice->device));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciADSLDevice subsystem_vendor=%x\n",pBusResources->pPciADSLDevice->subsystem_vendor));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciADSLDevice subsystem_device=%x\n",pBusResources->pPciADSLDevice->subsystem_device));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciADSLDevice class=%x\n",pBusResources->pPciADSLDevice->class));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciADSLDevice hdr_type=%x\n",pBusResources->pPciADSLDevice->hdr_type));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciADSLDevice rom_base_reg=%x\n",pBusResources->pPciADSLDevice->rom_base_reg));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciADSLDevice irq=%x\n",pBusResources->pPciADSLDevice->irq));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciADSLDevice name=%s\n",pBusResources->pPciADSLDevice->name));
	DBG_CDSL_DISPLAY(DBG_LVL_MISC_INFO, pCnxAdapter->PersistData.DebugFlag, ("pPciADSLDevice slot_name=%s\n",pBusResources->pPciADSLDevice->slot_name));
#endif

	// *
	// * Configure Bus Controllers
	// *
	Results = calBusCtlP46ConfigBusController( pBusResources->pPciARMDevice );
	if (Results != CHIPAL_SUCCESS)
	{
		FREE_MEMORY
		(
		    pBusResources,
		    sizeof(BUS_RESOURCE_T),
		    CHIPAL_CONTEXT_MEM_FLAGS
		);
		return(NULL_PCI_RECOURCES);
	}

	Results = calBusCtlP46ConfigBusController( pBusResources->pPciADSLDevice );
	if (Results != CHIPAL_SUCCESS)
	{
		FREE_MEMORY
		(
		    pBusResources,
		    sizeof(BUS_RESOURCE_T),
		    CHIPAL_CONTEXT_MEM_FLAGS
		);
		return(NULL_PCI_RECOURCES);
	}

	(*MyAdapterCount)++;
	pBusResources->InstanceNumber = *MyAdapterCount;

	// *
	// * Allocate Os Resources (Memory Ranges)
	// *
	Results = calBusCtlP46AllocSystemResource( pBusResources);
	if (Results != CHIPAL_SUCCESS)
	{
		FREE_MEMORY
		(
		    pBusResources,
		    sizeof(BUS_RESOURCE_T),
		    CHIPAL_CONTEXT_MEM_FLAGS
		);
		return(NULL_PCI_RECOURCES);
	}

	// *
	// * Register Interrupt
	// *
	Results = calBusCtlP46RegisterInterrupt(pCnxAdapter, pBusResources);
	if (Results != CHIPAL_SUCCESS)
	{
		FREE_MEMORY
		(
		    pBusResources,
		    sizeof(BUS_RESOURCE_T),
		    CHIPAL_CONTEXT_MEM_FLAGS
		);
		return(NULL_PCI_RECOURCES);
	}

	pBusResources->InterruptsInitialized = TRUE;

	// set the master enable
	CalBusCtlP46EnableInterrupt(pBusResources, ARM_FUNCTION, 0);
	CalBusCtlP46EnableInterrupt(pBusResources, DSL_FUNCTION, 0);
	
	// Put the card in an initial state.  I had to add this because
	// Linux on the 486 does not unload the driver properly. KLE
	pArmCSR = (DWORD*)pBusResources->VirtualAddressARMBAR0;	
	*pArmCSR |= 1;
	MICRO_DELAY(10000);
	*pArmCSR &= ~1;

	return(pBusResources);
}


/******************************************************************************
FUNCTION NAME:
	CalBusCtlP46Release

ABSTRACT:
	Release any resources obtained by the CalBusCtlP46Init function


RETURN:
	VOID


DETAILS:
	This function is called from the public vector table BusControlP46 -
	element Release.
******************************************************************************/
MODULAR void  CalBusCtlP46Release (
    I_O	CDSL_ADAPTER_T	* pCnxAdapter,
    I_O BUS_RESOURCE_T *	pBusResources )
{

	//release the interrupts
	if ( pBusResources->pPciARMDevice )
		free_irq( pBusResources->pPciARMDevice->irq,
		          pCnxAdapter );
	pci_disable_device(pBusResources->pPciARMDevice);
	pci_disable_device(pBusResources->pPciADSLDevice);

	//release the shared memory
	if ( pBusResources->VirtualAddressSharedMem )
		kfree( pBusResources->VirtualAddressSharedMem );

	//release the mapped virtual addresses
	if ( pBusResources->VirtualAddressBAR0 )
		iounmap(  pBusResources->VirtualAddressBAR0 );

	if ( pBusResources->VirtualAddressBAR1 )
		iounmap(  pBusResources->VirtualAddressBAR1 );

	if ( pBusResources->VirtualAddressARMBAR0 )
		iounmap(  pBusResources->VirtualAddressARMBAR0 );

	// *
	// * Release Bus Controller Context Context
	// *
	FREE_MEMORY
	(
	    pBusResources,
	    sizeof(BUS_RESOURCE_T),
	    CHIPAL_CONTEXT_MEM_FLAGS
	);
}


/******************************************************************************
FUNCTION NAME:
	CalBusCtlP46EnableInterrupt

ABSTRACT:
	Enable specified "OWNED" interrupt. If a NULL EnableInterupts 
	value is passed, all shared Interrupts will be enabled.
	When a value is specified by EnableInterrupts, it will be stored in
	the static OwnedInterrupts element.	 A value is removed from the
	OwnedInterrupts element by a call to CalBusCtlP46DisableInterrupt
	with the DisableInterrupts value set.


RETURN:
	VOID


DETAILS:
	This function is called from the public vector table BusControlP46 - 
	element Enable.
******************************************************************************/
MODULAR void CalBusCtlP46EnableInterrupt (
    I_O BUS_RESOURCE_T	* pBusResources,
    IN  DWORD             FunctSel,
    IN  DWORD		  InterruptFlags )
{
	if ( FunctSel == ARM_FUNCTION )
	{
		ARM_CTRL_S			ControlReg;
		IO_ADDRESS_T			BaseAddress;

		if(InterruptFlags == 0)
		{
			pBusResources->DisableCount = (pBusResources->DisableCount)?pBusResources->DisableCount-1:0;
			if(pBusResources->DisableCount)
			{
				// This is a nested disable. Do nothing
				return;
			}
		}
		
		BaseAddress.pByte = pBusResources->VirtualAddressARMBAR0;

		pBusResources->InterruptReg |= InterruptFlags;

		ControlReg.Value = 0l;

		if(InterruptFlags == 0)
		{
			//* Make Sure all Enabled bit were written to Hardware
			//* Read Interrupt Enable Register
			ControlReg.Value = *(DWORD *)(BaseAddress.pByte + CSR_ARM_IER);

			//* If hardware enabled interrupts are not equal to software enabled
			//* interrupts, update hardware.
			if(ControlReg.Value != pBusResources->InterruptReg)
			{
				ControlReg.Value &= ~(pBusResources->InterruptReg);
				ControlReg.Value |= pBusResources->InterruptReg;
				*(DWORD *)(BaseAddress.pByte + CSR_ARM_IER) = ControlReg.Value;
				NO_PCI_BURST(BaseAddress);
			}

			//* Enable Master Bit
			ControlReg.Value = *(DWORD *)(BaseAddress.pByte + CSR_ARM_CTRL);
			ControlReg.Value |= ArmControlRegister[CnxtDslPhysicalDriverType].CsrCtrlAie;
			*(DWORD *)(BaseAddress.pByte + CSR_ARM_CTRL) = ControlReg.Value;
			NO_PCI_BURST(BaseAddress);
		}
		else
		{
			ControlReg.Value = *(DWORD *)(BaseAddress.pByte + CSR_ARM_IER);
			ControlReg.Value &= ~(pBusResources->InterruptReg);
			ControlReg.Value |= pBusResources->InterruptReg;
			*(DWORD *)(BaseAddress.pByte + CSR_ARM_IER) =ControlReg.Value;
			NO_PCI_BURST(BaseAddress);
		}

		return;
	}
	else
	{
		DWORD 		       	* pPhysicalBase	= (DWORD *) pBusResources->VirtualAddressBAR0;
		volatile DWORD		RegValue;

		if(InterruptFlags == 0)
		{
			pBusResources->DisableCount = (pBusResources->DisableCount)?pBusResources->DisableCount-1:0;
			if(pBusResources->DisableCount)
			{
				// This is a nested disable. Do nothing
				return;
			}
		}

		// *
		// * Get a copy of the current active Interrupts.  There should be none
		// * enabled at this time.  However, If some other driver enabled some
		// * interrupts while we had them disabled, we do not want them lost.
		// *
		if (InterruptFlags)
		{
			pBusResources->OwnedInterrupts |= InterruptFlags;
			pBusResources->SharedInterrupts |= InterruptFlags;
			*GET_ADDRESS(pPhysicalBase,DSL_IER_OFFSET) = pBusResources->SharedInterrupts;
		}
		else
		{
			// *
			// * Master Interrupt Enable for Function
			// *
			RegValue = *GET_ADDRESS(pPhysicalBase,DSL_CTRL_OFFSET);
			RegValue |=  CSR_BIT_AIE;
			*GET_ADDRESS(pPhysicalBase,DSL_CTRL_OFFSET) = RegValue;
		}

		RegValue = *GET_ADDRESS(pPhysicalBase,DSL_CTRL_OFFSET);		// Prevent PCI Burst
	}
}


/******************************************************************************
FUNCTION NAME:
	CalBusCtlP46DisableInterrupt

ABSTRACT:
	Disable all or a given set of interrupts. A NULL (0) 
	DisableInterrupt value will result in interrupts being temporarily 
	disabled. Any bit set (1) will permanently disable the interrupt.

	NOTE:  CalBusCtlP46DisableInterrupt must be called before 
	CalBusCtlP46EnableInterrupt.  In the case of two device drivers,
	the shared interrupts will be read and stored in the SharedInterrupt
	variable.


RETURN:
	VOID


DETAILS:
	This function is called from the public vector table BusControlP46 - 
	element Disable.
******************************************************************************/
MODULAR void CalBusCtlP46DisableInterrupt (
    I_O BUS_RESOURCE_T	* pBusResources,
    IN  DWORD             FunctSel,
    IN  DWORD		  DisableInterrupts )
{
	DWORD           * pPhysicalBase;
	volatile DWORD	  RegValue;

	if(DisableInterrupts == 0)
	{
		++pBusResources->DisableCount;
		if(pBusResources->DisableCount > 1)
		{
			// This is a nested disable. Do nothing
			return;
		}
	}

	if ( FunctSel == DSL_FUNCTION )
	{
		pPhysicalBase	= (DWORD *) pBusResources->VirtualAddressBAR0;

		// Get a copy of the current active Interrupts
		pBusResources->OwnedInterrupts =*GET_ADDRESS(pPhysicalBase,DSL_IER_OFFSET);

		if (DisableInterrupts)
		{
			// *
			// * We are disabling only select interrupts
			// *
			pBusResources->OwnedInterrupts &= ~DisableInterrupts;

			// Disable Selected Interrupts
			*GET_ADDRESS(pPhysicalBase,DSL_IER_OFFSET) =pBusResources->OwnedInterrupts;
		}
		else
		{
			//Disable All Interrupts
			RegValue = *GET_ADDRESS(pPhysicalBase,DSL_CTRL_OFFSET);
			RegValue &= ~CSR_BIT_AIE;
			*GET_ADDRESS(pPhysicalBase,DSL_CTRL_OFFSET) = RegValue;
		}

		RegValue = *GET_ADDRESS(pPhysicalBase,DSL_CTRL_OFFSET);		// Prevent PCI Burst
	}
	else
	{
		pPhysicalBase	= (DWORD *) pBusResources->VirtualAddressARMBAR0;

		// Get a copy of the current active Interrupts
		RegValue = *GET_ADDRESS(pPhysicalBase, CSR_ARM_IER);

		if (DisableInterrupts)
		{
			// *
			// * We are disabling only select interrupts
			// *
			RegValue &= ~DisableInterrupts;

			// Disable Selected Interrupts
			*GET_ADDRESS(pPhysicalBase, CSR_ARM_IER ) = RegValue;
		}
		else
		{
			//Disable All Interrupts
			RegValue = *GET_ADDRESS(pPhysicalBase,CSR_ARM_CTRL);
			RegValue &= ~(ArmControlRegister[CnxtDslPhysicalDriverType].CsrCtrlAie);
			*GET_ADDRESS(pPhysicalBase,CSR_ARM_CTRL) = RegValue;
		}

		RegValue = *GET_ADDRESS(pPhysicalBase,CSR_ARM_CTRL);		// Prevent PCI Burst
	}
}


/******************************************************************************
FUNCTION NAME:
	CalBusCtlP46GetInterruptStatus

ABSTRACT:
	Return any Asserted Owned Interrupts.


RETURN:
	DWORD Value indicating which Interrupt(s) were asserted. The bit field
	matches interrupts enabled by the bit field parameter "EnableInterrupts"
	of the CalBusCtlP46EnableInterrupt function call.


DETAILS:
	This function is called from the public vector table BusControlP46 - 
	element GetStatus.
******************************************************************************/
MODULAR VOID CalBusCtlP46GetInterruptStatus (
    IN BUS_RESOURCE_T	* pBusResources,
    IN DWORD			  FunctSel,
    OUT DWORD           * pInterrupts )
{
	DWORD * pPhysicalBase;

	if ( (FunctSel == DSL_FUNCTION) && pBusResources->VirtualAddressBAR0 )
	{
		pPhysicalBase	= (DWORD *) pBusResources->VirtualAddressBAR0;

		*pInterrupts = *GET_ADDRESS(pPhysicalBase, DSL_IER_OFFSET)
		               & *GET_ADDRESS(pPhysicalBase,  DSL_ISR_OFFSET)
		               & MAX_INTERRUPT_BITS;

		return;
	}
	else if ( ( FunctSel == ARM_FUNCTION ) && pBusResources->VirtualAddressARMBAR0 )
	{
		pPhysicalBase	= (DWORD *) pBusResources->VirtualAddressARMBAR0;

		*pInterrupts = ( *GET_ADDRESS( pPhysicalBase, CSR_ARM_IER )
		                 & *GET_ADDRESS(pPhysicalBase, CSR_ARM_ISR  )
		                 & MAX_INTERRUPT_BITS);

		return;
	}

	*pInterrupts = 0;
}


/******************************************************************************
FUNCTION NAME:
	CalBusCtlP46ClearInterruptStatus

ABSTRACT:
	Clear any Asserted Owned Interrupts.


RETURN:
	VOID

DETAILS:
	This function is called from the public vector table BusControlP46 - 
	element ClearStatus.
******************************************************************************/
MODULAR void CalBusCtlP46ClearInterruptStatus (
    I_O BUS_RESOURCE_T	* pBusResources,
    IN  ULONG		  FunctSel,
    IN  DWORD		  ClearInterrupts )
{
	DWORD 			* pPhysicalBase;
	DWORD 			CsrOffSet;
	volatile DWORD	        NoBurst;

	if ( FunctSel == DSL_FUNCTION )
	{
		pPhysicalBase	= (DWORD *) pBusResources->VirtualAddressBAR0;
		CsrOffSet = DSL_ISR_OFFSET;
		*GET_ADDRESS(pPhysicalBase, CsrOffSet) = ClearInterrupts & pBusResources->OwnedInterrupts;
		NoBurst = *GET_ADDRESS(pPhysicalBase, CsrOffSet);
	}
	else
	{
		pPhysicalBase	= (DWORD *) pBusResources->VirtualAddressARMBAR0;
		CsrOffSet = CSR_ARM_ISR;
		*GET_ADDRESS(pPhysicalBase, CsrOffSet) = ClearInterrupts;
	}
}


/******************************************************************************
FUNCTION NAME:
	CalBusCtlP46GetErrorCount

ABSTRACT:
	Get and Clear Errors for a particular device/channel


RETURN:
	Number of Errors for a device/channel

DETAILS:
	Errors are read and cleared. The Basic2 uses a bitmask to track
	errors.  Therefore, we only know if a single error occured. This
	implementation returns a max count of ONE error.

	The Basic2 device only counts errors for the DMA channels 0-13.
	All other CalBusCtlP46GetErrorCount requests are 
	reported as 0 (no errors). 

	This function is called from the public vector table BusControlP46 - 
	element GetError.
******************************************************************************/
MODULAR DWORD CalBusCtlP46GetErrorCount (
    I_O BUS_RESOURCE_T	* pBusResources,
    IN  DWORD			FunctSel )
{
	return(0);
}


/*******************************************************************************
 Private Functions
*******************************************************************************/




/******************************************************************************
FUNCTION NAME:
	calBusCtlP46ConfigBusController

ABSTRACT:
	Configure the bus controller device with proper operation (Bus Master
	and Memory Space)


RETURN:
	ChipAL Fail Code


DETAILS:
******************************************************************************/
INLINE LOCAL DWORD calBusCtlP46ConfigBusController (
    IN struct pci_dev* pPciDevice
)
{
	unsigned short command;
	int error;

	// read the pci configuration flags
	error = pci_read_config_word( pPciDevice,
	                              PCI_COMMAND,
	                              &command );
	if ( error )
	{
		//	Log error message and exit.
		NdisWriteErrorLogEntry(
		    MiniportAdapterHandle,
		    NDIS_ERROR_CODE_OUT_OF_RESOURCES,
		    5,
		    BUS_ERROR_FATAL(CHIPAL_RESOURCE_ALLOCATION_FAILURE),
		    Resource->SlotNumber.u.AsULONG,
		    __FILE__,
		    __LINE__,
		    RSS_INIT_ERROR_READING_PCICONFIG0_VERIFY0_1 );

		DBG_CDSL_DISPLAY(DBG_LVL_GENERIC, DBG_FLAG_DONT_CARE, ("CnxATM Config word read failure"));
		return -EDEVICEFAIL;
	}

	// make sure the config space has bus master and memory space enabled
	if ( !(command & PCI_COMMAND_MEMORY ) ||
	        !(command & PCI_COMMAND_MASTER) ||
	        !(command & PCI_COMMAND_SERR)  )
	{
		command |= PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER | PCI_COMMAND_SERR;

		error = pci_write_config_word( pPciDevice,
		                               PCI_COMMAND,
		                               command );

		if ( error )
		{
			//	Log error message and exit.
			NdisWriteErrorLogEntry(
			    MiniportAdapterHandle,
			    NDIS_ERROR_CODE_OUT_OF_RESOURCES,
			    5,
			    BUS_ERROR_FATAL(CHIPAL_RESOURCE_ALLOCATION_FAILURE) ,
			    Resource->SlotNumber.u.AsULONG,
			    __FILE__,
			    __LINE__,
			    RSS_INIT_ERROR_WRITING_PCICONFIG0 );

			DBG_CDSL_DISPLAY(DBG_LVL_GENERIC, DBG_FLAG_DONT_CARE, ("CnxATM Config word read failure"));
			return -EDEVICEFAIL;
		}
	}

	return(CHIPAL_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	calBusCtlP46AllocSystemResource

ABSTRACT:
	Allocated Operating System Resources (Memory, etc...)



RETURN:
	ChipAL Fail Code


DETAILS:
******************************************************************************/
INLINE LOCAL DWORD calBusCtlP46AllocSystemResource (
    I_O BUS_RESOURCE_T * 	Resource )
{
	// set the address space sizes
	Resource->LenBar0  = ADSL_CSR_SIZE;
	Resource->LenBar1 = ADSL_FUNC_SIZE;
	Resource->LenARMBar0  = ARM_CSR_SIZE;

	// map the addresses into virtual to use them
	Resource->VirtualAddressBAR0 = ioremap(
		Resource->pPciADSLDevice->resource[0].start,
		Resource->LenBar0);
	Resource->VirtualAddressBAR1 = ioremap(
		Resource->pPciADSLDevice->resource[1].start,
		Resource->LenBar1);
	Resource->VirtualAddressARMBAR0 = ioremap(
		Resource->pPciARMDevice->resource[0].start,
		Resource->LenARMBar0);

	// set the vendor ids
	Resource->PciFunctionZero.VendorId = Resource->pPciADSLDevice->vendor;
	Resource->PciFunctionZero.DeviceId = Resource->pPciADSLDevice->device;
	Resource->PciFunctionOne.VendorId = Resource->pPciARMDevice->vendor;
	Resource->PciFunctionOne.DeviceId = Resource->pPciARMDevice->device;

	// allocate contiguous memory for DMA
	Resource->VirtualAddressSharedMem = kmalloc( Resource->RequiredSharedMemory,
	                                    GFP_KERNEL | GFP_DMA );
	if ( !Resource->VirtualAddressSharedMem )
	{
		DBG_CDSL_DISPLAY(DBG_LVL_GENERIC, DBG_FLAG_DONT_CARE, ("CnxADSL Shared memory allocation failure"));
		return -ENOMEM;
	}

	Resource->PhysicalAddressSharedMem = (char *)virt_to_bus( Resource->VirtualAddressSharedMem );

	CLEAR_MEMORY( Resource->VirtualAddressSharedMem, Resource->RequiredSharedMemory );

	return(CHIPAL_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	calBusCtlP46RegisterInterrupt

ABSTRACT:
	Map ISR and ISR DPC routines to a bus relative vector and level


RETURN:
	ChipAL Fail Code


DETAILS:
******************************************************************************/
INLINE LOCAL DWORD calBusCtlP46RegisterInterrupt(IN CDSL_ADAPTER_T *pCnxAdapter, I_O BUS_RESOURCE_T *Resource)
{
	DWORD retval;
	int err;
#ifdef PIM_DEBUG
	printk(KERN_ALERT "CnxADSL: calBusCtlP46RegisterInterrupt\n");
#endif

	Resource->PriIntRegistered = FALSE;

	/* Make sure Interrupts are not enabled */
	CalBusCtlP46DisableInterrupt(Resource, ARM_FUNCTION, AVAILABLE_INTERRUPTS);
	CalBusCtlP46DisableInterrupt(Resource, DSL_FUNCTION, AVAILABLE_INTERRUPTS);

	/* initialize the interrupt task queue structure */
	tasklet_init(&pCnxAdapter->DSLInterruptQueStr, (void (*)(unsigned long))ChipALDSLIsrDPCHandler, (unsigned long) pCnxAdapter);
	tasklet_init(&pCnxAdapter->ARMInterruptQueStr, (void (*)(unsigned long))ChipALARMIsrDPCHandler, (unsigned long) pCnxAdapter);

	/* request the interrupt - the ARM and ADSL functions both use the same interrupt */
	err = request_irq(Resource->pPciARMDevice->irq, ChipALIsrHandler, IRQF_SHARED, CNX_DEV_LABEL, pCnxAdapter);
	if (err) {
		printk(KERN_ERR "CnxADSL: calBusCtlP46RegisterInterrupt failed to register interrupt (error %d)\n", err);
		retval = -ENOINTERRUPT;
	} else {
		Resource->PriIntRegistered = TRUE;
		retval = CHIPAL_SUCCESS;
	}

	return retval;
}

/******************************************************************************
FUNCTION NAME:
	CalBusCtlP46GetVersion

ABSTRACT:
	Get Version Number of Bus Interface Device


RETURN:
	Revision from Pci Configuration Space - Offset 0x08, 1 byte


DETAILS:
******************************************************************************/
GLOBAL DWORD CalBusCtlP46GetVersion (
    IN BUS_RESOURCE_T	* pBusResources,
    IN CDSL_ADAPTER_T	* pThisAdapter)
{
	NOT_USED(pThisAdapter);

	return(pBusResources->PciFunctionZero.RevisionId);

}


/******************************************************************************
FUNCTION NAME:
	CalBusCtlP46GetConfig

ABSTRACT:
	Get Public Configuration Data


RETURN:
	ChipAL Fail Code:
	
		CHIPAL_SUCCESS - Current Config Data returned in pConfig structure
		CHIPAL_MODULE_NOT_INITIALIZED - ChipAl is not Initialized

DETAILS:
******************************************************************************/
MODULAR CHIPAL_FAILCODE_T CalBusCtlP46GetConfig(
    IN	 CDSL_ADAPTER_T 		* CdslAdapter,
    CAL_CONFIG_PARAMS_T			* pConfig)
{
	BUS_RESOURCE_T	* pBusResources;
	CHIPAL_T 		* pThisInterface;

	CHIPAL_ASSERT_TRACE
	(
	    "CalBusCtlP46GetConfig() ",
	    (CdslAdapter != (CDSL_ADAPTER_T *)0
	     && pConfig != NULL_CAL_CONFIG_PARAMS)
	)

	pThisInterface = (CHIPAL_T *) CdslAdapter->ChipMod;
	if (pThisInterface == NULL_CHIPAL_T )
	{
		return(BUS_ERROR_FATAL(CHIPAL_MODULE_NOT_INITIALIZED));
	}
	pBusResources =pThisInterface->BusResources;

	// *
	// * Copy Configuration to User Space
	// *
	pConfig->BusConfig.BAR0 = pBusResources->VirtualAddressBAR0;
	pConfig->BusConfig.BAR1 = pBusResources->VirtualAddressBAR1;
	pConfig->BusConfig.VendorId = pBusResources->PciFunctionZero.VendorId;
	pConfig->BusConfig.DeviceId = pBusResources->PciFunctionZero.DeviceId;
	pConfig->BusConfig.SubVendorId = pBusResources->PciFunctionZero.SubVendorId;
	pConfig->BusConfig.SubSystemId = pBusResources->PciFunctionZero.SubSystemId;

	return(CHIPAL_SUCCESS);

}


/******************************************************************************
FUNCTION NAME:
	CalBusCtlP46SetConfig

ABSTRACT:
	Set Public Configuration Data


RETURN:
	ChipAL Fail Code:
	
		CHIPAL_SERVICE_UNAVAILABLE - Not Yet Implemented

DETAILS:
******************************************************************************/
MODULAR CHIPAL_FAILCODE_T CalBusCtlP46SetConfig(
    IN	 CDSL_ADAPTER_T 		* CdslAdapter,
    CAL_CONFIG_PARAMS_T			* pConfig)
{
	NOT_USED(CdslAdapter);
	NOT_USED(pConfig);

	return(BUS_ERROR_WARNING(CHIPAL_SERVICE_UNAVAILABLE));
}
