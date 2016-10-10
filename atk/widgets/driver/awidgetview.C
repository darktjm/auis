/***************************************************
 *	Copyright Carnegie Mellon, 1995 - All Rights Reserved
 *	For full copyright information see:'andrew/doc/COPYRITE'
 ***************************************************
 */

/* awidgetview.c	
	The view module for the widget dataobject
 */

#include <andrewos.h>
ATK_IMPL("awidgetview.H")

#include <graphic.H>
#include <view.H>
#include <fontdesc.H>

#include <menulist.H>
#include <keymap.H>
#include <keystate.H>
#include <bind.H>
#include <matte.H>
#include <im.H>
#include <print.H>
#include <rect.h>
#include <aaction.H>

#include <awidget.H>
#include <awidgetview.H>
#include <ashadow.H>
#include <texttroff.H>
#include <physical.h>

ASlot_NAME(selected);
ASlot_NAME(printImage);
ASlot_NAME(designFont);

ATK_CLASS(fontdesc);
ATK_CLASS(matte);


static char  debug;      /* This debug switch is toggled with ESC-^D-D */
#define DEBUG(s) {if (debug) {printf s ; fflush(stdout);}}
#define ENTER(r) DEBUG(("Enter %s(0x%p)\n", Stringize(r), this))
#define LEAVE(r) DEBUG(("Leave %s(0x%p)\n", Stringize(r), this))

	
ATKdefineRegistry(AWidgetView, view,
                  AWidgetView::InitializeClass);
ATKdefineRegistryNoInit(AFormulaViewAct,AFormulaAct);

static void ToggleDebug(class AWidgetView  *self, long  rock);


/* - - - - - - - - - - - - - - - - - - *\
 *
 *	Supporting procedures
 *	
\* - - - - - - - - - - - - - - - - - - */


/* - - - - - - - - - - - - - - - - - - *\
 *	
 *	User Interface 
 *	Routines called from keystrokes or menu
\* - - - - - - - - - - - - - - - - - - */

	static void
ToggleDebug(class AWidgetView  *, long  )  {
	debug = ! debug;
	printf("debug is now %d\n", debug);  fflush (stdout);
}


/*  Menus and Keymaps
	The menus and keymaps are initialized to those of
	EmbeddedMenus and EmbeddedKeymap in InitializeObject.
	The ApplicationMenus and ApplicationKeymap get
	added to the menus and keymap in GetApplicationLayer

	Debugging can be toggled with   ESC-^D -D

*/
static class menulist  *EmbeddedMenus, *ApplicationMenus;
static class keymap  *EmbeddedKeymap, *ApplicationKeymap;

static struct bind_Description EmbeddedBindings[]={
    {"AWidgetView-invert-debug", "\033\004W", // ESC - ^D -W
			0, 0, 0, 0, (proctable_fptr)ToggleDebug,
			"Toggle the AWidgetView debug flag", NULL},
	NULL
};
static struct bind_Description ApplicationBindings[]={
	NULL
};

/* - - - - - - - - - - - - - - - - - - *\
 *	
 *	Constructors / Destructors
 *	
\* - - - - - - - - - - - - - - - - - - */

	boolean
AWidgetView::InitializeClass()  {
	ASlot::Init();
	EmbeddedMenus = new menulist;
	ApplicationMenus = new menulist;
	EmbeddedKeymap = new keymap;
	ApplicationKeymap = new keymap;
	if (EmbeddedMenus == NULL 
			|| ApplicationMenus == NULL
			|| EmbeddedKeymap == NULL 
			|| ApplicationKeymap == NULL)
		return FALSE;
	bind::BindList(EmbeddedBindings, 
				EmbeddedKeymap, EmbeddedMenus,
				&AWidgetView_ATKregistry_ );
	bind::BindList(ApplicationBindings, 
				ApplicationKeymap, ApplicationMenus, 
				&AWidgetView_ATKregistry_ );

	return TRUE;
}

