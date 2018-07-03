/******************************************************************************
*******************************************************************************
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
**		Util (Utilities)
**
**	FILE NAME:
**		UtilDbg.h
**
**	ABSTRACT:
**		Adapter debugging aid definiations
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux 2.4 ATM Driver/KernelModule/UtilDbg.h $
** $Revision: 4 $
** $Date: 2/28/01 11:05a $
*******************************************************************************
******************************************************************************/
#ifndef _UTILDBG_H_		//	File Wrapper,
#define _UTILDBG_H_		//	prevents multiple inclusions

#ifdef OS_LINUX
#include <linux/kernel.h>
#endif


///////////////////////////////////////////////////////////////////

#ifdef DBG
//	define BREAKPOINT	_asm int 3

#	ifndef MODULE_ID
#		define	MODULE_ID		DEBUG_MODULE_ID_MASK
#	endif

#	if defined(OS_LINUX)					// Linux

#		define	DBG_BREAKPOINT	DbgBreakPoint()
#		define	DBG_DISPLAY(S...) printk( S )
#		define	DBG_CDSL_BREAKPOINT(DebugLvl,DebugFlagSettings)

#	elif defined(OS_WIN_95)					// OS_WIN_95, added for Win95 WAN for Tigris-KT

#		define	DBG_BREAKPOINT
#		define	DBG_DISPLAY(S)
#		define	DBG_CDSL_BREAKPOINT(DebugLvl,DebugFlagSettings)

#	else									// NOT Linux and NOT Win95

#		define	DBG_BREAKPOINT	DbgBreakPoint()
#		define	DBG_DISPLAY(S)	{ DbgPrint S;}		//	S - is a parenthesised printf string
#		define	DBG_CDSL_BREAKPOINT(DebugLvl,DebugFlagSettings) \
				{ \
					if ( DebugLvl == DBG_LVL_GENERIC ) \
					{ \
						ASSERTMSG( "CnxAdslS Driver:  Generic Breakpoint", FALSE ); \
					} \
					else if ( (DebugFlagSettings & DBG_ENA_BREAKPOINTS) != 0 ) \
					{ \
						if ( DebugLvl == DBG_LVL_MISC_INFO ) \
						{ \
							if ( (DebugFlagSettings & DBG_ENA_MISC_BRK) != 0 ) \
							{ \
								ASSERTMSG( "CnxAdslS Driver:  Misc./Info. Breakpoint.", FALSE ); \
							} \
						} \
						else if ( DebugLvl == DBG_LVL_WARNING ) \
						{ \
							if ( (DebugFlagSettings & DBG_ENA_WARNING_BRK) != 0 ) \
							{ \
								ASSERTMSG( "CnxAdslS Driver:  Warning Breakpoint!", FALSE ); \
							} \
							else \
							{ \
								ASSERTMSG( "DebugFlag => Warning Breakpoints Disabled!", FALSE ); \
							} \
						} \
						else if ( DebugLvl == DBG_LVL_ERROR ) \
						{ \
							if ( (DebugFlagSettings & DBG_ENA_ERROR_BRK) != 0 ) \
							{ \
								ASSERTMSG( "CnxAdslS Driver:  Error Breakpoint!", FALSE ); \
							} \
							else \
							{ \
								ASSERTMSG( "DebugFlag => Error Breakpoints Disabled!", FALSE ); \
							} \
						} \
						else if ( DebugLvl == DBG_LVL_DRVR_ENTRY ) \
						{ \
							if ( (DebugFlagSettings & DBG_ENA_DRVR_ENTRY_BRK) != 0 ) \
							{ \
								DBG_BREAKPOINT; \
							} \
						} \
					} \
					else if ( DebugLvl == DBG_LVL_WARNING ) \
					{ \
						ASSERTMSG( "DebugFlag => Breakpoints Disabled!", FALSE ); \
					} \
					else if ( DebugLvl == DBG_LVL_ERROR ) \
					{ \
						ASSERTMSG( "DebugFlag => Breakpoints Disabled!", FALSE ); \
					} \
				}
