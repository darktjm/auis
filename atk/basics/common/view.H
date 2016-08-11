ATK_INTER
#ifndef _view_H_
#define _view_H_
/* ********************************************************************** *\
 * 	Copyright IBM Corporation 1988,1991 - All Rights Reserved 
 * 	Copyright Carnegie Mellon University 1992, 1996 - All rights reserved
 * 	For full copyright information see:'andrew/config/COPYRITE'
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


#define view_VERSION 1

enum view_UpdateType  {
    view_FullRedraw,
    view_PartialRedraw,
    view_LastPartialRedraw,
    view_MoveNoRedraw,
    view_Remove
};

enum view_MouseAction  {
    view_NoMouseEvent,
    view_LeftDown,
    view_LeftUp,
    view_LeftMovement,
    view_RightDown,
    view_RightUp,
    view_RightMovement,
    view_UpMovement,	/* ! 6/96 -wjh */
    /* File drop actions are only available for DRAGDROP_ENV */
    view_LeftFileDrop,
    view_MiddleFileDrop,
    view_RightFileDrop
};

enum view_DSpass {
    view_NoSet,
    view_WidthSet,
    view_HeightSet
};

//enum view_Traversal {
//    view_NextView,
//    view_PrevView,
//    view_NextGroup,
//    view_PrevGroup
//};


typedef int view_DSattributes;

#define view_Fixed 0
#define view_WidthLarger 1
#define view_WidthSmaller 2
#define view_WidthFlexible 3
#define view_HeightLarger 4
#define view_HeightSmaller 8
#define view_HeightFlexible 12

#define view_NOMOVEMENT			    0
#define view_ATLEFT			    1
#define view_ATRIGHT			    2
#define view_BETWEENLEFTANDRIGHT	    4
#define view_ATTOP			  010
#define view_ATBOTTOM			  020
#define view_ATBASELINE			  040
#define view_BETWEENTOPANDBOTTOM	 0100
#define view_BETWEENTOPANDBASELINE	 0200

///* Error codes for the description procedure */
//enum view_DescriberErrs {
//    view_NoDescribeError,
//    view_NoDescriptionAvailable,
//    view_NoDescribeString,
//    view_NoDescribeObject,
//    view_WrongSubclassDescription
//};

#include <atom.H>
#include <graphic.H>
#include <observable.H>
#include <color.H>

struct view_printopt {
    atom *name, *type;
    char *label;
};
struct view_printoptlist {
    struct view_printopt *list; /* array of structures */
    int numoptions; /* size of list */
    struct view_printoptlist *parent; 
};
// Sigh, ScrollInterface isn't actually defined until supportviews...
class ScrollInterface;

class view : public  observable {
public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();
	virtual ~view();
	view();

    // override
    void ObservedChanged (observable *changed, long value);

    // view methods.  Override-able, but not usually overridden.
    virtual void SetName( atomlist *name );
    virtual atomlist *GetName();
    virtual atomlist *GetClass();

    virtual void InsertViewRegion(view *parent, region *region);
    virtual void InsertView(view *parent, struct rectangle *enclosingRect);
    virtual void InsertViewSize(view *EnclosingView, 
		long xOriginInParent, long yOriginInParent, long w, long h);
    virtual void SetDataObject(class dataobject *dobj);
    inline class dataobject *GetDataObject() { return ((this)->dataobject); }

    // view methods that should be overridden
    virtual void FullUpdate(enum view_UpdateType type, 
			long l, long t, long w, long h);
    virtual void Update();
    virtual void Print(FILE *file, char *proc, char *finalForm, boolean top);
    virtual view *Hit (enum view_MouseAction action, long x, long y, 
			long numberOfClicks);
    virtual view_DSattributes DesiredSize(long w, long h, 
			enum view_DSpass pass, long *dWidth, long *dheight);

// view methods that can be overridden
// input focus 
    virtual void ReceiveInputFocus();
    virtual void LoseInputFocus();
    virtual boolean AcceptingFocus();
    virtual void ChildLosingInputFocus();
    virtual void ChildReceivingInputFocus();

// application layer (what to do if alone in a window)
    virtual view *GetApplicationLayer();
    virtual void DeleteApplicationLayer(view *applicationLayer);