AWidgetView::AWidgetView()  {
	ATKinit;

	border = NULL;
	Menus = (EmbeddedMenus)->DuplicateML( this);
	Keystate = keystate::Create(this, EmbeddedKeymap);

	leftDown = rightDown = FALSE;

	embedded = TRUE;
	HasInputFocus = FALSE;
	updateRequested = FALSE;

	defaultfont = fontdesc::Create("andy", fontdesc_Plain, 12);

        mouseFocus=NULL;
        
	THROWONFAILURE(Menus && Keystate 
			&& defaultfont);
}

AWidgetView::~AWidgetView()  {
	delete Menus;
	delete Keystate;
        delete border;
        if(mouseFocus) mouseFocus->RemoveObserver(this);
}

/* - - - - - - - - - - - - - - - - - - *\
 *	
 *	Overrides
 *	
\* - - - - - - - - - - - - - - - - - - */

	void
AWidgetView::LinkTree(view *parent)  {
	view::LinkTree(parent);
}

	void 
AWidgetView::InitChildren() {
}

	void
AWidgetView::ObservedChanged(class observable  *changed, long  status)  {
	if (status == observable::OBJECTDESTROYED) 
        {
            if(changed==mouseFocus) mouseFocus=NULL;
        }
	else if (changed==GetDataObject() && status == awidget_DATACHANGED) {
		WantUpdate(this);
        }
}

	class view *
AWidgetView::GetApplicationLayer()  {
	this->embedded = FALSE;
	(this->Menus)->ChainBeforeML( 
			(ApplicationMenus)->DuplicateML(this), 
			(long)NULL);
	(keystate::Create(this,ApplicationKeymap))
			->AddBefore( this->Keystate);
	return this;
}

	void 
AWidgetView::WantUpdate(view *requestor) {
	if (requestor == this) {
		if (updateRequested) return;
		updateRequested = TRUE;
	}
	this->view::WantUpdate(requestor);
}

void AWidgetView::PostMenus(class menulist  *ml)
{

    if(Menus) {
	if(ml) (Menus)->ChainAfterML( ml, (long)ml);
	(this)->view::PostMenus( Menus);
    } else (this)->view::PostMenus( ml);
}

void AWidgetView::PostKeyState(class keystate  *ks)
{
    if(Keystate) {
	(Keystate)->AddBefore( ks);
	(this)->view::PostKeyState( Keystate);
    } else (this)->view::PostKeyState( ks);
}

/* DON'T DO screen operations in ReceiveInputFocus */

	void 
AWidgetView::ReceiveInputFocus()  {
	ENTER(awidgetv_ReceiveInputFocus);
	PostMenus(NULL);
        PostKeyState(NULL);
	HasInputFocus = TRUE;
	// Do not bother `viewer` with knowledge of input focus
	//  xxx needs to change when allow in-place editing of image
	LEAVE(awidgetv_ReceiveInputFocus);
}

/* DON'T DO screen operations in LoseInputFocus,
	but DO remove any defined cursor */

	void 
AWidgetView::LoseInputFocus() {
	ENTER(AWidgetView_LoseInputFocus);
	HasInputFocus = FALSE;
	/* menus and keystate are deactivated by parent */
	LEAVE(AWidgetView_LoseInputFocus);
}

	static void
UpdateBorder(AWidgetView *self, enum view::UpdateType type) {
	AWidget *dobj = (class AWidget *)self->GetDataObject();
	AShadow *border = self->GetBorder();
	struct rectangle r;
	self->GetLogicalBounds(&r);
	if(border && (type==view::FullRedraw 
	    || type==view::LastPartialRedraw)) {
		border->SetHighlighted(self->GetHasInputFocus());
		border->SetSensitive(dobj->sensitive);
		ASlot *selected = dobj->Get(slot_selected);
		if (selected) 
			border->SetSelected((boolean)*selected);
		border->DrawBorder(self, &r);
	}
	if (self->parent->IsType(class_matte)) {
		matte *ma = (class matte *)self->parent;
		if (((boolean)ma->resizing==0)  
					!=  ((boolean)dobj->resizable==0))
			ma->SetResizing(0);  // arg is ignored
	}
}

