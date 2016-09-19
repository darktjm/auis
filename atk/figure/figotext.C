/* figotext.c - fig element object: text */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/ 
#include <andrewos.h>
ATK_IMPL("figotext.H")
#include <ctype.h>

#include <figotext.H>

#include <figattr.H>
#include <view.H>
#include <figview.H>
#include <figure.H>
#include <figtoolview.H>
#include <fontdesc.H>
#include <message.H>
#include <region.H>
#include <keymap.H>
#include <keystate.H>
#include <proctable.H>
#include <print.H>
#include <simpletext.H>
#include <search.H>

static class keymap *EmbeddedKeymap;
static class region *tmpreg;

#define figotext_Leading (1)

       



ATKdefineRegistry(figotext, figobj, figotext::InitializeClass);
static void IncreaseNumChars(class figotext  *self, int  val);
static void CompleteProc(class figotext  *self, int  rock);
static void KillDotProc(class figotext  *self);
static void MoveDot(class figotext  *self, int  pos    /* -1 to remove dot */);
static void MoveDotProc(class figotext  *self, int  towhere);
static void KillLineProc(class figotext  *self, int  rock);
static void TwiddleCharsProc(class figotext  *self, int  rock);
static void InsertProc(class figotext  *self, int  rock);
static void DeleteProc(class figotext  *self, int  rock);
static void MoveHandle(class figotext  *self, long  x , long  y , long  ptref);


boolean figotext::InitializeClass()
{
    struct proctable_Entry *proc = NULL;
    int ix;
    char str[2];

    tmpreg = region::CreateEmptyRegion();

    EmbeddedKeymap = new keymap;

    proc = proctable::DefineProc("figotext-insert-char", (proctable_fptr)InsertProc, &figotext_ATKregistry_ , NULL, "Insert a character into this object.");
    (EmbeddedKeymap)->BindToKey( "\015", proc, '\n'); /* ctrl-M */
    str[1] = '\0';
    for (ix = 32; ix < 127; ix++)  {
	str[0] = ix;
	(EmbeddedKeymap)->BindToKey( str, proc, ix);
    }
    proc = proctable::DefineProc("figotext-delete-char", (proctable_fptr)DeleteProc, &figotext_ATKregistry_ , NULL, "Delete a character from this object.");
    (EmbeddedKeymap)->BindToKey( "\010", proc, 0); /* ctrl-H */
    (EmbeddedKeymap)->BindToKey( "\177", proc, 0); /* DEL */

    proc = proctable::DefineProc("figotext-kill-line", (proctable_fptr)KillLineProc, &figotext_ATKregistry_ , NULL, "Delete chars starting at dot.");
    (EmbeddedKeymap)->BindToKey( "\013", proc, 0); /* ctrl-K */

    proc = proctable::DefineProc("figotext-move-dot", (proctable_fptr)MoveDotProc, &figotext_ATKregistry_ , NULL, "Move the dot in this object.");
    (EmbeddedKeymap)->BindToKey( "\002", proc, 0);  /* ctrl-B */
    (EmbeddedKeymap)->BindToKey( "\033D", proc, 0); /* esc-D */
    (EmbeddedKeymap)->BindToKey( "\006", proc, 1);  /* ctrl-F */
    (EmbeddedKeymap)->BindToKey( "\033C", proc, 1); /* esc-C */
    (EmbeddedKeymap)->BindToKey( "\001", proc, 2);  /* ctrl-A */
    (EmbeddedKeymap)->BindToKey( "\005", proc, 3);  /* ctrl-E */
    (EmbeddedKeymap)->BindToKey( "\020", proc, 4);  /* ctrl-P */
    (EmbeddedKeymap)->BindToKey( "\033A", proc, 4);  /* esc-A */
    (EmbeddedKeymap)->BindToKey( "\016", proc, 5);  /* ctrl-N */
    (EmbeddedKeymap)->BindToKey( "\033B", proc, 5);  /* esc-B */

    proc = proctable::DefineProc("figotext-twiddle-chars", (proctable_fptr)TwiddleCharsProc, &figotext_ATKregistry_ , NULL, "Move the dot in this object.");
    (EmbeddedKeymap)->BindToKey( "\024", proc, 0);  /* ctrl-T */

    proc = proctable::DefineProc("figotext-complete-entry", (proctable_fptr)CompleteProc, &figotext_ATKregistry_ , NULL, "Finish entering text into this object.");
    (EmbeddedKeymap)->BindToKey( "\033\015", proc, 0);  /* esc-ctrl-M */

    return TRUE;
}

