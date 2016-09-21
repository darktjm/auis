#ifndef _zipifm00_h_
#define _zipifm00_h_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libzip
 * @{ */
/* zipifm00.h	Internal Macros						      */
/* Author	TC Peters & RM LeVine					      */
/* Information Technology Center		   Carnegie-Mellon University */


/***  SPECIFICATION -- Internal Facility Suite  *********************************

TITLE	Zip Figure X

MODULE	zipifm00.h

NOTICE	IBM Internal Use Only

DESCRIPTION
	These facilities are the Internal Facility Macros.

HISTORY
  05/16/86	Created (RM LeVine)
  05/16/86	Added ZIP_... ( EEFN, FEFN, SEFN, IEFN, PEFN) (RML)
  05/18/86	Added ZIP_xSTATUS ( E, S, I, F, P ) (TCP/RML)
  07/04/86	Added ZIP_REFN and ZIP_RSTATUS (RML)
  09/20/86	Added xEFN check for null panes, etc (TCP)
  08/17/87	Migration: introduce ZIP_WM_... macros (TCP)
  03/31/88	Revise for ATK (TCP)

END-SPECIFICATION  ************************************************************/

#define ZIP_EFN( FACILITY ) self->data_object->facility = FACILITY

#define ZIP_STATUS(zip)\
	if ( (zip->status = status) != zip_success )\
	  {(zip)->Try_general_Exception_Handler(  );}

#define ZIP_Select_Figure_Font( obj, figure )\
	(obj->view_object)->SetFont( obj->data_object->fonts->zip_fonts_vector[figure->zip_figure_font].zip_fonts_table_font)

#define ZIP_Select_Image_Font( obj, image )\
	(obj->view_object)->SetFont( obj->data_object->fonts->zip_fonts_vector[image->zip_image_font].zip_fonts_table_font )

#define ZIP_Select_Stream_Font( obj, stream )\
	(obj->view_object)->SetFont( obj->data_object->fonts->zip_fonts_vector[stream->zip_stream_font].zip_fonts_table_font )

#define ZIP_Select_Pane_Font( obj, pane )\
	(obj->view_object)->SetFont( obj->data_object->fonts->zip_fonts_vector[pane->zip_pane_font].zip_fonts_table_font )
/** @} */
#endif /* _zipifm00_h_ */