/* FullUpdate(self, type, left, top, width, height)
	Update and redraw part or all of the image.
	(The last four arguments apply when 'type' is 
	view::PartialRedraw;  they specify which part to update.)
*/
	void 
AWidgetView::FullUpdate(enum view::UpdateType   type, 
			long left, long top, long width, long height)  {
	DEBUG(("FullUpdate(%d, %ld, %ld, %ld, %ld)\n", 
			type, left, top, width, height));

	if (border) UpdateBorder(this, type);
	updateRequested = FALSE;
	LEAVE(AWidgetView_FullUpdate);
}

	void 
AWidgetView::Update() {
	ENTER(AWidgetView_Update);
	if (border) UpdateBorder(this, view::FullRedraw);
	updateRequested = FALSE;
	LEAVE(AWidgetView_Update);
}

void AWidgetView::SetDataObject(class dataobject *d) {
	view::SetDataObject(d);
	AWidget *w = (AWidget *)d;
	border = AShadow::Create(this, w);
	double units = w->units;
	if (units == 0.0) uxscale = uyscale = 1.0;
	else  {
		uxscale = 7200.0 / GetHorizontalResolution() / units;
		uyscale = 7200.0 / GetVerticalResolution() / units;
	}
}

	class view *
AWidgetView::Hit(enum view::MouseAction action, 
			long x, long y, long num_clicks) {
	DEBUG(("Hit at (%ld, %ld) type %d\n", x, y, action));
	AWidget *dobj = (class AWidget *)GetDataObject();
	if (action == view::NoMouseEvent)
		return (class view *)this;

	if ( ! (boolean)dobj->sensitive) return this;

        view *nf=NULL;
        if(mouseFocus) {
            graphic *drawable=GetDrawable();
            long mx=physical_LogicalXToGlobalX(drawable, x);
            long my=physical_LogicalYToGlobalY(drawable, y);
            nf=(mouseFocus)->Hit( action,                          physical_GlobalXToLogicalX( (mouseFocus)->GetDrawable(), mx),                          physical_GlobalYToLogicalY( (mouseFocus)->GetDrawable(), my),                          num_clicks);
            if(action==view::LeftUp || action==view::RightUp || action==view::UpMovement) nf=NULL;
        } else {
            // see if hit is within the rectangle
            rectangle r, interior;
            (this)->GetLogicalBounds( &r); // find rectangle bounds
            boolean within = 
              (x >= rectangle_Left(&r)  
               &&  x < rectangle_Right(&r)
               && y >= rectangle_Top(&r)   
               &&  y < rectangle_Bottom(&r));
            if(border) border->InteriorRect(&r,&interior);
            else interior=r;

            long xunits = ToUnitX(x-interior.left), yunits = ToUnitY(y-interior.top);

            avalueflex destview;

            // take appropriate action
            switch (action) {
                case view::LeftDown:
                    if (leftDown) 
                        // lost an up.  invent one
                        Hit(view::LeftUp, x, y, 1);
                    leftDown = TRUE;
                    (dobj->mouseLeftDown)(dobj, avalueflex() | this
                                          | xunits | yunits | (long)within | (long)action 
                                          | num_clicks, destview);
                    break;
                case view::LeftMovement:
                    if ( ! leftDown) 
                        // lost a down.  invent one
                        Hit(view::LeftDown, x, y, 0);
                    leftDown = TRUE;
                    (dobj->mouseLeftMove)(dobj, avalueflex() | this
                                          | xunits | yunits | (long)within | (long)action 
                                          | num_clicks, destview);
                    break;
                case view::LeftUp:
                    if ( ! leftDown) 
                        // lost a down.  invent one
                        Hit(view::LeftDown, x, y, 0);
                    leftDown = FALSE;
                    (dobj->mouseLeftUp)(dobj, avalueflex() | this
                                        | xunits | yunits | (long)within | (long)action 
                                        | num_clicks, destview);
                    break;
                case view::RightDown:
                    if (rightDown) 
                        // lost an up.  invent one
                        Hit(view::RightUp, x, y, 1);
                    rightDown = TRUE;
                    (dobj->mouseRightDown)(dobj, avalueflex() | this
                                           | xunits | yunits | (long)within | (long)action 
                                           | num_clicks, destview);
                    break;
                case view::RightMovement:
                    if ( ! rightDown) 
                        // lost a down.  invent one
                        Hit(view::RightDown, x, y, 0);
                    rightDown = TRUE;
                    (dobj->mouseRightMove)(dobj, avalueflex() | this
                                           | xunits | yunits | (long)within | (long)action 
                                           | num_clicks, destview);
                    break;
                case view::RightUp:
                    if ( ! rightDown) 
                        // lost a down.  invent one
                        Hit(view::RightDown, x, y, 0);
                    rightDown = FALSE;
                    (dobj->mouseRightUp)(dobj, avalueflex() | this
                                         | xunits | yunits | (long)within | (long)action 
                                         | num_clicks, destview);
                    break;
                case view::LeftFileDrop:
                case view::MiddleFileDrop:
                case view::RightFileDrop:
                    // implement filedrops someday... XXX
                    break;
		case view::UpMovement: /* tjm - unused */
		case view::NoMouseEvent: /* tjm - unused */
		    break;
            }

            (dobj->mouseAny)(dobj, avalueflex() | this
                             | xunits | yunits | (long)within | (long)action 
                             | num_clicks, destview);

            if(destview.GetN()>0) {
                nf=(view *)destview[destview.GetN()-1].ATKObject();
            } else nf=this;
        }
        if(nf && nf->GetIM()!=GetIM()) {
            if(mouseFocus) mouseFocus->RemoveObserver(this);
            mouseFocus=NULL;
            return nf;
        } else if(nf!=mouseFocus && (action==view::LeftDown || action==view::RightDown || nf==NULL)) {
            if(mouseFocus) mouseFocus->RemoveObserver(this);
            if(nf!=this) {
                if(nf) nf->AddObserver(this);
                mouseFocus=nf;
            } else mouseFocus=NULL;
        }
	LEAVE(AWidgetView_Hit);
        return mouseFocus?mouseFocus:this;	// where to send subsequent hits
}

	view::DSattributes
