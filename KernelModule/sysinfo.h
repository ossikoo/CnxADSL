/****************************************************************************
*  $Workfile: sysinfo.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/sysinfo.h 21    6/26/01 8:39p Lauds $
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
*  Original Written By:  David Lau & Chi Tse                                *
*  Original Date:   8/13/96                                                 *
*                                                                           *
*****************************************************************************
    Description:

    This is System Information(sysinfo) data. It contains typedefs and
   defines as well as data structure for all statistical, errored,
   alarm information. Each data structure shall have its own
    define to define the structure size using sizeof().
****************************************************************************/
#ifndef SYSINFO_H
#define SYSINFO_H


/****************************************************************************
                              INCLUDE SECTION
****************************************************************************/
#include "types.h"
#include "xcvrinfo.h"
#ifdef GHS_ENABLE
#include "ghsutil.h"
#endif
#ifdef TOMCAT
#include "rcbapp.h"
#endif

/****************************************************************************
                              Enumerators for restart status
****************************************************************************/

enum { NO_RESTART=0, WARM_RESTART, COLD_RESTART };

/****************************************************************************
                      SMART TERMINAL DEFINITION
****************************************************************************/



#define SYS_SMRT_MSG_LEN         300  // max length= SYNC+RESV+LENG+CMD+DATA+CHKSUM
                                      // with F1 and F0 having been explained as F0 01 and F0 00.

                              // Note: 'length' is defined as 1 byte. However
                              // if the input byte is F0 or F1, they should
                              // be replaced by F0 00, F0 01. We consider
                              // some of replacement in a 256 bytes data stream*/

// paramteres used for Response Cmd status byte
typedef enum SYS_STATUS_BYTE_TYPE
{
    SYS_CMD_SUCCESS,        // 'set' cmd successfully done
    SYS_PARM_INVALID,       // parameters invalid
    SYS_COMM_ERR,           // communication error(e.g. chksum error).
    SYS_RUN_ERR,            // communication error(e.g. chksum error).
    SYS_DATA_NOT_COMPLETED, // for TAO Mode, data not completed
    SYS_NOT_IMPLEMENTED     // for extension: not implemented command
} SYS_STATUS_BYTE_TYPE;

typedef struct
{
    BOOLEAN volatile    send_status;              // =FALSE;
    TICK_TYPE           send_time;
    XCVR_CFG_TYPE       xcvr_cfg;
    XCVR_ALM_CFG_TYPE   xcvr_alm_cfg;
} EOC_SEND_INFO_TYPE;

#define SYS_SMRT_FRAMING_CHAR   0xF1   // framing char for smart terminal

#define SYS_SMRT_LU_SYNC_BYTE   0x81   // SYNC byte from smart terminal to LU

// offset in a message passing to high level for msg processing
enum { CMD_IDX, DATA_IDX};

// definition of function pointer to smart terminal cmmmand
typedef void (*CMD_FUNC)(PDATAPUMP_CONTEXT pDatapump, UINT8 * msg, GEN_UINT msg_len);

// poll flag operation: clear, set.
typedef enum
{
    CLEAR_POLL_FLAG,
    SET_POLL_FLAG
}SYS_POLL_FLAG_VALUE;

typedef enum
{
    TIME_DATE,                 // sys info group
    CIRCUIT_ID,
    UNIT_ID,
    CIRCUIT_MAP,
    TEST_RESULT_AVAILABLE,
    TIME_SLOT_15MIN_CROSSED,
    TIME_SLOT_24HR_CROSSED,
    LU_BRIDGE_ROUTER_COM_CFG,  // lu bridge/router cfg group
    LU_BRIDGE_CFG,
    LU_ROUTER_CFG,
    LU_SNMP_CFG,
    RT_BRIDGE_ROUTER_COM_CFG,  // rt bridge/router cfg group
    RT_BRIDGE_CFG,
    RT_ROUTER_CFG,
    RT_SNMP_CFG,
    LU_DATA_PORT_STATISTICS,  // lu bridge/router status group
    LU_DATA_PORT_STATUS,
    LU_DATA_PORT_ALARM_CFG,
    LU_DATA_PORT_ALARM_STATUS,
    RT_DATA_PORT_STATISTICS,  // rt bridge/router status group
    RT_DATA_PORT_STATUS,
    RT_DATA_PORT_ALARM_CFG,
    RT_DATA_PORT_ALARM_STATUS,
    XCVR_CFG,                 // xcvr cfg group
    XCVR_STATUS,
    ERROR_COUNT,
    SPAN_ALARM_STATUS,
    SPAN_ALARM_CFG
}POLL_FLAG;



#ifdef LITTLE_ENDIAN_MEMORY

// This structure is defined based on smart termminal spec
typedef struct
{
    UINT16 lof          :1;
    UINT16 margin       :1;
    UINT16 es           :1;
    UINT16 ses          :1;
    UINT16 los          :1;   // ---only DMT support it
    UINT16 di           :1;   // ---only DMT support it
    UINT16 excess_lof   :1;
    UINT16 excess_ses   :1;
    UINT16 excess_uas   :1;
    UINT16 excess_los   :1;   // ---only DMT support it
    UINT16 init_fail    :1;   // ---only DMT support it
    UINT16              :5;
}ALARM_STATUS;

#else

