typedef struct
{
    UINT8  SFcnt;
    UINT8  store_avg_cnt;
    UINT32 avg_store_value;
    UINT32 store_accumulator;
} NORMAL_MAINTENANCE_TYPE;

typedef struct
{
    UINT8   SFcnt;
    UINT8   result;
    UINT16  ref;      // Not used by lof. For LOS it is half_refer_pilot_pwr
} MON_EVENT_TYPE;

DPCCALL void IRQ1_ISR(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void IRQ2_ISR(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MON_TSYNC_IntrHandler(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MON_RSYNC_IntrHandler(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MON_CrcIntHandler(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MONXCVR_Restart(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MONXCVR_ResetDatabase(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void MON_Update1SecRemoteData(PDATAPUMP_CONTEXT pDatapump);    // Update remote data such as
                                        //  SYSTEM_MEASUREMENT
