/******************************************************************************
*******************************************************************************
****	Copyright (c) 1997, 1998
****	Rockwell Semiconductor Systems
****	Personal Computing Division
****	All Rights Reserved
****
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****
*******************************************************************************
**
**	MODULE NAME:
**		Util (Utilities)
**
**	FILE NAME:
**		UtilTmr.c
**
**	ABSTRACT:
**		Timer routines.
**
**	DETAILS:
**		If the function of the expired timer calls
**		UtilTmrClearTmr then calls timer start then the current timer block will
**		be freed and timer start may use it to schedule the new timer
**		which ofcourse will be freed when the function returns true.
**		Quick fix: use active timer to not clear currently active timer.
**		Once the code is re written to use linked lists then all side effects
**		will be fixed.
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/UtilTmr.c $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/

//	Unique file ID for error logging
#define		__FILEID__		249


#include "CardMgmtV.h"



///////////////////////////////////////////////////////////////////////////////
//Functions:
///////////////////////////////////////////////////////////////////////////////



/******************************************************************************
*	FUNCTION NAME:
*		
*
*	ABSTRACT:
*		Start the specified timer (one of the timers in the timer info block).
*
*	RETURN:
*		Boolean TRUE or FALSE.
******************************************************************************/
BOOLEAN UtilTmrStartTmr(
    TIMER_INFO_BLOCK	* pThisInstance,
    void				* RefOne,
    DWORD				RefTwo,
    DWORD				msecTime,
    BOOLEAN				(* pActionRtn)(void * Param),
    void				* Param )
{
	DWORD		i;

	//	find an empty spot. It will not be that of the active timer since function can not be NULL
	for ( i = 0;   i < pThisInstance->NumTimers;   i++ )
	{
		if ( pThisInstance->TimerInfo[i].pActionRtn == NULL )
			break;
	}

	if( i == pThisInstance->NumTimers )
	{
		return FALSE;
	}

	pThisInstance->TimerInfo[i].pActionRtn = pActionRtn;
	pThisInstance->TimerInfo[i].Param = Param;
	pThisInstance->TimerInfo[i].msecTime = msecTime;
	pThisInstance->TimerInfo[i].RefOne = RefOne;
	pThisInstance->TimerInfo[i].RefTwo = RefTwo;
	pThisInstance->TimerInfo[i].RestartCount = 0;

	//	Increment count of active timers if this index is beyond
	//	current count of active timers
	if ( i + 1  >  pThisInstance->TimersActive )
	{
		pThisInstance->TimersActive = i + 1;
	}

	return TRUE;
}



/******************************************************************************
*	FUNCTION NAME:
*		UtilTmrClearTmr
*
*	ABSTRACT:
*		Clear the specified timer (one of the timers in the timer info block).
*
*	RETURN:
*		Boolean TRUE or FALSE.
*
*	DETAILS:
******************************************************************************/
BOOLEAN UtilTmrClearTmr(
    TIMER_INFO_BLOCK	* pThisInstance,
    void				* RefOne,
    DWORD				RefTwo )
{
	DWORD	i;
	int		x;

	//find the specified timer
	for ( i = 0;   i < pThisInstance->NumTimers;   i++ )
	{
		if
		(
		    (pThisInstance->TimerInfo[i].RefOne == RefOne)
		    &&
		    (pThisInstance->TimerInfo[i].RefTwo==RefTwo)
		    &&
		    (i != pThisInstance->ActiveTimer)
		)	//may not clear timer whose function has just been called.
			break;
	}

	if ( i == pThisInstance->NumTimers )
	{
		return FALSE;
	}

	pThisInstance->TimerInfo[i].pActionRtn = NULL;
	pThisInstance->TimerInfo[i].Param = NULL;
	pThisInstance->TimerInfo[i].msecTime = 0;
	pThisInstance->TimerInfo[i].RefOne = NULL;
	pThisInstance->TimerInfo[i].RefTwo = 0;
	pThisInstance->TimerInfo[i].RestartCount = 0;

	if ( i  ==  pThisInstance->TimersActive - 1 )
	{
		// Loop backward to find next active timer
		for ( x = i - 1;   x >= 0;   x-- )
		{
			if ( pThisInstance->TimerInfo[x].pActionRtn != NULL )
			{
				break;
			}
		}
		pThisInstance->TimersActive = x + 1;
	}
	return TRUE;
}



