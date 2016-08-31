#ifndef _ziposymbol_H_
#define _ziposymbol_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


 

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*
ziposymbol.H

  07/10/88	Create for ATK (TCP)
*/

#define  ziposymbol_VERSION    1


struct symbol
  {
  short					  symbol_serial;
  char					 *symbol_icon;
  char					 *symbol_name;
  char					 *symbol_algorithm;
  short					  symbol_left, symbol_top, symbol_right, symbol_bottom;
  boolean				  symbol_highlighted;
  };

struct symbol_set
  {
  char					 *set_name;
  const char				 *set_path;
  FILE					 *set_file;
  struct symbol				 *set_symbols;
  short					  set_symbols_count;
  short					  set_left, set_top, set_right, set_bottom;
  boolean				  set_highlighted;
  };


#include <ziporect.H>
class ziposymbol : public  ziporect
  {
public:
	virtual ATKregistryEntry *ATKregistry();


  
	ziposymbol();
  
	virtual ~ziposymbol();


   char Object_Icon();
   char Object_Icon_Cursor();
   char Object_Datastream_Code();

   long Build_Object( zip_type_pane pane, enum view_MouseAction action, long x, long y, long clicks, zip_type_point X, zip_type_point Y );
   long Show_Object_Properties( zip_type_pane pane, zip_type_figure figure );
   long Draw_Object( zip_type_figure object, zip_type_pane pane );
   long Clear_Object( zip_type_figure object, zip_type_pane pane );
   long Print_Object( zip_type_figure object, zip_type_pane pane );


  long					  symbol_view_left,  symbol_view_top,
					  symbol_view_width, symbol_view_height,
					  symbol_view_right, symbol_view_bottom;
  boolean				  symbol_selected;
  char					  selected_symbol_set_name[257],
					  selected_symbol_index_name[25];
  struct symbol_set			 *selected_symbol_set;
  short					  selected_symbol_set_index;
  struct symbol				 *selected_symbol;
  struct symbol_set			 *symbol_set_displayed,
					 *current_symbol_set;
  short					  current_symbol_set_index;
  class graphic				 *gray_graphic;
  char					  gray_shade, line_width, line_style;
  long					  last_number;
  boolean				  outstanding_surround;
  long					  outstanding_left, outstanding_top,
					  outstanding_width, outstanding_height;
  struct zip_paths			 *library_paths;
  };


#endif /* _ziposymbol_H_ */
