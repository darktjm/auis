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

 /* xgraphic.c
  */

#include <andrewos.h>
ATK_IMPL("xgraphic.H")
#include <stdio.h>
#include <util.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

ATK_IMPL("xgraphic.H")
#include <physical.h>
#include <graphic.H>
#include <pixelimage.H>
#include <fontdesc.H>
#include <environ.H>
#include <view.H>
#include <xfontdesc.H>
#include <region.H>
#include <image.H>
#include <xcolor.H>
#include <xcolormap.H>
#include <xgraphic.H>
#include <xddimage.H>

static int regionDebug = 0;
static int imageDebug = 0;
static int bltDebug = 0;

#define	XFullBrightness	65535.0

struct xgraphic_GrayBlock {
    struct xgraphic_GrayBlock * nextBlock;
    Display * displayUsed;
    int screenUsed;
    class xgraphic * graphic_shades[17];
};

struct image_prefs {
    boolean alwaysUsePrivateCmap;
    boolean alwaysFitFixed;
    boolean alwaysDither;
} imagePrefs;

static struct xgraphic_GrayBlock * grayBlockHeader = NULL;

#ifdef OLDXBUG
/********** Special declarations used in an attempt to circumvent an Xlib bug with regions. These declarations and their use shold be removed since they depend on internal (and therefore changable) structure of Xlib **********/
typedef struct {
    short x1, x2, y1, y2;
} Box, BOX, BoxRec, *BoxPtr;

typedef struct _XRegion {
    short size;
    short numRects;
    BOX *rects;
    BOX extents;
} REGION;
#endif /* OLDXBUG */

/******* end of special declarations *********/

ATKdefineRegistry(xgraphic, graphic, xgraphic::InitializeClass);
class xgraphic * * xgraphic_FindGrayBlock(Display  * WhichDisplay, int  WhichScreen );
struct  xgraphic_UpdateBlock * xgraphic_FindUpdateBlock(Display  * WhichDisplay, Drawable  WhichWindow);
static void InstallUpdateRegion(class xgraphic  * self );
static void xgraphic_LocalSetTransferFunction(class xgraphic  * self, int  prevValue);
static void  ReallySetFont(class xgraphic  *self);
static void xgraphic_DrawChars(class xgraphic  * self,const char  * Text,short  Operation ,short  StringMode,long  TextLength );
static void SetUpPixImage(class xgraphic  *self, class pixelimage  *pixelimage);
static unsigned int  bitsPerPixelAtDepth(Display       *disp, int            scrn, unsigned int depth);
XImageInfo * imageToXImage( class xgraphic  *self, class image  *image, unsigned int private_cmap, unsigned int fit );
static void SetUpXImage( class xgraphic  *self, class image  *image );
void  sendXImage( class xgraphic  *self, XImageInfo   *ximageinfo, int           src_x , int           src_y , int           dst_x , int           dst_y, unsigned int w , unsigned int h );
void xgraphic_LocalSetClippingRect(class xgraphic  * self ,struct xgraphic_UpdateBlock  * updateBlk );
static void xgraphicClearGrayLevels(class xgraphic  *self);
static void CacheShades(class xgraphic  *self);
static void GetShades(class xgraphic  *self);
class xgraphic * xgraphicGrayShade (class xgraphic  *self, long  index	);
static void SetFGPixel( class xgraphic  *self, unsigned long  pixel );
static void SetBGPixel( class xgraphic  *self, unsigned long  pixel );
static short xgraphic_ApproximateColor( class xgraphic  *self, unsigned short  *red , unsigned short  *green , unsigned short  *blue );
static void SetStipple(class xgraphic  *self, long  index);
static long RealDisplayClass( class xgraphic		     *self );


class xgraphic * * xgraphic_FindGrayBlock(Display  * WhichDisplay, int  WhichScreen )
{
    struct xgraphic_GrayBlock * CurBlock;
    int i;

    for (CurBlock=grayBlockHeader;CurBlock;CurBlock=CurBlock->nextBlock){
        if ( (CurBlock->displayUsed == WhichDisplay) &&
	     (CurBlock->screenUsed == WhichScreen)) 
	    return CurBlock->graphic_shades;
    }
    /* Not there, so add one */
    CurBlock = (struct xgraphic_GrayBlock *) malloc(sizeof(struct xgraphic_GrayBlock));
    CurBlock->nextBlock=grayBlockHeader;
    CurBlock->displayUsed = WhichDisplay;
    CurBlock->screenUsed = WhichScreen;
    for (i=0;i<17;i++) CurBlock->graphic_shades[i] =  NULL;
    grayBlockHeader = CurBlock;
    return CurBlock->graphic_shades;
}

/* C hacks to keep everything looknig the same */
struct font {
    XFontStruct dummy;
};


#define VerifyUpdateClipping(selfParam) \
    if ((selfParam)->lastUpdateRegionIDUsed != curUpdateRegionID) InstallUpdateRegion((selfParam))

static long curUpdateRegionID = 0; /* which region is currently being used */

struct xgraphic_UpdateBlock {
    struct xgraphic_UpdateBlock * nextBlock;
    Display * displayUsed;
    Drawable windowUsed;
    Region updateRegionInUse;
    long RegionCounter;
};

static struct xgraphic_UpdateBlock * updateBlockHeader = NULL;

struct  xgraphic_UpdateBlock * xgraphic_FindUpdateBlock(Display  * WhichDisplay, Drawable  WhichWindow)
{
    struct xgraphic_UpdateBlock * CurBlock;

    if (regionDebug) printf("FindUpdateBlock: looking for display %p, window %lX\n", WhichDisplay, WhichWindow);

    for (CurBlock=updateBlockHeader;CurBlock;CurBlock=CurBlock->nextBlock){
        if ( (CurBlock->displayUsed == WhichDisplay) &&
	     (CurBlock->windowUsed == WhichWindow)) {
	    if (regionDebug) printf("FindUpdate: found old %p\n", CurBlock);
	    return CurBlock;
	}
    }
    /* Not there, so add one */
    CurBlock = (struct xgraphic_UpdateBlock *) malloc(sizeof(struct xgraphic_UpdateBlock));
    if (regionDebug) printf("FindUpdate: making new %p\n", CurBlock);
    CurBlock->nextBlock=updateBlockHeader;
    CurBlock->displayUsed = WhichDisplay;
    CurBlock->windowUsed = WhichWindow;
    CurBlock->updateRegionInUse = NULL;
    CurBlock->RegionCounter = -1;
    updateBlockHeader = CurBlock;
    return CurBlock;
}


/* forward declaration */



static void InstallUpdateRegion(class xgraphic  * self )
{
    struct xgraphic_UpdateBlock * curBlock;

    if (regionDebug) printf("InstallUpdateRegion: new region, cur glob ID %ld\n", curUpdateRegionID);
    /* find out whether a real change has happened or just a false alarm */
    curBlock = xgraphic_FindUpdateBlock((self)->XDisplay(), (self)->XWindow());
    if (curBlock->RegionCounter == self->lastUpdateRegionIDUsed) {
	/* False alarm, someone else bumped counter, nothing for this graphic (window/display) has changed, so update our counter to show that we really are current with the latest changes */
	if (regionDebug) printf("InstallUpdateRegion: no change, curBlockID %ld, graphic ID %ld\n", curBlock->RegionCounter, self->lastUpdateRegionIDUsed);
	self->lastUpdateRegionIDUsed = curUpdateRegionID;
    }
    else {
	/* New region, so let's go use it */
	xgraphic_LocalSetClippingRect(self,curBlock);
    }
}

void xgraphic::SetUpdateRegion(Region  Rgn,Display * whichDisplay,Drawable  whichWindow )
{
	ATKinit;

    struct xgraphic_UpdateBlock * curBlock;

    if (regionDebug) printf("SetUpdateRegion: Rgn %p, whichDisplay %p, whichWindow %lX\n", Rgn, whichDisplay, whichWindow);

    curUpdateRegionID++;
    /* update list of regions and their use */
    curBlock = xgraphic_FindUpdateBlock(whichDisplay,whichWindow);
    curBlock->updateRegionInUse = Rgn;
    curBlock->RegionCounter = curUpdateRegionID;
    if (regionDebug) printf("SetUpdateRegion: for block %p, setting counter %ld, region %p\n", curBlock, curBlock->RegionCounter, curBlock->updateRegionInUse);
}

void xgraphic::FinalizeWindow(Display  *WhichDisplay, Drawable  WhichWindow)
{
	ATKinit;

    struct xgraphic_UpdateBlock *CurBlock, *NextBlock, *last=NULL;
    for (CurBlock=updateBlockHeader ; CurBlock ; CurBlock=NextBlock){
	NextBlock=CurBlock->nextBlock;
	if ( (CurBlock->displayUsed == WhichDisplay) &&
	     (CurBlock->windowUsed == WhichWindow) ) {
	    if(last) last->nextBlock=NextBlock;
	    else updateBlockHeader=NextBlock;
	    if(CurBlock->updateRegionInUse) XDestroyRegion(CurBlock->updateRegionInUse);
	    free(CurBlock);
	} else
	    last=CurBlock;
    }
}

/* since X uses 0=black and ATK uses 0=white, */
boolean xgraphic::IsImageInverted()
{
    return TRUE;
}

static void xgraphic_LocalSetTransferFunction(class xgraphic  * self, int  prevValue)
{
/* This discussion documents how the X transfer function was calculated.

Graphic works on the following bit encoding:
0=white, 1= black

Source	1   1	0   0	    b	b   w	w
Destin	1   0	1   0	    b	w   b	w
Result	?   ?	?   ?	    ?	?   ?	?

X uses 0=black, 1 = white

We construct the following table of all transfer modes of Graphic (16
possible functions), determine what the visual effects are in terms of
black and white, and then remap the b/w values into X's 0/1 scheme.
Finally, we determine which X function causes the visual effect
and consider it the mapped ALU function for X.

	G Code	Appear	X Code X Function   X Func Encoding

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	0000	wwww	1111	GXset	    1111

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	0001	wwwb	1110	GXnand	    1110

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	0010	wwbw	1101	GXorInverted 1101

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	0011	wwbb	1100	GXcopyInverted 1100

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	0100	wbww	1011	GXorReverse 1011

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	0101	wbwb	1010	GXinvert    1010

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	0110	wbbw	1001	GXequiv	    1001

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	0111	wbbb	1000	GXnor	    1000

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	1000	bwww	0111	GXor	    0111

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	1001	bwwb	0110	GXxor	    0110

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	1010	bwbw	0101	GXnoop	    0101

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	1011	bwbb	0100	GXandInverted 0100

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	1100	bbww	0011	GXcopy	0011

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	1101	bbwb	0010	GXandReverse 0010

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	1110	bbbw	0001	GXand	0001

Src	1100	bbww	0011
Dst	1010	bwbw	0101
Result	1111	bbbb	0000	GXclear	0000

Therefore, the X transfer mode is exactly bit reverse of the graphic
transfer mode.

 */

    int xMode = (~self->transferMode) & 0xF;

/* Setup the foreground and background colors right. One would like to do this
 * using the transfer modes, but that is not the way X11 works...
 */
    if (self->transferMode == graphic_WHITE)  {
        xMode = GXcopy;
        if (prevValue != graphic_WHITE)  {
            XSetBackground((self)->XDisplay(), (self)->XGC(),	self->foregroundpixel);
            XSetBackground((self)->XDisplay(), (self)->XFillGC(), self->foregroundpixel);
            XSetForeground((self)->XDisplay(), (self)->XGC(),	self->backgroundpixel);
            XSetForeground((self)->XDisplay(), (self)->XFillGC(), self->backgroundpixel);
            self->lastFillPixel = self->backgroundpixel;
        }
    }
#ifndef PLANEMASK_ENV
    else if (self->transferMode == graphic_XOR)  {
        if (prevValue != graphic_XOR)  {
            XSetBackground((self)->XDisplay(), (self)->XGC(),	0);
            XSetBackground((self)->XDisplay(), (self)->XFillGC(), 0);
            XSetForeground((self)->XDisplay(), (self)->XGC(),	self->foregroundpixel ^ self->backgroundpixel);
            XSetForeground((self)->XDisplay(), (self)->XFillGC(), self->foregroundpixel ^ self->backgroundpixel);
	    self->lastFillPixel = self->foregroundpixel ^ self->backgroundpixel;
	}
	xMode = GXxor;
    }
#endif /* PLANEMASK_ENV */
    else {
        if (prevValue == graphic_WHITE || prevValue == graphic_XOR)  {
            XSetBackground((self)->XDisplay(), (self)->XGC(),	self->backgroundpixel);
            XSetBackground((self)->XDisplay(), (self)->XFillGC(), self->backgroundpixel);
            XSetForeground((self)->XDisplay(), (self)->XGC(),	self->foregroundpixel);
            XSetForeground((self)->XDisplay(), (self)->XFillGC(), self->foregroundpixel);
            self->lastFillPixel = self->foregroundpixel;
        }
        if (self->transferMode == graphic_BLACK)
            xMode = GXcopy;
    }

    if (self->transferMode == graphic_INVERT)  {
	if (prevValue != graphic_INVERT)  {
	    XSetPlaneMask((self)->XDisplay(), (self)->XGC(), self->foregroundpixel ^ self->backgroundpixel);
	    XSetPlaneMask((self)->XDisplay(), (self)->XFillGC(), self->foregroundpixel ^ self->backgroundpixel);
	}
    }
    else  {
	if (prevValue == graphic_INVERT)  {
	    XSetPlaneMask((self)->XDisplay(), (self)->XGC(), AllPlanes);
	    XSetPlaneMask((self)->XDisplay(), (self)->XFillGC(), AllPlanes);
	}
    }

    if(xMode==GXcopy && self->flipforstipple) {
	XSetFunction((self)->XDisplay(), (self)->XGC(), GXcopyInverted);
    } else {
	XSetFunction((self)->XDisplay(),
                  (self)->XGC(),
		     xMode);
    }
    XSetFunction((self)->XDisplay(),
                  self->localFillGraphicContext,
		  xMode);

    if(self->transferMode == graphic_XOR) {
	class xgraphic *tile=xgraphicGrayShade(self, 16);
	XSetStipple((self)->XDisplay(),  (self)->XGC(), tile->localWindow);
    } else if(self->lastStipple) XSetStipple((self)->XDisplay(), (self)->XGC(), self->lastStipple->localWindow);
}



void xgraphic::DrawLineTo(long  XEnd, long  YEnd )
{
    VerifyUpdateClipping(this);

    XDrawLine((this)->XDisplay(),
	  (this)->XWindow(),
	  (this)->XGC(),
	  physical_LogicalPtToGlobalX(this,&this->currentPoint),
	  physical_LogicalPtToGlobalY(this,&this->currentPoint),
	  physical_LogicalXToGlobalX(this,XEnd),
	  physical_LogicalYToGlobalY(this,YEnd));
    point_SetPt(&this->currentPoint,XEnd,YEnd);
}

