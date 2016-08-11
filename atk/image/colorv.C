

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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/image/RCS/colorv.C,v 1.2 1994/11/30 20:42:06 rr2b Stab74 $";
#endif
#include <andrewos.h>
ATK_IMPL("colorv.H")
#include <rect.h>
#include <view.H>
#include <color.H>
#include <colorv.H>


ATKdefineRegistry(colorv, view, colorv::InitializeClass);
#ifndef NORCSID
#endif


boolean
colorv::InitializeClass( )
  {
  return(TRUE);
}

colorv::colorv( )
    {
	ATKinit;

  THROWONFAILURE((TRUE));
}

colorv::~colorv( )
    {
	ATKinit;


}

void
colorv::FullUpdate( enum view_UpdateType  type, long  left , long  top , long  width , long  height )
      {
  class color *c = (class color*) (this)->GetDataObject();
  unsigned short R, G, B;
  struct rectangle r;

  (c)->GetRGB( R, G, B);
  (this)->SetForegroundColor( (c)->Name(), R, G, B);
  (this)->GetVisualBounds( &r);
  (this)->FillRect( &r, (this)->BlackPattern());
}

void
colorv::Update( )
  {
  (this)->FullUpdate( view_FullRedraw, 0, 0, 0, 0);
}