    virtual void GetOrigin(long w, long h, long *originX, long *originY);
    virtual void GetPrintOrigin(long w, long h, long *originX, long *originY);

// color
    virtual void ReceiveColormap(colormap *cmap);
    virtual void LoseColormap(colormap *cmap);
    virtual void WantColormap(view *requestor, colormap **cmap);

    virtual void WantUpdate(view *requestor);
    virtual void WantInputFocus(view *requestor);
    virtual void WantNewSize(view *requestor);
    virtual void PostKeyState(class keystate *keystate);
    virtual void PostMenus(class menulist *menulist);

    virtual void PostCursor(struct rectangle *rec, class cursor *cursor);
    virtual void RetractCursor(class cursor *cursor);
    virtual void RetractViewCursors(view *requestor);

    virtual ATK  *WantHandler(char *handlerName);
    virtual void PostDefaultHandler(char *handlerName, ATK  *handler);
    virtual char *GetInterface(char *type);
    virtual char *WantInformation(char *key);


// View linking/unlinking functions
    virtual void LinkTree(view *parent);
    virtual void UnlinkTree();
    virtual boolean IsAncestor(view *possibleAncestor);
    virtual void UnlinkNotification(view *unlinkedTree);
    virtual void ExposeSelf(boolean recurse);
    virtual void ExposeChild(view *v);

    virtual char *GetWindowManagerType();
    virtual long GetDevice();
    virtual void InitChildren();
    virtual boolean CanView(char *TypeName);

// Support for selections ala X 
    virtual void LoseSelectionOwnership();
    virtual long WriteSelection(FILE *out);

// support for recursive search
    virtual boolean RecSearch(struct SearchPattern *pat, boolean toplevel);
    virtual boolean RecSrchResume(struct SearchPattern *pat);
    virtual boolean RecSrchReplace(class dataobject *text, long pos, long len);
    virtual void RecSrchExpose(const struct rectangle &logical, 
			struct rectangle &hit);
    
// Postscript printing
    virtual void *GetPSPrintInterface(char *printtype);
    virtual void DesiredPrintSize(long w, long h, enum view_DSpass pass, 
			long *desiredwidth, long *desiredheight);
    virtual void PrintPSDoc(FILE *outfile, long pagew, long pageh);
    virtual void PrintPSRect(FILE *outfile, long logwidth, long logheight, 
			struct rectangle *visrect);
    virtual struct view_printoptlist *PrintOptions();
    virtual long GetPrintOption(atom *popt);
    virtual void SetPrintOption(struct view_printopt *vopt, long value);

    virtual boolean Gifify(char *filename, long *pmaxw, long *pmaxh, 
			struct rectangle *visrect);
	/* put a gifified version of the view into the named file
		The pmax args incoming values are the maximum gif size;
		they are set to the actual gif size by the function.
		Visrect, if non-NULL, specifies the subrectangle to gifify 
		Return FALSE for failure. */

    //    virtual void Traverse(enum view_Traversal trav);
    virtual void WantExposure(view *requestor, struct rectangle *childrect);

    // GetScrollInterface will create an appropriate object of
    // a class derived from ScrollInterface.  The caller is responsible
    // for destroying the interface.  NULL is returned if there
    // is no appropriate interface.
    virtual ScrollInterface *GetScrollInterface();

    // resources and parameters
    virtual short GetParameter(atomlist *name, atom *type, long *datum );
    virtual short GetResource(atomlist *name, atomlist *class_c,
		atom *type, long *datum );
    virtual void GetManyParameters(struct resourceList *resources,
    atomlist *name, atomlist *class_c );		      
    virtual void PostResource(atomlist *path, atom *type, long datum );


	// The following avoid multiple inheritance
	// Each method of graphic is made a method of view

#define DRAW(x) (drawable->x)

