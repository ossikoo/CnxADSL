/****************************************************************************
*  $Workfile: xcvrinfo.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/xcvrinfo.h 124   8/27/01 4:14p Lauds $
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
*  Original Written By:  David Lau                                          *
*  Original Date:  11/13/96                                                 *
*                                                                           *
*  Reviewed, Edited by  : Tiet Pham                                         *
*                       : May.28 97 to ....                                 *
*****************************************************************************
    Description:

        This is transceiver information(sysinfo) data. It contains typedefs and
        defines as well as data structure for all statistical, errored,
        alarm information. Each data structure shall have its own
        define to define the structure size using sizeof().
****************************************************************************/
#ifndef XCVRINFO_H
#define XCVRINFO_H

/****************************************************************************
                              INCLUDE SECTION
****************************************************************************/
#include "types.h"
#include "xcvrver.h"

typedef struct
{
    UINT8  teq_rx_chnl_start,        // active rx chnl range during TEQ
           teq_rx_chnl_end,          // training
           teq_update_start,         // teq chnl that we will update during TEQ
           teq_update_end;           // training
} FALCON_CONFIG_OPTION_TYPE;

/****************************************************************************
                           DEFINE
****************************************************************************/

#ifdef CONEXANT_AFE
#define    AFE_OPTION      "Conexant"


#else
#ifdef DATAPATH
#define    AFE_OPTION      "DataPath"
#else
#define    AFE_OPTION      "Harris"
#endif
#endif

#define ASIC_UPSTREAM_PAYLOAD_LIMIT     928000  // Upstream max payload bits/second

/* transceiver modes */
enum XCVR_MODE_CHOICE
{
   ATU_C,
   ATU_R
};

/****************************************************************************
                      HARDWARE INDEPENDENT CO DEFINITION
    Note:   1. DWN_STREAM is always at ATU_R side
            2. UP_STREAM  is alwayd at ATU_c side
            3. NEAR_END is always at the unit side you are working on
            4. FAR_END  is always at the other unit side.
****************************************************************************/
#ifdef CO
#define XCVR_MODE              ATU_C
#define ATU_STR                "ATU-C"
enum { UP_STREAM, DWN_STREAM };

/****************************************************************************
                      HARDWARE INDEPENDENT RT DEFINITION
****************************************************************************/
#else  /* ATU_R */
#define XCVR_MODE              ATU_R
#define ATU_STR                "ATU-R"
enum { DWN_STREAM, UP_STREAM };
#endif


/****************************************************************************
                       ATM CONFIG
****************************************************************************/
typedef struct
{
    BOOLEAN Disc_Idle_Cells;
    UINT8   TxSoc;
    UINT8   RxSoc;
} XCVR_ATM_CFG_TYPE;


/****************************************************************************
                       SELF TEST STATES DEFINITION
****************************************************************************/
typedef enum
{
    SELF_TEST,
    PROM_TEST,
    RAM_TEST,
    NVRAM_TEST,
    LED_TEST,
    BRIDGE_TEST,
    BIT_ERROR_TEST,
    FALCON_TEST,
    BERT_TEST,
    MAX_TEST_MODE
}XCVR_SELFTEST_ID_TYPE;


// transceiver operation states
enum
{
    XCVR_IDLE_MODE           = 0x00,
    XCVR_DATA_MODE,
    XCVR_HANDSHAKE_STARTUP   = 0x10,
    XCVR_TRAINING_STARTUP    = 0x18,
    XCVR_ANALOG_LB_TST       = 0x80,
    XCVR_DTE_HOST_LB_TST,
    XCVR_DTE_CORE_LB_TST,
    XCVR_DIGITAL_LB_TST,
    XCVR_SPECTRUM_TST
};

/****************************************************************************
    Note:   1. DWN_STREAM is always at ATU_R side
            2. UP_STREAM  is alwayd at ATU_c side
            3. NEAR_END is always at the unit side you are working on
            4. FAR_END  is always at the other unit side.
    These two sometimes will intermix when indexing to SYS_measurement[]

****************************************************************************/

enum {NEAR_END, FAR_END, NUM_ATU_MODE};


enum FALCON_SUPPORT_PATH
{
    PATH_AS0,
    PATH_AS1,
    PATH_LS0,
    PATH_LS1
};

/****************************************************************************
              DMT ADSL overhead control channel (AOC) DEFINE
****************************************************************************/
// refer to T1E1,4/95 section 13 for details on the following definition

enum AOC_MSG_HEADER
{
    AOC_IDLE_MODE              = 0x00,
    AOC_PROFILE_MGMT           = 0x03,
    AOC_RECONFIG_CMD           = 0x0F,
    AOC_UNABLE_TO_COMPLY       = 0xF0,
    AOC_EXT_BIT_SWAP           = 0xFC,
    AOC_BIT_SWAP               = 0xFF
};

enum AOC_BIT_SWAP_REQ_CMD
{
    BIT_SWAP_DO_NOTHING          = 0x00,
    BIT_SWAP_ADD_1_BIT           = 0x01,
    BIT_SWAP_MINUS_1_BIT         = 0x02,
    BIT_SWAP_ADD_1_DB_TXGAIN     = 0x03,
    BIT_SWAP_ADD_2_DB_TXGAIN     = 0x04,
    BIT_SWAP_ADD_3_DB_TXGAIN     = 0x05,
    BIT_SWAP_MINUS_1_DB_TXGAIN   = 0x06,
    BIT_SWAP_MINUS_2_DB_TXGAIN   = 0x07
};

#ifdef TOMCAT
#define MAX_PROFILE_NUM     0
#else
#define MAX_PROFILE_NUM     7   // profile number can be from (0 to MAX_PROFILE_NUM)
#endif

enum AOC_PROFILE_MGMT_REQ_CMD
{
    PROFILE_MGMT_REQ         = 0x01,
    PROFILE_MGMT_ACK         = 0x02
};


typedef struct
{
    UINT8 command;             // contains one of the AOC_BIT_SWAP_REQ_CMD
    UINT8 chnl_index;          // subchnl index for the bit swapping
}BIT_SWAP_MSG_TYPE;

// definition of a bit swap acknowledg message
typedef struct
{
    UINT8               all_1s;          // 8 binary ones indicating bit sway acknowledge
    UINT8               superframe_cnt;  // bit swap superframe counter number
}BIT_SWAP_ACK_TYPE;


typedef union
{
    BIT_SWAP_MSG_TYPE   ext_bit_swap_msg[6]; // this contains the 6 ext bit swap msg fields
    BIT_SWAP_MSG_TYPE   bit_swap_msg[4];     // this contains the 4 bit swap msg fields
    BIT_SWAP_ACK_TYPE   bit_swap_ack;
    UINT8               data[1];             // ???
}AOC_MSG_TYPE;



/****************************************************************************
                      SYSTEM MESSAGE DEFINITION
****************************************************************************/
enum SYS_MSG {UNABLE_TO_COMPLY,
              MEM_BLK_RD_REQ,
              MEM_BLK_RD_REPLY,
              MEM_BLK_WR_REQ,
              MEM_BLK_WR_REPLY,
              RESET_DATABASE,
              BRIDGE_BLK_RD_REQ,
              BRIDGE_BLK_RD_REPLY,
              BRIDGE_BLK_WR_REQ,
              BRIDGE_BLK_WR_REPLY,
              ASIC_BLK_RD_REQ,
              ASIC_BLK_RD_REPLY,
              ASIC_BLK_WR_REQ,
              ASIC_BLK_WR_REPLY,
              RUN_SELFTEST,
              SOFT_RESET,
              BIT_SWAPPING_MODE,
              REMOTE_LOGON_MODE,
              EQUALIZER_MODE,
              DAYLIGHT_SAVING_MODE,
              SET_SYSTEM_DATE,
              REMOTE_LOGON_CONTROL,
              REMOTE_LOGON_REQ,
              REMOTE_LOGON_REPLY,
              TRANSCEIVER_TEST_MODE,
              DIAG_LOG_CONTROL,
              EOC_NAK_CMD,
              NUM_OF_SYS_CMD
            };


/****************************************************************************
                   TRANSCEIVER CONFIGURATION
****************************************************************************/

//typedef GEN_UINT PDATAPUMP_CONTEXT;

typedef SINT16    MARGIN_TYPE;    // make sure it's signed type since we can have negative margin
#define Q_MARGIN    (8)         // Q8 type 
#define CONVERT_MARGIN_TYPE(x)    ((MARGIN_TYPE)(x)<<Q_MARGIN)
#define UNCONVERT_MARGIN_TYPE(x)    ((MARGIN_TYPE)(x)>>Q_MARGIN)


// Use for the ADSL configuration menu: AdslXcvrCfgScrDisp() in dpudmt.c
// The option ranges from 0dB to 15dB.  Value of 16dB shall be used to indicate
// 'ignore' which means that as long as the startup attemp is successful, the
// system shall enter 'DATA' regardless of the measured signal quality.

#define MAX_STARTUP_SNR         15   // max startup snr margin (dB)
#define MIN_STARTUP_SNR         0   // min startup snr margin (dB)
#define NUM_STARTUP_SNR_MAR_OPT (MAX_STARTUP_SNR - MIN_STARTUP_SNR + 1)
#define DFLT_STARTUP_SNR        6   // Unit in dB: default setting

// use the max snr option to be the IGNORE mode
#define IGNORE_MARGIN_MODE      31      // 31 is the agreed value for ignore mode

// The following enum shall be used as index for displaying also:
enum
{
    FIXED_RATE_TEST,
    FIXED_RATE_NORMAL,
    ADAPTIVE_RATE,
    ADAPT_AT_RUN_TIME,
    MAX_STARTUP_OP
};

#define MIN_DATA_RATE                 64  // 64 kbps
#define MAX_UP_DATA_RATE             928  // 928 kbps
#define MAX_UP_DATA_RATE_LITE        928  // 928 kbps
#define MAX_DWN_DATA_RATE           8128  // 8128 kbps  max rate if RS diabled and reduced overhead
#define MAX_DWN_DATA_RATE_LITE      1536  // 1536 kbps
// full rate and lite bits per channel
#define MAX_BITS_PER_CHNL             15 

#define FULLRATE_BITS_PER_CHNL        (XCVR_MODE == ATU_C ? MAX_BITS_PER_CHNL : 12)  // limit to 12 bit per chnl upstream to avoid >928 kbps
#define LITE_BITS_PER_CHNL            10


/* Basic configuration */
typedef struct
{
    UINT8  startup_op;                  // startup option:  FIXED_RATE_TEST
                                        //                  FIXED_RATE_NORMAL
                                        //                  ADAPTIVE_RATE
                                        //                  ADAPT_AT_RUN_TIME
    SINT8  up_startup_snr_mar;          // Unit in dB.
    UINT8  up_max_data_rate;            // Expressed in multiples of 32kbps
    UINT8  up_max_data_rate_lite;            // Expressed in multiples of 32kbps
    UINT8  dwn_max_data_rate;           // Expressed in multiples of 32kbps
    UINT8  dwn_max_data_rate_lite;      // Expressed in multiples of 32kbps
    UINT8  up_min_data_rate;            // Expressed in multiples of 32kbps
    UINT8  dwn_min_data_rate;           // Expressed in multiples of 32kbps
    SINT8  up_transmit_pwr;             // max nominal up_transmit_pwr in dBm/Hz from
    SINT8  down_transmit_pwr;           // max nominal down_transmit_pwr in dBm/Hz from
    SINT8  max_snr_mar;                 // max allowable SNR before an upshift
                                        //  in data rate is considered
    SINT8  min_snr_mar;                 // min allowable SNR before an downshift
                                        //in data rate is considered
    UINT16 upshift_time;                // min time (in sec) that the current margin
                                        // is above the max SNR margin before an
                                        // upshift in data rate occurs.
    UINT16 dwnshift_time;               // min time (in sec) that the current margin
                                        // is below the min SNR margin before an
                                        // downshift in data rate occurs.
    UINT8  dwn_data_rate_limit;         // Expressed in multiples of 32kbps
    UINT8  up_max_interleave_option;    // up interleaving option in msec
    UINT8  dwn_max_interleave_option;   // down interleaving option in msec

    UINT8  bandwidth_alloc;
    UINT8  framing_mode;                // desired framing mode for C_MSGS1
    UINT8  up_payload_path;             // fast or interleave path for upstream
    UINT8  down_payload_path;           // fast or interleave path for downstream
    SINT8  dwn_startup_snr_mar;         // Unit in dB.
} XCVR_CFG_TYPE;

#define   XCVR_CFG_SIZE   sizeof(XCVR_CFG_TYPE)

#define XCVR_DEFLT_CFG                                              \
{                                                                   \
    ADAPTIVE_RATE,          /*  data rate option                */  \
    DFLT_STARTUP_SNR,       /*  up_startup_snr_mar              */  \
    MAX_UP_DATA_RATE/32,    /*  up_max_data_rate                */  \
    MAX_UP_DATA_RATE_LITE/32,    /*  up_max_data_rate                */  \
    MAX_DWN_DATA_RATE/32,   /*  dwn_max_data_rate   */  \
    MAX_DWN_DATA_RATE_LITE/32,   /*  dwn_max_data_rate_lite    */  \
    MIN_DATA_RATE/32,       /*  up_min_data_rate                */  \
    MIN_DATA_RATE/32,       /*  dwn_min_data_rate               */  \
    0,                      /*  max dB from nominal up_transmit_pwr   */  \
    0,                      /*  max dB from nominal down_transmit_pwr */  \
    0,                      /*  max_snr_margin                  */  \
    0,                      /*  min_snr_margin                  */  \
    0,                      /*  upshift_time                    */  \
    0,                      /*  dwnshift_time                   */  \
    MAX_DWN_DATA_RATE/32,   /*  dwn_data_rate_limit  */  \
    4,                      /*  up interleaving option in msec  */  \
    4,                      /*  down interleaving option in msec*/  \
    0,                      /*  bandwidth alloc                 */  \
    DESIRED_OVERHEAD_MODE,  /*  desired framing mode for C_MSGS1*/  \
    INTERLEAVED_BUFFER,     /*  fast or interleave path for upstream   */ \
    INTERLEAVED_BUFFER,     /*  fast or interleave path for downstream */ \
    DFLT_STARTUP_SNR        /*  dwn_startup_snr_mar              */  \
}

// The number of alarm used for dumb terminal
// EXCESS_LOF, EXCESS_LOS, EXCESS_SES alarm history will not be displayed
// on dumb terminal
#define   NUM_OF_DUMB_ALARM    7

// types of alarm to be used in AdslAlarmCfgVarDisp() in dpudmt.c
// The order of alarm should be the same as that in CAP, except for LOS, DI.
// SELFTEST alarm is exceptional because it will not be included in the
// interface to DSLAM and NMA.

#ifdef ES
    #undef ES   // remove Linux def for ES for this one
#endif
enum ALARM_EVENT
{
    // DMT alarm ,for both dumb terminal and NMA
    LOF,        // lof_alarm
    MARGIN,     // low_snr_margin_alarm
    ES,         // es_alarm
    SES,        // ses alarm
    LOS,        // los_alarm
    LCD,        // lcd_alarm
    // system alarm  , for both dumb terminal and NMA
    SELFTEST,   // selftest alarm
    NUM_OF_DUMB_DISP_ALARM, //   number of alarm that dumb terminal can display.
    // NMA only alarm
    EXCESS_LOF = NUM_OF_DUMB_DISP_ALARM, // 15-minute LOF exceed threshold
    EXCESS_SES, // 15-minute SES exceed threshold
    EXCESS_UAS, // 15-minute UAS exceed threshold
    EXCESS_LOS, // 15-minute LOS exceed threshold
    EXCESS_LCD, // 15-minute LCD exceed threshold
    INIT_FAILURE,           // Initialization Failure
    NUM_OF_SMRT_DISP_ALARM, // number of alarm displayed in smart terminal
                            // CAP has some reserved alarm. Once these
                            // reserved alarm are used up,
                            // NUM_OF_SMRT_DISP_ALARM should be replaced
                            // by NUM_OF_ALARMALLOC.

    NUM_OF_ALARMALLOC = NUM_OF_SMRT_DISP_ALARM
};

#define ALARM_STRING                                  \
{                                                     \
   "LOF",        /* lof_alarm                      */ \
   "MAR",        /* low_snr_margin_alarm           */ \
   "ES",         /* es_alarm                       */ \
   "SES",        /* sever error second             */ \
   "LOS",        /* los_alarm                      */ \
   "LCD",        /* lcd_alarm                      */ \
   "SELF",       /* self test                      */ \
   "ELOF",       /* excessive LOF within 15 minute */ \
   "ESES",       /* excessive SES within 15 minute */ \
   "EUAS",       /* excessive UAS within 15 minute */ \
   "ELOS",       /* excessive LOS within 15 minute */ \
   "ELCD",       /* excessive LCD within 15 minute */ \
   "FINIT"       /* Initialization Failure */ \
}

