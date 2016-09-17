/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/* sys/types.h in AIX PS2 defines "struct label", causing a type name clash.
  Avoid this by temporarily redefining "label" to be something else in the preprocessor. */
#define label gezornenplatz
#include <andrewos.h> /* strings.h */
#undef label
#include <stdio.h>
#include <util.h>
#include <ctype.h>

#include <rect.h>

#include <atom.H>	/* For all the strings we pass to the value */
#include <atomlist.H>	/* ditto */
#include <bind.H>	/* Binding menus */
#include <bpair.H>	/* lpairs without lines in between */
#include <buffer.H>	/* For when we create the window */
#include <checkv.H>	/* Checklist widget */
#include <dataobject.H>	/* General observer type things */
#include <environ.H>	/* For changing print options */
#include <graphic.H>	/* Drawing boxes and stuff */
#include <fontdesc.H>	/* for face codes */
#include <frame.H>	/* For creating the new window in */
#include <im.H>	/* For the new window */
#include <event.H>	/* For im_SetInteractionEvent() to DestroyWindow */
#include <keymap.H>	/* Shortcuts for the menus */
#include <keystate.H>	/* Needed, since we are playing with keymaps */
#include <label.H>	/* Titles, etc */
#include <labelview.H>	/* ditto */
#include <lpair.H>	/* General layout */
#include <menulist.H>	/* Menus */
#include <message.H>	/* General messages */
#include <strinput.H>	/* For text input - printername */

#ifdef POPTS_USE_SUITE
#include <suite.H>	/* For the buttons */
#else /* POPTS_USE_SUITE */
#include <pushbutton.H>
#include <pshbttnv.H>
#endif /* POPTS_USE_SUITE */

#include <text.H>	/* General text */
#include <value.H>	/* To help out with checklists */
#include <view.H>	/* Our parent */

#include <printopts.H>

static boolean  debug = FALSE;
#define DEBUG(s) {if (debug) {printf s ; fflush(stdout);}}
#define ENTER(r) DEBUG(("Enter %s(0x%lx)\n", Stringize(r)))
#define LEAVE(r) DEBUG(("Leave %s(0x%lx)\n", Stringize(r)))

/* Keep the following in sync with the values
 defined in txttroff.c  (we should consolidate
 them into a .h file) */

#define ENDNOTESONLY FALSE
#define	CONTENTSBYDEFAULT FALSE
#define AUTOENUMERATEDEFAULT FALSE
#define DUPLEXBYDEFAULT FALSE

#define BASERESOLUTION 72

#define SCALEWIDTH(v, x) (x*((class view *)(v))->GetHorizontalResolution()/BASERESOLUTION)
#define SCALEHEIGHT(v, y) (y*((class view *)(v))->GetVerticalResolution()/BASERESOLUTION)
#define DIMENSION_X SCALEWIDTH(vp_IM, 350)
#define DIMENSION_Y SCALEHEIGHT(vp_IM, 99)
#define INSET_X 1
#define INSET_Y 1

ATKdefineRegistry(printopts, view, printopts::InitializeClass);
#ifndef POPTS_USE_SUITE
static struct buttonList *printopts_MakeButton(struct buttonList  *blist, char  *text, observable_fptr function, class view  *object);
static struct lplist *printopts_MakeLpair(struct lplist  *lpl);
#endif /* POPTS_USE_SUITE */
void doValueChange(class printopts  *self, class value  *observed, long  rock, long  rock2);
void PrinterSet(class printopts  *self, long  rock);
void printopts_Nop(class printopts  *self, long  rock);
void ResetValues(class printopts  *self);
#ifdef POPTS_USE_SUITE
class view * Control(class printopts  *self, class suite  *suite, MISSING_ARGUMENT_TYPE item, long  object, enum view_MouseAction  action, long  x , long  y , long  clicks);
#endif /* POPTS_USE_SUITE */
void MenuDone(class printopts  *self, long  rock);
void MenuCancel(class printopts  *self, long  rock);
void ApplyValues(class printopts  *self, long  rock);
void CreateWindow(class view  *vp, long  rock);
void DestroyWindow( class printopts  *self );

