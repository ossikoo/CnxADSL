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
**		LnxDefs.h
**
**	ABSTRACT:
**		This file contains type definitions to ease the port from the windows
**		world to the Linux world.
**
**	DETAILS:
**		
**
*******************************************************************************
** KEYWORDS:
**	$Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux_2.2.16_ATM_Driver/LnxDefs.h $
**	$Revision: 6 $
**	$Date: 5/21/01 1:36p $
*******************************************************************************
******************************************************************************/

/////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifndef _LNXDEFS_H_
#define _LNXDEFS_H_


extern int		errno;

typedef long int				NTSTATUS;
typedef long int				NDIS_STATUS;

#ifdef INT
#undef INT
#endif

typedef int						INT;
typedef unsigned int			UINT;
typedef unsigned char			UCHAR;
typedef unsigned short			USHORT;
typedef unsigned long			ULONG;
typedef unsigned long *			PULONG;
typedef void					VOID;
typedef void *					PVOID;

typedef unsigned short			WORD;
#define WORD_MAX ((WORD)-1)
typedef unsigned long			DWORD;
typedef unsigned char			BYTE;
#define BYTE_MAX ((BYTE)-1)
typedef signed char				CHAR;
typedef signed short int		SHORT;
typedef signed long int			LONG;
typedef long long int			LONGLONG;
#define LONGLONG_MAX ((LONGLONG)(((unsigned long long)-1)>>1))
typedef unsigned long long int	ULONGLONG;
typedef long long int			LARGE_INTEGER;
typedef unsigned char *			PUCHAR;

typedef UCHAR					BOOL;
typedef UCHAR					BOOLEAN;
typedef BOOLEAN *				PBOOLEAN;

typedef void *					PFILE_OBJECT;

typedef struct					sk_buff SK_BUFF_T;
typedef struct					sk_buff_head SK_BUFF_QUEUE_T;



#define KdPrint( x )		printk x

#define TRUE				1
#define FALSE				0

// #define LONG_MIN	(-LONG_MAX - 1L)
#ifndef INT_MIN
#define INT_MIN				(-INT_MAX - 1)
#endif


#define IN
#define OUT
#define INOUT
#define I_O
#define GLOBAL

#define ASSERT( X )
#define ASSERTMSG( X, Y )

#define NDIS_STRING_CONST( x )	x
typedef char *					NDIS_STRING;
typedef char *					PNDIS_STRING;
typedef DWORD					NDIS_PARAMETER_TYPE;
#define	NdisParameterInteger	1
#define	NdisParameterString		2

enum ConnectStatus
{
    MediaNotConnected,
    MediaConnected
};

#define TRAP()					asm("int3")

#define NdisWriteErrorLogEntry( Handle, Code, NumValues... )



/////////////////////////////////////////////////////////////////////
// Error Codes
/////////////////////////////////////////////////////////////////////
#define EFAIL				1024
#define ERR_NOFLOAT			-2048
#define ERR_FORMAT			-2049
#define ERR_SHUTDOWN		-2050

#define	EALREADYLOADED		(-STATUS_IMAGE_ALREADY_LOADED)				/* device already loaded */
#define	EINVALIDTYPE		(-STATUS_BAD_MASTER_BOOT_RECORD)			/* invalid record type */
#define	ETIMEOUT			(-STATUS_IO_TIMEOUT)						/* command timeout */
#define	ENOTREADY			(-STATUS_DEVICE_NOT_READY)					/* device not ready */
#define	EBADLENGTH			(-STATUS_INVALID_BLOCK_LENGTH)				/* invalid length */
#define EDEVICEFAIL			(-STATUS_IO_DEVICE_ERROR)					/* requested IO failed */
#define ENOINTERRUPT		(-STATUS_BIOS_FAILED_TO_CONNECT_INTERRUPT)	/* requested interrupt unavailable */
#define ESVCINVAL			(-STATUS_INVALID_DEVICE_REQUEST)			/* invalid service */
#define COMMAND_PENDING		(-STATUS_PENDING)							/* command is pending */
#define STATUS_UNSUPPORTED	(-STATUS_NOT_SUPPORTED)						/* command is not supported */
//#define STATUS_INVALID_PARAMETER	(-STATUS_INVALID_PARAMETER)			/*invalid parameter passed in to function */


//
// Generic test for success on any status value (non-negative numbers
// indicate success).
//

#define NT_SUCCESS(Status)			((NTSTATUS)(Status) >= 0)

//
// Generic test for information on any status value.
//

#define NT_INFORMATION(Status)		((ULONG)(Status) >> 30 == 1)

//
// Generic test for warning on any status value.
//

#define NT_WARNING(Status)			((ULONG)(Status) >> 30 == 2)

//
// Generic test for error on any status value.
//

#define NT_ERROR(Status)			((ULONG)(Status) >> 30 == 3)

// begin_winnt
#define APPLICATION_ERROR_MASK				0x20000000
#define ERROR_SEVERITY_SUCCESS				0x00000000
#define ERROR_SEVERITY_INFORMATIONAL		0x40000000
#define ERROR_SEVERITY_WARNING				0x80000000
#define ERROR_SEVERITY_ERROR				0xC0000000
// end_winnt


