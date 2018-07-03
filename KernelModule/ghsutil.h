/****************************************************************************
*  $Workfile: ghsutil.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/ghsutil.h 19    6/26/01 8:11p Lauds $
*****************************************************************************
*  Copyright 1999 PairGain Technologies as unpublished work                 *
*  All Rights Reserved                                                      *
*                                                                           *
*  The information contained herein is the confidential property            *
*  of PairGain Technologies.  The use, copying, transfer or                 *
*  disclosure of such information is prohibited except by express           *
*  written agreement with PairGain Technologies.                            *
*                                                                           *
*                                                                           *
*  Original Written By:  James Infuisno                                     *
*  Original Date:  11/30/98                                                 *
*                                                                           *
*  Reviewed, Edited by  :                                                   *
*****************************************************************************
   Description:   This header file contains all project specific parameters
                  for the system.
****************************************************************************/
#ifndef GHSUTIL_H
#define GHSUTIL_H

typedef enum
{
    GHS_DATA,
    GHS_CTRL_DATA
}GHS_TXDATA_TYPE;

typedef enum
{
    NO_COMPLETE_FRAME=0,
    VALID_FRAME,
    INVALID_FRAME_OVERSIZE,
    INVALID_FRAME_BAD_CRC,
    INVALID_FRAME_TIMEOUT
}GHS_RX_HDLC_FRAME_RESULT_TYPE;

typedef enum
{
    START_OTHER=0,
    START_FAST_RETRAIN_REQUEST_ABORT,
    START_ESCAPE_TO_FAST_RETRAIN,
    START_FAST_RETRAIN_TRUNCATE,
    START_UNKNOWN_PROFILE
} GHS_START_RESON_TYPE;

typedef enum
{
    GHS_GOT_NO_TONE=0,
    GHS_GOT_GHS_TONE,
    GHS_GOT_PILOT_RECOV_TONE
} GHS_TONE_TYPE;

typedef enum
{
    SELECT_G9921A=0,
    SELECT_G9922ABFAST,
    SELECT_G9922ABFULL
} GHS_SELECTION_TYPE;

typedef enum
{
    NO_ABORT=0,
    ABORT_BAD_CRC,
    ABORT_TIMEOUT,
    ABORT_NAK1,
    ABORT_LOCAL_SILENCE_REQUEST,
    ABORT_REMOTE_SILENCE_REQUEST,
    ABORT_GHS_COMPLETE,
    ABORT_HOST_REQUEST,
    ABORT_FAIL_TO_TRAIN,
    ABORT_SILENCE_PERIOD_EXPIRED,
    ABORT_NO_COMPATIBLE_STANDARD,
    ABORT_LOW_REC_PWR,
    NUM_ABORT_REASONS
}GHS_ABORT_REASON_TYPE;

#ifdef RT
typedef enum
{
    GHS_DISABLED=0,
    GHS_RSILENT1,
    GHS_RTONES_REQ,
    GHS_RSILENT2,
    GHS_RTONE1,
    GHS_RDATA_MODE,
    GHS_IDLE,
    NUM_GHS_PHASE
} GHS_PHASE_TYPE;
#else
typedef enum
{
    GHS_DISABLED=0,
    GHS_CSILENT1,
    GHS_CTONES,
    GHS_CGALF,
    GHS_CDATA_MODE,
    GHS_IDLE,
    NUM_GHS_PHASE
} GHS_PHASE_TYPE;
#endif

typedef enum
{
    ENTRY_FROM_STARTUP=0,
    GOT_REMOTE_CAPABILITIES,
    LOCAL_NOT_READY,
    REMOTE_NOT_READY,
    LOCAL_MODE_NOT_SUPPORTED,
    REMOTE_MODE_NOT_SUPPORTED,
    LOCAL_NOT_UNDERSTOOD,
    REMOTE_NOT_UNDERSTOOD,
#ifdef CO
    INSIST_MR,
    DEFER_MS,
    NEED_INFO,
#endif
    NUM_PROT_RESTART_REASON
} GHS_PROTOCOL_RESTART_REASON_TYPE;

#ifdef RT
typedef enum
{
    START_MR=0,   // default
    START_MS,
    START_CLR,
    NUM_PROT_START_MSG
} GHS_PROT_START_MSG;
#endif

#ifdef CO
typedef enum
{
    FORCE_DONT_CARE=0,  // default
    FORCE_LOCAL_MS,
    FORCE_REMOTE_MS,
    NUM_FORCE_TRANSACTION
} GHS_FORCE_TRANSACTION;
#endif

