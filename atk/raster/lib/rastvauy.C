/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h> /* strings.h */

#define AUXMODULE 1
#include <rasterview.H>

#include <im.H>
#include <frame.H>
#include <buffer.H>
#include <view.H>
#include <text.H>
#include <dataobject.H>
#include <sys/param.h> /* Defines MAXPATHLEN among other things */
#include <menulist.H>
#include <cursor.H>
#include <message.H>
#include <rect.h>
#include <rasterimage.H>
#include <raster.H>
#include <rastoolview.H>
#include <heximage.H>
#include <dispbox.h>

 
#include <rastvaux.h>


void rasterview_ToggleCoordProc(class rasterview  *self, char  *rock);
void rasterview_MakeToolsetProc(class rasterview  *self, char  *rock);
void rasterview_KillToolsetProc(class rasterview  *self, char  *rock);
static boolean objecttest(class rasterview  *self, const char  *name, const char  *desiredname);
void rasterview_OverlayInsetProc(class rasterview  *self, const char  *rock);
void rasterview_RemoveInsetProc(class rasterview  *self, char  *rock);
void rasterview_ResizeInsetProc(class rasterview  *self, char  *rock);
void rasterview_ImprintInsetProc(class rasterview  *self, long  rock);


/* clean happy one-pixel draw. Displays correctly in rasterview self, even if zoomed, but does *not* notify other observers. If (x, y) is out of bounds, nothing happens. bit may be TRUE, FALSE, or DRAW_REVERSE_PIXEL. Note that this does more than the internal function SetPixel(). */
void rasterview::SetPixel(class raster  *ras, long  x , long  y, int  bit)
{
    class rasterimage *pix;
    struct rectangle SR;

    if ((ras == NULL) || ((pix = (ras)->GetPix()) == NULL)) return;

    SR = this->ViewSelection;
    /* Note that coordinates are constrained to refer to actual pixels
      (left+width and top+height are the addresses of pixels
	just outside the raster) */
    if ((x < rectangle_Left(&SR)) 
	 || (x > rectangle_Left(&SR)+rectangle_Width(&SR)-1) 
	 || (y < rectangle_Top(&SR)) 
	 || (y > rectangle_Top(&SR)+rectangle_Height(&SR)-1)) {
	return;
    }

    if (bit==DRAW_REVERSE_PIXEL) {
	bit = !	(pix)->GetPixel( x, y); 
    }
    if (NotFullSize(this)) 
	rasterview_SetPixelBehindDisplayBox(this, this->Expansion, x, y, bit);
    else {
	/* The following line does not work in X windows currently.
	 graphic::SetBitAtLoc(rasterview_GetDrawable(self),
			    x - self->Xoff, y - self->Yoff, bit); */
	struct rectangle sub;
	rectangle_SetRectSize(&sub, x - this->Xoff, y - this->Yoff, 1, 1);
	(this)->SetTransferMode( graphic::COPY);
	(this)->FillRect( &sub, ((bit) ? this->BlackPattern : this->WhitePattern));
	(pix)->SetPixel( x, y, ((bit) ? 1 : 0)); 
    }
}

/* the following method replicates code from rasterview__SetPixel(). This is just to improve the speed. Any changes to __SetPixel() should be added to  __PatternSetPixel() also. */

/* like rasterview_SetPixel(), but uses an 8x8 pattern instead of solid black or white. If pattern is NULL, invert the pixel. */
void rasterview::PatternSetPixel(long  x , long  y, const unsigned char *pattern)
{
    class rasterimage *pix;
    struct rectangle SR;
    int bit;
    class raster *ras = (class raster *)(this)->GetDataObject();

    if ((ras == NULL) || ((pix = (ras)->GetPix()) == NULL)) return;

    SR = this->ViewSelection;
    /* Note that coordinates are constrained to refer to actual pixels
      (left+width and top+height are the addresses of pixels
	just outside the raster) */
    if ((x < rectangle_Left(&SR)) 
	 || (x > rectangle_Left(&SR)+rectangle_Width(&SR)-1) 
	 || (y < rectangle_Top(&SR)) 
	 || (y > rectangle_Top(&SR)+rectangle_Height(&SR)-1)) {
	return;
    }

    if (!pattern) {
	bit = !	(pix)->GetPixel( x, y); 
    }
    else {
	bit = (pattern[y&7] >> (x&7)) & 1;
    }

    if (NotFullSize(this)) 
	rasterview_SetPixelBehindDisplayBox(this, this->Expansion, x, y, bit);
    else {
	/* The following line does not work in X windows currently.
	 graphic::SetBitAtLoc(rasterview_GetDrawable(self),
			     x - self->Xoff, y - self->Yoff, bit); */
	struct rectangle sub;
	rectangle_SetRectSize(&sub, x - this->Xoff, y - this->Yoff, 1, 1);
	(this)->SetTransferMode( graphic::COPY);
	(this)->FillRect( &sub, ((bit) ? this->BlackPattern : this->WhitePattern));
	(pix)->SetPixel( x, y, ((bit) ? 1 : 0)); 
    }

}

