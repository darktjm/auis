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

#include <andrewos.h> /* sys/time.h sys/types.h */
ATK_IMPL("fadview.H")
#include <sys/stat.h>

#include <fadview.H>
#include <proctable.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <graphic.H>
#include <cursor.H>
#include <message.H>
#include <fontdesc.H>
#include <environ.H>
#include <bind.H>
#include <view.H>
#include <filetype.H>
#include <im.H>
#include <completion.H>
#include <event.H>

#define GetDelay(A) (event_MSECtoTU(20))
#define SetMode(A,B) (A->mode = (B))
#define GetMode(A) (A->mode)
#define IsMode(A,B) (A->mode & B)

#define SAMPLEFONT "/bin/samplefont" /* assume that this samplefont supports the -t mode needed by fad */
#define LINEMODE 0
#define BOXMODE 1
#define ICONMODE 2
#define GROUPMODE 4 /* Not Implemented */
#define LABELMODE 8
#define ANIMATEBUTTONMODE 16
#define WAITMODE 64

#define BOXMODEON
#define	NORESHAPE   /* Won't attempt to scale the drawing to the window */
#define MAXWIDTH 20000
#define MAXHEIGHT 20000
#define	STARTHEIGHT 256	/* initial default height */
#define FRTIME 30 	/* default minimum number of milliseconds between frames */
#define UPDATEICON -10010
#define OLD 1
#define NEW 0
#define PFUG 4
#define min(a,b) (a < b) ? a:b
#define BBUFSIZE 2048
#define ICONFLAG -10
#define ldraw(P,A) if(IsMode(A,BOXMODE))\
   {(P)->MoveTo((long)A->x1,(long)A->y1);\
   (P)->DrawLineTo((long)A->x2,(long)A->y1);\
   (P)->DrawLineTo((long)A->x2,(long)A->y2);\
   (P)->DrawLineTo((long)A->x1,(long)A->y2);\
   (P)->DrawLineTo((long)A->x1,(long)A->y1);}\
   else {(P)->MoveTo((long)A->x1,(long)A->y1);\
   (P)->DrawLineTo((long)A->x2,(long)A->y2);} 


struct aniinfo {
    class fadview *self;
    struct anivect *anbuf,*eap;
    struct fad_frame *lf,*sf;
    int i;
};

#define fdraw(A) if (ISICONORLABEL(A->x2)) idraw(self,A); else ldraw(self,A)
#define ObjectOf(ll) ll->dataobject
#define findpic(ll) ((class fad *)ObjectOf(ll))
#define TellUser(self, s) message::DisplayString(self,0,s)
#define BOXTEST(A) IsMode(A,BOXMODE)
static class keymap *fadviewKeymap;
static class menulist *fadviewMenulist;
static struct proctable_Entry *FadProc;
static int samplefont_failed ;




ATKdefineRegistry(fadview, view, fadview::InitializeClass);
static void MySetCursor(class fadview  *self,class fontdesc  *f,int  i);
static void MySetStandardCursor(class fadview  *self,short  i);
static int CurrentFrame(class fadview  *self);
static class fontdesc *my_DefineFont(const char  *fname);
static void UpdateCursor(class fadview  *self);
static void fontinit(class fad  *cp);
void HaltAnimation(class fadview  *self);
void queup(class fadview  *self);
static void dodoan(class fadview  *self);
static boolean DoAnimation(class fadview  *self);
boolean doan(struct aniinfo  *anobj);
static int recalc(class fadview  *self);
static void picset(class fadview  *self,int  flag);
static void clearfad(class fadview  *self);
static void AddMenus(class fadview  *self,class menulist  *ml,struct proctable_Entry  *menuProc);
static void KeyIn(class fadview  *self,long  cr);
static void nameframe(class fadview  *self);
boolean QueueAnimation(class fadview  *self,enum view_MouseAction  action,long  mousex ,long  mousey);
static void drawlist(class fadview  *self,register class fad  *cpic);
static void getlist(register class fadview  *self,register struct fadpoint  *ppt);
static int seticon(class fadview  *self);
static void ReadIcons(FILE  *f,class fadview  *self);
static void idraw(register class fadview  *self,register struct anivect  *A);
static void vecdraw(register class fadview  *self,register struct fadvector  *v);
static char labelfonttype(class fadview  *self);
static int labelfontsize(class fadview  *self);
static void PrintVec(class fad  *cp,struct fadvector  *v);
static void xx_MoveTo(long  x ,long  y			/* in fractional screen units */);
static void xx_DrawTo(long  x ,long  y			/* in fractional screen units */);
static void BeginTroff(FILE  *file,int  yneed			/* dots per inch */,class fadview  *self);

static void EndTroff(long yneed);

static void MySetCursor(class fadview  *self,class fontdesc  *f,int  i)
{
    (self->cursor)->SetGlyph(f,i);
    if(!(self->cursor)->IsPosted()){
	struct rectangle tr;
	(self)->GetVisualBounds(&tr);
	(self)->PostCursor(&tr,self->cursor);
    }
}
static void MySetStandardCursor(class fadview  *self,short  i)
{
    (self->cursor)->SetStandard(i);
    if(!(self->cursor)->IsPosted()){
	struct rectangle tr;
	(self)->GetVisualBounds(&tr);
	(self)->PostCursor(&tr,self->cursor);
    }
}
static int CurrentFrame(class fadview  *self)
{
    register int i;
    register struct fad_frame *fra;
    for(i = 1,fra = findpic(self)->bf;fra != NULL && fra != self->f ; fra = fra->f)
	i++;
    return i;
}
static class fontdesc *my_DefineFont(const char  *fname)
{
    char familyname[256];
    long fontStyle;
    long  fontSize;
    fontdesc::ExplodeFontName(fname,familyname, sizeof(familyname), &fontStyle, &fontSize);
    return fontdesc::Create(familyname,  fontStyle, fontSize);
}
static void UpdateCursor(class fadview  *self)
{
    class fad *cp;
    static class fontdesc *i12font = NULL;
    cp = findpic(self);
    if(self->HasFocus) {
	if(IsMode(self,LABELMODE))
	    MySetCursor(self,cp->labelfont,'a');
	else if(IsMode(self,ICONMODE)){
	    if(cp->iconpointnow == NULL && (cp)->flipicons() == 0){ MySetStandardCursor(self,Cursor_Arrow);
	    return;
	    }
	    MySetCursor(self,cp->currentfont,cp->currenticon);
	}
#ifdef ANIMATEBUTTONMODE
	else if(IsMode(self,ANIMATEBUTTONMODE)){
	    if(i12font == NULL) i12font = my_DefineFont("icon12");
	    MySetCursor(self,i12font,'L');
/*	    MySetStandardCursor(self,Cursor_Octagon); */
	}
#endif /* ANIMATEBUTTONMODE */
	else if(IsMode(self,BOXMODE))
	    MySetStandardCursor(self,Cursor_CrossHairs);
	else if(IsMode(self,WAITMODE))
	    MySetStandardCursor(self,Cursor_Wait);
	else MySetStandardCursor(self,Cursor_Arrow);
    }
    else MySetStandardCursor(self,Cursor_Arrow);
}

