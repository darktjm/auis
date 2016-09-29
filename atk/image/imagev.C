/* imagev.c - class description for view on images */
/*
	Copyright Carnegie Mellon University 1992, 1996 - All rights reserved
*/

#include <andrewos.h>
ATK_IMPL("imagev.H")

#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>
#include <rect.h>
#include <attribs.h>
#include <filetype.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <cursor.H>
#include <bind.H>
#include <environ.H>
#include <completion.H>
#include <message.H>
#include <scroll.H>
#include <view.H>
#include <buffer.H>
#include <print.H>
#include <image.H>
#include <graphic.H>
#include <frame.H>
#include <im.H>
#include <texttroff.H>
#include <sbuttonv.H>
#include <sbutton.H>
#include <region.H>
#include <color.H>
#include <imageio.H>
#include <imagev.H>
#include <util.h>

#define CONTINUOUS_PAN	0
#define DISCRETE_PAN	1

#define	IMAGEV_DEFAULTMENUS		(1<<6)
#define	IMAGEV_READWRITEMENUS		(1<<7)
#define	IMAGEV_READONLYMENUS		(1<<8)
#define	IMAGEV_GOODIMAGEMENUS		(1<<9)
#define IMAGEV_JPEGFORMATMENUS		(1<<10)
#define IMAGEV_CANHAVEPRIVATECMAP	(1<<11)
#define IMAGEV_HASPRIVATECMAP		(1<<12)
#define IMAGEV_HASNOTPRIVATECMAP	(1<<13)

static class keymap *imagev_keymap;
static class menulist *internalMenulist = NULL;
static class cursor *waitCursor;

#define image_STARTHEIGHT (150)
#define image_STARTWIDTH (150)
#define DEFAULT_BORDER_SIZE (5)

/* This macro accesses the displayed copy of the image if possible, otherwise it uses the original.
 It should only be used where the scaled image is supposed to be used.  */

#define IMAGE(self) ((struct image*) (self->scaled ? self->scaled : self->orig))

#define CLEARRECT(self, RectPtr)			\
    (self)->SetTransferMode( graphic_WHITE);		\
    (self)->FillRect( RectPtr, (self)->BlackPattern())

ATKdefineRegistry(imagev, view, imagev::InitializeClass);

static void PostCursor( class imagev  *self, int  type );
static void DrawBorder( class imagev  *self, struct rectangle  *outer , struct rectangle  *inner );
void GetScreenCoordinates( class imagev  *self, struct rectangle  *pixRect );
static void SaveAs( class imagev  *self, long  rock );
static int  WriteToFile( class imagev   *self, const char  *filename );
#ifdef THISWORKS
static void Dither( class imagev  *self );
static void Halftone( class imagev  *self );
static void Reduce( class imagev  *self );
static void Gray( class imagev  *self );
static void Normalize( class imagev  *self );
static void Brighten( class imagev  *self );
static void GammaCorrect( class imagev  *self );
static void ScaleToFit( class imagev  *self );
#endif
#if 0
static void InfoCmd( class imagev  *self );
#endif
static void ShowTrue( class imagev  *self );
static void ShowFixed( class imagev  *self );
static void Write_Postscript( class imagev  *self );
static void ReadCmd( class imagev  *self );
static void ChangeZoomCmd( class imagev  *self, long  rock );
static void PanToOriginCmd( class imagev  *self, long  rock );
static void SetSaveQuality( class imagev  *self, long  rock );
static void SetSaveFormat( class imagev  *self, long  rock );

enum image_fileType {
  cmuwm_imageType,			/* CMU WM Raster */
  any_imageType				/* File type supported by devil */
};

static void Import_Cmd( class imagev  *self, enum image_fileType  type );
static void Export_Cmd( class imagev  *self, enum image_fileType  type );

static struct bind_Description imagevBindings[] = {

  {"imagev-import-any", NULL, any_imageType, "Import~30, Almost Any~12", any_imageType, IMAGEV_DEFAULTMENUS, (proctable_fptr) Import_Cmd, "Import an image."},

  {"imagev-import-cmuwm", NULL, cmuwm_imageType, "Import~30, ATK Raster~18", cmuwm_imageType, IMAGEV_DEFAULTMENUS, (proctable_fptr) Import_Cmd, "Import a cmuwm image."},

  {"imagev-export-any", NULL, any_imageType, "Export~40, By Name~17", any_imageType, IMAGEV_DEFAULTMENUS, (proctable_fptr) (proctable_fptr) Export_Cmd, "Write out an image."},

  {"imagev-export-postscript", NULL, 0, "Export~40, Postscript~12", 0, IMAGEV_DEFAULTMENUS, (proctable_fptr) Write_Postscript, "Write out a Postscript file."},

  {"imagev-export-cmuwm", NULL, cmuwm_imageType, "Export~40, CMU WM~16", cmuwm_imageType, IMAGEV_DEFAULTMENUS, (proctable_fptr) (proctable_fptr) Export_Cmd, "Write out a cmuwm image."},

#ifdef THISWORKS
  {"imagev-gray", NULL, 0, "Image~20, gray~20", 0, IMAGEV_GOODIMAGEMENUS, (proctable_fptr) Gray, "GrayScale an image."},

  {"imagev-dither", NULL, 0, "Image~20, dither~21", 0, IMAGEV_GOODIMAGEMENUS, (proctable_fptr) Dither, "Dither an image."},

  {"imagev-scale-to-fit", NULL, 0, "Image~20, Scale to fit~10", 0, IMAGEV_GOODIMAGEMENUS, (proctable_fptr) ScaleToFit, "Scale an image to fit the available screen space."},

  {"imagev-halftone", NULL, 0, "Image~20, halftone~30", 0, IMAGEV_GOODIMAGEMENUS, (proctable_fptr) Halftone, "Halftone an image."},

  {"imagev-reduce", NULL, 0, "Image~20, reduce~31", 0, IMAGEV_GOODIMAGEMENUS, (proctable_fptr) Reduce, "Reduce an image."},

  {"imagev-normalize", NULL, 0, "Image~20, normalize~32", 0, IMAGEV_GOODIMAGEMENUS, (proctable_fptr) Normalize, "Normalize an image."},

  {"imagev-brighten", NULL, 0, "Image~20, brighten~33", 0, IMAGEV_GOODIMAGEMENUS, (proctable_fptr) Brighten, "Brighten an image."},

  {"imagev-gamma-correct", NULL, 0, "Image~20, gamma correct~34", 0, IMAGEV_GOODIMAGEMENUS, (proctable_fptr) GammaCorrect, "GammaCorrect an image."},

#endif

  {"imagev-image-read", NULL, 0, "Image~20, Read~10", 0, IMAGEV_DEFAULTMENUS, (proctable_fptr) ReadCmd, "Read in a new image file."},

  {"imagev-image-write", NULL, 0, "Image~20, Write~11", 0, IMAGEV_DEFAULTMENUS, (proctable_fptr) SaveAs, "Save image to file."},

