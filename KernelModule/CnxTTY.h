#ifndef _CNXTTY_H
#define _CNXTTY_H

#include <linux/tty.h>

extern struct tty_driver cnxtty_driver;

int cnxtty_init ( CDSL_ADAPTER_T *pAdapter ) ;
DPCCALL void cnxtty_SER_XmitMsg ( PDATAPUMP_CONTEXT pDatapump, const char *msg, UINT16 size ) ;


#endif /* _CNXTTY_H */