// This structure is defined based on smart termminal spec
typedef struct
{
    UINT16              :5;
    UINT16 init_fail    :1;   // ---only DMT support it
    UINT16 excess_los   :1;   // ---only DMT support it
    UINT16 excess_uas   :1;
    UINT16 excess_ses   :1;
    UINT16 excess_lof   :1;
    UINT16 di           :1;   // ---only DMT support it
    UINT16 los          :1;   // ---only DMT support it
    UINT16 ses          :1;
    UINT16 es           :1;
    UINT16 margin       :1;
    UINT16 lof          :1;
}ALARM_STATUS;

#endif




// This structure is used as interface to DSLAM or NMA
typedef struct
{
    ALARM_STATUS NearEnd;
    ALARM_STATUS FarEnd;
}SYS_ALARM_STATUS_TYPE;



/****************************************************************************
                      FLASH DOWNLOAD HEADER DEFINITION
****************************************************************************/
#define DWLD_HEADER_INFO  (*(volatile DWLD_HEADER_TYPE far *)(GUARANTEE_FLASH_CODE + RAM_START_ADDR + FLASH_DWLD_CODE_SIZE))
/* note that the fields of this structure has to be carefully arranged such
   that no holes are inserted (e.g. C16X inserts holes to enforce all integer
   fields to be word aligned) */
typedef struct
{
   UINT8 logo[10];         // 'PaIrGaIn'
   UINT8 fw_type;          // currently not used, can be used to distinguish data rate and ATU type
   UINT8 hw_type;          // 'A' for ADSL
   UINT8 hdr_type;         // hardware type, currently not used.
   UINT8 protect_mode;     // bit 0: Normal Code
                           // bit 1: Boot up code
                           // bit 2: Download logic code
                           // note : if the protect_mode = 7, the software will
                           //        download the whole program which includes
                           //        the application code, boot up code and
                           //        the download code
   UINT16 num_rec;         // Number of record to be downloaded
   UINT8 date[10];         // date
   UINT8 time[10];         // time
   UINT8  vers[6];         // Firmware Version message A.BB
   UINT16 chksum;          // checksum of entire rom (only app and dwld chksum total)
                           // not currently used for code validation
   UINT8  start_baud_ind;   // 0(9600), 1(19.2), 2(38.4), 3(56), 4(115.2)
   UINT8  end_baud_ind;    // 0(9600), 1(19.2), 2(38.4), 3(56), 4(115.2)

                           // WARNING: There is code in cstart.asm that
                           // depends on the location of the following data
                           // moving them will require changing cstart.asm

                           // As well as, bdnl.c and bmerge.c (and thus
                           // dnl.h) utilities depend on this structure.

   UINT16 appchksum;       // checksum of app area only
   UINT16 dwldchksum;      // checksum of download area only
   UINT8 reserv[14];
}DWLD_HEADER_TYPE;

/****************************************************************************
                      TERMINAL INTERFACE DEFINITION
****************************************************************************/
typedef enum LOGON_MODE
{
   LOCAL_LOGON,
   REMOTE_LOGON,
   NUM_LOGON_MODE
} SYS_LOGON_MODE_TYPE;

#define SYS_LOGON_MODE_SIZE (sizeof(SYS_LOGON_MODE_TYPE))

typedef enum SERIAL_MODE
{
    DUMB_TERMINAL,
    SMART_TERMINAL,
    NETWORK_MANAGEMENT,
    NUM_SERIAL_MODE,
    INVALID_MODE
} SYS_SERIAL_MODE_TYPE;

#define SYS_SERIAL_MODE_SIZE  (sizeof(SYS_SERIAL_MODE_TYPE))

/****************************************************************************
                      PERIODIC TASK  DEFINITION
****************************************************************************/
typedef struct
{
   UINT16 period;                 // period for this periodic task
   const JOB_TYPE *job_list;            // list of jobs perform within each period
   UINT8 num_of_job;              // number of jobs in the list above
}PERIODIC_TASK_TYPE;

typedef struct
{
   UINT16 tick_cnt;    // tick counter if it is time to execute the job(s) in the list
   UINT8  job_index;   // current index in the job list to be executed
}PERIODIC_TASK_TRACE_TYPE;

enum PERIOD_OPTION{EVERY_TICK, HALF_SEC, ONE_SEC, TEN_SEC, FIFTEEN_MIN, ONE_HOUR, NUM_OF_TASK};

/****************************************************************************
                       SYSTEM ERROR COUNT DEFINITION
****************************************************************************/
typedef struct
{
   UINT32 febe;                         // (Farend)  TX CRC count
   UINT32 crc;                          // (Nearend) RX CRC count
   UINT32 cur_sec_febe;                 // far-end CRCs in current second; used to evaluate SES
   UINT32 cur_sec_crc;                  // near-end CRCs in current second; used to evaluate SES
   UINT32 ffec;                         // Far end corrected code word
   UINT32 fec;                          // Near end corrected code word
   UINT32 fhec;                         // Far end badhec count
   UINT32 hec;                          // Near end badhec count
   UINT32 total_bad_hec;                // total near-end bad HEC count
   BOOLEAN near_end_ses;                // near-end SES condion
   BOOLEAN far_end_ses;                 // far-end SES condion
} SYS_ERR_CNT_TYPE;

#define   SYS_ERR_CNT_SIZE   sizeof(SYS_ERR_CNT_TYPE)
/****************************************************************************
                      SYSTEM INFO DEFINITION
****************************************************************************/

#define MAX_RESET_CODE 0x33         // maximum reset code passed to STARTUP_TRACE_ROUTINE
#define MAX_STATIC_MAC_ENTRY    10  // Number of static mac address entry
                                    // to store on the nvram

#define MAX_PASSWORD_LEN 9

