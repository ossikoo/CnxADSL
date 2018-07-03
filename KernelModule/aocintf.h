/************************************************************************
*  $Workfile: aocintf.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/aocintf.h 4     7/10/01 12:54p Lauds $
*
*************************************************************************
*       Copyright 1999 PairGain Technologies as unpublished work        *
*       All Rights Reserved                                             *
*                                                                       *
*       The information contained herein is the confidential property   *
*       of PairGain Technologies.  The use, copying, transfer or        *
*       disclosure of such information is prohibited except by express  *
*       written agreement with PairGain Technologies.                   *
*                                                                       *
*************************************************************************
    Description: This module provides header defination for communicating between ATUs
    over the adsl overhead channel (AOC).

*************************************************************************/

#ifndef AOCINTF_H
#define AOCINTF_H
/****************************** INCLUDE FILES ********************************/
#include "types.h"                  // User define type
#include "util.h"

/*************************** TYPES & DEFINITIONS *****************************/
#define AOC_MAX_TX_BUF             3

enum AOC_RX_STATE
{
    WAITING_HEADER,
    WAITING_ALL_1s,
    WAITING_DATA
};

typedef struct
{
    GEN_UINT            count;           // keep track of # of this msg has been rcvd
    GEN_UINT            header;          // this header can contain any aoc msg
    AOC_MSG_TYPE        msg;
}AOC_BUF_TYPE;


#ifdef EOCAOC_DISPLAY
#define MAX_AOC_REC_PAGE    4
#define MAX_AOC_REC     100*MAX_AOC_REC_PAGE    // display 100 by 4 pages
#define AOC_TX          0
#define AOC_RX          1
#endif

typedef struct AOC_DB_TYPE
{
    // since we need to receive 3 out of 5 msg to act on a request.  We need
    // 3 buffers to store
    AOC_BUF_TYPE   rx_buffer[3];
    AOC_BUF_TYPE   temp_buffer;
    UINT8          num_idle_rcvd, num_unable_comply_rcvd;
    UINT8          rx_state, rx_index, rx_size;    // index and size of data bytes
    AOC_BUF_TYPE   xmit_req_buffer[AOC_MAX_TX_BUF];
    FIFO_TYPE      xmit_reqq;
#ifdef EOCAOC_DISPLAY
    // Use in Dumb Terminal to debug AOC channel
    UINT8          rec_buf[MAX_AOC_REC];           // data tx or rx
    UINT8          rec_fcount[MAX_AOC_REC];        // Super frame counter
    BOOLEAN        rec_rwflag[MAX_AOC_REC];        // tx or rx?
    UINT16         rec_next;                       // next position
#endif
}AOC_DB_TYPE;

typedef struct
{
    AOC_DB_TYPE aoc_database;
    volatile LOCK_TYPE xmit_lock;    // reset semaphore
    volatile FLAG_TYPE pairgain_eoc_protocol;
    #if AOC_RX_BUFFER
    #define RX_BUFFER_SIZE 500
    UINT16 aoc_rx_index;
//    UINT8 aoc_rx_buffer [RX_BUFFER_SIZE] ;
    #endif
}LOCAL_AOCINTF_TYPE;


DPCCALL void AOC_InitXmitReqQ(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void AOC_TxAocIntrHandler(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void AOC_RxAocIntrHandler(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void AOC_XmitMsg(PDATAPUMP_CONTEXT pDatapump, GEN_UINT, const AOC_MSG_TYPE *);
DPCCALL void AOC_SendProfileMgmtReq(PDATAPUMP_CONTEXT pDatapump, UINT8);
DPCCALL void AOC_Bitswap_Enable(PDATAPUMP_CONTEXT pDatapump, BOOLEAN enable);

#endif                                  // AOCINTF_H

/*****************************************************************************
* $Log: /Projects/Tigris and Derivatives/Reference Designs/Tigris/DMT DP Controller Driver/aocintf.h $
 * 
 * 4     7/10/01 12:54p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 * 
 * 3     6/26/01 7:38p Lauds
 * multi-instance version integration
 * 
 * 2     6/22/01 3:06p Lauds
 * minor cleanup and bug fix in miscutil.c and terminal.c
 * 
 * 1     6/21/01 2:55p Lauds
 * Multi-instance initial version
 * 
 * 2     9/27/00 7:44p Lauds
 * more cleanup, reduce warning messages
 * 
 * 1     12/07/99 10:21a Lewisrc
 * Administrative Operations Channel interface header file.
 * Introduced in 2.02g Pairgain release.
 * 
 *    Rev 1.0   Dec 03 1999 11:32:52   nordees
 * Initial version.
*****************************************************************************/
