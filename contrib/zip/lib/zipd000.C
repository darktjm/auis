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

/* zipd000.c	Zip Data-object	-- general			      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */


/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip Data-object -- general	

MODULE	zipd000.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support general facilities
	of the Zip Data-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  03/31/88	Created (TCP)
  11/17/88	Add Contextual_Figure_Line_Width (TCP/SCG)
  05/10/89	Declare Contextual_Figure_Line_Width as "unsigned char" (SCG)
  08/14/90	Add Contextual methods for color and line style attributes (SCG)
  08/14/90	Add Allocate methods for dynamic color structures (SCG)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "view.H"
#include "fontdesc.H"
#include "zip.H"
#include <util.h>

#define	 Data this

static int ZIP_Default_Exception_Handler( class zip	    	  *self );
int strhash (char  *string, unsigned int size);


struct zip_color_values *
zip::Allocate_Color_Values( )
{
  return ( struct zip_color_values * ) calloc( 1, sizeof( struct zip_color_values ));
}

struct zip_color *
zip::Allocate_Color( )
{
  return ( struct zip_color * ) calloc( 1, sizeof( struct zip_color ));
}

char
zip::Contextual_Figure_Pattern( zip_type_figure	   figure )
      {
  char			  pattern = 0;

  IN(zip::Contextual_Figure_Pattern);
  if ( figure->zip_figure_fill.zip_figure_pattern )
    pattern = figure->zip_figure_fill.zip_figure_pattern;
  else
  if ( figure->zip_figure_image->zip_image_fill.zip_image_pattern )
    pattern = figure->zip_figure_image->zip_image_fill.zip_image_pattern;
  else
  if ( ( pattern = (this)->Superior_Image_Pattern(  figure->zip_figure_image->zip_image_superior ) ) )
    {}
  else
  if ( figure->zip_figure_image->zip_image_stream->zip_stream_fill.zip_stream_pattern )
    pattern = figure->zip_figure_image->zip_image_stream->zip_stream_fill.zip_stream_pattern;
  OUT(zip::Contextual_Figure_Pattern);
  return  pattern;
  }

char
zip::Contextual_Figure_Shade( zip_type_figure	   figure )
      {
  char			  shade = 0;

  IN(zip::Contextual_Figure_Shade);
  if ( figure->zip_figure_fill.zip_figure_shade )
    shade = figure->zip_figure_fill.zip_figure_shade;
  else
  if ( figure->zip_figure_image->zip_image_fill.zip_image_shade )
    shade = figure->zip_figure_image->zip_image_fill.zip_image_shade;
  else
  if ( ( shade = (this)->Superior_Image_Shade(  figure->zip_figure_image->zip_image_superior ) ) )
    {}
  else
  if ( figure->zip_figure_image->zip_image_stream->zip_stream_fill.zip_stream_shade )
    shade = figure->zip_figure_image->zip_image_stream->zip_stream_fill.zip_stream_shade;
  OUT(zip::Contextual_Figure_Shade);
  return  shade;
  }

unsigned char
zip::Contextual_Figure_Line_Width( zip_type_figure	   figure )
      {
  unsigned char	  width = 0;

  IN(zip_Contextual_Figure_Line_Width);
  if ( figure->zip_figure_line_width != 255 )
    width = figure->zip_figure_line_width;
  else
  if ( figure->zip_figure_image->zip_image_line_width != 255 )
    width = figure->zip_figure_image->zip_image_line_width;
  else
  if (( width =
      (this)->Superior_Image_Line_Width(  figure->zip_figure_image->zip_image_superior ))
      != 255 )
    {}
  else
  if ( figure->zip_figure_image->zip_image_stream->zip_stream_line_width != 255 )
    width = figure->zip_figure_image->zip_image_stream->zip_stream_line_width;
  OUT(zip_Contextual_Figure_Line_Width);
  return  width;
  }

