#include "KThread.h"
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/signal.h>

/* a structure to store all information we need for our thread */
typedef struct kthread_struct
{
        /* wrapped closure */
        int (*func)(struct CDSL_ADAPTER_S *);
        struct CDSL_ADAPTER_S *adapter;

        /* daemon name */
        const char *name;

        /* semaphore needed on start of thread */
        struct semaphore start_sem;
} kthread_t;

/* initialize new created thread. Called by the new thread. */
int init_kthread(kthread_t *kthread)
{
        /* get closure from kthread data */
        int (*const func)(struct CDSL_ADAPTER_S *) = kthread->func;
        struct CDSL_ADAPTER_S *const adapter = kthread->adapter;

        /* set signal mask to what we want to respond */
        siginitsetinv(&current->blocked, sigmask(SIGKILL)|sigmask(SIGINT)|sigmask(SIGTERM));

        /* ensure we have really read all kthread data before upping makes our creator destroy it */
        mb();

        /* tell the creator that we are ready and let him continue */
        up(&kthread->start_sem);

        return (*func)(adapter);
}

/* create a new kernel thread. Called by the creator. */
struct task_struct *start_kthread(int (*func)(struct CDSL_ADAPTER_S *), struct CDSL_ADAPTER_S *adapter, const char *name)
{
        struct kthread_struct kthread = {
                .func = func,
                .adapter = adapter,
                .name = name,
                .start_sem = __SEMAPHORE_INITIALIZER(kthread.start_sem, 0)
        };

        /* make sure kthread init is visible on spawned thread */
        mb();

        {
                /* create the new thread by running a task through keventd */
                struct task_struct *const thread = kthread_run((int(*)(void*))&init_kthread, &kthread, name);

                /* wait till it has reached the setup_thread routine */
                down(&kthread.start_sem);

                return thread;
        }
}