  {"imagev-zoom-in", "\033Z", 1, "Image~20, Zoom In~20", 1, IMAGEV_GOODIMAGEMENUS, (proctable_fptr) ChangeZoomCmd, "Change scale of view."},

  {"imagev-zoom-out", "\033z", -1, "Image~20, Zoom Out~22", -1, IMAGEV_GOODIMAGEMENUS, (proctable_fptr) ChangeZoomCmd, "Change scale of view."},

  {"imagev-zoom-normal", "\033n", 0, "Image~20, Normal Size~24", 0, IMAGEV_GOODIMAGEMENUS, (proctable_fptr) ChangeZoomCmd, "Change scale of view."},

  {"imagev-pan-to-origin", "\033o", 0, "Image~20, Pan to Origin~30", 0, IMAGEV_GOODIMAGEMENUS, (proctable_fptr) PanToOriginCmd, "Pan to (0,0)."},

{"imagev-show-true", NULL, 0, "Image~20, Show true~40", 0, IMAGEV_GOODIMAGEMENUS | IMAGEV_CANHAVEPRIVATECMAP | IMAGEV_HASNOTPRIVATECMAP, (proctable_fptr) ShowTrue, "Show image with private cmap."},

{"imagev-show-fixed", NULL, 0, "Image~20, Show fixed~42", 0, IMAGEV_GOODIMAGEMENUS | IMAGEV_CANHAVEPRIVATECMAP | IMAGEV_HASPRIVATECMAP, (proctable_fptr) ShowFixed, "Show image with fixed cmap."},

  {"imagev-set-save-quality", NULL, 0, "Image~20, Set Save Quality~50", 0, IMAGEV_GOODIMAGEMENUS | IMAGEV_JPEGFORMATMENUS, (proctable_fptr) SetSaveQuality, "Set the quality level for the JPEG compressor [5-95]."},

  {"imagev-set-save-format", NULL, 0, "Image~20, Set Save Format~60", 0, IMAGEV_DEFAULTMENUS, (proctable_fptr) SetSaveFormat, "Set the save format to be used by image. Can be png, gif or jpeg."},

#if 0
  {"imagev-image-info", NULL, 0, "Image~20, Info~60", 0, IMAGEV_DEFAULTMENUS, (proctable_fptr) InfoCmd, "Get information about image."},
#endif

  NULL
};

#define INSETRECT(r, deltax, deltay) rectangle_SetRectSize((r), rectangle_Left((r)) + (deltax), rectangle_Top((r)) + (deltay), rectangle_Width((r)) - 2*(deltax), rectangle_Height((r)) - 2*(deltay));

#define POSTWAITCURSOR(self) im::SetProcessCursor(waitCursor)
#define RETRACTWAITCURSOR(self) im::SetProcessCursor(NULL)

static void
PostCursor( class imagev  *self, int  type )
{
    struct rectangle r;
    if (self->cursorPosted) {
	self->cursorPosted = FALSE;
	(self)->RetractCursor( self->cursor);
    }
    (self)->GetVisualBounds(&r);
    (self->cursor)->SetStandard( type);
    (self)->view::PostCursor( &r, self->cursor);
    self->cursorPosted = TRUE;
}

boolean
imagev::InitializeClass( )
{
    struct ATKregistryEntry  *classInfo = NULL;
    imagev_keymap = new keymap;
    internalMenulist = new menulist;
    classInfo = ATK::LoadClass("imagev");
    bind::BindList(imagevBindings, imagev_keymap, internalMenulist, classInfo);
    waitCursor = cursor::Create(NULL);
    (waitCursor)->SetStandard( Cursor_Wait);
    return(TRUE);
}

static const char panebutton[]="panebutton";

imagev::imagev( )
{
    ATKinit;

    this->orig = this->scaled = NULL;
    if(!(this->keystatep = keystate::Create(this, imagev_keymap))) {
	fprintf(stderr,"imagev: could not create keystate.\n");
	THROWONFAILURE((FALSE));
    }
    this->menulistp = (internalMenulist)->DuplicateML( this);
    this->cursor = cursor::Create(this);
    this->cursorPosted = FALSE;
    (this->cursor)->SetStandard( Cursor_Arrow);
    this->do_fullupdate = TRUE;
    this->do_renderupdate = FALSE;
    this->image_modified = FALSE;
    this->buttonprefs = sbutton::GetNewPrefs(panebutton);
    sbutton::InitPrefs(this->buttonprefs, panebutton);
    this->haveFocus = this->isLinked = FALSE;
    this->canvas = NULL;
    this->onScreen = FALSE;
    this->embedded = TRUE;
    this->scale = imagev_NormScale;
    this->panx = this->pany = 0;
    this->bordersize = environ::GetProfileInt("imagebordersize", DEFAULT_BORDER_SIZE);
    this->panStyle = environ::GetProfileInt("imagepanstyle", CONTINUOUS_PAN);
    this->click_to_draw = environ::GetProfileSwitch("imageclickdraw", FALSE);
    this->click_skip_update = FALSE;

    this->lastPixRect = (struct rectangle *) calloc(1, sizeof(struct rectangle));
    rectangle_EmptyRect(this->lastPixRect);
    this->havePrivateCmap = FALSE;
    this->privateCmap = NULL;
    THROWONFAILURE((TRUE));
}


imagev::~imagev( )
{
    delete cursor;
    cursor=NULL;
    
    dimage.RemoveObserver(this);

    if(this->buttonprefs) {
	sbutton::FreePrefs(this->buttonprefs);
	this->buttonprefs=NULL;
    }

    if(this->privateCmap) {
	this->privateCmap->Destroy();
	this->privateCmap=NULL;
    }
    if(this->canvas) {
	free(this->canvas);
	this->canvas=NULL;
    }

    if(this->scaled) {
	(this->scaled)->Destroy();
	this->scaled = NULL;
    }
    if(this->menulistp) {
	delete this->menulistp;
	this->menulistp=NULL;
    }
    if(this->keystatep) {
	delete this->keystatep;
	this->keystatep=NULL;
    }
    if(this->lastPixRect) {
	free(this->lastPixRect);
	this->lastPixRect=NULL;
    }
}

static const struct sbutton_info thebutton = {
    NULL,   /* the prefs struct will be filled later */
    "",	    /* the label is empty */
    0,	    /* the rock isn't needed */
    NULL,   /* ditto for the trigger atom */
    FALSE,  /* initially not lit, will be set appropriately */
};

#define SENDIMAGE(self, DestPt, SrcRect)		\
((self))->WriteImage(		\
		   point_X(DestPt), point_Y(DestPt),	\
		   self->dimage,				\
		   rectangle_Left(SrcRect),		\
		   rectangle_Top(SrcRect),		\
		   rectangle_Width(SrcRect),		\
		   rectangle_Height(SrcRect))


