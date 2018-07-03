/******************************************************************************
****	
****	Copyright (c) 1997, 1998
****	Rockwell International
****	All Rights Reserved
****	
****	CONFIDENTIAL and PROPRIETARY --
****		No Dissemination or use without prior written permission.
****	
****	Technical Contact:
****		Rockwell Semiconductor Systems
****		Personal Computing Division
****		Huntsville Design Center
****		6703 Odyssey Drive, Suite 303
****		Huntsville, AL   35806
****	
*******************************************************************************


MODULE NAME:
	EE 	

FILE NAME:
	EEtype.h

ABSTRACT:
	Header file for eedll.c

DETAILS:

KEYWORDS:
	$Archive: /Projects/Tigris and Derivatives/Reference Designs/Tigris/Linux/Kernel Driver/EEType.h $
	$Revision: 1 $
	$Date: 1/09/01 10:54a $

******************************************************************************/

#if !defined(EETYPE_H_)
#define	EETYPE_H_





//******************************************************************
// This section contains the PCI Configuration Device ID & Vendor ID
// that is used by the Windows PnP Device Manager.

// **** CONSTANTS - ToDo:  Define each product in this section.
#define ROCKWELL_VENDOR_ID		0X127a				// Rockwell assigned Manufacturer ID

#define ZEPHYR_DEVICE_ID		0X1003				// Rockwell HW device ID
#define ZEPHYR_SUB_DEVICE_ID	0Xb0bc				// Compaq HW device ID for HCF 56K
#define ZEPHYR_SUB_VENDOR_ID	0X0e11				// Compaq PCI assigned ID.

#define V90_ONLY_DEVICE_ID		0X1003				// Rockwell HW device ID
#define V90_ONLY_SUB_DEVICE_ID	0Xb114				// Compaq HW device ID for HCF 56K
#define V90_ONLY_SUB_VENDOR_ID	0X0e11				// Compaq PCI assigned ID.

#define CHEETAH_DEVICE_ID		0X1600				// Rockwell HW device ID
#define CHEETAH_SUB_DEVICE_ID	0Xb115				// Compaq HW device ID for ADSL/HCF 56K
#define CHEETAH_SUB_VENDOR_ID	0X0e11				// Compaq PCI assigned ID.



// **** ToDo:  Change these to match the product being built.
#undef PCI_VENDOR_ID
#define PCI_VENDOR_ID	 		ROCKWELL_VENDOR_ID	// HW Manufacturer assigned ID
#define PCI_DEVICE_ID_			V90_ONLY_DEVICE_ID	 // Each HW/product has it's own ID.
#define PCI_SUB_DEVICE_ID		V90_ONLY_SUB_DEVICE_ID// Customer differentiates their modem products.
#define PCI_SUB_VENDOR_ID		V90_ONLY_SUB_VENDOR_ID// Customer's PCI SIG assigned ID




//******************************************************************
// This section contains the PCI Configuration space offset definitions
// as listed in Section 4.1.2 PCI CONFIGURATION REGISTERS in
// Rockwell BASIC2 Architecture & Device Electrical Description.
// This is the actual offset in the PCI configuration space.

#define DEVICE_ID_OFFSET			0X00
#define SUBDEVICE_ID_OFFSET			0X2C
#define MAX_LATENCY_OFFSET			0X3C

// Offset address in EEprom where
// start of MAC address is stored.
// See header in file EEMACDLL.C
#define MAC_ADDRESS_OFFSET			0X30




//******************************************************************
// This section contains the structure definition of the configuration
// data stored in the EEProm.  The EEprom is a 16 bit device.
// #pragma pack(1)
typedef struct tagEEPROMDATASTRUCT
{
	unsigned short DeviceID;				// 0x0 - offset in eeprom
	unsigned short VendorID;				// 0x1
	unsigned short SubDeviceID;				// 0x2
	unsigned short SubVendorID;				// 0x3
	unsigned char MinGrant;					// 0x4 LSB
	unsigned char MaxLatency;				// 0x4 MSB
	unsigned short PowerScales;				// 0x5
	unsigned char D2PowerConsumed;			// 0x6 LSB
	unsigned char D3PowerConsumed;			// 0x6 MSB
	unsigned char D0PowerConsumed;			// 0x7 LSB
	unsigned char D1PowerConsumed;			// 0x7 MSB
	unsigned char D2PowerDissipated;		// 0x8 LSB
	unsigned char D3PowerDissipated;		// 0x8 MSB
	unsigned char D0PowerDissipated;		// 0x9 LSB
	unsigned char D1PowerDissipated;		// 0x9 MSB
	unsigned char LoadCIS;					// 0xA LSB
	unsigned char PMESupport;				// 0xA MSB
} __attribute__ ((packed)) EEPROMDATASTRUCT;

typedef EEPROMDATASTRUCT *FPEEPROMDATASTRUCT;
//#pragma pack()

#endif