void
zip::Contextual_Figure_Line_Dash( zip_type_figure	   figure, char			  **pattern, int			  *offset, short		  *type )
            {
  IN(zip_Contextual_Figure_Line_Dash);
  if ( figure->zip_figure_line_dash_pattern )
  {
    *pattern = figure->zip_figure_line_dash_pattern;
    *offset = figure->zip_figure_line_dash_offset;
    *type = figure->zip_figure_line_dash_type;
  }
  else
  if ( figure->zip_figure_image->zip_image_line_dash_pattern )
  {
    *pattern = figure->zip_figure_image->zip_image_line_dash_pattern;
    *offset = figure->zip_figure_image->zip_image_line_dash_offset;
    *type = figure->zip_figure_image->zip_image_line_dash_type;
  }
  else
  if ( (this)->Superior_Image_Line_Dash(  figure->zip_figure_image->zip_image_superior, pattern, offset, type ) != zip_failure )
    {}
  else
  if ( figure->zip_figure_image->zip_image_stream->zip_stream_line_dash_pattern )
  {
    *pattern = figure->zip_figure_image->zip_image_stream->zip_stream_line_dash_pattern;
    *offset = figure->zip_figure_image->zip_image_stream->zip_stream_line_dash_offset;
    *type = figure->zip_figure_image->zip_image_stream->zip_stream_line_dash_type;
  }
  OUT(zip_Contextual_Figure_Line_Dash);
  }

short
zip::Contextual_Figure_Line_Join( zip_type_figure	   figure )
      {
  short	  join = -1;

  IN(zip_Contextual_Figure_Line_Join);
  if ( figure->zip_figure_line_join != -1 )
    join = figure->zip_figure_line_join;
  else
  if ( figure->zip_figure_image->zip_image_line_join != -1 )
    join = figure->zip_figure_image->zip_image_line_join;
  else
  if (( join =
      (this)->Superior_Image_Line_Join(  figure->zip_figure_image->zip_image_superior ))
      != -1 )
    {}
  else
  if ( figure->zip_figure_image->zip_image_stream->zip_stream_line_join != -1 )
    join = figure->zip_figure_image->zip_image_stream->zip_stream_line_join;
  OUT(zip_Contextual_Figure_Line_Join);
  return  join;
  }

short
zip::Contextual_Figure_Line_Cap( zip_type_figure	   figure )
      {
  short	  cap = -1;

  IN(zip_Contextual_Figure_Line_Cap);
  if ( figure->zip_figure_line_cap != -1 )
    cap = figure->zip_figure_line_cap;
  else
  if ( figure->zip_figure_image->zip_image_line_cap != -1 )
    cap = figure->zip_figure_image->zip_image_line_cap;
  else
  if (( cap =
      (this)->Superior_Image_Line_Cap(  figure->zip_figure_image->zip_image_superior ))
      != -1 )
    {}
  else
  if ( figure->zip_figure_image->zip_image_stream->zip_stream_line_cap != -1 )
    cap = figure->zip_figure_image->zip_image_stream->zip_stream_line_cap;
  OUT(zip_Contextual_Figure_Line_Cap);
  return  cap;
  }

long
zip::Contextual_Figure_Line_Color( zip_type_figure	     figure, double		     *red , double		     *green , double		     *blue )
        {
  struct zip_color	    *color;
  long			    status = zip_ok;

  IN(zip_Contextual_Figure_Line_Color);
  if ( figure->zip_figure_color_values && figure->zip_figure_color_values->line )
  {
      *red = figure->zip_figure_color_values->line->red;
      *green = figure->zip_figure_color_values->line->green;
      *blue = figure->zip_figure_color_values->line->blue;
  }
  else
  if ( figure->zip_figure_image->zip_image_color_values && ( color = figure->zip_figure_image->zip_image_color_values->line ))
  {
      *red = color->red;
      *green = color->green;
      *blue = color->blue;
  }
  else
  if ( ( color = (this)->Superior_Image_Line_Color(  figure->zip_figure_image->zip_image_superior )))
    {
      *red = color->red;
      *green = color->green;
      *blue = color->blue;
  }
  else
  if ( figure->zip_figure_image->zip_image_stream->zip_stream_color_values && ( color = figure->zip_figure_image->zip_image_stream->zip_stream_color_values->line ))
  {
      *red = color->red;
      *green = color->green;
      *blue = color->blue;
  }
  else status = zip_failure;
  OUT(zip_Contextual_Figure_Line_Color);
  return status;
  }