static void
DrawBorder( class imagev  *self, struct rectangle  *outer , struct rectangle  *inner )
{
    if(self->bordersize==0) {
	if(inner) *inner=*outer;
    } else {
	sbuttonv::DrawRectBorder(self, outer, self->buttonprefs, self->haveFocus, TRUE, inner, FALSE);
	sbuttonv::DrawRectBorder(self, inner, self->buttonprefs, !self->haveFocus, TRUE, inner, FALSE);
    }
}

void
imagev::FullUpdate( enum view_UpdateType type, long left, long top, long width, long height )
{
    this->isLinked = TRUE;
    if(type == view_FullRedraw || type == view_LastPartialRedraw) {
	class region *clipRgn = region::CreateEmptyRegion();
	class region *clipEstablished;

	this->onScreen = TRUE;
	clipEstablished = (this)->GetClippingRegion( clipRgn);
	if(!this->do_renderupdate || do_fullupdate) {
	    struct rectangle r;

	    if(this->canvas) {
		free(this->canvas);
		this->canvas=NULL;
	    }
	    (this)->GetLogicalBounds( &r);
	    this->canvas = rectangle_Duplicate(&r);
	    DrawBorder(this, &r, this->canvas);
	}
	if (!haveFocus && click_to_draw) {
	    /* Do not draw.  Fill with a gray pattern. */
	    click_skip_update = TRUE;
	    SetTransferMode(graphic_COPY);
	    FillRect(canvas, GrayPattern(1, 2));
        } else if((IMAGE(this))->Data()) {
            if(canvas==NULL) {
                struct rectangle r;
                (this)->GetLogicalBounds( &r);
                canvas = rectangle_Duplicate(&r);
                DrawBorder(this, &r, canvas);
            }
	    if(GetDrawable()) {
		dimage.SetColormap(CurrentColormap());
	    }
	    class region *rgn1 = region::CreateEmptyRegion();
	    struct rectangle Pix, Src;
	    struct point Dest;

	    Pix.left=panx;
	    Pix.top=pany;
	    Pix.width=IMAGE(this)->Width();
	    Pix.height=IMAGE(this)->Height();
	    
	    (rgn1)->RectRegion( this->canvas);
	    if(clipEstablished)
		(rgn1)->IntersectRegion( clipRgn, rgn1);
	    (this)->SetClippingRegion( rgn1);
	    (this)->SetTransferMode( graphic_COPY);

	    point_SetX(&Dest, MAX(Pix.left, rectangle_Left(this->canvas)));
	    point_SetY(&Dest, MAX(Pix.top, rectangle_Top(this->canvas)));
	    rectangle_IntersectRect(&Src, &Pix, this->canvas);
	    rectangle_SetLeft(&Src, MAX(rectangle_Left(this->canvas) - Pix.left, 0));
	    rectangle_SetTop(&Src, MAX(rectangle_Top(this->canvas) - Pix.top, 0));

	    SENDIMAGE(this, &Dest, &Src);

	    rectangle_IntersectRect(&Pix, &Pix, this->canvas);
	    if(this->do_renderupdate) {
		if(!rectangle_IsEmptyRect(&Pix)) {
		    class region *rgn2 = region::CreateEmptyRegion();
		    (rgn2)->RectRegion( &Pix);
		    (rgn1)->XorRegion( rgn2, rgn1);

		    if(!rectangle_IsEmptyRect(this->lastPixRect)) {
			class region *lastPixRgn = region::CreateEmptyRegion();
			(lastPixRgn)->RectRegion( this->lastPixRect);
			(rgn1)->IntersectRegion( lastPixRgn, rgn1);
			delete lastPixRgn;
		    }

		    (this)->SetClippingRegion( rgn1);
		    CLEARRECT(this, this->canvas);
		    delete rgn2;
		}
		else
		    CLEARRECT(this, this->canvas);
            }
	    else
		::PostCursor(this, Cursor_Arrow);
	    *this->lastPixRect = Pix;
	    delete rgn1;
        }
	if(clipEstablished)
	    (this)->SetClippingRegion( clipRgn);
	else
	    (this)->ClearClippingRect();
	delete clipRgn;
    }
    else if(type == view_MoveNoRedraw) {
	(this)->RetractCursor( this->cursor);
	::PostCursor(this, Cursor_Arrow);
    }
    else if(type == view_Remove) {
	if(this->canvas) {
	    free(this->canvas);
	    this->canvas = NULL;
	}
	(this)->RetractCursor( this->cursor);
	this->onScreen = FALSE;
	if(dimage.IDDImage()) dimage.Release();
    }
    this->do_fullupdate = this->do_renderupdate = FALSE;
}

static void ConstrainPanning(imagev *iv) {
	if(iv->panx>0) iv->panx=0;
	if(iv->pany>0) iv->pany=0;
	long yextreme= -(IMAGE(iv)->Height() - iv->GetLogicalHeight()/2);
	long xextreme= -(IMAGE(iv)->Width() - iv->GetLogicalWidth()/2);
	if(iv->pany<yextreme) iv->pany=yextreme;
	if(iv->panx<xextreme) iv->panx=xextreme;
}

class view *
imagev::Hit( enum view_MouseAction  action, long  ix, long  iy, long  numberOfClicks)
{
    long iw, ih, px, py;
    if(canvas==NULL) return this;
    
    iw = this->canvas->width;
    ih = this->canvas->height;
    if (!this->haveFocus) {
	(this)->WantInputFocus( this);
    }

    switch(action) {
	case view_LeftDown:
	    break;
	case view_RightDown:
	    ::PostCursor(this, Cursor_CrossHairs);
	    this->rockx = ix;
	    this->rocky = iy;
	    if(this->panStyle == DISCRETE_PAN) {
		px = ix;
		py = iy;
		(this)->SetTransferMode( graphic_INVERT);
		(this)->MoveTo( 0, py);
		(this)->DrawLineTo( iw-1, py);
		(this)->MoveTo( px, 0);
		(this)->DrawLineTo( px, ih-1);
	    }
	    this->lastx = ix;
	    this->lasty = iy;
	    break;
	case view_LeftMovement:
	    break;
	case view_RightMovement:
	    if(this->panStyle == DISCRETE_PAN) {
		px =  ix;
		py = iy;
		if( px != this->lastx || py != this->lasty ) {
		    (this)->SetTransferMode( graphic_INVERT);
		    (this)->MoveTo( 0, this->lasty);
		    (this)->DrawLineTo( iw-1, this->lasty);
		    (this)->MoveTo( this->lastx, 0);
		    (this)->DrawLineTo( this->lastx, ih-1);
		    (this)->MoveTo( 0, py);
		    (this)->DrawLineTo( iw-1, py);
		    (this)->MoveTo( px, 0);
		    (this)->DrawLineTo( px, ih-1);
		    this->lastx = px;
		    this->lasty = py;
		}

		break;
	    }
	case view_RightUp:
	    if(this->panStyle == DISCRETE_PAN) {
		(this)->SetTransferMode( graphic_INVERT);
		(this)->MoveTo( 0, this->lasty);
		(this)->DrawLineTo( iw-1, this->lasty);
		(this)->MoveTo( this->lastx, 0);
		(this)->DrawLineTo( this->lastx, ih-1);
	    }
	    this->panx += (ix - this->rockx);
	    this->pany += (iy - this->rocky);
	    rockx=ix;
	    rocky=iy;
	    ConstrainPanning(this);
	    this->do_renderupdate = TRUE;
	    (this)->WantUpdate( this);
	    break;
	case view_LeftUp:
	default:
	    break;
    }
    return((class view *) this);
}

