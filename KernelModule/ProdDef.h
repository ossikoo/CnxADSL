/******************************************************************************
*******************************************************************************
****	Copyright (c) 1997, 1998
****	Rockwell Semiconductor Systems
****	Personal Computing Division
****	All Rights Reserved
****
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****
*******************************************************************************
**
**	MODULE NAME:
**		Athena
**
**	FILE NAME:
**		ProdDef.h
**
**	ABSTRACT:
**		This files contains defines for defining the project name and various
**		components/modules/sections of the project/device.
**
**	DETAILS:
**		The product's header file, "Project.h", should include this header file
**		so that the product's header file can define the product's name,
**		medium, protocol, etc. (i.e., PROJECTS_NAME, PROJECTS_MEDIUM,
**		PROJECTS_PROTOCOL, etc., respectively) to the appropriate assignment.
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/ProdDef.h $
** $Revision: 1 $
** $Date: 1/09/01 10:53a $
*******************************************************************************
******************************************************************************/
#ifndef _PRODDEF_H_
#define _PRODDEF_H_



///////////////////////////////////////////////////////
//	Project Names		0x0101 through 0x01FF
//		PROJECTS_NAME should be set to one of the
//		following defines in the product's "Product.h"
//		header file.
///////////////////////////////////////////////////////

#define PROJECT_NAMES			0x0100

#define PROJECT_NAME_ATHENA			PROJECT_NAMES + 1	//	Original Reference Design Driver

#define PROJECT_NAME_CHEETAH_1		PROJECT_NAMES + 2	//	Compaq OEM with Alcatel ADSL
//	on PCI Basic2.
#define PROJECT_NAME_TESTAPP_ATHENA	PROJECT_NAMES + 3	//  Test App for Athena.

#define PROJECT_NAME_TESTAPP_CHEETAH_1	PROJECT_NAMES +4 // Test App for Cheetah_1

#define PROJECT_NAME_TIGRIS_PROTO_1	PROJECT_NAMES + 5	// Tigris Prototype Board (Rev 1)

#define PROJECT_NAME_TIGRIS			PROJECT_NAME_TIGRIS_PROTO_1 +1 // Tigris Prototype Board (Rev 1)

#define PROJECT_NAME_MADMAX			(PROJECT_NAME_TIGRIS + 1)

#define PROJECT_NAME_NILE_TEST      (PROJECT_NAME_MADMAX + 1)       // Nile Test Driver (Altera FPGA)

#define PROJECT_NAME_WDM_FILTER      PROJECT_NAME_NILE_TEST+1

///////////////////////////////////////////////////////
//	Project Mediums		0x0201 through 0x02FF
//		PROJECTS_MEDIUM should be set to one of the
//		following defines in the product's "Product.h"
//		header file.
///////////////////////////////////////////////////////

#define PROJECT_MEDIUMS			0x0200

#define PROJECT_MEDIUM_CDSL			PROJECT_MEDIUMS + 1

#define PROJECT_MEDIUM_ADSL			PROJECT_MEDIUMS + 2

#define PROJECT_MEDIUM_ISDN			PROJECT_MEDIUMS + 3



///////////////////////////////////////////////////////
//	Project Protocols	0x0301 through 0x03FF
//		PROJECTS_PROTOCOL should be set to one of the
//		following defines in the product's "Product.h"
//		header file.
///////////////////////////////////////////////////////

#define PROJECT_PROTOCOLS		0x0300

#define PROJECT_PROTOCOL_HDLC		PROJECT_PROTOCOLS + 1

#define PROJECT_PROTOCOL_ATM		PROJECT_PROTOCOLS + 2

#define PROJECT_PROTOCOL_ISDN		PROJECT_PROTOCOLS + 3



///////////////////////////////////////////////////////
//	Project NDIS Interfaces	0x0401 through 0x04FF
//		PROJECTS_NDIS_IF should be set to one of the
//		following defines in the product's "Product.h"
//		header file.
///////////////////////////////////////////////////////

#define PROJECT_NDIS_IFS		0x0400

#define PROJECT_NDIS_IF_WAN			PROJECT_NDIS_IFS + 1

#define PROJECT_NDIS_IF_LAN			PROJECT_NDIS_IFS + 2

#define PROJECT_NDIS_IF_CO			PROJECT_NDIS_IFS + 3


//*
//* 	Windows Versions are in sequential order.  Winnt is greater than any Win9x version
//*
#define WIN_IF_WIN9X				0x00010000
#define WIN_IF_NTX					0x00020000

#define WIN_IF_W98_GM				(WIN_IF_WIN9X 	| 0x00000001)
#define WIN_IF_W98_SP				(WIN_IF_WIN9X 	| 0x00000002)  /* Requires W2k Beta 3 DDK or better */
#define WIN_NT_4_SP4				(WIN_IF_NTX		| 0x00000001)
#define WIN_2K						(WIN_IF_NTX		| 0x00000002)