view_DSattributes fadview::DesiredSize(long  width , long  height, enum view_DSpass  pass, long  *desiredwidth , long  *desiredheight)

{
    class fad *cp;
    cp = findpic(this);
    if(pass ==  view_NoSet &&  cp!= NULL && cp->desw > 0 && cp->desh > 0){
	*desiredheight = min(height,cp->desh) ;
	*desiredwidth =  min(width,cp->desw); 
	return( view_WidthFlexible | view_HeightFlexible) ;
    }
    *desiredwidth = width;
    *desiredheight = (height > 2048) ? ((cp) ? cp->desh:STARTHEIGHT) :height;
    return(view_Fixed);
}

static void fontinit(class fad  *cp)
{
    while(cp->initializedfonts < cp->topinmp){
	cp->fontpt[cp->initializedfonts]  = my_DefineFont(cp->inmp[cp->initializedfonts]);
	(cp->initializedfonts)++;
    }
    if(cp->currentfont == NULL && cp->initializedfonts > 1) cp->currentfont = cp->fontpt[cp->initializedfonts - 1] ;
}
void HaltAnimation(class fadview  *self)
{
    if(self->nextevent){
	(self->nextevent)->Cancel();
	self->nextevent = NULL;
    }
    if(self->anobj){
	self->f = self->anobj->sf ;
	if(self->anobj->i != 0){
	    /* redraw the last frame */
	    self->Redraw = TRUE;
/*	    fadview_WantUpdate(self,self); */
	}
	else 	self->Redraw = FALSE;
	(self)->FlushGraphics();
	free(self->anobj->anbuf);
	free(self->anobj);
	self->FrameChanged = TRUE;
	self->anobj = NULL;
	(self)->WantUpdate(self);
    }
}
void	
fadview::Update()
{
    struct fadvector *vc;
    class fad *cp;
    if(this->removed) return;
    (this)->SetTransferMode(graphic_INVERT);
    this->needUpdate = FALSE;
    if(this->anobj) { doan(this->anobj); return ;}
    if(this->DoAnimation){ if(!::DoAnimation(this)) return; }
    cp = findpic(this);
    if(cp->labelfont == NULL) cp->labelfont = my_DefineFont(cp->labelfontname);
    if(cp->topinmp != cp->initializedfonts) fontinit(cp);
    if(this->Redraw){
	clearfad(this);
	for(vc = this->f->v; vc != NULL ; vc = vc->v)
	    vecdraw(this,vc);
    }
    else{
	if(this->FocusChanged)	    
	    (this)->DrawRectSize(0,0, (this)->GetLogicalWidth() -1, (this)->GetLogicalHeight()-1);
	this->Redraw = TRUE;
    }	
    this->FocusChanged = FALSE;
    UpdateCursor(this);
    if(this->FrameChanged){
	if(this->mode != WAITMODE) nameframe(this);
	this->FrameChanged = FALSE;
    }
    if(this->mode == WAITMODE) 
	message::DisplayString(this,0,"PICK DESIRED ICONS AND QUIT SAMPLEFONT");

    (this)->FlushGraphics();
}
void fadview::FullUpdate(enum view_UpdateType  type,long  left,long  top,long  width,long  height)
{
    if(type == view_MoveNoRedraw){
	UpdateCursor(this);
	return;
    } 
    if(type == view_Remove){
	this->removed = TRUE;
	return;
    }
    this->removed = FALSE;
    this->Redraw = TRUE;
    this->FrameChanged = TRUE;
    recalc(this);
    if(this->anobj) {
	/* reset the ongoing animation */
	HaltAnimation(this);
	this->DoAnimation = TRUE;
    }
    (this)->Update();
}
void fadview::aniframe(int  framecount,int  startat,int  gofor,int  mtm)
{
    this->framecount = framecount;
    this->startat = startat;
    this->gofor = gofor;
    this->mtm = mtm * 1000;
    this->DoAnimation = TRUE;
    (this)->WantUpdate(this);
}

void queup(class fadview  *self)
{
	class fad *cp = findpic(self);
	self->nextevent = im::EnqueueEvent((event_fptr)dodoan,(char *)self,event_MSECtoTU(cp->frtime));
}

static void
dodoan(class fadview  *self)
{
    self->nextevent = NULL;
    if(self->anobj){
	if(! self->updatedue){
	    queup(self);
	    (self)->WantUpdate(self);
	}
	self->updatedue = TRUE;
    }
}