void xgraphic::DrawLine(long  DeltaX , long  DeltaY )
{

    struct point OldPt;

    VerifyUpdateClipping(this);

    OldPt = this->currentPoint;
    point_OffsetPoint(&this->currentPoint,DeltaX,DeltaY);
    XDrawLine((this)->XDisplay(),
	  (this)->XWindow(),
	  (this)->XGC(),
	  physical_LogicalPtToGlobalX(this,&OldPt),
	  physical_LogicalPtToGlobalY(this,&OldPt),
	  physical_LogicalPtToGlobalX(this,&this->currentPoint),
	  physical_LogicalPtToGlobalY(this,&this->currentPoint));

}

	static void 
ReallySetFont(class xgraphic  *self)
	{
	/* Select the font for later text drawing:
		Note: only localGC used for text, not fillGC */
	XFontStruct *f = (XFontStruct *)((class xfontdesc *)self->currentFont)->GetRealFontDesc(
			 self);
	if (f != NULL)  
		XSetFont((self)->XDisplay(),  (self)->XGC(), f->fid);
}


void xgraphic::SetFont(class fontdesc  * ChosenFont)
{

    if (ChosenFont) {
	if (this->currentFont == (class fontdesc *) ChosenFont)  {
	    return;
	}
	else {
	    this->currentFont = (class fontdesc *) ChosenFont;
	}
    }
    /* Damn, nothing there, so switch back to default */
    else {
        this->currentFont = fontdesc::Create("andysans", fontdesc_Plain, 12);
    }

    ReallySetFont(this);
}


/* Descriptions of string formats for string and text */

#define xgraphic_NULLTERMINATED 0
#define xgraphic_LENGTHGIVEN 1

static void xgraphic_DrawChars(class xgraphic  * self,const char  * Text,short  Operation ,short  StringMode,long  TextLength )
{

    XCharStruct *maxChar;
    long x = point_X(&self->currentPoint);
    long y = point_Y(&self->currentPoint);

    VerifyUpdateClipping(self);

    /* Do we need to generate a count? */
    if (StringMode==xgraphic_NULLTERMINATED) TextLength = strlen(Text);

    if (Operation /* !=graphic_NOMOVEMENT */) {
        /* GetRealFontDesc is used to load the font cache in fontdesc */
        maxChar =
        &(self->currentFont)->GetRealFontDesc( self)->dummy.max_bounds;

        if (Operation&
	   (graphic_ATTOP|graphic_BETWEENTOPANDBOTTOM|graphic_ATBOTTOM)){
	    y += maxChar->ascent;
        }
        if (Operation&graphic_BETWEENTOPANDBASELINE) {
            y += maxChar->ascent >> 1;
        }
        if (Operation&graphic_ATBOTTOM) {
	    y -= maxChar->ascent + maxChar->descent;
        }
        if (Operation&graphic_BETWEENTOPANDBOTTOM) {
	    y -= (maxChar->ascent + maxChar->descent)>>1;
        }
        if (Operation&(graphic_ATRIGHT|graphic_BETWEENLEFTANDRIGHT)) {
	    long LastXWidth;
	    LastXWidth = XTextWidth(
	        &((self->currentFont)->GetRealFontDesc( self)->dummy),
	        Text,TextLength);
	    if (Operation&graphic_ATRIGHT) {
	        x -= LastXWidth;
	    } else {
	        x -= LastXWidth>>1;
	    }
        }

    }
    /* Put out the actual characters */
    
    if (self->spaceShim) {
	/* Have space shim, must break up text and dump it */

#define InitialWordGuess 10

	static XTextItem * tip = NULL;
	static int NumAlloc = InitialWordGuess;
        int WdCnt;
	const char * WordStart;
	int i;
	int CharCount;

	if( tip == NULL )
	    tip = (XTextItem *)
			 malloc(NumAlloc * sizeof(XTextItem));
	tip[0].chars = (char *)Text;
        tip[0].delta = 0;
	tip[0].font = 0; /* use GC font */
	WordStart = Text;
	CharCount = 0;
	WdCnt = 0;
	for(i=0;i<TextLength;i++) {
	    CharCount++;
	    if (Text[i]==' ') {
		/* Finish current slot (include the space) */
		tip[WdCnt].nchars = CharCount;
		/* Advance the pointers */
		WdCnt++;
		WordStart += CharCount;
		CharCount = 0;
		/* Room for one more? */
	        if (WdCnt == NumAlloc) {
		    /* Used up everything, allocate some more space */
		    NumAlloc += InitialWordGuess;
		    tip = (XTextItem *)
			 realloc(tip, NumAlloc * sizeof(XTextItem));
		}
		/* Fill in next slot */
		tip[WdCnt].chars = (char *)WordStart;
		tip[WdCnt].delta = self->spaceShim;
		tip[WdCnt].font = 0; /* use GC font */
	    }
	}
	/* Check to see if a last word is present */
	if (CharCount) {
	    /* Yes, so fill in remaining values */
	    tip[WdCnt].nchars = CharCount;
	    WdCnt++;
	}
	/* Go write it */
	XDrawText((self)->XDisplay(),
	    (self)->XWindow(),
	    (self)->XGC(),
	    physical_LogicalXToGlobalX(self,x),
	    physical_LogicalYToGlobalY(self,y),tip,WdCnt);
    } /* end of space shim test */
    else {
	/* No shim, we can just go for it */
	XDrawString((self)->XDisplay(),
	    (self)->XWindow(),
	    (self)->XGC(),
	    physical_LogicalXToGlobalX(self,x),
	    physical_LogicalYToGlobalY(self,y),Text,TextLength);	
    }

}

void xgraphic::DrawString(const char  * Text, short  Operation )
{

    xgraphic_DrawChars(this,Text,Operation,xgraphic_NULLTERMINATED,0);
}


void xgraphic::DrawText(const char  * Text, long  TextLength, short  Operation )
{

    xgraphic_DrawChars(this,Text,Operation,xgraphic_LENGTHGIVEN,
			    TextLength);
}

void xgraphic::DrawRectSize(long  x, long  y,long  width,long  height)
{
    VerifyUpdateClipping(this);

    XDrawRectangle((this)->XDisplay(),
		(this)->XWindow(),
		(this)->XGC(),
		physical_LogicalXToGlobalX(this,x),
		physical_LogicalYToGlobalY(this,y),
		width,height);

}

void xgraphic::DrawPolygon(struct point  * PointArray, short  PointCount )
{
    static XPoint * PolygonPts = NULL;
    static int numXPoints = 0;
    int i;

    VerifyUpdateClipping(this);

    if( numXPoints < (PointCount + 1) ) {
	numXPoints = PointCount + 1;
	if( PolygonPts == NULL )
	    PolygonPts = (XPoint *) malloc(sizeof(XPoint) * numXPoints);
	else
	    PolygonPts = (XPoint *) realloc(PolygonPts, numXPoints * sizeof(XPoint));
    }
    for(i=0;i<PointCount;i++){
	PolygonPts[i].x = physical_LogicalXToGlobalX(this, PointArray[i].x);
	PolygonPts[i].y = physical_LogicalYToGlobalY(this, PointArray[i].y);
	}
    PolygonPts[PointCount].x = physical_LogicalXToGlobalX(this, PointArray[0].x);
    PolygonPts[PointCount].y = physical_LogicalYToGlobalY(this, PointArray[0].y);
    XDrawLines((this)->XDisplay(), (this)->XWindow(), (this)->XGC(), PolygonPts, PointCount+1, CoordModeOrigin);
}

void xgraphic::DrawPath(struct point  * PointArray, short  PointCount )
{
    static XPoint * PolygonPts = NULL;
    static int numXPoints = 0;
    int i;

    VerifyUpdateClipping(this);

    if( numXPoints < PointCount ) {
	numXPoints = PointCount;
	if( PolygonPts == NULL )
	    PolygonPts = (XPoint *) malloc(sizeof(XPoint) * numXPoints);
	else
	    PolygonPts = (XPoint *) realloc(PolygonPts, numXPoints * sizeof(XPoint));
    }
    for(i=0;i<PointCount;i++){
	PolygonPts[i].x = physical_LogicalXToGlobalX(this, PointArray[i].x);
	PolygonPts[i].y = physical_LogicalYToGlobalY(this, PointArray[i].y);
	}
    XDrawLines((this)->XDisplay(), (this)->XWindow(), (this)->XGC(), PolygonPts, PointCount, CoordModeOrigin);
}

void xgraphic::DrawOvalSize(long  x,long  y,long  width,long  height )
{

    VerifyUpdateClipping(this);

    XDrawArc((this)->XDisplay(), (this)->XWindow(), (this)->XGC(), physical_LogicalXToGlobalX(this,x), physical_LogicalYToGlobalY(this,y), width, height, 0, 360*64);

}

void xgraphic::DrawArcSize(long  x,long  y,long  width,long  height, short  StartAngle, short  OffsetAngle)
{
    int StartXAngle, OffsetXAngle;

    VerifyUpdateClipping(this);

    StartXAngle = (90-StartAngle) <<6;
    OffsetXAngle = (-OffsetAngle) <<6;

    XDrawArc((this)->XDisplay(), (this)->XWindow(), (this)->XGC(), physical_LogicalXToGlobalX(this,x), physical_LogicalYToGlobalY(this,y), width, height, StartXAngle, OffsetXAngle);
}

void xgraphic::DrawRRectSize(long  x,long  y,long  width,long  height,long  cornerWidth ,long  cornerHeight )
{

    VerifyUpdateClipping(this);

    /* Handle pathologic cases in system indepedent manner
	(luser desires to bite bullet in efficiency) */

    if ( (2*cornerHeight >= height) || (2*cornerWidth >= width)) {
	/* Bizarre -- corners are bigger than rectangle, so 
	   make an appropriate looking oval */
	 if ( (2*cornerHeight >= height) && (2*cornerWidth >= width))
	    (this)->DrawOvalSize(x,y,width,height);
         else if (2*cornerHeight >= height) {
	    /* Draw left semi-oval */
	    (this)->DrawArcSize(x,y,2*cornerWidth,height,0,-180);
	    /* Draw Top line */
	    (this)->MoveTo(x+cornerWidth,y);
	    (this)->DrawLine(width-2*cornerWidth,0);
	    /* Draw right semi-oval */
	    (this)->DrawArcSize(x+width-2*cornerWidth,y,2*cornerWidth,height,0,180);
	    /* Draw bottom line */
	    (this)->MoveTo(x+cornerWidth,y+height);
	    (this)->DrawLine(width-2*cornerWidth,0);
	 }
	 else { /* assuming (2*cornerWidth >= width) */
	    /* Draw top semi-oval */
	    (this)->DrawArcSize(x,y,width,2*cornerHeight,-90,180);
	    /* Draw right line */
	    (this)->MoveTo(x+width,y+cornerHeight);
	    (this)->DrawLine(0,height-2*cornerHeight);
	    /* Draw bottom semi-oval */
	    (this)->DrawArcSize(x,y+height-2*cornerHeight,width,2*cornerHeight,90,180);
	    /* Draw left line */
	    (this)->MoveTo(x,y+cornerHeight);
	    (this)->DrawLine(0,height-2*cornerHeight);
	 }
	 return;
    }

{   XArc RRPath[8];
    long realX, realY, realEndX, realEndY;

    realX = physical_LogicalXToGlobalX(this,x);
    realY = physical_LogicalYToGlobalY(this,y);
    realEndX = realX + width;
    realEndY = realY + height;


    /* Upper left hand corner */
    RRPath[0].x = realX;
    RRPath[0].y = realY;
    RRPath[0].width = cornerWidth * 2;
    RRPath[0].height = cornerHeight * 2;
    RRPath[0].angle1 = -180 * 64; /* 9 o'clock */
    RRPath[0].angle2 = -90 * 64; /* 90 degreee clockwise turn */
    /* Top line */
    RRPath[1].x = realX + cornerWidth;
    RRPath[1].y = realY;
    RRPath[1].width = width - 2*cornerWidth;
    RRPath[1].height = 0;
    RRPath[1].angle1 = -180 * 64; /* 9 o'clock position */
    RRPath[1].angle2 = 180 * 64; /* move through straight line */
    /* upper right corner */
    RRPath[2].x = realEndX - 2*cornerWidth;
    RRPath[2].y = realY;
    RRPath[2].width = cornerWidth * 2;
    RRPath[2].height = cornerHeight * 2;
    RRPath[2].angle1 = 90 * 64; /* 12 o'clock position */
    RRPath[2].angle2 = -90 * 64; /* move through right turn */
    /* right side down */
    RRPath[3].x = realEndX;
    RRPath[3].y = realY + cornerHeight;
    RRPath[3].width = 0;
    RRPath[3].height = height - 2*cornerHeight;
    RRPath[3].angle1 = 90 * 64; /* 12 o'clock position */
    RRPath[3].angle2 = -180 * 64; /* line straight down */
    /* lower right corner */
    RRPath[4].x = realEndX - 2*cornerWidth;
    RRPath[4].y = realEndY - 2*cornerHeight;
    RRPath[4].width = cornerWidth * 2;
    RRPath[4].height = cornerHeight * 2;
    RRPath[4].angle1 = 0; /* 3 o'clock */
    RRPath[4].angle2 = -90 * 64; /* 90 degrees clockwise */
    /* Bottom line */
    RRPath[5].x = realX + cornerWidth;
    RRPath[5].y = realEndY;
    RRPath[5].width = width - 2*cornerWidth;
    RRPath[5].height = 0;
    RRPath[5].angle1 = 0; /* 3 o'clock */
    RRPath[5].angle2 = -180 * 64;  /* move 180 degrees */
    /* lower left corner */
    RRPath[6].x = realX;
    RRPath[6].y = realEndY - 2*cornerHeight;
    RRPath[6].width = cornerWidth * 2;
    RRPath[6].height = cornerHeight * 2;
    RRPath[6].angle1 = -90 * 64 ; /* 6 o'clock */
    RRPath[6].angle2 = -90 * 64; /* 90 degrees clockwise */
    /* left side */
    RRPath[7].x = realX;
    RRPath[7].y = realY + cornerHeight;
    RRPath[7].width = 0;
    RRPath[7].height = height - 2*cornerHeight;
    RRPath[7].angle1 = -90 * 64; /* 6 o'clock */
    RRPath[7].angle2 = -180 * 64; /* move 180 degrees */

    XDrawArcs((this)->XDisplay(), (this)->XWindow(), (this)->XGC(), RRPath, 8);
}

}
static atom_def trans("transparent");
void xgraphic::DrawTrapezoid(long  topX ,long  topY ,long  topWidth ,long  bottomX ,long  bottomY ,long  bottomWidth)
{
    XPoint PolygonPts[5];

    VerifyUpdateClipping(this);

    PolygonPts[0].x = PolygonPts[4].x = physical_LogicalXToGlobalX(this, topX);
    PolygonPts[0].y = PolygonPts[4].y = physical_LogicalYToGlobalY(this, topY);
    PolygonPts[1].x = physical_LogicalXToGlobalX(this, topX+topWidth);
    PolygonPts[1].y = physical_LogicalYToGlobalY(this, topY);
    PolygonPts[2].x = physical_LogicalXToGlobalX(this, bottomX+bottomWidth);
    PolygonPts[2].y = physical_LogicalYToGlobalY(this, bottomY);
    PolygonPts[3].x = physical_LogicalXToGlobalX(this, bottomX);
    PolygonPts[3].y = physical_LogicalYToGlobalY(this, bottomY);

    XDrawLines((this)->XDisplay(), (this)->XWindow(), (this)->XGC(), PolygonPts, 5, CoordModeOrigin);
}