#ifdef RT
typedef enum
{
    PRO_INITIAL_TRANSACTION,
    PRO_SENDING_MS,
    PRO_SENDING_MR,
    PRO_SENDING_CLR,
    PRO_PARTIAL_CL,
    PRO_PARTIAL_MS,
    PRO_CL_OK,
    PRO_MS_OK,
    PRO_NOT_READY,
    PRO_NOT_SUPPORTED,
    PRO_NOT_UNDERSTOOD,
    PRO_SELECT_MODE,
    NUM_PRO_STATES
}   GHS_PRO_STATES;
#else
typedef enum
{
    PRO_INITIAL_TRANSACTION,
    PRO_SENDING_MS,
    PRO_PARTIAL_CLR,
    PRO_PARTIAL_MS,
    PRO_MS_OK,
    PRO_CLR_OK,
    PRO_NOT_READY,
    PRO_NOT_SUPPORTED,
    PRO_NOT_UNDERSTOOD,
    PRO_INSIST,
    PRO_DEFER,
    PRO_NEED_INFO,
    PRO_SELECT_MODE,
    NUM_PRO_STATES
}   GHS_PRO_STATES;
#endif

typedef enum
{
    G9921A_COMPATIBLE=0,
    G9921A_NOT_AVAILABLE,
    G9921A_INCOMP_RACK,
    G9921A_INCOMP_XTM,
    G9921A_INCOMP_DN_STRM,
    G9921A_INCOMP_UP_STRM,
    NUM_G9921A_NEG
} G9921A_NEG_TYPE;

typedef enum
{
    G9921B_COMPATIBLE=0,
    G9921B_NOT_AVAILABLE,
    G9921B_INCOMP_RACK,
    G9921B_INCOMP_XTM,
    G9921B_INCOMP_DN_STRM,
    G9921B_INCOMP_UP_STRM,
    NUM_G9921B_NEG
} G9921B_NEG_TYPE;


typedef enum
{
    G9922AB_COMPATIBLE=0,
    G9922AB_NOT_AVAILABLE,
    G9922AB_INCOMP_RACK,
    NUM_G9922AB_NEG
} G9922AB_NEG_TYPE;

typedef struct
{
    UINT8   country_code;
    UINT8   reserved;
    UINT8   provider_code[4];
    UINT8   vendor_spec_info[2];
}GHS_VENDOR_ID_FIELD;


#define GHS_RESTART_TIMEOUT_MS 4000
#define GHS_RX_OCTET_DELAY 32
#define GHS_CD_SYM_AVG  AVG_128SYM
#define GHS_OCTET_BUF_SIZE 128
#define GHS_MSG_HISTORY_SIZE 16
#define TXFLAG_LEN 5
#define MAX_PAYLOAD_SIZE 64
#define MAX_FRAME_CNT 256

typedef struct

{   
    UINT16  common;              // Standard Info NPar(1) capabilities
    UINT16  standards;
    UINT32  g9921a;               // G.9921a capabilities mask 
    UINT32  g9921b;               // G.9921b capabilities mask 
    UINT16  g9922ab;              // G.9922ab capabilities mask
}GHS_CAPABILITIES_TYPE;

