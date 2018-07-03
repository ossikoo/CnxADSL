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
**		CardAL (Card Abstraction Layer)
**
**	FILE NAME:
**		CardALV.h
**
**	ABSTRACT:
**		This file implements the top level routines that interface to the
**		Linux ATM layer.
**
**	DETAILS:
**		????
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/CardMgmt.c $
**	$Revision: 23 $
**	$Date: 7/27/01 8:46a $
*******************************************************************************
******************************************************************************/

#define has_tty 0 /*CLN*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/proc_fs.h>
#include "LnxTools.h"
#include "../CommonData.h"
#include "Version.h"
#include "CnxADSL.h"
#include "CnxTTY.h"
#include "ARMAbstract.h"
#include "CardMgmtVcV.h"
#include "CardMgmtLinkV.h"
#include "CardMgmtV.h"
#include "CardMgmtCtl.h"
#include "SmSysIf.h"
#include "SmSysIfv.h"
#include "CardALV.h"
#include "KThread.h"
#include <linux/sched.h>
#include <asm/unistd.h>
#include <linux/signal.h>
#include <linux/kthread.h>

/******************************************************************************
**	defines for this function
******************************************************************************/
#define ERROR_FATAL(error)	(LEVEL_FATAL   | MODULE_CARDMGMT | INSTANCE_CARDMGMT | error)
#define ERROR_WARNING(error)	(LEVEL_WARNING | MODULE_CARDMGMT | INSTANCE_CARDMGMT | error)
#define ERROR_NORMAL(error)	(LEVEL_NORMAL  | MODULE_CARDMGMT | INSTANCE_CARDMGMT | error)

#define BD_HEADER_SIZE		(sizeof(MINI_BACK_DOOR_T) - sizeof(MINI_BD_STRUC_UNION_T))

#define ONE_SECOND		1000

CDSL_ADAPTER_T* gpAdapterList;

int CnxAdslReadProcDebugQuery(
	char		* ProcPage,
	char		** ProcPageStart,
	off_t		ProcPageOffset,
	int		ProcPageLen,
	int		* bEndOfFile,
	void		* ProcData);
const char CnxAdslProcName[] = "CnxAdslDebugQuery";
struct proc_dir_entry		* pCnxAdslProcEntry;


/******************************************************************************
**	External global variables...
******************************************************************************/
DWORD gAllocCnt;

#if MEM_ALLOC_DBG
ALLOC_HEAD_T gAllocList;
#endif

/******************************************************************************
**	Internal global variables...
******************************************************************************/
BOOLEAN						AdapterFullyInitialized;
BOOLEAN						AdapterStructureAllocated;

/* the variables that contains the thread data */
struct task_struct *		BGThread;
struct task_struct *		SIThread;
struct task_struct *		TMThread;

///////////////////////////////////////////////////////////////////////////////
// Local functions
///////////////////////////////////////////////////////////////////////////////
NDIS_STATUS CmgmtAdapterReset(IN CDSL_ADAPTER_T	*pThisAdapter);
VOID CmgmtMainTimerInterrupt(IN ULONG pThisAdapter);

/******************************************************************************
**	This is the master vector table for all module generic functions.
******************************************************************************/

static MODULE_FUNCTIONS_T DownloadModuleMethods[] =
    {
        CHIPAL_FNS,		    // Chip Abstraction Layer Module
        ARMAL_FNS			// ARM Abstraction module
    };

static MODULE_FUNCTIONS_T ModemModuleMethods[] =
    {
        CARDAL_FNS,	//Card Abstraction Module
        BUFMGMT_FNS,	//Buffer Management Module
        FRAMEAL_FNS,	//Frame Abstraction Module
        CELLTEST_FNS    //Cell data test Module
    };

// Structure of function pointers used by the Linux ATM layer
static const struct atmdev_ops DeviceOperations = {
	.dev_close  = CmgmtDeviceClose,
	.open       = CmgmtOpen,
	.close      = CmgmtClose,
	.ioctl      = CmgmtProcIOCTL,
	.getsockopt = CmgmtGetSocketOption,
	.setsockopt = CmgmtSetSocketOption,
	.send       = CmgmtSendBuffer,
	.proc_read  = CmgmtProcessRead,
};

static const struct atmphy_ops PhyOperations = {
	.start     = CmgmtPhyStart,
	.ioctl     = CmgmtPhyIOCTL,
	.interrupt = CmgmtPhyInterrupt,
};

//////////////////////////////////////////////////////////////////////
// Global Variables
//////////////////////////////////////////////////////////////////////

 // These variables are expected to be
 // set by the "insmod" command (when
 // the module is installed by the kernel).
int			CnxtDslVendorId = 0x14F1;
int			CnxtDslArmDeviceId = 0x1610;
int			CnxtDslAdslDeviceId = 0x1611;
int			CnxtDslPhysicalDriverType = 1;

#ifdef MODULE /*CLN!!!*/
//EXPORT_NO_SYMBOLS;

MODULE_AUTHOR("Conexant Systems Inc");
MODULE_DESCRIPTION("Conexant ADSL ATM driver");
MODULE_LICENSE("Proprietary");

module_param(CnxtDslVendorId, int, 0);
module_param(CnxtDslArmDeviceId, int, 0);
module_param(CnxtDslAdslDeviceId, int, 0);
module_param(CnxtDslPhysicalDriverType, int, 0);

/*++
    init_module

Routine Description:

    This is the initial entry point for the Tigris ADSL driver.  It is
    called once when the module is loaded. It is responsible for 
    finding all of the devices and registering as an ATM network driver.

Arguments:

    None

Return Value:

    The status of the operation.

--*/
int CnxtDsl_init(void)
 {
 int Status;
 struct pci_dev* pci_dev=NULL;
 CDSL_ADAPTER_T* pAdapter;

#ifdef PIM_DEBUG
printk(KERN_ALERT "CnxADSL: CnxtDsl_init Starting\n");
#endif

 gpAdapterList = NULL;
 gAllocCnt = 0;

 if (CnxtDslVendorId == NO_VENDOR_ID)
  return -ENODEV;
 if (CnxtDslArmDeviceId == NO_DEVICE_ID)
  return -ENODEV;
 if (CnxtDslAdslDeviceId == NO_DEVICE_ID)
  return -ENODEV;
 switch (CnxtDslPhysicalDriverType)
   {
   case DRIVER_TYPE_TIGRIS:
    break;
   case DRIVER_TYPE_YUKON:
    break;
   default:
#ifdef PIM_DEBUG
printk(KERN_ALERT "CnxADSL: CnxtDsl_init The CnxtDslPhysicalDriverType variable not set during driver load/installation!\n");
#endif
    break;
   }
 
 // find a device, break if none found
 // I'm just verifying that we have a device here, the
 // bus controller finds it later and saves the pointer
 // This device is enabled in ChipALBusCtlP46.c
 pci_dev=pci_get_device(CnxtDslVendorId, CnxtDslArmDeviceId, pci_dev);
 if (!pci_dev)
   {
#ifdef PIM_DEBUG
printk(KERN_ALERT "CnxADSL: CnxtDsl_init Failed to find PCI ARM device!\n");
#endif
   return -ENODEV;
   }

 // make sure both functions are there
 pci_dev = pci_get_device(CnxtDslVendorId, CnxtDslAdslDeviceId, pci_dev);
 if (!pci_dev)
  {
#ifdef PIM_DEBUG
printk(KERN_ALERT "CnxADSL: CnxtDsl_init Failed to find PCI ADSL device!\n");
#endif
  return -ENODEV;
  }

 // enable the device
 Status = pci_enable_device(pci_dev);
 if (Status != 0)
  return Status;

 // allocate an empty device structure
 Status=ALLOCATE_MEMORY ((PVOID)&pAdapter, sizeof(CDSL_ADAPTER_T), 0);
 if (Status!=STATUS_SUCCESS)
  return -ENOMEM;
 Status =ALLOCATE_MEMORY ((PVOID)&pAdapter->dbg_fops, sizeof(struct file_operations), 0);
 if (Status!=STATUS_SUCCESS)
  return -ENOMEM;
 AdapterStructureAllocated=TRUE;

 // init the structure
 memset(pAdapter, 0, sizeof( CDSL_ADAPTER_T ));

 // initialize the new device structure
 pAdapter->MainTimerState=TMR_UNINITIALIZED;
 pAdapter->shutdown=FALSE;
 pAdapter->TimerResolution=DEFAULT_TIMER_RESOLUTION;
 pAdapter->Showtime=FALSE;
	
 /******************************************************************************
  **	Load the modules methods table...
  ******************************************************************************/
 // register as an ATM device
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
 pAdapter->pAtmDevice=atm_dev_register(CNX_DEV_LABEL, &DeviceOperations, -1, NULL);
#else
 pAdapter->pAtmDevice=atm_dev_register(CNX_DEV_LABEL, &pci_dev->dev, &DeviceOperations, -1, NULL);
#endif
 pAdapter->pAtmDevice->dev_data=pAdapter;

 // initialize the driver fields of the atm device structure
 pAdapter->pAtmDevice->ci_range.vpi_bits=10;
 pAdapter->pAtmDevice->ci_range.vci_bits=16;
 pAdapter->pAtmDevice->link_rate=ATM_OC3_PCR;

 // insert my phy function pointers
 pAdapter->pAtmDevice->phy=&PhyOperations;

 // save the device structure
 gpAdapterList=pAdapter;
 
 // Init the tty interface
 if (has_tty)
  cnxtty_init(pAdapter);

 
 // pAdapter->dbg_fops->read = CnxAdslReadProcDebugQuery;
 //pCnxAdslProcEntry = create_proc_read_entry( CnxAdslProcName, (S_IFREG | S_IRUGO), NULL, CnxAdslReadProcDebugQuery, NULL );
 // pCnxAdslProcEntry = proc_create( CnxAdslProcName, (S_IFREG | S_IRUGO), NULL, pAdapter->dbg_fops); 
 return 0;
 }


