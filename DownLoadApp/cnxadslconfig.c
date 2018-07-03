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
**		CnxConfig.c
**
**	ABSTRACT:
**		This file implements the configuration utility for Tigris ADSL modem
**		driver.  The application will loop through the command line parameters
**		and issue IOCTL commands to the driver.
**
**	DETAILS:
**		????
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Yukon-AV/Linux_2.4_ATM_Driver/DownLoadApp/cnxadslconfig.c $
**	$Revision: 12 $
**	$Date: 5/22/01 10:12a $
*******************************************************************************
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
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
#include "atm.h"

// local functions
static void Usage( void );
static void Usage_Engineering( void );
static int AdjustIntParameter( char *pParameterLabelStr, char *pNewValueStr, char *pFormat );
static int ValidateInteger( char *pParameterLabelStr,  char *pNewValueStr, char Foramt );
static void ClearStatistics();
static int ReadParameter (  char *pParameterLabelStr, char *Found_Ptr ) ;
static void ReadAllParameters ( void ) ;
static void ReadCommonParameters ( void ) ;
static void PrintHeadendCodes ( void );
static int ParseParm ( char *optarg, char *ParmName, char *Format, char *Value ) ;

static int itf = 0; /*CLN*/

#define MAX_PARM_NAME_LEN	80
#define MAX_FORMAT_LEN		80
#define MAX_VALUE_LEN		80

/******************************************************************************
*	FUNCTION NAME:
*		main
*
*	ABSTRACT:
*		This is main entrypoint for the download executable.  It is
*		responsible for parsing the command line and sending the
*		commands.
*
*	PARAMETERS:
*	    argc - command line parameter count
*	    argv - command line parameters
*
*	RETURN:
*		The status of the operation
*
*	DETAILS:
*		See http://www.skip.adb.gu.se/~nacka/glibc/glibc_479.html#SEC486
*		for details on using getopt_long
******************************************************************************/