AWidgetView::DesiredSize(long  /* width */, long  /* height */, 
			enum view::DSpass  /* pass */, 
			long  *dWidth, long  *dHeight)  {
	class AWidget *dobj = (class AWidget *)GetDataObject();
	aaction *computeViewSize;
	double fscale;

	if ( ! dobj->GetResourceClass())
		dobj->FetchInitialResources(
				atom_def("WidgetSelector"));

	fontdesc *areafont = defaultfont;  // xxx
			// xxx need to get area font from view parent

	fontdesc *priorfont;
	fontdesc *designfont = dobj->designFont;

	if (designfont == NULLFONTDESC) {
		aaction *computeDesignSize 
				= dobj->computeDesignSize;
                if (computeDesignSize) {
                {
                    avalueflex dummy;
			dobj->designFont = areafont;
			(*computeDesignSize)(dobj, 
					avalueflex() | this | areafont, 
                                             dummy);
                }
			// cache results if appropriate
			priorfont = dobj->priorViewFont;
			if (priorfont == areafont) {
				dobj->desiredWidth = &dobj->designWidth;
				dobj->desiredHeight 
						= &dobj->designHeight;
				if (dobj->priorViewFont.IsDefault())	// guck
					dobj->priorViewFont = areafont;
				goto done;
			}
		}
		else {
			// xxx error: neither a designFont 
			//		nor a computeDesignFont()
			fprintf(stderr, "AWidgetView: %s for %s\n",
				"no designFont or computeDesignFont",
				dobj->GetResourceClass()->Name());
			dobj->designFont = areafont;  // xxx kludge
		}
	}

	// is view size cached?
	priorfont = dobj->priorViewFont;
	if (! dobj->priorViewFont.IsDefault() 
				&& priorfont == areafont) 
		goto done;

	computeViewSize = dobj->computeViewSize;
        if (computeViewSize) {
        {
            avalueflex dummy;
            (*computeViewSize)(dobj, avalueflex() | this | areafont,
                               dummy);
        }
		goto done;
	}

	// default viewsize computation
	fscale = 1.0 * areafont->GetFontSize()
			/ ((fontdesc *)dobj->designFont)->GetFontSize();
	dobj->desiredWidth = (long)((long)dobj->designWidth
				* (double)dobj->scaleWidth * fscale);
	dobj->desiredHeight = (long)((long)dobj->designHeight
				* (double)dobj->scaleHeight * fscale);
	dobj->priorViewFont = areafont;

	done:
	*dWidth = ToPixX(dobj->desiredWidth);
	*dHeight = ToPixY(dobj->desiredHeight);
	DEBUG(("Leave DesiredSize: %ld x %ld\n", 
				*dWidth, *dHeight));
	return view::Fixed;
}

	void 
