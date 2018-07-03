#ifndef _AUTOSENSE_H_
#define _AUTOSENSE_H_


// AutosenseFSM States
typedef enum
{
	AUTOSENSE_UNLOCKED_SEARCH_STATE,
	AUTOSENSE_WIRING_LOCKED_SEARCH_STATE,
	AUTOSENSE_LOCKED_SEARCH_STATE,
	AUTOSENSE_LOCKED_STATE
} AUTOSENSE_STATE_T ;

//FSM Super States
typedef enum
{
	SUPER_REGULAR_STATE,
	SUPER_PAUSED_STATE,
} SUPER_STATE_T ;

// FSM Super events
typedef enum
{
	SUPER_POLL_EVENT,
	SUPER_RESTART_PAUSE_REQ_EVENT,
	SUPER_SEARCH_PAUSE_REQ_EVENT,
	SUPER_PAUSE_EXP_EVENT,
	SUPER_RESTART_EVENT,
} SUPER_EVENT_T ;



// Complimentary FSM states
typedef enum
{
	COMPL_EMPTY_REPORTED_STATE,
	COMPL_EMPTY_STATE,
	
	COMPL_YING_STATE,
	COMPL_YING_YANG_STATE,
	COMPL_YING_YANG_YING_STATE,
	
	COMPL_YANG_STATE,
	COMPL_YANG_YING_STATE,
	COMPL_YANG_YING_YANG_STATE,
	
} COMPL_STATE_T ;

// Complimentary FSM Events
typedef enum
{
	COMPL_YING_EVENT,
	COMPL_YANG_EVENT,
	
	COMPL_NO_EVENT
} COMPL_EVENT_T ;

// Autosense FSM events
typedef enum
{
									//---begin enum header ---
									AUTOSENSE_FSM_EVENT_FIRST = 0,
									AUTOSENSE_FSM_EVENT_START = AUTOSENSE_FSM_EVENT_FIRST-1,
									//---end enum header ---
	
	//external FSM events
		// complementary pair - must remain together
		AUTOSENSE_DMT_FSM_RESET_EVENT,
		AUTOSENSE_TONE_DETECTED_EVENT,
	AUTOSENSE_TIMEOUT_EVENT,
	AUTOSENSE_RESTART_EVENT,
	
	// internal FSM events
	AUTOSENSE_PAUSED_TIMEOUT_EVENT,
	AUTOSENSE_PAUSED_RESTART_EVENT,

									//---begin enum trailer ---
									AUTOSENSE_FSM_EVENT_END,
									AUTOSENSE_FSM_EVENT_LAST = AUTOSENSE_FSM_EVENT_END-1
									//---end enum trailer ---
} AUTOSENSE_EVENT_T ;

// FSM Queued Events (successive to AUTOSENSE_EVENT_T )
typedef enum
{
									//---begin enum header ---
									AUTOSENSE_FSM_QUEUED_EVENT_FIRST = AUTOSENSE_FSM_EVENT_END,
									AUTOSENSE_FSM_QUEUED_EVENT_START = AUTOSENSE_FSM_QUEUED_EVENT_FIRST-1,
									//---end enum header ---
	
	AUTOSENSE_QUEUED_NULL_EVENT,	// never actually on queue, but may be returned from get queue routine
	AUTOSENSE_QUEUED_TD_OR_DFR_EVENT,	// TD=Tone Detected		DFR = DMT FSM Reset

									//---begin enum trailer ---
									AUTOSENSE_FSM_QUEUED_EVENT_END,
									AUTOSENSE_FSM_QUEUED_EVENT_LAST = AUTOSENSE_FSM_QUEUED_EVENT_END-1
									//---end enum trailer ---
	
} AUTOSENSE_QUEUED_EVENT_T ;

#define NUM_WIRINGS 4						// maximum for sizing - actual value is given by xcvr_other_cfg.sys_autowiring_num_combos
#define NUM_HANDSHAKES 2					// Actual number of handshakes not counting automatic selections
#define MAX_NUM_QUEUE_EVENTS	10

typedef struct
{
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
//    UINT32      autosense_detect_power[NUM_WIRINGS][NUM_HANDSHAKES];
    UINT16      autosense_queue_put_index;
    UINT16      autosense_queue_get_index;
//    UINT16      autosense_queue_data[MAX_NUM_QUEUE_EVENTS] ;
    UINT16      autosense_queue_max_usage;
}LOCAL_AUTOSENSE_TYPE;

DPCCALL void Auto_AutoSenseFSMReset (PDATAPUMP_CONTEXT pDatapump );
DPCCALL void auto_ClearAutoSenseQueue (PDATAPUMP_CONTEXT pDatapump );
DPCCALL void auto_PutAutoSenseQueue (PDATAPUMP_CONTEXT pDatapump, AUTOSENSE_QUEUED_EVENT_T event );
DPCCALL AUTOSENSE_QUEUED_EVENT_T auto_GetAutoSenseQueue (PDATAPUMP_CONTEXT pDatapump );
DPCCALL void Auto_GetAutoSense
(
	PDATAPUMP_CONTEXT             pDatapump,
	SYS_HANDSHAKE_TYPE			* AutoHandshake, 
	SYS_WIRING_SELECTION_TYPE	* AutoWiringSelection,
	BOOLEAN						* Autosense_WiresFirst
);
DPCCALL void Auto_GetAutoSenseStatus
(
	PDATAPUMP_CONTEXT             pDatapump,
	SYS_HANDSHAKE_TYPE			* Handshake,
	SYS_WIRING_SELECTION_TYPE	* Wiring,
	SYS_AUTO_SENSING_STATE_TYPE	* Autosense_State
);

DPCCALL void Auto_RestartAutoSense (PDATAPUMP_CONTEXT pDatapump );
DPCCALL void Auto_InitAutoSense (PDATAPUMP_CONTEXT pDatapump );
DPCCALL void Auto_SetAutoSense
(
	PDATAPUMP_CONTEXT             pDatapump,
	SYS_HANDSHAKE_TYPE			AutoHandshake, 
	SYS_WIRING_SELECTION_TYPE	AutoWiringSelection,
	BOOLEAN						Autosense_WiresFirst
);
DPCCALL void Auto_StartAutoSense
(
	PDATAPUMP_CONTEXT             pDatapump
);
DPCCALL void Auto_StopAutoSense
(
	PDATAPUMP_CONTEXT             pDatapump
);
DPCCALL BOOLEAN Auto_IsAutoSenseStartable
(
	PDATAPUMP_CONTEXT             pDatapump
);
DPCCALL void Auto_AutoSenseNoTone (PDATAPUMP_CONTEXT pDatapump );
DPCCALL void Auto_AutoSenseToneDetected (PDATAPUMP_CONTEXT pDatapump, UINT32 power );
DPCCALL void Auto_AutoSensePoll (PDATAPUMP_CONTEXT pDatapump );

#endif // _AUTOSENSE_H_