void
imagev::ReceiveInputFocus( )
{
    if(this->haveFocus == FALSE) {
	this->haveFocus = TRUE;
	(this)->view::ReceiveInputFocus();
	if(this->embedded) {
	    struct rectangle r, childrect;
	    (this)->GetLogicalBounds( &r);
	    DrawBorder(this, &r, &childrect);
	}
	if (click_to_draw && click_skip_update) {
	    click_skip_update = FALSE;
	    WantUpdate(this);
	}
    }
}

void
imagev::LoseInputFocus( )
{   
    if(this->haveFocus == TRUE) {
	this->haveFocus = FALSE;
	(this)->view::LoseInputFocus();
	if(this->isLinked && this->onScreen && this->embedded) {
	    struct rectangle r, childrect;
	    (this)->GetLogicalBounds( &r);
	    DrawBorder(this, &r, &childrect);
	}
    }
}

view_DSattributes 
imagev::DesiredSize( long  width , long  height, enum view_DSpass  pass, long  *desiredWidth , long  *desiredHeight )
{
    class image *image = orig;
    *desiredWidth = (image)->Width() > 0 ?
      ((image)->Width() ):
      image_STARTWIDTH;
    *desiredHeight = (image)->Height() > 0 ?
      (image)->Height() :
      image_STARTHEIGHT ;
    if(bordersize>0) {
	sbuttonv::SizeForBorder(this, sbuttonv_Enclosing, buttonprefs, haveFocus, *desiredWidth, *desiredHeight, desiredWidth, desiredHeight);
	sbuttonv::SizeForBorder(this, sbuttonv_Enclosing, buttonprefs, !haveFocus, *desiredWidth, *desiredHeight, desiredWidth, desiredHeight);
    }
    return(view_Fixed);
}

void
imagev::PostMenus( class menulist  *menulist )
{
    int mask = IMAGEV_DEFAULTMENUS;

    (this->menulistp)->ClearChain();
    if(this->orig->Data()) {
	mask |= IMAGEV_GOODIMAGEMENUS;
	if(strcmp((this->orig)->SaveFormatString(), "jpeg") == 0)
	    mask |= IMAGEV_JPEGFORMATMENUS;
	if((this)->DisplayClass() &
	   (graphic_PseudoColor | graphic_DirectColor | graphic_GrayScale)) {
	    mask |= IMAGEV_CANHAVEPRIVATECMAP;
	    if(this->havePrivateCmap) {
		mask |= IMAGEV_HASPRIVATECMAP;
		mask &= ~IMAGEV_HASNOTPRIVATECMAP;
	    }
	    else {
		mask |= IMAGEV_HASNOTPRIVATECMAP;
		mask &= ~IMAGEV_HASPRIVATECMAP;
	    }
	}
	else
	    mask &= ~IMAGEV_CANHAVEPRIVATECMAP;
    }
    (this->menulistp)->SetMask( mask);
    if(menulist) 
	(this->menulistp)->ChainAfterML( menulist, 0);
    (this)->view::PostMenus( this->menulistp);
}

void
imagev::ObservedChanged( class observable  *changed, long  value )
{
    if(changed==dimage.IDDImage() && value==observable_OBJECTCHANGED) {
        if(GetLogicalWidth()>0 && GetLogicalHeight()>0) {
            this->do_renderupdate = TRUE;
            (this)->WantUpdate( this);
        }
        this->orig->inited=FALSE;
    } else if((class image *) changed == this->orig)
	switch(value) {
	    case image_NEW:
		(this)->WantNewSize( this);
		/* reset the panning and scale for the new image */
		this->scale=imagev_NormScale;
		this->originx = (this->orig)->GetOriginX();
		this->originy = (this->orig)->GetOriginY();
		this->panx = this->originx;
		this->pany = this->originy;
		/* make the 1:1 scaled copy to insulate the original from any necessary dithering for display. */
		dimage=(image *)NULL;
		if(this->scaled==NULL) this->scaled=new image;
		if(this->scaled) {
		    this->scaled->Reset();
		    this->orig->Duplicate(this->scaled);
		    this->scaled->inited=FALSE;
		}
		dimage=IMAGE(this);
		dimage.AddObserver(this);
		this->do_renderupdate = TRUE;
		this->do_fullupdate = TRUE;
		this->orig->inited=FALSE;
		(this)->WantUpdate( this);
	    case observable_OBJECTCHANGED:
		break;
	    case observable_OBJECTDESTROYED:
		break;
	}
}

void
imagev::Update( )
{
    if(this->do_fullupdate) {
	struct rectangle r;
	(this)->GetVisualBounds( &r);
	CLEARRECT(this, &r);
	(this)->FullUpdate( view_FullRedraw, r.left, r.top, r.width, r.height);
    }
    else { /* do_renderupdate */
	(this)->FullUpdate( view_FullRedraw, 0, 0, 0, 0);
    }
}

static const char *
imageTypeName(enum image_fileType  type)
{
    static const char *objName = NULL;
    switch (type) {
	case any_imageType:
	    objName = "imageio";
	    break;
	case cmuwm_imageType:
	    objName = "cmuwm";
	    break;
	default:
	    fprintf(stderr, "imagev: no such image type.\n");
	    return(NULL);
    }
    return(objName);
}

static class image *
image_Import( const char  *filename, enum image_fileType  type )
{
    class image *image;
    const char *objName = NULL;
    objName = imageTypeName(type);
    if((image = (class image *) ATK::NewObject(objName))) {
	if((image)->Load(filename, NULL) < 0) {
	    delete image;
	    return NULL;
	}
    }
    return(image);
}

static int
image_Export( class image  *image, const char  *filename, enum image_fileType  type )
{
    int ret = -1;
    class image *newimage;
    const char *objName = NULL;
    objName = imageTypeName(type);
    if((newimage = (class image*) ATK::NewObject(objName))) {
	(image)->Duplicate( newimage);
	ret = (newimage)->WriteNative( NULL, filename);
	(newimage)->Destroy();
    }
    return(ret);
}