static int CheckType = 0L;
static int NumberOfChecks;
static class menulist *Menus;
static class keymap *Keys;
static struct Option options[] = {
#ifdef LANDSCAPE
    { "Print in	Landscape mode", "doc-landscape",
    "Portrait", PROFILETYPESWITCH, "no", "yes", NULL, 0 },
#endif

    { "Print table of contents with documents", "doc-contents",
    "PrintContents", PROFILETYPESWITCH, "yes", "no", NULL, CONTENTSBYDEFAULT },

    { "Enumerate contents automatically", "doc-enumerate",
    "AutoEnumerate", PROFILETYPESWITCH, NULL, NULL, NULL, AUTOENUMERATEDEFAULT },

#ifdef POPTSINDEX
    { "Print index with documents", "doc-index", 
    "PrintIndex", PROFILETYPESWITCH, "yes", "no", NULL, 0 },

#endif /* POPTSINDEX */
    { "Print footnotes at end of documents", "doc-footnotes",
    "Endnotes", PROFILETYPEEXISTS, "yes", NULL, NULL, ENDNOTESONLY },

    { "Swap right and left headers on even pages", "doc-duplex", "Duplex", PROFILETYPESWITCH, "yes", "no", NULL, DUPLEXBYDEFAULT },

    { NULL, NULL, NULL, PROFILETYPENONE, 0, 0, NULL, 0 }
};












static struct bind_Description MenuOptions[] = {
    {"printopts-post-window", NULL, 0, "Set Print Options", 0, 0, (proctable_fptr)CreateWindow, "Set options for printing of document", NULL},
    NULL
};
static struct bind_Description PrivateOptions[] = {
    {"printopts-nop", "\012", 0, NULL, 0, 0, (proctable_fptr)printopts_Nop, NULL},
    {"printopts-nop", "\015", 0, NULL, 0, 0, (proctable_fptr)printopts_Nop, NULL},
    {"printopts-apply", NULL, 0, "Apply Values", 0, 0, (proctable_fptr)ApplyValues, NULL},
    {"printopts-done", NULL, 0, "Done", 0, 0, (proctable_fptr)MenuDone, NULL},
    {"printopts-cancel", NULL, 0, "Cancel", 0, 0, (proctable_fptr)MenuCancel, NULL},
    NULL
};

/* To communicate resources to valueviews, the strings should be formed
 * into atomlists. This is done once for the major strings, to save cycles
 */
static class atomlist *AL_bodyfont;
static class atomlist *AL_bodyfont_size;
static class atomlist *AL_label;
static class atomlist *AL_checktype;
static class atomlist *AL_borderwidth;
static class atom *A_long;
static class atom *A_string;
#define InternAtoms ( \
   AL_bodyfont = atomlist::StringToAtomlist("bodyfont") ,\
   AL_bodyfont_size = atomlist::StringToAtomlist("bodyfont-size") ,\
   AL_label = atomlist::StringToAtomlist("label") ,\
   AL_checktype = atomlist::StringToAtomlist("checktype") ,\
   AL_borderwidth = atomlist::StringToAtomlist("border-width"), \
   A_long = atom::Intern("long") ,\
   A_string = atom::Intern("string") )


#ifdef POPTS_USE_SUITE
/* THis is not nice, unfortunately there are not many nice ways of doing this */
#define ApplyCode 1
#define DoneCode 2
#define CancelCode 3
static suite_Specification apply[] = {
    suite_ItemCaption("Apply"),
    suite_ItemDatum(ApplyCode),
    NULL
};
static suite_Specification done[] = {
    suite_ItemCaption("Done"),
    suite_ItemDatum(DoneCode),
    NULL
};
static suite_Specification cancel[] = {
    suite_ItemCaption("Cancel"),
    suite_ItemDatum(CancelCode),
    NULL
};