/******************************************************************************
*	FUNCTION NAME:
*		UtilTmrClearAllTmrs
*
*	ABSTRACT:
*		Clear all timers (in the timer info block).
*
*	RETURN:
*		Boolean TRUE or FALSE.
*
*	DETAILS:
******************************************************************************/
BOOLEAN UtilTmrClearAllTmrs(
    TIMER_INFO_BLOCK	* pThisInstance,
    void				* RefOne )
{
	DWORD	i;
	DWORD	count = 0;
	int		x;

	//find the specified timer
	for ( i = 0;   i < pThisInstance->NumTimers;   i++ )
	{
		if
		(
		    (pThisInstance->TimerInfo[i].RefOne == RefOne)
		    &&
		    (i != pThisInstance->ActiveTimer)
		)	//may not clear timer whose function has just been called.
		{
			pThisInstance->TimerInfo[i].pActionRtn = NULL;
			pThisInstance->TimerInfo[i].Param = NULL;
			pThisInstance->TimerInfo[i].msecTime = 0;
			pThisInstance->TimerInfo[i].RefOne = NULL;
			pThisInstance->TimerInfo[i].RefTwo = 0;
			pThisInstance->TimerInfo[i].RestartCount = 0;

			count++;
		}
	}

	if ( count == 0 )
	{
		return FALSE;
	}

	// Loop backward to find last active timer
	for ( x = pThisInstance->TimersActive - 1;   x >= 0;   x-- )
	{
		if ( pThisInstance->TimerInfo[x].pActionRtn != NULL )
		{
			break;
		}
	}
	pThisInstance->TimersActive = x + 1;

	return TRUE;
}



/******************************************************************************
*	FUNCTION NAME:
*		UtilTmrReStartTmr
*
*	ABSTRACT:
*		Restart the specified timer (one of the timers in the timer info block).
*
*	RETURN:
*		Boolean TRUE or FALSE.
*
*	DETAILS:
******************************************************************************/
BOOLEAN UtilTmrReStartTmr(
    TIMER_INFO_BLOCK	* pThisInstance,
    void				* RefOne,
    DWORD				RefTwo,
    DWORD				msecTime )
{
	DWORD		i;

	//	find the specified timer
	for ( i = 0;   i < pThisInstance->NumTimers;   i++ )
	{
		if
		(
		    (pThisInstance->TimerInfo[i].RefOne == RefOne)
		    &&
		    (pThisInstance->TimerInfo[i].RefTwo==RefTwo)
		)
		{
			break;
		}
	}

	if( i == pThisInstance->NumTimers )
	{
		return FALSE;
	}

	pThisInstance->TimerInfo[i].msecTime = msecTime;
	pThisInstance->TimerInfo[i].RestartCount++;

	return TRUE;
}



/******************************************************************************
*	FUNCTION NAME:
*		UtilTmrGetTmrReStartCount
*
*	ABSTRACT:
*		Get the restart count for the specified timer
*		(one of the timers in the timer info block).
*
*	RETURN:
*		DWORD, 0 if invalid timer specified, otherwise the restart count of
*		the timer.
*
*	DETAILS:
******************************************************************************/
DWORD UtilTmrGetTmrReStartCount(
    TIMER_INFO_BLOCK	* pThisInstance,
    void				* RefOne,
    DWORD				RefTwo )
{
	DWORD		i;

	//	find the specified timer
	for ( i = 0;   i < pThisInstance->NumTimers;   i++ )
	{
		if
		(
		    (pThisInstance->TimerInfo[i].RefOne == RefOne)
		    &&
		    (pThisInstance->TimerInfo[i].RefTwo == RefTwo)
		)
		{
			break;
		}
	}

	if( i == pThisInstance->NumTimers )
	{
		return 0;
	}

	return pThisInstance->TimerInfo[i].RestartCount;
}



