/****************************************************************************
* MODULE NAME : stdeoc.h                                                    *
* $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/stdeoc.h 10    7/10/01 1:06p Lauds $
*****************************************************************************
*   Copyright 1998 PairGain Technologies as unpublished work                *
*   All Rights Reserved                                                     *
*                                                                           *
*   The information contained herein is the confidential property           *
*   of PairGain Technologies.  The use, copying, transfer or                *
*   disclosure of such information is prohibited except by express          *
*   written agreement with PairGain Technologies.                           *
*                                                                           *
*   Original Written By :   Tiet Pham                                       *
*   Date                :   2/04/98                                         *
*****************************************************************************
    Detail Module Description:

    This is system firmware defines for the standard EOC modules
    Ref:  Section 'Embedded operations channel (eoc) requirements'

****************************************************************************/

#ifndef STDEOC_H
#define STDEOC_H

#include "types.h"                  // User define type
#include "falcon.h"
#include "util.h"


// the values in the EOC message address field

enum    EOC_ADDR        
{
    EOC_ADDR_ATUR=0,       
    EOC_ADDR_ATUC=3
};

// the values in the EOC mode field
enum    EOC_MODE        
{
    EOC_MODE_DATA=0,       
    EOC_MODE_OPCODE=1
};

// the values in the EOC parity field
enum    EOC_PARITY      
{
    EOC_PARITY_EVEN=0,     
    EOC_PARITY_ODD =1
};
// the values in the EOC autonomous (rt_response) field
enum    EOC_AUTO        
{
    EOC_AUTO_YES=0,        
    EOC_AUTO_NO =1
};


// the bit definitions of the latching states (stored in holdflags)
#define EOC_HOLD_REQCOR     0x01    // this unit is transmitting corrupt crc's
#define EOC_HOLD_NOTCOR     0x02    // this unit is receiving corrupt crc's

// definition of the information save for each pending EOC request made by
// higher level routines
typedef volatile struct
{
    UINT8           outbyte;    // the cmd (value in data field)--can be READx/WRITEx
                                // or some other EOC command (e.g., HOLD, RTN)
    UINT8           len;        // if the cmd is READx or WRITEx, the length to xfer
} SUBMIT_BUF_TYPE;

#ifdef EOCAOC_DISPLAY
#define MAX_EOC_REC     100
#define EOC_TX          0
#define EOC_RX          1
#endif

// Clear EOC definition
#define EOC_CLEAR_MAXMSG_SIZE   510
#define EOC_CLEAR_MAX_SIZE      (EOC_CLEAR_MAXMSG_SIZE+6)

#define EOC_SUBMIT_BUFSIZE      16      // the number of EOC cmds that can be pending
#define EOC_XMIT_BUFSIZE        (8+4*EOC_CLEAR_MAX_SIZE)        // the number of EOC messages waiting
                                                                // to be send by hardware

/***************************************************************************
 *  EOC state machine STATES
 *  Ref: T1.413 Figure 34 and Figure 35
 ***************************************************************************/
typedef enum
{
    EOC_STATE_IDLE,
    EOC_STATE_PRE_READ,
    EOC_STATE_READ_ODD,
    EOC_STATE_READ_EVEN,
    EOC_STATE_PRE_WRITE,
    EOC_STATE_WRITE_ODD,
    EOC_STATE_WRITE_EVEN
} EOC_STATE;

#define EOC_REG_REV_LEN     4
#define EOC_REG_VEN0_LEN    4           // used to test write state machine
#define EOC_REG_VEN1_LEN    4           // since only reg4/reg5 are r/w currently
#define EOC_REG_STR_LEN     4

/****************************************************************************
    Define ATU-R Data Registers.
****************************************************************************/
typedef enum {
    EOC_REG_VENID=0,
    EOC_REG_REV,                        // Revision
    EOC_REG_SN,                         // Serial Number
    EOC_REG_STR,                        // Self Testing result
    EOC_REG_VEN0,                       // Vendor discretionary
    EOC_REG_VEN1,                       // Vendor discretionary
    EOC_REG_ATN,                        // Line attenucation
    EOC_REG_SNR,
    EOC_REG_CONF,                       // ATU-R configuration
    EOC_REG_RESERVED0,
    EOC_REG_STATE,                      // Line state
    EOC_REG_RESERVED1,
    EOC_REG_RESERVED2,
    EOC_REG_RESERVED3,
    EOC_REG_RESERVED4,
    EOC_REG_RESERVED5,
    EOC_REG_NUM
}EOC_REG_TYPE;