long
zip::Contextual_Figure_FillFG_Color( zip_type_figure	     figure, double		     *red , double		     *green , double		     *blue )
        {
  struct zip_color	    *color;
  long			    status = zip_ok;

  IN(zip_Contextual_Figure_FillFG_Color);
  if ( figure->zip_figure_color_values && figure->zip_figure_color_values->fillfg )
  {
      *red = figure->zip_figure_color_values->fillfg->red;
      *green = figure->zip_figure_color_values->fillfg->green;
      *blue = figure->zip_figure_color_values->fillfg->blue;
  }
  else
  if ( figure->zip_figure_image->zip_image_color_values && ( color = figure->zip_figure_image->zip_image_color_values->fillfg ))
  {
      *red = color->red;
      *green = color->green;
      *blue = color->blue;
  }
  else
  if ( ( color = (this)->Superior_Image_FillFG_Color(  figure->zip_figure_image->zip_image_superior )))
    {
      *red = color->red;
      *green = color->green;
      *blue = color->blue;
  }
  else
  if ( figure->zip_figure_image->zip_image_stream->zip_stream_color_values && ( color = figure->zip_figure_image->zip_image_stream->zip_stream_color_values->fillfg ))
  {
      *red = color->red;
      *green = color->green;
      *blue = color->blue;
  }
  else status = zip_failure;
  OUT(zip_Contextual_Figure_FillFG_Color);
  return status;
  }


long
zip::Contextual_Figure_FillBG_Color( zip_type_figure	     figure, double		     *red , double		     *green , double		     *blue )
        {
  struct zip_color	    *color;
  long			    status = zip_ok;

  IN(zip_Contextual_Figure_FillBG_Color);
  if ( figure->zip_figure_color_values && figure->zip_figure_color_values->fillbg )
  {
      *red = figure->zip_figure_color_values->fillbg->red;
      *green = figure->zip_figure_color_values->fillbg->green;
      *blue = figure->zip_figure_color_values->fillbg->blue;
  }
  else
  if ( figure->zip_figure_image->zip_image_color_values && ( color = figure->zip_figure_image->zip_image_color_values->fillbg ))
  {
      *red = color->red;
      *green = color->green;
      *blue = color->blue;
  }
  else
  if ( ( color = (this)->Superior_Image_FillBG_Color(  figure->zip_figure_image->zip_image_superior )) )
    {
      *red = color->red;
      *green = color->green;
      *blue = color->blue;
  }
  else
  if ( figure->zip_figure_image->zip_image_stream->zip_stream_color_values && ( color = figure->zip_figure_image->zip_image_stream->zip_stream_color_values->fillbg ))
  {
      *red = color->red;
      *green = color->green;
      *blue = color->blue;
  }
  else status = zip_failure;
  OUT(zip_Contextual_Figure_FillBG_Color);
  return status;
  }