/* Alarm Configuration */
typedef struct
{
    UINT8  alarm_cfg[NUM_OF_ALARMALLOC];      // alarms configuration (list)
    UINT8  unused;
    UINT16 mar_threshold[NUM_ATU_MODE];       // margin Threshold
    UINT16 es_threshold[NUM_ATU_MODE];        // ES threshold
    UINT16 lof_15min_threshold[NUM_ATU_MODE]; // 15 minute threshold for the number of LOF occured
    UINT16 ses_15min_threshold[NUM_ATU_MODE]; // 15 minute threshold for the number of SES occured
    UINT16 uas_15min_threshold[NUM_ATU_MODE]; // 15 minute threshold for the number of UAS occured
    UINT16 los_15min_threshold[NUM_ATU_MODE]; // 15 minute threshold for the number of LOS occured
    UINT16 lcd_15min_threshold[NUM_ATU_MODE]; // 15 minute threshold for the number of LOS occured
}XCVR_ALM_CFG_TYPE;

#define   XCVR_ALM_CFG_SIZE   sizeof(XCVR_ALM_CFG_TYPE)
#define XCVR_DEFLT_ALM_CFG                                          \
{                                                                   \
    {1,1,1,1,1,1,1,0,0,0,0,0,0},/*  alarms configuration (list)     */  \
    0,                      /*  unused                          */  \
    {3,   3  },             /*  margin Threshold                */  \
    {100, 100},             /*  ES threshold                    */  \
    {100, 100},             /*  15 minute threshold for the number of LOF occured */  \
    {100, 100},             /*  15 minute threshold for the number of LOS occured */  \
    {100, 100},             /*  15 minute threshold for the number of SES occured */  \
    {100, 100},             /*  15 minute threshold for the number of UAS occured */  \
    {100, 100}              /*  15 minute threshold for the number of LCD occured */  \
}

// see Annex D of T1.413 for details.
enum ADSL_VENDORS
{
    UNUSED_VENDOR_ID_0        =  0x0000,
    UNUSED_VENDOR_ID_1        =  0x0001,
    WESTELL_ID                =  0x0002,
    ECI_TELECOM_ID            =  0x0003,
    TEXAS_INSTRUMENTS_ID      =  0x0004,
    INTEL_ID                  =  0x0005,
    AMATI_ID                  =  0x0006,
    GENERAL_DATA_COM_ID       =  0x0007,
    LEVEL1_COM_ID             =  0x0008,
    CRYSTAL_SEMI_ID           =  0x0009,
    LUCENT_ID                 =  0x000A,
    AWARE_ID                  =  0x000B,
    BROOKTREE_ID              =  0x000C,
    NEC_ID                    =  0x000D,
    SAMSUNG_ID                =  0x000E,
    NORTHERN_TELECOM_ID       =  0x000F,
    PAIRGAIN_ID               =  0x0010,
    PARADYNE_ID               =  0x0011,
    ADTRAN_ID                 =  0x0012,
    INC_ID                    =  0x0013,
    ADC_TELECOM_ID            =  0x0014,
    MOTOROLA_ID               =  0x0015,
    IBM_ID                    =  0x0016,
    NEWBRIDGE_ID              =  0x0017,
    DSC_ID                    =  0x0018,
    TELTREND_ID               =  0x0019,
    EXAR_ID                   =  0x001A,
    SIEMENS_SC_ID             =  0x001B,
    ANALOG_DEVICES_ID         =  0x001C,
    NOKIA_ID                  =  0x001D,
    ERICSSON_ID               =  0x001E,
    TELLABS_ID                =  0x001F,
    ORCKIT_ID                 =  0x0020,
    AWA_ID                    =  0x0021,
    ALCATEL_NETWORK_ID        =  0x0022,
    NATIONAL_SEMI_ID          =  0x0023,
    ITALTEL_ID                =  0x0024,
    SAT_ID                    =  0x0025,
    FUJITSU_ID                =  0x0026,
    MITEL_ID                  =  0x0027,
    CONKLIN_ID                =  0x0028,
    DIAMOND_LANE_ID           =  0x0029,
    CABLETRON_ID              =  0x002A,
    DAVICOM_ID                =  0x002B,
    METALINK_ID               =  0x002C,
    PULSECOM_ID               =  0x002D,
    US_ROBOTICS_ID            =  0x002E,
    AG_COM_ID                 =  0x002F,
    CONEXANT_ID               =  0x0030,
    HARRIS_ID                 =  0x0031,
    HAYES_ID                  =  0x0032,
    CO_OPTIC_ID               =  0x0033,
    NETSPEED_ID               =  0x0034,
    THREE_COM_ID              =  0x0035,
    COPPER_MOUNTAIN_ID        =  0x0036,
    SILICON_AUTOMATION_SYS_ID =  0x0037,
    ASCOM_ID                  =  0x0038,
    GLOBESPAN_ID              =  0x0039,
    ST_MICRO_ID               =  0x003A,
    COPPERCOM_ID              =  0x003B,
    COMPAQ_ID                 =  0x003C,
    INTEGRATED_TECH_ID        =  0x003D,
    BAY_NETWORK_ID            =  0x003E,
    NEXT_LEVEL_COM_ID         =  0x003F,
    MULTI_TECH_SYS_ID         =  0x0040,
    AMD_ID                    =  0x0041,
    SUMITOMO_ID               =  0x0042,
    PHILIPS_ID                =  0x0043,
    EFFICIENT_ID              =  0x0044,
    INTERSPEED_ID             =  0x0045,
    CISCO_ID                  =  0x0046,
    TOLLGRADE_ID              =  0x0047,
    CAYMAN_ID                 =  0x0048,
    FLOWPOINT_ID              =  0x0049,
    IC_COM_ID                 =  0x004A,
    MATSUSHITA_ID             =  0x004B,
    SIEMENS_SEMI_ID           =  0x004C,
    DIGITAL_LINK_ID           =  0x004D,
    DIGITEL_ID                =  0x004E,
    ALCATEL_MICRO_ID          =  0x004F,
    CENTILLIUM_ID             =  0x0050,
    APPLIED_DIGITAL_ACCESS_ID =  0x0051,
    SMART_LINK_ID             =  0x0052,
    NUM_NORMAL_VENDOR_ID,
    MEDIALINCS_ID             =  0xB6DB  // Medialincs has an abnormal vendor ID
};

// Handshake Protocols supported
typedef enum SYS_HANDSHAKE
{
    GHS_TONE_MODE,                      // G.hs first
    TONE_GHS_MODE,                      // tone first, will remove later (for smart terminal now)
    G_HANDSHAKE_MODE,                   // G.HS
    G_TONE_MODE,                        // tone-based (i.e., normal T1.413)
    
    NUM_PROTOCOLS,
    PLACE_HOLDER=NUM_PROTOCOLS-1,       // decrement count so NUM_PROTOCOLS is not taking up a value   

    G_DMT_MODE,                         // Not a real value - used for cfging autosense only - translated to G_HANDSHAKE_MODE
    G_LITE_MODE                         // Not a real value - used for cfging autosense only - translated to G_HANDSHAKE_MODE
} SYS_HANDSHAKE_TYPE;

typedef enum SYS_WIRING_SELECTION
{
    WIRING_COMBO_AUTO,            // "Automatic"
    WIRING_COMBO1,                // "Line Tip/Ring"
    WIRING_COMBO2,                // "Line A/A1"
    WIRING_COMBO3,                // "Aux Tip/Ring"
    WIRING_COMBO4,                // "Aux A/A1"
} SYS_WIRING_SELECTION_TYPE;


// state of the auto sense FSM
typedef enum SYS_AUTO_SENSING_STATE
{
    AUTO_SENSE_SEARCHING,
    AUTO_SENSE_LOCKED,
    AUTO_SENSE_WIRING_LOCKED_ON_LAST,
    AUTO_SENSE_WIRING_LOCKED_ON_DEFAULT,
    AUTO_SENSE_RESTART_PAUSE,
    AUTO_SENSE_PERMUTE_PAUSE,
    AUTO_SENSE_FIXED,
} SYS_AUTO_SENSING_STATE_TYPE;

#define AUTOSENSE_WIRING_COMBO_MAX_NUM_COMBO 4  // maximum number of combinations of relay settings
#define AUTOSENSE_WIRING_COMBO_MAX_NUM_GPIO 4   // maximum number of GPIOs that must be asserted to select a relay combo


// Standar EOC Channel config
// Use this Bit to indicate which reg still not read from RT
#define     EOC_REG_VENID_CHECK         0x0001
#define     EOC_REG_REV_CHECK           0x0002
#define     EOC_REG_SN_CHECK            0x0004
#define     EOC_REG_STR_CHECK           0x0008
#define     EOC_REG_VEN0_CHECK          0x0010
#define     EOC_REG_VEN1_CHECK          0x0020
#define     EOC_REG_ATN_CHECK           0x0040
#define     EOC_REG_SNR_CHECK           0x0080
#define     EOC_REG_CONF_CHECK          0x0100
#define     EOC_REG_RESERVED0_CHECK     0x0200
#define     EOC_REG_STATE_CHECK         0x0400
#define     EOC_REG_RESERVED1_CHECK     0x0800
#define     EOC_REG_RESERVED2_CHECK     0x1000
#define     EOC_REG_RESERVED3_CHECK     0x2000
#define     EOC_REG_RESERVED4_CHECK     0x4000
#define     EOC_REG_RESERVED5_CHECK     0x8000

// For default, when reach showtime, read vendor_id and serial number
// SNR and attenuation will read each second by setting from nvram (XCVR_OTHER_CFG)
#define     EOC_REG_CHECK_ONCE_DEFLT    (EOC_REG_VENID_CHECK|EOC_REG_REV_CHECK|EOC_REG_SN_CHECK)
// Max supportted check items
#define     EOC_REG_CHECK_ONCE_MAX      (EOC_REG_VENID_CHECK|EOC_REG_REV_CHECK|EOC_REG_SN_CHECK|EOC_REG_STR_CHECK\
                                            |EOC_REG_VEN0_CHECK|EOC_REG_VEN1_CHECK)
#define     EOC_REG_CHECK_SEC_DEFLT     (EOC_REG_ATN_CHECK|EOC_REG_SNR_CHECK)

#define VENDOR_ID_SIZE              8   // G.lite = 8, T1.413 and G.dmt only use 2
#define NUMBER_OF_ADSL_OPTIONS     20   // don't put this in the enumeration, used in other enum type

enum
{
    ADSL_OPTION1 = 0,
    ADSL_MARGIN_ADJUST = ADSL_OPTION1,    
    ADSL_OPTION2,
    ADSL_FEC_THRESHOLD = ADSL_OPTION2,    
    ADSL_OPTION3,
    ADSL_PWR_BOOST_FACTOR = ADSL_OPTION3, 
    ADSL_OPTION4,
    ADSL_MAX_RX_BIT_PER_CHNL = ADSL_OPTION4, 
    ADSL_OPTION5,
    ADSL_MAX_SEC_CRC_DROP = ADSL_OPTION5,
    ADSL_OPTION6,
    ADSL_MAX_CRC_DROP = ADSL_OPTION6,
    ADSL_OPTION7,
    ADSL_OPTION8,
    ADSL_OPTION9,
    ADSL_OPTION10,
    ADSL_OPTION11,
    ADSL_OPTION12,
    ADSL_OPTION13,
    ADSL_OPTION14,
    ADSL_OPTION15,
    ADSL_OPTION16,
    ADSL_OPTION17,
    ADSL_OPTION18,
    ADSL_OPTION19,
    ADSL_OPTION20,
};

enum
{
    HEAD_END_ENV_NON_SPECIFIC = 0,  // Non-Specific"
    HEAD_END_ENV_NO_LINE_DRIVER     // No Line Driver (BNA)
};

enum
{
    NON_SPECIFIC_ADSL_TELCO_ID=0,
    HANARO_ADSL_TELCO_ID,
    CHUNG_HO_ADSL_TELCO_ID,
    CHUNG_HWA_ADSL_TELCO_ID,
    NUM_ADSL_TELCO_D
};

/* Other Configuration -- Do not need transfter to other end when both side is save vendor product
 * Include -- G.hs configuration
 *         -- Eoc, Aoc channel configuration
 *         -- Other configuration need to save to NVRAM
 * Note: This configuration will keep in NVRAM and load when power/warm up
 *       duplicate to global database for faster access
 *       remember reload it after make change in NVRAM
 *       TRY MOVE MOST DEFINATION TO HERE!!!
 *
 * NOTE:  If you add/delete anything from this structure, you will need to add/
 *  delete from the "default value" definitions below to keep everything aligned
 *  properly.
 */
typedef struct
{
    // EOC Related
    UINT8                   vendor_id[VENDOR_ID_SIZE];  // msb first
    UINT16                  stdeoc_check_once_list;     // which need to read from RT at ST only once
    UINT16                  stdeoc_check_sec_list;      // which need read each seconds
    BOOLEAN                 stdeoc_clear_enable;        // enable clear eoc channel
    // AOC Related
    BOOLEAN                 bitswap_enable;             // true will enable bit swap command in AOC channel
    BOOLEAN                 trellis_option;             // trellis option
    BOOLEAN                 sys_autosense_wiresfirst;
    SYS_WIRING_SELECTION_TYPE   sys_autowiring_selection;
    UINT8                   sys_autowiring_num_combos;
    UINT16                  AdslHeadEnd;
    UINT8                   AdslHeadEndEnvironment;
    UINT16                  AdslTelcoId;
    UINT16                  AdslCRCPerSES;
    UINT16                  AdslSESToRetrain;
    GEN_UINT                AdslSNR_SecsToRetrain;
    GEN_UINT                AdslSES_Sensitivity;
    SYS_HANDSHAKE_TYPE      sys_autohandshake;
    UINT16                  CAP_common;      // Standard Info NPar(1) capabilities
    UINT16                  CAP_standards;
// Fast Retrain Line Probe Related
    BOOLEAN                 fast_retrain_enable;
    BOOLEAN                 escape_to_fast_retrain_enable;
#ifdef RT
    UINT8                   max_rt_cutback;
    UINT8                   max_co_rel_cutback;
    UINT8                   min_rt_cutback;
    UINT8                   min_co_rel_cutback;
    UINT8                   cutback_step;
    UINT8                   max_phone_distortion;
    SINT8                   min_noise_floor;
    BOOLEAN                 offhook_detect;  // TRUE will enable hook detector
#endif
    UINT16                  AdslOptions[NUMBER_OF_ADSL_OPTIONS];
}XCVR_OTHER_CFG_TYPE;

#define   XCVR_OTHER_CFG_SIZE   sizeof(XCVR_OTHER_CFG_TYPE)


#ifdef CO
#define XCVR_DEFLT_OTHER_CFG                                                    \
{                                                                               \
    {0,0,0,0,0,0,CONEXANT_ID>>8,CONEXANT_ID},                                   \
    EOC_REG_CHECK_ONCE_DEFLT,                                                   \
    EOC_REG_CHECK_SEC_DEFLT,                                                    \
    TRUE,                                                                       \
    FALSE,          /* Not enable bit swap */                                   \
    TRUE,           /* enable trellis option */                                 \
    FALSE,          /*  permute wires first */                                  \
    1,              /*  auto wiring selection */                                \
    3,              /*  sys_autowiring_num_combos */                            \
    0,              /* AdslHeadEnd */                                           \
    HEAD_END_ENV_NON_SPECIFIC,  /* AdslHeadEndEnvironment */                    \
    NON_SPECIFIC_ADSL_TELCO_ID, /* AdslTelcoId */                               \
    14,             /* AdslCRCPerSES */                                         \
    5,              /* AdslSESToRetrain */                                      \
    0,              /* AdslSNR_SecsToRetrain */                                 \
    0,              /* AdslSES_Sensitivity */                                   \
    GHS_TONE_MODE,  /*  auto handshake mode */                                  \
    0,                                                                          \
    ANNEX_MODE == ANNEX_B ? CAP_STANDARD_G9921B : CAP_STANDARD_G9921A|CAP_STANDARD_G9922AB,  \
    TRUE,           /*  default enable fast retrain*/                           \
    FALSE,           /*  default enable escape to fast retrain*/                 \
    {0},            /*  AdslOptions[NUMBER_OF_ADSL_OPTIONS] */                  \
}
#else                                   // RT

#define XCVR_DEFLT_OTHER_CFG                                                    \
{                                                                               \
    {0,0,0,0,0,0,CONEXANT_ID>>8,CONEXANT_ID},                                   \
    EOC_REG_CHECK_ONCE_DEFLT,                                                   \
    EOC_REG_CHECK_SEC_DEFLT,                                                    \
    TRUE,                                                                       \
    FALSE,          /* Not enable bit swap */                                   \
    TRUE,           /* Trellis Problem is fixed. */ \
    FALSE,          /*  permute wires first */                                  \
    1,              /*  auto wiring selection */                                \
    3,              /*  sys_autowiring_num_combos */                            \
    0,              /* AdslHeadEnd */                                           \
    HEAD_END_ENV_NON_SPECIFIC,  /* AdslHeadEndEnvironment */                    \
    NON_SPECIFIC_ADSL_TELCO_ID, /* AdslTelcoId */                               \
    14,             /* AdslCRCPerSES */                                         \
    5,              /* AdslSESToRetrain */                                      \
    0,              /* AdslSNR_SecsToRetrain */                                 \
    0,              /* AdslSES_Sensitivity */                                   \
    GHS_TONE_MODE,  /*  auto handshake mode */                                  \
    0,                                                                          \
    ANNEX_MODE == ANNEX_B ? CAP_STANDARD_G9921B : CAP_STANDARD_G9921A|CAP_STANDARD_G9922AB,                                   \
    TRUE,           /*  default enable fast retrain*/                           \
    FALSE,           /*  default enable escape to fast retrain*/                 \
    20,             /*  max rt cutback dB */                                    \
    20,             /*  max co rel cutback dB*/                                 \
    0,              /*  min rt cutback dB*/                                     \
    0,              /*  min co rel cutback dB*/                                 \
    4,              /*  Line probe cutback step dB */                           \
    6,              /*  Max phone distortion power of 2 */                      \
    -30,            /*  Min noise floor power of 2 */                           \
    FALSE,          /*  Disable detect off hood */                              \
    {0},            /*  AdslOptions[NUMBER_OF_ADSL_OPTIONS] */                  \
}

