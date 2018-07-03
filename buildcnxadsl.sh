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
#**		Shell script used to generate Conexant ADSL Linux driver delivery RPM.
#**
#**	DETAILS:
#**		
#**
#******************************************************************************/

# for debugging
# set -o xtrace

if [ -z "$1" ]; then
   echo "Usage:   buildcnxadsl.sh  <device>  <product>      <version>  <release>"
   echo "Example: buildcnxadsl.sh  CnxADSL   TgrATM_k2.4.0  1.5.3      1"
   exit 1;
fi
if [ -z "$2" ]; then
   echo "Usage:   buildcnxadsl.sh  <device>  <product>      <version>  <release>"
   echo "Example: buildcnxadsl.sh  CnxADSL   TgrATM_k2.4.0  1.5.3      1"
   exit 1;
fi
if [ -z "$3" ]; then
   echo "Usage:   buildcnxadsl.sh  <device>  <product>      <version>  <release>"
   echo "Example: buildcnxadsl.sh  CnxADSL   TgrATM_k2.4.0  1.5.3      1"
   exit 1;
fi
if [ -z "$4" ]; then
   echo "Usage:   buildcnxadsl.sh  <device>  <product>      <version>  <release>"
   echo "Example: buildcnxadsl.sh  CnxADSL   TgrATM_k2.4.0  1.5.3      1"
   exit 1;
fi

# make scripts executable since attributes are lost converting
# from windows to Linux
chmod a+x cnxadslctl.sh
chmod a+x buildcnxcvs.sh
chmod a+x buildcnxrpm.sh
chmod a+x buildcnxadsl.sh
chmod a+x FilterLog.sh

# edit readme text file, "readme.txt", change Version to version
# specified by this script's parameter
cat Instructions/readme.txt \
        | sed "s/VERSION == .*/VERSION == ${3}/" \
        > Instructions/readmetmp1.txt
rm -f Instructions/readme.txt
mv Instructions/readmetmp1.txt Instructions/readme.txt

# edit RPM script/spec file, "cnxadslrpm.spec", to use the
# device-product name, version, and release specified
# by this script's parameters
cat cnxadslrpm.spec \
        | sed "s/Name: .*/Name: ${1}-${2}/" \
        > cnxadslrpmtmp1.spec
cat cnxadslrpmtmp1.spec \
        | sed "s/Version: .*/Version: ${3}/" \
        > cnxadslrpmtmp2.spec
cat cnxadslrpmtmp2.spec \
        | sed "s/Release: .*/Release: ${4}/" \
        > cnxadslrpmtmp3.spec
rm -f cnxadslrpm.spec
mv cnxadslrpmtmp3.spec cnxadslrpm.spec
rm -f cnxadslrpmtmp?.spec
cp cnxadslrpm.spec /usr/src/redhat/SPECS/cnxadslrpm.spec

#make temporary build directory and change to it
mkdir -p /tmp/cnx-$$
cp -r . /tmp/cnx-$$/CnxADSL
cd /tmp/cnx-$$;

# rename temporary build directory to match release
mv CnxADSL ${1}-${2}-${3}

# create restricted source tarball and copy to permanent folder
tar cvfz ${1}-${2}-${3}.dpcontroller.tar.gz ${1}-${2}-${3}/KernelModule/DpController
mv ${1}-${2}-${3}.dpcontroller.tar.gz /usr/src/redhat/SOURCES

# make object modules released (module source files not released)
cd ${1}-${2}-${3}
make modules
cd ..

# create source tarball and copy to permanent folder
tar cvfz ${1}-${2}-${3}.tar.gz ${1}-${2}-${3}
mv ${1}-${2}-${3}.tar.gz /usr/src/redhat/SOURCES

# remove the temporary folder
rm -rf ${1}-${2}-${3}
cd /usr/src/redhat/SPECS
rm -rf /tmp/cnx-$$
rm -rf /tmp/cnx-cvs

#create the RPM
rpmbuild -ba /usr/src/redhat/SPECS/cnxadslrpm.spec
