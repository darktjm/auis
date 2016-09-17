ATK_INTER
#ifndef _xcursor_H_
#define _xcursor_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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
 *	cursor.ch
 */

#include <rect.h>

#include <cursor.H>
class xcursor : public  cursor {
public:
	virtual ATKregistryEntry *ATKregistry();


	virtual void SetStandard(short ch);
	virtual void ChangeShape();

	virtual void Make(class xim *im);

	
	xcursor();
	
	virtual ~xcursor(); /*  used to get rid of active cursors */
	
	static boolean InitializeClass();

	static void FinalizeDisplay(Display *dpy);

	Cursor  Xc;
	Window Xw;
	Display * Xd;
	class xim *im;
	boolean valid;
};

#endif /* _xcursor_H_ */