class fontdesc *
zip::Define_Font( const char *font_name, short             	  *font_index )
        {
  int			  loop_index;
  char				  family_name[257];
  long				  font_style;
  long				  font_size;
  class fontdesc	 *font = NULL;

  IN(zip::Define_Font);
  DEBUGst(Font-name,font_name);
  if ( font_index )  *font_index = 0;
  if ( Fonts == NULL )
    Fonts =
		(zip_type_fonts) calloc( 1, sizeof(struct zip_fonts) );
  if ( Fonts )
    {
    for ( loop_index = 0; loop_index < Fonts->zip_fonts_count; loop_index++ )
      if ( apt_MM_Compare( font_name,
	 	Fonts->
		  zip_fonts_vector[loop_index].zip_fonts_table_name ) == 0 )
        {
        if ( font_index )  *font_index = loop_index;
	font = Fonts->zip_fonts_vector[loop_index].zip_fonts_table_font;
	DEBUGdt(Old,loop_index);
        break;
        }
    if ( font == NULL )
      {
      if ( ( Fonts = (zip_type_fonts)
	    realloc( Fonts, sizeof(struct zip_fonts) +
	    ((Fonts->zip_fonts_count + 1) *
		 sizeof(struct zip_fonts_table)) ) ) )
        {
        if ( font_index )  *font_index = loop_index;
        if ( ( Fonts->zip_fonts_vector[Fonts->zip_fonts_count].
		 zip_fonts_table_name = strdup( font_name ) ) )
	  {
	  fontdesc::ExplodeFontName( font_name, family_name, sizeof( family_name ),
				  &font_style, &font_size );
          font = Fonts->zip_fonts_vector[Fonts->zip_fonts_count].
	    zip_fonts_table_font = fontdesc::Create( family_name, font_style, font_size );
	  Fonts->zip_fonts_count++;
	  DEBUGdt(New,Fonts->zip_fonts_count);
	  }
        }
      }
    }
  DEBUGxt(Font,font);
  OUT(zip::Define_Font);
  return  font;
  }

int	
zip::Try_Figure_Exception_Handler( zip_type_figure	       figure )
      {
  IN(zip::Try_Figure_Exception_Handler);
  if ( FigureExceptionHandler )
    {
    (*FigureExceptionHandler)( this, figure );
    OUT(zip::Try_Figure_Exception_Handler);
    return true;
    }
    else
    {
    OUT(zip::Try_Figure_Exception_Handler);
    return (this)->Try_general_Exception_Handler();
    }
  return false;
  }

int
zip::Try_Image_Exception_Handler( zip_type_image	       image )
      {
  IN(zip::Try_Image_Exception_Handler);
  if ( ImageExceptionHandler )
    {
    (*ImageExceptionHandler)( this, image );
    OUT(zip::Try_Image_Exception_Handler);
    return true;
    }
    else
    {
    OUT(zip::Try_Image_Exception_Handler);
    return (this)->Try_general_Exception_Handler( );
    }
  }

int
zip::Try_Stream_Exception_Handler( zip_type_stream	       stream )
      {
  IN(zip::Try_Stream_Exception_Handler);
  if ( StreamExceptionHandler )
    {
    (*StreamExceptionHandler)( this, stream );
    OUT(zip::Try_Stream_Exception_Handler);
    return true;
    }
    else
    {
    OUT(zip::Try_Stream_Exception_Handler);
    return (this)->Try_general_Exception_Handler( );
    }
  return false;
  }

int
zip::Try_general_Exception_Handler( )
    {
  IN(zip::Try_general_Exception_Handler);
  if ( generalExceptionHandler )
    {
    (*generalExceptionHandler) (this, Facility, STATUS);
    OUT(zip::Try_general_Exception_Handler);
    return true;
    }
    else
    {
    OUT(zip::Try_general_Exception_Handler);
    return ZIP_Default_Exception_Handler( this );
    }
  return false;
  }

static int ZIP_Default_Exception_Handler( class zip *self )
{
    IN(ZIP_Default_Exception_Handler);
    /*===*/
    OUT(ZIP_Default_Exception_Handler);
    return(0);
}

#include <errno.h>



/*=== Borrowed from MH Conner, April 16, 1984 (his April 14, 1984 Version) ===*/



