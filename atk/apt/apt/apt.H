ATK_INTER
#ifndef _apt_H_
#define _apt_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Apt Data-object

MODULE	apt.H

VERSION	0.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Apt Data-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  02/23/88	Created (TCP)
  05/23/89	Add apt_LeftArea, etc (TCP)

END-SPECIFICATION  ************************************************************/

#define  apt_VERSION    1

/*  Placements  */
#define  apt_Left		      (1<<0)
#define  apt_Center		      (1<<1)
#define  apt_Right		      (1<<2)
#define  apt_Top		      (1<<3)
#define  apt_Middle		      (1<<4)
#define  apt_Bottom		      (1<<5)

/*  Enclosure Areas  */
#define  apt_LeftArea		      (0)
#define  apt_TopArea		      (1)
#define  apt_RightArea		      (2)
#define  apt_BottomArea		      (3)

/*  Title/Legend Formatting Options  */
#define  apt_Spread		      (1<<0)
#define  apt_LeftRight		      (1<<1)

struct apt_strings
  {
  long				      strings_count;
  char					**strings;
  char				     *font_name;
  long				      mode;
  };

struct apt_field
  {
  const char			     *name;
  const char			     *content;
  };

struct apt_fields
  {
  long				      count;
  struct apt_field		      field[1];
  };

struct apt_field_contents
  {
  long				      count;
  struct apt_field		      field[1];
  };

  class apt;
  typedef void (*apt_readfptr)(class apt *self);
  typedef void (*apt_writefptr)(class apt *self);
#include <dataobject.H>
class apt : public  dataobject
  {
public:
	virtual ATKregistryEntry *ATKregistry();






  virtual void SetAreaTitle( const char *title, long area );
  virtual void SetAreaSpreadTitle( const char **title, long count, long area, long mode );
  virtual void SetAreaTitleFontName( const char *font, long area );
  virtual void SetAreaLegend( const char *legend, long area  );
  virtual void SetAreaSpreadLegend( const char **legend, long count, long area, long mode );
  virtual void SetAreaLegendFontName( const char *font, long area );
  virtual long ReadObject( FILE *file, long id, apt_readfptr reader )     ;
  virtual struct apt_field * ReadObjectField()				 ;
  virtual char ** ParseFieldContent( const char *string )		 ;
  virtual struct apt_field_contents * ParseFieldContents( const char *string )		 ;
  virtual void WriteObject( FILE *file, long id, long level, apt_writefptr writer );
  virtual void WriteObjectField(struct apt_field *field);



  inline char ** AreaTitlesAnchor( int area)	    { return ((this)->titles[area].strings); }
  inline long AreaTitlesCount( int area)	    { return ((this)->titles[area].strings_count); }
  inline const char * AreaTitle( int area)		    { return (((this)->titles[area].strings) ?
					 ((this)->titles[area].strings)[0] : NULL); }
  inline const char * AreaTitles( int area, int i)		    { return (((this)->titles[area].strings)[i]); }
  inline const char * AreaTitleFontName( int area)	    { return ((this)->titles[area].font_name); }
  inline char ** AreaLegendsAnchor( int area)	    { return ((this)->legends[area].strings); }
  inline long AreaLegendsCount( int area)	    { return ((this)->legends[area].strings_count); }
  inline const char * AreaLegend( int area)		    { return (((this)->legends[area].strings) ?
					 ((this)->legends[area].strings)[0] : NULL); }
  inline const char * AreaLegends( int area, int i)	    { return (((this)->legends[area].strings)[i]); }
  inline const char * AreaLegendFontName( int area)	    { return ((this)->legends[area].font_name); }



  
	apt();
  
	virtual ~apt();
  
	static boolean InitializeClass();




  struct apt_strings		      titles[4];
  struct apt_strings		      legends[4];
  long				      id;
  FILE				     *field_file;
  long				      field_index;
  struct apt_fields		     *fields;
  };


