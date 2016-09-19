ATK_INTER
#ifndef _apts_H_
#define _apts_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Apt Tool Set

MODULE	apts.ch

VERSION	0.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Procedures that support the Apt Tool Set.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  05/18/89	Created (TCP)

END-SPECIFICATION  ************************************************************/

#define  apts_VERSION    1

#include <ATK.H>
class apts : public ATK 
  {
public:
	virtual ATKregistryEntry *ATKregistry();




  static long CompareStrings( const char *s1, const char *s2 )			 ;
  static long SubstringIndex( const char *pattern, const char *string )		 ;
  static char * StripString( char *string)				 ;
  static long CaptureString( const char *source, char **target )		 ;

  static void HourMinuteSecond( long *hour, long *minute, long *second );
  static void HourOfDay( char *hour );
  static void MinuteOfHour( char *minute );
  static void SecondOfMinute( char *second );

  static long YearMonthDay( long *year, long *month, long *day )	 ;

  static long DaysInMonth( long year, long month )			 ;
  static long WeekDayOffset( long year, long month, long day )	 ;

  };


/*
    $Log: apts.H,v $
// Revision 1.5  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.4  1993/06/10  20:20:57  rr2b
// Great atk_inited removal.
//
 * Revision 1.3  1993/05/18  15:21:34  rr2b
 * No changes
 *
 * Revision 1.2  1993/05/13  14:15:35  rr2b
 * Fixed second argument to CaptureString to be a char **.
 *
*Revision 1.5  1992/12/14  23:20:33  rr2b
*add $Logs back after disclaimerization took them out
*
Revision 1.3  1991/09/12  19:20:03  bobg
Update copyright notice

Revision 1.2  1989/05/18  22:45:00  tom
Add more misc tool items.

Revision 1.1  89/05/18  20:31:13  tom
Initial revision

*/

#endif /* _apts_H_ */
