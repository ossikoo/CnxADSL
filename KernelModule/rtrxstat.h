#ifndef _RXSTAT_H_
#define _RXSTAT_H_
typedef struct
{
    GEN_UINT entry_count;
    GEN_UINT current_state;
    GEN_UINT next_state;
    GEN_UINT break_point_state; // initialized the breakpoint to a unreachable state
} LOCAL_RXSTAT_TYPE;

/*****                     Global functions                           ******/
DPCCALL void    RX_ResetStateMachine(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    RX_SetBreakPoint(PDATAPUMP_CONTEXT pDatapump,UINT8);
DPCCALL UINT8   RX_GetBreakPoint(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    RX_RSTATE_IntrHandler(PDATAPUMP_CONTEXT pDatapump);
DPCCALL UINT8   RX_GetCurrentState(PDATAPUMP_CONTEXT pDatapump);
DPCCALL char *  RX_GetStateStr(UINT8 state);
DPCCALL BOOLEAN RX_GotoWarmStart(PDATAPUMP_CONTEXT pDatapump);

#ifdef GHS_ENABLE
DPCCALL BOOLEAN RX_GHSWarmStart(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void RX_InitFastRetrain(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    RX_GHS_Done(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void RX_GHS_ResetStateMachine(PDATAPUMP_CONTEXT pDatapump);

#endif

#endif // _RXSTAT_H_

/*****************************************************************************
 * $Log: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/rtrxstat.h $
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
 *************  end of file $Workfile: rtrxstat.h $          ****************/

