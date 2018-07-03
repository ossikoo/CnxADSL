/******************************************************************************
*******************************************************************************
****	Copyright (c) 1997, 1998, 1999
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
**		Tigris
**
**	FILE NAME:
**		Product.h
**
**	ABSTRACT:
**		This files contains defines for configuring the driver
**		for the product..
**
**	DETAILS:
**
*******************************************************************************
** $Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/Product.h $
** $Revision: 1 $
** $Date: 1/09/01 10:54a $
*******************************************************************************
******************************************************************************/
#ifndef _PRODUCT_H_
#define _PRODUCT_H_

#include "ProdDef.h"


///////////////////////////////////////////////////////
//	Product Definition
///////////////////////////////////////////////////////

#ifdef CHICAGO
	#ifndef NDIS30_MINIPORT
	#define NDIS30_MINIPORT
	#endif
#else
	#ifdef NDIS50_MINIPORT
	#define NDIS40_MINIPORT
	#endif
#endif

#define PROJECTS_NAME			PROJECT_NAME_TIGRIS

#define PROJECTS_MEDIUM			PROJECT_MEDIUM_ADSL

#define PROJECTS_PROTOCOL		PROJECT_PROTOCOL_ATM

#define PROJECTS_NDIS_IF		PROJECT_NDIS_IF_LAN


// THESE RFCxxxx DEFINES WILL EXIST AT THIS PLACE IN THIS FILE UNTIL A
// DECISION ON HOW TO STRUCTURE A LAN vs WAN PRODUCT BUILD IS RESOLVED
#define	RFC1483						1
#define RFC2364						0

#if	RFC1483 || RFC2364
#else
	#error	**DEFINE ERROR!  No RFCxxx Defined!
#endif

#if	RFC1483 && RFC2364
	#error	**DEFINE ERROR!  Define ONLY ONE RFCxxx!
#else
#endif

//*
//* Determine External Driver Interface
//*
#define EXTERNAL_DRIVER_NAME_WC	TIGRIS_NDIS_EXT_DRIVER_NAME_WC
#define IRP_INTERFACE_NDIS		1
#define IRP_INTERFACE_KERNEL 	0

#define MAX_DRIVER_MODULES 	6

#if ! defined(CHIPAL_REMOTE)
	#define CHIPAL_REMOTE 0
#endif 

//////////////////////
//	Registry entries
//////////////////////

