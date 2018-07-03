/****************************************************************************
*  $Workfile: dpu.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/dpu.h 5     6/26/01 7:53p Lauds $
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
*  Original Written By: Chi Tse                                             *
*  Original Date: 1/29/97                                                   *
*                                                                           *
*****************************************************************************
   Description:
    This is display processing unit (dpu). It contains typedefs and
   defines as well as data structure for the dumb terminal screens.

****************************************************************************/
#ifndef DPU_H
#define DPU_H

/****************************************************************************
                              INCLUDE SECTION
****************************************************************************/
#include "types.h"      // short form typedef define
#include "ansi.h"       // escape sequence definition
#include "dpuid.h"      // all screens id and dpu messages id

/****************************************************************************
                                  DEFINE
****************************************************************************/
/* special character define */
#define TABCHR         '\t'
#define BACKSP         '\x08'
#define LFCHR          '\x0A'
#define CRCHR          '\x0D'
#define ESCCHR         '\x1B'
#define SP_BAR         '\x20'
#define NEXT           'N'
#define NULLSTR        "\x00"
#define CLR_CHR        ' '
#define CTRL_A         '\x01'    // undocumented feature:
                                 // pressing CTRL-A followed immediately
                                 // by an A key again, will log you
                                 // into special account
#define CTRL_R         '\x12'    // refreshes screen
#define CTRL_D         '\x04'    // disconnects remote session
#define CTRL_T         '\x14'    // toggles between remote/local screen
#define CTRL_X         '\x18'    // exit from dumb terminal
#define PWCHAR           '*'     // password display character

// display message flag
enum
{
   DPU_DISP_NORMAL,
   DPU_DISP_HIGHLIGHT
};

/****************************************************************************
                             MACRO DEFINITION
****************************************************************************/
#define GOTOXY(X,Y)      CUP(X,Y)        // ESC sequence to move the cursor
#define CLRSCREEN        ED_ALL          // ESC sequence to clear the screen
#define CLR_MSG          GOTOXY("1","23") EL_ACTIVE_TO_END // clear message
#define CLR_FOOTER       GOTOXY("1","22") EL_ACTIVE_TO_END // clear footer
#define CLR_USER_INPUT   GOTOXY("16","24")EL_ACTIVE_TO_END // clear input prompt
#define ENTER_CHOICE_X   16              // X position of the user prompt

/****************************************************************************
           ESC SEQUENCE OF SCREEN CURSOR POSITION DEFINITION
****************************************************************************/
// any position on the screen can be defined by the escape sequence of
// the following:
//    CSI Y ; X H
//
//    CSI - includes these characters (\x1b [)
//    Y   - two digits of row number
//    ;   - separator
//    X   - two digits of col number
//    H   - control character for cursor postion

#define DPU_CUR_POS_SIZE   sizeof(CSI"YY;XXH")
typedef char DPU_CUR_POS[DPU_CUR_POS_SIZE];

/****************************************************************************
                  SCREEN MODE DEFINITION
****************************************************************************/
// configuration mode define
enum
{
   SELECT_MODE,
   SELECT_MODE2,    // second layer of the select mode
   ENTRY_MODE,
   ENTRY_MODE2,     // second layer of entry mode
   CONFIRM_MODE
};

typedef struct
{
   UINT8 confirm;        // confirm message indicator
   UINT8 spec_pw;        // 1: special password was entered
   UINT8 wait_for_data;  // 1: waiting for data
   UINT8 cfg_mode;       // 0: select mode
                         // 1: entry mode
                         // 2: entry mode2
                         // 3: confirm mode
   UINT8 quiet;          // 1: disables all auto-updates
   UINT8 co_access_only; // 1: enable co_access_only
   UINT8 bypass_pw;      // 1: using bypass password to login
   UINT8 num_tries_pw;   // number of tries changing password so far
}DPU_SCR_MODE;

#define   DPU_SCR_MODE_SIZE   sizeof(DPU_SCR_MODE)

/****************************************************************************
                  STORAGE OF USER'S INPUT DEFINITION
****************************************************************************/
#define DPU_IP_BUF_SIZE    65    // Maximun size of the input buffer

typedef struct
{
   char buffer[DPU_IP_BUF_SIZE]; // buffer
   UINT8 index;                  // point to the nex available space
}DPU_USER_BUF;

#define   DPU_USER_BUF_SIZE   sizeof(DPU_USER_BUF)