typedef struct
{
   char primary[MAX_PASSWORD_LEN];
   char special[MAX_PASSWORD_LEN];
}SYS_PASSWORD_TYPE;

#define   SYS_PASSWORD_SIZE    sizeof(SYS_PASSWORD_TYPE)

// the serial number is represented by a maximum of 32 alpha digits
#define MAX_SERIAL_NUM (32+1)     // includes 1 for NULL char

// Product Id Length
#define MAX_PRODUCT_NUM        sizeof(DEF_PRODUCT_NUM)

#define MAX_PRODUCT_HW_REV     sizeof(DEF_HW_REV)

// Number of days in operation
// already in xcvrinfo.c    
//typedef UINT16 SYS_OPERATING_DAYS_TYPE;

// Xcvr Hardware
#define MAX_XCVR_HW_NUM        sizeof(DEF_XCVR_HW_NUM)
#define MAX_XCVR_HW_REV        sizeof(DEF_XCVR_HW_REV)
#define MAX_XCVR_OTHER_ID (9+1) // includes 1 for NULL


// Should be larger than any string possible to be
// returned by VERS_GetSWVerStr();
#define MAX_VER_LEN 24

// Prom Checksum
typedef struct
{
    UINT16 checksum;
    UINT16 evenchecksum;
    UINT16 oddchecksum;
}
SYS_PROM_CHECKSUM_TYPE;

// Default HW information (product num and revision)

#ifdef TOMCAT
#ifdef CO

// BEGIN DMT CO HW info
#define DEF_PRODUCT_NUM "150-2102-01" // CO part num
#define DEF_HW_REV      "P00 "        // CO rev
#define DEF_XCVR_HW_NUM "100-1756-01" // transceiver part num
#define DEF_XCVR_HW_REV "P00 "        // transceiver rev

#else

// BEGIN DMT CP HW info
#define DEF_PRODUCT_NUM "150-2103-01" // CP part num
#define DEF_HW_REV      "P00 "        // CP rev
#define DEF_XCVR_HW_NUM "100-1766-01" // transceiver part num
#define DEF_XCVR_HW_REV "P00 "        // transceiver rev

#endif

#else   // DMT

#ifdef CO

// BEGIN DMT CO HW info
#define DEF_PRODUCT_NUM "150-1705-01" // CO part num
#define DEF_HW_REV      "P00 "        // CO rev
#define DEF_XCVR_HW_NUM "100-1756-01" // transceiver part num
#define DEF_XCVR_HW_REV "P00 "        // transceiver rev

#else

// BEGIN DMT CP HW info
#define DEF_PRODUCT_NUM "150-1705-11" // CP part num
#define DEF_HW_REV      "P00 "        // CP rev
#define DEF_XCVR_HW_NUM "100-1766-01" // transceiver part num
#define DEF_XCVR_HW_REV "P00 "        // transceiver rev

#endif

#endif




/****************************************************************************
                       FRONT PANEL LED DEFINITION
****************************************************************************/
enum
{
   ADSL_SYNC_UNKNOWN, ADSL_SYNC_NORMAL, ADSL_SYNC_STARTUP, ADSL_SYNC_TEST,
   ADSL_ALM_UNKNOWN, ADSL_ALM_NORMAL, ADSL_ALM_ACTIVE, ADSL_ALM_TEST,
   ADSL_MISC_UNKNOWN, ADSL_MISC_NORMAL, ADSL_MISC_STARTUP, ADSL_MISC_TEST
};

/****************************************************************************
                       SYSTEM SELFTEST DEFINITION
****************************************************************************/
#ifdef LITTLE_ENDIAN_MEMORY
typedef struct
{
    UINT16 prom    : 1;
    UINT16 ram     : 1;
    UINT16 nvram   : 1;
    UINT16 rw_actel: 1;
    UINT16 bridge  : 1;
    UINT16 framer  : 1;
    UINT16 xcvr_1  : 1;
    UINT16 xcvr_2  : 1;
    UINT16         : 1;
    UINT16 up_ber  : 1;
    UINT16 dwn_ber : 1;
    UINT16         : 5;
    UINT16         : 16;
}SELFTEST_BF_TYPE;  // note: total number of bit must be added up to 32

#else

typedef struct
{
    UINT16         : 5;
    UINT16 dwn_ber : 1;
    UINT16 up_ber  : 1;
    UINT16         : 1;
    UINT16 xcvr_2  : 1;
    UINT16 xcvr_1  : 1;
    UINT16 framer  : 1;
    UINT16 bridge  : 1;
    UINT16 rw_actel: 1;
    UINT16 nvram   : 1;
    UINT16 ram     : 1;
    UINT16 prom    : 1;
    UINT16         : 16;
}SELFTEST_BF_TYPE;  // note: total number of bit must be added up to 32
#endif
typedef union
{
    UINT32           image;
    SELFTEST_BF_TYPE bit_field;
}SYS_SELFTEST_RESULT_TYPE;

#define SYS_SELFTEST_RESULT_SIZE   (sizeof(SYS_SELFTEST_RESULT_TYPE))

// the first byte is selftest result
#define SYS_SELFTEST_MASK 0x000000FF

// selftest result default value
#define DEFAULT_SELFTEST_RESULT {(UINT32)0}

/****************************************************************************
                      GLOBAL DATABASE DEFINITION
****************************************************************************/

