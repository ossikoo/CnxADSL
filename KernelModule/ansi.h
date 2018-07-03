/****************************************************************************
*  $Workfile: ansi.h $
*
*  $Header: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/ansi.h 2     6/26/01 7:37p Lauds $
*****************************************************************************
*  Copyright 1996 PairGain Technologies as unpublished work                 *
*  All Rights Reserved                                                      *
*                                                                           *
*  The information contained herein is the confidential property            *
*  of PairGain Technologies.  The use, copying, transfer or                 *
*  disclosure of such information is prohibited except by express           *
*  written agreement with PairGain Technologies.                            *
*                                                                           *
*                                                                           *
*  Original Written By:                                                     *
*  Original Date:                                                           *
*                                                                           *
*****************************************************************************
   Description: This file includes all the defines for the ANSI X3.64 escape
                sequence.
****************************************************************************/
#ifndef ANSI_H
#define ANSI_H

/****************************************************************************
                           DEFINE
****************************************************************************/
#define ESCSTR "\x1B"

#define CBT(X)                  CSI X "Z"           /* Cursor Backward Tab  */
#define CHA(X)                  CSI X "G"           /* Cursor Hori. Abs.    */
#define CHT(X)                  CSI X "l"           /* Cursor Hori. Tab.    */
#define CNL(X)                  CSI X "E"           /* Cursor Next Line     */
#define CPL(X)                  CSI X "F"           /* Cursor Preceding Line*/
#define CPR(X,Y)                CSI X ";" Y "R"     /* Cursor Pos. Report   */
#define CSI                     ESCSTR"["           /* Control Sequence Int.*/

                                                    /* Cursor Tab Ctrl      */
#define CTC_SET_HT              CSI"0W"             /* Set HT at active pos.*/
#define CTC_SET_VT              CSI"1W"             /* Set VT at active lin.*/
#define CTC_CLR_HT              CSI"2W"             /* Clr HT at active pos.*/
#define CTC_CLR_VT              CSI"3W"             /* Clr VT at active lin.*/
#define CTC_CLR_ALL_HT1         CSI"4W"             /* Clr all HT at active */
#define CTC_CLR_ALL_HT2         CSI"5W"             /* Clr all HT in device */
#define CTC_CLR_ALL_VT          CSI"6W"             /* Clr all VT in device */

#define CUB(X)                  CSI X "D"           /* Cursor Backward      */
#define CUD(X)                  CSI X "B"           /* Cursor Down          */
#define CUF(X)                  CSI X "C"           /* Cursor Forward       */
#define CUP(X,Y)                CSI Y ";" X "H"     /* Cursor Position      */
#define CUU(X)                  CSI X "A"           /* Cursor Up            */
#define CVT(X)                  CSI X "Y"           /* Cursor Vertical Tab  */
#define DA(X)                   CSI X "c"           /* Device Attributes    */

                                                    /* Define Area Qualif.  */
#define DAQ_ALL_IP              CSI"0o"             /* Accept all Input     */
#define DAQ_NO_IP_NO_TXMIT      CSI"1o"             /* No Input or transmit */
#define DAQ_GRAPHICS            CSI"2o"             /* Accept Graphics      */
#define DAQ_NUMERICS            CSI"3o"             /* Accept Numerics      */
#define DAQ_ALPH                CSI"4o"             /* Accept Alphabetics   */
#define DAQ_R_JUSTIFY           CSI"5o"             /* Right Justify in Area*/
#define DAQ_ZERO_FILL           CSI"6o"             /* Zero-fill in area    */
#define DAQ_HT                  CSI"7o"             /* HT at start of area  */
#define DAQ_NO_IP_Y_TXMIT       CSI"8o"             /* No input;permit txmit*/
#define DAQ_SPACE_FILL          CSI"6o"             /* Space-fill area      */

#define DCH(X)                  CSI X "P"           /* Delete Character     */
#define DL(X)                   CSI X "M"           /* Delete Line          */

                                                    /* Device Status Report */
#define DSR_READY               CSI"0n"             /* Device Ready         */
#define DSR_BUSY_NO_NOTIFY      CSI"1n"             /* Device Busy,no notify*/
#define DSR_BYSY_W_NOTIFY       CSI"2n"             /* Device Busy, w\notify*/
#define DSR_MALFUNC_NO_NOTIFY   CSI"3n"             /* Malfunction no notify*/
#define DSR_MALFUNC_W_NOTIFY    CSI"4n"             /* Malfunction w\notify */
#define DSR_VIA_DSR_OR_DSC      CSI"5n"             /* Report status via DSR*/
#define DSR_VIA_CPR             CSI"6n"             /* Report Status via CPR*/

                                                    /* Erase in Area        */
#define EA_ACTIVE_TO_END        CSI"0O"             /* From act. pos. to end*/
#define EA_START_TO_ACTIVE      CSI"1O"             /* From start to act.pos*/
#define EA_ALL                  CSI"2O"             /* all                  */

#define ECH(X)                  CSI X "X"           /* Erase Character      */

                                                    /* Erase in Display     */
#define ED_ACTIVE_TO_END        CSI"0J"             /* From act. pos. to end*/
#define ED_START_TO_ACTIVE      CSI"1J"             /* From start to act.pos*/
#define ED_ALL                  CSI"2J"             /* all                  */

                                                    /* Erase in field       */
#define EF_ACTIVE_TO_END        CSI"0N"             /* From act. pos. to end*/
#define EF_START_TO_ACTIVE      CSI"1N"             /* From start to act.pos*/
#define EF_ALL                  CSI"2N"             /* all                  */

                                                    /* Erase in Line        */
