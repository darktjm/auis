/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include <andrewos.h>
#include "rect.h"

#include "text.H"
#include "textview.H"
#include "view.H"
#include "graphic.H"
#include "fontdesc.H"
#include "icon.H"
#include "dataobject.H"
#include "iconview.H"
#include "bind.H"
#include "menulist.H"
#include "keymap.H"
#include "buffer.H"
#include <im.H>

#undef MIN
#define MIN(x,y) ((x<y)?(x):(y))

static class iconview *First;

/* Global determines if new icon views will draw open or closed. */
static int iconopen = TRUE;    /* default initial closed */

/* default values for closed icon character */
#define ICONFONT "icon"
#define ICONSTYLE fontdesc_Plain
#define ICONPTS 12
#define ICONCHAR '4'
#define TITLEFONT "andysans"
#define TITLESTYLE fontdesc_Plain
#define TITLEPTS 12
#define WIDTH 200
#define HEIGHT 100
#define	HANDLE	17

/****************************************************************/
/*		private functions				*/
/****************************************************************/

/* Draw the iconview in the case when its open (subview visable) */

ATKdefineRegistry(iconview, view, iconview::InitializeClass);
#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
#endif /* !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS) */
static void DrawOpen(class iconview  * self, enum view_UpdateType  type, long  ax, long  ay, long  aw, long  ah  /* area "A"ffected by this fullupdate */);
static void DrawClosed(class iconview  * self, enum view_UpdateType  type, long  ax, long  ay, long  aw, long  ah  /* area "A"ffected by this fullupdate */);
static void SlayChild(class iconview  * self);
static void AdoptNewChild(class iconview  * self,class icon  * dobj);
long string_width(const char  * string, class fontdesc  * font, class graphic  * graphic);
#if 0
void iconview__SetIconFontname (class iconview  * self,char  *name);
#endif 


static void
DrawOpen(class iconview  * self, enum view_UpdateType  type, long  ax, long  ay, long  aw, long  ah  /* area "A"ffected by this fullupdate */)
               {  
    long x,y,w,h;   /* my coordinate space */
    long cx, cy, cw, ch; /* my "C"hilds coordinate space */
    long handle_height;
    long tx, ty, tw, th; /* "T"itle coordinate space */
    struct FontSummary * titlesummary;
    short * titlefontwidths;
    const char * title;
    
    if (self->neednewsize) {
	self->neednewsize = 0;
	(self)->WantNewSize(self);
	return;
    }



    /* Get my size. Are width and height returning bogosity? */
    x = (self)->GetLogicalLeft();
    y = (self)->GetLogicalTop();
    w = (self)->GetLogicalWidth() - 1;
    h = (self)->GetLogicalHeight() - 1;

    /* Draw a frame */
    (self)->SetTransferMode( graphic_COPY);
    (self)->DrawRectSize( x, y, w, h);

    if (self->titlefont == (class fontdesc *)0)
	self->titlefont = fontdesc::Create(TITLEFONT, TITLESTYLE, TITLEPTS);
    titlesummary = (self->titlefont)->FontSummary( self->drawable);
    titlefontwidths = (self->titlefont)->WidthTable( self->drawable);

    /* get the title and calculate its width */
    title = ((class icon *) self->dataobject)->GetTitle();
    tw = MIN((string_width(title, self->titlefont, self->drawable) + (*title ? titlesummary->maxSpacing : 0)), w - 2); /* add one-character-width padding if title is non-null */

    handle_height = MIN(titlesummary->newlineHeight + 2, h);

    th = MIN(titlesummary->newlineHeight, handle_height - 2);
    tx = x + (w - tw) / 2;
    ty = y + (handle_height - th) / 2;

    /* With a gray stripe on top. */
    (self)->FillRectSize( x+1, y+1, w-1, handle_height-1, (self)->GrayPattern(4,16));

    /* draw the title */
    (self)->SetTransferMode( graphic_WHITE);
    (self)->FillRectSize( tx, ty, tw, th, graphic_WHITE);
    (self)->SetTransferMode( graphic_BLACK);
    (self)->SetFont( self->titlefont);
    (self)->MoveTo( x + w / 2, y + handle_height / 2);
    (self)->DrawString( title, (graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBOTTOM));

    /* cacluate place for children */
    cx = x + 1; /* one pixel frame */
    cy = y + handle_height;
    cw = w - 2; /* two one pixel sides */
    ch = h - handle_height - 1; /* handle and frame at bottom */

    if (self->child != (class view *)0) {
	/* is this the right place to do this? (inside my fullupdate)*/
	(self->child)->InsertViewSize( self, cx, cy, cw, ch);

	/* calculate the fullupdate coordinates in childs space */
	ax = ax - cx;
	ay = ay - cy;
	if (ax + aw > cx + cw)
	    aw = cx + cw - ax;
	if (ay + ah > cx + ch)
	    ah = cy + ch - ay;
	
	(self->child)->FullUpdate( type, 0, 0, cw, ch);
    }

    /* record childs extents */
    self->cx = cx;
    self->cy = cy;
    self->cw = cw;
    self->ch = ch;

    /* remember our size so we can rerequest it if necessary */
    (self)->RecommendSize( w+1, h+1);

}



