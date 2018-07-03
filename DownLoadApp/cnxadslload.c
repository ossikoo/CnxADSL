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
**		CnxLoad.c
**
**	ABSTRACT:
**		This file implements the download application for the Tigris ADSL
**		device.  It will open and read the hex record file, process the
**		record into binary data and then send them to the driver for
**		execution.
**
**	DETAILS:
**		????
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4_ATM_Driver/DownLoadApp/cnxadslload.c $
**	$Revision: 4 $
**	$Date: 7/05/01 9:50p $
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

#include "cnxadslload.h"

#include "../KernelModule/Product.h"
#include "../KernelModule/DevIo.h"
#include "atm.h"

// local functions

DWORD
atoh(
	CHAR *		cInStr,
	DWORD		uNumChars );

NTSTATUS
SendRecord(
	IN int			SocketID,
	IN int			CommandID,
	IN PVOID		cInRec,
	IN int			RecordLength );

NTSTATUS
downLoadMicroCode(
	IN int		SocketID,
	IN int		FileHandle );

NTSTATUS
SendUserParams(
	IN int					SocketID,
	IN char *				FileName,
	OUT unsigned long *		pAutoLogEnable,
	OUT char *				AutoLogPath,
	OUT char *				FirmwareFilename,
	OUT unsigned long *		pAutoLogFrequency,
	OUT unsigned long *		pAutoLogMaxSize,
	OUT unsigned long *		pAutoLogOverWrite );

static int itf = 0; /*CLN*/

/******************************************************************************
FUNCTION NAME:
	main

ABSTRACT:
	This is main entrypoint for the download executable.  It will
	open the socket and make calls to first start the download,
	execute the download, and then complete the download.

PARAMETERS:
	argc - command line parameter count
	argv - command line parameters

RETURN:
	The status of the operation.

DETAILS:
******************************************************************************/
int main(int argc, char **argv)
 {
 NTSTATUS				NTStatus=STATUS_SUCCESS;
 int						Status;
 int						SocketID;
 struct atmif_sioc		Socketioc;
 int						FileHandle;
 char					Dummy [8];
 char					FileName [512];
 char					FileNameFromConf [512];
 char					AutoLogFile [512];
 unsigned long			AutoLogEnable;
 unsigned long			AutoLogFrequency;
 unsigned long			AutoLogMaxSize;
 unsigned long			AutoLogOverwrite;

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
	  printf("Usage: cnxadslload [-h][-i itf] Path\n");
	  if (h_opt)
	    printf("-h     help\n"
		   "-i itf atm interface number\n");
	  exit(1);
	}

	// open the socket to the device.  This socket
	// will be used to issue the load commands
	SocketID = socket( PF_ATMPVC, SOCK_DGRAM, 0 );
	if ( SocketID < 0 )
	{
		printf( "Socket open failure\n" );
		return STATUS_FAILURE;
	}

 // send the user configurable parameters to the
 // driver and get the autologging parameters
 NTStatus=SendUserParams(SocketID,
			 argv[optind],
			 &AutoLogEnable,
			 AutoLogFile,
			 FileNameFromConf,
			 &AutoLogFrequency,
			 &AutoLogMaxSize,
			 &AutoLogOverwrite);
 if ( ! NT_SUCCESS(NTStatus))
  {
  printf( "send user parameters (SendUserParams) failed, NTStatus=%lx\n", NTStatus );
  return STATUS_FAILURE;
  }


 // open the firmware file and validate that we got it open
 if (FileNameFromConf[0] == '/')
     strcpy(FileName, FileNameFromConf);
 else
 {
     strcpy(FileName, "/usr/lib/");
     strcat(FileName, FileNameFromConf);
 }
 FileHandle=open(FileName, O_RDONLY);
 if (FileHandle < 0)
  {
  printf( "Firmware file not found (%s)\n", FileName );
  return STATUS_FAILURE;
  }

 // issue the prepare for download command
 Socketioc.number=itf;
 Socketioc.arg=Dummy;  		// Linux doesn't like null here
 Socketioc.length = 8;		// so I am sending fake stuff

 Status=ioctl(SocketID, TIG_LOAD_START, &Socketioc);

 // if status is success then it needs a download.
 // failure indicates that the download is already
 // done and therefore not required
 if ( Status >= 0 )
  {
  // call a function to execute the download
  NTStatus=downLoadMicroCode(SocketID, FileHandle);

  // close the firmware file
  close(FileHandle);

  if (!NT_SUCCESS(NTStatus))
   {
   printf( " download ARM micro code (downLoadMicroCode) failed, NTStatus=%lx\n", NTStatus );
   return STATUS_FAILURE;
   }

  // issue the download complete command
  Status=ioctl( SocketID, TIG_LOAD_DONE, &Socketioc );
  if (Status < 0)
   {
   printf( " bad ioctl=%x (TIG_LOAD_DONE), Status=%x\n", TIG_LOAD_DONE, Status );
   return STATUS_FAILURE;
   }
  }
 else
   {
   // close the firmware file
   close(FileHandle);
   printf( " download is already done, Status=%x\n", Status);
   }

 // issue the start dsl function command
 Status = ioctl( SocketID, TIG_START_DSL, &Socketioc );
 if (Status < 0)
  {
  printf( " bad ioctl=%x (TIG_START_DSL), Status=%x\n", TIG_START_DSL, Status );
  return STATUS_FAILURE;
  }
 else
  {
  printf( "\nConexant AccessRunner load complete!\n" );
  }

 // close out this socket
 close(SocketID);
 
 return 0;
 }




