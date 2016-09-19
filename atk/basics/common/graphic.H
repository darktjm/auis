ATK_INTER
#ifndef _graphic_H_
#define _graphic_H_

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <point.h>
#include <rect.h>
#include <shadows.h>
#define graphic_PROGRAMMERVERSION	7

/* It would be nice if there were enums instead of defines, but
unfortunately, one cannot guarantee that enums can be easily or'd
(of course I really want sets of enums, but...) */

#define graphic_NOMOVEMENT		    0
#define graphic_ATLEFT			    1
#define graphic_ATRIGHT			    2
#define graphic_BETWEENLEFTANDRIGHT	    4
#define graphic_ATTOP			  010
#define graphic_ATBOTTOM		  020
#define graphic_ATBASELINE		  040
#define graphic_BETWEENTOPANDBOTTOM	 0100
#define graphic_BETWEENTOPANDBASELINE	 0200

/* LineDash */
#define graphic_LineSolid	0
#define graphic_LineDoubleDash	1
#define graphic_LineOnOffDash	2

/* LineCap */
#define graphic_CapNotLast	0
#define graphic_CapButt		1
#define graphic_CapRound	2
#define graphic_CapProjecting	3

/* LineJoin */
#define graphic_JoinMiter	0
#define graphic_JoinRound	1
#define graphic_JoinBevel	2

/* Display Classes */
#define	graphic_Monochrome	(1<<1)
#define	graphic_Color		(1<<2)
#define	graphic_PseudoColor	(1<<3)
#define	graphic_GrayScale	(1<<4)
#define	graphic_DirectColor	(1<<5)
#define	graphic_TrueColor	(1<<6)
#define	graphic_StaticColor	(1<<7)
#define	graphic_StaticGray	(1<<8)

#include <observable.H>
#include <color.H>

class ddimage;
class graphic : public  observable {
public:
	virtual ATKregistryEntry *ATKregistry();


    virtual void MoveToPt(struct point * Pt);
    virtual void MoveTo(long NewX, long NewY);
    virtual void Move(long DeltaX, long DeltaY);
    virtual void GetCurrentPt(struct point * Pt);

    virtual void DrawLineToPt(struct point * LineEnd);
    virtual void DrawLineTo(long XEnd, long YEnd);
    virtual void DrawLine(long DeltaX, long DeltaY);

    virtual void DrawString(const char * Text, short Operation);
    virtual void DrawText(const char * Text, long TextLength, short Operation);
    virtual void SetSpaceShim(short Amount);
    virtual short GetSpaceShim()  ;
    virtual void SetFont(class fontdesc * ChosenFont);
    virtual class fontdesc * GetFont()  ;

    virtual void DrawRect(struct rectangle * Rect);
    virtual void DrawRectSize(long x,long y,long width,long height);
    virtual void DrawPolygon(struct point * PointArray, short PointCount);
    virtual void DrawTrapezoid(long topX, long topY, long topWidth, 
		long bottomX, long bottomY, long bottomWidth);
    virtual void DrawPath(struct point * PointArray, short PointCount);
    virtual void DrawOval(struct rectangle * Rect);
    virtual void DrawOvalSize(long x,long y,long width,long height);
    virtual void DrawArc(struct rectangle * EnclRect, short StartAngle, short OffsetAngle);
    virtual void DrawArcSize(long x,long y,long width,long height, short StartAngle, short OffsetAngle);
    virtual void DrawRRect(struct rectangle * OuterBox, struct rectangle * InnerBox); 
    virtual void DrawRRectSize(long x,long y,long width,long height, long cornerWidth,long cornerHeight);
    virtual void DrawRgn(class region * Rgn);

    virtual void FillRect(struct rectangle * Rect, class graphic * Tile);
    virtual void FillRectSize(long x,long y,long width,long height, class graphic * Tile);
    virtual void FillTrapezoid(long topX, long topY, long topWidth, 
		long bottomX, long bottomY, long bottomWidth, class graphic * Tile);
    virtual void FillPolygon(struct point * PointArray, short PointCount, class graphic * Tile);
    virtual void FillOval(struct rectangle * Rect, class graphic * Tile);
    virtual void FillOvalSize(long x,long y,long width,long height, class graphic * Tile);
    virtual void FillArc(struct rectangle * EnclRect, short StartAngle, short OffsetAngle, class graphic * Tile);
    virtual void FillArcSize(long x,long y,long width,long height, 
		short StartAngle, short OffsetAngle, class graphic * Tile);
    virtual void FillRRect(struct rectangle * OuterBox, struct rectangle * InnerBox, class graphic * Tile);
    virtual void FillRRectSize(long x, long y, long width, long height, long cornerWidth, long cornerHeight, class graphic * Tile);
    virtual void FillRgn(class region * Rgn, class graphic * Tile);

