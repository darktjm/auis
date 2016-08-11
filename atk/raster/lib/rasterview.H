ATK_INTER
#ifndef _rasterview_H_
#define _rasterview_H_
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
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*
rasterview.H

	Class definitions for raster view

*/
#define BORDER 2
#define TWOBORDER 4

#define  rasterview_PROGRAMMERVERSION    2
#define rasterview_SCALECHANGED 10

#include <view.H>
class rasterview : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();



	 void SetDataObject(class dataobject *ras);
		/* establishes data object */
	 void FullUpdate(enum view_UpdateType type, 
			long left, long top, long width, long height);
	 void Update();
	 class view * Hit(enum view_MouseAction action, 
			long x, long y, long n)	 ;
	 view_DSattributes DesiredSize(long width, long height, 
			enum view_DSpass pass, long *desiredWidth, long *desiredHeight) 
			 ;
	 void ReceiveInputFocus();
	 void LoseInputFocus();
	 void ObservedChanged(class observable *dobj, long status);
	 void Print(FILE *file, char *processor, 
			char *finalFormat, boolean topLevel);
	 class view * GetApplicationLayer()  ;
	 char *GetInterface(char *interface_name) 
			 ;
	 void WantUpdate(class view *requestor);
	 void PostMenus(class menulist *ml);
	 void LinkTree(class view *parent);

	 void PrintPSDoc(FILE *outfile, long pagew, long pageh);
	 void *GetPSPrintInterface(char *printtype);
	 void DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight);
	 void PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect);

	 virtual boolean Gifify(char *filename, long *pmaxw, long *pmaxh, 
			struct rectangle *visrect);
		// currently ignoring pmax...

	inline long GetScale() { return ((this)->Scale); }
        inline void SetCoordinateDisplay(boolean val) { ((this)->ShowCoords = (val)); }
        inline class view  * GetOverlaidInset() { return ((this)->inset); }


/*  -----------  Added 4/6/89 -------------- */
	virtual void FitToSize(struct rectangle logicalrect );
 	virtual void AutoCenter();
	virtual void ZoomRaster(boolean zoomIn );
	virtual void SetPan();
	virtual void SetRegionSelect();
	virtual void SetTouchUp();
	virtual void RotateRaster();
/*  -----------  Added  4/6/89 -------------- */

/*  -----------  Added 5/23/91 -------------- */
	virtual void SetScale(long newscale);
	virtual void ResizeRaster(long w, long h);
/*  -----------  Added 5/23/91 -------------- */

/* --- added 5/27/92 --- */
	virtual void SetToolMode();
	virtual int GetMode()  ;
	virtual void SetDesiredSelection(struct rectangle *R);
	virtual void CopySelection();
	virtual void DestroyToolset();
	virtual void SetPixel(class raster *ras, long x, long y, int bit); 
	virtual void BrushSetPixel(long x, long y, unsigned char *pattern, unsigned char *brush); 
	virtual void PatternSetPixel(long x, long y, unsigned char *pattern); 