figotext::figotext()
{
	ATKinit;

    this->Keystate = keystate::Create(this, EmbeddedKeymap);

    (this)->AttributesUsed() = (1<<figattr_FontSize) | (1<<figattr_FontStyle) | (1<<figattr_FontFamily) | (1<<figattr_Color) | (1<<figattr_TextPos);
    (this)->SetNumHandles( 4);

    this->basis = NULL;
    this->buildstate = 0;
    this->text_size = 8;
    this->text = (char *)malloc(this->text_size * sizeof(char));
    this->textdirty = TRUE;
    this->yoffset = 0;
    this->dotpos = (-1);
    this->excessx = 0;
    this->excessy = 0;
    this->recsearchvalid = FALSE;

    THROWONFAILURE( TRUE);
}

class figotext *figotext::Create(char  *chars, long  xpos , long  ypos)
{
	ATKinit;

    class figotext *res = new figotext;
    if (!res) return NULL;

    (res)->PosX() = xpos;
    (res)->PosY() = ypos;

    IncreaseNumChars(res, strlen(chars)+2);
    strcpy(res->text, chars);
    res->textdirty = TRUE;
    (res)->RecomputeBounds();

    return res;
}

figotext::~figotext()
{
	ATKinit;

    if (this->text)
	free(this->text);
}

const char *figotext::ToolName(class figtoolview  *v, long  rock)
{
    return "Text";
}

/* set bounding box and handle list in fig coordinates. Note that the bounding box is computed by scaling a normal-size font's box, whereas the actual drawing is done with a scaled-size font.  */
void figotext::RecomputeBounds()
{   
    long x, y, w, h, texw, texh;
    struct rectangle altrec;
    long x0 = 0, y0;
    char *fam;
    long size, style, textpos;

    fam = ((this)->GetVAttributes())->GetFontFamily( (this)->GetIVAttributes());
    size = ((this)->GetVAttributes())->GetFontSize( (this)->GetIVAttributes());
    style = ((this)->GetVAttributes())->GetFontStyle( (this)->GetIVAttributes());
    textpos = ((this)->GetVAttributes())->GetTextPos( (this)->GetIVAttributes());

    this->fdesc = fontdesc::Create(fam, style, size);
    if (!this->fdesc) return;

    if (this->textdirty) {
	if (this->basis) {
	    int tmp1, tmp2, count;
	    char *cx, *cxend;

	    this->textw = 0;
	    this->texth = 0;

	    count=0;
	    cx = this->text;

	    this->yoffset = 0;
	    do {
		cxend = strchr(cx, '\n');
		if (cxend) 
		    *cxend = '\0';
		if (!(this->fdesc)->StringBoundingBox( (this->basis)->GetDrawable(), cx, &tmp1, &tmp2)) {
		    tmp1 = 10;
		    tmp2 = 10;
		}

		if (this->textw < tmp1)
		    this->textw = tmp1;
		if (count==0) {
		    this->texth = tmp2;
		    this->yoffset = tmp2 / 2;
		}
		else {
		    tmp2 = count*(size+figotext_Leading) + tmp2/2;
		    this->texth = tmp2 + this->yoffset;
		}

		if (cxend) {
		    *cxend = '\n';
		    cx = cxend+1;
		}
		count++;
	    } while (cxend);
	}
	else {
	    this->textw = 10;
	    this->texth = 10;
	}
	this->textdirty = FALSE;
    }

    texw = (this->textw+1) * figview_FigUPerPix;
    texh = (this->texth+1) * figview_FigUPerPix;
    w = texw + this->excessx;
    h = texh + this->excessy;

    altrec.width = texw;
    altrec.height = texh;

    x = (this)->PosX();
    y = (this)->PosY();
    switch (textpos) {
	case figattr_PosCenter:
	    x0 = x-w/2;
	    altrec.left = x - texw/2;
	    break;
	case figattr_PosLeft:
	    x0 = x-this->excessx/2;
	    altrec.left = x;
	    break;
	case figattr_PosRight:
	    x0 = x-w+this->excessx/2;
	    altrec.left = x - texw;
	    break;
    }

    y0 = y - (this->yoffset * figview_FigUPerPix + this->excessy/2);
    altrec.top = y - (this->yoffset * figview_FigUPerPix);

    (this)->SetHandle( 0, x0+w, y0);
    (this)->SetHandle( 1, x0, y0);
    (this)->SetHandle( 2, x0, y0+h);
    (this)->SetHandle( 3, x0+w, y0+h);

    if (w>=0) {
	this->handlerect.left = x0;
	this->handlerect.width = w;
    }
    else {
	this->handlerect.left = x0 + w;
	this->handlerect.width = (-w);
    }

    if (h>=0) {
	this->handlerect.top = y0;
	this->handlerect.height = h;
    }
    else {
	this->handlerect.top = y0 + h;
	this->handlerect.height = (-h);
    }

    rectangle_UnionRect(&altrec, &altrec, &this->handlerect);
    (this)->SetBoundsRect( altrec.left, altrec.top, altrec.width, altrec.height);

    (this)->ComputeSelectedBounds();
    (this)->UpdateParentBounds();
}

