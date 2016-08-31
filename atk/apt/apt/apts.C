/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/*
    $Log: apts.C,v $
 * Revision 1.6  1995/11/07  20:17:10  robr
 * OS/2 port
 *
 * Revision 1.5  1994/11/30  20:42:06  rr2b
 * Start of Imakefile cleanup and pragma implementation/interface hack for g++
 *
 * Revision 1.4  1993/06/13  12:18:15  rr2b
 * Fixed to use time_t in place of long for the return parameter
 * from time().
 *
 * Revision 1.3  1993/05/18  15:21:20  rr2b
 * No changes.
 *
 * Revision 1.2  1993/05/13  14:15:00  rr2b
 * Removed #include "class.h"
 * Removed local declaration of the function localtime.
 *
 * Revision 1.5  1992/12/15  21:25:24  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.4  1992/12/14  20:33:21  rr2b
 * disclaimerization
 *
 * Revision 1.3  1991/09/12  15:56:25  bobg
 * Update copyright notice and rcsid
 *
 * Revision 1.2  1989/05/18  22:44:39  tom
 * Add more misc tool items.
 *
 * Revision 1.1  89/05/18  20:31:24  tom
 * Initial revision
 * 
*/

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Apt Tool Set

MODULE	apts.c

VERSION	0.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Procedure that support the Apt Tool Set

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

#include <andrewos.h>
ATK_IMPL("apts.H")
#include  "apt.H"
#include  "apts.H"
#include  <ctype.h>


/******************************************************************************\
*                                                                              *
*                           String Facilities                                  *
*                                                                              *
\******************************************************************************/


ATKdefineRegistry(apts, ATK, NULL);


long
apts::CompareStrings( const char			  *s1 , const char			  *s2 )
      {
  long			  result = 0;
  unsigned char	  c1, c2;

  IN(apts_CompareStrings);
  do
    {
    c1 = isupper( *s1 ) ? tolower( *s1++ ) : *s1++;
    c2 = isupper( *s2 ) ? tolower( *s2++ ) : *s2++;
    if ( c1 != c2 )
      break;
    } while ( c1 );
  if ( c1 != c2 )
    if ( c1 > c2 )
      result = 1;
      else result = -1;
  OUT(apts_CompareStrings);
  return  result;
  }

long
apts::SubstringIndex( const char		  	  *pattern , const char		  	  *string )
      {
  long			  i, position = -1;
  const char			 *origin = string;

  IN(apts_SubstringIndex);
  while ( *string  &&  position == -1 )
    {
    if ( tolower(*pattern) == tolower(*string) )
      {
      i = 0;
      while ( *(pattern + i)  &&  tolower(*(pattern + i)) == tolower(*(string + i)) )
        i++;
      if ( *(pattern + i) == 0 )
        position = string - origin;
      }
    string++;
    }
  OUT(apts_SubstringIndex);
  return  position;
  }

char *
apts::StripString( char			  *string )
      {
  char			 *source = string,
				 *cursor = string;

  IN(apts_StripString);
  if ( string  &&  *string )
    {
    while ( *source == ' '  ||  *source == '\t'  ||  *source == '\n' )  source++;
    if ( source != cursor )
      { while ( *cursor++ = *source++ ) ; cursor--; }
      else
      cursor = string + strlen( string );
    while ( *(--cursor) == ' '  ||  *cursor == '\t'  ||  *cursor == '\n')  *cursor = 0;
    }
  OUT(apts_StripString);
  return  string;
  }

long
apts::CaptureString( const char			  *source , char			  **target )
      {
  char			 *cursor;
  long			  status = ok;

  IN(apts_CaptureString);
  if ( source  &&  target )
    {
    if ( *target )  free( *target );
    *target = NULL;
    if ( *source )
      {
      if ( *target = cursor = (char *) malloc( strlen( source ) + 1 ) )
	{
	while ( *source == ' '  ||  *source == '\t'  ||  *source == '\n' )  source++;
	while ( *cursor++ = *source++ ) ;
	cursor--;
	while ( *(--cursor) == ' '  ||  *cursor == '\t'  ||  *cursor == '\n')  *cursor = 0;
	}
	else  status = failure;
      }
    }
  OUT(apts_CaptureString);
  return  status;
  }

