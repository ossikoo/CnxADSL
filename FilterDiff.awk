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
##		FilterDiff (Filter CVS Diff file)
##
##	FILE NAME:
##		FilterDiff.awk
##
##	ABSTRACT:
##		This program scans CVS diff file and filters out CVS command lines and then
##		looks for entries that contain entries that are not empty and are not
##		"Only in" entries.
##
##
##	DETAILS:
##		This program is invoked as follows:
##		   gawk -f FilterDiff.awk [Summary=1] [Filter=1] CvsDiff.txt
##		where:
##		gawk is the name of the gawk interpreter
##		FilterDiff.awk is the name of this program
##		Summary=1: Turn on summary output
##		Filter=1:  Turn on Filtered output
##		CvsDiff.txt is the input file
##
##		Requirements:
##		 ignore CVS command lines
##		 ignore CVS Log keyword and all subsequent log entries (starts with > or < 
##			and followed by spaces and *) and all delta indicators (line beginning
##			with line number or "---" separator until a different type line is found.
##		 ignore files that are empty after having deleted CVS lines
##		 ignore "Only in" files
##
################################################################################
## $Archive: /Projects/Tigris and Derivatives Linux/Reference Designs/Tigris/Linux 2.4 ATM Driver/KernelModule/CardALTigris.c $
## $Revision: 6 $
## $Date: 3/01/01 3:46p $
################################################################################
################################################################################



# Variables:
#	Filter:			Command Line parm to enable filtered output
#	Summary:		Command line parm to enable summar info
#	Diff_Cnt:		Count of significant differences found under current command line
#	File_Buffer:	Buffer to hold last command line - not printed until we find
#					out whether we have any signicant differences
#	Diff_Buffer:	Buffer to hold the delta lines
#	File_State:		File line parser state
#	Log_State:		Log line parser state
#	File_Sep:		Constant - print separator between files
#	Entry_Sep:		Constant - print separator between Entries
#	Entry_Indent:	Constant - print indentation on entry lines
#	Usage_Printed:	Notes whether usage statement has been printed yet


################################################################################
#	FUNCTION NAME:
#		Usage
#
#	ABSTRACT:
#		Print usage information
#	RETURN:
#		NA
#
#	DETAILS:
################################################################################
function Usage()
{
	if ( (Filter==0 ) && (Summary==0) && (Usage_Printed==0) )
	{
		# note it is printed so it will only print once
		Usage_Printed = 1

		# print usage
		print "This program is invoked as follows:"
		print "    gawk -f FilterDiff.awk [Summary=1] [Filter=1] CvsDiff.txt"
		print "where:"
		print "    gawk is the name of the gawk interpreter"
		print "    FilterDiff.awk is the name of this program"
		print "    Summary=1: Turn on summary output"
		print "    Filter=1:  Turn on Filtered output"
		print "    CvsDiff.txt is the input file"
	}
}



################################################################################
#	FUNCTION NAME:
#		CheckFlush
#
#	ABSTRACT:
#		Print command line and change lines and flush buffers
#	RETURN:
#		NA
#
#	DETAILS:
#		If any signficant changes have been found, print preceeding command line
#		if it has not already been printed and then print the changes.
#		Both File_Buffer and the Diff_Buffer are then flushed so as not to
#		be reprinted
################################################################################
function CheckFlush()
{
	# have we found significant differences
	if ( Diff_Cnt > 0 )
	{
		# reset count of significant differences
		Diff_Cnt = 0

		# is the command buffer non empty?
		if ( File_Buffer != "" )
		{
			# save it for listing out changed files later
			Diff_List_Buffer = Diff_List_Buffer File_Buffer "\n" 

			# print and flush command buffer
			if ( Filter )
			{
				print File_Sep File_Buffer
			}
			File_Buffer = ""
		}

		# print diff buffer
		if ( Filter )
		{
			printf("%s", Diff_Buffer )
		}
	}
	Diff_Buffer = ""
}



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
#		Set the indention level of non-command lines to make reading easier.
#		This is done here as a variable so there is only one place to change
#		to customize this.
################################################################################
BEGIN \
{
	#set the indention of file sections (all but the first line) to make
	#it easier to visually separate files.
	File_Sep = "\n\n\n"
	Entry_Sep = "\n"
	Entry_Indent = "    "
}



