#!/bin/bash
#
# List of commands to make sure driver is shut down:
#
#    (this command may take several seconds)
#    (this command should make the next three lines unnecessary)
if [ -f /etc/rc.d/init.d/cnxadslctl.sh ]; then
   /etc/rc.d/init.d/cnxadslctl.sh stop
fi
if [ -f /usr/bin/cnxadslctl.sh ]; then
   /usr/bin/cnxadslctl.sh stop
fi
#
echo "If the 'cnxadslctl.sh stop' command did not work,"
echo "try the following three lines."
echo "    'killall br2684ctl'"
echo "    'rmmod -s br2684'"
echo "    'rmmod -s CnxADSL'"
#
#
if [ -d  /etc/Conexant/ ]; then
   rm -rf /etc/Conexant/
fi
if [ -f /etc/rc.d/init.d/cnxadslctl.sh ]; then
   rm -f /etc/rc.d/init.d/cnxadslctl.sh
fi
if [ -f /usr/bin/cnxadslctl.sh ]; then
   rm -f /usr/bin/cnxadslctl.sh
fi
if [ -f /etc/rc.d/rc0.d/K??cnxadsl ]; then
   rm -f /etc/rc.d/rc0.d/K??cnxadsl
fi
if [ -f /etc/rc.d/rc1.d/K??cnxadsl ]; then
   rm -f /etc/rc.d/rc1.d/K??cnxadsl
fi
if [ -f /etc/rc.d/rc2.d/K??cnxadsl ]; then
   rm -f /etc/rc.d/rc2.d/K??cnxadsl
fi
if [ -f /etc/rc.d/rc3.d/S??cnxadsl ]; then
   rm -f /etc/rc.d/rc3.d/S??cnxadsl
fi
if [ -f /etc/rc.d/rc4.d/S??cnxadsl ]; then
   rm -f /etc/rc.d/rc4.d/S??cnxadsl
fi
if [ -f /etc/rc.d/rc5.d/S??cnxadsl ]; then
   rm -f /etc/rc.d/rc5.d/S??cnxadsl
fi
if [ -f /etc/rc.d/rc6.d/K??cnxadsl ]; then
   rm -f /etc/rc.d/rc6.d/K??cnxadsl
fi
if [ -f /etc/sysconfig/cnxadslvpivci.conf ]; then
   rm -f /etc/sysconfig/cnxadslvpivci.conf
fi
if [ -f /lib/modules/2.2.16/misc/CnxADSL.o ]; then
   rm -f /lib/modules/2.2.16/misc/CnxADSL.o
fi
if [ -f /lib/modules/2.4.0/misc/CnxADSL.o ]; then
   rm -f /lib/modules/2.4.0/misc/CnxADSL.o
fi
if [ -f /usr/bin/cnxadslconfig ]; then
   rm -f /usr/bin/cnxadslconfig
fi
if [ -f /usr/bin/cnxadslstatus ]; then
   rm -f /usr/bin/cnxadslstatus
fi
if [ -f /var/lock/subsys/cnxadsllock ]; then
   rm -f /var/lock/subsys/cnxadsllock
fi
if [ -f /sbin/vpi_vci.sh ]; then
   rm -f /sbin/vpi_vci.sh
fi
