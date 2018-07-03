#ifndef _TXSTAT_H_
#define _TXSTAT_H_
typedef struct
{
    BOOLEAN first_superframe;
    GEN_UINT rttxstat_entry_count;
    GEN_UINT rttxstat_current_state, rttxstat_next_state;
    GEN_UINT rttxstat_break_point_state;
    near void (* rttxstat_break_point)(struct DATAPUMP *pDatapump);
    GEN_UINT quiet_duration_counter;
    UINT8 ACT_REQ_pwr_state;
}LOCAL_TXSTAT_TYPE;

DPCCALL char *  TX_GetStateStr(UINT8 state);
DPCCALL void    TX_ResetStateMachine(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    TX_SetBreakPoint(PDATAPUMP_CONTEXT pDatapump, UINT8);
DPCCALL UINT8   TX_GetBreakPoint(PDATAPUMP_CONTEXT pDatapump);
DPCCALL UINT8   TX_GetCurrentState(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    TX_TSTATE_IntrHandler(PDATAPUMP_CONTEXT pDatapump);
DPCCALL BOOLEAN TX_IssueHostCmd(PDATAPUMP_CONTEXT pDatapump, HOST_CMD_TYPE);
DPCCALL char *  TX_GetHostCmdStr(HOST_CMD_TYPE);
DPCCALL BOOLEAN TX_GotoWarmStart(PDATAPUMP_CONTEXT pDatapump);
#ifdef GHS_ENABLE
DPCCALL void    TX_GHS_Done(PDATAPUMP_CONTEXT pDatapump);
DPCCALL BOOLEAN TX_GHSWarmStart(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void TX_InitFastRetrain(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void TX_ghs_idle_transmitter(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void TX_GHS_ResetStateMachine(PDATAPUMP_CONTEXT pDatapump);
#endif

#endif // _TXSTAT_H_

/*****************************************************************************
 * $Log: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/rttxstat.h $
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
 *************  end of file $Workfile: rttxstat.h $          ****************/