/*++
    cleanup_module

Routine Description:

    This is the exit entry point for the Tigris ADSL driver.  It is
    called once when the module is unloaded. It is responsible for
    terminating any open transfers and cleaning up any resources allocated.

Arguments:

    None

Return Value:

    None

--*/
void CnxtDsl_exit(void)
 {
 MODULE_FUNCTIONS_T	*pContext;
 int			cnt;
 int			Status;
 CDSL_LINK_T		*pThisLink;
 BOOLEAN			TimerCancelled;
 CDSL_ADAPTER_T		*pThisAdapter;

#ifdef PIM_DEBUG
printk(KERN_ALERT "CnxADSL: CnxtDsl_exit Ending\n");
#endif
	DBG_CDSL_DISPLAY(/*CLN!!!*/
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("CnxADSL=>CardMgmt:cleanup_module (entry point)"));

	if (pCnxAdslProcEntry)
//	 remove_proc_entry( CnxAdslProcName, pCnxAdslProcEntry ); // PIM Dunno why I did this, someone asked me too!
	 remove_proc_entry( CnxAdslProcName, NULL );

	pThisAdapter = gpAdapterList;
	gpAdapterList = NULL;

///	// Note we are going down and then sync with threads
///	// Note: We don't currently sync but pause which should guarantee they have
///	// returned back to user space.
	pThisAdapter->shutdown = TRUE;
	SLEEP( ONE_SECOND );

	if ( AdapterFullyInitialized )
	{
		AdapterFullyInitialized = FALSE;

		kthread_stop(BGThread);
		kthread_stop(SIThread);
		kthread_stop(TMThread);

		ChipALDisableInterruptHandler (	pThisAdapter );

		// close the message handler
		SmSysIfShutdown( pThisAdapter );

		// Shutdown the link
		pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if ( (pThisLink != NULL)  &&  (pThisLink->Allocated == TRUE) )
		{
			UtilTmrClearAllTmrs( &(pThisAdapter->TimerBlock), pThisLink );
			CMVCShutdownAllVcs(pThisAdapter, pThisLink);
			CMLCallAllModuleLinkShutdowns( pThisLink );
		}


		// Make sure SystemResources were previously allocated
		if (pThisAdapter->AdapterInitComplete)
		{
			//	Cancel 'MainTimer' events associated with this adapter.
			CANCEL_TIMER( &(pThisAdapter->MainTimer), &TimerCancelled );
			if ( TimerCancelled == TRUE )
			{
				pThisAdapter->MainTimerState = TMR_CANCELLED;
			}
			else
			{
				if (	(pThisAdapter->MainTimerState == TMR_TIMING)
				        ||	(pThisAdapter->MainTimerState == TMR_EXECUTING) )
				{
					pThisAdapter->MainTimerState = TMR_UNLOADING;

					TimerCancelled = WAIT_EVENT(	&pThisAdapter->MainTimerStoppedEvent,
					                             TIMER_RESOLUTION_5_SEC );
					if ( TimerCancelled == TRUE )
					{
						//pThisAdapter->MainTimerState = TMR_CANCELLED;
					}
					else
					{
						//	Timer *NOT* Cancelled!!
						DBG_CDSL_DISPLAY(
							DBG_LVL_GENERIC,
							DBG_FLAG_DONT_CARE,
							("CnxADSL=>CardMgmt:cleanup_module  MainTimer NOT Cancelled (1)!"));
					}
				}
			}
		}
	}

	/******************************************************************************
	**	Call each modules Shutdown function to free memory and prepare driver for
	**	unloading.
	******************************************************************************/
	if ( pThisAdapter->AdapterInitComplete  )
	{
		for(cnt = 0;
		        cnt < sizeof(ModemModuleMethods)/sizeof(MODULE_FUNCTIONS_T);
		        cnt++)
		{
			pContext = &ModemModuleMethods[cnt];
			if(pContext->Shutdown != NULL)
			{
				Status = (pContext->Shutdown)(pThisAdapter);
				if(Status != STATUS_SUCCESS)
				{
					DBG_CDSL_DISPLAY(
						DBG_LVL_GENERIC,
						DBG_FLAG_DONT_CARE,
						(	"CnxADSL=>CardMgmt:cleanup_module  Status %i returned from the %s, Adapter shutdown function",
							Status,
							ModemModuleMethods[cnt].Description));
				}
			}
		}
	}

	if (pThisAdapter->DownloadInitComplete )
	{
		for(cnt = 0;
		        cnt < sizeof(DownloadModuleMethods)/sizeof(MODULE_FUNCTIONS_T);
		        cnt++)
		{
			pContext = &DownloadModuleMethods[cnt];

			if(pContext->Shutdown != NULL)
			{
				Status = (pContext->Shutdown)(pThisAdapter);
				if(Status != STATUS_SUCCESS)
				{
					DBG_CDSL_DISPLAY(
						DBG_LVL_GENERIC,
						DBG_FLAG_DONT_CARE,
						(	"CnxADSL=>CardMgmt:cleanup_module  Status %i returned from the %s, AdapterInit function",
							Status,
							DownloadModuleMethods[cnt].Description));
				}
			}
		}
	}

	// de-register the ATM device
	atm_dev_deregister(	pThisAdapter->pAtmDevice );

	// release the memory
	FREE_MEMORY (pThisAdapter,0,0);

	// check for mem leaks
#ifdef DBG
	if ( gAllocCnt )
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("CnxADSL=>CardMgmt:cleanup_module driver had %ld memory leaks",gAllocCnt));
#endif

#if MEM_ALLOC_DBG
	while ( !IsListEmpty(&gAllocList.List) )
	{
		PALLOC_HEAD_T pList;

		pList = (PALLOC_HEAD_T)RemoveHeadList(&gAllocList.List);

		if ( !pList || pList->Magic != ALLOC_MAGIC )
		{
			DBG_CDSL_DISPLAY(
				DBG_LVL_GENERIC,
				DBG_FLAG_DONT_CARE,
				("CnxADSL=>CardMgmt:cleanup_module Corrupt memory allocation list"));
			break;
		}

		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("CnxADSL=>CardMgmt:cleanup_module  Memory leak ptr 0x%08lx  tag %4s",(DWORD)pList+sizeof(ALLOC_HEAD_T),(char*)&pList->Tag));
	}
#endif

	//
	// cleanup the tty interface
	if (has_tty)
	  tty_unregister_driver(&cnxtty_driver);
}

module_init(CnxtDsl_init);
module_exit(CnxtDsl_exit);

#else

/********** monolithic entry **********/

int __init cnx_detect (void) {
  int devs = 0;

  return devs;
}

#endif

/*++
    CmgmtDeviceClose

Routine Description:

    This function is called when the ATM device is closed.  It
	performs the adapter halt operation.

Arguments:

    pDev - identifies the device to close

Return Value:

    None

--*/
void CmgmtDeviceClose(IN struct atm_dev* pDev)
 {
 DBG_CDSL_DISPLAY(DBG_LVL_GENERIC, DBG_FLAG_DONT_CARE, ("CnxADSL=>CardMgmt:CmgmtDeviceClose"));
#ifdef PIM_DEBUG
printk(KERN_ALERT "CnxADSL: CmgmtDeviceClose\n");
#endif
 }


/*++
    CmgmtOpen

Routine Description:

    This function is called to open a new virtual connection.  It should
    validate the connection parameters and then prepare the connection to 
    send and receive data.

Arguments:

    pVcc - linked list of defined connections

Return Value:

    The status of the operation.

--*/
int CmgmtOpen(IN struct atm_vcc* pVcc)
 {
 int Status = STATUS_SUCCESS;
 CDSL_VC_T * pVCStr;
 CDSL_ADAPTER_T* pAdapter = (CDSL_ADAPTER_T*)pVcc->dev->dev_data;
 short vpi;
 int vci;
 
#ifdef PIM_DEBUG
printk(KERN_ALERT "CnxADSL: CmgmtOpen opening\n");
#endif

 vpi=pVcc->vpi;
 vci=pVcc->vci;

// if the device is not connected then fail the request
// if we do this then you won't be able to bring up the connection
// at boot time, but the user will have to do it manually later	
//	if ( pAdapter->MediaConnectStatus != MediaConnected )
//		return -ENOTREADY;
 if (!AdapterFullyInitialized)
  return -ENOTREADY;

// if this VC is partially open then close it to try again
 if (test_bit(ATM_VF_PARTIAL,&pVcc->flags) && pVcc->dev_data)
  CMVCReleaseVc( (CDSL_VC_T*)pVcc->dev_data );
 
 pVcc->dev_data = NULL;
 
 if (vci == ATM_VPI_UNSPEC || vpi == ATM_VCI_UNSPEC)
  return -EINVAL;

// set the inuse flag for this connection
 set_bit(ATM_VF_ADDR, &pVcc->flags);

// only AAL5 is currently supported
 if (pVcc->qos.aal != ATM_AAL5)
  return -EINVAL;

 pVCStr=CMVCAllocateVc(pAdapter,
		       GET_LINK_FROM_ADAPTER( pAdapter ),
		       vpi,
		       vci,
		       pVcc);

 // save the device specific pointer
 pVcc->dev_data=pVCStr;

 // indicate that the vc is ready for use
 set_bit(ATM_VF_READY,&pVcc->flags);

 // increment the use count so the module is not unloaded
 // with the vc open
 //MOD_INC_USE_COUNT;

 return Status;
 }


/*++
    CmgmtClose

Routine Description:

    This function is called to close a virtual connection.  It should
    terminate any active transfers and release any resources allocated
    to the connection.

Arguments:

    pVcc - identifies the virtual connection to close

Return Value:

    None

--*/
void CmgmtClose(IN struct atm_vcc* pVcc)
 {
#ifdef PIM_DEBUG
printk(KERN_ALERT "CnxADSL: CmgmtClose Closing\n");
#endif

	// if this vc is not open then Linux is confused,
	// so just ignore it.
	if (!pVcc->dev_data)
	 return;

	// indicate that the VC is not available for use
	clear_bit(ATM_VF_READY,&pVcc->flags);

	// call the vc module to deactivate the vc
	CMVCReleaseVc( (CDSL_VC_T*)pVcc->dev_data );

	// clear the pointer to the device specific data
	pVcc->dev_data = NULL;

	// clear the assignment bit
	clear_bit(ATM_VF_ADDR,&pVcc->flags);

	// decrement the use count so the module can be closed
	//MOD_DEC_USE_COUNT;
}


