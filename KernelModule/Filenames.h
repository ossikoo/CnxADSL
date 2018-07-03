/******************************************************************************
****	
****	Copyright © 1999, 2000
****	Conexant Systems Inc.
****	All Rights Reserved
****	
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****	
****	Technical Contact:
****		Conexant Systems Inc.
****		Personal Computing Division
****		Huntsville Design Center
****		6703 Odyssey Drive, Suite 303
****		Huntsville, AL   35806
****	
*******************************************************************************

MODULE NAME:
	Product Filenames	

FILE NAME:
	Filenames.h

ABSTRACT:
	This file contains a list of all filenames for manual install found on Disk 1
	
DETAILS:

KEYWORDS:
	$Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/Filenames.h $
	$Revision: 1 $
	$Date: 1/09/01 10:54a $

******************************************************************************/


#ifndef _FILENAMES_H_
#define _FILENAMES_H_



//*********************************************************
//				D I S K   1   F I L E S
//*********************************************************


#define ATM_CELLA_FILENAME				_T("AtmCellA.bin")
#define ATM_CELLB_FILENAME				_T("AtmCellB.bin")

#define ALCATEL_CHEETAH_BIN_FILENAME 	_T("A20136.000")	
#define ALCATEL_CHEETAH_BINB_FILENAME 	_T("A20136B.000")

#define ALCATEL_DUCATI_BIN_FILENAME	 	_T("A20136.001")	
#define ALCATEL_DUCATI_BINB_FILENAME 	_T("A20136B.001")	

#define CPL_APPLET_FILENAME				_T("CnxAdslc.cpl")
#define CSA_CPL_FILENAME				_T("csacpl.cpl")	

#define CAT_FILENAME					_T("cl201003.cat")

#define CTY_FILENAME					_T("cpqdsl.cty")
#define CTY_DCT_FILENAME				_T("cpq1602.cty")

#define HELP_FILENAME					_T("CnxAdslH.chm")
#define INSTALL_WIZARD_HELP_FILENAME	_T("CnxWzHlp.chm")

// This file should reside
// on the installation disk.
// It contains the number of
// bytes of disk space needed
// for installation.
#define D1_SIZE_FILENAME				_T("size.dat")	


// This is the name of the installation DLL
// that is used when finding
// the full installation path.
#define INSTALL_API_FILENAME			_T("CnxDslWz.dll")

// This is the name of the installation DLL
// INI file that provides custom configuration
// information.
#define INSTALL_API_INI_FILENAME		_T("CnxWzCfg.ini")

// This is the name of the installation DLL
// INS file that provides custom DSL line
// configuration information.
#define INSTALL_API_INS_FILENAME		_T("*.ins")

// This is the name of the installation DLL
// Generic WAN INS file that provides custom DSL line
// configuration information.
#define INSTALL_WAN_INS_FILENAME		_T("CnxWzWan.ins")

// This is the name of the installation DLL
// Generic LAN INS file that provides custom DSL line
// configuration information.
#define INSTALL_LAN_INS_FILENAME		_T("CnxWzLan.ins")


#define HCFCSA_FILENAME					_T("hcfcsa.dll")	
#define HCFCSA32_FILENAME				_T("hcfcsa32.dll")
#define HCF_READER_FILENAME				_T("hcfreadr.dll")
#define EE_REPROGRAM_DLL_FILENAME		_T("eedll.dll")
#define UNINSTALL_API_DLL_FILENAME		_T("Cnxuninst.dll")
#define HWIO_DLL_FILENAME				_T("CnxHwIo.dll")	
#define HWIO_DLL_SHORT_FILENAME			_T("CnxHwIo")	

#define HC_EXE_FILENAME					_T("hc.exe")		
#define UNINSTALL_FILENAME				_T("cnxunist.exe")

#define ADSL_PARENT_INF_FILENAME		_T("cnxadsl.inf")	
#define ADSL_CHILD_INF_FILENAME			_T("cnxnet.inf")	
#define MODEM_CHILD_INF_FILENAME		_T("cpqdsl.inf")	
#define NT40_INF_FILENAME				_T("oemsetnt.inf")	


#define RBF_FILENAME					_T("hobbes.rbf")	

#define NDIS_DRIVER_FILENAME			_T("cnxadsls.sys")
#define NDIS_MP_DRIVER_FILENAME			"CnxAdslS.sys"
#define NDIS_EXTERNAL_DRIVER_NAME		"CnxAdslS"
#define NDIS_EXTERNAL_DRIVER_NAME_WC 	L"CnxAdslS"

#define NDIS_DRIVER_LINK_NAME			"\\\\.\\CnxAdslS"


#define ENUMERATOR_FILENAME				_T("HCFpnp.vxd")	
#define DPAL_VXD_FILENAME				_T("dpal.vxd")	
#define ROKMOSD_VXD_FILENAME			_T("rokkmosd.vxd")
#define V42_VXD_FILENAME				_T("rokv42.vxd")	
#define TURBO_VBF_FILENAME				_T("turbovbf.vxd")
#define TURBO_VCD_FILENAME				_T("turbovcd.vxd")
#define WIN95AC_VXD_FILENAME			_T("win95ac.vxd")	