suite_Specification control_spec[] = {
#if 0
    suite_Item(apply),
#endif
    suite_Item(done),
    suite_Item(cancel),
    suite_ItemCaptionFontName( "AndySans10b" ),
    suite_GutterSize( 5 ),
    suite_BorderSize( 0 ),
    suite_ItemOrder( suite_ColumnMajor ),
    suite_Arrangement( suite_Matrix ),
    suite_SelectionMode( suite_Exclusive ),
    suite_HitHandler(Control),
    NULL
};

#else /* POPTS_USE_SUITE */
/* Structure is linked list of buttons */   
  
static const char * const printopts_bNames[] = 
{ "Done", "Cancel", NULL };

static observable_fptr printopts_bFuncs[]=
{ (observable_fptr)MenuDone, (observable_fptr)MenuCancel, NULL };


static struct buttonList *printopts_MakeButton(struct buttonList  *blist, char  *text, observable_fptr function, class view  *object) 
/* Creates a new button and with given attributes
   returns [newbutton::blist]
 */
{
    struct buttonList *button = (struct buttonList *)     malloc(sizeof(struct buttonList));

    button->butt  = new pushbutton;
    button->buttv = new pushbuttonview;
    (button->butt)->SetStyle( 1);
    (button->butt)->SetText( text);
    (button->buttv)->SetDataObject( button->butt);
    (button->buttv)->AddRecipient(   atom::Intern("buttonpushed"), object, function, 0L);
      
    button->next = blist;
    return button;
}

static struct lplist *printopts_MakeLpair(struct lplist  *lpl)
/* Creates a new lpair and returns [newlpair::lplist]
 */
{
    struct lplist *lpair = (struct lplist *) malloc(sizeof(struct lplist));
    
    lpair->lp = new lpair;
    lpair->next = lpl;
    return lpair;
}


#endif /* POPTS_USE_SUITE */

boolean
printopts::InitializeClass()
{
    struct ATKregistryEntry  *viewClassinfo;
   
    for (NumberOfChecks = 0; options[NumberOfChecks].name != NULL; NumberOfChecks++);

    InternAtoms;

    /* We want one or two of our own local functions */
    Menus = new menulist;
    Keys  = new keymap;
    bind::BindList(PrivateOptions, Keys, Menus, &printopts_ATKregistry_ );

    /* We want our function bound to view, not to self */
    viewClassinfo = ATK::LoadClass("view");
    if (viewClassinfo != NULL) {
        bind::BindList(MenuOptions, NULL, NULL, viewClassinfo);
        return TRUE;
    }
    return FALSE;
}

#define CheckHeight 18