static enum figobj_HandleType handletypes[4]={
    figobj_URCorner,
    figobj_ULCorner,
    figobj_LLCorner,
    figobj_LRCorner
};

enum figobj_HandleType figotext::GetHandleType(long  num)
{
    if(num>=0 && num<=3) return handletypes[num];
    else return figobj_None;
}

static long canonical[] = {
    1, 3, figobj_NULLREF
};

long *figotext::GetCanonicalHandles()
{
    return canonical;
}

struct rectangle *figotext::GetBounds(class figview  *vv)
{
    if (!vv) {
	if (!this->basis) {
	    (this)->RecomputeBounds();
	}
    }
    else if (vv != this->basis) {
	this->basis = vv;
	this->textdirty = TRUE;
	(this)->RecomputeBounds();
    }

    return (this)->figobj::GetBounds( vv);
}

void figotext::Draw(class figview  *v) 
{
    long gray, count;
    const char *fam, *col, *cx, *cxend;
    long size, style, textpos, grapos = 0, grax;
    struct rectangle *rec = (this)->GetBounds( v);
    class region *viewclip;
    struct rectangle bb;

    fam = ((this)->GetVAttributes())->GetFontFamily( 
		(this)->GetIVAttributes());
    size = ((this)->GetVAttributes())->GetFontSize( 
		(this)->GetIVAttributes());
    style = ((this)->GetVAttributes())->GetFontStyle( 
		(this)->GetIVAttributes());
    textpos = ((this)->GetVAttributes())->GetTextPos( 
		(this)->GetIVAttributes());

// The following line was here and was WRONG.  X scales fonts
// so that a 12 point font (or whatever) will appear the same size
// on every screen.
// Unfortunately, removing the line will make smaller the text in 
// all existing figures.
//
//     size = (v)->ToPixW( size * figview_FigUPerPix);  

    this->fdesc = fontdesc::Create(fam, style, size);
    if (!this->fdesc) return;

    (v)->SetTransferMode( graphic_COPY);
    col = ((this)->GetVAttributes())->GetColor( (this)->GetIVAttributes());
    (v)->SetForegroundColor( col, 0, 0, 0); 

    bb.left = (v)->ToPixX( rec->left) - 2;
    bb.top = (v)->ToPixY( rec->top) - 2;
    bb.width = (v)->ToPixW( rec->width) + 4;
    bb.height = (v)->ToPixH( rec->height) + 4;

    viewclip = (v)->GetCurrentClipRegion();

    (tmpreg)->RectRegion( &bb); 
    if (viewclip)
	(tmpreg)->IntersectRegion( viewclip, tmpreg);
    (v)->SetClippingRegion( tmpreg); 

    (v)->SetFont( this->fdesc);
    
    gray = (v)->ToPixY( (this)->PosY());
    switch (textpos) {
	case figattr_PosCenter:
	    grapos = graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBOTTOM;
	    grax = (v)->ToPixX( (this)->PosX());
	    break;
	case figattr_PosLeft:
	    grapos = graphic_ATLEFT | graphic_BETWEENTOPANDBOTTOM;
	    grax = (v)->ToPixX( (this)->PosX());
	    break;
	case figattr_PosRight:
	    grapos = graphic_ATRIGHT | graphic_BETWEENTOPANDBOTTOM;
	    grax = (v)->ToPixX( (this)->PosX());
	    break;
    }
    cx = this->text;
    count = 0;
    do {
	(v)->MoveTo( grax, gray+count*(size+figotext_Leading));
	cxend = strchr(cx, '\n');
	if (cxend)
	    (v)->DrawText( cx, (cxend-cx), grapos);
	else
	    (v)->DrawString( cx, grapos);
	count++;
	if (cxend)
	    cx = cxend+1;
    } while (cxend);
    /*figview_DrawString(v, self->text, graphic_BETWEENLEFTANDRIGHT | graphic_BETWEENTOPANDBOTTOM);*/

    if (viewclip)
	(v)->SetClippingRegion( viewclip);
    else
	(v)->ClearClippingRect();
}