    virtual void EraseRect(struct rectangle *Rect);
    virtual void EraseRectSize(long x,long y,long width,long height);
    virtual void EraseVisualRect();

    virtual void BitBlt(struct rectangle *SrcRect, 
		class graphic *DestGraphic, struct point * DstOrigin, 
		struct rectangle *ClipRectList);
    virtual void BitBltSize(long srcX, long srcY, 
		long dstX, long dstY, long Width, long Height, 
		class graphic *DstGraphic, 
		long clipX, long clipY, long clipWidth, long clipHeight);
    virtual void SetBitAtLoc(long XPos, long YPos, boolean NewValue);

    virtual void WritePixImage(long DestX, long DestY, 
		class pixelimage *SrcPixels, long SrcX, long SrcY, 
		long width, long height);
    virtual void ReadPixImage(long SrcX, long SrcY, 
		class pixelimage *DestPixels, long DestX, long DestY, 
		long width, long height);

    virtual void InsertGraphic(class graphic * EnclosingGraphic, struct rectangle * EnclosedRectangle);
    virtual void InsertGraphicSize(class graphic * EnclosingGraphic, 
		long xOriginInParent, long yOriginInParent, long width, long height);
    virtual void InsertGraphicRegion(class graphic *EnclosingGraphic, class region *regions);

    virtual void SetVisualRegion(class region *region);
    virtual class region * GetVisualRegion(class region *retRegion)  ;

    virtual void MoveLogicalOrigin(long DeltaX, long DeltaY);
    virtual void SetLogicalOrigin(long NewX, long NewY);
    virtual void SetLogicalOriginPt(struct point * Pt);
    virtual void GetLogicalBounds(struct rectangle * Rect);
    virtual long GetLogicalRight()  ;
    virtual long GetLogicalBottom()  ;

    virtual void GetEnclosedBounds(struct rectangle * Rect);
    virtual long GetEnclosedRight()  ;
    virtual long GetEnclosedBottom()  ;

    virtual void GetVisualBounds(struct rectangle * Rect);
    virtual long GetVisualRight()  ;
    virtual long GetVisualBottom()  ;

    virtual void SetClippingRegion(class region *region);
    virtual class region * GetClippingRegion(class region *retRegion)  ;

    virtual void SetClippingRect(struct rectangle * AdditionalRect);
    virtual void SetClippingRectSize(long x, long y, long w, long h);
    virtual void ClearClippingRect();
    virtual void GetClippingRect(struct rectangle * Rect);

    virtual void SetLineWidth(short NewLineWidth);
    virtual short GetLineWidth()  ;

    virtual void SetLineDash( const char *dashPattern, int dashOffset, short dashType );
    virtual void GetLineDash( char **dashPattern, int *dashOffset, short *dashType );

    virtual void SetLineCap( short NewLineCap );
    virtual short GetLineCap()  ;

    virtual void SetLineJoin( short NewLineJoin );
    virtual short GetLineJoin()  ;

    virtual void SetTransferMode(short NewTransferMode);
    virtual short GetTransferMode()  ;

    virtual void RestoreGraphicsState();
    virtual void FlushGraphics();

    virtual class graphic * WhitePattern()  ;
    virtual class graphic * BlackPattern()  ;
    virtual class graphic * GrayPattern(short IntensityNum, short IntensityDenom)  ;
    virtual void SetPatternOrigin(long xpos, long ypos);
    virtual void GetPatternOrigin(long *xpos, long *ypos);

    /* If colorName is NULL, use the RGB. */
    virtual void SetForegroundColor(const char *colorName, long red, long green, long blue);
    virtual void GetForegroundColor(const char **colorName, long *red, long *green, long *blue);
    virtual void SetBackgroundColor(const char *colorName, long red, long green, long blue);
    virtual void GetBackgroundColor(const char **colorName, long *red, long *green, long *blue);
    virtual void SetForegroundColor(color *c);
    virtual void SetBackgroundColor(color *c);

