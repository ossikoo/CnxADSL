################################################################################
################################################################################
####	Copyright (c) 2001
####	Conexant Systems Inc. (formerly Rockwell Semiconductor Systems)
####	Personal Computing Division
####	All Rights Reserved
####
####	CONFIDENTIAL and PROPRIETARY --
####		No Dissemination or use without prior written permission.
####
###############################################################################
##
##	MODULE NAME:
##		FilterLog (Filter Log file)
##
##	FILE NAME:
##		FilterLog.awk
##
##	ABSTRACT:
##		This program scans the log file and outputs a file containing just the
##		Linux entries.
##
##
##	DETAILS:
##		This program is invoked as follows:
##		   gawk -f FilterLog.awk log.txt
##		where:
##		   gawk is the name of the gawk interpreter
##		   FilterLog.awk is the name of this program
##		   log.txt is the input file
##
##		Requirements:
##			Detect begining of entry and save until "SYSTEM:" line is found
##			Detect "SYSTEM:" line and determine if Linux is on this line
##			print saved entry lines (if Linux) and this line and all until
##				beginning of next entry
##
################################################################################
## $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux 2.4 ATM Driver/KernelModule/CardALTigris.c $
## $Revision: 6 $
## $Date: 3/01/01 3:46p $
################################################################################
################################################################################



# Variables:
#	State:	Keeps track of what we are looking for
#	Buffer:	Store first lines of entry until we know if this is one to print


################################################################################
#	FUNCTION NAME:
#		BEGIN
#
#	ABSTRACT:
#		Initialization
#	RETURN:
#		NA
#
#	DETAILS:
################################################################################
BEGIN \
{
	# searches (e.g. match) should be case insensitive.
	# special reserved variable for gawk
	IGNORECASE = 1

	#set initial state
	State = "LOOK_FOR_ENTRY"
}


################################################################################
#	FUNCTION NAME:
#		(main)
#
#	ABSTRACT:
#		Process each input line printing linux entries only
#	RETURN:
#		NA
#
#	DETAILS:
################################################################################

# Process each input line
{	  
	#LOOK_FOR_ENTRY
	if ( State == "LOOK_FOR_ENTRY" )
	{
		# is line asterisks?
		if ( substr($0, 1, 10) == "**********" )
		{
			# now look for system line
			State = "LOOK_FOR_SYSTEM"

			# save this line
			Buffer = $0
		}
	}

	# LOOK_FOR_SYSTEM
	else if ( State == "LOOK_FOR_SYSTEM" )
	{
		# save this line
		Buffer = Buffer "\n" $0

		# does line start with "SYSTEM:" ?
		if ( substr($0, 1, 7) == "SYSTEM:" )
		{
			# Does line contain "Linux"
			if ( match( $0, /(Linux)|([ \t]ALL)/) != 0 )
			{
				# print all lines in this entry to this point
				print "\n" Buffer
				Buffer = ""

				# now look for next entry and print all lines til found
				State = "LOOK_FOR_TERM_PRINTING"
			}
			else
			{
				# now look for next entry without printing
				State = "LOOK_FOR_TERM_SKIPPING"
			}
		}
	}

	# LOOK_FOR_TERM_PRINTING
	else if ( State == "LOOK_FOR_TERM_PRINTING" )
	{
		# print this line
		print $0

		# is line asterisks?
		if ( substr($0, 1, 10) == "**********" )
		{
			# now look for next entry
			State = "LOOK_FOR_ENTRY"
		}
	}

	# LOOK_FOR_TERM_SKIPPING
	else if ( State == "LOOK_FOR_TERM_SKIPPING" )
	{
		# is line asterisks?
		if ( substr($0, 1, 10) == "**********" )
		{
			# now look for next entry
			State = "LOOK_FOR_ENTRY"
		}
	}
}


################################################################################
#	FUNCTION NAME:
#		END
#
#	ABSTRACT:
#		Epilog
#	RETURN:
#		NA
#
#	DETAILS:
################################################################################
# Epilog
END \
{
}