/****************************************************************************
                  CONFIGURATION CHANGE FIELD DEFINITION
****************************************************************************/
// Since all the changes on the configuration menus won't take an
// effect until the user confirmed, therefore the DPU needs a RAM
// space to store a temporary copy of the system configuration for
// the user to modify. The DPU_CFG_CHANGE_SIZE structure is the dirty
// bit of this configuration block.
typedef struct
{
   UINT16 cfg_field1  : 1;
   UINT16 cfg_field2  : 1;
   UINT16 cfg_field3  : 1;
   UINT16 cfg_field4  : 1;
   UINT16 cfg_field5  : 1;
   UINT16 cfg_field6  : 1;
   UINT16 cfg_field7  : 1;
   UINT16 cfg_field8  : 1;
   UINT16 cfg_field9  : 1;
   UINT16 cfg_field10 : 1;
   UINT16 cfg_field11 : 1;
   UINT16 cfg_field12 : 1;
   UINT16 cfg_field13 : 1;
   UINT16 cfg_field14 : 1;
   UINT16 cfg_field15 : 1;
   UINT16 cfg_field16 : 1;
}DPU_CFG_CHANGE_TYPE;

#define DPU_CFG_CHANGE_SIZE   sizeof(DPU_CFG_CHANGE_TYPE)
#define DEFAULT_CHANGE_FIELD  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}


/****************************************************************************
                            DPU WORKAREA DEFINITION
****************************************************************************/
#define OUT_BUF_SIZE       256  // Maximun size of the output buffer

#define TEMP_BUF_SIZE       200

// Work area definition, the DPU requires this work area from each
// of the interface source. (note the structure order of the fields
// panel_id and tx_routine should not changed since terminal.c
// is initializing such data type in that particular order
typedef struct
{
    PDATAPUMP_CONTEXT pDatapump;            // current xcvr channel
    UINT8 panel_id;                      // current panel ID
    void (*tx_routine)(PDATAPUMP_CONTEXT pDatapump, const char *, unsigned int);  // pointer to the transmit function
    DPU_SCR_MODE scr_mode;               // current mode of the current screen
    UINT8 cur_option;                    // the last option was selected by user
    UINT8 option_index;                  // current index of the selected option
    char output_buf[OUT_BUF_SIZE];       // screen storage space
    DPU_USER_BUF storage;                // storage for user inputs (ex. buffering password)
    union
    {
        UINT8  uint8[TEMP_BUF_SIZE];     // temp_buf field is used to stored
        UINT16 uint16[1];                // any temporary data associated
        UINT32 uint32[1];                // to the corresponding screen
        SINT8  sint8[1];                 // e.g. it is used to buffer the
        SINT16 sint16[1];                // configuration data in ebm, system
        SINT32 sint32[1];                // configuration
        unsigned int uint[1];            // uint and sint is added to provided
        int          sint[1];            // for types dependent routines (e.g. sscanf)
    }temp_buf;
}DPU_SCR_INFO;


#define   DPU_SCR_INFO_SIZE   sizeof(DPU_SCR_INFO)

/****************************************************************************
                            SCREEN DEFINITION
****************************************************************************/
typedef struct
{
                    /*****       standard menu       *****/
   char *gen_stat_str;              /* general menu static string        */
   DPU_CUR_POS *gen_no_ud_lst;      /* no update variables position list */
   UINT8 gen_no_ud_size;            /* size of the list above            */
   DPU_CUR_POS *gen_w_ud_lst;       /* update variables position list    */
   UINT8 gen_w_ud_size;             /* size of the list above            */

                    /*****      additional menu      *****/
   char *add_stat_str;              /* additional menu static string     */
   DPU_CUR_POS *add_no_ud_lst;      /* no update variables position list */
   UINT8 add_no_ud_size;            /* size of the list above            */
   DPU_CUR_POS *add_w_ud_lst;       /* update variables position list    */
   UINT8 add_w_ud_size;             /* size of the list above            */

                    /***** three auxiliary functions *****/
   void (*InputRespRoutine)(DPU_SCR_INFO *, UINT8);
   void (*DisplayRoutine)(DPU_SCR_INFO *);
   void (*PeriodicRefRoutine)(DPU_SCR_INFO *);
}DPU_DISP_PANEL ;

extern const DPU_DISP_PANEL  * const panel[NUM_OF_SCR];
// dpu.c panels
extern const DPU_DISP_PANEL DPU_logon_menu;
extern const DPU_DISP_PANEL DPU_main_menu;

// dpusys.c panels
extern const DPU_DISP_PANEL DPUS_sys_main_menu;
extern const DPU_DISP_PANEL DPUS_sys_info_menu;
extern const DPU_DISP_PANEL DPUS_sys_cfg_menu;
extern const DPU_DISP_PANEL DPUS_sys_misccfg_menu;
extern const DPU_DISP_PANEL DPUS_sys_misccfg_sys_menu;
extern const DPU_DISP_PANEL DPUS_sys_misccfg_other_menu;

