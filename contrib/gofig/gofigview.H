#ifndef _gofigview_H_
#define _gofigview_H_
/* ********************************************************************** *\
 *	   Copyright Carnegie Mellon, 1994 - All Rights Reserved          *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libgofig
 * @{ */
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

#include "gofig.H"
#include <view.H>
/** A view for \ref gofig objects.
 *  This view presents a full or partial 25x25 Go board with pieces and
 *  annotations.
 */
class gofigview  : public  view    {
public:
	TRACED_CLASS(gofigview);

	void FullUpdate(enum view::UpdateType type, 
		long left, long top, long width, long height);
	void Update();
	class view * Hit(enum view::MouseAction action, long x, long y, long n);
	view::DSattributes DesiredSize(long width, long height, 
		enum view::DSpass pass, long *desiredWidth, long *desiredHeight);
	void ReceiveInputFocus();
	void LoseInputFocus();
	void ObservedChanged(class observable *dobj, long status);

	virtual void DesiredPrintSize(long width, long height, 
		enum view::DSpass pass, long *desiredwidth, long *desiredheight);
	virtual void *GetPSPrintInterface(const char *printtype);
	virtual void PrintPSRect(FILE *outfile, long logwidth, long logheight, 
		struct rectangle *visrect);
	virtual void PrintPSDoc(FILE *outfile, long pagew, long pageh);
	virtual void Print(FILE *file, const char *processor, const char *finalFormat, 
		boolean topLevel);
	
	static boolean InitializeClass();

	gofigview();

	class menulist  *Menus;
	class keystate *Keystate;
	class color *boardcolor;

	boolean OnScreen;		/* if not view::Removed */
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
/** @} */
#endif /* _gofigview_H_ */