/* Draw the iconview when it is closed (subview invisible) */
static void
DrawClosed(class iconview  * self, enum view_UpdateType  type, long  ax, long  ay, long  aw, long  ah  /* area "A"ffected by this fullupdate */)
               {  
    long x,y;
    struct fontdesc_charInfo iconinfo;
    short iconx, icony;

    if (self->neednewsize) {
	self->neednewsize = 0;
	(self)->WantNewSize(self);
	return;
    }
    /* Get my icon font. Is this sure not to return NULL? */
    if (self->iconfont == (class fontdesc *)0)
	self->iconfont = fontdesc::Create(ICONFONT, ICONSTYLE, ICONPTS);
    (self->iconfont)->CharSummary( self->drawable,
			  self->iconchar, &iconinfo);
    iconx = iconinfo.xOriginOffset;
    icony = iconinfo.yOriginOffset;

    /* Get my size. Are width and height returning bogosity? */
    x = (self)->GetLogicalLeft();
    y = (self)->GetLogicalTop();

    (self)->SetFont( self->iconfont);
    (self)->SetTransferMode( graphic_BLACK);
    (self)->MoveTo( x + iconx, y + icony);
    (self)->DrawText( &(self->iconchar), 1, graphic_NOMOVEMENT);
} 


static
void SlayChild(class iconview  * self)
    {  
    int twoviews;
    if (self->child != (class view *)0) {
	if (self->isopen)
	    (self->child)->UnlinkTree();
	twoviews = self->child != self->bottomview;
	(self->child)->Destroy();
	if (twoviews)
	    (self->bottomview)->Destroy();
	self->child = (class view *)0;
	self->bottomview = (class view *)0;
    }
}


static
void AdoptNewChild(class iconview  * self,class icon  * dobj)
        {  
    long x,y;
    const char * viewclass;
    class dataobject * d;

    SlayChild(self);

    d = ((class icon *)dobj)->GetChild();

    if (d != (class dataobject *)0) {
	viewclass = (d)->ViewName();
	self->child = (class view *)0;
	self->bottomview = (class view *)ATK::NewObject(viewclass);
	if (self->bottomview != (class view *)0) {
	    (self->bottomview)->SetDataObject(d);
	    self->child = (self->bottomview)->GetApplicationLayer();
	    if (self->isopen) {
		(self->child)->LinkTree(self);
		(self)->WantUpdate(self);
	    }
	}
    }
    ((class icon *)dobj)->GetSize( &x, &y);
    (self)->RecommendSize(x,y);
    self->neednewsize = 1;
}


long string_width(const char  * string, class fontdesc  * font, class graphic  * graphic)
{
    short * widthtable, totalwidth = 0;
    widthtable = (font)->WidthTable( graphic);
    while (*string) 
	totalwidth = totalwidth + widthtable[*string++];
    return(totalwidth);
}


/****************************************************************/
/*		class procedures				*/
/****************************************************************/

boolean
iconview::InitializeClass()
    {
First = NULL;
return TRUE;
} 


void iconview::GetOrigin(long  width, long  height, long  *originX, long  *originY)
                    {
    *originX = 0;
    *originY = 14;
}

iconview::iconview()
        {
	ATKinit;
  
    this->isopen = iconopen;
    this->dw = WIDTH;
    this->dh = HEIGHT;
    this->child = (class view *)0;
    this->bottomview = (class view *)0;
    this->neednewsize = 0;
    this->next = First;
    this->iconchar = ICONCHAR;
    this->iconfont =  fontdesc::Create(ICONFONT, ICONSTYLE, ICONPTS);
    this->titlefont =  fontdesc::Create(TITLEFONT, TITLESTYLE, TITLEPTS);
    First = this;
    THROWONFAILURE( TRUE);
}


iconview::~iconview()
        {
	ATKinit;
  
    class iconview *p;
    if(this == First){
	First = this->next;
    }
    else {
	for(p=First; p != NULL ; p = p->next){
	    if(p->next == this){
		p->next = this->next;
		break;
	    }
	}
    }
    SlayChild(this);
}