/******************************************************************************
*	FUNCTION NAME:
*		UtilTmrIsTmrActive
*
*	ABSTRACT:
*		Test to see if the timer specified (one of the timers in the timer
*		info block) is active (running).
*
*	RETURN:
*		Boolean TRUE or FALSE.
*
*	DETAILS:
******************************************************************************/
BOOLEAN UtilTmrIsTmrActive(
    TIMER_INFO_BLOCK	* pThisInstance,
    void				* RefOne,
    DWORD				RefTwo )
{
	DWORD		i;

	//	find the specified timer
	for ( i = 0;   i < pThisInstance->NumTimers;   i++ )
	{
		if
		(
		    (pThisInstance->TimerInfo[i].RefOne == RefOne)
		    &&
		    (pThisInstance->TimerInfo[i].RefTwo == RefTwo)
		)
		{
			break;
		}
	}

	if( i == pThisInstance->NumTimers )
	{
		return FALSE;
	}

	if ( pThisInstance->TimerInfo[i].msecTime > 0 )
	{
		return TRUE;
	}

	return FALSE;
}



/******************************************************************************
*	FUNCTION NAME:
*		UtilTmrTmrBackground
*
*	ABSTRACT:
*		Background timer 'task', called every timer interrupt period (by
*		MainTimerInterrupt) to decrement all of the timers defined (in the
*		timer info block) and active.  If an active timer has expired, then
*		its 'action return' function is executed.
*
*	RETURN:
*
*	DETAILS:
******************************************************************************/
void UtilTmrTmrBackground(
    TIMER_INFO_BLOCK	* pThisInstance,
    DWORD				msecElapsed )
{
	DWORD	i,
	count;
	int		x;

	//	find active timers
	count = pThisInstance->TimersActive;

	for ( i = 0;   i < pThisInstance->NumTimers;   i++ )
	{
		if ( pThisInstance->TimerInfo[i].pActionRtn != NULL )
		{
			if ( pThisInstance->TimerInfo[i].msecTime > msecElapsed )
			{
				pThisInstance->TimerInfo[i].msecTime -= msecElapsed;
			}
			else	//expired
			{
				pThisInstance->ActiveTimer = i;
				//	call function
				//	timer info may be destroyed otherwise function would have called restart
				if ( (*(pThisInstance->TimerInfo[i].pActionRtn))(pThisInstance->TimerInfo[i].Param) == TRUE )
				{
					//clear info block for this timer
					pThisInstance->TimerInfo[i].pActionRtn = NULL;
					pThisInstance->TimerInfo[i].Param = NULL;
					pThisInstance->TimerInfo[i].msecTime = 0;
					pThisInstance->TimerInfo[i].RefOne = NULL;
					pThisInstance->TimerInfo[i].RefTwo = 0;
					pThisInstance->TimerInfo[i].RestartCount = 0;
					if ( i  ==  pThisInstance->TimersActive - 1 )
					{
						// Loop backward to find next active timer
						for (x = i - 1;   x >= 0;   x--)
						{
							if ( pThisInstance->TimerInfo[x].pActionRtn != NULL )
							{
								break;
							}
						}
						pThisInstance->TimersActive = x + 1;
					}
				}
			}
		}
	}
	//
	//All timers are now free for all operations
	//
	pThisInstance->ActiveTimer = (DWORD)-1;
}



/******************************************************************************
*	FUNCTION NAME:
*		UtilTmrResetTmr
*
*	ABSTRACT:
*		Reset timers (all of the timers in the timer info block).
*
*	RETURN:
*
*	DETAILS:
******************************************************************************/
void UtilTmrResetTmr(
    TIMER_INFO_BLOCK	* pThisInstance,
    DWORD				NumTimers )
{

	CLEAR_MEMORY( (PVOID )pThisInstance, sizeof( TIMER_INFO_BLOCK ) );

	pThisInstance->NumTimers = NumTimers;
	pThisInstance->ActiveTimer = (DWORD)-1;
	pThisInstance->TimersActive = 0;
}