typedef struct GHS_DB_TYPE_S
{
// public  (supervisor)
    GHS_START_RESON_TYPE start_reason;  // reason ghs was started
    UINT8   txdata_buf[64];           // ghs frame buffer
    UINT8   msg_history[GHS_MSG_HISTORY_SIZE];  // rx/tx msg history
    UINT8   msg_history_write_cnt;              // ptr to last write
    UINT8   bad_phy_train_cnt;         // number of unsuccessfull GHS transactions + number of times data mode not reached
    UINT8   bad_ghs_transaction_cnt;   // number of unsuccessfull GHS transactions
    BOOLEAN txer_busy;                 // user read transmitter busy
    BOOLEAN got_remote_capabilities;     // have we rxed a CL or CLR
    GHS_PROTOCOL_RESTART_REASON_TYPE protocol_restart_reason;  // reason protocol restarted
    GHS_PRO_STATES protocol_state;
    BOOLEAN reached_data_mode;          // did the selected modulation reach data mode

#ifdef RT
    GHS_PROT_START_MSG prot_default_start_msg;
#endif
#ifdef CO
    BOOLEAN force_cap_exchange;         // force a capabilities exchange if got_remote_capabilities is false
    GHS_FORCE_TRANSACTION prot_force_transaction;
#endif
    UINT8   tone_cnt;                  // counter to validate tone presence

    UINT8   rx_hdlc_frame_buf[GHS_OCTET_BUF_SIZE];      // location of unparsed hdlc frames
    UINT8   rx_hdlc_frame_cnt;          //  size of data in buffer
    GHS_RX_HDLC_FRAME_RESULT_TYPE rx_hdlc_frame_result;
    UINT16  msg_size;
    GHS_VENDOR_ID_FIELD remote_vendor_id;
    GHS_CAPABILITIES_TYPE local_capabilities, remote_capabilities, negotiated_capabilities;
    G9921A_NEG_TYPE g9921a_neg_result;
    G9921B_NEG_TYPE g9921b_neg_result;
    G9922AB_NEG_TYPE g9922ab_neg_result;
// message transmission variables

    UINT8 *tx_msg_ptr;                  // start of data for HDLC frame txer
    UINT16 msg_octets_remaining;        //  octets in message not framed yet
    UINT16 frames_per_msg;              // (frames left to transmit in current message)-1

// HDLC tx framing variables
    UINT8   txframe_buf[MAX_PAYLOAD_SIZE*9];          // this is the longest
                                       // frame size (64*8)bytes *1  // NOT LONG ENOUGH !!!!
    UINT16  txframe_cnt;               // size of data in buffer

    UINT8   txtemp_buf[TXFLAG_LEN*8];           // temp buffere for 7e filling

    UINT8   last_tx_symbol;            // for the d in dpsk
    UINT8   olast_tx_symbol;           // for the d in dpsk
    UINT8   txdata;                    // global variable for dpsk symbol expansion
    UINT8   txhold_cnt;                // hold off the txer to ensure tx of flags between frames.

    UINT16  tx_fifo;                 // current di_tx_fifo buffer being used

    UINT8   manual_tx_symbol_cnt;       // state counter for falcon fifo buffer issue.

    // variables for rxer decode and bitparse
    UINT8  *rx_octet_buf_start;  // rx octet buffer pointers
    UINT16 rx_octet_buf_size;   // rx octet buffer size
    UINT16 readaddr;
    UINT8 sync_octet;
    BOOLEAN sync_mode;
    UINT8 bitaccm;
    SINT8 lastreversal,sumaccm;
    SINT8 dmtsymcnt;
    UINT8 dpskoctet,dpskbitcount;
    UINT8 last_octet;
    BOOLEAN rx_data_available;

    void (* far rxraw)(UINT8,PDATAPUMP_CONTEXT);    //void *,UINT8
    void (* far rxraw_next)(UINT8,PDATAPUMP_CONTEXT);   //void *,UINT8
    void (* far rxparse)(UINT8,UINT16, PDATAPUMP_CONTEXT);
    void (* far rxparse_next)(UINT8,UINT16, PDATAPUMP_CONTEXT);


// state transition flags
    BOOLEAN ghs_enable;                 // this flag enable the g.hs in the local xcvr
    BOOLEAN ghs_local_initiate;         // this flag forces local xcvr to start ghs transaction
                                        //  if ghs is enabled enabled.

    UINT8   silence_holdoff_cnt;        // RT ONLY!
    BOOLEAN ctones_detected;

    BOOLEAN rx_data_mode;               //set when txer and rxer enter
    BOOLEAN tx_data_mode;               //  default xmission states
    BOOLEAN tx_terminating_galfs;       // send glafs befor tx shutdown.
#ifdef CO
    BOOLEAN reset_other_pingpong_buffer;  // force flags to be written both ping pong buffers
#endif
    BOOLEAN txer_go_idle;               // commands issued by bg
    BOOLEAN rxer_go_idle;

// debug flags
    BOOLEAN ghs_halt_on_all_errors;     // false: behave according to g.hs spec
                                        // true: halt and leave debug info on terminal

//  background state variables
    void (* far bg_state)           (PDATAPUMP_CONTEXT);
    void (* far bg_state_next)      (PDATAPUMP_CONTEXT);
    void (* far bg_state_after_wait)(PDATAPUMP_CONTEXT);
    GHS_ABORT_REASON_TYPE abort_reason;
    GHS_ABORT_REASON_TYPE last_abort_reason;

// physical layer data

    BOOLEAN rxtone_cnvg[GHS_TONE_RX_BAND_SIZE];
    BOOLEAN timer_expired;
    GEN_UINT time_cnt;                  // counter decremented every 100ms.
    UINT16 silence_cnt;
    TICK_TYPE no_connect_tick;          // time spent waiting for connection
    BOOLEAN reset_in_silence_period;
    GHS_PHASE_TYPE phase;
    UINT8 rxtone_cnvg_count;
    UINT8 carrier_loss_cnt;
    UINT16 rx_crc;
    UINT32 rxerravg[GHS_TONE_RX_BAND_SIZE];
    UINT32 pwravg_o;                 // out-band avg tone power
    UINT32 pwravg_i;                 // in-band avg tone power
#ifdef CO
    UINT32 active_power;
    SINT16 nco;
    BOOLEAN carrier_derotate;
    UINT8 bcount,oldbcount,oldoldbcount;

#endif

}GHS_DB_TYPE;





