#ifndef _TIMER_H_
#define _TIMER_H_
typedef struct
{
    TIME_TYPE SysTime;//={0,0,0};
    DATE_TYPE    sys_date;
    PERIODIC_TASK_TRACE_TYPE task_trace_list[NUM_OF_TASK];
    TICK_TYPE current_tick;
}LOCAL_TIMER_TYPE;

/*****                     Global functions                           ******/
DPCCALL DATE_TYPE_RETURN TM_GetPreviousDate(DATE_TYPE);
DPCCALL UINT16      TM_TimeDiff(TIME_TYPE start, TIME_TYPE stop);
DPCCALL BOOLEAN     TM_SetSysDate(PDATAPUMP_CONTEXT pDatapump, UINT16 year,UINT8 month,UINT8 day);
DPCCALL DATE_TYPE_RETURN TM_GetSysDate(PDATAPUMP_CONTEXT pDatapump);
DPCCALL BOOLEAN     TM_SetSysTime(PDATAPUMP_CONTEXT pDatapump, TIME_TYPE time);
DPCCALL char*       TM_GetDateStr(DATE_TYPE date);
DPCCALL void        TM_GetDateStr2(DATE_TYPE date, char *buffer);
DPCCALL char*       TM_GetTimeStr(TIME_TYPE time);
DPCCALL TIME_TYPE_RETURN TM_GetSysTime(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void        TM_LoResTimerIntHandler(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void        TM_InitPeriodicScheduler(PDATAPUMP_CONTEXT pDatapump, UINT16);
DPCCALL void        TM_UpdateDateTime(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void        TM_UpdateSysDateDspl(void);
DPCCALL void        TM_SetDaylightSavingMode(BOOLEAN);
DPCCALL DATE_TIME_TYPE_RETURN TM_GetDateAndTime(DATE_TYPE date, TIME_TYPE time);
DPCCALL TICK_TYPE TM_GetCurrentTick(PDATAPUMP_CONTEXT pDatapump);
DPCCALL TICK_TYPE TM_SecondElapsed(PDATAPUMP_CONTEXT pDatapump, TICK_TYPE tick);
#endif
/*****************************************************************************
 * $Log: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/timer.h $
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
 * 2     7/10/01 1:07p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
*****************  end of file $Workfile: timer.h $          ****************/
