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

/* zipvr00.c	Zip PrintView-object				      */
/* Author	TC Peters					      */
/* Information Technology Center	   Carnegie-Mellon University */



/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Zip PrintView-object

MODULE	zipvr00.c

NOTICE	IBM Internal Use Only

DESCRIPTION
	This is the suite of Methods that support the Printing facilities
	of the Zip View-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  03/31/88	Created (TCP)
  11/18/88	Add line-width checking (TCP)
  11/21/88	Fix Poly-Line shading action (TCP)
  06/08/89	Drop troff emission of ".wh -1i, etc" (TCP)
  06/09/89	Accomodate new tmac macros for full-page drawings (TCP)
  07/11/89	Fix Level-One printing (troff macro glitch) (TCP)
		Remove unused troff code and tables.
   08/16/90	Add support for printing with line styles (SCG)

END-SPECIFICATION  ************************************************************/


#include <andrewos.h>
#include <math.h>
#include "view.H"
#include "environ.H"
#include "fontdesc.H"
#include "apts.H"
#include "zip.H"
#include "zipview.H"
#include "zipprint.H"
#include "texttroff.H"

#define  Data			(self->data_object)
#define  View			(self->view_object)

#define  PrintFile		(Printing->zip_printing_file)
#define  PrintLevel		(Printing->zip_printing_level)
#define  PrintLanguage		(Printing->zip_printing_language)
#define  PrintProcessor		(Printing->zip_printing_processor)
#define  PrintOrientation	(Printing->zip_printing_orientation)
#define  PrintPrefix		(Printing->zip_printing_prefix)
#define  PostScriptLanguage	(PrintLanguage == zip_postscript)
#define  TroffLanguage		(PrintLanguage == zip_troff)
#define  PostScriptProcessor	(PrintProcessor == zip_postscript)
#define  TroffProcessor		(PrintProcessor == zip_troff)

#define  InchWidth		(Printing->zip_printing_inch_width)
#define  InchHeight		(Printing->zip_printing_inch_height)


/*=== PostScript usage should be more sophisticated... ===*/
/* Split up header so module will compile on Suns - tpn */
static const char				  ZIP_postscript_header1[] =
"\n"
"\\!%!PS-Adobe-2.0 EPSF-1.2\n"
"\\!% Begin Zip PostScript Prelude  Version 0.0\n"
"\\!gsave  % Save Environment Around Zip Drawing\n"
"\\!1 -1 scale\n"
"\\!1 setlinewidth  2 setlinejoin\n"
"\\!\n"
"\\!/zip_Rectangle {\n"
"\\!	gsave newpath\n"
"\\!	0 setgray /shade exch def\n"
"\\!	/B exch def  /R exch def  /T exch def  /L exch def\n"
"\\!	L T moveto  R T lineto  R B lineto  L B lineto  L T lineto\n"
"\\!	shade 0 gt {gsave shade setgray fill grestore} if\n"
"\\!	closepath currentlinewidth 0 gt {stroke} if  grestore\n"
"\\!	}def\n"
"\\!/zip_Round_Rectangle {\n"
"\\!	gsave newpath\n"
"\\!	0 setgray /shade exch def\n"
"\\!	/YR exch def\n"
"\\!	/XR exch def\n"
"\\!	/B exch def  /R exch def  /T exch def  /L exch def\n"
"\\!	L XR add   T moveto\n"
"\\!	R T  R B XR arcto  4 {pop} repeat\n"
"\\!	R B  L B XR arcto  4 {pop} repeat\n"
"\\!	L B  L T XR arcto  4 {pop} repeat\n"
"\\!	L T  R T XR arcto  4 {pop} repeat\n"
"\\!	shade 0 gt {gsave shade setgray fill grestore} if\n"
"\\!	closepath currentlinewidth 0 gt {stroke}if  grestore\n"
"\\!	}def\n"
"\\!/zip_Line {\n"
"\\!	0 setgray moveto lineto stroke\n"
"\\!	}def\n";

