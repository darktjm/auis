#ifndef _xgraphic_H_
#define _xgraphic_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <fontdesc.H>
#include <point.h>
#include <rect.h>
#include <region.H>

/** \addtogroup libbasics
 * @{ */
#define xgraphic_PROGRAMMERVERSION	1
#include <image.H>

#include <graphic.H>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
typedef struct {
  Display  *disp;       /* destination display */
  int       scrn;       /* destination screen */
  int       depth;      /* depth of drawable we want/have */
  Drawable  drawable;   /* drawable to send image to */
  Pixel     foreground; /* foreground and background pixels for mono images */
  Pixel     background;
  Colormap  cmap;       /* colormap used for image */
  GC        gc;         /* cached gc for sending image */
  XImage   *ximage;     /* ximage structure */
} XImageInfo;

class ddimage;

struct xgraphic_UpdateBlock;

class xgraphic : public  graphic {
public:
	TRACED_CLASS(xgraphic);


    virtual void DrawLineTo(long XEnd, long YEnd);
    virtual void DrawLine(long DeltaX, long DeltaY);

    virtual void DrawString(const char *Text, short Operation);
    virtual void DrawText(const char *Text, long TextLength, short Operation);

    virtual void SetFont(class fontdesc *ChosenFont);

    virtual void DrawRectSize(long x,long y,long width,long height);
    virtual void DrawPolygon(const struct point *PointArray, short PointCount);
    virtual void DrawPath(const struct point *PointArray, short PointCount);
    virtual void DrawOvalSize(long x,long y,long width,long height);
    virtual void DrawArcSize(long x,long y,long width,long height, short StartAngle, short OffsetAngle);
    virtual void DrawRRectSize(long x,long y,long width,long height, long cornerWidth,long cornerHeight);
    virtual void DrawTrapezoid(long topX, long topY, long topWidth, long bottomX, long bottomY, long bottomWidth);


    virtual void FillRectSize(long x,long y,long width,long height, const class graphic *Tile);
    virtual void FillPolygon(const struct point * PointArray, short PointCount, const class graphic *Tile);
    virtual void FillOvalSize(long x,long y,long width,long height, const class graphic *Tile);
    virtual void FillArcSize(long x,long y,long width,long height, short StartAngle, short OffsetAndle, const class graphic *Tile);
    virtual void FillRRectSize(long x, long y, long width, long height, long cornerWidth, long cornerHeight, const class graphic *Tile);
    virtual void FillRgn(region * Rgn, const class graphic *Tile);
    virtual void FillTrapezoid(long topX, long topY, long topWidth, long bottomX, long bottomY, long bottomWidth, const class graphic *Tile);

    virtual void BitBlt(const struct rectangle *SrcRect, 
		class graphic *DestGraphic, const struct point *DstOrigin, 
		const struct rectangle *ClipRectList);
    virtual void SetBitAtLoc(long XPos, long YPos, boolean NewValue);
    virtual void WritePixImage( long DestX, long DestY, 
		const class pixelimage *SrcPixels, long SrcX, long SrcY, 
		long width, long height);

    virtual void ReadPixImage(long SrcX, long SrcY, 
		class pixelimage *DestPixels, long DestX, long DestY, 
		long width, long height);
    virtual void WriteImage(long DestX, long DestY, 
			    ddimage &image, long SrcX, long SrcY, 
			    long width, long height);

   
    virtual void InsertGraphicRegion(class graphic *EnclosingGraphic, const class region *regions);
    virtual void InsertGraphic(class graphic *EnclosingGraphic, const struct rectangle *EnclosedRectangle);
    virtual void InsertGraphicSize(class graphic *EnclosingGraphic, 
		long xOriginInParent, long yOriginInParent, long width, long height);

    virtual void SetVisualRegion(class region *region);
    virtual void SetClippingRegion(class region *region);
    virtual void SetClippingRect(struct rectangle * AdditionalRect);
    virtual void ClearClippingRect();

    virtual void SetLineWidth(short NewLineWidth);
    virtual void SetLineDash( const unsigned char *dashPattern, int dashOffset, LineDash dashType );
    virtual void SetLineCap( LineCap newLineCap );
    virtual void SetLineJoin( LineJoin newLineJoin );

    virtual void SetTransferMode(short NewTransferMode);