#endif                                  // CO

/****************************************************************************
                       SYSTEM ERROR COUNT DEFINITION
****************************************************************************/

// total event count since last reset
typedef struct
{
    UINT32 uas;             // unavailable second
    UINT32 es;              // errored second
    UINT32 ses;             // severely errored second
    UINT32 lof;
    UINT32 los;             // only DMT suport it
    UINT32 cs;              // corrected second
} XCVR_TOTAL_EVENT_COUNT;

#define   XCVR_TOTAL_EVENT_SIZE   sizeof(XCVR_TOTAL_EVENT_COUNT)

typedef struct
{
    UINT32 febe;
    UINT32 crc;
} SYS_CNT_TYPE;

#define   SYS_CNT_SIZE   sizeof(SYS_CNT_TYPE)

#define   DEFAULT_SYS_COUNT  {0,0}

/****************************************************************************
                       XCVR MASTER CLOCK DEFINITION
****************************************************************************/
#define XCVR_MCLK                   35328   // master clock frequency, in Khz
                       //  [C +  A/(A+|B|)] * F_out * 2^div_cnt = Fin = MCLK

/****************************************************************************
                       FALCON XCVR RELATED DEFINITION
****************************************************************************/
// Note that volatile is essential any access to FALCON is not optimized
#define FALCON_OFFSET(xcvr_index, offset) (((UINT16 volatile far *)FALCON_START_ADDR(xcvr_index))[offset])

enum {UNUSED_CHNL,
      MONITOR_CHNL,
      DATA_BIT_CHNL};

// UNUSED_CHNL in tone_order_map means it's unused
// these values are also used in chnl_type field of rx bit tone table to indicate
// if chnl is used for monitoring or carry bits. Note that only MONITOR_CHNL
// (value = 0x1) can have lsb set. This is to set the monitor bit in FALCON.

// # of xmit super frame in one sec and # of rcv super frame in one sec,
// (i.e.) 1/(68/4kHz)

#define RCVR_SF_IN_SEC               58
#define TXMIT_SF_IN_SEC              58
#define NUM_OF_ADSL_CHNL            256     // # of possible DMT subchnls
#define NUM_UPSTREAM_CHNL           (ANNEX_MODE == ANNEX_A ? 32 : 64)
#define NUM_DOWNSTREAM_CHNL         256
#define NUM_EC_COEF                 256

// tx pwr density when tx gain is set to highest value: 0
#define XCVR_NORMINAL_UP_TX_POWER_DENSITY    -38       // -38 dbm/Hz
// tx pwr density when tx gain is set to highest value: 0
#define XCVR_NORMINAL_DWN_TX_POWER_DENSITY    -40       // -40 dbm/Hz

// Note the following has to use #define instead of enumeration, otherwise
// preprocessor doesn't recognize them correctly in upcoming #if
#define ANNEX_A      0
#define ANNEX_B      1
#define ANNEX_C      2

#define ANNEX_MODE  ANNEX_A

#if (ANNEX_MODE == ANNEX_A)
    // ref A3.8 and A3.9 of G992.1
#define UP_MESS_LO_START_TONE 10
#define UP_MESS_HI_START_TONE 20
#define DWN_MESS_LO_START_TONE 43
#define DWN_MESS_HI_START_TONE 91

#else
    // ref B3.9 and B3.10 of G992.1
    enum 
    {
        UP_MESS_LO_START_TONE   =  44,     
        UP_MESS_HI_START_TONE   =  49,     
        DWN_MESS_LO_START_TONE  =  75,     
        DWN_MESS_HI_START_TONE  =  91
    }ANNEX_B_MESS_TONE_SET;

#endif

#define ANNEX_B_UP_TONE_UNDER_32     (FALSE)      // default not to support upstream chnl < 32


// NUM_FIR_COEF maybe different for CO and RT
#define MAX_NUM_FIR_COEF           62
#define MAX_NUM_TEQ_COEF           64

#define DACSAMPLES                 (TX_OVER_SAMPLING_RATIO * N_VALUE)
#define ADCSAMPLES                 (RX_OVER_SAMPLING_RATIO * M_VALUE)
#define QEDFER_RX_CHNL             (RX_OVER_SAMPLING_RATIO * NUM_RX_CHNL)
#define NIFFT_VALUE                (TX_OVER_SAMPLING_RATIO * NUM_TX_CHNL)
#define NFFT_VALUE                 (RX_OVER_SAMPLING_RATIO * NUM_RX_CHNL)
#define NUM_TX_CHNL                (N_VALUE/2)
#define NUM_RX_CHNL                (M_VALUE/2)
#define ADCPHASE_PERIOD            (MCLK_SYMBOL/(ADCSAMPLES))

enum CHNL_NUM_CHOICE {CHNL_32, CHNL_64, CHNL_128, CHNL_256};

#define DEFAULT_ADC_SWEET_VALUE          0
#ifdef DATAPATH
#define ADFSV                         2.5   // A/D full scale voltage
#else
#define ADFSV                           2   // A/D full scale voltage
#endif
#define DEFAULT_FEQ_GAIN               0.5  // default gain for FEQ coef.
#define DEFAULT_DGC_FACTOR               4  // default Dynamic Gain scale factor
#define FFTDYN_GAIN                 0.0625  // init FFT dynamic gain to 1/16

// The following constants define constant 'k' in T1E1 for each tone:
// For CO:

#define C_TONE_TONE                     72  // = 310.5Khz/4.3125Khz
#define C_ACT1_TONE                     48  // = 207Khz/4.3125Khz
#define C_ACT2_TONE                     44  // = 189.75Khz/4.3125Khz
#define C_ACT3_TONE                     52  // = 224.25Khz/4.3125Khz
#define C_ACT4_TONE                     60  // = 258.75Khz/4.3125Khz
#define C_REVEILLE_TONE                 56  // = 241.5Khz/4.3125Khz
#define C_RECOV_TONE                    68

// For RT:

#define R_ACT_REQ_TONE                  8   // = 34.5Khz/4.3125Khz
#define R_ACK1_TONE                     10  // = 43.125Khz/4.3125Khz
#define R_ACK2_TONE                     12  // =
#define R_ACK3_TONE                     14  // = 60.375Khz/4.3125Khz
#define R_RECOV_TONE                    20

#define FEQ_LARGE_BETA                  FEQ_2NEG5
#define FEQ_MEDIUM_BETA                 FEQ_2NEG8
#define FEQ_SMALL_BETA                  FEQ_2NEG12
#define FEQ_VERY_SMALL_BETA             FEQ_2NEG20


#define DESIRED_OVERHEAD_MODE     MERGED_FAST_SYNC

/****************************************************************************
            TRELLIS FUNCTIONS FOR ADSL SYSTEM
****************************************************************************/



// 1 extra overhead bit is added for Trellis coding each pair of active chnls
// (round to even if active chnls are odd).  It needs 3 overhead bits for the
// last 2 pairs of active chnls. (i.e. We add extra four bits for overhead)
#define TRELLIS_OVERHEAD(num_active_chnls)   (((num_active_chnls)+1)/2 + 4)

/****************************************************************************
            BITFIELD TYPE FOR ADSL SYSTEM
****************************************************************************/

// message format for C_MSGS1 and R_MSGS1: 48-bit message
// Ref. Section 12.6.4 T1E1
//      Anex D, Vendor Identification numbers, T1E1
enum FRAMING_MODE_OPTION
{
    FULL_OVRHD_ENA_SYNC,    // full overhead with enabled sync control
    FULL_OVRHD_DIS_SYNC,    // full overhead with disabled sync control
    SEPARATE_FAST_SYNC,     // reduced overhead with separate fast and sync bytes
    MERGED_FAST_SYNC,       // reduced overhead with merged fast and sync bytes for single
                            // latency and separate fast and sync bytes for dual latency
    NUM_FRAMING_MODE_OPTIONS
};

enum INIT_TX_PSD_OPTION
{
    NEG_52_DBM_PER_HZ = 1,
    NEG_50_DBM_PER_HZ,
    NEG_48_DBM_PER_HZ,
    NEG_46_DBM_PER_HZ,
    NEG_44_DBM_PER_HZ,
    NEG_42_DBM_PER_HZ,
    NEG_40_DBM_PER_HZ
};


enum ISSUE_OPTION {ISSUE_1, ISSUE_2};


#ifdef LITTLE_ENDIAN_MEMORY

// Little Endian Version

typedef struct
{
    UINT16  bits_per_chnl      : 4;  // Max # bit/sub-chan
    UINT16                     : 2;  // Reserved
    UINT16  InitTxPSD          : 3;  // Tx PSD during initialization
    UINT16  framing_mode       : 2;  // 4 framing mode determining synchronization control
    UINT16  network_timing_ref : 1;  // a bit indicating if ATU_C will use
                                     // indicator bits ib23 to ib20 to support
                                     // reconstruction of the network timing reference
                                     // 1 : support ; 0 : not supported
    UINT16  max_tx_psd         : 2;  // Maximum tx pwr spectrum density
    UINT16                     : 1;  // Reserved
    UINT16  exp_exchg_seq      : 1;  // expanded exchange sequence
    UINT16  ec_option          : 1;  // Echo Cancelling option
    UINT16  trellis_option     : 1;  // trellis_option coding option
    UINT16  version            : 5;  // Vendor revision #
    UINT16  issue              : 3;  // Issue # - 1 (i.e. T1.413 revision #)
    UINT16                     : 1;  // Reserved
                                     // note that vendor id is broken down into
                                     // lo and hi to avoid compiler padded bits
    UINT16  U_ADSL_Ack         : 1;  // U-ADSL Acknowledgement
    UINT16  VendorID_lo        : 4;  // = 0x???(?) last 4 bit of vendor id
    UINT16  VendorID_hi        :12;  // = 0x(???)? first 12 bit of vendor id
    UINT16  min_snr_margin     : 4;  // Minimum required SNR margin in dB
}C_MSGS1_TYPE;


// Ref. Section 12.7.6 T1E1
typedef struct
{
    UINT16  bits_per_chnl      : 4;  // Max # bit/sub-chan
    UINT16                     : 5;  // Reserved
    UINT16  framing_mode       : 2;  // 4 framing mode determining synchronization control
    UINT16  network_timing_ref : 1;  // a bit indicating if ATU_C will use
                                     // indicator bits ib23 to ib20 to support
                                     // reconstruction of the network timing reference
                                     // 1 : support ; 0 : not supported
    UINT16  up_dual_latency    : 1;  // support of upstream dual latency
    UINT16  down_dual_latency  : 1;  // support of downstream dual latency
    UINT16  above_8Mbps        : 1;  // support of higher bit rates, have this
                                     // bit set support operation with S=1/2
                                     // thru interleaved A0.  This allows downstream
                                     // interleaved bit rate above 8Mbps
    UINT16  exp_exchg_seq      : 1;  // expanded exchange sequence
    UINT16  ec_option          : 1;  // Echo Cancelling option
    UINT16  trellis_option     : 1;  // trellis_option coding option
    UINT16  version            : 5;  // Version #
    UINT16  issue              : 3;  // Issue # - 1 (i.e. T1.413 revision #)
    UINT16                     : 2;  // Reserved
                                     // note that vendor id is broken down into
                                     // lo and hi to avoid compiler padded bits
    UINT16  VendorID_lo        : 4;  // = 0x???(?) last 4 bit of vendor id
    UINT16  VendorID_hi        :12;  // = 0x(???)? first 12 bit of vendor id
    UINT16                     : 4;  // Reserved
}R_MSGS1_TYPE;

// Ref. Section ??? issue 2 T1E1
typedef struct
{
    UINT16                     :16;  // Reserved  (set to 0)
    UINT16                     :16;  // Reserved  (set to 0)
    SINT16  max_noise_margin   : 6;  // Max noise margin in steady state
    SINT16  min_noise_margin   : 6;  // Min noise margin in steady state
    UINT16  min_req_snr        : 4;  // Tx PSD during initialization
}C_MSG_RA_TYPE;


// Ref. Section 9.9.2 issue 2 T1E1.4
enum MAX_INTER_DEPTH_OPTION {INTER_DEPTH_64,INTER_DEPTH_128,INTER_DEPTH_256,INTER_DEPTH_512};
typedef struct
{
    UINT16  bits_per_sym       :12;  // Max # bit/DMT symbol
    UINT16  max_inter_depth    : 2;  // Maximum interleave depth
    UINT16                     : 2;  // Reserved (set to 0)
    UINT16  performance_margin : 5;  // performance margin with selected rate option
    UINT16  coding_gain        : 4;  // coding gain in 0.5 db increment
    UINT16  loop_attenuation   : 7;  // estimated average loop attenuation
    UINT16  num_data_rate_chnl : 8;  // num of tones carrying payload
    UINT16  payload_lo         : 8;  // num of Reed Solomon payload bytes (8 lsb), K
    UINT16  payload_hi         : 1;  // num of RS payload bytes (1 msb), K
    UINT16  overhead           : 7;  // num of RS overhead bytes, (R)
    UINT16                     : 8;  // Reserved (set to 0)
    UINT16                     :16;  // Reserved (set to 0)
}R_MSG_RA_TYPE;


// message format for C_MSGS2 and R_MSGS2 (format refer to adsl standard)
// Ref: Section 12.8.3 T1E1

typedef struct
{
    UINT16  bits_per_sym       : 9;   // Total number of bits supported per sym
    UINT16                     : 7;   // Reserved = 0
    UINT16  performance_margin : 5;   // performance margin with selected rate option
    UINT16  erase_all_profile  : 1;   // erase all stored profiles
    UINT16                     : 4;   // Reserved = 0
    UINT16  loop_attenuation   : 6;   // estimated average loop attenuation
}C_MSGS2_TYPE;

// Ref: Section 12.9.2 T1E1

typedef struct
{
    UINT16  bits_per_sym       :12;   // Total number of bits supported per sym
    UINT16                     : 4;   // Reserved = 0
    UINT16  performance_margin : 5;   // Performance mar with selected rate
    UINT16  erase_all_profile  : 1;   // erase all stored profiles
    UINT16                     : 3;   // Reserved = 0
    UINT16  loop_attenuation   : 7;   // Estimated average loop attenuation
}R_MSGS2_TYPE;


// message format for C_RATES2 and R_RATES2 (format refer to adsl standard)
// This format combines the downstream rate info contained in R_RATES2 with
// the option number of the highest upstream data rate that can be
// supported based on the measure SNR of the upstream channel.  It thus
// transmit the final decision on the rates that will be used in both
// directions
// Ref: Section 12.8.5 T1E1

typedef struct
{
    UINT16 Upstream   : 4;
    UINT16 Downstream : 4;
    UINT16            : 8;
}C_RATES2_TYPE;



typedef struct
{
    UINT16 Downstream       : 4;
    UINT16 DownstreamMirror : 4;  // this mirror field contains
                                  // the same info as the other field for R_RATES2
                                  // (although might not be the same in R_RATES2_RA)
    UINT16                  : 8;
}R_RATES2_TYPE;

typedef struct
{
    UINT16  down_max_tx_psd       : 5;  // Fast Retrain Politeness Power Cutback PSD level, 2dB per step
    UINT16  fast_retrain_truncate : 1;  // trucate fast retrain and goes to full init
    UINT16                        : 2;  // Reserved (set to 0)
    UINT16                        : 8;
}C_MSG_FR1_TYPE;

typedef struct
{
    UINT16  down_max_tx_psd       : 5;  // Relative Downstream Fast Retrain Power Cutback PSD level, 2dB per step
    UINT16  fast_retrain_truncate : 1;  // trucate fast retrain and goes to full init
    UINT16                        : 2;  // Reserved (set to 0)
    UINT16  up_max_tx_psd         : 5;  // Absolute Upstream Fast Retrain Power Cutback PSD level, 2dB per step
    UINT16                        : 3;  // Reserved (set to 0)
}R_MSG_FR1_TYPE;

typedef struct
{
    UINT16  profile_index         : 4;  // profile index (0 to 15)
    UINT16  unknown_profile       : 1;  // (0) known profile, (1) unknown profile
    UINT16                        : 3;  // Reserved (set to 0)
    UINT16                        : 8;  // Reserved (set to 0)
}C_MSG_FR2_TYPE;