void figotext::Sketch(class figview  *v) 
{
    /*super_Sketch(self, v);*/
    long x, y, w, h;

    x = (v)->ToPixX( this->handlerect.left);
    y = (v)->ToPixY( this->handlerect.top);
    w = (v)->ToPixW( this->handlerect.width);
    h = (v)->ToPixH( this->handlerect.height);
    (v)->SetTransferMode( graphic_INVERT);
    (v)->DrawRectSize( x, y, w, h);
}

static void IncreaseNumChars(class figotext  *self, int  val)
{
    if (val > self->text_size) {
	while (val > self->text_size)
	    self->text_size *= 2;

	self->text = (char *)realloc(self->text, self->text_size);
    }
}

enum figobj_Status figotext::Build(class figview  *v, enum view_MouseAction  action, long  x , long  y /* in fig coords */, long  clicks)   
{
    int ix;

    if (clicks==0) {
	KillDotProc(this);
	ix = strlen(this->text);
	if (ix) {
	    (this)->RecomputeBounds();
	    (this)->SetModified();
	    return figobj_Done;
	}
	else {
	    message::DisplayString(v, 10, "No string entered; object aborted.");
	    return figobj_Failed;
	}
    }

    switch (action) {
	case view_LeftDown:
	    if (this->buildstate==0) {
		(this)->PosX() = x;
		(this)->PosY() = y;
		strcpy(this->text, "|");
		this->dotpos = 0;
		this->buildstate = 1;
		this->textdirty = TRUE;
		(v)->SetBuildKeystate( this->Keystate);
		this->buildview = v;
		(this)->RecomputeBounds();
		(this)->SetModified();
		(v)->WantUpdate( v);
		return figobj_NotDone;
	    }
	    else {
		(this)->PosX() = x;
		(this)->PosY() = y;
		(this)->RecomputeBounds();
		(this)->SetModified();
		(v)->WantUpdate( v);
		return figobj_NotDone;
	    }
	case view_LeftMovement:
	    if ((this)->PosX() != x || (this)->PosY() != y) {
		(this)->PosX() = x;
		(this)->PosY() = y;
		(this)->RecomputeBounds();
		(this)->SetModified();
		(v)->WantUpdate( v);
	    }
	    return figobj_NotDone;
	case view_LeftUp:
	    return figobj_NotDone;
	case view_RightDown:
	    KillDotProc(this);
	    ix = strlen(this->text);
	    if (ix)  {
		(this)->RecomputeBounds();
		(this)->SetModified();
		return figobj_Done;
	    }
	    else {
		message::DisplayString(v, 10, "No string entered; object aborted.");
		return figobj_Failed;
	    }
	default:
	    return figobj_Failed;
    }
}

enum figobj_HitVal figotext::HitMe(long  x , long  y, long  delta, long  *ptref) 
{
    enum figobj_HitVal res;
    res = (this)->BasicHitMe( x, y, delta, ptref);

    if (res==figobj_HitInside) {
	res = figobj_HitBody;
	if (ptref)
	    *ptref = 0; /* only one body part */
    }
    return res;
}