    void SetFGColorCell(color *c);
    void SetBGColorCell(color *c);
    
    virtual void SetFGColor( double red, double green, double blue );
    virtual void GetFGColor( double *red, double *green, double *blue );
    virtual void SetBGColor( double red, double green, double blue );
    virtual void GetBGColor( double *red, double *green, double *blue );

    virtual long GetHorizontalResolution()  ;
    virtual long GetVerticalResolution()  ;
    virtual const char * GetWindowManagerType()  ;
    virtual long GetDevice()  ;
    virtual long DisplayClass()  ;
    virtual boolean IsImageInverted()  ;
    virtual void WriteImage(long DestX, long DestY, 
		class image *image, long SrcX, long SrcY, 
		long width, long height);
    virtual void ReadImage(long SrcX, long SrcY, 
		class image *DestImage, long DestX, long DestY, 
		long width, long height);
    virtual void WriteImage(long DestX, long DestY, 
		ddimage &image, long SrcX, long SrcY, 
		long width, long height);
    virtual void SetFGToShadow(int shadow);  /* see installed shadows.h */
    


    inline long GetLogicalWidth() { return ((this)->localBounds.width); }
    inline long GetLogicalHeight() { return ((this)->localBounds.height); }
    inline long GetLogicalLeft() { return ((this)->localBounds.left); }
    inline long GetLogicalTop() { return ((this)->localBounds.top); }

    inline long GetVisualWidth() { return ((this)->visualBounds.width); }
    inline long GetVisualHeight() { return ((this)->visualBounds.height); }
    inline long GetVisualLeft() { return ((this)->visualBounds.left); }
    inline long GetVisualTop() { return ((this)->visualBounds.top); }

    inline long GetEnclosedWidth() { return ((this)->localBounds.width); }
    inline long GetEnclosedHeight() { return ((this)->localBounds.height); }
    inline long GetEnclosedLeft() { return (point_X(&((this)->enclosedOrigin))); }
    inline long GetEnclosedTop() { return (point_Y(&((this)->enclosedOrigin))); }

    inline class colormap  ** GetColormap() { return ((this)->colormap); }
    inline void SetColormap(class colormap **cmap) { ((this)->colormap = (cmap)); }
    inline class colormap **GetInheritedColormap() { return ((this)->inheritedColormap ? (this)->inheritedColormap : NULL); }
    inline void SetInheritedColormap(class colormap **cmap) { ((this)->inheritedColormap = (cmap)); }
    inline void ClearInheritedColormap() { ((this)->inheritedColormap = NULL); }
    inline class colormap  ** CurrentColormap() { return ((this->colormap && *(this->colormap)) ? this->colormap : this->inheritedColormap); }

    inline color *GetForegroundColor() {
	return fore;
    }
    inline color *GetBackgroundColor() {
	return back;
    }

    static void SetDefaultColors(const char *foreground, const char *background);
    static void GetDefaultColors(const char **foreground, const char **background);
    static class graphic * CreateGraphic(class view *v)  ;
    
	virtual ~graphic();
    
	graphic();
    static void ComputeShadow(long  br , long  bg , long  bb, long  *rr , long  *rg , long  *rb, int  shadow);
    static void ComputeShadowDouble(double br, double bg, double bb, double *rr, double *rg, double *rb, int shadow);
    
 
    struct rectangle localBounds;    /* What is size of rectangle */
    struct rectangle visualBounds;   /* How much of rect is seen */
    class region *localRegion;
    class region *visualRegion;
    struct point savedOrigin;	     /* Logical offsets of origin */
				     /* Saved across insertions */
    struct point enclosedOrigin;     /* local rect in parent's coord*/
    class fontdesc * currentFont;   /* Current font for text drawing */
    short  spaceShim;
    short  transferMode;	    /* ALU function for bitblit */
    short  lineWidth;		    /* How thick to make outlines */
    short  lineDashType;	    /* Dash style for lines */
    int	   lineDashOffset;	    /* Dash offset (phase) for lines */
    char   *lineDashPattern;	    /* Dash pattern for lines */
    short  lineCap;		    /* Cap style for lines */
    short  lineJoin;		    /* Join style for lines */
    class region * clippingRegion; /* Rects for clipping */
    struct point    currentPoint;	    /* Start of drawing operations */
    class colormap **colormap,**inheritedColormap;
/* private data */
    struct point physicalOrigin;
    struct point patternOrigin;
    icolormap colors;
    color *fore, *back;
};


