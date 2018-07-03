#ifndef _MONITOR_H_
#define _MONITOR_H_
// monitor.h
#define NUM_OF_15MIN_SLOTS        96       // 96 of 15-min slots = 1 day
#define SECS_IN_15MIN             900
#define MAX_DURATION              0xffff   // the maximum duration for a word w/o overflowing
#define SNR_CHECK_BYPASS_DURATION 3        // Number of seconds to skip SNR alarms after getting to showtime


typedef struct
{
    BOOLEAN nearend_es,
            farend_es,
            nearend_mar,
            farend_mar,
            nearend_ses,
            farend_ses,
            nearend_excess_ses,
            farend_excess_ses,
            nearend_excess_uas,
            farend_excess_uas,
            nearend_excess_lof,
            farend_excess_lof,
            nearend_excess_los,
            farend_excess_los,
            nearend_excess_lcd,
            farend_excess_lcd,
            nearend_init;
}   LOCAL_ALARM_TYPE;
typedef struct 
{
    TIME_TYPE time; // time alarm history was cleared last
    DATE_TYPE date; // date alarm history was cleared last
}LAST_CLEARED;

typedef struct
{    
    UINT8 ses_count[NUM_ATU_MODE];
    LAST_CLEARED last_cleared;
    SYS_LOFLOS_STATUS_TYPE prv_loflos_status;
    // index to the current 15-minute window of the 24 hour performance history

    UINT8 cur_15min_slot;
    UINT8 current_day;
    LOCAL_ALARM_TYPE local_alarm;
    GEN_UINT snr_count;
    BOOLEAN previous_link_active; // Previous Link State for all adsl channels
    UINT32 Local_FarEnd_SESCount;   // vld 03-21-01
    // monxcvr:
    MON_EVENT_TYPE los_event[NUM_ATU_MODE];
    MON_EVENT_TYPE    lof_event;
    MON_EVENT_TYPE    lcd_event[NUM_ATU_MODE]; // near_end and far_end
    int SyncCount; 
}LOCAL_MONITOR_TYPE;


// functions invoked by periodic interrupt
// NOTE: The following 4 functions are invoked by periodic interrupts and
//          they are executued for ALL adsl channels:

DPCCALL void MON_Update1SecDatabase(PDATAPUMP_CONTEXT pDatapump);      // Update both local/remote data base
DPCCALL void MON_Update15MinDatabase(PDATAPUMP_CONTEXT pDatapump );
DPCCALL void MON_UpdateDailyMonData(PDATAPUMP_CONTEXT pDatapump );

// The following functions are executed per adsl channel basis:

DPCCALL void  MON_GetAlarmStatus         (PDATAPUMP_CONTEXT pDatapump, SYS_ALARM_STATUS_TYPE*);
DPCCALL UINT8 MON_Get15MinHistIdx       (PDATAPUMP_CONTEXT pDatapump, UINT8 entry_to_hist);
DPCCALL UINT8 MON_Get7DayHistIdx        (PDATAPUMP_CONTEXT pDatapump, UINT8 entry_to_hist);

// interface function to DSLAM

DPCCALL void  MON_Get15MinPerf      (PDATAPUMP_CONTEXT pDatapump, UINT8 entry2table,
                           SYS_ALL_EVENT_TYPE* up_data_ptr,
                           SYS_ALL_EVENT_TYPE* dwn_data_ptr);
DPCCALL void  MON_Get7DayPerf       (PDATAPUMP_CONTEXT pDatapump, UINT8 entry2table,
                           SYS_WEEKLY_ALL_EVENT_TYPE* up_data_ptr,
                           SYS_WEEKLY_ALL_EVENT_TYPE* dwn_data_ptr);
DPCCALL void  MON_GetFreeRunCnt     (PDATAPUMP_CONTEXT pDatapump,SYS_TOTAL_EVENT_COUNT * up_data_ptr,
                             SYS_TOTAL_EVENT_COUNT* dwn_data_ptr);

// utility functions

DPCCALL void MON_ResetAlarm(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MON_ResetDailyHist(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MON_ResetWeeklyHist(PDATAPUMP_CONTEXT pDatapump);

DPCCALL void MON_Init(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MON_SetInitFailureCondition(PDATAPUMP_CONTEXT pDatapump, UINT8 near_far, LINE_STATUS_DATA_TYPE failure_reason);


DPCCALL UINT8                       MON_FailureEvaluation(PDATAPUMP_CONTEXT pDatapump,UINT8 failure_option,
                                                        UINT8 defectflag);
DPCCALL SYS_DATA_TYPE             * MON_GetDailyData(PDATAPUMP_CONTEXT pDatapump, UINT8 index);
DPCCALL SYS_WEEKLY_MON_DATA_TYPE  * MON_GetWeeklyData(PDATAPUMP_CONTEXT pDatapump,UINT8 index);
DPCCALL TIME_TYPE_RETURN            MON_GetLastClearedAlarmTime(PDATAPUMP_CONTEXT pDatapump);
DPCCALL DATE_TYPE_RETURN            MON_GetLastClearedAlarmDate(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void                        MON_UpdateSelftestAlarm(PDATAPUMP_CONTEXT pDatapump,UINT32 selftest_status);

DPCCALL void MON_ResetSESCount(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MON_ResetSNRCount(PDATAPUMP_CONTEXT pDatapump);
#endif

/*****************************************************************************
 * $Log: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/monitor.h $
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
 * 2     7/10/01 1:03p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 **************  end of file $Workfile: monitor.h $          ****************/
