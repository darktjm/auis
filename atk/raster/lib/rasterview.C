/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* Added X Window Dump read/write menu options  -paul@athena.mit.edu 3/7/90; Added X Bitmap read/write menu options - paul@athena.mit.edu 5/16/90 */

/* Some comments on changes checked in on 8/10/88
>> SetPixel:
>>    The pix local variable is modified (somehow) by the
>>    call to graphic::SetBitAtLoc. Line after this call
>>    changed to use the value of raster_GetPix rather then pix.
>> Changed all occurances of:
>>    pixelimage_DATACHANGED --> raster_BITSCHANGED
>>    rasterimage_DATACHANGED --> raster_BITSCHANGED
>> ScaleCommand:
>>    Keeps copy of original pixels when scaling. Thus if one scales
>>    and then back up, the code extrapolates from the original rather
>>    then the scaled down version --> eliminates massive loss of
>>    information.
>> Post Menus:
>>    Reorganized to avoid duplication of code and to include several
>>    items when InMessages.
>> rasterview_FinalizeObject:
>>    Added: If original is non-NULL then
>> 		Destroy original and set original equal to NULL
>> rasterview_SetDataObject:
>>    Added Debug statement.
>> rasterview_ObservedChanged:
>>    When BITSCHANGED or BOUNDSCHANGED
>> 	if original is non-NULL then
>> 		Destroy original and set original equal to NULL
>>    Added: rasterview_SCALECHANGED.
>> rasterview_FullUpdate:
>>    if the ViewSelection is Empty then
>> 	set both the ViewSelection and DesiredSelection to the subraster.
*/

/* rasterv.c	

	The view module for the raster dataobject

*/

#include <andrewos.h> /* strings.h */
ATK_IMPL("rasterview.H")
#include <stdio.h>
#include <util.h>

#include <im.H>
#include <graphic.H>
#include <frame.H>
#include <buffer.H>

#include <view.H>
#include <fontdesc.H>
#include <sys/param.h> /* Defines MAXPATHLEN among other things */

#include <menulist.H>
#include <keymap.H>
#include <keystate.H>
#include <cursor.H>
/* #include <bind.ih> */
#include <proctable.H>
#include <message.H>
#include <completion.H>
#include <scroll.H>
#include <environ.H>

#include <rect.h>
#include <texttroff.H>

#include <rasterimage.H>
#include <raster.H>
#include <rasterview.H>
#include <rastoolview.H>
#include <heximage.H>
#include <paint.H>
#include <oldRF.H>
#include <xwdio.H>
#include <xbm.H>

#include <dispbox.h>
#include <rastvaux.h>

NO_DLL_EXPORT boolean rasterview_debug=FALSE;
static boolean RastersInitiallyShrunk = FALSE;










 

   



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *	User Interface 
 *	
 *	Routines called from keystrokes or menu

	XXX  when first created, there is no rasterimage hanging from the struct raster.
	Some menu operations will have to cause one to exist 

\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* masks for menu items */

#define	menuCopy	(1<<0)	/* Copy, usually, and even when read only */
#define	menuRead	(1<<1)	/* ReadRaster, usually, and also if no pix */
#define	menuOps 	(1<<2)	/* Replace, MirrorXX */
#define	menuCrop	(1<<3)	/* Crop, usual, when selection < subraster */
#define	menuUncrop	(1<<4)	/* Uncrop, only when cropped */
#define	menuRotate	(1<<5)	/* Rotate, only when whole rasterimage selected */
#define	menuNegative	(1<<6)	/* Negative */
#define	menuExtend	(1<<7)	/* ExtendToMat, when matte exceeds image */
#define	menuShrink	(1<<8)	/* ShrinkToButton, when RasterInitiallyShrunk */
#define menuMoveDB	(1<<9)	/* Move the Display Box */
#define menuHideDB	(1<<10)	/* Hide the Display Box */
#define menuTouchUp	(1<<11)	/* Touch Up */
#define menuSelect	(1<<12)	/* Selection Mode */
#define menuPan		(1<<13)
#define menuWrite	(1<<14)
#define menuZoomIn	(1<<15)
#define menuZoomOut	(1<<16)
#define menuScale	(1<<17)
#define menuCenter	(1<<18)
#define menuUpperLeft	(1<<19)
#define menuSelectAll	(1<<20)
#define	menuToolset	(1<<21)
#define	menuToolsetKill	(1<<22)
#define	menuInsetCreate	(1<<23)
#define	menuInsetThere	(1<<24)

/* masks for menus */

#define	menuNoPix	(menuRead | menuWrite)
#define	menuReadOnly	menuCopy
#define	menuReadWrite	(menuCopy | menuOps | menuNegative | menuRead | menuWrite)

static class menulist *Menus;
static class keymap *Keymap;


ATKdefineRegistry(rasterview, view, rasterview::InitializeClass);
static void ToggleDebug();
static void CropCommand(class rasterview  *self, long  rock);
static void UncropCommand(class rasterview  *self, long  rock);
static void ShrinkCommand(class rasterview  *self, long  rock);
void rasterview_CenterCommand(class rasterview  *self, long  rock);
static void UpperLeftCommand(class rasterview  *self, long  rock);
static void SelectAllCommand(class rasterview  *self, long  rock);
void rasterview_ZoomInCommand(class rasterview  *self, long  rock);
void rasterview_ReflectChangesInExpansion(class rasterview  *self, struct rectangle  *R);
static void HideDisplayBox(class rasterview  *self);
void rasterview_MoveDisplayBoxCommand(class rasterview  *self, long  rock);
void rasterview_FinishMovingDisplayBox(class rasterview  *self, long  x , long  y);
static void HideDisplayBoxCommand(class rasterview  *self, long  rock);
void rasterview_ZoomOutCommand(class rasterview  *self, long  rock);
static void NormalSizeCommand(class rasterview  *self, long  rock);
void rasterview_RegionSelectCommand(class rasterview  *self, long  rock);
void TouchUpCommand(class rasterview  *self, long  rock);
void PanCommand(class rasterview  *self, long  rock);
void ToolCommand(class rasterview  *self, long  rock);
static void ModifyCommand(class rasterview  *self, long  rock);
void rasterview_RotateCommand(class rasterview  *self, long  rock);
void ReadRaster(class rasterview  *self, class raster  *ras, char  *filename);
static void ReadFileCommand(class rasterview  *self, long  rock);
void WriteFileCommand(class rasterview  *self );
enum RasterIOType {
	InMacPaint, 
	OutMacPaint, 
	OutRasterFile, 
	OutPostscript,
	Inxwd,
	Outxwd,
	Inxbm,
#ifndef X11_ENV
	Outxbm
#else
	Outxbm,
	MakeWD,
	MakeAsnap
#endif
};

static void RasterIOCommand(class rasterview  *self, long  rock);
static void CopyCommand(class rasterview  *self);
static void ReplaceCommand (class rasterview  *self);
boolean MatExtendPossible(class rasterview  *self);
static void ExtendToMatCommand(class rasterview  *self);
static void SetPrintSizeCommand(class rasterview  *self);
static void ScaleCommand(class rasterview  *self);
static void ScaleReplaceCommand(class rasterview  *self);
void rasterview_RealPostMenus(class rasterview  *self);


static void ToggleDebug()
{
    rasterview_debug = ! rasterview_debug;
    printf("Debugging is now %s\n", (rasterview_debug) ? "On" : "Off");  fflush (stdout);
}

static void CropCommand(class rasterview  *self, long  rock)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    if (ras == NULL) return;

    /* Until Crop works when Zoomed. */
    if (NotFullSize(self)) return;

    self->ViewSelection = self->DesiredSelection;
    ras->subraster = self->ViewSelection;

    if (self->embedded) {
	self->Xscroll = self->Yscroll = 0;
	(ras)->NotifyObservers( raster_BOUNDSCHANGED);
    }
    else {
	rasterview_CenterViewSelection(self);
	self->needsFullUpdate = TRUE;
	(self)->WantUpdate( self);
    }
}

static void UncropCommand(class rasterview  *self, long  rock)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    if (ras == NULL) return;

    if (! self->embedded) {
	self->Xscroll += rectangle_Left(&self->ViewSelection);
	self->Yscroll += rectangle_Top(&self->ViewSelection); }

    rectangle_SetRectSize(&self->ViewSelection, 0, 0,
			   (ras)->GetWidth(), (ras)->GetHeight());
    ras->subraster = self->ViewSelection;

    if (self->embedded) {
	self->Xscroll = self->Yscroll = 0;
	ras->subraster = self->ViewSelection;
	(ras)->NotifyObservers( raster_BOUNDSCHANGED);
    }
    else {
	self->needsFullUpdate = TRUE;
	(self)->WantUpdate( self);
    }
}

static void ShrinkCommand(class rasterview  *self, long  rock)
{
    self->Shrunken = TRUE;
    self->needsFullUpdate = TRUE;
    (self)->WantNewSize( self);
    /* the new size will force FullUpdate, so we don't call WantUpdate */
    /* XXX WantNewSize does nothing if is an ApplicationLayer */
    if ( ! self->embedded)
	(self)->WantUpdate( self);
}