static boolean xgraphic_SetupFillGC(class xgraphic  * self, class graphic  * Tile )
{
    int grayIndex;

class xgraphic * tile = (class xgraphic *)Tile;
unsigned long	fgPixel;

    /* See if transfer mode will take care of it, i.e., mode is source independent. If so, just make sure that a fillsolid mode is picked in the belief that the server won't be smart enough to realize that only the shape matters and not to waste time aligning any random tile that was left over */
    if ( (self->transferMode == graphic_BLACK) ||
	 (self->transferMode == graphic_WHITE) ||
         (self->transferMode == graphic_INVERT) ||
	 (self->transferMode == graphic_DEST) ) {
	 /* source independent, just optimize server */
         if (self->lastFillStyle != FillSolid) {
	    XSetFillStyle((self)->XDisplay(), (self)->XFillGC(), FillSolid);
	    self->lastFillStyle = FillSolid;
	 }
	 if(self->transferMode==graphic_BLACK && self->fore->name==trans) return FALSE;
	 else if(self->transferMode==graphic_WHITE && self->back->name==trans) return FALSE;
	 return TRUE;
    }

    if ( tile == NULL &&
	(( (self )->DisplayClass( ) & (graphic_Color | graphic_GrayScale)) /* ||
	 ( xgraphic_DisplayClass( self ) & graphic_StaticGray ) */))
      tile = ( class xgraphic * ) (self)->BlackPattern();
    else if ( tile == NULL )
      tile = (self->lastFillTile != NULL) ? self->lastFillTile : (class xgraphic *) (self)->BlackPattern();

    if(self->transferMode == graphic_XOR ) {
	if(tile==(class xgraphic *)(self)->BlackPattern()) tile = self->gray_shades[16];
    }
    
    /* Hm, depends on sources, but source may be white or black, so let's special case those as well */
    if (!((self)->DisplayClass() & graphic_StaticGray)
	&& self->gray_levels[16] != NULL && self->gray_levels[16] == tile) {
	/* We're using black, make sure context is OK */
	fgPixel = (self->transferMode == graphic_XOR) ? self->foregroundpixel ^ self->backgroundpixel : self->foregroundpixel;

        if (self->lastFillStyle != FillSolid) {
	    XSetFillStyle((self)->XDisplay(), (self)->XFillGC(), FillSolid);
	    self->lastFillStyle = FillSolid;
	}
        if (self->lastFillPixel != fgPixel) {
	    XSetForeground((self)->XDisplay(), (self)->XFillGC(),  fgPixel);
	    self->lastFillPixel = fgPixel;
	}
	self->lastFillTile = self->gray_levels[16];
    }
    else if (!((self)->DisplayClass() & graphic_StaticGray)
	     && self->gray_levels[0] != NULL && self->gray_levels[0] == tile) {
	/* We're using white, make sure content is OK */

       if (self->lastFillStyle != FillSolid) {
	    XSetFillStyle((self)->XDisplay(), (self)->XFillGC(), FillSolid);
	    self->lastFillStyle = FillSolid;
	}
        if (self->lastFillPixel !=  self->backgroundpixel) {
	    XSetForeground((self)->XDisplay(), (self)->XFillGC(),  self->backgroundpixel);
	    self->lastFillPixel = self->backgroundpixel;
	}
	self->lastFillTile = self->gray_levels[0];
    }
    /* Not black or white, but maybe predefind gray that is already down loaded, if so just set fill style and don't download pixmap again. Note: we are exceedingly tricky by using the assignment statement to pickup the gray shade that matches, and by picking what we think will be most common ones first. We assume, as per C book, that evaluatio stops with first true test.  We include 0 and 16 for the monochrome case. */
    else   {
	static const int ind[] = { 0, 16, 8, 4, 12, 1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15};
	unsigned int i = sizeof(ind)/sizeof(int);

	if (tile != NULL)  {
	    for (i = 0; i < (sizeof(ind)/sizeof(int)) && self->gray_shades[ind[i]] != tile; i++)
		;
	}
	if (i < (sizeof(ind)/sizeof(int)))  {
	    grayIndex = ind[i];

	    /* Lucky us, reusing a preloaded gray shade, so just make sure that fill style is set correctly, and see if we have alredy downloaded tile */
	    fgPixel = (self->transferMode == graphic_XOR) ? self->foregroundpixel ^ self->backgroundpixel : self->foregroundpixel;
	    
	    if (self->lastFillPixel != fgPixel) {
		XSetForeground((self)->XDisplay(), (self)->XFillGC(),  fgPixel);
		self->lastFillPixel = fgPixel;
	    }
	    if (self->lastFillStyle != FillOpaqueStippled) {
		XSetFillStyle((self)->XDisplay(), (self)->XFillGC(), FillOpaqueStippled );
		self->lastFillStyle = FillOpaqueStippled;
	    }
	    if (self->lastFillTile != self->gray_shades[grayIndex]) {
		/* oh well, let's download the gray pattern */
		XSetStipple((self)->XDisplay(),
			 self->localFillGraphicContext,
			 tile->localWindow);
		self->lastFillTile = self->gray_shades[grayIndex];
	    }
	}
	/* Unknown, or unused tile, so download it and use it */
	else {
	    fgPixel = (self->transferMode == graphic_XOR) ? self->foregroundpixel ^ self->backgroundpixel : self->foregroundpixel;

	    if (self->lastFillPixel != fgPixel) {
		XSetForeground((self)->XDisplay(), (self)->XFillGC(),  fgPixel);
		self->lastFillPixel = fgPixel;
	    }
	    /* Let's see if style is right */
	    if (self->lastFillStyle != FillOpaqueStippled) {
		XSetFillStyle((self)->XDisplay(), (self)->XFillGC(), FillOpaqueStippled);
		self->lastFillStyle = FillOpaqueStippled;
	    }
	    /* And let's send down the tile */
	    XSetStipple((self)->XDisplay(),
		     self->localFillGraphicContext,
		     tile->localWindow);
	    self->lastFillTile = tile;

	}
    }
    if(self->fore->name==trans && self->lastFillTile==self->BlackPattern()) {
	return FALSE;
    } else if(self->back->name==trans && self->lastFillTile==self->WhitePattern()) {
	return FALSE;
    }
    return TRUE;
}

void xgraphic::FillRectSize(long  x,long  y,long  width,long  height,class graphic *Tile )
{

    if (width <= 0 || height <= 0)  return;

    VerifyUpdateClipping(this);

    /* First change the pattern to match the tile
	    (we should special case white and black for efficiency) */
    if(xgraphic_SetupFillGC(this, Tile)) {

    /* And fill */
	XFillRectangle((this)->XDisplay(), (this)->XWindow(), (this)->XFillGC(), physical_LogicalXToGlobalX(this,x), physical_LogicalYToGlobalY(this,y), width, height);
    }
}


void xgraphic::FillPolygon(struct point  *PointArray, short  PointCount, class graphic  *Tile)
{

    static XPoint *PolygonPts = NULL;
    static int numXPoints = 0;
    int i;

    VerifyUpdateClipping(this);

    if( numXPoints < PointCount ) {
	numXPoints = PointCount;
	if( PolygonPts == NULL )
	    PolygonPts = (XPoint *) malloc(sizeof(XPoint) * numXPoints);
	else
	    PolygonPts = (XPoint *) realloc(PolygonPts, numXPoints * sizeof(XPoint));
    }
    for(i=0;i<PointCount;i++){
	PolygonPts[i].x = physical_LogicalXToGlobalX(this, PointArray[i].x);
	PolygonPts[i].y = physical_LogicalYToGlobalY(this, PointArray[i].y);
	}
	
    /* First change the pattern to match the tile (we should special case white and black for efficiency) */
    if( xgraphic_SetupFillGC(this, Tile)) {

	XFillPolygon((this)->XDisplay(), (this)->XWindow(), (this)->XFillGC(), PolygonPts, PointCount, Complex, CoordModeOrigin);
    }
}

void xgraphic::FillOvalSize(long  x,long  y,long  width,long  height,class graphic *Tile)
{

    VerifyUpdateClipping(this);

    /* First change the pattern to match the tile (we should special case white and black for efficiency) */
    if(xgraphic_SetupFillGC(this, Tile)) {

	XFillArc((this)->XDisplay(), (this)->XWindow(), (this)->XFillGC(), physical_LogicalXToGlobalX(this,x), physical_LogicalYToGlobalY(this,y), width, height, 0, 360 * 64);
    }

}

void xgraphic::FillArcSize(long  x,long  y,long  width,long  height,short  StartAngle, short  OffsetAngle,class graphic *Tile)
{
    int StartXAngle, OffsetXAngle;

    VerifyUpdateClipping(this);

    StartXAngle = ((-StartAngle)+90) <<6;
    OffsetXAngle = (-OffsetAngle) <<6;

    /* First change the pattern to match the tile (we should special case white and black for efficiency) */
    if(xgraphic_SetupFillGC(this, Tile)) {
/*    XSetTile(xgraphic_XDisplay(self),
		xgraphic_XFillGC(self),
		Tile->localWindow,0,0); */
	XFillArc((this)->XDisplay(), (this)->XWindow(), (this)->XFillGC(), physical_LogicalXToGlobalX(this,x), physical_LogicalYToGlobalY(this,y), width, height, StartXAngle, OffsetXAngle);
    }

}


void xgraphic::FillRRectSize(long  x,long  y,long  width,long  height,long  cornerWidth ,long  cornerHeight,class graphic *Tile)
{
    /* Handle pathologic cases in system indepedent manner
	(luser desires to bite bullet in efficiency) */

    VerifyUpdateClipping(this);

    if ( (2*cornerHeight >= height) || (2*cornerWidth >= width)) {
	/* Bizarre -- corners are bigger than rectangle, so 
	   make an appropriate looking oval */
	 if ( (2*cornerHeight >= height) && (2*cornerWidth >= width))
	    (this)->FillOvalSize(x,y,width,height,Tile);
         else if (2*cornerHeight >= height) {
	    /* Fill left semi-oval */
	    (this)->FillArcSize(x,y,2*cornerWidth,height,0,-180,Tile);
	    /* Fill vertical rectangle */
	    (this)->FillRectSize(x+cornerWidth,y,width-2*cornerWidth,height,Tile);
	    /* Fill right semi-oval */
	    (this)->FillArcSize(x+width-2*cornerWidth,y,2*cornerWidth,height,0,180,Tile);
	 }
	 else { /* assuming (2*cornerWidth >= width) */
	    /* Fill top semi-oval */
	    (this)->FillArcSize(x,y,width,2*cornerHeight,-90,180,Tile);
	    /* Fill horizontal rectangle */
	    (this)->FillRectSize(x,y+cornerHeight,width,height-2*cornerHeight,Tile);
	    /* Fill bottom semi-oval */
	    (this)->FillArcSize(x,y+height-2*cornerHeight,width,2*cornerHeight,90,180,Tile);
	 }
         return;
    }

    {   XArc FourCorners[4];
    XRectangle InteriorRects[3];
    int i;
    long realX, realY, realEndX, realEndY;

    /* First change the pattern to match the tile (we should special case white and black for efficiency) */
    if(xgraphic_SetupFillGC(this, Tile)) {
	/*    XSetTile(xgraphic_XDisplay(self),
	 xgraphic_XFillGC(self),
	 Tile->localWindow,0,0); */

	/* Now, we have to fill in four arcs and three rectangles */

	realX = physical_LogicalXToGlobalX(this,x);
	realY = physical_LogicalYToGlobalY(this,y);
	realEndX = realX + width;
	realEndY = realY + height;
	for (i=0;i<4;i++) {
	    FourCorners[i].width = 2 * cornerWidth;
	    FourCorners[i].height = 2* cornerHeight;
	    FourCorners[i].angle2 = -90 << 6; /* make a clockwise right angle */
	}
	FourCorners[0].x = realX;
	FourCorners[0].y = realY;
	FourCorners[0].angle1 = 180 * 64; /* 9 o'clock */
	FourCorners[1].x = realEndX-2*cornerWidth;
	FourCorners[1].y = realY;
	FourCorners[1].angle1 = 90 * 64; /* 12 o'clock */
	FourCorners[2].x = realEndX-2*cornerWidth;
	FourCorners[2].y = realEndY-2*cornerHeight;
	FourCorners[2].angle1 = 0; /* 3 o'clock */
	FourCorners[3].x = realX;
	FourCorners[3].y = realEndY-2*cornerHeight;
	FourCorners[3].angle1 = -90 * 64; /* 6 o'clock */
	XFillArcs((this)->XDisplay(), (this)->XWindow(), (this)->XFillGC(), FourCorners, 4);

	InteriorRects[0].x = realX+cornerWidth;
	InteriorRects[0].y = realY;
	InteriorRects[0].width = width - 2*cornerWidth;
	InteriorRects[0].height = height;
	InteriorRects[1].x = realX;
	InteriorRects[1].y = realY+cornerHeight;
	InteriorRects[1].width = cornerWidth;
	InteriorRects[1].height = height - 2* cornerHeight;
	InteriorRects[2].x = realEndX - cornerWidth;
	InteriorRects[2].y = realY+cornerHeight;
	InteriorRects[2].width = cornerWidth;
	InteriorRects[2].height = height - 2 * cornerHeight;
	XFillRectangles((this)->XDisplay(), (this)->XWindow(), (this)->XFillGC(), InteriorRects, 3);
    }
    }

}

