/******************************************************************************
********************************************************************************
****	Copyright (c) 1997, 1998, 1999, 2000, 2001
****	Conexant Systems Inc. (formerly Rockwell Semiconductor Systems)
****	Personal Computing Division
****	All Rights Reserved
****
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****
*******************************************************************************
**
**	MODULE NAME:
**		????
**
**	FILE NAME:
**		CnxTTY.c
**
**	ABSTRACT:
**		This file implements tty interface
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.4.3_ATM_Driver/KernelModule/CnxTTY.c $
**	$Revision: 5 $
**	$Date: 8/09/01 1:47p $
*******************************************************************************
******************************************************************************/

#include <linux/tty.h>
#include <linux/mm.h>
#include "LnxTools.h"
#include <linux/module.h>

#include <asm/uaccess.h>

#include "CardMgmt.h"
#include "CardALTigrisDp.h"
#include "datapump.h"
#include "sysinfo.h"
#include "serial.h"
#include "terminal.h"



#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31)
#define RECEIVE_BUF(ldisc) ((ldisc)->ops->receive_buf)
#else
#define RECEIVE_BUF(ldisc) ((*(ldisc))->ops->receive_buf)
#endif

//#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
#define TERMIOS(tty) (((tty)->termios))
//#else
//#define TERMIOS(tty) (*(tty)->termios)
//#endif

/*  #define CNXTTY_MAJOR 47 */
#define CNXTTY_MAJOR 0
#define CNXTTY_NORMAL_TYPE 1
#define CNXTTY_PORTS 1 /* if this is not 1, a lot of code is broken */

struct tty_struct *cnxtty_table[CNXTTY_PORTS];
static struct ktermios *cnxtty_termios[CNXTTY_PORTS];
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
static struct ktermios *cnxtty_termios_locked[CNXTTY_PORTS];
#endif
struct tty_driver cnxtty_driver;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
static int cnxtty_refcount = 0;
#endif
static unsigned char *temp_buffer;
static struct semaphore temp_buffer_sem;


#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#define MEMSET(p,c,s)                              \
{                                                  \
	unsigned char *t = (unsigned char *)(p);       \
	unsigned char v = (unsigned char)c;            \
	long l = (unsigned long)s;                     \
	while(l) {                                     \
		t[--l] = v;                                \
	}                                              \
}

#ifdef DEBUG_CNXTTY2
#define DBP(a,b...)	printk("\n\r %s: " a, __FUNCTION__ , ## b) 
#else
#define DBP(a,b...)
#endif 

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
#define DBP_TERMIOS_LOCKED(d) DBP("**termios_locked %x", (unsigned int) (d)->termios_locked;)
#else
#define DBP_TERMIOS_LOCKED(d)
#endif

#define DBP_TTY_DRIVER(d) \
DBP("TTY DRIVER");                          \
DBP("magic            %d", (d)->magic);       \
DBP("driver_name      %s", (d)->driver_name); \
DBP("name             %s", (d)->name);        \
DBP("name_base        %d", (d)->name_base);   \
DBP("major            %d", (d)->major);       \
DBP("minor_start      %d", (d)->minor_start); \
DBP("num              %d", (d)->num);         \
DBP("type             %d", (d)->type);        \
DBP("subtype          %d", (d)->subtype);     \
DBP("init_termios --not done--");           \
DBP("flags            %d\n", (d)->flags);     \
DBP("*refcount        %x", (unsigned int) (d)->refcount);    \
DBP("*proc_entry      %x", (unsigned int) (d)->proc_entry);  \
DBP("*other           %x", (unsigned int) (d)->other);       \
DBP("**table          %x", (unsigned int) (d)->table);       \
DBP("**termios        %x", (unsigned int) (d)->termios);     \
DBP_TERMIOS_LOCKED(d)\
DBP("*driver_state    %x", (unsigned int) (d)->driver_state);\
DBP("*open            %x", (unsigned int) (d)->open);        \
DBP("*close           %x", (unsigned int) (d)->close);        \
DBP("*write           %x", (unsigned int) (d)->write);       \
DBP("*put_char        %x", (unsigned int) (d)->put_char);    \
DBP("*flush_chars     %x", (unsigned int) (d)->flush_chars); \
DBP("*write_room      %x", (unsigned int) (d)->write_room);  \
DBP("*chars_in_buffer %x", (unsigned int) (d)->chars_in_buffer); \
DBP("*ioctl           %x", (unsigned int) (d)->ioctl);       \
DBP("*set_termios     %x", (unsigned int) (d)->set_termios); \
DBP("*throttle        %x", (unsigned int) (d)->throttle);    \
DBP("*unthrottle      %x", (unsigned int) (d)->unthrottle);  \
DBP("*stop            %x", (unsigned int) (d)->unthrottle);  \
DBP("*start           %x", (unsigned int) (d)->start);       \
DBP("*hangup          %x", (unsigned int) (d)->hangup);      \
DBP("*break_ctl       %x", (unsigned int) (d)->break_ctl);   \
DBP("*flush_buffer    %x", (unsigned int) (d)->flush_buffer);\
DBP("*set_ldisc       %x", (unsigned int) (d)->set_ldisc);   \
DBP("*wait_until_sent %x", (unsigned int) (d)->wait_until_sent);\
DBP("*send_xchar      %x", (unsigned int) (d)->send_xchar);  \
DBP("*read_proc       %x", (unsigned int) (d)->read_proc);   \
DBP("*write_proc      %x", (unsigned int) (d)->write_proc);  \
DBP("*next            %x", (unsigned int) (d)->next);        \
DBP("*prev            %x", (unsigned int) (d)->prev);