/* Transfer function bit encoding. One specifies the transfer function
   by giving a table source and destination bits, encoded in a hex digit.
   The binary interpretation is that 1 is black and 0 white.
   The table encodes bits as follows:
	Bit 3   Bit 2   Bit 1   Bit 0
Source	    1(B)    1(B)    0(W)    0(W)
Dest	    1(B)    0(W)    1(B)    0(W)
Result	    ?	    ?	    ?	    ?
Thus, Bit 3 specifies what the result should be if both the source and
destination are 1; bit 2 specifies the result if the source is 1 and the
destination 0; bit 1, when the source is 0 and the destination 1; and
bit 0 when both the source and destination are 0. For example, to
generate the AND function, bit 3 would be one and bits 2, 1 and 0 would
be 0, so the resulting hex digit would be 0x4. If we want the source to
be copied directly without regard to the current value of the 
destination, bits 3 and 2 would be 1 and bits 1 and 0 would be 0,giving
the hex digit 0xc.

Because we believe that we might expand our transfer function to
three inputs (using a mask as a third input), we now require the transfer
function be two hexadecimal digits -- the one generated as above 
repeated. Thus, the value for AND would be 0x44 and for copying the
source, 0xcc.

To help construct the desired function, two constants graphic_SOURCE
and graphic_DEST have been defined that allow a programmer to specify
the desired effects without having to do the bit by bit calculations.
Instead, one can just perform the logical operations directly on
the constants. For example, the AND operation can be expressed as
(graphic_SOURCE & graphic_DEST). Some common combinations are provided
as well.
*/

/* Predefined constants for use in transfer modes */
#define graphic_SOURCE	0xCC
#define graphic_DEST	0xAA

/* Some synonyms for ease of use */
#define graphic_COPY	graphic_SOURCE
#define graphic_OR	(graphic_SOURCE | graphic_DEST)
#define graphic_XOR	(graphic_SOURCE ^ graphic_DEST)
#define graphic_AND	(graphic_SOURCE & graphic_DEST)
#define graphic_WHITE	0x00
#define graphic_BLACK	0xFF
#define graphic_INVERT	((~graphic_DEST)&0xFF)

/* Some notes on the structure:
    The attached graphic provides some bits to write on. Nothing is
assumed initially about the relationship between bounds in a graphic
and graphic bounds. A graphic is associated witha graphic in one of two
ways. First, a graphic can be directly laid on top of
graphic (Setgraphic), in which case the call explicitly links the portion 
of graphic to the graphic. Second, a graphic can be inserted into
another graphic, in which case the graphic information is gleaned
relative to the parent. Either case is very system dependent. One 
assumes that only im will do the former, and that everyone else will
do the latter which will change with each machine, but the change
should be invisible to the user.

The local bounds specify, in local graphic coordinates, what the
coorindate system is for this graphic. Initially this is 0,0 in upper
left corner, though it can be changed by moving the origin explicitly.
The visual rectangle shows
which part of the local bounds is actually displayed on the screen
(also in local coordinates). The enclosedRect is the same as the local
bounds rectangle except that it is expressed in the parent's coordinate
space (if any exists). Like the physical rectangle of the graphic, the
width and height information are redundant with the width and height of
the localbounds.

 If any mapping between local coordinates
and underlying window manager coordinates is necessary, the window
manager system will take care of it (state will be part of graphic).

graphic will contain the window manager depedent information, such as 
window id number. The fontdesc is the font descriptor to be used for
diplaying text. When a descriptior was created, a size, famliy and
style was specified, this to change to bold or larger font, a 
new descirptor must be set.

The transfer mode explains how bits will be pasted on the screen when
drawing lines, writing text, blitting bits or filling figures. 
Essentially the same defintion as for wm is used, except the tiling mask
is available only for bitblitting.

The line width is self-explanatory. The current point is used to 
determine where to draw lines or text. The clipping rectangle is a user
controllable way to limit drawing commands. It is in local coordinates,
but is usually null (no explicit clipping).

*/



#endif /* _graphic_H_ */
