#!/bin/bash
#/******************************************************************************
#********************************************************************************
#****	Copyright (c) 2000, 2001
#****	Conexant Systems Inc. (formerly Rockwell Semiconductor Systems)
#****	Personal Computing Division
#****	All Rights Reserved
#****
#****	CONFIDENTIAL and PROPRIETARY --
#****		No Dissemination or use without prior written permission.
#****
#*******************************************************************************
#**
#**	FILE NAME:
#**		buildcnxrpm.sh
#**
#**	ABSTRACT:
#**		Shell script used to generate Conexant Yukon ADSL Linux ATM driver
#**		(for Linux kernel version 2.4.3) delivery RPM.
#**
#**	DETAILS:
#**		
#**
#******************************************************************************/
if [ -z "$1" ]; then
   echo "Usage:   buildcnxrpm.sh  <minor version>  <release>"
   echo "Example: buildcnxrpm.sh   006              2"
   exit 1;
fi
if [ -z "$2" ]; then
   echo "Usage:   buildcnxrpm.sh  <minor version>  <release>"
   echo "Example: buildcnxrpm.sh   006              2"
   exit 1;
fi

device="CnxADSL"
product="TgrATM_k2.4.20-8"
hwprodver="2.0.1."

sh buildcnxadsl.sh ${device} ${product} ${hwprodver}${1} ${2}
