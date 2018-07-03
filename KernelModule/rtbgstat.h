#ifndef _BGSTAT_H_
#define _BGSTAT_H_
typedef struct
{
    FR_DATABASE_TYPE      FR_DB;
    GEN_UINT entry_count;
    GEN_UINT current_state, next_state;
    GEN_UINT break_point_state; // initialized the breakpoint to a unreachable state
    near void (* rtbgstat_break_point)(struct DATAPUMP *pDatapump);
    TICK_TYPE fr_start_tick;                   // timer tick for fr
    TICK_TYPE fr_delta_tick;                   // delta timer tick for fr
}LOCAL_BGSTAT_TYPE;

// rtbgstat.h
DPCCALL char  * BG_GetStateStr(UINT8 state);
DPCCALL void    BG_ResetStateMachine(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    BG_ProcessHandler (PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    BG_SetBreakPoint(PDATAPUMP_CONTEXT pDatapump, UINT8 state);
DPCCALL UINT8   BG_GetBreakPoint(PDATAPUMP_CONTEXT pDatapump);
DPCCALL UINT8   BG_GetCurrentState(PDATAPUMP_CONTEXT pDatapump);
DPCCALL BOOLEAN BG_GotoWarmStart(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    BG_UpdateSnrMargin(PDATAPUMP_CONTEXT pDatapump);
DPCCALL SINT16  BG_GetSnrMargin(PDATAPUMP_CONTEXT pDatapump,UINT8 tone);
#ifdef GHS_ENABLE
DPCCALL void    BG_ResetFastRetrain(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    BG_GHS_Done(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    BG_GHS_ResetStateMachine(PDATAPUMP_CONTEXT pDatapump);
DPCCALL BOOLEAN BG_GHSWarmStart(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void BG_InitFastRetrain(PDATAPUMP_CONTEXT pDatapump);
#endif
DPCCALL void InitFEQ(PDATAPUMP_CONTEXT pDatapump, BOOLEAN dgc_only);
#ifdef CO
void detect_pwr_wink_2(PDATAPUMP_CONTEXT pDatapump);
void detect_pwr_wink_3(PDATAPUMP_CONTEXT pDatapump);
void detect_pwr_wink_1(PDATAPUMP_CONTEXT pDatapump);
#endif


#endif   //_BGSTAT_H_

/*****************************************************************************
 * $Log: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/rtbgstat.h $
 * 
 * 3     7/12/01 8:29a Davidsdj
 * PROBLEM:
 * Warning generated in Linux compiles that there is a comment within a
 * comment.
 * 
 * SOLUTION:
 * Removed the slash ('/') from line within the comment block that was
 * generating the warning message.
 * 
 * 2     7/10/01 1:04p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 *************  end of file $Workfile: rtbgstat.h $          ****************/