void rasterview_CenterCommand(class rasterview  *self, long  rock)
{
    struct rectangle VB;
    (self)->GetVisualBounds( &VB);

    if (RegionSelect(self)) {
	self->Xscroll = rectangle_Left(&self->DesiredSelection) - rectangle_Left(&self->ViewSelection) -
	  (rectangle_Width(&VB)/self->Scale - rectangle_Width(&self->DesiredSelection) - 1)/2;
	self->Yscroll = rectangle_Top(&self->DesiredSelection) - rectangle_Top(&self->ViewSelection) -
	  (rectangle_Height(&VB)/self->Scale - rectangle_Height(&self->DesiredSelection) - 1)/2;
	if (NotFullSize(self))
	    rasterview_ZoomToVisualBounds(self, self->Xscroll, self->Yscroll); }
	else {
	    rasterview_CenterViewSelection(self);
	    if (NotFullSize(self))
		rasterview_ZoomToVisualBounds(self,
				   self->Xscroll/self->Scale,
				   self->Yscroll/self->Scale); }

    self->needsFullUpdate = TRUE;
    (self)->WantUpdate( self);
}

static void UpperLeftCommand(class rasterview  *self, long  rock)
{
    class raster *ras = (class raster *)(self)->GetDataObject();

    if (! RegionSelect(self) || ras == NULL) return;

    if (FullSize(self)) {
	self->Xscroll = rectangle_Left(&self->DesiredSelection);
	self->Yscroll = rectangle_Top(&self->DesiredSelection); }
    else
	rasterview_UpdateZoomedSelection(self,
			      rectangle_Left(&self->DesiredSelection),
			      rectangle_Top(&self->DesiredSelection));

    self->needsFullUpdate = TRUE;
    (self)->WantUpdate( self);
}

static void SelectAllCommand(class rasterview  *self, long  rock)
{
    class raster *ras = (class raster *)(self)->GetDataObject();

    if (! RegionSelect(self) || ras == NULL) return;

    self->DesiredSelection = self->ViewSelection;
    rasterview_CorrectHighlight(self);
    self->needsFullUpdate=FALSE;
    (self)->WantUpdate( self);
}

void rasterview::SetScale(long  newscale)
{
    long x, y;
    struct rectangle VB;
    double ratio, adjustment;

    /* Until zoom works with cropping... */
    if (Cropped(this)) {
	return;
    }

    if ((newscale < 1) || (newscale == this->Scale)) {
	return;
    }

    (this)->GetVisualBounds( &VB);

    if (this->Scale < newscale) {
	if (rectangle_Width(&VB) + TWOBORDER < newscale
	 || rectangle_Height(&VB) + TWOBORDER < newscale) {
	    message::DisplayString(this, 0, "Zoom too large to fit current screen");
	    return;
	}
    }

    if (FullSize(this)) {
	/* Create the pixelimage which will hold the expanded version of the dataobj. */
	this->Expansion = rasterimage::Create(0,0);
    }

    ratio = ((double) newscale) / ((double) this->Scale);
    this->Scale = newscale;

    this->DisplayBoxHidden = (this->Scale < 4);

    /* Force rasterview_ZoomToVisualBounds to place the Display Box in the default location and to determine the zoomed selection. */
    SetLeftRect(&this->DisplayBox, -1);
    SetTopRect(&this->DisplayBox, -1);
    rectangle_EmptyRect(&this->DisplayBoxSelection);

    /* The scrolling of the zoomed in raster can be determined by considering the vector between the center of the screen and the upper left corner of the current View Selection. That vector is scaled (in this case) by a factor of two. The x component of the vector can be calculated as the sum of the current Xscroll and one half the difference of new and old visual widths. The new Xscroll can then be calculated as ratio times the x component of the vector plus one half the computed visual width difference. The new Yscroll can be calculated in a similar fashion. The (x, y) to pass to rasterview_ZoomToVisualBounds is then the scroll divided by the current scale. */
    
    adjustment = (ratio - 1.0) / 2.0;

    x = ((long) ((ratio * this->Xscroll + adjustment * rectangle_Width(&VB))))
		/ this->Scale;
    y = ((long) ((ratio * this->Yscroll + adjustment * rectangle_Height(&VB))))
		/ this->Scale;

    if (FullSize(this)) {
	(this->Expansion)->Destroy();
	this->Expansion = NULL;
	this->DisplayBoxOverlapped = FALSE;
	rectangle_EmptyRect(&this->DisplayBox);
	this->Xscroll = x;
	this->Yscroll = y;
    }
    else {
	rasterview_ZoomToVisualBounds(this, x, y);
    }

    rasterview_RealPostMenus(this);
    this->needsFullUpdate = TRUE;
    (this)->WantUpdate( this);

} /* rasterview__SetScale */

void rasterview_ZoomInCommand(class rasterview  *self, long  rock)
{
    DEBUG(("Scroll: (%ld,%ld)\n", self->Xscroll, self->Yscroll));
    DEBUG(("Zoom In Scale: %ld VS: (%ld,%ld,%ld,%ld)\n", self->Scale * 2,
	    rectangle_Left(&self->ViewSelection),
	    rectangle_Top(&self->ViewSelection),
	    rectangle_Width(&self->ViewSelection),
	    rectangle_Height(&self->ViewSelection)));

    (self)->SetScale( self->Scale * 2);
}

void rasterview_ReflectChangesInExpansion(class rasterview  *self, struct rectangle  *R)
/* R is the rectangle within the full image which has been changed */
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    class rasterimage *pix, *new_c;
    struct rectangle change;
    long x, y, w, h;

    if ((ras == NULL) || ((pix = (ras)->GetPix()) == NULL))  return;

    rectangle_IntersectRect(&change, R, &self->DisplayBoxSelection);
    if (IsEmptyRect(&change)) return;

    rectangle_GetRectSize(&change, &x, &y, &w, &h);
    w *= self->Scale;
    h *= self->Scale;

    new_c = rasterimage::Create(w, h);

    (pix)->GetScaledSubraster( &change, w, h, new_c);

    x = rectangle_Left(&change) - rectangle_Left(&self->DisplayBoxSelection);
    y = rectangle_Top(&change) - rectangle_Top(&self->DisplayBoxSelection);
    rectangle_SetRectSize(&change, 0, 0, w, h);

    (self->Expansion)->BlitSubraster( x*self->Scale, y*self->Scale, new_c, &change, pixelimage_COPY);
    (new_c)->Destroy();

    /* If the Display Box is Empty then do not blit anything into the image. */
    if (IsNotEmptyRect(&self->DisplayBox))
	DisplayBoxBlitOverlap(self, pix);
}

static void HideDisplayBox(class rasterview  *self)
{
    if (RegionSelect(self))
	rasterview_ViewHideHighlight(self);
    rasterview_DisplayBoxHide(self);
}

void rasterview_MoveDisplayBoxCommand(class rasterview  *self, long  rock)
{
    if (rock == 1 && FullSize(self)) return;

    if (rock >= 0)
	message::DisplayString(self, 0,
			      "Click Left to selection new position of Display Box");

    DEBUG(("Moving Display Box\n"));
    HideDisplayBox(self);
    self->MovingDisplayBox = TRUE;
}

void rasterview_FinishMovingDisplayBox(class rasterview  *self, long  x , long  y)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    class rasterimage *pix;
    class graphic *G = (self)->GetDrawable();
    struct rectangle VB;

    ENTER(FinishMovingDisplayBox);

    message::DisplayString(self, 0, "");

    if ((ras == NULL) || ((pix = (ras)->GetPix()) == NULL))  return;
    if (! self->MovingDisplayBox) return;

    (self)->GetVisualBounds( &VB);

    SetLeftRect(&self->DisplayBox, x);
    SetTopRect(&self->DisplayBox, y);
    /* Keep the Display Box within the Visual Rectangle. */
    if (rectangle_Left(&self->DisplayBox) - 4*BORDER <= 0) {
	SetLeftRect(&self->DisplayBox, 4*BORDER); }
    if (rectangle_Top(&self->DisplayBox) - 4*BORDER <= 0) {
	SetTopRect(&self->DisplayBox, 4*BORDER); }
    if (rectangle_Right(&self->DisplayBox) + 3*BORDER > rectangle_Width(&VB)) {
	long left = rectangle_Right(&VB) - 2*BORDER;
	left -= (left % self->Scale);
	SetLeftRect(&self->DisplayBox,
		    left - rectangle_Width(&self->DisplayBox) - 3*BORDER); }
    if (rectangle_Bottom(&self->DisplayBox) + 3*BORDER > rectangle_Height(&VB)) {
	long top = rectangle_Bottom(&VB) - 2*BORDER;
	top -= (top % self->Scale);
	SetTopRect(&self->DisplayBox,
		   top - rectangle_Height(&self->DisplayBox) - 3*BORDER); }

    DEBUG(("Moving Display Box to: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&self->DisplayBox),
	    rectangle_Top(&self->DisplayBox),
	    rectangle_Width(&self->DisplayBox),
	    rectangle_Height(&self->DisplayBox)));

    self->DisplayBoxHidden = FALSE;
    self->MovingDisplayBox = FALSE;

    DisplayBoxBlitOverlap(self, pix);
    rasterview_DisplayBoxWritePixImageFull(self, G, pix);

    if (RegionSelect(self))
	rasterview_CorrectHighlight(self);
    if (Pan(self)) {
	rasterview_DrawPanHighlight(self, graphic::BLACK); }

    rasterview_RealPostMenus(self);
}

static void HideDisplayBoxCommand(class rasterview  *self, long  rock)
{
    HideDisplayBox(self);
    SetLeftRect(&self->DisplayBox, -3*rectangle_Width(&self->DisplayBox));
    if (RegionSelect(self))
	rasterview_CorrectHighlight(self);
    self->DisplayBoxHidden = TRUE;
    rasterview_RealPostMenus(self);
}