void xgraphic::FillRgn(class region  * Rgn,class graphic *Tile)
{
    class region * tmpRegion;
    class region * visRegion;
    struct xgraphic_UpdateBlock * curBlock;

/*    VerifyUpdateClipping(self); */ /* unneeded here since we will explicitly pull out the update region for our graphics hack */

    /* First change the pattern to match the tile (we should special case white and black for efficiency) */
    if(xgraphic_SetupFillGC(this, Tile)) {

	/* Now we do something tricky: we set the clipping rectangles of the GC to match those of the region and then fill everything. The steps are:
	 1. Copy the region
	 2. Intersect with visual rectangle
	 3. Intersect with clipping rectangle
	 4. Offset to physical rectangle
	 5. set to clipping gc
	 6. fill everything
	 7. toss extra region
	 8. restore clipping rectangle
	 */
	/* Step 1 - make a copy */
	tmpRegion = (Rgn)->DuplicateRegion();

	/* Step 2 - clip to visual */
	if (this->visualRegion == NULL) {
	    visRegion = region::CreateRectRegion(&this->visualBounds);
	    (tmpRegion)->IntersectRegion(visRegion,tmpRegion);
	    delete visRegion;
	}
	else {
	    (tmpRegion)->IntersectRegion( this->visualRegion, tmpRegion);
	}

	/* Step 3 - clip to graphic clipping */
	if (this->clippingRegion != NULL) {
	    (tmpRegion)->IntersectRegion( this->clippingRegion, tmpRegion);
	}

	/* Step 4 - offset by physical coordinates */
	(tmpRegion)->OffsetRegion( physical_LogicalXToGlobalX(this,0), physical_LogicalYToGlobalY(this,0));

#if 0  // disable updateRegionInUse; see comments in xgraphic_LocalSetClippingRect below for why
	/* Step 4a Check for compounding update regions */
	curBlock = xgraphic_FindUpdateBlock((this)->XDisplay(), (this)->XWindow());
	if (curBlock->updateRegionInUse) XIntersectRegion(tmpRegion->regionData, curBlock->updateRegionInUse, tmpRegion->regionData);
#else
	curBlock = NULL;
#endif

	/* Step 5 - installing clipping rectangles */
	XSetRegion((this)->XDisplay(), (this)->XFillGC(), tmpRegion->regionData);

	/* Step 6 - fill entire visual */
	XFillRectangle((this)->XDisplay(),
		       (this)->XWindow(),
		       (this)->XFillGC(),
		       point_X(&this->physicalOrigin),
		       point_Y(&this->physicalOrigin),
		       (this)->GetVisualWidth(),
		       (this)->GetVisualHeight());

	/* reset the clipping rectangle */
	xgraphic_LocalSetClippingRect(this,curBlock);

	/* throw away temp region */
	delete tmpRegion;
    }
}

void xgraphic::FillTrapezoid(long  topX , long  topY , long  topWidth , long  bottomX , long  bottomY , long  bottomWidth , class graphic  *Tile)
{
    XPoint PolygonPts[4];

    VerifyUpdateClipping(this);

    PolygonPts[0].x = physical_LogicalXToGlobalX(this, topX);
    PolygonPts[0].y = physical_LogicalYToGlobalY(this, topY);
    PolygonPts[1].x = physical_LogicalXToGlobalX(this, topX+topWidth);
    PolygonPts[1].y = physical_LogicalYToGlobalY(this, topY);
    PolygonPts[2].x = physical_LogicalXToGlobalX(this, bottomX+bottomWidth);
    PolygonPts[2].y = physical_LogicalYToGlobalY(this, bottomY);
    PolygonPts[3].x = physical_LogicalXToGlobalX(this, bottomX);
    PolygonPts[3].y = physical_LogicalYToGlobalY(this, bottomY);
	
    /* First change the pattern to match the tile (we should special case white and black for efficiency) */
    if(xgraphic_SetupFillGC(this, Tile)) {
	XFillPolygon((this)->XDisplay(), (this)->XWindow(), (this)->XFillGC(), PolygonPts, 4, Convex, CoordModeOrigin);
    }
}


void xgraphic::BitBlt(struct rectangle  * SrcRect, class graphic  *DstGraphic, struct point  * DstOrigin, struct rectangle  * ClipRect)
{
    if (rectangle_Width(SrcRect) != 0 && rectangle_Height(SrcRect) != 0)  {

	VerifyUpdateClipping(this);
	VerifyUpdateClipping((class xgraphic *)DstGraphic);
	if (bltDebug) {
	    printf("SrcWin: %ld DestWin: %ld\n",
		   (this)->XWindow(),
		   ((class xgraphic *)DstGraphic)->XWindow());
	    printf("src (%d,%d)  %ud x %ud  dest (%d,%d)\n",
		  (int)physical_LogicalXToGlobalX(this,rectangle_Left(SrcRect)),
		  (int)physical_LogicalYToGlobalY(this,rectangle_Top(SrcRect)),
		  (unsigned int)rectangle_Width(SrcRect),
		  (unsigned int)rectangle_Height(SrcRect),
		  (int)physical_LogicalXToGlobalX(DstGraphic,point_X(DstOrigin)),
		  (int)physical_LogicalYToGlobalY(DstGraphic,point_Y(DstOrigin)));
	}
	if(this->flipforstipple) {
	    XSetFunction((this)->XDisplay(), ((class xgraphic *)DstGraphic)->XGC(), GXcopy);
	}
	XCopyArea((this)->XDisplay(), (this)->XWindow(), ((class xgraphic *)DstGraphic)->XWindow(),
		  ((class xgraphic *)DstGraphic)->XGC(),
		  (int)physical_LogicalXToGlobalX(this,rectangle_Left(SrcRect)),
		  (int)physical_LogicalYToGlobalY(this,rectangle_Top(SrcRect)),
		  (unsigned int)rectangle_Width(SrcRect),
		  (unsigned int)rectangle_Height(SrcRect),
		  (int)physical_LogicalXToGlobalX(DstGraphic,point_X(DstOrigin)),
		  (int)physical_LogicalYToGlobalY(DstGraphic,point_Y(DstOrigin)));
	if(this->flipforstipple) {
	    XSetFunction((this)->XDisplay(), ((class xgraphic *)DstGraphic)->XGC(), GXcopyInverted);
	}
	(this)->FlushGraphics();
    }
}

void xgraphic::SetBitAtLoc(long  XPos ,long  YPos,boolean  NewValue )
{

    VerifyUpdateClipping(this);

/* slimy, but works. We reuse the fill gc, set the mode to copy, the fill style to solid and set the pixel as appropriate. The mode gets reset by the localtransfer calculation, and the fill mode gets reset whenever encessary by the next fill operation */

    XSetFunction((this)->XDisplay(),
	    (this)->XFillGC(),
	    GXcopy);

    XSetFillStyle((this)->XDisplay(), (this)->XFillGC(), FillSolid);
    this->lastFillStyle = FillSolid;

    if (NewValue == 0) this->lastFillPixel = this->backgroundpixel;
    else if (NewValue == 1) this->lastFillPixel = this->foregroundpixel;
    else { /* error */
    }

    XSetForeground((this)->XDisplay(), (this)->XFillGC(), this->lastFillPixel);
    
    xgraphic_LocalSetTransferFunction(this, graphic_COPY);
}

static XImage *PixImage = NULL;
static Display *PixDisplay = NULL;

static void
SetUpPixImage(class xgraphic  *self, class pixelimage  *pixelimage)
		{
    VerifyUpdateClipping(self);

    if(!PixImage || (self)->XDisplay() != PixDisplay) {
	/* make a new PixImage */
	if (PixImage != NULL) {
	    XDestroyImage(PixImage);
	}
	PixDisplay = (self)->XDisplay();
	PixImage = XCreateImage(PixDisplay, DefaultVisualOfScreen( DefaultScreenOfDisplay(PixDisplay)), 1, XYBitmap, 0, NULL, 0, 0, 16, 0);
	/* XXX ? Should the following affect the routines in the PixImage ??? */
	PixImage->bitmap_bit_order = MSBFirst;
	PixImage->byte_order = MSBFirst;
    }
    /* fill in the PixImage */
    PixImage->width = pixelimage->pixelsPerRow;
    PixImage->height = pixelimage->numRows;
    PixImage->bytes_per_line = pixelimage->RowWidth;
    PixImage->data = (char *)pixelimage->bits;
}
	

void 
xgraphic::WritePixImage(long  DestX , long  DestY , class pixelimage  *SrcPixels, long  SrcX , long  SrcY , long  width , long  height)
			{
    if (width > 0 && height > 0) {
	VerifyUpdateClipping(this);

	SetUpPixImage(this, SrcPixels);
	XPutImage(PixDisplay, 
		  (this)->XWindow(),
		  (this)->XGC(),
		  PixImage,
		  SrcX, SrcY, 
		  physical_LogicalXToGlobalX(this, DestX),
		  physical_LogicalYToGlobalY(this, DestY),
		  width, height);
    }
}
	
void 
xgraphic::ReadPixImage(long  SrcX , long  SrcY , class pixelimage  *DestPixels, long  DestX , long  DestY , long  width , long  height)
{
    VerifyUpdateClipping(this);

    SetUpPixImage(this, DestPixels);
    /* The following will not necessarily work; it can fail and generate an X error. The necessary conditions are that the window be mapped and that the subrect requested be within the window bounds, or that the drawable be an off-screen pixmap.  */
    XGetSubImage(PixDisplay, 
		  (this)->XWindow(),
		  physical_LogicalXToGlobalX(this, SrcX), physical_LogicalYToGlobalY(this, SrcY), width, height, 
		  1,  /* mask for only one plane,  XXX I hope.*/
		  XYPixmap, PixImage,  DestX, DestY);
}

/* find the best pixmap depth supported by the server for a particular
 * visual and return that depth.
 *
 * this is complicated by R3's lack of XListPixmapFormats so we fake it
 * by looking at the structure ourselves.
 */

static unsigned int 
bitsPerPixelAtDepth(Display       *disp, int            scrn, unsigned int depth)
               {
#ifdef PRE_X11R4_ENV
  unsigned int a;

  for (a= 0; a < disp->nformats; a++)
    if (disp->pixmap_format[a].depth == depth)
      return(disp->pixmap_format[a].bits_per_pixel);

#else /* the way things should be */
  XPixmapFormatValues *xf;
  int nxf, a;

  xf = XListPixmapFormats(disp, &nxf);
  for (a = 0; a < nxf; a++)
      if (xf[a].depth == (int)depth) {
	  unsigned int bpp=xf[a].bits_per_pixel;
	  XFree((char *)xf);
	  return bpp;
      }
  XFree((char *)xf);
#endif

  /* this should never happen; if it does, we're in trouble
   */

  fprintf(stderr, "bitsPerPixelAtDepth: Can't find pixmap depth info!\n");
  return 1;
}

struct pixelmap {
    unsigned int redcolors, greencolors, bluecolors;
    int firstredbit, firstgreenbit, firstbluebit;
    unsigned long red_mask, green_mask, blue_mask;
    unsigned int bits_per_pixel;
};

static void ExplodePixelMasks(pixelmap *pm, unsigned long red_mask, unsigned long green_mask, unsigned long blue_mask) {

    pm->red_mask=red_mask;
    pm->green_mask=green_mask;
    pm->blue_mask=blue_mask;
    pm->firstredbit=pm->firstgreenbit=pm->firstbluebit=(-1);
    pm->redcolors = pm->greencolors = pm->bluecolors = 1;
    pm->bits_per_pixel=0;
    int i=0;
    for (unsigned long pixval = 1; pixval; i++, pixval <<= 1) {
	if (pixval & red_mask) {
	    if(pm->firstredbit<0) pm->firstredbit=i;
	    pm->redcolors <<= 1;
	    pm->bits_per_pixel++;
	}
	if (pixval & green_mask) {
	    if(pm->firstgreenbit<0) pm->firstgreenbit=i;
	    pm->greencolors <<= 1;
	    pm->bits_per_pixel++;
	}
	if (pixval & blue_mask) {
	    if(pm->firstbluebit<0) pm->firstbluebit=i;
	    pm->bluecolors <<= 1;
	    pm->bits_per_pixel++;
	}
    }
    
}