/* NOTE: When adding elements, to be backward compatible with shipped units
   add to the END of the list.

   If you're going to change the size of a data element, it should be made
   bigger for the same reason above, and the old data must be a subset of
   the existing, or logic must be handled to interface with both systems.
*/
typedef enum GLOBAL_DATA_OFFSET
     {SYSTEM_MEASUREMENT,
      GD_NOT_USED1,
      SERIAL_NUMBER,
      PRODUCT_NUM,
      PRODUCT_REV,
      MANUFACTURE_DATE,
      XCVR_HARDWARE_NUM,
      XCVR_HARDWARE_REV,
      XCVR_OTHER_ID,
      SOFTWARE_REV,
      PROM_CHECKSUM,
      DAYS_IN_OPERATION,
      GD_NOT_USED2,
      GD_NOT_USED3,
      GD_NOT_USED4,
      GD_NOT_USED5,
      GD_NOT_USED6,
      SYSTEM_STATE,
      SELFTEST_RESULT,
      XCVR_CONFIG,
      XCVR_ALM_CONFIG,
      XCVR_OTHER_CONFIG,
      GLOBAL_DATA_SIZE
      }GLOBAL_DATA_OFFSET_TYPE;

/****************************************************************************
                       SYSTEM NVRAM RAM IMAGE DEFINITION
****************************************************************************/


// This image must be identical to that stored in NVRAM
// For backward compatibility, add new elements at end

// when ordering elements of structure
// try to avoid internal packing bytes for word alignment

#ifdef TOMCAT
#define NVRAM_VERSION_NUM (2) // modify any time a change is made
#else
#define NVRAM_VERSION_NUM (1) // modify any time a change is made which requires
                              // the NVRAM to be re-written on startup
                              // NOTE: Only the non-factory data will be updated
#endif

typedef struct
{
   UINT8                    total_used; // includes checksum and this byte
   DATE_TYPE                manufacture_date;              // factory data --v
   UINT8                    product_num[MAX_PRODUCT_NUM];
   UINT8                    hw_rev[MAX_PRODUCT_HW_REV];
   UINT8                    xcvr_hw_num[MAX_XCVR_HW_NUM];
   UINT8                    xcvr_hw_rev[MAX_XCVR_HW_REV];
   UINT8                    serial_num[MAX_SERIAL_NUM];
   UINT16                   checksum;   // simple checksum updated on each write - not used now
                                        // only calc'd on data below
   SYS_PASSWORD_TYPE        password;
   DATE_TYPE                todays_date;
   XCVR_CFG_TYPE            xcvr_cfg;   // This config may exchange with both side Pairgain product
   XCVR_ALM_CFG_TYPE        xcvr_alm_cfg;   // This config may exchange with both side Pairgain product
   XCVR_OTHER_CFG_TYPE      xcvr_other_cfg;  // Save all other config information, keep a copy at global database
   SYS_SELFTEST_RESULT_TYPE selftest_result;
   SYS_OPERATING_DAYS_TYPE  days_in_operation;

#ifdef TOMCAT
   UINT8                    burnin_status;
   UINT8                    rtu_disable;
   UINT8                    lan_mode;
   UINT16                   aging_time;
   SRDB_ENTRY_TYPE          static_mac_addr[MAX_STATIC_MAC_ENTRY];
#endif

   UINT8                    nvram_version; // current version
}SYS_NVRAM_IMAGE_TYPE;


// This definition should parallel the above definition.
// It is used to access the image and NVRAM.
typedef enum NVRAM_DATA
{
   NVRAM_BYTES_USED,
   NVRAM_MFG_DATE,
   NVRAM_PRODUCT_NUM,
   NVRAM_HW_REV,
   NVRAM_XCVR_HW_NUM,
   NVRAM_XCVR_HW_REV,
   NVRAM_SERIAL_NUM,
   NVRAM_CHECKSUM,
   NVRAM_PASSWORD,
   NVRAM_DATE,
   NVRAM_XCVR_CFG,
   NVRAM_XCVR_ALM_CFG,
   NVRAM_XCVR_OTHER_CFG,
   NVRAM_SELFTEST_RESULT,
   NVRAM_OPERATING_DAYS,
#ifdef TOMCAT
   NVRAM_BURNIN_STATUS,
   NVRAM_RTU_DISABLE,
   NVRAM_LAN_MODE,
   NVRAM_AGING_TIME,
   NVRAM_STATIC_MAC_ADDR,
#endif
   NVRAM_VERSION,
   NVRAM_NUM_ELEMENTS
}SYS_NVRAM_DATA_TYPE;

enum {READ_ONLY, READ_WRITE};
enum {AVAILABLE, NOT_AVAILABLE};

typedef struct
{

   size_t   offset;            // offset in 
   UINT8    item;               // Use when data is Nvram
   UINT8    size;               // size of data element
   UINT8    remote_copy_offset; // offset to remote copy of data (if exists)
   UINT16   LocalAccessMode:1;  // whether local copy can be written
                                // remote copy can always be written
   UINT16   RemoteCopy:1;       // whether there is a remote copy
   UINT16   NVRAMData:1;        // whether it is in NVRAM
   UINT16   MultipleXcvrItem;   // whether this item has multiple copies for individual xcvr
}GLOBAL_DATA_INDEX_TYPE;

// Definitions to access both RAM image and NVRAM by application
// code to keep images consistent
typedef struct
{
   size_t offset;     // location of RAM image
   UINT8 size;     // size of data
   UINT8 data_type;// 3 types:
                   // SYSTEM_INFO -- Product specific info will not be reset.
                   //                It is used in sys. CFg menu.
                   // PRODUCT_INFO-- Every thing will be reset to default value
                   //                except manufacture date and serial number.
                   //                It is used in Production Menu.
                   // MANUFACTURE_INFO-- once it is updated by
                   //                    manufacturing, it will not
                   //                    be changed by default value.
}SYS_NVRAM_TABLE_ENTRY_TYPE;

