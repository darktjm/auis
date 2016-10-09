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
#include <fontdesc.H>
#include <sys/param.h> /* Defines MAXPATHLEN among other things */
#include <menulist.H>
#include <keymap.H>
#include <keystate.H>
#include <cursor.H>
#include <proctable.H>
#include <message.H>
#include <scroll.H>
#include <environ.H>
#include <rect.h>
#include <texttroff.H>
#include <rasterimage.H>
#include <raster.H>
#include <rastoolview.H>
#include <heximage.H>
#include <imageio.H>
#include <cmuwm.H>
#include <print.H>
#include <dispbox.h>

#include <rastvaux.h>


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *	
 *	Methods
 *	
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*  -----------  Added  4/6/89 -------------- */
/* 
  Note:  for a lot of these I call the menu command functions.  At the
      end of these functions there is a PostMenus function call.  Since this
           function does nothing, I'm assuming that's not a problem.
   Note:  for FitToSize, I have no clue what the "Original" stuff is all
       about - I just copied the code from ScaleCommand.
*/

static void RedrawRaster(class rasterview  *self, enum view_UpdateType  type, long  left , long  top , long  width , long  height);

static void x_getinfo(class rasterview   *self, struct range   *total , struct range   *seen , struct range   *dot);
static long x_whatisat(class rasterview   *self, long   coordinate , long   outof);
static void  x_setframe(class rasterview  *self, int   position, long   coordinate , long   outof);
static void y_getinfo(class rasterview   *self, struct range   *total , struct range   *seen , struct range   *dot);
static long y_whatisat(class rasterview  *self, long   coordinate , long   outof);
static void  y_setframe(class rasterview  *self, int   position, long   coordinate , long   outof);
boolean rasterview_FindFrameHelp(class frame  *frame, class im  *im);
void rasterview_CurrentDirectory(class rasterview  *self, char  *f);
void rasterview_CenterViewSelection(class rasterview  *self);
void rasterview_ViewHideHighlight(class rasterview  *self);
void rasterview_CorrectHighlight(class rasterview  *self);
void rasterview_DrawPanHighlight(class rasterview  *self, short  g);
static void DrawTarget(class rasterview  *self, long  x , long  y);
static void HideTarget(class rasterview  *self, long  x , long  y);
static void StartPanning(class rasterview  *self, long  x , long  y);
static void ContinuePanning(class rasterview  *self, long  x , long  y);
static void ClipScroll(class rasterview  *self);
static void FinishPanning(class rasterview  *self, long  x , long  y);
static void SetPixel(class rasterview  *self, long  x , long  y, boolean  bit);
static void DrawLineTo(class rasterview   *self, long  x1 , long  y1, boolean  bit);
void rasterview_ZoomToVisualBounds(class rasterview  *self, long  x , long  y);
void rasterview_UpdateZoomedSelection(class rasterview  *self, long  x , long  y);


void rasterview::FitToSize(struct rectangle  logicalrect  )
{  
    class raster *ras = (class raster *)(this)->GetDataObject();
    class rasterimage *pix;
    long x, y, w, h;
    long NewX, NewY, NewW, NewH;

    if (ras == NULL || (pix = (ras)->GetPix()) == NULL) return;

    /* Until Scaling works while Zoomed... */
    if (NotFullSize(this)) return;

    rectangle_GetRectSize(&this->DesiredSelection, &x, &y, &w, &h);
    rectangle_GetRectSize(&logicalrect, &NewX, &NewY, &NewW, &NewH);

 
    DEBUG(("Original is%s NULL\n", ((this->Original == NULL) ? "" : " not")));
    DEBUG(("New Absolute: (%ld,%ld)\n", NewW, NewH));

    if (this->Original == NULL) {
	this->Original = (class rasterimage *)(pix)->Clone();
	(pix)->SetBitsPtr( NULL);
	(this->Original)->GetScaledSubraster(
				       &this->DesiredSelection, NewW, NewH, pix);
    }
    else {
	struct rectangle sub, original;
	/* map Desired Selection within scaled version to selection within the Original */
	rectangle_SetRectSize(&original, 0, 0,
			      (this->Original)->GetWidth(),
			      (this->Original)->GetHeight());
	if (FullySelected(this))
	    sub = original;
	else {
	    float wscale =
	      (this->Original)->GetWidth()/(pix)->GetWidth();
	    float hscale =
	      (this->Original)->GetHeight()/(pix)->GetHeight();
	    rectangle_SetRectSize(&sub,
				  (long)(rectangle_Left(&this->DesiredSelection)*wscale),
				  (long)(rectangle_Top(&this->DesiredSelection)*hscale),
				  (long)(rectangle_Width(&this->DesiredSelection)*wscale),
				  (long)(rectangle_Height(&this->DesiredSelection) * hscale));
	    rectangle_IntersectRect(&sub, &sub, &original); }
	DEBUG(("Original: (%ld,%ld,%ld,%ld)\n",
	       rectangle_Left(&original), rectangle_Top(&original),
	       rectangle_Width(&original), rectangle_Height(&original)));
	DEBUG(("Selection: (%ld,%ld,%ld,%ld)\n",
	       rectangle_Left(&sub), rectangle_Top(&sub),
	       rectangle_Width(&sub), rectangle_Height(&sub)));
	DEBUG(("%s: 0x%p\n", (this->Original)->GetTypeName(), this->Original));

	(this->Original)->GetScaledSubraster( &sub, NewW, NewH, pix);
    }

    rectangle_SetRectSize(&this->DesiredSelection, 0, 0, NewW, NewH);
    rectangle_SetRectSize(&this->ViewSelection, 0, 0, NewW, NewH);

    if (NotFullSize(this))
	rasterview_ZoomToVisualBounds(this, 0, 0);

    rasterview_CenterViewSelection(this);

    (pix)->NotifyObservers( rasterview_SCALECHANGED);
}  
  

void rasterview::AutoCenter()
{   
    rasterview_CenterCommand(this, 0);
}

void rasterview::ZoomRaster(boolean  zoomIn  )
{ 
    if (zoomIn)
	rasterview_ZoomInCommand(this, 0);
    else
	rasterview_ZoomOutCommand(this, 0);
}

void rasterview::SetToolMode()
{   
    ToolCommand(this, 0);
}
	
void rasterview::SetPan()
{   
    PanCommand(this, 0);
}
	
void rasterview::SetRegionSelect()
{   
    rasterview_RegionSelectCommand(this, 0);
}
	
void rasterview::SetTouchUp()
{   
    TouchUpCommand(this, 0);
}
	
void rasterview::RotateRaster()
{   
    rasterview_RotateCommand(this, 0);
}

void rasterview::SetDataObject(class dataobject  *dras)
{
    class raster *oldras = (class raster *)(this)->GetDataObject();
    class raster *ras=(class raster *)dras;
    DEBUG(("rasterview__SetDataObject(0x%p, 0x%p) was 0x%p\n", this, ras, oldras));
    if (oldras == ras) return;	/* this is needed to avoid
				 Destroy'ing oldras in RemoveObserver */
    (this)->view::SetDataObject( ras);
    if (ras != NULL) {
	this->ViewSelection = this->DesiredSelection = ras->subraster;
	DEBUG(("VS: (%ld,%ld,%ld,%ld)\n",
	       rectangle_Left(&this->ViewSelection),
	       rectangle_Top(&this->ViewSelection),
	       rectangle_Width(&this->ViewSelection),
	       rectangle_Height(&this->ViewSelection))); }
    else 
	rectangle_EmptyRect(&this->DesiredSelection);
    (this)->WantNewSize( this);
}

void rasterview::ObservedChanged(class observable  *obs, long  status)
{
    class raster *ras = (class raster *)(this)->GetDataObject();
    class rasterimage *pix;

    DEBUG(("Enter rasterview__ObservedChanged(0x%p, 0x%p, %ld)   ras: 0x%p\n", this, obs, status, ras));

    if (obs == (class observable *)this->toolset) {
	if (status==observable::OBJECTDESTROYED) {
	    fprintf(stderr, "rasterview: observed toolset destroyed!\n");
	}
	else {
	    int toolnum = (this->toolset)->GetToolnum();
	    switch (toolnum) {
		case RASTOOL_PAN:
		    if (!Pan(this))
			(this)->SetPan();
		    break;
		case RASTOOL_SELECT:
		    if (!RegionSelect(this))
			(this)->SetRegionSelect();
		    break;
		case RASTOOL_TOUCHUP:
		    if (!TouchUp(this))
			(this)->SetTouchUp();
		    break;
		default:
		    /* tool mode; always call SetToolMode, because we might be changing from one tool to another */
		    (this)->SetToolMode();
		    break;
	    }
	}
    }
    else {
	class raster *dobj = (class raster *)obs;
	/* dobj should be the same as ras here; if they're not, things will seriously screw up */

	if (ras == NULL) return;
	pix = (ras)->GetPix();

	if (status == observable::OBJECTDESTROYED) return;

	switch (status) {
	    case 0:
		this->UpdateWanted = FALSE;
		this->needsFullUpdate = TRUE;
		(this)->WantUpdate(this);
		break;
	    case raster_BITSCHANGED:
	    case pixelimage_DATACHANGED:
		DEBUG(("   BitsChanged\n"));
		if (this->Original != NULL) {
		    (this->Original)->Destroy();
		    this->Original = NULL; }

		if ((pix)->GetResized()) {
		    this->PixChanged = this->ViewSelection;
		}
		else {
		    struct rectangle *C = (pix)->GetChanged();
		    if (NotFullSize(this)) {
			/* we now hack around somebody's stupidity. We have to get self->PixChanged in normal (not zoomed) coordinates; there's no such thing. Thus, we invent it by untransforming PixChanged, union it with C, and retransform it. */
			struct rectangle OPC;
			if (rectangle_IsEmptyRect(&this->PixChanged))
			    rectangle_EmptyRect(&OPC);
			else {
			    rectangle_SetRectSize(&OPC, rectangle_Left(&this->PixChanged) / this->Scale + rectangle_Left (&this->DisplayBoxSelection), rectangle_Top(&this->PixChanged) / this->Scale + rectangle_Top (&this->DisplayBoxSelection), rectangle_Width(&this->PixChanged) / this->Scale, rectangle_Height(&this->PixChanged) / this->Scale);
			}
			rectangle_UnionRect(&OPC, &OPC, C);
			rectangle_SetRectSize(&this->PixChanged, (rectangle_Left(&OPC) - rectangle_Left(&this->DisplayBoxSelection)) *this->Scale, (rectangle_Top(&OPC) - rectangle_Top(&this->DisplayBoxSelection)) *this->Scale, rectangle_Width(&OPC)*this->Scale, rectangle_Height(&OPC)*this->Scale);
			rasterview_ReflectChangesInExpansion(this, &OPC); 
		    }
		    else {
			rectangle_UnionRect(&this->PixChanged, &this->PixChanged, C); 
		    }
		}
		DEBUG(("PixChanged: (%ld,%ld,%ld,%ld)\n", 
		       this->PixChanged.left, this->PixChanged.top, 
		       this->PixChanged.width, this->PixChanged.height));
		if(rectangle_IsEmptyRect(&this->PixChanged))
		    this->PixChanged = this->ViewSelection;
		(this)->WantUpdate( this);
		break;
	    case raster_BOUNDSCHANGED:
		DEBUG(("   BoundsChanged\n"));
		/* The following is in the wrong place. What is needed is to be able to determine if the bounds changed in such a manner that the Original's bounds could be changed to reflect the bounds change. Since the bounds can change such that white space is added to the left/top of the image, resizing the Original is not enough. I am not sure that one CAN determine what to do in the general case. For now... */
		if (this->Original != NULL) {
		    (this->Original)->Destroy();
		    this->Original = NULL; }
		/* Break is intensionally left out here. */
	    case rasterview_SCALECHANGED:
		DEBUG(("   ScaleChanged\n"));
		this->PixChanged = this->ViewSelection;	/* copy rectangle */
		/* make sure the selection is inside the subraster, but otherwise retain the selection */
		rectangle_IntersectRect(&this->DesiredSelection,
					&this->DesiredSelection,
					&this->ViewSelection);
		this->needsFullUpdate = TRUE;
		(this)->WantNewSize( this);
		/* the new size will force FullUpdate,
		 so we don't call WantUpdate */
		/* XXX WantNewSize does nothing if is an ApplicationLayer */
		if ( ! this->embedded)
		    (this)->WantUpdate( this);
		break;
	    default:
		this->UpdateWanted = FALSE;
		this->needsFullUpdate = TRUE;
		(this)->WantUpdate(this);
		break;
	}
	if (dobj == ras)
	    (ras)->SetModified();    /* this is a bogus place for this  XXX */

    }
}