/***********************************/
XImageInfo *
imageToXImage( class xgraphic  *self, class image  *image, unsigned int private_cmap, unsigned int fit )
                { 
/***********************************/
  Display *disp;
  int scrn;
  Visual *visual; /* visual to use */
  Pixel *redvalue, *greenvalue, *bluevalue;
  unsigned int linelen, dpixlen, dbits;
  unsigned int x, y, a, b;
  unsigned short red, green, blue;
  class xcolor *xc;
  XImageInfo *ximageinfo;
  unsigned int ddepth; /* depth of the visual to use */
  class xcolormap **cmap = (class xcolormap**) (self)->CurrentColormap();

  if (cmap == NULL)
      return NULL;
  disp = (self)->XDisplay();
  scrn = DefaultScreen(disp);

  visual = DefaultVisual(disp, scrn);
  ddepth = DefaultDepth(disp, scrn);

  redvalue = greenvalue = bluevalue = NULL;
  if(self->index) {
      self->numColorsInIndex = 0;
      free(self->index);
      self->index=NULL;
  }
  ximageinfo = (XImageInfo*) malloc(sizeof(XImageInfo));
  ximageinfo->disp = disp;
  ximageinfo->scrn = scrn;
  ximageinfo->depth = 0;
  ximageinfo->drawable = (self)->XWindow();
  ximageinfo->foreground = ximageinfo->background = 0;
  ximageinfo->cmap = (*cmap)->XColorMap;
  ximageinfo->gc = NULL;
  ximageinfo->ximage = NULL;

  /* process image based on type of visual we're sending to
   */

  switch (image->type) {
      case ITRUE:
	  switch (visual->c_class) {
	      case TrueColor:
	      case DirectColor:
		  /* goody goody */
		  break;
	      default:
		  if (visual->bits_per_rgb > 1)
		      image = (image)->Reduce( visual->map_entries);
		  else
		      image = (image)->Dither();
	  }
	  break;
      case IGREYSCALE:
      case IRGB:
	  switch(visual->c_class) {
	      case TrueColor:
	      case DirectColor:
		  /* no problem, we handle this just fine */
		  break;
	      default:
		  if (visual->bits_per_rgb > 1 && visual->map_entries > 2)
		      image = (image)->Reduce(visual->map_entries);
		  else
		      image = (image)->Dither();
		  break;
	  }
      case IBITMAP:
	  /* no processing ever needs to be done for bitmaps */
	  break;
  }

  /* do color allocation */
  if(visual->map_entries > 2) {
      switch (visual->c_class) {
	  case TrueColor:
	      {
	      struct pixelmap pm;
	      redvalue = (Pixel *) malloc(sizeof(Pixel) * 256);
	      greenvalue = (Pixel *) malloc(sizeof(Pixel) * 256);
	      bluevalue = (Pixel *) malloc(sizeof(Pixel) * 256);

/* calculate number of distinct colors in each band */

	      ExplodePixelMasks(&pm, visual->red_mask, visual->green_mask, visual->blue_mask);
	      
	  /* sanity check
	   */

	      if( ((int)pm.redcolors > visual->map_entries) ||
		 ((int)pm.greencolors > visual->map_entries) ||
		 ((int)pm.bluecolors > visual->map_entries)) {
		  if(imageDebug)
		      fprintf(stderr, "Warning: inconsistency in color information (this may be ugly)\n");
	      }

	      for(a=0;a<256;a++) {
		  redvalue[a]=((pm.redcolors*a)/256)<<pm.firstredbit;
		  greenvalue[a]=((pm.greencolors*a)/256)<<pm.firstgreenbit;
		  bluevalue[a]=((pm.bluecolors)*a/256)<<pm.firstbluebit;
	      }
	      }
	      break;

	  default:
	      retry: /* this tag is used when retrying because we couldn't get a fit */
		if(self->index) free(self->index);
	      self->index = (Pixel *) malloc(sizeof(Pixel) * image->rgb->used);

 /* either create a new colormap or fit the image into the one we
 * have.  to create a new one, we create a private cmap and allocate
 * the colors writable.  fitting the colors is harder, we have to:
 *  1. grab the server so no one can goof with the colormap.
 *  2. count the available colors using XAllocColorCells.
 *  3. free the colors we just allocated.
 *  4. reduce the depth of the image to fit.
 *  5. allocate the colors again shareable.
 *  6. ungrab the server and continue on our way.
 * someone should shoot the people who designed X color allocation.
 */

	      if (private_cmap) {

		  if (fit)
		      XGrabServer(disp);

		  for (a = 0; a < image->rgb->used; a++, self->numColorsInIndex++) 
		  /* count entries we've got */
		      if (! XAllocColorCells(disp, ximageinfo->cmap, False, NULL, 0, self->index + a, 1))
			  break;

		  if (a > 0) {
		      XFreeColors(disp, ximageinfo->cmap, self->index, a, 0);
		      memset(self->index, 0, self->numColorsInIndex * sizeof(Pixel));
		      self->numColorsInIndex = 0;
		  }
		  
		  if (a == 0) {
		      free(self->index);
		      self->index = NULL;
		      self->numColorsInIndex = 0;
		      return(NULL);
		  }
		  
		  if (a <= 2 && image->rgb->used>2) {
		      image = (image)->Dither();
		      fit = 0;
		      free(self->index);
		      self->index = NULL;
		      self->numColorsInIndex = 0;
		      goto retry;
		  }

		  if (a < image->rgb->used)
		      image = (image)->Reduce(a);

#define XStandardPixel(map, cols, i, shft) ((map)->base_pixel + \
  (((*(cols->red + i)>>8) * (map)->red_max + (1<<(shft-1))) >> shft) * \
		  (map)->red_mult + \
  (((*(cols->green + i)>>8) * (map)->green_max + (1<<(shft-1))) >> shft) * \
		  (map)->green_mult + \
  (((*(cols->blue + i)>>8) * (map)->blue_max + (1<<(shft-1))) >> shft) * \
		  (map)->blue_mult)

		  if (fit) {
		      for(a = 0; a < image->rgb->used; a++, self->numColorsInIndex++)
			  *(self->index + a) = 
			    (*cmap)->CubePixel(*(image->rgb->red + a),
				    *(image->rgb->green + a),
				    *(image->rgb->blue + a));
		      XUngrabServer(disp);
		  }
		  else {
		      for (b = 0; b < a; b++) {
			  red = *(image->rgb->red + b);
			  green = *(image->rgb->green + b);
			  blue = *(image->rgb->blue + b);
			  xc = (class xcolor *)(*cmap)->Alloc( red, green, blue);
			  if(xc)
			      *(self->index + b) = (xc)->PixelRef();
		      }
		  }
	      }
	      else if ((visual == DefaultVisual(disp, scrn)) ||
		       (visual->c_class == StaticGray) ||
		       (visual->c_class == StaticColor)) {

/* allocate colors shareable (if we can)
 */
		  for  (a = 0; a < image->rgb->used; a++)
		      *(self->index + a) = 
			(*cmap)
			->CubePixel(
				    *(image->rgb->red + a),
				    *(image->rgb->green + a),
				    *(image->rgb->blue + a));
	      }
	      break;
      }
  }
/* create an XImage and related colormap based on the image type
 * we have.
 */

  switch(image->type) {
      case IBITMAP:
	  { byte *data;
	  int destlinelen = (image->width / 8) + (image->width % 8 ? 1 : 0);

	  data = (byte*) malloc(destlinelen * image->height);
	  memmove(data, (byte*) image->data, destlinelen * image->height);
	  ximageinfo->ximage = XCreateImage(disp, visual, 1, XYBitmap, 0, (char *)data, image->width, image->height, 8, 0);
	  ximageinfo->depth = ddepth;
	  ximageinfo->foreground = self->foregroundpixel;
	  ximageinfo->background = self->backgroundpixel;
	  ximageinfo->ximage->bitmap_bit_order = MSBFirst;
	  ximageinfo->ximage->byte_order = MSBFirst;
	  break;
	  }

      case IGREYSCALE:
      case IRGB:
      case ITRUE:

	  /* modify image data to match visual and colormap
	   */

	  dbits = bitsPerPixelAtDepth(disp, scrn, ddepth);
	  ximageinfo->depth = ddepth;
	  dpixlen= (dbits + 7) / 8;

	  switch (visual->c_class) {
	      case TrueColor:
		  { byte *data, *destptr, *srcptr;
		  Pixel pixval, newpixval;

		  ximageinfo->ximage = XCreateImage(disp, visual, ddepth, ZPixmap, 0, NULL, image->width, image->height, 8, 0);
		  data = (byte*) malloc(image->width * image->height * dpixlen);
		  ximageinfo->ximage->data = (char *)data;
		  destptr = data;
		  srcptr = image->data;
		  switch (image->type) {
		      case ITRUE:
			  for (y = 0; y < image->height; y++)
			      for (x = 0; x < image->width; x++) {
				  pixval = memToVal(srcptr, image->pixlen);
				  newpixval = redvalue[TRUE_RED(pixval)] |
				    greenvalue[TRUE_GREEN(pixval)] | bluevalue[TRUE_BLUE(pixval)];
				  valToMem(newpixval, destptr, dpixlen);
				  srcptr += image->pixlen;
				  destptr += dpixlen;
			      }
			  break;
		      case IGREYSCALE:
		      case IRGB:
			  for (y = 0; y < image->height; y++)
			      for (x = 0; x < image->width; x++) {
				  pixval = memToVal(srcptr, image->pixlen);
				  pixval = redvalue[image->rgb->red[pixval] >> 8] |
				    greenvalue[image->rgb->green[pixval] >> 8] |
				    bluevalue[image->rgb->blue[pixval] >> 8];
				  valToMem(pixval, destptr, dpixlen);
				  srcptr += image->pixlen;
				  destptr += dpixlen;
			      }
			  break;
		      default: /* something's broken */
			  return NULL;
		  }
		  ximageinfo->ximage->byte_order = MSBFirst;
		  break;
		  }
	      default:

/* only IRGB images make it this far.
 */

/* if our XImage doesn't have modulus 8 bits per pixel, it's unclear
 * how to pack bits so we instead use an XYPixmap image.  this is
 * slower.
 */

		  if (dbits % 8) {
		      byte *data, *destdata, *destptr, *srcptr, mask;
		      Pixel pixmask, pixval;

		      ximageinfo->ximage = XCreateImage(disp, visual, ddepth, XYPixmap, 0, NULL, image->width, image->height, 8, 0);
		      data = (byte *)malloc(image->width * image->height * dpixlen);
		      ximageinfo->ximage->data = (char *)data;
		      memset(data, 0, image->width * image->height * dpixlen);
		      ximageinfo->ximage->bitmap_bit_order = MSBFirst;
		      ximageinfo->ximage->byte_order = MSBFirst;
		      linelen = (image->width + 7) / 8;
		      for (a = 0; a < dbits; a++) {
			  pixmask = 1 << a;
			  destdata = data + ((dbits - a - 1) * image->height * linelen);
			  srcptr = image->data;
			  for (y = 0; y < image->height; y++) {
			      destptr = destdata + (y * linelen);
			      *destptr = 0;
			      mask = 0x80;
			      for (x = 0; x < image->width; x++) {
				  pixval= memToVal(srcptr, image->pixlen);
				  srcptr += image->pixlen;
				  if (self->index[pixval] & pixmask)
				      *destptr |= mask;
				  mask >>= 1;
				  if (mask == 0) {
				      mask = 0x80;
				      destptr++;
				  }
			      }
			  }
		      }
		  }
		  else {
		      byte *data, *srcptr, *destptr;

		      ximageinfo->ximage = XCreateImage(disp, visual, ddepth, ZPixmap, 0, NULL, image->width, image->height, 8, 0);
		      dpixlen = (ximageinfo->ximage->bits_per_pixel + 7) / 8;
		      data = (byte*) malloc(image->width * image->height * dpixlen);
		      ximageinfo->ximage->data = (char *)data;
		      ximageinfo->ximage->byte_order = MSBFirst;
		      srcptr = image->data;
		      destptr = data;
		      for (y = 0; y < image->height; y++)
			  for (x = 0; x < image->width; x++) {
			      valToMem(self->index[memToVal(srcptr, image->pixlen)], destptr, dpixlen);
			      srcptr += image->pixlen;
			      destptr += dpixlen;
			  }
		  }
		  break;
	  }
  }
  if (redvalue) {
    free((byte *)redvalue);
    free((byte *)greenvalue);
    free((byte *)bluevalue);
  }
  return(ximageinfo);
}


static void DestroyXImageInfo(xgraphic *self) {
    if(self->ximageinfo && self->ximageinfo->ximage) {
	XDestroyImage(self->ximageinfo->ximage);
	free(self->ximageinfo);
	self->ximageinfo=NULL;
    }
}

static void
SetUpXImage( class xgraphic  *self, class image  *image )
        { static int private_cmap = FALSE;
  static int fit = FALSE;
  Display *dpy = (self)->XDisplay();
  int scrn = DefaultScreen(dpy);
  class xcolormap **cmap = (class xcolormap**) (self)->CurrentColormap();

  if (cmap == NULL)
      return;
  private_cmap = (DefaultColormap(dpy,scrn) != (*cmap)->XColorMap);
  fit = imagePrefs.alwaysFitFixed;
  VerifyUpdateClipping(self);
 
  if ((self)->XDisplay() != PixDisplay) {
      if (self->ximageinfo) DestroyXImageInfo(self);
      PixDisplay = (Display *) (self)->XDisplay();
  }
  if(!image->inited || !self->ximageinfo) {
      if(self->ximageinfo) DestroyXImageInfo(self);
      
      if((self->ximageinfo = imageToXImage(self, image, private_cmap, fit))) {
	  image->inited = TRUE;
  }
      }
}

/* Given an XImage and a drawable, move a rectangle from the Ximage
 * to the drawable.
 */

void 
sendXImage( class xgraphic  *self, XImageInfo   *ximageinfo, int           src_x , int           src_y , int           dst_x , int           dst_y, unsigned int w , unsigned int h )
                {
  ximageinfo->gc = (self)->XGC();
  ximageinfo->drawable = (self)->XWindow();
  XPutImage(ximageinfo->disp, ximageinfo->drawable, ximageinfo->gc,
	    ximageinfo->ximage, src_x, src_y, dst_x, dst_y, w, h);
}

void 
xgraphic::WriteImage(long  DestX , long  DestY , class image  *image, long  SrcX , long  SrcY , long  width , long  height)
            {
    if ((width>0) && (height>0)) {
	VerifyUpdateClipping(this);

	SetUpXImage(this, image);
	if(image->inited && this->ximageinfo)
	    sendXImage(this, this->ximageinfo, SrcX, SrcY, physical_LogicalXToGlobalX(this, DestX), physical_LogicalYToGlobalY(this, DestY), width, height);
    }
}

void 
xgraphic::WriteImage(long  DestX , long  DestY , ddimage &image, long  SrcX , long  SrcY , long  width , long  height)
{
    if ((width>0) && (height>0)) {
	VerifyUpdateClipping(this);
	xddimage *di=(xddimage *)image.IDDImage();
	if(di) {
	    image.Draw(this, SrcX, SrcY, width, height, DestX, DestY);
	}
    }
}

static int AllocateImageColor(image *img, unsigned short r, unsigned short g, unsigned short b) {
    int i=img->RGBUsed()-1;
    while(i>=0) {
	if(img->RedPixel(i)==r && img->GreenPixel(i)==g && img->BluePixel(i)==b) return i;
	i--;
    }
    if(img->RGBUsed()>=img->RGBSize()) {
	img->Compress();
	if(img->RGBUsed()>=img->RGBSize()) {
	    int bestdiff=0, best=(-1);
	    unsigned int k;
	    for(k=0;k<img->RGBUsed();k++) {
		int rd=(img->RedPixel(k)-r)>>8;
		int gd=(img->GreenPixel(k)-g)>>8;
		int bd=(img->BluePixel(k)-b)>>8;
		int diff=rd*rd+gd*gd+bd*bd;
		if(best<0 || diff<bestdiff) {
		    bestdiff=diff;
		    best=k;
		}
	    }
	    return k;
	}
    }
    int j=img->RGBUsed();
    img->RedPixel(j)=r;
    img->GreenPixel(j)=g;
    img->BluePixel(j)=b;
    img->RGBUsed()++;
    return j;
}