/* draws up to 64 pixels according to an 8x8 brush. If the brush is NULL, draw just one pixel. Uses rasterview_PatternSetPixel(). */
void rasterview::BrushSetPixel(long  x , long  y, const unsigned char *pattern, const unsigned char *brush)
{
    if (!brush) {
	(this)->PatternSetPixel( x, y, pattern);
    }
    else {
	int ix, jx;
	char ch;

	x -= 3;
	y -= 3;
	for (jx=0; jx<8; jx++) {
	    ch = brush[jx];
	    for (ix=0; ix<8; ix++) {
		if ((ch>>ix)&1) 
		    (this)->PatternSetPixel( x+ix, y+jx, pattern);
	    }
	}
    }
}

void rasterview_ToggleCoordProc(class rasterview  *self, char  *rock)
{
    self->ShowCoords = !(self->ShowCoords);
    
    if (self->ShowCoords)
	message::DisplayString(self, 20, "Mouse coordinates will be displayed.");
    else
	message::DisplayString(self, 20, "Mouse coordinates will not be displayed.");
}

/* set the selected region rectangle and redraw the highlight. R may be empty, and need not be within the raster boundaries (if it is, it will be trimmed or emptied).
  This does *not* redraw the scrollbars; to do that, use
      self->needsFullUpdate=FALSE;
      rasterview_WantUpdate(self, caller); 
*/
void rasterview::SetDesiredSelection(struct rectangle  *R)
{
    rectangle_IntersectRect(&this->DesiredSelection, R, &this->ViewSelection); 
    rasterview_CorrectHighlight(this);
}

void rasterview_MakeToolsetProc(class rasterview  *self, char  *rock)
{
    class raster *r = (class raster *)(self)->GetDataObject();
    class im *im;
    class frame *fr;
    class rastoolview *tv;
    boolean res;

    if (self->toolset)  {
	message::DisplayString(self, 40, "There is already a toolset window.");
	return;
    }

    if (!r) {
	message::DisplayString(self, 40, "This view has no data!");
	return;
    }

    im = im::Create(NULL);
    if (!im) {
	message::DisplayString(self, 40, "Unable to create new window.");
	return;
    }

    fr = new frame;
    if (!fr) {
	message::DisplayString(self, 40, "Unable to create window frame.");
	return;
    }

    /*frame_SetCommandEnable(fr, TRUE);*/
    (fr)->PostDefaultHandler( "message", (fr)->WantHandler( "message"));

    tv = new rastoolview;
    if (!tv) {
	message::DisplayString(self, 40, "Unable to create toolset.");
	return;
    }

    res = (tv)->SetPrimaryView( self);
    if (!res) {
	message::DisplayString(self, 40, "Unable to initialize toolset (this shouldn't happen).");
	return;
    }

    self->toolset = tv;
    (fr)->SetView( tv);
    /*(fr)->SetQuitWindowFlag( TRUE);*/
    (im)->SetView( fr);
    {   
	class buffer *buf = buffer::FindBufferByData(r); 
	char *nm;
	if (buf) {
	    nm = (buf)->GetFilename();
	    if (nm)
		(im)->SetTitle( nm);
	}
    }
    (tv)->WantInputFocus( tv);
    rasterview_RealPostMenus(self);
}

void rasterview_KillToolsetProc(class rasterview  *self, char  *rock)
{
    class im *toolim = NULL;
    class frame *toolfr = NULL;

    if (!self->toolset) {
	message::DisplayString(self, 40, "There is no toolset window.");
	return;
    }

    toolim = (self->toolset)->GetIM();
    if (toolim)
	toolfr = (class frame *)toolim->topLevel;

    (self->toolset)->RemoveObserver( self);
    (self)->RemoveObserver( self->toolset);

    (self->toolset)->SetMoribund( 1); /* ensure that toolset doesn't try to kill itself when it sees it's being unlinked */

    if (toolim) {
	(toolim)->SetView( NULL);
    }

    if (toolfr) {
	(toolfr)->SetView( NULL);
    }

    (self->toolset)->Destroy();
    self->toolset = NULL;

    if (toolim) {
	(toolim)->Destroy(); 
    }
    if (toolfr) {
	(toolfr)->Destroy();
    }

    (self)->SetPan(); /* does rasterview_RealPostMenus() */
}