/*++
    CmgmtProcIOCTL

Routine Description:

    This is the command entry point for the Tigris ADSL driver.  It
    is called whenever an application issues an IOCTL request to the
    driver.  This driver uses it to download the link parameters and
    the device firmware.

Arguments:

    pDev - identifies the device for the command
    Command - command code
    pArgument - send or receive data associated with the command

Return Value:

    The status of the operation.

--*/
int CmgmtProcIOCTL(IN struct atm_dev* pDev, IN UINT Command, INOUT PVOID pArgument)
 {
 int Status=STATUS_SUCCESS;
 int cnt;
 CDSL_ADAPTER_T* pAdapter=(CDSL_ADAPTER_T*)pDev->dev_data;

// if shutting down blow it off
 if (pAdapter->shutdown)
   // Note this accurrs normally as the user space threads have no way of
   // knowing of the shutdown other than sending an IOCTL and it failing
   // They should quit after the first failure.
  Status = ERR_SHUTDOWN ; 

 // if the IO command is a load record type then
 // send it to the ARM abstraction module
 else if ( (Command==TIG_GET_PRINT_BUFF) || (Command==TIG_GET_DEBUG_DATA) )
  {
  Status=ARMALIoctl(pAdapter, Command, pArgument);
  if (Status != STATUS_SUCCESS)
   {
   printk(KERN_ALERT "CnxADSL: CmgmtProcIOCTL ARM command failed, command=%d, Status=0x%x\n", Command, Status);
   return Status;
   }
  }
 else if ( ((Command==TIG_LOAD_START) || (Command==TIG_LOAD_RECORD)|| (Command == TIG_LOAD_DONE)) &&
	   !AdapterFullyInitialized)
  {
  Status=ARMALIoctl(pAdapter, Command, pArgument);
  if (Status != STATUS_SUCCESS)
   {
   printk(KERN_ALERT "CnxADSL: CmgmtProcIOCTL ARM Load command failed, command=%d, Status=0x%x\n", Command, Status);
   return Status;
   }
  }
 else if ( (Command == TIG_START_DSL) && !AdapterFullyInitialized )
  {
  // init the message system
  SmSysIfInit(pAdapter);

  // tell the chip al module to initialize services
  Status = ChipALInitServices( pAdapter );
  for(cnt=0; cnt < (sizeof(ModemModuleMethods)/sizeof(MODULE_FUNCTIONS_T)); cnt++)
   {
   if (ModemModuleMethods[cnt].AdapterInit != NULL)
    {
    Status=ModemModuleMethods[cnt].AdapterInit(pAdapter);
    if ( Status != STATUS_SUCCESS )
     {
     printk(KERN_ALERT "CnxADSL: CmgmtProcIOCTL Module init fail %d 0x%x\n", cnt, Status);
     return Status;
     }
    }
   }
  
		INIT_TIMER(
		    &pAdapter->MainTimer,
		    CmgmtMainTimerInterrupt,
		    (DWORD)pAdapter );

		pAdapter->MainTimerState = TMR_INITIALIZED;

		INIT_SPIN_LOCK( &pAdapter->Lock );
		pAdapter->AdapterInitComplete = TRUE;

		Status = CmgmtAdapterReset( pAdapter );
		if ( Status != STATUS_SUCCESS )
		{
			DBG_CDSL_DISPLAY(
				DBG_LVL_GENERIC,
				DBG_FLAG_DONT_CARE,
				("CnxADSL=>CardMgmt:CmgmtProcIOCTL  function CmgmtAdapterReset failed Status=0x%x",Status));
			return STATUS_FAILURE;
		}

		//	start back ground loop
		INIT_EVENT(&pAdapter->MainTimerStoppedEvent);
		START_TIMER( &pAdapter->MainTimer, pAdapter->TimerResolution );
		pAdapter->MainTimerState = TMR_TIMING;

		//	Automatically Start (everything necessary) the ADSL
		if(!pAdapter->PersistData.LineAutoActivation)
			CDALSetLineState(pAdapter, CDAL_LINE_STATE_INT);
		else
			CDALSetLineState(pAdapter,CDAL_LINE_STATE_PERSIST_INITIALIZE);

		//	Start the kernel threads
		BGThread = start_kthread(cardALDeviceBGThread, pAdapter, "cnx_bgthread");
		SIThread = start_kthread(cardALDeviceSIThread, pAdapter, "cnx_sithread");
		TMThread = start_kthread(cardALDeviceTmrThread, pAdapter, "cnx_tmthread");

		AdapterFullyInitialized = TRUE;
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			pAdapter->DebugFlag,
			("CnxADSL=>CardMgmt:CmgmtProcIOCTL  TIG_START_DSL command completed successfully"));
	}

	// if the record is user configurable parameters
	// then save them off
	else if (Command == TIG_SET_PARAMS)
	{
		PTIG_USER_PARAMS	pTigParams = (PTIG_USER_PARAMS)pArgument;

		if (pAdapter->DownloadInitComplete)
		{
			DBG_CDSL_DISPLAY(
				DBG_LVL_GENERIC,
				DBG_FLAG_DONT_CARE,
				("CnxADSL=>CardMgmt:CmgmtProcIOCTL  TIG_SET_PARAMS command, DownloadInitComplete already set"));

			pTigParams->CommandStatus = STATUS_SUCCESS;
		}
		else
		{
			DBG_CDSL_DISPLAY(
				DBG_LVL_GENERIC,
				DBG_FLAG_DONT_CARE,
				("CnxADSL=>CardMgmt:CmgmtProcIOCTL  TIG_SET_PARAMS command"));

			//
			// load the configuration parameters into the adapter structure
			//
			// load the parameters passed in the IOCTL
			COPY_MEMORY( &pAdapter->PersistData, pTigParams, sizeof( pAdapter->PersistData ));
			// load the parameters specified when the module was loaded/installed
			pAdapter->PersistData.CnxtVendorId = CnxtDslVendorId;
			pAdapter->PersistData.CnxtArmDeviceId = CnxtDslArmDeviceId;
			pAdapter->PersistData.CnxtAdslDeviceId = CnxtDslAdslDeviceId;
			pAdapter->PersistData.CnxtPhysicalDriverType = (PHYSICAL_DRIVER_TYPE_T) CnxtDslPhysicalDriverType;
			#ifdef DBG
				CardALDebugFlag = pAdapter->PersistData.DebugFlag;
			#endif

			// run the configuration init on each module
			for(	cnt = 0;
					cnt < sizeof(DownloadModuleMethods)/sizeof(MODULE_FUNCTIONS_T);
					cnt++ )
			{
				if (DownloadModuleMethods[cnt].CfgInit != NULL)
				{
					DownloadModuleMethods[cnt].CfgInit(pAdapter, pTigParams);
				}
			}

			for(	cnt = 0;
					cnt < sizeof(ModemModuleMethods)/sizeof(MODULE_FUNCTIONS_T);
					cnt++ )
			{
				if (ModemModuleMethods[cnt].CfgInit != NULL)
				{
					ModemModuleMethods[cnt].CfgInit(pAdapter, pTigParams);
				}
			}

			// Initialize the download modules.  The modem modules will be
			// initialized after the download is complete.
			for(	cnt = 0;
					cnt < sizeof(DownloadModuleMethods)/sizeof(MODULE_FUNCTIONS_T);
					cnt++ )
			{
				if (DownloadModuleMethods[cnt].AdapterInit != NULL)
				{
					Status = DownloadModuleMethods[cnt].AdapterInit(  pAdapter );
					if ( Status != STATUS_SUCCESS )
					{
						DBG_CDSL_DISPLAY(
							DBG_LVL_GENERIC,
							DBG_FLAG_DONT_CARE,
							("CnxADSL=>CardMgmt:CmgmtProcIOCTL  Module init fail %d %x",cnt,Status));
						pTigParams->CommandStatus = Status;
						return Status;
					}
				}
			}

			pTigParams->CommandStatus = STATUS_SUCCESS;

			// mark the download modules as initialized
			pAdapter->DownloadInitComplete = TRUE;
		}
	}

	// is it a device specific request
	else if (Command == TIG_DEVICE_SPEC)
	{
		TIG_DEVICE_SPEC_T *pCommand = (TIG_DEVICE_SPEC_T*)pArgument;

		// loop through all of the modules to let each have a
		// crack at it.  Stop at the first one that handles it
		Status = STATUS_UNSUPPORTED;

		// check the card managment module
		Status = cmCtlDeviceSpecific(	pAdapter,
		                              (BACK_DOOR_T*)&pCommand->BackDoorBuf,
		                              &pCommand->RequiredSize,
		                              &pCommand->ReturnSize );

		// run the ioctl handler on each module
		for(cnt = 0;
		        (cnt < sizeof(DownloadModuleMethods)/sizeof(MODULE_FUNCTIONS_T)) &&
		        (Status == STATUS_UNSUPPORTED);
		        cnt++ )
		{
			if (DownloadModuleMethods[cnt].IoctlHdlr != NULL)
				Status = DownloadModuleMethods[cnt].IoctlHdlr(	pAdapter,
				         (BACK_DOOR_T*)&pCommand->BackDoorBuf,
				         &pCommand->RequiredSize,
				         &pCommand->ReturnSize );
		}

		for(cnt = 0;
		        (cnt < sizeof(ModemModuleMethods)/sizeof(MODULE_FUNCTIONS_T)) &&
		        (Status == STATUS_UNSUPPORTED);
		        cnt++ )
		{
			if (ModemModuleMethods[cnt].IoctlHdlr != NULL)
				Status = ModemModuleMethods[cnt].IoctlHdlr(	pAdapter,
				         (BACK_DOOR_T*)&pCommand->BackDoorBuf,
				         &pCommand->RequiredSize,
				         &pCommand->ReturnSize);
		}
	}

	// see if its a showtime status request
	else if (Command == TIG_IS_SHOWTIME)
	{
		*(DWORD*)pArgument = pAdapter->Showtime;
	}
	// see if its a log record request
	else if (Command == TIG_GET_LOG )
	{
		char *pReturn = (char*)pArgument;
		
		Status = CdalDiagGetTxtLog( pAdapter,
					    pReturn,
					    256  );
	}
	
	
	// see if its a log record request
	else if (Command == TIG_CLEAR_STATS )
	{
		CDSL_LINK_T* pThisLink = GET_LINK_FROM_ADAPTER( pAdapter );
		FrameALClearStats( pThisLink );
	}
	
	
	// otherwise its not a valid command so err it
	else
	{
		Status = -EBADRQC;
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("CnxADSL=>CardMgmt:CmgmtProcIOCTL  Unknown IOCTL command, command=%d Status=0x%x",Command,Status));
	}

 return Status;
 }



/*++
    CmgmtGetSocketOption

Routine Description:

    This function is called to get option settings for a 
    given connection.  We don't currently have any readable
    options.

Arguments:

    pVcc - identifies the virtual connection
    OptionLevel - level of the option to get
    OptionName - name (actually number) of the option to get
    pOptionValue - pointer to value to get the option
    OptionLength - length of the option value field

Return Value:

    The status of the operation.

--*/
int
CmgmtGetSocketOption(
    IN struct atm_vcc*	pVcc,
    IN int		OptionLevel,
    IN int		OptionName,
    OUT PVOID		pOptionValue,
    IN int		OptionLength )
{
	int Status = STATUS_SUCCESS;

	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		(	"CnxADSL=>CardMgmt:CmgmtGetSocketOption  pVcc %lx level %d  name %d  lth %d",
			(ULONG)pVcc, OptionLevel, OptionName, OptionLength ));

	return Status;
}



/*++
    CmgmtSetSocketOption

Routine Description:

    This function is called to set an option value
    for a specified connection.  There are currently not
    any settable option values.

Arguments:

    pVcc - identifies the virtual connection
    OptionLevel - level of the option to set
    OptionName - name (actually number) of the option to set
    pOptionValue - pointer to value to set the option to
    OptionLength - length of the option value field

Return Value:

    The status of the operation.

--*/
int
CmgmtSetSocketOption(
    IN struct atm_vcc*	pVcc,
    IN int		OptionLevel,
    IN int		OptionName,
    IN PVOID		OptionValue,
    IN OptionLength_t	OptionLength)
{
	int Status = STATUS_SUCCESS;

	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("CnxADSL=>CardMgmt:CmgmtSetSocketOption"));

	return Status;
}