/****************************************************************************
                      PERFORMANCE MONITOR DATABASE TYPE DEFINITION
****************************************************************************/
// Note: Originally the following define is assigned as INT_MAX, which is
//  signed 16 bits, defined in limits.h from BSO.
//  In DSLAM limits.h define INT_MAX as sign 32 bit !!! so we can not
//  equate to INT_MAX here, we changed it to sint16_MAX.
//  DSLAM will redefined in its types.h for sint16_MAX as signed short int

#define SYS_IGNORE_INT16    sint16_MAX // ignore value used in smart terminal for var of 2 bytes
                                       // = 0x7FFF
#define SYS_IGNORE_INT8     0x7F       // ignore value used in smart terminal for var of 1 byte

#define DAYS_IN_WEEK        7          // 7 days = 1 week

enum {DEFECT, NO_DEFECT};
enum {FAILURE, NO_FAILURE};
enum {ANOMALY, NO_ANOMALY};
enum {EVENT, NO_EVENT};


enum
{
    SYSTEM_INFO,      // system info data which does not include product info.
    PRODUCT_INFO,     // product info data which includes system info and product
                      // specific info, e.g. hw num/rev.
    MANUFACTURE_INFO  // manufacture info: manufacture date, serial number
                      // which will not be updated when we set default
                      // factory configuration through PRODUCTION TEST MENU.
};

enum FAILURE_EVENT
{
   NEAR_END_LOF,
   FAR_END_LOF,
   NEAR_END_LOS,
   FAR_END_LOS,
   NEAR_END_LCD,
   FAR_END_LCD,
   NUM_OF_FAILTYPE
};


// This structure contains all the event to be monitored for current and
// previous 15-minute history for ADSL MIB
typedef struct
{
   SINT16  SNR;    // mininum SNR
   UINT16  UAS,    // Unavailable Second
           CRCES,  // CRC Errored Second
           SES,    // severe errored second
           LOF,    // Loss of Frame Failure Count
           LOS,
           FECCS,  // FEC codeword corrected
           LCD;    // Loss of Cell Delineation
}SYS_ALL_EVENT_TYPE;

#define SYS_ALL_EVENT_SIZE  sizeof(SYS_ALL_EVENT_TYPE)

typedef struct
{
   SYS_ALL_EVENT_TYPE NearEnd;
   SYS_ALL_EVENT_TYPE FarEnd;
} SYS_ALL_DATA_TYPE;

#define NULL_MON_ALL_EVENT {SYS_IGNORE_INT16,0,0,0,0,0,0}

// This structure contains all the event to be monitored for current and
// previous day history for ADSL MIB

typedef struct
{
   SINT16  SNR;    // mininum SNR
   UINT32  UAS,    // Unavailable Second
           CRCES,  // CRC Errored Second
           SES,    // severe errored second
           LOF,    // Loss of Frame Failure Count
           LOS,    // Loss of Signal failure Count
           FECCS,  // FEC codeword corrected
           LCD;    // Loss of Cell Delineation
}SYS_WEEKLY_ALL_EVENT_TYPE;

#define SYS_WEEKLY_ALL_EVENT_SIZE sizeof(SYS_WEEKLY_ALL_EVENT_TYPE)

#define NULL_MON_WEEKLY_ALL_EVENT {SYS_IGNORE_INT16,0,0,0,0,0,0}

typedef struct
{
   SYS_WEEKLY_ALL_EVENT_TYPE NearEnd;
   SYS_WEEKLY_ALL_EVENT_TYPE FarEnd;
}SYS_WEEKLY_ALL_DATA_TYPE;

// This structure has been modified for ADSL MIB
typedef struct
{
   SINT16  SNR;    // mininum SNR
   UINT16  UAS,    // Unavailable Second
           CRCES;  // CRC Errored Second
}SYS_EVENT_TYPE;

#define SYS_EVENT_SIZE       sizeof(SYS_EVENT_TYPE)

#define NULL_MONITOR_EVENT {SYS_IGNORE_INT16, 0, 0}

typedef struct
{
   SYS_EVENT_TYPE NearEnd;
   SYS_EVENT_TYPE FarEnd;
}SYS_DATA_TYPE;


// The following struct is used to keep track of LOF and LOS status in every
// second.
typedef struct
{
   UINT16  LOS,    // Loss of Signal Current Second Status
           LOF;    // Loss of Frame Current Second Status
}SYS_LOFLOS_EVENT_STATUS_TYPE;


typedef struct
{
   SYS_LOFLOS_EVENT_STATUS_TYPE NearEnd;
   SYS_LOFLOS_EVENT_STATUS_TYPE FarEnd;
}SYS_LOFLOS_STATUS_TYPE;

// weekly database structure
typedef struct
{
   SINT16  SNR;   // mininum SNR
   UINT32  UAS,   // Unavailable Second
           CRCES; // CRC Errored Second
}SYS_WEEKLY_MON_EVENT_TYPE;

#define SYS_WEEKLY_MON_EVENT_SIZE sizeof(SYS_WEEKLY_MON_EVENT_TYPE)

#define NULL_WEEKLY_MONITOR_EVENT {SYS_IGNORE_INT16,0,0}

