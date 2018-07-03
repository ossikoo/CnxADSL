// adslinfo.h
#ifndef _ADSLINFO_H_
#define _ADSLINFO_H_

typedef struct
{
    BOOLEAN link_active;
    SYS_PROM_CHECKSUM_TYPE SYS_prom_checksum[NUM_ATU_MODE];
} LOCAL_ADSLINFO_TYPE;
DPCCALL void  SYS_UpdateLinkStatus(PDATAPUMP_CONTEXT pDatapump, UINT8 active);
DPCCALL BOOLEAN SYS_WriteNvram(PDATAPUMP_CONTEXT pDatapump, size_t offset,SYS_NVRAM_DATA_TYPE data_type );
DPCCALL void SYS_ResetNonVolatileDatabase(PDATAPUMP_CONTEXT pDatapump, UINT8 data_type_range);
DPCCALL void SYS_RestoreNonVolatileDatabase(PDATAPUMP_CONTEXT pDatapump);
#endif
/*****************************************************************************
 * $Log: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/adslinfo.h $
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
 * 2     7/10/01 12:53p Lauds
 * Create local_var structure for each module.  Add #ifndef _???_H_ in all
 * header files.
 *************  end of file $Workfile: adslinfo.h $          ****************/