typedef struct
{
    UINT16  profile_index         : 4;  // profile index (0 to 15)
    UINT16  unknown_profile       : 1;  // (0) known profile, (1) unknown profile
    UINT16                        : 3;  // Reserved (set to 0)
    UINT16                        : 8;  // Reserved (set to 0)
}R_MSG_FR2_TYPE;


// message format for C_B&G and R_B&G (format refer to adsl standard)
// Ref: Section 12.8.5 T1E1
// NOTE: 'gain' is represented as 12-bit fix point binary with the binary
//          point assumed just to the right of the third msb.


typedef struct
{
    UINT16 bits : 4;     // # bits to be coded by ATU-R trasmitter onto the
                        // a upstream carrier.
    UINT16 gain :12;     // Scale factor, relative to the gain that was used
                        // for a carrier during the transmission of R_MEDLEY
}BITS_GAIN_TYPE;


// Data format for Tx and Rx bit tone table (format refer to FALCON user
// guide on transfer format to and from the MIF scratch pad RAM):
//  The QEDFER Transmit Bit/Tone Table is a 256*12-bit memory
//  Ref:    Section 12.9.4 QEDFER transmit Bit/Tone Table, FALCON user's guide
//          Section 6.6.4, Constellation Encoder, T1E1
typedef struct
{
    UINT16            : 2;   //  Unused
    UINT16 tag        : 1;   //  0: FFTA use FADJUST2 on sync symbol
                             //  1: FFTA use FADJUST1 on sync symbol
    UINT16 monitor    : 1;   //  monitor subchnl flag
    UINT16 tone       : 8;   //  QAM sub-chan. addr
    UINT16 bits       : 4;   // 'b' bits are to be assigned to a QAM sub-chan
                             //  ( 0 to 15 bits )
                             //  The hardware BPQE block shall read this value
                             //  and use it to extract number of bits out of
                             //  input data source, then perform QAM constellation
                             //  encoding on the data bits.
}TX_BIT_TONE_TYPE;


/***************************************************************************/
//  The QEDFER Receive Bit/Tone Table is a 256*18-bit memory.  Note that
//  the actual number of bits allocated for bit/tone table is still 12
//  the other 6 bits (in subsequence address) hold FEQ coefficient update
//  info and DGC gain info as defined in TONE_INFO_TYPE data structure
//  Ref:    Section 12.9.5 QEDFER receive Bit/Tone Table, FALCON user's guide
//          Section 3.2.8.1 Receive bit/tone table update, FALCON user's
//          guide

typedef struct
{
    UINT16 tone       : 8;   // Tone info
    UINT16 bits       : 4;   // Bit info
    UINT16 chnl_type  : 2;   // chnl type can be UNUSED_CHNL, MONITOR_CHNL, or DATA_BIT_CHNL
                             // lsb: monitor subchnl flag
                             // msb: for tone order sorting purpose (not used by FALCON)
    UINT16            : 2;   // unused
}RX_BIT_TONE_TYPE;          // Addr 0x306

// Data format for Rx tone info table (format refer to FALCON user
// guide on transfer format to and from the MIF scratch pad RAM)

typedef struct
{
    UINT16                  : 6;    // Unused
    UINT16 DGC              : 4;    // dynamic gain control (DGC)
    UINT16 FEQ_Adaptation   : 1;    // FEQ adaptation for that subchannel
                                    // (LMS info) or subchnl freeze control
                                    // 1 : Allow to update
                                    // 0 : Freeze
    UINT16 PRBS_Match       : 1;    // this field determines if the subchannel
                                    // is contributing to the PRBS match counter
    UINT16                  : 4;    // unused
}TONE_INFO_TYPE;                    // Addr, 0x307

/****************************************************************************
                       CRC FUNCTIONALITY DEFINITION
****************************************************************************/

//   12th, 5th and 0th coefficients set
//                                   CRC nth tap value
//                              1 1 1 1 1 1
//                              5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
#define CRC_CHECK_POLYNOMIAL  {{0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,1}}

typedef union
{
    struct
    {
        UINT16 tap_15 :1;
        UINT16 tap_14 :1;
        UINT16 tap_13 :1;
        UINT16 tap_12 :1;
        UINT16 tap_11 :1;
        UINT16 tap_10 :1;
        UINT16 tap_9  :1;
        UINT16 tap_8  :1;
        UINT16 tap_7  :1;
        UINT16 tap_6  :1;
        UINT16 tap_5  :1;
        UINT16 tap_4  :1;
        UINT16 tap_3  :1;
        UINT16 tap_2  :1;
        UINT16 tap_1  :1;
        UINT16 tap_0  :1;
    }bvalue;
    UINT16 wvalue;
}CRC_CHECK_TYPE;


#else
// Big Endian Version

typedef struct
{
    UINT16  exp_exchg_seq      : 1;  // expanded exchange sequence
    UINT16                     : 1;  // Reserved (set to 0)
    UINT16  max_tx_psd         : 2;  // Maximum tx pwr spectrum density
    UINT16  network_timing_ref : 1;  // a bit indicating if ATU_C will use
                                     // indicator bits ib23 to ib20 to support
                                     // reconstruction of the network timing reference
                                     // 1 : support ; 0 : not supported
    UINT16  framing_mode       : 2;  // 4 framing mode determining synchronization control
    UINT16  InitTxPSD          : 3;  // Tx PSD during initialization
    UINT16                     : 2;  // Reserved (set to 0)
    UINT16  bits_per_chnl      : 4;  // Max # bit/sub-chan

    UINT16  VendorID_lo        : 4;  // = 0x???(?) last 4 bit of vendor id
    UINT16  U_ADSL_Ack         : 1;  // U-ADSL Acknowledgement
    UINT16                     : 1;  // Reserved (set to 0)
                                     // note that vendor id is broken down into
                                     // lo and hi to avoid compiler padded bits
    UINT16  issue              : 3;  // Issue # - 1 (i.e. T1.413 revision #)
    UINT16  version            : 5;  // Version #
    UINT16  trellis_option     : 1;  // trellis_option coding option
    UINT16  ec_option          : 1;  // Echo Cancelling option

    UINT16  min_snr_margin     : 4;  // Minimum required SNR margin in dB
    UINT16  VendorID_hi        :12;  // = 0x(???)? first 12 bit of vendor id
}C_MSGS1_TYPE;


// Ref. Section 12.7.6 T1E1
typedef struct
{
    UINT16  exp_exchg_seq      : 1;  // expanded exchange sequence
    UINT16  above_8Mbps        : 1;  // support of higher bit rates, have this
                                     // bit set support operation with S=1/2
                                     // thru interleaved A0.  This allows downstream
                                     // interleaved bit rate above 8Mbps
    UINT16  down_dual_latency  : 1;  // support of downstream dual latency
    UINT16  up_dual_latency    : 1;  // support of upstream dual latency
    UINT16  network_timing_ref : 1;  // a bit indicating if ATU_C will use
                                     // indicator bits ib23 to ib20 to support
                                     // reconstruction of the network timing reference
                                     // 1 : support ; 0 : not supported
    UINT16  framing_mode       : 2;  // 4 framing mode determining synchronization control
    UINT16                     : 5;  // Reserved (set to 0)
    UINT16  bits_per_chnl      : 4;  // Max # bit/sub-chan

    UINT16  VendorID_lo        : 4;  // = 0x???(?) last 4 bit of vendor id
    UINT16                     : 2;  // Reserved (set to 0)
                                     // note that vendor id is broken down into
                                     // lo and hi to avoid compiler padded bits
    UINT16  issue              : 3;  // Issue # - 1 (i.e. T1.413 revision #)
    UINT16  version            : 5;  // Version #
    UINT16  trellis_option     : 1;  // trellis_option coding option
    UINT16  ec_option          : 1;  // Echo Cancelling option
    UINT16                     : 4;  // Reserved (set to 0)
    UINT16  VendorID_hi        :12;  // = 0x(???)? first 12 bit of vendor id
}R_MSGS1_TYPE;

// Ref. Section ??? issue 2 T1E1
typedef struct
{
    UINT16                     :16;  // Reserved  (set to 0)
    UINT16                     :16;  // Reserved  (set to 0)
    UINT16  min_req_snr        : 4;  // Tx PSD during initialization
    SINT16  min_noise_margin   : 6;  // Min noise margin in steady state
    SINT16  max_noise_margin   : 6;  // Max noise margin in steady state
}C_MSG_RA_TYPE;


// Ref. Section 9.9.2 issue 2 T1E1.4
enum MAX_INTER_DEPTH_OPTION {INTER_DEPTH_64,INTER_DEPTH_128,INTER_DEPTH_256,INTER_DEPTH_512};
typedef struct
{
    UINT16                     : 2;  // Reserved (set to 0)
    UINT16  max_inter_depth    : 2;  // Maximum interleave depth
    UINT16  bits_per_sym       :12;  // Max # bit/DMT symbol


    UINT16  loop_attenuation   : 7;  // estimated average loop attenuation
    UINT16  coding_gain        : 4;  // coding gain in 0.5 db increment
    UINT16  performance_margin : 5;  // performance margin with selected rate option

    UINT16  payload_lo         : 8;  // num of Reed Solomon payload bytes (8 lsb), K
    UINT16  num_data_rate_chnl : 8;  // num of tones carrying payload

    UINT16                     : 8;  // Reserved (set to 0)
    UINT16  overhead           : 7;  // num of RS overhead bytes, (R)
    UINT16  payload_hi         : 1;  // num of RS payload bytes (1 msb), K

    UINT16                     :16;  // Reserved (set to 0)
}R_MSG_RA_TYPE;


// message format for C_MSGS2 and R_MSGS2 (format refer to adsl standard)
// Ref: Section 12.8.3 T1E1

typedef struct
{
    UINT16                     : 7;   // Reserved (set to 0)
    UINT16  bits_per_sym       : 9;   // Total number of bits supported per sym

    UINT16  loop_attenuation   : 6;   // estimated average loop attenuation
    UINT16                     : 4;   // Reserved (set to 0)
    UINT16  erase_all_profile  : 1;   // erase all stored profiles
    UINT16  performance_margin : 5;   // performance margin with selected rate option
}C_MSGS2_TYPE;

// Ref: Section 12.9.2 T1E1

typedef struct
{
    UINT16                     : 4;   // Reserved (set to 0)
    UINT16  bits_per_sym       :12;   // Total number of bits supported per sym

    UINT16  loop_attenuation   : 7;   // Estimated average loop attenuation
    UINT16                     : 3;   // Reserved (set to 0)
    UINT16  erase_all_profile  : 1;   // erase all stored profiles
    UINT16  performance_margin : 5;   // Performance mar with selected rate
}R_MSGS2_TYPE;


// message format for C_RATES2 and R_RATES2 (format refer to adsl standard)
// This format combines the downstream rate info contained in R_RATES2 with
// the option number of the highest upstream data rate that can be
// supported based on the measure SNR of the upstream channel.  It thus
// transmit the final decision on the rates that will be used in both
// directions
// Ref: Section 12.8.5 T1E1

typedef struct
{
    UINT16 Downstream : 4;
    UINT16 Upstream   : 4;
    UINT16            : 8;
}C_RATES2_TYPE;



typedef struct
{
    UINT16 DownstreamMirror : 4;  // this mirror field contains
                                  // the same info as the other field for R_RATES2
                                  // (although might not be the same in R_RATES2_RA)
    UINT16 Downstream       : 4;
    UINT16                  : 8;
}R_RATES2_TYPE;

typedef struct
{
    UINT16                        : 2;  // Reserved (set to 0)
    UINT16  fast_retrain_truncate : 1;  // trucate fast retrain and goes to full init
    UINT16  down_max_tx_psd       : 5;  // Fast Retrain Politeness Power Cutback PSD level, 2dB per step
    UINT16                        : 8;
}C_MSG_FR1_TYPE;

typedef struct
{
    UINT16                        : 2;  // Reserved (set to 0)
    UINT16  fast_retrain_truncate : 1;  // trucate fast retrain and goes to full init
    UINT16  down_max_tx_psd       : 5;  // Relative Downstream Fast Retrain Power Cutback PSD level, 2dB per step
    UINT16                        : 3;  // Reserved (set to 0)
    UINT16  up_max_tx_psd         : 5;  // Absolute Upstream Fast Retrain Power Cutback PSD level, 2dB per step
}R_MSG_FR1_TYPE;


typedef struct
{
    UINT16                        : 3;  // Reserved (set to 0)
    UINT16  unknown_profile       : 1;  // (0) known profile, (1) unknown profile
    UINT16  profile_index         : 4;  // profile index (0 to 15)
    UINT16                        : 8;  // Reserved (set to 0)
}C_MSG_FR2_TYPE;

typedef struct
{
    UINT16                        : 3;  // Reserved (set to 0)
    UINT16  unknown_profile       : 1;  // (0) known profile, (1) unknown profile
    UINT16  profile_index         : 4;  // profile index (0 to 15)
    UINT16                        : 8;  // Reserved (set to 0)
}R_MSG_FR2_TYPE;


// message format for C_B&G and R_B&G (format refer to adsl standard)
// Ref: Section 12.8.5 T1E1
// NOTE: 'gain' is represented as 12-bit fix point binary with the binary
//          point assumed just to the right of the third msb.


typedef struct
{
    UINT16 gain :12;     // Scale factor, relative to the gain that was used
                        // for a carrier during the transmission of R_MEDLEY
    UINT16 bits : 4;     // # bits to be coded by ATU-R trasmitter onto the
                        // a upstream carrier.
}BITS_GAIN_TYPE;


// Data format for Tx and Rx bit tone table (format refer to FALCON user
// guide on transfer format to and from the MIF scratch pad RAM):
//  The QEDFER Transmit Bit/Tone Table is a 256*12-bit memory
//  Ref:    Section 12.9.4 QEDFER transmit Bit/Tone Table, FALCON user's guide
//          Section 6.6.4, Constellation Encoder, T1E1
typedef struct
{
    UINT16 bits       : 4;   // 'b' bits are to be assigned to a QAM sub-chan
                             //  ( 0 to 15 bits )
                             //  The hardware BPQE block shall read this value
                             //  and use it to extract number of bits out of
                             //  input data source, then perform QAM constellation
                             //  encoding on the data bits.
    UINT16 tone       : 8;   //  QAM sub-chan. addr
    UINT16 monitor    : 1;   //  monitor subchnl flag
    UINT16 tag        : 1;   //  0: FFTA use FADJUST2 on sync symbol
                             //  1: FFTA use FADJUST1 on sync symbol
    UINT16            : 2;   //  Unused

}TX_BIT_TONE_TYPE;


/***************************************************************************/
//  The QEDFER Receive Bit/Tone Table is a 256*18-bit memory.  Note that
//  the actual number of bits allocated for bit/tone table is still 12
//  the other 6 bits (in subsequence address) hold FEQ coefficient update
//  info and DGC gain info as defined in TONE_INFO_TYPE data structure
//  Ref:    Section 12.9.5 QEDFER receive Bit/Tone Table, FALCON user's guide
//          Section 3.2.8.1 Receive bit/tone table update, FALCON user's
//          guide

typedef struct
{
    UINT16            : 2;   // unused
    UINT16 chnl_type  : 2;   // chnl type can be UNUSED_CHNL, MONITOR_CHNL, or DATA_BIT_CHNL
                             // lsb: monitor subchnl flag
                             // msb: for tone order sorting purpose (not used by FALCON)
    UINT16 bits       : 4;   // Bit info
    UINT16 tone       : 8;   // Tone info
}RX_BIT_TONE_TYPE;          // Addr 0x306

// Data format for Rx tone info table (format refer to FALCON user
// guide on transfer format to and from the MIF scratch pad RAM)

typedef struct
{
    UINT16                  : 4;    // unused
    UINT16 PRBS_Match       : 1;    // this field determines if the subchannel
                                    // is contributing to the PRBS match counter
    UINT16 FEQ_Adaptation   : 1;    // FEQ adaptation for that subchannel
                                    // (LMS info) or subchnl freeze control
                                    // 1 : Allow to update
                                    // 0 : Freeze
    UINT16 DGC              : 4;    // dynamic gain control (DGC)
    UINT16                  : 6;    // Unused
}TONE_INFO_TYPE;                    // Addr, 0x307

/****************************************************************************
                       CRC FUNCTIONALITY DEFINITION
****************************************************************************/

//   12th, 5th and 0th coefficients set
//                                   CRC nth tap value
//                                                  1 1 1 1 1 1
//                              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
#define CRC_CHECK_POLYNOMIAL  {{1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0}}

typedef union
{
    struct
    {
        UINT16 tap_0  :1;
        UINT16 tap_1  :1;
        UINT16 tap_2  :1;
        UINT16 tap_3  :1;
        UINT16 tap_4  :1;
        UINT16 tap_5  :1;
        UINT16 tap_6  :1;
        UINT16 tap_7  :1;
        UINT16 tap_8  :1;
        UINT16 tap_9  :1;
        UINT16 tap_10 :1;
        UINT16 tap_11 :1;
        UINT16 tap_12 :1;
        UINT16 tap_13 :1;
        UINT16 tap_14 :1;
        UINT16 tap_15 :1;
    }bvalue;
    UINT16 wvalue;
}CRC_CHECK_TYPE;

#endif