typedef struct
{
   SYS_WEEKLY_MON_EVENT_TYPE NearEnd;
   SYS_WEEKLY_MON_EVENT_TYPE FarEnd;
}SYS_WEEKLY_MON_DATA_TYPE;

// free-running counts. These counts run all the time and are reset only
// upon powerup or reset.
typedef struct
{
    UINT32 uas,             // unavailable second
           es,              // errored second
           ses,             // severely errored second
           lof,
           los,             // only DMT suport it
           cs,              // corrected second
           lcd;             // Loss of Cell Delineation
} SYS_TOTAL_EVENT_COUNT;

#define   SYS_TOTAL_EVENT_SIZE   sizeof(SYS_TOTAL_EVENT_COUNT)

typedef struct
{
   UINT16   SFcnt;          // = 0
   UINT8    status;         // = NO_FAILURE
}FAILURE_TYPE;


typedef struct
{
   UINT16       duration;               // Alarm duration in sec
   TIME_TYPE    first_time, last_time;  // first and last date and time
   DATE_TYPE    first_date, last_date;  // of alarm condition since last clear
   BOOLEAN      active;                 // current alarm status
   BOOLEAN      old_status;             // previous alarm status
}ALARM_TYPE;

typedef struct
{
    // This variable is used to keep track of current second LOF and LOS event status
    SYS_LOFLOS_STATUS_TYPE      cur_sec_event_status;
    // This variable is used to keep track of current second event count
    SYS_ALL_DATA_TYPE           cur_sec_failure;
    // This variable is used to keep track of event counts for current and
    // previous 15 minutes and also for current and previous day.
    // This is requested by ADSL MIB
    SYS_ALL_DATA_TYPE           cur_15min_count,
                                prv_15min_count;
    SYS_WEEKLY_ALL_DATA_TYPE    cur_day_count,
                                prv_day_count;
    // Number of total ES/UAS count, since the last cleared
    SYS_TOTAL_EVENT_COUNT       total_event_cnt[NUM_ATU_MODE];
    UINT16                      restart_count;  // number of restartup attempt
    UINT32                      cur_showtime_sec_count;
} SYS_MON_DATA_TYPE;

#define SYS_MON_DATA_SIZE   sizeof(SYS_MON_DATA_TYPE)


//Current line status definitions; used for MIB TR-006
typedef enum LINE_STATUS
{
    LINESTATUS_CLEAR                = 0,    /* Used to clear status */
    LINESTATUS_DEBUG_MODE           = 0,    /* Pairgain equipement in debug mode (Not part of TR-006) */
    LINESTATUS_NO_DEFECT            = 1,    /* Line up and perfect */
    LINESTATUS_LOSS_OF_FRAMING      = 2,    /* LOF Alarm */
    LINESTATUS_LOSS_OF_SIGNAL       = 4,    /* LOS Alarm */
    LINESTATUS_LOSS_OF_POWER        = 8,    /* Not supported */
    LINESTATUS_LOSS_OF_LINK         = 16,   /* Not supported */
    LINESTATUS_LOSS_OF_SIGNAL_QUALITY = 32, /* ES, UAS, or SNR Margin Alarms */
    LINESTATUS_DATA_INIT_FAILURE    = 64,   /* Init Failure due to bit errors */
    LINESTATUS_CONFIG_INIT_FAILURE  = 128,  /* Init Failure due to line can't support config */
    LINESTATUS_PROTOCOL_INIT_FAILURE = 256, /* Not supported */
    LINESTATUS_NO_PEER_ATU_PRESENT  = 512   /* No modem on far end */
}LINE_STATUS_DATA_TYPE;

//Mask used to clear all fields except Init Failure Conditions
#define SAVE_INIT_FAILURES (LINESTATUS_DATA_INIT_FAILURE     + \
                            LINESTATUS_CONFIG_INIT_FAILURE   + \
                            LINESTATUS_PROTOCOL_INIT_FAILURE + \
                            LINESTATUS_NO_PEER_ATU_PRESENT)

//Mask used to clear only Init Failure Conditions from line status
#define CLEAR_INIT_FAILURES (SAVE_INIT_FAILURES^0xFFFF)


/****************************************************************************
                      DIAGNOSTIC LOG DEFINITION
****************************************************************************/
typedef const char * DIAG_ENTRY_TYPE;
typedef GEN_UINT     DIAG_SUBCODE_TYPE;


// With the following definition we can do sprintf without knowing the actual
// data structure of the diagnostic log entry.
// e.g. sprintf(" 1:" DIAG_ENTRY_DISPLAY_FLAG " subcode is "
//              DIAG_SUBCODE_DISPLAY_FLAG, diag_log.entry, diag_log.subcode);

#define DIAG_ENTRY_DISPLAY_FLAG       "%s"
#define DIAG_SUBCODE_DISPLAY_FLAG    " %02lX"

#define NULL_DIAG_ENTRY        NULL

#define NULL_DIAG_SUBCODE      ((DIAG_SUBCODE_TYPE)(-1))

typedef struct
{
    DIAG_ENTRY_TYPE    entry;
    DIAG_SUBCODE_TYPE  subcode;
    TIME_TYPE          timestamp;
}DIAGNOSTIC_TYPE;







/****************************************************************************
                       DATA INTERFACE CONTROL OF THE FPGA
****************************************************************************/
// TX_CLT state define
enum TX_CLT_STATE
{
   TX_CTL_NORMAL = 0,
   TX_CTL_INSERT_ZERO,
   TX_CTL_INSERT_ONE,
   TX_CTL_LOOPBACK,
   TX_CTL_ADSL_TST,
   TX_CTL_BRIDGE_TST
};