// dpuadsl.c panels
extern const DPU_DISP_PANEL DPUA_adsl_performance_status_menu;
extern const DPU_DISP_PANEL DPUA_adsl_24hr_history_menu;
extern const DPU_DISP_PANEL DPUA_adsl_7day_history_menu;
extern const DPU_DISP_PANEL DPUA_adsl_alarm_history_menu;


// dpumisc.c panels
extern const DPU_DISP_PANEL DPUM_remote_logon_menu;
extern const DPU_DISP_PANEL DPUM_diagnostic_log_menu;
extern const DPU_DISP_PANEL DPUM_trace_cfg_menu;
extern const DPU_DISP_PANEL DPUM_trace_log_menu;
extern const DPU_DISP_PANEL DPUM_peek_poke_mem_menu;
extern const DPU_DISP_PANEL DPUM_peek_poke_nvram_menu;

// dputest.c panels
extern const DPU_DISP_PANEL DPUT_external_bert_setup_menu;
extern const DPU_DISP_PANEL DPUT_production_support_menu;
extern const DPU_DISP_PANEL DPUT_sys_test_menu;

// dpudmt.c panels
extern const DPU_DISP_PANEL DPUD_adsl_main_menu;
extern const DPU_DISP_PANEL DPUD_adsl_xcvr_main_menu;
extern const DPU_DISP_PANEL DPUD_adsl_falcon_status_menu ;
extern const DPU_DISP_PANEL DPUD_adsl_bit_allocation_menu ;
extern const DPU_DISP_PANEL DPUD_adsl_snr_menu ;
extern const DPU_DISP_PANEL DPUD_adsl_xcvr_cfg_menu ;
extern const DPU_DISP_PANEL DPUD_adsl_xcvr_test_menu ;
extern const DPU_DISP_PANEL DPUD_adsl_xcvr_tx_test_menu ;
extern const DPU_DISP_PANEL DPUD_adsl_xcvr_rx_test_menu ;
extern const DPU_DISP_PANEL DPUD_adsl_xcvr_afe_test_menu ;
extern const DPU_DISP_PANEL DPUD_adsl_falcon_view_buf_menu ;
extern const DPU_DISP_PANEL DPUD_peek_poke_main_menu ;
extern const DPU_DISP_PANEL DPUD_peek_poke_falcon_menu;

#ifdef GHS_ENABLE
extern const DPU_DISP_PANEL DPUD_fr_menu;
#ifdef RT
extern const DPU_DISP_PANEL DPUD_fr_probe_menu;
#endif
extern const DPU_DISP_PANEL DPUD_ghs_phi_menu;
extern const DPU_DISP_PANEL DPUD_ghs_pro_menu;
extern const DPU_DISP_PANEL DPUD_ghs_9921a_menu;
extern const DPU_DISP_PANEL DPUD_ghs_9922ab_menu;
extern const DPU_DISP_PANEL DPUD_ghs_common_menu;
extern const DPU_DISP_PANEL DPUD_adsl_power_menu;
#endif
#ifdef EOCAOC_DISPLAY
extern const DPU_DISP_PANEL DPUD_adsl_eocaoc_menu;
extern const DPU_DISP_PANEL DPUD_adsl_eoc_menu;
extern const DPU_DISP_PANEL DPUD_adsl_aoc_menu;
#endif

// dpuebm.c panels
#ifdef TOMCAT
extern const DPU_DISP_PANEL DPUE_bridge_main_menu;
extern const DPU_DISP_PANEL DPUE_peek_poke_rcb_menu;
extern const DPU_DISP_PANEL DPUE_rcb_error_cnt_menu;
extern const DPU_DISP_PANEL DPUE_rcb_wan_port_ctrl_menu;
extern const DPU_DISP_PANEL DPUE_rcb_srdb_menu;
extern const DPU_DISP_PANEL DPUE_rcb_lktbl_menu;
extern const DPU_DISP_PANEL DPUE_rcb_frmbuf_menu;
#endif

#define   DPU_DISP_PANEL_SIZE   sizeof(DPU_DISP_PANEL)
extern const char * const DPU_video_str[2];

/****************************************************************************
                            PEEK/POKE DEFINITION
****************************************************************************/
typedef struct
{
   UINT16 (*ReadRoutine)(PDATAPUMP_CONTEXT pDatapump, UINT32 address);
   void (*WriteRoutine)(PDATAPUMP_CONTEXT pDatapump, UINT32 address, UINT16 data);
} DPU_PEEK_POKE_TYPE;