static boolean DoAnimation(class fadview  *self)
{

    class fad *cp;
    struct fad_frame *lf,*sf;
    struct fadvector *vc;
    int i;
    float fc;
    struct aniinfo *anobj;
    register struct anivect *ap;
    register struct fadvector *vp,*nvp;
    struct anivect *anbuf,*endanbuf;

    anobj = (struct aniinfo *) malloc(sizeof(struct aniinfo));
    self->DoAnimation = FALSE;
    cp = findpic(self);
    lf = cp->bf;
    if(self->startat != 0) self->startat--;
    for(i = 0 ; i < self->startat && lf != NULL ; i++)
	lf = lf->f;
    sf = lf;
    fc = (float) self->framecount;
    if(lf == NULL || lf->f == NULL){
	if(self->startat == 0) TellUser(self, "at Last Frame");
	else TellUser(self, "Not that many frames");
	return FALSE;
    }
    anbuf = (struct anivect *) malloc(sizeof(struct anivect) * self->anbufsize);
    if(anbuf == NULL) {
	TellUser(self, "Insufficient Memory To Animate");
	return FALSE;
    }
    endanbuf = anbuf + self->anbufsize -1;
    ap = anbuf;
    mloop:	for(vp = lf->v,nvp = lf->f->v;vp != NULL && nvp != NULL;
    vp = vp->v,nvp = nvp->v){
	if((nvp->p1->x == vp->p1->x) && (nvp->p1->y ==vp->p1->y ) && 
	   (nvp->p2->x ==  vp->p2->x) && (nvp->p2->y == vp->p2->y)) {
	    vp->stat = 'C';
	    continue;
	}
	if((ISICONORLABEL(nvp->p2->x) != ISICONORLABEL(vp->p2->x))){
	    /* don't animate icons or labels that turn into 
			     lines or boxes (or visa-versa) */
	     vp->stat = 'D';
	     continue;
	}
	vp->stat = 'N';
	ap->x1 = (float) vp->p1->x;
	ap->y1 = (float) vp->p1->y;
	ap->x2 = (float) vp->p2->x;
	ap->y2 = (float) vp->p2->y;
	ap->dx1 = ((float) nvp->p1->x - ap->x1) / fc;
	ap->dy1 = ((float) nvp->p1->y - ap->y1) / fc;
	ap->dx2 = ((float) nvp->p2->x - ap->x2) / fc;
	ap->dy2 = ((float) nvp->p2->y - ap->y2) / fc;
	ap->label = vp->label;
	ap->mode = vp->mode;
	if(++ap >= endanbuf){
	    /* ran out of room, reset the size we need and restart */
	    self->anbufsize += 64;
	    free(anbuf);
	    self->DoAnimation = TRUE;
	    (self)->WantUpdate(self);
	    return FALSE;
	}
    }
    for(;vp != NULL; vp = vp->v) vp->stat = 'C';
    lf->av = ap;

    if(lf->f->f != NULL && --self->gofor ){
	lf = lf->f;
	goto mloop;
    }
    if(self->f != sf){
	clearfad(self);
	/* graphic_SetTransferMode(self,graphic_INVERT); */
	for(vc = sf->v; vc != NULL ; vc = vc->v)
	    vecdraw(self,vc);
    }
    lf = lf->f;
    anobj->self= self;
    anobj->anbuf = anbuf;
    anobj->eap = anbuf;
    anobj->i = 0;
    anobj->sf = sf;
    anobj->lf = lf;
    self->anobj = anobj;
    doan(anobj);
    queup(self);
    return FALSE;
 /*    */
}
boolean doan(struct aniinfo  *anobj)
{
    class fadview *self;
    struct fad_frame *lf,*sf;
    struct fadvector *vc,*vc1,*ov;
    int i;
    register struct anivect *ap,*eap;


    self= anobj->self;
    eap = anobj->eap;
    lf = anobj->lf;
    sf = anobj->sf;
    i = anobj->i;
    if(i == self->framecount){
	/* advance to next frame */
	i = 0;
	anobj->i = i;
	(self->startat)++;
	for( ap = eap  ;ap != sf->av; ap++)
	    fdraw(ap);
	ov = sf->v;	
	eap = sf->av;
	anobj->eap = eap;
	sf= sf->f;
	anobj->sf = sf;
	/* 	draw any new vectors for next frame */
	for(vc = sf->v,vc1 = ov; vc != NULL && vc1 != NULL; vc = vc->v,vc1 = vc1->v) {
	    if(vc1->stat != 'C') vecdraw(self,vc);	
	    if(vc1->stat == 'D') vecdraw(self,vc1);
	}
	/* 	clear any deleted vectors */
	for(;vc1!= NULL;vc1 = vc1->v)
	    vecdraw(self,vc1);
	for(;vc!= NULL;vc = vc->v)
	    vecdraw(self,vc);
    }
    else i++;
    if(sf == lf){
	/* done */
	HaltAnimation(self);
	return TRUE;
    }
    for( ap = eap  ;ap != sf->av; ap++){
	fdraw(ap);
	ap->x1 += ap->dx1;
	ap->y1 += ap->dy1;
	if(ISICONORLABEL(ap->x2)) {
	    idraw(self,ap);
	}
	else {
	    ap->x2 += ap->dx2;
	    ap->y2 += ap->dy2;
	    ldraw(self,ap);
	}
    }
/*    fprintf(stdout,"drawing %d - %d\n",lf,i); fflush(stdout); */
    if(self->FocusChanged){
	(self)->DrawRectSize(0,0, (self)->GetLogicalWidth() -1, (self)->GetLogicalHeight()-1);
	self->FocusChanged = FALSE;
    }
    (self)->FlushGraphics();
    anobj->lf = lf;
    anobj->i = i;
    self->updatedue = FALSE;
    if (self->nextevent == NULL)
	queup(self);
    return FALSE;
}
void fadview::nextframe(class fad  *cp)
{
    struct fad_frame *lf;
    char resp[64];
    lf = this->f->f;
    if(lf == NULL){
	if(cp->readonly) return;
	if(message::AskForString(this,0,"At end.Create blank frame?[n]",0,resp,63)== -1) 
	    return;
	if(!(*resp == 'y' || *resp == 'Y')) return ;
	lf = (cp)->newframe();
	this->f->f = lf;
    }
    this->f = lf;
    this->FrameChanged = TRUE;
    (this)->WantUpdate(this);
}
void fadview::lastframe(class fad  *cp)
{
    struct fad_frame *lf;
    if(this->f == cp->bf){
	TellUser(this, "At Base Frame");
	return;
    }
    for(lf=cp->bf;lf->f != this->f; lf = lf->f) ;
    this->f  = lf;
    this->FrameChanged = TRUE;
    (this)->WantUpdate(this);
}

static int recalc(class fadview  *self)
{
    struct fad_frame *frr;
    struct fadpoint *pp;
    class fad *cp;
#ifndef NORESHAPE
    float xfac,yfac;
#endif /* NORESHAPE */
    /* 	int xmove,ymove; */
    int wd,ht;  
    cp = findpic(self);
    if(cp->w  == (self)->GetLogicalWidth()  &&  cp->h == (self)->GetLogicalHeight() &&
	cp->ox == 0 && cp->oy == 0)
	return(0);
    if(cp->w == 0 || cp->h == 0) {
	picset(self,0);
	return(1);
    }
#ifdef NORESHAPE
    if(cp->ox == 0 && cp->oy == 0){
	if(cp->fad_square) picset(self,0);
	return(2);
    }
#endif /* NORESHAPE */
    wd = cp->w;
    ht = cp->h;
    if(cp->fad_square){
	ht = min(ht,wd);
	wd = ht;
    }
#ifndef NORESHAPE
    xfac = (float) wd / (float) cp->w ;
    yfac = (float)ht / (float) cp->h ;
#endif /* NORESHAPE */
    for(frr = cp->bf; frr != NULL; frr = frr->f){
	for(pp = frr->p; pp != NULL; pp = pp->p){
	    if(ISICONORLABEL(pp->x)) continue;
#ifdef NORESHAPE
	    pp->x = pp->x - cp->ox/*  + xmove */;
	    pp->y = pp->y - cp->oy /* + ymove */;
#else /* NORESHAPE */
	    pp->x = (long)((float)(pp->x - cp->ox) * xfac) /* + xmove */;
	    pp->y = (long)((float)(pp->y - cp->oy) * yfac) /* + ymove */;
#endif /* NORESHAPE */
	}
    }
    picset(self,1);
    return(2);
}
static void picset(class fadview  *self,int  flag)
{
    class fad *pc = findpic(self);
    if(flag){
	pc->ox = 0 /* self->in.rect.left */;
	pc->oy = 0 /* self->in.rect.top */; 
    }
    if(pc->fad_square){
	pc->w = min((self)->GetLogicalWidth(),(self)->GetLogicalHeight());
	pc->h = pc->w;
    }
    else {
	pc->w  = (self)->GetLogicalWidth();
	pc->h  = (self)->GetLogicalHeight();
    }
}
static void clearfad(class fadview  *self)
{
    (self)->SetTransferMode(graphic_WHITE);
    (self)->EraseVisualRect();	
    (self)->SetTransferMode(graphic_INVERT);
    if(self->HasFocus){
	(self)->DrawRectSize(0,0,(self)->GetLogicalWidth()-1,(self)->GetLogicalHeight()-1);
    }
}