/****************************************************************************
                          SELF-TEST DEFINITION
****************************************************************************/
typedef enum
{
    TST_IN_PROGRESS,
    TST_PASS,
    TST_FAIL,
    TST_ABORT,
    MAX_TEST_STATUS
}SYS_ST_RET_TYPE;

typedef struct
{
   char             *test_name;             // test name
   void             (*init_test)(void);     // intialization function
   SYS_ST_RET_TYPE  (*monitor_test)(void);  // monitoring function
   void             (*exit_test)(void);     // exiting function
}SYS_SELFTEST_TYPE;

#define   SYS_SELFTEST_SIZE   (sizeof(SYS_SELFTEST_TYPE))

/****************************************************************************
                       SYSTEM MARGIN DEFINITION
    Note: The following structure defines various parameters which are
            dynamically measured during operation.

    'mar_min'   : store the smallest margin while measuring margin such as in
                    monitor.c. Do not confused with the minimum margin setting,
                    below which we have alarm (CAP/DMT)
    'mar_max'   : Not used at time being
    'mar_cur'   : Current measurement margin (CAP/DMT)
    'line_att'  : Current line attenuation measurement (CAP/DMT)
    'xmit_pwr'  : Current xmit power.  Do not confuse with the max xmit power
                    setting in the xcvr_cfg
                  Currently, only CAP support dymaic measurement of xmit
                    power.  DMT does not support this
****************************************************************************/
typedef struct
{
   SINT16 mar_min;              // Minimum SNR margin, unit in dB
   SINT16 mar_max;              // Max SNR margin, unit in dB (Not used at time being)
   SINT16 mar_cur;              // Current SNR margin, unit in dB
   SINT16 line_atn;             // Line attenuation, unit in dB
   SINT16 xmit_pwr;             // Unit in dB
   UINT16 mar_min_bin;          // DMT only
   UINT16 unused[4];            // for future expansion
} SYS_MEASUREMENT_TYPE;

#define SYS_MEASUREMENT_SIZE   (sizeof(SYS_MEASUREMENT_TYPE))

/****************************************************************************
                       DSLAM command interface DEFINITION
****************************************************************************/

enum {SYS_SET_XCVR_CFG=0x00,    SYS_SET_XCVR_STATE, SYS_MAX_SET};
enum {SYS_GET_XCVR_CFG=0x00,    SYS_GET_XCVR_STATE, SYS_GET_XCVR_STATUS};
enum {SYS_GET_15MIN_PERF=0x00,  SYS_GET_7DAY_PERF,  SYS_GET_FREE_ERR_CNT};

extern const SYS_NVRAM_TABLE_ENTRY_TYPE SYS_nvram_lookup_table[NVRAM_NUM_ELEMENTS];