#define FINAL_PAR1      0x80
#define FINAL_PAR2_3    0x40

#define MSG_TYPE_MS     0x00
#define MSG_TYPE_MR     0x01
#define MSG_TYPE_CL     0x02
#define MSG_TYPE_CLR    0x03
#define MSG_TYPE_ACK1   0x10
#define MSG_TYPE_ACK2   0x11
#define MSG_TYPE_NAK_EF 0x20
#define MSG_TYPE_NAK_NR 0x21
#define MSG_TYPE_NAK_NS 0x22
#define MSG_TYPE_NAK_NU 0x23
#define MSG_TYPE_REQ_MS 0x34
#define MSG_TYPE_REQ_MR 0x35
#define MSG_TYPE_REQ_CLR 0x37

#define O_MSG_TYPE_MS   0x0001
#define O_MSG_TYPE_MR   0x0002
#define O_MSG_TYPE_CL   0x0004
#define O_MSG_TYPE_CLR  0x0008
#define O_MSG_TYPE_ACK1  0x0010
#define O_MSG_TYPE_ACK2  0x0020
#define O_MSG_TYPE_NAK_EF 0x0040
#define O_MSG_TYPE_NAK_NR 0x0080
#define O_MSG_TYPE_NAK_NS 0x0100
#define O_MSG_TYPE_NAK_NU 0x0200
#define O_MSG_TYPE_REQ_MS 0x0400
#define O_MSG_TYPE_REQ_MR 0x0800
#define O_MSG_TYPE_REQ_CLR 0x1000

#define HIST_MSG_EMPTY     0
#define HIST_MSG_UNKNOWN   1
#define HIST_MSG_MS        2
#define HIST_MSG_MR        3
#define HIST_MSG_CL        4
#define HIST_MSG_CLR       5
#define HIST_MSG_ACK1      6
#define HIST_MSG_ACK2      7
#define HIST_MSG_EF        8
#define HIST_MSG_NR        9
#define HIST_MSG_NS        10
#define HIST_MSG_NU        11
#define HIST_MSG_REQ_MS    12
#define HIST_MSG_REQ_MR    13
#define HIST_MSG_REQ_CLR   14
#define A_MSG_INFO         0x7F
#define O_HIST_MSG_RX      0x80
#define O_HIST_MSG_TX      0x00

#define O_MSG_TYPE_NAK  (O_MSG_TYPE_NAK_EF|O_MSG_TYPE_NAK_NR|O_MSG_TYPE_NAK_NS| \
                        O_MSG_TYPE_NAK_NU|O_MSG_TYPE_REQ_MS|O_MSG_TYPE_REQ_MR| \
                        O_MSG_TYPE_REQ_CLR)

#define O_MSG_TYPE_ACK  (O_MSG_TYPE_ACK1|O_MSG_TYPE_ACK2)

#define GHS_REV         0x01

// protocol flags
#define GOT_REMOTE_CAPABILITIES 0x0001


typedef union
{
    UINT8 uint8[1];
    struct
    {
        UINT8   msg_type;
        UINT8   rev;
    } st;
} GHS_HEADER;

/*
typedef struct
{
    UINT8   dat     : 7;
    UINT8   delim1  : 1;                 //
}Par1;

typedef struct
{
    UINT8   dat     : 6;
    UINT8   delim2_3: 1;
    UINT8   delim1  : 1;                 //
}Par2_3;
*/
#define Par1 UINT8
#define Par2_3 UINT8



#define PARAM_NSF               0x40

#define PARAM_NET_RATE_UP       0x01
#define PARAM_NET_RATE_DN       0x02
#define PARAM_DATA_FLOW_UP      0x04
#define PARAM_DATA_FLOW_DN      0x08
#define PARAM_RT_SPLITTER       0x10
#define PARAM_CO_SPLITTER       0x20

#define NUM_STANDARD 5

#define INFO_VB_V8          0x01
#define INFO_VB_V8BIS       0x02
#define INFO_SILENT         0x04
#define INFO_GPLOAM         0x08

#define INFO_G9921A         0x01
#define INFO_G9921B         0x02
#define INFO_G9921C         0x04
#define INFO_G9922AB        0x08
#define INFO_G9922C         0x10

#define INFO_RACK1          0x01
#define INFO_RACK2          0x02
#define INFO_UP_TONE_1_32   0x04    // Annex B only
#define INFO_FAST_RTRN      0x08
#define INFO_STM            0x08
#define INFO_RS16           0x10
#define INFO_ATM            0x10
#define INFO_GPLOAM_CL_EOC  0x20

