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
**		CardAl.c
**
**	ABSTRACT:
**		Card Abstraction Layer
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/CardAL.c $
**	$Revision: 2 $
**	$Date: 1/22/01 11:38a $
*******************************************************************************
******************************************************************************/
#define _CARDAL_C_	1
#define		MODULE_ID			DBG_ENABLE_CARDAL

#include "Product.h"
#include "CardMgmt.h"
#include "CardALV.h"
#include "SmSysIf.h"

#ifndef MEMORY_TAG
#define	MEMORY_TAG		'DXNC'
#endif

/*******************************************************************************
Private Data Types
*******************************************************************************/

typedef struct CARDAL_CONTEXT_S
{
	void 						* AdapterContext;					// Global Adapter Context from CardAlInit
	MODULE_LINE_CARD_METHODS_T	LineCard[ CDAL_LINE_END ];			// PC Card with External Line Interface
	MODULE_CARDAL_OPTIONAL_T	CardAlModule[ CDAL_OPTIONAL_END - CDAL_LINE_END ];
	VOID						* ModuleContext[ CDAL_MODULES_END ];
} CARDAL_CONTEXT_T;
#define NULL_CARDAL_CONTEXT	((CARDAL_CONTEXT_T *)0)


/*******************************************************************************
Module Private Function Prototypes
*******************************************************************************/
LOCAL INLINE NDIS_STATUS cdalInitModules(
    CARDAL_CONTEXT_T 	* pCardAl
);

LOCAL INLINE NDIS_STATUS cdalEnableModules(
    CARDAL_CONTEXT_T 	* pCardAl
);
/*******************************************************************************
Module Public Functions
*******************************************************************************/


/******************************************************************************
*	FUNCTION NAME:
*		CardALCfgInit
*
*	ABSTRACT:
*		Requirement of the Architecture to read tuning parameters from a
*		Windowsxx Registry for the CardAL module registry parameters.
*
*	RETURN:
*		NDIS_STATUS
******************************************************************************/
NDIS_STATUS CardALCfgInit(
    IN	CDSL_ADAPTER_T	 *pThisAdapter,
    IN	PTIG_USER_PARAMS pParams  //    parameter struct from the download app
)
{
	NDIS_STATUS			Status=STATUS_SUCCESS;

	// read debug flag from registry cache into adapter var
	pThisAdapter->DebugFlag = pThisAdapter->PersistData.DebugFlag ;

	return Status;
}

/******************************************************************************
FUNCTION NAME:
	CardAlInit

ABSTRACT:
	CardAl Initialization of context structure. All CardAL child modules
	are initialized in a two stage process. First the Init function is called.
	If all goes well, the Enable function will then be called.

	* Note: All Child functions are optional


RETURN:
	Pointer to CardAl Context


DETAILS:
******************************************************************************/
NDIS_STATUS CardALAdapterInit(
    CDSL_ADAPTER_T		* pThisAdapter	)
{
	NDIS_STATUS			Status;
	CARDAL_CONTEXT_T 	* pCardAl = NULL_CARDAL_CONTEXT;

	//*
	//* Allocate Parent CardAl Context		Init Phase 0
	//*
	//*
	Status = ALLOCATE_MEMORY( (PVOID)&pCardAl, sizeof(CARDAL_CONTEXT_T), MEMORY_TAG );
	if(Status != STATUS_SUCCESS)
	{
		NdisWriteErrorLogEntry(
		    pThisAdapter->MiniportAdapterHandle,
		    NDIS_ERROR_CODE_OUT_OF_RESOURCES,
		    4,
		    NdisStatus,
		    __FILEID__,
		    __LINE__,
		    RSS_INIT_ERROR_FAILED_ALLOCATE_CARDAL_ADPTR );
		return(STATUS_FAILURE);
	}
	CLEAR_MEMORY((PVOID)pCardAl, sizeof(CARDAL_CONTEXT_T));

	//*
	//* Sanity Check
	//*
	#ifdef DBG
	if (sizeof(pCardAl->LineCard) != sizeof(gLineCards)
	        || sizeof(pCardAl->CardAlModule) != sizeof(gOptCardModules)
	        || (MAX_LINE_CARDS + MAX_OPT_CARD_MODS) != CDAL_MODULES_END)
	{
		//* This is a build problem. The Global arrays of functions must match
		//* the pCardAl context size. The summation of the number of arrays must equal the
		//* number of context pointers.
		ErrPrt("Failed cardAL context size check\n");
		FREE_MEMORY(pCardAl,sizeof(CARDAL_CONTEXT_T),0);
		return(STATUS_FAILURE);
	}
	#endif

	//*
	//* Exchanged pointers with public context structure
	//*
	SET_MODULE_CONTEXT(pThisAdapter, pCardAl, IF_CARDAL_ID)
	pCardAl->AdapterContext = pThisAdapter;

	//*
	//* Load CardAl Module Entry Points
	//*
	COPY_MEMORY(	&(pCardAl->LineCard[0]),
	             &gLineCards[0],
	             sizeof(pCardAl->LineCard));

	COPY_MEMORY(	&(pCardAl->CardAlModule[0]),
	             &gOptCardModules[0],
	             sizeof(pCardAl->CardAlModule));

	//*
	//* Initialize and enable all modules
	//*
	Status = cdalInitModules(pCardAl);
	if(Status != STATUS_SUCCESS)
	{
		CardALAdapterHalt(pThisAdapter);
		return(Status);
	}

	Status = cdalEnableModules(pCardAl);
	if(Status != STATUS_SUCCESS)
	{
		CardALAdapterHalt(pThisAdapter);
		return(Status);
	}

	//*
	//* Successful Initialization Complete
	//*
	return(STATUS_SUCCESS);
}

