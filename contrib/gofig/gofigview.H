#ifndef _gofigview_H_
#define _gofigview_H_
/* ********************************************************************** *\
 *	   Copyright Carnegie Mellon, 1994 - All Rights Reserved
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
gofigv.H

	Class definitions for gofig inset view
*/
/*
 *    $Log: gofigview.H,v $
 * Revision 1.1  1994/10/14  21:04:02  wjh
 * Initial revision
 * 
 * Revision 1.0  94/08/17  18:52:51  wjh
 * Copied from /usr/andrew.C++/lib/null
 */

enum actionopts {Alternate, White, Black, Empty, Select};
enum noteopts {Number,Letter,None};

#include <view.H>
class gofigview  : public  view    {
public:
	virtual ATKregistryEntry *ATKregistry();

	void FullUpdate(enum view_UpdateType type, 
		long left, long top, long width, long height);
	void Update();
	class view * Hit(enum view_MouseAction action, long x, long y, long n);
	view_DSattributes DesiredSize(long width, long height, 
		enum view_DSpass pass, long *desiredWidth, long *desiredHeight);
	void ReceiveInputFocus();
	void LoseInputFocus();
	void ObservedChanged(class observable *dobj, long status);

	virtual void DesiredPrintSize(long width, long height, 
		enum view_DSpass pass, long *desiredwidth, long *desiredheight);
	virtual void *GetPSPrintInterface(char *printtype);
	virtual void PrintPSRect(FILE *outfile, long logwidth, long logheight, 
		struct rectangle *visrect);
	virtual void PrintPSDoc(FILE *outfile, long pagew, long pageh);
	virtual void Print(FILE *file, char *processor, char *finalFormat, 
		boolean topLevel);
	
	static boolean InitializeClass();

	gofigview();
	virtual ~gofigview();

	class menulist  *Menus;
	class keystate *Keystate;
	class color *boardcolor;

	boolean OnScreen;		/* if not view_Removed */
	boolean embedded;	/* TRUE if no call to GetApplicationLayer */
	boolean HasInputFocus;	/* T if received input focus */

	boolean updaterequested;
	int visibleaccnum;	/* accnum of most recently displayed stone */
	int curspotrow, curspotcol;	/* current intersection or stone */
		/* curspotrow is -1 if there is no current spot */

	int spotwidth;
	int spotheight;
	int xoff,  yoff;	/* offset of where to begin drawing */
	int linethickness;	/* grid lines and white stone border */
	int radius;		/* radius of stone image
				(if linethickness is odd, actual radius is 1/2 larger) */
	long xhoshi, yhoshi;	/* bit vectors i is a ?hoshi coord if (?hoshi&(1<<i)) */

	fontdesc *notefont;	/* currently selected font */
	int fontxw;		/* width of X in notefont */
	int fontsz;			/* nominal fontsize of notefont */

	enum actionopts hitaction, menuhitaction;
	enum noteopts hitnote, menuhitnote;
	char nextcolor;
	int nextnum;
	char nextlet;			/* 'a' ... 'n' */

	struct stone stonewheredown;	/* stone at point of mouse down */
	boolean downstonemodified;		/* changed color or note */
	int previousrow, previouscol;		/* most recent mouse coords */
		/* previousrow and stonewheredown.row may be -1 */
};

#endif /* _gofigview_H_ */