void rasterview::ReceiveInputFocus()
{
    this->Keystate->next = NULL;
    (this)->PostKeyState( this->Keystate);

    this->HasInputFocus = TRUE;
    (this->Menus)->SetMask( 0);	/* ensure that menus are posted */
    rasterview_RealPostMenus(this);
    (this)->WantUpdate( this);
}

void rasterview::LoseInputFocus()
{
    this->HasInputFocus = FALSE;
    (this)->WantUpdate( this);
    /* this is the wrong place? Moved to within the Hit
      if (TouchUp(self)) {
	  struct raster *ras = (struct raster *)rasterview_GetDataObject(self);
	  struct rasterimage *pix;
	  if (ras != NULL && (pix = raster_GetPix(ras)) != NULL)
	      rasterimage_NotifyObservers(pix, raster_BITSCHANGED); } */
}

void rasterview::WantUpdate(class view  *requestor)
{
    if (this->inset && this->inset==requestor) {
	if (!this->InsetUpdateWanted) {
	    this->InsetUpdateWanted = TRUE;
	    (this)->view::WantUpdate( this); 
	}
    }
    else {
	if (!this->UpdateWanted) {
	    this->UpdateWanted = TRUE;
	    (this)->view::WantUpdate( this); 
	}
    }
}



static void RedrawRaster(class rasterview  *self, enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    class rasterimage *pix;
    struct rectangle SRC;	/* image rectangle to redraw */
    struct rectangle DEST;	/* the screen rectangle to be redrawn */
    struct rectangle LB;	/* logical rectangle, inset by BORDER */
    struct rectangle VB;	/* visible rectangle inside border */

    DEBUG(("RedrawRaster(%d (%ld,%ld,%ld,%ld))\n", type, left, top, width, height));

    self->UpdateWanted = FALSE;
    if (ras == NULL) return;
    if (self->NeedsCentering) {
	self->NeedsCentering = FALSE;
	if (! self->embedded) {
	    rasterview_CenterViewSelection(self);
	    rasterview_RealPostMenus(self); 
	}
    }
    /* Just in case the window has been resized. */
    ClipScroll(self);
    pix = (NotFullSize(self)) ? self->Expansion : (ras)->GetPix();
    if (pix == NULL) {
	/* XXX Kludge: If there is no rasterimage, we create one */
	(ras)->SetPix( pix = rasterimage::Create(91, 91));
	rectangle_SetRectSize(&self->DesiredSelection, 0, 0, 91, 91);
	self->ViewSelection = self->DesiredSelection;
	self->Shrunken = FALSE;
	/* This is to include the execution of the code which would be executed in raster_SetPix which is special cased to NOT inform the upper layers that the raster has changed.  This is a kludge to fix the problem when raster is run with a new file one should be able to quit without the message 'Are you sure you want to Quit?' Posting Menus should be sufficient. */
	rasterview_RealPostMenus(self); 
    }
    /* compute LB, the entire allocated rectangle of pixels, but inside by BORDER pixels on all edges. */
    (self)->GetLogicalBounds( &LB);
    InsetRect(&LB, BORDER, BORDER);
    DEBUG(("LB: (%ld,%ld,%ld,%ld) Scroll: (%ld,%ld)\n",
	    rectangle_Left(&LB), rectangle_Top(&LB), rectangle_Width(&LB),
	    rectangle_Height(&LB), self->Xscroll, self->Yscroll));

    /* compute Xoff,Yoff, increments which map a point in the logical view rectangle to a point in the stored raster image. */

    self->Xoff = - rectangle_Left(&LB);
    self->Yoff = - rectangle_Top(&LB);
    if (FullSize(self)) {
	self->Xoff += rectangle_Left(&self->ViewSelection) + self->Xscroll;
	self->Yoff += rectangle_Top(&self->ViewSelection) + self->Yscroll; 
    }
    else {
	if (self->Xscroll < 0)
	    self->Xoff += self->Xscroll;
	if (self->Yscroll < 0)
	    self->Yoff += self->Yscroll;
    }

    if (type == view_FullRedraw) rectangle_EmptyRect(&self->CurSelection);
    else rasterview_ViewHideHighlight(self);	/* (requires Xoff and Yoff) */

    /* compute VB, the displayed portion of the image.  It is the intersection of the visual rectangle with LB */
    (self)->GetVisualBounds( &VB);
    rectangle_IntersectRect(&VB, &VB, &LB);

    DEBUG(("VB: (%ld,%ld,%ld,%ld) Offset: (%ld,%ld)\n",
	    rectangle_Left(&VB), rectangle_Top(&VB), rectangle_Width(&VB),
	    rectangle_Height(&VB), self->Xoff, self->Yoff));

    /* compute SRC, a rectangle showing which bits of the image need to be replotted.  First we map the parameters given above into the pixel image coordinates, offsetting by the subraster amount.  Then we take the union with the PixChanged rect. */
    if (type == view_FullRedraw)
	SRC = VB;
    else {
	/* set T to pixels changed in view coords */
	struct rectangle T;
	T = self->PixChanged;
	DEBUG(("Changed: (%ld,%ld,%ld,%ld)\n",
	       rectangle_Left(&T), rectangle_Top(&T),
	       rectangle_Width(&T), rectangle_Height(&T)));
	OffsetRect(&T, -self->Xoff, -self->Yoff);
	/* set SRC to rectangle arg to Update() */
	rectangle_SetRectSize(&SRC, left, top, width, height);
	/* merge with changes noted in the data (PixChanged) */
	rectangle_UnionRect(&SRC, &SRC, &T);
	/* restrict the SRC to viewable portion */
	rectangle_IntersectRect(&SRC, &SRC, &VB);
    }
    DEBUG(("preSRC: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&SRC), rectangle_Top(&SRC),
	    rectangle_Width(&SRC), rectangle_Height(&SRC)));
    /* map SRC into the raster coords */
    OffsetRect(&SRC, self->Xoff, self->Yoff);
    /* and finally, restrict to the portion selected for this view */
    if (FullSize(self))
	rectangle_IntersectRect(&SRC, &SRC, &self->ViewSelection);
    else {
	struct rectangle NS;
	rectangle_SetRectSize(&NS, 0, 0,
			      rectangle_Width(&self->ViewSelection)*self->Scale,
			      rectangle_Height(&self->ViewSelection)*self->Scale);
	rectangle_IntersectRect(&SRC, &SRC, &NS); 
    }

    DEBUG(("SRC: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&SRC), rectangle_Top(&SRC),
	    rectangle_Width(&SRC), rectangle_Height(&SRC)));

    /* compute DEST by mapping SRC into the logical rectangle coordinates 
      and then take the intersection with the visible rect. */
    DEST = SRC;
    OffsetRect(&DEST, -self->Xoff, -self->Yoff);
    rectangle_IntersectRect(&DEST, &DEST, &VB);

    /* SRC is the source in the image space.  DEST is the destination, in
      screen coordinates.  DEST also gives the visible width and height. */

    rectangle_GetRectSize(&DEST, &left, &top, &width, &height);

    DEBUG(("DEST: (%ld,%ld,%ld,%ld)\n", left, top, width, height));

    /* XXX need to deal with ras->options */

    (self)->SetTransferMode( graphic::WHITE);
    /* There must have been some reason to special case pan mode when embedded but I (pasieka) can not remember why.  Maybe someone will point out a bug.
	if (self->embedded && (Pan(self) || type == view_FullRedraw)
	    ) {
	    rasterview_GetVisualBounds(self, &VB);
	    InsetRect(&VB, BORDER, BORDER);
	    rasterview_FillRect(self, &VB, self->WhitePattern);
	}
	else */
    if (type == view_FullRedraw) {
	(self)->GetVisualBounds( &VB);
	if (self->embedded) InsetRect(&VB, BORDER, BORDER);
	(self)->FillRect( &VB, self->WhitePattern);
    }
    else if (width > 0  &&  height > 0) {
	/* erase the DEST because only the black bits get painted */
	(self)->FillRect( &DEST, self->WhitePattern);
    }

    /* XXX Is a TransferMode needed with WritePixImage ??? */
    (self)->SetTransferMode( graphic::COPY);
    if (width > 0  &&  height > 0) {
	class graphic *G = (self)->GetDrawable();
	(G)->WritePixImage( left, top,
			      pix, rectangle_Left(&SRC), rectangle_Top(&SRC),
			      width, height);

	/*if Zoomed draw the display box in the right place */
	if (NotFullSize(self) && ! self->DisplayBoxHidden)
	    rasterview_DisplayBoxWritePixImage(self, G);
    }

    if (RegionSelect(self) || (Tool(self) && (self->toolset)->WantSelectionHighlighted()))
	rasterview_CorrectHighlight(self);
    if (Pan(self)) {
	rasterview_DrawPanHighlight(self, graphic::BLACK); }

    if (self->embedded) {
	(self)->GetVisualBounds( &VB);
	InsetRect(&VB, BORDER, BORDER);
	DrawHighlightScreenCoordinates(self, (self)->GetDrawable(), VB,
				       graphic::BLACK, graphic::WHITE);
    }

    rectangle_EmptyRect(&self->PixChanged);
    self->needsFullUpdate = FALSE;

}