    inline void MoveToPt(struct point *Pt) { DRAW(MoveToPt((Pt))); }
    inline void MoveTo(long NewX,long NewY) { DRAW(MoveTo((NewX),(NewY))); }
    inline void Move(long DX, long DY) { DRAW(Move((DX),(DY))); }

    inline void GetLogicalBounds(struct rectangle *Rect) 
		{ DRAW(GetLogicalBounds((Rect))); }
    inline long GetLogicalRight() { return DRAW(GetLogicalRight()); }
    inline long GetLogicalBottom() { return DRAW(GetLogicalBottom()); }
    inline long GetLogicalLeft() { return DRAW(GetLogicalLeft()); }
    inline long GetLogicalHeight() { return DRAW(GetLogicalHeight()); }
    inline long GetLogicalWidth() { return DRAW(GetLogicalWidth()); }
    inline long GetLogicalTop() { return DRAW(GetLogicalTop()); }

    inline void SetVisualRegion(region *region) 
		{ DRAW(SetVisualRegion((region))); }
    inline region *GetVisualRegion(region *region) 
		{ return DRAW(GetVisualRegion((region))); }
    inline void GetVisualBounds(struct rectangle *Rect) 
		{ DRAW(GetVisualBounds((Rect))); }
    inline long GetVisualRight() { return DRAW(GetVisualRight()); }
    inline long GetVisualBottom() { return DRAW(GetVisualBottom()); }
    inline long GetVisualWidth() { return DRAW(GetVisualWidth()); }
    inline long GetVisualHeight() { return DRAW(GetVisualHeight()); }
    inline long GetVisualLeft() { return DRAW(GetVisualLeft()); }
    inline long GetVisualTop() { return DRAW(GetVisualTop()); }

    inline void GetEnclosedBounds(struct rectangle *Rect)
		{ DRAW(GetEnclosedBounds(Rect)); }
    inline long GetEnclosedRight() { return DRAW(GetEnclosedRight()); }
    inline long GetEnclosedBottom() { return DRAW(GetEnclosedBottom()); }
    inline long GetEnclosedLeft() { return DRAW(GetEnclosedLeft()); }
    inline long GetEnclosedTop() { return DRAW(GetEnclosedTop()); }
    inline long GetEnclosedWidth() { return DRAW(GetEnclosedWidth()); }
    inline long GetEnclosedHeight() { return DRAW(GetEnclosedHeight()); }

    inline void GetCurrentPt(struct point *Pt) { DRAW(GetCurrentPt((Pt))); }

    inline void DrawLineToPt(struct point *LineEnd)
		{ DRAW(DrawLineToPt((LineEnd))); }
    inline void DrawLineTo(long XEnd, long YEnd)
		{ DRAW(DrawLineTo((XEnd), (YEnd))); }
    inline void DrawLine(long DeltaX, long DeltaY)
		{ DRAW(DrawLine((DeltaX), (DeltaY))); }

    inline void DrawString(char *Text, short Operation)
		{ DRAW(DrawString((Text), (Operation))); }
    inline void DrawText(char *Text, long TextLength, short Operation)
		{ DRAW(DrawText((Text), (TextLength), (Operation))); }
    inline void SetSpaceShim(short Amount) { DRAW(SetSpaceShim((Amount))); }
    inline short GetSpaceShim() { return DRAW(GetSpaceShim()); }
    inline void SetFont(fontdesc *ChosenFont)
		{ DRAW(SetFont((ChosenFont))); }
    inline fontdesc *GetFont() { return DRAW(GetFont()); }

