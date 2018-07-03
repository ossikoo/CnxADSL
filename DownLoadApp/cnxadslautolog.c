/******************************************************************************
********************************************************************************
****	Copyright (c) 2001
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
**		cnxadslautolog.c
**
**	ABSTRACT:
**		This file implements the auto log application for the Tigris ADSL
**		device.
**
**	DETAILS:
**		????
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4_ATM_Driver/DownLoadApp/cnxadslautolog.c $
**	$Revision: 1 $
**	$Date: 7/05/01 4:45p $
*******************************************************************************
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <getopt.h> /*CLN*/
#include <wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/atm.h>


#include "../LnxDefs.h"
#include "../CommonData.h"
#include "../ConfigLabels.h"

#include "../KernelModule/Product.h"
#include "../KernelModule/DevIo.h"

#include "atm.h"

#define SOCKET_LEN				256
#define POLL_INTERVAL_1_SEC		1
#define SHOWTIME_MSG			"Line connected.\n"
#define NO_SHOWTIME_MSG			"Line not connected!\n"
#define OPEN_WRITEONLY_EMPTY	O_WRONLY | O_TRUNC | O_CREAT
#define OPEN_WRITEONLY			O_WRONLY | O_CREAT
#define OPEN_WRITEONLY_APPEND	O_WRONLY | O_APPEND
#define FILE_PERMISSIONS		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP




const char		csShowTimeMsg[] = {SHOWTIME_MSG};
const char		csNoShowTimeMsg[] = {NO_SHOWTIME_MSG};




// local functions

NTSTATUS SendIoctlCmd(	IN int			SocketId,
						IN int			CommandId,
						IN PVOID		cInRec,
						IN int			RecordLength);

NTSTATUS ReadUserParams(	IN  char *				csPath,
							OUT unsigned long *		pAutoLogEnable,
							OUT char *				csAutoLogPath,
							OUT char *				csFirmwareFilename,
							OUT unsigned long *		pAutoLogFrequency,
							OUT unsigned long *		pAutoLogMaxSize,
							OUT unsigned long *		pAutoLogOverWrite );

void* DoAutoLogPoll(	IN unsigned long		AutoLogEnable,
						IN char *				csAutoLogFile,
						IN unsigned long		AutoLogFrequency,
						IN unsigned long		AutoLogMaxSize,
						IN unsigned long		AutoOverwrite );

static int itf = 0; /*CLN*/

/******************************************************************************
FUNCTION NAME:
	main

ABSTRACT:
	This is main entry point.

PARAMETERS:
	argc -	Command line parameter count
	argv -	Command line parameters

RETURN:
	The status of the operation.

DETAILS:
******************************************************************************/
int main( int argc,char **argv )
{
	NTSTATUS			NTStatus;
	char				csPath [256];
	char				csFileNameFromConf [512];
	char				csAutoLogFile [512];
	unsigned long		AutoLogEnable;
	unsigned long		AutoLogFrequency;
	unsigned long		AutoLogMaxSize;
	unsigned long		AutoLogOverwrite;

	int					c;
	int					h_opt = 0;
	int					opt_err = 0;

	while ((c = getopt(argc,argv,"hi:")) != EOF) { /*CLN*/
	  switch (c) {
	  case 'h' :
	    h_opt |= 1;
	    opt_err |= 1;
	    break;
	  case 'i' :
	    itf = atoi(optarg);
	    break;
	  default :
	    opt_err |= 1;
	    break;
	  }
	}

	if (opt_err || ((argc - optind) != 1)) {
	  printf("Usage: cnxadslautolog [-h][-i itf] Path\n");
	  if (h_opt)
	    printf("-h     help\n"
		   "-i itf atm interface number\n");
	  exit(1);
	}

	// save the path
	strcpy( csPath, argv[optind] );
	strcat( csPath, "/");


	// read the user configurable parameters from the driver configuration
	// file to get the autologging parameters.
	NTStatus = ReadUserParams(	csPath,
								&AutoLogEnable,
								csAutoLogFile,
								csFileNameFromConf,
								&AutoLogFrequency,
								&AutoLogMaxSize,
								&AutoLogOverwrite );
	if ( ! NT_SUCCESS(NTStatus) )
	{
		printf( "***ERROR - read user parameters (ReadUserParams) failed, NTStatus=%lx\n", NTStatus );
		return STATUS_FAILURE;
	}


	DoAutoLogPoll(	AutoLogEnable,
					csAutoLogFile,
					AutoLogFrequency,
					AutoLogMaxSize,
					AutoLogOverwrite );

	return STATUS_SUCCESS;
}