DPCCALL void    SYS_GetRemDB(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    SYS_ReadRemDB(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void    SYS_ResetGlobalDatabase(PDATAPUMP_CONTEXT pDatapump);
DPCCALL BOOLEAN SYS_UpdateRamAndNvram(PDATAPUMP_CONTEXT pDatapump,UINT8 near_far,
                                void *psrc, SYS_NVRAM_DATA_TYPE data_type);
DPCCALL BOOLEAN SYS_GlobalDataWrite(PDATAPUMP_CONTEXT pDatapump,UINT8, void *, UINT8, UINT8);
DPCCALL BOOLEAN SYS_GlobalDataRead(PDATAPUMP_CONTEXT pDatapump,void *, UINT8, UINT8, UINT8);
DPCCALL UINT8   SYS_GetGlobalDataSize(UINT8);
DPCCALL BOOLEAN  SYS_GetSysNvram(PDATAPUMP_CONTEXT pDatapump,UINT8 near_far,
                                SYS_NVRAM_DATA_TYPE data_type, void* pdest);
DPCCALL BOOLEAN SYS_SetSysNvram(PDATAPUMP_CONTEXT pDatapump,UINT8 near_far,
                                SYS_NVRAM_DATA_TYPE data_type, void* psrc);
DPCCALL UINT8   SYS_GetSysNvramSize(PDATAPUMP_CONTEXT pDatapump,UINT8 near_far,
                                SYS_NVRAM_DATA_TYPE data_type);
DPCCALL BOOLEAN         SYS_UpdateRemoteXcvrCfg(PDATAPUMP_CONTEXT pDatapump);
DPCCALL char    *SYS_GetSWVerStr(void);

#endif
/***************************************************************************
 * $Log: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/sysinfo.h $
 * 
 * 21    6/26/01 8:39p Lauds
 * multi-instance version integration
 * 
 * 2     6/22/01 3:14p Lauds
 * minor cleanup and bug fix in miscutil.c and terminal.c
 * 
 * 1     6/21/01 3:12p Lauds
 * Multi-instance initial version
 * 
 * 20    5/01/01 7:48p Lauds
 * Annex B support for C_B&G
 * 
 * 19    1/09/01 10:13a Lewisrc
 * Wirespeed/Redhat had to make minor changes to DMT files to compile
 * under GNU
 * 
 * Removed duplicate of SYS_OPERATING_DAYS_TYPE (also in xcvrinfo.h
 * 
 * 18    12/12/00 4:56p Lauds
 * Redefine SES definition
 * 
 * 17    12/01/00 10:07a Lauds
 * increase MAX_SERIAL_NUM from 12 to 32 for Ron Lewis. Add AddDiagnostic
 * extern to ease debugging.
 * 
 * 16    11/09/00 5:40p Lauds
 * keep track of total bad HEC cells 
 * 
 * 15    9/27/00 7:55p Lauds
 * more cleanup, reduce warning messages
 * 
 * 14    9/25/00 6:12p Lauds
 * change DIAG_SUBCODE_TYPE to GEN_UINT
 * 
 * 13    9/18/00 7:01p Lauhk
 * Added compiler switch to help porting to Euphrates.
 * 
 * 12    5/08/00 9:49a Bradforw
 * Added code to delay reporting of crc/fec statistics for 2 seconds
 * Added code to initialize tx filter coefficients to zero
 * 
 * 11    3/08/00 2:03p Lewisrc
 * add MAX_RESET_CODE
 * 
 * 10    2/28/00 10:48a Lewisrc
 * 2.03 Pairgain release for San Jose Code Interop
 * 
 *    Rev 1.153   Jan 31 2000 13:29:26   zhaos
 * Add parameters for API
 * 
 *    Rev 1.152   Jan 26 2000 13:06:32   yangl
 * -- Fix compile error for c163
 *
 *    Rev 1.151   Jan 26 2000 11:46:46   yangl
 * -- Add STDEOC API
 * -- Support clear EOC
 * -- Add misc cfg menu
 *
 *    Rev 1.150   Oct 11 1999 11:48:48   yangl
 * Increase MAX_VER_LEN to 24
 *
 *    Rev 1.149   Oct 08 1999 11:04:48   phang
 * Increase storage for MAC address to 10
 *
 *    Rev 1.148   Sep 27 1999 17:11:28   nordees
 * Support for a running CRC error count.
 *
 *    Rev 1.147   Sep 27 1999 16:27:30   phang
 * Save SRDB_ENTRY_TYPE instead MAC_ADDR_TYPE
 *
 *    Rev 1.146   Sep 16 1999 13:35:34   phang
 * add rcb configuration to nvram.
 *
 *    Rev 1.145   Sep 16 1999 09:42:10   yangl
 * Support to save basic G.hs options to nvram.
 * May need add more later.
 *
 *    Rev 1.144   Aug 20 1999 09:47:04   phang
 * Change project number for tomcat.
 *
 *    Rev 1.143   Aug 04 1999 16:16:08   phang
 * Add variable to hold checksum.
 *
 *    Rev 1.142   Jun 23 1999 16:53:10   nordees
 * Line status returns 0 when system is in DEBUG mode.
 *
 *    Rev 1.141   May 20 1999 16:17:20   laud
 * fix xmit_pwr in SYS_measurement
 *
 *    Rev 1.140   Apr 15 1999 11:28:48   nordees
 * Support for both G.DMT and G.HANDSHAKE.
 *
 *    Rev 1.139   Apr 07 1999 18:22:30   laud
 * Get rid of SourceCom EBM code
 *
 *    Rev 1.138   Mar 31 1999 12:42:20   nordees
 * Initialization failure alarm support (MIB TR-006).
 *
 *    Rev 1.137   Mar 15 1999 14:36:40   nordees
 * Line status code cleaned up.
 *
 *    Rev 1.136   Feb 17 1999 18:59:10   laud
 * get rid of unused NMA interface
 *
 *    Rev 1.135   Feb 15 1999 15:22:40   nordees
 * Current line status support for MIB.
 *
 *    Rev 1.134   Feb 09 1999 18:29:26   laud
 * LCD anomaly, defect, and failure evaluation
 *
 *    Rev 1.133   Feb 04 1999 18:49:18   laud
 * split up startup snr margin to upstream and downstream
 *
 *    Rev 1.132   Sep 30 1998 17:49:28   laud
 * eoc support for multiple xcvr and sw ver API
 *
 *    Rev 1.131   Sep 21 1998 12:46:28   zhaos
 *
 *
 *    Rev 1.130   Jul 07 1998 19:00:50   laud
 * big endian support for global data access
 *
 *    Rev 1.129   Jul 06 1998 19:17:48   laud
 *
 *
 *    Rev 1.128   Jul 01 1998 15:25:40   laud
 * change NULL_DIAG_SUBCODE value
 *
 *    Rev 1.127   19 May 1998 19:17:12   laud
 * fix compatibility problem with DSLAM
 *
 *    Rev 1.126   24 Mar 1998 08:27:38   phamt
 * 1. Build old (V1.01G) xcvr_cfg structure on the fly
 * to make it compatible.
 * 2. In sysinfo.h: For CAP only, put back the product number
 * and the transceiver hardware num to be xxxx-yy
 *
 *    Rev 1.125   16 Feb 1998 17:55:10   phamt
 * Put some comment on SYS_measurement[]
 *
 *    Rev 1.124   13 Feb 1998 17:15:28   hej
 * change the name IGNORE_VALUE to NMA_IGNORE_INT16,
 * change the name IGNORE_VALUE2 to NMA_IGNORE_INT8,
 *
 *    Rev 1.123   11 Feb 1998 16:45:26   hej
 * re-order ALARM_STATUS structure's element.
 *
 *    Rev 1.101   21 Nov 1997 08:31:28   phamt
 * dslam
 *
 *    Rev 1.100   19 Nov 1997 10:55:58   HEJ
 * Add default cfg value for EBM.
 *
 *
 *    Rev 1.0   14 Oct 1996 15:33:58   TSE
 * Initial revision.
****************************************************************************/
/**********     end of file $Workfile: sysinfo.h $        ***************/