/******************************************************************************\
*                                                                              *
*                             Time Facilities                                  *
*                                                                              *
\******************************************************************************/

void
apts::HourMinuteSecond( long		          *hour , long		          *minute , long		          *second )
      {
  struct tm			 *time_units;
  time_t			  clock;   

  IN(apts::HourMinuteSecond);
  time( &clock );
  time_units = localtime( &clock );
  *hour = time_units->tm_hour;
  *minute = time_units->tm_min;
  *second = time_units->tm_sec;
  OUT(apts::HourMinuteSecond);
  }

void
apts::HourOfDay( char		   	  *hour )
      {
  long				  hours, minutes, seconds;

  IN(apts_HourOfDay);
  apts::HourMinuteSecond( &hours, &minutes, &seconds );
  if ( hours == 0 )  hours = 12;
  sprintf( hour, "%02ld", hours );
  OUT(apts_HourOfDay);
  }

void
apts::MinuteOfHour( char			  *minute )
      {
  long				  hours, minutes, seconds;

  IN(apts_MinuteOfHour);
  apts::HourMinuteSecond( &hours, &minutes, &seconds );
  sprintf( minute, "%02ld", minutes );
  OUT(apts_MinuteOfHour);
  }

void
apts::SecondOfMinute( char			  *second )
      {
  long				  hours, minutes, seconds;

  IN(apts_SecondOfMinute);
  apts::HourMinuteSecond( &hours, &minutes, &seconds );
  sprintf( second, "%02ld", seconds );
  OUT(apts_SecondOfMinute);
  }

/******************************************************************************\
*                                                                              *
*                             Date Facilities                                  *
*                                                                              *
\******************************************************************************/

static long			  days_per_month[2][13] =
    {
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };

long   /* Returns 1 if Leap-year, 0 otherwise */
apts::YearMonthDay( long			  *year , long			  *month , long			  *day )
      {
  long			  leap; 
  struct tm			 *time_units;
  time_t			  clock;

  IN(apts_YearMonthDay);
  time( &clock );
  time_units = localtime( &clock );
  *year = time_units->tm_year + 1900;
  *month = time_units->tm_mon + 1;
  *day = time_units->tm_mday;
  if ( *year % 4 == 0  &&  *year % 100 != 0  ||  *year % 400 == 0 )
    leap = 1;
    else leap = 0;
  OUT(apts_YearMonthDay);
  return  leap;
  }

long
apts::DaysInMonth( long			   year , long			   month )
      {
  long			  leap;

  IN(apts_DaysInMonth);
  if ( year % 4 == 0  &&  year % 100 != 0  ||  year % 400 == 0 )
    leap = 1;
    else leap = 0;
  OUT(apts_DaysInMonth);
  return  days_per_month[leap][month];
  }

long   /* Return 0 thru 6: 0=Sunday, 1=Monday, ..., 6=Saturday */
apts::WeekDayOffset( long			   year , long			   month , long			   day )
      {
  long			  i, leap, years, offset = day;

  IN(apts_WeekDayOffset);
  leap = year % 4 == 0  &&  year % 100 != 0  ||  year % 400 == 0;
  offset++; /*===hack*/
  for ( i = 1; i < month; i++ )
    offset += days_per_month[leap][i];
  for ( years = 1980; years < year; years++ )
    offset += 365 + (years % 4 == 0  &&  years % 100 != 0  ||  years % 400 == 0);
  offset = offset % 7;
  OUT(apts_WeekDayOffset);
  return  offset;
  }