void xgraphic::ReadImage(long SrcX, long SrcY, class image *img, long DestX, long DestY, long width, long height) {

    struct pixelmap pm;
    Visual *v=DefaultVisual(XDisplay(), XScreen());

    XImage *ximg=XGetImage(XDisplay(), XWindow(), SrcX, SrcY, width, height, ~0, ZPixmap);
    if(ximg==NULL) {
	fprintf(stderr, "ReadImage error: Couldn't get TrueColor image.\n");
	return;
    }
    if(ximg->red_mask==0) {
	ximg->red_mask=v->red_mask;
	ximg->green_mask=v->green_mask;
	ximg->blue_mask=v->blue_mask;
    }
    ExplodePixelMasks(&pm, ximg->red_mask, ximg->green_mask, ximg->blue_mask);
    
    if((v->c_class==TrueColor || v->c_class==DirectColor) && (pm.redcolors<2 || pm.greencolors<2 || pm.bluecolors<2)) {
	fprintf(stderr, "ReadImage error: Can't make sense of %dx%dx%d color map for a DirectColor or TrueColor display.\n", pm.redcolors, pm.greencolors, pm.bluecolors);
	XDestroyImage(ximg);
	return;
    }
	
    if(img->Data()==NULL) {
	img->Reset();
	switch(v->c_class) {
	    case TrueColor:
		img->newTrueImage(width+DestX, height+DestY);
		break;
	    case DirectColor:
		if(pm.bits_per_pixel>8) img->newTrueImage(width+DestX, height+DestY);
		else img->newRGBImage(width+DestX, height+DestY, 8);
		break;
	    default:
		if(v->map_entries<=256) {
		    img->newRGBImage(width+DestX, height+DestY, 8);
		    img->newRGBMapData(v->map_entries);
		} else {
		    img->newTrueImage(width+DestX, height+DestY);
		}
		break;
	}
    }
    
    if(!(TRUEP(img) || (RGBP(img) && img->Depth()==8))) {
	fprintf(stderr, "ReadImage error: Only TrueColor and 8 bit RGB images are supported.\n");
	XDestroyImage(ximg);
	return;
    }
    if(DestX+width>img->Width() || DestY+height>img->Height()) {
	fprintf(stderr, "ReadImage error: destination extends outside the image.\n");
	XDestroyImage(ximg);
	return;
    }

    switch(v->c_class) {
	case TrueColor: {
	    int x, y;
	    pm.redcolors--;
	    pm.greencolors--;
	    pm.bluecolors--;
	    if(TRUEP(img)) {
		for(y=0;y<height;y++) {
		    for(x=0;x<width;x++) {
			unsigned long pix=XGetPixel(ximg, x, y);
			unsigned char *p=img->Data() + ((DestY + y) * width * 3) + (DestX + x) * 3;
			p[0]=((pix & pm.red_mask ) >> pm.firstredbit ) * 255 / pm.redcolors;
			p[1]=(( pix & pm.green_mask ) >> pm.firstgreenbit ) * 255 / pm.greencolors;
			p[2]=(( pix & pm.blue_mask ) >> pm.firstbluebit) * 255 / pm.bluecolors;
		    }
		}
	    } else {
		for(y=0;y<height;y++) {
		    for(x=0;x<width;x++) {
			unsigned long pix=XGetPixel(ximg, x, y);
			unsigned char *p=img->Data() + ((DestY + y) * width) + (DestX + x);
			p[0]=AllocateImageColor(img, ((pix & pm.red_mask ) >> pm.firstredbit ) * 65535 / pm.redcolors, (( pix & pm.green_mask ) >> pm.firstgreenbit ) * 65535 / pm.greencolors, (( pix & pm.blue_mask ) >> pm.firstbluebit) * 65535 / pm.bluecolors);
		    }
		}
	    }

	    }
	    break;
	case DirectColor: {
	    if(pm.bits_per_pixel<=8) goto AnyOl8BitOrLessDisplay;
	    unsigned int maxcolors=MAX(MAX(pm.redcolors, pm.greencolors), pm.bluecolors);

	    unsigned int i;
	    int x, y;
	    XColor *pixels=(XColor *)malloc(sizeof(XColor)*maxcolors);
	    if(pixels==NULL) {
		fprintf(stderr, "ReadImage error: Couldn't get colormap.\n");
		XDestroyImage(ximg);
		return;
	    }
	    for(i=0;i<maxcolors;i++) {
		pixels[i].pixel=( (i<<pm.firstredbit) & pm.red_mask)| ((i<<pm.firstgreenbit)&pm.green_mask) |((i<<pm.firstbluebit)&pm.blue_mask);
	    }
	    XQueryColors(XDisplay(), ((xcolormap *)*CurrentColormap())->XColorMap, pixels, maxcolors);

	    if(TRUEP(img)) {
		for(y=0;y<height;y++) {
		    for(x=0;x<width;x++) {
			unsigned long pix=XGetPixel(ximg, x, y);
			unsigned char *p=img->Data() + ((DestY + y) * width * 3) + (DestX + x) * 3;
			p[0]=pixels[((pix & pm.red_mask ) >> pm.firstredbit)].red;
			p[1]=pixels[(( pix & pm.green_mask ) >> pm.firstgreenbit)].green;
			p[2]=pixels[(( pix & pm.blue_mask ) >> pm.firstbluebit)].blue;
		    }
		}
	    } else {
		for(y=0;y<height;y++) {
		    for(x=0;x<width;x++) {
			unsigned long pix=XGetPixel(ximg, x, y);
			unsigned char *p=img->Data() + ((DestY + y) * width) + (DestX + x);
			p[0]=AllocateImageColor(img, pixels[((pix & pm.red_mask ) >> pm.firstredbit)].red, pixels[(( pix & pm.green_mask ) >> pm.firstgreenbit)].green, pixels[(( pix & pm.blue_mask ) >> pm.firstbluebit)].blue);
		    }
		}
	    }
	    free(pixels);
	    }
	    break;
	default:
	    AnyOl8BitOrLessDisplay:
	    {
	    int i, x, y;
	    XColor *pixels=(XColor *)malloc(sizeof(XColor)*v->map_entries);
	    if(pixels==NULL) {
		fprintf(stderr, "ReadImage error: Couldn't get colormap.\n");
		XDestroyImage(ximg);
		return;
	    }
	    for(i=0;i<v->map_entries;i++) pixels[i].pixel=i;
	    XQueryColors(XDisplay(), ((xcolormap *)*CurrentColormap())->XColorMap, pixels, v->map_entries);
	    if(TRUEP(img)) for(y=0;y<height;y++) {
		for(x=0;x<width;x++) {
		    unsigned long pix=XGetPixel(ximg, x, y);
		    unsigned char *p=img->Data() + ((DestY + y) * width*3) + (DestX + x) * 3;
		    p[0]=pixels[pix].red>>8;
		    p[1]=pixels[pix].green>>8;
		    p[2]=pixels[pix].blue>>8;
		}
	    } else {
		int *map=(int *)malloc(sizeof(int) * v->map_entries);
		if(map==NULL) {
		    fprintf(stderr, "ReadImage error: Couldn't get colormap.\n");
		    free(pixels);
		    XDestroyImage(ximg);
		    return;
		}
		for(i=0;i<v->map_entries;i++) map[i]=(-1);
		for(y=0;y<height;y++) {
		    for(x=0;x<width;x++) {
			unsigned long pix=XGetPixel(ximg, x, y);
			unsigned char res;
			if(map[pix]>=0) res=map[pix];
			else {
			    res=AllocateImageColor(img, pixels[pix].red, pixels[pix].green, pixels[pix].blue);
			    map[pix]=res;
			}
			*(img->Data() + (DestY+y) * width + (DestX+x)) = res;
		    }
		}
		free(map);
	    }
	    free(pixels);
	    }

    }
    XDestroyImage(ximg);
}

void xgraphic_LocalSetClippingRect(class xgraphic  * self ,struct xgraphic_UpdateBlock  * updateBlk )
{
    struct rectangle Temp;
    XRectangle XRect[1];

    /* If the visualBounds rect is empty convert it to an empty rectangle X can grok. */
    if(self->visualBounds.width<0 || self->visualBounds.height<0) self->visualBounds.width=self->visualBounds.height=0;
    
/* The clipping rectangle for a graphic is the intersection of the visual bounds of the graphic, the updtae clipping rectangle, and any clipping rectangle provided by the client of the graphic */

    
    if (regionDebug) {
	printf("LocalSetClippingRect: entering with updateBlk %p\n",updateBlk);
    }

    /* First check to see if we have any kind of update region to contend with */

    if (!updateBlk) updateBlk = xgraphic_FindUpdateBlock((self)->XDisplay(), (self)->XWindow());

    /* Calculate the limiting rectangle based on visual bounds and any set rectangle (and turn them into physical coorindates) */

    if (self->visualRegion == NULL &&
	 self->clippingRegion == NULL &&
	 ! updateBlk->updateRegionInUse) {
	(self)->GetVisualBounds(&Temp);
	physical_LogicalToGlobalRect(self,&Temp);
	XRect[0].x = rectangle_Left(&Temp);
	XRect[0].y = rectangle_Top(&Temp);
	XRect[0].width = rectangle_Width(&Temp);
	XRect[0].height = rectangle_Height(&Temp);

	if (regionDebug) printf("localsetclip: no update rgn, ignoring it\n");
	XSetClipRectangles((self)->XDisplay(),
			   (self)->XGC(),
			   /* Clip origin */0,0,XRect,/* Num Rects */1,
			   YXBanded);
	XSetClipRectangles((self)->XDisplay(),
			   (self)->XFillGC(),
			   /* Clip origin */0,0,XRect,/* Num Rects */1,
			   YXBanded);
	if (regionDebug) printf("LocalSetClip: finished with clip and visual: x %ld, y %ld, width %ld, height %ld\n", rectangle_Left(&Temp), rectangle_Top(&Temp), rectangle_Width(&Temp), rectangle_Height(&Temp));
    }
    else {
	class region *clipRegion;

	if (self->visualRegion == NULL) {
	    clipRegion = region::CreateRectRegion(&self->visualBounds);
	}
	else {
	    clipRegion = new region;

	    region::CopyRegion(clipRegion, self->visualRegion);
	}

	/* If we have a clipping region, factor it in */

	if (self->clippingRegion != NULL)  {
	    (clipRegion)->IntersectRegion( self->clippingRegion, clipRegion);
	    if (regionDebug)
		self->clippingRegion->GetBoundingBox(&Temp);
	    if (regionDebug) printf("LocalSetClip: clip: x %ld, y %ld, width %ld, height %ld\n", rectangle_Left(&Temp), rectangle_Top(&Temp), rectangle_Width(&Temp), rectangle_Height(&Temp));
	}

	/* map it to physical space (X coordinates) */

	(clipRegion)->OffsetRegion( physical_LogicalXToGlobalX(self, 0),
			    physical_LogicalYToGlobalY(self, 0));

	if (regionDebug)
	    clipRegion->GetBoundingBox(&Temp);

	if (regionDebug) printf("LocalSetClip: finished with clip and visual: x %ld, y %ld, width %ld, height %ld\n", rectangle_Left(&Temp), rectangle_Top(&Temp), rectangle_Width(&Temp), rectangle_Height(&Temp));

	if (regionDebug) printf("LocalSetClip: Using block %p\n", updateBlk);

	if (regionDebug) printf("localsetclip: region counter in update block %ld, region %p\n", updateBlk->RegionCounter, updateBlk->updateRegionInUse);

	if (regionDebug && updateBlk->updateRegionInUse) {
	    XClipBox(updateBlk->updateRegionInUse, XRect);
	    printf("LocalSetClip: update region bounds %dx%d@%d,%d\n", XRect->width, XRect->height, XRect->x, XRect->y);
	}
	/* many objects change appearance outside of the "updateRegionInUse" on resize */
	/* e.g. borders need to be removed inside, and redrawn in new positions */
	/* e.g. text which wraps needs to be redrawn to new boundaries */
	/* for now, just update the whole region instead of just what was exposed */
	/* for non-resize updates, this is expensive, but I see no immediate easy fix */
	if (0 && updateBlk->updateRegionInUse) {
	    /* Intersect it with the update region */
	    XIntersectRegion((clipRegion)->GetRegionData(),
			     updateBlk->updateRegionInUse,
			     (clipRegion)->GetRegionData());
	    /* Set clipping rectangles */

	    if (regionDebug){
		printf("localsetclip: setting clip rect to intersected form\n");
		XClipBox((clipRegion)->GetRegionData(),XRect);
		printf("xgraphic: clipbox of final clipping region is %d, %d, %d, %d\n", XRect->x, XRect->y, XRect->width, XRect->height);
	    }
	}

	XSetRegion((self)->XDisplay(), (self)->XGC(), (clipRegion)->GetRegionData());
	XSetRegion((self)->XDisplay(), (self)->XFillGC(), (clipRegion)->GetRegionData());
	/* and toss temporary region */
        delete clipRegion;
    }

    self->lastUpdateRegionIDUsed = curUpdateRegionID;
}

void xgraphic::SetClippingRegion(class region  *region)
{
    /* Machine independent stuff */
    (this)->graphic::SetClippingRegion(region);
    /* Machine dependent actions */
    xgraphic_LocalSetClippingRect(this,NULL);
}

void xgraphic::SetClippingRect(struct rectangle  * AdditionalRect)
{
    /* Machine independent stuff */
    (this)->graphic::SetClippingRect(AdditionalRect);
    /* Machine dependent actions */
    xgraphic_LocalSetClippingRect(this,NULL);
}

void xgraphic::ClearClippingRect()
{
    /* Machine independent part */
    (this)->graphic::ClearClippingRect();
    /* Machine dependent */
    xgraphic_LocalSetClippingRect(this,NULL);
}

void xgraphic::SetLineWidth(short  NewLineWidth)
{   XGCValues tempGC;

    if ( (this )->GetLineWidth( ) != NewLineWidth )
    {
      (this)->graphic::SetLineWidth(  NewLineWidth );
      this->lineWidth = NewLineWidth;
      tempGC.line_width = NewLineWidth;

      /* special case 1 to use the underlying hardware (width 0); the performance hit is just too great for a real width of 1 */
      if (NewLineWidth == 1) tempGC.line_width = 0;

      XChangeGC((this)->XDisplay(),
		(this)->XGC(),
		GCLineWidth,&tempGC);
    }
}

void xgraphic::SetLineDash( const char		 *dashPattern, int		 dashOffset, short		 dashType )
{
    XGCValues tempGC;
    int	n = 0;
    const char	*p;
    short	type = dashType;
    char		*oldPattern = NULL;
    int			oldOffset;
    short		oldType;

    if ( dashPattern == NULL ) type = graphic_LineSolid;
    (this)->GetLineDash(  &oldPattern, &oldOffset, &oldType );
    if ( oldPattern && dashPattern && ( strcmp( oldPattern, dashPattern ) == 0 ) && dashOffset == oldOffset && type == oldType );
    else
    {
      (this)->graphic::SetLineDash(  dashPattern, dashOffset, type );
      switch( type )
      {
	case graphic_LineOnOffDash: tempGC.line_style = LineOnOffDash; break;
	case graphic_LineDoubleDash: tempGC.line_style = LineDoubleDash; break;
	case graphic_LineSolid:
        default: tempGC.line_style = LineSolid; break;
      }
      XChangeGC( (this)->XDisplay(), (this)->XGC(), GCLineStyle, &tempGC );
      if ( dashPattern )
      {
        p = dashPattern;
        while ( *p++ != 0 ) n++;
        if ( n ) XSetDashes( (this )->XDisplay( ), (this )->XGC( ), dashOffset, dashPattern, n );
      }
    }
    if(oldPattern) free(oldPattern);
}

void xgraphic::SetLineCap( short		 newLineCap )
{
    XGCValues tempGC;

    if ( (this )->GetLineCap( ) != newLineCap )
    {
      (this)->graphic::SetLineCap(  newLineCap );
      switch( newLineCap )
      {
	case graphic_CapNotLast: tempGC.cap_style = CapNotLast; break;
	case graphic_CapRound: tempGC.cap_style = CapRound; break;
	case graphic_CapProjecting: tempGC.cap_style = CapProjecting; break;
	case graphic_CapButt:
        default: tempGC.cap_style = CapButt; break;
      }
      XChangeGC( (this)->XDisplay(), (this)->XGC(), GCCapStyle, &tempGC );
    }
}