/*++
    CmgmtSendBuffer

Routine Description:

    This function is called to transmit a buffer to the 
    ATM device.

Arguments:

    pVcc - identifies the connection to use for the send
    pSkb - transmit buffer identifier

Return Value:

    The status of the operation.

--*/
int
CmgmtSendBuffer(
    IN struct atm_vcc*	pVcc,
    IN SK_BUFF_T*	pSkb )
{
	int Status = STATUS_SUCCESS;
	CDSL_VC_T * pVCStr;

	//	DBG_CDSL_DISPLAY(
	//		DBG_LVL_MISC_INFO,
	//		DBG_FLAG_DONT_CARE,
	//		("CnxADSL=>CardMgmt:CmgmtSendBuffer"));

	// get the device vc pointer
	pVCStr = (CDSL_VC_T*)pVcc->dev_data;

	// if the vcc is not enabled then fail
	if ( !pVCStr->Connected )
	{
			DBG_CDSL_DISPLAY(
				DBG_LVL_GENERIC,
				DBG_FLAG_DONT_CARE,
			("CnxADSL=>CardMgmt:CmgmtSendBuffer  Attempt to send on an inactive VC"));
		RELEASE_SKB( pVcc, pSkb );
		return -EINVAL;
	}

	// pass it to the Buffer management module to be transmitted.
	// Status pending means it was successfully queued so tell
	// Linux it was successful.
	Status = BufMgmtSend( pVCStr, pSkb );
	if ( Status == COMMAND_PENDING )
		Status = STATUS_SUCCESS;

	return Status;
}


/*++
    CmgmtScatGatSendQuery

Routine Description:

    This function is called to query whether the specified buffer
    should be copied before sending it to the send function.  If
    the buffer is not contiguous and the device does not support
    scatter/gather, then the function should return false to force
    the copy.
    
Arguments:

    pVcc   - identifies the connection to be used
    BufferStart - pointer to the start of the buffer
    BufferSize - length of the buffer in bytes

Return Value:

    The status of the operation.

--*/
int
CmgmtScatGatSendQuery(
    IN struct atm_vcc*	pVcc,
    IN ULONG		BufferStart,
    IN ULONG		BufferSize )
{
	int Status = STATUS_SUCCESS;

	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("CnxADSL=>CardMgmt:CmgmtScatGatSendQuery"));

	return Status;
}


/*++
    CmgmtSendOAMCell

Routine Description:

    This function is called to send an operation and maintenance
    cell.  It is currently not used by Linux and not implemented
    here.

Arguments:

    pVcc  - connection to use for the transfer  
    pCell  - pointer to the cell data
    flags - send flags


Return Value:

    -ENOSYS - function not implemented

--*/
int
CmgmtSendOAMCell(
    IN struct atm_vcc*	pVcc,
    IN void		*pCell,
    IN int		flags)
{
	int Status = -ENOSYS;

	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("CnxADSL=>CardMgmt:CmgmtSendOAMCell"));

	return Status;
}


/*++
    CmgmtPhyPut

Routine Description:

    This function is called by the Linux ATM kernel to 
    write directly to the PHY module.  This driver does
    not currently use this function.

Arguments:

    pDev - selects the modem device
    value - value to put
    addr - where to put the value


Return Value:

    None

--*/
void
CmgmtPhyPut(
    IN struct atm_dev*	pDev,
    IN UCHAR		value,
    IN ULONG		addr)
{

	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("CnxADSL=>CardMgmt:CmgmtPhyPut"));

}


/*++
    CmgmtPhyGet

Routine Description:

    This function is called to get information directly
    from the physical device.  It is currently not 
    implemented for this driver.

Arguments:

    pDev - selects the device to get from
    addr - where to get the data

Return Value:

    -ENOSYS - function not implemented

--*/
UCHAR
CmgmtPhyGet(
    IN struct atm_dev*	pDev,
    OUT ULONG		addr)
{
	UCHAR Status = -ENOSYS;

	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("CnxADSL=>CardMgmt:CmgmtPhyGet"));

	return Status;
}


/*++
    CmgmtFeedback

Routine Description:

    This function is called when the protocol has determined where
    an incoming SDU will be stored.  It can be used by the driver
    to adjust heuristic parameters to predict optimal buffer allocation.

Arguments:

    pVcc - identifies the virtual connection
    skb - receive buffer identifier
    start - actual start of the buffer
    dest - destination for buffer storage
    len - length of the buffer in bytes

Return Value:

    None

--*/
void
CmgmtFeedback(
    IN struct atm_vcc*	pVcc,
    SK_BUFF_T*		skb,
    ULONG		start,
    ULONG		dest,
    int			len)
{

	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("CnxADSL=>CardMgmt:CmgmtFeedback"));

}


/*++
    CmgmtChangeQOS

Routine Description:

    This function is called to adjust the QOS parameters for
    an already open connection.

Arguments:

    pVcc - pointer to the connection to adjust 
    pQOS - new quality of service parameters
    flags - flag word

Return Value:

    None

--*/
int
CmgmtChangeQOS(	IN struct atm_vcc*	pVcc,
                IN struct atm_qos*	pQOS,
                IN int			flags)
{
	int Status = -ENOSYS;

	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("CnxADSL=>CardMgmt:CmgmtChangeQOS  is not implemented"));

	return Status;
}


/*++
    CmgmtFreeRxSkb

Routine Description:

    This function is called by the protocol to release a 
    receive buffer

Arguments:

    pVcc - pointer to the connection that received the buffer
    pSkb - buffer identifier

Return Value:

    The status of the operation.

--*/
void
CmgmtFreeRxSkb(
    IN struct atm_vcc*	pVcc,
    IN SK_BUFF_T*	pSkb)
{

	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("CnxADSL=>CardMgmt:CmgmtFreeRxSkb"));

}


