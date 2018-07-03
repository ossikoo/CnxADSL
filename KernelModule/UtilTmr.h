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
**		UtilTmr.h
**
**	ABSTRACT:
**		Timer routines.
**		This module contains the declarations for UtilTmr.c.
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/UtilTmr.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
#ifndef _UTILTMR_H_		//	File Wrapper,
#define _UTILTMR_H_		//	prevents multiple inclusions


typedef struct _TIMER_INFO_BLOCK
{
	//
	//Number of timers in use
	//
	DWORD		NumTimers;
	//
	//The timer whose function is being called now.
	//
	DWORD		ActiveTimer;
	//
	// The number of timers actually being used
	//
	DWORD		TimersActive;
	//
	//Array of structures for timer info storage
	//
	struct {
		//
		// pointer to action routine to be dispatched
		//
		BOOLEAN		(* pActionRtn)(void * Param);
		//
		//generic pointer to data which may be used  by the action routine.*/
		//
		void		* Param;
		//Remaining Time in msec
		DWORD		msecTime;
		//
		//Two Refernces that uniquely identify the timer
		//
		void		* RefOne;
		DWORD		RefTwo;
		//
		//Number of times the timer has been restarted
		//
		DWORD		RestartCount;
	}TimerInfo[MAX_TIMERS];

} TIMER_INFO_BLOCK;



///////////////////////////////////////////////////////////////////////////////
//	Function Prototypes
///////////////////////////////////////////////////////////////////////////////

BOOLEAN UtilTmrStartTmr(
    TIMER_INFO_BLOCK	* pThisInstance,
    void				* RefOne,
    DWORD				RefTwo,
    DWORD				msecTime,
    BOOLEAN				(* pActionRtn)(void * Param),
    void				* Param );

BOOLEAN UtilTmrClearTmr(
    TIMER_INFO_BLOCK	* pThisInstance,
    void				* RefOne,
    DWORD				RefTwo );

BOOLEAN UtilTmrClearAllTmrs(
    TIMER_INFO_BLOCK	* pThisInstance,
    void				* RefOne );

BOOLEAN UtilTmrReStartTmr(
    TIMER_INFO_BLOCK	* pThisInstance,
    void				* RefOne,
    DWORD				RefTwo,
    DWORD				msecTime );

DWORD UtilTmrGetTmrReStartCount(
    TIMER_INFO_BLOCK	* pThisInstance,
    void				* RefOne,
    DWORD				RefTwo );

BOOLEAN UtilTmrIsTmrActive(
    TIMER_INFO_BLOCK	* pThisInstance,
    void				* RefOne,
    DWORD				RefTwo );

void UtilTmrTmrBackground(
    TIMER_INFO_BLOCK	* pThisInstance,
    DWORD				msecElapsed );

void UtilTmrResetTmr(
    TIMER_INFO_BLOCK	* pThisInstance,
    DWORD				NumTimers );



#endif		//	#ifndef _UTIL_TMR_H_