// C_RATES1 is structure of 4 options, each option consists of 3 fields:
//          BF (10 bytes for Fast buffer data rates)
//          BI (10 bytes for Interleaved buffer data rates)
//          RRSI ( 5 bytes for  # parity bytes/symbol in fast buf. downstream
//                              # parity bytes/symbol in Interleave buf downstream
//                              # symbols per codeword, downstream
//                              Interleave depth in codewords for the inetrvleave buf, downstream
//                              Frame size (in bytes) of the bearer service
//                                  transported in LS2 channel, downstream
//                  5 bytes for same parameters in upstream)
// Usage: The ATU-C sends this 4-option downstream rate C_RATES1 to ATU-R (during
//          channel analysys cycle", the ATU-R shall determine which one has
//          the highest bit capacity based on the measured NSR and sent back
//          the option number to the ATU-C in R_RATES2.
// NOTE: Reference 12.7.4 T1E1:
//          For the present issue of the standard, ATU-C has control over all
//          the data rates, so R_RATES1 is copied from the appropriate fields
//          in C_RATES1 ( shown in BG_MSGDEC1routine()->ASIC_GenerateR_RATES1()
//          in the rtbgstat.c )
//          although we are not using fast path for payload
//          it is still advantageous to put check bytes in
//          in the fast path for the eoc and indicator bits.
//          S for fast buffer is set to be larger than 1 so we waste
//          fewer than 2 overhead check bytes for fast path (see S_FAST in asicutil.c)

// message buffers for transmitting and receiving messages


enum RATES1OPTIONS{RATES1_OPTION1=0,
                   RATES1_OPTION2,
                   RATES1_OPTION3,
                   RATES1_OPTION4,
                   NUMRATES1OPTION};

enum RATES2OPTIONS{RATES2_FAILOPTION = 0x00,
                   RATES2_OPTION1    = 0x01,
                   RATES2_OPTION2    = 0x02,
                   RATES2_OPTION3    = 0x04,
                   RATES2_OPTION4    = 0x08};

// the organization of the following bit-field data structures is governed by
// 1. the way how the table is burst transfer to and from the MIF scratch pad RAM
// Ref. Section 12.6.2 T1E1

// message format for C_RATES1 and R_RATES1 (format refer to adsl standard)

// BEWARE !!! Following structure type definitions have to be defined such
// that no byte "holes" are inserted within the structure, since these data
// will be transferred in the data line.

#define RATES1_PREFIX 0x55555555L

typedef struct
{
    /************************  Option 1 *****************************/
    UINT8  option1_LS0_F;       // # bytes in LS0 fast buffer (Upstream)
    UINT8  option1_LS1_F;       // # bytes in LS1 fast buffer (Upstream)
    UINT8  option1_LS2_F;       // # bytes in LS2 fast buffer (Upstream)
    UINT8  option1_LS0_I;       // # bytes in LS0 Interleave buffer (Upstream)
    UINT8  option1_LS1_I;       // # bytes in LS1 Interleave buffer (Upstream)
    UINT8  option1_LS2_I;       // # bytes in LS2 Interleave buffer (Upstream)
    UINT8  option1_RF;          // number of parity bytes per symbol in the fast buffer
    UINT8  option1_RI;          // number of parity bytes per symbol in the interleave buffer
    UINT8  option1_S;           // number of symbols per codeword
    UINT8  option1_I;           // interleave depth in codewords for the interleave buffer
    UINT8  option1_FS;          // the frame size (in bytes) of the bearer service

    /************************  option 2 *****************************/
    UINT8  option2_LS0_F;
    UINT8  option2_LS1_F;
    UINT8  option2_LS2_F;
    UINT8  option2_LS0_I;
    UINT8  option2_LS1_I;
    UINT8  option2_LS2_I;
    UINT8  option2_RF;
    UINT8  option2_RI;
    UINT8  option2_S;
    UINT8  option2_I;
    UINT8  option2_FS;

    /************************  option 3 *****************************/
    UINT8  option3_LS0_F;
    UINT8  option3_LS1_F;
    UINT8  option3_LS2_F;
    UINT8  option3_LS0_I;
    UINT8  option3_LS1_I;
    UINT8  option3_LS2_I;
    UINT8  option3_RF;
    UINT8  option3_RI;
    UINT8  option3_S;
    UINT8  option3_I;
    UINT8  option3_FS;

    /************************  option 4 *****************************/
    UINT8  option4_LS0_F;
    UINT8  option4_LS1_F;
    UINT8  option4_LS2_F;
    UINT8  option4_LS0_I;
    UINT8  option4_LS1_I;
    UINT8  option4_LS2_I;
    UINT8  option4_RF;
    UINT8  option4_RI;
    UINT8  option4_S;
    UINT8  option4_I;
    UINT8  option4_FS;

}R_RATES_OPTION_TYPE;

typedef struct
{
    UINT32                  Prefix;
    R_RATES_OPTION_TYPE     option;

}R_RATES1_TYPE;



typedef struct
{
    struct
    {
        UINT8  AS0;          // # bytes in AS0   (Fast/Interleave buf)
        UINT8  AS1;          // # bytes in AS1   (Fast/Interleave buf)
        UINT8  AS2;          // # bytes in AS2   (Fast/Interleave buf)
        UINT8  AS3;          // # bytes in AS3   (Fast/Interleave buf)
        UINT8  LS0_DWN;      // # bytes in LS0   (Downstream)
        UINT8  LS1_DWN;      // # bytes in LS1   (Downstream)
        UINT8  LS2_DWN;      // # bytes in LS2   (Downstream)
        UINT8  LS0_UP;       // # bytes in LS0   (Upstream)
        UINT8  LS1_UP;       // # bytes in LS1   (Upstream)
        UINT8  LS2_UP;       // # bytes in LS2   (Upstream)
    }fast_buffer,interleave_buffer;
    UINT8  RF_DWN;      // number of parity bytes per symbol in the fast buffer
    UINT8  RI_DWN;      // number of parity bytes per symbol in the interleave buffer
    UINT8  S_DWN;       // number of symbols per codeword
    UINT8  I_DWN;       // interleave depth in codewords for the interleave buffer
    UINT8  FS_DWN;      // the frame size (in bytes) of the bearer service
                        // transported in LS2 channel
    UINT8  RF_UP;       // number of parity bytes per symbol in the fast buffer
    UINT8  RI_UP;       // number of parity bytes per symbol in the interleave buffer
    UINT8  S_UP;        // number of symbols per codeword
    UINT8  I_UP;        // interleave depth in codewords for the interleave buffer
    UINT8  FS_UP;       // the frame size (in bytes) of the bearer service
                        // transported in LS2 channel
}C_RATES_OPTION_TYPE;

// The following construct define a complete C_RATES1 signal.  The purpose
// of C_RATES1 is to trasnmit four options for data rates and formats to
// ATU-R

typedef struct
{
    UINT32 Prefix;                                  // 4-byte prefix
    C_RATES_OPTION_TYPE option[NUMRATES1OPTION];
}C_RATES1_TYPE;

typedef struct
{
    C_RATES_OPTION_TYPE option[NUMRATES1OPTION];
}C_RATES_RA_TYPE;


typedef R_RATES2_TYPE    R_RATES_RA_TYPE;

// NOTE: The total bits used for the above two data structure are 18 bits,
//
/***************************************************************************/
// data format for the scrach pad RAM, a union is used so burst transfers
// can be achieved in different format.

typedef union
{
    UINT16 *uint16;
    UINT8  *uint8;
    UINT32 *uint32;
    SINT8  *sint8;
    SINT16 *sint16;
    SINT32 *sint32;
    SINT16CMPLX *sint16cmplx;
    SINT32CMPLX *sint32cmplx;
    BITS_GAIN_TYPE *BitsGain;
} SCRATCHPADTYPE;

// This struct. will be used in MISC_GetChnlInfo() and dpudmt.c for
// displaying bit allocation.

typedef struct
{
    char  status;       // upstream,downstream or not used in a subchannel
    UINT8  bits;         // actual number of bits(integer) used in a subchannel
    SINT16 bit_capacity; // bits capacity (include fractional bit) used in a subchannel
}CHNL_INFO_TYPE;

MAKE_RETURN_TYPE(CHNL_INFO_TYPE)

#define PBOOST_START             50       // power boost is only for ATU-C
#define PBOOST_END              255


// this is a type definition of adjusted square root of the NSR used for bit allocation
typedef struct
{
    UINT16   data;
    UINT16   scale;
}ADJ_SQRT_NSR_TYPE;


// data format different message we are going to transfer across the link
// during training and startup.

typedef union
{
    UINT8  uint8[512];  // Note that this buffer type definition is used as message
                        // buffer during startup and eoc buffer during normal operation
    struct
    {
        C_RATES1_TYPE    C_RATES1;
        UINT16           C_CRC1;
        C_MSGS1_TYPE     C_MSGS1;
        UINT16           C_CRC2;
    }msg1;
    struct
    {
        C_MSGS2_TYPE     C_MSGS2;
        UINT16           C_CRC3;
        C_RATES2_TYPE    C_RATES2;
        UINT16           C_CRC4;
        BITS_GAIN_TYPE   C_BandG[NUM_UPSTREAM_CHNL-1];
        UINT16           C_CRC5;
    }msg2;
    struct
    {
        C_RATES_RA_TYPE  C_RATES_RA;
        UINT16           C_CRC_RA1;
        C_MSG_RA_TYPE    C_MSG_RA;
        UINT16           C_CRC_RA2;
    }msg_ra;
    struct
    {
        C_MSG_FR1_TYPE   C_MSG_FR1;
        UINT16           C_CRC_FR1;
    }msg_fr1;
    struct
    {
        C_MSG_FR2_TYPE   C_MSG_FR2;
        UINT16           C_CRC_FR2;
    }msg_fr2;
}ADSL_DWNSTREAM_MSG_BUF_TYPE;

typedef union
{
    UINT8  uint8[2048];  // Note that this buffer type definition is used as message
                         // buffer during startup and eoc buffer during normal operation
                         // Upstream needs larger buffer to allow remote logon.
    struct
    {
        R_RATES1_TYPE    R_RATES1;
        UINT16           R_CRC1;
        R_MSGS1_TYPE     R_MSGS1;
        UINT16           R_CRC2;
    }msg1;
    struct
    {
        R_MSGS2_TYPE     R_MSGS2;
        UINT16           R_CRC3;
        R_RATES2_TYPE    R_RATES2;
        UINT16           R_CRC4;
    }msg2;
    struct
    {
        BITS_GAIN_TYPE   R_BandG[NUM_DOWNSTREAM_CHNL-1];
        UINT16           R_CRC5;
    }msg3;
    struct
    {
        R_MSG_RA_TYPE    R_MSG_RA;
        UINT16           R_CRC_RA1;
        R_RATES_RA_TYPE  R_RATES_RA;
        UINT16           R_CRC_RA2;
    }msg_ra;
    struct
    {
        R_MSG_FR1_TYPE   R_MSG_FR1;
        UINT16           R_CRC_FR1;
    }msg_fr1;
    struct
    {
        R_MSG_FR2_TYPE   R_MSG_FR2;
        UINT16           R_CRC_FR2;
    }msg_fr2;
}ADSL_UPSTREAM_MSG_BUF_TYPE;

/****************************************************************************
            AdslHeadEndEnvironment Parameter

For Building aNd Apartment environments,(BNA) where the CO does not have a line
driver, which somehow translates into tranmitting at a lower level, even on
0 length lines, the registry value AdslHeadEndEnvironment should be set to 1.
This flags the RT controller code to select an optimal transmit filter, along
with a lower transmit value in order to reduce Tx to Rx interference.
****************************************************************************/
// #define BNA                      1

/****************************************************************************
            TRAINING PARAMETERS (common for both CO and RT)
    There will be an array of this data structure type, each array element
    represents one fteq training stage.

****************************************************************************/

typedef struct
{
    UINT16 iteration;       // # of iteration b4 switching to next training stage
    UINT8  teq_win_size;
    UINT8  fir_win_size;
    UINT16  beta;
}XCVR_FTEQ_TRAIN_CONFIG_TYPE;

typedef SINT16 XCVR_TX_FILTER_TYPE;
#define UNITY_TX_FILTER_GAIN         ((UINT16) FLT2INT16(1.0))
#define USE_EXISTING_TX_FILTER_GAIN  (0)



/****************************************************************************
                             CO DEFINITION
****************************************************************************/
#ifdef CO

#define SNR_REF                FLT2EXT_INT16(21.5)


#define TX_OVER_SAMPLING_RATIO    1
#ifdef ECHO_TRAINING
#define RX_OVER_SAMPLING_RATIO    1
#else
#define RX_OVER_SAMPLING_RATIO    2
#endif

#define NUM_TEQ_COEF              64
#define NUM_FIR_COEF              62

#define DEFAULT_TEQ_CTR_TAP        0.25  // default gain tap for TEQ coef.

// This is the master clock per symbol w/o cyclic prefix
// the one with cyclic prefix is 8704 = 35.328Mhz/4.0588Khz
// Where the master clock rate for Falcon is MCLK = 35.328Mhz
//  and Nominal symbol rate is 4.0588Khz(246.38us, with cyclic prefix)
//      Nominal symbol rate is 4.3125Khz(no cyclic prefix)
// Ref: Projected Maximum Clock Rates, Falcon user's guide

#define MCLK_SYMBOL                 8192    // = 35.328Mhz/4.3125Khz
#define M_VALUE                     64      // # of rx samples (size of FFT)
#define N_VALUE                     512     // # of tx samples (size of IFFT)

#define TCP_VALUE             (TX_OVER_SAMPLING_RATIO * 32)  // # of tx cyclic prefix samples
#define RCP_VALUE             (RX_OVER_SAMPLING_RATIO * 4)   // # of rx cyclic prefix samples
#define NUM_SAMPLING_PHASE              8

#define DEFAULT_VALID_TX_CHNL_START     39
#define DEFAULT_VALID_TX_CHNL_END       255

#define DEFAULT_VALID_RX_CHNL_START     6
#define DEFAULT_VALID_RX_CHNL_END       31


#define TX_PILOT_TONE                   (ANNEX_MODE == ANNEX_B ? 96 : 64)      // Tx PILOT_TONE for downstream
#define RX_PILOT_TONE                   16      // Rx PILOT_TONE for upstream

#define TPRBS_GEN_POLYNOMIAL          TAP4_9GEN  // tap 4 and 9
#define RPRBS_GEN_POLYNOMIAL          TAP5_6GEN  // tap 5 and 6
#define TX_FILTER_PASSBAND_GAIN          1       // passband gain for Tx filter
#define RX_FILTER_PASSBAND_GAIN          1       // passband gain for Rx filter
#define DAC_TX_FILTER_SIZ               32
#define RX_PREFILTER_SIZ                32


/****************************************************************************
                    TONE DETECTION PARAMETERS
****************************************************************************/

// The following is the list of tones that ATU_C is responsible to detect
#define TONE_DETECTION_LIST               \
{                                         \
    R_ACT_REQ_TONE,        /* tone 8  */  \
    R_ACK1_TONE,           /* tone 10 */  \
    R_ACK2_TONE,           /* tone 12 */  \
    R_RECOV_TONE,          /* tone 20 for fast retrain*/  \
    R_ACK3_TONE            /* this tone always goes with R_ACK1 or R_ACK2 for Fast Retrain */  \
}

#define TONE_DETECTION_LIST_SIZ    4    // does not include the last tone which is for second tone detection
#define SECOND_TONE_INDEX          4    // always the last tone in TONE_DETECTION_LIST



#define GHS_TONE_RX_BAND                              \
{                                                  \
    9, /* adsl tone 9 */  \
    17,/* adsl tone 17 */ \
    25,/* adsl tone 25 */ \
    20,/* adsl tone 20 */ \
}

#define GHS_TONE_RX_TRANSFER_SIZE 4
#define GHS_TONE_RX_BAND_SIZE 3
#define GHS_RX_RECOV_TONE_INDEX 3
#define GHS_RX_PILOT_TONE_INDEX 1

#define GHS_TONE_RX_OUT_BAND     \
{                                \
    5,/* adsl tone 3 */ \
    6,/* adsl tone 6 */ \
    7,/*  adsl tone 8 */ \
    10,/* adsl tone 11 */ \
    11,/* adsl tone 12 */ \
    13,/* adsl tone 13 */ \
    15,/* adsl tone 14 */ \
    16,/* adsl tone 15 */ \
    19,/* adsl tone 19 */ \
    21,/* adsl tone 21 */ \
    22,/* adsl tone 22 */ \
    23,/* adsl tone 23 */ \
    27,/* adsl tone 27 */ \
    28,/* adsl tone 28 */ \
    29,/* adsl tone 29 */ \
    30,/* adsl tone 30 */ \
}

#define GHS_TONE_RX_OUT_BAND_SIZE 16

#define GHS_TONE_TX_BAND     \
{       \
    40, \
    56, \
    64, \
}

#define GHS_TONE_TX_BAND_SIZE 3


// Host command issued by the user

typedef enum
{
    C_TONE_CMD,
    C_IDLE_IGNORE_CMD,
    C_ACTIVATE_CMD,
    C_SELFTEST_CMD,
    C_IDLE_MONITOR_CMD,
    C_RECONFIG2_CMD,
    NUM_OF_HOST_CMD
}HOST_CMD_TYPE;