void rasterview::FullUpdate(enum view_UpdateType   type, long   left , long   top , long   width , long   height)
{
    class raster *ras = (class raster *)(this)->GetDataObject();
    struct rectangle VB;

    (this)->GetVisualBounds( &VB);
    if((type == view_Remove) || (type == view_MoveNoRedraw)) {
	if(type == view_Remove) {
	    this->OnScreen = FALSE;
	    (this)->RetractCursor( this->Cursor[this->Mode]);
	}
	else if(type == view_MoveNoRedraw) {
	    (this)->RetractCursor( this->Cursor[this->Mode]);
	    (this)->PostCursor( &VB, this->Cursor[this->Mode]);
	}
	return;
    }
    if(IsEmptyRect(&VB)) {
	return;
    }

    /* fixes bug that the view selection is not updated when raster_Read is called. */
    if (rectangle_IsEmptyRect(&this->ViewSelection))
	this->ViewSelection = this->DesiredSelection = ras->subraster;

#if 0
/* should only post menus in ReceiveInputFocus */
    (this->Menus)->SetMask( 0);  /* force post */
    rasterview_RealPostMenus(this);
#endif
    this->OnScreen = TRUE;
    /* 
      XXX this code sets ReadOnly in the 
      rasterimage if the parent is 'messages'
	  */
    if (! this->CheckedParent) {
	class rasterimage *pix = (ras)->GetPix();
	class view *v;
	this->CheckedParent = TRUE;
	for (v = (class view *)this; v != NULL; v = v->parent) {
	    const char *nm = (v)->GetTypeName();
	    DEBUG(("parent: %s\n", nm));
	    if (strcmp(nm, "messages") == 0) {
		this->InMessages = TRUE;
		if (pix != NULL)
		    (pix)->SetRO( ras->readOnly = TRUE);
		break; } }
	DEBUG(("Done Checking parents.\n")); }
    if (type == view_FullRedraw || type == view_LastPartialRedraw) {
	/* must recompute graphics info because image
	 may be on different display hardware */
	this->WhitePattern = (this)->view::WhitePattern();
	this->BlackPattern = (this)->view::BlackPattern();
	this->GreyPattern = (this)->view::GrayPattern( 1, 2);

	if (this->Cursor[0] == NULL) {
	    class fontdesc *fd;
	    long junk;
	    DEBUG(("Creating Cursors\n"));
	    this->Cursor[RegionSelectMode] = cursor::Create(this);
	    this->Cursor[TouchUpMode] = cursor::Create(this);
	    this->Cursor[PanMode] = cursor::Create(this);
	    this->Cursor[WaitMode] = cursor::Create(this);
	    this->Cursor[ToolMode] = cursor::Create(this);
	    fd = fontdesc::Create("icon", 0, 12);
	    (this->Cursor[RegionSelectMode])->SetStandard( Cursor_Gunsight);
	    if ((fd)->StringSize( (this)->GetDrawable(), "!", &junk, &junk)
		> 5)
		(this->Cursor[TouchUpMode])->SetGlyph( fd, '!');
	    else
		(this->Cursor[TouchUpMode])->SetStandard( Cursor_Arrow);
	    (this->Cursor[PanMode])->SetStandard( Cursor_CrossHairs); 
	    (this->Cursor[ToolMode])->SetStandard( Cursor_CrossHairs); 
	    (this->Cursor[WaitMode])->SetStandard( Cursor_Wait);
	}
	/* reset the cursor */
	DEBUG(("Finished Posting Cursors\n")); fflush(stdout);
	(this)->PostCursor( &VB, this->Cursor[this->Mode]); }

    if (NotFullSize(this)) {
	DEBUG(("Call rasterview_ZoomToVisualBounds\n"));
	rasterview_ZoomToVisualBounds(this, this->Xscroll/this->Scale, this->Yscroll/this->Scale); }

    DEBUG(("Call RedrawRaster\n"));
    RedrawRaster(this, type, left, top, width, height);

    if (this->inset && FullSize(this)) {
	(this)->EraseRectSize( this->InsetBox.left-this->Xoff, this->InsetBox.top-this->Yoff, this->InsetBox.width, this->InsetBox.height); 
	(this->inset)->FullUpdate( view_FullRedraw, 0, 0, -1, -1);
    }
    }

void rasterview::Update()
{
    
    if (! this->OnScreen) return;

    rasterview_RealPostMenus(this);

    if (this->UpdateWanted) {
	if (this->needsFullUpdate)
	    RedrawRaster(this, view_FullRedraw, 0, 0, -1, -1);
	else
	    RedrawRaster(this, view_LastPartialRedraw, 0, 0, -1, -1);
    }

    if (this->inset && FullSize(this)) {
	(this->inset)->InsertViewSize( this, this->InsetBox.left-this->Xoff, this->InsetBox.top-this->Yoff, this->InsetBox.width, this->InsetBox.height);
	(this->inset)->FullUpdate( view_FullRedraw, 0, 0, -1, -1);
	this->InsetUpdateWanted = FALSE;
    }
    }

view_DSattributes rasterview::DesiredSize(long  width, long  height, enum view_DSpass  pass,
						long  *desiredWidth, long  *desiredHeight) 
{
    class raster *ras = (class raster *)(this)->GetDataObject();
    class rasterimage *pix;
    
    pix = (ras)->GetPix();
    if (pix == NULL) {
	/* XXX Kludge: If there is no rasterimage, we create one */
	(ras)->SetPix( pix = rasterimage::Create(91, 91));
	rectangle_SetRectSize(&this->DesiredSelection, 0, 0, 91, 91);
	this->Shrunken = FALSE; }
    if (ras != NULL && IsNotEmptyRect(&ras->subraster)) {
	/* leave BORDER pixels all around the image */
	*desiredWidth = rectangle_Width(&ras->subraster) + (2*BORDER);
	*desiredHeight = rectangle_Height(&ras->subraster) + (2*BORDER); }
    else {
	*desiredWidth = 95;
	*desiredHeight = 95; }
    if (this->Shrunken && *desiredHeight > 20) 
	*desiredHeight = 20;

    DEBUG(("Leave DesiredSize: %ld x %ld\n", *desiredWidth, *desiredHeight));

    return view_Fixed;
}

static const char * const MouseEvent[] = {
    "No Mouse Event",
    "Left Down",
    "Left Up",
    "Left Movement",
    "Right Down",
    "Right Up",
    "Right Movement" };
enum { DragCorner, DragTop, DragBottom, DragLeft, DragRight };

class view * rasterview::Hit(enum view_MouseAction   action, long   x , long   y , long   num_clicks)
{
    class raster *ras = (class raster *)(this)->GetDataObject();
    struct point truepoint;

    DEBUG(("rasterview_Hit %s at (%ld, %ld)\n", MouseEvent[(long)action], x, y));
    if (ras == NULL || action == view_NoMouseEvent)
	return (class view *)this;
    if (! this->OnScreen)
	return NULL;

    point_SetPt(&truepoint, x+this->Xoff, y+this->Yoff);

    /* Ignore all hits until next up transition. */
    if (this->IgnoreUntilNextUpTransition) {
	if (action == view_LeftUp || action == view_RightUp) {
	    this->IgnoreUntilNextUpTransition = FALSE;
	    return NULL; }
	else
	    DEBUG(("Ignoring Mouse Movements until up transition.\n")); }

    else if (this->Shrunken) {
	this->Shrunken = FALSE;
	this->needsFullUpdate = TRUE;
	(this)->WantNewSize( this);
	/* the new size will force FullUpdate, so we don't call WantUpdate */
	/* XXX WantNewSize does nothing if is an ApplicationLayer */
	if (! this->embedded)
	    (this)->WantUpdate( this);
	/* This covers the case when the raster is shrunken and does not have the input focus. */
	if (! this->HasInputFocus)
	    (this)->WantInputFocus( this);
	this->IgnoreUntilNextUpTransition = TRUE; }

    /* this tests for clicking in the overlaid inset */
    else if (this->inset 
	      && (action==view_LeftDown || action==view_RightDown) 
	      && (FullSize(this))
	      && rectangle_IsPtInRect(&truepoint, &this->InsetBox)) {
	long sx = (this->inset)->EnclosedXToLocalX( x);
	long sy = (this->inset)->EnclosedYToLocalY( y);
	this->DesiredSelection = this->InsetBox;
	return (this->inset)->Hit( action, sx, sy, num_clicks); }

    /* This covers the case when the raster is not shrunken and one just wants the input focus. */
    else if (! this->HasInputFocus) {
	(this)->WantInputFocus( this);
	this->IgnoreUntilNextUpTransition = TRUE; }

    else if (this->MovingDisplayBox) {
	if (action == view_LeftUp) {
	    rasterview_FinishMovingDisplayBox(this, x, y);
	    return NULL; }
	else
	    DEBUG(("Still Moving Display Box\n")); }

    /* This tests for clicking left within the Display Box. */
    else if (! this->DisplayBoxHidden
	      && NotFullSize(this) && action == view_LeftDown
	      && x > rectangle_Left(&this->DisplayBox) - 2*BORDER
	      && x < rectangle_Right(&this->DisplayBox) + 2*BORDER
	      && y > rectangle_Top(&this->DisplayBox) - 2*BORDER
	      && y < rectangle_Bottom(&this->DisplayBox) + 2*BORDER) {
	/* Begin moving the Display Box. */
	rasterview_MoveDisplayBoxCommand(this, -1); }

    /* If in Pan Mode then do functions of panning. */
    else if (Pan(this)) {
	if (action == view_LeftDown || action == view_RightDown)
	    StartPanning(this, x, y);
	else if (action == view_LeftUp || action == view_RightUp) {
	    FinishPanning(this, x, y);
	    return NULL; }
	else
	    ContinuePanning(this, x, y); }