void
iconview::CloseRelated(class view  *v)
{
	ATKinit;

    class iconview *p;
    class im *im;

    iconopen = FALSE; /* All new notes draw closed. */

    im = (v)->GetIM();
    if(im == NULL) return;
    for(p=First; p != NULL ; p = p->next){
	if(p->isopen == TRUE && im == (p)->GetIM()) (p)->Close();
    }
}

void
iconview::OpenRelated(class view  *v)
{
	ATKinit;

    class iconview *p;
    class im *im;

    iconopen = TRUE; /* All new notes draw open. */

    im = (v)->GetIM();
    if(im == NULL) return;
    for(p=First; p != NULL ; p = p->next){
	if(p->isopen == FALSE && im == (p)->GetIM()) (p)->Open();
    }
}
/****************************************************************/
/*		instance methods				*/
/****************************************************************/

class view *
iconview::GetChild()
    {
    return this->child;
}

void
iconview::SetChild(char  *viewclass)
          {
    class icon * dobj = (class icon *)(this)->GetDataObject();
    class dataobject * d;

    SlayChild(this);

    d = ((class icon *)dobj)->GetChild();

    if (d != (class dataobject *)0) {
	this->child = (class view *)0;
	this->bottomview = (class view *)ATK::NewObject(viewclass);
	if (this->bottomview != (class view *)0) {
	    (this->bottomview)->SetDataObject(d);
	    this->child = (this->bottomview)->GetApplicationLayer();
	    if (this->isopen) {
		(this->child)->LinkTree(this);
		(this)->WantUpdate(this);
	    }
	}
    }
    (this)->WantUpdate( this);
}

void
iconview::Update()
     {
    struct rectangle r;

    (this)->SetTransferMode( graphic_COPY);
    (this)->EraseVisualRect();
    (this)->GetLogicalBounds( &r);
    (this)->FullUpdate( view_FullRedraw, r.left, r.top, r.width, r.height);
} /* iconview_Update */

void
iconview::FullUpdate(enum view_UpdateType  type, long  x , long  y , long  w , long  h)
               {  
    switch(type) {
	case view_FullRedraw:
	case view_PartialRedraw:
	case view_LastPartialRedraw: {
	    struct rectangle r;
	    (this)->GetLogicalBounds( &r);
	    if (this->isopen)
		DrawOpen(this, type, r.left, r.top, r.width, r.height);
	    else
		DrawClosed(this, type, r.left, r.top, r.width, r.height);
	    }
	    break;
	case view_MoveNoRedraw:
	case view_Remove:
	    (this->child)->FullUpdate( type, 0, 0, this->cw, this->ch);
	    break;
    }
}


view_DSattributes
iconview::DesiredSize(long  w , long  h, enum view_DSpass  pass, long  *dw , long  *dh)
                {  
	struct fontdesc_charInfo iconinfo;
	if (!this->isopen) {
	    (this->iconfont)->CharSummary( this->drawable,
				 this->iconchar, &iconinfo);
	    *dw = iconinfo.width;
	    *dh = iconinfo.height;
	    return (view_Fixed);
	} else {
	    *dw = this->dw;
	    *dh = this->dh;
	    (this)->DecidedSize( this->dw, this->dh);
	    return (view_WidthFlexible | view_HeightFlexible);
	}

}


class view *
iconview::Hit(enum view_MouseAction  action, long  x, long  y, long  clicks)
                {  
    if (this->isopen) {
	if (this->child != (class view *)0 &&
	    x >= this->cx &&
	    y >= this->cy &&
	    x < this->cx + this->cw &&
	    y < this->cy + this->ch) {
	    x -= this->cx;
	    y -= this->cy;
	    return (this->child)->Hit( action, x, y, clicks);
	} else {
	    if (y < this->cy
		&& action == view_LeftUp
		|| action == view_RightUp) {
		class view *v;
		(this)->Close();
		v = (class view *) this;
		v = v->parent;
		if(v != NULL && strcmp((v)->GetTypeName(),"matte") == 0)
		    v = v->parent;
		if(v)
		    (v)->WantInputFocus(v);
	    }
	    return (class view *)this;
	}
    } else {
	if (action == view_LeftUp || action == view_RightUp) {
	    (this)->Open();
	    if (this->child != (class view *)0) {
		(this->bottomview)->WantInputFocus(
				    this->bottomview);
	    } else {
		(this)->WantInputFocus( this);
	    }

	} else
	    return (class view *)this;
    }
    return (class view *)this;
}

void
iconview::ReceiveInputFocus()
{
    if (this->isopen && this->child != (class view *)0) {
	(this->bottomview)->WantInputFocus(
			    this->bottomview);
    } else {
	class view *parent = ((class view *)this)->parent;
	while (parent && !ATK::IsTypeByName ((parent)->GetTypeName(), "textview"))
	    parent = parent->parent;
	if (parent) (parent)->WantInputFocus( parent);
    }
}
    

