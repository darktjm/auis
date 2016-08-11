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
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*
zipopolygon.H

  03/31/88	Create for ATK (TCP)
*/

#define  zipopolygon_VERSION    1

#define  Data				(self->header.zipobject.data_object)
#define  Env				(Data->env)
#define  View				(self->header.zipobject.view_object)
#define  Edit				(self->header.zipobject.edit_object)
#define  Print				(self->header.zipobject.print_object)


class zipopolygon[zipopoly] : zipocirc
  {

overrides:

  Object_Icon()						returns char;
  Object_Icon_Cursor()					returns char;
  Object_Datastream_Code()				returns char;

  Build_Object( pane, action, x, y, clicks, X, Y )	returns long;
  Show_Object_Properties( pane, figure )		returns long;
  Draw_Object( object, pane )				returns long;
  Clear_Object( object, pane )				returns long;
  Print_Object( object, pane )				returns long;

  };