    /* At this point the hit is either for TouchUp or Region Select, or for one of the toolsetv tools. */
    else {
	struct rectangle SR;

	/* map coordinates to full size raster space. */
	x = truepoint.x;
	y = truepoint.y;
	if (NotFullSize(this)) {
	    x = rectangle_Left(&this->DisplayBoxSelection) + x/this->Scale;
	    y = rectangle_Top(&this->DisplayBoxSelection) + y/this->Scale;
	    SR = this->DisplayBoxSelection; }
	else
	    SR = this->ViewSelection;

	/* note that toolset hits are *not* constrained to be within the raster */
	if (this->toolset) {
	    int toolnum = (this->toolset)->GetToolnum();
	    if (toolnum!=RASTOOL_PAN && toolnum!=RASTOOL_SELECT && toolnum!=RASTOOL_TOUCHUP) {
		rastoolview_toolfptr tproc;
		if (this->ShowCoords) {
		    static char cb[20];
		    sprintf(cb, "(%ld,%ld)", x, y);
		    message::DisplayString(this, 0, cb);
		}
		tproc = (this->toolset)->GetToolProc();
		if (tproc)
		    (*tproc) (this->toolset, action, x, y, num_clicks);
		return (class view *)this;
	    }
	}

	/* Note that coordinates are constrained to refer to actual pixels
	 (left+width and top+height are the addresses of pixels
	  just outside the raster) */
	if (x < rectangle_Left(&SR)) x = rectangle_Left(&SR);
	else if (x > rectangle_Left(&SR)+rectangle_Width(&SR)-1)
	    x = rectangle_Left(&SR)+rectangle_Width(&SR)-1;
	if (y < rectangle_Top(&SR)) y = rectangle_Top(&SR);
	else if (y > rectangle_Top(&SR)+rectangle_Height(&SR)-1)
	    y = rectangle_Top(&SR)+rectangle_Height(&SR)-1;

	if (TouchUp(this)) {
	    if (this->ShowCoords) {
		static char cb[20];
		sprintf(cb, "(%ld,%ld)", x, y);
		message::DisplayString(this, 0, cb);
	    }

	    if (action == view_LeftDown)
		::SetPixel(this, x, y, TRUE);
	    else if (action == view_LeftMovement)
		::DrawLineTo(this, x, y, TRUE);
	    else if (action == view_RightDown)
		::SetPixel(this, x, y, FALSE);
	    else if (action == view_RightMovement)
		::DrawLineTo(this, x, y, FALSE);
	    else {
		class rasterimage *pix;
		if ((pix = (ras)->GetPix()) != NULL) {
		    DEBUG(("Notify Observers of Touchup\n"));
		    (pix)->NotifyObservers( raster_BITSCHANGED); } }
	    this->TouchUpX = x;
	    this->TouchUpY = y; }
	else {
	    long l, t, w, h;	/* top, left, width, height */
	    long farx, fary;

	    /* while mouse is down, have raster cursor throughout the window */
	    if (action == view_RightDown || action == view_LeftDown) {
		((this)->GetIM())->SetWindowCursor( this->Cursor[this->Mode]);
	    }
	    else if (action == view_RightUp || action == view_LeftUp)
		((this)->GetIM())->SetWindowCursor( NULL);

	    if (action == view_LeftUp) {
		if (num_clicks == 2) {
		    /* Double Left Click selects entire raster. */
		    rectangle_GetRectSize(&SR, &l, &t, &w, &h);
		    x = l;
		    y = t;
		    this->Xdown = x + w - 1;
		    this->Ydown = y + h - 1; } }
	    else if (action == view_LeftDown) {
		/* Single Left Click sets the desired selection to the single point under the mouse. */
		this->Xdown = x;
		this->Ydown = y; }
	    else if (action == view_RightDown) {
		/* Drag either a corner or an edge while right down. */
		struct rectangle *R = &this->DesiredSelection;
		long l, t, w, h;
		long r, b;
		rectangle_GetRectSize(R, &l, &t, &w, &h);
		r = l+w;
		b = t+h;
		if ((x < l || x > r) && y > t && y < b) {
		    if (y <= t+h/2)
			this->DraggingEdge = (int)DragTop;
		    else
			this->DraggingEdge = (int)DragBottom; }
		else if ((y < t || y > b) && x > l && x < r) {
		    if (x <= l+w/2)
			this->DraggingEdge = (int)DragLeft;
		    else 
			this->DraggingEdge = (int)DragRight; }
		else 
		    this->DraggingEdge = (int)DragCorner;
		/* set (Xdown, Ydown) to the furthest corner of old selection */
		farx = (x < l  ||  (x < r  &&  x-l < r-x)) ? r-1 : l;
		fary = (y < t  ||  (y < b  &&  y-t < b-y)) ? b-1 : t;
		this->Xdown = farx;
		this->Ydown = fary; }

	    switch (this->DraggingEdge) {
		case DragCorner:
		    break;
		case DragTop:
		    y = rectangle_Top(&this->DesiredSelection);
		    break;
		case DragBottom:
		    y = rectangle_Bottom(&this->DesiredSelection) - 1;
		    break;
		case DragLeft:
		    x = rectangle_Left(&this->DesiredSelection);
		    break;
		case DragRight:
		    x = rectangle_Right(&this->DesiredSelection) - 1; }

	    /* set DesiredSelection to have corners (x,y) and (self->Xdown,self->Ydown) */
	    farx = this->Xdown;
	    fary = this->Ydown;
	    l = (x<farx) ? x : farx;
	    t = (y<fary) ? y : fary;
	    w = x - farx;   if (w<0) w = -w;   w++;
	    h = y - fary;   if (h<0) h = -h;   h++;
	    rectangle_SetRectSize(&this->DesiredSelection, l, t, w, h);

	    if (this->ShowCoords) {
		static char cb[32];
		sprintf(cb, "(%ld by %ld) at (%ld,%ld)", w, h, l, t);
		message::DisplayString(this, 10, cb);
	    }

	    /* if rightup or leftup then update the scroll bars. */
	    if (action == view_LeftUp  ||  action == view_RightUp) {
		this->DraggingEdge = 0;
		this->needsFullUpdate=FALSE;
		(this)->WantUpdate( this); }
	    rasterview_CorrectHighlight(this);
	} }

    if (action == view_LeftUp  ||  action == view_RightUp)
	rasterview_RealPostMenus(this);

    /*  XXX when we have drawing operations,
      the first mouse hit without a self->dataobject->pix
      must create the pix */

        return (class view *)this;		/* where to send subsequent hits */
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *
 *	Printing
 *
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifdef _IBMR2
#define NO_FLOATS_IN_PRINTF_ENV
#endif

static const char * const PSheader[] = {
#ifndef NO_FLOATS_IN_PRINTF_ENV
	"%s  /width %d def  /height %d def /xScale %0.4f def /yScale %0.4f def\n",
#endif
	"%s     width xScale mul height yScale mul scale\n",
	"%s     /picstr width 7 add 8 idiv string def\n",
	"%s        width height 1\n",
	"%s        [width    0    0    height neg    0    height]\n",
	"%s        {currentfile picstr readhexstring pop}\n",
	"%s     image\n",
	NULL };

void rasterview::Print(FILE   	 *file, const char	 *processor, const char	 *format, boolean	 toplevel)
{
    class raster *ras = (class raster *)(this)->GetDataObject();
    class rasterimage *pix;
    const char * const *psx;
    long row;
    short buf[300];
    long left, top, width, height;
    long wpts, hpts;  /* image dimensions in points */
    float xdscale, ydscale;
    const char *prefix;

    if (ras == NULL || (pix=(ras)->GetPix()) == NULL) return;

    rectangle_GetRectSize(&this->ViewSelection, &left, &top, &width, &height);

    /* compute size in points and scaling as float values 
      add half unit to numerator to achieve rounding */
    wpts = (width * ras->xScale + (raster_UNITSCALE/2)) / raster_UNITSCALE;
    hpts = (height * ras->yScale + (raster_UNITSCALE/2)) / raster_UNITSCALE;

    if (ras->xScale == raster_UNITSCALE/2 
	 &&  ras->yScale == raster_UNITSCALE/2) {
	/* restrict to 6" by 9"  (1 inch = 72 points) */
	if (wpts > 6 * 72) { 
	    ras->xScale = ras->yScale =
	      6 * 72 * raster_UNITSCALE / width;
	    hpts = (height * ras->yScale + (raster_UNITSCALE/2)) 
	      / raster_UNITSCALE; }
	if (hpts > 9 * 72) 
	    ras->xScale = ras->yScale = 9 * 72 * raster_UNITSCALE / height;
	wpts = (width * ras->xScale + (raster_UNITSCALE/2)) / raster_UNITSCALE;
	hpts = (height * ras->yScale + (raster_UNITSCALE/2)) / raster_UNITSCALE; }
    xdscale = ((float) ras->xScale) / raster_UNITSCALE;
    ydscale = ((float) ras->yScale) / raster_UNITSCALE;

    if (strcmp(processor, "troff") == 0) {
	/* output to troff */
	if (toplevel)
	    /* take care of initial troff stream */
	    texttroff::BeginDoc(file);
	/*  Put macro to interface to postscript */
	texttroff::BeginPS(file, wpts, hpts);
	prefix = "\\!  ";
    }
    else if (strcmp(format, "troff") == 0)
	prefix = "\\!  ";
    else prefix = "";

#ifdef NO_FLOATS_IN_PRINTF_ENV
    fprintf(file, prefix);
    fprintf(file, "  /width %d def  /height %d def /xScale ", width, height);
    fprintf(file, "%d.%.4d def /yScale %d.%.4d def\n", (long)xdscale, (long)((xdscale-(long)xdscale+.00005)*10000), (long)ydscale, (long)((ydscale-(long)ydscale+.00005)*10000));
#endif
    
    for (psx = PSheader; *psx; psx++)
#ifndef NO_FLOATS_IN_PRINTF_ENV
	fprintf(file, *psx, prefix, width, height, xdscale, ydscale);
#else
	fprintf(file, *psx, prefix);
#endif

    /* generate bits of image
      XXX punt rotation and reflection  */
    for (row = top;  row < top+height; row++) {
	(pix)->GetRow( left, row, width, (unsigned short *)buf); 
	fprintf(file, "%s  ", prefix);
	heximage::WriteRow(file, (unsigned char *)buf, width, ! (ras->options & raster_INVERT)); }

    if (strcmp(processor, "troff") == 0) {
	texttroff::EndPS(file, wpts, hpts);
	if (toplevel)
	    texttroff::EndDoc(file); }

}

#define MARGIN (36)  /* leave a half-inch margin, ideally */

void rasterview::PrintPSDoc(FILE *outfile, long pagew, long pageh)
{
    class raster *ras = (class raster *)(this)->GetDataObject();
    class rasterimage *pix;
    long left, top, width, height;
    long wpts, hpts;  /* image dimensions in points */
    long marginw, marginh;
    long areaw, areah; /* size of printing area (page size minus margin) */
    long posx, posy;
    struct rectangle visrect;

    if (ras == NULL || (pix=(ras)->GetPix()) == NULL) {
	/* no pages */
	return;
    }

    rectangle_GetRectSize(&this->ViewSelection, &left, &top, &width, &height);

    /* compute size in points and scaling as double values. 
      add half unit to numerator to achieve rounding */
    wpts = (width * ras->xScale + (raster_UNITSCALE/2)) / raster_UNITSCALE;
    hpts = (height * ras->yScale + (raster_UNITSCALE/2)) / raster_UNITSCALE;

    if (pagew >= 4*MARGIN)
	marginw = MARGIN;
    else
	marginw = 0;
    if (pageh >= 4*MARGIN)
	marginh = MARGIN;
    else
	marginh = 0;
    areaw = pagew - (2*marginw);
    areah = pageh - (2*marginh);

    fprintf(outfile, "%% This document was generated by an AUIS raster object.\n");

    for (posy=0; posy<hpts; posy+=areah)
	for (posx=0; posx<wpts; posx+=areaw) {
	    print::PSNewPage(print_UsualPageNumbering);
	    /* posx,posy are raster coords of upper left corner of this image section */
	    fprintf(outfile, "%ld %ld translate\n", marginw-posx, marginh+posy-(hpts-areah));
	    visrect.left = 0 + posx;
	    visrect.top = hpts - posy;
	    visrect.width = areaw;
	    visrect.height = areah;
	    fprintf(outfile, "%ld %ld moveto\n", visrect.left, visrect.top-visrect.height);
	    fprintf(outfile, "%ld 0 rlineto  0 %ld rlineto  %ld 0 rlineto  0 %ld rlineto\n", visrect.width, visrect.height, -visrect.width, -visrect.height);
	    fprintf(outfile, "closepath clip newpath\n");
	    this->PrintPSRect(outfile, wpts, hpts, &visrect);
	}
}

void rasterview::PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect)
{
    class raster *ras = (class raster *)(this)->GetDataObject();
    class rasterimage *pix;
    long left, top, width, height;
    long vleft, vtop, vwidth, vheight;
    long partoffx, partoffy;
    double xdscale, ydscale;
    long row, rowbytes;
    void *buf;

    if (ras == NULL || (pix=(ras)->GetPix()) == NULL) {
	/* no image */
	return;
    }

    rectangle_GetRectSize(&this->ViewSelection, &left, &top, &width, &height);

    xdscale = ((double) ras->xScale) / ((double)raster_UNITSCALE);
    ydscale = ((double) ras->yScale) / ((double)raster_UNITSCALE);

#if 0
    /* first, we do this the horribly inefficient way: dump the whole raster, even if only part of it is visible */

    vleft = left;
    vtop = top;
    vwidth = width;
    vheight = height;
    partoffx = 0;
    partoffy = 0;
#else
    /* now, we try to do it cleverly. */
    {
	long vright, vbottom;
	/* set vcoords to the raster area that will be visible. */
	vleft = (long)((double)(visrect->left) / xdscale);
	vtop = (long)((double)(logheight-visrect->top) / ydscale);
	vright = (long)((double)(visrect->left+visrect->width) / xdscale);
	vbottom = (long)((double)(logheight-(visrect->top-visrect->height)) / ydscale);
	if (vleft<left) vleft = left;
	if (vtop<top) vtop = top;
	if (vright>left+width) vright = (left+width);
	if (vbottom>top+height) vbottom = (top+height);

	partoffx = (long)((vleft-left) * xdscale);
	partoffy = (long)(((top+height)-vbottom) * ydscale);
	vwidth = (vright - vleft);
	vheight = (vbottom - vtop);
    }
#endif

    rowbytes = ((vwidth+7) / 8);
    buf = (void *)malloc(rowbytes + (rowbytes%2)); /* increase it to an even number, just in case */

    if (!buf)
	return;

    print::PSRegisterDef("rasterStr", "0");

    fprintf(outfile, "0 setgray\n");
    fprintf(outfile, "/rasterStr %ld string store\n", rowbytes);
    if (partoffx || partoffy)
	fprintf(outfile, "%ld %ld translate\n", partoffx, partoffy);
    fprintf(outfile, "%g %g scale\n", ((double)vwidth)*xdscale, ((double)vheight)*ydscale); /* transform the desired rectangle of user space to a unit square */
    fprintf(outfile, "%ld %ld 1\n", vwidth, vheight);
    fprintf(outfile, "[%ld 0 0 %ld %d %ld]\n", vwidth, -vheight, 0, vheight); /* maps the unit square of user space to the raster rectangle */
    fprintf(outfile, "{currentfile rasterStr readhexstring pop} image\n");

    for (row = vtop;  row < vtop+vheight; row++) {
	(pix)->GetRow( vleft, row, vwidth, (unsigned short *)buf); 
	heximage::WriteRow(outfile, (unsigned char *)buf, vwidth, ! (ras->options & raster_INVERT));
    }

    free(buf);
}