///////////////////////////////////////////////////////
//
// Registry Keys
//
///////////////////////////////////////////////////////

#define REG_CLASS_KEY_STRING					_T("System\\CurrentControlSet\\Services\\Class\\")
#define REG_ENUM_KEY_STRING						_T("Enum\\")
#define REG_ENUM_PCI_KEY_STRING					_T("Enum\\PCI\\")

#define REG_ENUM_NETWORK_KEY_STRING				_T("Enum\\Network\\")
#define REG_MSTCP_KEYNAME_STRING				_T("MSTCP")
#define REG_BINDINGS_KEYNAME_STRING				_T("Bindings")

#define REG_WINDOWS_EXPLORER_KEY_STRING			_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\")
#define REG_SHELL_FOLDERS_KEYNAME_STRING		_T("Shell Folders")

#define REG_WINDOWS_INSTALLED_FILES_STRING		_T("System\\CurrentControlSet\\Control\\InstalledFiles")

#define REG_INSTALL_LOCATIONS_MRU_KEY_STRING	_T("InstallLocationsMRU\\")


#define REG_REGISTER_CONTROL_PANEL_APPLET_KEY_STRING	_T("Software\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\Cpls\\")	// Win2000 Only
#define REG_WINDOWS_SETUP_KEY_STRING			_T("Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\")
#define REG_WINDOWS_SETUPX_CATALOG_KEY_STRING	_T("Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\SetupX\\Catalogs\\")		// Win9x Only
#define REG_WINDOWS_INF_OEM_NAME_KEY_STRING		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\SetupX\\INF\\OEM Name\\")	// Win9x Only
#define REG_WINDOWS_UNINSTALL_KEY_STRING		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\")
#define REG_WINDOWS_SOUND_PROPERTY_KEY_STRING	_T("AppEvents\\Schemes\\Apps\\ConexantADSL\\")
//
// Win9x TCP/IP Keys
//
#define REG_SERVICES_VXD_MSTCP_KEY_STRING		_T("System\\CurrentControlSet\\Services\\VxD\\MSTCP\\")


//
// Win2000 TCP/IP Keys
//
#define REG_SERVICES_TCPIP_KEY_STRING			_T("System\\CurrentControlSet\\Services\\Tcpip\\")
#define REG_PARAMETERS_KEYNAME_STRING			_T("Parameters")
#define REG_INTERFACES_KEYNAME_STRING			_T("Interfaces")


//
// The following registry keys are used by RegistryUtilities.cpp

#define REG_TAPI_DEVICES_KEY_STRING				_T("SOFTWARE\\Microsoft\\RAS\\TAPI DEVICES")
#define REG_SYSTEM_SELECT_KEY_STRING			_T("SYSTEM\\Select")
#define REG_CONTROLSET001_SERVICES_KEY_STRING	_T("SYSTEM\\ControlSet001\\Services")
#define REG_CONTROLSET00_KEY_STRING				_T("SYSTEM\\ControlSet00")
#define REG_RESOURCEMAP_OTHERDRIVERS_KEY_STRING	_T("HARDWARE\\ResourceMap\\OtherDrivers\\")
#define REG_CURRENTCONTROLSET_SERVICES_KEY_STRING	_T("SYSTEM\\CurrentControlSet\\Services\\")
#define REG_DSL_DESCRIPTION_KEY_STRING			_T("System\\CurrentControlSet\\Services\\Class\\Net")
#define REG_MODEM_DESCRIPTION_KEY_STRING		_T("System\\CurrentControlSet\\Services\\Class\\Modem")
#define REG_DSL_W2000_DESCRIPTION_KEY_STRING	_T("System\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}")
#define REG_WINDOWS_CURRENTVERSION_KEY_STRING	_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion")

///////////////////////////////////////////////////////
//
// Registry Values
//
///////////////////////////////////////////////////////

#define REG_DEVICE_DRIVER_VALUE_STRING			_T("DeviceDriver")

// Values found under subkeys of "\Enum\"
#define REG_DRIVER_VALUE_STRING					_T("Driver")

// Values found under driver subkeys of the following keys:
// Win9x	-> "\System\CurrentControlSet\Services\Class\"
// Win2000	-> "\System\CurrentControlSet\Control\Class\"
#define REG_DEVICE_DESCRIPTION_VALUE_STRING		_T("DeviceDesc")
#define REG_MATCHING_DEVICE_ID_VALUE_STRING		_T("MatchingDeviceId")

// Win2000 values found under driver subkeys of "HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Class\"
#define REG_NETCFG_INSTANCE_ID_VALUE_STRING		_T("NetCfgInstanceId")