//*********************************************************
//						T I G R I S
//*********************************************************


#define TIGRIS_NDIS_DRIVER_FILENAME			_T("CnxTgN.sys")
#define TIGRIS_NDIS_MP_DRIVER_FILENAME		"CnxTgN.sys"
#define TIGRIS_NDIS_EXT_DRIVER_NAME			"CnxTgN"
#define TIGRIS_NDIS_EXT_DRIVER_NAME_WC		L"CnxTgN"
#define TIGRIS_NDIS_DRIVER_LINK_NAME		"\\\\.\\CnxTgN"
#define TIGRIS_P46ARB_DRIVER_FILENAME		_T("CnxTgR.sys")
#define TIGRIS_P46ARM_HEX_FILENAME			_T("CnxTgF.hex")
#define TIGRIS_ADSL_INF_FILENAME			_T("Cxt1611C.inf")
#define TIGRIS_P46ARB1_INF_FILENAME			_T("Cxt1610R.inf")
#define TIGRIS_P46ARB2_INF_FILENAME			_T("Cxt1611R.inf")
#define TIGRIS_ADSL_CAT_FILENAME			_T("4342???c.cat")
#define TIGRIS_P46ARB1_CAT_FILENAME			_T("4340???r.cat")
#define TIGRIS_P46ARB2_CAT_FILENAME			_T("4342???r.cat")
#define TIGRIS_P46ARB_WDM_FILENAME			"CnxTgR.sys"
#define TIGRIS_P46ARB_DRIVER_NAME			"CnxTgR"
#define	TIGRIS_P46ARB_DRIVER_NAME_WC		L"CnxTgR"
#define FIRMWARE_FILE_TIGRIS				"CnxTgF.hex"

#define TIGRIS_NILE_WDM_FILENAME			"P46Nile.sys"
#define TIGRIS_NILE_DRIVER_NAME				"P46Nile"
#define	TIGRIS_NILE_DRIVER_NAME_WC			L"P46Nile"

#define TIGRIS_WDM_FILTER_FILENAME			"CnxTgW.sys"
#define TIGRIS_WDM_FILTER_NAME				"CnxTgW"
#define	TIGRIS_WDM_FILTER_NAME_WC			L"CnxTgW"


//*********************************************************
//					T I G R I S \ H C F
//*********************************************************

#define TIGRIS_HCF_ARB1_INF_FILENAME		_T("Cxt1612R.inf")
#define TIGRIS_HCF_MODEM_ENUM_INF_FILENAME	_T("Cxt1612E.inf")
#define TIGRIS_HCF_MODEM_INF_FILENAME		_T("Cxt1612C.inf")
#define TIGRIS_HCF_ARB2_INF_FILENAME		_T("Cxt1613R.inf")
#define TIGRIS_HCF_ADSL_INF_FILENAME		_T("Cxt1613C.inf")
#define TIGRIS_HCF_ADSL_CAT_FILENAME		_T("1613???c.cat")
#define TIGRIS_HCF_P46ARB1_CAT_FILENAME		_T("1612???r.cat")
#define TIGRIS_HCF_P46ARB2_CAT_FILENAME		_T("1613???r.cat")
#define TIGRIS_HCF_MODEM_CAT_FILENAME		_T("1612???c.cat")
#define TIGRIS_HCF_MODEM_ENUM_CAT_FILENAME	_T("1612???e.cat")

#define TIGRIS_HCF_MODEM_DRIVER_FILENAME	_T("twinacnt.sys")

//*********************************************************
//					M I C R O S O F T
//*********************************************************

#define MICROSOFT_CCPORT_SYS_FILENAME		_T("ccport.sys")
#define MICROSOFT_WDMMDMLD_VXD_FILENAME		_T("wdmmdmld.vxd")

//*********************************************************
//			D I S K   1   C A B I N E T   F I L E S
//*********************************************************
#define	PRODUCTS_CABINET_FILE				_T("CnxDslCb.cab")


//*********************************************************
//		T I G R I S   E N G I N E E R I N G   T O O L S
//*********************************************************

#define TIGRIS_NDIS_DRIVER_FILES			_T("CnxTgN.???")
#define TIGRIS_P46ARBIT_DRIVER_FILES		_T("CnxTgR.???")
#define TIGRIS_P46ARM_FW_DEBUG_FILES		_T("CnxTgF.???")
#define TIGRIS_SERIAL_DRIVER_FILES			_T("VxComms.???")
#define TIGRIS_SERIAL_DRIVER_FILE			_T("VxComms.sys")
#define TIGRIS_SERIAL_W98_INF_FILES			_T("VxComm?.inf")
#define TIGRIS_SERIAL_W2K_INF_FILES			_T("VxComm.inf")
#define TIGRIS_SERIAL_SYS_FILES				_T("*.sys")
#define TIGRIS_SERIAL_VXD_FILES				_T("*.vxd")


//*********************************************************
//						V 9 0   O N L Y
//*********************************************************