fadview::~fadview()
{
	ATKinit;

    HaltAnimation(this);
    if(this->menulistp) delete this->menulistp;

}

fadview::fadview()
{
	ATKinit;

    this->keystatep = keystate::Create(this, fadviewKeymap);
    this->menulistp = NULL;
    this->cursor = cursor::Create(this);
    this->DoAnimation = FALSE;
    this->HasFocus = 0;
    this->removed = FALSE;
    this->Moving = 0;
    this->anbufsize = 128;
    this->Redraw = TRUE;    
    this->FrameChanged = TRUE;
    this->animationPending = FALSE;
    this->needUpdate = FALSE;
    this->FocusChanged = FALSE;
    this->mode = LINEMODE;
    this->anobj = NULL;
    this->nextevent = NULL;
    THROWONFAILURE( TRUE);
}

static void AddMenus(class fadview  *self,class menulist  *ml,struct proctable_Entry  *menuProc)
{
    int readonly = 0;
    if(self && findpic(self))
	readonly = findpic(self)->readonly;
    if(!readonly) {
	(ml)->AddToML("File,Read Fad~50",menuProc,(long) 'R',0);
	(ml)->AddToML("Icon~5,Next icon~10",menuProc,(long) 'f',0);
	(ml)->AddToML("Icon~5,Previous icon~11",menuProc,(long) 'u',0); 
	(ml)->AddToML("Icon~5,Pick new icons~21",menuProc,(long) 'I',0);
	(ml)->AddToML("Mode~2,Line~1",menuProc,(long) 'N',0);
	(ml)->AddToML("Mode~2,Icon~2",menuProc,(long) 'E',0);
	(ml)->AddToML("Mode~2,Label~3",menuProc,(long) 'L',0);
#ifdef BOXMODEON
	(ml)->AddToML("Mode~2,Box~4",menuProc,(long) 'b',0);
#endif /* BOXMODEON */
#ifdef ANIMATEBUTTONMODE
	(ml)->AddToML("Mode~2,Action Button~5",menuProc,(long) 'B',0);
#endif /* ANIMATEBUTTONMODE */
	(ml)->AddToML("Misc~7,Delete last item~10",menuProc,(long) 'd',0);
	(ml)->AddToML("Misc~7,Delete current frame~11",menuProc,(long) 'D',0);
	(ml)->AddToML("Misc~7,Set Label Font~30",menuProc,(long) 'F',0);
	(ml)->AddToML("Misc~7,Set # animation frames~31",menuProc,(long) '#',0);
	(ml)->AddToML("Misc~7,Set frame delay~32",menuProc,(long) 't',0);
	(ml)->AddToML("Fad~1,Repeat frame~30",menuProc,(long) 'r',0);
    }
    (ml)->AddToML("Fad~1,Pick frame~22",menuProc,(long) 'p',0);
    (ml)->AddToML("Fad~1,Next frame~20",menuProc,(long) 'n',0);
    (ml)->AddToML("Fad~1,Previous frame~21",menuProc,(long) 'l',0);
    (ml)->AddToML("Fad~1,Animate~5",menuProc,(long) 'a',0);
    (ml)->AddToML("Fad~1,Animate portion~6",menuProc,(long) 'A',0);
    /* 	menulist_AddToML(ml,"Fad,Copy last frame",menuProc,(long) 'c'); */
}

void 
fadview::showfad(int  i,class fad  *cp)
{
    register struct fad_frame *fra;
    for(fra = cp->bf;fra != NULL && i > 1 ; fra = fra->f)
	i--;
    if(fra != NULL){
	this->f = fra;
	(this)->WantUpdate((class view *)this);
    }
}