static const char				  ZIP_postscript_header2[] =
"\\!/zip_Poly_Line {\n"
"\\!	newpath 0 setgray\n"
"\\!	2 sub /npoints exch def\n"
"\\!	moveto currentpoint\n"
"\\!	/lastY exch def   /lastX exch def\n"
"\\!	0 1 npoints {pop lineto} for\n"
"\\!	lastx lasty lineto\n"
"\\!	/lastx lastX def  /lasty lastY def\n"
"\\!	shade 0 gt {gsave shade setgray fill grestore} if\n"
"\\!	currentlinewidth 0 gt {stroke} if\n"
"\\!	}def\n"
"\\!/zip_Trapezoid {\n"
"\\!    setgray\n"
"\\!    moveto  3 {lineto} repeat\n"
"\\!    fill  0 setgray\n"
"\\!    }def\n"
"\\!/zip_Ellipse {\n"
"\\!    moveto  /R exch def\n"
"\\!    gsave scale\n"
"\\!    currentpoint newpath\n"
"\\!    R 0 360 arc  closepath stroke grestore\n"
"\\!    }def\n"
"\\!/zip_Fill_Ellipse {\n"
"\\!    moveto  /R exch def\n"
"\\!    gsave scale\n"
"\\!    currentpoint newpath\n"
"\\!    R 0 360 arc  closepath setgray fill  grestore\n"
"\\!    }def\n"
"\\!/zip_Center_X {\n"
"\\!    stringwidth pop  2 div sx exch sub /sx exch def\n"
"\\!    }def\n"
"\\!/zip_Right_X {\n"
"\\!    stringwidth pop  sx exch sub /sx exch def\n"
"\\!    }def\n"
"\\!/zip_Middle_Y {\n"
"\\!    2 div sy exch sub /sy exch def\n"
"\\!    }def\n"
"\\!/zip_Bottom_Y {\n"
"\\!    sy exch sub /sy exch def\n"
"\\!    }def\n"
"\\!% End Zip PostScript Prelude\n"
"\\!\n"
"\\!% Begin Zip Drawing\n";



int zipprint_Write_Print_Datastream_Header( class zipprint	          *self );
int zipprint_Write_Print_Datastream_Trailer( class zipprint	          *self );
static char * zipprint_Line_Attributes_String( class zipprint	          *self );


int
zipprint_Write_Print_Datastream_Header( class zipprint	          *self )
    {
  int				  status = zip_ok;
  long				  IH72 = (long) (72*InchHeight),
					  IW72 = (long) (72*InchWidth), W, H;
  char				  *cursor;
  char					  *ZIP_postscript_header;

  IN(zipprint_Write_Print_Datastream_Header);
  if ( PostScriptLanguage )
    {
    if ( PrintOrientation == zip_landscape )
      {W = IH72; H = IW72;}
      else
      {W = IW72; H = IH72;}
    if( PrintLevel == 1  &&  TroffProcessor )
      {
      texttroff::BeginDoc( PrintFile );
/*=== fprintf( PrintFile, ".wh -1i\n.rm NP\n.sp -0.5i\n.po 0\n" )*/
      fprintf( PrintFile, ".pl 12i\n.rm NP\n.sp -0.5i\n.po 0\n" );
      }
    if ( TroffProcessor )
      {
      PrintPrefix = "\\!";
      texttroff::BeginPS( PrintFile, W, H );
      }
      else  if ( PostScriptProcessor )
      {
      PrintPrefix = "  ";
      fprintf( PrintFile, "%%!\n" );
      }
    ZIP_postscript_header = (char *) malloc(strlen(ZIP_postscript_header1) + strlen(ZIP_postscript_header2) + 1);
    strcpy(ZIP_postscript_header,ZIP_postscript_header1);
    strcat(ZIP_postscript_header,ZIP_postscript_header2);
    cursor = ZIP_postscript_header;
    while ( *cursor )
      {
      if ( *cursor  &&  *cursor == '\n' )
	{
	cursor++;
	if ( *cursor )	*cursor = PrintPrefix[0];
	cursor++;
	if ( *cursor )	*cursor = PrintPrefix[1];
	}
      cursor++;
      }
    fprintf( PrintFile, "%s", ZIP_postscript_header );
    free(ZIP_postscript_header );
    if ( PrintOrientation == zip_landscape )
      {
      fprintf( PrintFile, "%s -90 rotate  -17 0 translate  %% LandScape Mode\n",
	       PrintPrefix );
      }
      else
      fprintf( PrintFile, "%s %d %ld translate  %% Portrait Mode\n",
	       PrintPrefix, 0, -(H-17) );
    fprintf( PrintFile, "%s /width %ld def /height %ld def  %% Set Clip Rectangle\n",
	     PrintPrefix, IW72, IH72 );
    fprintf( PrintFile, "%s newpath 0 0 moveto\n", PrintPrefix );
    fprintf( PrintFile, "%s 0 height lineto width height lineto\n", PrintPrefix );
    fprintf( PrintFile, "%s width 0 lineto 0 0 lineto clip  newpath\n", PrintPrefix );
    }
  else status = zip_failure;
  OUT(zipprint_Write_Print_Datastream_Header);
  return status;
  }