#define V90_CAT_FILENAME				_T("b11415.cat")	// Modem Version 154.00x
#define V90_CATM_FILENAME				_T("b114154m.cat")
#define HCF_156_CAT_FILENAME			_T("b114156.cat")	// Modem Version 156.00x
#define HCF_156_CATM_FILENAME			_T("b114156m.cat")
#define V90_DCT_CAT_FILENAME			_T("1602156.cat")
#define V90_DCT_CATM_FILENAME			_T("1602156m.cat")

#define HCF_CMP_CSA_CPL_FILENAME		_T("csacpl.cp_")

#define HCF_CMP_HCFAPI_FILENAME			_T("hcfapi.dl_")
#define HCF_CMP_HCF_READER_FILENAME		_T("hcfreadr.dl_")
#define HCF_CMP_HCFCSA32_FILENAME		_T("hcfcsa32.dl_")
#define HCF_CMP_HCFCSA_FILENAME			_T("hcfcsa.dl_")

#define HCF_CMP_HC_EXE_FILENAME			_T("hc.ex_")
#define HCF_UNINSTALL_FILENAME			_T("infunist.exe")
#define HCF_CMP_UNINSTALL_FILENAME		_T("infunist.ex_")
#define HCF_CLEANUP_FILENAME			_T("imagefix.exe")

#define MODEM_DCT_CHILD_INF_FILENAME	_T("cpq1602.inf")	
#define MODEM_DCT_PARENT_INF_FILENAME	_T("cpq1602m.inf")	
#define MODEM_PARENT_INF_FILENAME		_T("cpqdslm.inf")	

#define HCF_DSLR_TXT_FILENAME			_T("cpqdslr.txt")
#define HCF_DCT_DSLR_TXT_FILENAME		_T("cpq1602r.txt")

#define HCF_CMP_WIN95AC_VXD_FILENAME	_T("win95ac.vx_")
#define HCF_CMP_ROKMOSD_VXD_FILENAME	_T("rokkmosd.vx_")
#define HCF_CMP_DPAL_VXD_FILENAME		_T("dpal.vx_")
#define HCF_CMP_TURBO_VCD_FILENAME		_T("turbovcd.vx_")
#define HCF_CMP_TURBO_VBF_FILENAME		_T("turbovbf.vx_")
#define HCF_CMP_V42_VXD_FILENAME		_T("rokv42.vx_")


//*********************************************************
//			U T I L I T Y    F I L E N A M E S
//*********************************************************

#define DEFAULT_STATISTICS_LOG_FILENAME	_T("CnxAdslL.log")
#define AUTOLOG_TOOL_FILENAME			_T("CnxAdslL.exe")
#define AUTOLOG_TOOL_SHORTCUT_NAME		_T("CnxAdslL.lnk")
#define CPL_GLOBE_AVI_NAME				_T("CnxGlobe.AVI")


//*********************************************************
//					O E M   S P E C I F I C
//*********************************************************


// This is the name of the DLL displayed on
// the title bar and is used when finding
// the full installation path.
#define BBI_DLL_FILENAME				"OemRsDsl.dll"

// This name should be exactly as DLL_FILENAME
// without the extension.  This value is stored
// in a CPQ registry key for BBI support.
#define DLL_SHORT_FILENAME				"OemRsDsl"

// This name is defined by CPQ and is the
// provisioning information for the product
// This file is copied to the C:\CPQBBI folder.
#define PROVISIONING_FILENAME			"CPQPROV.DAT"


#define HW_DET_VXD_FILENAME				_T("HwDet.vxd")	
// This is the VXD name in the format needed to load
// the VxD with the CreateFile() function.
#define	HW_DET_VXD_EXECUTION_NAME		"\\\\.\\HwDet.Vxd"

#define DUN_CONNECTOID_ICON_NAME		_T("MaxDigitalModem.ico")

#define CNX_DUN_CONNECTOID_ICON_NAME	_T("CnxDunI.ico")

//
// This is the name of the OemRsDsl.dll Debug Log
#define DEBUG_LOG_FILENAME				_T("OEMRSDSL.log")

//
// This is the name of the CnxDslWz.dll Debug Log
#define CNX_DEBUG_LOG_FILENAME			_T("CnxDslWz.log")


//*********************************************************
//					D S L   W I Z A R D
//*********************************************************

// The name of the install/configuration wizard.
#define DSL_WIZARD_FILENAME				_T("Setup.exe")

#define AUTORUN_INF_FILENAME			_T("Autorun.inf")


//*********************************************************
//				D S L   T E S T  F I X T U R E
//*********************************************************

#define DSL_TESTFIXTURE_FILENAME		_T("CnxTgT.exe")
#define TESTFIXTURE_DEBUG_LOG_FILENAME	_T("CnxTgT.log")
#define TESTFIXTURE_INI_FILENAME		_T("CnxTgI.ini")


//*********************************************************
//				D S L   T A S K   B A R    A P P
//*********************************************************
#define TB_CONNECTED_SOUND_FILENAME			_T("CnxUp.wav")
#define TB_DISCONNECTED_SOUND_FILENAME		_T("CnxDown.wav")
#define TB_DEFAULT_SOUND_FILENAME			_T("Ding.Wav")

#endif