static
void KeyIn(class fadview  *self,long  cr)
{
    char frs[256],fff[256];
    int i,startas = 0,gofor = 0;
    struct fad_frame *fra;
    struct fadvector *vec,*nv;
    struct fadpoint *fp,*lp;
    class fad *cp = findpic(self);
    if(self->anobj) {
	if(cr == 'a' || cr == 'p' || cr == 'P'){
	    HaltAnimation(self);
	    (self)->WantUpdate(self);
	}
	return;
    }
    if(self->mode == WAITMODE)  return;
    switch(cr){
	case 'L':
	    if(IsMode(self,LABELMODE)) 
		SetMode(self,LINEMODE);
	    else
		SetMode(self,LABELMODE);
	    UpdateCursor(self);
	    break;
	case 'F':
	    if(message::AskForString(self,0,"Label Font? ",cp->labelfontname,frs,256)== -1) break;
	    cp->labelfont = my_DefineFont(frs);
	    strcpy(cp->labelfontname,frs);
	    break;
	case 'n':
	    (self)->nextframe(cp);
	    break;
	case 'l':
	    (self)->lastframe(cp);
	    break;
	case 'a':
	case 'H':
	    (self)->aniframe(cp->Frames,0,0,cp->frtime);
	    break;
	case 'm':
	    (self)->aniframe(cp->Frames,0,0,0);
	    break;
	case 't':
	    sprintf(frs,"%d",cp->frtime);
	    sprintf(fff,"%d",cp->frtime);
	    if(message::AskForString(self,0," minimum number of milliseconds between frames> ",fff,frs,256) == -1) break;;
	    cp->frtime = atoi(frs);
	    if(cp->frtime > 1000) cp->frtime = 1000;
	    break;
	case 'A':
	    if(startas < 1) startas = 1;
	    sprintf(frs,"%d",startas);
	    sprintf(fff,"%d",startas);
	    if(message::AskForString(self,0,"Starting frame number ",fff,frs,256)== -1) break;
	    startas = atoi(frs);
	    sprintf(frs,"%d",gofor);
	    sprintf(fff,"%d",gofor);
	    if(message::AskForString(self,0,"number of frames ",fff,frs,256) == -1) break;
	    gofor = atoi(frs);
	    (self)->aniframe(cp->Frames,startas,gofor,cp->frtime);
	    break;
	case '#':
	    /* set speed */
	    sprintf(frs,"%d",cp->Frames);
	    sprintf(fff,"%d",cp->Frames);
	    if(message::AskForString(self,0,"# of animation frames> ",fff,frs,256)== -1) break;
	    if((i = atoi(frs)) > 0)
		cp->Frames = i;
	    break;
	case 'D':
	    /* delete frame */
	    if(cp->readonly) break;
	    (cp)->SetModified();
	    fra = self->f;
	    if(self->f == cp->bf){
		if(self->f->f != NULL){
		    self->f = self->f->f;
		}
		else self->f = (cp)->newframe();
		cp->bf = self->f;
		cp->deleated = fra;
		(cp)->freeframe(fra);
		(self)->WantUpdate((class view *)self);
		(cp)->NotifyObservers(CurrentFrame(self));
		break;
	    }
	    (self)->lastframe(cp);
	    if(self->f == NULL) break;
	    if(self->f->f != NULL)  	self->f->f  = self->f->f->f;
	    cp->deleated = fra;
	    (cp)->freeframe(fra);
	    (cp)->NotifyObservers(CurrentFrame(self));
	    break;
	case 'd':
	    if(cp->readonly) break;
	    (cp)->delvector(self->f);
	    (cp)->SetModified();
	    (cp)->NotifyObservers(CurrentFrame(self));
	    (self)->WantUpdate((class view *)self);
	    break;
#ifdef SQUAREWORKS
	case 'T':
	    cp->fad_square = ! cp->fad_square;
	    break;
#endif /* SQUAREWORKS */
#ifdef NOEZ
	case 'S':
	    /* get file to save */

	    if(message::AskForString(self,0,"File to save",cp->fadname,frs,256) == -1 ||
	       *frs == '\0') break;
	    filetype::CanonicalizeFilename(cp->fadname, frs, sizeof(cp->fadname) - 1);
	case 's':
	    /* save file */
	    if(cp->readonly) break;
	    if(*(cp->fadname) == '\0' || (::f = fopen(cp->fadname,"w")) == NULL) {
		sprintf(frs,"ERROR:Can't Write %s",cp->fadname);
		TellUser(self, frs);
		break;
	    }
	    (cp)->Write(::f,0L,0);
	    fclose(::f);
	    break;
#endif /* NOEZ */
#ifdef BOXMODEON
	case 'b':
	    if(cp->readonly) break;
	    SetMode(self,BOXMODE);
	    UpdateCursor(self);
	    break;
#endif /* BOXMODEON */
	case 'R':
	    if(cp->readonly) break;
/*
	    if(message_AskForString(self,0,"File to read ",cp->fadname,frs,256) == -1 ||
	       *frs == '\0') break;
	    filetype_CanonicalizeFilename(cp->fadname, frs, sizeof(cp->fadname) - 1);
*/
	    if(completion::GetFilename(self,"File to read: ",NULL,frs,256,FALSE,TRUE) == -1)
		break;
	    strcpy(cp->fadname,frs);
	    (self)->fileread(cp->fadname);
	    break;
	case 'p':
	    for(i = 1,fra = cp->bf;fra != NULL && fra != self->f ; fra = fra->f)
		i++;
	    sprintf(frs,"%d",i);
	    sprintf(fff,"%d",i);
	    if(message::AskForString(self,0,"frame #",fff,frs,256) == -1) break;
	    if( (i = atoi(frs)) <= 0) break;
	    self->FrameChanged = TRUE;
	    (self)->showfad(i,cp);
	    break;
	case 'r':
	    if(cp->readonly) break;
	    fra = self->f->f;
	    if(fra == NULL){
		fra = (cp)->newframe();
		self->f->f = fra;
	    }
	    else {
		self->f->f = (cp)->newframe();
		self->f->f->f = fra;
	    }
	    for(vec  = self->f->v; vec != NULL; vec = vec->v){
		fp=(cp)->setpoint(vec->p1->x,vec->p1->y,NEW,self->f->f);
		lp=(cp)->setpoint(vec->p2->x,vec->p2->y,NEW,self->f->f);	
		nv = (cp)->setvector(fp,lp,self->f->f);
		if(vec->label){
		    nv->label = (char *)malloc(strlen( vec->label) + 1);
		    strcpy(nv->label,vec->label);
		}
		nv->mode = vec->mode;
	    }
	    (self)->nextframe(cp);
	    self->Redraw = FALSE;
	    break;
	case 'I':
	    if(cp->readonly) break;
	    SetMode(self,ICONMODE);
	    if( !seticon(self)){
		if(self->mode != WAITMODE) SetMode(self,LINEMODE);
		break;
	    }
	    UpdateCursor(self);
	    break;
	case 'f': /* flip to next icon */
	    if(cp->readonly || cp->currentfont == 0) break;
	    if((cp)->flipicons() == 0) break;
	    self->mode = ICONMODE;
	    UpdateCursor(self);
	    break;
	case 'u': /* flip to last icon */
	    if(cp->readonly || cp->currentfont == 0) break;
	    if((cp)->unflipicons() == 0) break;
	    self->mode = ICONMODE;
	    UpdateCursor(self);
	    break;
	case 'E':	/* Toggle icon mode */
	    if(cp->readonly) break;
	    self->mode = ICONMODE;
	    if(cp->currentfont == 0 && !seticon(self)) {
		if(self->mode != WAITMODE) self->mode = LINEMODE;
		break;
	    }
	    UpdateCursor(self);
	    break;
	case 'P':
	    self->DoAnimation = TRUE;
	    (self->startat)++;
	    (self)->WantUpdate(self);
	    break;
	case 'N':
	    self->mode = LINEMODE;
	    UpdateCursor(self);
	    break;
#ifdef ANIMATEBUTTONMODE
	case 'B':   /* toggle animate mode */
	    self->mode = ANIMATEBUTTONMODE;
	    UpdateCursor(self);
	    break;
#endif /* ANIMATEBUTTONMODE */
#ifdef GROUPMODEWORKS
	case 'g':   /* toggle group mode */
	    self->mode = GROUPMODE;
	    UpdateCursor(self);
	    break;
#endif /* GROUPMODEWORKS */
	default:
	    break;
    }
    return/* ((struct view *)self) */;
}
void
fadview::ReceiveInputFocus()
{
    this->HasFocus = 1;
    this->FocusChanged = TRUE;
    this->Redraw = FALSE;
    (this)->WantUpdate((class view *)this);
    this->keystatep->next = NULL;
    (this)->PostKeyState( this->keystatep);
    (this)->PostMenus(this->menulistp);
}
void
fadview::LoseInputFocus()
{
    this->HasFocus = 0;
    this->FocusChanged = TRUE;
    this->Redraw = FALSE;
    (this)->WantUpdate((class view *)this);
}