// Transmit State of CO

enum TX_STATE { C_TESTMODE = 0,
                C_SELFTEST,
                C_FAIL,
                C_TONE,

                C_TONE_2,
                C_ACT,
                C_ACT_2,
                C_QUIET1,
                C_IDLE,
                C_QUIET2,
                C_QUIET2_2,
                C_REVEILLE,
                C_REVEILLE_2,
                C_QUIET3_or_PILOT1,
                C_REVERB1,
                C_QUIET4_or_PILOT2,
                C_ECT,
                C_REVERB2,
                C_QUIET5_or_PILOT3,
                C_REVERB3,
                C_SEGUE1,
                C_RATES1_to_CRC2,
                C_MEDLEY,
                C_REVERB4,
                C_SEGUE2,
                C_RATES_to_CRC_RA2,
                C_REVERB_RA,
                C_SEGUE_RA,
                C_MSGS2_to_CRC5,
                C_REVERB5,
                C_REVERB5_2,
                C_SEGUE3,
                C_SEGUE3_2,
                C_SHOWTIME,
                C_REVERB1_WS,
                C_SEGUE1_WS,
// fast retrain states
                C_RECOV,
                C_REVERB_FR1,
                C_PILOT_FR1_1,
                C_PILOT_FR1_2,
                C_REVERB_FR2,
                C_SEGUE_FR1,
                C_MSGFR1,
                C_REVERB_FR3,
                C_PILOT_FR2,
                C_REVERB_FR4,
                C_ECT_FR,
                C_REVERB_FR5,
                C_PILOT_FR3,
                C_REVERB_FR6,
                C_SEGUE_FR2,
                C_MEDELY_FR,
                C_REVERB_FR7,
                C_SEGUE_FR3,
                C_MSGFR2,
                C_REVERB_FR8_1,
                C_REVERB_FR8_2,
                C_REVERB_FR8_3,
                C_REVERB_FR8_4,
                SET_FRSHOWTIME,

                GHS_TX_C_TONES,
                GHS_TX_GALF,
                GHS_TX_DATA,
                GHS_TX_IDLE,
                NUMTXSTATES};

// Receive State of CO

enum RX_STATE { R_TESTMODE = 0,
                R_FAIL,
                R_4QAMDEC,
                R_4QAMDEC_2,
                R_SEGUE1DEC,
                R_4QAMDEC_3,
                R_SEGUE2DEC,
                R_RATES1_to_CRC2DEC,
                R_MEDLEYDEC,
                R_SEGUE3DEC,
                R_MSG_to_CRC_RA2DEC,
                R_REVERB_RADEC,
                R_SEGUE_RADEC,
                R_MSGS2_to_CRC4DEC,
                R_4QAMDEC_4,
                R_SEGUE4DEC,
                R_BandG_to_CRC5DEC,
                R_4QAMDEC_5,
                R_SEGUE5DEC,
                R_NORMDEC,
                R_REVERB1DEC_WS,
                R_SEGUE1DEC_WS,
// fast retrain states
                R_REVERB_FR1,
                R_SEGUE_FR1,
                R_LINE_PROBE_FR,
                R_MSGFR1,
                R_QUIET_FR1,
                R_REVERB_FR3,
                R_QUIET_FR2_1,
                R_QUIET_FR2_2,
                R_REVERB_FR5,
                R_SEGUE_FR3,
                R_MEDELY_FR_1,
                R_MEDELY_FR_2,
                R_MEDELY_FR_3,
                R_MSGFR2,
                R_REVERB_FR7,
                R_SEGUE_FR4,
                SET_FRNORMAL,
                GHS_WAIT_START,
                GHS_START_EQ,
                GHS_SET_THRESHOLD,
                GHS_FILL_MSGBANK,
                GHS_RX_DATA,
                GHS_RX_IDLE,
                NUMRXSTATES};

// Process (Background) State of CO

enum BG_STATE { BG_TESTMODE = 0,
                BG_SELFTEST,
                BG_FAIL,
                BG_ACT_REQ_TONEDETECT,
                BG_ACK_TONEDETECT,
                BG_REVERBDETECT,
                BG_RCVGAINADJ,
                BG_FTEQ_TRAIN,
#ifdef ECHO_TRAINING
                BG_EC_TRAIN,
#endif
                BG_FRAME_BOUNDARY_SELECTION,
                BG_MSGDEC1,
                BG_BITALLOC,
                BG_MSG_RADEC,
                BG_MSGDEC2,
                BG_MSGDEC3,
                BG_NORMAL,
                BG_WARMSTART1,
                BG_WARMSTART2,
                BG_WARMSTART3,
// fast retrain states
                INIT_BG_FRSTATE,
                BG_R_RECOV_DETECT,
                BG_R_RECOV_MEASURE,
                BG_R_REVERB_FR1_DETECT,
                BG_PARSE_R_MSG_FR1,
                BG_SET_FRRXTXGAIN,
                ALLIGN_FRFRAME_BOUNDARY,
                BG_C_MEDELY_FR,
                BG_PARSE_R_MSG_FR2,
                UNKNOWN_PROFILE_ABORT,

                GHS_SELFTEST,
                GHS_HUNT_RTONES_REQ,
                GHS_BG_SCHEDULE,
                GHS_STOPPED,
                NUMBGSTATES};


#else  /* ATU_R */

/****************************************************************************
                             RT DEFINITION
****************************************************************************/

#define SNR_REF                FLT2EXT_INT16(27.7)


#define TCP_RCP_DELAY_PADDING     6 // Used to extend R_REVERB2 (See RTTXSTAT.C for comments)

#ifdef ECHO_TRAINING
#define TX_OVER_SAMPLING_RATIO    1
#else
#define TX_OVER_SAMPLING_RATIO    (ANNEX_MODE == ANNEX_A ? 4 : 2)
#endif
#define RX_OVER_SAMPLING_RATIO    1

#define NUM_TEQ_COEF             32
#define NUM_FIR_COEF             30

#define DEFAULT_TEQ_CTR_TAP          0.125  // default gain tap for TEQ coef.

// This is the master clock per symbol w/o cyclic prefix
//  the one with cyclic prefix is 8704 = 35.328Mhz/4.0588Khz where the master
//  clock rate for Falcon is MCLK = 35.328Mhz and Nominal symbol rate is
//  4.0588Khz(246.38us, with cyclic prefix)
//  Nominal symbol rate is 4.3125Khz(no cyclic prefix)
// Ref: Projected Maximum Clock Rates, Falcon user's guide

#define MCLK_SYMBOL                 8192    // = 35.328Mhz/4.3125Khz
#define M_VALUE                     512     // # of rx samples (size of FFT)
#define N_VALUE               ((ANNEX_MODE == ANNEX_B)? 128 : 64)      // # of tx samples (size of IFFT)

#define TCP_VALUE             (TX_OVER_SAMPLING_RATIO * 4)   // # of tx cyclic prefix samples
#define RCP_VALUE             (RX_OVER_SAMPLING_RATIO * 32)  // # of rx cyclic prefix samples
#define NUM_SAMPLING_PHASE          16

// Note that Annex B can support upstream chnl < 32
#define DEFAULT_VALID_TX_CHNL_START     (ANNEX_MODE == ANNEX_B && ! ANNEX_B_UP_TONE_UNDER_32 ? 32: 5)
#define DEFAULT_VALID_TX_CHNL_END       (ANNEX_MODE == ANNEX_B ? 63 : 31)

#define DEFAULT_VALID_RX_CHNL_START     (ANNEX_MODE == ANNEX_B ? 65 : 39)
#define DEFAULT_VALID_RX_CHNL_END       255

#define TX_PILOT_TONE               16      // Tx PILOT_TONE for upstream
#define RX_PILOT_TONE               (ANNEX_MODE == ANNEX_B ? 96 : 64)      // Rx PILOT_TONE for downstream

#define TPRBS_GEN_POLYNOMIAL          TAP5_6GEN  // tap 5 and 6
#define RPRBS_GEN_POLYNOMIAL          TAP4_9GEN  // tap 4 and 9
#define TX_FILTER_PASSBAND_GAIN          1       // passband gain for Tx filter
#define RX_FILTER_PASSBAND_GAIN        0.5       // passband gain for Rx filter
#define DAC_TX_FILTER_SIZ                64
#define RX_PREFILTER_SIZ                 16


/****************************************************************************
                    TONE DETECTION PARAMETERS
****************************************************************************/
// The following is the list of tones that ATU_R is responsible to detect

#define TONE_DETECTION_LIST               \
{                                         \
    C_TONE_TONE,           /* tone 72 */  \
    C_ACT1_TONE,           /* tone 48 */  \
    C_ACT2_TONE,           /* tone 44 */  \
    C_ACT3_TONE,           /* tone 52 */  \
    C_ACT4_TONE,           /* tone 60 */  \
    C_REVEILLE_TONE,       /* tone 56 */  \
    C_RECOV_TONE,         /* tone 68 for fast retrain */ \
    RX_PILOT_TONE         /* Pilot tone can either come by itself or goes with C_RECOV_TONE */ \
}

#define TONE_DETECTION_LIST_SIZ    7    // does not include the last tone which is for second tone detection
#define SECOND_TONE_INDEX          7    // always the last tone in TONE_DETECTION_LIST

#if (ANNEX_MODE == ANNEX_A) // ref 6.1.1 in G994.1

    #define GHS_TONE_RX_BAND                              \
    {                                                  \
        40, /* adsl tone 40 */  \
        56, /* adsl tone 56 */ \
        RX_PILOT_TONE, /* adsl tone 64  and CPilot*/ \
        C_RECOV_TONE  /* adsl tone 64  and CRecov*/ \
    }

    #define GHS_TONE_RX_TRANSFER_SIZE 4
    #define GHS_TONE_RX_BAND_SIZE 3
    #define GHS_RX_PILOT_TONE_INDEX 2
    #define GHS_RX_RECOV_TONE_INDEX 3
     // make sure none of these are signalling tones!
    #define GHS_TONE_RX_OUT_BAND     \
    {                                \
        39, \
        41, \
        55, \
        57, \
        63, \
        65, \
        47, \
        60, \
    }

    #define GHS_TONE_RX_OUT_BAND_SIZE 8


    #define GHS_TONE_TX_BAND     \
    {       \
        9,  \
        17, \
        25, \
    }

    #define GHS_TONE_TX_BAND_SIZE 3

#else  // ANNEX_MODE  Annex B
    // ref 6.1.1 in G994.1
    #define GHS_TONE_RX_BAND                               \
    {                                                  \
        72, /* adsl tone 72 */  \
        88, /* adsl tone 88 */ \
        RX_PILOT_TONE, /* adsl tone 96  and CPilot*/ \
        C_RECOV_TONE  /* adsl tone 96  and CRecov*/ \
    }

    #define GHS_TONE_RX_TRANSFER_SIZE 4
    #define GHS_TONE_RX_BAND_SIZE 3
    #define GHS_RX_PILOT_TONE_INDEX 2
    #define GHS_RX_RECOV_TONE_INDEX 3
     // make sure none of these are signalling tones!
    #define GHS_TONE_RX_OUT_BAND     \
    {                                \
        79, \
        81, \
        83, \
        85, \
        87, \
        89, \
        91, \
        93, \
    }

    #define GHS_TONE_RX_OUT_BAND_SIZE 8

    #define GHS_TONE_TX_BAND     \
    {       \
        37,  \
        45, \
        53, \
    }

    #define GHS_TONE_TX_BAND_SIZE 3

#endif // ANNEX_MODE


// no host command available in ATU-R

typedef enum
{
    NUM_OF_HOST_CMD =0
}HOST_CMD_TYPE;


// Transmit State of RT
enum TX_STATE { R_TESTMODE,
                R_SELFTEST,
                R_FAIL,
                R_SILENT0,
                R_QUIET1,
                R_ACT_REQ,
                R_ACK,
                R_ACK_2,
                R_QUIET2,
                R_QUIET2_2,
                R_REVERB1,
                R_QUIET3_or_PILOT1,
                R_ECT,
                R_REVERB2,
                R_SEGUE1,
                R_REVERB3,
                R_REVERB3_2,
                R_SEGUE2,
                R_RATES1_to_CRC2,
                R_MEDLEY,
                R_REVERB4,
                R_SEGUE3,
                R_MSG_to_CRC_RA2,
                R_REVERB_RA,
                R_REVERB_RA_2,
                R_SEGUE_RA,
                R_MSGS2_to_CRC4,
                R_REVERB5,
                R_REVERB5_2,
                R_SEGUE4,
                R_BandG_to_CRC5,
                R_REVERB6,
                R_SEGUE5,
                R_SEGUE5_2,
                R_SHOWTIME,
                R_REVERB1_WS,
                R_SEGUE1_WS,
// fast retrain states
                R_RECOV,
                R_REVERB_FR1,
                R_SEGUE_FR1,
                R_LINE_PROBE_FR,
                R_REVERB_FR2,
                R_SEGUE_FR2,
                R_MSGFR1,
                R_QUIET_FR1,
                R_REVERB_FR3,
                R_QUIET_FR2,
                R_REVERB_FR4,
                R_ECT_FR,
                R_REVERB_FR5,
                R_SEGUE_FR3,
                R_MEDELY_FR1,
                R_REVERB_FR6,
                R_SEGUE_FR4,
                R_MSGFR2,
                R_REVERB_FR7_1,
                R_REVERB_FR7_2,
                R_REVERB_FR7_3,
                SET_FRRSHOWTIME,

                GHS_SILENT0A,
                GHS_SILENT0B,
                GHS_INV_RTONES_REQ,
                GHS_TX_RTONE,
                GHS_INIT_TXER,
                GHS_TX_DATA,
                GHS_MANUAL_7E,
                GHS_TX_IDLE,
                R_QUIET_LITE,
                NUMTXSTATES};

// Receive State of RT

enum RX_STATE { C_TESTMODE = 0,
                C_FAIL,
                C_4QAMDEC,
                C_4QAMDEC_2,
                C_4QAMDEC_3,
                C_4QAMDEC_4,
                C_SEGUE1DEC,
                C_RATES1_to_CRC2DEC,
                C_MEDLEYDEC,
                C_4QAMDEC_5,
                C_SEGUE2DEC,
                C_RATES_to_CRC_RA2DEC,
                C_RATES_to_CRC_RA2DEC_2,
                C_REVERB_RADEC,
                C_SEGUE_RADEC,
                C_MSGS2_to_CRC5DEC,
                C_4QAMDEC_6,
                C_SEGUE3DEC,
                C_NORMDEC1ST,
                C_NORMDEC,
                C_REVERB1DEC_WS,
                C_SEGUE1DEC_WS,
// fast retrain states
                C_PILOT_FR1,
                C_REVERB_FR2,
                C_SEGUE_FR1,
                C_MSGFR1,
                C_REVERB_FR3,
                C_PILOT_FR2,
                C_REVERB_FR4,
                C_ECT_FR,
                C_REVERB_FR5,
                C_PILOT_FR3,
                C_REVERB_FR6,
                C_MEDELY_FR_1,
                C_MEDELY_FR_2,
                C_MEDELY_FR_3,
                C_REVERB_FR7,
                C_MSGFR2,
                C_REVERB_FR8,
                C_SEGUE_FR4DETECT,
                C_FRINIT_SHOWTIME,
                C_FRINIT_NORMAL,

                GHS_WAIT_START,
                GHS_START_EQ0,
                GHS_START_EQ,
                GHS_SET_THRESHOLD,
                GHS_FILL_MSGBANK,
                GHS_RX_DATA,
                GHS_RX_IDLE,
                NUMRXSTATES};

// Process (Background) State of RT

enum BG_STATE { BG_TESTMODE = 0,
                BG_SELFTEST,
                BG_FAIL,
                BG_ACT_TONEDETECT,
                BG_ACT_NOTONEDETECT,
                BG_REVEILLE_TONEDETECT,
                BG_QUIET_PILOTDETECT,
                BG_HYBRIDSELECT,
                BG_REVERBDETECT,
                BG_RCVGAINADJ,
                BG_PHASE_SELECTION,
                BG_FTEQ_TRAIN,
#ifdef ECHO_TRAINING
                BG_EC_TRAIN,
#endif
                BG_FRAME_BOUNDARY_SELECTION,
                BG_MSGDEC1,
                BG_BITALLOC,
                BG_MSG_RADEC,
                BG_MSGDEC2,
                BG_NORMAL,
                BG_WARMSTART1,
                BG_WARMSTART2,
                BG_WARMSTART3,
// fast retrain states
                INIT_BG_FRSTATE,
                BG_C_RECOV_MEASURE,
                BG_C_REVERB_FR1_DETECT,
                BG_RECOV_DETECT,
                BG_FR_SET_INITAL_RXGAIN,
                BG_FR_RECOVER_TIMING,
                BG_FR_FEQ_TRAIN,
                BG_R_LINE_PROBE_FR,
                BG_C_REVERB_FR2_DETECT,
                BG_PARSE_C_MSG_FR1,
                BG_C_REVERB_FR5_DETECT,
                BG_R_ECT_FR,
                BG_R_MEDELY_FR,
                BG_PARSE_MSGCRC_FR2DEC,