void rasterview::DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight)
{
    class raster *ras = (class raster *)(this)->GetDataObject();
    class rasterimage *pix;
    long left, top;
    long wpts, hpts;  /* image dimensions in points */

    if (ras == NULL || (pix=(ras)->GetPix()) == NULL) {
	/* no image */
	*desiredwidth = 0;
	*desiredheight = 0;
	return;
    }
    
    rectangle_GetRectSize(&this->ViewSelection, &left, &top, &width, &height);

    /* yes, we're repeating the calculations from above. */
    wpts = (width * ras->xScale + (raster_UNITSCALE/2)) / raster_UNITSCALE;
    hpts = (height * ras->yScale + (raster_UNITSCALE/2)) / raster_UNITSCALE;

    *desiredwidth = wpts;
    *desiredheight = hpts;
}

void *rasterview::GetPSPrintInterface(const char *printtype)
{
    if (!strcmp(printtype, "generic"))
	return (void *)this;

    return NULL;
}

	boolean 
rasterview::Gifify(const char *filename, long *pmaxw, long *pmaxh, 
			struct rectangle *visrect) {
	cmuwm in;	// temp image of type ATK raster
	imageio out;		// temp image of type any
	FILE *rastemp = tmpfile();	// temp file for raster stream
	raster *dobj = (raster *)GetDataObject();

	/* XXX need to deal with *pmax and *pmaxh; do scaling*/

	dobj->WriteSubRaster(rastemp, 1, visrect);    // write view's data
	rewind(rastemp);
	in.Load(NULL, rastemp);		// read data into ATK-raster image
	fclose(rastemp);
	in.Duplicate(&out);			// copy to image
	out.SetSaveFormatString("gif");  // <sigh> it *is* Gifify, after all
	long ret = out.WriteNative(NULL, filename);	// write image
	*pmaxw = out.Width();
	*pmaxh = out.Height();
	return (ret == 0);	// success if 0
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *
 *	Scrolling
 *	
 *	The coordinate space defined for the scrollbars by getinfo uses the 
 *	self->dataobject->subraster.  Its upper left is 0,0.  The ends of the scroll bar
 *	correspond to the width and height of the subraster.
 *	
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   
 
static const struct scrollfns	vertical_scroll_interface =
		{(scroll_getinfofptr)y_getinfo, (scroll_setframefptr)y_setframe, NULL, (scroll_whatfptr)y_whatisat};
static const struct scrollfns	horizontal_scroll_interface =
		{(scroll_getinfofptr)x_getinfo, (scroll_setframefptr)x_setframe, NULL, (scroll_whatfptr)x_whatisat};

static void x_getinfo(class rasterview   *self, struct range   *total , struct range   *seen , struct range   *dot)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    if (ras == NULL) {
	total->beg = seen->beg = seen->end = dot->beg = dot->end = 0;
	total->end = 1;
	return; }

    dot->beg = rectangle_Left(&self->DesiredSelection);
    dot->end = rectangle_Right(&self->DesiredSelection);
    if (NotFullSize(self)) {
	total->beg = rectangle_Left(&self->ViewSelection)*self->Scale;
	total->end = rectangle_Right(&self->ViewSelection)*self->Scale;
	seen->beg = rectangle_Left(&self->DisplayBoxSelection)*self->Scale;
	seen->end = rectangle_Right(&self->DisplayBoxSelection)*self->Scale;
    	dot->beg *= self->Scale;
	dot->end *= self->Scale;
    }
    else {
	total->beg = rectangle_Left(&self->ViewSelection);
	total->end = rectangle_Right(&self->ViewSelection);
	seen->beg = rectangle_Left(&self->ViewSelection) + self->Xscroll;
	seen->end = seen->beg + (self)->GetLogicalWidth() - (2*BORDER);
    }

    if (seen->beg < total->beg) seen->beg = total->beg;
    if (seen->end > total->end) seen->end = total->end;
    if (dot->end > total->end) dot->end = total->end;
    DEBUG(("X info => total (%ld, %ld) seen (%ld, %ld) dot (%ld, %ld) Scr: %ld\n", 
	    total->beg, total->end, seen->beg, seen->end, dot->beg, dot->end,
	    self->Xscroll));
}

static long x_whatisat(class rasterview   *self, long   coordinate , long   outof)
{
    long  value;
    class raster *ras = (class raster *)(self)->GetDataObject();
    if (ras == NULL) return 0;
    if (NotFullSize(self))
	value = (coordinate - (coordinate % self->Scale)) +
	  rectangle_Left(&self->DisplayBoxSelection)*self->Scale +
	  ((self->Xscroll < 0) ? self->Xscroll : 0);
    else
	value = coordinate + rectangle_Left(&self->ViewSelection) + self->Xscroll;
    DEBUG(("X what (%ld, %ld) => %ld\n", coordinate, outof, value));
    return value;
}

static void  x_setframe(class rasterview  *self, int   position, long   coordinate , long   outof) 
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    long oldscroll = self->Xscroll;
    if (ras == NULL) return;

    DEBUG(("Enter x_setframe(%d,%ld,%ld)\n", position, coordinate, outof));

    if (coordinate != 0 && position == rectangle_Left(&self->ViewSelection))
	position = self->Xscroll + rectangle_Left(&self->ViewSelection);
    if (Cropped(self))
	position -= rectangle_Left(&self->ViewSelection);

    self->Xscroll = position - coordinate;
    ClipScroll(self);

    if (self->Xscroll != oldscroll) {
	if (NotFullSize(self)) {
	    rectangle_EmptyRect(&self->DisplayBoxSelection);
	    rasterview_UpdateZoomedSelection(self,
				  self->Xscroll/self->Scale,
				  self->Yscroll/self->Scale); }
	self->needsFullUpdate = TRUE;
	(self)->WantUpdate( self); }

    DEBUG(("X set (%d, %ld, %ld) => %ld Old: %ld\n", 
	    position, coordinate, outof, self->Xscroll, oldscroll));
    DEBUG(("Scroll: (%ld,%ld)\n", self->Xscroll, self->Yscroll));
}

static void y_getinfo(class rasterview   *self, struct range   *total , struct range   *seen , struct range   *dot)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    if (ras == NULL) {
	total->beg = seen->beg = seen->end = dot->beg = dot->end = 0;
	total->end = 1;
	return; }

    dot->beg = rectangle_Top(&self->DesiredSelection);
    dot->end = rectangle_Bottom(&self->DesiredSelection);
    if (NotFullSize(self)) {
	total->beg = rectangle_Top(&self->ViewSelection)*self->Scale;
	total->end = rectangle_Bottom(&self->ViewSelection)*self->Scale;
	seen->beg = rectangle_Top(&self->DisplayBoxSelection)*self->Scale;
	seen->end = rectangle_Bottom(&self->DisplayBoxSelection)*self->Scale;
    	dot->beg *= self->Scale;
	dot->end *= self->Scale;
    }
    else {
	total->beg = rectangle_Top(&self->ViewSelection);
	total->end = rectangle_Bottom(&self->ViewSelection);
	seen->beg = rectangle_Top(&self->ViewSelection) + self->Yscroll;
	seen->end = seen->beg + (self)->GetLogicalHeight() - (2*BORDER);
    }

    if (seen->beg < total->beg) seen->beg = total->beg;
    if (seen->end > total->end) seen->end = total->end;
    if (dot->end > total->end) dot->end = total->end;
    DEBUG(("Y info => total (%ld, %ld) seen (%ld, %ld) dot (%ld, %ld) Scr: %ld\n", 
	    total->beg, total->end, seen->beg, seen->end, dot->beg, dot->end,
	    self->Yscroll));
}

static long y_whatisat(class rasterview  *self, long   coordinate , long   outof)
{
    long  value;
    class raster *ras = (class raster *)(self)->GetDataObject();
    if (ras == NULL) return 0;
    if (NotFullSize(self))
	value = (coordinate - (coordinate % self->Scale)) +
	  rectangle_Top(&self->DisplayBoxSelection)*self->Scale +
	  ((self->Yscroll < 0) ? self->Yscroll : 0);
    else
	value = coordinate + rectangle_Top(&self->ViewSelection) + self->Yscroll;
    DEBUG(("Y what (%ld, %ld) => %ld\n", coordinate, outof, value));
    return value;
}