/*++
    CmgmtProcessRead

Routine Description:

    This function is called when a process reads from the
	modem device created by the Linux ATM module.  Commonly
	it is a cat of the device by the operator.  So it is
	used to display general driver information and statistics.

Arguments:

    pDev - identifies the device  
    pPOS - 
    pPage -


Return Value:

    -ENOSYS - function not implemented

--*/
int
CmgmtProcessRead( IN struct atm_dev*	pDev,
                  IN loff_t*		pPOS,
                  OUT char*		pPage)
{
	int RetBytes = 0;
	int State = *pPOS;
	int VCIndex;
	char Buffer[sizeof(TIG_DEVICE_SPEC_T) + sizeof(BACK_DOOR_T)];
	TIG_DEVICE_SPEC_T  *pDevSpecific=(TIG_DEVICE_SPEC_T*)Buffer;
	BACK_DOOR_T        *pBackDoorBuf=(BACK_DOOR_T*)pDevSpecific->BackDoorBuf;
	CDSL_ADAPTER_T     *pAdapter = (CDSL_ADAPTER_T*)pDev->dev_data;

	//	DBG_CDSL_DISPLAY(
	//		DBG_LVL_MISC_INFO,
	//		pAdapter->DebugFlag,
	//		("CnxADSL=>CardMgmt:CmgmtProcessRead"));

	////////////////////////////////////////////////////////////////////////////
	// General information
	////////////////////////////////////////////////////////////////////////////
	if (!State--)
	{
		BD_ADSL_LINE_STATUS_T* pLineStat = &pBackDoorBuf->Params.BdAdslLineStatus;
		BD_ADSL_CFG_T		   BdCfg;

		//		{
		//			BD_ADSL_PERFORMANCE_T pBuff;
		//		    CdalDiagGetADSLPerf( pAdapter->CardMod, &pBuff, sizeof( BD_ADSL_PERFORMANCE_T ));
		//		}

		// print out the header information
		RetBytes  = sprintf(pPage,"\n\nConexant AccessRunner PCI ADSL Modem Adapter Status\n");
		RetBytes  += sprintf(pPage+RetBytes,"----------------------------------\n\n");

		// init the statistics structure
		CLEAR_MEMORY( pBackDoorBuf, sizeof(BACK_DOOR_T));
		pBackDoorBuf->TotalSize = sizeof(BACK_DOOR_T);
		pBackDoorBuf->ReqCode   = BD_CARDMGMT_GET_CONNECT_STATUS;

		// read the line statistics
		CmgmtProcIOCTL( pDev, TIG_DEVICE_SPEC, pDevSpecific);

		// report the connection status
		if ( pAdapter->Showtime )
		{
			RetBytes += sprintf(pPage+RetBytes,"ADSL Line Connected\n\n");

			// init the statistics structure
			CLEAR_MEMORY( pBackDoorBuf, sizeof(BACK_DOOR_T));
			pBackDoorBuf->TotalSize = sizeof(BACK_DOOR_T);
			pBackDoorBuf->ReqCode   = BD_ADSL_GET_LINE_STATUS;

			// read the line statistics
			CmgmtProcIOCTL( pDev, TIG_DEVICE_SPEC, pDevSpecific);

			RetBytes += sprintf(pPage+RetBytes,"Line Rates:   Receive %ld kbps     Transmit %ld kbps\n\n",
			                    pLineStat->LineSpeedDownOrNear,
			                    pLineStat->LineSpeedUpOrFar);
		}
		else
		{
			RetBytes += sprintf(pPage+RetBytes,"ADSL Line Not Connected\nLine Rates: Not available\n\n");
		}
			
		// display the modulation
		CdalDiagGetAdslConfig( pAdapter, (char*)&BdCfg, sizeof(BdCfg));
		
		RetBytes += sprintf(pPage+RetBytes,"ADSL Modulation:");
			 
		switch ( BdCfg.AdslDpRateMode )
		{
    	case BD_ADSL_DP_RATE_G_DOT_DMT:
			RetBytes += sprintf(pPage+RetBytes,"G.DMT, Rate Unlimited  /  Full Rate\n\n");
			break;

	    case BD_ADSL_DP_RATE_G_DOT_LITE:
			RetBytes += sprintf(pPage+RetBytes,"G.Lite, Rate Limited (1.5 Mbps)\n\n");
			break;

	   	case BD_ADSL_DP_RATE_T1_DOT_413:
			RetBytes += sprintf(pPage+RetBytes," T1.413  Issue 2\n\n");
			break;

    	case BD_ADSL_DP_RATE_NOT_AVAIL:
		default:
			RetBytes += sprintf(pPage+RetBytes,"Not Available\n\n");
			break;
		}
 
		// display the connection info
// --PIM--
                {
                struct sock *s;
                struct atm_vcc* pVcc;
		struct hlist_node *node;
		int i;
                int nfound=0;

                read_lock(&vcc_sklist_lock);
        	for(i = 0; i < VCC_HTABLE_SIZE; ++i) {
                	struct hlist_head *head = &vcc_hash[i];

                	sk_for_each(s, head) {
                        	pVcc = atm_sk(s);
                        	if (pVcc->dev != pDev)
                                	continue;
				nfound++;
                        	if (nfound==1)
                                	RetBytes += sprintf(pPage+RetBytes,"ATM Virtual Channel IDs: ");
                        	else
                                	RetBytes += sprintf(pPage+RetBytes,"                         ");
                        	RetBytes += sprintf(pPage+RetBytes,"VPI: %d   VCI: %d\n\n",pVcc->vpi,pVcc->vci);
                        	}
                	}
                read_unlock(&vcc_sklist_lock);
                if (nfound==0)
                        RetBytes += sprintf(pPage+RetBytes,"ATM Virtual Channel IDs: no connections open\n\n");
                }
//--/PIM--
		return RetBytes;
	}

	////////////////////////////////////////////////////////////////////////////
	// Physical layer statistics
	////////////////////////////////////////////////////////////////////////////
	if (!State--)
	{
		BD_CARDAL_ADSL_TRANSCEIVER_STATUS_T *pStats = &pBackDoorBuf->Params.AdslTransceiverStatus;

		// init the statistics structure
		CLEAR_MEMORY( pBackDoorBuf, sizeof(BACK_DOOR_T));
		pBackDoorBuf->TotalSize = sizeof(BACK_DOOR_T);
		pBackDoorBuf->ReqCode   = BD_CARDAL_GET_ADSL_TRANSCEIVER_STATUS;

		// read the line statistics
		CmgmtProcIOCTL( pDev, TIG_DEVICE_SPEC, pDevSpecific);

		RetBytes = sprintf(pPage,"Physical Layer Stats:      Receive                   Transmit\n");

		// print the snr values, they are in 8.8 format
		RetBytes += sprintf(pPage+RetBytes," Noise Margin      %3d.%02d/%3d.%02d: %3d           %3d.%02d/%3d.%02d: %3d\n",
		                    pStats->Down_SNR_Margin.Mar_Cur>>8,
							((((pStats->Down_SNR_Margin.Mar_Cur)&0xff)*100)/256),
		                    pStats->Down_SNR_Margin.Mar_Min>>8,
							((((pStats->Down_SNR_Margin.Mar_Min)&0xff)*100)/256),
							pStats->Down_SNR_Margin.Mar_Min_Bin,
		                    pStats->Up_SNR_Margin.Mar_Cur>>8,
							((((pStats->Up_SNR_Margin.Mar_Cur)&0xff)*100)/256),
		                    pStats->Up_SNR_Margin.Mar_Min>>8,
							((((pStats->Up_SNR_Margin.Mar_Min)&0xff)*100)/256),
							pStats->Up_SNR_Margin.Mar_Min_Bin);

		// print the attenuation values, they are in 7.9 format
		RetBytes += sprintf(pPage+RetBytes," Attenuation             %3d.%02d                    %3d.%02d\n",
		                    pStats->Down_Attn>>9,
							((((pStats->Down_Attn)&0x1ff)*100)/512),
		                    pStats->Up_Attn>>9,
							((((pStats->Up_Attn)&0x1ff)*100)/512));

		// print the transmit power, there is no receive power
		RetBytes += sprintf(pPage+RetBytes," Power                                          %9d\n",
		                    pStats->Tx_Power);

		return RetBytes;
	}

	////////////////////////////////////////////////////////////////////////////
	// Frame abstraction layer statistics
	////////////////////////////////////////////////////////////////////////////
	if (!State--)
	{
		BD_FRAMEAL_ATM_STATS_T *pStats = &pBackDoorBuf->Params.BdFrameALAtmStats;

		// init the statistics structure
		CLEAR_MEMORY( pBackDoorBuf, sizeof(BACK_DOOR_T));
		pBackDoorBuf->TotalSize = sizeof(BACK_DOOR_T);
		pBackDoorBuf->ReqCode   = BD_FRAMEAL_ATM_GET_STATS;
		pStats->VcIndex         = BD_ATM_STATS_INDEX_FOR_LINK;

		// read the per vc statistics
		CmgmtProcIOCTL( pDev, TIG_DEVICE_SPEC, pDevSpecific);

		RetBytes = sprintf(pPage,"\nATM Link Stats:      Receive           Transmit\n");
		RetBytes += sprintf(pPage+RetBytes," Bytes             %9ld          %9ld\n",pStats->NumRxBytes,pStats->NumTxBytes);
		RetBytes += sprintf(pPage+RetBytes," Cells             %9ld          %9ld\n",pStats->NumRxCells,pStats->NumTxCells);
		RetBytes += sprintf(pPage+RetBytes," Management Cells  %9ld          %9ld\n",pStats->NumRxMgmtCells,pStats->NumTxMgmtCells);
		RetBytes += sprintf(pPage+RetBytes," CLP=0 Cells       %9ld          %9ld\n",pStats->NumRxClpEqual0Cells,pStats->NumTxClpEqual0Cells);
		RetBytes += sprintf(pPage+RetBytes," CLP=1 Cells       %9ld          %9ld\n",pStats->NumRxClpEqual1Cells,pStats->NumTxClpEqual1Cells);
		RetBytes += sprintf(pPage+RetBytes," HEC Errors        %9ld \n",pStats->NumRxHecErrs);
		RetBytes += sprintf(pPage+RetBytes," Alignment Errors  %9ld \n",pStats->NumRxCellAlignErrs);
		RetBytes += sprintf(pPage+RetBytes," Unroutable Cells  %9ld \n",pStats->NumRxUnroutCellErrs);
		return RetBytes;
	}

	////////////////////////////////////////////////////////////////////////////
	// AAL layer statistics
	////////////////////////////////////////////////////////////////////////////
	if (!State--)
	{
		BD_FRAMEAL_AAL_STATS_T *pStats = &pBackDoorBuf->Params.BdFrameALAalStats;

		// init the statistics structure
		CLEAR_MEMORY( pBackDoorBuf, sizeof(BACK_DOOR_T));
		pBackDoorBuf->TotalSize = sizeof(BACK_DOOR_T);
		pBackDoorBuf->ReqCode   = BD_FRAMEAL_AAL_GET_STATS;
		pBackDoorBuf->Params.BdFrameALAtmStats.VcIndex = BD_AAL_STATS_INDEX_FOR_LINK;

		// read the per vc statistics
		CmgmtProcIOCTL( pDev, TIG_DEVICE_SPEC, pDevSpecific);

		RetBytes = sprintf(pPage,"\nAAL Link Stats:          Receive           Transmit\n");
		RetBytes += sprintf(pPage+RetBytes," Good Bytes            %9ld          %9ld\n",pStats->NumRxGoodBytes,pStats->NumTxGoodBytes);
		RetBytes += sprintf(pPage+RetBytes," Good Frames           %9ld          %9ld\n",pStats->NumRxGoodFrames,pStats->NumTxGoodFrames);
		RetBytes += sprintf(pPage+RetBytes," Discarded Bytes       %9ld \n",pStats->NumRxDiscardedBytes);
		RetBytes += sprintf(pPage+RetBytes," Discarded Frames      %9ld \n",pStats->NumRxDiscardedFrames);
		RetBytes += sprintf(pPage+RetBytes," CRC Errors            %9ld \n",pStats->NumRxCrcErrs);
		RetBytes += sprintf(pPage+RetBytes," Invalid Length Errors %9ld \n",pStats->NumRxInvalidLenErrs);
		RetBytes += sprintf(pPage+RetBytes," Timeout Errors        %9ld \n\n",pStats->NumRxTimeoutErrs);
		return RetBytes;
	}

	////////////////////////////////////////////////////////////////////////////
	// display ATM statistics per VC
	////////////////////////////////////////////////////////////////////////////
	for ( VCIndex=0; VCIndex<MAX_VC_PER_LINK; VCIndex++)
	{
		// init the statistics structure
		CLEAR_MEMORY( pBackDoorBuf, sizeof(BACK_DOOR_T));
		pBackDoorBuf->TotalSize = sizeof(BACK_DOOR_T);
		pBackDoorBuf->ReqCode   = BD_FRAMEAL_ATM_GET_STATS;
		pBackDoorBuf->Params.BdFrameALAtmStats.VcIndex = VCIndex;

		// read the per vc statistics 
		CmgmtProcIOCTL( pDev, TIG_DEVICE_SPEC, pDevSpecific);

		if (pBackDoorBuf->ResultCode == RESULT_SUCCESS)
		{
			if (!State--)
			{
				BD_FRAMEAL_ATM_STATS_T *pStats = &pBackDoorBuf->Params.BdFrameALAtmStats;
				RetBytes = sprintf(pPage,"ATM Channel Stats:       Receive           Transmit\n");
				RetBytes += sprintf(pPage+RetBytes," Bytes                 %9ld          %9ld\n",pStats->NumRxBytes,pStats->NumTxBytes);
				RetBytes += sprintf(pPage+RetBytes," Cells                 %9ld          %9ld\n",pStats->NumRxCells,pStats->NumTxCells);
				RetBytes += sprintf(pPage+RetBytes," Mgmt Cells            %9ld          %9ld\n",pStats->NumRxMgmtCells,pStats->NumTxMgmtCells);
				RetBytes += sprintf(pPage+RetBytes," CLP=0 Cells           %9ld          %9ld\n",pStats->NumRxClpEqual0Cells,pStats->NumTxClpEqual0Cells);
				RetBytes += sprintf(pPage+RetBytes," CLP=1 Cells           %9ld          %9ld\n",pStats->NumRxClpEqual1Cells,pStats->NumTxClpEqual1Cells);
				RetBytes += sprintf(pPage+RetBytes," HEC Errors            %9ld \n",pStats->NumRxHecErrs);
				RetBytes += sprintf(pPage+RetBytes," Alignment Errors      %9ld \n",pStats->NumRxCellAlignErrs);
				RetBytes += sprintf(pPage+RetBytes," Unroutable Cells      %9ld \n\n",pStats->NumRxUnroutCellErrs);
				return RetBytes;
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////
	// display AAL stattistics per VC
	////////////////////////////////////////////////////////////////////////////
	for ( VCIndex=0; VCIndex<MAX_VC_PER_LINK; VCIndex++)
	{
		// init the statistics structure
		CLEAR_MEMORY( pBackDoorBuf, sizeof(BACK_DOOR_T));
		pBackDoorBuf->TotalSize = sizeof(BACK_DOOR_T);
		pBackDoorBuf->ReqCode   = BD_FRAMEAL_AAL_GET_STATS;
		pBackDoorBuf->Params.BdFrameALAtmStats.VcIndex = VCIndex;

		// read the per vc statistics
		CmgmtProcIOCTL( pDev, TIG_DEVICE_SPEC, pDevSpecific);

		if (pBackDoorBuf->ResultCode == RESULT_SUCCESS)
		{
			if (!State--)
			{
				BD_FRAMEAL_AAL_STATS_T *pStats = &pBackDoorBuf->Params.BdFrameALAalStats;
				RetBytes = sprintf(pPage,"AAL 5  Channel Stats:    Receive           Transmit\n");
				RetBytes += sprintf(pPage+RetBytes," Good Bytes            %9ld          %9ld\n",pStats->NumRxGoodBytes,pStats->NumTxGoodBytes);
				RetBytes += sprintf(pPage+RetBytes," Good Frames           %9ld          %9ld\n",pStats->NumRxGoodFrames,pStats->NumTxGoodFrames);
				RetBytes += sprintf(pPage+RetBytes," Discarded Bytes       %9ld\n",pStats->NumRxDiscardedBytes);
				RetBytes += sprintf(pPage+RetBytes," Discarded Frames      %9ld\n",pStats->NumRxDiscardedFrames);
				RetBytes += sprintf(pPage+RetBytes," CRC Errors            %9ld \n",pStats->NumRxCrcErrs);
				RetBytes += sprintf(pPage+RetBytes," Invalid Length Errors %9ld \n",pStats->NumRxInvalidLenErrs);
				RetBytes += sprintf(pPage+RetBytes," Timeout Errors        %9ld \n\n",pStats->NumRxTimeoutErrs);
				return RetBytes;
			}
		}
	}

	if (!State--)
	{
		BD_MINI_DATA_BLOCK_T*  pProdVer  = &pBackDoorBuf->Params.BdCardMgmtProdVer;
	
		// print the driver version
		CLEAR_MEMORY( pBackDoorBuf, sizeof(BACK_DOOR_T));
		pBackDoorBuf->TotalSize = sizeof(BACK_DOOR_T);
		pBackDoorBuf->ReqCode   = BD_CARDMGMT_GET_PRODUCT_VERSION;

		// get the driver version
		CmgmtProcIOCTL( pDev, TIG_DEVICE_SPEC, pDevSpecific);

		// output it
		RetBytes += sprintf(pPage+RetBytes,"Driver version - %s\n", (UCHAR*)pProdVer);

		// print the mac address
		CLEAR_MEMORY( pBackDoorBuf, sizeof(BACK_DOOR_T));
		pBackDoorBuf->TotalSize = sizeof(BACK_DOOR_T);
		pBackDoorBuf->ReqCode   = BD_CARDMGMT_GET_EEPROM_MAC_ADDR;

		// get the mac address
		CmgmtProcIOCTL( pDev, TIG_DEVICE_SPEC, pDevSpecific);

		RetBytes += sprintf(pPage+RetBytes,"EE PROM Mac Address %02x.%02x.%02x.%02x.%02x.%02x\n\n",
		                    pBackDoorBuf->Params.BdCardMgmtMacAddr[0],
		                    pBackDoorBuf->Params.BdCardMgmtMacAddr[1],
		                    pBackDoorBuf->Params.BdCardMgmtMacAddr[2],
		                    pBackDoorBuf->Params.BdCardMgmtMacAddr[3],
		                    pBackDoorBuf->Params.BdCardMgmtMacAddr[4],
		                    pBackDoorBuf->Params.BdCardMgmtMacAddr[5]);
		
		return RetBytes;
	}

	RetBytes = 0;

	return RetBytes;
}



/*++
    CmgmtPhyStart

Routine Description:

    Function called to start the physical device.  Currently
	not used.

Arguments:

    pDev - identifies the device.

Return Value:

    The status of the operation.

--*/
int
CmgmtPhyStart(
    IN struct atm_dev*	pDev
)
{
	int Status = STATUS_SUCCESS;

	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("CnxADSL=>CardMgmt:CmgmtPhyStart"));

	return Status;
}



/*++
    CmgmtPhyIOCTL

Routine Description:

    This function is called to pass commands directly to the 
    phy module.  This is not used by the driver.

Arguments:

    pDev  - identifies the device
    cmd - command to execute
    arg - data associated with the command

Return Value:

    -ENOSYS - function not implemented

--*/
int
CmgmtPhyIOCTL( IN struct atm_dev*	pDev,
               IN unsigned int		cmd,
               INOUT void*		arg)
{
	int Status = -ENOSYS;

	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("CnxADSL=>CardMgmt:CmgmtPhyIOCTL"));

	return Status;
}



/*++
    CmgmtPhyInterrupt

Routine Description:

    This function is called to generate an interrupt to
    the physical device.  Not used by this driver.

Arguments:

    pDev - identifies the device

Return Value:

    None

--*/
void
CmgmtPhyInterrupt(
    IN struct atm_dev*	pDev
)
{

	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("CnxADSL=>CardMgmt:CmgmtPhyInterrupt"));

}


