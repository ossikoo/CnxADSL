#ifndef _DATAPUMP_H_
#define _DATAPUMP_H_
typedef struct DATAPUMP * PDATAPUMP_CONTEXT;
#ifndef offsetof
    #define offsetof(s,m)   (size_t)&(((s *)0)->m)
#endif

#include "types.h"
#include "fftblk.h"
#include "xcvrinfo.h"
#include "sysinfo.h"
#include "aocintf.h"
#include "autosense.h"
#include "util.h"
#include "diagnos.h"
#include "asicutil.h"
#include "eocintf.h"
#include "stdeoc.h"
#include "monxcvr.h"
#include "monitor.h"
#include "dmtdbg.h"
#include "micro.h"
#include "falcon.h"             // rwb - per David Lau
#include "miscutil.h"
#include "rttxstat.h"
#include "dmtintf.h"
#include "rtrxstat.h"
#include "rtbgstat.h"
#include "qedfer.h"
#include "adslinfo.h"
#include "ghsutil.h"
#include "timer.h"
#include "afeintf.h"
#include "fpga.h"
#include "nvram.h"
#include "dpu.h"
#include "dpuutil.h"
#include "dpudmt.h"
#include "selftest.h"
#include "dmtinfo.h"
#include "terminal.h"
#include "serial.h"
#include "microxcv.h"
#include "smrt.h"
#include "smrtxcvr.h"


typedef struct DIAGNOSTIC_LOG_TYPE
{
    DIAGNOSTIC_TYPE buffer[MAX_LOG_SIZE];
    UINT8 current_index;
}DIAGNOSTIC_LOG_TYPE;