/****************************************************************************
The	Following functions	were taken directly	from the NB	RipTide	Cmdif.c	/ Cmdif.h files
then modified to allow either CR/LF or just LF for Linux
****************************************************************************/

/******************************************************************************
FUNCTION NAME:
	downLoadMicroCode

ABSTRACT:
	Use	IFSMgr services	to download	a controller
	code HEX file to the RAM of	the	device

PARAMETERS:
	char *sFN is the pathname of the file

RETURN:
	Error code 0 --	success, otherwise fail

DETAILS:
******************************************************************************/
NTSTATUS downLoadMicroCode(IN int SocketID, IN int FileHandle)
 {
	DWORD					BytesRead;
	DWORD					dwSegAddr = 0;			// Extended	segment	address
	DWORD					dwLinAddr = 0;			// Extended	linear address
	DWORD					uRecType;				// extracted record type
	DWORD					dwIPValue;
	BOOL					FoundLastRecord = FALSE;
	HEX_HEADER_T *			pRecHeader;			// ptr to location in LoadRecord of hex header
	DWORD					RecordLen;				// extracted and coverted to binary record length
	NTSTATUS				NTStatus = STATUS_SUCCESS;
	CHAR *					pBuffer;				// ptr to buffer within LoadRecord
	TIG_LOAD_RECORD_T *		pLoadRecord = NULL;	// image of one line from hex file
	BOOL					MarkFound;

	//
	// Setup
	//

	// allocate a record and point the header into it
	pLoadRecord = malloc( MAX_RECORD_LENGTH );
	if ( !pLoadRecord )
	{
		printf( "Error - Insufficient memory, Load terminating\n" );
		return -ENOMEM;
	}

	pRecHeader = (HEX_HEADER_T*) &pLoadRecord->RecordData[0];
	pBuffer = &pLoadRecord->RecordData[0];


	//
	//	Read file and process each line
	//
	do
	{
		// read char by char until we find Record Header Mark skipping all whitespace
		MarkFound = FALSE;
		while ( TRUE )
		{
			//* Read 1 byte searching for Record Header mark
			BytesRead = read( FileHandle, (PVOID)pRecHeader->Mark, 1 );
			if ( BytesRead != 1 )
			{
				NTStatus = STATUS_FAILURE;
				printf( "Error - Read header fail, Load terminating\n" );
				break;
			}

			// exit if Mark found
			if ( *pRecHeader->Mark == ':' )
			{
				MarkFound = TRUE;
				break;
			}

			// otherwise must be whitespace
			if ( ! isspace( *pRecHeader->Mark ) )
			{
				NTStatus = STATUS_FAILURE;
				printf( "Error - Read header fail, Load terminating\n" );
				break;
			}
		}
		if ( ! MarkFound )
		{
			break;
		}


		//* Terminate Buffer to ensure we find good data
		pRecHeader->Mark[1] = 0x00;

		//* Read Record Header into location after Mark
		BytesRead = read( FileHandle, &pRecHeader->Mark[1], sizeof( HEX_HEADER_T )-1 );

		if(BytesRead != (sizeof(HEX_HEADER_T)-1))
		{
			NTStatus = STATUS_FAILURE;
			printf( "Error - Read header fail, Load terminating\n" );
			break;
		}

		// Determine variable record length if applicable
		RecordLen = atoh(pRecHeader->Length, REC_DATA_LEN);
		RecordLen *= 2;				// Two Bytes are used to represent each hex digit

		//* Read The Rest of the Record. At minimum, read Checksum
		RecordLen +=  REC_CHECKSUM_LEN;
		if ( MAX_RECORD_LENGTH < (sizeof( HEX_HEADER_T ) + sizeof( TIG_LOAD_RECORD_T ) + RecordLen) )
		{
			NTStatus = STATUS_FAILURE;
			printf( "Error - Record length error, Load terminating\n" );
			break;
		}

		BytesRead = read(
						FileHandle,
						(PVOID) (pBuffer + sizeof( HEX_HEADER_T )),
						RecordLen);

		if(BytesRead != RecordLen)
		{
			NTStatus = STATUS_FAILURE;
			printf( "Error - Record read failure, Load terminating\n" );
			break;
		}

		//* Do not consider Checksum as part of the real data record
		RecordLen -= REC_CHECKSUM_LEN;

		//* Parse the record type
		uRecType = atoh( pRecHeader->Type, REC_TYPE_LEN );

		switch(	uRecType )
		{

			case DATA_REC:
				//*
				//* Data Record
				//*
				pLoadRecord->RecordType		= DATA_REC;
				pLoadRecord->RecordAddress	= dwLinAddr + dwSegAddr;
				pLoadRecord->RecordLength	= RecordLen + sizeof( HEX_HEADER_T );

				NTStatus = SendRecord(
								SocketID,
								TIG_LOAD_RECORD,
								pLoadRecord,
								pLoadRecord->RecordLength );
				break;

			case EXT_SEG_ADDR_REC:
				//*
				//* Extended Segment Address
				//*
				dwSegAddr =	atoh( &pBuffer[ sizeof(HEX_HEADER_T) ], RecordLen ) << 4;
				break;

			case EXT_LIN_ADDR_REC:
				//*
				//* Extended Linear Address
				//*
				dwLinAddr =	atoh( &pBuffer[sizeof( HEX_HEADER_T )], RecordLen ) << 16;
				break;

			case EXT_GOTO_CMD_REC:
				//*
				//* Start Segment Address (GOTO)
				//*
				dwIPValue =	atoh( &pBuffer[sizeof( HEX_HEADER_T )], RecordLen );

				pLoadRecord->RecordType		= EXT_GOTO_CMD_REC;
				pLoadRecord->RecordAddress	= dwIPValue;
				pLoadRecord->RecordLength	= 0;

				// execute the record
				NTStatus = SendRecord(
								SocketID,
								TIG_LOAD_RECORD,
								pLoadRecord,
								sizeof( TIG_LOAD_RECORD_T ) );
				break;

			case EXT_END_OF_FILE:
				//*
				//* End Of File
				//*
				FoundLastRecord = TRUE;
				break;

			default:
				//*
				//* Unknown Record
				//*
				NTStatus = STATUS_FAILURE;
				printf( "Error - Invalid record type, Load terminating\n" );
				break;
		}

	} while ( !FoundLastRecord && NT_SUCCESS( NTStatus ) );

	if ( pLoadRecord )
		free( pLoadRecord );

	//*
	//* Make Sure Download completed
	//*
	if( !FoundLastRecord )
	{
		printf( "Last record not found - load terminating\n" );
		NTStatus = STATUS_FAILURE;
	}

	return NTStatus;
}