static void  y_setframe(class rasterview  *self, int   position, long   coordinate , long   outof)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    long oldscroll = self->Yscroll;
    if (ras == NULL) return;

    DEBUG(("Enter y_setframe(%d,%ld,%ld)\n", position, coordinate, outof));

    if (coordinate != 0 && position == rectangle_Top(&self->ViewSelection))
	position = self->Yscroll + rectangle_Top(&self->ViewSelection);
    if (Cropped(self))
	position -= rectangle_Top(&self->ViewSelection);

    self->Yscroll = position - coordinate;
    ClipScroll(self);

    if (self->Yscroll != oldscroll) {
	if (NotFullSize(self)) {
	    rectangle_EmptyRect(&self->DisplayBoxSelection);
	    rasterview_UpdateZoomedSelection(self,
				  self->Xscroll/self->Scale,
				  self->Yscroll/self->Scale); }
	self->needsFullUpdate = TRUE;
	(self)->WantUpdate( self); }

    DEBUG(("Y set (%d, %ld, %ld) => %ld Old: %ld\n",
	    position, coordinate, outof, self->Yscroll, oldscroll));
    DEBUG(("Scroll: (%ld,%ld)\n", self->Xscroll, self->Yscroll));
}

const void * rasterview::GetInterface(const char   *interface_name)
{
    const struct scrollfns *interface;
    DEBUG(("GetInterface(%s)\n", interface_name));
    if (strcmp(interface_name, "scroll,vertical") == 0)
	interface = &vertical_scroll_interface;
    else if (strcmp(interface_name, "scroll,horizontal") == 0)
	interface = &horizontal_scroll_interface;
    else
	interface = NULL;
    return interface;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *
 *	Supporting procedures
 *	
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

boolean rasterview_FindFrameHelp(class frame  *frame, class im  *im)
{
    return ((frame)->GetIM() == im);
}

void rasterview_CurrentDirectory(class rasterview  *self, char  *f)
{ 
    struct frame *frame = frame::Enumerate((frame_effptr) rasterview_FindFrameHelp, (long) (self)->GetIM()); 
    struct buffer *buffer = (frame)->GetBuffer(); 
    char *filename; 
    if (frame != NULL && buffer != NULL) { 
	filename = (buffer)->GetFilename(); 
	if (filename && strlen(filename) != 0) { 
	    char *slash; 
	    strcpy(f, filename); 
	    DEBUG(("CurrrentFilename: '%s'\n", f)); 
	    slash = (char*) strrchr(f, '/'); 
	    if (slash != NULL) 
		slash[1] = '\0'; } 
	else { 
	    im::GetDirectory(f); 
	    strcat(f, "/"); } } 
    else { 
	im::GetDirectory(f); 
	strcat(f, "/"); } 

    DEBUG(("Return value: '%s'\n", f)); 
} 

void rasterview_CenterViewSelection(class rasterview  *self)
{
    struct rectangle VB;
    (self)->GetVisualBounds( &VB);
    DEBUG(("Centering VB: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&VB), rectangle_Top(&VB),
	    rectangle_Width(&VB), rectangle_Height(&VB)));
    self->Xscroll =
      (rectangle_Width(&self->ViewSelection)*self->Scale - rectangle_Width(&VB))/2;
    self->Yscroll =
      (rectangle_Height(&self->ViewSelection)*self->Scale - rectangle_Height(&VB))/2;
}

/* selection
	The selected subrectangle is highlighted with a two pixel border,
	the inner pixel is white and the outer is black.
	
	To make room for this, the requested space for the image
	leaves room for two pixels of white around the image on all sides.
*/

/* ViewHideHighlight(self)
	repaints the selection area with the correct bits.
	paints white if painting in the two pixel border
	sets CurSelection to empty,
		but leaves DesiredSelection alone

	Strategy: paint the two pixel borders as though they did not overlap 
	the two pixel white space surrounding the image.
	Then test for overlap and, if any, repaint the entire 
	surrounding whitespace.
*/

void rasterview_ViewHideHighlight(class rasterview  *self)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    class rasterimage *pix;
    class graphic *G = (self)->GetDrawable();
    long Xoff = self->Xoff, Yoff = self->Yoff;
    long clipw, cliph;

    if (ras == NULL || (pix = (ras)->GetPix()) == NULL) return;
    if (IsEmptyRect(&self->CurSelection)) return;
    if (NotFullSize(self)) pix = self->Expansion;

    clipw = (pix)->GetWidth();
    cliph = (pix)->GetHeight();

    if (self->DisplayBoxOverlapped)
	rasterview_DisplayBoxHideOverlappingHighlight(self, G, pix);
    else {
	long l, t, w, h;			/* dimensions of selection */
	long leftThick, rightThick;		/* thickness of line to repair */
	long topThick, botThick;
	long vsl, vst, vsw, vsh;		/* dimensions of self->ViewSelection. */
	struct rectangle CS;

	CS = self->CurSelection;

	/* restore bits from image */

	if (NotFullSize(self)) {
	    struct rectangle R;
	    R = self->DisplayBoxSelection;
	    rectangle_IntersectRect(&R, &R, &CS);
	    if (IsEmptyRect(&R)) return;
	    l = (rectangle_Left(&R) - rectangle_Left(&self->DisplayBoxSelection)) * self->Scale;
	    t = (rectangle_Top(&R) - rectangle_Top(&self->DisplayBoxSelection)) * self->Scale;
	    w = rectangle_Width(&R)*self->Scale;
	    h = rectangle_Height(&R)*self->Scale;
	    vsl = vst = 0;
	    vsw = rectangle_Width(&self->DisplayBoxSelection)*self->Scale;
	    vsh = rectangle_Height(&self->DisplayBoxSelection)*self->Scale;
	    rectangle_SetRectSize(&CS, l, t, w, h);
	}
	else {
	    rectangle_GetRectSize(&CS, &l, &t, &w, &h);
	    rectangle_GetRectSize(&self->ViewSelection, &vsl, &vst, &vsw, &vsh); }

	/* we have to do white first, because writepiximage is bogus (because defined transfer modes are bogus). Draw one white line overlapping the black border */
	DrawHighlight(self, G, CS, graphic::WHITE, -1);

	DEBUG(("CurSel: (%ld,%ld,%ld,%ld)\n", l, t, w, h));
	DEBUG(("VSel: (%ld,%ld,%ld,%ld)\n", vsl, vst, vsw, vsh));

	(self)->SetTransferMode( graphic::COPY);

	/* repair left edge */
	leftThick =  (l-vsl <= 1) ? l - vsl : 2;
	if (leftThick) 
	    /* repaint left edge with pixels (do not do top and bottom corners) */
	    ClipAndWritePixImage(clipw, cliph,
				 G, l-leftThick-Xoff, t-Yoff, pix,
				 l-leftThick, t, leftThick, h);

	/* repair right edge */
	rightThick = (vsl+vsw) - (l+w);
	if (rightThick > 2) rightThick = 2;
	if (rightThick) 
	    /* redraw pixels of right edge (excluding corners) */
	    ClipAndWritePixImage(clipw, cliph,
				 G, l+w-Xoff, t-Yoff, pix, l+w, t, rightThick, h);

	/* repair top edge */
	topThick =  (t-vst <= 1) ? t-vst : 2;
	if (topThick) 
	    /* redraw pixels of top edge (including corners) */
	    ClipAndWritePixImage(clipw, cliph,
				 G, l-leftThick-Xoff, t-topThick-Yoff, pix,
				 l-leftThick, t-topThick,
				 w+leftThick+rightThick, topThick);

	/* repair bottom edge */
	botThick = (vst+vsh) - (t+h);
	if (botThick > 2) botThick = 2;
	if (botThick) 
	    /* redraw pixels of bottom edge (including corners) */
	    ClipAndWritePixImage(clipw, cliph,
				 G, l-leftThick-Xoff, t+h-Yoff, pix,
				 l-leftThick, t+h,
				 w+leftThick+rightThick, botThick);
    }
    if (NotFullSize(self))
	rasterview_DisplayBoxHideHighlight(self, G);

    rectangle_EmptyRect(&self->CurSelection);

}

/* rasterview_CorrectHighlight(self) 
	checks the selection highlighting parameters, CurSelection and DesiredSelection.
	If they differ, hides the old highlight and draws the new

	Knows about input focus and uses a grey highlight if we haven't got it
*/
void rasterview_CorrectHighlight(class rasterview  *self) 
{
    class graphic *G = (self)->GetDrawable();
    struct rectangle *CS = &self->CurSelection;
    struct rectangle *DS = &self->DesiredSelection;

    if (rectangle_IsEqualRect(CS, DS)
	 && self->HighlightIsGrey == self->HasInputFocus)
	/* highlight is correct */
	return;

    if (IsNotEmptyRect(&self->CurSelection))
	rasterview_ViewHideHighlight(self);	/* remove the old highlight */

    self->HighlightIsGrey = self->HasInputFocus;
    *CS = *DS;
    if (IsEmptyRect(CS)) {
	/* There is no current selection.  This happens when there is no raster.  To show the user where we are, draw a rect.  Use Visual Bounds. */
	struct rectangle VB;
	(self)->GetVisualBounds( &VB);
	(self)->SetTransferMode( graphic::BLACK);
	SetWidthRect(&VB, rectangle_Width(&VB)-1);
	SetHeightRect(&VB, rectangle_Height(&VB)-1);
	(G)->DrawRect( &VB);
	return; }

    if (! TouchUp(self)) {
	if (self->HasInputFocus) {
	    /* draw black and white lines */
	    if (NotFullSize(self)) {
		rasterview_DrawHighlightBehindDisplayBox(self, G, 0);
		rasterview_DisplayBoxDrawHighlight(self, G); }
	    else
		DrawHighlightBlackAndWhite(self, G, self->DesiredSelection);
	}
	else {
	    /* draw grey border */
	    if (NotFullSize(self)) {
		rasterview_DrawHighlightBehindDisplayBox(self, G, 1);
		rasterview_DisplayBoxDrawHighlightGray(self, G); }
	    else {
		long l, t, w, h;
		rectangle_GetRectSize(&self->DesiredSelection, &l, &t, &w, &h);
		l -= self->Xoff;  /* convert to screen coords */
		t -= self->Yoff;
		(self)->SetTransferMode( graphic::COPY);
		(self)->FillRectSize( l-2, t-2, w+4, 2, self->GreyPattern);
		(self)->FillRectSize( l-2, t+h, w+4, 2, self->GreyPattern);
		(self)->FillRectSize( l-2, t, 2, h, self->GreyPattern);
		(self)->FillRectSize( l+w, t, 2, h, self->GreyPattern); }
	}
    }
}

#define CalculatePanHighlight(self, R) {					\
if (FullSize((self)))								\
    *(R) = (self)->ViewSelection;						\
else {										\
    *(R) = (self)->ViewSelection;						\
    rectangle_SetRectSize((R),							\
			   (rectangle_Left((R)) - rectangle_Left(&(self)->DisplayBoxSelection)) * (self)->Scale,					\
			   (rectangle_Top((R)) - rectangle_Top(&(self)->DisplayBoxSelection)) * (self)->Scale,					\
			   rectangle_Width((R)) * (self)->Scale,		\
			   rectangle_Height((R)) * (self)->Scale); } }

