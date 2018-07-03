/**************************************************************************************
Module:		ChipAL
File:		ChipALRemoteI.h
Function:	Public Definitions for ChipAL Remote Interface 
Description:                        	
			

			Copyright 1998 Rockwell International
***************************************************************************************
$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4_ATM_Driver/KernelModule/ChipALRemoteI.h $
$Revision: 2 $
$Date: 5/22/01 10:38a $
**************************************************************************************/

#ifndef _CHIPALREMOTEI_H_					//	FILE WRAPPER,

#define _CHIPALREMOTEI_H_					//	prevents multiple inclusions

#include "ChipALCommon.h"
#include "ChipALInterf.h"

typedef enum
{
    CHIPAL_BASIC_INTERFACE = 0,
    CHIPAL_CONFIG_INTERFACE,
    CHIPAL_LINK_INTERFACE
} CHIPAL_INTERFACE_TYPE_ENUM;

typedef struct CHIPAL_REMOTE_INF
{
	USHORT 				SizeofInterface;
	CHIPAL_INTERFACE_TYPE_ENUM   InterfaceType;
	CHIPAL_INTERFACE   	CALIntf;
} CHIPAL_REMOTE_INF, *PCHIPAL_REMOTE_INF;

typedef struct CHIPAL_CONFIG_INF_S
{
	USHORT 						SizeofInterface;
	CHIPAL_INTERFACE_TYPE_ENUM  InterfaceType;
	CHIPAL_CONFIG_INTERFACE_T	ConfigInterface;
}   CHIPAL_CONFIG_INF_T;


typedef struct CHIPAL_LINK_INF_S
{
	USHORT 						SizeofInterface;
	CHIPAL_INTERFACE_TYPE_ENUM  InterfaceType;
	CHIPAL_LINK_INTERFACE_T		LinkInterface;
}   CHIPAL_LINK_INF_T;


typedef struct CHIPAL_INF_S
{
	CHIPAL_INTERFACE   			CALIntf;
	CHIPAL_CONFIG_INTERFACE_T 	ConfigInterface;
	CHIPAL_LINK_INTERFACE_T		LinkInterface;
} CHIPAL_INF_T, *PCHIPAL_INF_T;



#endif
