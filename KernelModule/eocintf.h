/****************************************************************************
*  $Workfile: eocintf.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/eocintf.h 4     7/10/01 12:58p Lauds $
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
*  Original Written By:  Todd Thompson                                      *
*  Original Date: 12/3/96                                                   *
*                                                                           *
*****************************************************************************
   Description: This header file contains shared type definitions needed
       to use the EOC Interface functions.

****************************************************************************/
#ifndef _EOCINTF_H_
#define _EOCINTF_H_

/*****                      INCLUDE SECTION                           ******/

/*****               GLOBAL DECLARATION SECTION                       ******/


/* These are all the possible commands that may be sent over the EOC */

/* NOTE:0xAA can NOT be used as a command value, since this is the sync byte */
typedef enum eoc_cmd_type{

    // Commands which require confirmation msg:
    EOC_WRITE_BLOCK_CONFIRM=0x01,
    EOC_RESET_DATABASE_CONFIRM,
    EOC_RESET_SYSTEM_CONFIRM,
    EOC_SET_TEST_MODE,
    EOC_TEST_LINK,
    EOC_NUM_CONFIRM_CMDS,     // add new elements before this


    // Commands which don't require a confirmation msg:
    EOC_INIT,                // Init link, sent by ATU-C
    EOC_INIT_REPLY,          // Reply sent by ATU-R
    EOC_CONFIRMATION,
    EOC_RESET_SYSTEM,
    EOC_RESET_DATABASE,
    EOC_READ_BLOCK,
    EOC_READ_REPLY,
    EOC_WRITE_BLOCK,
    EOC_GET_BRIDGE,
    EOC_GET_BRIDGE_REPLY,
    EOC_SET_BRIDGE,
    EOC_SET_SYSTEM_DATE,
    EOC_SET_REMOTE_LOGON,
    EOC_REMOTE_KEY_DATA,
    EOC_REMOTE_SCREEN_DATA,
    EOC_SET_RT_LOGIN_MODE,
    EOC_IDLE_CODE,
    EOC_UNABLE_TO_COMPLY
} EOC_CMD_TYPE;

typedef enum eoc_reset_database_type{
    EOC_DIAG_LOG = 0x01,         // To reset diagnostic log
    EOC_ALARMS = 0x02,           // To reset alarms
    EOC_HISTORY_24 = 0x04,       // To reset 24 hour history
    EOC_HISTORY_7 = 0x08,        // To reset 7 day history
    EOC_LAST_ERD_TYPE = 0x10
} EOC_RESET_DATABASE_TYPE;

typedef enum eoc_error_type{
    EOC_UNKNOWN_CMD=0x00,
    EOC_CMD_INVALID,
    EOC_MSG_FORMAT_ERR
} EOC_ERROR_TYPE;

typedef enum eoc_request_state_type{
    EOC_NO_REQUEST_PENDING,     // command available
    EOC_WAITING_CONFIRMATION,   // command sent waiting confirmation
    EOC_CONFIRMATION_RECEIVED,  // command confirmed
    EOC_NO_COMPLY_RECEIVED,     // error on commmand received
    EOC_UNKNOWN                 // this command not supported
} EOC_REQUEST_STATE_TYPE;

typedef enum eoc_set_test_type{
    EOC_START_TEST,                // starts test
    EOC_REQUEST_NUM_BIT_ERRS,      // request num bit errors detected during system test
                                   //     confirmation will contain num
                                   //     of errors detected
	EOC_REQUEST_ST_STATUS,         // get selftest results
    EOC_STOP_TEST                  // stops test
} EOC_SET_TEST_TYPE;

typedef enum eoc_remote_logon_type{
    EOC_LOGON,                  // request to logon
    EOC_LOGOFF                  // request to logoff
}EOC_REMOTE_LOGON_TYPE;

enum eoc_protocol_type{
	EOC_SIMPLE_PROTOCOL = 1
};

typedef UINT8 EOC_PROTOCOL_TYPE;

