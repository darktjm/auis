/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/* zipstat.c	Zip Status-object				      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/*
    $Log: zipstatus.C,v $
 * Revision 1.3  1993/06/17  04:28:00  rr2b
 * made apt_MM_compare a macro which uses ULstrcmp
 * to avoid multiple definitions of apt_MM_compare.
 *
 * Revision 1.2  1993/06/09  17:45:37  gk5g
 * functional C++ version
 *
 * Revision 1.3  1992/12/15  21:58:46  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.2  1992/12/15  04:00:36  rr2b
 * disclaimerization
 *
 * Revision 1.1  1992/10/06  18:33:00  susan
 * Initial revision
 *
 * Revision 2.6  1991/09/12  16:44:17  bobg
 * Update copyright notice and rcsid
 *
 * Revision 2.5  1989/02/17  18:09:03  ghoti
 * ifdef/endif,etc. label fixing - courtesy of Ness
 *
 * Revision 2.4  89/02/08  16:52:07  ghoti
 * change copyright notice
 * 
 * Revision 2.3  89/02/07  20:29:31  ghoti
 * first pass porting changes: filenames and references to them
 * 
 * Revision 2.2  88/10/11  20:42:05  tom
 * Add return statemetns.
 * 
 * Revision 2.1  88/09/27  18:19:13  ghoti
 * adjusting rcs #
 * 
 * Revision 1.2  88/09/15  17:48:13  ghoti
 * copyright fix
 * 
 * Revision 1.1  88/09/14  17:47:15  tom
 * Initial revision
 * 
*/

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip Status-object