/******************************************************************************
FUNCTION NAME:
	DoAutoLogPoll

ABSTRACT:
	This function will open the socket and make calls to the driver to
	implement the automatic logging.

PARAMETERS:
	AutoLogEnable -		Flag enabling the logging function
	csAutoLogFile -		File to receive log data
	AutoLogFrequency -	Frequency to perform logging
	AutoLogMaxSize -	Maximum allowed size of the log file
	AutoLogOverwrite -	Clear the log file before starting

RETURN:
	None

DETAILS:
	This function periodically issues an IOCTL call to the driver to execute
	automatic logging of the ADSL driver events.
	The polling of the driver will continue until the one of the IOCTL
	commands fail, which probably indicates that the driver has been unloaded.
******************************************************************************/
void* DoAutoLogPoll(	IN unsigned long		AutoLogEnable,
						IN char *				csAutoLogFile,
						IN unsigned long		AutoLogFrequency,
						IN unsigned long		AutoLogMaxSize,
						IN unsigned long		AutoLogOverwrite )
{
	int						Status;
	long int				LogStat;
	int						SocketId;
	DWORD					Buffer [SOCKET_LEN];
	int						Length;
	int						FilePtr;
	TIG_DEVICE_SPEC_T		DevSpecific;
	BACK_DOOR_T *			pBackDoorBuf;
	ULONG					ShowTimeFlag;
	int						FileLength;


	pBackDoorBuf = (BACK_DOOR_T*) &DevSpecific.BackDoorBuf;
	Status = STATUS_SUCCESS;
	FileLength = 0;

	if ( AutoLogEnable )
	{
		DWORD		OpenFlags;


		// if overwriting then clear the file first
		if ( AutoLogOverwrite )
		{
			OpenFlags = OPEN_WRITEONLY_EMPTY;
		}
		else
		{
			OpenFlags = OPEN_WRITEONLY;
		}

		FilePtr = open(	csAutoLogFile, OpenFlags, FILE_PERMISSIONS );

		if ( FilePtr >= 0 )
		{
			close( FilePtr );
		}
		else
		{
			close( FilePtr );
			printf( "***ERROR - Failed to open AutoLog file!\n ");
			printf( "\nAutoLog NOT started!\n ");
			return NULL;
		}
	}
	else
	{
		printf( "AutoLog is not enabled in the driver configuration file.\n ");
		printf( "\nAutoLog NOT started!\n ");
		return NULL;
	}

	// open the socket to the device.  This socket
	// will be used to issue the load commands
	SocketId = socket( PF_ATMPVC, SOCK_DGRAM, 0 );
	if ( SocketId < 0 )
	{
		printf( "Socket open failure.\n" );
		printf( "\nAutoLog NOT started!\n" );
		return NULL;
	}

	// periodically poll the driver
	while ( Status == STATUS_SUCCESS )
	{
		// the poll should be right at one second.
		// But it's not really that critical because
		// the driver actually uses the realtime clock value for timing
		sleep( POLL_INTERVAL_1_SEC );

		// open the log file
		FilePtr = open(	csAutoLogFile, OPEN_WRITEONLY_APPEND );

		if ( FilePtr >= 0 )
		{
			FileLength = lseek( FilePtr, 0, SEEK_END );
		}

		if ( (FilePtr >= 0) && (!AutoLogMaxSize || (FileLength < AutoLogMaxSize)) )
		{
			// initialize the length
			Buffer[0] = 0;

			// call the function to get the log data
			LogStat = SendIoctlCmd(	SocketId,
									TIG_GET_LOG,
									&Buffer,
									SOCKET_LEN );
			if ( LogStat != STATUS_SUCCESS )
			{
				break;
			}

			Length = Buffer[0] >> 16;

			// if there is anything to log
			// then log it
			if ( (LogStat == STATUS_SUCCESS) && (Length != 0) )
			{
				write( FilePtr, &Buffer[3], strlen( (char*) &Buffer[3] ) );
			}

			// now log some additional statistics
			// get the showtime status
			LogStat = SendIoctlCmd(	SocketId,
									TIG_IS_SHOWTIME,
									&ShowTimeFlag,
									sizeof( ShowTimeFlag ) );
			if (LogStat != STATUS_SUCCESS)
			{
				printf( "The AutoLog IOCTL command to the driver failed!\n" );
				break;
			}

			// if not in showtime - log not connected
			if ( !ShowTimeFlag )
			{
				write( FilePtr, csNoShowTimeMsg, strlen( csNoShowTimeMsg ) );
			}
			else
			{
				BD_FRAMEAL_ATM_STATS_T *		pStats;
				char							PrintBuf [512];


				pStats = &pBackDoorBuf->Params.BdFrameALAtmStats;

				// get the stats

				memset( pBackDoorBuf, 0, sizeof( BACK_DOOR_T ) );
				pBackDoorBuf->TotalSize = sizeof( BACK_DOOR_T );
				pBackDoorBuf->ReqCode = BD_FRAMEAL_ATM_GET_STATS;
				pStats->VcIndex = BD_ATM_STATS_INDEX_FOR_LINK;

				LogStat = SendIoctlCmd(	SocketId,
										TIG_DEVICE_SPEC,
										&DevSpecific,
										sizeof( DevSpecific ) );
				if ( LogStat != STATUS_SUCCESS )
				{
					printf( "The AutoLog IOCTL command to the driver failed!\n" );
					break;
				}

				sprintf(	PrintBuf,
							"Cells Received: %ld   Transmitted: %ld\n",
							pStats->NumRxCells,
							pStats->NumTxCells);

				write( FilePtr, PrintBuf, strlen( PrintBuf ) );
			}

			// close the file
			close( FilePtr );
		}
	}

	printf( "\nAutoLog Terminated!\n" );

	// clean up after yourself before leaving
	close( SocketId );

	return NULL;
}