/******************************************************************************
*	FUNCTION NAME:
*		CmgmtIndicateLinkUp
*
*	ABSTRACT:
*		.
*		This function is called by the chip abstraction layer to indicate
*		that the ADSL link is up.
*
*	RETURN:
*
*	DETAILS:
******************************************************************************/
void CmgmtIndicateLinkUp(
    IN CDSL_ADAPTER_T		* pThisAdapter )
{
	CDSL_LINK_T				* pThisLink;

	//	Indicate showtime
	pThisAdapter->Showtime = TRUE;
	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		DBG_FLAG_DONT_CARE,
		("CnxADSL=>CardMgmt:CmgmtIndicateLinkUp - It's SHOWTIME!!!"));

	ChipALLinkEnable(pThisAdapter,ADSL_CHANNEL);

	// infrom FrameAL to get one cell sent
	pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );
	FrameALLinkUp(pThisLink);
}


/******************************************************************************
*	FUNCTION NAME:
*		CmgmtIndicateLinkDown
*
*	ABSTRACT:
*		.
*		This function is called by the chip abstraction layer to indicate
*		that the ADSL link is down.
*
*	RETURN:
*
*	DETAILS:
******************************************************************************/
void CmgmtIndicateLinkDown(
    IN CDSL_ADAPTER_T		* pThisAdapter )
{
	CDSL_LINK_T				* pThisLink;

	pThisAdapter->Showtime = FALSE;
	DBG_CDSL_DISPLAY(
		DBG_LVL_GENERIC,
		pThisAdapter->DebugFlag,
		("CnxADSL=>CardMgmt:CmgmtIndicateLinkDown - Show's over! :("));

	ChipALLinkDisable(pThisAdapter,ADSL_CHANNEL);

	pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );

	FrameALLinkDown(pThisLink);
}


/******************************************************************************
*	FUNCTION NAME:
*		CmgmtAdapterReset
*
*	ABSTRACT:
*		Reset the adapter and driver.
*
*	RETURN:
*		status of initialization and setup
*
*	DETAILS:
******************************************************************************/
NDIS_STATUS CmgmtAdapterReset(IN CDSL_ADAPTER_T	* pThisAdapter )
{
	CDSL_LINK_T				* pThisLink;
	NDIS_STATUS				Status;

	//
	//Reset link data structures for all links
	//
	Status = CMLInitAllLinks( pThisAdapter );
	if ( Status != STATUS_SUCCESS )
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("CnxADSL=>CardMgmt:CmgmtAdapterReset  ERROR;  CMLInitAllLinks Failure!"));
		return Status;
	}

	//	Allocate the links (lines, line devices) for this adapter.
	pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );
	if ( pThisLink != NULL )
	{
		pThisLink = CMLAllocateLink(
		                pThisAdapter );
	}

	//
	//Reset VC data structures for all VCs
	//
	pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );
	Status = CMVCInitAllVcs( pThisAdapter, pThisLink );
	if ( Status != STATUS_SUCCESS )
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("CnxADSL=>CardMgmt:CmgmtAdapterReset  ERROR;  CMVCInitAllVcs Failure!"));
		return Status;
	}

	Status = CMLCallAllModuleLinkInits( pThisAdapter, pThisLink );
	if ( Status != STATUS_SUCCESS )
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("CnxADSL=>CardMgmt:CmgmtAdapterReset  ERROR;  CMLCallAllModuleLinkInits Failure!"));
		return Status;
	}

	//
	//Reset Timer Info
	//
	UtilTmrResetTmr(&(pThisAdapter->TimerBlock), MAX_TIMERS);

	return Status;
}


/******************************************************************************
*	FUNCTION NAME:
*		CmgmtMainTimerInterrupt
*
*	ABSTRACT:
*		Function called back from Linux (registering the Miniport driver
*		initialized a timer with this call-back function).
*		This function is called by Linux periodically (whenever the timer period
*		registered/initalized expires).
*
*	RETURN:
*
*	DETAILS:
*		This timer function calls UtilTmrTmrBackground which handles multiple timers
*		for the miniport driver.
*		The timer is restarted before returning to NDIS.
******************************************************************************/
VOID CmgmtMainTimerInterrupt(
    IN 	ULONG	 pAdapt
)
{
	CDSL_ADAPTER_T		* pThisAdapter=(CDSL_ADAPTER_T*)pAdapt;
	DWORD				LockFlag;

	if ( AdapterStructureAllocated )
	{
		if ( pThisAdapter->MainTimerState == TMR_TIMING )
		{
			pThisAdapter->MainTimerState = TMR_EXECUTING;

			ACQUIRE_LOCK( &pThisAdapter->Lock, LockFlag );

			UtilTmrTmrBackground( &(pThisAdapter->TimerBlock), pThisAdapter->TimerResolution );

			RELEASE_LOCK( &pThisAdapter->Lock, LockFlag );

			//	Normally this line would be used
			//		pThisAdapter->MainTimerState = TMR_COMPLETED;
			//	Instead, re set the timer to fire
			START_TIMER(&pThisAdapter->MainTimer, pThisAdapter->TimerResolution );
			pThisAdapter->MainTimerState = TMR_TIMING;
		}
		else
		{
			// Timer is being shutdown or stopped
			SET_EVENT( &pThisAdapter->MainTimerStoppedEvent );
		}
	}
	else
	{
		DBG_CDSL_DISPLAY(
			DBG_LVL_GENERIC,
			DBG_FLAG_DONT_CARE,
			("CnxADSL=>CardMgmt:CmgmtMainTimerInterrupt  Adapter Structure NOT Allocated!"));
	}
}