#define ADAPTERCFID_STRING			NDIS_STRING_CONST("AdapterCFID")
#define ADAPTERTYPE_STRING			NDIS_STRING_CONST("AdapterType")
#define ADDRESSLIST_STRING			NDIS_STRING_CONST("AddressList")
#define ADSLDATAPATHSELECT_STRING	NDIS_STRING_CONST("AdslDataPathSelection")
#define ADSLDPRATEMODE_STRING		NDIS_STRING_CONST("AdslDpRateMode")
#define ADSLPSDTESTMODE_STRING		NDIS_STRING_CONST("AdslPsdTestMode")
#define ADSLSUBFUNCTIONMODE_STRING	NDIS_STRING_CONST("AdslSubfunctionalityMode")
#define ADVANCED_STRING				NDIS_STRING_CONST("Advanced")
#define APIHWIFMODE_STRING			NDIS_STRING_CONST("ApiHwIfMode")
#define AUTOLOG_ENABLED_STRING		NDIS_STRING_CONST("AutoLogEnabled")
#define AUTOLOG_MAX_SIZE_STRING		NDIS_STRING_CONST("AutoLogMaxFileSize")
#define AUTOLOG_OVERWRITE_STRING	NDIS_STRING_CONST("AutoLogOverwrite")
#define AUTOLOG_PATH_STRING			NDIS_STRING_CONST("AutoLogPath")
#define AUTOLOG_FREQUENCY_STRING	NDIS_STRING_CONST("AutoLogUpdateFrequency")
#define BOARDNAME_STRING			NDIS_STRING_CONST("BoardName")
#define BUSNUMBER_STRING			NDIS_STRING_CONST("BusNumber")
#define BUSTYPE_STRING				NDIS_STRING_CONST("BusType")
#define CELLBURSTSIZE_STRING		NDIS_STRING_CONST("CellBurstSize")
#define DEBUGFLAG_STRING			NDIS_STRING_CONST("DebugFlag")
#define DEVICEDESC_STRING			NDIS_STRING_CONST("DriverDesc")
#define DEVICENAME_STRING			NDIS_STRING_CONST("DeviceName")
#define DPAUTODOWNLOAD_STRING		NDIS_STRING_CONST("DpAutoDownload")
#define DPBOOTDNLDFILENAME_STRING	NDIS_STRING_CONST("DpBootFilename")
#define DPBOOTJUMPADDR_STRING		NDIS_STRING_CONST("DpBootJumpAddress")
#define DPDNLDFILENAME_STRING		NDIS_STRING_CONST("DpFilename")
#define DPJUMPADDR_STRING			NDIS_STRING_CONST("DpJumpAddress")
#define DUN_NAME_STRING				NDIS_STRING_CONST("DunName")
#define ENVIRONMENT_STRING			NDIS_STRING_CONST("Environment")
#define LINEAUTOACTIVATION_STRING	NDIS_STRING_CONST("LineAutoActivation")
#define LINEPERSISTENTACT_STRING	NDIS_STRING_CONST("LinePersistentActivation")
#define LINEPERSISTENCETMR_STRING	NDIS_STRING_CONST("LinePersistenceTimer")
#define LTMODE_STRING				NDIS_STRING_CONST("LTMode")
#define MAXTRANSMIT_STRING			NDIS_STRING_CONST("MaxTransmit")
#define MEDIATYPE_STRING			NDIS_STRING_CONST("MediaType")
#define NUMBEROFLINES_STRING		NDIS_STRING_CONST("NumberOfLines")
#define RATE_MODE_STRING			NDIS_STRING_CONST("RateMode")
#define RFCENCAPSULATIONMODE_STRING	NDIS_STRING_CONST("RfcEncapsulationMode")
#define	RXBUFALLOCLIMIT_STRING		NDIS_STRING_CONST("RxBufAllocLimit")
#define	RXBUFRATIOLIMIT_STRING		NDIS_STRING_CONST("RxBufRatioLimit")
#define RXMAXFRAMESIZE_STRING		NDIS_STRING_CONST("RxMaxFrameSize")
#define RX_RATE_STRING				NDIS_STRING_CONST("RxRate")
#define RX_SPEED_STRING				NDIS_STRING_CONST("RxSpeed")
#define SYMBOL_RX_RATE_STRING		NDIS_STRING_CONST("SymbolRXRate")
#define SYMBOL_TX_RATE_STRING		NDIS_STRING_CONST("SymbolTXRate")
#define TOOLTIPS_STRING				NDIS_STRING_CONST("Tooltips")
#define TXMAXFRAMESIZE_STRING		NDIS_STRING_CONST("TxMaxFrameSize")
#define TX_POWER_MODE_STRING		NDIS_STRING_CONST("TxPowerMode")
#define TX_POWER_VALUE_STRING		NDIS_STRING_CONST("TxPowerValue")
#define TX_RATE_STRING				NDIS_STRING_CONST("TxRate")
#define TX_SPEED_STRING				NDIS_STRING_CONST("TxSpeed")
#define VCLIST_STRING				NDIS_STRING_CONST("VcList")
#define VENDORDESCRIP_STRING		NDIS_STRING_CONST("VendorDescription")
#define VENDORID_STRING				NDIS_STRING_CONST("VendorId")
#define WANADDRESS_STRING			NDIS_STRING_CONST("WanAddress")
/*
#define CONTROLLER_LOG_ENABLE		NDIS_STRING_CONST("ControllerLogEnable")
#define CAPABILITIES_G922  			NDIS_STRING_CONST("CapabilitiesG922")
#define CAPABILITIES_G922ANNEX		NDIS_STRING_CONST("CapabilitiesG922Annex")
#define VENDOR_NEAR_ID				NDIS_STRING_CONST("VendorNearId")
#define MAC_ADDRESS					NDIS_STRING_CONST("MACAddress")
#define AUTO_SENSE_HANDSHAKE		NDIS_STRING_CONST("AutoSenseHandshakeMode")
#define AUTO_SENSE_WIRING			NDIS_STRING_CONST("AutoSenseWiringMode")
#define AUTO_SENSE_WIRES_FIRST		NDIS_STRING_CONST("AutoSenseWiresFirst")
*/
#ifdef CHICAGO