printopts::printopts()
{
	ATKinit;

    long i;
    class view *dummy;
    class view *oldchecklist;
#ifndef POPTS_USE_SUITE
    struct lplist     *lplist = NULL;
    struct buttonList *blist = NULL;
#endif /* POPTS_USE_SUITE */
    class lpair *lp;
    char *t;


    this->embedded = TRUE;
    this->OnScreen = FALSE;
    this->ResourcesPosted = FALSE;
    this->keys  = keystate::Create(this, Keys);
    this->menus = (Menus)->DuplicateML( this);

    this->values = (int *)calloc (NumberOfChecks, sizeof(int));

    /* copyin the options from static storage */
    /* Don't free the strings, they're static */
    for (i = 0; i < NumberOfChecks; i++)
	this->values[i]	= options[i].value;

    CheckType = environ::GetProfileInt("CheckType", 1);

    this->printername = new strinput;
    (this->printername)->SetPrompt( "Name of printer: ");
    if((t = environ::Get("LPDEST")) == NULL)
    	if((t = environ::Get("PRINTER")) == NULL)
	    t = environ::GetProfile("spooldir");
    if (t)
	strncpy(this->pnamevalue, t, 80);
    else
	strcpy(this->pnamevalue, "");
    (this->printername)->SetInput( this->pnamevalue);

#ifdef POPTS_USE_SUITE
    this->suite = suite::Create(control_spec, this);
#endif /* POPTS_USE_SUITE */

    dummy = new view;

    /* Construct the checklist */
    this->checklist = (class bpair *) dummy;
    for (i = 0; i < NumberOfChecks; i++) {
	this->check[i].obj  = new value;
	this->check[i].view = new checkv;
	/* The rest of the setting up of the options takes place in FullUpdate */
	oldchecklist = (class view *)this->checklist;
	this->checklist = new bpair; /* it accumulates */
	(this->checklist)->VFixed( oldchecklist,
		     this->check[i].view, CheckHeight, 0);
    }
    lp = new lpair;
    (lp)->VFixed( this->printername, this->checklist, NumberOfChecks*CheckHeight + 2, FALSE); /* Doesn't account for resolution of device.... */
    this->image = new lpair;

#ifdef POPTS_USE_SUITE
    this->image = (this->image)->HSplit( lp, this->suite, 20, FALSE);
#else /* POPTS_USE_SUITE */

    /* Yucky code follows! Make the sets of buttons */
    this->blist = NULL;
    this->lplist = NULL;
    for ( i=0 ; printopts_bNames[i] != NULL ; i++ ) {
	this->blist = printopts_MakeButton(this->blist, printopts_bNames[i], printopts_bFuncs[i], (class view *) this);
	if (printopts_bNames[i+1] != NULL) 
	    this->lplist = printopts_MakeLpair(this->lplist);
    } /* for - initializing buttons */

    /*
      * The below lines glue the buttons into their lpairs, using the
      * linked lists blist and lplist
      * The lpairs are split so as each button is the same size
      * Making all buttons the same size is done by the code in the while loop, making
	  * the n'th button be 1/n fraction of the size of the panel. 
  * i.e. The lpair_HSplit splits it 1/(n+1) to button and n/(n+1) to the previous lpair.
  * The entire panel is finally placed into self->buttons
  */

    blist = this->blist;
    lplist = this->lplist;

    (lplist->lp)->VSplit( blist->next->buttv, blist->buttv, 50, FALSE);
    blist  = blist->next->next;
    i = 2;
    while (blist != NULL) {
	(lplist->next->lp)->VSplit( blist->buttv, lplist->lp, (long int) 100*i/(i+1), FALSE);
	lplist = lplist->next;
	blist  = blist->next;
	i++;
    } 

    this->image = (this->image)->HSplit( lp, lplist->lp, 20, FALSE);

#endif /* POPTS_USE_SUITE */
    this->pname=lp;
    THROWONFAILURE( TRUE);
}

printopts::~printopts()
{
	ATKinit;

    class bpair *checklist, *nextlist;
    class lpair *leftSide = (class lpair*) (this->image)->GetNth(0);
    int j;
#ifndef POPTS_USE_SUITE
    struct buttonList *blist, *next_blist;
    struct lplist *lplist, *next_lplist;
#endif /* POPTS_USE_SUITE */
    class checkv *v;
    class value *o;

    if(this->keys) delete this->keys;
    if(this->menus) delete this->menus;
    if(this->values) free(this->values);

    (this->image)->UnlinkTree();
    (this->image)->Destroy();
    (leftSide)->Destroy(); /* Contains strinput and skewed bpair of checkv's. */
/*
  lpair_Destroy(rightSide); /* Contains lplist->lp; We deal with that below.
*/

    (this->printername)->Destroy();

    checklist = this->checklist;
    for (j = NumberOfChecks - 1; j >= 0; j--) {
	o = this->check[j].obj;
	v = this->check[j].view;
	if(j > 0) {
	    nextlist = (class bpair*) (checklist)->GetNth( 0);
	    (checklist)->Destroy();
	    checklist = nextlist;
	}
	else {
	    class view *dummy = (checklist)->GetNth( 0);
	    (checklist)->Destroy();
	    (dummy)->Destroy();
	}
	(v)->Destroy();
/* Disable any callback that may be generated when the value object is destroyed. */
	(o)->RemoveCallBack( this, (value_fptr)doValueChange);
	if (options[j].func)
	    (o)->RemoveCallBack( this, (value_fptr)options[j].func);
	(o)->RemoveCallBackObserver( this);
	(o)->Destroy();
    }

#ifdef POPTS_USE_SUITE
    (this->suite)->Destroy();
#else
    lplist = this->lplist;
    blist = this->blist;
    while(lplist) {
	next_lplist = lplist->next;
/* We have to set these because we're walking the skewed tree from the bottom. */
	lplist->lp->obj[0] = NULL;
	lplist->lp->obj[1] = NULL;
	(lplist->lp)->Destroy();
	free(lplist);
	lplist = next_lplist;
    }
    while(blist) {
	next_blist = blist->next;
	(blist->buttv)->Destroy();
	(blist->butt)->Destroy();
	free(blist);
	blist = next_blist;
    }
#endif /* POPTS_USE_SUITE */
}