int
zipprint_Write_Print_Datastream_Trailer( class zipprint	          *self )
    {
  int				  status = zip_ok;

  IN(zipprint_Write_Print_Datastream_Trailer);
  if ( PostScriptLanguage )
    {
    fprintf( PrintFile, "%s grestore  %% Restore Environment Around Zip Drawing\n",
	     PrintPrefix );
    fprintf( PrintFile, "%s %% End Zip Drawing\n", PrintPrefix );
    if ( TroffProcessor )
      {
      if ( PrintOrientation == zip_landscape )
        texttroff::EndPS(PrintFile, (int)(72*InchHeight), (int)(72*InchWidth) );
        else
        texttroff::EndPS(PrintFile, (int)(72*InchWidth), (int)(72*InchHeight) );
      if( PrintLevel == 1 )  texttroff::EndDoc( PrintFile );
      }
      else
      if ( PostScriptProcessor )
	fprintf( PrintFile, "   showpage  %% Emit Zip Drawing\n" );
    }
  else status = zip_failure;
  OUT(zipprint_Write_Print_Datastream_Trailer);
  return status;
  }

void zipprint::Set_Line_Width( long	line_width )
{
    class zipprint *self=this;
    IN(zipprint_Set_Line_Width);
    Printing->zip_printing_line_width = line_width;
    OUT(zipprint_Set_Line_Width);
}

long zipprint::Ensure_Line_Attributes( zip_type_figure		 figure )
{
  class zipprint *self=this;
  unsigned char		lwidth;
  long				status = zip_ok;
  char					*pattern = NULL;
  int					offset;
  short					dashtype, value;

  if ((  Printing->zip_printing_line_width =
	 (Data)->Contextual_Figure_Line_Width(  figure )) > 0 )
    {    
    (Data)->Contextual_Figure_Line_Dash(  figure, &pattern, &offset, &dashtype );
    if ( pattern )
    {
      Printing->zip_printing_line_dash_pattern = pattern;
      Printing->zip_printing_line_dash_offset = offset;
    }
    else Printing->zip_printing_line_dash_pattern = NULL;
    if (( value = (Data)->Contextual_Figure_Line_Cap(  figure )) != -1 )
      Printing->zip_printing_line_cap = value;
    else       Printing->zip_printing_line_cap = graphic_CapButt;
    if (( value = (Data)->Contextual_Figure_Line_Join(  figure )) != -1 )
      Printing->zip_printing_line_join = value;
    else Printing->zip_printing_line_join = graphic_JoinMiter;
    }
    else Printing->zip_printing_line_dash_pattern = NULL;
    return status;
  }

static char *
zipprint_Line_Attributes_String( class zipprint	          *self )
    {
  static char			string[300];
  char				temp[100];
  const char		       *p;
  short				cap, join;

    switch ( Printing->zip_printing_line_cap )
    {
	case graphic_CapNotLast:
	case graphic_CapButt: cap = 0; break;
	case graphic_CapRound: cap = 1; break;
	case graphic_CapProjecting: cap = 2; break;
    }
    switch ( Printing->zip_printing_line_join )
    {
	case graphic_JoinMiter: join = 0; break;
	case graphic_JoinRound: join = 1; break;
	case graphic_JoinBevel: join = 2; break;
    }
    sprintf( string, "%s %d setlinecap %d setlinejoin %d setlinewidth\n", PrintPrefix, cap, join, Printing->zip_printing_line_width );
    if ( Printing->zip_printing_line_dash_pattern )
    {
      sprintf( temp, "%s [", PrintPrefix );
      strcat( string, temp ); 
      p = Printing->zip_printing_line_dash_pattern;
      while( *p != 0 )
      {
	sprintf( temp, "%d ", *p );
	strcat( string, temp );
	p++;
      }
      sprintf( temp, "] %d setdash\n", Printing->zip_printing_line_dash_offset );
      strcat( string, temp );
    }
    else
    {
	sprintf( temp, "%s [] 0 setdash\n", PrintPrefix );
	strcat( string, temp );
    }
    return string;
  }