/******************************************************************************
	FUNCTION NAME:
		cmCtlDeviceSpecific

	ABSTRACT:
		This function coordinates all of device specific
		data requeest for actions that are performed for 
		Card Management module.

	DETAILS:
		EVERY MODULE WHICH SUPPORTS *ANY* BACKDOOR ACTION
		(EVEN ONE) *MUST* IMPLEMENT THIS FUNCTION
		IN AT LEAST A SIMILAR MANNER OR FASHION!!!!!!!!

		BACKDOOR CODE/SUBCODE VALUES *WILL BE DESIGNED*
		TO **GUARANTEE** THAT EACH CODE/SUBCODE VALUE
		WILL BE DIRECTED TO *ONLY ONE* MODULE!!

		NOTE:	The BackDoor Status should ONLY BE SET TO FAILURE
			at the beginning of BackDoor parsing by the first
			OID parser (CardMgmt)!!!
			ALL OTHER successor OID parsing of BackDoor should
			only set the BackDoor Status when it is intended
			to be set to a specific Status (i.e., BAD_SIGNATURE,
			FAILURE, or SUCCESS)!!  The BackDoor Status should
			not be set at the beginning of BackDoor parsing
			intending to change it if needed!!!
			This is to keep other modules (BackDoor parsing)
			from overwriting the BackDoor status once a module
			has parsed the BackDoor structure and acted upon
			the data!!!
			If the BackDoor Status is not set to CODE_NOT_SUPPORTED,
			then parsing can be aborted/stopped since some other
			BackDoor parser (module) has parsed the BackDoor
			Code request and indicated an error condition or
			acted upon the request.
******************************************************************************/
NDIS_STATUS cmCtlDeviceSpecific(
    IN CDSL_ADAPTER_T				* pThisAdapter,
    IN BACK_DOOR_T				* pBackDoorBuf,
    OUT PULONG					pNumBytesNeeded,
    OUT PULONG					pNumBytesWritten )
{
	NDIS_STATUS		Status = STATUS_SUCCESS;
	CDSL_LINK_T		* pThisLink;
	CDSL_LINK_CARD_MGMT_T	* pThisCMContext;
	DWORD			StrSize = 0;
	DWORD 			SysIfUpdated;
	DWORD			SysIfStatus;

	if(pBackDoorBuf->TotalSize >= sizeof(MINI_BACK_DOOR_T) )
	{
		SysIfUpdated = pBackDoorBuf->TotalSize - BD_HEADER_SIZE;
	}
	else
	{
		SysIfUpdated = 0l;
	}

	//	Assume BACKDOOR ReqCode will be processed,
	//	set the BACKDOOR Status to Code Not Supported.
	//	NOTE:	The BackDoor Status should ONLY BE SET TO
	//		CODE_NOT_SUPPORTED at the beginning of BackDoor
	//		parsing by the first OID parser (CardMgmt)!!!
	//		ALL OTHER successor OID parsing of BackDoor should
	//		only set the BackDoor Status when it is intended
	//		to be set to a specific Status (i.e., BAD_SIGNATURE,
	//		FAILURE, or SUCCESS)!!  The BackDoor Status should
	//		not be set at the beginning of BackDoor parsing
	//		intending to change it if needed!!!
	pBackDoorBuf->ResultCode = ERROR_WARNING( RESULT_REQ_NOT_SUPPORTED );

	if ( pBackDoorBuf->TotalSize < SIZEOF_DWORD_ALIGN( BACK_DOOR_T ) )
	{
		if ( pBackDoorBuf->TotalSize != SIZEOF_DWORD_ALIGN( MINI_BACK_DOOR_T ) )
		{
			pBackDoorBuf->ResultCode = ERROR_WARNING( RESULT_REQ_BUFF_TOO_SMALL );
			pBackDoorBuf->NeededSize = SIZEOF_DWORD_ALIGN( BACK_DOOR_T );
			*pNumBytesNeeded = SIZEOF_DWORD_ALIGN( BACK_DOOR_T );
			DBG_CDSL_DISPLAY(
				DBG_LVL_WARNING,
				pThisAdapter->DebugFlag,
				(	"CnxADSL=>CardMgmt:cmCtlDeviceSpecific  BACK_DOOR:   Entry, Size Failure;  ReqCode=0x%08lX;  NeededSize=0x%08lX.",
					pBackDoorBuf->ReqCode,
					pBackDoorBuf->NeededSize));
			return ERROR_WARNING( RESULT_REQ_BUFF_TOO_SMALL );
		}
	}

	switch (pBackDoorBuf->ReqCode)
	{

#if 0
	case BD_CDSL_GET_PRODUCT_INFO:
		COPY_MEMORY(
		    ((PUCHAR) pBackDoorBuf->Params.BdCdslProdInfo.RemoteManufacturer),
		    pThisAdapter->CdslProdInfo.RemoteManufacturer,
		    MAX_MANUFACTURER_LENGTH );
		pBackDoorBuf->Params.BdCdslProdInfo.CdslHwRevision = pThisAdapter->CdslProdInfo.CdslHwRevision;
		pBackDoorBuf->Params.BdCdslProdInfo.RemoteVendorId = pThisAdapter->CdslProdInfo.RemoteVendorId;

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdCdslProdInfo )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CDSL_GET_OPERATIONAL_PARAMS:
		pBackDoorBuf->Params.BdCdslOperational.RateMode		= pThisAdapter->CdslOperational.RateMode;
		pBackDoorBuf->Params.BdCdslOperational.RxRate		= pThisAdapter->CdslOperational.RxRate;
		pBackDoorBuf->Params.BdCdslOperational.RxSymRate	= pThisAdapter->CdslOperational.RxSymRate;
		pBackDoorBuf->Params.BdCdslOperational.TxRate		= pThisAdapter->CdslOperational.TxRate;
		pBackDoorBuf->Params.BdCdslOperational.TxSymRate	= pThisAdapter->CdslOperational.TxSymRate;
		pBackDoorBuf->Params.BdCdslOperational.TxPwrMode	= pThisAdapter->CdslOperational.TxPwrMode;
		pBackDoorBuf->Params.BdCdslOperational.TxPwrVal		= pThisAdapter->CdslOperational.TxPwrVal;
		pBackDoorBuf->Params.BdCdslOperational.RxLevel		= pThisAdapter->CdslOperational.RxLevel;			//	Get/Read ONLY!
		pBackDoorBuf->Params.BdCdslOperational.SnMargin		= pThisAdapter->CdslOperational.SnMargin;			//	Get/Read ONLY!

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdCdslOperational )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CDSL_SET_OPERATIONAL_PARAMS:
		pThisAdapter->CdslOperational.RateMode	= pBackDoorBuf->Params.BdCdslOperational.RateMode;
		pThisAdapter->CdslOperational.RxRate	= pBackDoorBuf->Params.BdCdslOperational.RxRate;
		pThisAdapter->CdslOperational.RxSymRate	= pBackDoorBuf->Params.BdCdslOperational.RxSymRate;
		pThisAdapter->CdslOperational.TxRate	= pBackDoorBuf->Params.BdCdslOperational.TxRate;
		pThisAdapter->CdslOperational.TxSymRate	= pBackDoorBuf->Params.BdCdslOperational.TxSymRate;
		pThisAdapter->CdslOperational.TxPwrMode	= pBackDoorBuf->Params.BdCdslOperational.TxPwrMode;
		pThisAdapter->CdslOperational.TxPwrVal	= pBackDoorBuf->Params.BdCdslOperational.TxPwrVal;
		//	pThisAdapter->CdslOperational.RxLevel	= pBackDoorBuf->Params.BdCdslOperational.RxLevel;	//	Get/Read ONLY!
		//	pThisAdapter->CdslOperational.SnMargin	= pBackDoorBuf->Params.BdCdslOperational.SnMargin;	//	Get/Read ONLY!

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdCdslOperational )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

#endif
	case BD_CARDMGMT_GET_CONNECT_STATUS:
		pBackDoorBuf->Params.BdCardMgmtConnStatus = (DWORD) BACKDOOR_STATUS_IDLE;
		pBackDoorBuf->ResultCode = ERROR_FATAL( RESULT_ADAPTER_NOT_FOUND );
		pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if ( pThisLink != NULL )
		{
			//	We can't allow indications to NULL...
			if ( pThisLink->CardMgmt != NULL )
			{
				pThisCMContext = (CDSL_LINK_CARD_MGMT_T *)pThisLink->CardMgmt;
				pThisCMContext->State = LINK_STATE_CONNECTED;
				if ( pThisCMContext->State == LINK_STATE_CONNECTED )
				{
					pBackDoorBuf->Params.BdCardMgmtConnStatus = (DWORD) BACKDOOR_STATUS_DATAMODE;
				}
				pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			}
		}

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdCardMgmtConnStatus )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CARDMGMT_GET_DEBUG_FLAG:
		pBackDoorBuf->Params.BdCardMgmtDebugFlag = pThisAdapter->DebugFlag;

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdCardMgmtDebugFlag )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CARDMGMT_SET_DEBUG_FLAG:
		pThisAdapter->DebugFlag = pBackDoorBuf->Params.BdCardMgmtDebugFlag;

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded = SIZEOF_DWORD_ALIGN( BACK_DOOR_T );
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdCardMgmtDebugFlag )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CARDMGMT_GET_PRODUCT_INFO:
		pBackDoorBuf->Params.BdCardMgmtProdInfo.DrvrSwVer	= pThisAdapter->ProductVersion;
		pBackDoorBuf->Params.BdCardMgmtProdInfo.DrvrTapiVer	= TAPI_API_VERSION;
		pBackDoorBuf->Params.BdCardMgmtProdInfo.DrvrNdisVer	= DRIVER_VERSION_OF_NDIS;

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->Params.BdCardMgmtProdInfo )
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;



	case BD_CARDMGMT_OPEN_BD_INSTANCE:
		pBackDoorBuf->ResultCode = ERROR_FATAL( RESULT_ADAPTER_NOT_FOUND );
		pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if ( pThisLink != NULL )
		{
			pBackDoorBuf->Params.BdCardMgmtInstanceStatus = BD_STATUS_INSTANCE_FAILURE;
			pBackDoorBuf->Params.BdCardMgmtInstanceStatus = BD_STATUS_INSTANCE_UNAVAILABLE;
			pBackDoorBuf->Params.BdCardMgmtInstanceStatus = BD_STATUS_INSTANCE_EMPTY;
			pBackDoorBuf->Params.BdCardMgmtInstanceStatus = BD_STATUS_INSTANCE_SUCCESS;
			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		}

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CARDMGMT_CLOSE_BD_INSTANCE:
		pBackDoorBuf->ResultCode = ERROR_FATAL( RESULT_ADAPTER_NOT_FOUND );
		pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if ( pThisLink != NULL )
		{
			pBackDoorBuf->Params.BdCardMgmtInstanceStatus = BD_STATUS_INSTANCE_FAILURE;
			pBackDoorBuf->Params.BdCardMgmtInstanceStatus = BD_STATUS_INSTANCE_UNAVAILABLE;
			pBackDoorBuf->Params.BdCardMgmtInstanceStatus = BD_STATUS_INSTANCE_EMPTY;
			pBackDoorBuf->Params.BdCardMgmtInstanceStatus = BD_STATUS_INSTANCE_SUCCESS;
			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		}

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CARDMGMT_GET_PRODUCT_VERSION:
		pBackDoorBuf->ResultCode = ERROR_FATAL( RESULT_ADAPTER_NOT_FOUND );
		pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if ( pThisLink != NULL )
		{
			StrSize = strlen( REVISION_LEVEL ) + 1;
			if ( StrSize > sizeof( BD_MINI_DATA_BLOCK_T ) )
			{
				StrSize = sizeof( BD_MINI_DATA_BLOCK_T ) - 1;
				pBackDoorBuf->Params.BdCardMgmtProdVer[StrSize] = 0;
				COPY_MEMORY(
				    ((PUCHAR) &pBackDoorBuf->Params.BdCardMgmtProdVer),
				    REVISION_LEVEL,
				    StrSize );
				pBackDoorBuf->ResultCode = ERROR_NORMAL( RESULT_DATA_FAILURE );
			}
			else
			{
				COPY_MEMORY(
				    ((PUCHAR) &pBackDoorBuf->Params.BdCardMgmtProdVer),
				    REVISION_LEVEL,
				    StrSize );
				pBackDoorBuf->ResultCode = RESULT_SUCCESS;
			}
		}

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	StrSize
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CARDMGMT_GET_CURRENT_MAC_ADDR:
		pBackDoorBuf->ResultCode = ERROR_FATAL( RESULT_ADAPTER_NOT_FOUND );
		pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if ( pThisLink != NULL )
		{
			//******************************************************************************
			//** These addresses ETH_ADDRESS_LENGTH & WAN_ADDRESS_LENGTH are both 6 bytes.
			//******************************************************************************
			COPY_MEMORY(
			    ((PUCHAR) &pBackDoorBuf->Params.BdCardMgmtMacAddr),
			    pThisAdapter->PermanentEthAddress,
			    ETH_ADDRESS_LENGTH );
			StrSize = ETH_ADDRESS_LENGTH;

			pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		}

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	StrSize
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CARDMGMT_GET_EEPROM_MAC_ADDR:
		pBackDoorBuf->ResultCode = ERROR_FATAL( RESULT_ADAPTER_NOT_FOUND );
		pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if ( pThisLink != NULL )
		{
			pBackDoorBuf->ResultCode = CardALChipGetMacAddr(
			                               pThisAdapter,
			                               pBackDoorBuf->Params.BdCardMgmtMacAddr,
			                               ETH_ADDRESS_LENGTH );
		}

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	ETH_ADDRESS_LENGTH
		                    + sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	case BD_CARDMGMT_SET_EEPROM_MAC_ADDR:
		pBackDoorBuf->ResultCode = ERROR_FATAL( RESULT_ADAPTER_NOT_FOUND );
		pThisLink = GET_LINK_FROM_ADAPTER( pThisAdapter );
		if ( pThisLink != NULL )
		{
			pBackDoorBuf->ResultCode = CardALChipSetMacAddr(
			                               pThisAdapter,
			                               pBackDoorBuf->Params.BdCardMgmtMacAddr,
			                               ETH_ADDRESS_LENGTH );
		}

		pBackDoorBuf->ResultCode = RESULT_SUCCESS;
		pBackDoorBuf->NeededSize = pBackDoorBuf->TotalSize;

		*pNumBytesNeeded = pBackDoorBuf->TotalSize;
		*pNumBytesWritten =	sizeof( pBackDoorBuf->ResultCode )
		                    + sizeof( pBackDoorBuf->NeededSize );
		break;

	default:
		Status = STATUS_UNSUPPORTED;
		break;
	}

	// * Notify System Interace of new API Command
	SysIfStatus = SmSysIfBroadcastMessage(
		pThisAdapter,
		MESS_API_COMMAND,			// Unique to each channel
		pBackDoorBuf->ReqCode,			// Embedded Message key
		(CHAR *) &pBackDoorBuf->Params,		// Message Buffer - Send only Params Union
		&SysIfUpdated);				// Length only reflects Params Union

	if(SysIfStatus != STATUS_INVALID_PARAMETER)
	{
		// * Invalid parameters only mean the event is not recognized or was not handled
		// * Otherwise, we attempted to handle and must update status.
		*pNumBytesNeeded += SysIfUpdated  + BD_HEADER_SIZE;
		*pNumBytesWritten += SysIfUpdated + BD_HEADER_SIZE;
		if(SysIfStatus == -EBADLENGTH)
		{
			pBackDoorBuf->ResultCode = ERROR_WARNING( RESULT_REQ_BUFF_TOO_SMALL );
			pBackDoorBuf->NeededSize = SysIfUpdated;
		}
		else
		{
			pBackDoorBuf->ResultCode = SysIfStatus;
		}

	}
	
	return Status;
}