void rasterview_ZoomOutCommand(class rasterview  *self, long  rock)
{
    DEBUG(("Zoom Out Scale: %ld\n", self->Scale / 2));
    (self)->SetScale( self->Scale / 2);
}

static void NormalSizeCommand(class rasterview  *self, long  rock)
{
    if (NotFullSize(self)) {
	(self)->SetScale( 1);
    }
}

void rasterview_RegionSelectCommand(class rasterview  *self, long  rock)
{
    struct rectangle r;
    (self)->GetVisualBounds( &r);

    message::DisplayString(self, 0, "Region Select Mode");
    (self)->RetractCursor( self->Cursor[self->Mode]);

    if (Pan(self))
	rasterview_DrawPanHighlight(self, graphic::WHITE);

    self->Mode = RegionSelectMode;
    rasterview_CorrectHighlight(self);

    (self)->NotifyObservers(0); 

    (self)->PostCursor( &r, self->Cursor[self->Mode]);
    rasterview_RealPostMenus(self);
}

void TouchUpCommand(class rasterview  *self, long  rock)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    struct rectangle r;
    (self)->GetVisualBounds( &r);

    if (ras == NULL || (ras)->GetPix() == NULL) return;

    message::DisplayString(self, 0, "Touch Up Mode");
    (self)->RetractCursor( self->Cursor[self->Mode]);

    if (RegionSelect(self) || 
	 (Tool(self) && !rectangle_IsEmptyRect(&(self->CurSelection))))
	rasterview_ViewHideHighlight(self);
    else if (Pan(self))
	rasterview_DrawPanHighlight(self, graphic::WHITE);

    self->Mode = TouchUpMode;

    (self)->NotifyObservers(0); 

    (self)->PostCursor( &r, self->Cursor[self->Mode]);
    rasterview_RealPostMenus(self);
}

void PanCommand(class rasterview  *self, long  rock)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    struct rectangle r;
    (self)->GetVisualBounds( &r);
    if (ras == NULL || (ras)->GetPix() == NULL) return;

    message::DisplayString(self, 0, "Pan Mode");
    (self)->RetractCursor( self->Cursor[self->Mode]);

    if (RegionSelect(self) || 
	 (Tool(self) && !rectangle_IsEmptyRect(&(self->CurSelection))))
	rasterview_ViewHideHighlight(self);

    self->Mode = PanMode;
    rasterview_DrawPanHighlight(self, graphic::BLACK);

    (self)->NotifyObservers(0); 

    (self)->PostCursor( &r, self->Cursor[self->Mode]);
    rasterview_RealPostMenus(self);
}

void ToolCommand(class rasterview  *self, long  rock)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    struct rectangle r;

    (self)->GetVisualBounds( &r);
    if (ras == NULL || (ras)->GetPix() == NULL) return;

    message::DisplayString(self, 0, "Special Tool Mode");
    (self)->RetractCursor( self->Cursor[self->Mode]);

    if (RegionSelect(self) || 
	 (Tool(self) && !rectangle_IsEmptyRect(&(self->CurSelection)))) {
	rasterview_ViewHideHighlight(self);
    }
    else if (Pan(self))
	rasterview_DrawPanHighlight(self, graphic::WHITE);

    self->Mode = ToolMode;

    if ((self->toolset)->WantSelectionHighlighted())
	rasterview_CorrectHighlight(self);

    (self)->NotifyObservers(0); 

    (self)->PostCursor( &r, self->Cursor[self->Mode]);
    rasterview_RealPostMenus(self);
}

static void ModifyCommand(class rasterview  *self, long  rock)
{
    /* -1 = negative, 0 = white, 1 = black, 2 = Gray */
    class raster *ras = (class raster *)(self)->GetDataObject();
    class rasterimage *pix;
    struct rectangle R;

    R = self->DesiredSelection;

    if (ras == NULL || (pix = (ras)->GetPix()) == NULL) return;

    if (rock == 0)
	(pix)->PaintSubraster( &R, WHITEBYTE);
    else if (rock == 1)
	(pix)->PaintSubraster( &R, BLACKBYTE);
    else if (rock == -1)
	(pix)->InvertSubraster( &R);
    else if (rock == 2) {
	char inbuf[50];
	long level;
	AskOrCancel(self, "Gray level (1..15)[8]: ", inbuf);
	/* If there is a value returned then if the value cannot be parsed then error. */
	if (*inbuf) {
	    if (sscanf(inbuf, "%ld", &level) != 1)	
		DisplayAndReturn(self, "Value must be digits with no decimal point."); }
	else level = 8;
	(pix)->GraySubraster( &R, level); }
    else if (rock == 3)
	(pix)->MirrorLRSubraster( &R);
    else if (rock == 4)
	(pix)->MirrorUDSubraster( &R);
 
    if (NotFullSize(self))
	rasterview_ReflectChangesInExpansion(self, &R);

    (pix)->NotifyObservers( raster_BITSCHANGED);
}

/* RotateCommand(self, rock)
	Rotates the entire raster image of self.

	Strategy is to clone the rasterimage, set the bit array pointer to null in 
	the original and resize it.  Then the Rotate operation is done by copying
	the bits from the clone (which has the original bit array) to the new
	bit array (which is attached to the original rasterimage).
	Finally, the clone and the old bit array are discarded.
*/
void rasterview_RotateCommand(class rasterview  *self, long  rock)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    class rasterimage *pix;
    class pixelimage *temp;
    long w, h;
    struct rectangle R;

    if ((ras == NULL) || ((pix = (ras)->GetPix()) == NULL)) return;

    /* Until Rotation works while Zoomed... */
    if (NotFullSize(self)) return;

    w = (pix)->GetWidth();
    h = (pix)->GetHeight();

    temp = (pix)->Clone();
    (pix)->SetBitsPtr( NULL);
    (pix)->Resize( h, w);

    rectangle_SetRectSize(&R, 0, 0, w, h);
    ((class pixelimage *)temp)->GetRotatedSubraster( &R,
				    (class pixelimage *)pix);
    (temp)->Destroy();
    rectangle_SetRectSize(&self->DesiredSelection, 0, 0, h, w);
    rectangle_SetRectSize(&self->ViewSelection, 0, 0, h, w);
    rasterview_CenterViewSelection(self);
    (pix)->NotifyObservers( raster_BITSCHANGED);
}

void ReadRaster(class rasterview  *self, class raster  *ras, char  *filename)
{
    /* need to use FindFile XXX */
    long readresult = dataobject::OBJECTCREATIONFAILED;
    FILE *f;
    int c;

    f = fopen(filename, "r");
    if (f == NULL) {
	char err[MAXPATHLEN + 50];
	sprintf(err, "Could not read file %s", filename);
	message::DisplayString((class view *)self, 50, err);
	return; }

    /* The raster_Read routine must assume the header is gone therefore we have to read it. For compatability, we do not read beyond an initial 0xF1, which raster_Read knows about */

    ungetc((c=getc(f)), f);
    if (c == 0xF1) 
	readresult = (ras)->Read( f, 0);
    else {
	/* XXX If the file does not begin with a raster, we may as well scan until we find one */
	static const char hdr[] = "\\begindata{raster,";
	const char *hx = hdr;

	while ((c = getc(f)) != EOF && *hx)
	    if (c == *hx) hx++;
	    else hx = (c == *hdr) ? hdr+1 : hdr;
	if (*hx) {
	    char err[MAXPATHLEN + 50];
	    sprintf(err, "No raster found in file %s", filename);
	    message::DisplayString((class view *)self, 50, err);
	    fclose(f);
	    return; }
	while ((c=getc(f)) != '\n' && c != EOF) 
	{}	/* skip to end of header */
	readresult = (ras)->Read( f, 0); }
    fclose(f);
    /* XXX need to inform observers of scroll change. */

    if (readresult == dataobject::NOREADERROR) {
	self->needsFullUpdate = TRUE;
	self->Xscroll = self->Yscroll = 0;
	/* select the entire raster */
	self->ViewSelection = self->DesiredSelection = ras->subraster;
	if (! self->embedded)
	    rasterview_CenterViewSelection(self);
    }
    else {
	char err[MAXPATHLEN + 50];
	sprintf(err, "Error %ld while reading file %s", readresult, filename);
	message::DisplayString((class view *)self, 50, err); }
}

static void ReadFileCommand(class rasterview  *self, long  rock)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    char filename[MAXPATHLEN];
    enum message_CompletionCode result;

    ENTER(ReadFileCommand);
    if (ras == NULL) return;

    rasterview_CurrentDirectory(self, filename);

    result = (enum message_CompletionCode)
      completion::GetFilename(self, 
			      "Read Raster File: ", 	/* prompt */
			      filename,			/* initial string */
			      filename,			/* working area */
			      MAXPATHLEN,		/* size of working area */
			      FALSE,			/* want a file, not a directory */
			      TRUE);			/* must be existing file */

    if (result != message_Complete && result != message_CompleteValid) {
	message::DisplayString((class view *)self, 0, "Cancelled.");
	return; }

    ReadRaster(self, ras, filename);
    if (! self->embedded) {
	class buffer *buffer = NULL;

	buffer = (frame::Enumerate((frame_effptr)rasterview_FindFrameHelp, (long)(self)->GetIM()))->GetBuffer();
	if(buffer) (buffer)->SetFilename( filename);
    }

    /* the Read will cause a NotifyObservers, which will call WantUpdate */
}

