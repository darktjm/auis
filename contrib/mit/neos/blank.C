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
 * blank.c
 *
 * Make a blank rectangle in an lset.
 */

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *   For full copyright information see:'mit-copyright.h'     *
 *        This class by Nick Williams, June 1990  
 ************************************************************ */

#include <andrewos.h>
#include <mit-copyright.h>

#include "view.H"
#include "blank.H"
#include "graphic.H"


ATKdefineRegistry(blank, view, NULL);

void
blank::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    (this)->EraseVisualRect();
}

void
blank::Update()
{
    struct rectangle r;
    (this)->GetLogicalBounds( &r);
    (this)->FullUpdate( view_FullRedraw, r.left, r.top, r.width, r.height);
}