    inline void DrawRect(struct rectangle *Rect) { DRAW(DrawRect((Rect))); }
    inline void DrawRectSize(long x,long y,long w,long h)
		{ DRAW(DrawRectSize((x),(y),(w),(h))); }
    inline void DrawTrapezoid(long topX, long topY, long topW, 
			long botX, long botY, long botW)
		{ DRAW(DrawTrapezoid((topX),(topY),(topW),
			(botX),(botY),(botW))); }
    inline void DrawPolygon(struct point *PointArray, short PointCount)
		{ DRAW(DrawPolygon((PointArray), (PointCount))); }
    inline void DrawPath(struct point *PointArray, short PointCount)
		{ DRAW(DrawPath((PointArray), (PointCount))); }
    inline void DrawOval(struct rectangle *Rect) { DRAW(DrawOval((Rect))); }
    inline void DrawOvalSize(long x,long y,long w,long h)
		{ DRAW(DrawOvalSize((x), (y), (w), (h))); }
    inline void DrawArc(struct rectangle *EnclRect, 
			short StartAngle, short OffsetAngle)
		{ DRAW(DrawArc((EnclRect),(StartAngle),(OffsetAngle))); }
    inline void DrawArcSize(long x,long y,long w,long h, 
			short StartAngle, short OffsetAngle)
		{ DRAW(DrawArcSize((x),(y),(w),(h),
			(StartAngle),(OffsetAngle))); }
    inline void DrawRRect(struct rectangle *Outer, struct rectangle *Inner)
		{ DRAW(DrawRRect((Outer), (Inner))); }
    inline void DrawRRectSize(long x,long y,long w,long h, 
			long cornerWidth,long cornerHeight)
		{ DRAW(DrawRRectSize((x), (y), (w), (h), 
			(cornerWidth), (cornerHeight))); }
    inline void DrawRgn(region *Rgn)
		{ DRAW(DrawRgn((Rgn))); }

    inline void FillRect(struct rectangle *Rect, graphic *Tile)
		{ DRAW(FillRect((Rect),(Tile))); }
    inline void FillRectSize(long x,long y,long w,long h, graphic *Tile)
		{ DRAW(FillRectSize((x), (y), (w), (h), (Tile))); }
    inline void FillTrapezoid(long topX, long topY, long topW, 
			long botX, long botY, long botW, graphic *Tile)
		{ DRAW(FillTrapezoid((topX), (topY), (topW), 
			(botX), (botY), (botW), (Tile))); }
    inline void FillPolygon(struct point *PtArray, short PtCount, 
			graphic *Tile)
		{ DRAW(FillPolygon((PtArray), (PtCount), (Tile))); }
    inline void FillOval(struct rectangle *Rect, graphic *Tile)
		{ DRAW(FillOval((Rect), (Tile))); }
    inline void FillOvalSize(long x,long y,long w,long h, graphic *Tile)
		{ DRAW(FillOvalSize((x), (y), (w), (h), (Tile))); }
    inline void FillArc(struct rectangle *EnclRect, 
			short StartAngle, short OffsetAngle, graphic *Tile)
		{ DRAW(FillArc((EnclRect), (StartAngle), (OffsetAngle),(Tile))); }
    inline void FillArcSize(long x,long y,long w,long h, 
			short StartAngle, short OffsetAngle, graphic *Tile)
		{ DRAW(FillArcSize((x), (y), (w), (h), 
			(StartAngle), (OffsetAngle), (Tile))); }
    inline void FillRRect(struct rectangle *Outer, struct rectangle *Inner, 
			graphic *Tile)
		{ DRAW(FillRRect((Outer), (Inner), (Tile))); }
    inline void FillRRectSize(long x, long y, long w, long h, 
			long cornerW, long cornerHeight, graphic *Tile)
		{ DRAW(FillRRectSize((x), (y), (w), (h), 
			(cornerW), (cornerHeight), (Tile))); }
    inline void FillRgn(region *Rgn, graphic *Tile)
		{ DRAW(FillRgn((Rgn), (Tile))); }

    inline void EraseRect(struct rectangle *Rect) { DRAW(EraseRect((Rect))); }
    inline void EraseRectSize(long x,long y,long w,long h)
		{ DRAW(EraseRectSize((x), (y), (w), (h))); }
    inline void EraseVisualRect() { DRAW(EraseVisualRect()); }

