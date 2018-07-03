#ifndef _DPUDMT_H_
#define _DPUDMT_H_
typedef struct
{
    MEASURED_FR_POWERLEVELS fr_powerlevels;
    AOC_DB_TYPE AdslAocMenuScrDisp_dupptr;          // This really use lot memory, any better way???
    STDEOC_DB AdslEocMenuScrDisp_dupptr;
    BOOLEAN  power_down_mode;
    SPRAM_TYPE vbuffer;         // buffer to be viewed in FALCON TEST menu
}LOCAL_DPUDMT_TYPE;

extern const char * far const ghs_handshake_strings[NUM_PROTOCOLS];
extern const char * far const ghs_abort_strings[NUM_ABORT_REASONS];
#endif
/*****************************************************************************
 * $Log: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4.3_ATM_Driver/KernelModule/dpudmt.h $
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
 ***************  end of file $Workfile: dpudmt.h $          ****************/