int main(int argc,char **argv)
{
	int c;
	int value;
	NTSTATUS Status=STATUS_SUCCESS;


	// show usage if nothing is requested
	if (argc == 1)
	{
		Usage();
	}

	while (1)
	{
		enum
		{
			PARM_INDEX,
			VPI_INDEX,
			VCI_INDEX
		} ;

		static struct option long_options[] =
		{
			{"parm", required_argument, 0, PARM_INDEX},
			{"vpi",  required_argument, 0, VPI_INDEX},
			{"vci",  required_argument, 0, VCI_INDEX},
			{0, 0, 0, 0}
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "a:ce:f:h:i:m:p:q:w:s:u:?",
						long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
		{
			break;
		}

		switch (c)
		{
		case PARM_INDEX:
		{
			char ParmName [MAX_PARM_NAME_LEN+1] ;
			char Format [MAX_FORMAT_LEN+1] ;
			char Value [MAX_VALUE_LEN+1] ;

			ParseParm ( optarg, ParmName, Format, Value ) ;

			Status = AdjustIntParameter( ParmName,  Value, Format  );
			break ;
		}

		case VPI_INDEX:
			// its all ok so set the value in the file
			Status = AdjustIntParameter( ATM_VPI,  optarg, ""  );
			break;

		case VCI_INDEX:
			// its all ok so set the value in the file
			Status = AdjustIntParameter( ATM_VCI,  optarg, ""  );
			break;

		case 'q' :

			// its all ok 
			switch ( *optarg )
			{
				case '0' :
					ReadCommonParameters () ;
					break ;

				case '1' :
					ReadAllParameters () ;
					break ;

				case 'h' :
					PrintHeadendCodes () ;
					break ;

				case 'e' :
					Usage () ;
					Usage_Engineering () ;
					break ;

				default:
					printf("Parameter error: Query must be between 0, 1 or \"h\"\n" ); 

			}
			break ;

		// h argument is for ADSL headend (Vendor)
		case 'h' :
		{
			// validate that it's an integer string
			Status = ValidateInteger( ADSL_HEAD_END,  optarg, 'x' );
			if (Status != STATUS_SUCCESS)
				break;

			// get the value and range check it

			sscanf(optarg,"%x",&value);
			if
			(
				(( value<2 ) || (value>0x51))
			&&
				( value != 0 )
			&&
				( value != 0xB6DB )
			)
			{
				printf("Parameter error: %s must be 0, 0xB6DB or between 2 and 0x51\n", ADSL_HEAD_END ); 
				break;
			}

			// its all ok so set the value in the file
			Status = AdjustIntParameter( ADSL_HEAD_END,  optarg, "%lx" );

			break ;
		}

		case 'i' :
		  itf = atoi(optarg);
		  break;

		// e argument is for ADSL headend Environment
		case 'e':
		{
			// validate that it's an integer string
			Status = ValidateInteger( ADSL_HEAD_END_ENVIRONMENT,  optarg, 'd' );
			if (Status != STATUS_SUCCESS)
				break;

			// get the value and range check it

			sscanf(optarg,"%d",&value);
			if (value>1)
			{
				printf("Parameter error: %s must be between 0 and 1\n", ADSL_HEAD_END_ENVIRONMENT ); 
				break;
			}

			// its all ok so set the value in the file
			Status = AdjustIntParameter( ADSL_HEAD_END_ENVIRONMENT,  optarg, "" );

			break;
		}

		// w argument is for wiring selection
		case 'w':
		{
			// validate that it's an integer string
			Status = ValidateInteger( AUTO_WIRING_SELECTION,  optarg, 'd' );
			if (Status != STATUS_SUCCESS)
				break;

			// get the value and range check it

			sscanf(optarg,"%d",&value);
			if (value>4)
			{
				printf("Parameter error: %s must be between 0 and 4\n", AUTO_WIRING_SELECTION ); 
				break;
			}

			// its all ok so set the value in the file
			Status = AdjustIntParameter( AUTO_WIRING_SELECTION,  optarg, ""  );

			break;
		}

		// m argument is for modulation
		case 'm':
			// validate that it's an integer string
			Status = ValidateInteger( AUTO_SENSE_HANDSHAKE,  optarg, 'd' );
			if (Status != STATUS_SUCCESS)
				break;

			// get the value and range check it
			sscanf(optarg,"%d",&value);
			if (value>3)
			{
				printf("Parameter error: %s must be between 0 and 3\n", AUTO_SENSE_HANDSHAKE ); 
				break;
			}

			// its all ok so set the value in the file
			Status = AdjustIntParameter( AUTO_SENSE_HANDSHAKE,  optarg, ""  );

			break;

		// a argument enables/disables autologging
		case 'a':
		{
			if ( !strcmp( optarg,"enable"))
				Status = AdjustIntParameter( AUTOLOG_ENABLED,  "1", ""  );
			else if ( !strcmp( optarg,"disable"))
				Status = AdjustIntParameter( AUTOLOG_ENABLED,  "0", ""  );
			else
			{
				Usage();
				Status = EFAIL;
			}
			break;
		}

		// f argument sets the autolog file name
		case 'f':
		{
			FILE *fptr;

			// first check to see if I can open the file for writing
			fptr = fopen( optarg, "wb");

			// if so then set the parameter
			if ( fptr > 0 )
			{
				Status = AdjustIntParameter( AUTOLOG_PATH,  optarg, ""  );
			}

			// otherwise error out
			else
			{
				printf("Failure: cannot create new log file.\n");
				Status = EFAIL;
			}

			break;
		}

		// u selects the autolog update frequency
		case 'u':
		{
			// validate that it's an integer string
			Status = ValidateInteger( AUTOLOG_UPDATE_FREQUENCY,  optarg, 'd' );
			if (Status != STATUS_SUCCESS)
				break;

			// its ok so set the value in the file
			Status = AdjustIntParameter( AUTOLOG_UPDATE_FREQUENCY,  optarg, ""  );

			break;
		}

		// p selects the peak cell rate in cells per second
		case 'p':
		{
			// validate that it's an integer string
			Status = ValidateInteger( PEAK_CELL_RATE,  optarg, 'd' );
			if (Status != STATUS_SUCCESS)
				break;

			// its ok so set the value in the file
			Status = AdjustIntParameter( PEAK_CELL_RATE,  optarg, ""  );

			break;
		}

		// s set the max autolog file size
		case 's':
		{
			// validate that it's an integer string
			Status = ValidateInteger( AUTOLOG_MAX_FILE_SIZE,  optarg, 'd' );
			if (Status != STATUS_SUCCESS)
				break;

			// its ok so set the value in the file
			Status = AdjustIntParameter( AUTOLOG_MAX_FILE_SIZE,  optarg, ""  );

			break;
		}


		// s set the max autolog file size
		case 'c':
			ClearStatistics();
			break;

		case '?':
		default:
		/* getopt_long already printed an error message. */
			Usage();
			return( 1 );
			break;
		}
	}

	/* Print any remaining command line arguments (not options). */
	if (optind < argc)
	{
		printf ("non-option ARGV-elements: ");
		while (optind < argc)
		{
			printf ("%s ", argv[optind++]);
		}
		putchar ('\n');
	}

	exit (0);
}



/******************************************************************************
*	FUNCTION NAME:
*		Usage
*
*	ABSTRACT:
*		This function prints out the usage script
*
*	PARAMETERS:
*		None
*
*	RETURN:
*		None
*
*	DETAILS:
******************************************************************************/
static void Usage()
{
printf("\
Usage: cnxadslconfig --vpi=0 --vci=35 -q0 -w0 -m0 -p100000 -c -h0 -e0 -aenable -u10 -flog.txt -s100000 \
Used to set Conexant AccessRunner(TM) ADSL modem configuration parameters. \
Note that values will not take effect until the driver is restarted. \
(e.g. cnxadslctl.sh stop / cnxadslctl.sh start)	\
Parameter definitions: \
     -i itf \
  Query Parameters: \
     -q Query \
          0 - query current common settings \
          1 - query current settings (all) \
          h - query ADSL headend choices from ANSI T1.413 Annex D \
 \
  ATM Parameters \
     --vpi VPI \
     --vci VCI \
 \
  AutoSense Parameters: \
     -w Wiring Selection: \
          0 - automatic wiring detection \
          1 - selects combo1 wiring \
          2 - selects combo2 wiring \
          3 - selects combo3 wiring \
          4 - selects combo4 wiring \
     -m Modulation: \
          0 - selects Auto modulation (G.HS preferred) \
          1 - selects Auto modulation (ANSI T1.413 preferred) \
          2 - selects G.HS modulation \
          3 - selects ANSI T1.413 modulation \
 \
  Data Traffic Parameters: \
     -p Peak Cell Rate in cells per second (0 for max) \
     -c Clears transfer statistics \
 \
  ADSL Headend Parameters: \
     -h ADSL headend (vendor) code from ANSI T1.413 annex D \
          0 - Non-specific \
          2-51, B6DB - hex value selects specific vendor \
                       (to see vendor codes, type cnxadslconfig -qh) \
     -e ADSL headend environment \
          0 - selects non-specific \
          1 - selects no line-driver \
 \
  Autolog Parameters: \
     -a enables or disables autologing: \
          enable  - enable logging \
          disable - disable logging \
     -f sets autolog filename \
     -u sets autolog update frequency in logs per 10 seconds \
     -s sets autolog max file size \
") ;
}


/******************************************************************************
*	FUNCTION NAME:
*		Usage
*
*	ABSTRACT:
*		This function prints out the usage script for the engineering functions
*
*	PARAMETERS:
*		None
*
*	RETURN:
*		None
*
*	DETAILS:
******************************************************************************/
static void Usage_Engineering()
{
printf("\
 \
  Miscellenous Parameters: \
     --parm PARM=[(FORMAT)]VAL \
          where PARM is any parameter name \
          where FORMAT is an optional format of the form: \
             %%[m]c \
                 where \"m\" is the optional length modifier \
                       b = byte \
                       h = short int \
                       l = long int (default) \
                 and \"c\" is the conversion character \
                       d = decimal \
                       x = hex \
                 default is %%ld \
          where VAL is any value \
          e.g. --parm \"DebugFlag=(%%lx)FFFFFFFF\" \
		  Note that quotes are required when using parenthesis. \
") ;
}





/******************************************************************************
*	FUNCTION NAME:
*		PrintHeadendCodes
*
*	ABSTRACT:
*		This function prints out the headend codes.
*
*	RETURN:
*		None
*
*	DETAILS:
******************************************************************************/

static void PrintHeadendCodes ( void )
{
	printf ( "ADSL headend choices from ANSI T1.413 Annex D (in hex): \
0000=Non-specific \
0002=Westell Inc. \
0003=ECI Telecom \
0004=Texas Instruments \
0005=Intel \
0006=Amati Communications Corp. \
0007=General Data Communication \
0008=Level One Communications \
0009=Crystal Semiconductor \
000A=AT&T - Network Systems \
000B=Aware, Inc. \
000C=Brooktree \
000D=NEC \
000E=Samsung \
000F=Northern Telecom, Inc. \
0010=PairGain Technologies \
0011=AT&T - Paradyne \
0012=Adtran \
0013=INC \
0014=ADC Telecommunications \
0015=Motorola \
0016=IBM Corp. \
0017=Newbridge Networks Corp. \
0018=DSC \
0019=TelTrend \
001A=Exar Corp. \
001B=Siemens Stromberg-Carlson \
001C=Analog Devices \
001D=Nokia \
001E=Ericsson Systems \
001F=Tellabs Operations, Inc. \
0020=Orckit Communications, Inc \
0021=AWA \
0022=Alcatel Network Systems In \
0023=National Semiconductor Sys \
0024=Italtel \
0025=SAT \
0026=Fujitsu Network Transmissi \
0027=MITEL \
0028=Conklin Instrument Corp. \
0029=Diamond Lane \
002A=Cabletron Systems, Inc. \
002B=Davicom Semiconductor, Inc \
002C=Metalink \
002D=Pulsecom \
002E=US Robotics \
002F=AG Communications Systems \
0030=Conexant Systems Inc. \
0031=Harris \
0032=Hayes Microcomputer Produc \
0033=Co-optic \
0034=Netspeed Inc. \
0035=3-Com \
0036=Copper Mountain Inc. \
0037=Silicon Automation Systems \
0038=Ascom \
0039=Globespan Inc. \
003A=ST Microelectronics \
003B=Coppercom \
003C=Compaq Computer Corp. \
003D=Integrated Technology Expr \
003E=Bay Networks, Inc. \
003F=Next Level Communications \
0040=Multitech Systems, Inc. \
0041=AMD \
0042=Sumitomo Electric \
0043=Phillips M&N Electronics \
0044=Efficient Networks, Inc. \
0045=Interspeed \
0046=Cisco Systems \
0047=Tollgrade Communciations, \
0048=Cayman Systems \
0049=FlowPoint Corp. \
004A=I.C.COM \
004B=Matsushita \
004C=Siemens Semiconductor \
004D=Digital Link \
004E=Digitel \
004F=Alcatel Microelectronics \
0050=Centillium Corp. \
0051=Applied Digital Access, In \
0052=Smart Link, Ltd. \
B6DB=Medialincs (Korean) \
" ) ;
}


/******************************************************************************
*	FUNCTION NAME:
*		GetPadding
*
*	ABSTRACT:
*		Return a string of spaces to pad the string to the desired column
*
*	PARAMETERS:
*		SizeOfString - size of the string to be padded
*		LeftCol - Column number of start of string
*		RightCol - Column number to pad string to
*
*	RETURN:
*		Ptr to string of spaces of desired length
*
*	DETAILS:
*		Calculates the number of spaces needed to pad the given string to
*		the given column. The string will always be at least one space long
*		to ensure the output fields are separated by spaces.
*		The variable length string is actually just a variable amount of the 
*		trailing portion of a fixed length string. This string is dynamically
*		initialized so that its size can be determined at compile time. To
*		statically intialize it would have required a static constant string that
*		can not be guaranteed to be of matching length.
******************************************************************************/

#define LEFT_COL 1
#define FORMAT_COL 27
#define VALUE_COL 31
#define RIGHT_COL VALUE_COL

char Spacing[RIGHT_COL] ;

static char * GetPadding ( int SizeOfString, int LeftCol, int RightCol )
{
	#define min(a,b) (((a)<(b))?(a):(b))

	int Offset ;

	// initialize spacing array if not already done
	if ( Spacing [0] == 0 )
	{
		int Index ;
		for ( Index = 0 ; Index < ( sizeof ( Spacing) -1 )  ; Index ++ )
		{
			Spacing [Index] = ' ' ;
		}
	}

	// start with offset to last space
	Offset = sizeof(Spacing) - 1 ;

	// backup number of spaces desired
	Offset -= RightCol - LeftCol ;

	// Then forward number of chars in string to subtract these
	Offset += SizeOfString ;

	// Do not exceed past the last space (guarantee one space -
	// and do not run past end of Spacing array if string is too long)
	Offset = min ( Offset, sizeof(Spacing) - 2 ) ;

	return & Spacing[Offset]  ;
}	

/******************************************************************************
*	FUNCTION NAME:
*		ParseParm
*
*	ABSTRACT:
*		This function parses a parm command of the form PARM=[(FORMAT)]VALUE
*
*	RETURN:
*		success or failure of test
*
*	DETAILS:
******************************************************************************/
static int ParseParm ( char *optarg, char *ParmName, char *Format, char *Value )
{
	int Start = 0 ;
	int Len ;
	char *pSep ;



	//*
	//* Extract PARM_NAME string
	//*

	// find end of parm name via search for "="
	pSep = strchr ( &optarg[Start], '=' ) ;
	if ( pSep == NULL )
	{
		// missing mandatory "+"
		printf ( "Missing \"=\" - %s\n", optarg ) ;
		return STATUS_FAILURE ;
	}

	// calc length of parm name
	Len = pSep - &optarg [Start] ;
	if ( Len > MAX_PARM_NAME_LEN )
	{
		// name is too long
		printf ( "Parameter name is too long - %s\n", optarg ) ;
		return STATUS_FAILURE ;
	}

	// copy the name
	strncpy ( ParmName, &optarg[Start], Len ) ;
	ParmName[Len] = 0 ;

	// calc new start of next field
	Start = Start + Len + 1 ;



	//*
	//* Extract optional FORMAT	string
	//*

	// Do we have an optional format (denoted by parenthesis)
	if ( optarg[Start] == '(' )
	{

		// calc start of actual format (past parenthesis)
		Start ++ ;

		// find end of format via search for closing parenthesis
		pSep = strchr ( optarg, ')' ) ;
		if ( pSep == NULL )
		{
			// missing mandatory closing parenthesis
			printf ( "Missing \")\" - %s\n", optarg ) ;
			return STATUS_FAILURE ;
		}

		// calc len of format
		Len = pSep- &optarg [Start];
		if ( Len > MAX_FORMAT_LEN )
		{
			// format is too long
			printf ( "Format is too long - %s\n", optarg ) ;
			return STATUS_FAILURE ;
		}

		// copy the format
		strncpy ( Format, &optarg[Start], Len ) ;
		Format[Len] = 0 ;

		// calc new start of next field
		Start = Start + Len + 1 ;
	}
	else
	{
		Format [0] = 0 ;
	}

	//*
	//* Extract Value string
	//*

	// find end of value via search for 0 string terminator
	pSep = strchr ( &optarg[Start], 0 ) ;
	if ( pSep == NULL )
	{
		// missing mandatory 0 string terminator
		printf ( "Missing value - %s\n", optarg ) ;
		return STATUS_FAILURE ;
	}

	// calc len of value
	Len = pSep - &optarg [Start] ;
	if ( Len > MAX_PARM_NAME_LEN )
	{
		// value is too long
		printf ( "Value name is too long - %s\n", optarg ) ;
		return STATUS_FAILURE ;
	}

	// copy the value
	strncpy ( Value, &optarg[Start], Len ) ;
	Value[Len] = 0 ;

	return STATUS_SUCCESS ;
}	



/******************************************************************************
*	FUNCTION NAME:
*		AdjustParameter
*
*	ABSTRACT:
*		Changes the parameters in the config file.  It will	create a new file 
*		and copy the old file into up to the parameter to be changed and then
*		replace the parameter and copy the rest of the old file.  If the parameter
*		is never found in the old file, it will append it to the end of the new 
*		file.  If all of this was successful, it delete the old file and rename
*		the new file.
*
*	PARAMETERS:
*		pParameterLabelStr - string defining the parameter
*		pNewValueStr - new value
*		pFormat - format to be used (optional)
*
*	RETURN:
*		success or failure of operation
*
*	DETAILS:
******************************************************************************/

static int AdjustIntParameter( char* pParameterLabelStr, char* pNewValueStr, char* pFormat )
{
	int Status = STATUS_SUCCESS;
	char* IStat;
	int   OStat;
	FILE *hOldFile, *hNewFile;
	char OldFileName[512],NewFileName[512];
	char LabelLine[512],*pLabel;
	
	// make the file names
	strcpy(OldFileName, CONFIGURATION_FILE_PATH);
	strcat(OldFileName, CONFIGURATION_FILENAME);
	
	strcpy(NewFileName, CONFIGURATION_FILE_PATH);
	strcat(NewFileName, SCRATCH_CONF_NAME);
	
	// open the files
	hOldFile = fopen( OldFileName, "r");
	if ( hOldFile <= 0 )
	{
		printf("User params file not found %s\n",OldFileName);
		return STATUS_FAILURE;
	}
	
	hNewFile = fopen( NewFileName, "wb");
	if ( hNewFile <= 0 )
	{
		printf("Failure: cannot create new parameter file.\n");
		return STATUS_FAILURE;
	}
	
	// loop copying parameters from the old file to the new file
	// until the requested parameter is found
	while ( Status == STATUS_SUCCESS )
	{
		// jump out on eof
		IStat = fgets(LabelLine, 256, hOldFile );
		if ( !IStat )
		{
			printf("Warning! User param not found: %s. Unrecognized param added.\n", pParameterLabelStr);
			break;
		}

		// skip leading spaces
		pLabel = LabelLine;
		while ((*pLabel == ' ')  ||
		       (*pLabel == '\t')    )
		{
			pLabel++;
		}

		// is this the label?
		if (strncmp( pLabel, pParameterLabelStr, strlen( pParameterLabelStr ) ) == 0 )
		{
			// make sure we matched all of the conf file's label
			if ( ( isspace ( pLabel[strlen( pParameterLabelStr )]) )
			||   ( pLabel[strlen( pParameterLabelStr )] == pFormat[0] ) )
			{
				// this is the label so fall out of the loop
				break;
			}
		}

		// it's a comment or the wrong label, just
		// copy it over
		OStat = fputs(LabelLine, hNewFile );
		if ( !OStat )
		{
			Status = EFAIL;
		}
	}
	
	// put the new parameter setting in the file
	if ( Status == STATUS_SUCCESS )
	{
		sprintf( LabelLine,"%s%s%s%s%s\n",
			pParameterLabelStr,		// Label
			GetPadding ( strlen(pParameterLabelStr), LEFT_COL, FORMAT_COL ),
			pFormat,				// Format
			GetPadding ( strlen(pFormat), FORMAT_COL, VALUE_COL ),
			pNewValueStr			// Value
		);

		OStat = fputs(LabelLine, hNewFile );
		if ( !OStat )
		{
			Status = EFAIL;
		}
	}
	
	// loop copying the rest of the parameters. note skip
	// the selected parameter if it is found again
	while ( Status == STATUS_SUCCESS )
	{
		// jump out on eof
		IStat = fgets(LabelLine, 256, hOldFile );
		if ( !IStat )
			break;

		// skip leading spaces
		pLabel = LabelLine;
		while ((*pLabel == ' ')  ||
		       (*pLabel == '\t')    )
		{
			pLabel++;
		}

		// is this the label?
		if (strncmp( pLabel, pParameterLabelStr, strlen( pParameterLabelStr ) ) == 0 )
		{
			// make sure we matched all of the conf file's label
			if ( ( isspace ( pLabel[strlen( pParameterLabelStr )]) )
			||   ( pLabel[strlen( pParameterLabelStr )] == pFormat[0] ) )
			{
				// ignore this line
				continue;
			}
		}

		// it's a comment or the wrong label, just
		// copy it over
		OStat = fputs(LabelLine, hNewFile );
		if ( !OStat )
		{
			Status = EFAIL;
		}
	}
	
	// close the files
	fclose( hNewFile );
	fclose( hOldFile );
	
	// if all clean to this point
	// delete the old file
	// rename the new file to the old name
	if ( Status == STATUS_SUCCESS )
	{
		unlink( OldFileName );
		rename( NewFileName, OldFileName );
	}
	
	// otherwise delete the new file
	else
		unlink( NewFileName );
	
	return Status;
}


/******************************************************************************
*	FUNCTION NAME:
*		ValidateInteger
*
*	ABSTRACT:
*		validates that the user requested value is an integer string
*
*	RETURN:
*		success or failure of test
*
*	DETAILS:
******************************************************************************/

int ValidateInteger( char *pParameterLabelStr,  char *pNewValueStr, char Format )
{
	int Status = STATUS_SUCCESS;
	
	// skip leading spaces
	while( *pNewValueStr == ' ')
	{
		pNewValueStr++;
	}

	// loop through all chars until an EOF
	do
	{
		switch ( Format )
		{
			case 'd' :
				if ((*pNewValueStr < '0') || (*pNewValueStr > '9'))
				{
					printf("Parameter failure: %s must be an integer value\n",pParameterLabelStr);
					Status = EFAIL;
					break;
				}
				break ;

			case 'x' :
				if (((*pNewValueStr < '0') || (*pNewValueStr > '9'))
				&&
				    ((*pNewValueStr < 'a') || (*pNewValueStr > 'f'))
				&&
				    ((*pNewValueStr < 'A') || (*pNewValueStr > 'F')))
				{
		
					printf("Parameter failure: %s must be a hexadecimal value\n", pParameterLabelStr);
					Status = EFAIL;
					break;
				}
				break ;
		}

		pNewValueStr++;
	} while (*pNewValueStr != 0);

	return Status;
} 


/******************************************************************************
*	FUNCTION NAME:
*		ClearStatistics
*
*	ABSTRACT:
*		This function sends an ioctl to driver to zero out statistics
*
*	PARAMETERS:
*		None
*
*	RETURN:
*		None
*
*	DETAILS:
******************************************************************************/
static void ClearStatistics(void)
{
	int SocketID;
	struct atmif_sioc Socketioc;
	char Dummy[8];
	
	// open the socket 
	SocketID = socket(PF_ATMPVC, SOCK_DGRAM, 0);
	if (SocketID < 0) 
	{
		printf("Socket open failure\n");
		return;
	}

	// build and issue the command
	Socketioc.number = itf;
	Socketioc.arg    = Dummy; // Linux doesn't like null here
	Socketioc.length = 8;     // so I am sending fake stuff

	ioctl( SocketID,
		   TIG_CLEAR_STATS,
		   &Socketioc); 
	 
	// close the socket
	close( SocketID );
}



/******************************************************************************
*	FUNCTION NAME:
*		ReadAllParameters
*
*	ABSTRACT:
*		Read and print the All parameters.
*
*	RETURN:
*		NA
*
*	DETAILS:
******************************************************************************/
static void ReadAllParameters ( void )
{
	int Status = STATUS_SUCCESS;
	char* IStat;
	FILE *hOldFile;
	char LabelLine[512] ;
	char *pLabel;

	// open the file
	hOldFile = fopen( CONFIGURATION_FILE_PATH CONFIGURATION_FILENAME, "r");
	if ( hOldFile <= 0 )
	{
		printf("User params file not found %s\n", CONFIGURATION_FILE_PATH CONFIGURATION_FILENAME);
		return ;
	}
	
	// loop copying parameters from the old file to the new file
	// until the requested parameter is found
	while ( Status == STATUS_SUCCESS )
	{
		// jump out on eof
		IStat = fgets(LabelLine, 256, hOldFile );
		if ( !IStat )
		{
			break;
		}

		// skip leading spaces
		pLabel = LabelLine;
		while ((*pLabel == ' ')  ||
		       (*pLabel == '\t')    )
		{
			pLabel++;
		}

		printf ( "%s", pLabel ) ;
	}
	
	
	// close the files
	fclose( hOldFile );
	
	return;
;
}	
	

/******************************************************************************
*	FUNCTION NAME:
*		ReadCommonParameters
*
*	ABSTRACT:
*		Read and print the supported parameters.
*
*	RETURN:
*		NA
*
*	DETAILS:
******************************************************************************/
static void ReadCommonParameters ( void )
{
	char Conf_Line [512] ;
	char *List [] =
	{
		AUTO_WIRING_SELECTION,
		AUTO_SENSE_HANDSHAKE,
		ADSL_HEAD_END_ENVIRONMENT,
		ADSL_HEAD_END,
		AUTOLOG_ENABLED,
		AUTOLOG_PATH,
		AUTOLOG_UPDATE_FREQUENCY,
		PEAK_CELL_RATE,
		AUTOLOG_MAX_FILE_SIZE,
		ATM_VPI,
		ATM_VCI,
		NULL
	} ;
	int Index ;

	for ( Index = 0 ; List[Index] != NULL ; Index++ )
	{
		if ( ReadParameter ( List[Index], Conf_Line ) == STATUS_SUCCESS )
		{
			printf ( "%s", Conf_Line ) ;
		}
	}
}


/******************************************************************************
*	FUNCTION NAME:
*		ReadParameter
*
*	ABSTRACT:
*		Read and print the given parameter.
*
*	RETURN:
*		success or failure of operation
*
*	DETAILS:
******************************************************************************/

static int ReadParameter (  char *pParameterLabelStr, char *Found_Ptr )
{
	int Status = STATUS_SUCCESS;
	char* IStat;
	FILE *hOldFile;
	//char OldFileName[512] ;
	char LabelLine[512] ;
	char *pLabel;

	*Found_Ptr = 0 ;

	// open the file
	hOldFile = fopen( CONFIGURATION_FILE_PATH CONFIGURATION_FILENAME, "r");
	if ( hOldFile <= 0 )
	{
		printf("User params file not found %s\n", CONFIGURATION_FILE_PATH CONFIGURATION_FILENAME);
		return STATUS_FAILURE;
	}
	
	// loop copying parameters from the old file to the new file
	// until the requested parameter is found
	while ( Status == STATUS_SUCCESS )
	{
		// jump out on eof
		IStat = fgets(LabelLine, 256, hOldFile );
		if ( !IStat )
		{
			printf("User param not found %s\n", pParameterLabelStr);
			break;
		}

		// skip leading spaces
		pLabel = LabelLine;
		while ((*pLabel == ' ')  ||
		       (*pLabel == '\t')    )
		{
			pLabel++;
		}

		if (! strncmp( pLabel, pParameterLabelStr, strlen( pParameterLabelStr ) ) )
		{
			// this is the label so fall out of the loop
			strcpy ( Found_Ptr, pLabel ) ;
			break;
		}
	}


	// close the files
	fclose( hOldFile );

	return Status;
}