################################################################################
#	FUNCTION NAME:
#		(main)
#
#	ABSTRACT:
#		Process each input line printing File lines and signficant changes only
#	RETURN:
#		NA
#
#	DETAILS:
#		First check for command lines
#			"Only in":
#				print any lines in buffers and note state
#			"diff -wr":
#				print any lines in buffers, save command line and note state
#		Non-File Lines
#			File_State ONLY_IN:
#				ignore all lines
#			File_State "diff -wr":
#				if line is VCS keyword
#					ignore it
#				else is first char of line a digit (This is a "delta" type line)?
#					print any diffence found thus far and save this line
#					(does not count as a signficant difference)
#				else does line starts with "---" (This is a "delta" type line)
#					save this line (does not count as a signficant difference)
#				else Is this line start a Log or History?
#					note we in process of skipping log 
#				else are we in process of skipping log
#				and this line is a delta line?
#					ignore line
#				else nothing special about this line
#					end process of skipping log 
#					if line is non-empty
#						increment significant difference count
#						Save line
################################################################################

# Process each input line
{
	# print usage if needed and not yet done
	Usage()

	# is this line a "Only in" line and thus to be ignored?
	if ( substr($0, 1, 7) == "Only in" )
	{
		# note that this ends log state now
		Log_State = ""

		#print any diffence found thus far
		CheckFlush()

		# print this line
		if ( Filter )
		{
			print File_Sep $0
		}

		# save it for listing out changed files later
		Only_In_List_Buffer = Only_In_List_Buffer $0 "\n"

		# note state for non-command lines
		File_State = "ONLY_IN"
	}

	# is this line a "diff" line and thus to be ignored?
	else if ( substr($0, 1, 8) == "diff -wr" )
	{
		# note that this ends log state now
		Log_State = ""

		#print any diffence found thus far
		CheckFlush()

		# note state for non-command lines
		File_State = "DIFF"

		# save the command line to print if we find significant differences later
		File_Buffer = $0
	}

	# not a command line
	else
	{
		if ( File_State == "ONLY_IN" )
		{
			#NOTHING TO DO - WE IGNORE THIS LINE
		}
		else if ( File_State == "DIFF" )
		{
			# is this line a CVS keyword and therefor to be ignored?
			if ( (match ( $0, /(<|>).*\$(Revision|Date|Archive|Header):.*\$/ )) == 1 )
			{
				#NOTHING TO DO - WE IGNORE THIS LINE
			}

			# is first char of line a digit (This is a "delta" type line)?
			else if ( ( substr( $0, 1, 1) >= "0" ) && ( substr( $0, 1, 1) <= "9" ) )
			{
				#print any diffence found thus far
				CheckFlush()

				# save it but it does not count as a difference
				Diff_Buffer = Diff_Buffer Entry_Sep Entry_Indent $0 "\n"
			}

			# does line starts with "---" (This is a "delta" type line)
			else if ( substr( $0, 1, 3 ) == "---" )
			{
				# save it but it does not count as a difference
				Diff_Buffer = Diff_Buffer Entry_Indent $0 "\n"
			}

			# Is this line start a Log or History?
			else if ( (match ( $0, /(<|>).*\$(Log|History):.*\$/ )) == 1 )
			{
				#print any diffence found thus far
				CheckFlush()

				# note we are now in Log state and will ignore to end of log
				Log_State = "LOG"
				#NOTHING TO DO - WE IGNORE THIS LINE
			}

			# are we in process of skipping log?
			# and this line is a delta line
			else if (( Log_State == "LOG" ) \
			&& ( match($0, /(<|>)/ ) == 1 ))
			{
				#NOTHING TO DO - WE IGNORE THIS LINE
			}

			# nothing special about this line
			else
			{
				# note that this ends log state now
				Log_State = ""

				# if line is non-empty
				if ( $0 != "" )
				{
					# save line
					Diff_Buffer = Diff_Buffer Entry_Indent $0 "\n"

					# increment difference count
					Diff_Cnt ++
				}
			}
		}
	}
}


################################################################################
#	FUNCTION NAME:
#		END
#
#	ABSTRACT:
#		Epilog - Print any lines held in buffer
#	RETURN:
#		NA
#
#	DETAILS:
################################################################################
# Epilog
END \
{
	#print any diffence found thus far
	CheckFlush()

	if ( Summary )
	{	
		print "\n\n\n-----SUMMARY--------"
		printf ("%s", Only_In_List_Buffer )
		printf ("%s", Diff_List_Buffer )
	}
}