int CnxAdslReadProcDebugQuery(
	char		* ProcPage,
	char		** ProcPageStart,
	off_t		ProcPageOffset,
	int		ProcPageLen,
	int		* bEndOfFile,
	void		* ProcData)
{
#	define		BUFFER_LIMIT		(PAGE_SIZE - 80)
#	define		FOREVER				1
	int		NumBytesWritten;


	NumBytesWritten = 0;

	do
	{

		NumBytesWritten = sprintf(
			(ProcPage + NumBytesWritten),
			"VendorId = %s\n",
			gpAdapterList->VendorId);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"NeedReset = %d\n",
			gpAdapterList->NeedReset);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"Loading = %d\n",
			gpAdapterList->Loading);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"AutoDownload = %d\n",
			gpAdapterList->AutoDownload);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"MediaConnectStatus = %ld\n",
			gpAdapterList->MediaConnectStatus);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"Showtime = %d\n",
			gpAdapterList->Showtime);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"InterruptsInitialized = %d\n",
			gpAdapterList->InterruptsInitialized);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"InstanceNumber = %d\n",
			gpAdapterList->InstanceNumber);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"AdapterInitComplete = %d\n",
			gpAdapterList->AdapterInitComplete);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"DownloadInitComplete = %d\n",
			gpAdapterList->DownloadInitComplete);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.CommandStatus = %ld\n",
			gpAdapterList->PersistData.CommandStatus);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.DebugFlag = 0x%08lX\n",
			gpAdapterList->PersistData.DebugFlag);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.AutoSenseHandshake = %ld\n",
			gpAdapterList->PersistData.AutoSenseHandshake);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.AutoSenseWiresFirst = %ld\n",
			gpAdapterList->PersistData.AutoSenseWiresFirst);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.AutoWiringSelection = %ld\n",
			gpAdapterList->PersistData.AutoWiringSelection);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.AutoWiringRelayDelay = %ld\n",
			gpAdapterList->PersistData.AutoWiringRelayDelay);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.AutoWiringRelayEnrg = %ld\n",
			gpAdapterList->PersistData.AutoWiringRelayEnrg);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.AutoWiringNumCombos = %ld\n",
			gpAdapterList->PersistData.AutoWiringNumCombos);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.ARMHwId = 0x%08lX\n",
			gpAdapterList->PersistData.ARMHwId);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.ADSLHwId = 0x%08lX\n",
			gpAdapterList->PersistData.ADSLHwId);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.ARMFunctionIdentifier = %ld\n",
			gpAdapterList->PersistData.ARMFunctionIdentifier);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.ADSLFunctionIdentifier = %ld\n",
			gpAdapterList->PersistData.ADSLFunctionIdentifier);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

//		NumBytesWritten += sprintf(
//			(ProcPage + NumBytesWritten),
//			"PersistData.HwDebugFlag = 0x%08lX\n",
//			gpAdapterList->PersistData.HwDebugFlag);
//		if ( NumBytesWritten > BUFFER_LIMIT )
//		{
//			break;
//		}
//
		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.PhysicalDriverType = %ld\n",
			gpAdapterList->PersistData.PhysicalDriverType);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.GpioADSLFunctionMask = 0x%08lX\n",
			gpAdapterList->PersistData.GpioADSLFunctionMask);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.GpioARMFunctionMask = 0x%08lX\n",
			gpAdapterList->PersistData.GpioARMFunctionMask);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.GpioDirectionMask = 0x%08lX\n",
			gpAdapterList->PersistData.GpioDirectionMask);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.GpioEdgeMask = 0x%08lX\n",
			gpAdapterList->PersistData.GpioEdgeMask);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.CnxtVendorId = 0x%08lX\n",
			gpAdapterList->PersistData.CnxtVendorId);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.CnxtArmDeviceId = 0x%08lX\n",
			gpAdapterList->PersistData.CnxtArmDeviceId);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.CnxtAdslDeviceId = 0x%08lX\n",
			gpAdapterList->PersistData.CnxtAdslDeviceId);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		NumBytesWritten += sprintf(
			(ProcPage + NumBytesWritten),
			"PersistData.CnxtPhysicalDriverType = %d\n",
			gpAdapterList->PersistData.CnxtPhysicalDriverType);
		if ( NumBytesWritten > BUFFER_LIMIT )
		{
			break;
		}

		*bEndOfFile = TRUE;
		break;

	} while ( FOREVER );

	return NumBytesWritten;
}



/******************************************************************************
FUNCTION NAME:
	CmgmtGetApiList

ABSTRACT:
	Return List of commands for user API. The BdSupportedList must be an
	INORDER list.


RETURN:
	NDIS_STATUS_SUCCESS


DETAILS:
	The storage for BdSupportedList cannot be released until after
	Driver Adapter Inits are complete.  In this case, it is a static so it is
	not necessary to release.
******************************************************************************/
DWORD CmgmtGetApiList(
	VOID				* ExternHandle,
	DWORD				** MessageList,
	DWORD				* MaxMessages
	)
{
	static DWORD	ApiSupportedList[] =
		{
			BD_ADSL_GET_PERFORMANCE,
			BD_ADSL_INIT_DATA_PUMP,	
			BD_ADSL_GET_CONN_CONFIG,
			BD_ADSL_SET_CONN_CONFIG,
			BD_ADSL_GET_MODEM_DATA,	
			BD_ADSL_GET_DATA_PATH_SELECTION,
			BD_ADSL_SET_DATA_PATH_SELECTION,
			BD_ADSL_GET_LINE_STATUS,
			BD_ADSL_GET_LINE_STATE,	
			BD_ADSL_GET_DP_VERSIONS,
			BD_ADSL_GET_SPECIFIC_CMD_RESP,
			BD_ADSL_GET_BUS_CONTROLLER,	/* 0x00060011 */
			BD_ADSL_SET_TRACE,
			BD_ADSL_DEBUG,

			BD_CARDAL_GET_REGISTER,		/* 0x00070001 */
			BD_CARDAL_SET_REGISTER,
			BD_USER_ACTIVATE_LINE,
			BD_USER_DEACTIVATE_LINE,
			BD_CARDAL_GET_TEXT_LOG,
			BD_ADSL_GET_CONFIG,	
			BD_ADSL_SET_CONFIG,	

			BD_PM_GET_POWER_STATE,		/* 0x00090001 */
			BD_PM_SET_POWER_STATE,

			BD_CARDAL_GET_ADSL_TRANSCEIVER_STATUS,   /* 0x000A0005 */

			BD_ADSL_GET_ALARMS,				
			BD_ADSL_GET_SNR_TABLE,				
			BD_ADSL_GET_CONTROLLER_LOG_CLT,	
			BD_ADSL_SET_CONTROLLER_LOG_CLT,	
			BD_ADSL_GET_BIT_ALLOCATION_TABLE,	
			BD_ADSL_GET_GHS_CAP,				
			BD_ADSL_GET_GHS_LOCAL_CAP,

			BD_ADSL_SET_GHS_LOCAL_CAP,
			BD_ADSL_SET_AUTO_SENSE
		};

	*MaxMessages = sizeof(ApiSupportedList)/sizeof(ApiSupportedList[0]);
	*MessageList = (DWORD *)ApiSupportedList;
	return(STATUS_SUCCESS);
}		
