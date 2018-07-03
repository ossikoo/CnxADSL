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
**		????
**
**	FILE NAME:
**		LnxTools.h
**
**	ABSTRACT:
**		This file contains type definitions to ease the port from the windows
**		world to the Linux world.
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4_ATM_Driver/KernelModule/LnxTools.h $
**	$Revision: 7 $
**	$Date: 5/22/01 10:37a $
*******************************************************************************
******************************************************************************/

#ifndef _LNXTOOLS_H_
#define _LNXTOOLS_H_

#include "../LnxDefs.h"

#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/string.h>
#include <linux/delay.h>

#include "Version.h"

/* Call sequence for functions in dpcontroller.o.{proprietary,new} */

#define DPCCALL asmlinkage

extern ULONG gAllocCnt;

////////////////////////////////////////////////////////
// timers
////////////////////////////////////////////////////////
typedef struct timer_list LINUX_TIMER;

static __inline void  INIT_TIMER( IN LINUX_TIMER* pTimer,
                                  IN void (*pTimerFunction)(ULONG),
                                  IN ULONG pUserData )
{
	pTimer->data		= pUserData;
	pTimer->function	= pTimerFunction;

	init_timer( pTimer );
}

static __inline void CANCEL_TIMER( LINUX_TIMER* Tmr, BOOLEAN* Stat)
{
	int TimerStat;

	TimerStat = del_timer(	Tmr );

	if (TimerStat == 0)
		*Stat = FALSE;
	else
		*Stat = TRUE;
}

static __inline void START_TIMER( LINUX_TIMER* Tmr, ULONG msTime )
{
	Tmr->expires = jiffies + ((msTime*HZ) / 1000);
	add_timer(	Tmr );
}

///////////////////////////////////////////////////////
// spin locks
///////////////////////////////////////////////////////
typedef spinlock_t SPIN_LOCK;

#define INIT_SPIN_LOCK(pLock) spin_lock_init(pLock)
#define ACQUIRE_LOCK(pLock, flag) spin_lock_irqsave(pLock, flag)
#define RELEASE_LOCK(pLock, flag) spin_unlock_irqrestore(pLock, flag)
#define ACQUIRE_LOCK_AT_ISR(pLock) spin_lock(pLock)
#define RELEASE_LOCK_AT_ISR(pLock) spin_unlock(pLock)
#define FREE_SPIN_LOCK(pLock)

////////////////////////////////////////////////////////
// EVENTS
////////////////////////////////////////////////////////
//
//  This implements the manual set/reset event functions.
//
typedef struct _EVENT_HNDL_
{
	volatile int				   SetFlag;
	volatile int                WaitCnt;
	wait_queue_head_t  WaitQue;
	spinlock_t		   FlagLock;
} EVENT_HNDL;

void INIT_EVENT( EVENT_HNDL* pEvent );
int WAIT_EVENT( EVENT_HNDL* pEvent, ULONG msecDelay );
void SET_EVENT( EVENT_HNDL* pEvent ) ;
void SET_EVENT_FROM_ISR( EVENT_HNDL* pEvent ) ;
void RESET_EVENT( EVENT_HNDL* pEvent );
DPCCALL int vsprintf_dpc(char *buf, const char *fmt, va_list args);


///////////////////////////////////////////////////////////
// Sleep Function
///////////////////////////////////////////////////////////
static __inline void SLEEP( ULONG timems )
{
	current->state = TASK_INTERRUPTIBLE;
	schedule_timeout(timems*HZ/1000);
}

static __inline void MICRO_DELAY( ULONG MicroSecs )
{
	udelay( MicroSecs );
}

/////////////////////////////////////////////////////////
// Que handling
/////////////////////////////////////////////////////////
typedef struct _LIST_ENTRY_
{
	struct _LIST_ENTRY_ *pNext;
	struct _LIST_ENTRY_ *pPrev;
} LIST_ENTRY, *PLIST_ENTRY;

static __inline VOID InitializeListHead( PLIST_ENTRY pList )
{
	pList->pNext = pList;
	pList->pPrev = pList;
}

static __inline BOOL IsListEmpty( PLIST_ENTRY pList )
{
	return ( pList->pNext == pList );
}

static __inline PLIST_ENTRY RemoveTailList(PLIST_ENTRY pList)
{
	PLIST_ENTRY pRetList = pList->pPrev;

	if ( IsListEmpty( pList ) )
		return NULL;

	pRetList->pPrev->pNext = pList;
	pList->pPrev		   = pRetList->pPrev;

	pRetList->pNext = NULL;
	pRetList->pPrev = NULL;

	return pRetList;
}

static __inline PLIST_ENTRY RemoveHeadList(PLIST_ENTRY pList)
{
	PLIST_ENTRY pRetList = pList->pNext;

	if ( IsListEmpty( pList ) )
		return NULL;

	pRetList->pNext->pPrev = pList;
	pList->pNext		   = pRetList->pNext;

	pRetList->pNext = NULL;
	pRetList->pPrev = NULL;

	return pRetList;
}