/*
    $Log: apt.H,v $
// Revision 1.6  1995/11/07  20:17:10  robr
// OS/2 port
//
// Revision 1.5  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.4  1993/06/10  20:20:51  rr2b
// Great atk_inited removal.
//
 * Revision 1.3  1993/05/18  15:20:28  rr2b
 *  Corrected for the proper type of the strings member of the apt_strings structure.
 * (It was a *(*)[], but it is really just a **.)
 *
 * Revision 1.2  1993/05/13  14:11:38  rr2b
 *  Fixed char *((*)[]) types to be char ***.
 * Added typedefs for functions in ReadObject and WriteObject.
 * Fixed MISSING_ARGUMENT_TYPEs.
 * Removed \'s to satisfy cfront...
 *
*Revision 1.6  1992/12/14  23:20:33  rr2b
*add $Logs back after disclaimerization took them out
*
Revision 1.4  1991/09/12  19:19:54  bobg
Update copyright notice

Revision 1.3  1990/04/11  14:11:06  gk5g
Removed initialization of variable debug from apt.h and put it in apt.c:apt__InitializeClass.  Create apt__InitializeClass.

Revision 1.2  89/05/24  19:44:52  tom
Add apt_LeftArea, etc.

Revision 1.1  89/04/28  17:45:45  tom
Initial revision

*/
 
  
#define  ASSERT( c, m )	    if ( !(c) )  printf( "Assert Failure: %s\n", Stringize(m) );
  
#ifndef DEBUG
/* char    debug; */
#ifdef  DB
#define  IN( x )	    if ( debug ) printf( ">" #x "\n" );
#define  OUT( x )	    if ( debug ) printf( "<" #x "\n" );
#define  DEBUG( y )	    if ( debug ) printf( " %s\n", #y );
#define  DEBUGc( y )	    if ( debug ) printf( " '%c'\n", #y );
#define  DEBUGct( t, y )    if ( debug ) printf( " "#t": '%c'\n", y );
#define  DEBUGs( y )	    if ( debug ) printf( " '%s'\n", #y );
#define  DEBUGst( t, y )    if ( debug ) printf( " "#t": '%s'\n", y );
#define  DEBUGd( y )	    if ( debug ) printf( " %d\n", y );
#define  DEBUGdt( t, y )    if ( debug ) printf( " "#t": %d\n", y );
#define  DEBUGlt( t, y )    if ( debug ) printf( " "#t": %D\n", y );
#define  DEBUGgt( t, y )    if ( debug ) printf( " "#t": %g\n", y );
#define  DEBUGx( y )	    if ( debug ) printf( " %x\n", y );
#define  DEBUGxt( t, y )    if ( debug ) printf( " "#t": %x\n", y );
#else
#define  IN( x )
#define  OUT( x )
#define  DEBUG( y )
#define  DEBUGc( y )
#define  DEBUGct( t, y )
#define  DEBUGs( y )
#define  DEBUGst( t, y )
#define  DEBUGd( y ) 
#define  DEBUGdt( t, y )
#define  DEBUGlt( t, y )
#define  DEBUGgt( t, y )
#define  DEBUGx( y ) 
#define  DEBUGxt( t, y )
#endif  /* DB */
#endif  /* DEBUG */
  
#define  ok	0
#define  failure 1
  
#define  yes	1
#define  no	0
#define  true	1
#define  false	0
#define  on	1
#define  off	0


/* tjm - FIXME: only define these in C files that actually use the timers */
static UNUSED struct timeval	  tod;
static UNUSED struct timezone	  tzone;
static UNUSED float	    	  elapsed_time;

#define  apt_StartTimer	  {gettimeofday( &tod, &tzone );\
			  elapsed_time = (tod.tv_sec * 1000) + (tod.tv_usec / 1000);}
#define  apt_PrintTimer(x)	  {gettimeofday( &tod, &tzone );\
			  elapsed_time = ((tod.tv_sec * 1000) + (tod.tv_usec / 1000)) - elapsed_time;\
			  printf( "%s -- %0.2g Seconds\n", "x", elapsed_time/1000 );}
  
#endif /* _apt_H_ */