#define INFO_SUB_CH_INFO    0x01
#define INFO_SPEC_FREQ_UP   0x02
#define INFO_SPEC_FREQ_DN   0x04

#define INFO_AS0_DN         0x01
#define INFO_AS1_DN         0x02
#define INFO_AS2_DN         0x04
#define INFO_AS3_DN         0x08
#define INFO_LS0_DN         0x10

#define INFO_SUBCH1_VALID   (INFO_AS0_DN| \
                            INFO_AS1_DN| \
                            INFO_AS2_DN| \
                            INFO_AS3_DN| \
                            INFO_LS0_DN)

#define INFO_LS1_DN         0x01
#define INFO_LS2_DN         0x02
#define INFO_LS0_UP         0x04
#define INFO_LS1_UP         0x08
#define INFO_LS2_UP         0x10

#define INFO_SUBCH2_VALID   (INFO_LS1_DN| \
                            INFO_LS2_DN| \
                            INFO_LS0_UP| \
                            INFO_LS1_UP| \
                            INFO_LS2_UP)

#define CAP_STANDARD_G9921A  INFO_G9921A
#define CAP_STANDARD_G9921B  INFO_G9921B
#define CAP_STANDARD_G9921C  INFO_G9921C
#define CAP_STANDARD_G9922AB INFO_G9922AB
#define CAP_STANDARD_G9922C  INFO_G9922C

// Annex A G.dmt standard information field
#define NUM_GHS9921A_CAP    15
#define CAP_9921A_RACK1         0x0001
#define CAP_9921A_RACK2         0x0002
#define CAP_9921A_STM           0x0004
#define CAP_9921A_ATM           0x0008
#define CAP_9921A_GPLOAM_EOC    0x0010
#define CAP_9921A_AS0D          0x0020
// amount of shifting for CAP_9921A_AS0D to bit 0 of subchnl info in NPar(3) Octet 1
#define CAP_9921A_AS0D_SHIFT    5      
#define CAP_9921A_AS1D          0x0040
#define CAP_9921A_AS2D          0x0080
#define CAP_9921A_AS3D          0x0100
#define CAP_9921A_LS0D          0x0200
#define CAP_9921A_LS1D          0x0400
// amount of shifting for CAP_9921A_LS1D to bit 0 of subchnl info in NPar(3) Octet 2
#define CAP_9921A_LS1D_SHIFT    10     
#define CAP_9921A_LS2D          0x0800
#define CAP_9921A_LS0U          0x1000
#define CAP_9921A_LS1U          0x2000
#define CAP_9921A_LS2U          0x4000
#define CAP_9921A_ASLS      (CAP_9921A_AS0D|  \
                            CAP_9921A_AS1D|  \
                            CAP_9921A_AS2D|  \
                            CAP_9921A_AS3D|  \
                            CAP_9921A_LS0D|  \
                            CAP_9921A_LS1D|  \
                            CAP_9921A_LS2D|  \
                            CAP_9921A_LS0U|  \
                            CAP_9921A_LS1U|  \
                            CAP_9921A_LS2U)
// Annex B G.dmt standard information field

#define NUM_GHS9921B_CAP    16
#define CAP_9921B_RACK1         0x0001
#define CAP_9921B_RACK2         0x0002
#define CAP_9921B_UP_TONE_1_32  0x0004
#define CAP_9921B_STM           0x0008
#define CAP_9921B_ATM           0x0010
#define CAP_9921B_GPLOAM_EOC    0x0020
#define CAP_9921B_AS0D          0x0040
// amount of shifting for CAP_9921B_AS0D to bit 0 of subchnl info in NPar(3) Octet 1
#define CAP_9921B_AS0D_SHIFT    6      
#define CAP_9921B_AS1D          0x0080
#define CAP_9921B_AS2D          0x0100
#define CAP_9921B_AS3D          0x0200
#define CAP_9921B_LS0D          0x0400
#define CAP_9921B_LS1D          0x0800
// amount of shifting for CAP_9921B_LS1D to bit 0 of subchnl info in NPar(3) Octet 2
#define CAP_9921B_LS1D_SHIFT    11     
#define CAP_9921B_LS2D          0x1000
#define CAP_9921B_LS0U          0x2000
#define CAP_9921B_LS1U          0x4000
#define CAP_9921B_LS2U          0x8000
#define CAP_9921B_ASLS     (CAP_9921B_AS0D|  \
                            CAP_9921B_AS1D|  \
                            CAP_9921B_AS2D|  \
                            CAP_9921B_AS3D|  \
                            CAP_9921B_LS0D|  \
                            CAP_9921B_LS1D|  \
                            CAP_9921B_LS2D|  \
                            CAP_9921B_LS0U|  \
                            CAP_9921B_LS1U|  \
                            CAP_9921B_LS2U)