/* #define debug */
/*
PALLOC - April 14, 1885
--Propertry of IBM--
--IBM Internal Use Only--

NAME

palloc, palloc_create_pool, palloc_destroy_pool - simple, allocate only storage 
	management

SYNOPSIS

DESCRIPTION

The routines implement a very effishent (in both time and storage), but
very constrained storage manager.  Once a pool is created by a call to 
palloc_create_pool chunks of storage can be allocated by calls to palloc.
The big constraint is that only the whole pool can ever be freed (via a 
call to palloc_destroy_pool).

DIAGNOSTICS

Return codes (except palloc)

	-1 -- Unable to perform operation due to resource limitations
		--not yet checked--
	0 -- Operation completed normally

Return codes for palloc:

	0 -- Unable to perform operation due to resource limitations
		--not yet checked--
	>0 -- address of allocated chunk

AUTHOR

Michael H Conner

FILES

BUGS

*/
struct block {
    struct block *next_block;
    unsigned char body[1];		/* variable length byte array */
};

typedef struct {
    unsigned int block_size;
    struct block *current_block;
    unsigned char *pos_in_current_block;
    unsigned int space_left_in_current_block;
    struct block first_block;
} pool_type;

int palloc_create_pool (pool_type  **pool, unsigned int expected_size);
unsigned char *palloc (pool_type  *pool, unsigned int size);
unsigned int palloc_destroy_pool (pool_type  *pool);

int palloc_create_pool (pool_type  **pool, unsigned int expected_size)
        {
/*===
    /! allocate the pool discriptor including the first block !/

    *pool = (pool_type *) malloc (sizeof(pool_type)+expected_size);

    /! verify that the allocation was sucessful, return if not !/
    if (*pool == 0) {
	return (-1);
    }

     /! initialize the pool descriptor !/

    (*pool)->block_size = (expected_size > 4000) ? (expected_size / 4) :
         1000;  /! the minimum additional block is 1000 bytes !/

    (*pool)->current_block = &((*pool)->first_block);

    (*pool)->pos_in_current_block = &((*pool)->first_block.body[0]);

    (*pool)->space_left_in_current_block = expected_size;

    /! initialize first block !/

    (*pool)->first_block.next_block = 0;
===*/
    return (0);
}

unsigned char *palloc (pool_type  *pool, unsigned int size)
        {
return (unsigned char *) malloc( (size + 3) & (~3) );
/*===
    unsigned char *chunk; /! holds address of allocated chunk !/

/! force size to be a multiple of 4 for alignment !/
    size = (size + 3) & (~3);
    /! check for available space !/
    if (size > pool->space_left_in_current_block) {
	/! not enough space left- go get another block and chain it on !/
	if (size > pool->block_size) {
	    pool->block_size = size; /! adjust block size so chunk will just
	        fit. This should not happen, but just in case ... !/
	}
	(pool->current_block)->next_block = (struct block *) malloc (
	    sizeof (struct block *) + pool->block_size);
	/! verify that the allocation was sucessful, return is not !/
	if ((pool->current_block)->next_block == 0) {
	    return (0);
	}
	pool->current_block = (pool->current_block)->next_block;
	(pool->current_block)->next_block = 0;
        pool->pos_in_current_block = &((pool->current_block)->body[0]);
        pool->space_left_in_current_block = pool->block_size;
    }
    /! allocate chunk and increment stuff !/
    chunk = pool->pos_in_current_block;
    pool->pos_in_current_block += size;
    pool->space_left_in_current_block -= size;
    return (chunk);
===*/
}

unsigned int palloc_destroy_pool (pool_type  *pool)
    {
/*===
    struct block *this_block;
    struct block *next_block;

    /! set up for scan and free pool descriptor which includes the first block !/
    this_block = pool->first_block.next_block;
   free (pool);

    /! free each of the blocks !/
   
    while (this_block != 0) {
	next_block = this_block->next_block;
	free (this_block);
	this_block = next_block;
    }
===*/
    return(0);
}


