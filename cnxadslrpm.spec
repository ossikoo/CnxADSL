#/******************************************************************************
#********************************************************************************
#****	Copyright (c) 1997, 1998, 1999, 2000, 2001
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
#**		cnxadslrpm.spec
#**
#**	ABSTRACT:
#**		The RPM spec file used to build the Conexant ADSL modem
#**		driver installation. 
#**
#**	DETAILS:
#**
#******************************************************************************/

Summary: Conexant AccessRunner PCI ADSL Adapter driver
# The following labels must be the first occurance of label name in this file
# in order for the "buildcnxadsl.sh" script to automatically edit the field properly
Name: CnxADSL-TgrATM_k2.4.20-8
Version: 2.0.1
Release: 2

Copyright: Conexant Systems, Inc.
Group: System Environment/Kernel
Source: %{name}-%{version}.tar.gz
Vendor: Conexant Systems, Inc.
Packager: Conexant Systems, Inc.

%description
This is the driver for the Conexant AccessRunner ADSL modem.

%prep
%setup -q

%build
make all

%install
rm -rf /etc/Conexant
mkdir /etc/Conexant
make install
cd ..

%post
rm -f /usr/bin/cnxadslctl.sh
ln -s /etc/rc.d/init.d/cnxadslctl /usr/bin/cnxadslctl.sh
chkconfig --add cnxadslctl

echo "!!!!!!!  Run 'cnxadslconfig' to configure your vpi and vci  !!!!!!!"
echo "Then run 'cnxadslctl.sh start' if no reboot is required."
echo "FYI: These VPI and VCI parameters can be obtained from your provider"

%preun 
/usr/bin/cnxadslctl.sh stop
chkconfig --del cnxadslctl

%postun
rm -rf /etc/Conexant
rm -f  /usr/bin/cnxadslctl.sh
rm -f  /var/lock/subsys/cnxadslctl

%files
/etc/Conexant/cnxadslload
/etc/rc.d/init.d/cnxadslctl
/usr/bin/cnxadslconfig
/usr/bin/cnxadslstatus
/usr/bin/cnxadslautolog
/etc/Conexant/CnxTgF.hex
/etc/Conexant/cnxadsl.conf
/lib/modules/2.4.20-8/kernel/net/CnxADSL.o