void rasterview_DrawPanHighlight(class rasterview  *self, short  g)
{
    class graphic *G = (self)->GetDrawable();

    DEBUG(("Drawing Pan Highlight\n"));
    if (g == graphic::BLACK) {
	if (self->DisplayBoxHidden) {
	    struct rectangle PH;
	    CalculatePanHighlight(self, &PH);
	    DrawHighlightBlackAndWhite(self, G, PH); }
	else {
	    struct rectangle savedDS;
	    savedDS = self->DesiredSelection;
	    self->DesiredSelection = self->ViewSelection;
	    rasterview_DrawHighlightBehindDisplayBox(self, G, 0);
	    rasterview_DisplayBoxDrawHighlight(self, G);
	    self->DesiredSelection = savedDS; } }
    else {
	struct rectangle R;
	CalculatePanHighlight(self, &R);
	DrawHighlightWhite(self, G, R);
	if (! self->DisplayBoxHidden) {
	    rasterview_DisplayBoxWritePixImageFull(self, G,
					((class raster *) (self)->GetDataObject())->GetPix()); }
    }
    if (self->embedded) {
	struct rectangle VB;
	(self)->GetVisualBounds( &VB);
	InsetRect(&VB, BORDER, BORDER);
	DrawHighlightScreenCoordinates(self, (self)->GetDrawable(), VB,
				       graphic::BLACK, -1); }
}

/* Draw a Target across the entire Visual Bounds less a BORDER all around. Target is three pixels wide: a black line surrounded by two white lines.
*/
static void DrawTarget(class rasterview  *self, long  x , long  y)
{
    struct rectangle VB;
    struct rectangle PH;
    long w, h;

    (self)->GetVisualBounds( &VB);
    InsetRect(&VB, BORDER, BORDER);
    w = rectangle_Width(&VB);
    h = rectangle_Height(&VB);

    (self)->SetTransferMode( graphic::BLACK);
    (self)->MoveTo( x, BORDER);
    (self)->DrawLineTo( x, h);
    (self)->MoveTo( BORDER, y);
    (self)->DrawLineTo( w, y);

    rasterview_CorrectHighlight(self);

    CalculatePanHighlight(self, &PH);
    OffsetRect(&PH, - self->Xoff - BORDER,- self->Yoff - BORDER)

    rectangle_IntersectRect(&VB, &VB, &PH);
    if (IsNotEmptyRect(&VB)) {
	long l, t, w, h, r, b;
	rectangle_GetRectSize(&VB, &l, &t, &w, &h);
	l++; t++;
	r = l + w;
	b = t + h;
	DEBUG(("White Target: (%ld,%ld,%ld,%ld)\n", l, t, w, h));
	(self)->SetTransferMode( graphic::WHITE);
	(self)->MoveTo( x+1, t);
	(self)->DrawLineTo( x+1, y-1);
	(self)->DrawLineTo( r, y-1);
	(self)->MoveTo( r, y+1);
	(self)->DrawLineTo( x+1, y+1);
	(self)->DrawLineTo( x+1, b);
	(self)->MoveTo( x-1, b);
	(self)->DrawLineTo( x-1, y+1);
	(self)->DrawLineTo( l, y+1);
	(self)->MoveTo( l, y-1);
	(self)->DrawLineTo( x-1, y-1);
	(self)->DrawLineTo( x-1, t);

    }

    /* Just in case any of the above white lines or the preceeding Hide overlapped the Start. */
    (self)->SetTransferMode( graphic::BLACK);
    (self)->MoveTo( self->StartPanX, BORDER);
    (self)->DrawLineTo( self->StartPanX, h);
    (self)->MoveTo( BORDER, self->StartPanY);
    (self)->DrawLineTo( w, self->StartPanY);
}

static void HideTarget(class rasterview  *self, long  x , long  y)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    class rasterimage *pix;
    class graphic *G = (self)->GetDrawable();
    long clipw, cliph;
    struct rectangle VB;
    struct rectangle VS;
    long vsl, vst, vsw, vsh, vsr, vsb;

    if (ras == NULL || (pix = (ras)->GetPix()) == NULL) return;
    if (NotFullSize(self)) pix = self->Expansion;

    clipw = (pix)->GetWidth();
    cliph = (pix)->GetHeight();

    (self)->GetVisualBounds( &VB);
    InsetRect(&VB, BORDER, BORDER);
    (self)->SetTransferMode( graphic::WHITE);
    (self)->MoveTo( x, BORDER);
    (self)->DrawLineTo( x, rectangle_Height(&VB));
    (self)->MoveTo( BORDER, y);
    (self)->DrawLineTo( rectangle_Width(&VB), y);
    rasterview_DrawPanHighlight(self, graphic::BLACK);

    if (FullSize(self)) {
	VS = self->ViewSelection;
	OffsetRect(&VS,
		   -self->Xscroll + BORDER - rectangle_Left(&VS),
		   -self->Yscroll + BORDER - rectangle_Top(&VS)); }
    else {
	VS = self->DisplayBoxSelection;
	rectangle_SetRectSize(&VS,
			      BORDER + ((self->Xscroll < 0) ?
					-self->Xscroll : 0),
			      BORDER + ((self->Yscroll < 0) ?
					-self->Yscroll : 0),
			      rectangle_Width(&VS)*self->Scale,
			      rectangle_Height(&VS)*self->Scale); }
    rectangle_IntersectRect(&VS, &VS, &VB);
    rectangle_GetRectSize(&VS, &vsl, &vst, &vsw, &vsh);
    vsr = vsl + vsw;
    vsb = vst + vsh;
    DEBUG(("Hide Area: (%ld,%ld,%ld,%ld) Scl:(%ld,%ld) \n",
	    vsl, vst, vsw, vsh, self->Xscroll, self->Yscroll));

    (self)->SetTransferMode( graphic::COPY);

    if (x+1 >= vsl && x-1 <= vsr) {
	/* Repair Vertical strip */
	long l = (x-1 < vsl) ? vsl : x-1;
	l = (l+3 > vsr) ? vsr-3 : l;
	DEBUG(("Ver:(%ld,%ld,%d,%ld) Off:(%ld,%ld)\n",
	       l, vst, 3, vsh, self->Xoff, self->Yoff));
	ClipAndWritePixImage(clipw, cliph, G, l, vst, pix,
			     l+self->Xoff, vst+self->Yoff, 3, vsh);
    }
    if (y+1 >= vst && y-1 <= vsb) {
	/* Repair Horizontal strip */
	long t = (y-1 < vst) ? vst : y-1;
	t = (t+3 > vsb) ? vsb-3 : t;
	DEBUG(("Hor:(%ld,%ld,%ld,%d) Off:(%ld,%ld)\n",
	       vsl, t, vsw, 3, self->Xoff, self->Yoff));
	ClipAndWritePixImage(clipw, cliph, G, vsl, t, pix,
			     vsl+self->Xoff, t+self->Yoff, vsw, 3);
    }
    rasterview_DrawPanHighlight(self, graphic::BLACK);

    if (! self->DisplayBoxHidden && IsNotEmptyRect(&self->DisplayBox)
	 && ((x-1 <= rectangle_Left(&self->DisplayBox)-2*BORDER
	      && x+1 >= rectangle_Right(&self->DisplayBox)+2*BORDER)
	     || (x+1 >= rectangle_Left(&self->DisplayBox)-2*BORDER
		 && x-1 <= rectangle_Right(&self->DisplayBox)+2*BORDER)
	     || (y-1 <= rectangle_Top(&self->DisplayBox)
		 && y+1 >= rectangle_Bottom(&self->DisplayBox))
	     || (y+1 >= rectangle_Top(&self->DisplayBox)-2*BORDER
		 && y-1 <= rectangle_Bottom(&self->DisplayBox)+2*BORDER)))
	rasterview_DisplayBoxWritePixImage(self, G);
}

static void StartPanning(class rasterview  *self, long  x , long  y)
{
    DEBUG(("Start Panning at: (%ld,%ld)\n", x, y));
    self->StartPanX = x;
    self->StartPanY = y;
    self->PanX = x;
    self->PanY = y;
    self->SavedDesiredSelection = self->DesiredSelection;
    self->DesiredSelection = self->ViewSelection;
    /* the following hide forces a calculation of the PanHighlight */
    HideTarget(self, x, y);
    DrawTarget(self, x, y);
}

static void ContinuePanning(class rasterview  *self, long  x , long  y)
{
    DEBUG(("Continue Panning at: (%ld,%ld)\n", x, y));
    if (x == self->PanX && y == self->PanY) return;

    if (self->ShowCoords) {
	static char cb[20];
	sprintf(cb, "(%ld,%ld)", x-self->StartPanX, y-self->StartPanY);
	message::DisplayString(self, 0, cb);
    }

    HideTarget(self, self->PanX, self->PanY);
    DrawTarget(self, self->StartPanX, self->StartPanY);
    DrawTarget(self, x, y);
    self->PanX = x;
    self->PanY = y;
    if (self->embedded) {
	struct rectangle VB;
	(self)->GetVisualBounds( &VB);
	InsetRect(&VB, BORDER, BORDER);
	DrawHighlightScreenCoordinates(self, (self)->GetDrawable(), VB,
				       graphic::BLACK, -1); }
}

static void ClipScroll(class rasterview  *self)
{
    long minLeft, maxRight;
    long minTop, maxBottom;
    struct rectangle VB;

    (self)->GetVisualBounds( &VB);
    if (NotFullSize(self)) {
	self->Xscroll -= (self->Xscroll % self->Scale);
	minLeft = - (rectangle_Width(&VB) - 40);
	maxRight = (rectangle_Width(&self->ViewSelection) - 20) * self->Scale;
	self->Yscroll -= (self->Yscroll % self->Scale);
	minTop = - (rectangle_Height(&VB) - 40);
	maxBottom = (rectangle_Height(&self->ViewSelection) - 20) * self->Scale;
    }
    else {
	minLeft = - (rectangle_Width(&VB) - 18);
	maxRight = rectangle_Width(&self->ViewSelection) - 18;
	minTop = - (rectangle_Height(&VB) - 18);
	maxBottom = rectangle_Height(&self->ViewSelection) - 18;
    }
    if (minLeft > 0) minLeft = 0;
    if (minTop > 0) minTop = 0;
    if (maxRight < 0) maxRight = 0;
    if (maxBottom < 0) maxBottom = 0;

    DEBUG(("ClipScroll:\n   Visual: (%ld,%ld,%ld,%ld)\n   VS: (%ld,%ld,%ld,%ld)\n   Min: (%ld,%ld) Max: (%ld,%ld)\n",
	    rectangle_Left(&VB), rectangle_Top(&VB),
	    rectangle_Width(&VB), rectangle_Height(&VB),
	    rectangle_Left(&self->ViewSelection), rectangle_Top(&self->ViewSelection),
	    rectangle_Width(&self->ViewSelection), rectangle_Height(&self->ViewSelection),
	    minLeft, minTop, maxRight, maxBottom));

    if (self->Xscroll < minLeft) self->Xscroll = minLeft;
    if (self->Xscroll > maxRight) self->Xscroll = maxRight;
    if (self->Yscroll < minTop) self->Yscroll = minTop;
    if (self->Yscroll > maxBottom) self->Yscroll = maxBottom;
    
    DEBUG(("   Scroll: (%ld,%ld)\n", self->Xscroll, self->Yscroll));

}