// TCP/IP Values found under the following keys:
// Win9x	-> "HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\Class\NetTrans\xxxx\"
// Win2000	-> "HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\Tcpip\Parameters\Interfaces\{xxxx}\"
#define REG_IP_ADDRESS_VALUE_STRING				_T("IPAddress")
#define REG_IP_MASK_VALUE_STRING				_T("IPMask")			// Win9x Only
#define REG_SUBNET_MASK_VALUE_STRING			_T("SubnetMask")		// Win2000 Only
#define REG_ENABLE_DHCP_VALUE_STRING			_T("EnableDHCP")		// Win2000 Only
#define REG_DEFAULT_GATEWAY_VALUE_STRING		_T("DefaultGateway")

#define REG_NODE_TYPE_VALUE_STRING				_T("NodeType")

// DNS Values
#define REG_ENABLE_DNS_VALUE_STRING				_T("EnableDNS")
#define REG_DOMAIN_VALUE_STRING					_T("Domain")
#define REG_HOST_NAME_VALUE_STRING				_T("HostName")			// Win9x Only
#define REG_HOSTNAME_VALUE_STRING				_T("Hostname")			// Win2000 Only
#define REG_NAME_SERVER_VALUE_STRING			_T("NameServer")
#define REG_SEARCH_LIST_VALUE_STRING			_T("SearchList")

// Values found under "HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders\"
#define REG_APP_DATA_VALUE_STRING				_T("AppData")		// example: "C:\Windows\Profiles\Username\Application Data"
#define REG_CACHE_VALUE_STRING					_T("Cache")			// example: "C:\Windows\Temporary Internet Files"
#define REG_COOKIES_VALUE_STRING				_T("Cookies")		// example: "C:\Windows\Cookies"
#define REG_DESKTOP_VALUE_STRING				_T("Desktop")		// example: "C:\Windows\Desktop"
#define REG_FAVORITES_VALUE_STRING				_T("Favorites")		// example: "C:\Windows\Favorites"
#define REG_FONTS_VALUE_STRING					_T("Fonts")			// example: "C:\Windows\Fonts"
#define REG_HISTORY_VALUE_STRING				_T("History")		// example: "C:\Windows\History"
#define REG_NET_HOOD_VALUE_STRING				_T("NetHood")		// example: "C:\Windows\NetHood"
#define REG_PERSONAL_VALUE_STRING				_T("Personal")		// example: "C:\My Documents"
#define REG_PRINT_HOOD_VALUE_STRING				_T("PrintHood")		// example: "C:\Windows\PrintHood"
#define REG_PROGRAMS_VALUE_STRING				_T("Programs")		// example: "C:\Windows\Start Menu\Programs"
#define REG_RECENT_VALUE_STRING					_T("Recent")		// example: "C:\Windows\Recent"
#define REG_SEND_TO_VALUE_STRING				_T("SendTo")		// example: "C:\Windows\SendTo"
#define REG_START_MENU_VALUE_STRING				_T("Start Menu")	// example: "C:\Windows\Start Menu"
#define REG_STARTUP_VALUE_STRING				_T("Startup")		// example: "C:\Windows\Start Menu\Programs\StartUp"
#define REG_TEMPLATES_VALUE_STRING				_T("Templates")		// example: "C:\Windows\Templates"


// Values found under "HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders"
#define REG_COMMON_DESKTOP_VALUE_STRING			_T("Common Desktop")
#define REG_COMMON_STARTUP_VALUE_STRING			_T("Common Startup")

// Values found under "HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\"
#define REG_PROGRAM_FILES_PATH_VALUE_STRING		_T("ProgramFilesPath")
#define REG_MEDIA_VALUE_STRING					_T("MediaPath")			// example: "C:\Windows\Media"

// Values found under subkeys of "HKEY_CURRENT_USER\InstallLocationsMRU\"
#define REG_MRU_LIST_VALUE_STRING				_T("MRUList")

// Values found under subkeys of "HKEY_CURRENT_USER\AppEvents\Schemes\Apps\ConexantADSL\"
#define REG_SOUND_CONNECT_STRING				_T("Connected\\.Current")
#define REG_SOUND_DISCONNECT_STRING				_T("Disconnected\\.Current")


//
//	Help file names for the AccessRunner
//

#define WINDOWS_HELP_DIR_NAME		_T("Help")		// Name of the help directory, a subdirectory
//  of the Windows root directory
//  (e.g.,	root dir="C:\Windows",
//			help subdir="Help").
#define WINDOWS_HELP_EXE_NAME		_T("hh.exe")	// Name of the HTML Help executable file.



#endif		//#ifndef _PRODDEF_H_