void xgraphic::SetLineJoin( short		 newLineJoin )
{
    XGCValues tempGC;

    if ( (this )->GetLineJoin( ) != newLineJoin )
    {
      (this)->graphic::SetLineJoin(  newLineJoin );
      switch( newLineJoin )
      {
          case graphic_JoinRound: tempGC.join_style = JoinRound; break;
          case graphic_JoinBevel: tempGC.join_style = JoinBevel; break;
	  case graphic_JoinMiter:
	  default: tempGC.join_style = JoinMiter; break;
      }
      XChangeGC( (this)->XDisplay(), (this)->XGC(), GCJoinStyle, &tempGC );
    }
}

void xgraphic::SetTransferMode(short  NewTransferMode)
{

    short prevValue = this->transferMode;
    this->transferMode = 0xFF & NewTransferMode;
    xgraphic_LocalSetTransferFunction(this, prevValue);
}

static void xgraphicClearGrayLevels(class xgraphic  *self)
     {
  int i;

  /* clear the gray_scale to gray_level mapping table maintained for monochrome displays */
  for (i = 0;  i <= 16;  i++)
    self->gray_levels[i] = NULL;
}

static void HandleInsertion(class xgraphic  *self, class graphic  *E)
{
    XGCValues tempGCValues;
    long tmpx, tmpy;
    class xgraphic *EnclosingGraphic=(class xgraphic *)E;
    /* First see if we are moving between windows or displays */
    /* We actually may be too aggressive here, but it is probably safe
       for now, e.g., movement within the same display/screen pair
       may not need a recreation of stuff, but there is no
       guarantee in X that one wil have the same window attributes,
       since an offscreen bitmap may have a differentdepth even if
       created on the same display/screen pair. */
    if (
        (self->localWindow != 
	    (EnclosingGraphic)->XWindow()) ||
	(self->displayUsed !=
	    (EnclosingGraphic)->XDisplay()) ||
	(self->screenUsed !=
	 (EnclosingGraphic)->XScreen() ) ) {

	/* Yep, we changed, everything you know is wrong! */
	if ((self)->XGC()) {
	    XFreeGC((self)->XDisplay(), (self)->XGC());
	    XFreeGC((self)->XDisplay(), (self)->XFillGC());
	    (self)->XGC() = NULL;
	    (self)->XFillGC() = NULL;
	}
	self->DisplayClassVal = E->DisplayClass();

	/* Copy over the machine dependent window capability */
	self->localWindow = (EnclosingGraphic)->XWindow();
	self->displayUsed = (EnclosingGraphic)->XDisplay();
	self->screenUsed =  (EnclosingGraphic)->XScreen();
	self->xfd = EnclosingGraphic->xfd;
    }
    self->valid = (EnclosingGraphic)->Valid();

    /* And pick up the appropriate gray shades for this display */
    xgraphicClearGrayLevels(self);
    self->gray_shades = xgraphic_FindGrayBlock((self)->XDisplay(),
	    (self)->XScreen());

    if ((self)->Valid())  {
	if (!(self)->XGC()) {

	    tempGCValues.line_width=self->lineWidth;
	    /* special case hardware (width 0) for line width of 1 */
	    if (tempGCValues.line_width == 1 ) tempGCValues.line_width = 0;
	    self->foregroundpixel = BlackPixel((self)->XDisplay(), (self)->XScreen());
	    tempGCValues.foreground = self->foregroundpixel;
	    self->backgroundpixel = WhitePixel((self)->XDisplay(), (self)->XScreen());
	    tempGCValues.background = self->backgroundpixel;
#ifdef PLANEMASK_ENV
	    tempGCValues.plane_mask = tempGCValues.foreground ^ tempGCValues.background;

	    (self)->XGC() = XCreateGC((self)->XDisplay(),
					   (self)->XWindow(),  GCLineWidth | GCForeground | GCBackground | GCPlaneMask, &tempGCValues);
#else /* PLANEMASK_ENV */
	    (self)->XGC() = XCreateGC((self)->XDisplay(),
					   (self)->XWindow(),  GCLineWidth | GCForeground | GCBackground, &tempGCValues);
#endif /* PLANEMASK_ENV */

	    tempGCValues.fill_style = FillSolid;
	    self->lastFillStyle = FillSolid;
	    self->lastFillPixel = self->foregroundpixel;
	    self->lastFillTile = NULL;

	    (self)->GetPatternOrigin( &tmpx, &tmpy);
	    tempGCValues.ts_x_origin = tmpx;
	    tempGCValues.ts_y_origin = tmpy;

#ifdef PLANEMASK_ENV
	    (self)->XFillGC() =
	      XCreateGC((self)->XDisplay(),
			(self)->XWindow(),GCFillStyle | GCForeground | GCBackground | GCPlaneMask | GCTileStipXOrigin | GCTileStipYOrigin, &tempGCValues);
#else /* PLANEMASK_ENV */
	    (self)->XFillGC() =
	      XCreateGC((self)->XDisplay(),
			(self)->XWindow(),GCFillStyle | GCForeground | GCBackground | GCTileStipXOrigin | GCTileStipYOrigin, &tempGCValues);

#endif /* PLANEMASK_ENV */

	    xgraphic_LocalSetTransferFunction(self, graphic_COPY);
	    ReallySetFont(self);

	}
	xgraphic_LocalSetClippingRect(self,NULL);
	self->SetForegroundColor(self->fore);
	self->SetBackgroundColor(self->back);
    }
}


void xgraphic::InsertGraphicSize(class graphic  * EnclosingGraphic, long  xOriginInParent ,
	long  yOriginInParent , long  width , long  height )
{
    struct rectangle r;

    rectangle_SetRectSize(&r,xOriginInParent, yOriginInParent,
	    width,height);
    (this)->InsertGraphic(EnclosingGraphic,&r);
}

void xgraphic::InsertGraphic(class graphic  * EnclosingGraphic, struct rectangle  * EnclosedRectangle)
{
    /* First do the machine independent stuff */

    (this)->graphic::InsertGraphic(EnclosingGraphic, EnclosedRectangle);

    HandleInsertion(this, EnclosingGraphic);
}

void xgraphic::InsertGraphicRegion(class graphic  * EnclosingGraphic, class region  * region)
{
    (this)->graphic::InsertGraphicRegion(EnclosingGraphic, region);

    HandleInsertion(this, EnclosingGraphic);
}

void xgraphic::SetVisualRegion(class region  *region)
{
    (this)->graphic::SetVisualRegion( region);

    xgraphic_LocalSetClippingRect(this,NULL);
}
    
void xgraphic::FlushGraphics()
{
    XFlush((this)->XDisplay());
}

void xgraphic::SetPatternOrigin(long  xpos , long  ypos)
{
    (this)->graphic::SetPatternOrigin( xpos, ypos);
    XSetTSOrigin((this)->XDisplay(), (this)->XFillGC(), xpos, ypos);
}

class graphic * xgraphic::WhitePattern()
{
    if (this->gray_levels[0] != NULL)
      return (class graphic *) this->gray_levels[0];
    else
    return (class graphic *) (this)->GrayPattern(0,16);
}

class graphic * xgraphic::BlackPattern()
{
    if (this->gray_levels[16] != NULL)
      return (class graphic *) this->gray_levels[16];
    else
    return (class graphic *) (this)->GrayPattern(16,16);
}

static class fontdesc *  xgraphic_shadeFont = NULL;
#define GETXFONT(self, graphic)  \
	((XFontStruct *)(self)->GetRealFontDesc( graphic))


static void CacheShades(class xgraphic  *self)
{
    class xgraphic * RetValue;
    long width, height;
    long x, y;
    XFontStruct *info;
    XCharStruct *ci;
    Pixmap newPixmap;
    GC gce=None, gcd=None;
    char str[1];
    XGCValues gcattr;
    Display *dpy=XOpenDisplay(DisplayString((self)->XDisplay()));
    Window root;
    char buf[1024], *pb=buf;
    long SpecialChar;
    Atom atk_shades;

    if(dpy==NULL) return;
    
    atk_shades=XInternAtom(dpy, "ATK_SHADES", FALSE);
    root=RootWindow(dpy, DefaultScreen(dpy));
    XSetCloseDownMode(dpy, RetainPermanent);
    
     /* The following is bogus: it should be a in-core graphic
      and created the same way -- insert it into a "universal pixmap"
      such as the root of the window and use ordinary graphic operations
      to fill in the values instead of going through the X versions.
      This should allow 1) transportability to other systems and
2) consistent semantics with other xgraphic objects */
   
    if (!xgraphic_shadeFont)
	xgraphic_shadeFont = fontdesc::Create("xshape",fontdesc_Plain,10);

    info = XLoadQueryFont(dpy, "xshape10"); /* should be freed but no good way to check for failure, since the return value is then random */
    if(info==NULL) {
	XCloseDisplay(dpy);
	return;
    }
    for(SpecialChar=0;SpecialChar<=16;SpecialChar++) {
	RetValue = new xgraphic;
	if(RetValue == NULL) {
	    XCloseDisplay(dpy);
	    return;
	}
	
	ci = (info->per_char != NULL) ? &(info->per_char[SpecialChar - info->min_char_or_byte2]) : &(info->max_bounds);
	width = ci->width;
	height = info->max_bounds.ascent + info->max_bounds.descent;
	/* Older code - no longer seems to work properly
	 width = ci->rbearing - ci->lbearing;
	 height = ci->descent + ci->ascent;
	 */
	x = 0;
	/* Older code - no longer seems to work properly
	 x = -ci->lbearing;
	 */
	y = ci->ascent;
	/* Note: we could have an empty character, in which case, we simulate it with a 1 by 1 character. Too bad X doesn't allow 0 sized pixmaps */
	if (!width) {
	    fprintf(stderr,"xfontdesc_CvtCharToGraphic: 0 width character %ld in %p\n", SpecialChar, self);
	    width++;
	}
	if (!height) {
	    fprintf(stderr,"xfontdesc_CvtCharToGraphic: 0 height character %ld in %p\n", SpecialChar, self);
	    height++;
	}
	newPixmap = XCreatePixmap(dpy, root, width, height, 1);
	if(gce == None) {
	    gcattr.fill_style = FillSolid;
	    gcattr.foreground = 0;
	    gce = XCreateGC(dpy, newPixmap, GCFillStyle | GCForeground, &gcattr);
	    gcattr.foreground = 1;
	    gcattr.font = info->fid;
	    gcattr.function = GXcopy;
	    gcd = XCreateGC(dpy, newPixmap, GCFunction | GCFont | GCFillStyle | GCForeground, &gcattr);

	}
	XFillRectangle(dpy, newPixmap, gce, 0, 0, width, height);
	str[0] = SpecialChar;
	XDrawString(dpy, newPixmap, gcd, x, y, str, 1);
	RetValue->localWindow = newPixmap;
	RetValue->displayUsed = (self)->XDisplay();
	RetValue->screenUsed = (self)->XScreen();
	RetValue->localGraphicContext = NULL;
	RetValue->localFillGraphicContext = NULL;
	self->gray_shades[SpecialChar] = RetValue;
	sprintf(pb, "%ld ", newPixmap);
	pb+=strlen(pb);
    }
    XChangeProperty(dpy, root, atk_shades, XA_STRING, 8, PropModeReplace, (unsigned char*)buf, strlen(buf));
    XFreeGC(dpy, gce);
    XFreeGC(dpy, gcd);
    XCloseDisplay(dpy);
}


static void GetShades(class xgraphic  *self)
{
    Display *dpy=(self)->XDisplay();
    Window root=RootWindow(dpy, DefaultScreen(dpy));
    Atom atk_shades=XInternAtom(dpy, "ATK_SHADES", FALSE);
    Atom RetAtom; 
    int RetFormat;
    unsigned long RetNumItems;
    unsigned long RetBytesAfter;
    unsigned char *RetData = NULL;

    if(XGetWindowProperty(dpy,root, atk_shades, 0L, 1024L, False, XA_STRING, &RetAtom, &RetFormat, &RetNumItems, &RetBytesAfter,  &RetData)!=Success || RetAtom==None) {
	CacheShades(self);
    } else {
	int i;
	char *p=(char *)RetData;
	for(i=0;i<=16;i++) {
	    self->gray_shades[i]=NULL;
	}
	for(i=0;i<=16 && p && *p;i++) {
	    class xgraphic *RetValue = new xgraphic;
	    Pixmap pix = atol(p);
	    p=::strchr(p, ' ');
	    RetValue->localWindow = pix;
	    RetValue->displayUsed = (self)->XDisplay();
	    RetValue->screenUsed = (self)->XScreen();
	    RetValue->localGraphicContext = NULL;
	    RetValue->localFillGraphicContext = NULL;
	    self->gray_shades[i] = RetValue;
	    if(p) p++;
	}
	if(i<17) {
	    fprintf(stderr, "xgraphic: warning: ATK_SHADES property too short!\n");
	}
	XFree((char *)RetData);
    }
}
static boolean cacheshades=FALSE;

class xgraphic * xgraphicGrayShade (class xgraphic  *self, long  index	/* between 0 and 16, inclusive */)
          {
  if (index < 0 || index > 16)
    return (NULL);

  if (self->gray_shades[index] == NULL)
    {
      if(!cacheshades) {
	  if (!xgraphic_shadeFont)
	      xgraphic_shadeFont = fontdesc::Create("xshape",fontdesc_Plain,10);

	  self->gray_shades[index] = (class xgraphic *) (xgraphic_shadeFont)->CvtCharToGraphic( self, index);
      } else {
	  GetShades(self);
      }
  }
  return (self->gray_shades[index]);
}