void WriteFileCommand(class rasterview  *self )
{
    /* write using filename from read as default. */
    message::DisplayString((class view *)self, 0, "Write Raster not yet implemented");
}

static const char * const prompts[] = {
	"Read MacPaint file: ",
	"Write MacPaint file: ",
	"Write RasterFile file: ",
	"Write Postscript file: ",
	"Read X Window Dump file: ",
	"Write X Window Dump file: ",
	"Read X Bitmap file: ",
#ifndef X11_ENV
	"Write X Bitmap file: "
#else
	"Write X Bitmap file: ",
	"Window Dump: select a window.",
	"Area Dump: sweep out an area."
#endif
};

static void RasterIOCommand(class rasterview  *self, long  rock)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    FILE *f;
    boolean inputop;
    char filename[MAXPATHLEN];
    enum message_CompletionCode result;
    int savedMode;
    struct rectangle r;

    ENTER(RasterIOCommand);
    if (ras == NULL) return;

#ifndef X11_ENV
    inputop =  (rock == InMacPaint || rock == Inxwd || rock == Inxbm);
#else
    inputop =  (rock == InMacPaint || rock == Inxwd || rock == Inxbm || (rock == MakeWD) || (rock == MakeAsnap));
#endif
    /* XXX adjust if add more input types */

    rasterview_CurrentDirectory(self, filename);

#ifdef X11_ENV
    if (rock != MakeWD && rock != MakeAsnap) {
#endif
	result = (enum message_CompletionCode)
	  completion::GetFilename(self, 
				 prompts[(int)rock], 	/* prompt */
				 filename,	 		/* initial string */
				 filename,			/* working area */
				 MAXPATHLEN,		/* size of working area */
				 FALSE,			/* want a file, not a directory */
				 inputop);			/* must be existing file */

	if (result != message_Complete && result != message_CompleteValid) {
	    message::DisplayString((class view *)self, 0, 
				  "Cancelled.");
	    return;
	}
#ifdef X11_ENV
    }
#endif

    savedMode = self->Mode;
    (self)->RetractCursor( self->Cursor[self->Mode]);
    self->Mode = WaitMode;
    (self)->GetVisualBounds( &r);
    (self)->PostCursor( &r, self->Cursor[self->Mode]);

#ifdef X11_ENV
    if (rock == MakeWD || rock == MakeAsnap) {
	/* Dump a window and set it up to read */
	char cmd[100];
	int res;
	strcpy(filename, "/tmp/wdXXXXXX");
	res = mkstemp(filename);
	close(res);
	if (rock == MakeWD)
	    sprintf(cmd, "xwd %s -out %s", ((self)->DisplayClass() & graphic::Monochrome) ? "-xy" : "", filename);
	else
	    sprintf(cmd, "asnap -atk -noshow -file %s", filename);
	res = system(cmd);
	if (res != 0) {
	    char err[500];
	    if (rock == MakeWD) sprintf(err, "Failed: could not find xwd.");
	    else sprintf(err, "Failed: could not find asnap.");
	    message::DisplayString((class view *)self, 0, err);
	    (self)->RetractCursor( self->Cursor[self->Mode]);
	    self->Mode = savedMode;
	    (self)->PostCursor( &r, self->Cursor[self->Mode]);
	    return;
 	}
    }
#endif
    
    /* need to use FindFile XXX */
    /* need to check if output file exists XXX */

    f = fopen(filename, (inputop) ? "r" : "w");
    if (f == NULL) {
	char err[MAXPATHLEN + 50];
	sprintf(err, "Could not %s file %s", (inputop) ? "read" : "write", filename);
	message::DisplayString((class view *)self, 0, err);
	(self)->RetractCursor( self->Cursor[self->Mode]);
	self->Mode = savedMode;
	(self)->PostCursor( &r, self->Cursor[self->Mode]);
	return;
    }

    switch (rock) {
	case InMacPaint:
	    if (paint::ReadImage(f, (ras)->GetPix()) 
		!= dataobject::NOREADERROR) {
		char msg[MAXPATHLEN + 50];
		sprintf(msg, "File %s apparently not in MacPaint format", filename);
		message::DisplayString((class view *)self, 0, msg); }
	    else {
		self->Xscroll = self->Yscroll = 0;
		self->needsFullUpdate = TRUE;
		self->ViewSelection = self->DesiredSelection = ras->subraster; 
		/* the Read will cause a NotifyObservers,
		 which will call WantUpdate */
	    }
	    break;
	case OutMacPaint:
	    paint::WriteImage(f, (ras)->GetPix(), &self->ViewSelection);
	    break;
	case OutRasterFile:
	    oldRF::WriteImage(f, (ras)->GetPix(), &self->ViewSelection);
	    break;
	case OutPostscript:
	    heximage::WritePostscript(f, (ras)->GetPix(), &self->ViewSelection, 
				     (ras->xScale + 0e0) / raster_UNITSCALE, 
				     (ras->yScale + 0e0) / raster_UNITSCALE);
	    break;
	case Inxwd:
#ifdef X11_ENV
	case MakeWD:
#endif
	    if (xwdio::ReadImage(f, (ras)->GetPix()) 
		!= dataobject::NOREADERROR) {
		char msg[MAXPATHLEN + 50];
		sprintf(msg, "File %s apparently not in X Window Dump format", filename);
		message::DisplayString((class view *)self, 0, msg); }
	    else {
		self->Xscroll = self->Yscroll = 0;
		self->needsFullUpdate = TRUE;
		self->ViewSelection = self->DesiredSelection = ras->subraster; 
		/* the Read will cause a NotifyObservers,
		 which will call WantUpdate */
	    }
	    break;
#ifdef X11_ENV
	case MakeAsnap:
	    ReadRaster(self, ras, filename);
	    break;
#endif
	case Outxwd:
	    xwdio::WriteImage(f, (ras)->GetPix(),
			      &self->ViewSelection);
	    break;
	case Inxbm:
	    if (xbm::ReadImage(f, (ras)->GetPix()) 
		!= dataobject::NOREADERROR) {
		char msg[MAXPATHLEN + 50];
		sprintf(msg, "File %s apparently not in X Bitmap format", filename);
		message::DisplayString((class view *)self, 0, msg); }
	    else {
		self->Xscroll = self->Yscroll = 0;
		self->needsFullUpdate = TRUE;
		self->ViewSelection = self->DesiredSelection = ras->subraster; 
		/* the Read will cause a NotifyObservers,
		 which will call WantUpdate */
	    }
	    break;
	case Outxbm:
	    xbm::WriteImage(f, (ras)->GetPix(),
			      &self->ViewSelection);
	    break;

    }
    fclose(f);
#ifndef X11_ENV
    if (! self->embedded && inputop) 
#else
    if (rock == MakeWD || rock == MakeAsnap) unlink(filename);
    
    if (rock != MakeWD && rock != MakeAsnap && ! self->embedded && inputop) 
#endif
        {
	class buffer *buffer = NULL;

	buffer = (frame::Enumerate((frame_effptr)rasterview_FindFrameHelp, (long)(self)->GetIM()))->GetBuffer();
	if(buffer) (buffer)->SetFilename( filename);
    }

    (self)->RetractCursor( self->Cursor[self->Mode]);
    self->Mode = savedMode;
    (self)->PostCursor( &r, self->Cursor[self->Mode]);

    if ( ! inputop) {
	char msg[MAXPATHLEN + 50];
	sprintf(msg, "Wrote file %s", filename);
	message::DisplayString((class view *)self, 0, msg); }

}

static void CopyCommand(class rasterview  *self) 
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    FILE *copyFile;

    if (ras == NULL) return;
    copyFile = ((self)->GetIM())->ToCutBuffer();
    (ras)->WriteSubRaster( copyFile, 91, &self->DesiredSelection);
    /* the 91 is any non-zero object id */
    ((self)->GetIM())->CloseToCutBuffer( copyFile);

    if (self->ShowCoords) {
	static char cb[40];
	sprintf(cb, "Copied area %ld by %ld at (%ld,%ld)", rectangle_Width(&self->DesiredSelection), rectangle_Height(&self->DesiredSelection), rectangle_Left(&self->DesiredSelection), rectangle_Top(&self->DesiredSelection));
	message::DisplayString(self, 10, cb);
    }
}

static void ReplaceCommand (class rasterview  *self)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    FILE *pasteFile;
    static const char hdr[] = "\\begindata{raster,";
    const char *hx = hdr;
    int c;

    pasteFile = ((self)->GetIM())->FromCutBuffer();

    /* if the file does not begin with a raster, 
	we may as well scan until we find one XXX */

    while ((c = getc(pasteFile)) != EOF && *hx)
	if (c == *hx) hx++;
	else hx = (c == *hdr) ? hdr+1 : hdr;
    if (*hx) 
	message::DisplayString((class view *)self, 0, "No raster found in cut buffer");
    else {
	while ((c=getc(pasteFile)) != '\n' && c != EOF) 
	{}	/* skip to end of header */
	if (FullySelected(self) && Uncropped(self)) {
	    (ras)->Read( pasteFile, 1);
	    self->ViewSelection = self->DesiredSelection = ras->subraster;
	}
	else
	    (ras)->ReadSubRaster( pasteFile, &self->DesiredSelection);
    }
    ((self)->GetIM())->CloseFromCutBuffer( pasteFile);
}