// the following structure holds all of the variables used by the
// EOC standard protocol
typedef volatile struct
{
    EOC_STATE       eoc_state;          // the currently state within the EOC state machine
    UINT8           msgcnt;             // how many identical messages have been received in a row
    UINT8           bytecount;          // if read/write, the offset into the register --1 based
    UINT8           regnum;             // if read/write, the register number
    UINT8           holdflags;          // the latched states (CCR xmit, CCR rcv)
    EOC_TYPE        lastmsg;            // the last EOC message received
    EOC_TYPE        prev_state_msg;     // the last message that was received 3 times,
                                        // that caused transition to this state
    BOOLEAN         xmit_active;        // did we have data to send to the hardware
                                        // at the last EOC xmit interrupt, or do we
                                        // need to restart xmit (hardware xmit is empty)
    FIFO_TYPE       xmit_fifo;          // the following 2 are necessary to use
                                        // the standard FIFO routines for storing EOC
                                        // messages ready to be sent to the hardware
    EOC_TYPE        xmit_buf[EOC_XMIT_BUFSIZE];
    DYN_FIFO_TYPE   eoc_clear_recv_fifo;// The FIFO for save clear eoc message
    UINT8           eoc_clear_recv_buf[EOC_CLEAR_MAXMSG_SIZE*4];
    UINT8           eoc_clear_msg_buf[EOC_CLEAR_MAX_SIZE];
    UINT16          eoc_clear_count;    // point to next position in eoc_clear_msg_buf
    UINT16          eoc_clear_crc;
    BOOLEAN         eoc_clear_active;   // Busy for process clear eoc message
#ifdef CO
    EOC_TYPE        xmitmsg;            // the message the CO sent--usually needs to be echoed
    UINT8           timeout;            // timeout in 100ms increments waiting for
                                        // the RT to respond
    UINT8           outstanding_count;  // if following the allowed protocol of
                                        // sending msgs, ahead the count of those
                                        // outstanding (i.e., not responded to by RT)
    UINT8           rw_len;             // if read/write, the max len desired by the
                                        // issuer of the request (can be different than max in table)
    BOOLEAN         command_active;     // are we still in the midst of completing
                                        // an EOC cmd (protocol sequence), or can we
                                        // start a new request
    FIFO_TYPE       submit_fifo;        // the following 2 are necessary to use
                                        // the standard FIFO routines for high level
                                        // submitted requests
    UINT8           time_limit;         // Tick number for check time out
    SUBMIT_BUF_TYPE submit_buf[EOC_SUBMIT_BUFSIZE];
    UINT16          check_list;         // when reach show time, what need to read from RT reg
    UINT8           tpu_count;          // test parameters update count
    UINT8           rt_serial_num[32];  // For CO to store RT serial number only
    UINT8           rt_revision[EOC_REG_REV_LEN];
                                        // For CO to store RT Revision
    UINT8           rt_str[EOC_REG_STR_LEN];
                                        // For CO to store RT Self test result
    UINT8           rt_ven0[EOC_REG_VEN0_LEN];
                                        // For CO to store RT Vendor discretionary
    UINT8           rt_ven1[EOC_REG_VEN1_LEN];
                                        // For CO to store RT Vendor discretionary
#else
    UINT8           timeout;            // Fake for RT anyway
    UINT8           time_limit;
#endif
#ifdef EOCAOC_DISPLAY
    // Use in Dumb Terminal to debug EOC channel
    UINT8           rec_buf[MAX_EOC_REC];       // data tx or rx
    UINT8           rec_fcount[MAX_EOC_REC];    // Super frame counter
    BOOLEAN         rec_rwflag[MAX_EOC_REC];    // tx or rx?
    UINT8           rec_next;           // next position
    UINT16          dgasp_count;
    UINT16          wrongcode_count;    // count for wrong address or wrong opcode
    UINT16          unexpect_count;     // count for answer without ask or no outstanding message
    UINT16          timeout_count;      // time out count
    UINT16          utc_count;          // UTC count
#endif
} STDEOC_DB;

typedef enum
{
    GSPN_EOC_INACTIVE,
    GSPN_EOC_ACTIVE,
    GSPN_EOC_PROCESSING,
    GSPN_EOC_WAIT_RESPONSE,
    GSPN_EOC_FLOODING,
}GSPN_EOC_STATE_TYPE;

#define MAX_GSPN_RECORD_LIMIT 20    // 20 is more than sufficient
typedef struct
{
    GSPN_EOC_STATE_TYPE state;    
    EOC_TYPE xmit_record[MAX_GSPN_RECORD_LIMIT];
    GEN_UINT xmit_size;
}GSPN_EOC_CTL_TYPE;

extern const UINT8 EOC_register_maxlen_init[EOC_REG_NUM];

typedef struct
{
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
}LOCAL_STDEOC_TYPE;


/*****                     Global functions                           ******/