//ChipAL Definitions -(Char Strings to be used with STR_TO_NDIS_STR)
#define DEBUG_FLAG_USTRING			"DebugFlag"
#define	IRQ_STRING					"IRQ"
#define PCIDEVICEID_STRING			"PciDeviceId"
#define PCIVENDORID_STRING			"PciVendorId"
#define PHYSICAL_BASE_STRING		"PhysicalBase"
#define RXINTERRUPTRATE_STRING		"RxInterruptRate"
#define RXMAXLATENCY_STRING			"RxMaxLatency"
#define RXMINLATENCY_STRING			"RxMinLatency"
#define RXSPEED_STRING				"RxSpeed"
#define TXINTERRUPTRATE_STRING		"TxInterruptRate"
#define TXMAXLATENCY_STRING			"TxMaxLatency"
#define TXMINLATENCY_STRING			"TxMinLatency"
#define TXSPEED_STRING				"TxSpeed"
#define RXFIFO_SIZE					"RxFifoSize"
#define TXFIFO_SIZE					"TxFifoSize"

#else	// CHICAGO
 
//ChipAL Definitions -(Unicode Strings to be used with STR_TO_NDIS_STR)
#define DEBUG_FLAG_USTRING			L"DebugFlag"
#define	IRQ_STRING					L"IRQ"
#define PCIDEVICEID_STRING			L"PciDeviceId"
#define PCIVENDORID_STRING			L"PciVendorId"
#define PHYSICAL_BASE_STRING		L"PhysicalBase"
#define RXINTERRUPTRATE_STRING		L"RxInterruptRate"
#define RXMAXLATENCY_STRING			L"RxMaxLatency"
#define RXMINLATENCY_STRING			L"RxMinLatency"
#define RXSPEED_STRING				L"RxSpeed"
#define TXINTERRUPTRATE_STRING		L"TxInterruptRate"
#define TXMAXLATENCY_STRING			L"TxMaxLatency"
#define TXMINLATENCY_STRING			L"TxMinLatency"
#define TXSPEED_STRING				L"TxSpeed"
#define RXFIFO_SIZE					L"RxFifoSize"
#define TXFIFO_SIZE					L"TxFifoSize"

#endif	// CHICAGO

#if PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
#define ADSL_RX_IDLE_CHAR			L"AdslRxIdleChar"
#define ADSL_RX_IDLE_TIMER			L"AdslRxIdleTimer"
#define ATM_TX_CELLA_FILE			L"AtmTxCellAFile"
#define ATM_TX_CELLB_FILE			L"AtmTxCellBFile"
#define FPGA_PROGRAM_FILE			L"FpgaProgramFile"
#define FPGA_PROGRAM_ENABLE			L"FpgaProgramEnable"
#endif									//PROJECTS_NAME == PROJECT_NAME_CHEETAH_1
// End ChipAL Definitions


#define ADSL_PSD_TEST_MODE_NONE		0
#define ADSL_PSD_TEST_MODE_NO_CLOSE	1
#define API_HW_IF_MODE_NORMAL		0
#define API_HW_IF_MODE_HIJACK_OID	1

#define LT_MODE_NT					0
#define LT_MODE_LT					1


#define LINK_MODE					0		// clear ch h/w (s/w HDLC) or h/w HDLC
#define CRC_SIZE					16		// Size of CRC (0, 8, 16, 32)
#define CRC_FILTER					TRUE	// Indicates whether received CRC bytes

#define MAX_WINDOW_SIZE				127
#define MIN_WINDOW_SIZE				1
#define DEFAULT_WINDOWSIZE			MIN_WINDOW_SIZE

#define MAX_OS_FRAMES_PER_LINK		MAX_WINDOW_SIZE