static void
Import_Cmd( class imagev  *self, enum image_fileType  type )
{
    class image *image = self->orig;
    char filename[MAXPATHLEN + 1];
    char message[MAXPATHLEN];
    class image *newimage;

    im::GetDirectory(filename);
    strcat(filename, "/");
    if(completion::GetFilename(self, "Import image file: ", filename, filename, sizeof(filename), FALSE, FALSE) == -1)
	return;
    POSTWAITCURSOR(self);
    if((newimage = image_Import(filename, type))) { /* OK */
	boolean havecmap= self->havePrivateCmap;
	if(havecmap) {
	    (self)->WantColormap( self , NULL);
	    (self)->WantUpdate( self);
	    if(self->privateCmap) {
		self->privateCmap->Destroy();
		self->privateCmap=NULL;
	    }
	}
	(image)->Reset();
	(newimage)->Duplicate(image);
	(newimage)->Destroy();
	(image)->Compress();
	if(havecmap) {
	    self->privateCmap = ((self)->GetIM())->CreateColormap();
	    (self)->WantColormap( self, &self->privateCmap);
	}
	(image)->NotifyObservers( image_NEW);
	sprintf(message, "Imported image file '%.*s'", MAXPATHLEN, filename);
	(image)->SetModified();
	(self)->PostMenus( NULL);
    }
    else { /* Error ! */
	sprintf(message, "Could not Import image file '%.*s'", MAXPATHLEN, filename);
    }
    RETRACTWAITCURSOR(self);
    message::DisplayString(self, 0, message);
}

static void
Export_Cmd( class imagev  *self, enum image_fileType  type )
{
    class image *image = self->orig;
    char filename[MAXPATHLEN + 1];
    char message[MAXPATHLEN];
    int result = 0;

    im::GetDirectory(filename);
    strcat(filename, "/");
    if(completion::GetFilename(self, "Export image file: ", filename, filename, sizeof(filename), FALSE, FALSE) == -1)
	return;
    POSTWAITCURSOR(self);
    if((result = image_Export(image, filename, type)) != 0) /* Error */
        sprintf(message, "Could not export image file '%.*s'", MAXPATHLEN, filename);
    else sprintf(message, "Wrote image to file '%.*s'", MAXPATHLEN, filename);
    RETRACTWAITCURSOR(self);
    message::DisplayString(self, 0, message);
}

static void
SaveAs( class imagev  *self, long  rock )
{
    char filename[MAXPATHLEN + 1];
    char message[MAXPATHLEN];
    int result = 0;

    im::GetDirectory(filename);
    strcat(filename, "/");
    if(completion::GetFilename(self, "Write image to file: ", filename, filename, sizeof(filename), FALSE, FALSE) == -1)
	return;
    POSTWAITCURSOR(self);
    result = WriteToFile(self, filename);
    RETRACTWAITCURSOR(self);
    if(result == 0)
	sprintf(message, "Wrote image to file '%.*s'", MAXPATHLEN, filename);
    else
	sprintf(message, "Could not write image to file '%.*s'", MAXPATHLEN, filename);
    message::DisplayString(self, 0, message);
}

static int 
WriteToFile( class imagev   *self, const char  *filename )
{ class image *image = self->orig;
  char realName[MAXPATHLEN], tempFilename[MAXPATHLEN];
  const char *originalFilename = NULL;
  char *endString;
  int closeCode, errorCode, originalMode, fd, counter = 1;
  FILE *outFile;
  struct stat statBuf;

  errorCode = 0;
  filetype::CanonicalizeFilename(realName, filename, sizeof(realName) - 1);
  filename = realName;
  if((access(filename, W_OK) < 0) && (errno == EACCES))
    return(-1);
  if(stat(filename, &statBuf) >= 0)
    originalMode = statBuf.st_mode & (~S_IFMT);
  else originalMode = 0666;
#ifndef USESHORTFILENAMES
  strcpy(tempFilename,filename);
  strcat(tempFilename,".NEW");
  endString = tempFilename + strlen(tempFilename);
  while(access(tempFilename,F_OK) >= 0) /* While the file exists. */
    sprintf(endString,".%d",counter++);
#else /* USESHORTFILENAMES */
  strcpy(tempFilename,filename);
  char *basename = strrchr(tempFilename,'/');
  if(!basename) basename = tempFilename;
  else basename++;
  if(strlen(basename) > 8) basename[8] = '\0';
  strcat(tempFilename,".NEW");
  endString = tempFilename + strlen(tempFilename);
  while(access(tempFilename,F_OK) >= 0 && counter < 10)
    sprintf(endString,".%d", counter++);
  if(counter == 10) return(-1);
#endif /* USESHORTFILENAMES */
  originalFilename = filename;
  filename = tempFilename;
  if((fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT,originalMode)) < 0
      || (outFile = fdopen(fd,"w")) == NULL)
    return(-1);
  (image)->Write( outFile, im::GetWriteID(), 0);
  fflush(outFile);
  if(ferror(outFile)) {
    fclose(outFile);
    errorCode = 0;
    closeCode = -1;
  }
  else {
    if((closeCode = close(fileno(outFile))) < 0) /* stdio can trash errno. */
      errorCode = errno; /* Protect it from the fclose below. */
    else if(originalFilename != NULL)
      if((closeCode = rename(filename, originalFilename)) < 0)
        errorCode = errno;
    fclose(outFile); /* Free stdio resources. */
    if(closeCode >= 0) { /* Reset readonly mode. */
      struct attributes attributes;

      attributes.next = NULL;
      attributes.key = "readonly";
      if(access(filename,W_OK) == -1 && errno == EACCES)
        attributes.value.integer = TRUE;
      else attributes.value.integer = FALSE;
      (image)->SetAttributes( &attributes);
      self->image_modified = (image)->GetModified();
    }
  }
  sprintf(tempFilename,"%s.CKP",filename);
  if(access(tempFilename,F_OK) >= 0)
    unlink(tempFilename);
  errno = errorCode;
  return(closeCode);
}

#ifdef THISWORKS
static void
Dither( class imagev  *self )
{
    class image *image = self->orig;

    POSTWAITCURSOR(self);
    (image)->Dither();
    (image)->SetModified();
    image->inited = FALSE;
    if(self->scaled) {
	(self->scaled)->Dither();
	(self->scaled)->SetModified();
	self->scaled->inited = FALSE;
    }
    RETRACTWAITCURSOR(self);
    self->do_renderupdate = TRUE;
    (self)->WantUpdate( self);
}

static void
Halftone( class imagev  *self )
{
    class image *image = self->orig;

    POSTWAITCURSOR(self);
    (image)->Halftone();
    (image)->SetModified();
    image->inited = FALSE;
    if(self->scaled) {
	(self->scaled)->Halftone();
	(self->scaled)->SetModified();
	self->scaled->inited = FALSE;
    }
    RETRACTWAITCURSOR(self);
    self->do_renderupdate = TRUE;
    (self)->WantUpdate( self);
}

static void
Reduce( class imagev  *self )
{
    class image *image = self->orig;
    const char prompt[] = "Reduce to how many colors?";
    char answer[MAXPATHLEN];
    int reduction = 0;

    if(message::AskForString(self, 0, prompt, NULL, answer, sizeof(answer)) == -1)
	return;
    reduction = atoi(answer);
    POSTWAITCURSOR(self);
    (image)->Reduce(reduction);
    (image)->SetModified();
    image->inited = FALSE;
    if(self->scaled){
	(self->scaled)->Reduce(reduction);
	(self->scaled)->SetModified();
	self->scaled->inited = FALSE;
    }
    RETRACTWAITCURSOR(self);
    self->do_renderupdate = TRUE;
    (self)->WantUpdate( self);
}