MODULE	zipstat.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support Status Messages.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  04/15/88	Created (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include <errno.h>
#include <string.h>
#include "zipefn00.h"
#include "zip.H"
#include "zipview.H"
#include "zipstatus.H"

static const char		 * const zip_status_messages[] =
					  {
				/* 000 */ "Success",
					  "Failure",
					  "X",
					  "Y",
					  "Z",

				/* 005 */ "Insufficient Space",
					  "Insufficient Stream Space",
					  "Insufficient Image Space",
					  "Insufficient Figure Space",
					  "Insufficient Pane Space",

				/* 010 */ "Insufficient X Space",
					  "Pane Non-Existent",
					  "Stream Non-Existent",
					  "Pane Not Exposed",
					  "Pane Not Hidden",

				/* 015 */ "Stream Syntax Error",
					  "Stream Positioning Error",
					  "Inappropriate Stream Attribute",
					  "Unrecognized Stream Object Attribute",
					  "Unrecognized Stream Figure Type",

				/* 020 */ "Image Non-Existent",
					  "Figure Non-Existent",
					  "Redraw Procedure Non-Existent",
					  "Quit Requested",
					  "Redraw Requested",

				/* 025 */ "Editing Complete",
					  "Duplicate Image Name",
					  "Duplicate Figure Name",
					  "Image Already Hooked",
					  "Image Already Un-hooked",

				/* 030 */ "Figure Already Hooked",
					  "Figure Already Un-hooked"


				/* Values above 1000 are "errno" values */
					  };

static const char		 * const zip_figure_facility_names[] =
				{
			/* 000 */ "<<< UNKNOWN FACILITY NAME >>>",
				"zip_Initialize",
				"zip_Terminate",
				"zip_Set_general_Exception_Handler",
				"zip_Reset_general_Exception_Handler",

			/* 005 */ "zip_Figure",
				"zip_Image_Figure",
				"zip_Figure_Image",
				"zip_Figure_Name",
				"zip_Figure_Type",

			/* 010 */ "zip_Figure_01",
				"zip_Containing_Figure_Stream",
				"zip_Containing_Figure_Environment",
				"zip_Containing_Figure_Image",
				"zip_Set_Figure_Exception_Handler",

			/* 015 */ "zip_Reset_Figure_Exception_Handler",
				"zip_Set_Figure_Text",
				"zip_Set_Figure_Pattern",
				"zip_Set_Figure_Font",
				"zip_Display_Figure",

			/* 020 */ "zip_Draw_Figure",
				"zip_Clear_Figure",
				"zip_Which_Figure",
				"zip_Highlight_Figure_Points",
				"zip_Normalize_Figure_Points",

			/* 025 */ "zip_Hide_Figure_Points",
				"zip_Expose_Figure_Points",
				"zip_Change_Figure_Point",
				"zip_Remove_Figure_Point",
				"zip_Add_Figure_Point"
				};

static const char	 * const zip_image_facility_names[] =
				{
			/* 000 */ "<<< UNKNOWN FACILITY NAME >>>",
				"zip_Initialize",
				"zip_Terminate",
				"zip_Set_general_Exception_Handler",
				"zip_Reset_general_Exception_Handler",

			/* 005 */ "zip_Image",
				"zip_Stream_Image",
				"zip_Image_Name",
				"zip_Image_01",
				"zip_Containing_Image_Stream",

			/* 010 */ "zip_Containing_Image_Image",
				"zip_Containing_Image_Environment",
				"zip_Environment_Image???",
				"zip_Set_Image_Exception_Handler",
				"zip_Reset_Image_Exception_Handler",	

			/* 015 */ "zip_Set_Image_Text",
				"zip_Set_Image_Pattern",
				"zip_Set_Image_Font",
				"zip_Display_Image", 
				"zip_Draw_Image",

			/* 020 */"zip_Which_Image",
				"zip_Highlight_Image_Points",
				"zip_Normalize_Image_Points",
				"zip_Hide_Image_Points",
				"zip_Expose_Image_Points"
				};

static const char		 * const zip_stream_facility_names[] =
				{
			/* 000 */ "<<< UNKNOWN FACILITY NAME >>>",
				"zip_Initialize",
				"zip_Terminate",
				"zip_Set_general_Exception_Handler",
				"zip_Reset_general_Exception_Handler",

			/* 005 */ "zip_Stream",
				"zip_Stream_Name",
				"zip_Create_Stream",
				"zip_Destroy_Stream",
				"zip_Open_Stream",

			/* 010 */ "zip_Destroy_Stream",
				"zip_Open_Stream",
				"zip_Close_Stream",
				"zip_Read_Stream",
				"zip_Write_Stream",

			/* 015 */ "zip_Containing_Stream_Environment",
				"zip_Set_Stream_Exception_Handler",
				"zip_Reset_Stream_Exception_Handler",
				"zip_Set_Stream_Text",
				"zip_Set_Stream_Pattern",

			/* 020 */"zip_Set_Stream_Font",
				"zip_Set_Stream_Source",
				"zip_Display_Stream", 
				"zip_Draw_Stream",
				"zip_Which_Stream",

			/* 025 */"zip_Copy_Stream"
				};

static const char		 * const zip_pane_facility_names[] =
				{
			/* 000 */ "<<< UNKNOWN FACILITY NAME >>>",
				"zip_Initialize",
				"zip_Terminate",
				"zip_Set_general_Exception_Handler",
				"zip_Reset_general_Exception_Handler",

			/* 005 */ "zip_Pane",
				"zip_Pane_Name",
				"zip_Pane_Stream",
				"zip_Create_Window_Pane",
				"zip_Create_Panel_Layout",

			/* 010 */ "zip_Create_Layout_Pane",
				"zip_Destroy_Pane",
				"zip_Redisplay_Panes",
				"zip_Set_Pane_Exception_Handler",
				"zip_Reset_Pane_Exception_Handler",

			/* 015 */ "zip_Set_Pane_Coordinates",
				"zip_Set_Pane_Border",
				"zip_Set_Pane_Stream",
				"zip_Set_Pane_Image",
				"zip_Set_Pane_Figure",

			/* 020 */ "zip_Set_Pane_Zoom_Factor",
				"zip_Set_Pane_Text",
				"zip_Set_Pane_Pattern",
				"zip_Set_Pane_Font",
				"zip_Draw",

			/* 025 */ "zip_Which_Pane",
				"zip_Highlight_Pane_Points",
				"zip_Normalize_Pane_Points",
				"zip_Hide_Pane_Points",
				"zip_Expose_Pane_Points",

			/* 030 */ "zip_Display_Pane",
				"zip_Print_Pane",
				"zip_Clear_Pane",
				"zip_Invert_Pane",
				"zip_Zoom_Pane",

			/* 035 */ "zip_Scale_Pane",
				"zip_Handle_Planning",
				"zip_Pan_Pane",
				"zip_Pan_Pane_To_Edge",
				"zip_Flip_Pane",

			/* 040 */ "zip_Flop_Pane",
				"zip_Balance_Pane",
				"zip_Hide_Pane",
				"zip_Expose_Pane",
				"zip_Hide_Pane_Points",

			/* 045 */ "zip_Expose_Pane_Points",
				"zip_Hide_Pane_Coordinates",
				"zip_Expose_Pane_Coordinates",
				"zip_Hide_Pane_Coordinates",
				"zip_Expose_Pane_Coordinates",

			/* 050 */ "zip_Which_Pane"
				};


ATKdefineRegistryNoInit(zipstatus, zipview);
static char * Format_Message( class zipstatus	      *self, int			       facility, int			       status );
static char * Format_Figure_Status_Message ( class zipstatus		  *self, zip_type_figure		   figure );
static char * Format_Image_Status_Message ( class zipstatus		  *self, zip_type_image		   image );
static char * Format_Pane_Status_Message ( class zipstatus		  *self, zip_type_pane		   pane);
static char * Format_Stream_Status_Message( class zipstatus		  *self, zip_type_stream		   stream );


zipstatus::zipstatus( )
      {
  long			      status = zip_ok;
  IN(zipstatus::zipstatus);
/*===*/
  OUT(zipstatus::zipstatus);
  THROWONFAILURE(  (status == zip_ok));
  }

zipstatus::~zipstatus( )
    {
  IN(zipstatus::~zipstatus);
/*===*/
  OUT(zipstatus::~zipstatus);
  }

static char *
Format_Message( class zipstatus	      *self, int			       facility, int			       status )
        {
  static char				  msg[1001];
  const char				 *facility_name;
  char					  facility_msg[101];

  if ( facility >= zip_lowest_figure_facility_name    &&
       facility <= zip_highest_figure_facility_name )
    facility_name = zip_figure_facility_names[facility - zip_lowest_figure_facility_name];
  else
  if ( facility >= zip_lowest_image_facility_name   &&
      facility <= zip_highest_image_facility_name )
    facility_name = zip_image_facility_names[facility - zip_lowest_image_facility_name ];
  else
  if ( facility >= zip_lowest_stream_facility_name   &&
       facility <= zip_highest_stream_facility_name )
    facility_name = zip_stream_facility_names[facility - zip_lowest_stream_facility_name];
  else
  if ( facility >= zip_lowest_pane_facility_name   &&
       facility <= zip_highest_pane_facility_name )
    facility_name = zip_pane_facility_names[facility - zip_lowest_pane_facility_name];
  else
    {
    sprintf( facility_msg, "<<< Un-Tabularized Facility Name: %d >>>", facility );
    facility_name = facility_msg;
    }
  if ( status >= zip_lowest_status_value  &&  status <= zip_highest_status_value )
    sprintf( msg, "%s: %s", facility_name, zip_status_messages[status] );
  else
    sprintf( msg, "%s (System Msg): %s", facility_name,
	     strerror(status - zip_system_status_value_boundary) );

  return msg;
  }

long
zipstatus::Issue_Message( char				  *msg )
      {
/*===  return (*MessageWriter)( self, msg );*/
return 0;
  }


long
zipstatus::Acknowledge_Message( char				  *msg )
      {
/*===  return (*MessageAcknowledger)( self, msg );*/
return 0;
  }


long
zipstatus::Clear_Message( )
    {
/*===  return (*MessageClearer)( self );*/
return 0;
  }

long
zipstatus::Issue_Status_Message( long facility, long status )
{
  return (this)->Issue_Message(  Format_Message( this, facility, status ) );
  }


long
zipstatus::Acknowledge_Status_Message( long				   facility, long				   status )
        {
  return (this)->Acknowledge_Message(  Format_Message( this, facility, status ) );
  }

static char *
Format_Figure_Status_Message ( class zipstatus		  *self, zip_type_figure		   figure )
      {
  static char				  msg[1001];
/*===
  char				 *facility_name;
  char					  facility_msg[101];
  int 				  status;
  int				  facility;

  facility = figure -> zip_figure_image ->zip_image_stream ->
		zip_stream_environment -> zip_env_facility;
  status = figure -> zip_figure_image -> zip_image_stream ->
		zip_stream_environment -> zip_env_status;
  if ( facility < zip_lowest_figure_facility_name  ||
       facility > zip_highest_figure_facility_name )
    {
    sprintf( facility_msg, "<<< Un-Tabularized Figure Facility Name: %d >>>", facility);
    facility_name = facility_msg;
    }
    else
    facility_name = zip_figure_facility_names[ facility - zip_lowest_figure_facility_name];
  if ( status >= zip_lowest_status_value  &&
       status <= zip_highest_status_value )
    sprintf( msg, "%s (Figure '%s'): %s",
	 facility_name, figure->zip_figure_name,
	 zip_status_messages[status] );
    else
      sprintf( msg, "%s (Figure '%s') (System Msg): %s",
	 facility_name, figure->zip_figure_name,
	 strerror(status - zip_system_status_value_boundary) );
===*/
  return msg;
  }

long
zipstatus::Issue_Figure_Status_Message( zip_type_figure		   figure )
      {
  return (this)->Issue_Message(  Format_Figure_Status_Message( this, figure ));
  }


long
zipstatus::Acknowledge_Figure_Status_Message( zip_type_figure		   figure )
      {
  return (this)->Acknowledge_Message(  Format_Figure_Status_Message( this, figure ));
  }

static char *
Format_Image_Status_Message ( class zipstatus		  *self, zip_type_image		   image )
      {
  static char				  msg[1001];
/*===
  char				 *facility_name;
  char					  facility_msg[101];
  int 				  status;
  int				  facility;

  facility = image->zip_image_stream->zip_stream_environment->zip_env_facility;
  status = image->zip_image_stream->zip_stream_environment->zip_env_status;
  if ( facility < zip_lowest_image_facility_name  ||
       facility > zip_highest_image_facility_name )
    {
    sprintf( facility_msg, "<<< Un-Tabularized Image Facility Name: %d >>>", facility);
    facility_name = facility_msg;
    }
    else
    facility_name = zip_image_facility_names[ facility - zip_lowest_image_facility_name];
  if ( status >= zip_lowest_status_value  &&
       status <= zip_highest_status_value )
    sprintf( msg, "%s (Image '%s'): %s",
	 facility_name, image->zip_image_name,
	 zip_status_messages[status] );
    else
      sprintf( msg, "%s (Image '%s') (System Msg): %s",
	 facility_name, image->zip_image_name,
	 strerror(status - zip_system_status_value_boundary) );
===*/
  return msg;
  }

long
zipstatus::Issue_Image_Status_Message( zip_type_image		   image )
      {
  return (this)->Issue_Message(  Format_Image_Status_Message( this, image ) );
  }


long
zipstatus::Acknowledge_Image_Status_Message( zip_type_image		   image )
      {
  return (this)->Acknowledge_Message(  Format_Image_Status_Message( this, image ) );
  }


static char *
Format_Pane_Status_Message ( class zipstatus		  *self, zip_type_pane		   pane)
      {
  static char				  msg[1001];
/*===
  char				 *facility_name;
  char					  facility_msg[101];
  int 				  status;
  int				  facility;

  facility = pane-> zip_pane_environment->zip_env_facility;
  status = pane-> zip_pane_environment->zip_env_status;
  if ( facility < zip_lowest_pane_facility_name  ||
       facility > zip_highest_pane_facility_name )
    {
    sprintf( facility_msg, "<<< Un-Tabularized Pane Facility Name: %d >>>", facility);
    facility_name = facility_msg;
    }
    else
    facility_name = zip_pane_facility_names[ facility - zip_lowest_pane_facility_name];
  if ( status >= zip_lowest_status_value  &&
       status <= zip_highest_status_value )
    sprintf( msg, "%s (Pane '%s'): %s",
	 facility_name, pane->zip_pane_name,
	 zip_status_messages[status] );
    else
      sprintf( msg, "%s (Pane '%s') (System Msg): %s",
	 facility_name, pane->zip_pane_name,
	 strerror(status - zip_system_status_value_boundary) );
===*/
 return msg;
  }

long
zipstatus::Issue_Pane_Status_Message( zip_type_pane		   pane)
      {
  return (this)->Issue_Message(  Format_Pane_Status_Message( this, pane ) );
  }


long
zipstatus::Acknowledge_Pane_Status_Message( zip_type_pane		   pane )
      {
  return (this)->Acknowledge_Message(  Format_Pane_Status_Message( this, pane ));
  }

static char *
Format_Stream_Status_Message( class zipstatus		  *self, zip_type_stream		   stream )
      {
  static char				  msg[1001];
/*===
  char				 *facility_name;
  char					  facility_msg[101];
  int 				  status;
  int				  facility;

  IN(Format_Stream_Status_Message);
  facility = stream -> zip_stream_environment -> zip_env_facility;
  status = stream -> zip_stream_environment -> zip_env_status;
  if ( facility < zip_lowest_stream_facility_name  ||
       facility > zip_highest_stream_facility_name )
    {
    sprintf( facility_msg, "<<< Un-Tabularized Stream Facility Name: %d >>>", facility);
    facility_name = facility_msg;
    }
    else
    facility_name = zip_stream_facility_names[ facility - zip_lowest_stream_facility_name];
  if ( status >= zip_lowest_status_value  &&
       status <= zip_highest_status_value )
    sprintf( msg, "%s (Stream '%s'): %s",
	 facility_name, stream->zip_stream_name,
	 zip_status_messages[status] );
    else
      sprintf( msg, "%s (Stream '%s') (System Msg): %s",
	 facility_name, stream->zip_stream_name,
	 strerror(status - zip_system_status_value_boundary) );
  OUT(Format_Stream_Status_Message);
===*/
  return msg;
  }

long
zipstatus::Issue_Stream_Status_Message( zip_type_stream		   stream )
      {
  return (this)->Issue_Message(  Format_Stream_Status_Message( this, stream ) );
  }


long
zipstatus::Acknowledge_Stream_Status_Message( zip_type_stream		   stream )
      {
  return (this)->Acknowledge_Message(  Format_Stream_Status_Message( this, stream ) );
  }