typedef struct DATAPUMP
{
    void * pExternalContext;    // points to the external structure which includes DATAPUMP
    // adslinfo:
    LOCAL_ADSLINFO_TYPE local_var_adslinfo;
    BOOLEAN link_active;
    SYS_PROM_CHECKSUM_TYPE SYS_prom_checksum[NUM_ATU_MODE];
    // afeintf:
    LOCAL_AFEINTF_TYPE local_var_afeintf;
    AFE_RX_GAIN_SETTING_TYPE rx_gain_setting;
    CNXT_AFE_TYPE cnxt_afe_rev;  // default to 0 (i.e. UNKNOWN_AFE)
    UINT8 afe_register_image[NUM_AFE_REG_ADRS]; // Contains values written to AFE.
    #ifdef CO
    GEN_SINT rx_setting;
    #endif
    SINT8 tx_setting;
    // asicutil:
    LOCAL_ASICUTIL_TYPE local_var_asicutil;
    #ifdef ECHO_TRAINING
    UINT8 echo_cancel_Beta[2];//={6,7};
    #endif
    volatile LOCK_TYPE mif_lock;

    volatile LOCK_TYPE fftb_lock;

    volatile LOCK_TYPE qedfer_algo_lock;

    volatile LOCK_TYPE tone_avg_lock;
    FTEQ_SETUP_TYPE fteq_setup;
    FIFO_TYPE xfr_reqq;
    MIF_XFER_REQ_TYPE xfr_req_buffer[MAX_QLEN], current_req;
    BIT_SWAP_MODE bit_swap_mode;
    AVG_PWR_REQ_TYPE pwravg_req;
    AVG_SIG_REQ_TYPE sigavg_req;
    FFTB_REQ_TYPE current_fftb_req;
    #ifdef FAST_PATH_FIX
    UINT8 data_mode;
    #endif
    BIT_SWAP_REQ_TYPE tx_bit_swap_req, rx_bit_swap_req;
    BOOLEAN globespan_eoc_mode;
    // aocintf:
    LOCAL_AOCINTF_TYPE local_var_aocintf;
    AOC_DB_TYPE aoc_database;
    volatile LOCK_TYPE xmit_lock;    // reset semaphore
    #if AOC_RX_BUFFER
    #define RX_BUFFER_SIZE 500
    UINT16 aoc_rx_index;
    UINT8 aoc_rx_buffer [RX_BUFFER_SIZE] ;
    #endif

    volatile FLAG_TYPE pairgain_eoc_protocol;

    // autosense:
    LOCAL_AUTOSENSE_TYPE local_var_autosense;
    AUTOSENSE_STATE_T   autosense_state;
    AUTOSENSE_STATE_T   autosense_prev_state;
    SUPER_STATE_T   autosense_super_state;
    SUPER_STATE_T   autosense_prev_super_state;
    COMPL_STATE_T   autosense_compl_queue_fsm_state;
    BOOLEAN         autosense_initialized;
    BOOLEAN     autosense_started;
    BOOLEAN     autosense_shutdown_req;
    BOOLEAN     autosense_timeout_done;
    BOOLEAN     autosense_restart_req;
    BOOLEAN     autosense_pause_req;
    BOOLEAN     autosense_extension_state;
    SUPER_EVENT_T   autosense_pause_event;
    TICK_TYPE   autosense_pause_start;
    TICK_TYPE   autosense_pause_duration;
    TICK_TYPE   autosense_timeout_max;
    TICK_TYPE   autosense_timeout_extension;
    TICK_TYPE   autosense_tick;
    SYS_WIRING_SELECTION_TYPE   autosense_locked_wiring;
    UINT8       autosense_permutation_loop_count;
    UINT8       autowiring_permutation_count;
    UINT8       autohandshake_permutation_count;
    UINT32      autosense_detect_power[NUM_WIRINGS][NUM_HANDSHAKES];
    UINT16      autosense_queue_put_index;
    UINT16      autosense_queue_get_index;
    UINT16      autosense_queue_data[MAX_NUM_QUEUE_EVENTS] ;
    UINT16      autosense_queue_max_usage;
    // dmtintf:
    LOCAL_DMTINTF_TYPE local_var_dmtintf;
    BOOLEAN coldstart_flag;
    // dpudmt:
    LOCAL_DPUDMT_TYPE local_var_dpudmt;
    MEASURED_FR_POWERLEVELS fr_powerlevels;
    AOC_DB_TYPE AdslAocMenuScrDisp_dupptr;          // This really use lot memory, any better way???
    STDEOC_DB AdslEocMenuScrDisp_dupptr;
    BOOLEAN  power_down_mode;
    SPRAM_TYPE vbuffer;         // buffer to be viewed in FALCON TEST menu
    // eocintf:
    LOCAL_EOCINTF_TYPE local_var_eocintf;
    UINT8 txeoc_running; /* 0->EOC tx channel is not running, 1->it's running */
    //micro:
    LOCAL_MICRO_TYPE local_var_micro;
    BOOLEAN SoftwareSwitchHookState;    // On Hook, by default
    BOOLEAN UseSoftwareSwitchHookState; // Use Hardare, by default;
    // monitor:
    LOCAL_MONITOR_TYPE local_var_monitor;
    ALARM_TYPE           MON_alarm[NUM_ATU_MODE][NUM_OF_ALARMALLOC];
    UINT8 ses_count[NUM_ATU_MODE];
    LAST_CLEARED last_cleared;
    SYS_LOFLOS_STATUS_TYPE prv_loflos_status;
    SYS_DATA_TYPE            daily_monitor_data[NUM_OF_15MIN_SLOTS];
    SYS_WEEKLY_MON_DATA_TYPE cur_day_monitor_data;
    SYS_WEEKLY_MON_DATA_TYPE weekly_monitor_data[DAYS_IN_WEEK];
    SYS_OPERATING_DAYS_TYPE  days_in_operation;
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

    // smrt:
    LOCAL_SMRT_TYPE local_var_smrt;
    UINT8 smrt_in_buf[SYS_SMRT_CMD_DATA_SIZE+SMRT_MSG_OVERHEAD];
    UINT16 rx_byte_num;           // number of bytes received in a frame:
                                          // SYNC_BYTE, RESV_BYTE,...CHKSUM.
                                          // No framing char(0xF1) is included.

    // rtbgstat:
    LOCAL_BGSTAT_TYPE local_var_bgstat;
    FR_DATABASE_TYPE      FR_DB;
    GEN_UINT rtbgstat_entry_count;
    GEN_UINT rtbgstat_current_state, rtbgstat_next_state;
    GEN_UINT rtbgstat_break_point_state; // initialized the breakpoint to a unreachable state
    near void (* rtbgstat_break_point)(struct DATAPUMP *pDatapump);
    TICK_TYPE fr_start_tick;                   // timer tick for fr
    TICK_TYPE fr_delta_tick;                   // delta timer tick for fr
    UINT32 fr_last_bkgnd_pwr;                  // last background power value
    BOOLEAN fr_chk_reverb;
    UINT32 delta_store;
    SINT32 old_store_in;
    UINT32 STORE_CNVG_THRESHOLD;
    SINT16 Politeness_rxpwr;
    UINT16 Politeness_fftgain;
    SINT16 fft_rx_gain;
    SINT16 afe_rx_gain;
    //UINT8 prbs_detect_channels;
    TICK_TYPE modem_detect_tick;  // Timeout for detecting far end modem
    XCVR_STARTUP_FLAG_TYPE  xcvr_startup_flag;
    // this variable stores a stable value for STORE for GHS and FR
    UINT32 Saved_store;
    // This scratch buffer is used for general-purpose temporary transitional
    // storage, several local data structures are pointing to this to conserve memory

    union
    {
       UINT16 uint16[ADCSAMPLES+MAX_NUM_FIR_COEF];  // we need the extra words for fteq training
       UINT32 uint32[NUM_RX_CHNL];
       SINT16 sint16[ADCSAMPLES];
       SINT32 sint32[NUM_RX_CHNL];
       ADJ_SQRT_NSR_TYPE adj_sqrt_nsr[NUM_RX_CHNL];
    } scratch_buffer;
    BOOLEAN four_plot_request;
    UINT32  normal_NSR[256] ;     
    UINT32  only_ISI_NSR[256];   
    UINT32  only_echo_NSR[256];
    UINT32  only_noise_NSR[256];
    SINT32  ghs_tone_detection_pwr[GHS_TONE_RX_TRANSFER_SIZE+GHS_TONE_RX_OUT_BAND_SIZE];
    SINT16  snr_margin_tbl[NUM_RX_CHNL];

    UINT16   rates_br[NUMRATES1OPTION];
    TICK_TYPE rtbgstat_start_tick;                   // showtime start timer tick
    BOOLEAN one_shot_after_showtime;        // do these tasks once after showtime
    BOOLEAN reduce_upstream_capacity;
    #ifdef GHS_ENABLE
    UINT32 bkgnd_pwr,forgnd_pwr;
    #endif
    UINT8 retry_count;  // retry count for rx gain adjustment
    GEN_UINT fteq_iteration, fteq_training_stage;
    BOOLEAN update_snr_margin;
    TICK_TYPE warm_start_start_tick;  // record start time for warm start
    FLAG_TYPE on_off_hook_status; // stored status for on-hook and off-hook condition
    UINT8 curHybrid;//  = 1;   //Default to Hybrid 1 Selected (which is the AFE default)
    UINT8 bestHybrid;// = 1;
    UINT32 bestSigAvePwr;// = 0xFFFFFFFF;  // Default to max. possible value
    UINT32 curSigAvePwr;
    GEN_UINT  no_showtime_count;    // default to zero
    BOOLEAN   clr_retrain_count;    // default to FALSE
    UINT8 NSR_accumulation_count;  // keeps track of how many times we do error averaging
    // an index indicating which tone in the global tone_detection_list we are looking at
    GEN_UINT detect_tone_index;
    #ifdef ECHO_TRAINING
    // This is to keep track of the num of FTEQ training iteration and training stage
    GEN_UINT ec_interation;
    #endif
    BOOLEAN use_abbreviated_ghs_clr;
    UINT16 sig_detection_pwr;
    UINT8 fast_rtrn_count;
    TICK_TYPE reverb_start_tick;                   // showtime start timer tick
    SINT32 delta_feq_error;
    UINT32 old_total_error;
    GEN_UINT max_bit_per_chnl;
    // rtrxstat:
    LOCAL_RXSTAT_TYPE local_var_rxstat;
    GEN_UINT rtrxstat_entry_count;
    GEN_UINT rtrxstat_current_state;
    GEN_UINT rtrxstat_next_state;
    GEN_UINT rtrxstat_break_point_state; // initialized the breakpoint to a unreachable state
    near void (*rtrxstat_break_point)(struct DATAPUMP *pDatapump);
    // rttxstat:
    LOCAL_TXSTAT_TYPE local_var_txstat;
    BOOLEAN first_superframe;
    GEN_UINT rttxstat_entry_count;
    GEN_UINT rttxstat_current_state, rttxstat_next_state;
    GEN_UINT rttxstat_break_point_state;
    near void (* rttxstat_break_point)(struct DATAPUMP *pDatapump);
    GEN_UINT quiet_duration_counter;
    UINT8 ACT_REQ_pwr_state;
    // ghsutil:
    GHS_DB_TYPE      ghs_db;
    //serial:
    LOCAL_SERIAL_TYPE local_var_serial;
    SYS_SERIAL_MODE_TYPE serial_mode;
    UINT8 tx_msgq_buffer[MAX_TXQ_SIZE];
    UINT8 rx_msgq_buffer[MAX_RXQ_SIZE];
    // this lock indicates if the current PEC transfer buffer is available:
    //volatile LOCK_TYPE tx_serial_lock;
    //
    // Indicates whether the terminal connected to our serial port
    // has established a remote logon, e.g. whether the other end is
    // set up to accept and respond to keystrokes.
    // Gets set to REMOTE_LOGON when the user selects remote logon
    // from the menu

    SYS_LOGON_MODE_TYPE logon_mode;
    //
    // Indicates if the user is currently looking at the local display
    // or the remote display. REMOTE_LOGON means the user is logged
    // on the the remote unit and is currently interacting with the
    // remote unit's menu system

    SYS_LOGON_MODE_TYPE display_mode;
    DYN_FIFO_TYPE    serial_tx_msgq;
    FIFO_TYPE        dumb_term_rx_msgq;
    // for terminal detect/autobaud

    //UINT8 reset_terminal;    // signal to reset to the password screen
                                      // and send out a new screen
    //UINT8 terminal_detected; // indicates terminal unplugged and plugged
                                      // in again also indicates errors
                                      // indicate should go back through autobaud
    // sysinfo:
    UINT8 SYS_GetRemDBupdated;
    EOC_SEND_INFO_TYPE   eoc_send_info;
    //XCVR_TYPE               sys_num_xcvrs;
    FAILURE_TYPE            sys_restart_info;
    FAILURE_TYPE            sys_failure[NUM_OF_FAILTYPE];
    SYS_ERR_CNT_TYPE        sys_err_cnt;             //  = {0,0}
    UINT8                   sys_state[NUM_ATU_MODE]; // Sys state for NEAR and FAR_END
    SYS_MON_DATA_TYPE       sys_mon_data;
    TIME_TYPE               SYS_time;
    SYS_NVRAM_IMAGE_TYPE    sys_nvram[NUM_ATU_MODE]; // init'd in code
    UINT8                   SYS_sw_vers[NUM_ATU_MODE][MAX_VER_LEN];
    UINT16                  sys_line_status[NUM_ATU_MODE];   //L ine Status for MIB

    // smrtxcvr:
    BOOLEAN priority_voice_mode;  // default to be FALSE
    SPRAM_TYPE debug_buffer;
    OUT_BUF_TYPE out_buf;
    UINT16 debug_parameters[10];
    // smrt:
    TICK_TYPE smrt_start_tick;    // keep track of the start time interval of the
    DYN_FIFO_TYPE    smrt_term_rx_msgq;
    UINT8 TAO_mode;


    // miscutil:
    LOCAL_MISCUTIL_TYPE local_var_miscutil;
    UINT32 saved_store;
    UINT16 teq_gain_scale;
    UINT16 saved_tx_filter_gain;
    // stdeoc
    LOCAL_STDEOC_TYPE local_var_stdeoc;
    STDEOC_DB  stdeoc_db;
    BOOLEAN  FIFO_Semiphor[EOC_REG_NUM];
    BOOLEAN  FIFO_Semiphor_ReqTpu;
    UINT8 EOC_register_maxlen[EOC_REG_NUM];
    GEN_UINT disqualifying_counter;
    GSPN_EOC_CTL_TYPE gspn_eoc_ctl;
    UINT8 processing_superframe;        // superframe that we process pending EOC msg
    UINT8 start_flooding_superframe;    // superframe that we start sending EOC
    UINT8 stop_flooding_superframe;     // superframe that we stop sending EOC
    UINT8 eoc_snr_margin;
    GEN_UINT last_crc_count;
    // xcvrinfo:
    XCVR_DATABASE_TYPE      XCVR_database;
    XCVR_ATM_CFG_TYPE  xcvr_atm_cfg;
    SYS_MEASUREMENT_TYPE SYS_measurement[NUM_ATU_MODE];
    FALCON_CONFIG_OPTION_TYPE falcon_config_option;
    UINT8    XCVR_other_id[NUM_ATU_MODE][MAX_XCVR_OTHER_ID];
    BOOLEAN XCVR_Falcon_Tx_Clk_Mode;
    BOOLEAN update_remote_xcvr_cfg; // set flag by ATU_C to update

    #ifdef CO
    ADSL_DWNSTREAM_MSG_BUF_TYPE    adsl_tx_msg_buffer;
    ADSL_UPSTREAM_MSG_BUF_TYPE     adsl_rx_msg_buffer;
    #else
    ADSL_UPSTREAM_MSG_BUF_TYPE     adsl_tx_msg_buffer;
    ADSL_DWNSTREAM_MSG_BUF_TYPE    adsl_rx_msg_buffer;
    #endif
    //UINT16 XCVR_Framing_Mode;
    XCVR_FTEQ_TRAIN_CONFIG_TYPE xcvr_fteq_config[4];
    //                   Echo Canceller Coefficients
    UINT16 xcvr_ec_coef[NUM_EC_COEF];   // init to all zeros
    UINT16 xcvr_rx_filter[NUM_FIR_COEF];
    UINT16 TEQ_margin;
    // This allows programmable coding gain
    //UINT8 XCVR_default_coding_gain;// = 4;
    UINT16 analog_rx_pwr_margin;
    UINT16 analog_tx_pwr_margin;
    BOOLEAN    XCVR_dis_restart;     // Default is enabled
    XCVR_TX_FILTER_TYPE xcvr_default_tx_filter[DAC_TX_FILTER_SIZ];
    // ??? This is a test code variable to allow extended time for TEQ training
    UINT16 XCVR_ext_teq_timing;

    // falcon.c:
    FALCON_TYPE  falcon;
    //CardAlTigris:
    void * Falcon_Space;
    WORD AdslLocalG922Cap;
    WORD AdslLocalG922AnnexCap;
    BOOLEAN CdALDiagControllerLogEnable;
    //terminal:
    #ifdef VCOMM
    DPU_SCR_INFO dpu_scr_info[NUM_LOGON_MODE];
    volatile LOCK_TYPE dpu_local_work_area_lock;
    volatile LOCK_TYPE dpu_remote_work_area_lock;
    BOOLEAN TRM_xmit_startup_log;
    #endif

    //timer:
    LOCAL_TIMER_TYPE local_var_timer;
    TIME_TYPE SysTime;//={0,0,0};
    DATE_TYPE    sys_date;
    PERIODIC_TASK_TRACE_TYPE task_trace_list[NUM_OF_TASK];
    TICK_TYPE current_tick;

    #ifdef DEBUG_AFE
    //UINT16  AfePokeFailedCnt;
    #endif
    // nvram:
    UINT8 NV_Array [sizeof (SYS_NVRAM_IMAGE_TYPE)];


    // extra word in BitsGainTbl is used for carrying the CRC word for downstream

    // Note that    NUM_TX_CHNL = N_VALUE/2 = number of tx samples (size of IFFT)/2
    //                          = 512/2
    //              NUM_RX_CHNL = M_VALUE/2 = number of rx samples (size of FFT)/2
    //                          = 64/2






    // PROTOCOL



    #if DMT_DBG_TRACE_RAM
        #if DBG_PRINT_PORT_TRACE
            BOOLEAN Dbg_Print_Port_Trace;   // set to true with debugger to trace to debug port
        #endif //DBG_PRINT_PORT_TRACE
        UINT16 DbgPutLineIndex;
        UINT16 DbgGetLineIndex;
        DMT_DBG_TRACE_TYPE DbgBuf[DBG_LINE_CNT+1] ; // one spare line at end to accept overflow of last used line
    #endif //DMT_DBG_TRACE_RAM
    UINT16 diagnostic_reset_histogram [MAX_RESET_CODE+1];   //plus 1 for location 0
    DIAGNOSTIC_LOG_TYPE diagnostic_log;
    #if DIAGNOS_WRITE_BUS
    BOOLEAN Diagnos_Write_Bus;
    #endif
    #if DIAGNOS_LOG_TRACE
    UINT8 Diagnos_Log_Trace;
    #endif
    #if AOC_TRACE
    BOOLEAN Aoc_Trace;
    #endif 
    #if EOC_TRACE
    UINT8 Eoc_Trace;
    #endif

    #if AFE_WRITE_TRACE
    UINT8 Afe_Write_Trace;
    #endif
    #if DMT_FSM_TRACE
    BOOLEAN Dmt_Fsm_Trace;
    #endif  //DMT_FSM_TRACE
    #if AUTOSENSE_TRACE
        BOOLEAN Autosense_Trace;
    #endif

    // note that this buffer has to locate in the first 64K segment to make
    //  PEC transfer feasible

    #ifdef CO
    // cobgstat:
    UINT16   rates1_br[NUMRATES1OPTION];
    GEN_UINT corxstat_entry_count;
    GEN_UINT corxstat_current_state;
    GEN_UINT corxstat_next_state;
    GEN_UINT corxstat_break_point_state; // initialized the breakpoint to a unreachable state
    near void (*corxstat_break_point)(struct DATAPUMP *pDatapump);
    BOOLEAN ghcobgst_rst;
    // cotxstat:
    GEN_UINT cotxstat_entry_count;
    GEN_UINT cotxstat_current_state, cotxstat_next_state;
    GEN_UINT cotxstat_break_point_state;
    near void (* cotxstat_break_point)(struct DATAPUMP *pDatapump);
    GEN_UINT cobgstat_entry_count;
    GEN_UINT cobgstat_current_state, cobgstat_next_state;
    GEN_UINT cobgstat_break_point_state; // initialized the breakpoint to a unreachable state
    near void (* cobgstat_break_point)(struct DATAPUMP *pDatapump);
    TICK_TYPE PILOT1routine_start_tick;
    TICK_TYPE cobgstat_start_tick;
    UINT8 C_ACT_entry_count;
    UINT16 ref_sig_pwr;
    #endif

} DATAPUMP_CONTEXT ;

#endif
/*****************************************************************************
 * $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/datapump.h $
 * 
 * 7     8/31/01 2:38p Lauds
 * increase debug parameters from 5 to 10
 * 
 * 6     8/22/01 4:21p Lauds
 * Smart Terminal to support multiple bit allocation plots
 * 
 * 5     7/12/01 8:28a Davidsdj
 * PROBLEM:
 * Warning generated in Linux compiles that there is a comment within a
 * comment.
 * 
 * SOLUTION:
 * Removed the slash ('/') from line within the comment block that was
 * generating the warning message.
 * 
 * 4     7/10/01 12:56p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 *************  end of file $Workfile: datapump.h $          ****************/