static void
Gray( class imagev  *self )
{
    class image *image = self->orig;

    POSTWAITCURSOR(self);
    (image)->Gray();
    (image)->SetModified();
    image->inited = FALSE;
    if(self->scaled) {
	(self->scaled)->Gray();
	(self->scaled)->SetModified();
	self->scaled->inited = FALSE;
    }
    RETRACTWAITCURSOR(self);
    self->do_renderupdate = TRUE;
    (self)->WantUpdate( self);
}

static void
Normalize( class imagev  *self )
{
    class image *image = self->orig;

    POSTWAITCURSOR(self);
    (image)->Normalize();
    (image)->SetModified();
    image->inited = FALSE;
    if(self->scaled) {
	(self->scaled)->Normalize();
	(self->scaled)->SetModified();
	self->scaled->inited = FALSE;
    }
    RETRACTWAITCURSOR(self);
    self->do_renderupdate = TRUE;
    (self)->WantUpdate( self);
}

static void
Brighten( class imagev  *self )
{
    class image *image = self->orig;
    const char prompt[] = "Brighten percent?";
    char answer[MAXPATHLEN];
    unsigned int brighten = 0;

    if(message::AskForString(self, 0, prompt, NULL, answer, sizeof(answer)) == -1)
	return;
    brighten = (unsigned int) abs(atoi(answer));
    POSTWAITCURSOR(self);
    (image)->Brighten(brighten);
    (image)->SetModified();
    image->inited = FALSE;
    if(self->scaled) {
	(self->scaled)->Brighten(brighten);
	(self->scaled)->SetModified();
	self->scaled->inited = FALSE;
    }
    RETRACTWAITCURSOR(self);
    self->do_renderupdate = TRUE;
    (self)->WantUpdate( self);
}

static void
GammaCorrect( class imagev  *self )
{
    class image *image = self->orig;
    const char prompt[] = "Gamma level?";
    char answer[MAXPATHLEN];
    float gamma = 0;

    if(message::AskForString(self, 0, prompt, NULL, answer, sizeof(answer)) == -1)
	return;
    gamma = atof(answer);
    POSTWAITCURSOR(self);
    (image)->GammaCorrect(gamma);
    (image)->SetModified();
    image->inited = FALSE;
    if(self->scaled) {
	(self->scaled)->GammaCorrect(gamma);
	(self->scaled)->SetModified();
	self->scaled->inited = FALSE;
    }
    RETRACTWAITCURSOR(self);
    self->do_renderupdate = TRUE;
    (self)->WantUpdate( self);
}

static void
ScaleToFit( class imagev  *self )
{
    class image *image = self->orig;
    struct rectangle r,lb;
    float imageWidth, imageHeight;
    float screenWidth, screenHeight;
    unsigned int w, h;

    (self)->GetLogicalBounds(&lb);
    sbuttonv::DrawRectBorder(self, &lb, self->buttonprefs, self->haveFocus, FALSE, &r, FALSE);
    sbuttonv::DrawRectBorder(self, &r, self->buttonprefs, !self->haveFocus, FALSE, &r, FALSE);
    POSTWAITCURSOR(self);
    imageWidth = (float) (image)->Width();
    imageHeight = (float) (image)->Height();
    screenWidth = (float) rectangle_Width(&r);
    screenHeight = (float) rectangle_Height(&r);
    w = (unsigned int)((screenWidth * 100)/imageWidth);
    h = (unsigned int)((screenHeight * 100)/imageHeight);
    (image)->Zoom(w, h);
    (image)->SetModified();
    image->inited = FALSE;
    self->dimage=(class image *)NULL;
    if(self->scaled) {
	(self->scaled)->Destroy();
	self->scaled = NULL;
    }
    self->dimage=IMAGE(self);
    self->dimage.AddObserver(self);
    RETRACTWAITCURSOR(self);
    self->do_renderupdate = TRUE;
    (self)->WantUpdate(self);
}
#endif

extern int writePS(class imagev *v, FILE *tmpFile, int *wpts, int *hpts, int toplevel);

void
imagev::Print( FILE  *f, const char  *process, const char  *final, int  toplevel )
{
    FILE *tmpFile;
    if((tmpFile = tmpfile())) {
	int wpts, hpts;
	char buf[BUFSIZ], buf1[BUFSIZ];
	const char *prefix;
	writePS(this, tmpFile, &wpts, &hpts, toplevel);
	if(!fflush(tmpFile) && !fseek(tmpFile, 0, SEEK_SET)) {
	    if(strcmp(process, "troff") == 0) {
		if(toplevel)
		    texttroff::BeginDoc(f);
		texttroff::BeginPS(f, wpts, hpts);
		prefix = "\\!  ";
	    }
	    else if(strcmp(final, "troff") == 0)
		prefix = "\\!  ";
	    else prefix = "";
	    while(fgets(buf, sizeof(buf), tmpFile)) {
		sprintf(buf1, "%s%s", prefix, buf);
		fputs(buf1, f);
	    }
	    fclose(tmpFile);
	    if (strcmp(process, "troff") == 0) {
		texttroff::EndPS(f, wpts, hpts);
		if (toplevel)
		    texttroff::EndDoc(f);
	    }
	}
    }
}

void imagev::PrintPSDoc(FILE *outfile, long pagew, long pageh)
{
    int wpts, hpts;
    boolean dopage;
    int margin = 36; /* half inch */

    dopage = print::PSNewPage(print_UsualPageNumbering);
    if (dopage) {
	fprintf(outfile, "%d %d moveto  %ld %d lineto %ld %ld lineto\n %d %ld lineto closepath clip newpath\n", margin, margin, pagew-margin, margin, pagew-margin, pageh-margin, margin, pageh-margin);
	fprintf(outfile, "%d %d translate\n", margin, margin);
	writePS(this, outfile, &wpts, &hpts, imagev_REGISTERED_POSTSCRIPT);
    }
}

void *imagev::GetPSPrintInterface(const char *printtype)
{
    if (!strcmp(printtype, "generic"))
	return (void *)this;

    return NULL;
}

void imagev::PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect)
{
    int wpts, hpts;
    writePS(this, outfile, &wpts, &hpts, imagev_REGISTERED_POSTSCRIPT);
}

void imagev::DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight)
{
    this->view::DesiredPrintSize(width, height, pass, desiredwidth, desiredheight);
}


	boolean 