#	endif

#	if defined(OS_LINUX)					// Linux
#		define	DBG_PRINT	printk
#	else									// NOT Linux
#		define	DBG_PRINT	DbgPrint
#		define	KERN_EMERG	
#		define	KERN_ALERT	
#		define	KERN_CRIT	
#		define	KERN_ERR	
#		define	KERN_WARNING
#		define	KERN_NOTICE	
#		define	KERN_INFO	
#		define	KERN_DEBUG	
#	endif

#	define	DBG_CDSL_DISPLAY(DebugLvl,DebugFlagSettings,DebugString)				\
			{																		\
				if ( (DebugFlagSettings & MODULE_ID) != 0 )							\
				{																	\
					if ( DebugLvl == DBG_LVL_GENERIC )								\
					{																\
						DBG_PRINT( "-- CDSL Driver Generic:  " );					\
						DBG_PRINT DebugString ;										\
						DBG_PRINT( "\n" );											\
					}																\
					else if ( (DebugFlagSettings & DBG_ENA_MESSAGES) != 0 )			\
					{																\
						if ( DebugLvl == DBG_LVL_MISC_INFO )						\
						{															\
							if ( (DebugFlagSettings & DBG_ENA_MISC_MSG) != 0 )		\
							{														\
								DBG_PRINT( "-- CDSL Driver Misc.:  " );				\
								DBG_PRINT DebugString ;								\
								DBG_PRINT( "\n" );									\
							}														\
						}															\
						else if ( DebugLvl == DBG_LVL_WARNING )						\
						{															\
							if ( (DebugFlagSettings & DBG_ENA_WARNING_MSG) != 0 )	\
							{														\
								DBG_PRINT( "!! CDSL Driver Warning:  " );			\
								DBG_PRINT DebugString ;								\
								DBG_PRINT( "\n" );									\
							}														\
						}															\
						else if ( DebugLvl == DBG_LVL_ERROR )						\
						{															\
							if ( (DebugFlagSettings & DBG_ENA_ERROR_MSG) != 0 )		\
							{														\
								DBG_PRINT( "** CDSL Driver Error:  " );				\
								DBG_PRINT DebugString ;								\
								DBG_PRINT( "\n" );									\
							}														\
						}															\
					}																\
				}																	\
			}

#	define	DBG_CDSL_DISPLAY_FULL(DebugFlagSettings,DebugString)	\
			{ 														\
				if ( (DebugFlagSettings & MODULE_ID) != 0 )			\
				{													\
					DBG_PRINT( __FILE__ " Line #%d : ", __LINE__ );	\
					DBG_PRINT DebugString ;							\
					DBG_PRINT( "\n" );								\
				}													\
			}

#	define	ErrPrt(a...) DBG_PRINT(a)

#else

#	define	DBG_BREAKPOINT
#	define	DBG_DISPLAY(S...)
#	define	DBG_CDSL_BREAKPOINT(DebugLvl,DebugFlagSettings)
#	define	DBG_CDSL_DISPLAY(DebugLvl,DebugFlagSettings,DebugString)
#	define	DBG_CDSL_DISPLAY_FULL(DebugFlagSettings,DebugString)

#	define	ErrPrt(a...)

#endif


#define	DBG_LVL_NO_DEBUG			0x0
#define	DBG_LVL_GENERIC				0x1
#define	DBG_LVL_MISC_INFO			0x2
#define	DBG_LVL_WARNING				0x3
#define	DBG_LVL_ERROR				0x4
#define	DBG_LVL_DRVR_ENTRY			0x5


#define	DBG_FLAG_DONT_CARE			0xFFFFFFFF


//
//	The DebugFlag bit fields are defined in TapiComm.h
//	so that the control panel applet and/or others may have access
//	to the bit fields definitions also.
//


#endif		//#ifndef _UTILDBG_H_
