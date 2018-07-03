/******************************************************************************
********************************************************************************
****	Copyright (c) 1997, 1998, 1999, 2000, 2001
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
**		????
**
**	FILE NAME:
**		CommonData.h
**
**	ABSTRACT:
**		This file contains type definitions and structures that are common
**		between the application and the driver
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/CommonData.h $
**	$Revision: 14 $
**	$Date: 5/23/01 8:37a $
*******************************************************************************
******************************************************************************/

#ifndef _COMMON_DATA_H_
#define  _COMMON_DATA_H_


/*******************************************************************************
	AUTOSENSE
*******************************************************************************/
// these must match values in xcvrinfo.h which is a DMT core file which can't
// on the Linux shared file directory and naming structure
#define AUTOSENSE_WIRING_COMBO_MAX_NUM_COMBO	4	// maximum number of combinations of relay settings
#define AUTOSENSE_WIRING_COMBO_MAX_NUM_GPIO		4	// maximum number of GPIOs that must be asserted to select a relay combo


/*******************************************************************************
	PHYSICAL DRIVER TYPE
*******************************************************************************/
#define NO_VENDOR_ID				0xFFFF
#define NO_DEVICE_ID				0xFFFF
#define NO_PHYSICAL_DRIVER			0
#define PHYSICAL_DRIVER_TIGRIS		1
#define PHYSICAL_DRIVER_YUKON		2

typedef enum PHYSICAL_DRIVER_TYPE_E
{
	DRIVER_TYPE_NONE	= NO_PHYSICAL_DRIVER,
	DRIVER_TYPE_TIGRIS	= PHYSICAL_DRIVER_TIGRIS,
	DRIVER_TYPE_YUKON	= PHYSICAL_DRIVER_YUKON
} PHYSICAL_DRIVER_TYPE_T;





/*******************************************************************************
	IO control commands
*******************************************************************************/

#define TIGR_MAGIC '%'

#define TIG_LOAD_START		_IOW(TIGR_MAGIC, 1,struct atmif_sioc)
#define TIG_LOAD_RECORD		_IOW(TIGR_MAGIC, 2,struct atmif_sioc)
#define TIG_LOAD_DONE		_IOW(TIGR_MAGIC, 3,struct atmif_sioc)
#define TIG_SET_PARAMS		_IOW(TIGR_MAGIC, 4,struct atmif_sioc)
#define TIG_START_DSL		_IOW(TIGR_MAGIC, 5,struct atmif_sioc)
#define TIG_DEVICE_SPEC		_IOW(TIGR_MAGIC, 6,struct atmif_sioc)
#define TIG_IS_SHOWTIME		_IOW(TIGR_MAGIC, 7,struct atmif_sioc)
#define TIG_BKGND_POLL		_IOW(TIGR_MAGIC, 8,struct atmif_sioc)
#define TIG_SIDEVICE_POLL	_IOW(TIGR_MAGIC, 9,struct atmif_sioc)
#define TIG_CLEAR_STATS		_IOW(TIGR_MAGIC,10,struct atmif_sioc)
// this function is used to read controller logs.  The argument must be
// a buffer at least 256 bytes long.  The return will be a null terminated
// character string
#define TIG_GET_LOG			_IOW(TIGR_MAGIC,11,struct atmif_sioc) 
#define TIG_GET_PRINT_BUFF	_IOW(TIGR_MAGIC,12,struct atmif_sioc)
#define TIG_GET_DEBUG_DATA	_IOW(TIGR_MAGIC,13,struct atmif_sioc)
#define TIG_TMR_POLL		_IOW(TIGR_MAGIC,14,struct atmif_sioc)

#define NUM_LOG_PARMS 6
#define NUM_LOG_ENTRIES (2*1000)


/*******************************************************************************
	TIG_GET_PRINT_BUFF
*******************************************************************************/

typedef struct
{
	char		*pFormat ;	// NULL terminates list
	DWORD		Data[NUM_LOG_PARMS] ;
} LOG_ENTRY_T ;

typedef struct
{
	BOOLEAN		 Cumulative ;			// Cumulative vs Delta (only those lines since last read)
	char		*pString_Buffer ;		// holds multiple strings
	DWORD		 String_Buffer_Size ;	// size in bytes

	LOG_ENTRY_T *pBuff ;
	DWORD		 Buff_Size ;
} TIG_PRINT_DESC ;



/*******************************************************************************
	TIG_GET_DEBUG_DATA
*******************************************************************************/
typedef struct
{
	DWORD		 Data[16] ;
} TIG_DEBUG_DATA_DESC ;



/*******************************************************************************
	TIG_LOAD_RECORD
*******************************************************************************/
// Intel 32 hex data record definitions
#define DATA_REC			0
#define EXT_SEG_ADDR_REC	2
#define EXT_LIN_ADDR_REC	4
#define EXT_END_OF_FILE		1
#define EXT_GOTO_CMD_REC	3