imagev::Gifify(const char *filename, long *pmaxw, long *pmaxh, 
			struct rectangle *visrect) {
		// (is supposed to pay attention to input values of pmax*)
	image *src = (image *)GetDataObject();	// get the image object
	imageio out;
	src->Duplicate(&out);			// Duplicate into a gif object
	out.SetSaveFormatString("gif"); // <sigh> it *is* Gifify, after all
	if (pmaxw) *pmaxw = out.Width();	// report size
	if (pmaxh) *pmaxh = out.Height();
	return (out.WriteNative(NULL, filename) == 0);	// success if 0
}


#if 0
static void
InfoCmd( class imagev  *self )
{
    class im *im = im::Create(NULL);
    class frame *framep = new frame;
    class colormap **cmap = (self)->CurrentColormap();
    class buffer *buffer = buffer::Create("Current Colormap", NULL, NULL, *cmap);
    if(im && framep && buffer) {
	(framep)->SetCommandEnable( TRUE);
	(im)->SetInheritedColormap( cmap);
	(im)->SetView( framep);
	(framep)->PostDefaultHandler("message", (framep)->WantHandler("message"));
	(framep)->SetBuffer( buffer, TRUE);
    }
    else {
	if(im) (im)->Destroy();
	if(framep) (framep)->Destroy();
	if(buffer) (buffer)->Destroy();
    }
}
#endif

static void
ShowTrue( class imagev  *self )
{
    self->privateCmap = ((self)->GetIM())->CreateColormap();
    (self)->WantColormap( self, &self->privateCmap);
    (self)->WantUpdate( self);
}

void
imagev::ReceiveColormap( class colormap  *cmap )
{
    class image *image = IMAGE(this);
    image->inited = FALSE;
    this->do_fullupdate = TRUE;
    this->havePrivateCmap = TRUE;
    (this)->WantUpdate( this);
    (this)->PostMenus( NULL);
}

static void
ShowFixed( class imagev  *self )
{

    (self)->WantColormap( self , NULL);
    (self)->WantUpdate( self);
    if(self->privateCmap) {
	self->privateCmap->Destroy();
	self->privateCmap=NULL;
    }
}

void
imagev::LoseColormap( class colormap  *cmap )
{
    class image *image = IMAGE(this);

    image->inited = FALSE;
    this->do_fullupdate = TRUE;
    this->havePrivateCmap = FALSE;
    (this)->WantUpdate( this);
    (this)->PostMenus( NULL);
}

static void
Write_Postscript( class imagev  *self )
{
    char filename[MAXPATHLEN + 1];
    char message[MAXPATHLEN];
    FILE *f;
    
    im::GetDirectory(filename);
    strcat(filename, "/");
    if(completion::GetFilename(self, "Postscript file: ", filename, filename, sizeof(filename), FALSE, FALSE) == -1)
	return;
    POSTWAITCURSOR(self);
    if((f = fopen(filename, "w"))) {
	writePS(self, f, 0, 0, imagev_PURE_POSTSCRIPT);
	fclose(f);
	RETRACTWAITCURSOR(self);
	sprintf(message, "Wrote Postscript file %s.\n", filename);
    }
    else {
	RETRACTWAITCURSOR(self);
	sprintf(message, "Couldn't open %s for writing.\n", filename);
    }
    message::DisplayString(self, 0, message);
}


void imagevInterface::Absolute(long totalx, long x, long totaly, long y) {
    if(iv==NULL) return;
    long dx=-iv->panx, dy=-iv->pany;
    
    if(totalx) {
	dx=(long)((double)x*IMAGE(iv)->Width())/totalx;
    }

    if(totaly) {
	dy=(long)((double)y*IMAGE(iv)->Height())/totaly;
    }
    if(dx!=iv->panx || dy!=iv->pany) {
	iv->panx=-dx;
	iv->pany=-dy;
	ConstrainPanning(iv);
	iv->do_renderupdate = TRUE;
	iv->WantUpdate( iv);
    }
}

void imagevInterface::ScreenDelta(long dx, long dy) {
    if(iv==NULL) return;
    if(dx!=0 || dy!=0) {
	iv->panx+=dx;
	iv->pany+=dy;
	ConstrainPanning(iv);
	iv->do_renderupdate = TRUE;
	iv->WantUpdate( iv);
    }      
}

void imagevInterface::Shift(scroll_Direction dir) {
    if(iv==NULL) return;
    long px=0, py=0;
    switch(dir) {
	case scroll_Up:
	    py++;
	    break;
	case scroll_Down:
	    py--;
	    break;
	case scroll_Left:
	    px++;
	    break;
	case scroll_Right:
	    px--;
	    break;
	case scroll_None:
	    break;
    }
    if(px!=0 || py!=0) {
	iv->panx+=px;
	iv->pany+=py;
	ConstrainPanning(iv);
	iv->do_renderupdate=TRUE;
	iv->WantUpdate(iv);
    }
}

void imagevInterface::Extreme(scroll_Direction dir) {
    if(iv==NULL) return;
    long px=iv->panx, py=iv->pany;
    switch(dir) {
	case scroll_Up:
	    py=0;
	    break;
	case scroll_Down:
	    py=-(IMAGE(iv)->Height()) + iv->GetLogicalHeight()/2;
	    break;
	case scroll_Left:
	    px=0;
	    break;
	case scroll_Right:
	    px=-(IMAGE(iv)->Width()) + iv->GetLogicalWidth()/2;
	    break;
        case scroll_None:
	    break;
    }
    if(px!=iv->panx || py!=iv->pany) {
	iv->panx=px;
	iv->pany=py;
	iv->do_renderupdate=TRUE;
	iv->WantUpdate(iv);
    }	
}

void imagevInterface::UpdateRegions(class scroll &sc) {
    long xend=-iv->panx + iv->GetLogicalWidth();
    if(xend>IMAGE(iv)->Width()) xend=IMAGE(iv)->Width();
    long yend=-iv->pany + iv->GetLogicalHeight();
    if(yend>IMAGE(iv)->Height()) yend=IMAGE(iv)->Height();
    if(sc.Region(scroll_Elevator)) sc.Region(scroll_Elevator)->SetRanges(IMAGE(iv)->Width(), -iv->panx, xend, IMAGE(iv)->Height(), -iv->pany, yend);
}

ScrollInterface *imagev::GetScrollInterface() {
    return new imagevInterface(this);
}

class view *
imagev::GetApplicationLayer( )
{
    class scroll *view;
    view = scroll::CreateScroller(this, scroll_LEFT | scroll_BOTTOM, environ::GetProfile("imagescrollclass"));

    this->embedded = FALSE;
    this->bordersize = 0;
    return (class view *) view;
}

void imagev::DeleteApplicationLayer(class view *scrollbar) {
    scrollbar->Destroy();
}

void
imagev::SetDataObject( class dataobject  *dataobj )
{
    this->orig = (class image *) dataobj;
    (this)->view::SetDataObject(dataobj);
    /* reset the panning and scale for the new image */
    this->originx = (this->orig)->GetOriginX();
    this->originy = (this->orig)->GetOriginY();
    this->panx = this->originx;
    this->pany = this->originy;
    /* make the 1:1 scaled copy to insulate the original from any necessary dithering for display. */
    dimage=(class image *)NULL;
    if(this->scaled==NULL) this->scaled=new image;
    if(this->scaled) {
	this->scaled->Reset();
	this->orig->Duplicate(this->scaled);
	this->scaled->inited=FALSE;
    }
    this->orig->inited = FALSE;
    dimage.RemoveObserver(this);
    dimage=IMAGE(this);
    dimage.AddObserver(this);
}