typedef struct eoc_version_type
{
	EOC_PROTOCOL_TYPE protocol_type; // for major changes to protocol
	UINT8 version;                   // for minor changes to protocol
}EOC_VERSION_TYPE;
#define MAX_LEN         253   /* data field maximum length */
#define MAX_MSG        (MAX_LEN + MSG_OVERHEAD) /* data + len + cmd */
#define MSG_OVERHEAD      2   /* overhead bytes in addition to data */
#define MAX_GLOBAL_DATA  64   /* practical length of a global data */
                              /* entry. Must be larger than than the size */
                              /* of any "size" field in global_data_index_tbl */
                              /* including overhead and less than MAX_MSG */
                              /* NOTE: when changing this value, must */
                              /* remember that this many bytes will be */
                              /* allocated on the stack in reentrant */
                              /* routines below */

                              /* It must also be larger than any single */
                              /* EBM data element to be read/written */

#define MAX_SCREEN_DATA 50    /* Maximum amount of screen data that maybe */
                              /* sent by the EOC layer at one time. */
                              /* If more is sent, the EOC breaks it up. */


typedef struct
{
    EOC_VERSION_TYPE eoc_protocol[NUM_ATU_MODE];
    DYN_FIFO_TYPE tx_msgq;
    DYN_FIFO_TYPE rx_msgq;
    UINT8 eoc_initd;  // TRUE indicates protocol has been
                                            // initialized
    UINT8 eoc_cw_sem; // confirm write semaphore
    UINT8 eoc_rd_sem; // confirm reset db semaphore
    UINT8 eoc_rs_sem; // confirm reset system semaphore
    UINT8 eoc_st_sem; // confirm set test mode sem.
    UINT8 eoc_tl_sem; // confirm test link semaphore
    UINT8 in_buf[MAX_MSG];
    UINT16 eocintf_length;             /* expected data length */
    UINT8 byte_num;              /* offset into rcv'd msg */
    UINT8  rx_state;   /* state of msg reception */
    UINT8  rx_crc;              /* stores calculated crc */
    UINT8 tx_state;
    UINT8 tx_crc;
    UINT8 num_byte;
    UINT8 tx_length;
    GEN_UINT last_parity;
    UINT8* out_msg_ptr; /* points to next byte of msg */
    UINT8 tmp_buf[MAX_GLOBAL_DATA];
    UINT8 eoc_request_states[EOC_NUM_CONFIRM_CMDS+1];
    // this eocintf_logon_mode has a different meaning as in serial.c,
    // when set to LOCAL_LOGON, it means that this ATU is not currently
    // being logged on by the remote ATU. If set to REMOTE_LOGON, this ATU
    // is being logged on. All remote Logon Message Requests will be responded
    // to the remote ATU via the eoc channel.
    SYS_LOGON_MODE_TYPE eocintf_logon_mode;
}LOCAL_EOCINTF_TYPE;



DPCCALL void    EOC_Reset(PDATAPUMP_CONTEXT pDatapump);

DPCCALL void    EOC_InitMsgQ(PDATAPUMP_CONTEXT pDatapump, void * txq_buf, void * rxq_buf,
                  UINT16 txq_buf_size, UINT16 rxq_buf_size);
DPCCALL BOOLEAN EOC_IsRemoteDumbTermLogon(PDATAPUMP_CONTEXT pDatapump);  // By terminal.c

// Sending messages to other end:
DPCCALL void    EOC_ReadRemoteData(PDATAPUMP_CONTEXT pDatapump,const UINT8 index);
DPCCALL void    EOC_ResetDatabase(PDATAPUMP_CONTEXT pDatapump, const EOC_RESET_DATABASE_TYPE db);
DPCCALL void    EOC_SetDateTime(PDATAPUMP_CONTEXT pDatapump);