static __inline void RemoveListEntry(PLIST_ENTRY pEntry)
{
	PLIST_ENTRY pNextEntry = pEntry->pNext;
	PLIST_ENTRY pPrevEntry = pEntry->pPrev;

	pPrevEntry->pNext = pNextEntry;
	pNextEntry->pPrev = pPrevEntry;

	pEntry->pNext = NULL;
	pEntry->pPrev = NULL;
}

static __inline VOID InsertHeadList(PLIST_ENTRY pList, PLIST_ENTRY pListEntry)
{
	// set the forward and back on the new entry
	pListEntry->pPrev		    = pList;
	pListEntry->pNext		    = pList->pNext;
	pListEntry->pNext->pPrev	= pListEntry;
	pList->pNext			    = pListEntry;
}

static __inline VOID InsertTailList(PLIST_ENTRY pList, PLIST_ENTRY pListEntry)
{
	// set the forward and back on the new entry
	pListEntry->pNext		    = pList;
	pListEntry->pPrev		    = pList->pPrev;
	pListEntry->pPrev->pNext	= pListEntry;
	pList->pPrev			    = pListEntry;
}

////////////////////////////////////////////////////////
// Kernel memory operations
////////////////////////////////////////////////////////
// This flag enables the memory allocation debugger - set it to
// nonzero to enable it
/* CLN!!! */
#ifndef MEM_ALLOC_DBG
#define MEM_ALLOC_DBG 0
#endif

#if MEM_ALLOC_DBG
typedef struct ALLOC_HEAD
{
	LIST_ENTRY	List;
	ULONG	    Magic; // number used to indicate allocated memory
	ULONG		Tag;
} ALLOC_HEAD_T, *PALLOC_HEAD_T;

#define ALLOC_MAGIC 0x01216560

extern ALLOC_HEAD_T gAllocList;
#endif 

static __inline 	NTSTATUS ALLOCATE_MEMORY(	OUT PVOID* pMem,
        IN  ULONG size,
        IN  ULONG tag )
{
	NTSTATUS Status = STATUS_SUCCESS;

	// if in memory debug keep a list of all allocated memory
	// blocks to help chase down leaks
#if MEM_ALLOC_DBG
	UCHAR 		 *pAll;
	PALLOC_HEAD_T pAllocEntry;

	pAll= kmalloc (size+sizeof(ALLOC_HEAD_T), GFP_KERNEL);

	if (!pAll)
	{
		*pMem = NULL;
		printk("<1> memory allocation failure\n");
		Status = -ENOMEM;
	}

	else
	{
		pAllocEntry = (PALLOC_HEAD_T)pAll;
		pAllocEntry->Magic = ALLOC_MAGIC;
		InsertTailList(&gAllocList.List, (PLIST_ENTRY)&pAllocEntry->List);
		pAllocEntry->Tag = tag;

		*pMem = pAll+sizeof(ALLOC_HEAD_T);
		gAllocCnt++;
	}

#else

// if not in memory debug then just allocate
	*pMem = kmalloc (size, GFP_ATOMIC);

	if (!*pMem)
	{
		printk("<1> memory allocation failure\n");
		Status = -ENOMEM;
	}
	else
		gAllocCnt++;

#endif

	return Status;
}

static __inline VOID FREE_MEMORY( IN PVOID pMem,
                                  IN ULONG Length,
                                  IN ULONG Flags )
{
	// if in memory debug then validate the block before freeing it
#if MEM_ALLOC_DBG
	PALLOC_HEAD_T pAllocEntry=(PALLOC_HEAD_T)((ULONG)pMem-sizeof(ALLOC_HEAD_T));

	// validate that it was allocated memory
	if ( pAllocEntry->Magic != ALLOC_MAGIC )
	{
		printk("<1>CnxADSL attempting to free invalid memory block %lx\n",(DWORD)pMem);
		return;
	}

	// clear the magic number to catch extra frees on the same block
	pAllocEntry->Magic = 0;

	// now take it out of the alloc list
	RemoveListEntry( &pAllocEntry->List );

	// and free it
	kfree( pAllocEntry );

#else
kfree( pMem );
#endif

	gAllocCnt--;
}

#define CLEAR_MEMORY( pMem, size ) memset( pMem, 0, size )

#define MEMORY_SET( pMem, value, size ) memset( pMem, value, size )

#define COPY_MEMORY( pDest, pSrc, size ) memcpy( pDest, pSrc, size )


////////////////////////////////////////////////////////
// ATM support functions
////////////////////////////////////////////////////////
#define RELEASE_SKB( pVcc, skb )	\
{									\
	if (pVcc->pop)					\
		pVcc->pop(pVcc,skb);		\
	else							\
		dev_kfree_skb(skb);			\
}

// Union to be used in function return type forcing returning struct
// through extra pointer argument instead of in register
#define MAKE_RETURN_TYPE(t) typedef union{t return_value;char pad[9];} t##_RETURN;

#endif //_LNXTOOLS_H_
