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
#**		buildcnxadsl.sh
#**
#**	ABSTRACT:
#**		Shell script used to retrieve source files from Linux CVS system and
#**		generate Conexant ADSL Linux driver delivery RPM.
#**
#**	DETAILS:
#**		
#**
#******************************************************************************/

if [ -z "$CVSROOT" ]; then
   echo "You need a CVSROOT enviroment variable"
   exit 1;
fi

if [ -z "CVS_RSH" ]; then
   echo "CVS_RSH environment variable should be set to ssh"
   exit 1
fi

if [ -z "$1" ]; then
   echo "Usage:   buildcnxcvs.sh  <version>  <release>"
   echo "Example: buildcnxcvs.sh  1.5.3      1"
   exit 1;
fi
if [ -z "$2" ]; then
   echo "Usage:   buildcnxcvs.sh  <version>  <release>"
   echo "Example: buildcnxcvs.sh  1.5.3      1"
   exit 1;
fi

mkdir /tmp/cnx-cvs
cd /tmp/cnx-cvs
cvs co CnxADSL
find CnxADSL -name 'CVS' | xargs rm -rf

cd CnxADSL
sh buildcnxrpm.sh ${1} ${2}