// Remote Logon Request/Maintenance
DPCCALL void    EOC_SendRemoteLogonCmd(PDATAPUMP_CONTEXT pDatapump, EOC_REMOTE_LOGON_TYPE logon_request);
DPCCALL void    EOC_SendRemoteKey(PDATAPUMP_CONTEXT pDatapump, UINT8 *msg, UINT8 length);
DPCCALL void    EOC_SendRemoteScreenData(PDATAPUMP_CONTEXT pDatapump, const char *msg, unsigned size);
DPCCALL void    EOC_ClearRemoteLogon(PDATAPUMP_CONTEXT pDatapump);

// Sending messages to other end when a confirmation is requested
DPCCALL EOC_REQUEST_STATE_TYPE EOC_CheckConfirmState(PDATAPUMP_CONTEXT pDatapump,EOC_CMD_TYPE cmd);

DPCCALL BOOLEAN EOC_ConfirmWrite(   PDATAPUMP_CONTEXT pDatapump,
                            const UINT8 index,
                            const UINT8 which_copy, const void * msg);
DPCCALL BOOLEAN EOC_ClearConfirmState(PDATAPUMP_CONTEXT pDatapump,EOC_CMD_TYPE cmd);
DPCCALL BOOLEAN EOC_ConfirmSetTestMode(PDATAPUMP_CONTEXT pDatapump,
                                const EOC_SET_TEST_TYPE test_request,
                                const XCVR_SELFTEST_ID_TYPE test_type);

// Tx and Rx Message Handlers - To be called only from one process

DPCCALL void    EOC_ProcessRxByte(PDATAPUMP_CONTEXT pDatapump,UINT8 inbyte);
DPCCALL UINT16  EOC_GetTxByte(PDATAPUMP_CONTEXT pDatapump, UINT8 *msg_state);
DPCCALL void    EOC_ProcessRxcMsg(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void EOC_RxEocIntrHandler(PDATAPUMP_CONTEXT pDatapump);

DPCCALL void EOC_InitLocalStatic(PDATAPUMP_CONTEXT pDatapump);

#endif
/***************************************************************************
 * $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/eocintf.h $
 * 
 * 4     7/10/01 12:58p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 * 
 * 3     6/26/01 8:01p Lauds
 * multi-instance version integration
 * 
 * 2     6/22/01 3:10p Lauds
 * minor cleanup and bug fix in miscutil.c and terminal.c
 * 
 * 1     6/21/01 3:07p Lauds
 * Multi-instance initial version
 * 
 * 2     10/11/99 8:38a Richarjc
 * 09/30/99 2.02C release changes
 * 
 *    Rev 1.10   Sep 29 1999 10:09:14   phang
 * Add EOC message to disable local login on RT
 * 
 *    Rev 1.9   Sep 30 1998 17:49:26   laud
 * eoc support for multiple xcvr and sw ver API
 *
 *    Rev 1.8   27 Oct 1997 15:52:24   phamt
 * Port to DSLAM design
 *
 *    Rev 1.7   10 Apr 1997 14:06:36   THOMPSON
 * Move global database to sysinfo.c. Added error checking to eoc,
 * some new commands and init protocol. Removed unused items
 * from global database and added some new ones.
 *
 *    Rev 1.6   28 Mar 1997 17:12:50   THOMPSON
 *
 * Added init link reply and modified selftest commands.
 *
 *    Rev 1.5   04 Mar 1997 11:05:50   THOMPSON
 * Remote logon implemented and tested.
 *
 *    Rev 1.4   28 Feb 1997 10:48:46   TSE
 * Added calling functions for the system bit error test.
 *
 *    Rev 1.3   24 Feb 1997 15:44:58   THOMPSON
 * Added a new function and new commands to perform system
 * testing. Sends EOC messages to start/stop and get results.
 *
 *    Rev 1.2   19 Dec 1996 14:28:12   THOMPSON
 * Added commands with confirmation
 *
 *    Rev 1.1   18 Dec 1996 16:06:10   THOMPSON
 * Added some types needed to support new commands.
 *
 *    Rev 1.0   05 Dec 1996 16:27:54   THOMPSON
 * Initial revision.
****************************************************************************/
/**********                end of file $Workfile: eocintf.h $          *****************/