                GHS_SELFTEST,
                GHS_HUNT_NOISE_FLOOR,
                GHS_HUNT_C_TONES,
                GHS_CNVG_TIMELOOP,
                GHS_CHECK_EQ_CNVG,
                GHS_BG_SCHEDULE,
                GHS_STOPPED,
                NUMBGSTATES};

#endif                              // End of RT definition

/****************************************************************************
                       SYSTEM STATE STATES DEFINITION
    The following states shall be displayed in menu ADSL PERFORMANCE STATUS->
    SYSTEM STATE
****************************************************************************/
enum SYSTEM_MODE
{
    INIT,               // Initialization
    START_UP,
    DATA,               // Data mode (Active mode)
    XCVR_TEST,
    CSTART,
    WSTART,
    DEBUG,
    CO_FAST_RETRAIN,   // for local or remote initiated FR
    RT_FAST_RETRAIN,   // for local or remote initiated FR
    XCVR_PWR_DWN,
    NUM_SYS_STATE
};

/***************************************************************************/
// Define main xcvr main states.  Each xcvr main state may have several
//  sub-states.  Each syb-state name shall have prefix of 'sub' before its
//  corresponding main state name.

typedef struct
{
    UINT8   main_st;
    UINT8   sub_st;
} XCVR_MAIN_SUB_STATE_TYPE;

// Subsequent definitions for xcvr sub-states of each xcvr main state:

typedef enum
{
    SUB_INIT
}SUB_INIT_STATE;

typedef enum
{
    SUB_ACK,                // Startup mode: Activation & Acknowledge
    SUB_TRAIN,              // Startup mode: Transceiver training
    SUB_CHANA,              // Startup mode: Channel Analysys
    SUB_EXCH               // Startup mode: Exchange
}SUB_STARTUP_STATE;


typedef enum
{
    SUB_DATA              // normal state
}SUB_DATA_STATE;

typedef enum
{
    SUB_IDLE,               // Test mode: Idle
    SUB_TONE,               // Test mode: Transmit ton
    SUB_TXON,               // Test mode: Spectrum
    SUB_PRBS                // Test mode: Transmit PRBS
}SUB_XCVR_TEST_STATE;

/****************************************************************************
                       BIT CAPACITY INFO DEFINITION
****************************************************************************/

// this is a type definition to stored all bit capactiy info that is not already stored in FALCON
typedef struct
{
    SINT16   fraction_bit[NUM_RX_CHNL];  // SSSSSSSS XXXX.XXXX
    UINT8    valid_chnl[NUM_RX_CHNL];    // List of valid channel
                                        // (channel with bit capacity >0)
    GEN_UINT num_of_active_chnls;       // number of chnls that actually carry bits
    GEN_UINT num_of_valid_chnls;        // number of chnls that can carry bits
    SINT16   F;       // Summation of the fractional portion bit capacity of all subchans.
                      //          (i.e. fraction_bit[])
    SINT16   B;       // Summation of the integral portion bit capacity of all subchans
    UINT16   BR_used; // BR we picked from the RATES1 or RATE_RA options
    // The following parameters available only after startup. Attainable
    // data rate is available only to the ATU_C and only in rate
    // adaptive mode.
    UINT16   upstream_data_rate,
             downstream_data_rate,
             upstream_attainable_data_rate,
             downstream_attainable_data_rate;
}BIT_CAPACITY_INFO_TYPE;



/****************************************************************************
                       SYSTEM ERROR COUNT DEFINITION
****************************************************************************/
typedef struct
{
   UINT32 febe;
   UINT32 crc;
} ERR_CNT_TYPE;

#define   ERR_CNT_SIZE   sizeof(ERR_CNT_TYPE)


typedef struct
{
    UINT8               sys_state[2];               // Sys state for NEAR and FAR_END
    ERR_CNT_TYPE        err_cnt;                    //  = {0,0}
} XCVR_TYPE, *pXCVR_TYPE;


/****************************************************************************
            PROFILE PARAMETERS (common for both CO and RT)
    These are startup profile parameters for adsl standard Fast Retrain.

****************************************************************************/

typedef struct
{
    BITS_GAIN_TYPE   tx_BandG[NUM_TX_CHNL-1];
    // in ATU_C, Upstream chnl doesn't equal to num rx chnl in Annex B yet,
    // if we can change NUM_RX_CHNL to 64 in CO, we can change it back to NUM_RX_CHNL-1
    BITS_GAIN_TYPE   rx_BandG[XCVR_MODE == ATU_C ? NUM_UPSTREAM_CHNL-1: NUM_DOWNSTREAM_CHNL-1];
    C_RATES_OPTION_TYPE  rate_option;
    UINT16 bit_usage;
    SINT8  afe_rx_gain;
    SINT8  tx_gain;
    BOOLEAN valid_tx_profile;
#ifdef RT
    BOOLEAN hookstate;
#else
    UINT16 rt_abs_cutback;
#endif
    UINT16   upstream_data_rate,
             downstream_data_rate;

}XCVR_PROFILE_TYPE;

// this means 2 stored profiles is minimum
#define MIN_NUM_STORED_PROFILES 1

/****************************************************************************
            startup mode definitions
    variable to check which startup mode was last used

****************************************************************************/
typedef enum
{
    STARTUP_GHS,
    STARTUP_T1_413
} STARTUP_MODE_TYPE;


/****************************************************************************
                       XCVR DEPENDENT DATABASE DEFINITION
****************************************************************************/
typedef UINT8  TONE_MAP_TYPE;
typedef UINT16 GAIN_TBL_TYPE;
typedef GEN_UINT  VENDOR_ID_TYPE;


typedef struct
{
    C_MSGS2_TYPE        C_MSGS2;
    C_MSGS1_TYPE        C_MSGS1;
    C_RATES_OPTION_TYPE C_RATES_OPTION[NUMRATES1OPTION];
    C_MSG_RA_TYPE       C_MSG_RA;
    C_RATES2_TYPE       C_RATES2;
    BITS_GAIN_TYPE      C_BandG[NUM_UPSTREAM_CHNL-1];
    C_MSG_FR1_TYPE      C_MSG_FR1;
    C_MSG_FR2_TYPE      C_MSG_FR2;
    R_MSGS1_TYPE        R_MSGS1;
    R_MSG_RA_TYPE       R_MSG_RA ;
    R_RATES1_TYPE       R_RATES1;
    R_RATES2_TYPE       R_RATES2;
    R_RATES_RA_TYPE     R_RATES_RA;
    R_MSGS2_TYPE        R_MSGS2;
    BITS_GAIN_TYPE      R_BandG[NUM_DOWNSTREAM_CHNL-1];
    R_MSG_FR1_TYPE      R_MSG_FR1;
    R_MSG_FR2_TYPE      R_MSG_FR2;

    TONE_MAP_TYPE       tone_order_map[NUM_OF_ADSL_CHNL];

    TX_BIT_TONE_TYPE    tx_bit_tone_tbl[NUM_TX_CHNL];
    RX_BIT_TONE_TYPE    rx_bit_tone_tbl[QEDFER_RX_CHNL];
    TONE_INFO_TYPE      rx_tone_info_tbl[QEDFER_RX_CHNL];

    GAIN_TBL_TYPE       tx_gain_tbl[NUM_TX_CHNL];
    GAIN_TBL_TYPE       rx_gain_tbl[QEDFER_RX_CHNL];
    BIT_CAPACITY_INFO_TYPE bit_capacity_info;
    XCVR_PROFILE_TYPE   fast_retrain_profile[MAX_PROFILE_NUM+1];

    SINT16 tx_bit_swap_superframe, rx_bit_swap_superframe;
    UINT8  tx_superframe_count,
           rx_superframe_count;
    GEN_UINT actual_rx_chnl_start, actual_rx_chnl_end;
    GEN_UINT startup_tx_chnl_start, startup_tx_chnl_end;
    GEN_UINT showtime_tx_chnl_start, showtime_tx_chnl_end;
    GEN_UINT loop_timing;

    // In ATU_C (master), this will be set depending on the flag loop_timing
    UINT8  C_ACT_tone;

    // In ATU_R (slave), this will be set depending which C_ACT is rcvd from ATU_C
    UINT8  R_ACK_tone;

    BOOLEAN accept_pilot_xmit;
    BOOLEAN is_same_vendor;
    UINT8  overhead_mode;
    UINT8  trellis_mode;
    UINT8  tone_addr_format;   // this one is to decide if we are using primary or backup tone for 8 bit/sym
    MARGIN_TYPE  coding_gain;
    BOOLEAN G_lite;
    STARTUP_MODE_TYPE  last_startup_mode;  // indicates which method was used for startup (ie G.HS or T1.413)
    BOOLEAN rs16_supported;
    UINT8   max_num_remote_profiles;    // maximum number of profiles the remote can store profiles
    UINT8   num_active_profiles;  // number of rx active profiles (i.e. <= MAX_PROFILE_NUM + 1)
    BOOLEAN new_profile_request;  // this bit indicates the next pwr up setup will be a profile
    BOOLEAN TxGainEstablished;   // this indicates weather a transmit power has been set by a
                                  // fast retrain in G.LITE this is generally reset at power up
    SINT8   TxGain;               // Current xcvr Tx gain level in db (AFE and FFT)
                                  // This variable is set at the following occasions:
                                        // 1) at xcvr startup this is set to 0
                                        // 2) CO politeness cutback during full train
                                        // 3) fast retrain
    SINT8   COPolTxGain;          // Tx level measured during showtime for politesess cutback
#ifdef RT
    BOOLEAN onhook_teq_stored;
#endif
    SINT32 TEQ_estimate[NUM_TEQ_COEF];  // estimate of teq coeficients.
#ifdef CO
    SINT32 BEST_BC_teq[NUM_TEQ_COEF];   // teq estimate with best bit capacity
#endif
    SINT8   up_tx_power;                // Nominal Power offset from default RT=-38, CO=-40, read from nvram
    SINT8   down_tx_power;
    UINT8   profile_index;
#ifdef CO
    UINT8   maxRS;                      // maximum supported RS value
#endif
    SYS_AUTO_SENSING_STATE_TYPE sys_autosensing_state;
    SYS_WIRING_SELECTION_TYPE   sys_wiring;
    SYS_HANDSHAKE_TYPE          sys_handshake;
    GEN_UINT Estimate26AWGkFt;          // Estimate of 26 AWG equivalent cable
    VENDOR_ID_TYPE received_vendor_id;    // Vendor ID of far end equipment received via either G.HS, C/R_MSG1, or EOC
    GEN_UINT retrain_count;     // counter to keep track of number of retrain before achieving showtime
    UINT16 sync_symbol_gain;    // possible gain adjust value for sync symbol during Showtime (unsigned Q8 format)
	BOOLEAN enable_self_vendor_id_faking; // Assumed OK until proven wrong.  Comes in handy sometimes.
} XCVR_DATABASE_TYPE;

// For fast retrain
typedef struct
{
    UINT8 frstart_state;
#ifdef RT
    BOOLEAN hook_status;
    //static SINT16 pilot_atn1;         // Pilot tone attenuation in nominal power
    SINT16 pilot_atn2;                  // Pilot tone attenuation in DMAX-DPF power
    SINT16 noise_log[16];               // noise (loged)
    SINT16 floor_log;
                                        // calculate result Linear line: noise_log=a1*(gain+30)/2+a0
    SINT16 zero_floor_log;
    SINT16 a1;
    SINT16 a0;
    UINT8 stop;
#else //CO
    SINT16 reverb1_atn;
#endif
    SINT16 recov_atn;                   // Attenuation of R-Recov or C-Recov tone
}FR_DATABASE_TYPE;


/****************************************************************************
                      XCVR STARTUP DETECTION FLAGS DEFINITION
****************************************************************************/
#ifdef CO
typedef struct
{
    BOOLEAN   C_SEGUE2_ready,                // Some synchronization flags during startup
              C_SEGUE_RA_ready,
              C_TX_DELAY_done,
              R_ACK_detected,
              R_MSG_to_CRC_RA2_rcvd,
              R_MEDLEY_started,
              R_MSGS2_RATES2_rcvd,
              R_BandG_rcvd,
              R_REVERB1_detected,
              R_REVERB2_started,
#ifdef ECHO_TRAINING
              R_QUIET3_or_PILOT1_started,
#endif
              C_REVERB1_started,
              C_QUIET3_or_PILOT1_started,
              C_QUIET4_or_PILOT2_started,
              C_QUIET5_or_PILOT3_started,
              normal_mode_started,

              r_msg_fr1_received,
              c_reverb_fr3_started,
              finaltxgainset,
              r_reverb_fr3_started,
              reallign_frame_boundary,
              start_channel_error_measurement,
              r_msg_fr2_received,
              c_segue_fr4_started;

}XCVR_STARTUP_FLAG_TYPE;

#else

typedef struct
{
    BOOLEAN   C_TONE_detected,              // Some synchronization flags during startup
              C_SEGUE3_detected,
              C_RATES_to_CRC_RA2_rcvd,
              C_MEDLEY_started,
              C_MSGS2_RATES2_BandG_rcvd,
              C_REVEILLE_rcvd,
              C_QUIET2_rcvd,
              C_REVERB1_detect_ready,
              C_REVERB2_started,
#ifdef ECHO_TRAINING
              C_QUIET5_or_PILOT3_started,
#endif
              C_REVERB3_started,
              normal_mode_started,

              r_line_probe_started,
              start_c_reverb_fr2_detect,
              c_msg_fr1_received,
              c_reverb_fr5_started,
              start_ect_training,
              start_channel_error_measurement,
              c_msg_fr2_received;
}XCVR_STARTUP_FLAG_TYPE;

#endif

// fast retrain power levels
typedef struct
{
    // absolute measured power levels
    // during fast retrain procedure
    SINT8 nominal;                      // measured from R-RECOV or C-RECOV
    SINT8 politeness;                   // measured at R-REVERB-FR1 or C-REVERB-FR2
    SINT8 final;                        // measured at R-REVERB-FR3 or C-REVERB-FR3
} MEASURED_FR_POWERLEVELS;


/****************************************************************************
                      XCVR SYSTEM INFO DEFINITION
                      Used as return values for XCVR_GetSysInfo
****************************************************************************/
typedef UINT16 SYS_OPERATING_DAYS_TYPE;
//#include "sysinfo.h"
typedef struct
{
    UINT16                    Modem_Vendor_ID ;
    UINT8                     Modem_Vendor_Version ;
    UINT8                   * Modem_Product_Number ;
    char                    * Modem_Manufacture_Date ;
    UINT8                   * Modem_Serial_Number ;
    char                    * Modem_Software_Revision ;
    UINT16                    Modem_Prom_Checksum ;
    UINT8                   * Transceiver_Hardware_Number ;
    char                    * Transceiver_Software_Revision ;
    char                    * Falcon_ID ;
    SYS_OPERATING_DAYS_TYPE   Days_In_Operation ;
} SYS_INFO_TYPE ;

typedef struct
{
    BOOLEAN                   Modem_Vendor_ID ;
    BOOLEAN                   Modem_Vendor_Version ;
    BOOLEAN                   Modem_Product_Number ;
    BOOLEAN                   Modem_Manufacture_Date ;
    BOOLEAN                   Modem_Serial_Number ;
    BOOLEAN                   Modem_Software_Revision ;
    BOOLEAN                   Modem_Prom_Checksum ;
    BOOLEAN                   Transceiver_Hardware_Number ;
    BOOLEAN                   Transceiver_Software_Revision ;
    BOOLEAN                   Falcon_ID ;
    BOOLEAN                   Days_In_Operation ;
} SYS_INFO_VALID_TYPE ;


DPCCALL void XCVR_GetSysInfo
(

    PDATAPUMP_CONTEXT             pDatapump,
    UINT8                     Direction,            // DWN_STREAM (RT) vs UP_STREAM (CO)
    SYS_INFO_VALID_TYPE     * Sys_Info_Valid_Ptr,   // no initialization necessary - all ptrs are filled in by this function
    SYS_INFO_TYPE           * Sys_Info_Ptr          // No initialization necessary
) ;


typedef enum
{
    //BOUNDARY MARKERS
    FIRST_PORTION,                      //first portion enumeration marker
    PRE_PORTION = FIRST_PORTION-1,      //DO NOT USE

    //USABLE PORTIONS
    //must be enumerated in order of left to right
    MAJOR_PORTION,                      //major version portion of string   (D.ddz)
    MINOR_PORTION,                      //minor version portion of string   (d.DDz)
    SUB_PORTION,                        //sub version portion of string     (d.ddZ)

    //BOUNDARY MARKERS
    POST_PORTION,                       //DO NOT USE
    LAST_PORTION = POST_PORTION-1,      //Last portion enumeration marker
} PORTION_E ;

DPCCALL GEN_UINT XCVR_ExtractVersionPortion
(
    const char * version_str,                       // Version string in the form d.dda
    PORTION_E portion                               // portion desired to be extracted
) ;

typedef enum
{
    CLR_FILTER_COEF,
    ACQUISITION,
    TRACKING,
    GET_STORE,
    SET_STORE,
    CLR_STORE
}LOOP_TIMING_MODE;



#define DBGP(x) ( (SINT16) pDatapump->debug_parameters[x]  ) 