void
iconview::Close()
{
    if(this->isopen == TRUE){
	this->isopen = FALSE;
	if (this->child != (class view *)0)
	    (this->child)->UnlinkTree();
	(this)->WantNewSize(this);
    }
}

void
iconview::Open()
{
    if(this->isopen == FALSE){
	this->isopen = TRUE;
	(this)->WantNewSize(this);
	if (this->child != (class view *)0) {
	    (this->child)->LinkTree( this);
	}
    }
}

void
iconview::RecommendSize(long  w, long  h)
            {  
	if (w > 0 && h > 0) {
	    this->dw = w;
	    this->dh = h;
	}
}



void
iconview::DecidedSize(long  w, long  h)
            {  
    ((class icon *)this->dataobject)->SetSize(w,h);
}



void
iconview::SetDataObject(class dataobject  * dobj)
        {  
    (this)->view::SetDataObject(dobj);
    AdoptNewChild(this,(class icon *)dobj);
}


void
iconview::ObservedChanged(class observable  * data, long  value )
            {  
    if (value == observable_OBJECTDESTROYED) {
	SlayChild(this);
    } else if (data == (class observable *)(this)->GetDataObject()){
	switch(value) {
	  case icon_TitleChanged:
	  case icon_SizeChanged:
	  case icon_ChangedSomehow:
	    (this)->WantUpdate( this);
	    break;
	  case icon_ChildChanged:
	    AdoptNewChild(this,(class icon *)data);
	    break;
	  default:
	    (this)->view::ObservedChanged( data, value);
	} /* switch */
    } else {
	(this)->view::ObservedChanged( data, value);
    }
}

void
iconview::LinkTree(class view  *parent)
        {
	(this)->view::LinkTree( parent);
	if(this->child) (this->child)->LinkTree( this);
}

void
iconview::SetIconFont(const char  * iconfont,int  iconstyle,int  iconpts)
    {
 
    this->iconfont = fontdesc::Create(iconfont, iconstyle, iconpts);
}

#if 0
void
iconview__SetIconFontname (class iconview  * self,char  *name)
  {
     self->iconfont = (class fontdesc *)0; /* force replacement */
     if (self->iconfontname) free (self->iconfontname);
     self->iconfontname = (char *)malloc(strlen(name) +1);
     bcopy (name, self->iconfontname, strlen(name));
}
#endif 

void
iconview::SetIconChar(char  iconchar)
{
    this->iconchar = iconchar;
}

void
iconview::SetTitleFont(const char  * titlefont,int  titlestyle,int  titlepts)
    {
 
    this->titlefont = fontdesc::Create(titlefont, titlestyle, titlepts);
}

boolean iconview::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    boolean res;

    if (this->child) {
	if (!this->isopen)
	    this->Open();
	res = this->child->RecSearch(pat, toplevel);
	return res;
    }

    return FALSE;
}

boolean iconview::RecSrchResume(struct SearchPattern *pat)
{
    boolean res;

    if (this->child) {
	if (!this->isopen)
	    this->Open();
	res = this->child->RecSrchResume(pat);
	return res;
    }
    return FALSE;
}

boolean iconview::RecSrchReplace(class dataobject *text, long pos, long len)
{
    boolean res;

    if (this->child) {
	if (!this->isopen)
	    this->Open();
	res = this->child->RecSrchReplace(text, pos, len);
	return res;
    }
    return FALSE;
}

void iconview::RecSrchExpose(const struct rectangle &logical, struct rectangle &hit)
{
    long x,y,w,h;   /* my coordinate space */
     long cx, cy, cw, ch; /* my "C"hilds coordinate space */
      long handle_height;
      struct FontSummary * titlesummary;
      short * titlefontwidths;
      const char * title;

      if (this->child) {
	  if (!this->isopen)
	      this->Open();

	  x = logical.left;
	  y = logical.top;
	  w = logical.width - 1;
	  h = logical.height - 1;
	  
	  if (this->titlefont == (class fontdesc *)0)
	      this->titlefont = fontdesc::Create(TITLEFONT, TITLESTYLE, TITLEPTS);
	  titlesummary = (this->titlefont)->FontSummary( this->drawable);

	  title = ((class icon *) this->dataobject)->GetTitle();

	  handle_height = MIN(titlesummary->newlineHeight + 2, h);

	  cx = x + 1; 
	  cy = y + handle_height;
	  cw = w - 2; 
	  ch = h - handle_height - 1;

	  struct rectangle child;
	  child.left=cx;
	  child.top=cy;
	  child.width=cw;
	  child.height=ch;
	  this->child->RecSrchExpose(child, hit);
      }
}

void iconview::ExposeChild(class view *v)
{
    if (!this->isopen) {
	this->Open();
    }
}