    inline void BitBlt(struct rectangle *SrcRect, view *DestView, 
		struct point *DstOrigin, struct rectangle *ClipRectList)
		{ DRAW(BitBlt((SrcRect), (((DestView))->drawable), 
			(DstOrigin), (ClipRectList))); }
    inline void BitBltSize(long srcX, long srcY, long dstX, long dstY, 
			long W, long H, view *DstView, 
			long clipX, long clipY, long clipW, long clipH)
		{ DRAW(BitBltSize((srcX), (srcY), (dstX), (dstY), (W),(H), 
			((DstView)->drawable), 
			(clipX),(clipY), (clipW), (clipH))); }
    inline void SetBitAtLoc(long XPos, long YPos, boolean NewValue)
		{ DRAW(SetBitAtLoc((XPos), (YPos), (NewValue))); }

    inline void WritePixImage(long DestX, long DestY, pixelimage *SrcPixels, 
				long SrcX, long SrcY, long w, long h) 
		{ DRAW(WritePixImage(DestX,DestY, SrcPixels, SrcX,SrcY, w,h));}
    inline void ReadPixImage(long SrcX, long SrcY, pixelimage *DestPixels, 
				long DestX, long DestY, long w, long h)
		{ DRAW(ReadPixImage(SrcX,SrcY, DestPixels, DestX,DestY, w,h));}
    inline void ReadImage(long SrcX, long SrcY, image *DestImage, 
				long DestX, long DestY, long w, long h)
		{ DRAW(ReadImage(SrcX, SrcY, DestImage, DestX, DestY, w, h)); }
    inline void WriteImage(long DestX, long DestY, ddimage &SrcImage, 
				long SrcX, long SrcY, long w, long h) 
		{ DRAW(WriteImage(DestX, DestY, SrcImage, SrcX, SrcY, w, h)); }

    inline void MoveLogicalOrigin(long DeltaX, long DeltaY)
		{ DRAW(MoveLogicalOrigin((DeltaX), (DeltaY))); }
    inline void SetLogicalOrigin(long NewX, long NewY)
		{ DRAW(SetLogicalOrigin((NewX), (NewY))); }
    inline void SetLogicalOriginPt(struct point *Pt)
		{ DRAW(SetLogicalOriginPt((Pt))); }


    inline void SetClippingRegion(region *region)
		{ DRAW(SetClippingRegion((region))); }
    inline region *GetClippingRegion(/*MAYBE MISSING */ region *region)
		{ return DRAW(GetClippingRegion((region))); }
    inline void SetClippingRect(/*MAYBE MISSING */ struct rectangle *ClipRect)
		{ DRAW(SetClippingRect((ClipRect))); }
    inline void SetClippingRectSize(long x, long y, long w, long h)
		{ DRAW(SetClippingRectSize((x),(y),(w),(h))); }
    inline void ClearClippingRect() { DRAW(ClearClippingRect()); }
    inline void GetClippingRect(struct rectangle *Rect)
		{ DRAW(GetClippingRect((Rect))); }

    inline void SetLineWidth(short NewLineW) { DRAW(SetLineWidth((NewLineW)));}
    inline short GetLineWidth() { return DRAW(GetLineWidth()); }

    inline void SetLineDash(char *dashPattern, int dashOffset, short dashType)
		{ DRAW(SetLineDash((dashPattern), (dashOffset), (dashType))); }
    inline void GetLineDash(char **dashPat, int *dashOffset, short *dashType)
		{ DRAW(GetLineDash((dashPat), (dashOffset), (dashType))); }

    inline void SetLineCap(short NewLineCap) { DRAW(SetLineCap((NewLineCap)));}
    inline short GetLineCap() { return DRAW(GetLineCap()); }

    inline void SetLineJoin(short NewLineJoin)
		{ DRAW(SetLineJoin((NewLineJoin))); }
    inline short GetLineJoin() { return DRAW(GetLineJoin()); }

    inline void SetTransferMode(short mode){ DRAW(SetTransferMode(mode)); }
    inline short GetTransferMode() { return DRAW(GetTransferMode()); }