// record format for transfer to the driver
typedef struct TIG_LOAD_RECORD_S
{
	DWORD		CommandStatus;
	USHORT		RecordType; // type field from the record
	DWORD		RecordAddress;  // store address for the record
	DWORD		RecordLength;  // byte length not including the header - 4096 max
	CHAR		RecordData [1]; // first byte of the record data
} TIG_LOAD_RECORD_T;



/*******************************************************************************
	TIG_DEVICE_SPEC
*******************************************************************************/
// wrapper for device specific IOCTL
typedef struct TIG_DEVICE_SPEC_S
{
	DWORD		CommandStatus;
	ULONG		RequiredSize;	// number of bytes required for the response.
	ULONG		ReturnSize;		// number of bytes returned
	char		BackDoorBuf [1];	// command specific data area
} TIG_DEVICE_SPEC_T;



/*******************************************************************************
	TIG_SET_PARAMS
*******************************************************************************/
//
// User configurable parameters structure
//
typedef struct _TIG_USER_PARAMS_
{
	// status flag
	DWORD			CommandStatus;

	// debug flag
	unsigned long	DebugFlag;

	// card management settings
	unsigned long	ApiHwIfMode;

	// Chip abstraction layer settings
	unsigned long	RxMaxLatency;
	unsigned long	RxMinLatency;
	unsigned long	TxMaxLatency;
	unsigned long	TxMinLatency;
	unsigned long	RxInterruptRate;
	unsigned long	TxInterruptRate;
	unsigned long	RxSpeed;
	unsigned long	TxSpeed;
	unsigned long	RxFifoSize;
	unsigned long	TxFifoSize;
	unsigned long	RxChannelSize;
	unsigned long	RxSegments;
	unsigned long	TxChannelSize;
	unsigned long	TxSegments;
	BOOL			RxChannelSizeCfg;
	BOOL			RxSegmentsCfg;
	BOOL			TxChannelSizeCfg;
	BOOL			TxSegmentsCfg;

	// Card abstraction layer settings
	unsigned long	AdslDpRateMode;
	unsigned long	AdslPsdTestMode;
	unsigned long	LinePersistentAct;
	unsigned long	LinePersistenceTmr;
	unsigned long	CdALDiagControllerLogEnable;
	unsigned long	BdADSLLocalG922Cap;
	unsigned long	BdADSLLocalG922AnnexCap;

	unsigned long	AutoSenseHandshake;
	unsigned long	AutoSenseWiresFirst;
	unsigned long	AutoWiringSelection;
	unsigned long	AutoWiringRelayDelay;
	unsigned long	AutoWiringRelayEnrg;
	unsigned long	AutoWiringNumCombos;
	unsigned char	AutoWiringComboGPIO [AUTOSENSE_WIRING_COMBO_MAX_NUM_COMBO][AUTOSENSE_WIRING_COMBO_MAX_NUM_GPIO];
	unsigned char	AutoWiringOrder[AUTOSENSE_WIRING_COMBO_MAX_NUM_COMBO];

	unsigned long	AdslHeadEnd;
	unsigned long	AdslHeadEndEnvironment;
	unsigned long	LineAutoActivation;
	unsigned long	VendorNearId;

	// Buffer management settings
	unsigned long	RXBufAllocLimit;
	unsigned long	RXBufRatioLimit;
	unsigned long	RxMaxFrameSize;
	unsigned long	TxMaxFrameSize;
	unsigned long	MaxTransmit;

	// Frame abstraction settings
	unsigned long	RfcEncapsulationMode;
	unsigned long	CellBurstSize;
	unsigned long	PeakCellRate;

	unsigned long	OverrideMacAddress;
	char			MACAddress[6];

	char			MACPad[2];

	// auto logging parameters
	unsigned long	AutoLogEnabled;
	unsigned long	AutoLogMaxFileSize;
	unsigned long	AutoLogUpdateFrequency;
	unsigned long	AutoLogOverwrite;

	// misc
	unsigned long	ARMHwId;
	unsigned long	ADSLHwId;
	unsigned long	ARMFunctionIdentifier;
	unsigned long	ADSLFunctionIdentifier;
	unsigned long	HwDebugFlag;
	unsigned long	PhysicalDriverType;

	// GPIO initialization
	unsigned long	GpioADSLFunctionMask;
	unsigned long	GpioARMFunctionMask;
	unsigned long	GpioDirectionMask;
	unsigned long	GpioEdgeMask;

	// ADSL Card configuration
	// ******* THESE VALUES ARE SET, DURING DRIVER INITIALIZATION,
	// ******* FROM THE VALUES PASSED TO THE DRIVER
	// ******* DURING INSTALL, VIA "insmod" COMMAND LINE!
	DWORD						CnxtVendorId;
	DWORD						CnxtArmDeviceId;
	DWORD						CnxtAdslDeviceId;
	PHYSICAL_DRIVER_TYPE_T		CnxtPhysicalDriverType;
}TIG_USER_PARAMS, *PTIG_USER_PARAMS;




#endif //  _COMMONDATA_H_