class view *
printopts::GetApplicationLayer()
{
    class frame *frame;

    this->embedded = FALSE;
    frame = new frame;
    (frame)->SetView( this);
    (frame)->PostDefaultHandler( "message", (frame)->WantHandler( "message"));

    return frame;
}

void
printopts::PostMenus(class menulist  *ml)
{
    (this)->view::PostMenus( this->menus);
}

void printopts::PostKeyState(class keystate  *ks)
/* Want to add our own keybindings into the chain that gets passed to us */
{
    this->keys->next = NULL;
    (this->keys)->AddBefore( ks); 
    (this)->view::PostKeyState( this->keys);
}

void 
printopts::FullUpdate(enum view_UpdateType   type, long   left , long   top , long   width , long   height)
{
    struct rectangle r;
    this->OnScreen = (type != view_Remove);
    (this)->GetLogicalBounds( &r);
    DEBUG(("FullUpdate type %d  redraw (%d,%d,%d,%d) within (%d,%d,%d,%d)\n", 
	    type, left, top, width, height, r.left, r.top, r.width, r.height));

    /* Now that we are updating, the views exist, so we can set up those views */
    if (!this->ResourcesPosted) {
	int i;
	class checkv *v;
	class value *o;

	/* Checklist */
	ResetValues(this);
	for (i = 0; i < NumberOfChecks; i++) {
	    class atomlist *cname=atomlist::StringToAtomlist(options[i].name);
	    v = this->check[i].view;
	    o = this->check[i].obj;
	    (v)->SetName(cname );
	    delete cname;
	    (v)->PostResource( AL_checktype, A_long, CheckType);
	    (v)->PostResource( AL_borderwidth, A_long, 0L);
	    (v)->PostResource( AL_label, A_string, (long)options[i].label);
	    (v)->PostResource( AL_bodyfont, A_string, (long)"andy");
	    (v)->PostResource( AL_bodyfont_size, A_long, 12);
	    (o)->AddCallBackObserver( this, (value_fptr)doValueChange, i);
	    if (options[i].func)
		(o)->AddCallBackObserver( this, (value_fptr)options[i].func, i);
	    (o)->SetValue( this->values[i]);
	    (v)->SetDataObject( o);
	}
    }
    this->ResourcesPosted = TRUE;

    if (type == view_FullRedraw && this->embedded && this->OnScreen) {
	(this)->SetTransferMode( graphic_COPY);
	(this)->MoveTo( 0, 0);
	(this)->DrawLineTo( r.width-INSET_X, 0);
	(this)->DrawLineTo( r.width-INSET_X, r.height-INSET_Y);
	(this)->DrawLineTo( 0, r.height-INSET_Y);
	(this)->DrawLineTo( 0, 0);

	r.top+=INSET_Y, r.left+=INSET_X, r.height-=(2*INSET_Y), r.width-=(2*INSET_X);
    }

    DEBUG(("	Drawable at 0x%lx\n", (this)->GetDrawable()));

    if (type != view_PartialRedraw 
	 && type != view_LastPartialRedraw) {
	(this->image)->LinkTree( this);
	(this->image)->InsertView( this, &r);
    }
    (this->image)->FullUpdate(  type, 0, 0, r.width, r.height);
}