/////////////////////////////////////////////////////////////////////////
//
// Standard Success values
//
//
/////////////////////////////////////////////////////////////////////////


//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_RPC_RUNTIME				0x02
#define FACILITY_RPC_STUBS					0x03
#define FACILITY_IO_ERROR_CODE				0x04
#define FACILITY_USB_ERROR_CODE				0x10
#define FACILITY_HID_ERROR_CODE				0x11
#define FACILITY_FIREWIRE_ERROR_CODE		0x12
#define FACILITY_CLUSTER_ERROR_CODE			0x13


//
// Define the severity codes
//
#define STATUS_SEVERITY_SUCCESS				0x0
#define STATUS_SEVERITY_INFORMATIONAL		0x1
#define STATUS_SEVERITY_WARNING				0x2
#define STATUS_SEVERITY_ERROR				0x3

//
// The success status codes 0 - 63 are reserved for wait completion status.
// FacilityCodes 0x5 - 0xF have been allocated by various drivers.
//
#define STATUS_SUCCESS								((NTSTATUS)0x00000000L)		// ntsubauth

//
// MessageId: STATUS_ALERTED
//
// MessageText:
//
//  The success status codes 256, 257, 258, and 258 are reserved for
//  User APC, Kernel APC, Alerted, and Timeout.
//
#define STATUS_ALERTED								((NTSTATUS)0x00000101L)

//
// MessageId: STATUS_TIMEOUT
//
// MessageText:
//
//  The success status codes 256, 257, 258, and 258 are reserved for
//  User APC, Kernel APC, Alerted, and Timeout.
//
#define STATUS_TIMEOUT								((NTSTATUS)0x00000102L)		// winnt

//
// MessageId: STATUS_PENDING
//
// MessageText:
//
//  The operation that was requested is pending completion.
//
#define STATUS_PENDING								((NTSTATUS)0x00000103L)		// winnt



// MessageId: STATUS_UNSUCCESSFUL
//
// MessageText:
//
//  {Operation Failed}
//  The requested operation was unsuccessful.
//
#define STATUS_UNSUCCESSFUL							((NTSTATUS)0xC0000001L)
#define STATUS_FAILURE								((NTSTATUS)0xC0000001L)

//
// MessageId: STATUS_NOT_IMPLEMENTED
//
// MessageText:
//
//  {Not Implemented}
//  The requested operation is not implemented.
//
#define STATUS_NOT_IMPLEMENTED						((NTSTATUS)0xC0000002L)



//
// MessageId: STATUS_INVALID_PARAMETER
//
// MessageText:
//
//  An invalid parameter was passed to a service or function.
//
#define STATUS_INVALID_PARAMETER					((NTSTATUS)0xC000000DL)



//
// MessageId: STATUS_INVALID_DEVICE_REQUEST
//
// MessageText:
//
//  The specified request is not a valid operation for the target device.
//
#define STATUS_INVALID_DEVICE_REQUEST				((NTSTATUS)0xC0000010L)



//
// MessageId: STATUS_BAD_MASTER_BOOT_RECORD
//
// MessageText:
//
//  The type of a token object is inappropriate for its attempted use.
//
#define STATUS_BAD_MASTER_BOOT_RECORD				((NTSTATUS)0xC00000A9L)



//
// MessageId: STATUS_DEVICE_NOT_READY
//
// MessageText:
//
//  {Drive Not Ready}
//  The drive is not ready for use; its door may be open.
//  Please check drive %s and make sure that a disk is inserted
//  and that the drive door is closed.
//
#define STATUS_DEVICE_NOT_READY						((NTSTATUS)0xC00000A3L)



//
// MessageId: STATUS_IO_TIMEOUT
//
// MessageText:
//
//  {Device Timeout}
//  The specified I/O operation on %s was not completed before the time-out
//  period expired.
//
#define STATUS_IO_TIMEOUT							((NTSTATUS)0xC00000B5L)



//
//
// MessageId: STATUS_NOT_SUPPORTED
//
// MessageText:
//
//  The network request is not supported.
//
#define STATUS_NOT_SUPPORTED						((NTSTATUS)0xC00000BBL)



//
// MessageId: STATUS_IMAGE_ALREADY_LOADED
//
// MessageText:
//
//  Indicates that the specified image is already loaded.
//
#define STATUS_IMAGE_ALREADY_LOADED					((NTSTATUS)0xC000010EL)



//
// MessageId: STATUS_BIOS_FAILED_TO_CONNECT_INTERRUPT
//
// MessageText:
//
//  The system bios failed to connect a system interrupt to the device or bus for
//  which the device is connected.
//
#define STATUS_BIOS_FAILED_TO_CONNECT_INTERRUPT		((NTSTATUS)0xC000016EL)



//
// MessageId: STATUS_INVALID_BLOCK_LENGTH
//
// MessageText:
//
//  When accessing a new tape of a multivolume partition, the current
//  blocksize is incorrect.
//
#define STATUS_INVALID_BLOCK_LENGTH					((NTSTATUS)0xC0000173L)



//
// MessageId: STATUS_IO_DEVICE_ERROR
//
// MessageText:
//
//  The I/O device reported an I/O error.
//
#define STATUS_IO_DEVICE_ERROR						((NTSTATUS)0xC0000185L)



#endif //ifndef _LNXDEFS_H_










