/************************************************************************
*  $Workfile: micro.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/micro.h 26    7/10/01 1:01p Lauds $
*
*****************************************************************************
*  Copyright 1996 PairGain Technologies as unpublished work                 *
*  All Rights Reserved                                                      *
*                                                                           *
*  The information contained herein is the confidential property            *
*  of PairGain Technologies.  The use, copying, transfer or                 *
*  disclosure of such information is prohibited except by express           *
*  written agreement with PairGain Technologies.                            *
*                                                                           *
*                                                                           *
*  Original Written By:  David S. Lau                                       *
*  Original Date: 7/29/96                                                   *
*                                                                           *
*****************************************************************************
   Description:  This module should contain all microcontroller and compiler
     dependent routines and parameters
****************************************************************************/

#ifndef MICRO_H
#define MICRO_H
#include "c166.h"                // micro controller dependent header file
#include "types.h"
#include "microxcv.h"
#include "CardALTigrisDp.h"

/****************************************************************************/

typedef struct
{
    BOOLEAN SoftwareSwitchHookState;    // On Hook, by default
    BOOLEAN UseSoftwareSwitchHookState; // Use Hardare, by default;
}LOCAL_MICRO_TYPE;

DPCCALL UINT16 MIC_GetVendorId ( void ) ;
DPCCALL void MIC_GetNonVolatileParms(PDATAPUMP_CONTEXT pDatapump, XCVR_OTHER_CFG_TYPE *);

// This macro should only be called by background thread to wait
// and service watchdog timer while waiting.
DPCCALL void MIC_DelayMsec(PDATAPUMP_CONTEXT pDatapump, TICK_TYPE duration);
#define DELAY_MSEC(duration) MIC_DelayMsec(pDatapump, duration)

DPCCALL void MIC_ChangeAFESetting(PDATAPUMP_CONTEXT pDatapump, const UINT8 * buffer, UINT8 size);
DPCCALL void MIC_SetAFEHybridSelect(PDATAPUMP_CONTEXT pDatapump, UINT8, BOOLEAN);
DPCCALL TICK_TYPE MIC_GetCurrentTick(PDATAPUMP_CONTEXT pDatapump);
DPCCALL TICK_TYPE MIC_MsecElapsed(PDATAPUMP_CONTEXT pDatapump, TICK_TYPE start_tick);
DPCCALL BOOLEAN MIC_DetectOffHook(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MIC_EnableHookStateDetect(BOOLEAN state);
DPCCALL void MIC_SystemReset(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MIC_RockwellRxGain(PDATAPUMP_CONTEXT pDatapump, UINT8 * buffer);
DPCCALL void MIC_ActivateTrap(PDATAPUMP_CONTEXT pDatapump);
DPCCALL UINT8 MIC_ReadAFEReg(PDATAPUMP_CONTEXT pDatapump, UINT8 address);

/***************************************************************************
   Macro         : TEST_AND_SET
   Parameters    : flag = the flag we want to set, we will set it if it's
                   not already set.
   Returns       : return TRUE when flag is not set
                   and FALSE if already set
   Description   : This macro will perform atomic testing and setting
                   of the flag.
   Note          : BSO provides a library macro for C16x to perform this
                   operation.
****************************************************************************/
#define TEST_AND_SET(x) ChipAlTigrisDpTestAndSet (&x)




/***************************************************************************
    Macro           : ATOMIC_START_DURATION(num_instructions)
                        ATOMIC_END_DURATION()
    num_instructions: Number of micro instructions which can not be interrupted
    Description     : These two macros are provided to disable interrupt for
                        number of micro instructions.
                      The Siemens C166 provides _atomic(x) to disable interrupt
                        for 'x' number of instructions which is unique to this
                        micro, most other micro do not have this capability
                        so these micros can emulate this feature by redefine
                        these macros to disable interrupt - action - then
                        enable interrupt
****************************************************************************/

/* --PIM-- Unsed?
#define ATOMIC_START_DURATION(num_instructions) ChipAlTigrisDpAtomicStart ()
#define ATOMIC_END_DURATION() ChipAlTigrisDpAtomicEnd ()
 --/PIM-- */




/***************************************************************************
   Macro         : FALCON_WAIT
   Parameters    : lock1 = first lock to check that indicates whether we should
                 :         wait on the corresponding flag
                 ; flag1 = first flag that we should wait on if first lock is true
                 : lock2 = 2nd lock to check that indicates whether we should
                 :         wait on the corresponding flag
                 ; flag2 = 2nd flag that we should wait on if 2nd lock is true
                 : resolution = Used as value to pause as alternative to waiting
                 ; timeout = maximum time to wait on signal before giving up
   Returns       : none
   Description   : Sets Qedfer wait flag that tells int handlers whether we need
                 : to be signalled to run again. 
****************************************************************************/
#ifdef QEDFER_USES_WAIT
    #define FALCON_WAIT( lock1, flag1, lock2, flag2, resolution, timeout )      \
    {                                                                           \
        QEDFER_FALCON_TYPE Qedfer_Wait_Flag ;                                   \
                                                                                \
        *(UINT16 *) &Qedfer_Wait_Flag = 0 ;                                     \
        Qedfer_Wait_Flag.flag1 = lock1 ? 1 : 0 ;                                \
        Qedfer_Wait_Flag.flag2 = lock2 ? 1 : 0 ;                                \
        ChipAlTigrisDpWaitQedfer ( *(WORD *)&Qedfer_Wait_Flag, resolution, timeout ) ; \
    }
    
#else
    #define FALCON_WAIT( lock1, flag1, lock2, flag2, resolution, timeout )      \
        DELAY_MSEC(resolution); // give up CPU in case of RTOS
#endif




/***************************************************************************
   Macro         : FALCON_SIGNAL
   Parameters    : lock = lock to clear to indicate operation completed
                 ; flag = flag that we should set to indicate which int has occurred
   Returns       : none
   Description   : Signals that indicated int has occurred which may satisyf wait.
****************************************************************************/
#ifdef QEDFER_USES_WAIT
    #define FALCON_SIGNAL( lock, flag )                                         \
    {                                                                           \
        QEDFER_FALCON_TYPE Qedfer_Signal_Flag ;                                 \
                                                                                \
        lock = FALSE ;                                                          \
        *(UINT16 *) &Qedfer_Signal_Flag = 0 ;                                   \
        Qedfer_Signal_Flag.flag = 1 ;                                           \
        ChipAlTigrisDpSignalQedfer ( *(WORD *) &Qedfer_Signal_Flag ) ;          \
    }
    
#else    
    #define FALCON_SIGNAL( lock, flag )                                         \
        lock = FALSE ;
#endif


#endif

