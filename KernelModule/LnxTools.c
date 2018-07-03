#include "LnxTools.h"

// Check for compile time options
#ifndef OS_LINUX
#error "OS_LINUX not defined, this driver only works for linux"
#endif

#ifndef MODULE
#error "This driver can only be compiled as a module"
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
#error "This driver ONLY works with linux kernel 2.6.27 and up"
#endif

#ifndef CONFIG_ATM
#ifndef CONFIG_ATM_MODULE
#error "Kernel has no ATM support: you will not be able to load this module"
#endif
#endif

#ifndef CONFIG_PPP
#ifndef CONFIG_PPP_MODULE
#warning "Kernel has no PPP support: your ADSL line almost certainly requires this"
#endif
#endif

#ifndef CONFIG_PPPOATM
#ifndef CONFIG_PPPOATM_MODULE
#warning "Kernel has no PPP over ATM support: your ADSL line almost certainly requires this"
#endif
#endif

#ifdef CONFIG_PREEMPT
#warning "Kernel is configured with pre-emptive scheduling: this module may not work with it"
#endif

void INIT_EVENT( EVENT_HNDL* pEvent )
{
	pEvent->SetFlag = 0;
	pEvent->WaitCnt = 0;
	init_waitqueue_head( &pEvent->WaitQue);
	INIT_SPIN_LOCK( &pEvent->FlagLock );
}

int WAIT_EVENT( EVENT_HNDL* pEvent, ULONG msecDelay )
 { 
 int TimeRemain;
 //unsigned long flags ;
 DWORD LockFlag;

 // wait on the queue
 TimeRemain = (msecDelay * HZ) / 1000;

 // if no wait time then just return failure
 if (! msecDelay)
  return 0;

 // atomically sleep if event is not set - we must protect the window between
 // finding out if the flag is already set and sleeping as if an interrupt
 // occurs in the window and calls SET_EVENT, SET_EVENT will signal the wait
 // queue before we get placed on it and we will miss this signal.
 ACQUIRE_LOCK( &pEvent->FlagLock, LockFlag );
// local_irq_save(flags);

 // indicate that I am waiting
 pEvent->WaitCnt++;

 if (! pEvent->SetFlag)
  {
    //while (!pEvent->SetFlag && TimeRemain)
    //{
   // safe to call sleep with interrupts off as kernel will re-enable
   // interrupts after placing us on the queue and before calling schedule
   RELEASE_LOCK( &pEvent->FlagLock, LockFlag );
   //TimeRemain = interruptible_sleep_on_timeout ( &pEvent->WaitQue,  TimeRemain );
   TimeRemain = wait_event_interruptible_timeout( pEvent->WaitQue, pEvent->SetFlag, TimeRemain);
   ACQUIRE_LOCK( &pEvent->FlagLock, LockFlag );
   //}
  }
 // remove wait indication
 pEvent->WaitCnt--;

 RELEASE_LOCK( &pEvent->FlagLock, LockFlag );
// local_irq_restore(flags) ;

 // return setflag
 return pEvent->SetFlag;
 }

void SET_EVENT( EVENT_HNDL* pEvent )
{
	DWORD LockFlag;

	// if the event is cleared then
	// set the flag, wake up all waiting tasks.
	ACQUIRE_LOCK( &pEvent->FlagLock, LockFlag );
	if ( !pEvent->SetFlag )
	{
		int cnt;
		pEvent->SetFlag = TRUE;
		for  ( cnt=0; cnt<pEvent->WaitCnt; cnt++ )
		{
			wake_up_interruptible ( &pEvent->WaitQue  );
		}
	}
	RELEASE_LOCK( &pEvent->FlagLock, LockFlag );
}

void SET_EVENT_FROM_ISR( EVENT_HNDL* pEvent )
{
	// if the event is cleared then
	// set the flag, wake up all waiting tasks.
	ACQUIRE_LOCK_AT_ISR( &pEvent->FlagLock );
	if ( !pEvent->SetFlag )
	{
		int cnt;
		pEvent->SetFlag = TRUE;
		for  ( cnt=0; cnt<pEvent->WaitCnt; cnt++ )
		{
			wake_up_interruptible ( &pEvent->WaitQue  );
		}
	}
	RELEASE_LOCK_AT_ISR( &pEvent->FlagLock );
}

void RESET_EVENT( EVENT_HNDL* pEvent )
{
	DWORD LockFlag;

	// clear the event flag
	ACQUIRE_LOCK( &pEvent->FlagLock, LockFlag );
	pEvent->SetFlag = FALSE;

	// empty out the queue
	while ( waitqueue_active( &pEvent->WaitQue ) )
	  wait_event_interruptible( pEvent->WaitQue, pEvent->SetFlag );

	RELEASE_LOCK( &pEvent->FlagLock, LockFlag );
}

DPCCALL int vsprintf_dpc(char *buf, const char *fmt, va_list args)
{
	return vsprintf(buf, fmt, args);
}