// Annex A G.lite standard information field

#define NUM_GHS9922AB_CAP    5
#define CAP_9922AB_RACK1     0x0001
#define CAP_9922AB_RACK2     0x0002
#define CAP_9922AB_FAST_RTRN 0x0004
#define CAP_9922AB_RS16      0x0008
#define CAP_9922AB_GPLOAM_EOC 0x0010

#define NUM_CAP_COMMON       4
#define CAP_COMMON_V8        INFO_VB_V8
#define CAP_COMMON_V8BIS     INFO_VB_V8BIS
#define CAP_COMMON_SILENT    INFO_SILENT
#define CAP_COMMON_GPLOAM    INFO_GPLOAM

typedef union
{
    UINT8 uint8[1];
    struct
    {
        UINT8 octet_1;
        UINT8 octet_2;
    } st;
} GHS_CRC_FIELD;

typedef enum
{
    PAR_COMPLETE=0,
    PAR_INCOMPLETE,
    PAR_BAD_TERMINATING_PAR2
} GHS_PAR_PARSE_RESULT_TYPE;


typedef struct
{
  UINT16    npar1_octets;               // size of npar1 in octets
  UINT16    spar1_octets;               // size of spar1 in octets
  UINT16    total_par2_blocks;          // from spar1 information
  UINT16    rxed_par2_blocks;           // complete at parse time
  UINT16    field_size;
  GHS_PAR_PARSE_RESULT_TYPE   result;
} GHS_PAR_STAT_TYPE;

typedef enum
{
    PAR2_COMPLETE=0,
    PAR2_BAD
} GHS_PAR2_PARSE_RESULT_TYPE;

typedef struct
{
  UINT16    npar2_octets;
  UINT16    spar2_octets;
  UINT16    total_npar3_blocks;
  UINT16    field_size;
  GHS_PAR2_PARSE_RESULT_TYPE   result;   // if expected npar3 blocks does not match actual number
                                        // result = PAR2_BAD
} GHS_PAR2_STAT_TYPE;

typedef struct
{
    UINT8   total_nsf_blocks;
    UINT8   rxed_nsf_blocks;
} GHS_NSF_STAT_TYPE;

typedef struct
{
    UINT16 o_msg_type;
    GHS_HEADER *pheader;
    GHS_VENDOR_ID_FIELD *pvid;
    UINT8 *ppar[2];
    GHS_PAR_STAT_TYPE par_field[2];     // up to 2 par fields in message
    UINT8 *pnsf;
    GHS_NSF_STAT_TYPE nsf_field;
} GHS_MSG_STAT_TYPE;

typedef enum
{
    PARSE_NOT_SUPPORTED=0,
    PARSE_OK,
    PARSE_NEED_INFO,
    PARSE_BAD_PAR2_BLOCK,
#ifdef CO
    PARSE_WANT_LOCAL_MS,
#endif
    NUM_PARSE_RESULTS
} GHS_PARSE_RESULT;

typedef enum
{
    INFO_FIELD_MS=0,
    INFO_FIELD_CLX
} GHS_INFO_FIELD_TYPE;

typedef void (* LOCAL_GHS_FUNC)(PDATAPUMP_CONTEXT);