#define DBP_TTY(t)                       \
DBP("\n\rTTY");                          \
DBP("magic            %d", (t)->magic);  \
DBP_TTY_DRIVER((t)->driver);             \
DBP("pgrp             %d", (t)->pgrp);   \
DBP("session          %d", (t)->session);\
DBP("flags            %d", (t)->flags);  \
DBP("count            %d", (t)->count); 

static CDSL_ADAPTER_T *pThisAdapter ;

static int _alloc_temp_buffer(void)
{
	int returnValue = -1;

	if (!temp_buffer) {
		temp_buffer = (unsigned char *) __get_free_page(GFP_KERNEL);
		if (!temp_buffer) {
			goto _alloc_temp_buffer_end;
		}
	}

	returnValue = 0;

_alloc_temp_buffer_end:

	return returnValue;
}

static int _free_temp_buffer(void)
{
	int returnValue = -1;

	if (temp_buffer) {
		free_page((unsigned long)temp_buffer);
		temp_buffer = 0;
	}

	returnValue = 0;
	return returnValue;
}

static int cnxtty_open(struct tty_struct *tty, struct file * filp)
{
	int returnValue = -ENODEV;

	if(_alloc_temp_buffer()) {
		DBP("ERROR allocating memory");
		returnValue = -ENOMEM;
		goto cnxtty_open_end;
	}

//	printk ( "cnxtty_open driver_data=%p\n", tty->driver_data ) ;
	ChipAlTigrisDpSER_DetectTerminalConnected ( pThisAdapter /*tty->driver_data*/ ) ;

	returnValue = 0;

	//MOD_INC_USE_COUNT;

cnxtty_open_end:

	/*  	DBP("returns %d\n", returnValue); */

	return returnValue;
}

static void cnxtty_close(struct tty_struct * tty, struct file * filp)
{
	int returnValue = -1;

//	printk ( "cnxtty_close driver_data=%p\n", tty->driver_data ) ;
	ChipAlTigrisDpTRM_AutoUpdateDisable ( pThisAdapter /*tty->driver_data*/, LOCAL_LOGON, TRUE ) ;
	ChipAlTigrisDpSER_ClearRemoteLogon ( pThisAdapter /*tty->driver_data*/ ) ;

	_free_temp_buffer();

	//MOD_DEC_USE_COUNT;

	returnValue = 0;

	return;
}

/*
static void _cnxtty_write_wait(struct tty_struct * tty) {
	long timeout = MAX_SCHEDULE_TIMEOUT;

	DECLARE_WAITQUEUE(wait, current);
	add_wait_queue(&tty->write_wait, &wait);
	do {
		set_current_state(TASK_INTERRUPTIBLE);
		if (signal_pending(current))
			goto stop_waiting;
		timeout = schedule_timeout(timeout);
	} while (timeout);

stop_waiting:
	current->state = TASK_RUNNING;
	remove_wait_queue(&tty->write_wait, &wait);
}
*/

struct tty_struct * gTTY=0;
int cnxtty_putchar(int c) {
	if (gTTY) {
		char ch[1];
		ch[0] = (char)c;
		if (0 < gTTY->receive_room) {
			RECEIVE_BUF(&gTTY->ldisc)(gTTY, &ch[0], 0, 1);
		}
	}
	
	return 1;
}
static int cnxtty_write(struct tty_struct * tty, const unsigned char *buf, int count)
{
	int returnValue = 0;
	int numberToWrite;
	int charLoop;
	unsigned char *pBuffer;

	/* check space so we don't copy needlessly */
	numberToWrite = MIN(count, tty->receive_room);

	if (!numberToWrite) {
		DBP("ERROR no room in the receive buffer");
		goto cnxtty_write_end;
	}

	pBuffer = (unsigned char *)buf; /* cast for cc */

//	printk ( "cnxtty_write driver_data=%p\n", tty->driver_data ) ;
	for (charLoop = 0; charLoop < numberToWrite; charLoop++) {
		/*  	  tty->ldisc.receive_buf(tty, &pBuffer[charLoop], 0, 1); */
		/*  	  gTTY = tty; */
		ChipAlTigrisDpSER_RcvIntrHandler ( pThisAdapter /*tty->driver_data*/, pBuffer[charLoop] ) ;
	}

	returnValue = numberToWrite;

cnxtty_write_end:

	/*    	DBP("returns %d\n", returnValue);  */

	/*  	_cnxtty_write_wait(tty); */

	return returnValue;
}