/******************************************************************************
FUNCTION NAME:
	ReadUserParams

ABSTRACT:
	

PARAMETERS:
		IN  csPath -				String indicating the path to the driver
									configuration file
		OUT pAutoLogEnable -		
		OUT csAutoLogPath -			
		OUT csFirmwareFilename -	
		OUT pAutoLogFrequency -		
		OUT pAutoLogMaxSize -		
		OUT pAutoLogOverwrite -		

RETURN:
	The status of the operation.

DETAILS:
	
******************************************************************************/
NTSTATUS ReadUserParams(	IN  char *				csPath,
							OUT unsigned long *		pAutoLogEnable,
							OUT char *				csAutoLogPath,
							OUT char *				csFirmwareFilename,
							OUT unsigned long *		pAutoLogFrequency,
							OUT unsigned long *		pAutoLogMaxSize,
							OUT unsigned long *		pAutoLogOverwrite)
{
	TIG_USER_PARAMS		TigParams;
	char				LabelLine [257];
	char *				pLabel;
	char *				pValue;
	FILE *				FileHandle;
	int					NumCfgEntries;
	char				csFileName [512];
	NTSTATUS			NTStatus;
	//
	// This structure defines the labels and offsets for each
	// of the user configurable parameters.
	//
	typedef struct _USER_CFG
	{
		char *		Label;
		ULONG *		Address;
		BOOL *		pConfiged;
	} USER_CFG;
	const USER_CFG		ConfigParams[] =
	{
		{RX_MAX_LATENCY,				&TigParams.RxMaxLatency,				NULL},
		{RX_MIN_LATENCY,				&TigParams.RxMinLatency,				NULL},
		{TX_MAX_LATENCY,				&TigParams.TxMaxLatency,				NULL},
		{TX_MIN_LATENCY,				&TigParams.TxMinLatency,				NULL},
		{RX_INTERRUPT_RATE,				&TigParams.RxInterruptRate,				NULL},
		{TX_INTERRUPT_RATE,				&TigParams.TxInterruptRate,				NULL},
		{RX_SPEED,						&TigParams.RxSpeed,						NULL},
		{TX_SPEED,						&TigParams.TxSpeed,						NULL},
		{RX_FIFO_SIZE,					&TigParams.RxFifoSize,					NULL},
		{TX_FIFO_SIZE,					&TigParams.TxFifoSize,					NULL},
		{RX_CHANNEL_SIZE,				&TigParams.RxChannelSize,				&TigParams.RxChannelSizeCfg},
		{RX_SEGMENTS,					&TigParams.RxSegments,					&TigParams.RxSegmentsCfg},
		{TX_CHANNEL_SIZE,				&TigParams.TxChannelSize,				&TigParams.TxChannelSizeCfg},
		{TX_SEGMENTS,					&TigParams.TxSegments,					&TigParams.TxSegmentsCfg},
		{ADSL_PSD_TEST_MODE,			&TigParams.AdslPsdTestMode,				NULL},
		{LINE_PERSISTENT_ACT,			&TigParams.LinePersistentAct,			NULL},
		{LINE_PERSISTENCE_TMR,			&TigParams.LinePersistenceTmr,			NULL},
		{RX_BUF_ALLOC_LIMIT,			&TigParams.RXBufAllocLimit,				NULL},
		{RX_BUF_RATIO_LIMIT,			&TigParams.RXBufRatioLimit,				NULL},
		{RX_MAX_FRAME_SIZE,				&TigParams.RxMaxFrameSize,				NULL},
		{TX_MAX_FRAME_SIZE,				&TigParams.TxMaxFrameSize,				NULL},
		{MAX_TRANSMIT,					&TigParams.MaxTransmit,					NULL},
		{RFC_ENCAPSULATION_MODE,		&TigParams.RfcEncapsulationMode,		NULL},
		{CELL_BURST_SIZE,				&TigParams.CellBurstSize,				NULL},
		{PEAK_CELL_RATE,				&TigParams.PeakCellRate,				NULL},
		{CONTROLLER_LOG_ENABLE,			&TigParams.CdALDiagControllerLogEnable,	NULL},
		{CAPABILITIES_G922,				&TigParams.BdADSLLocalG922Cap,			NULL},
		{CAPABILITIES_G922_ANNEX,		&TigParams.BdADSLLocalG922AnnexCap,		NULL},
		{LINE_AUTO_ACTIVATION,			&TigParams.LineAutoActivation,			NULL},
		{AUTO_SENSE_HANDSHAKE,			&TigParams.AutoSenseHandshake,			NULL},
		{AUTO_SENSE_WIRES_FIRST,		&TigParams.AutoSenseWiresFirst,			NULL},
		{AUTO_WIRING_SELECTION,			&TigParams.AutoWiringSelection,			NULL},
		{AUTO_WIRING_RELAY_DELAY,		&TigParams.AutoWiringRelayDelay,		NULL},
		{AUTO_WIRING_RELAY_ENRG,		&TigParams.AutoWiringRelayEnrg,			NULL},
		{AUTO_WIRING_NUM_COMBOS,		&TigParams.AutoWiringNumCombos,			NULL},
		{AUTO_WIRING_COMBO_GPIO,		(ULONG *) &TigParams.AutoWiringComboGPIO,NULL},
		{AUTO_WIRING_ORDER,				(ULONG *) &TigParams.AutoWiringOrder,	NULL},
		{ADSL_HEAD_END,					&TigParams.AdslHeadEnd,					NULL},
		{ADSL_HEAD_END_ENVIRONMENT,		&TigParams.AdslHeadEndEnvironment,		NULL},
		{VENDOR_NEAR_ID,				&TigParams.VendorNearId,				NULL},
		{DEBUG_FLAG,					&TigParams.DebugFlag,					NULL},
		{API_HWIF_MODE,					&TigParams.ApiHwIfMode,					NULL},
		{OVERRIDE_MAC_ADDRESS,			&TigParams.OverrideMacAddress,			NULL},
		{MAC_ADDRESS,					(ULONG *) &TigParams.MACAddress,		NULL},
		{AUTOLOG_ENABLED,				&TigParams.AutoLogEnabled,				NULL},
		{AUTOLOG_MAX_FILE_SIZE,			&TigParams.AutoLogMaxFileSize,			NULL},
		{AUTOLOG_UPDATE_FREQUENCY,		&TigParams.AutoLogUpdateFrequency,		NULL},
		{AUTOLOG_OVERWRITE,				&TigParams.AutoLogOverwrite,			NULL},
		{AUTOLOG_PATH,					(ULONG *) csAutoLogPath,				NULL},
		{ARM_HW_ID,						&TigParams.ARMHwId,						NULL},
		{ADSL_HW_ID,					&TigParams.ADSLHwId,					NULL},
		{ARM_FUNCTION_IDENTIFIER,		&TigParams.ARMFunctionIdentifier,		NULL},
		{ADSL_FUNCTION_IDENTIFIER,		&TigParams.ADSLFunctionIdentifier,		NULL},
		{HW_DEBUG_FLAG,					&TigParams.HwDebugFlag,					NULL},
		{PHYSICAL_DRIVER_TYPE,			&TigParams.PhysicalDriverType,			NULL},
		{GPIO_ADSL_FUNCTION_MASK,		&TigParams.GpioADSLFunctionMask,		NULL},
		{GPIO_ARM_FUNCTION_MASK,		&TigParams.GpioARMFunctionMask,			NULL},
		{GPIO_DIRECTION_MASK,			&TigParams.GpioDirectionMask,			NULL},
		{GPIO_EDGE_MASK,				&TigParams.GpioEdgeMask,				NULL},
		{FIRMWARE_FILENAME,				(ULONG *) csFirmwareFilename,			NULL}
	};


	// init
	NTStatus = STATUS_SUCCESS;
	LabelLine[256] = 0;
	NumCfgEntries = sizeof( ConfigParams ) / sizeof( USER_CFG );

	// set the entries to default values
	memset( &TigParams, 0, sizeof( TigParams ) );
	strcpy( csFirmwareFilename, "Default.hex" );


	// open the user configuration file
	strcpy( csFileName, csPath );
	strcat( csFileName, CONFIGURATION_FILENAME );
	FileHandle = fopen( csFileName, "r");
	if ( FileHandle <= 0 )
	{
		printf( "***ERROR - User params file (%s) not found or unable to open!\n", csFileName );
		return STATUS_FAILURE;
	}

	// loop while there are still lines in the file
	while ( 1 )
	{
		char *		ReadStat;
		int			CharCnt;


		// read a line and quit on eof
		ReadStat = fgets( LabelLine, 256, FileHandle );
		if ( !ReadStat )
		{
			break;
		}

		// skip leading spaces
		pLabel = LabelLine;
		while (	(*pLabel == ' ')  ||
				(*pLabel == '\t') ||
				(*pLabel == 0x0a) ||
				(*pLabel == 0x0d) )
		{
			pLabel++;
		}

		// skip it if its a comment. a comment starts
		// with any non-alpha character
		if ( !pLabel || !isalpha( *pLabel ) )
		{
			continue;
		}

		// find the value
		pValue = pLabel;
		for ( CharCnt = 0; CharCnt < 256; CharCnt++ )
		{
			if ( !isalpha( *pValue ) )
			{
				*pValue = 0;
				pValue++;
				break;
			}

			pValue++;
		}


		// find the label and set the value
		{
			long		cnt;

			// for each entry in table
			for ( cnt=0; cnt < NumCfgEntries; cnt++ )
			{
				// does this line match table entry?
				if ( !strcmp( pLabel, ConfigParams[cnt].Label ) )
				{
					long		value;
					void *		pStorage;		// location converted value is saved to
					BOOL *		pConfiged;		// location of indicator that value was found
					void *		pConv;			// location to convert value to
					char		CfgFormat [] = "%ld";	// default format
					char		ScanFormat [] = "%ld";	// default format


					// skip intervening whitespace
					while (	(*pValue == ' ') || (*pValue == '\t') )
					{
						pValue++;
					}

					// is there a format string?
					if ( *pValue == '%' )
					{
						// skip over '%'
						pValue ++;

						// does the format string have a length modifier
						switch ( *pValue )
						{
							case 'b':
							case 'h':
							case 'l':
								CfgFormat[1] = *pValue;
								pValue ++;
								break;
							default:
								CfgFormat[1] = 'l';
						}

						// default converstion location is temp var
						pConv = &value;

						// which conversion char is specified
						switch ( *pValue )
						{
							case 'x':
							case 'd':
								CfgFormat[2] = *pValue;
								ScanFormat[2] = *pValue;
								pValue ++;
								break;

							case 's':
								CfgFormat[1] = *pValue;
								ScanFormat[1] = *pValue;

								CfgFormat[2] = 0;
								ScanFormat[2] = 0;

								// convert strings rigth into storage location
								pConv = ConfigParams[cnt].Address;

								pValue ++;

								break;

							default:
								CfgFormat[2] = 'd';
								ScanFormat[2] = 'd';
								break;
						}
					}
					else
					{
						pConv = &value;
					}

					// Get ptr to where to store value
					pStorage = ConfigParams[cnt].Address;

					// convert each value
					while ( sscanf(pValue, ScanFormat , pConv) != EOF )
					{
						// transfer according to type
						switch ( CfgFormat[1] )
						{
							case 's':
							{
								// nothing to do, value was scanned to storage location
								break;
							}

							case 'b':
							{
								unsigned char *pbVal;

								pbVal = (unsigned char *) pStorage;
								* pbVal = (unsigned char) value;
								pbVal ++;
								pStorage = (ULONG *) pbVal;

								break;
							}

							case 'h':
							{
								unsigned short int *phVal;

								phVal = (unsigned short int *) pStorage;
								* phVal = (unsigned short int) value;
								phVal ++;
								pStorage = (ULONG *) phVal;

								break;
							}

							case 'l':
							default:
							{
								unsigned long int *plVal;

								plVal = (unsigned long int *) pStorage;
								* plVal = (unsigned long int) value;
								plVal ++;
								pStorage = (ULONG *) plVal;

								break;
							}
						}

						// skip to next field
						while ( *pValue != ',' )
						{
							if ( *pValue == 0 )
							{
								pValue --;
								break;
							}
							pValue ++;
						}
						pValue ++;
					}

					// Get ptr to where to store indicator that it was found
					pConfiged = ConfigParams[cnt].pConfiged;
					if ( pConfiged != NULL )
					{
						*pConfiged = TRUE;
					}

					// exit label search loop
					break;
				}
			}
		}
	}

	// close the file
	fclose( FileHandle );

	*pAutoLogEnable = TigParams.AutoLogEnabled;
	*pAutoLogFrequency = TigParams.AutoLogUpdateFrequency;
	*pAutoLogMaxSize = TigParams.AutoLogMaxFileSize;
	*pAutoLogOverwrite = TigParams.AutoLogOverwrite;

	return NTStatus;
}




/******************************************************************************
FUNCTION NAME:
	SendIoctlCmd

ABSTRACT:
	This function is called to send an IOCTL to the device driver.

PARAMETERS:
	SocketId -		identifies the socket to send to
	CommandId -		
	cInRec -		pointer to the record data to send
	RecordLength -	length of the record in bytes

RETURN:
	The status of the operation.

DETAILS:
******************************************************************************/
NTSTATUS SendIoctlCmd(	IN int			SocketId,
						IN int			CommandId,
						IN PVOID		cInRec,
						IN int			RecordLength )
{
	int						Status;
	struct atmif_sioc		Socketioc;


	Socketioc.number = itf;
	Socketioc.arg = cInRec;
	Socketioc.length = RecordLength;

	Status = ioctl( SocketId, CommandId, &Socketioc );
	if (Status < 0)
	{
		printf( "The AutoLog IOCTL command (0x%x) to the driver failed (Status=0x%x)!\n", CommandId, Status);
		return STATUS_FAILURE;
	}

	return STATUS_SUCCESS;
}