///////////////////////////////////////////////////////////////////////////////
//	Configuration for Cheetah1 ADSL:
//		Physical:
//			1 Physical Line				same as Link structure in driver
//		TAPI (logical/virtual):
//			2 TAPI Line Devices			logical, same as 2 VCs
//				also known in TAPI
//				as Lines or Links
//			1 Address per Line
//			1 Call per Address
//	Result - VC maps to TAPI lines and/or link with only 1 address and 1 call.
///////////////////////////////////////////////////////////////////////////////

//xdefine MIN_CHANNEL_NUM				0
//xdefine MAX_CHANNEL_NUM				3

#define _MAX_LINKS						1		// link is same as line device for NDIS TAPI

//xdefine NUM_LINKS_PER_LINE			1


// Maximum packet size allowed by the adapter -- Currently restricted to
// 1500 bytes at this point, but allows for frames at least 32 bytes longer.

#define	MAX_HDR_SIZE		32
#define MAX_PACKET_SIZE		(1500 + MAX_HDR_SIZE)
#define MAX_FRAME_SIZE		(MAX_PACKET_SIZE - MAX_HDR_SIZE)

// WAN packets don't have a MAC header.
#define MAC_HEADER_SIZE		0

// Used to indicate the maximum padding required by our	driver to 
// prepend header info in any of the RFCxxxx modes of operation. 
#define MAX_HDR_PADDING		10

// Used to indicate maximum tail padding required for AAL5 trailer
#define MAX_TAIL_PADDING	55

// The WAN miniport driver must indicate the entire packet when it is received.
#define MAX_LOOKAHEAD		(MAX_PACKET_SIZE - MAC_HEADER_SIZE)

///////////////////////////////////////////////////////////////////

#define MAXWAMESSAGE		4
#define MAXLOOPBACKMESSAGE	2048	//must be a multiple of 4
///////////////////////////////////////////////////////////////////

// The length of the buffer that stores the numbers to dial
#define MAX_CALLED_NUMBER_LENGTH	40

#define MAX_LEN_LINEADDRESS			100


///////////////////////////////////////////////////////////////////
//
// DEVELOPMENT SETTINGS
//
///////////////////////////////////////////////////////////////////
//
//
//

#define USE_DEVICE_TIMER_FOR_DELAY 1	// Whether to use the timer on the device
										// to implement delays or use NDIS services 
										// which have only 10ms resolution in NT
										// and 2.5 ms in Win98
///////////////////////////////////////////////////////////////////
//
// NDIS WAN
//
///////////////////////////////////////////////////////////////////
//
//NDIS version reported to NDIS Wrapper
//
#ifdef NDIS50_MINIPORT
#define NDIS_MAJOR_VERSION			0x05
#define NDIS_MINOR_VERSION			0x00
#elif defined NDIS40_MINIPORT
#define NDIS_MAJOR_VERSION			0x04
#define NDIS_MINOR_VERSION			0x00
#else
#ifndef CHICAGO
	#define NDIS_MAJOR_VERSION		0x03
	#define NDIS_MINOR_VERSION		0x00
#endif
#endif
//
//amount of space to reserve for LineAddress
//
#define LINEADDRESS_LENGTH			32		//must be a multiple of 4
//
//amount of space to reserve for Media Type
//
#define MEDIATYPE_LENGTH			32		//must be a multiple of 4
//
//amount of space to reserve for Board Name
//
#define BOARDNAME_LENGTH			44		//must be a multiple of 4
//
//amount of space to reserve for Device Name
//
#define DEVICENAME_LENGTH			44		//must be a multiple of 4
//
//amount of space to reserve for addresslist string
//
#define ADDRESSLIST_LENGTH			1024	//6 * 128 to nearest power of 2
#define	ADDR_TERMINATOR_NT			'\0'
#define	ADDR_TERMINATOR_95			'*'
//
//amount of space to reserve for VC List string
//
#define VCLIST_LENGTH				53		//	Should be a multiple of 13 plus
											//	one for terminator (now 4 VCs).
											// VC List is "XX YYYY ZZZZ*XX YYYY ZZZZ*..."
											// where XX is the ASCII hexadecimal
											// value for VPI, YYYY is the ASCII
											// hexadecimal value for VCI, ZZZZ
											// is the ASCII hexadecimal value
											// for PCR, and '*' is the VC
											// terminator ('\0' for NT and '*'
											// for 95/98).
											// These MUST always be in groups
											// (VPI, VCI then PCR) for each VC!