static void CompleteProc(class figotext  *self, int  rock)
{
    if (self->buildview && self->buildview->toolset) {
	(self->buildview->toolset)->AbortObjectBuilding();
    }
}

static void KillDotProc(class figotext  *self)
{
    char *ch;
    if (self->dotpos == (-1))
	return;

    for (ch = self->text+self->dotpos; *ch; ch++)
	*ch = *(ch+1);
    self->dotpos = (-1);
    self->textdirty = TRUE;
}

static void MoveDot(class figotext  *self, int  pos    /* -1 to remove dot */)
{
    int ix, len;

    if (pos == self->dotpos)
	return;

    if (pos == (-1)) {
	KillDotProc(self);
	return;
    }

    if (self->dotpos == (-1)) {
	len = strlen(self->text);

	if (len+2 >= self->text_size)
	    IncreaseNumChars(self, len+2);

	self->dotpos = pos;
	for (ix = len+1; ix>self->dotpos; ix--)
	    self->text[ix] = self->text[ix-1];
	self->text[self->dotpos] = '|';
    }
    else {
	if (pos < self->dotpos) {
	    for (ix=self->dotpos; ix>pos; ix--)
		self->text[ix] = self->text[ix-1];
	    self->dotpos = pos;
	    self->text[self->dotpos] = '|';
	}
	else {
	    for (ix=self->dotpos; ix<pos; ix++)
		self->text[ix] = self->text[ix+1];
	    self->dotpos = pos;
	    self->text[self->dotpos] = '|';
	}
    }
}

static void MoveDotProc(class figotext  *self, int  towhere)
{
    int pos, len;
    int ix;

    if (self->dotpos == (-1))
	return;
    switch (towhere) {
	case 0: /* back */
	    if (self->dotpos == 0)
		return;
	    MoveDot(self, self->dotpos-1);
	    break;
	case 1: /* forward */
	    if (self->text[self->dotpos+1] == '\0')
		return;
	    MoveDot(self, self->dotpos+1);
	    break;
	case 2: /* beginning */
	    MoveDot(self, 0);
	    break;
	case 3: /* end */
	    MoveDot(self, strlen(self->text)-1);
	    break;
	case 4: /* up */
	    for (ix=self->dotpos; ix && self->text[ix] != '\n'; ix--);
	    if (!ix) return;
	    pos = ix;
	    len = self->dotpos - (pos+1);
	    for (ix=pos-1; ix && self->text[ix] != '\n'; ix--);
	    if (self->text[ix] == '\n')
		ix++;
	    ix += len;
	    if (ix>pos)
		ix = pos;
	    MoveDot(self, ix);
	    break;
	case 5: /* down */
	    for (ix=self->dotpos; ix && self->text[ix] != '\n'; ix--);
	    if (!ix) ix--;
	    len = self->dotpos - (ix+1);
	    for (ix=self->dotpos; self->text[ix] && self->text[ix] != '\n'; ix++);
	    if (!self->text[ix])
		ix--;
	    else {
		pos = ix+1;
		for (ix=pos; ix-pos < len && self->text[ix] && self->text[ix] != '\n'; ix++);
		ix--;
	    }
	    MoveDot(self, ix);
	    break;
	default:
	    return;
    }

    self->textdirty = TRUE;
    (self)->RecomputeBounds();
    (self)->SetModified();
    (self->buildview)->WantUpdate( self->buildview);
}

static void KillLineProc(class figotext  *self, int  rock)
{
    if (self->dotpos == (-1))
	return;

    self->text[self->dotpos+1] = '\0';

    self->textdirty = TRUE;
    (self)->RecomputeBounds();
    (self)->SetModified();
    (self->buildview)->WantUpdate( self->buildview);
}

static void TwiddleCharsProc(class figotext  *self, int  rock)
{
    char tmp;
    if (self->dotpos < 2)
	return;

    tmp = self->text[self->dotpos-1];
    self->text[self->dotpos-1] = self->text[self->dotpos-2];
    self->text[self->dotpos-2] = tmp;

    self->textdirty = TRUE;
    (self)->RecomputeBounds();
    (self)->SetModified();
    (self->buildview)->WantUpdate( self->buildview);
}