boolean MatExtendPossible(class rasterview  *self)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    struct rectangle VB;

    (self)->GetVisualBounds( &VB);
    InsetRect(&VB, BORDER, BORDER);
    DEBUG(("Ext-P: Scroll: (%ld,%ld) Raster: (%ld,%ld)\n",
	    self->Xscroll, self->Yscroll,
	    (ras)->GetWidth(), (ras)->GetHeight()));
    DEBUG(("       VS: (%ld,%ld,%ld,%ld) VB: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&self->ViewSelection),
	    rectangle_Top(&self->ViewSelection),
	    rectangle_Width(&self->ViewSelection),
	    rectangle_Height(&self->ViewSelection),
	    rectangle_Left(&VB),
	    rectangle_Top(&VB),
	    rectangle_Width(&VB),
	    rectangle_Height(&VB)));
    return ((self->Xscroll + rectangle_Width(&VB))/self->Scale > (ras)->GetWidth()
	     || (self->Yscroll + rectangle_Height(&VB))/self->Scale > (ras)->GetHeight()
	     || self->Xscroll < 0
	     || self->Yscroll < 0);
}

void rasterview::ResizeRaster(long  width , long  height)
{
    class raster *ras;
    class rasterimage *pix;
    class rasterimage *new_c;
    long xscr, yscr, w, h;

    ras = (class raster *) (this)->GetDataObject();
    w = (ras)->GetWidth();
    h = (ras)->GetHeight();

    pix = (ras)->GetPix();
    (ras)->Resize( width, height);

    if ((this->Xscroll < 0) || (this->Yscroll < 0)) {
	if (this->Xscroll < 0) {
	    xscr = - this->Xscroll / this->Scale;
	    this->Xscroll = 0;
	}
	else {
	    xscr = 0;
	}
	if (this->Yscroll < 0) {
	    yscr = - this->Yscroll / this->Scale;
	    this->Yscroll = 0;
	}
	else {
	    yscr = 0;
	}

	new_c = rasterimage::Create(w, h);

	(new_c)->BlitSubraster( 0, 0, pix, &this->ViewSelection, pixelimage_COPY);
	(pix)->Clear();
	(pix)->BlitSubraster( xscr, yscr, new_c, &this->ViewSelection, pixelimage_COPY);
	(new_c)->Destroy();
    }

    rectangle_SetRectSize(&this->ViewSelection, 0, 0, width, height);
    rectangle_SetRectSize(&this->DesiredSelection, 0, 0, width, height);

    if (NotFullSize(this)) {
	rasterview_ZoomToVisualBounds(this,
			   this->Xscroll/this->Scale,
			   this->Yscroll/this->Scale);
    }

    (pix)->NotifyObservers( raster_BITSCHANGED);
    (pix)->NotifyObservers( raster_BOUNDSCHANGED);
} /* rasterview__ResizeRaster */

/* ExtendToMatCommand(self)
	Extend the raster size to the size of the matte, if bigger 
*/
static void ExtendToMatCommand(class rasterview  *self)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    long l, t, w, h, r, b;
    long vr, vb;
    long width, height;
    struct rectangle VB;	/* the visual bounds measure the matte size */

    if (! MatExtendPossible(self)) return;

    (self)->GetVisualBounds( &VB);
    InsetRect(&VB, BORDER, BORDER);
    vr = rectangle_Width(&VB);
    vb = rectangle_Height(&VB);

    l = -self->Xscroll;
    t = -self->Yscroll;
    w = (ras)->GetWidth()*self->Scale;
    h = (ras)->GetHeight()*self->Scale;
    r = l + w;
    b = t + h;

    width = (w + ((vr > r) ? (vr - r) : 0) + ((l > 0) ? l : 0))/self->Scale;
    height = (h + ((vb > b) ? (vb - b) : 0) + ((t > 0) ? t : 0))/self->Scale;

    DEBUG(("Visual: %ld x %ld\nCurrent Sides: (%ld,%ld,%ld,%ld)\nNew Size: %ld x %ld\n",
	    vr, vb,
	    l/self->Scale, t/self->Scale, r/self->Scale, b/self->Scale,
	    width, height));

    (self)->ResizeRaster( width, height);
}

/* SetPrintSizeCommand(self)
	set scaling factors used for printing
	prompts for print size and sets scaling accordingly
	Note that the subraster is either the entire raster or the cropped raster.
*/
static void SetPrintSizeCommand(class rasterview  *self)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    long w = rectangle_Width(&self->ViewSelection);
    long h = rectangle_Height(&self->ViewSelection);

    double newdw = 0.0, newdh = 0.0;
    long newyScale = 0, newxScale = 0;
    char newxsize[75], newysize[75], request[150];

    /* ask for height */
    sprintf(request, "Print height %0.2f  width %0.2f in.   New height [scale to width] ", 
	     h / 72.0 * ras->yScale / raster_UNITSCALE,
	     w / 72.0 * ras->xScale / raster_UNITSCALE);
    AskOrCancel(self, request, newysize);
    if (*newysize) {
	/* height specified.  parse it and set width request */
	if (sscanf(newysize, "%lf", &newdh) != 1)		
	    DisplayAndReturn(self,
			     "Value must be digits with at most one decimal point.");
	newyScale = (long)((newdh * 72.0 * raster_UNITSCALE + (h/2)) / h);
	/* Change x scaling factor to correspond to the new y scaling factor */
	newxScale = newyScale;

	sprintf(request, "Print height %0.2f in.   New width [%0.2f] ", 
		h / 72.0 * newyScale / raster_UNITSCALE,
		w / 72.0 * newxScale / raster_UNITSCALE); }
    else {
	/* no height specified; scale to width, default original width */
	sprintf(request, "Print height %0.2f  width %0.2f in.   Scale to width [original:  %0.2f] ", 
		h / 72.0 * ras->yScale / raster_UNITSCALE,
		w / 72.0 * ras->xScale / raster_UNITSCALE,
		w / 72.0 / 2.0); }

    /* request new width */
    AskOrCancel(self, request, newxsize);
    if (*newxsize && sscanf(newxsize, "%lf", &newdw) != 1)
	DisplayAndReturn(self, "Value must be digits with at most one decimal point.");
    /* now analyze results.   set unspecified new..Scale  */
    if (*newxsize) {
	newxScale = (long)((newdw * 72.0 * raster_UNITSCALE + (w/2)) / w);
	if (*newysize == '\0') 
	    newyScale = newxScale; }
    else if (*newysize == '\0') {
	/* no change specified, revert to 1/2 screen size */
	newxScale = raster_UNITSCALE / 2;
	newyScale = raster_UNITSCALE / 2; }

    /* Store new scaling factors. */
    ras->xScale = newxScale;
    ras->yScale = newyScale;

    DEBUG(("chosen size inches: %0.4f x %0.4f   points:  %ld x %ld   (scaled 0x%lx  0x%lx)  \n", 
	    newdw, newdh,
	    (w * ras->xScale + (raster_UNITSCALE/2)) / raster_UNITSCALE,
	    (h * ras->yScale + (raster_UNITSCALE/2)) / raster_UNITSCALE, 
	    newxScale, newyScale));

    /* display the new size */
    sprintf(request, "Print size is now height %0.2f width %0.2f in. ", 
	     h / 72.0 * ras->yScale / raster_UNITSCALE,
	     w / 72.0 * ras->xScale / raster_UNITSCALE);
    message::DisplayString(self, 0, request);
}

/* This keeps the orignal pixel image around so that
  one can extrapolate from the original rather then the
  scaled version of the image. */