static void FinishPanning(class rasterview  *self, long  x , long  y)
{
    DEBUG(("Finish Panning at: (%ld,%ld)\n", x, y));
    DEBUG(("Started at: (%ld,%ld)\n", self->StartPanX, self->StartPanY));
    if (x == self->StartPanX && y == self->StartPanY)
	HideTarget(self, x, y);
    else {
	if (FullSize(self)) {
	    self->Xscroll += self->StartPanX - x;
	    self->Yscroll += self->StartPanY - y;
	    ClipScroll(self); }
	else {
	    long dx, dy;
	    dx = self->StartPanX - (self->StartPanX % self->Scale);
	    dy = self->StartPanY - (self->StartPanY % self->Scale);
	    dx -= x;
	    dy -= y;
	    dx -= (dx % self->Scale);
	    dy -= (dy % self->Scale);
	    DEBUG(("Delta: (%ld,%ld)\n", dx, dy));
	    self->Xscroll += dx;
	    self->Yscroll += dy;
	    ClipScroll(self);
	    if (dx !=0 || dy != 0) {
		rectangle_EmptyRect(&self->DisplayBoxSelection);
		rasterview_UpdateZoomedSelection(self,
				      self->Xscroll/self->Scale,
				      self->Yscroll/self->Scale); }
	}

	self->DesiredSelection = self->SavedDesiredSelection;

	self->needsFullUpdate = TRUE;
	(self)->WantUpdate( self); }
}

/* quick-n-dirty one-pixel draw. Displays correctly in rasterview self, even if zoomed, but does *not* notify other observers. x and y must be legal values. Note that bit is a boolean value. Also note that this is slightly different from the exported method rasterview_SetPixel(). */
static void SetPixel(class rasterview  *self, long  x , long  y, boolean  bit)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    class rasterimage *pix;

    if ((ras == NULL) || ((pix = (ras)->GetPix()) == NULL)) return;
    DEBUG(("Set Pixel in 0x%p to %s at (%ld,%ld)\n", pix, ((bit) ? "Black" : "White"), x, y));
    if (NotFullSize(self))
	rasterview_SetPixelBehindDisplayBox(self, self->Expansion, x, y, bit);
    else {
	/* The following line does not work in X windows currently.
	graphic::SetBitAtLoc(rasterview_GetDrawable(self),
			    x - self->Xoff, y - self->Yoff, bit); */
	struct rectangle sub;
	rectangle_SetRectSize(&sub, x - self->Xoff, y - self->Yoff, 1, 1);
	(self)->SetTransferMode( graphic::COPY);
	(self)->FillRect( &sub, ((bit) ? self->BlackPattern : self->WhitePattern));
	((ras)->GetPix())->SetPixel( x, y, ((bit) ? 1 : 0));
    }
}

/* draw a line from self->Touch{X,Y} to x1, y1. Uses SetPixel. */
/* the following algorithm is taken from "Fundamentals of Interactive Graphics" by J. D. Foley and A. Van Dam, page 435.*/
static void DrawLineTo(class rasterview   *self, long  x1 , long  y1, boolean  bit)
/* draw the line in the unzoomed version and reflect the changes up. */
{
    class raster *ras = (class raster *)(self)->GetDataObject();

    long x2 = self->TouchUpX, y2 = self->TouchUpY;

    /* start algorithm */
    long dx, dy;
    long incr1, incr2, d, x, y;
    float slope;

    if (ras == NULL || (ras)->GetPix() == NULL) return;

    dx = x2-x1;
    dy = y2-y1;
 
    if (dx == 0) {
       long yend;
       if (y1 < y2) {
          y = y1; yend = y2; }
        else {
           y = y2; yend = y1; }
       for (; y<= yend; y++)
          SetPixel(self, x1, y, bit);
       return; }

    slope = (float)dy/dx;
    dx = (dx>=0) ? dx : -dx;
    dy = (dy>=0) ? dy : -dy;

    /* both x and y are either increasing or decreasing */
    if (slope > 0) {
	if (slope <= 1) {
	    long xend;
	    /* Slope is between 0 and 1. */
	    if (x1 > x2) {
		x = x2;  y = y2;  xend = x1; }
	    else {
		x = x1;  y = y1;  xend = x2; }
	    SetPixel(self, x, y, bit);
	    d = 2*dy - dx;
	    incr1 = 2*dy;
	    incr2 = 2*(dy-dx);
	    while (x < xend) {
		x++;
		if (d < 0) d += incr1;
		else {
		    y++;
		    d += incr2; }
		SetPixel(self, x, y, bit); } }
	else {
	    long yend;
	    /* Slope is greater than 1. */
	    if (y1 > y2) {
		x = x2;  y = y2;  yend = y1; }
	    else {
		x = x1;  y = y1;  yend = y2; }
	    SetPixel(self, x, y, bit);
	    d = 2*dx - dy;
	    incr1 = 2*dx;
	    incr2 = 2*(dx-dy);
	    while (y < yend) {
		y++;
		if (d < 0) d += incr1;
		else {
		    x++;
		    d += incr2; }
		SetPixel(self, x, y, bit); } }
    }
    else {
	if (slope >= -1) {
	    long xend;
	    /* Slope is between 0 and -1. */
	    if (x1 < x2) {
		x = x2;  y = y2;  xend = x1; }
	    else {
		x = x1;  y = y1;  xend = x2; }
	    SetPixel(self, x, y, bit);
	    d = 2*dy - dx;
	    incr1 = 2*dy;
	    incr2 = 2*(dy-dx);
	    while (x > xend) {
		x--;
		if (d < 0) d += incr1;
		else {
		    y++;
		    d += incr2; }
		SetPixel(self, x, y, bit); } }
	else {
	    long yend;
	    /* Slope is less than -1. */
	    if (y1 < y2) {
		x = x2;  y = y2;  yend = y1; }
	    else {
		x = x1;  y = y1;  yend = y2; }
	    SetPixel(self, x, y, bit);
	    d = 2*dx - dy;
	    incr1 = 2*dx;
	    incr2 = 2*(dx-dy);
	    while (y > yend) {
		y--;
		if (d < 0) d += incr1;
		else {
		    x++;
		    d += incr2; }
		SetPixel(self, x, y, bit); } }
    } }

/* The (x, y) argument here is the upper left bit of the current View Selection to be displayed on the screen. If the x value is negative then display (0, y) at a negative xscroll. If the y value is negative then display (x, 0) at a negative yscroll. */
void rasterview_ZoomToVisualBounds(class rasterview  *self, long  x , long  y)
{
    class raster *ras = (class raster *)(self)->GetDataObject();
    class rasterimage *pix = (ras)->GetPix();
    struct rectangle VB;
    struct rectangle OldDisplayBoxSelection;
    long UseableW, SmallW, ZoomW;
    long UseableH, SmallH, ZoomH;
    long diff;

    OldDisplayBoxSelection = self->DisplayBoxSelection;

    DEBUG(("Enter rasterview_ZoomToVisualBounds: (%ld, %ld)\n", x, y));

    /* Update the scroll values given the new (x, y). */
    self->Xscroll = x*self->Scale;
    self->Yscroll = y*self->Scale;
    ClipScroll(self);
    /* Use clipped values for (x, y) */
    x = self->Xscroll/self->Scale;
    y = self->Yscroll/self->Scale;

    self->DBXscroll = (self->Xscroll < 0) ? self->Xscroll/self->Scale : 0;
    self->DBYscroll = (self->Yscroll < 0) ? self->Yscroll/self->Scale : 0;
    self->Xoff = ((self->Xscroll < 0) ? self->Xscroll : 0) - BORDER;
    self->Yoff = ((self->Yscroll < 0) ? self->Yscroll : 0) - BORDER;

    /* Useable Width is the number of pixels we can use for display of the pixelimg.  Small Width is the number of pixels in the original raster which we are going to Zoom.  The Small Width is the smaller of the Useable Width divided by the scale of expansion and the number of pixels between the left edge of the selected region and the right edge of the image.  The Height can be determined in a similar manner. Note that the Display Box is always shown in the lower left corner whether or not it overlaps the Zoomed version of the image.  If there is an overlap, blit into the pixelimage the portion of the Display Box (including the border) which overlaps so that the updating of the selected region rectangle on the screen will not mis-paint the screen with the wrong information. */

    (self)->GetVisualBounds( &VB);

    UseableW = rectangle_Width(&VB) - 2*BORDER;
    UseableW = UseableW - (UseableW % self->Scale);
    SmallW = UseableW/self->Scale;
    SetWidthRect(&self->DisplayBox, SmallW);
    if (x < 0) {    
	SmallW += x;
	x = 0; }
    diff = rectangle_Right(&self->ViewSelection) - x;
    if (SmallW > diff && diff > 0) SmallW = diff;
    SetWidthRect(&self->DisplayBoxSelection, SmallW);

    UseableH = rectangle_Height(&VB) - 2*BORDER;
    UseableH = UseableH - (UseableH % self->Scale);
    SmallH = UseableH/self->Scale;
    SetHeightRect(&self->DisplayBox, SmallH);
    if (y < 0) {    
	SmallH += y;
	y = 0; }
    diff = rectangle_Bottom(&self->ViewSelection) - y;
    if (SmallH > diff && diff > 0) SmallH = diff;
    SetHeightRect(&self->DisplayBoxSelection, SmallH);

    SetLeftRect(&self->DisplayBoxSelection, x);
    SetTopRect(&self->DisplayBoxSelection, y);

    if (self->DisplayBoxHidden) {
	SetLeftRect(&self->DisplayBox, -3*rectangle_Width(&self->DisplayBox)); }
    else {
	/* Force the Display Box to be located within the Visual Bounds. */
	if (rectangle_Left(&self->DisplayBox) < rectangle_Left(&VB)
	    || rectangle_Right(&self->DisplayBox) > rectangle_Right(&VB))
	    SetLeftRect(&self->DisplayBox, 5*self->Scale);
	if (rectangle_Top(&self->DisplayBox) < rectangle_Top(&VB)
	    || rectangle_Bottom(&self->DisplayBox) > rectangle_Bottom(&VB))
	    SetTopRect(&self->DisplayBox,
		       rectangle_Bottom(&VB) - rectangle_Height(&self->DisplayBox) - 5*self->Scale); }

    DEBUG(("ZoomVB: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&VB),
	    rectangle_Top(&VB),
	    rectangle_Width(&VB),
	    rectangle_Height(&VB)));
    DEBUG(("ZoomDB: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&self->DisplayBox),
	    rectangle_Top(&self->DisplayBox),
	    rectangle_Width(&self->DisplayBox),
	    rectangle_Height(&self->DisplayBox)));
    DEBUG(("ZoomDBS: (%ld,%ld,%ld,%ld)\n",
	    rectangle_Left(&self->DisplayBoxSelection),
	    rectangle_Top(&self->DisplayBoxSelection),
	    rectangle_Width(&self->DisplayBoxSelection),
	    rectangle_Height(&self->DisplayBoxSelection)));

    if (! rectangle_IsEqualRect(&OldDisplayBoxSelection, &self->DisplayBoxSelection)) {

	ZoomW = SmallW*self->Scale;
	ZoomH = SmallH*self->Scale;

	(self->Expansion)->Resize( ZoomW, ZoomH);
	(pix)->GetScaledSubraster( &self->DisplayBoxSelection,
				       ZoomW, ZoomH, self->Expansion);

	DEBUG(("ZoomSize: %ld x %ld\n", ZoomW, ZoomH));

	DisplayBoxBlitOverlap(self, pix); 
    }

    
}

void rasterview_UpdateZoomedSelection(class rasterview  *self, long  x , long  y)
{
    /* XXX Can be made MUCH more efficient. */
    rasterview_ZoomToVisualBounds(self, x, y);
}