static void InsertProc(class figotext  *self, int  rock)
{
    int ix;
    int len = strlen(self->text);

    if (self->dotpos == (-1))
	return;

    if (len+2 >= self->text_size)
	IncreaseNumChars(self, len+2);

    for (ix = len+1; ix>self->dotpos+1; ix--)
	self->text[ix] = self->text[ix-1];

    self->text[self->dotpos] = rock;
    self->dotpos++;
    self->text[self->dotpos] = '|';

    self->textdirty = TRUE;
    (self)->RecomputeBounds();
    (self)->SetModified();
    (self->buildview)->WantUpdate( self->buildview);
}

static void DeleteProc(class figotext  *self, int  rock)
{
    char *ch;
    if (self->dotpos<=0)
	return;

    for (ch = self->text+self->dotpos; *ch; ch++)
	*ch = *(ch+1);
    self->dotpos--;
    self->text[self->dotpos] = '|';

    self->textdirty = TRUE;
    (self)->RecomputeBounds();
    (self)->SetModified();
    (self->buildview)->WantUpdate( self->buildview);
}

static void MoveHandle(class figotext  *self, long  x , long  y , long  ptref)
{
    struct point *pt = &((self)->GetHandles()[ptref]);
    int val;

    if (ptref==1 || ptref==2) {
	val = pt->x - x;
	self->excessx += val;
	(self)->PosX() -= val/2;
    }
    else {
	val = x - pt->x;
	self->excessx += val;
	(self)->PosX() += val/2;
    }

    if (ptref==1 || ptref==0) {
	val = pt->y - y;
	self->excessy += val;
	(self)->PosY() -= val/2;
    }
    else {
	val = y - pt->y;
	self->excessy += val;
	(self)->PosY() += val/2;
    }

    /* figotext_Reposition(self, x - pt->x, y - pt->y); */
}

boolean figotext::AddParts(enum view_MouseAction  action, class figview  *v, long  x , long  y , boolean  handle, long  ptref)
{
    int len = strlen(this->text);
    
    if (action==view_LeftDown) {

	if (!v->toolset)
	    return FALSE;

	if (this->dotpos != (-1)) {
	    message::DisplayString(v, 10, "Text is already being edited.");
	    return FALSE;
	}

	if (len+2 >= this->text_size)
	    IncreaseNumChars(this, len+2);

	this->text[len] = '|';
	this->text[len+1] = '\0';
	this->dotpos = len;
	this->buildview = v;
	v->toolset->creating = (class figobj *)this; /* this is really, really ugly. Really. */

	(v)->SetBuildKeystate( this->Keystate);

	this->textdirty = TRUE;
	(this)->RecomputeBounds();
	(this)->SetModified();
	message::DisplayString(v, 10, "Editing text object.");
	(this->buildview)->WantUpdate( this->buildview);
	return TRUE;
    }
    else {
	return FALSE;
    }
}

boolean figotext::Reshape(enum view_MouseAction  action, class figview  *v, long  x , long  y , boolean  handle, long  ptref)
{
    if (!handle)
	return FALSE;
    switch (action) {
	case view_LeftDown:
	case view_RightDown:
	    if ((this)->GetReadOnly())
		return FALSE;
	    (this)->Sketch( v);
	    break;
	case view_LeftMovement:
	case view_RightMovement:
	    (this)->Sketch( v);
	    ::MoveHandle(this, x, y, ptref);
	    (this)->RecomputeBounds();
	    (this)->Sketch( v);
	    break;
	case view_LeftUp:
	case view_RightUp:
	    (this)->Sketch( v);
	    ::MoveHandle(this, x, y, ptref);
	    (this)->RecomputeBounds();
	    (this)->SetModified();
	    break;
	default: // filedrop, upmovement, nomouse
	    break;
    }
    return TRUE;
}

void figotext::MoveHandle(long  x , long  y , long  ptref)
{
    if ((this)->GetReadOnly())
	return;
    ::MoveHandle(this, x, y, ptref);
    (this)->SetModified();
    (this)->RecomputeBounds();
}

