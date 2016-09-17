ATK_INTER
#ifndef _nullview_H_
#define _nullview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *	   Copyright Carnegie Mellon, 1992 - All Rights Reserved
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
nullv.H

	Class definitions for null inset view

*/
/*
 *    $Log: nullview.H,v $
// Revision 1.6  1995/02/09  22:46:21  Zarf
// Added methods for new printing mechanism
//
// Revision 1.5  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.4  1993/09/28  04:52:23  rr2b
// Fixed view_DSattributes to be an int not an enum.
//
 * Revision 1.3  1993/06/10  20:24:44  rr2b
 * Great atk_inited removal.
 *
 * Revision 1.2  1993/06/05  00:32:29  rr2b
 * Converted to C++
 *
 * Revision 1.1  1993/06/04  23:26:51  rr2b
 * Initial revision
 *
*Revision 1.5  1992/12/14  20:40:31  rr2b
*disclaimerization
*
Revision 1.4  1992/06/22  21:48:52  wjh
minor reformatting
.ting fontdesc
made blobs chartreuse on color displays
removed extraneous clearing of screen in _ClearRectangle
check for NULLs in InitializeClass
give a default desiredSize
.

Revision 1.3  1991/09/12  19:31:03  bobg
Update copyright notice

Revision 1.2  1989/12/12  15:00:16  ghoti
sync with MIT tape

Revision 1.2  89/12/05  11:33:54  xguest
change instances of nullv to nullview

Revision 1.1  89/12/04  20:27:58  xguest
Initial revision

Revision 1.1  89/07/31  15:35:27  wjh
Initial revision


Revision 1.0  88/05/14  15:40:34  wjh
Copied from /usr/andrew/lib/genericinset
 */


#include <view.H>
class nullview  : public  view
{
public:
	virtual ATKregistryEntry *ATKregistry();



	 void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
	 void Update();
	 class view * Hit(enum view_MouseAction action, long x, long y, long n)	 ;
	 view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *desiredWidth, long *desiredHeight) 
				 ;
	 void ReceiveInputFocus();
	 void LoseInputFocus();
	 void ObservedChanged(class observable *dobj, long status);
	 void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
	 void *GetPSPrintInterface(const char *printtype);
	 void PrintPSDoc(FILE *outfile, long pagew, long pageh);
	 void PrintPSRect(FILE *file, long logwidth, long logheight, struct rectangle *visrect);
	 class view *GetApplicationLayer();
	
	/* $$$ methods unique to this view.
		These would be defined if a parent view were to be given 
		greater control over this view. */
	
	static boolean InitializeClass();


	nullview();
	virtual ~nullview();

/* Create default Keymap & Menus */

	class menulist  *Menus;
	class keystate *Keystate;

	class graphic  *bpattern;	/* these might differ between instances . . . */
	class graphic  *wpattern;	/*		. . . on different displays */

	boolean OnScreen;			/* if not view_Removed */
	boolean embedded;		/* TRUE if no call to GetApplicationLayer */
	boolean HasInputFocus;		/* T if received input focus */
	boolean ignoreUp;			/* T iff have just asked for InputFocus */
	boolean sizeknown;		/* T if haven't called ComputeArea */

	/* $$$ some facts basic to computing desired size
		null chooses to have a given area if the parent
		constrains height or width */
	long DesiredArea;			/* total space desirable in square pixels */

	/* $$$ save desired size if the computation of it was expensive */
	long DesiredHeight, DesiredWidth;	/* requested dimensions; set by 
						ComputeArea*/
};

#endif /* _nullview_H_ */