AWidgetView::GetOrigin(long /* width */, long height, 
		long *originX, long *originY) {
// xxx needs to consider baseline
	*originX = 0;
	*originY = height-10;	// -4 leaves it too high;  -14 is a little low
}


	boolean
AWidgetView::RecSearch(struct SearchPattern * , boolean ) {
	return FALSE;
}

	boolean
AWidgetView::RecSrchResume(struct SearchPattern *) {
	return FALSE;
}

	boolean
AWidgetView::RecSrchReplace(class dataobject *, long , long ) {
	return FALSE;
}

	void
AWidgetView::RecSrchExpose(const struct rectangle &, 
			struct rectangle &)  {
	
}


/* # # # # # # # # # # # # # # 
 *	PRINTING	
 *  # # # # # # # # # # # # #  */

static const char defaultImage[] =
"	newpath 0 0 moveto 0 19 lineto 19 19  lineto \n"
"	19 0 lineto closepath 1 setlinewidth stroke \n"
"	newpath 0 19 moveto 19 0 lineto 1 setlinewidth stroke \n";



/* AWidgetView::Print is the method that is used by the old printing mechanism. */
	void
AWidgetView::Print(FILE   *file, const char    *processor, const char    *, 
		boolean    topLevel)  {
	class AWidget *dobj = (class AWidget *)GetDataObject();
	ASlot *pimage = dobj->Get(slot_printImage);
	const char *imagePS = NULL;

	if (pimage == NULL) {}
	else if (pimage->IsType(ASlot::stringslotreg))
		imagePS = *pimage;
	else if (pimage->IsType(ASlot::functionslotreg))
		imagePS = (*pimage)(this);
		// pimage function could set nominalPrintXxxx

	if (imagePS == NULL) 
		imagePS = defaultImage;

	// printed image dimensions
	long pheight = dobj->nominalPrintHeight;
	long hpts = this->GetLogicalHeight();
	if (pheight < 0)
		pheight = (hpts > 0) ? hpts : 19;
	long pwidth = dobj->nominalPrintWidth;
	long wpts = this->GetLogicalWidth();
	if (pwidth < 0) 
		pwidth = (wpts > 0) ? wpts : 19;

	const char *prefix;	// prefix for troff stream
	if (strcmp(processor, "troff") == 0) {
		/* output to troff */
		if (topLevel)
			/* take care of initial troff stream */
			texttroff::BeginDoc(file);
		/*  Put macro to interface to postscript */
		texttroff::BeginPS(file, pwidth, pheight);
		prefix = "\\!  ";
	}
	else prefix = "";

	fprintf(file, "%s  gsave\n", prefix);

	//  XXX  We should restrict pwidth and pheight to (say) 576x720
	//	We could do so by scaling the image 
	// fprintf(file, "%s  %g %g scale\n", prefix, 
	//	scaledwidth/pwidth, scaledheight/pheight);

	// add prefix at front of every line in imagePS
	const char *p = imagePS;
	while (*p) {
		const char *newline = strchr(p, '\n');
		if ( ! newline) newline = p+strlen(p);  // pt at \0
		if (newline-p > 75) {
			// xxx should break line up xxx
		}
		fprintf(file, "%s  %.*s\n", prefix, (int)(newline-p), p);
		if ( ! *newline)  break;
		p = newline + 1;
	}
	fprintf(file, "%s  grestore\n", prefix);

	if (strcmp(processor, "troff") == 0){
		texttroff::EndPS(file, wpts, hpts);
		if (topLevel)
			texttroff::EndDoc(file);
	}
}