/****************************************************************************\
* 									     *
* SYMTAB - April 14, 1984						     *
* --Property of IBM--							     *
* --IBM Internel Use Only--						     *
* --Author: Michael Conner--						     *
* 									     *
* symtab.cc: package of routines to support symbol table usage		     *
* 									     *
* The following routines are provided:					     *
* 									     *
* symtab_create: Makes and initializes a symbol table			     *
* 									     *
* symtab_add: Adds an entry and its associated data to a symbol table	     *
* 									     *
* symtab_find: Locates an entry in a symbol table and returns its	     *
* 	associated data							     *
* 									     *
* symtab_find_update: The same as symtab_find except if the entry is not     *
* 	found then symtab_find_update will put it in the symbol table	     *
* 	and return a pointer to a pointer which the user can set to point    *
* 	to his user data.						     *
* 	--not yet available--						     *
* 									     *
* symtab_remove: removes an entry from a symbol table			     *
*        --not yet available--						     *
* 									     *
* symtab_destroy: uninitializes and destroys a symbol table		     *
*        --not yet available--						     *
* 									     *
* symtab_scan_reset: restarts scanning of a symbol table		     *
* 									     *
* symtab_scan_next: returns the next entry in a symbol table until	     *
*        there are none left, the first time this routine is called	     *
*        and any time it is called after a call to symtab_scan_reset	     *
*        a new scan is started						     *
* 									     *
* There are three types of parameters that these routines take		     *
* as follows:								     *
* 									     *
* symbol table: a point to a private struct used by the symbol table	     *
* 	routines to identify a particular symbol table			     *
* 									     *
* symbol: an unsigned char string which is the symbol			     *
* 									     *
* data: a pointer to a user private struct				     *
*        which contains whatever data the user choses			     *
* 									     *
* Return codes: Many of these routines return an integer value to indicate   *
* the outcome of the operation.  The integer can be interpreted as follows:  *
* 									     *
* 	-1 -- Resource depletion, unable to carry out operation because	     *
* 		of resource (usally memory) limitations.		     *
* 									     *
* 	0 -- Normal return, the operation did its normal thing		     *
* 									     *
* 	1 -- Entry not found or not previously in the table (in the case     *
* 		of symtab_find_update).  This is the return given by	     *
* 		symtab_scan_next when the table has been completly	     *
* 		scanned.						     *
* 									     *
\****************************************************************************/

/****************\
* 		 *
* symtab_create	 *
* 		 *
\****************/
int symtab_create (symtab_type  **symtab, int  expected_size)
    {
    int     i;
    pool_type *pool;
    int rc; 			/* return codes */

  /*IN(symtab_create);*/
rc = palloc_create_pool (&pool, expected_size*(sizeof(int *)+sizeof(symtab_entry_type))
    + sizeof(symtab_type));
if (rc < 0) {
    return (rc); /*unable to create pool */
}
    *symtab = (symtab_type *) palloc (pool, sizeof (symtab_type) + (expected_size)
	    * sizeof (int *));
    (*symtab)->size = expected_size;
    (*symtab)->pool = (char *)pool;
    for (i = 0; i < expected_size; i++) {
	((*symtab)->table)[i] = 0;
    }
  OUT(symtab_create);
return 0;
}

void symtab_destroy( symtab_type	          *symtab )
    {
  int			  i;
  symtab_entry_type	 *entry, *ptr;

  /*IN(symtab_destroy);*/
  for (i = 0; i < symtab->size; i++)
    {
    if ( (symtab->table)[i] )
      {
      entry = symtab->table[i];
      while ( entry )
	{
	ptr = entry->next;
	free( entry );
	entry = ptr;
	}
      }
    }
  free( symtab );
  OUT(symtab_destroy);
  }

