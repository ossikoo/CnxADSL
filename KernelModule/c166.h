/*
 *	@(#)c166.h	1.6	96/10/03
 */

/**************************************************************************
**                                                                        *
**  FILE        :  c166.h                                                 *
**                                                                        *
**  DESCRIPTION :  Include file with macros to enable or disable the      *
**                 usage of Tasking C-166 language extensions.            *
**                 c166 has the following predefined preprocessor         *
**                 symbols:                                               *
**                           _C166     value = 10   (Version 1.0)         *
**                           _MODEL    memory model used:                 *
**                                     tiny:        value is 't'          *
**                                     small:       value is 's'          *
**                                     medium:      value is 'm'          *
**                                     large:       value is 'l'          *
**                                                                        *
**  COPYRIGHT   :  1993 Tasking Software B.V., Amersfoort                 *
**                                                                        *
**************************************************************************/
#ifndef C166_INC


/* 
 * When not using the Tasking C-166 compiler:
 *	- remove the memory type specifiers
 *	-  redefine the 'bit', 'sfrbit', 'sfr' and 'bitword' type
 *	-  supply function prototypes for built-in functions: you should make
 *	   alternatives for these functions to create a runnable program.
 */
typedef	unsigned char	bit;
typedef	unsigned char	sfrbit;
typedef	unsigned int	sfr;
typedef	unsigned int	bitword;

/* NOTE : there may be compilers which recognise next keywords	*/
#define	near
#define far
#define	huge


#define MEDIUM	// Use Medium memory model

/* KEYWORDS */
#define iram

#define interrupt(x)
#define using(x)

#define	C166_INC
#endif
