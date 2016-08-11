/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
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


 

#include <fontdesc.ih>
#include <rect.h>

class stringV[stringv] : valueview[valuev] {
 overrides:
  DrawFromScratch(long x,y,width,height);
  DrawHighlight();
  DrawDehighlight();
  LookupParameters();
  DrawNewValue();
 classprocedures:
  InitializeClass() returns boolean;
 macromethods:
   GetUseAlt()(self->UseAlt)
   SetUseAlt(boolean val) (self->UseAlt = val)
 data:
  char * label;
  char * fontname;
  short fontsize;
  struct fontdesc * normalfont, * boldfont, * activefont;
  struct rectangle labelrec;
  long x,y,width,height;
  boolean UseAlt;
  char *plabel,*foreground,*background;
};

/* parameters to spst
   label		string		Defaults to NULL
   bodyfont		string		Defaults to "andytype"
   bodyfont-size	long		Defaults to 10
*/