void zipprint::Set_Shade( long  shade )
{
    class zipprint *self=this;
    IN(zipprint_Set_Shade);
    if ( shade )
    {
	Printing->zip_printing_shade = (100.0 - (shade - 1.0)) / 100.0;
	if ( Printing->zip_printing_shade <= 0.01 )
	    Printing->zip_printing_shade = 0.02;
    }
    else
	Printing->zip_printing_shade = 0.0;
    OUT(zipprint_Set_Shade);
}

void zipprint::Move_To( long x , long y )
      {
  class zipprint *self=this;
  IN(zipprint::Move_To);
  if ( PostScriptLanguage )
    fprintf( PrintFile, "%s newpath %.2f %.2f moveto\n", PrintPrefix, x/100.0, y/100.0 );
  OUT(zipprint::Move_To);
  }

void zipprint::Draw_To( long  x , long y )
{
  class zipprint *self=this;
  IN(zipprint::Draw_To);
  if ( PostScriptLanguage )
    fprintf( PrintFile, "%s %.2f %.2f lineto\n", PrintPrefix, x/100.0, y/100.0 );
  OUT(zipprint::Draw_To);
}

void zipprint::Close_Path( )
    {
  class zipprint *self=this;
  IN(zipprint::Close_Path);
  if ( PostScriptLanguage )
    fprintf( PrintFile, "%s closepath stroke\n", PrintPrefix );
  OUT(zipprint::Close_Path);
  }

void zipprint::Draw_Multi_Line( int npoints , int	x_origin , int y_origin, zip_type_point_pairs   points )
        {
  class zipprint *self=this;
  long				  i, count, nchunks, remainder;
  float			  shade = 0.0;

  IN(zipprint::Draw_Multi_Line);
  DEBUGdt(Points,npoints);
  if ( PostScriptLanguage )
    {
    i = 0 ;
    if ( x_origin == points->zip_points[npoints-1].zip_point_x  &&
	 y_origin == points->zip_points[npoints-1].zip_point_y )
      shade = zipprint_Printing_Shade( self );
    fprintf( PrintFile, "%s", zipprint_Line_Attributes_String( self ));
    fprintf( PrintFile, "%s /lastx %.2f def /lasty %.2f def /shade %.2f def\n",
	     PrintPrefix, x_origin/100.0, y_origin/100.0,
	     shade );
    nchunks = npoints / 200;
    remainder = npoints % 200;
    while ( i < npoints )
      {
      count = 0;
      fprintf( PrintFile, "%s ", PrintPrefix );
      while( count < 5 && ((i + count) < npoints) )
        {
        fprintf( PrintFile, " %.2f %.2f",
                 points->zip_points[i+count].zip_point_x/100.0,
                 points->zip_points[i+count].zip_point_y/100.0);
        if ( nchunks && (( (i + count + 1) % 200 ) == 0 ))
          fprintf( PrintFile, "\n%s 200 zip_Poly_Line", PrintPrefix );
    	if ( count == 5 )  fprintf( PrintFile, "\n%s ", PrintPrefix );
        count++;
        }
      fprintf( PrintFile, "\n" );
      i += count;
      }
    if (remainder) fprintf( PrintFile,"%s %ld zip_Poly_Line\n", PrintPrefix, remainder );
    }
  OUT(zipprint::Draw_Multi_Line);
  }

static   char   ZIP_pending_font[100] = ".ft R\n.ps 12"; /*=== improve ===*/
static   int CURRENTFONTSIZE;/*===*/