void rasterview::DestroyToolset()
{
    rasterview_KillToolsetProc(this, 0);
}

static boolean objecttest(class rasterview  *self, const char  *name, const char  *desiredname)
{
    if(ATK::LoadClass(name) == NULL){
	char foo[640];
	sprintf(foo,"Can't load %s",name);
	message::DisplayString(self, 0, foo);
	return(FALSE);
    }
    if(!ATK::IsTypeByName(name,desiredname)){
	char foo[640];
	sprintf(foo,"%s is not a %s",name,desiredname);
	message::DisplayString(self, 0, foo);
	return(FALSE);
    }
    return(TRUE);
}

/* overlay an inset, using self->DesiredSelection for the box dimensions. insetname should be the name of a subclass of dataobject. If insetname is NULL, the user will be prompted for a class name. */
void rasterview::OverlayInset(const char  *insetname)
{
    rasterview_OverlayInsetProc(this, insetname);
}

void rasterview_OverlayInsetProc(class rasterview  *self, const char  *rock)
{
    char iname[100], buf[200];
    const char *viewname;
    long pf;
    class dataobject *dobj;
    class raster *ras;

    if (!self) return;
    if (self->inset) return;
    ras = (class raster *)(self)->GetDataObject();
    if (!ras || ras->readOnly) return;

    if (!((self)->GetIM())->SupportsOffscreen()) {
	message::DisplayString(self, 10, "Overlaid insets will not work in this window manager.");
	return;
    }

    if (rectangle_IsEmptyRect(&self->DesiredSelection)) {
	message::DisplayString(self, 10, "You must select a region for the inset.");
	return;
    }

    if (rock == NULL || ((long)rock >= 0 && (long)rock < 256)) {
	pf = message::AskForString(self, 20, "Data object to insert here [text] : ", 0, iname, sizeof(iname));
        if (pf < 0){
            message::DisplayString(self, 0, "Cancelled.");
            return;
	}
	if (strlen(iname)==0) {
	    strcpy(iname, "text");
	}
    }
    else {
        strncpy(iname, rock, sizeof(iname));
    }

    if (objecttest(self, iname, "dataobject") == FALSE) return;
    dobj = (class dataobject *)ATK::NewObject(iname);
    if (!dobj) {
	message::DisplayString(self, 10, "Unable to create object.");
	return;
    }

    if (ATK::IsTypeByName(iname, "text")) {
	message::DisplayString(self, 0, "Reading template...");
	((class text *)dobj)->ReadTemplate( "default", FALSE);
    }

    viewname = (dobj)->ViewName();

    self->inset = (class view *)ATK::NewObject(viewname);
    if (!self->inset) {
	sprintf(buf, "Unable to create view %s.", viewname);
	message::DisplayString(self, 10, buf);
	return;
    }

    (self->inset)->SetDataObject( dobj);

    (self->inset)->LinkTree( self);

    self->InsetBox = self->DesiredSelection;
    (self->inset)->InsertViewSize( self, self->InsetBox.left-self->Xoff, self->InsetBox.top-self->Yoff, self->InsetBox.width, self->InsetBox.height);
    self->InsetUpdateWanted = TRUE;

    rasterview_RealPostMenus(self);
    (self->inset)->WantInputFocus( self->inset);
    (self)->NotifyObservers( 0); 

    sprintf(buf, "Inserted %s inset at %ld,%ld (%ld by %ld)", iname, self->InsetBox.left, self->InsetBox.top, self->InsetBox.width, self->InsetBox.height);
    message::DisplayString(self, 10, buf);
}

/* remove the overlaid inset */
void rasterview::RemoveInset()
{
    rasterview_RemoveInsetProc(this, NULL);
}

void rasterview_RemoveInsetProc(class rasterview  *self, char  *rock)
{
    if (!self->inset) return;

    (self)->WantInputFocus( self);
    (self->inset)->UnlinkTree();
    (self->inset)->Destroy();
    self->inset = NULL;

    self->DesiredSelection = self->InsetBox;

    if (FullSize(self)) {
	rectangle_UnionRect(&self->PixChanged, &self->PixChanged, &self->InsetBox);
    }

    rasterview_RealPostMenus(self);
    (self)->NotifyObservers( 0); 
    (self)->WantUpdate( self);
}