    virtual void FlushGraphics();

    virtual class graphic * WhitePattern()  ;
    virtual class graphic * BlackPattern()  ;
    virtual class graphic * GrayPattern(short IntensityNum, short IntensityDenom)  ;
    virtual void SetPatternOrigin(long xpos, long ypos);

    virtual void SetForegroundColor(color *c);
    virtual void SetBackgroundColor(color *c);
    
    virtual void SetForegroundColor(const char *name, long r, long g, long b);
    virtual void SetBackgroundColor(const char *name, long r, long g, long b);
    
    virtual long GetHorizontalResolution()  ;
    /**< Obtain the X display resolution, in dots per inch.
     *   \return the horizontal display resolution.
     *   \pref{XDPI,Integer,The default X display resolution.  This overrides
     *                       computation from the X server.}
     *   \pref{XHorizontalDPI,Integer,The default X horizontal resolution.
     *                                This overrides computation from the
     *                                X server as well as the XDPI preference.}
     */
    virtual long GetVerticalResolution()  ;
    /**< Obtain the X display resolution, in dots per inch.
     *   This will also default to the XDPI preference, if present.
     *   \return the vertical display resolution.
     *   \pref{XVerticalDPI,Integer,The default X vertical resolution.
     *                              This overrides computation from the
     *                              X server as well as the XDPI preference.}
     */
    virtual const char * GetWindowManagerType()  ;
    virtual long GetDevice()  ;
    virtual long DisplayClass()  ;
    virtual boolean IsImageInverted()  ;
    virtual void ReadImage(long SrcX, long SrcY, class image *img, long DestX, long DestY, long width, long height);
    virtual void WriteImage(long DestX, long DestY, 
		class image *image, long SrcX, long SrcY, 
		long width, long height);
    virtual void ObservedChanged(class observable *changed, long val);

    inline boolean &Valid() { return ((this)->valid); }
    inline Drawable &XWindow() { return ((this)->localWindow); }
    inline Display * &XDisplay() { return ((this)->displayUsed); }
    inline int &XScreen() { return ((this)->screenUsed); }
    inline GC &XGC()  { return ((this)->localGraphicContext); }
    inline GC &XFillGC() { return ((this)->localFillGraphicContext); }


    static void SetUpdateRegion(Region Rgn,Display* whichDisplay, Drawable whichWindow);
    static void FinalizeWindow(Display *whichDisplay, Drawable whichWindow);
    
    
	xgraphic();
    
	static boolean InitializeClass();

 
    class xgraphic **gray_levels;	/* mapping of gray pixmaps for monochrome rendering of colors */
    class xgraphic **gray_shades;   /* Current set of gray pixmaps */
    GC localGraphicContext;	/* Context for the pixmap */
    GC localFillGraphicContext;	/* Context for filling */
    long lastFillStyle;		/* Fill style used for fill gc */
    unsigned long lastFillPixel; /* Pixel used for fill gc */
    const class xgraphic *lastFillTile;  /* Tile used for fill gc */
    Drawable localWindow;		/* X pixmap id  */
    Display *displayUsed;		/* which display is used */
    int xfd;				/* fd for X connection (from *displayUsed) */
    int screenUsed;			/* which screen is used */
    long lastUpdateRegionIDUsed;    /* set the last update region sequence number that has been processed */
    unsigned long foregroundpixel; /* Pixel value of foreground color. */
    unsigned long backgroundpixel; /* Pixel value of background color. */
    long DisplayClassVal;
    boolean valid;		/* Set to true if we have a valid Xwindow, etc */
    class xgraphic *lastStipple;
    unsigned long *index;
    int numColorsInIndex;
    XImageInfo *ximageinfo;
    boolean flipforstipple;
    boolean haveMapInfo;
  private:
    void DrawChars(const char  * Text,short  Operation ,short  StringMode,long  TextLength );
    boolean SetupFillGC(const class graphic  * Tile );
    void SetStipple(long  index);
    void LocalSetClippingRect(struct xgraphic_UpdateBlock  * updateBlk );
    void InstallUpdateRegion();
    void SetUpPixImage(const class pixelimage  *pixelimage);
    void SetUpXImage( class image  *image );
    void HandleInsertion(class graphic  *E);

};
/** @} */
#endif /* _xgraphic_H_ */