void zipprint::Draw_String( int x , int y, char *string, long mode )
          {
  class zipprint *self=this;
  char					 *expansion,
					 *s, *t;
  boolean			  force_centered = false;

  IN(zipprint::Draw_String);
  if ( mode == 0 )
    force_centered = true;
  if ( PostScriptLanguage )
    {
    expansion = t = (char *) malloc( 2 * strlen( string ) + 1 );
    s = string;
    while ( *s )
      {
      if ( *s == '('  ||  *s == ')'  ||  *s == '\\'  ||  *s == '/' )
	*t++ = '\\';
      *t++ = *s++;
      }
    *t = 0;
    fprintf( PrintFile, "%s gsave 1 -1 scale\n", PrintPrefix );
    fprintf( PrintFile, "%s /sx %.2f def  /sy %.2f neg def\n", PrintPrefix, x/100.0, y/100.0 );
    if ( (mode & zip_center)  ||  force_centered )
      fprintf( PrintFile, "%s (%s) zip_Center_X\n", PrintPrefix, expansion );
    else if ( mode & zip_right )
      fprintf( PrintFile, "%s (%s) zip_Right_X\n", PrintPrefix, expansion );
/*===*/
    if ( (mode & zip_middle)  ||  force_centered )
      fprintf( PrintFile, "%s %d zip_Middle_Y\n", PrintPrefix, CURRENTFONTSIZE );
    else if ( mode & zip_bottom )
      fprintf( PrintFile, "%s %d zip_Bottom_Y\n", PrintPrefix, CURRENTFONTSIZE);
/*===*/
    fprintf( PrintFile, "%s sx sy moveto (%s) show grestore\n", PrintPrefix, expansion );
    free( expansion );
    }
  OUT(zipprint::Draw_String);
  }

/* FaceCode flags */
#define ZIP_BoldFace 1
#define ZIP_ItalicFace 2
#define ZIP_ShadowFace 4
#define ZIP_FixedWidthFace 010


void zipprint::Change_Font( class fontdesc  *font )
      {
  class zipprint *self=this;
  char				 *font_family;
  char					  face[50], ItalObl[20], Default[20];
  int				  font_face;
  int				  font_height;

  IN(zipprint::Change_Font);
  font_family = (font )->GetFontFamily( );
  font_face   = (font )->GetFontStyle( );
  font_height = (font )->GetFontSize( );
  CURRENTFONTSIZE=font_height;
  DEBUGst(Family,font_family);
  DEBUGdt(Face,  font_face);
  DEBUGdt(Height,font_height);
  *face = *ItalObl = *Default = 0;
  if(apts::CompareStrings("andysans",font_family)==0)strcpy(font_family,"Helvetica");
  else if(apts::CompareStrings("andy",font_family)==0)strcpy(font_family,"Times");
  if ( PostScriptLanguage )
    {
    if ( strcmp( font_family, "Times" ) == 0 )
      {
      strcpy( ItalObl, "Italic" );
      strcpy( Default, "-Roman" );
      }
    else if ( strcmp( font_family, "Helvetica" ) == 0 ||
              strcmp( font_family, "Courier" ) == 0 )
      {
      strcpy( ItalObl, "Oblique" );
      }
    switch ( font_face )
      {
      case ZIP_BoldFace + ZIP_ItalicFace: sprintf( face, "-Bold%s", ItalObl );
                                          break;
      case ZIP_BoldFace:                  sprintf( face, "-Bold" );
                                          break;
      case ZIP_ItalicFace:                sprintf( face, "-%s", ItalObl );
                                          break;
      default:                            sprintf( face, "%s", Default );
                                          break;
      }
    sprintf( ZIP_pending_font, "/%s%s findfont %d scalefont setfont\n",
                               font_family, face, font_height );
    fprintf( PrintFile, "%s /%s%s findfont %d scalefont setfont\n", PrintPrefix,
                               font_family, face, font_height );
    DEBUGst(Pending-Font,ZIP_pending_font);
    }
  OUT(zipprint::Change_Font);
  }

void zipprint::Restore_Font( )
    {
  IN(zipprint::Restore_Font);
  OUT(zipprint::Restore_Font);
  }