void 
printopts::Update( )
{
    (this->image)->Update();
}


class view *
printopts::Hit(enum view_MouseAction   action, long   x , long   y , long   num_clicks)
{
    (this->image)->Hit( action, x-INSET_X, y-INSET_Y, num_clicks);
    (this)->WantInputFocus( this->printername);
    return (class view*) this;
}

view_DSattributes
printopts::DesiredSize( long  width, long  height, enum view_DSpass  pass, 
			long  *desiredWidth, long  *desiredHeight ) 
{
    *desiredWidth = 550,  *desiredHeight = 322;
    DEBUG(("Desired Size %d x %d\n", *desiredWidth, *desiredHeight));
    return view_Fixed;
}

void
printopts::Print( FILE    *file, const char  	  *processor, const char  	  *format, boolean  	 level )
{
    /* never print anything */
}


void
doValueChange(class printopts  *self, class value  *observed, long  rock, long  rock2)
{
    self->values[rock] = (observed)->GetValue();
}

void
PrinterSet(class printopts  *self, long  rock)
{
    strcpy(self->pnamevalue, (self->printername)->GetInput( 80));
}

/* We use this to override newline self-insert. */
void
printopts_Nop(class printopts  *self, long  rock)
{
}

void
ResetValues(class printopts  *self)
{
    int i;
    char *t;

    for (i = 0; i < NumberOfChecks; i++) {
	if (options[i].func == NULL && options[i].env) {
	    if ((t = environ::Get(options[i].env)) == NULL) {
		switch (options[i].profiletype) {
		    case PROFILETYPENONE:
			break;
		    case PROFILETYPEEXISTS:
			if(environ::GetProfile (options[i].env) != NULL)
			    self->values[i] = 1;
			else
			    self->values[i] = 0;
			break;
		    case PROFILETYPESWITCH:
			self->values[i] = environ::GetProfileSwitch (options[i].env, options[i].value);
			break;
		}
	    } else {
		if (options[i].envOn && strcmp(t, options[i].envOn) == 0) 
		    self->values[i] = 1;
		else
		    self->values[i] = 0;
	    }
	}
    }
    t = environ::Get("LPDEST");
    if (t == NULL)
	t = environ::Get("PRINTER");
    if (t)
	strncpy(self->pnamevalue, t, 80);
    else
	strcpy(self->pnamevalue, "");
    (self->printername)->SetInput( self->pnamevalue);
}

#ifdef POPTS_USE_SUITE
class view *
Control(class printopts  *self, class suite  *suite, MISSING_ARGUMENT_TYPE item, long  object, enum view_MouseAction  action, long  x , long  y , long  clicks)
{
    if ((action == view_LeftUp) || (action == view_RightUp)) {
	if(item && (object == suite_ItemObject)) {
	    switch((suite)->ItemAttribute( item, suite_ItemData(0))) {
		case ApplyCode:
		    ApplyValues(self, 0);
		    break;
		case DoneCode:
		    MenuDone(self, 0);
		    break;
		case CancelCode:
		    MenuCancel(self, 0);
		    break;
	    }
	}
    }
    (suite)->Reset( suite_Normalize);
    return NULL;
}
#endif /* POPTS_USE_SUITE */

void
MenuDone(class printopts  *self, long  rock)
{
    ApplyValues(self, 0);

/* Avoid destroying window while within pushbuttonv_PullTrigger(). */
    im::ForceUpdate();
    im::EnqueueEvent((event_fptr)DestroyWindow, (char *)self, 0);
}