/******************************************************************************
FUNCTION NAME:
	atoh

ABSTRACT:
	Converts ASCII hex to DWORD

PARAMETERS:
	char *cInStr = pointer to string
	DWORD uNumChars	= number of	characters in the string

RETURN:
	none

DETAILS:
******************************************************************************/
DWORD
atoh(
	CHAR *		cInStr,
	DWORD		uNumChars )
{
	DWORD uSum = 0;
	DWORD uMult	= 1;

	while( uNumChars )
	{
		if (		cInStr[uNumChars - 1] >= '0' && cInStr[uNumChars - 1] <= '9' )
		{
			uSum += uMult * (cInStr[uNumChars - 1] - '0');
		}

		else if	(	cInStr[uNumChars - 1] >= 'A' &&	cInStr[uNumChars - 1] <= 'F' )
		{
			uSum += uMult * (cInStr[uNumChars - 1] - 'A' + 10);
		}

		else if	(	cInStr[uNumChars - 1] >= 'a' &&	cInStr[uNumChars - 1] <= 'f' )
		{
			uSum += uMult * (cInStr[uNumChars - 1] - 'a' + 10);
		}

		uMult *= 16;
		--uNumChars;
	}

	return uSum;
}




/******************************************************************************
FUNCTION NAME:
	SendRecord

ABSTRACT:
	This function is called to send a load record to the
	modem device.

PARAMETERS:
	SocketID - identifies the socket to send to
	cInRec - pointer to the record data to send
	RecordLength - length of the record in bytes

RETURN:
	The status of the operation.

DETAILS:
******************************************************************************/
NTSTATUS
SendRecord(
	IN int		SocketID,
	IN int		CommandID,
	IN PVOID	cInRec,
	IN int		RecordLength )
{
	int						Status;
	struct atmif_sioc		Socketioc;

	Socketioc.number = itf;
	Socketioc.arg = cInRec;
	Socketioc.length = RecordLength;

	Status = ioctl( SocketID, CommandID, &Socketioc );
	if ( Status < 0 )
	{
		printf( " bad ioctl %x, Status=%x\n", CommandID, Status );
		return STATUS_FAILURE;
	}

	return STATUS_SUCCESS;
}




