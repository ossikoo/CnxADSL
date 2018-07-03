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
**		ChipAl
**
**	FILE NAME:
**		chipalInterf.h
**
**	ABSTRACT:
**		ChipAl Exports
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4_ATM_Driver/KernelModule/ChipALInterf.h $
**	$Revision: 2 $
**	$Date: 5/22/01 10:38a $
*******************************************************************************
******************************************************************************/
#ifndef _CHIPALINTERF_H_
#define _CHIPALINTERF_H_ 

#include "ChipALCommon.h"			//* Exported Data Definitions

#define CHIPAL_INTERFACE_GUID { /* 54ce16e0-acd1-11d3-ae4a-00500462fa59 */	\
    0x54ce16e0,																\
    0xacd1,																	\
    0x11d3,																	\
    {0xae, 0x4a, 0x00, 0x50, 0x04, 0x62, 0xfa, 0x59}						\
  };

#define CHIPAL_LINK_GUID { /* 28f082e0-ce6d-11d3-ae4e-00500462fa59 */		\
    0x28f082e0,																\
    0xce6d,																	\
    0x11d3,																	\
    {0xae, 0x4e, 0x00, 0x50, 0x04, 0x62, 0xfa, 0x59}						\
  };

//* CHIPAL_CONFIG_GUID is a Legacy GUID that should NOT be used in new
//* implementations. It is currently required for the PairGain Controller
//* code. It may not be provided in future products.

#define CHIPAL_CONFIG_GUID { /* c0626d50-c82f-11d3-ae4d-00500462fa59 */		\
    0xc0626d50,																\
    0xc82f,																	\
    0x11d3,																	\
    {0xae, 0x4d, 0x00, 0x50, 0x04, 0x62, 0xfa, 0x59}						\
  };

// IRP_MN_QUERY_INTERFACE code and structures

typedef struct CHIPAL_INTERFACE
{
	INTERFACE iface;

	//CalInit is performed during QUERY_INTERFACE
	// CalShutdown is called when last reference is deleted
	// by calling InterfaceDereference()

	// UserContext is stored in Context
	// interface functions:

	CHIPAL_FAILCODE_T (* pCALRead)(VOID * UserContext,
	                               IN	HARDWARE_DEVICE_T	HwDevice,
	                               IN	DWORD			LogicalDevice,
	                               IN	DWORD			OffSetAddress,
	                               IN	DWORD			RegisterLength,
	                               OUT	PVOID 			 pValue );

	CHIPAL_FAILCODE_T ( * pCALWrite)(VOID * UserContext,
	                                 HARDWARE_DEVICE_T		HwDevice,
	                                 DWORD				LogicalDevice,
	                                 DWORD				OffsetAddress,
	                                 DWORD				RegisterLength,
	                                 DWORD				Value,
	                                 DWORD				Mask);


	VOID (* pCalEnableInterrupt)(VOID * UserContext,
	                             IN	DWORD		Interrupt);
	VOID (* pCalDisableInterrupt)(VOID * UserContext,
	                              IN	DWORD		Interrupt);

	CHIPAL_FAILCODE_T (* pCalAddEventHandler)(VOID * UserContext,
	        IN  DWORD				FunctSel,
	        IN 	CHIPAL_EVENT_T		Event,
	        IN 	HW_EVENT_TYPE_T		IrqLevel,
	        IN 	CHIPAL_EVENT_HANDLER_T	EventHandler,
	        IN 	VOID				* EventContext);

	CHIPAL_FAILCODE_T (* pCalRemoveEventHandler)(VOID * UserContext,
	        IN  DWORD               FunctSel,
	        IN 	CHIPAL_EVENT_T		Event,
	        IN 	HW_EVENT_TYPE_T		IrqLevel,
	        IN 	CHIPAL_EVENT_HANDLER_T	EventHandler);

	VOID (* pCalSetEvent)(VOID * UserContext,
	                      IN    DWORD               FunctSel,
	                      IN 	CHIPAL_EVENT_T		Event);

} CHIPAL_INTERFACE, *PCHIPAL_INTERFACE;

//* Begin definition of ChipAl GetConfig Interface
typedef struct CHIPAL_LINK_INTERFACE_S
#ifdef __cplusplus
: public _INTERFACE
#endif
{
	//* Reserve storage space for INTERFACE header
	#ifndef __cplusplus
	INTERFACE iface;
	//typedef struct _INTERFACE {
	//USHORT Size;
	//USHORT Version;
	//PVOID Context;
	//PINTERFACE_REFERENCE InterfaceReference;
	//PINTERFACE_DEREFERENCE InterfaceDereference;
	// interface specific entries go here
	//} INTERFACE, *PINTERFACE;
	#endif

	NTSTATUS (* pCalLinkInit)(
	    IN VOID		 			* UserContext,
	    IN VOID					* LinkHandle,
	    IN CAL_LINK_CFG_T		* pLinkConfig
	);

	NTSTATUS (* pCalLinkShutDown)(VOID * UserContext,
	                              IN	DWORD		ComChannel);
	NTSTATUS (* pCalLinkEnable)(VOID * UserContext,
	                            IN	DWORD		ComChannel);
	NTSTATUS (* pCalLinkDisable)(VOID * UserContext,
	                             IN	DWORD		ComChannel);
	NTSTATUS (* pCalStartTx)(VOID * UserContext,
	                         IN	DWORD		ComChannel);

} CHIPAL_LINK_INTERFACE_T, *PCHIPAL_LINK_INTERFACE;


//* Begin definition of ChipAl GetConfig Interface
typedef struct CHIPAL_CONFIG_INTERFACE_S
#ifdef __cplusplus
: public _INTERFACE
#endif
{
	//* Reserve storage space for INTERFACE header
	#ifndef __cplusplus
	INTERFACE iface;
	//typedef struct _INTERFACE {
	//USHORT Size;
	//USHORT Version;
	//PVOID Context;
	//PINTERFACE_REFERENCE InterfaceReference;
	//PINTERFACE_DEREFERENCE InterfaceDereference;
	// interface specific entries go here
	//} INTERFACE, *PINTERFACE;
	#endif

	CHIPAL_FAILCODE_T (* pCalGetConfig) (
	    VOID 					* pChipAl,
	    VOID				* Config
	);

	CHIPAL_FAILCODE_T (* pCalSetConfig) (
	    VOID	 				* pChipAl,
	    VOID				* Config
	);

} CHIPAL_CONFIG_INTERFACE_T;


#endif 	// File Wrapper
