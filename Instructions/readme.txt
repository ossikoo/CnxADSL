RELEASE NOTES FOR CONEXANT ADSL TIGRIS/YUKON LINUX ATM DRIVERS
VERSIONS: 6L.2.6.006 and 18L.2.6.006      10-April-2001
================================================================


Please see instructions in file "instructions.txt" for:
 - information on which Linux kernel version is supported by this driver,
 - upgrading the Linux kernel,
 - installing the driver, and
 - configuring the driver.



LIMITATIONS:
============

1.  This driver has only been tested in LAN (RFC 1483/2684) mode only.
    In WAN mode (PPP over ATM, RFC 2364), there are issues with the Linux
    "pppoatm" kernel module that have been acknowledged by its author.
    The issue is _not_ in the driver.  We are actively attempting to
    determine how to work around those issues, in cooperation with Red Hat.
    However, we do not yet have a date for resolution of this issue.

2.  Limited testing of the driver has been performed on different PCs.
    It is possible that system dependencies still exist that could cause
    problems on certain PCs.

3.  PINGs longer than about 16k bytes will fail.  This problem is
    under investigation.



RELEASE NOTES:
==============
Changes/Fixes since release 1.4.1:

1.  Tigris and Yukon reference designs are now supported, for both the
    Linux 2.4.0 and 2.2.16-4 kernels.     
2.  Revamped version numbering system as follows:  p.h.v.zzz-z

	Field	Meaning
	-----	--------
	
	p	Product Line (See Version Identification.doc for full details)
		6 = Tigris ATM (a.k.a. CO)
		18 = Yukon-AV ATM (a.k.a. CO)
	h	Hardware Version (See Version Identification.doc for full details)
		1 = P46 (Tigris)
		2 = P51 (Yukon)
	v	Software Major version 
	bbb	3 digit build number incremented each external release
	
3.  Added new cnxadslstatus utiltity
4.  Renamed scripts/utilities to be more consistent as follows
	vpi_vci		>>> vpi_vci.sh
	cnxadsl		>>> cnxadslctl.sh
	cnxadsl.spec	>>> cnxadslrpm.spec
	cnxadsld	>>> cnxadslload
	cnxconfig	>>> cnxadslconfig
5.  Fixed problem in autosensing where G.HS was falsely detected in some cases.
6.  Minor DSLAM compatibility/performance enhancments.
7.  Fix reading of MAC address from EEPROM for Yukon
8.  Fix sporadic panic halt upon enterring showtime
9.  Fix problem with G.HS training badly unless ANSI has been run at least once.
10. Made Tip/Ring the default to eliminate problem in auto line selection, since 
    auto select relays not being used by customers
11. The ADSL datapump code has been upgraded to the equivalent of Windows
    driver version 092.
11. Several system-dependent timing problems and other minor problems were
    resolved, that could sometimes cause lock-ups with the previous version.
12. Instructions updated and corrected.