// GetPSPrintInterface, PrintPSDoc and PrintPSRect 
//		are used by the new printing mechanism. 
	void *
AWidgetView::GetPSPrintInterface(const char *printtype)  {
    if (strcmp(printtype, "generic") != 0)
	return (void *)this;

    return NULL;
}

	void
AWidgetView::PrintPSDoc(FILE *outfile, long pagew, long pageh)  {
    boolean printthis;
    printthis = print::PSNewPage(print_UsualPageNumbering);
    if (printthis) {
	// if the user wants this page printed, print it. 
	this->PrintPSRect(outfile, pagew, pageh, NULL);
    }
}

	void
AWidgetView::PrintPSRect(FILE *file, long logwidth, long logheight, 
			struct rectangle *)  {
	AWidget *dobj = (AWidget *)GetDataObject();
	ASlot *pimage = dobj->Get(slot_printImage);
	const char *imagePS = NULL;


	if (pimage == NULL) {}
	else if (pimage->IsType(ASlot::stringslotreg))
		imagePS = *pimage;
	else if (pimage->IsType(ASlot::functionslotreg))
		imagePS = (*pimage)(this);
		// pimage function could set nominalPrintXxxx

	if (imagePS == NULL) {
		// use figureview to display image
		return;
	}

	fprintf(file, "gsave\n");

	// scale pwidth and pheight to logwidth and logheight
	// xxx Should do something about visrectangle
	fprintf(file, "%g %g scale\n", 
			logwidth/(double)dobj->nominalPrintWidth,
			logheight/(double)dobj->nominalPrintHeight);
	fprintf(file, "%s ", imagePS);
	fprintf(file, "grestore\n");
}


// width of the string in terms of data object's
// designFont and units slots
	long 
AWidgetView::StringWidth(const char *str) {
	fontdesc *font= NULL;
	AWidget *dobj = (AWidget *)GetDataObject();
	if (dobj) 
		font = (fontdesc *)dobj->designFont;
	if ( ! font) 
		font = defaultfont;
	return ToUnitX(font->StringSize(GetDrawable(), 
				(char *)str, NULL, NULL));
}

// distance above and below baseline for character c
// distance are computed in terms of the slots
//    designFont and units in the view's data object
// Returns total height
	long 
AWidgetView::CharVertical(const char c,
			long *ascent, long *descent) {
	fontdesc *font= NULL;
	AWidget *dobj = (AWidget *)GetDataObject();
	if (dobj) 
		font = (fontdesc *)dobj->designFont;
	if ( ! font) 
		font = defaultfont;
	fontdesc_charInfo info;
	font->CharSummary(GetDrawable(), c, &info);
	if (ascent) *ascent = ToUnitY(info.yOriginOffset);
	if (descent) 
		*descent = ToUnitY(info.height - info.yOriginOffset);
	return ToUnitY(info.height);
}
AFormulaViewAct::AFormulaViewAct(view *v, aaction *a, aaction *w) : AFormulaAct((ATK *)(v->GetDataObject()), a, w), bv((ATK *)v) {
}

AFormulaViewAct::~AFormulaViewAct() {
}

void AFormulaViewAct::Evaluate() {
    if(formula) {
	avalueflex out;
	(*formula)(object, avalueflex()|bv, out);
    }
}

void AFormulaViewAct::WantUpdate(ADependerKey key) {
    if(wantUpdateFunction) {
	avalueflex out;
	(*wantUpdateFunction)(object, avalueflex()|bv|(long)key, out);
    }
}