void figotext::InheritVAttributes(class figattr  *attr, unsigned long  mask)
{
    (this)->figobj::InheritVAttributes( attr, mask);

    if (mask & (~((this)->GetVAttributes()->active)) & ((1<<figattr_FontFamily) | (1<<figattr_FontStyle) | (1<<figattr_FontSize) | (1<<figattr_TextPos))) {
	this->textdirty = TRUE;
	(this)->RecomputeBounds();
    }
}

unsigned long figotext::UpdateVAttributes(class figattr  *attr, unsigned long  mask)
{
    mask = (this)->figobj::UpdateVAttributes( attr, mask);

    if (mask & ((1<<figattr_FontFamily) | (1<<figattr_FontStyle) | (1<<figattr_FontSize) | (1<<figattr_TextPos))) {
	this->textdirty = TRUE;
	(this)->RecomputeBounds();
    }
    return mask;
}

/* the format for writing the text:
nonprintable characters, including space, \, =: =000 (octal)
others: inserted normally.
A newline is inserted every LINESIZE output characters. The end is marked with =000.
*/
void figotext::WriteBody(FILE  *fp)
{
    int ix, ch;
    int count;

    (this)->figobj::WriteBody( fp);
#define LINESIZE (70)

    fprintf(fp, "$ %ld %ld\n", this->excessx, this->excessy);

    count=0;
    for (ix=0; 1; ix++) {
	ch = this->text[ix];
	if (ch=='\\' || ch=='=' || !isgraph(ch)) {
	    fprintf(fp, "=%03o", ch);
	    count+=4;
	}
	else {
	    fputc(ch, fp);
	    count++;
	}
	if (count >= LINESIZE || ch=='\0') {
	    fputc('\n', fp);
	    count=0;
	}

	if (ch=='\0')
	    break;
    }
}

long figotext::ReadBody(FILE  *fp, boolean  recompute)
{
    int	ix; 
    int count, ch = 0;
    long tmp1, tmp2;

#define LINELENGTH (250)
    char buf[LINELENGTH+1];

    ix = (this)->figobj::ReadBody( fp, FALSE);
    if (ix!=dataobject_NOREADERROR) return ix;

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;
    ix = sscanf(buf, "$ %ld %ld", &tmp1, &tmp2);
    if (ix!=2) return dataobject_BADFORMAT;
    this->excessx = tmp1;
    this->excessy = tmp2;

    count = 0;
    while (1) {
	if (fgets(buf, LINELENGTH, fp) == NULL)
	    return dataobject_PREMATUREEOF;

	for (ix=0; buf[ix]; ix++) {
	    if (buf[ix]=='\n') 
		continue;
	    IncreaseNumChars(this, count+2);
	    if (buf[ix]=='=') {
		ch = (buf[ix+1]-'0')*64 + (buf[ix+2]-'0')*8 + (buf[ix+3]-'0');
		ix += 3;
	    }
	    else {
		ch = buf[ix];
	    }
	    this->text[count] = ch;
	    count++;
	    if (ch=='\0')
		break;
	}
	if (ch=='\0')
	    break;
    }
    this->textdirty = TRUE;

    if (recompute) {
	(this)->RecomputeBounds();
	(this)->SetModified();
    }

    return dataobject_NOREADERROR;
}