/******************************************************************************
FUNCTION NAME:
	CardAlShutdown

ABSTRACT:
	Release all resources allocated by CardAlInit.  Processing order is reversed
	from Init.


RETURN:
	VOID


DETAILS:
******************************************************************************/
NDIS_STATUS CardALAdapterHalt(
    CDSL_ADAPTER_T		* pThisAdapter
)
{
	CARDAL_CONTEXT_T 	* pCardAl;
	DWORD 				Count;


	if(pThisAdapter == NULL)
	{
		return(STATUS_FAILURE);
	}

	GET_MODULE_CONTEXT(pCardAl, pThisAdapter, IF_CARDAL_ID);
	if(pCardAl == NULL)
	{
		return(STATUS_FAILURE);
	}

	//*
	//* Release Optional Modules
	//*
	for (Count = CDAL_OPTIONAL_START; Count <  CDAL_OPTIONAL_END; Count++)
	{
		if(pCardAl->CardAlModule[Count - CDAL_OPTIONAL_START].Shutdown)
		{
			( * pCardAl->CardAlModule[Count - CDAL_OPTIONAL_START].Shutdown)(pCardAl->ModuleContext[Count] );
			if((pCardAl->ModuleContext[Count]) == NULL)
			{
				CardALAdapterHalt(pThisAdapter);
				return(STATUS_FAILURE);
			}
		}
	}

	//*
	//* Release Line Cards
	//*
	for (Count = CDAL_MODULES_START; Count <  CDAL_LINE_END; Count++)
	{
		if(pCardAl->LineCard[Count].Shutdown)
		{
			( * pCardAl->LineCard[Count].Shutdown)(pCardAl->ModuleContext[Count] ) ;
			if((pCardAl->ModuleContext[Count]) == NULL)
			{
				CardALAdapterHalt(pThisAdapter);
				return(STATUS_FAILURE);
			}
		}
	}

	//*
	//* Release Primary Context
	//*
	SET_MODULE_CONTEXT(pThisAdapter, NULL, IF_CARDAL_ID);
	FREE_MEMORY(pCardAl,sizeof(CARDAL_CONTEXT_T),0);
	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	CDALSetLineState

ABSTRACT:
	Set Line State.  This function is used to enable or disable a line.


RETURN:
	NTSTATUS


DETAILS:
******************************************************************************/
NDIS_STATUS CDALSetLineState(
    CDSL_ADAPTER_T		* pThisAdapter,
    CARD_LINE_STATE_T	NewState
)
{
	NDIS_STATUS			Status = STATUS_SUCCESS;
	CARDAL_CONTEXT_T 	* pCardAl;
	DWORD				Count;

	GET_MODULE_CONTEXT(pCardAl, pThisAdapter, IF_CARDAL_ID);
	if(pCardAl == NULL)
	{
		return(STATUS_FAILURE);
	}

	for (
	    Count = CDAL_MODULES_START;
	    Count <  CDAL_LINE_END && Status == STATUS_SUCCESS;
	    Count++
	)
	{
		if(pCardAl->LineCard[Count].SetLineState)
		{
			Status = ( * pCardAl->LineCard[Count].SetLineState)(pCardAl->ModuleContext[Count], NewState);
		}
	}
	return(Status);
}


/*******************************************************************************
CARDAL Module Functions
*******************************************************************************/

/******************************************************************************
FUNCTION NAME:
	CDALGetModuleHandle

ABSTRACT:
	Return Handle of CardAL Module as related to the external 
	pThisAdapter handle


RETURN:
	Handle of requested module or Null if failure


DETAILS:
******************************************************************************/
VOID * CDALGetModuleHandle(
    CDSL_ADAPTER_T * pThisAdapter,
    CDAL_MODULES_T CDALModule)
{
	CARDAL_CONTEXT_T 	* pCardAl;

	if(pThisAdapter == NULL
	        || CDALModule >= CDAL_MODULES_END
	  )
	{
		return(NULL);
	}

	GET_MODULE_CONTEXT(pCardAl, pThisAdapter, IF_CARDAL_ID);
	if(pCardAl == NULL)
	{
		return(NULL);
	}
	return(pCardAl->ModuleContext[CDALModule]);
}



/*******************************************************************************
Private Functions
*******************************************************************************/


/******************************************************************************
FUNCTION NAME:
	cdalInitModules

ABSTRACT:
	Call Init functions for all CardAl Modules


RETURN:
	NDIS_STATUS:


DETAILS:
******************************************************************************/
LOCAL INLINE NDIS_STATUS cdalInitModules(
    CARDAL_CONTEXT_T 	* pCardAl
)
{
	DWORD	Count;

	for (Count = CDAL_MODULES_START; Count <  CDAL_LINE_END; Count++)
	{
		if(pCardAl->LineCard[Count].Init)
		{
			pCardAl->ModuleContext[Count] = ( * pCardAl->LineCard[Count].Init)(pCardAl->AdapterContext,pCardAl);
			if((pCardAl->ModuleContext[Count]) == NULL)
			{
				return(STATUS_FAILURE);
			}
		}
	}

	for (Count = CDAL_OPTIONAL_START; Count <  CDAL_OPTIONAL_END; Count++)
	{
		if(pCardAl->CardAlModule[Count - CDAL_OPTIONAL_START].Init)
		{
			pCardAl->ModuleContext[Count] = ( * pCardAl->CardAlModule[Count - CDAL_OPTIONAL_START].Init)(pCardAl->AdapterContext,pCardAl);
			if((pCardAl->ModuleContext[Count]) == NULL)
			{
				return(STATUS_FAILURE);
			}
		}
	}

	return(STATUS_SUCCESS);
}


/******************************************************************************
FUNCTION NAME:
	cdalEnableModules

ABSTRACT:
	Call Enable functions for all CardAl Modules


RETURN:
	NDIS_STATUS:


DETAILS:
******************************************************************************/
LOCAL INLINE NDIS_STATUS cdalEnableModules(
    CARDAL_CONTEXT_T 	* pCardAl
)
{
	DWORD 			Count;
	NDIS_STATUS		Status;

	for (Count = CDAL_MODULES_START; Count <  CDAL_LINE_END; Count++)
	{
		if(pCardAl->LineCard[Count].Enable)
		{
			Status = ( * pCardAl->LineCard[Count].Enable)(pCardAl->ModuleContext[Count]);
			if(Status != STATUS_SUCCESS)
			{
				return(STATUS_FAILURE);
			}
		}
	}

	for (Count = CDAL_OPTIONAL_START; Count <  CDAL_OPTIONAL_END; Count++)
	{
		if(pCardAl->CardAlModule[Count - CDAL_OPTIONAL_START].Enable)
		{
			Status = ( * pCardAl->CardAlModule[Count - CDAL_OPTIONAL_START].Enable)(pCardAl->ModuleContext[Count]);
			if(Status != STATUS_SUCCESS)
			{
				return(STATUS_FAILURE);
			}
		}
	}

	return(STATUS_SUCCESS);
}


/***************************************************************************
FUNCTION NAME:
	CardAlRegStoreHeadEnd

ABSTRACT:
	
	This function is not implemented for Linux.

RETURN:
	none


DETAILS:
******************************************************************************/
DPCCALL NDIS_STATUS CardAlRegStoreHeadEnd( DWORD VendorId )
{
	return(STATUS_SUCCESS);
}