static void ScaleCommand(class rasterview  *self)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    class pixelimage *pix;
    long x, y, w, h;
    long NewW, NewH;
    char c[10], inbuf[50], request[150];

    if (ras == NULL || (pix = (ras)->GetPix()) == NULL) return;

    /* Until Scaling works while Zoomed... */
    if (NotFullSize(self)) return;

    rectangle_GetRectSize(&self->DesiredSelection, &x, &y, &w, &h);

    /* ask for scaling factor as a number of pixels or relative
	to current size */
    sprintf(request, "Change scale via relative size or absolute pixel [relative]: ");
    AskOrCancel(self, request, inbuf);
    sscanf(inbuf, "%[ar]", &c[0]);
    if (strcmp(c,"a")==0) {
	/* Ask for absolute values */
	sprintf(request, "New pixel width [%ld]: ", w);
	AskOrCancel(self, request, inbuf);
	/* If there is a value returned then if the value cannot be parsed then error else calculate new height and default the new height correspondingly. Else default the new height and width to 1. */
	if (*inbuf) {
	    if (sscanf(inbuf, "%ld", &NewW) != 1) {		
		DisplayAndReturn(self, "Value must be digits with no decimal point."); }
	    else NewH = (h * NewW)/w; }
	else {
	    NewW = w;
	    NewH = h; }
	sprintf(request, "New pixel height [%ld]: ", NewH);

	AskOrCancel(self, request, inbuf);
	/* if there is a value returned and it cannot be parsed then error. */
	if (*inbuf && sscanf(inbuf, "%ld", &NewH) != 1)		
	    DisplayAndReturn(self, "Value must be digits with no decimal point.");
	if (! *inbuf) NewH = (h * NewW)/w;
    }
    else {
	float ScaleW, ScaleH;
	/* Ask for relative values. */
	AskOrCancel(self, "New relative width [1.0000]: ", inbuf);
	if (*inbuf) {
	    if (sscanf(inbuf, "%f", &ScaleW) != 1) {
		DisplayAndReturn(self,
				 "Value must be digits with at most one decimal point."); }
	    else ScaleH = ScaleW; }
	else {
	    ScaleW = 1.0;
	    ScaleH = 1.0; }
	sprintf(request, "New relative height [%0.4f]: ", ScaleH);

	AskOrCancel(self, request, inbuf);
	if (*inbuf && sscanf(inbuf, "%f", &ScaleH) != 1) {
	    DisplayAndReturn(self,
			     "Value must be digits with at most one decimal point."); }
	if (! *inbuf) ScaleH = ScaleW;
	NewW = (long)(w * ScaleW);
	NewH = (long)(h * ScaleH);
    }

    sprintf(request, "New pixel (width, height): (%ld, %ld)", NewW, NewH);
    message::DisplayString(self, 0, request);

    DEBUG(("Original is%s NULL\n", ((self->Original == NULL) ? "" : " not")));
    DEBUG(("New Absolute: (%ld,%ld)\n", NewW, NewH));

    if (self->Original == NULL) {
	self->Original = (class rasterimage *)(pix)->Clone();
	(pix)->SetBitsPtr( NULL);
	(self->Original)->GetScaledSubraster(
				       &self->DesiredSelection, NewW, NewH, pix);
    }
    else {
	struct rectangle sub, original;
	/* map Desired Selection within scaled version to selection within the Original */
	rectangle_SetRectSize(&original, 0, 0,
			      (self->Original)->GetWidth(),
			      (self->Original)->GetHeight());
	if (FullySelected(self))
	    sub = original;
	else {
	    float wscale =
	      (self->Original)->GetWidth()/(pix)->GetWidth();
	    float hscale =
	      (self->Original)->GetHeight()/(pix)->GetHeight();
	    rectangle_SetRectSize(&sub,
				  (long)(rectangle_Left(&self->DesiredSelection)*wscale),
				  (long)(rectangle_Top(&self->DesiredSelection)*hscale),
				  (long)(rectangle_Width(&self->DesiredSelection)*wscale),
				  (long)(rectangle_Height(&self->DesiredSelection) * hscale));
	    rectangle_IntersectRect(&sub, &sub, &original); }
	DEBUG(("Original: (%ld,%ld,%ld,%ld)\n",
	       rectangle_Left(&original), rectangle_Top(&original),
	       rectangle_Width(&original), rectangle_Height(&original)));
	DEBUG(("Selection: (%ld,%ld,%ld,%ld)\n",
	       rectangle_Left(&sub), rectangle_Top(&sub),
	       rectangle_Width(&sub), rectangle_Height(&sub)));
	DEBUG(("%s: %p\n", (self->Original)->GetTypeName(), self->Original));

	(self->Original)->GetScaledSubraster( &sub, NewW, NewH, pix);
    }

    rectangle_SetRectSize(&self->DesiredSelection, 0, 0, NewW, NewH);
    rectangle_SetRectSize(&self->ViewSelection, 0, 0, NewW, NewH);

    if (NotFullSize(self))
	rasterview_ZoomToVisualBounds(self, 0, 0);

    rasterview_CenterViewSelection(self);

    (pix)->NotifyObservers( rasterview_SCALECHANGED);
}

static void ScaleReplaceCommand(class rasterview  *self)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    class rasterimage *pix;
    FILE *pasteFile;
    static const char hdr[] = "\\begindata{raster,";
    const char *hx = hdr;
    int c;

    if (ras == NULL || (pix = (ras)->GetPix()) == NULL) return;

    /* Mostly taken from ReplaceCommand */
    pasteFile = ((self)->GetIM())->FromCutBuffer();

    /* if the file does not begin with a raster, 
	we may as well scan until we find one XXX */

    while ((c = getc(pasteFile)) != EOF && *hx)
	if (c == *hx) hx++;
	else hx = (c == *hdr) ? hdr+1 : hdr;
    if (*hx) 
	message::DisplayString((class view *)self, 0, "No raster found in cut buffer");
    else {
	class raster *RasFromPaste = raster::Create(0,0);
	class rasterimage *NewPix;
	struct rectangle R;
	long x, y, w, h;

	while ((c=getc(pasteFile)) != '\n' && c != EOF) 
	{}	/* skip to end of header */

	rectangle_GetRectSize(&self->DesiredSelection, &x, &y, &w, &h);

	(RasFromPaste)->Read( pasteFile, 1);
	NewPix = (RasFromPaste)->GetPix();

	(NewPix)->GetScaledSubraster( &RasFromPaste->subraster,
				       w, h, NewPix);
	rectangle_SetRectSize(&R, 0, 0, w, h);
	(pix)->BlitSubraster( x, y, NewPix, &R, pixelimage_COPY);

	(RasFromPaste)->Destroy();

	(pix)->NotifyObservers( raster_BITSCHANGED);
    }
    ((self)->GetIM())->CloseFromCutBuffer( pasteFile);
}

void rasterview_RealPostMenus(class rasterview  *self)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    class rasterimage *pix = NULL;
    long menumask;

    if ( !self->HasInputFocus ) return;
    if (ras != NULL) pix = (ras)->GetPix();

    /* choose 'menumask' value */
    if (pix == NULL)
	menumask = menuNoPix;
    else { 
	if (ras->readOnly) {
	    /* XXX probably should have a readonly bit in the raster data object */
	    menumask = menuReadOnly;
	    if (self->InMessages) {
		/* XXX in  messages allow Crop, Uncrop, and, if fully selected, Negative */
		if (RegionSelect(self))
		    menumask |= menuPan | menuCenter;
		else if (Pan(self))
		    menumask |= menuSelect;
		if (Uncropped(self) && FullySelected(self))
		    /* Selection and subraster are both full image, include Negative */
		    menumask |= menuNegative;
	    }
	    else {
		if (RegionSelect(self))
		    menumask |= menuPan | menuCenter;
		else if (Pan(self))
		    menumask |= menuSelect;
	    }
	}
	else {
	    menumask = menuReadWrite;

	    if (RegionSelect(self))
		menumask |= menuTouchUp | menuPan | menuCenter;
	    else if (TouchUp(self))
		menumask |= menuSelect | menuPan;
	    else if (Pan(self))
		menumask |= menuSelect | menuTouchUp;
	    else if (Tool(self))
		menumask |= menuSelect | menuTouchUp | menuPan;
	    if (FullSize(self))
		menumask |= menuScale;
	    if (Uncropped(self) && FullySelected(self)) {
		/* Selection and subraster are both full image, include Rotate */
		/* Until this work in Zoomed mode: */
		if (FullSize(self)) {
		    menumask |= menuRotate; }
		if (! self->Shrunken && MatExtendPossible(self))
		    menumask |= menuExtend;
	    }

	    if (!self->inset)
		menumask |= menuInsetCreate;
	    else
		menumask |= menuInsetThere;
	}

	if (RegionSelect(self)) {
	    if (rectangle_Left(&self->DesiredSelection) != self->Xscroll/self->Scale
		|| rectangle_Top(&self->DesiredSelection) != self->Yscroll/self->Scale
		|| rectangle_IsEmptyRect(&self->DesiredSelection))
		menumask |= menuUpperLeft;
	    if (! rectangle_IsEqualRect(&self->ViewSelection, &self->DesiredSelection))
		menumask |= menuSelectAll; }

	if (Cropped(self)) {
	    /* Until this work in Zoomed mode: */
	    if (FullSize(self)) {
		menumask |= menuUncrop; 
		if(! FullySelected(self))
		    menumask |= menuCrop;
	    }
	}
	else {
	    /* Until Zooming while Cropped works. */
	    menumask |= menuZoomIn;
	    if (! FullySelected(self)) {
		/* Until this work in Zoomed mode: */
		if (FullSize(self))
		    menumask |= menuCrop; 
	    }

	    if (NotFullSize(self)) {
		menumask |= menuZoomOut;
		menumask |= menuMoveDB;
		if (! self->DisplayBoxHidden)
		    menumask |= menuHideDB; 
	    }
	}

	if (self->toolset)
	    menumask |= menuToolsetKill;
	else
	    menumask |= menuToolset;

    } /* end else [there is a pix] */

    if ( ! self->Shrunken && RastersInitiallyShrunk && self->embedded) 
	menumask |= menuShrink;

    DEBUG(("MenuMask: 0x%lx\n", menumask));

    if ((self->Menus)->SetMask( menumask))
	(self)->PostMenus( NULL);
}

void rasterview::PostMenus(class menulist  *ml)
{
    /* Enable the menus for this object. */
    (this->Menus)->ClearChain();
    if (ml) (this->Menus)->ChainBeforeML( ml, (long)ml);
    (this)->view::PostMenus( this->Menus);
}


/*  -----------  Added 4/6/89 -------------- */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *	
 *	Override methods
 *	
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#define DEFINEPROC(name, proc, registry, module, doc) proctable::DefineProc(name, (proctable_fptr)proc, registry, module, doc)

