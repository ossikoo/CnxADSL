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
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux 2.4 ATM Driver/DownLoadApp/cnxadslstatus.c $
**	$Revision: 1 $
**	$Date: 3/29/01 8:24a $
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
DWORD	atoh( char* cInStr, DWORD uNumChars );

NTSTATUS  SendRecord(
			IN int		SocketID,
			IN int		CommandID,
			IN PVOID	cInRec,
			IN int		RecordLength);

NTSTATUS downLoadMicroCode (
				IN int	SocketID,
				IN int	FileHandle	);

NTSTATUS  SendUserParams(	IN int				SocketID,
							IN char*			Path,
							OUT unsigned long*	pAutoLogEnable,
							OUT char*			AutoLogPath,
							OUT char*			FirmwareFilename,
							OUT unsigned long*	pAutoLogFrequency,
							OUT unsigned long*	pAutoLogMaxSize,
							OUT unsigned long*	pAutoLogOverWrite );

void* BkGndThread(  );

void* SIDeviceThread(	IN unsigned long AutoLogEnable,
						IN char* AutoLogFile,
						IN unsigned long AutoLogFrequency,
						IN unsigned long AutoLogMaxSize,
						IN unsigned long AutoOverwrite  );

static int itf = 0; /*CLN*/

/*++
	main

Routine Description:

	This is main entrypoint for the download executable.  It will
	open the socket and make calls to first start the download,
	execute the download, and then complete the download.

Arguments:

	argc - command line parameter count
	argv - command line parameters

Return Value:

	The status of the operation.

--*/
int main(int argc,char **argv)
{
	int Status ;
	int SocketID;
	struct atmif_sioc Socketioc;
	ULONG				ShowTimeFlag;

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

	if (opt_err || ((argc - optind) != 0)) {
	  printf("Usage: cnxadslstatus [-h][-i itf] Path\n");
	  if (h_opt)
	    printf("-h     help\n"
		   "-i itf atm interface number\n");
	  exit(1);
	}

	// open the socket to the device.  This socket
	// will be used to issue the load commands
	SocketID = socket(PF_ATMPVC, SOCK_DGRAM, 0);
	if (SocketID < 0)
	{
		printf("Socket open failure\n");
		return STATUS_FAILURE;
	}

	// get the showtime status
	Socketioc.number = itf;
	Socketioc.arg    = &ShowTimeFlag;
	Socketioc.length = sizeof(ShowTimeFlag);

	Status = ioctl( SocketID, TIG_IS_SHOWTIME, &Socketioc);

	if (Status < 0)
	{
	        perror("");
	}
	else
	{
		if ( !ShowTimeFlag )
		{
			printf("NO Showtime!\n");
		}
		else
		{
			printf("Showtime.\n");
		}
	}

	// close out this socket
	close( SocketID );


	return 0 ;
}