    inline void FlushGraphics() { DRAW(FlushGraphics()); }

    inline graphic *WhitePattern() { return DRAW(WhitePattern()); }
    inline graphic *BlackPattern() { return DRAW(BlackPattern()); }
    inline graphic *GrayPattern(short IntensityNum, short IntensityDenom)
		{ return DRAW(GrayPattern((IntensityNum), (IntensityDenom))); }
    inline void SetPatternOrigin(long xpos, long ypos) 
		{ DRAW(SetPatternOrigin((xpos), (ypos))); }
    inline void GetPatternOrigin(long *xpos, long *ypos)
		{ DRAW(GetPatternOrigin((xpos), (ypos))); }
    inline void SetForegroundColor(color *c) { DRAW(SetForegroundColor(c)); }
    inline void SetBackgroundColor(color *c) { DRAW(SetBackgroundColor(c)); }
    inline void SetForegroundColor(char *colorName, long r, long g, long b)
		{ DRAW(SetForegroundColor((colorName), (r), (g), (b))); }
    inline void GetForegroundColor(char **name, long*retr,long*retg,long*retb)
		{ DRAW(GetForegroundColor((name), (retr), (retg), (retb))); }
    inline void SetBackgroundColor(char *colorName, long r, long g, long b)
		{ DRAW(SetBackgroundColor((colorName), (r),(g),(b))); }
    inline void GetBackgroundColor(char **name, long*retr,long*retg,long*retb)
		{ DRAW(GetBackgroundColor((name), (retr), (retg), (retb))); }

    inline void SetFGColorCell(color *color)
		{  DRAW(SetFGColorCell(color)); }
    inline void SetBGColorCell(color *color)
		{  DRAW(SetBGColorCell(color)); }

    inline void SetFGColor(double red, double green, double blue)
		{ DRAW(SetFGColor((red),(green),(blue))); }
    inline void GetFGColor(double *red, double *green, double *blue)
		{ DRAW(GetFGColor((red),(green),(blue))); }
    inline void SetBGColor(double red, double green, double blue)
		{ DRAW(SetBGColor((red),(green),(blue))); }
    inline void GetBGColor(double *red, double *green, double *blue)
		{ DRAW(GetBGColor((red),(green),(blue))); }

    inline long GetHorizontalResolution()
		{ return DRAW(GetHorizontalResolution()); }
    inline long GetVerticalResolution()
		{ return DRAW(GetVerticalResolution()); }

    inline class im *GetIM() { return imPtr; }
    inline class graphic *GetDrawable() { return drawable; }

    inline colormap **GetColormap() { return DRAW(GetColormap()); }
    inline void SetColormap(colormap **cmap) {DRAW(SetColormap(cmap));}
    inline colormap **GetInheritedColormap()
		{ return DRAW(GetInheritedColormap()); }
    inline void SetInheritedColormap(colormap **cmap) 
		{ DRAW(SetInheritedColormap(cmap)); }
    inline void ClearInheritedColormap()
		{  DRAW(ClearInheritedColormap()); }
    inline colormap **CurrentColormap()
		{ return DRAW(CurrentColormap()); }

    inline long  EnclosedXToLocalX(long xvalue)  
		{return (xvalue)-DRAW(GetEnclosedLeft())+DRAW(GetLogicalLeft());}
    inline long  EnclosedYToLocalY(long yvalue)   
		{return (yvalue)-DRAW(GetEnclosedTop())+DRAW(GetLogicalTop());}

    inline long DisplayClass() { return DRAW(DisplayClass()); }

#undef DRAW


    atomlist *name;
    atomlist *className;
    short  name_explicitly_set;
    im *imPtr;
    view *parent;
    class dataobject *dataobject;
    graphic *drawable;
};

 
//  virtual char *DescriptionObject(char *format, long rock);
//  virtual enum view_DescriberErrs Describe(char*format,FILE*file,long rock);


#endif /* _view_H_ */