void
MenuCancel(class printopts  *self, long  rock)
{
    int i;

    ResetValues(self);
    for (i = 0; i < NumberOfChecks; i++) {
	(self->check[i].obj)->SetValue( self->values[i]);
    }

/* Avoid destroying window while within pushbuttonv_PullTrigger(). */
    im::ForceUpdate();
    im::EnqueueEvent((event_fptr)DestroyWindow, (char *)self, 0);
}


void
ApplyValues(class printopts  *self, long  rock)
{
    int i;
    for (i = 0; i < NumberOfChecks; i++) {
	if (options[i].func == NULL && options[i].env) {
	    if (self->values[i]){
		if (options[i].envOn)
		    environ::Put(options[i].env, options[i].envOn);
		else
		    environ::Delete(options[i].env);
	    } else {
		if (options[i].envOff)
		    environ::Put(options[i].env, options[i].envOff);
		else
		    environ::Delete(options[i].env);
	    }
	}
    }
    strcpy(self->pnamevalue, (self->printername)->GetInput( 80));
    if (*self->pnamevalue) {
	environ::Put("LPDEST", self->pnamevalue);
	environ::Put("PRINTER", self->pnamevalue);
    }
}

void
CreateWindow(class view  *vp, long  rock)
{
    long left, top, width, height;
    class printopts *self;
    class im *vp_IM, *im;

    message::DisplayString(vp, 0, "Creating Dialog Box.  Please Wait.");
    im::ForceUpdate();

    self = new printopts;
    if (!self) {
	message::DisplayString(vp, 100, "Could not create the options.\nPerhaps we're out of memory?");
	return;
    }

    im::GetPreferedDimensions(&left, &top, &width, &height);
    
#define USEOVERRIDE

#ifdef USEOVERRIDE
    if ((vp_IM = (vp)->GetIM()) == NULL) {
	printf ("Attempt to create dialogue box for nonexistant view!\n Creating top level window.");
	im::SetPreferedDimensions(left, top, DIMENSION_X, DIMENSION_Y);
	im = im::Create(NULL);
    } else {
	im::SetPreferedDimensions(0, 0,DIMENSION_X, DIMENSION_Y);
	im = im::CreateOverride(vp_IM);
    }
#else /* USEOVERRIDE */
    im::SetPreferedDimensions(left, top, DIMENSION_X, DIMENSION_Y);
    im = im::Create(NULL);
#endif /* USEOVERRIDE */
    if (im) {
	(im)->SetTitle( "Print Options");
	(im)->SetView( self);
    } else {
	message::DisplayString(vp, 100, "Could not create a new window\nPerhaps there isn't enough memory?");
	(self)->Destroy();
	return;
    }

    im::SetPreferedDimensions(left, top, width, height);
    message::DisplayString(vp, 0, "Done.");

    (self)->WantInputFocus( self->printername);
}   

void
DestroyWindow( class printopts  *self )
{
    class im *pim=self->GetIM();
    if(pim) pim->SetView(NULL);
    self->Destroy();
    if(pim) pim->Destroy();
}

void
printopts::LinkTree( class view  *parent )
{
    (this)->view::LinkTree( parent);
    if(this->image)
	(this->image)->LinkTree( this);
    if (parent && (parent)->GetIM()) {
	int i;
	class bpair *bp=this->checklist;
	class view *dummy;
	struct ATKregistryEntry  *bpairinfo=NULL;

	if(bpairinfo==NULL) bpairinfo=ATK::LoadClass("bpair");

	while(bp && (bp)->ATKregistry()==bpairinfo) {
	    (bp)->VFixed( (bp)->GetNth( 0), (bp)->GetNth( 1), SCALEHEIGHT(parent, CheckHeight), 0);
	    bp=(class bpair *)(bp)->GetNth( 0);
	}
	(this->pname)->VFixed( this->printername, this->checklist, SCALEHEIGHT(parent, NumberOfChecks*CheckHeight) + 2, FALSE); 
    }
}