DPCCALL void GHS_Timer(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void ghs_start_timer(PDATAPUMP_CONTEXT pDatapump,GEN_UINT time_cnt);
DPCCALL void reset_ghs_bg(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void ghs_bg_state(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void ghs_rx_state(PDATAPUMP_CONTEXT pDatapump,UINT8 * rxbuf,SINT32 *avgerr);

DPCCALL void expand_txframe8(const UINT8 *sourceptr,GEN_UINT slength,PDATAPUMP_CONTEXT pDatapump);
DPCCALL void expand_txframe16(const UINT8 *sourceptr,GEN_UINT slength,PDATAPUMP_CONTEXT pDatapump);
DPCCALL void insert_dpsk_sym(PDATAPUMP_CONTEXT pDatapump);

DPCCALL void reset_rx_bitparse(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void rx_bitparse(UINT8 *rxbuf,UINT16 bufsize,PDATAPUMP_CONTEXT pDatapump);
DPCCALL void reset_filter_hdlc_data(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void filter_hdlc_data(UINT8 octet,PDATAPUMP_CONTEXT pDatapump);

DPCCALL void reset_TransmitGHSframe(PDATAPUMP_CONTEXT pDatapump);
DPCCALL BOOLEAN TransmitGHSframe(const UINT8 *source,GHS_TXDATA_TYPE format, UINT8 slength,PDATAPUMP_CONTEXT pDatapump);
DPCCALL BOOLEAN TransmitGHSmessage(const UINT8 *source,UINT16 slength,PDATAPUMP_CONTEXT pDatapump, LOCAL_GHS_FUNC);
DPCCALL BOOLEAN ContinueTxGHSmessage(PDATAPUMP_CONTEXT pDatapump, LOCAL_GHS_FUNC);
DPCCALL BOOLEAN ReceiveGHSframe(PDATAPUMP_CONTEXT pDatapump,UINT8 *buf,UINT16 size, LOCAL_GHS_FUNC);
DPCCALL void ReceiveGHSmessage(PDATAPUMP_CONTEXT pDatapump, LOCAL_GHS_FUNC);
DPCCALL void ContinueRxGHSmessage(PDATAPUMP_CONTEXT pDatapump, LOCAL_GHS_FUNC);


DPCCALL void toggle_tx_fifo(PDATAPUMP_CONTEXT pDatapump);

DPCCALL BOOLEAN ghs_pwr_tone_detect(const UINT32 *tone_pwr,PDATAPUMP_CONTEXT pDatapump);

#ifdef RT
DPCCALL GHS_TONE_TYPE ghs_pwr_mtone_detect(const UINT32 *tone_pwr,UINT32 forgnd_pwr);
#endif   // RT
DPCCALL BOOLEAN ghs_rx_eq_converge(SINT32 *erravg,PDATAPUMP_CONTEXT pDatapump);

DPCCALL void ghs_shutdown_xcvr(PDATAPUMP_CONTEXT pDatapump,BOOLEAN tx_terminating_galfs);

DPCCALL void ghs_handle_abort(PDATAPUMP_CONTEXT pDatapump);


DPCCALL void rx_hdlc_frame2(UINT8 octet,UINT16 msg,PDATAPUMP_CONTEXT pDatapump);
DPCCALL void disgard_rx_parse(UINT8 octet,UINT16 msg,PDATAPUMP_CONTEXT pDatapump);

DPCCALL void bg_startup(PDATAPUMP_CONTEXT pDatapump);

DPCCALL UINT16 ccitt_updcrc( UINT16 start, const UINT8* ptr, UINT16 size );

DPCCALL void bg_wait_end_tx_frame(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void bg_wait_rx_frame(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void bg_idle(PDATAPUMP_CONTEXT pDatapump);

#ifdef RT
DPCCALL void switch_to_hunt_7e(UINT8 octet,PDATAPUMP_CONTEXT pDatapump);
DPCCALL void switch_to_hunt_81(UINT8 octet,PDATAPUMP_CONTEXT pDatapump);
#endif     // RT

#ifdef CO
void validate_7e_f2p(UINT8 octet,PDATAPUMP_CONTEXT pDatapump);  // FALCON2P
void validate_rtones_req(UINT8 octet,PDATAPUMP_CONTEXT pDatapump);
void validate_rtones(UINT8 octet,PDATAPUMP_CONTEXT pDatapump);
#endif  // CO

DPCCALL UINT16 build_ms(UINT8 *buf,PDATAPUMP_CONTEXT pDatapump);
DPCCALL UINT16 build_nak(UINT8 *buf,GEN_UINT nack_type);
DPCCALL UINT16 build_ack(UINT8 *buf,GEN_UINT ack_type);
DPCCALL UINT16 build_mr(UINT8 *buf);
DPCCALL UINT16 build_clr(UINT8 *buf,PDATAPUMP_CONTEXT pDatapump);
DPCCALL GHS_PARSE_RESULT check_ms(GHS_MSG_STAT_TYPE *pmst,PDATAPUMP_CONTEXT pDatapump);


DPCCALL void get_msg_stats(PDATAPUMP_CONTEXT pDatapump,GHS_MSG_STAT_TYPE *pmst);
DPCCALL void diag_par_stats(GHS_PAR_STAT_TYPE * ppst,PDATAPUMP_CONTEXT pDatapump);
DPCCALL void diag_msg_stats(PDATAPUMP_CONTEXT pDatapump, GHS_MSG_STAT_TYPE * pmst);
DPCCALL void reset_msg_history(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void msg_history_write(PDATAPUMP_CONTEXT pDatapump,UINT8 msg_type,BOOLEAN rx);

DPCCALL void GHS_warm_start(PDATAPUMP_CONTEXT pDatapump);

//UINT16 get_length_of_subfield(UINT8 level,UINT8 *start);
DPCCALL UINT8 get_msg_bank(UINT16 fifo);

DPCCALL void reset_GHS_options(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void reset_GHS_state_vars(PDATAPUMP_CONTEXT pDatapump);

DPCCALL BOOLEAN ghs_select_best_standard(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void ghs_negotiate_all_capabilities(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void ghs_set_remote_cap(GHS_MSG_STAT_TYPE *pmst,PDATAPUMP_CONTEXT pDatapump,GHS_INFO_FIELD_TYPE ift);
DPCCALL void ghs_reset_remote_capabilities(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void diag_par2_stats(GHS_PAR2_STAT_TYPE * ppst,PDATAPUMP_CONTEXT pDatapump);
DPCCALL void wait_end_remote_xmit(PDATAPUMP_CONTEXT pDatapump,LOCAL_GHS_FUNC after_rx_galfs);

#endif  // End GHSUTIL_H

/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/ghsutil.h $
 * 
 * 19    6/26/01 8:11p Lauds
 * multi-instance version integration
 * 
 * 3     6/22/01 6:18p Lauds
 * First merge 06/22 tip to multi-instance
 * 
 * 2     6/22/01 3:11p Lauds
 * minor cleanup and bug fix in miscutil.c and terminal.c
 * 
 * 15    5/11/01 10:47a Lauds
 * Annex B stuff for G.HS
 * 
 * 14    12/01/00 3:48p Lauds
 * XCVR_INDEX_TYPE is UINT8.  This causes inefficiency in both speed and
 * space  in Euphrates and many 32-bit processor. Some mismatch type
 * caused by this fix.  Mismatch type for xcvr_num is change from UINT8 to
 * XCVR_INDEX_TYPE
 * 
 * 13    11/22/00 12:12p Lauhk
 * Cleanup to reduce compiler warnings in Euphrates.
 * 
 * 12    11/01/00 6:17p Lauds
 * cleanup code and fix g.hs hangup problem in GHS_INIT_TXER, GHS_RX_DATA,
 * GHS_BG_SCHEDULE
 * 
 * 11    2/01/00 8:34a Lewisrc
 * 2.02i Beta (Pre HSV Interop drop)
 * 
 * 12    1/31/00 2:29p Lewisrc
 * 2.02i Beta - prior to HSV interop
 * 
 *    Rev 1.13   Jan 26 2000 11:48:26   yangl
 * -- Add STDEOC API
 * -- Support clear EOC
 * -- Add misc cfg menu
 *
 *    Rev 1.12   Dec 03 1999 10:26:58   infusij
 * -merged co and rt ghs bitparse
 *
 *    Rev 1.11   Nov 24 1999 17:19:54   infusij
 * - removed falcon2 ghs tx patch for CO
 * - removed CO falcon2 support for g.hs. only falcon2p CO and higher now supported, RT falcon2 and higher
 * - added comments and function headers for g.hs files
 *
 *
 *    Rev 1.10   Oct 15 1999 10:31:46   yangl
 * Use T1.413 as default even with GHS_ENABLE
 *
 *    Rev 1.9   Oct 15 1999 05:24:50   InfusiJ
 *
 *
 *    Rev 1.8   Sep 16 1999 09:42:18   yangl
 * Support to save basic G.hs options to nvram.
 * May need add more later.
 *
 *    Rev 1.7   Aug 24 1999 13:17:28   InfusiJ
 * - Changes from UNH INTEROP 8-99
 * - TX gain setting now involves AFE and FFT gain always
 * - Reach is currently 19.5 Kf with G.HS and 19 Kf with T.413
 *
 *    Rev 1.6   Aug 02 1999 17:32:18   InfusiJ
 * -ghs phy layer fixes
 * -fr startup mods
 * -fr dpu page added
 *
 *    Rev 1.5   Jul 29 1999 15:54:04   InfusiJ
 * Initial Fast Retrain merge.
 * FR is enabled ONLY with G.hs
 * FR is in a Debug state
 * Escape to FR is not functional
 *
 *    Rev 1.4   May 06 1999 15:32:02   InfusiJ
 * ghs Belgium interop updates
 *
 *    Rev 1.3   Apr 21 1999 18:22:44   InfusiJ
 * -added G.hs -> G.lite(full retrain)
 * -improved G.hs -> G.dmt support
 * -ghs tone detectors are now 32 bit
 * -ghs states tested for Belgium interop.
 *
 *    Rev 1.2   Apr 14 1999 16:21:34   InfusiJ
 * g.hs update. 3com interop
 *
 *    Rev 1.1   Apr 01 1999 12:18:58   InfusiJ
 * g.hs update - co & rt code merged, still not operational
 *
 *    Rev 1.0   Mar 22 1999 15:47:18   nordees
 *
*
*****************************************************************************/