static void nameframe(class fadview  *self)
{
    char frs[32];
    sprintf(frs,"at frame #%d",CurrentFrame(self));
    TellUser(self, frs);
}
boolean QueueAnimation(class fadview  *self,enum view_MouseAction  action,long  mousex ,long  mousey)
{
    struct fadpoint *pt;
    struct fadvector *vc;
    class fad *cp = findpic(self);
    if(self->anobj) return(FALSE);
    if(action == view_LeftDown){
	if((pt = (cp)->setpoint(mousex,mousey,OLD,self->f)) != NULL){
	    for(vc = self->f->v; vc != NULL ; vc = vc->v){
		if(vc->p1 == pt && vc->mode == ANIMATEMODE){
		    self->animationPending = TRUE;
		    return TRUE;
		}
	    }
	}
    }
    else if(self->animationPending ) {
	if(action == view_LeftUp) {
	    self->animationPending = FALSE;
	    (self)->aniframe(cp->Frames,0,0,cp->frtime);
	}
	return TRUE;
    }
    return FALSE;
}

	   

class view *fadview::Hit(enum view_MouseAction  action,long  mousex ,long  mousey ,long  numberOfClicks) 
{
    static struct fadpoint ptmp;
    struct fadvector *cv;
    struct fad_frame *Cframe;
    class fad *cpic = findpic(this);
    Cframe = this->f;
    if(QueueAnimation(this,action,mousex,mousey)) return this;
    if(!(this->HasFocus)){
	(this)->WantInputFocus((class view *)this);
	cpic->fp = NULL;
	return( this);
    }
    if(this->anobj) return(this);
    if(cpic->readonly) {
	if(action == view_LeftUp || action == view_RightUp)	
	    (this)->aniframe(cpic->Frames,0,0,cpic->frtime);
	return this;
    }
    if(this->mode == WAITMODE) return this;
    (this)->SetTransferMode(graphic_INVERT);
    switch (action) {
	case view_LeftDown:
	    if(cpic->fp != NULL){
		cpic->fp = NULL;
		cpic->pltnum = 0;
	    }
	    else {
		cpic->fp=(cpic)->setpoint(mousex,mousey,NEW,Cframe);
		cpic->pltnum = 0;
		cpic->lp = NULL;
		/* graphic_SetTransferMode(self,graphic_INVERT); */
	    }
	    break;
	case view_LeftUp:
	    if(cpic->fp == NULL){
		break;
	    }
	    if(cpic->lp) drawlist(this,cpic);
	    cpic->pltnum = 0;
	    if(IsMode(this , LABELMODE)){
		char frs[256];
		int szz;
		if(message::AskForString(this,0,"Label String? ","",frs,256)== -1) break;
		if((szz = strlen(frs)) == 0 ) break;
		cpic->lp = (cpic)->setpoint(LABELFLAG,LABELFLAG,NEW,Cframe);	
		cv = (cpic)->setvector(cpic->fp,cpic->lp,Cframe);
		cv->label = (char *)malloc(szz + 1);
		strcpy(cv->label,frs);
	    }
	    else if(BOXTEST(this)){
		cpic->lp = (cpic)->setpoint(mousex,mousey,NEW,Cframe);
		cv = (cpic)->setvector(cpic->fp,cpic->lp,Cframe);   
		cv->mode = BOXMODE;
	    }
#ifdef ANIMATEBUTTONMODE
	    else if(IsMode(this , ANIMATEBUTTONMODE)){
		cpic->lp = (cpic)->setpoint((cpic)->iconnum("icon12"),'L',NEW,Cframe);
		cv = (cpic)->setvector(cpic->fp,cpic->lp,Cframe);
		cv->mode = ANIMATEMODE;
	    }
#endif /* ANIMATEBUTTONMODE */
	    else if(IsMode(this , ICONMODE)){
		cpic->lp = (cpic)->setpoint(cpic->currentfontindex,cpic->currenticon,NEW,Cframe);	
		cv = (cpic)->setvector(cpic->fp,cpic->lp,Cframe);
	    }
	    else {
		cpic->lp = (cpic)->setpoint(mousex,mousey,NEW,Cframe);
		cv = (cpic)->setvector(cpic->fp,cpic->lp,Cframe);
	    }
	    vecdraw(this,cv);
	    cpic->fp = NULL;
	    cpic->lp = NULL;
	    (cpic)->SetModified();
	    this->Redraw = FALSE;
	    (cpic)->NotifyObservers(CurrentFrame(this));
	    break;
	 case view_RightDown:
	     if(cpic->fp != NULL) cpic->fp = NULL;
	      else if((cpic->fp =(cpic)->setpoint(mousex,mousey,OLD,Cframe)) == NULL){
		  MySetStandardCursor(this,Cursor_Cross );
		  cpic->pltnum = -1;
	      }
	      else {
		  getlist(this,cpic->fp);
		  cpic->lp = cpic->fp;
		  /* graphic_SetTransferMode(self,graphic_INVERT); */
	      }
	     break;
	 case  view_RightUp:
	     if(cpic->fp == NULL){
		 UpdateCursor(this);
		 break;
	     }
	     if(cpic->lp) drawlist(this,cpic);
	     else cpic->lp = &ptmp;
	     this->Redraw = FALSE;
	     if(mousex < 0) {
		 mousex = 0;
	     }
	     if(mousey < 0) {
		 mousey = 0;
	     }
	     cpic->lp->x = mousex;
	     cpic->lp->y = mousey;
	     drawlist(this,cpic);
	     cpic->pltnum = 0;
	     cpic->fp->x = mousex;
	     cpic->fp->y = mousey;
/*	     fadview_WantUpdate(self,(struct view *)self); */
	     cpic->fp = NULL;
	     cpic->lp = NULL;
	     (cpic)->SetModified();
	     (cpic)->NotifyObservers(CurrentFrame(this));
	     break;
	 case view_LeftMovement:
	     if(IsMode(this , ICONMODE) && !BOXTEST(this)) break;
	 case view_RightMovement:
	     if(cpic->pltnum == -1 || cpic->fp == NULL) break;
	     if(mousex < 0 || mousey < 0) break;
	     if(cpic->lp) drawlist(this,cpic);
	     else cpic->lp = &ptmp;
	     cpic->lp->x = mousex;
	     cpic->lp->y = mousey;
	     drawlist(this,cpic);
	     break;
	 default :
	     break;
    }
    return(this);
}
static void drawlist(class fadview  *self,register class fad  *cpic)
{
    register int i;
    static struct fadvector tempvec;
    if(cpic->lp){
	tempvec.mode = self->mode;
	tempvec.p1 = cpic->fp;
	tempvec.p2 = cpic->lp;
	vecdraw(self,&tempvec);
	(self)->MoveTo(cpic->fp->x,cpic->fp->y);
	(self)->DrawLineTo(cpic->fp->x,cpic->fp->y);
    }
    for(i =cpic->pltnum; i--;){
	vecdraw(self,cpic->veclist[i]);
    }
}
static void getlist(register class fadview  *self,register struct fadpoint  *ppt)
{
    register int i = 0;
    register struct fadvector *vc;
    class fad *cpic = findpic(self);
    for(vc = self->f->v; vc != NULL ; vc = vc->v){
	if(vc->p1 == ppt || (vc->p2 == ppt))cpic->veclist[i++] = vc;
    }
    cpic->pltnum = i;
}