//////////////////////////////////////////
//	Registry default value definitions
//////////////////////////////////////////

#define MAX_VC_PER_LINK					4	// Size of driver's VC array
#define DEFAULT_ADSL_PSD_TEST_MODE		0
#define DEFAULT_ADSL_RX_IDLE_CHAR		"_"
#define DEFAULT_ADSL_RX_IDLE_TIMER		10
#define DEFAULT_API_HW_IF_MODE			0
#define DEFAULT_SZ_ATM_TX_CELL_A_FILE	"AtmCellA.bin"
#define DEFAULT_ATM_TX_CELL_A_FILE		NDIS_STRING_CONST("AtmCellA.bin")
#define DEFAULT_SZ_ATM_TX_CELL_B_FILE	"AtmCellB.bin"
#define DEFAULT_ATM_TX_CELL_B_FILE		NDIS_STRING_CONST("AtmCellB.bin")
#define DEFAULT_CELL_BURST_SIZE			1000
#define DEFAULT_DP_AUTO_DOWNLOAD		1
#define DEFAULT_SZ_DP_BOOT_FILENAME		"A20136B.bin"
#define DEFAULT_DP_BOOT_FILENAME		NDIS_STRING_CONST("A20136B.bin")
#define DEFAULT_DP_BOOT_JUMP_ADDRRESS	0
#define DEFAULT_SZ_DP_FILENAME			"A20136.bin"
#define DEFAULT_DP_FILENAME				NDIS_STRING_CONST("A20136.bin")
#define DEFAULT_DP_JUMP_ADDRESS			0
#define DEFAULT_FPGA_PROGRAM_ENABLE		1
#define DEFAULT_SZ_FPGA_PROGRAM_FILE	"Hobbes.rbf"
#define DEFAULT_FPGA_PROGRAM_FILE		NDIS_STRING_CONST("Hobbes.rbf")
#define DEFAULT_LINE_AUTO_ACTIVATION	1
#define DEFAULT_LINE_PERSISTENCE_TIMER	5
#define DEFAULT_LINE_PERSISTENT_ACT		1
#define DEFAULT_LT_MODE					LT_MODE_NT
#define DEFAULT_MAX_TRANSMIT			10
#define DEFAULT_NUMBER_OF_LINES			2	// Number of TAPI Line Devices
											// ** logical number, NOT physical **
#define DEFAULT_RX_BUF_ALLOC_LIMIT		10
#define DEFAULT_RX_BUF_RATIO_LIMIT		10
#define DEFAULT_RX_FIFO_SIZE			512
#define DEFAULT_RX_INTERRUPT_RATE		20
#define DEFAULT_RX_MAX_FRAME_SIZE		1500
#define DEFAULT_RX_MAX_LATENCY			200
#define DEFAULT_RX_MIN_LATENCY			45
#define DEFAULT_RX_SPEED				1500000
#define DEFAULT_TX_FIFO_SIZE			512
#define DEFAULT_TX_INTERRUPT_RATE		15
#define DEFAULT_TX_MAX_FRAME_SIZE		1500
#define DEFAULT_TX_MAX_LATENCY			65
#define DEFAULT_TX_MIN_LATENCY			45
#define DEFAULT_TX_SPEED				1000000

#define DEFAULT_CONTROLLER_LOG_ENABLE   0
#define DEFAULT_CAPABILITIES_G922		0
#define DEFAULT_CAPABILITIES_G922ANNEX  0x09	// CAP_STANDARD_G9921A|CAP_STANDARD_G9922AB
#define DEFAULT_VENDOR_NEAR_ID			0x0010	// Pairgain
// For Centillium only
//#define DEFAULT_AUTO_SENSE_HANDSHAKE	0x0000	// ADSL_GHS_TONE_MODE
#define DEFAULT_AUTO_SENSE_HANDSHAKE	0x0003	// ADSL_TONE_ONLY  
#define DEFAULT_AUTO_SENSE_WIRING		0x0000	// ADSL_WIRING_3_4_2_5
#define DEFAULT_AUTO_SENSE_WIRES_FIRST	0x0000	// FALSE (do handshake first)