#ifdef VCOMM

/*****          GLOBAL FUNCTION PROTOTYPE SECTION              ***************/
// utilities functions

DPCCALL void DPU_ScreenDisp(DPU_SCR_INFO *);        // invoke by application to display a new screen
DPCCALL void DPU_PeriodicRef(DPU_SCR_INFO *);       // invoke by one second timer interrupt
DPCCALL void DPU_InputResp(DPU_SCR_INFO *, UINT8);  // invoke by serial/eoc interrupt to
                                            // process user input.
#endif
#endif
/***************************************************************************
 * $Log: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/dpu.h $
 * 
 * 5     6/26/01 7:53p Lauds
 * multi-instance version integration
 * 
 * 2     6/22/01 3:09p Lauds
 * minor cleanup and bug fix in miscutil.c and terminal.c
 * 
 * 4     12/01/00 3:47p Lauds
 * XCVR_INDEX_TYPE is UINT8.  This causes inefficiency in both speed and
 * space  in Euphrates and many 32-bit processor. Some mismatch type
 * caused by this fix.  Mismatch type for xcvr_num is change from UINT8 to
 * XCVR_INDEX_TYPE
 * 
 * 3     8/05/99 4:22p Lewisrc
 * Ported as is from 2.01g' snapshot
 * 
 *    Rev 1.38   May 28 1999 19:06:36   laud
 * allow toggling between TAO menu and dumb terminal
 * 
 *    Rev 1.37   Apr 07 1999 18:22:16   laud
 * Get rid of SourceCom EBM code
 *
 *    Rev 1.36   Jun 30 1998 14:26:14   laud
 * allow sscanf to work in MIPS
 *
 *    Rev 1.35   Jun 24 1998 16:55:48   laud
 *
 *
 *    Rev 1.34   Jun 23 1998 18:45:50   laud
 * add xcvr_num in work area
 *
 *    Rev 1.33   09 Jan 1998 19:31:12   laud
 * fix bug on peek and poke functions; get rid of near
 *
 *    Rev 1.32   27 Oct 1997 15:52:54   phamt
 * Port to DSLAM design
 *
 *    Rev 1.31   17 Jul 1997 15:47:40   tse
 * added select mode2 for production support.
 *
 *    Rev 1.30   22 May 1997 16:07:22   TSE
 *
 * define enter choice x position.
 *
 *    Rev 1.29   21 Mar 1997 18:38:36   THOMPSON
 *
 * Don't allow system password change. Have user password
 * default be no password. Allow no password for user. Fix
 * bug allowing password of length 9 to be entered.
 *
 *    Rev 1.28   17 Mar 1997 09:05:44   THOMPSON
 * Added CTRL_A definition.
 *
 *    Rev 1.27   12 Mar 1997 11:27:56   TSE
 * bug fix : allow auto_refresh during remote logon if the screen mode
 * is waiting for bridge data. Rename the refresh field in the screen
 * mode type to wait_for_data.
 *
 *    Rev 1.26   07 Mar 1997 15:17:28   THOMPSON
 *
 * Added the "Bypass" password functionality for customers
 * that don't have their passwords available.
 *
 *    Rev 1.25   06 Mar 1997 15:01:34   DLAU
 * add near for DPU_PEEK_POKE_TYPE since they are for local functions
 *
 *    Rev 1.24   06 Mar 1997 09:17:50   TSE
 *
 * added a new mode to the dpu which allows only the ATU-C
 * to change the system configuration.
 *
 *    Rev 1.23   28 Feb 1997 10:22:22   DLAU
 * major cleanup on the workarea structure
 *
 *    Rev 1.22   27 Feb 1997 16:19:44   THOMPSON
 * check in for Dave
 *
 *    Rev 1.21   26 Feb 1997 15:04:54   THOMPSON
 *
 * Added a field to the DPU data structure to disable
 * on auto-updates on screen data.
 *
 *    Rev 1.20   25 Feb 1997 19:39:30   DLAU
 * reduce the buffer size from 2000 to 512 to conserve RAM
 *
 *    Rev 1.19   13 Feb 1997 11:53:42   TSE
 * set DPU_MAX_CFG_SIZE to SYS_SNMP_CFG_SIZE.
 *
 *    Rev 1.18   05 Feb 1997 11:28:10   TSE
 * add DPU_PEEK_POKE_TYPE
 *
 *    Rev 1.17   03 Feb 1997 11:33:02   TSE
 *
 * split dpu.h into dpu.h and dpuid.h
 * dpu.h is common to both CAP and DMT system
 * dpuid.h is not.
****************************************************************************/
/***** end of file $Workfile: dpu.h $ *****/