void figotext::PrintObject(class figview  *v, FILE  *file, const char  *prefix, boolean newstyle)
{
    int ix, ch, count;
    char *fam, fontname[256];
    long size, style, textpos;
    long x, y;
    const char *col;
    const char *posmod;
    double rcol, bcol, gcol;
    short *encoding;

    fprintf(file, "%s  %% text\n", prefix);
 
    col = ((this)->GetVAttributes())->GetColor( (this)->GetIVAttributes());
    print::LookUpColor(col, &rcol, &gcol, &bcol);

    x = (v)->ToPrintPixX( (this)->PosX());
    y = (v)->ToPrintPixY( (this)->PosY());
    fam = ((this)->GetVAttributes())->GetFontFamily( (this)->GetIVAttributes());
    size = ((this)->GetVAttributes())->GetFontSize( (this)->GetIVAttributes());
    style = ((this)->GetVAttributes())->GetFontStyle( (this)->GetIVAttributes());
    textpos = ((this)->GetVAttributes())->GetTextPos( (this)->GetIVAttributes());
    size = (v)->ToPrintPixW( size * figview_FigUPerPix);

    print::LookUpPSFont(fontname, &encoding, NULL, fam, size, style);

    switch (textpos) {
	case figattr_PosLeft:
	    posmod = "0";
	    break;
	case figattr_PosRight:
	    posmod = "dup stringwidth pop neg";
	    break;
	case figattr_PosCenter:
	default:
	    posmod = "dup stringwidth pop 2 div neg";
	    break;
    }
    fprintf(file, "%s  gsave\n", prefix);
    if (newstyle) {
	ix = print::PSRegisterFont(fontname);
	fprintf(file, "%s  %s%d %ld scalefont setfont\n", prefix, print_PSFontNameID, ix, size);
    }
    else {
	fprintf(file, "%s  /%s findfont %ld scalefont setfont\n", prefix, fontname, size);
    }
    fprintf(file, "%s  %g %g %g setrgbcolor\n", prefix, rcol, gcol, bcol);
    /*fprintf(file, "%s  0 setgray\n", prefix);*/
    fprintf(file, "%s  %ld %ld translate 1 -1 scale\n", prefix, x, y);

    fprintf(file, "%s  (", prefix);
    count = 0;
    for (ix=0; TRUE; ix++) {
	ch = this->text[ix];
	if (ch=='\0' || ch=='\n') {
	    fprintf(file, ") %s %ld moveto show\n", posmod, -count * (size+figotext_Leading));
	    if (ch=='\n') {
		fprintf(file, "%s  (", prefix);
		count++;
	    }
	    else
		break;
	}
	else {
	    if (ch==' ') {
		fputc(ch, file);
	    }
	    else if (ch=='\\' || ch=='(' || ch==')' || !isgraph(ch)) {
		fprintf(file, "\\%03o", ch);
	    }
	    else {
		fputc(ch, file);
	    }
	}
    }

    fprintf(file, "%s  grestore\n", prefix);
}

boolean figotext::ORecSearch(struct SearchPattern *pat)
{
    char *ts;
    int substart, pos;

    if (!(ts = this->text)) {
	this->recsearchvalid = FALSE;
	return FALSE;
    }

    pos = 0;
    substart = search::MatchPatternStr((unsigned char *)ts, pos, strlen(ts), pat);
    if (substart>=0) {
	this->recsearchvalid = TRUE;
	this->recsearchsubstart = substart;
	this->recsearchsublen = search::GetMatchLength();
	return TRUE;
    }

    this->recsearchvalid = FALSE;
    return FALSE;
}

boolean figotext::ORecSrchResume(struct SearchPattern *pat)
{
    char *ts;
    int substart, pos;

    if (!this->recsearchvalid || !(ts = this->text)) {
	this->recsearchvalid = FALSE;
	return FALSE;
    }

    pos = this->recsearchsubstart+this->recsearchsublen;

    substart = search::MatchPatternStr((unsigned char *)ts, pos, strlen(ts), pat);
    if (substart>=0) {
	this->recsearchvalid = TRUE;
	this->recsearchsubstart = substart;
	this->recsearchsublen = search::GetMatchLength();
	return TRUE;
    }

    this->recsearchvalid = FALSE;
    return FALSE;
}

boolean figotext::ORecSrchReplace(class dataobject *srcdobj, long srcpos, long srclen)
{
    char *buf;
    const char *ts;
    int substart, sublen;
    class simpletext *srctext;

    if (!this->recsearchvalid)
	return FALSE;

    if (!srcdobj->IsType("simpletext")) {
	return FALSE;
    }
    srctext = (class simpletext *)srcdobj;

    ts = this->Text();
    if (!ts)
	ts = "";
    substart = this->recsearchsubstart;
    sublen = this->recsearchsublen;
    buf = (char *)malloc(strlen(ts) - sublen + srclen + 2);
    strncpy(buf, ts, substart);
    srctext->CopySubString(srcpos, srclen, buf+substart, FALSE);
    strcpy(buf+substart+srclen, ts+substart+sublen);
    this->recsearchsublen = srclen;

    free(this->text);
    this->text = buf;

    this->textdirty = TRUE;
    this->RecomputeBounds();
    this->SetModified();
    this->GetAncestorFig()->NotifyObservers(figure_DATACHANGED);

    return TRUE;
}