#define WAN_ADDRESS_LENGTH				6
#define MAX_WAN_ADDRESS_LENGTH			WAN_ADDRESS_LENGTH + 1
#define ETH_ADDRESS_LENGTH				6
#define MAX_ETH_ADDRESS_LENGTH			ETH_ADDRESS_LENGTH + 1
#define VENDOR_ID_LENGTH				4
#define MAX_VENDOR_ID_LENGTH			VENDOR_ID_LENGTH + 1
#define VENDOR_DESCRIPTION_LENGTH		40
#define MAX_VENDOR_DESCRIPTION_LENGTH	VENDOR_DESCRIPTION_LENGTH + 1
#define MANUFACTURER_LENGTH				8
#define MAX_MANUFACTURER_LENGTH			MANUFACTURER_LENGTH + 1
#define DP_DNLD_FILENAME_LENGTH			80
#define MAX_DP_DNLD_FILENAME_LENGTH		DP_DNLD_FILENAME_LENGTH + 1


///////////////////////////////////////
//	Other default value definitions
///////////////////////////////////////
#define NUM_TAPI_PHONEDEVICES		0
#define TAPI_NUM_ADDR_PER_LINE		1		//	two per linedevice

//	There are two TAPI address ID per line device (zero based).
#define TAPI_ADDRESSID_BASE			0
#define TAPI_ADDRESSID_1			0
#define TAPI_ADDRESSID_2			1
#define TAPI_ADDRESSID_LOOPBACK		DEFAULT_NUMBER_OF_LINES

//	There is only one TAPI call per address instance.
#define TAPI_NUM_CALLS_PER_ADDR		1

//
//	This version number is used by the NDIS_TAPI_NEGOTIATE_TSPI_VER request.
//	This is only used when the NDISTAPI_EXTENSTIONS are turned on and must
//	be set to 1.4 for this release
//
#define TAPI_API_VERSION			0x00020000

//
//	This version number is used by the NDIS_TAPI_NEGOTIATE_EXT_VERSION request.
//	It is not used by this driver or the current NDISTAPI release.
//
#define TAPI_EXT_VERSION			0x00010000


//	These are used in the TapiGetID call and are used to hash the DeviceClass
//	strings into constants that we can use easier.
#define TAPI_DEVICECLASS_NAME		"tapi/line"
#define TAPI_DEVICECLASS_ID			1
#define NDIS_DEVICECLASS_NAME		"ndis"
#define NDIS_DEVICECLASS_ID			2

#define VENDOR_DEVICECLASS_NAME		"vendor-specific"
#define VENDOR_DEVICECLASS_ID		3

//	This value indicates how many point to point connections are allowed per WAN link.
#define PPP_CONNECTIONS_PER_LINK	DEFAULT_NUMBER_OF_LINES		\
									* TAPI_NUM_ADDR_PER_LINE	\
									* TAPI_NUM_CALLS_PER_ADDR

#define VENDOR_DRIVER_VERSION			0x00010001
#define DRIVER_VERSION_OF_NDIS			(NDIS_MAJOR_VERSION << 8) + NDIS_MINOR_VERSION
#define PROVIDER_INFO_STRING			"ISDN\0Rockwell\0"

#if		PROJECTS_NDIS_IF == PROJECT_NDIS_IF_LAN
	#define DEFAULT_MEDIA_SUPPORTED			NdisMedium802_3
	#define DEFAULT_MAC_OPTIONS				NDIS_MAC_OPTION_RECEIVE_SERIALIZED	\
											| NDIS_MAC_OPTION_NO_LOOPBACK		\
											| NDIS_MAC_OPTION_TRANSFERS_NOT_PEND
	#define DEFAULT_MAC_802_3_OPTIONS		0

#else

#define DEFAULT_MEDIA_SUPPORTED			NdisMediumWan
#define DEFAULT_MAC_OPTIONS				NDIS_MAC_OPTION_RECEIVE_SERIALIZED	\
										| NDIS_MAC_OPTION_NO_LOOPBACK		\
										| NDIS_MAC_OPTION_TRANSFERS_NOT_PEND
