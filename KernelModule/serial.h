#ifndef _SERIAL_H_
#define _SERIAL_H_
#define MAX_TXQ_SIZE        3000    // size of the transmit queue
#define MAX_RXQ_SIZE         500     // incoming bandwidth is signif. smaller

typedef struct
{
    SYS_SERIAL_MODE_TYPE serial_mode;
//    UINT8 tx_msgq_buffer[MAX_TXQ_SIZE];
//    UINT8 rx_msgq_buffer[MAX_RXQ_SIZE];
}LOCAL_SERIAL_TYPE;

/*****                     Global functions                           ******/
DPCCALL void SER_RcvIntrHandler(PDATAPUMP_CONTEXT pDatapump, char input_char);
DPCCALL void SER_XmitMsg(PDATAPUMP_CONTEXT pDatapump, const char * msg, unsigned int size);
DPCCALL void SER_XmitRemoteLogonReply(PDATAPUMP_CONTEXT pDatapump, const UINT8 *msg, UINT8 size);
DPCCALL void SER_ProcessRxcMsg(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void SER_ProcessDumbTermInput(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void SER_InitMsgQ(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void SER_DetectTerminalConnected(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void SER_ClearRemoteLogon(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void SER_LoginToRemote(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void SER_SetSerialMode(PDATAPUMP_CONTEXT pDatapump, UINT8);
DPCCALL void SER_DumbTermHandler(PDATAPUMP_CONTEXT pDatapump, const char *);

DPCCALL SYS_SERIAL_MODE_TYPE SER_QueryTerminalMode(PDATAPUMP_CONTEXT pDatapump);
DPCCALL SYS_LOGON_MODE_TYPE  SER_QueryLogonMode(PDATAPUMP_CONTEXT pDatapump);
#endif
/*****************************************************************************
 * $Log: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/serial.h $
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
 * 2     7/10/01 1:05p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 ***************  end of file $Workfile: serial.h $          ****************/