/******************************************************************************
FUNCTION NAME:
	SendUserParams

ABSTRACT:
	This function is called to send a user configurable parameters
	to the device driver.

PARAMETERS:
	SocketID - identifies the socket to send to

RETURN:
	The status of the operation.

DETAILS:
******************************************************************************/
NTSTATUS
SendUserParams(
	IN  int					SocketID,
	IN  char *				FileName,
	OUT unsigned long *		pAutoLogEnable,
	OUT char *				AutoLogPath,
	OUT char *				FirmwareFilename,
	OUT unsigned long *		pAutoLogFrequency,
	OUT unsigned long *		pAutoLogMaxSize,
	OUT unsigned long *		pAutoLogOverwrite )
{
	TIG_USER_PARAMS		TigParams;
	char				LabelLine [257];
	char *				pLabel;
	char *				pValue;
	FILE *				FileHandle;
	int					NumCfgEntries;
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

	const USER_CFG  ConfigParams[] =
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
		{AUTOLOG_PATH,					(ULONG *) AutoLogPath,					NULL},
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
		{FIRMWARE_FILENAME,				(ULONG *) FirmwareFilename,				NULL}
	};

	// init
	LabelLine[256] = 0;
	NumCfgEntries = sizeof( ConfigParams ) / sizeof( USER_CFG );

	// set the entries to default values
	memset ( &TigParams, 0, sizeof ( TigParams ) );
	strcpy( FirmwareFilename, "Default.hex" );


	// open the user configuration file
	FileHandle = fopen( FileName, "r" );
	if ( FileHandle <= 0 )
	{
		printf( "User params file not found %s\n", FileName );
		return STATUS_FAILURE;
	}

	// loop while there are still lines in the file
	while ( 1 )
	{
		char*	ReadStat;
		int		CharCnt;

		// read a line and quit on eof
		ReadStat = fgets( LabelLine, 256, FileHandle );
		if ( !ReadStat )
			break;

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
			continue;

		// find the value
		pValue = pLabel;
		for ( CharCnt=0; CharCnt<256; CharCnt++ )
		{
			if ( !isalpha(*pValue) )
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
			for ( cnt=0; cnt<NumCfgEntries; cnt++ )
			{
				// does this line match table entry?
				if ( !strcmp(pLabel,ConfigParams[cnt].Label) )
				{
					long		value;
					void *		pStorage;				// location converted value is saved to
					BOOL *		pConfiged;				// location of indicator that value was found
					void *		pConv;					// location to convert value to
					char		CfgFormat[] = "%ld";	//default format
					char		ScanFormat [] = "%ld";	// default format


					// skip intervening whitespace
					while (	(*pValue == ' ')  ||
							(*pValue == '\t') )
					{
						pValue++;
					}

					// is there a format string?
					if ( *pValue == '%' )
					{
						// skip over '%'
						pValue ++;

						// does the format string have a length modifier
						switch ( *pValue  )
						{
							case 'b' :
							case 'h' :
							case 'l' :
								CfgFormat[1] = *pValue;
								pValue ++;
								break;
							default:
								CfgFormat[1] = 'l';
						}

						// default converstion location is temp var
						pConv = &value;

						// which conversion char is specified
						switch ( *pValue  )
						{
							case 'x' :
							case 'd' :
								CfgFormat[2] = *pValue;
								ScanFormat[2] = *pValue;
								pValue ++;
								break;

							case 's' :
								CfgFormat[1] = *pValue;
								ScanFormat[1] = *pValue;

								CfgFormat[2] = 0;
								ScanFormat[2] = 0;

								// convert strings rigth into storage location
								pConv = ConfigParams[cnt].Address;

								pValue ++;

								break;

							default :
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
							case 's' :
							{
								// nothing to do, value was scanned to storage location
								break;
							}

							case 'b' :
							{
								unsigned char *		pbVal;

								pbVal = (unsigned char *) pStorage;
								* pbVal = (unsigned char) value;
								pbVal ++;
								pStorage = (ULONG *) pbVal;

								break;
							}

							case 'h' :
							{
								unsigned short int *		phVal;

								phVal = (unsigned short int *) pStorage;
								* phVal = (unsigned short int) value;
								phVal ++;
								pStorage = (ULONG *) phVal;

								break;
							}

							case 'l' :
							default:
							{
								unsigned long int *		plVal;

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

	// send the data to the driver
	NTStatus = SendRecord(
		SocketID,
		TIG_SET_PARAMS,
		&TigParams,
		sizeof( TigParams) );
	if ( NT_SUCCESS( NTStatus ) )
	{
		if ( ! NT_SUCCESS( TigParams.CommandStatus ) )
		{
			NTStatus = STATUS_FAILURE;
			printf( "TIG_SET_PARAMS IOCTL returned error in parameters, CommandStatus=%lx\n", TigParams.CommandStatus );
		}
	}

	return NTStatus;
}