DPCCALL void    STDEOC_Init(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    STDEOC_TxProcessing(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    STDEOC_RxProcessing(PDATAPUMP_CONTEXT pDatapump, EOC_TYPE rcvmsg);
DPCCALL BOOLEAN STDEOC_clear_sendmsg(PDATAPUMP_CONTEXT pDatapump, UINT8 *, UINT16);
DPCCALL void    STDEOC_EnableClearEocChannel(PDATAPUMP_CONTEXT pDatapump, BOOLEAN enable);

#ifdef CO
void    STDEOC_CheckTimeout(PDATAPUMP_CONTEXT pDatapump);

    // the following are temporary routines--until interface to rest of
    // system can be decided upon
void    STDEOC_SubmitCmd(PDATAPUMP_CONTEXT pDatapump, UINT8 index);
BOOLEAN    STDEOC_ReadReg(PDATAPUMP_CONTEXT pDatapump, UINT8 reg);
BOOLEAN STDEOC_Reqcor(PDATAPUMP_CONTEXT pDatapump);
BOOLEAN STDEOC_Reqend(PDATAPUMP_CONTEXT pDatapump);
BOOLEAN STDEOC_Notcor(PDATAPUMP_CONTEXT pDatapump);
BOOLEAN STDEOC_Notend(PDATAPUMP_CONTEXT pDatapump);
void STDEOC_ReqTpu(PDATAPUMP_CONTEXT pDatapump);
BOOLEAN STDEOC_SuspendPolling(PDATAPUMP_CONTEXT pDatapump, BOOLEAN state);    // True will suspend polling and False will reenable polling
BOOLEAN STDEOC_SetHoldState(PDATAPUMP_CONTEXT pDatapump);   
BOOLEAN STDEOC_SendSelfTest(PDATAPUMP_CONTEXT pDatapump);
BOOLEAN STDEOC_ReturnToNormal(PDATAPUMP_CONTEXT pDatapump);   // Return all active conditions to normal
BOOLEAN STDEOC_RequestCorruptCrcStart(PDATAPUMP_CONTEXT pDatapump);
GEN_UINT STDEOC_RequestCorruptCrcStop(PDATAPUMP_CONTEXT pDatapump);     // return number of corrupted CRC received since STDEOC_RequestCorruptCrcStart
BOOLEAN STDEOC_NotifyCorruptCrcStart(PDATAPUMP_CONTEXT pDatapump);      // start transmitting corrupted CRC
BOOLEAN STDEOC_NotifyCorruptCrcStop(PDATAPUMP_CONTEXT pDatapump);   // stop transmitting corrupted CRC
BOOLEAN STDEOC_ReadReg(PDATAPUMP_CONTEXT pDatapump, UINT8 reg);
BOOLEAN STDEOC_WriteReg(PDATAPUMP_CONTEXT pDatapump, UINT8 reg, const UINT8 * data);
BOOLEAN STDEOC_PollReadReg(PDATAPUMP_CONTEXT pDatapump, UINT8 reg, UINT8 size, UINT8 *data);
GEN_UINT STDEOC_PollDyingGasp(PDATAPUMP_CONTEXT pDatapump);   // poll how many Dying gasp has received since Showtime
#endif
DPCCALL void STDEOC_DyingGasp(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    STDEOC_GlobespanEocNotify(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void STDEOC_GlobespanEocProcessing(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void STDEOC_RxEocIntrHandler(PDATAPUMP_CONTEXT pDatapump);

#endif                                  // STDEOC_H

/****************************************************************************
    $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/stdeoc.h $
 * 
 * 10    7/10/01 1:06p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 * 
 * 9     6/27/01 2:08p Lauds
 * multi-instance version for CO
 * 
 * 8     6/26/01 8:36p Lauds
 * multi-instance version integration
 * 
 * 3     6/25/01 10:59a Lauds
 * UNH support
 * 
 * 2     6/22/01 3:14p Lauds
 * minor cleanup and bug fix in miscutil.c and terminal.c
 * 
 * 1     6/21/01 3:11p Lauds
 * Multi-instance initial version
 * 
 * 7     5/23/01 6:26p Lauds
 * move some enum and constants from stdeoc.c to stdeoc.h to fix CO
 * specific syntax error
 * 
 * 6     5/15/01 10:43p Lauds
 * move some defines from stdeoc.h to stdeoc.c
 * 
 * 5     9/29/00 7:23p Lauds
 * implement dying gasp for testing purpose
 * 
 * 4     2/28/00 10:48a Lewisrc
 * 2.03 Pairgain release for San Jose Code Interop
 * 
 *    Rev 1.7   Feb 09 2000 11:42:54   yangl
 * -- Huntsville interop merge
 *
 *    Rev 1.6   Jan 26 2000 13:05:10   yangl
 * -- Fix link error for c163
 *
 *    Rev 1.5   Jan 26 2000 11:48:22   yangl
 * -- Add STDEOC API
 * -- Support clear EOC
 * -- Add misc cfg menu
 *
 *    Rev 1.4   Dec 03 1999 12:28:30   nordees
 * Fix compilier bug for c166.
 *
 *    Rev 1.3   Dec 03 1999 11:12:38   yangl
 * -- Improve AOC channel
 * -- Add AOC menu
 * -- Add EOC menu for RT
 *
 *    Rev 1.2   Dec 02 1999 09:23:12   yangl
 * -- Fix compile error for DSLAM
 *
 *    Rev 1.1   Nov 24 1999 18:51:14   infusij
 * - EOC channel improve.
 *
 *    Rev 1.0   24 Mar 1998 09:45:56   phamt
 *
*
****************************************************************************/
/**********             end of file stdeoc.h                ****************/