#define EL_ACTIVE_TO_END        CSI"0K"             /* From act. pos. to end*/
#define EL_START_TO_ACTIVE      CSI"1K"             /* From start to act.pos*/
#define EL_ALL                  CSI"2K"             /* all                  */

#define EPA                     ESCSTR"W"           /* End of Protected area*/
#define ESA                     ESCSTR"G"           /* End of Selected Area */
#define FNT_SEL(X,Y)            CSI X ";" Y " D"    /* Font Selection       */
#define GSM(X,Y)                CSI X ";" Y " B"    /* Graphic Size Modify  */
#define GSS(X)                  CSI X " C"          /* Graphic Size Sel.    */
#define HPA(X)                  CSI X "\'"          /* Hori. Pos. Absolute  */
#define HPR(X)                  CSI X "a"           /* Hori. Pos. Relative  */
#define HTJ                     ESCHAR"\|"          /* Hori. Tab w\Justify  */
#define HTS                     ESCHAR"H"           /* Hori. Tab set        */
#define HVP(X,Y)                CSI X ";" Y "f"     /* Hori. & Verti. Pos.  */
#define ICH(X)                  CSI X "\@"          /* Insert Character     */
#define IL(X)                   CSI X "L"           /* Insert Line          */
#define IND                     ESCSTR"D"           /* Index                */
#define JFY(STRING)             CSI STRING " F"     /* Justify              */
#define MW                      ESCSTR"U"           /* Message Waiting      */
#define NEL                     ESCSTR"E"           /* Next Line            */
#define NP(X)                   CSI X "U"           /* Next Page            */
#define PLD                     ESCSTR"K"           /* Partial Line Down    */
#define PLU                     ESCSTR"L"           /* Partial Line Up      */
#define PP(X)                   CSI X "V"           /* Preceding Page       */
#define REP(X)                  CSI X "b"           /* Repeat Char. or Ctrl */
#define RI                      ESCSTR"M"           /* Reverse Index        */
#define RM                      CSI"/|"             /* Reset Mode           */
#define SD(X)                   CSI X "T"           /* Scroll Down          */

                                                    /* Select Extent Mode   */
#define SEM_DISPLAY             CSI"0Q"             /* Edit in display      */
#define SEM_ACTIVE_LINE         CSI"1Q"             /* Edit in active line  */
#define SEM_FIELD               CSI"2Q"             /* Edit in field        */
#define SEM_QUALIFIED_AREA      CSI"3Q"             /* Edit in Quail. area  */

                                                    /* Select Graphic Rend. */
#define SGR_PRIMARY             CSI"0m"             /* Primary rendition    */
#define SGR_BOLD                CSI"1m"             /* Bold | inc. intensity*/
#define SGR_DEC_INTENSITY       CSI"2m"             /* Faint |dec. intensity*/
#define SGR_INTALIC             CSI"3m"             /* Italic               */
#define SGR_UNDERSCORE          CSI"4m"             /* Underscore           */
#define SGR_SLOW_BLINK          CSI"5m"             /* Slow blink (<150/min)*/
#define SGR_RAPID_BLINK         CSI"6m"             /* Rapic blink          */
#define SGR_INV_NEG_IMAGE       CSI"7m"             /* Inverse neg. image   */

#define SL                      CSI X " \@"         /* Scroll Left          */
#define SM                      CSI"h"              /* Select Mode          */
#define SPA                     ESCSTR"V"           /* Start of Prot. area  */
#define SPI(X,Y)                CSI X ";" Y " G"    /* Spacing Increment    */
#define SR(X)                   CSI X " A"          /* Scroll Right         */
#define SSA                     ESCSTR"F"           /* Start of Sel. area   */
#define SU(X)                   CSI X "S"           /* Scroll Up            */

                                                    /* Tab Clear            */
#define TBC_CLR_HT              CSI"0g"             /* Clr HT at active pos.*/
#define TBC_CLR_VT              CSI"2g"             /* Clr VT at active lin.*/
#define TBC_CLR_HT_TO_ACTIVE    CSI"3g"             /* Clr all HT at act.pos*/
#define TBC_CLR_ALL_HT          CSI"4g"             /* Clr all HT stops     */
#define TBC_CLR_ALL_VT          CSI"5g"             /* Clr all VT stops     */

#define TSS                     CSI" E"             /* Thin Space Specific. */
#define VPA(X)                  CSI X "d"           /* Vertical Pos. Abs.   */
#define VPR(X)                  CSI X "e"           /* Vertical Pos. Rel.   */

#endif
/***************************************************************************
 * $Log: /Projects/Tigris and Derivatives/Reference Designs/Euphrates/DMT DP Controller/ansi.h $
 * 
 * 2     6/26/01 7:37p Lauds
 * multi-instance version integration
 * 
 * 1     4/14/99 11:09a Lewisrc
 * Added as new file from Pairgain 03/22/99 release for dumb terminal
 * 
 *    Rev 1.3   27 Oct 1997 15:52:32   phamt
 * Port to DSLAM design
 * 
 *    Rev 1.2   30 Jan 1997 16:22:08   TSE
 * change ESCCHAR to ESCSTR
 *
 *    Rev 1.1   15 Nov 1996 18:18:52   TSE
 * No change.
 *
 *    Rev 1.0   14 Oct 1996 15:27:52   TSE
 * Initial revision.
****************************************************************************/
/**********                end of file $Workfile: ansi.h $          *****************/