boolean rasterview::InitializeClass()
{
    struct proctable_Entry *proc = NULL;
    DEBUG(("ENTER rasterview__InitializeClass\n"));
    ::Menus = new menulist;
    Keymap = new keymap;

    proc = DEFINEPROC("rasterv-toolset-create", rasterview_MakeToolsetProc, &rasterview_ATKregistry_ , NULL, "Creates a window containing editing tools.");
    (::Menus)->AddToML( "Raster~30,Toolset~80", proc, 0, menuToolset); 

    proc = DEFINEPROC("rasterv-toolset-destroy", rasterview_KillToolsetProc, &rasterview_ATKregistry_ , 0, "Deletes toolset window.");
    (::Menus)->AddToML( "Raster~30,Remove Toolset~81", proc, 0, menuToolsetKill);

    proc = DEFINEPROC("rasterv-toggle-coord-display", rasterview_ToggleCoordProc, &rasterview_ATKregistry_ , 0, "Toggles display of mouse coordinates.");
    (::Menus)->AddToML( "Raster~30,Show/Hide Coords~57", proc, 0, 0);

    proc = DEFINEPROC("rasterv-overlay-inset", rasterview_OverlayInsetProc, &rasterview_ATKregistry_ , 0, "Overlay an inset on the raster view.");
    (::Menus)->AddToML( "Raster~30,Overlay Inset~72", proc, 0, menuInsetCreate);

    proc = DEFINEPROC("rasterv-remove-inset", rasterview_RemoveInsetProc, &rasterview_ATKregistry_ , 0, "Remove the overlaid inset.");
    (::Menus)->AddToML( "Raster~30,Remove Inset~76", proc, 0, menuInsetThere);

    proc = DEFINEPROC("rasterv-resize-inset", rasterview_ResizeInsetProc, &rasterview_ATKregistry_ , 0, "Resize the overlaid inset.");
    (::Menus)->AddToML( "Raster~30,Resize Inset~77", proc, 0, menuInsetThere);

    proc = DEFINEPROC("rasterv-imprint-inset", rasterview_ImprintInsetProc, &rasterview_ATKregistry_ , 0, "Imprint the overlaid inset.");
    (::Menus)->AddToML( "Raster~30,Paste Down Inset~75", proc, 99, menuInsetThere);

    proc = DEFINEPROC("rasterv-copy-subraster", CopyCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Copy selected subraster to kill-buffer.");
    (Keymap)->BindToKey( "\033w", proc, 0);	/* ESC - w */
    (::Menus)->AddToML( "Copy~3", proc, 0, menuCopy);

    proc = DEFINEPROC("rasterv-replace-subraster", ReplaceCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Replace selected subraster from kill-buffer.");
    (Keymap)->BindToKey( "\033y", proc, 0);	/* ESC - y */
    (::Menus)->AddToML( "Replace~13", proc, 0, menuOps);

    proc = DEFINEPROC("rasterv-replace-subraster", ReplaceCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Replace selected subraster from kill-buffer.");
    (Keymap)->BindToKey( "\031", proc, 0);	/* ^Y */
    (::Menus)->AddToML( "Replace~13", proc, 0, menuOps);

    proc = DEFINEPROC("rasterv-scale-replace", ScaleReplaceCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Scale image in paste buffer to fit in selected Region");
    (Keymap)->BindToKey( "\033\031", proc, 0);	/* ESC ^Y */
    (::Menus)->AddToML( "Replace Scaled~15", proc, 0, menuOps);

    proc = DEFINEPROC("rasterv-negative", ModifyCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Interchange black and white within selection");
    (Keymap)->BindToKey( "\033n", proc, -1);	/* ESC - n */
    (::Menus)->AddToML( "Raster Ops~20,Negative~21", proc, -1, menuNegative);

    proc = DEFINEPROC("rasterv-white", ModifyCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Set white within selection");
    (Keymap)->BindToKey( "\033W", proc, 0);	/* ESC - W */
    (::Menus)->AddToML( "Raster Ops~20,White~22", proc, 0, menuOps);

    proc = DEFINEPROC("rasterv-black", ModifyCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Set black within selection");
    (Keymap)->BindToKey( "\033B", proc, 1);	/* ESC - B */
    (::Menus)->AddToML( "Raster Ops~20,Black~23", proc, 1, menuOps);

    proc = DEFINEPROC("rasterv-gray", ModifyCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Set gray within selection");
    (Keymap)->BindToKey( "\033G", proc, 2);	/* ESC - G */
    (::Menus)->AddToML( "Raster Ops~20,Gray~24", proc, 2, menuOps);

    proc = DEFINEPROC("rasterv-mirror-left-right", ModifyCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Interchange left and right within selection");
    (Keymap)->BindToKey( "\033L", proc, 3);	/* ESC - L */
    (::Menus)->AddToML( "Raster Ops~20,Mirror LR~25", proc, 3, menuOps);

    proc = DEFINEPROC("rasterv-mirror-up-down", ModifyCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Interchange top and bottom within selection");
    (Keymap)->BindToKey( "\033U", proc, 4);	/* ESC - U */
    (::Menus)->AddToML( "Raster Ops~20,Mirror UD~26", proc, 4, menuOps);

    proc = DEFINEPROC("rasterv-rotate", rasterview_RotateCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Rotate entire raster image one-quarter turn clockwise");
    (Keymap)->BindToKey( "\033R", proc, 0);	/* ESC - R */
    (::Menus)->AddToML( "Raster Ops~20,Rotate~27", proc, 0, menuRotate);

    proc = DEFINEPROC("rasterv-scale", ScaleCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Scale the image");
    (Keymap)->BindToKey( "\033S", proc, 0);	/* ESC - S */	
    (::Menus)->AddToML( "Raster Ops~20,Scale~32", proc, 0, menuScale);

    proc = DEFINEPROC("rasterv-shrink-image", ShrinkCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Shrink the image of the raster to a button");
    (Keymap)->BindToKey( "\033s", proc, 0);	/* ESC - s */
    (::Menus)->AddToML( "Raster Ops~20,Shrink to Button~67", proc, 0, menuShrink);

    proc = DEFINEPROC("rasterv-center-image", rasterview_CenterCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Center the Selected Region");
    (Keymap)->BindToKey( "\033c", proc, 0);	/* ESC - c */
    (::Menus)->AddToML( "Raster Ops~20,Center Region~72", proc, 0, menuCenter);

    proc = DEFINEPROC("rasterv-upperleft-image", UpperLeftCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Upper Left the Selected Region");
    (Keymap)->BindToKey( "\033c", proc, 0);	/*  */
    (::Menus)->AddToML( "Raster Ops~20,Upper Left Region~73", proc, 0, menuUpperLeft);

    proc = DEFINEPROC("rasterv-zoom-in", rasterview_ZoomInCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Zoom In using upper left of selected region as starting point.");
    (Keymap)->BindToKey( "\033Z", proc, 0);	/* ESC - Z */
    (::Menus)->AddToML( "Raster~20,Zoom In~12", proc, 0, menuZoomIn);

    proc = DEFINEPROC("rasterv-zoom-out", rasterview_ZoomOutCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Zoom Out");
    (Keymap)->BindToKey( "\033z", proc, 0);	/* ESC - z */
    (::Menus)->AddToML( "Raster~20,Zoom Out~13", proc, 0, menuZoomOut);

    proc = DEFINEPROC("rasterv-zoom-out-to-normal-size", NormalSizeCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Normal Size");
    (Keymap)->BindToKey( "\033N", proc, 0);	/* ESC - N */
    (::Menus)->AddToML( "Raster~20,Normal Size~15", proc, 0, menuZoomOut);

    proc = DEFINEPROC("rasterv-select-entire", SelectAllCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Select entire raster");
    (Keymap)->BindToKey( "\033N", proc, 0);	/* */
    (::Menus)->AddToML( "Raster~20,Select All~22", proc, 0, menuSelectAll);

    proc = DEFINEPROC("rasterv-region-select", rasterview_RegionSelectCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Region Select Mode");
    (Keymap)->BindToKey( "\033\030R", proc, 0);	/* ESC - ^X R */
    (::Menus)->AddToML( "Raster~20,Region Select~22", proc, 0, menuSelect);

    proc = DEFINEPROC("rasterv-touchup", TouchUpCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Touch Up Mode");
    (Keymap)->BindToKey( "\033\030T", proc, 0);	/* ESC - ^X T */
    (::Menus)->AddToML( "Raster~20,Touch Up~23", proc, 0, menuTouchUp);

    proc = DEFINEPROC("rasterv-pan", PanCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Pan Mode");
    (Keymap)->BindToKey( "\033\030P", proc, 0);	/* ESC - ^X P */
    (::Menus)->AddToML( "Raster~20,Pan~24", proc, 0, menuPan);

    proc = DEFINEPROC("rasterv-move-display-box", rasterview_MoveDisplayBoxCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Move the location of the Display Box. ");
    (Keymap)->BindToKey( "\033M", proc, 1);	/* ESC - M */
    (::Menus)->AddToML( "Raster~20,Move Display Box~32", proc, 0, menuMoveDB);

    proc = DEFINEPROC("rasterv-hide-display-box", HideDisplayBoxCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Hide the Display Box. ");
    (Keymap)->BindToKey( "\033H", proc, 1);	/* ESC - H */
    (::Menus)->AddToML( "Raster~20,Hide Display Box~34", proc, 0, menuHideDB);

    proc = DEFINEPROC("rasterv-set-print-scaling", SetPrintSizeCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Set the scaling factors used when printing");
    (Keymap)->BindToKey( "\033P", proc, 0);	/* ESC - P */
    (::Menus)->AddToML( "Raster~30,Set Print Size~53", proc, 0, menuOps);

    proc = DEFINEPROC("rasterv-extend-to-mat", ExtendToMatCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Add pixels to image to reach the bordering matte");
    (Keymap)->BindToKey( "\033X", proc, 0);	/* ESC - X */
    (::Menus)->AddToML( "Raster~30,Extend to Mat~55", proc, 0, menuExtend);

    proc = DEFINEPROC("rasterv-crop", CropCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Crop image to the selection");
    (Keymap)->BindToKey( "\033C", proc, 0);	/* ESC - C */
    (::Menus)->AddToML( "Raster~30,Crop~63", proc, 0, menuCrop);

    proc = DEFINEPROC("rasterv-uncrop", UncropCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Revert image to entire raster");
    (Keymap)->BindToKey( "\033c", proc, 0);	/* ESC - c */
    (::Menus)->AddToML( "Raster~30,Uncrop~65", proc, 0, menuUncrop);

    proc = DEFINEPROC("rasterv-toggle-debug", ToggleDebug,
				 &rasterview_ATKregistry_ , 0,
				 "Toggle the rasterview debug flag");

    proc = DEFINEPROC("rasterv-read-file", ReadFileCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Read a raster file");
    (Keymap)->BindToKey( "\033\022r", proc, 0);	/* ESC - ^R - r */
    (::Menus)->AddToML( "File,Read Raster~12", proc, 0, menuRead);

    proc = DEFINEPROC("rasterv-read-xwdfile",
				 RasterIOCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Read file in X Window Dump format");
    (Keymap)->BindToKey( "\033\022x", proc, Inxwd);
    /* ESC - ^R - x*/
    (::Menus)->AddToML( "File,Read X Window Dump~13", proc, (long)Inxwd, menuRead);

    proc = DEFINEPROC("rasterv-read-xbmfile",
				 RasterIOCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Read file in X Bitmap format");
    (Keymap)->BindToKey( "\033\022b", proc, Inxbm);
    /* ESC - ^R - b*/
    (::Menus)->AddToML( "File,Read X Bitmap~14", proc, (long)Inxbm, menuRead);

    proc = DEFINEPROC("rasterv-read-file", ReadFileCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Read a raster file");
    (Keymap)->BindToKey( "\033\022r", proc, 0);	/* ESC - ^R - r */
    (::Menus)->AddToML( "Raster I/O~42,Read Raster~12", proc, 0, menuRead);

    proc = DEFINEPROC("rasterv-read-macpaint", RasterIOCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Read a file allegedly in MacPaint format");
    (Keymap)->BindToKey( "\033\022Im", proc, (long)InMacPaint);	/* ESC - ^R - I - m */
    (::Menus)->AddToML( "Raster I/O~42,Read MacPaint~22", proc, (long)InMacPaint, menuRead);

    proc = DEFINEPROC("rasterv-write-macpaint", RasterIOCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Write file in MacPaint format");
    (Keymap)->BindToKey( "\033\022IM", proc, (long)OutMacPaint);	/* ESC - ^R - I - M */
    (::Menus)->AddToML( "Raster I/O~42,Write MacPaint~24", proc, (long)OutMacPaint, menuCopy);

    proc = DEFINEPROC("rasterv-write-postscript", RasterIOCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Write file in postscript format");
    (Keymap)->BindToKey( "\033\022IP", proc, (long)OutPostscript);	/* ESC - ^R - I - P */
    (::Menus)->AddToML( "Raster I/O~42,Write Postscript~32", proc, (long)OutPostscript, menuCopy);

    proc = DEFINEPROC("rasterv-read-rasterfile", ReadFileCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Read a file allegedly in the old ITC RasterFile format");
    (Keymap)->BindToKey( "\033\022r", proc, 0);	/* ESC - ^R - r *//* same as Read Raster above */
    (::Menus)->AddToML( "Raster I/O~42,Read Old Raster~42", proc, 0, menuRead);

    /* paul's attempt to add XWD I/O : */

    proc = DEFINEPROC("rasterv-read-xwdfile",
				 RasterIOCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Read file in X Window Dump format");
    (Keymap)->BindToKey( "\033\022x", proc, 0);
    /* ESC - ^R - x*/
    (::Menus)->AddToML( "Raster I/O~42,Read X Window Dump~52", proc, (long)Inxwd, menuRead);


    proc = DEFINEPROC("rasterv-write-xwdfile",
				 RasterIOCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Write file in X Window Dump format");
    (Keymap)->BindToKey( "\033\022X", proc, 0);
    /* ESC - ^R - X*/
    (::Menus)->AddToML( "Raster I/O~42,Write X Window Dump~54", proc, (long)Outxwd, menuCopy);

/* end of paul's attempt to add XWD I/O : */

/* paul's attempt to add X Bitmap I/O : */

    proc = DEFINEPROC("rasterv-read-xbmfile",
				 RasterIOCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Read file in X Bitmap format");
    (Keymap)->BindToKey( "\033\022b", proc, 0);
    /* ESC - ^R - b*/
    (::Menus)->AddToML( "Raster I/O~42,Read X Bitmap~62", proc, (long)Inxbm, menuRead);


    proc = DEFINEPROC("rasterv-write-xbmfile",
				 RasterIOCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Write file in X Bitmap format");
    (Keymap)->BindToKey( "\033\022B", proc, 0);
    /* ESC - ^R - B*/
    (::Menus)->AddToML( "Raster I/O~42,Write X Bitmap~64", proc, (long)Outxbm, menuCopy);

/* end of paul's attempt to add X Bitmap I/O : */

#ifdef X11_ENV
    proc = DEFINEPROC("rasterv-make-xwd",
				 RasterIOCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Dump X window and read.");
    (Keymap)->BindToKey( "\033\022Iw", proc, 0);
    /* ESC - ^R - I - w*/
    (::Menus)->AddToML( "Raster I/O~42,Make Window Dump~70", proc, (long)MakeWD, menuRead);


    proc = DEFINEPROC("rasterv-make-asnap",
				 RasterIOCommand,
				 &rasterview_ATKregistry_ , 0,
				 "Sweep area and read.");
    (Keymap)->BindToKey( "\033\022Ia", proc, 0);
    /* ESC - ^R - I - a*/
    (::Menus)->AddToML( "Raster I/O~42,Make Area Dump~71", proc, (long)MakeAsnap, menuRead);
#endif

    RastersInitiallyShrunk = environ::GetProfileSwitch("RastersInitiallyShrunk", FALSE);
    DEBUG(("LEAVE rasterview__InitializeClass\n"));
    return TRUE;
}


rasterview::rasterview()
{
	ATKinit;
	this->UpdateWanted=FALSE;
	this->InsetUpdateWanted=FALSE;
    this->Menus = (::Menus)->DuplicateML( this);
    this->Keystate = keystate::Create(this, Keymap);
    this->Cursor[0] = NULL;
    this->Cursor[1] = NULL;
    this->Cursor[2] = NULL;
    this->Cursor[3] = NULL;
    this->Cursor[4] = NULL;
    rectangle_EmptyRect(&this->PixChanged);
    this->needsFullUpdate = TRUE;
    this->OnScreen = FALSE;
    this->embedded = TRUE;
    this->DraggingEdge = 0;
    this->HasInputFocus = FALSE;
    this->IgnoreUntilNextUpTransition = FALSE;
    this->Mode = 0;
    rectangle_EmptyRect(&this->ViewSelection);
    this->Scale = 1;
    this->Original = NULL;
    this->Expansion = NULL;
    this->TouchUpX = this->TouchUpY = 0;
    this->HighlightIsGrey = FALSE;
    this->Xscroll = this->Yscroll = 0;
    this->InMessages = FALSE;
    this->CheckedParent = FALSE;
    rectangle_EmptyRect(&this->CurSelection);
    rectangle_EmptyRect(&this->DesiredSelection);
    this->Shrunken = RastersInitiallyShrunk;
    this->NeedsCentering = TRUE;

    this->ShowCoords = 0;

    this->inset = NULL;
    rectangle_EmptyRect(&this->InsetBox);

    rectangle_EmptyRect(&this->DisplayBox);
    rectangle_EmptyRect(&this->DisplayBoxSelection);
    this->DBXscroll = this->DBYscroll = 0;
    this->MovingDisplayBox = FALSE;
    this->DisplayBoxHidden = TRUE;
    this->DisplayBoxOverlapped = FALSE;

    this->GreyPattern = NULL;
    this->WhitePattern = NULL;
    this->BlackPattern = NULL;

    this->toolset = NULL;

    THROWONFAILURE( TRUE);
}

rasterview::~rasterview()
{
	ATKinit;

    int ix;

    delete this->Menus;
    if (this->Original != NULL) {
	(this->Original)->Destroy();
	this->Original = NULL; }
    delete this->Keystate;

    for (ix=0; ix<5; ix++) {
	if (this->Cursor[ix] != NULL)
	    delete this->Cursor[ix];
    }

    if (this->toolset) 
	(this)->DestroyToolset();

    if (this->inset) {
	(this->inset)->UnlinkTree();
	(this->inset)->Destroy();
    }

}

class view * rasterview::GetApplicationLayer()
{
    class scroll *view;
    view = scroll::CreateScroller(this, scroll_LEFT | scroll_BOTTOM, environ::GetProfile("RasterScrollClass"));
    this->embedded = FALSE;
    this->Shrunken = FALSE;
    /*   (keystate::Create(this, Keymap))->AddBefore( this->Keystate);

     Huh?  this->Keystate already is a keystate based on Keymap, what's the point of duplicating it? If it is needed the extra keystate should be deleted sometime...
     -Rob Ryan 7/17/95 */
    (this)->WantInputFocus( this);
    return (class view *) view;
}

/* ------ following material was added 5/28/92 ------ */

int rasterview::GetMode()
{
    return this->Mode;
}

void rasterview::CopySelection()
{
    CopyCommand(this);
}