#define DEFAULT_QUALITY_OF_SERVICE		NdisWanRaw
#define DEFAULT_MEDIUM_SUBTYPE			NdisWanMediumIsdn
#define DEFAULT_WANINFO_ENDPOINTS		PPP_CONNECTIONS_PER_LINK
#define DEFAULT_WANINFO_MEMORY_FLAGS	0
#define DEFAULT_WANINFO_DESIRED_ACCM	0
#define DEFAULT_WANINFO_FRAMING_BITS	PPP_FRAMING | TAPI_PROVIDER
#define DEFAULT_WANLINKINFO_COMP_BITS	0
#endif 

#ifndef CHICAGO
#define DEFALUT_PHYSICAL_MEDIUM			NdisPhysicalMediumDSL
#endif

#define DEFALUT_LINK_SPEED				RATE_64000


//* 802.3 Specific
#define MAX_MULTICAST_ADDRESSES			32
#define ETH_LENGTH_OF_ADDRESS			6

#define ETH_ADDRESS						"\x00\x60\x08\xA6\x06\xF2"
//#define ETH_ADDRESS						"181234"

//* End 802.3 specific

#define MFG_KEYCODE				\
	(							\
		(((DWORD)'R') << 24)	\
		+ (((DWORD)'O') << 16)	\
		+ (((DWORD)'K') << 8)	\
		+ (((DWORD) 0) << 0)	\
	)






//	The maximum number of digits allowed to be in a dialing sequence.
#define MAX_DIALING_DIGITS			32



////////////////////////
//	Timer definitions
////////////////////////
#define MAX_TIMERS					10

#ifdef CHICAGO
#define DEFAULT_TIMER_RESOLUTION	20				//	20 msec (set timebase for W95)
#else
#define DEFAULT_TIMER_RESOLUTION	10				//	10 msec (set timebase for NT/W2K)
#endif

#define TIMER_RESOLUTION_1_MSEC		1				//	1 msec
#define TIMER_RESOLUTION_10_MSEC	10				//	10 msec
#define TIMER_RESOLUTION_20_MSEC	20				//	20 msec
#define TIMER_RESOLUTION_50_MSEC	50				//	50 msec
#define TIMER_RESOLUTION_100_MSEC	100				//	100 msec
#define TIMER_RESOLUTION_250_MSEC	250				//	250 msec
#define TIMER_RESOLUTION_500_MSEC	500				//	500 msec
#define TIMER_RESOLUTION_750_MSEC	750				//	750 msec
#define TIMER_RESOLUTION_1_SEC		1000			//	1 sec
#define TIMER_RESOLUTION_2_SEC		2000			//	2 sec
#define TIMER_RESOLUTION_5_SEC		5000			//	5 sec
#define TIMER_RESOLUTION_10_SEC		10000			//	10 sec
#define TIMER_RESOLUTION_100_SEC	100000			//	100 sec

//	Tx Rx Background loops Timer References
#define thRxPacketBackGround		1
#define thTxPacketBackGround		2
#define thSimulateIncoming			3
#define thSimulateRemoteDisconnect	4
#define thGoToConnected				5
#define thPersistentActivation		6
#define thWaitAfterDyingGaspCmd		7
#define thWaitAfterCloseCmd			8
#define thFirstAutoActivation		9

#define DEFAULT_LINE_AUTO_ACT_TMR		TIMER_RESOLUTION_1_SEC



