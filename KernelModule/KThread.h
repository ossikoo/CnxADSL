#ifndef _KTHREAD_H
#define _KTHREAD_H

struct task_struct;
struct CDSL_ADAPTER_S;

/* start new kthread (called by creator) */
struct task_struct *start_kthread(int (*func)(struct CDSL_ADAPTER_S *), struct CDSL_ADAPTER_S *adapter, const char *name);

#endif