/* resize the overlaid inset to the dimensions of self->DesiredSelection */
void rasterview::ResizeInset()
{
    rasterview_ResizeInsetProc(this, NULL);
}

void rasterview_ResizeInsetProc(class rasterview  *self, char  *rock)
{
    char buf[200];

    if (!self->inset) return;

    if (rectangle_IsEmptyRect(&self->DesiredSelection)) {
	message::DisplayString(self, 10, "You must select a region for the inset.");
	return;
    }

    if (FullSize(self)) {
	rectangle_UnionRect(&self->PixChanged, &self->PixChanged, &self->InsetBox);
    }

    self->InsetBox = self->DesiredSelection;
    (self->inset)->InsertViewSize( self, self->InsetBox.left-self->Xoff, self->InsetBox.top-self->Yoff, self->InsetBox.width, self->InsetBox.height);
    self->InsetUpdateWanted = TRUE;

    sprintf(buf, "Placed inset at %ld,%ld (%ld by %ld)", self->InsetBox.left, self->InsetBox.top, self->InsetBox.width, self->InsetBox.height);
    message::DisplayString(self, 10, buf);

    (self)->WantUpdate( self);
}

/* Imprint (paste down) the overlaid inset on the raster. */
void rasterview::ImprintInset(long  function)
{
    rasterview_ImprintInsetProc(this, (function % 16));
}

/* the rock determines the transfer mode (copy, or, xor, etc.) It should be a number between 0 and 15, as defined in graphic.ch. If it is not, it defaults to the toolset's selected mode, or COPY if there is no toolset. */
void rasterview_ImprintInsetProc(class rasterview  *self, long  rock)
{
    class im *offim;
    class graphic *offgr;
    class raster *ras;
    class rasterimage *pix, *pix2;
    struct rectangle R;

    DEBUG(("ImprintInsetProc: rock=%ld", rock));
    if (rock<0 || rock>=16) {
	if (self->toolset)
	    rock = (self->toolset)->GetPasteMode();
	else
	    rock = pixelimage_COPY;
    }
    DEBUG(("ImprintInsetProc: new rock=%ld", rock));

    if (!self->inset) return;

    ras = (class raster *)(self)->GetDataObject();
    if (!ras) return;
    pix = (ras)->GetPix();
    if (!pix) return;
    pix2 = new rasterimage;
    if (!pix2) return;

    self->DesiredSelection = self->InsetBox;
    (self)->WantInputFocus( self);
    (self->inset)->UnlinkTree();

    offim = im::CreateOffscreen((self)->GetIM(), self->InsetBox.width, self->InsetBox.height);

    if (!offim) {
	(self->inset)->Destroy();
	self->inset = NULL;

	if (FullSize(self)) {
	    rectangle_UnionRect(&self->PixChanged, &self->PixChanged, &self->InsetBox);
	}

	(self)->WantUpdate( self);
	message::DisplayString(self, 10, "Unable to create transfer pixmap");
	return;
    }

    (offim)->SetView( self->inset);
    (self->inset)->FullUpdate( view::FullRedraw, 0, 0, -1 ,-1);
    offgr = (offim)->GetDrawable();

    (pix2)->Resize( self->InsetBox.width, self->InsetBox.height);
    (offgr)->ReadPixImage( 0, 0, pix2, 0, 0, self->InsetBox.width, self->InsetBox.height);

    rectangle_SetRectSize(&R, 0, 0, self->InsetBox.width, self->InsetBox.height);

    if ((((class view *)offim)->drawable)->IsImageInverted())
	(pix2)->InvertSubraster( &R);

    (pix)->BlitSubraster( self->InsetBox.left, self->InsetBox.top, pix2, &R, rock);

    (pix2)->Destroy();
    (offim)->SetView(NULL);
    (offim)->Destroy();
    (self->inset)->Destroy();
    self->inset = NULL;
    
    /*rectangle_UnionRect(rasterimage_GetChanged(pix), rasterimage_GetChanged(pix), &self->InsetBox);*/

    rasterview_RealPostMenus(self);
    (self)->NotifyObservers( 0); 
    (pix)->NotifyObservers( rasterimage_DATACHANGED);
    message::DisplayString(self, 10, "Inset imprinted.");
}

void rasterview::LinkTree( class view  *parent )
{
    (this)->view::LinkTree( parent);
    if (parent && (this)->GetIM()) {
	if (this->inset)
	    (this->inset)->LinkTree( this);
    }
}