void
imagev::UnlinkTree( )
{
    if(dimage.IDDImage()) dimage.Release();
    this->onScreen = this->isLinked = FALSE;
    (this)->view::UnlinkTree();
}

void
imagev::UnlinkNotification( class view  *unlinkedTree )
{
    this->onScreen = this->isLinked = FALSE;
    (this)->view::UnlinkNotification( unlinkedTree);
}

void
imagev::LinkTree( class view  *parent )
{
    (this)->view::LinkTree( parent);
    if(parent && (this)->GetIM()) {
    }
    else {
	if(dimage.IDDImage()) dimage.Release();
	this->isLinked = this->onScreen = FALSE;
    }
}

static void
ReadCmd( class imagev  *self )
{
    class image *imagep = self->orig;
    char filename[MAXPATHLEN + 1];
    char message[MAXPATHLEN];
    FILE *in;

    im::GetDirectory(filename);
    strcat(filename, "/");
    if(completion::GetFilename(self, "Image file: ", filename, filename, sizeof(filename), FALSE, FALSE) == -1)
	return;
    POSTWAITCURSOR(self);
    if((in = fopen(filename, "r"))) { /* OK */
	long status;
	class image *new_c = new image;
	if(new_c) {
	    if((status = (new_c)->Read( in, 0)) == dataobject_NOREADERROR) {
		(imagep)->Reset();
		(new_c)->Duplicate(imagep);
		self->dimage=(class image *)NULL;
		if(self->scaled) {
		    (self->scaled)->Destroy();
		    self->scaled = NULL;
		}
		sprintf(message, "Read image file '%.*s'", MAXPATHLEN, filename);
		(imagep)->NotifyObservers(image_NEW);
		(imagep)->SetModified();
		(self)->PostMenus(NULL);
	    }
	    else {
		switch(status) {
		    case dataobject_NOTATKDATASTREAM:
			sprintf(message, "Contents of '%.*s' not ATK image data", MAXPATHLEN, filename);
			break;
		    case dataobject_PREMATUREEOF:
			sprintf(message, "Contents of '%.*s' truncated", MAXPATHLEN, filename);
			break;
		    case dataobject_OBJECTCREATIONFAILED:
			sprintf(message, "Failed to create a necessary object");
			break;
		    case dataobject_BADFORMAT:
			sprintf(message, "Contents of '%.*s' in bad format", MAXPATHLEN, filename);
			break;
		    default:
			sprintf(message, "Unknown error encountered while reading '%.*s'", MAXPATHLEN, filename);
			break;
		}
	    }
	    (new_c)->Destroy();
	}
	else { /* Error ! */
	 sprintf(message, "Couldn't allocate memory for new image.");
	}
    }
    else { /* Error ! */
     sprintf(message, "Could not open image file '%.*s'", MAXPATHLEN, filename);
    }
    RETRACTWAITCURSOR(self);
    message::DisplayString(self, 0, message);
}

static void
ChangeZoomCmd( class imagev  *self, long  rock )
{
    long newscale, fact;
    struct rectangle logicalRect;

    if(rock<-256 || rock>255) {
	char *crock=(char *)rock;
	rock=atoi(crock);
    }
    if (rock == 0 && self->scale == imagev_NormScale)
	return;
    POSTWAITCURSOR(self);
    (self)->GetLogicalBounds(&logicalRect);

    long left=-self->panx;
    long top=-self->pany;
    if(rock < 0) 
	newscale = self->scale / 2;
    else if(rock > 0)
	newscale = self->scale * 2;
    else newscale = imagev_NormScale;

    if(newscale < 1) newscale = 1;
    else if (newscale > 1024) newscale = 1024;

    if(newscale != self->scale) {
	left*=newscale;
	top*=newscale;
	left/=self->scale;
	top/=self->scale;
	self->dimage=(class image *)NULL;
	if(self->scaled) {
	    (self->scaled)->Destroy();
	    self->scaled = NULL;
	}
 	if(newscale != imagev_NormScale)  {
	    fact = (newscale * 100)/imagev_NormScale;
	    self->scaled = (self->orig)->Zoom(fact, fact);
	    self->scaled->inited = FALSE;
	}
	else {
	    if(self->scaled==NULL) self->scaled=new image;
	    if(self->scaled) {
		self->scaled->Reset();
		self->orig->Duplicate(self->scaled);
		self->scaled->inited=FALSE;
	    }

	    self->orig->inited = FALSE;
	}
	
	self->panx = -(left);
	self->pany = -(top);
	self->scale = newscale;
	self->do_renderupdate = TRUE;
	self->dimage=IMAGE(self);
	self->dimage.AddObserver(self);
	(self)->WantUpdate(self);
    }
    RETRACTWAITCURSOR(self);
}

static void
PanToOriginCmd( class imagev  *self, long  rock )
{
    if (self->panx == self->originx && self->pany == self->originy)
	return;

    self->panx = self->originx;
    self->pany = self->originy;
    self->do_renderupdate = TRUE;
    (self)->WantUpdate( self);
}

void
imagev::PostKeyState(class keystate  *ks)
{
    class keystate *new_c;
    new_c = (this->keystatep)->AddAfter( ks);
    (this)->view::PostKeyState( new_c);
}

static void
SetSaveQuality( class imagev  *self, long  rock )
{
    class image *image = self->orig;
    const char prompt[] = "Save quality [5-95]: ";
    char answer[MAXPATHLEN];
    int q;

    sprintf(answer, "%d", (image)->GetJPEGSaveQuality());
    if(message::AskForString(self, 0, prompt, answer, answer, sizeof(answer)) == -1)
	return;
    q = atoi(answer);
    (image)->SetJPEGSaveQuality( q);
    sprintf(answer, "Save quality set to %d", q);
    message::DisplayString(self, 0, answer);
}

static void
SetSaveFormat( class imagev *self, long  rock )
{
    class image *image = self->orig;
    const char prompt[] = "New Image Save-Format: ";
    char response[100], *choice;
    static const char * const choices[] = {"PNG", "GIF", "JPEG", NULL};
    long result = 0;

    if(message::MultipleChoiceQuestion(self, 100, prompt, 0, &result, choices, NULL) == -1)
	return;
    if((choice = strdup(choices[result]))) {
	(image)->SetSaveFormatString(choice);    
	if(self->scaled)
	    (self->scaled)->SetSaveFormatString(choice);
	sprintf(response, "Save-format set to %s", choices[result]);
	message::DisplayString(self, 0, response);
	(self)->PostMenus(NULL);
	free(choice);
    }
}