/* --- added 6/14/92 --- */
	virtual void ImprintInset(long function);
	virtual void RemoveInset();
	virtual void ResizeInset();
	virtual void OverlayInset(char *insetname);


	
	static boolean InitializeClass();
 
			/* Create default Keymap & Menus */
	
	rasterview();
	
	virtual ~rasterview();



	boolean UpdateWanted;		/* set TRUE by WantUpdate */
	boolean InsetUpdateWanted;
	boolean needsFullUpdate;		/* TRUE is Update should use FullRedraw */
	boolean OnScreen;			/* if not view_Removed */
	boolean embedded;		/* TRUE if no call to GetApplicationLayer */
	short DraggingEdge;	/* Non-Zero when dragging an edge. */

	struct rectangle PixChanged;	/* minimal rectangle enclosing changes */

	/* these might differ between instances on different displays */
	class graphic  *BlackPattern;
	class graphic  *WhitePattern;
	class graphic  *GreyPattern;

	class menulist  *Menus;
	class keystate *Keystate;
	class cursor *Cursor[5];

	/* Subtract the following values from the coordinates of a point in the raster space to
	get the corresponding point in the logical view space. */
	long Xoff, Yoff;

	/* The following are the coordinates of the (left, top) bit to show on the screen in
	non-Expanded mode.  They are relative to the left upper corner of ViewSelection.. */
	long Xscroll, Yscroll;

	/* The next three rectangles have coordinates in the space of the entire raster they
	should also be subrectangles of dataobject->subraster */

	/* The rectangle which is the currently displayable subraster (used in cropping). */
	struct rectangle ViewSelection;

	/* the rectangle currently surrounded on screen (is Empty if none) */
	struct rectangle CurSelection;

	/* the rectangle that should be highlighted on screen */
	struct rectangle DesiredSelection;

	boolean HighlightIsGrey;		/* TRUE iff grey instead of b/w */
	boolean HasInputFocus;		/* TRUE iff we have the input focus */
	boolean IgnoreUntilNextUpTransition;	/* Just asked for Input Focus */
	long Xdown, Ydown;		/* coordinates of where mouse went down */

	short Mode;

	long TouchUpX, TouchUpY;	/* X, Y of last bit changed in touchup mode */
	struct rectangle SavedDesiredSelection;
	long StartPanX, StartPanY;	/* X, Y of starting to Pan */
	long PanX, PanY;

	boolean NeedsCentering;	/* For initial Centering */

	long Scale;		/* Current Scale factor */
	class rasterimage *Original;	/* Needed ?? */
	class rasterimage *Expansion;
	struct rectangle DisplayBox;	/* The location of the Display Box within visual rect*/
	long DBXscroll, DBYscroll;
	boolean MovingDisplayBox;
	boolean DisplayBoxHidden;
	struct rectangle DisplayBoxSelection;	/* Within the full image */
	boolean DisplayBoxOverlapped;
	struct point DisplayBoxAvoidancePath1[6];
	struct point DisplayBoxAvoidancePath2[4];
	struct point DisplayBoxAvoidancePath3[6];
	struct point DisplayBoxAvoidancePath4[4];
	short DisplayBoxPathLength1, DisplayBoxPathLength2;

	boolean CheckedParent;		/* have we set InMessages ? */
	boolean InMessages;		/* are we in messages ? */

	boolean Shrunken;			/* TRUE if image is shrunken */

	boolean ShowCoords;
	
	class view *inset;
	struct rectangle InsetBox;

	class rastoolview *toolset;	/* pointer to toolbox, or NULL */
};

#define	DRAW_REVERSE_PIXEL	(2)

#define	RegionSelectMode	0
#define	TouchUpMode		1
#define	PanMode			2
#define WaitMode		3
#define	ToolMode		4

#define RegionSelect(self) ((self)->Mode == RegionSelectMode)
#define TouchUp(self) ((self)->Mode == TouchUpMode)
#define Pan(self) ((self)->Mode == PanMode)
#define Tool(self) ((self)->Mode == ToolMode)

#define FullSize(self) ((self)->Scale == 1)
#define NotFullSize(self) ((self)->Scale != 1)

/* Uncropped(self) 
	tests to see if the rasterview is an uncropped version of the whole
	that is, the ViewSelection selects the entire rastimg.
*/
#define Cropped(self)							\
 ((class raster *)(self)->GetDataObject())->GetWidth() !=	\
     rectangle_Width(&self->ViewSelection)				\
 || ((class raster *)(self)->GetDataObject())->GetHeight() !=	\
	rectangle_Height(&self->ViewSelection)

#define Uncropped(self)							\
 ((class raster *)(self)->GetDataObject())->GetWidth() ==	\
     rectangle_Width(&self->ViewSelection)				\
 && ((class raster *)(self)->GetDataObject())->GetHeight() ==	\
	rectangle_Height(&self->ViewSelection)

/* FullySelected(self) 
	tests to see if the DesiredSelection selects the entire current ViewSelection.
*/
#define FullySelected(self)						\
 rectangle_IsEqualRect(&self->ViewSelection, &self->DesiredSelection)


#endif /* _rasterview_H_ */