void zipprint::Draw_Line( int x1 , int y1 , int x2 , int y2 )
      {
  class zipprint *self=this;
  IN(zipprint::Draw_Line);
  if ( PostScriptLanguage )
    {
    if ( zipprint_Printing_Line_Width( self ) )
      {
        fprintf( PrintFile, "%s", zipprint_Line_Attributes_String( self ));
        fprintf( PrintFile, "%s %.2f %.2f %.2f %.2f zip_Line\n", PrintPrefix,
			x2/100.0, y2/100.0, x1/100.0, y1/100.0 );
      }
    }
  OUT(zipprint_Draw_Line);
  }

void zipprint::Draw_Rectangle( long	 left , long top , long right , long bottom )
      {
  class zipprint *self=this;
  static const char				  format[] =
"%s %.2f %.2f %.2f %.2f %.2f zip_Rectangle\n";


  IN(zipprint::Draw_Rectangle);
  if ( PostScriptLanguage )
    {
    fprintf( PrintFile, "%s", zipprint_Line_Attributes_String( self ));
    fprintf( PrintFile, format, PrintPrefix,
	     left/100.0, top/100.0, right/100.0, bottom/100.0,
	     zipprint_Printing_Shade( self ));
    }
  OUT(zipprint_Draw_Rectangle);
  }

void zipprint::Draw_Round_Rectangle( long left , long top , long	right , long	 bottom , long	 x_radius , long y_radius )
      {
  class zipprint *self=this;
  static const char			  format[] =
"%s %.2f %.2f %.2f %.2f %.2f %.2f %.2f zip_Round_Rectangle\n";

  IN(zipprint::Draw_Round_Rectangle);
  if ( PostScriptLanguage )
    {
    fprintf( PrintFile, "%s", zipprint_Line_Attributes_String( self ));
    fprintf( PrintFile, format, PrintPrefix,
	     left/100.0, top/100.0, right/100.0, bottom/100.0,
	     x_radius/100.0, y_radius/100.0,
	     zipprint_Printing_Shade( self ));
    }
  OUT(zipprint::DrawRound_Rectangle);
  }

void zipprint::Draw_Circle( int x_center , int y_center , int x_radius )
  {
  (this)->Draw_Ellipse(  x_center, y_center, x_radius, x_radius );
  }

void zipprint::Draw_Ellipse( int x_center , int y_center, int x_radius , int y_radius )
        {
  class zipprint *self=this;
  float			  x_scale, y_scale;

  IN(zipprint::Draw_Ellipse);
  DEBUGdt(X_radius,x_radius);  DEBUGdt(Y_radius,y_radius);
  if ( x_radius == 0 )	x_radius = 1;
  if ( y_radius == 0 )	y_radius = 1;
  if ( PostScriptLanguage )
    {
    if ( x_radius > y_radius )
      {
      y_scale = (1.0*y_radius) / (1.0*(x_radius) ? (x_radius) : 1);
      x_scale = 1.0;
      }
    else
      {
      y_scale = 1.0;
      x_scale = (1.0*x_radius) / (1.0*(y_radius) ? (y_radius) : 1);
      }
    DEBUGgt(X_scale,x_scale);  DEBUGgt(Y_scale,y_scale);
    if ( zipprint_Printing_Shade( self ) )
      {
      fprintf( PrintFile, "%s %.2f %.2f %.2f %.2f %.2f %.2f zip_Fill_Ellipse\n", PrintPrefix,
		    zipprint_Printing_Shade( self ),
                    x_scale, y_scale,
                    ( y_scale == 1.0 ) ? y_radius/100.0 : x_radius/100.0,
		    x_center/100.0, y_center/100.0 );

      }
    if ( zipprint_Printing_Line_Width( self ) )
      {
      fprintf( PrintFile, "%s", zipprint_Line_Attributes_String( self ));
      fprintf( PrintFile, "%s %.2f %.2f %.2f %.2f %.2f zip_Ellipse\n", PrintPrefix,
                    x_scale, y_scale,
                    ( y_scale == 1.0 ) ? y_radius/100.0 : x_radius/100.0,
		    x_center/100.0, y_center/100.0 );
      }
    }
  OUT(zipprint::Draw_Ellipse);
  }