/*************\
* 	      *
* symtab_add  *
* 	      *
\*************/
int symtab_add (symtab_type  *symtab, unsigned char *symbol, struct user_data  *data)
      {
    symtab_entry_type * entry;
    int     tabrow;		/* row in symtab->table where this symbols
				   entry goes */

 /* allocate and initialize entry */
    entry = (symtab_entry_type *) palloc((pool_type *) symtab->pool, sizeof (symtab_entry_type));
    if (!entry) {
	return (-1);
    }
    entry->image = symbol;
    entry->data = data;

 /* compute row in symbol table and insert entry */
    tabrow = strhash ((char *) symbol, symtab->size);
    entry->next = (symtab->table)[tabrow];
    (symtab->table)[tabrow] = entry;
    return 0;
}

int
symtab_delete( symtab_type  *symtab, unsigned char *symbol )
      {
  symtab_entry_type *entry, *ptr;
  int  tabrow;
  int	status = 0;

  /*IN(symtab_delete);*/
  tabrow = strhash( (char *) symbol, symtab->size );
  DEBUGdt(Tabrow,tabrow);
  entry = (symtab->table)[tabrow];
  while ( entry && (strcmp ((char *) entry->image, (char *)symbol) != 0))
    entry = entry->next;
  if ( entry )
    {
    DEBUG(Entry Found);
    ptr = (symtab->table)[tabrow];
    if ( ptr == entry )
      (symtab->table)[tabrow] = entry->next;
      else
      {
      while ( ptr->next != entry )
	ptr = ptr->next;
      ptr->next = entry->next;
      }
    free( entry );
    }
    else status = -1;
  OUT(symtab_delete);
  return status;
  }

/**************\
* 	       *
* symtab_find  *
* 	       *
\**************/
int symtab_find (symtab_type  *symtab, unsigned char *symbol, struct user_data  **data)
      {
    symtab_entry_type * entry;
    int     tabrow;		/* row in symbol table where symbol should
				   be */
    int     rc = 0;		/* return code */

 /* compute row in symbol table */
    tabrow = strhash ((char *) symbol, symtab->size);

 /* scan chain at appropriate row in table for symbol */
    entry = (symtab->table)[tabrow];
    while (entry && (strcmp ((char *) entry->image, (char *) symbol) != 0)) {
	entry = entry->next;
    }
 /* if entry = 0 then no match was found */
    if (entry) {
	*data = entry->data;
	rc = 0;
    }
    else {
	*data = 0;
	rc = 1;
    }
    return (rc);
}

#if 0 /* unused */
/********************\
* 		     *
* symtab_scan_reset  *
* 		     *
\********************/
void symtab_scan_reset (symtab_type  *symtab)
  {
    symtab->row = -1;
    symtab->next = 0;
}

/*******************\
* 		    *
* symtab_scan_next  *
* 		    *
\*******************/
int symtab_scan_next (symtab_type  *symtab, unsigned char **symbol, struct user_data  **data)
      {
    symtab_entry_type * entry = NULL;
    int     rc = 0;

    while (symtab->row < symtab->size) {
	entry = symtab->next;
	if (entry) {
	/* next entry found */
	    *symbol = entry->image;
	    *data = entry->data;
	    symtab->next = entry->next;
	    break;
	}
	else {
	    (symtab->row)++;
	    symtab->next = (symtab->table)[symtab->row];
	}
    }

/* if entry is 0 then no entry was found, set rc accordingly	*/
    if (entry == 0) {
	rc = 1;
    }
 return (rc);
}
#endif


/*************************************************************\
* 							      *
*  strhash: a routine to hash strings			      *
* 							      *
*  Parameters are:					      *
* 							      *
*  string(in): the string to be hashed			      *
* 							      *
*  size(in): the hash value returned will be >= 0 and < size  *
* 							      *
*  value(out): the hash value				      *
* 							      *
\*************************************************************/

int strhash (char  *string, unsigned int size)
    
{
    unsigned int    temp_sum = 0;

    while (*string) {
	temp_sum = (temp_sum * 31) + (*string - 31);
				/* overflow is ignored */
	string++;
    }
    return temp_sum % size;
}