/* 
 *   $Log: awidgetview.C,v $
 * Revision 1.32  1996/11/07  00:11:22  robr
 * Updated to pass along the key in a formula's WantUpdate aaction.
 * BUG
 *
 * Revision 1.31  1996/11/03  15:33:51  robr
 * Fixed to avoid bogus WantUpdate calls from ObservedChanged.
 * BUG
 *
 * Revision 1.30  1996/10/31  22:47:08  robr
 * Fixed to deal with descendent views which post a popup menu
 * on a mouse hit.
 * BUG
 *
 * Revision 1.29  1996/10/29  22:47:59  robr
 * Added support for the mouse hit functions to return the hit view.
 * If they do this the widgetview's mouse hit functions will *NOT* see
 * any further hits.  (Existing mouse functions will continue to work
 * as before since the new behavior is only triggered if the
 * mouse function returns something in its return value list.)
 * BUG
 *
 * Revision 1.28  1996/10/07  21:16:10  robr
 * Fixed to use PostMenus and PostKeystates.
 * The rule is that EVERY view which implements
 * any user callable proctable functions MUST implement
 * PostMenus and PostKeyState.
 * ReceiveInputFocus must ALWAYS call PostMenus & PostKeyState
 * with NULL arguments.  (The incoming argument is for derived
 * classes and children to use in passing in their keystates.)
 * BUG
 *
 * Revision 1.27  1996/10/01  20:23:10  robr
 * Adjust mouse hit coordinates so that (0,0) is the upper left of the
 * interior of any border being displayed.
 * BUG.
 *
 * Revision 1.26  1996/09/28  17:39:23  robr
 * Sigh... workaround a hp C++ bug
 * .BUG
 *
 * Revision 1.25  1996/09/25  19:32:44  robr
 * Fixed missing registry information for some of the formula classes.
 * Added AFormulaViewAct in awidgetview to support the widget convention of calling
 * aactions with the view as the first variable arg.
 * BUG/FEATURE
 *
 * Revision 1.24  1996/09/24  19:41:56  wjh
 * convert to use To{Pix,Unit}{X,Y}
 * considerable reformatting
 * revise for new scheme of not caching areafont
 *
 * Revision 1.23  1996/06/14  17:02:44  robr
 * Removed bogus WantUpdate calls in Receive/LoseInputFocus.
 * let the derived classes do that if desired.
 * BUG
 *
 * Revision 1.22  1996/05/20  17:04:25  robr
 * fixed warnings.
 * BUG
 *
 * Revision 1.21  1996/05/20  15:25:03  robr
 * /tmp/msg
 *
 * Revision 1.20  1996/05/13  17:14:00  wjh
 *  add kludge to eliminate the resizing cursors posted by the matte; added slots selected and designFont; added defaultfont; use slot_selected, if any, to set border's Selected flag; fix value of uxscale/uyscale for the case of units!=1; call mouseAny for any mouse hit; use IsDefault to determine if slot has been given a value; implement StringWidth and CharVertical
 *
 * Revision 1.19  1996/05/08  17:21:37  wjh
 * changed to wend widget as object to mouse hit aactions
 * removed spurious WantInputFocus
 *
 * Revision 1.18  1996/05/07  20:36:01  wjh
 * update border in both Update and FullUpdate; \
 * 	use function UpdateBorder; \
 * switch from DrawRectBorder to DrawBorder; \
 * use AShadow::Create; \
 * use units in communicating to widget; \
 * set so right button always gives IntputFocus; \
 * 	(probably wrong); \
 * revise for new size computation and units; \
 * printing draws a rectangle if there is no other option; \
 * moved future plans to Design document
 *
 * Revision 1.17  1996/05/02  02:46:56  wjh
 * truncated log; rewrote DesiredSize for new slot  definitions
 *
 * Revision 1.16  1996/04/26  17:12:15  robr
 * Removed figure support to afwidget.
 *
 * Revision 1.15  1996/04/22  19:31:09  robr
 * Took out bogus WantUpdate call in ::Hit.
 * If a hit caused a change which needs a WantUpdate it should
 * call WantUpdate itself.
 * BUG
  . . .
 * Revision 1.2  1995/12/10  15:03:16  wjh
 * working version
 *
 * Revision 1.1  1995/09/23  15:18:47  wjh
 * Initial revision
 * 
 * Revision 1.0  95/09/05  16:23:23  wjh
 * Copied from /afs/cs/misc/atk/@sys/alpha/lib/null
 */