class graphic * xgraphic::GrayPattern(short  IntensityNum , short  IntensityDenom)
         {

  short ind;

    if (IntensityDenom !=16) 
	IntensityNum = (IntensityNum * 16) / IntensityDenom;
    if (IntensityNum < 0 ) IntensityNum = 0;
    else if (IntensityNum > 16) IntensityNum = 16;

  if (this->gray_levels[IntensityNum]) 
      return (class graphic *) this->gray_levels[IntensityNum];

  if ((this)->DisplayClass() & graphic_StaticGray) {
      unsigned short red, blue, green;
      short FGindex, BGindex;

      xcolor *xc=(xcolor *)fore->FindMapping(*CurrentColormap());
      if(xc==NULL) {
	  xc=(xcolor *)(*CurrentColormap())->Allocate(fore);
	  if(xc==NULL) xc=(xcolor *)(*CurrentColormap())->Alloc(0, 0, 0);
	  if(xc) fore->AddMapping(xc);
      }
      xc->ddcolor::GetRGB(red, green, blue);
      
      FGindex = xgraphic_ApproximateColor (this, &red, &green, &blue);
      
      xc=(xcolor *)back->FindMapping(*CurrentColormap());
      if(xc==NULL) {
	  xc=(xcolor *)(*CurrentColormap())->Allocate(back);
	  if(xc==NULL) xc=(xcolor *)(*CurrentColormap())->Alloc(0, 0, 0);
	  if(xc) back->AddMapping(xc);
      }
      xc->ddcolor::GetRGB(red, green, blue);
      
      BGindex = xgraphic_ApproximateColor (this, &red, &green, &blue);
      ind = BGindex + ((FGindex - BGindex) * IntensityNum) / 16;
    }
  else {
      ind = IntensityNum;
    }

  return ((class graphic *) (this->gray_levels[IntensityNum] = xgraphicGrayShade(this, ind)));
}

static void SetFGPixel( class xgraphic  *self, unsigned long  pixel )
        {
    self->lastFillPixel = self->foregroundpixel = pixel;
    if (self->transferMode == graphic_WHITE)  {
	XSetBackground((self)->XDisplay(), (self)->XGC(), self->foregroundpixel);
	XSetBackground((self)->XDisplay(), (self)->XFillGC(), self->foregroundpixel);
    }
#ifndef PLANEMASK_ENV 
    else if (self->transferMode == graphic_XOR) {
	XSetForeground((self)->XDisplay(), (self)->XGC(), self->foregroundpixel ^ self->backgroundpixel);
	XSetForeground((self)->XDisplay(), (self)->XFillGC(), self->foregroundpixel ^ self->backgroundpixel);
    }
#endif /* PLANEMASK_ENV  */
 else  {
     XSetForeground((self)->XDisplay(), (self)->XGC(), self->foregroundpixel);
     XSetForeground((self)->XDisplay(), (self)->XFillGC(), self->foregroundpixel);
 }
#ifdef PLANEMASK_ENV
    XSetPlaneMask((self)->XDisplay(), (self)->XGC(), self->foregroundpixel ^ self->backgroundpixel);
    XSetPlaneMask((self)->XDisplay(), (self)->XFillGC(), self->foregroundpixel ^ self->backgroundpixel);
#endif /* PLANEMASK_ENV */
}

static void SetBGPixel( class xgraphic  *self, unsigned long  pixel )
        {
    self->backgroundpixel = pixel;
    if (self->transferMode == graphic_WHITE)  {
	XSetForeground((self)->XDisplay(), (self)->XGC(), self->backgroundpixel);
	XSetForeground((self)->XDisplay(), (self)->XFillGC(), self->backgroundpixel);
    }
#ifndef PLANEMASK_ENV 
    else if (self->transferMode == graphic_XOR) {
	/* At this point background should be 0.  And should remain so */
    }
#endif /* PLANEMASK_ENV  */
    else  {
	XSetBackground((self)->XDisplay(), (self)->XGC(),	self->backgroundpixel);
	XSetBackground((self)->XDisplay(), (self)->XFillGC(), self->backgroundpixel);
    }
#ifdef PLANEMASK_ENV
    XSetPlaneMask((self)->XDisplay(), (self)->XGC(), self->foregroundpixel ^ self->backgroundpixel);
    XSetPlaneMask((self)->XDisplay(), (self)->XFillGC(), self->foregroundpixel ^ self->backgroundpixel);
    XSetWindowBackground((self)->XDisplay(), (self)->XWindow(), self->backgroundpixel);
#endif /* PLANEMASK_ENV */
}


static short xgraphic_ApproximateColor( class xgraphic  *self,unsigned short  *red , unsigned short  *green , unsigned short  *blue )
      {
    short		    ind = 0;
  class xcolormap **cmap = (class xcolormap **) (self)->CurrentColormap();

  if (cmap == NULL)
      return(ind);
 
  if ( (self )->DisplayClass( ) & graphic_StaticGray ) {
	if (( *red == *green ) && ( *green == *blue ))
	    ind = 16 - ( short )( 16.0 * (( double )*red )/XFullBrightness + .5);
	else
	    ind = 16 - (short) (16.0 * (((double) *red) * 0.3 + ((double) *green) * 0.59 + ((double) *blue) * 0.11) / XFullBrightness + .5);
    }
  else if ( (self )->DisplayClass( ) & graphic_GrayScale ) {
	if (( *red == *green ) && ( *green == *blue ));
	else
	    *red = (long)(((double) *red) * 0.3 + ((double) *green) * 0.59 + ((double) *blue) * 0.11 + .5 );
    }
    return ind;
}

static void SetStipple(class xgraphic  *self, long  index)
{
    class xgraphic	*tile;

    tile = xgraphicGrayShade (self, index);
    xgraphic_SetupFillGC( self, tile );
    if (((self)->DisplayClass() & graphic_Monochrome) && (index == 0 || index == 16))
     {
	XSetFillStyle((self)->XDisplay(), (self)->XGC(), FillSolid );
	self->flipforstipple=(index==0);
	(self)->SetTransferMode( self->transferMode);
	self->lastStipple=NULL;
     }
    else {
	self->flipforstipple=FALSE;
	XSetFillStyle((self)->XDisplay(), (self)->XGC(), FillOpaqueStippled );
	self->lastStipple=tile;
	if(self->transferMode==graphic_XOR) XSetStipple((self)->XDisplay(), (self)->XGC(), self->lastFillTile?self->lastFillTile->localWindow:None);
	else XSetStipple((self)->XDisplay(),  (self)->XGC(), tile->localWindow);
    }
}
void xgraphic::SetForegroundColor(const char *name, long r, long g, long b) {
    graphic::SetForegroundColor(name, r, g,b);
}


void xgraphic::SetBackgroundColor(const char *name, long r, long g, long b) {
    graphic::SetBackgroundColor(name, r, g,b);
}


void xgraphic::SetForegroundColor(color *c) {
    graphic::SetForegroundColor(c);
    if(CurrentColormap()==NULL) return;
    xgraphicClearGrayLevels(this);
    xcolor *xc=(xcolor *)c->FindMapping(*CurrentColormap());
    if(xc==NULL) {
	xc=(xcolor *)(*CurrentColormap())->Allocate(c);
	if(xc==NULL) xc=(xcolor *)(*CurrentColormap())->Alloc(0, 0, 0);
	if(xc) c->AddMapping(xc);
    }
    unsigned short R, G, B;
    if(xc==NULL) {
	c->RGB(R, G, B);
	fprintf(stderr, "xgraphic: Couldn't allocate color %s(%d, %d, %d)\n", c->Name()?c->Name():"(NULL)", R, G, B);
	SetFGPixel(this, BlackPixel(XDisplay(), XScreen()));
	return;
    }
    if ( (this )->DisplayClass( ) & graphic_StaticGray) {
	xc->PixelRef()=BlackPixel(XDisplay(), XScreen());
	xc->ddcolor::GetRGB(R, G, B);
	short ind = xgraphic_ApproximateColor( this, &R, &G, &B );
	if (ind != 0 || ind != 16) {
	    SetStipple(this, ind);
	} else {
	    XSetFillStyle((this)->XDisplay(), (this)->XGC(), FillSolid);
	}
    }
    SetFGPixel(this, xc->PixelRef());
}
    
void xgraphic::SetBackgroundColor(color *c) {
    graphic::SetBackgroundColor(c);
    if(CurrentColormap()==NULL) return;
    xgraphicClearGrayLevels(this);
    xcolor *xc=(xcolor *)c->FindMapping(*CurrentColormap());
    if(xc==NULL) {
	xc=(xcolor *)(*CurrentColormap())->Allocate(c);
	if(xc==NULL) xc=(xcolor *)(*CurrentColormap())->Alloc(65535, 65535, 65535);
	if(xc) c->AddMapping(xc);
    }
    unsigned short R, G, B;
    c->RGB(R, G, B);
    if(xc==NULL) {
	fprintf(stderr, "xgraphic: Couldn't allocate color %s(%d, %d, %d)\n", c->Name()?c->Name():"(NULL)", R, G, B);
	SetBGPixel(this, WhitePixel(XDisplay(), XScreen()));
	return;
    }
    if ( DisplayClass() & graphic_StaticGray) {
	xc->PixelRef()=WhitePixel(XDisplay(), XScreen());
	xc->ddcolor::GetRGB(R, G, B);
	short ind = xgraphic_ApproximateColor( this, &R, &G, &B );
	if (ind != 0 && ind != 16) {
	    SetStipple(this, ind);
	} else {
	    XSetFillStyle((this)->XDisplay(), (this)->XGC(), FillSolid);
	}
    }
    SetBGPixel(this, xc->PixelRef());
}


long xgraphic::GetHorizontalResolution()
{
    long res;
    if (!(this)->XDisplay()) return 72;
    res=(long) ((25.4 * DisplayWidth((this)->XDisplay(), (this)->XScreen())) / (DisplayWidthMM((this)->XDisplay(), (this)->XScreen())));
    res=environ::GetProfileInt("XDPI", res);
    res=environ::GetProfileInt("XHorizontalDPI", res);
    return res;
}


long xgraphic::GetVerticalResolution()
{
    long res;
    if (!(this)->XDisplay()) return (long) 72;
    res=(long) ((25.4 * DisplayHeight((this)->XDisplay(), (this)->XScreen())) / (DisplayHeightMM((this)->XDisplay(), (this)->XScreen())));
    res=environ::GetProfileInt("XDPI", res);
    res=environ::GetProfileInt("XVerticalDPI", res);
    return res;
}

const char * xgraphic::GetWindowManagerType()
{
    return "XV11R1";
}

struct xgraphic_DeviceBlock {
    struct xgraphic_DeviceBlock * nextDevice;
    Display * displayUsed;
    int screenUsed;
};

static struct xgraphic_DeviceBlock * deviceList = NULL;

long xgraphic::GetDevice()
{
    struct xgraphic_DeviceBlock * tmp, * curDevice;

    /* Look for the matching device block for this graphic and call it the device */
    if (!this->valid) return 0;
    for (curDevice = deviceList; curDevice; curDevice = curDevice->nextDevice)
	if (curDevice->displayUsed == this->displayUsed && curDevice->screenUsed == this->screenUsed) return (long) curDevice;
    /* Block not found, so must add one */
    tmp = (struct xgraphic_DeviceBlock *) malloc (sizeof(struct xgraphic_DeviceBlock));
    tmp->displayUsed = this->displayUsed;
    tmp->screenUsed = this->screenUsed;
    tmp->nextDevice = deviceList;
    deviceList = tmp;
    return (long) tmp;
}

/* -------------------------------------------------- */
/*   Predefined procedures */
/* -------------------------------------------------- */

xgraphic::xgraphic()
{
	ATKinit;

    this->flipforstipple=FALSE;
    this->lastStipple=NULL;
    /* Note: these have to be set by IM when it creates a window,
	since premature setting may cause inability to place into
	a parent on a different screen */
    this->localGraphicContext = NULL;
    this->localFillGraphicContext = NULL;
    this->lastFillStyle = FillSolid;
    this->lastFillPixel =  0; /* black */
    this->lastFillTile = NULL;
    this->localWindow = None;
    this->displayUsed = NULL;
#if 0
    this->colormap = NULL;
    this->inheritedColormap = NULL;
#endif
    this->screenUsed = 0;
    /* we can't really fill in any of the graphic stuff until we
    have a parent or root */
    this->gray_shades = xgraphic_FindGrayBlock(NULL,0);
    this->gray_levels = (class xgraphic **) malloc(sizeof(class graphic *) * 17);
    xgraphicClearGrayLevels(this);
    this->valid = FALSE;
    this->lastUpdateRegionIDUsed = 0;
    this->DisplayClassVal = 0;
    this->ximageinfo = NULL;
    this->index = NULL;
    this->numColorsInIndex = 0;
    imagePrefs.alwaysUsePrivateCmap = environ::GetProfileSwitch("alwaysuseprivatecmap", FALSE);
    imagePrefs.alwaysDither = environ::GetProfileSwitch("alwaysdither", FALSE);
    imagePrefs.alwaysFitFixed = environ::GetProfileSwitch("alwaysfitfixed", FALSE);
    THROWONFAILURE( TRUE);
}

boolean xgraphic::InitializeClass()
{
    cacheshades=environ::GetProfileSwitch("CacheShades", FALSE);
    return TRUE;
}

xgraphic::~xgraphic()
{
	ATKinit;

    if (this->localGraphicContext != NULL) {
	XFreeGC(this->displayUsed, this->localGraphicContext);
    }
    if (this->localFillGraphicContext != NULL) {
	XFreeGC(this->displayUsed, this->localFillGraphicContext);
    }
    if (this->gray_levels) {
	free(this->gray_levels);
    }
    if(ximageinfo) DestroyXImageInfo(this);
}
	
static long RealDisplayClass( class xgraphic		     *self )
{
    XWindowAttributes atts;
    long		    c_class = 0;
    
    if ((self)->XDisplay() == NULL)
      return (c_class);
    if((!XGetWindowAttributes((self)->XDisplay(), (self)->XWindow(), &atts)) || atts.visual==NULL) return graphic_Monochrome | graphic_StaticGray;

    c_class= graphic_Monochrome | graphic_StaticGray;
    switch (atts.visual->c_class) {
	  case PseudoColor: c_class = graphic_PseudoColor;
	      c_class |= graphic_Color;
	      break;
	  case GrayScale: c_class = graphic_GrayScale;
	      break;
	  case DirectColor: c_class = graphic_DirectColor;
	      c_class |= graphic_Color;
	      break;
	  case TrueColor: c_class = graphic_TrueColor;
	      c_class |= graphic_Color;
	      break;
	  case StaticColor: c_class = graphic_StaticColor;
	      c_class |= graphic_Color;
	      break;
	  case StaticGray:
	      c_class = graphic_StaticGray;
	      if ( DisplayPlanes( (self )->XDisplay( ), (self )->XScreen( )) == 1 ) c_class |= graphic_Monochrome;
	      else {
		  Screen *s = DefaultScreenOfDisplay((self)->XDisplay());
		  if(CellsOfScreen(s) < environ::GetProfileInt("ForceMonochromeThreshold", 16)) c_class |= graphic_Monochrome ;
		  break;
	      }
    }
    return c_class;
}

long xgraphic::DisplayClass( )
{
    if(this->DisplayClassVal == 0) 
	this->DisplayClassVal = RealDisplayClass(this);
        return (this->DisplayClassVal);
}


void xgraphic::ObservedChanged(class observable *changed, long val) {
    graphic::ObservedChanged(changed,val);
    if(val==observable_OBJECTDESTROYED) {
	RemoveObserver(changed);
    }
}