static int cnxtty_write_room(struct tty_struct *tty)
{
	int returnValue = -1;

	returnValue = 0;

	/*  	DBP("returns %d\n", returnValue); */

	return returnValue;
}

static void cnxtty_flush_buffer(struct tty_struct *tty)
{
	int returnValue = -1;

	returnValue = 0;

	/*  	DBP("returns (not really) %d\n", returnValue); */

	return;
}

static int cnxtty_chars_in_buffer(struct tty_struct *tty)
{
	int returnValue = -1;

	returnValue = 1; /* just a lie to get minicom to go a read */

	DBP("returns %d\n", returnValue);

	return returnValue;

}

static void cnxtty_unthrottle(struct tty_struct * tty)
{
	int returnValue = -1;

	returnValue = 0;

	/*  	DBP("returns (not really) %d\n", returnValue); */

	return;
}

static void cnxtty_set_termios(struct tty_struct *tty,
                               struct ktermios *old_termios)
{
	int returnValue = -1;

	TERMIOS(tty).c_cflag &= ~(CSIZE | PARENB);
	TERMIOS(tty).c_cflag |= (CS8 | CREAD);

	returnValue = 0;

	if (returnValue) {
		DBP("returns (not really) %d\n", returnValue);
	}

	return;
}

static void cnxtty_hangup(struct tty_struct *tty)
{
	int returnValue = -1;

	returnValue = 0;

	/*     	DBP("returns (not really) %d\n", returnValue);  */

	return;
}

/*
 *  Setup
 */
int cnxtty_init ( CDSL_ADAPTER_T *pAdapter)
{
	int status;
	int returnValue = 1;

	/*   	DBP("Hello"); */

	/* init the globals */
	temp_buffer = 0;
	sema_init(&temp_buffer_sem, 1);

	MEMSET(&cnxtty_driver, 0, sizeof(struct tty_driver));
	cnxtty_driver.magic = TTY_DRIVER_MAGIC;
	cnxtty_driver.driver_name = "test_tty";
	cnxtty_driver.name = "ttyCX";
	cnxtty_driver.major = CNXTTY_MAJOR;
	cnxtty_driver.minor_start = 0;
	cnxtty_driver.num = CNXTTY_PORTS;
	cnxtty_driver.type = TTY_DRIVER_TYPE_SERIAL;
	cnxtty_driver.subtype = CNXTTY_NORMAL_TYPE;
	cnxtty_driver.init_termios = tty_std_termios;
	cnxtty_driver.init_termios.c_cflag = B38400 | CS8 | CREAD;
	cnxtty_driver.flags = TTY_DRIVER_RESET_TERMIOS | TTY_DRIVER_REAL_RAW;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
	cnxtty_driver.refcount = (int)&cnxtty_refcount; //APW says this must be a BUG! (APW just added the cast)
#endif
//	cnxtty_driver.table = cnxtty_table;
	cnxtty_driver.termios = cnxtty_termios;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)
	cnxtty_driver.termios_locked = cnxtty_termios_locked;
#endif

	{
		static const struct tty_operations cnxtty_ops = {
			.open            = cnxtty_open,
			.close           = cnxtty_close,
			.write           = cnxtty_write,
			.write_room      = cnxtty_write_room,
			.flush_buffer    = cnxtty_flush_buffer,
			.chars_in_buffer = cnxtty_chars_in_buffer,
			.unthrottle      = cnxtty_unthrottle,
			.set_termios     = cnxtty_set_termios,
			.hangup          = cnxtty_hangup
		};
		cnxtty_driver.ops = &cnxtty_ops;
	}

	/*  	DBP_TTY_DRIVER(&cnxtty_driver); */

	status = tty_register_driver(&cnxtty_driver);

	/*  	DBP_TTY_DRIVER(&cnxtty_driver); */

	/*  	DBP("tty_register_driver returns %d", status); */

	if (status) {
		goto cnxtty_init_end;
	}

//	printk ( "cnxtty_init pAdapter=%p\n", pAdapter ) ;
	pThisAdapter = pAdapter ;
//	(*cnxtty_driver.table)->driver_data = pAdapter ;

	returnValue = 0;

cnxtty_init_end:

	/*   	DBP("returns %d", returnValue); */

//	printk ( "cnxtty_init pAdapter=%p\n", pAdapter ) ;
	pThisAdapter = pAdapter ;
//	(*cnxtty_driver.table)->driver_data = pAdapter ;
	return returnValue;
}


void cnxtty_SER_XmitMsg ( PDATAPUMP_CONTEXT pDatapump, const char *buf, UINT16 count )
{
	struct tty_struct *tty = cnxtty_table[0];
	int numberToWrite;

	/* check space so we don't copy needlessly */
	numberToWrite = MIN(count, tty->receive_room);

	if (!numberToWrite) {
		DBP("ERROR no room in the receive buffer");
		return;
	}

	RECEIVE_BUF(&tty->ldisc)(tty, buf, 0, numberToWrite);

	return;
}