void zipprint::Draw_Arc( long x_center , long y_center, long x_radius , long  y_radius, long x_start , long y_start, long  x_end , long y_end )
      {
  class zipprint *self=this;
  float			  start_angle, end_angle, theta,
		    			  x_scale, y_scale;
  static const char			  format[] =
"%s /cmtx matrix currentmatrix def %.2f %.2f moveto gsave "
" %.2f %.2f scale currentpoint newpath %.2f %.2f %.2f arcn "
" cmtx setmatrix stroke grestore\n";

  if ( PostScriptLanguage )
    {
    if ( zipprint_Printing_Line_Width( self ) )
      {
      if ( x_radius > y_radius )
        {
        y_scale = (1.0*y_radius) / (1.0*(x_radius) ? (x_radius) : 1);
        x_scale = 1.0;
        }
        else
        {
        y_scale = 1.0;
        x_scale = (1.0*x_radius) / (1.0*(y_radius) ? (y_radius) : 1);
        }
      theta = atan2( 1.0 * ((y_center - y_start) ? (y_center - y_start) : 1),
		     1.0 * ((x_start - x_center) ? (x_start - x_center) : 1) );
      start_angle = 360.0 * (theta / (2.0 * M_PI));
      theta = atan2( 1.0 * ((y_center - y_end) ? (y_center - y_end) : 1),
		     1.0 * ((x_end - x_center) ? (x_end - x_center) : 1 ) );
      end_angle = 360.0 * (theta / (2.0 * M_PI));
      fprintf( PrintFile, "%s", zipprint_Line_Attributes_String( self ));
      fprintf( PrintFile, format, PrintPrefix,
        x_center/100.0, y_center/100.0, x_scale, -y_scale,
       ( y_scale == 1.0 ) ? y_radius/100.0 : x_radius/100.0,
        start_angle, end_angle );
      }
    }
  }

void zipprint::Arc_To( int x_center , int	 y_center, int x_radius , int y_radius, int x_start , int y_start, int x_end , int y_end )
{
  class zipprint *self=this;
  int				  x1, y1;

  if ( PostScriptLanguage )
    {
    if ( zipprint_Printing_Line_Width( self ) )
      {
      x1 = ( x_center == x_start ) ? x_end : x_start;
      y1 = ( y_center == y_start ) ? y_end : y_start;
      fprintf( PrintFile, "%s", zipprint_Line_Attributes_String( self ));
      fprintf( PrintFile, "%s %.2f %.2f %.2f %.2f %.2f arcto stroke 4 {pop} repeat\n", PrintPrefix,
                       x1/100.0, y1/100.0, x_end/100.0, y_end/100.0, x_radius/100.0 );
      }
    }
  }

void
zipprint::Fill_Trapezoid( int x1 , int   y1 , int   x2 , int   y2 , int   l1 , int   l2, char   pattern )
{
  class zipprint *self=this;
  long				  x3, x4;
  float			  gray = 1.0;

  if ( PostScriptLanguage )
    {
    DEBUGdt(X1,x1);    DEBUGdt(Y1,y1);
    DEBUGdt(X2,x2);    DEBUGdt(Y2,y2);
    DEBUGdt(L1,l1);    DEBUGdt(L2,l2);
    switch ( pattern )  /*=== HM shade font characters only!! ===*/
      {
    case '\002': gray = 0.9;               break;
    case '\004': gray = 0.7;               break;
    case '\010': gray = 0.5;               break;
    case '\014': gray = 0.3;               break;
    case '\016': gray = 0.1;               break;
    case '\020': gray = 0.0;               break;
    default:   gray = 1.0;
      }
    x3 = x2 + l2;
    x4 = x1 + l1;
    DEBUGdt(X3,x3);
    DEBUGdt(X4,x4);
    fprintf( PrintFile, "%s %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %f zip_Trapezoid\n", PrintPrefix,
                x1/100.0, y1/100.0, x2/100.0, y2/100.0, x3/100.0, y2/100.0,
		x4/100.0, y1/100.0, gray );
  }
  }

long
zipprint::Try_Printing_Exception_Handler( zip_type_printing		   printing )
{
  class zipprint *self=this;
/*===
  if ( PrintingExceptionHandler )
    {
    (*PrintingExceptionHandler)( self, printing );
    return true;
    }
    else
===*/
    return (Data)->Try_general_Exception_Handler( );
}
