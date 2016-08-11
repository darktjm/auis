/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	For full copyright information see: <src or dest>/config/COPYRITE

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/examples/ex1/RCS/hwview.C,v 1.4 1996/12/19 20:22:52 fred Exp $";
#endif



#include <andrewos.h>
ATK_IMPL("hwview.H")
#include "graphic.H"
#include "hwview.H"


ATKdefineRegistry(helloworldview, view, NULL);
#ifndef NORCSID
#endif


void helloworldview::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
                        {
    int x,y;
    struct rectangle VisualRect;		

    (this)->GetVisualBounds(&VisualRect);
    x = rectangle_Left(&VisualRect) + rectangle_Width(&VisualRect)/2;
    y = rectangle_Top(&VisualRect) + rectangle_Height(&VisualRect)/2;

    (this)->MoveTo(x,y);
    (this)->DrawString("hello world",
	graphic_BETWEENTOPANDBASELINE | graphic_BETWEENLEFTANDRIGHT);    

}