DPCCALL void    XCVR_ChangeXCVRDefaultCfg(PDATAPUMP_CONTEXT pDatapump, UINT8 inputkey);
DPCCALL UINT16  XCVR_GetDataRate(PDATAPUMP_CONTEXT pDatapump, UINT8);
DPCCALL UINT16  XCVR_GetAttainableDataRate(PDATAPUMP_CONTEXT pDatapump,UINT8 direction);
DPCCALL UINT16  XCVR_GetInterleaveDelay(PDATAPUMP_CONTEXT pDatapump,UINT8 direction);
DPCCALL void    XCVR_SetDataPathType(PDATAPUMP_CONTEXT pDatapump, UINT8 direction, UINT8 type);
DPCCALL void    XCVR_Init(PDATAPUMP_CONTEXT pDatapump);
DPCCALL BOOLEAN XCVR_LossOfSignal(PDATAPUMP_CONTEXT pDatapump, UINT8 direction);
DPCCALL BOOLEAN XCVR_LossOfFrame(PDATAPUMP_CONTEXT pDatapump, UINT8 direction);
DPCCALL BOOLEAN XCVR_LossOfCellDelineation(PDATAPUMP_CONTEXT pDatapump, UINT8 direction);
DPCCALL SINT16  XCVR_GetSNRmargin(PDATAPUMP_CONTEXT pDatapump,  UINT8 direction);
DPCCALL SINT16  XCVR_GetLineAttenuation(PDATAPUMP_CONTEXT pDatapump, UINT8 direction);
DPCCALL BOOLEAN XCVR_NoCellDelineation(PDATAPUMP_CONTEXT pDatapump, UINT8 direction);
DPCCALL UINT32  XCVR_BadHecCount(PDATAPUMP_CONTEXT pDatapump, UINT8 direction);
DPCCALL UINT32  XCVR_CrcCount(PDATAPUMP_CONTEXT pDatapump,  UINT8 direction);
DPCCALL UINT32  XCVR_FECCorrectedErrorCount(PDATAPUMP_CONTEXT pDatapump, UINT8 direction);
DPCCALL void    XCVR_ClearCrcCount(PDATAPUMP_CONTEXT pDatapump,  UINT8 direction);
DPCCALL void    XCVR_ClearFecErrorSecond(PDATAPUMP_CONTEXT pDatapump,  UINT8 direction);
DPCCALL void    XCVR_ClearBadHec(PDATAPUMP_CONTEXT pDatapump,  UINT8 direction);
DPCCALL void    XCVR_GetRSconfiguration(PDATAPUMP_CONTEXT pDatapump, C_RATES_OPTION_TYPE * upstream_rate, C_RATES_OPTION_TYPE * downstream_rate);
DPCCALL void    XCVR_SetRevision(PDATAPUMP_CONTEXT pDatapump, UINT8 version);
DPCCALL void    XCVR_SetVendorID(PDATAPUMP_CONTEXT pDatapump, UINT16 vendor_id);
DPCCALL void    XCVR_SetDataPathType(PDATAPUMP_CONTEXT pDatapump, UINT8 direction, UINT8 type);
DPCCALL void    XCVR_SetMinSNR(PDATAPUMP_CONTEXT pDatapump, GEN_SINT value);
DPCCALL void    XCVR_EnableTrellisCoding(PDATAPUMP_CONTEXT pDatapump, BOOLEAN enable);
DPCCALL void    XCVR_SetFramingMode(PDATAPUMP_CONTEXT pDatapump, UINT8 mode);
DPCCALL void    XCVR_SetMaxInterleaveDelay(PDATAPUMP_CONTEXT pDatapump, UINT8 direction, UINT8 delay);
DPCCALL void    XCVR_SetMinDataRate(PDATAPUMP_CONTEXT pDatapump, UINT8 direction, UINT16 rate);
DPCCALL void    XCVR_SetMaxDataRate(PDATAPUMP_CONTEXT pDatapump, UINT8 direction, UINT16 rate);
DPCCALL void    XCVR_EnableFalconTxClk(PDATAPUMP_CONTEXT pDatapump, BOOLEAN enable);
DPCCALL void    XCVR_PwrDwnMode (PDATAPUMP_CONTEXT pDatapump, BOOLEAN enable);

DPCCALL void    XCVR_SetHandshakeProtocol(PDATAPUMP_CONTEXT pDatapump, UINT8 protocol);
DPCCALL void    XCVR_Start_Fast_Retrain(PDATAPUMP_CONTEXT pDatapump);
DPCCALL UINT16  XCVR_GetItuStandards(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    XCVR_SetItuStandards(PDATAPUMP_CONTEXT pDatapump, UINT16 standards);

#ifdef FALCON_ATM_MODE
DPCCALL void    XCVR_SetupAtmBlock(PDATAPUMP_CONTEXT pDatapump, BOOLEAN idle_cells, UINT8 tx_soc, UINT8 rx_soc);
#endif

DPCCALL void    XCVR_SetCurStartState(PDATAPUMP_CONTEXT pDatapump, UINT8);
DPCCALL BOOLEAN XCVR_IsLinkActive(PDATAPUMP_CONTEXT pDatapump);
DPCCALL char   *XCVR_GetCurSysStateStr(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    XCVR_WriteRemoteXcvrCfg(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    XCVR_PeriodicRefreshRemoteXcvrCfg(PDATAPUMP_CONTEXT pDatapump);

DPCCALL const char *XCVR_GetOtherId(PDATAPUMP_CONTEXT pDatapump);

DPCCALL void XCVR_GetPerf(  PDATAPUMP_CONTEXT pDatapump, UINT8 cmd,
                    UINT8 index, void* up_ptr, void* dwn_ptr );

DPCCALL enum SYS_STATUS_BYTE_TYPE XCVR_SetXCVRState (PDATAPUMP_CONTEXT pDatapump,
                                            XCVR_MAIN_SUB_STATE_TYPE *ptr ,
                                            UINT8 near_far);
DPCCALL void                 XCVR_GetXCVRState(PDATAPUMP_CONTEXT pDatapump,XCVR_MAIN_SUB_STATE_TYPE* ,UINT8);

DPCCALL void                 XCVR_SaveProfile(PDATAPUMP_CONTEXT pDatapump, GEN_UINT, GEN_UINT);
DPCCALL void XCVR_GetDatabaseInfo(
    PDATAPUMP_CONTEXT pDatapump,
    UINT16 *versionFar, 
    UINT16 *VendorIDFar, 
    UINT16 *versionNear, 
    UINT16 *VendorIDNear);
DPCCALL MARGIN_TYPE XCVR_ComputeCodingGainKR (BOOLEAN is_trellis, BOOLEAN is_fec, GEN_UINT K, GEN_UINT R);
DPCCALL MARGIN_TYPE XCVR_ComputeCodingGainRS (GEN_UINT K, GEN_UINT R);
DPCCALL MARGIN_TYPE XCVR_ComputeCodingGainRSTCM (GEN_UINT K, GEN_UINT R);
DPCCALL MARGIN_TYPE XCVR_ComputeCodingGainTarget (BOOLEAN is_trellis, BOOLEAN is_fec);
DPCCALL MARGIN_TYPE XCVR_ComputeCodingGain (BOOLEAN is_trellis, BOOLEAN is_fec);
DPCCALL GEN_UINT XCVR_ExtractVersionPortion (const char * version_str, PORTION_E portion );
DPCCALL void  XCVR_SetSerialNumber (PDATAPUMP_CONTEXT pDatapump, UINT8 *Serial_Num);


extern const UINT8 prbs_match_chnl[];

extern const UINT8 PRBS_MATCH_CHNL_SIZE;
extern const UINT8 far tone_detection_list[];
extern const UINT8 far ghs_tone_rx_band[];
extern const UINT8 far ghs_tone_tx_band[];
extern const UINT8 far ghs_tone_rx_out_band[];
extern const UINT16 far XCVR_constellation_gain_adjust[16][2];
extern const UINT16 xcvr_rx_filter_init[];
extern const XCVR_TX_FILTER_TYPE xcvr_default_tx_filter_init[DAC_TX_FILTER_SIZ];   // ATU-R up to 64 tap Tx Filters #0-3
extern const char * far const XCVR_alarm_str[];
extern const FALCON_CONFIG_OPTION_TYPE falcon_config_option_init;
extern const XCVR_FTEQ_TRAIN_CONFIG_TYPE xcvr_fteq_config_init[];

#endif       // End if for 'ifndef XCVRINFO_H'
/***************************************************************************
 * $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/xcvrinfo.h $
 * 
 * 124   8/27/01 4:14p Lauds
 * change rx chnl start from 64 to 65 (quick fix for Annex B)
 * 
 * 123   8/21/01 4:03p Lauds
 * add in some comments
 * 
 * 122   8/01/01 5:11p Vivianjj
 * Added changes to increase T1.413 upstream rates against Globespan.
 * This included
 * 1) Faking our vender id to 'Alcatel' when connecting to Globespan in
 * T1.413 .
 * 2) Checking to see if pilot tone goes away in showtime after faking id.
 * If it does, disable faking of id.  Pilot tone will disappear if we fake
 * id to Alcatel with certain revs of Cisco/Globespan, 12.1(5) at least.
 * 
 * 121   7/30/01 6:55p Lauds
 * ANNEX B specific changes.  
 * 
 * 120   7/17/01 6:11p Lauds
 * minor Annex B bug in last check-in
 * 
 * 119   7/17/01 6:02p Lauds
 * Annex B support
 * 
 * 118   7/16/01 7:03p Lauds
 * change sync gain for Alcatel DSLAM from tone 16 to average B&G gain
 * 
 * 117   7/12/01 12:51p Lauds
 * Cleanup afeintf.c and add code to recognize Fetcher and utilize extra
 * hybrid gain in Fetcher
 * 
 * 116   7/05/01 7:59p Vivianjj
 * Added Fetcher capability to AFE gain range and AFE short loop
 * internal/external circuitry control.  Added afe_register_image[].
 * Cleaned up AFE_SetRxGain().
 * 
 * 115   6/26/01 8:50p Lauds
 * multi-instance version integration
 * 
 * 2     6/22/01 6:19p Lauds
 * First merge 06/22 tip to multi-instance
 * 
 * 1     6/21/01 3:13p Lauds
 * Multi-instance initial version
 * 
 * 109   5/30/01 6:03p Lauds
 * encapsulate all RT tx filter to rtbgstat.c.
 * 
 * 108   5/25/01 3:24p Vivianjj
 * Restrict vendor ID based retrains to ADI and Medialincs.
 * 
 * 107   5/24/01 6:32p Lauds
 * reduce tx oversampling ratio for Annex B
 * 
 * 106   5/23/01 1:28p Lauds
 * Get rid of the existing AdslOption1 to AdslOption3 and add them to the
 * AdslOptions array.
 * 
 * 105   5/17/01 5:59p Palazzjd
 * Changed  AdslOptions to UINT16 from SINT16.
 * 
 * 104   5/16/01 1:10p Palazzjd
 * 
 * 103   5/16/01 1:04p Palazzjd
 * Added SINT16 array to XCVR_OTHER_CFG_TYPE structure.
 * 
 * 102   5/11/01 10:44a Lauds
 * INF registry needed for power boosting in long loop
 * 
 * 101   5/02/01 12:12p Vivianjj
 * Added Annex B digital Tx filter.  Cleaned out old, unused filters.
 * 
 * 100   5/01/01 7:49p Lauds
 * Annex B support for C_B&G
 * 
 * 99    4/29/01 1:47p Abdallah
 * Improvement to the upstream connect rate above 15 kft with the ADI CSCO
 * DSLAM for Korea.
 * 
 * 98    4/27/01 5:58p Lauds
 * minor bugs and warning cleanup for Euphrates
 * 
 * 97    4/26/01 8:48p Lauds
 * pseudo Annex B support
 * 
 * 96    4/20/01 2:15p Lauds
 * change enumeration to #define
 * 
 * 95    4/19/01 7:44p Lauds
 * cleanup and change coding_gain from Q4 to Q8
 * 
 * 94    4/19/01 3:24p Lauds
 * rename inf entries of MarginAdjust and FecThreshold to AdslOption1 and
 * AdslOption2 respectively
 * 
 * 93    4/18/01 6:56p Lauds
 * add 2 registries in inf to adjust SNR margin and FEC threshold
 * 
 * 92    4/12/01 10:30a Lauds
 * Annex B support
 * 
 * 91    3/30/01 4:56p Lauds
 * CO only change. no need to limit bit per chnl in downstream
 * 
 * 90    3/30/01 6:47p Vivianjj
 * Added  AdslSNR_SecsToRetrain and AdslSES_Sensitivity to global
 * structure XCVR_OTHER_CFG_TYPE.
 * 
 * 89    3/29/01 7:27p Lauds
 * raise PASSBAND RX GAIN for CO from 0.5 to 1
 * 
 * 88    3/27/01 4:40p Lauds
 * CO specific changes for AFE gain setting
 * 
 * 87    3/22/01 10:57a Abdallah
 * Fixed the selection of the TX filter in BNA mode and removed the BNA
 * label and replaced it with HEAD_END_ENV_NO_LINE_DRIVER.
 * 
 * 86    3/21/01 3:57p Lauds
 * add retrain count in XCVR database
 * 
 * 85    3/21/01 3:07p Abdallah
 * Added new Tx filter in order to connect at 800 kpbs upstream with the
 * Serome and DigiTel cards with the ADI CSCO DSLAM in BNA mode. Also
 * reduced the long loop Tx gain to avoid getting FECs/CRCs above at 14
 * kft.
 * 
 * 84    3/16/01 12:28p Vivianjj
 * Added 
 * #define DBGP(x) ( (SINT16) debug_parameters[x]  ) 
 * to xcvrinfo.h
 * 
 * 83    3/13/01 12:50p Vivianjj
 * Added code to detect when upstream connects above 928 kbps.  When this
 * happens, retrain with lower max upstream bits/bin to reduce upstream
 * capability.
 * 
 * 82    3/06/01 6:10p Lauds
 * raise max downstream data rate for CO
 * 
 * 81    2/28/01 12:25p Lauds
 * change default SNR startup margin to 6 dB (only affect CO)
 * 
 * 80    2/22/01 4:13p Vivianjj
 * Added Chung Ho and Chung Hwa to ADSL_TELCO_ID enumeration list.
 * 
 * 79    2/12/01 1:49p Vivianjj
 * Added enumeration of AdslHeadEndEnv possible values.
 * 
 * 78    2/09/01 3:14p Vivianjj
 * Finished merging of 085.010 and 085.011 changes pertaining to
 * AdslTelcoID
 * 
 * 77    2/09/01 10:33a Palazzjd
 * Added support to read AdslTelcoId from the registry.
 * 
 * 76    2/07/01 11:30a Vivianjj
 * Added structure elements in XCVR_DATABASE to hold 26AWG loop length
 * estimate and received vendor id.
 * 
 * 75    2/05/01 9:15a Lauds
 * limit bits per symbol in R_MSGS1 from 15 to 12
 * 
 * 74    1/24/01 2:01p Abdallah
 * Added a new TX filter to improve upstream performance with ADI in BNA
 * mode.
 * 
 * 73    1/10/01 3:31p Palazzjd
 * Changed enum SYS_HANDSHAKE added G_DMT_MODE and      G_LITE_MODE and made
 * sure they are the same numerical
 * value as those defined in hwio.h used by the control panel.              
 * 
 * 72    1/09/01 10:16a Lewisrc
 * Wirespeed/Redhat had to make minor changes to DMT files to compile
 * under GNU
 * 
 * conditinally #undef LINUX ES before defining it
 * 
 * 71    1/05/01 4:33p Lauds
 * Rename all GHS_SILENT1 to GHS_SILENT0
 * 
 * 70    1/05/01 1:55p Lewisrc
 * Add SYS_HANDSHAKE_TYPE values to match new HW_IO values
 * 
 * 69    12/18/00 3:27p Bradforw
 * Added default values in XCVR_DEFLT_OTHER_CFG for newly
 * added elements in the associated structure.
 * 
 * 68    12/14/00 10:20a Palazzjd
 * Added AdslCRCPerSES and  AdslSESToRetrain to
 * XCVR_OTHER_CFG_TYPE data structure.
 * 
 * 67    12/08/00 1:32p Lauds
 * unleash max rate in CO
 * 
 * 66    12/04/00 6:07p Lauds
 * Correct spelling from R_QUITE to R_QUIET
 * 
 * 65    12/01/00 3:58p Lauds
 * XCVR_INDEX_TYPE is UINT8.  This causes inefficiency in both speed and
 * space  in Euphrates and many 32-bit processor. Some mismatch type
 * caused by this fix.  Mismatch type for xcvr_num is change from UINT8 to
 * XCVR_INDEX_TYPE
 * Change vendor ID to CONEXANT_ID
 * 
 * 64    11/29/00 6:49p Lauds
 * fix write transmit filter bug in Smart Terminal. Define array of tx
 * filter for CO
 * 
 * 63    11/28/00 9:45a Vivianjj
 * Made HYBRID_GAIN() macro definition dependant on #definition of the
 * HYBRID_PREGAIN_CIRCUIT.
 * 
****************************************************************************/
/**********                end of file $Workfile: xcvrinfo.h $          *****************/
