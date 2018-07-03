#ifndef _SMRT_H_
#define _SMRT_H_
/*****                     Global functions                           ******/
#define SMRT_MSG_OVERHEAD      5   // overhead= sync+resv+length+chksum(2).
#define SYS_SMRT_CMD_DATA_SIZE   251  // msg from user: cmd + data
                                      // 251 = 256-MSG_OVERHEAD.
                                      // 256 --  because 'length' is defined as 1 byte.

typedef union
{
    UINT8   uint8[SYS_SMRT_CMD_DATA_SIZE];
    UINT16  uint16[SYS_SMRT_CMD_DATA_SIZE/2];
    UINT32  uint32[SYS_SMRT_CMD_DATA_SIZE/4];
} OUT_BUF_TYPE;

typedef struct
{
    UINT8 smrt_in_buf[SYS_SMRT_CMD_DATA_SIZE+SMRT_MSG_OVERHEAD];
    UINT16 rx_byte_num;           // number of bytes received in a frame:
                                          // SYNC_BYTE, RESV_BYTE,...CHKSUM.
                                          // No framing char(0xF1) is included.
}LOCAL_SMRT_TYPE;

DPCCALL void SMRT_Init(PDATAPUMP_CONTEXT pDatapump, UINT8 *, UINT16);
DPCCALL void SMRT_Reset(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void SMRT_CheckMsgTimeout(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void SMRT_ProcessRxByte(PDATAPUMP_CONTEXT pDatapump, UINT8);
DPCCALL void SMRT_ProcessRxMsg(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void SMRT_XmitMsg(PDATAPUMP_CONTEXT pDatapump, UINT8 *, UINT8 );
DPCCALL void SMRT_SendCmdResponse(PDATAPUMP_CONTEXT pDatapump, UINT8 cmd, UINT8 result);
DPCCALL BOOLEAN SMRT_IsSysInTAOMode(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void SMRT_XmitTAOData(PDATAPUMP_CONTEXT pDatapump, const char *msg, unsigned int size);
#endif

/*****************************************************************************
 * $Log: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/smrt.h $
 * 
 * 3     7/12/01 8:29a Davidsdj
 * PROBLEM:
 * Warning generated in Linux compiles that there is a comment within a
 * comment.
 * 
 * SOLUTION:
 * Removed the slash ('/') from line within the comment block that was
 * generating the warning message.
 * 
 * 2     7/10/01 1:06p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 *****************  end of file $Workfile: smrt.h $          ****************/