static int seticon(class fadview  *self)
{
    char frs[256];
    class fad *cp = findpic(self);
    if(*(cp->cfname) == '\0') strcpy(cp->cfname,"icon12");
    if(message::AskForString(self,0,"Icon Font ",cp->cfname,frs,256) != -1) {
	strcpy(cp->cfname,frs);
	return((self)->geticons(cp->cfname));
    }
    return 0;
}

static void ReadIcons(FILE  *f,class fadview  *self)
{
    register int c;
    class fad *cpic = findpic(self);
    while(( c = getc(f))!= EOF){
	if(c == '\0') break;
	(cpic)->setpoint( (cpic)->iconnum(cpic->cfname), c,NEW,self->f);
	if (!FILE_HAS_IO(f)) break;
    }
    if(c == EOF){
	if(cpic->iconpointend == cpic->iconpoints)
	    /* No icons chosen */
	    self->mode = LINEMODE;
	else self->mode = ICONMODE;
	im::RemoveFileHandler(f);
	self->Redraw = FALSE;
	if(pclose(f) != 0){
	    samplefont_failed = 1;
	    if((self)->geticons(cpic->cfname))
		self->mode = ICONMODE;
	}
	else (self)->WantUpdate(self);
    }
}
int fadview::geticons(char  *s)
{
    char bb[512],samp[512],*cp;
    const char *andrewdir;
    FILE *ff;
    class fad *cpic = findpic(this);
    andrewdir = environ::AndrewDir(NULL);
    sprintf(samp,"%s%s",andrewdir,SAMPLEFONT);
    if(samplefont_failed == 3){
	/* see if there is a samplefont program */
	struct stat st;
	samplefont_failed = 1;
	if((stat(samp,&st) == 0) && (st.st_mode & S_IEXEC)){
	    samplefont_failed = 0;
	}
    }
    if(!samplefont_failed){
	message::DisplayString(this,0,"PICK DESIRED ICONS AND QUIT SAMPLEFONT");
	sprintf(bb,"%s -t %s",samp,s);
	if((ff = popen(bb,"r")) != NULL){
	    this->mode = WAITMODE;
	    im::AddFileHandler (ff, (im_filefptr)ReadIcons, (char *)this, 6);
	    return(0);
	}
	samplefont_failed = TRUE;
    }
    message::DisplayString(this,0,"");
    if(message::AskForString(this,75,"Characters for icons ","",bb,256)== -1) return(0);
    for(cp = bb; *cp; cp++){
	(cpic)->setpoint((cpic)->iconnum(cpic->cfname),(int)*cp,NEW,this->f);
    }
    return((cpic)->flipicons());
}


static void idraw(register class fadview  *self,register struct anivect  *A) 
{
    static char cc;
    class fad *cp = findpic(self);
    (self)->MoveTo((long)A->x1,(long)A->y1);
    if(A->label){
	(self)->SetFont(cp->labelfont);
	(self)->DrawString(A->label,0);
    }
    else {
	(self)->SetFont(cp->fontpt[-((int)(A->x2))]);
	cc = (char)(A->y2);
	(self)->DrawText(&cc,1,0);
    }
}	
static void vecdraw(register class fadview  *self,register struct fadvector  *v)
{
    static char cc;
    register class fad *cp = findpic(self);
    /* graphic_SetTransferMode(self,graphic_INVERT); */
    (self)->MoveTo(v->p1->x,v->p1->y);
    if(v->label){
	(self)->SetFont(cp->labelfont);
	(self)->DrawString(v->label,0);
    }
    else if(ISICON(v->p2->x)){
	(self)->SetFont(cp->fontpt[-(v->p2->x)]);
	cc = v->p2->y;
	(self)->DrawText(&cc,1,0);
    }
    else if(BOXTEST(v)){
	(self)->DrawLineTo(v->p2->x,v->p1->y);
	(self)->DrawLineTo(v->p2->x,v->p2->y);
	(self)->DrawLineTo(v->p1->x,v->p2->y);
	(self)->DrawLineTo(v->p1->x,v->p1->y);
    }
    else {
	(self)->DrawLineTo(v->p2->x,v->p2->y);
    }
}
void
fadview::fileread(char  *fnm)
{
    FILE *ff;
    class fad *cp = findpic(this);
    if((ff = fopen(fnm,"r")) != NULL) {
	class fad *cpf = new fad;
	fprintf(stderr, "Warning WARNING: this may not work...\n"); /* XXX */
	this->SetDataObject(cpf);
	cp->Destroy();
	cp=cpf;
	(cp )->Read(ff, 0L);
	fclose(ff);
	strcpy(cp->fadname,fnm);
    }
}
void
fadview::Print(FILE  *file, const char  *processor,const char  *finalFormat,boolean  topLevel)
{
    class fad *cp;
    register struct fadvector *vc;
    cp = findpic(this);
    BeginTroff(file, cp->desh,this);
    for(vc = this->f->v; vc != NULL ; vc = vc->v)
	PrintVec(cp,vc);
    EndTroff(cp->desh);
}

static char labelfonttype(class fadview  *self)

{
    class fad *cp;
    char *c;
    cp = findpic(self);
    c = cp->labelfontname;
    if(c == NULL || *c ==  '\0' || *c == '.') return('R');
    while(*c != '.' && *c != '\0') c++;
    c--;
    switch(*c){
	case 'i' : return('I');
	case 'b': return('B');
	default:
	    return('R');
    }
}
static int labelfontsize(class fadview  *self)
{
    class fad *cp;
    char *c;
    cp = findpic(self);
    c = cp->labelfontname;
    if(c == NULL || *c ==  '\0' || *c == '.') return(10);
    while(*c != '.' && *c != '\0' ){
	if(*c >= '0' && *c <= '9') return(atoi(c));
	c++;
    }
    return(10);
}