///////////////////////////////////////////////////////////////////
//
//	Fail to init error messages
//
///////////////////////////////////////////////////////////////////
#define RSS_INIT_ERROR_NO_SUPPORTED_MEDIUM			0x01
#define RSS_INIT_ERROR_FAILED_OPEN_REGISTERY		0x02
#define RSS_INIT_ERROR_FAILED_ALLOCATE_THISADAPTER	0x03
#define RSS_INIT_ERROR_REGISTRY_VALUE_READ_ERROR	0x04
//define RSS_INIT_ERROR_	0x05
#define RSS_INIT_ERROR_NO_ADAPTER_FOUND				0x06
#define RSS_INIT_ERROR_READING_PCICONFIG1_FIRSTTIME	0x07
#define RSS_INIT_ERROR_REGISTERING_PRI_INTERRUPT	0x08
#define RSS_INIT_ERROR_REGISTERING_SEC_INTERRUPT	0x09
#define RSS_INIT_ERROR_ASSIGN_PCI_RESOURCES0		0x0A
#define RSS_INIT_ERROR_READING_PCICONFIG0_VERIFY0_1	0x0B
#define RSS_INIT_ERROR_WRITING_PCICONFIG0			0x0C
#define RSS_INIT_ERROR_READING_PCICONFIG0_VERIFY0_2	0x0D
#define RSS_INIT_ERROR_NO_BUS_MASTER				0x0E
#define RSS_INIT_ERROR_ASSIGN_PCI_RESOURCES1		0x0F
#define RSS_INIT_ERROR_READING_PCICONFIG0_VERIFY1_1	0x10
#define RSS_INIT_ERROR_READING_PCICONFIG0_VERIFY1_2	0x11
#define RSS_INIT_ERROR_WRITING_PCICONFIG1			0x12
#define RSS_INIT_ERROR_NO_BUS_MEMORY_ACCESS			0x13
#define RSS_INIT_ERROR_NO_PRI_INTERRUPT_ASSIGNED	0x14
#define RSS_INIT_ERROR_NO_SEC_INTERRUPT_ASSIGNED	0x15
#define RSS_INIT_ERROR_NO_FUNCTION0_MEMORY_ASSIGNED	0x16
#define RSS_INIT_ERROR_NO_FUNCTION1_MEMORY_ASSIGNED	0x17
#define RSS_INIT_ERROR_NO_MAP_REGISTER				0x18
#define RSS_INIT_ERROR_NO_VIRTUAL_MEMORY0			0x19
#define RSS_INIT_ERROR_NO_VIRTUAL_MEMORY1			0x1A
#define RSS_INIT_ERROR_NO_SHARED_MEMORY				0x1B
#define RSS_INIT_ERROR_SAME_PCI_BASE_ADDRESS		0x1C
#define RSS_INIT_ERROR_FAILED_ALLOCATE_CARDAL_ADPTR	0x1D
#define RSS_INIT_ERROR_FAILED_ALLOCATE_DP_FILE_AREA	0x1E
#define RSS_INIT_ERROR_FAILED_READ_DP_FILE			0x1F
#define RSS_INIT_ERROR_FPGA_DOWNLOAD_FAILURE		0x20
#define RSS_INIT_ERROR_FPGA_VERIFICATION_FAILURE	0x21
#define RSS_INIT_ERROR_X_CFG_INIT_FUNCTION_FAILURE	0x22
#define RSS_INIT_ERROR_CHIPAL_ADAPTER_INIT_FAILURE	0x23
#define RSS_INIT_ERROR_FRAMEAL_ADAPTER_INIT_FAILURE	0x24
#define RSS_INIT_ERROR_BUFMGMT_ADAPTER_INIT_FAILURE	0x25
#define RSS_INIT_ERROR_CARDAL_ADAPTER_INIT_FAILURE	0x26
#define RSS_INIT_ERROR_INIT_ALL_LINKS_FAILURE		0x27
#define RSS_INIT_ERROR_INIT_ALL_VCS_FAILURE			0x28
#define RSS_INIT_ERROR_CHIPAL_LINK_INIT_FAILURE		0x29
#define RSS_INIT_ERROR_FRAMEAL_LINK_INIT_FAILURE	0x2A
#define RSS_INIT_ERROR_BUFMGMT_LINK_INIT_FAILURE	0x2B
#define RSS_INIT_ERROR_MODULE_X_LINK_INIT_FAILURE	0x2C


#ifdef NDIS50_MINIPORT

//*
//* Custom OIDs are a bit mapped DWORD value.  The implementation is 
//* as follows:
//*
//*	MSB		0xFF 	Indicates implementation specific OID  (REQUIRED)
//*	|		0x7F 	Rockwell unique identifier
//*	|		XX		(0x01 Mandatory, 0x02 Optional)
//*	LSB		XX		Custom OID number - providing 255 possible  oids
//*
#define	OID_CUSTOM_DEVICE_IO						0xFF7F0201
#define	OID_CUSTOM_DEVICE_CONTROL					0xFF7F0202

#endif									// NDIS50_MINIPORT







#endif		//#ifndef _PRODUCT_H_
