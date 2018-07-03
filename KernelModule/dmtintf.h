#ifndef _DMTINTF_H_
#define _DMTINTF_H_
typedef struct
{
    BOOLEAN coldstart_flag;
}LOCAL_DMTINTF_TYPE;

/*****                     Global functions                           ******/
DPCCALL void DMT_ProcCurStartUpState(PDATAPUMP_CONTEXT pDatapump, UINT8);
DPCCALL void DMT_ResetStartupStateMachines(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void DMT_PwrUpInit(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void DMT_IRQ1_ISR(PDATAPUMP_CONTEXT pDatapump);
DPCCALL void DMT_IRQ2_ISR(PDATAPUMP_CONTEXT pDatapump);
#endif
/*****************************************************************************
 * $Log: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/dmtintf.h $
 * 
 * 3     7/12/01 8:28a Davidsdj
 * PROBLEM:
 * Warning generated in Linux compiles that there is a comment within a
 * comment.
 * 
 * SOLUTION:
 * Removed the slash ('/') from line within the comment block that was
 * generating the warning message.
 * 
 * 2     7/10/01 12:57p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 **************  end of file $Workfile: dmtintf.h $          ****************/