/* 
  * Printing stuff.	
  */

#define ScreenXRes 80			/* screen units per inch */
#define ScreenYRes 80
/* typedef float FLOAT; */
static float fxmul, fymul;		/* fractional screen units to device units */
static float /* xmul,*/ ymul;		/* screen units to device units */
static int npoints = 0;			/* points since last reposition */
static float curx, cury;			/* current position in device units */
static long xorg=0, yorg=0;		/* offsets in fractional screen units */

#define FINISHLINE fprintf(printout,"\n.sp-1\n"), npoints=0;
#define STARTLINE  fprintf(printout,"\\h'%0.4fi'\\v'%0.4fi'",curx,cury);

static FILE *printout = NULL;
static
void PrintVec(class fad  *cp,struct fadvector  *v)
{
    if(v->label) {
	xx_MoveTo((v->p1->x - cp->ox )<<16,(v->p1->y - cp->oy) <<16);
	STARTLINE
	  fprintf(printout,"\\\n%s",v->label);
	FINISHLINE
    }
    else if(BOXTEST(v)){
	xx_MoveTo((v->p1->x - cp->ox )<<16,(v->p1->y - cp->oy) <<16);
	xx_DrawTo((v->p2->x- cp->ox )<<16,(v->p1->y- cp->oy)<<16);
	xx_DrawTo((v->p2->x- cp->ox )<<16,(v->p2->y- cp->oy)<<16);
	xx_DrawTo((v->p1->x- cp->ox )<<16,(v->p2->y- cp->oy)<<16);
	xx_DrawTo((v->p1->x- cp->ox )<<16,(v->p1->y- cp->oy)<<16);
    }
    else if(!ISICONORLABEL(v->p2->x)){
	xx_MoveTo((v->p1->x- cp->ox )<<16,(v->p1->y- cp->oy)<<16);
	xx_DrawTo((v->p2->x- cp->ox )<<16,(v->p2->y- cp->oy)<<16);
    }
}


static
void xx_MoveTo(long  x ,long  y			/* in fractional screen units */)
{
    x -= xorg, y -= yorg;
    if (npoints) FINISHLINE;
    curx = x*fxmul;  cury = y*fymul;
}


static
void xx_DrawTo(long  x ,long  y			/* in fractional screen units */)
{
    x -= xorg, y -= yorg;
    if (npoints==0) STARTLINE;
    fprintf(printout,"\\\n\\D'l %0.4fi %0.4fi'",
	     (x*fxmul)-curx, (y*fymul)-cury);
    if (++npoints>=10) FINISHLINE;
    curx = x*fxmul,  cury = y*fymul;
}
static void BeginTroff(FILE  *file,int  yneed			/* dots per inch */,class fadview  *self)
{

    printout = file;
    /* dimensions */
    /*    xmul = (FLOAT) 1/ScreenXRes;  */
    ymul = (float) 1/ScreenYRes;
    fxmul = 1/(ScreenXRes*65536.0);
    fymul = 1/(ScreenYRes*65536.0);
    /* save old state */
    fprintf(printout,"\n.nr @f \\n(.f\n");		/* font */
    fprintf(printout,".nr @i \\n(.i\n");		/* indent */
    fprintf(printout,".nr @j \\n(.j\n");		/* justification mode */
    fprintf(printout,".nr @l \\n(.l\n");		/* line length */
    fprintf(printout,".nr @s \\n(.s\n");		/* point size */
    fprintf(printout,".nr @u \\n(.u\n");		/* 0=nofill, 1=fill */
    /* establish new state */
    fprintf(printout,".br\n.sp-1\n");
    fprintf(printout,".cs CW 22\n.fi\n.ad b\n");
    fprintf(printout,".ft %c\n",labelfonttype(self));
    fprintf(printout,".ps %d\n",labelfontsize(self));


    /* need */
    fprintf(printout,".ne %.4fi\n",yneed*ymul);

}

void fadview::ObservedChanged(class observable  *changed, long  value)
{
    class fad *cpic = findpic(this);
    if (value == observable_OBJECTDESTROYED)
	(this)->Destroy();
    else {
	if(value == fad_NEWFAD){
	    (this)->showfad(1,cpic);
	    if(this->parent)
		(this->parent)->WantNewSize(this); /* Yes, it really has to be called this way */
	}
	else if(this->f == cpic->deleated || value == CurrentFrame(this))
	    (this)->showfad(value,cpic);
    }
}
void fadview::SetDataObject(class dataobject  *dataObject)
        {
    if (!ATK::IsTypeByName((dataObject)->GetTypeName(), "fad"))  {
	fprintf(stderr, "Incompatible dataobject associated with fadview\n");
	return;
    }

    (this)->view::SetDataObject( dataObject);
    this->f = ((class fad *)dataObject)->f;
    AddMenus(this,fadviewMenulist,FadProc);
    this->menulistp = (fadviewMenulist)->DuplicateML( this);
    }

static void EndTroff(long yneed)
{
    if (npoints) FINISHLINE;
    fprintf(printout,".sp %0.4fi\n",((yneed << 16 ) * fymul));

    /* restore old state */
    fprintf(printout,".ft \\n(@f\n");
    fprintf(printout,".in \\n(@iu\n");
    fprintf(printout,".ad \\n(@j\n");
    fprintf(printout,".ll \\n(@lu\n");
    fprintf(printout,".ps \\n(@s\n");
    fprintf(printout,".ie \\n(@u .fi\n.el .nf\n");
    fprintf(printout,".sp 1\n");

    fflush(printout);

    xorg = 0,  yorg = 0;
}
#if 0
/* this wasn't being used before... maybe it ought to be -rr2b */
void fadview::WantUpdate(class view  *requestor)
        {
    if (this->needUpdate && (class view *)this == requestor) return;
    (this)->view::WantUpdate( requestor);
    if((class view *)this == requestor) this->needUpdate = TRUE;
}
#endif

boolean fadview::InitializeClass()
{
    const char *c;
    char buf[2];

    fadviewMenulist = new menulist;
    fadviewKeymap = new keymap;

    FadProc = proctable::DefineProc("fadview-keys", (proctable_fptr)KeyIn,&fadview_ATKregistry_ ,NULL,"get keyed commands"); 
    for(c = "cnlramIfuEdDsRbBNqP",buf[1] = '\0'; *c; c++){
	*buf = *c;
	(fadviewKeymap)->BindToKey( buf, FadProc, *c);
    }
    samplefont_failed = 3;
    return TRUE;
}
