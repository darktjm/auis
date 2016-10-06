/*          Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1992 - All Rights Reserved
	For full copyright information see:'andrew/doc/COPYRITE'
*/

#include <andrewos.h>
ATK_IMPL("textview.H")
#include <ctype.h>
#include <scroll.H>
#include <dictionary.H>
#include <environ.H>
#include <environment.H>
#include <fontdesc.H>
#include <graphic.H>
#include <keystate.H>
#include <mark.H>
#include <matte.H>
#include <menulist.H>
#include <message.H>
#include <rectlist.H>
#include <style.H>
#include <stylesheet.H>
#include <text.H>
#include <texttroff.H>
#include <view.H>
#include <viewref.H>
#include <im.H>
#include <aaction.H>

#include <tabs.H>
#include <txtvinfo.h>
#include <textview.H>

#include <txtproc.h>
#include <util.h>

static class graphic *pat;

#define TEXT_VIEWREFCHAR '\377'  /* place holder character for viewrefs */
#define textview_MOVEVIEW 99999999

#define NLINES 200    /* This set to keep redisplay from going into an infinite loop if something goes drastically wrong */
#define BX 20
#define BY 5
#define EBX 2
#define EBY 2
#define BADCURPOS -1
#define MAXPARA 200
#define REMOVEDCURSOR 32765
#define TABBASE ((int) 'n')
#define FINESCROLL 7
#define FINEMASK 127 /* 2 ^ FINESCROLL - 1 */
#define FINEGRID 12

static class fontdesc *iconFont = NULL;
static const atom *A_boolean, *A_printoption;

static class keymap *textviewEmacsKeymap;
static class keymap *textviewViInputModeKeymap;
static class keymap *textviewViCommandModeKeymap;
static class menulist *viewMenus;
static boolean initialExposeStyles;
static boolean alwaysDisplayStyleMenus;
static boolean highlightToBorders;

static struct view_printopt textview_printoptels[6]; /* table of contents; index; footnotes/endnotes; swap headers; enumerate contents; two columns */
static struct view_printoptlist textview_printopts = {
    textview_printoptels,
    6,
    NULL /* this will be adjusted to be a pointer to the parent list */
};

#define Text(self) ((class text *) ((self)->dataobject))

ATKdefineRegistry(textview, view, textview::InitializeClass);
static void getinfo(class textview  *self, struct range  *total , struct range  *seen , struct range  *dot);
static long whatisat(class textview  *self, long  numerator , long  denominator);
static void setframe(class textview  *self, long  position , long  numerator , long  denominator);
static void endzone(class textview  *self, int  end, enum view_MouseAction  action);
static void hgetinfo(textview *self, struct range *total, struct range *seen, struct range *dot);
static void hsetframe(textview *self, long position, long numerator, long denominator);
static void hendzone(textview *self, int end, enum view_MouseAction action);
static long hwhatisat(textview *self, long numerator, long denominator);	

#include "shared.h"

static const struct scrollfns scrollInterface = {(scroll_getinfofptr)getinfo, (scroll_setframefptr)setframe, (scroll_endzonefptr)endzone, (scroll_whatfptr)whatisat};
static const struct scrollfns hscrollInterface = {(scroll_getinfofptr)hgetinfo,
  (scroll_setframefptr)hsetframe, (scroll_endzonefptr)hendzone, (scroll_whatfptr)hwhatisat};

textview::textview()
        {
	ATKinit;

    long fontSize = 12;
    char bodyFont[100];
    const char *font;
    long fontStyle = fontdesc_Plain;
    class style *defaultStyle;
    boolean justify;
    const char *editorPtr;

    password=FALSE;
    hitcallback=NULL;
    rows=columns=0;
    currentViewreference=NULL;
    this->displayLength = 0;
    this->hasInputFocus = FALSE;
    this->dot = NULL;
    this->top = NULL;
    this->recsearchpos = NULL;
    this->recsearchvalid = FALSE;
    this->recsearchchild = NULL;
    this->alwaysDrawDot = FALSE;
    this->frameDot = NULL;
    this->force = FALSE;
    this->nLines = 0;
    this->aLines = 0;
    this->lines = NULL;
    this->bx = BX;
    this->by = BY;
    this->ebx = EBX;
    this->eby = EBY;
    this->hasApplicationLayer = FALSE;
    
    show_para_display = FALSE;
    para_width = 0;
    para_width_default = environ::GetProfileInt("LineNumbersWidth", 35);
    if (para_width_default < 0)
	para_width_default = 0;	/* silly to use zero */
    three_d_display = TRUE;	/* init in first update for mono vs. color */
    para_font = NULL;
    para_last_top_pos = -1;	/* invalid */
    para_last_top_linenum = -1;	/* invalid */
    para_hit_callback = NULL;
    para_draw_callback = NULL;
    para_bgcolor = environ::GetProfile("LineNumbersBackground");
    para_fgcolor = environ::GetProfile("LineNumbersForeground");


    this->editor = EMACS;
    this->viMode = COMMAND;
    this->keystate = this->emacsKeystate = keystate::Create(this, textviewEmacsKeymap);
    this->viCommandModeKeystate = keystate::Create(this, textviewViCommandModeKeymap);
    this->viInputModeKeystate = keystate::Create(this, textviewViInputModeKeymap);
    /* Look first at preference, then at shell variables to determine editor */
    /* if VI user, set initial keystate to VI command mode */
    if ( (editorPtr = environ::GetProfile("editor")) != NULL )	
    {
	if ( strlen(editorPtr) >= 2 && !strcmp(editorPtr + strlen(editorPtr) - 2, "vi" ) )
	{
	    this->editor = VI;
	    this->viMode = COMMAND;
	    this->keystate = this->viCommandModeKeystate;
	}
    }
    this->styleMenus = NULL;
    this->menus = (viewMenus)->DuplicateML( this);
    (this->menus)->SetMask( textview_NoMenus);
    this->clearFrom = 32767; /* was 99999999, but clearFrom is a short int */
    this->csxPos = BADCURPOS;
    this->csyPos = BADCURPOS;
    this->cshPos = BADCURPOS;
    this->csbPos = BADCURPOS;
    this->cexPos = BADCURPOS;
    this->ceyPos = BADCURPOS;
    this->cehPos = BADCURPOS;
    this->cebPos = BADCURPOS;
    this->scroll = textview_NoScroll;
    this->scrollLine = 0;
    this->scrollDist = -1;
    this->needUpdate = FALSE;
    this->lastStyleMenuVersion = -1;
    this->atMarker = NULL;
    this->displayEnvironment = NULL;
    this->displayEnvironmentPosition = 0;
    this->exposeStyles = initialExposeStyles;
    this->showColorStyles = -1;		/* Update when view tree is linked. */
    this->cur_bgcolor = NULL;
    this->cur_fgcolor = NULL;
    this->tabWidth = 1;
    this->predrawn = this->prepredrawn = NULL;
    this->predrawnY = this->predrawnX = -1;

    this->extendDirection = textview_ExtendRight;
    
    defaultStyle = new style;
    (defaultStyle)->SetName( "default");

    this->ScreenScaleMul = environ::GetProfileInt("TabScalingMultiplier", 14);
    this->ScreenScaleDiv  = environ::GetProfileInt("TabScalingDivisor", 12);
    this->LineThruFormatNotes =
      environ::GetProfileSwitch("LineThruFormatNotes", FALSE);
    if ((font = environ::GetProfile("bodyfont")) == NULL || ! fontdesc::ExplodeFontName(font, bodyFont, sizeof(bodyFont), &fontStyle, &fontSize)) {
	strcpy(bodyFont, "Andy");
    }
    justify = environ::GetProfileSwitch("justified", TRUE);
    (defaultStyle)->SetFontFamily( bodyFont);
    (defaultStyle)->SetFontSize( style_ConstantFontSize, fontSize);
    (defaultStyle)->AddNewFontFace( fontStyle);
    if (! justify)
        (defaultStyle)->SetJustification( style_LeftJustified);
    (this)->SetDefaultStyle(defaultStyle);

    this->insertStack = NULL;
    this->insertEnvMark = NULL;

    this->pixelsShownOffTop = 0;
    this->pixelsReadyToBeOffTop = 0;
    this->pixelsComingOffTop = 0;
    this->scroll_percent = environ::GetProfileInt ("ScrollPercent", 67);
    this->hz_offset = 0;

    this->cachedlayout = NULL;
    this->cachedlayoutplan = NULL;
    this->cachedplanwid = 0;
    this->cachedplanhgt = 0;

    this->extension = NULL;	// Not used.  Use when experimenting with adding new features.

    THROWONFAILURE( TRUE);
}

void textview::LinkTree(class view  *parent)
{

    int cnt;
    (this)->view::LinkTree( parent);
    if (cur_fgcolor) {
	free(cur_fgcolor);
	cur_fgcolor = NULL;
    }
    if (cur_bgcolor) {
	free(cur_bgcolor);
	cur_bgcolor = NULL;
    }
    if((cnt = dictionary::CountRefs(this)) > 0){
        char **list;
	class viewref **vr;
	class view *view;
	if((list = (char **)malloc(cnt * sizeof(char *))) != NULL){
	    dictionary::ListRefs(this,list,cnt);
	    for(vr = (class viewref **)list; cnt--; vr++){
		view = (class view *) dictionary::LookUp(this,(char *)*vr);
		if(view){
		    (view)->LinkTree( this);
		}
	    }
            free(list);
        }
    }
}

static void DeleteSpots(spotcolor *&p) {
    while(p) {
	spotcolor *np=p->next;
	delete p;
	p=np;
    }
}

static void FreeTextData(class textview  *self)
{
    class text *t = Text(self);

    if(t!=NULL){
	if (self->dot != NULL) {
	    (t)->RemoveMark( self->dot);
	    delete self->dot;
	    self->dot = NULL;
	}
	if (self->top != NULL) {
	    (t)->RemoveMark( self->top);
	    delete self->top;
	    self->top = NULL;
	}
	if (self->recsearchpos != NULL) {
	    (t)->RemoveMark( self->recsearchpos);
	    delete self->recsearchpos;
	    self->recsearchpos = NULL;
	}
	if (self->frameDot != NULL) {
	    (t)->RemoveMark( self->frameDot);
	    delete self->frameDot;
	    self->frameDot = NULL;
	}
	if (self->predrawn != NULL) {
	    (t)->RemoveMark( self->predrawn);
	    delete self->predrawn;
	    self->predrawn = NULL;
	}
	if (self->prepredrawn != NULL) {
	    (t)->RemoveMark( self->prepredrawn);
	    delete self->prepredrawn;
	    self->prepredrawn = NULL;
	}
	if (self->insertEnvMark != NULL) {
	    (t)->RemoveMark( self->insertEnvMark);
	    delete self->insertEnvMark;
	    self->insertEnvMark = NULL;
	}
	if (self->atMarker != NULL) {
	    (t)->RemoveMark( self->atMarker);
	    delete self->atMarker;
	    self->atMarker = NULL;
	}
    }

    if (self->lines != NULL)  {
	int i;

	for(i = 0; i < self->aLines; i++)  {
	    if (t != NULL)
		(t)->RemoveMark( self->lines[i].data);
	    delete self->lines[i].data;
	    DeleteSpots(self->lines[i].spots);
	}
	free(self->lines);
	self->lines = NULL;
	self->nLines = 0;
	self->aLines = 0;
    }
}


textview::~textview()
{
    ATKinit;
    int cnt;
    if((cnt = dictionary::CountRefs(this)) > 0){
	char **list;
	class viewref **vr;
	class view *view;
	if((list = (char **)malloc(cnt * sizeof(char *))) != NULL){
	    dictionary::ListRefs(this,list,cnt);
	    for(vr = (class viewref **)list; cnt--; vr++){
		view = (class view *) dictionary::LookUp(this,(char *)*vr);
		if(view) (view)->Destroy();
		dictionary::Delete(this,(char *)*vr);
		(*vr)->RemoveObserver(this);
	    }
	    free(list);
	}
    }


    FreeTextData(this);

    delete cur_fgcolor;
    delete cur_bgcolor;
    delete this->defaultStyle;
    delete this->emacsKeystate;
    delete this->viInputModeKeystate;
    delete this->viCommandModeKeystate;
    if (this->menus != NULL)  {
	delete this->menus;
    }
    if (this->styleMenus != NULL)  {
	delete this->styleMenus;
    }
    if (this->insertStack != NULL) {
	(this)->ClearInsertStack();
    }
    textview_DestroyPrintingLayout(this);
    textview_DestroyPrintingLayoutPlan(this);
}

static class environment *CheckHidden(class text *self, long pos)
{
    class environment *env=(class environment *)self->rootEnvironment->GetInnerMost(pos);
    while(env!=NULL && env->type==environment_Style) {
	struct style *s=env->data.style;
	if(s && s->IsHiddenAdded()) break;
	env=(struct environment *)env->GetParent();
    }
    if(env && env->type==environment_Style) return env;
    return NULL;
}


static long ReverseNewline(class text *self, long pos)
{
    long tp;
    for (tp = pos-1; tp >= 0; tp--) {
	unsigned char c = self->GetChar(tp);

	if (c  == '\n' || c == '\r') {
	    class environment *env=CheckHidden(self, tp);
	    if(env) {
		tp=env->Eval();
		continue;
	    } else break;
	}
    }
    return tp;
}

boolean textview_PrevCharIsNewline(class text *self, long pos)
{
    class environment *env;
    unsigned char ch;
    while(pos>0) {
	env=CheckHidden(self, pos-1);
	if(env) {
	    pos=env->Eval();
	    continue;
	}
	ch=self->GetChar(pos-1);
	if(ch=='\n') return TRUE;
	else return FALSE;
    }
    return FALSE;			
}

long textview_LineStart(class textview *tv, long curx, long cury, long xs, long ys, long pos, long *lend, long *lheight) {
    struct formattinginfo info;
    long p2=ReverseNewline(Text(tv), pos);
    long height;
    class mark *lob=new mark;
    if(lob==NULL) return pos;
    p2++;
    do {
	lob->SetPos(p2);
	lob->SetLength(0);
	height = tv->LineRedraw( textview_GetHeight, lob, curx, cury, xs, ys, FALSE, NULL, NULL, &info);
	if(lheight) *lheight=height;
	p2=lob->GetPos()+info.lineLength;
	if(pos>=lob->GetPos() && (pos<p2 || (pos==Text(tv)->GetLength() && pos==p2))) {
	    if(lend) *lend=p2;
	    p2=lob->GetPos();
	    delete lob;
	    return p2;
	}
    } while(p2<Text(tv)->GetLength());
    return pos;    
}

static struct linesp {
    long pos;
    long height;
} *lsp=NULL;
static int lspsz=0;

static int EnsureLinesp(int c) {
    lspsz=c+100;
    if(lsp==NULL) lsp=(struct linesp *)malloc(sizeof(struct linesp)*lspsz);
    else lsp=(struct linesp *)realloc(lsp, sizeof(struct linesp)*lspsz);
    if(lsp==NULL) return 1;
    return 0;
}

/* Given the sizes of the future view area as in textview_ExposeRegion compute the first character and pixel offset which would put the given pos extra pixels from the top of the view. */
static long RealScrollBack(class textview *tv, long curx, long cury, long xs, long ys, long pos, long &extra, long &ts, long &lines) {
    long total=0;
    struct formattinginfo info;
    class mark *lob=new mark;
    int i=0;
    long prevlines=0;
    
    lines=0;
    
    if(Text(tv)->GetLength()<pos) pos=Text(tv)->GetLength();
    if(lob==NULL) {
	extra=0;
	return 0;
    }
    if(extra<0) {
	lob->SetPos(pos);
	lob->SetLength(0);
	long height=tv->LineRedraw( textview_GetHeight, lob, curx, cury, xs, ys, FALSE, NULL, NULL, &info);
	delete lob;
	if((-extra)<height) {
	    ts=extra;
	    return pos;
	} else {
	    extra=0;
	    return 0;
	}
    }
    if(pos<1 || lob==NULL) {
	extra=0;
	ts=0;
	delete lob;
	return 0;
    }
    long rp, sp=ReverseNewline(Text(tv), pos-1)+1;

    more:
    total=0;
    i=0;
    rp=sp;
    prevlines=lines;
    do {
	if(i>=lspsz && EnsureLinesp(i)) {
	    delete lob;
	    return pos;
	}
	lob->SetPos(sp);
	lsp[i].height=tv->LineRedraw(textview_GetHeight, lob, curx, cury, xs, ys, FALSE, NULL, NULL, &info);
	lines++;
	total+=lsp[i].height;
	lsp[i].pos=sp;
	i++;
	sp=lob->GetPos()+info.lineLength;
    } while(sp<pos);
    if(total==extra) {
	ts+=total;
	extra=0;
	delete lob;
	return rp;
    } else if(total<extra) {
	ts+=total;
	extra-=total;
	if(rp>0) {
	    pos=rp;
	    sp=ReverseNewline(Text(tv), rp-1)+1;
	    goto more;
	} else {
	    if(extra>0) extra=0;
	    delete lob;
	    return lsp[0].pos;
	}
    } else {
	lines=prevlines;
	while(--i>=0) {
	    extra-=lsp[i].height;
	    ts+=lsp[i].height;
	    lines++;
	    if(extra<0) {
		ts+=extra;
		delete lob;
		return lsp[i].pos;
	    }
	}
	if(extra>0) extra=0;
	delete lob;
	return lsp[i].pos;
    }
    fprintf(stderr, "Internal error in RealScrollback.\n");
    delete lob;
    return rp;
}

static void ComputeRegionHeight(class textview *tv, long curx, long cury, long xs, long ys, long pos, long pos1, long pos2, class view *inset, struct rectangle &hit, long &fheight) {
    struct formattinginfo info;
    long npos;
    /* compute the height of the region between pos1 (pos should be the first character of the logical line containing pos1)  and pos2.  If it is greater than the area which could be drawn then punt.*/
     class mark *lob=new mark;
     hit.top=cury;
     hit.left=0;
     hit.height=0;
     hit.width=xs+2*curx-1;
     fheight=1;
     if(lob==NULL) return;
     fheight=(-1);
     do {
	 lob->SetPos(pos);
	 (lob)->SetLength(0);
	 info.lineIP=0;
	 info.searchinset=inset;
	 long lh;
	 hit.height+=tv->LineRedraw( textview_GetHeight, lob, curx, cury, xs, ys, FALSE, NULL, &lh, &info);
	 if(fheight==-1) fheight=lh;
	 npos=lob->GetPos()+info.lineLength;
	/* if the indicated region is enclosed in this line then we need to compute the region's width. */
	 if(pos1>=pos && pos1<npos && (pos2<pos1 || (pos2>=pos && pos2<npos))) {
	     int i;
	     hit.width=0;
	     for(i=0;i<info.lineIP;i++) {
		 struct lineitem *li=(&info.lineItems[i]);
		 if(li->docPos>pos2 && li->docPos>pos1) break;
		 if((li->type==li_Plain || li->type==li_Expose || li->type==li_Octal)) {		
		     struct textitem *ti=(&li->itemdata.textitem);
		     char *str=(char *)info.lineBuffer+ti->lineBufPos;
		     int len=strlen(str);
		     long sw, sh;
		     if(pos1>=li->docPos && pos1<li->docPos+len) {
			 if(pos1>li->docPos) ti->font->TextSize(tv->GetDrawable(), str, pos1-li->docPos, &sw, &sh);
			 else sw=0;
			 hit.left=
			   li->xPos + info.locateX + sw ;
		     }
		     if(pos2>=pos1 && pos2>=li->docPos && pos2<li->docPos+len) {
			 ti->font->TextSize(tv->GetDrawable(), str, pos2-li->docPos+1, &sw, &sh);
			 hit.width= li->xPos+ info.locateX + sw - hit.left;
			 break;
		     }
		     if(len>0) ti->font->TextSize(tv->GetDrawable(), str, len, &sw, &sh);
		     else sw=0;
		     hit.width= li->xPos + info.locateX + sw - hit.left;
		 } else {
		     if(pos2>=pos1 && li->type==li_View) {
			 struct viewitem *vi=(struct viewitem *)&li->itemdata.viewitem;
			 if(li->docPos==pos1) {
			     if(vi->view==inset && inset!=NULL) {
				 hit.left=
				   info.insetrect.left;
				 hit.top=
				   info.insetrect.top;
				 hit.width=
				   info.insetrect.width;
				 hit.height=
				   info.insetrect.height;
				 break;
			     } else hit.left=li->xPos + info.locateX;
			 } else if(li->docPos==pos2) {
			     hit.width=li->xPos + info.locateX +  vi->width - hit.left;
			 }
		     }
		 }
	     }
	 }
	 pos=npos;
     } while((pos<=pos1 || pos<=pos2) && hit.height<ys);
     delete lob;
}

void textview_ComputeViewArea(class textview *tv, const struct rectangle &area, long &curx, long &cury, long &xs, long &ys) {
    long bx, by;
    by = (tv->hasApplicationLayer) ? tv->by : tv->eby;
    cury=area.top + by;
    bx = (tv->hasApplicationLayer) ? tv->bx : tv->ebx;
    curx = area.left + bx;
    xs = area.width - 2 * bx;
    ys = area.height - 2 * by;
}

long textview_ExposeRegion(class textview *tv, long pos1, long rlen, class view *inset, const struct rectangle &area, struct rectangle &hit, long &off, long extra) {
    if(pos1>Text(tv)->GetLength()) pos1=Text(tv)->GetLength();
    long pos2=pos1+rlen-1; // could be <pos1!
    if(pos2>Text(tv)->GetLength()) pos2=Text(tv)->GetLength();
    long curx, cury, xs, ys;
    long pos;
    long p1;
    long totalscroll=0;
    struct rectangle above;
    long fh=0;
    /* compute the portion of the logical area which would be
     drawn */
    textview_ComputeViewArea(tv, area, curx, cury, xs, ys);
    /* given the computations above compute the first character on the screen lines containing pos1 and pos2, if the textview were drawn with the given width and height. */
    p1=textview_LineStart(tv, curx, cury, xs, ys, pos1);

    ComputeRegionHeight(tv, curx, cury, xs, ys, p1, pos1, pos2, inset, hit, fh);
    
    if(tv->GetTopPosition()<=p1) {
	long dummy;
	if(tv->GetTopPosition()==p1) above.height=0;
	else ComputeRegionHeight(tv, curx, cury, xs, ys+tv->pixelsReadyToBeOffTop, tv->GetTopPosition(), tv->GetTopPosition(), p1-1, NULL, above, dummy);
	if(above.height+hit.height-tv->pixelsReadyToBeOffTop<=ys && hit.top-cury+above.height-tv->pixelsReadyToBeOffTop>=0) {
	    hit.top=above.height + hit.top  - tv->pixelsReadyToBeOffTop;
	    off=tv->pixelsReadyToBeOffTop;
	    return tv->GetTopPosition();
	}
    }
    extra-=hit.top-cury;
    if(fh+extra>=ys) extra=ys-fh-1;
    long lines;
    pos=RealScrollBack(tv, curx, cury, xs, ys, p1, extra, totalscroll, lines);
    hit.top=totalscroll+hit.top;
    off=(-extra);
    return pos;
}

static void HandleSelection(class textview *self, long len, boolean setlen)
{
    struct im *im=self->GetIM();

    if(self->dot->GetLength() && len==0) {
	if(im) im->GiveUpSelectionOwnership(self);
    } else if(len) {
	if(setlen) self->dot->SetLength(len);
	if(im) im->RequestSelectionOwnership(self);
	return;
    }
    if(setlen) self->dot->SetLength(len);
}

void textview::ObservedChanged(class observable  *changed, long  value)
            {
    class view *vself = (class view *) this;
    if (changed == (class observable *) vself->dataobject)  {
	if (value == observable_OBJECTDESTROYED) {
	    if (this->displayEnvironment != NULL) {
		(this)->ReleaseStyleInformation( this->displayEnvironment);
		this->displayEnvironment = NULL;
	    }
	    vself->dataobject = NULL;
	}
	else {
	    if(this->dot && this->dot->GetModified()) {
		this->dot->SetModified(FALSE);
		if(this->dot->GetLength()==0 && this->GetIM()) this->GetIM()->GiveUpSelectionOwnership(this);
	    }
	    (vself)->WantUpdate( vself);
	}
    }
    else if (value == observable_OBJECTDESTROYED &&
           (ATK::IsTypeByName((changed)->GetTypeName(), "viewref"))) {

               class view *vw;
               class viewref *vr = (class viewref *)changed;
               if((vw = (class view *)dictionary::LookUp(this,(char *)vr)) != NULL){
		   (vw)->UnlinkTree();
		   (vw)->Destroy(); 
                   dictionary::Delete(this,(char *)vr);
               }
           }
}

static void scrollforward(class textview *tv, long target);
static void scrollback(class textview *tv,long target);
void textview::WantExposure(class view *requestor, struct rectangle *childrect) {
    long pos;
    class text *t=Text(this);
    for(pos=0;pos<t->GetLength();pos++) {
	pos=t->Index(pos,0xff, t->GetLength()-pos);
	if(pos<0) break;

	class environment *env=(class environment *)t->rootEnvironment->GetInnerMost(pos);
	
	if(env && env->type==environment_View) {
	    class viewref *ref=env->data.viewref;
	    if(ref==NULL) return;

	    class view *vw=(class view *)dictionary::LookUp(this,(char *)ref);
	    if(vw==requestor && vw!=NULL) {
		struct rectangle logical, hit;
		struct rectangle cchild=(*childrect);
		GetLogicalBounds(&logical);
		long curx, cury, xs, ys;
		cchild.left+=vw->GetEnclosedLeft();
		cchild.top+=vw->GetEnclosedTop();
		rectangle_IntersectRect(&cchild, &cchild, &logical);
		if(cchild.top-vw->GetEnclosedTop()!=childrect->top || cchild.height!=childrect->height) {
		    textview_ComputeViewArea(this, logical, curx, cury, xs, ys); 
		    long off=0;
		    long extra=childrect->top + vw->GetEnclosedTop();
		    if(extra+childrect->height>ys) {
			extra=ys - childrect->height -  logical.top - childrect->top;
		    } else extra=(-childrect->top);
		    long p1=textview_ExposeRegion(this, pos, 1, vw, logical, hit, off, extra);
		    SetTopOffTop(p1, off);
		}
	    }
	}
    }
    
}

void textview::WantUpdate(class view  *requestor)
{
#if 1
    if(requestor!=this) {
	for(int i=0;i<nLines;i++) {
	    if(lines[i].containsView) {
		class mark *m=lines[i].data;
		if(m) {
		    class text *t=Text(this);
		    long start=m->GetPos();
		    long end=start+lines[i].nChars;
		    long pos;
		    for(pos=start;pos<end;pos++) {

			pos=t->Index(pos,0xff, end-pos);
			if(pos<0) break;

			class environment *env=(class environment *)t->rootEnvironment->GetInnerMost(pos);

			if(env && env->type==environment_View) {
			    class viewref *ref=env->data.viewref;
			    if(ref==NULL) continue;

			    class view *vw=(class view *)dictionary::LookUp(this,(char *)ref);
			    if(requestor!=NULL) {
				if(vw==requestor || requestor->IsAncestor(vw)) break;
			    }
			}
		    }
		    if(pos>=start && pos<end) {
			lines[i].childwantsupdate=1;
			break;
		    }
		}
	    }
	}
    }
#endif
    if ((class view *)this == requestor && this->needUpdate)
        return;

    if ((class view *)this == requestor)
	this->needUpdate = TRUE;
   (this)->view::WantUpdate( requestor);
}

void textview::SetDataObject(class dataobject  *dataObject)
        {
    if (!ATK::IsTypeByName((dataObject)->GetTypeName(), "text"))  {
	fprintf(stderr, "Incompatible dataobject associated with textview\n");
	return;
    }

    if (this->displayEnvironment != NULL) {
	(this)->ReleaseStyleInformation( this->displayEnvironment);
	this->displayEnvironment = NULL;
    }

    FreeTextData(this);
    this->force = TRUE;

    (this)->view::SetDataObject( dataObject);

    this->dot = ((class text *) dataObject)->CreateMark( 0, 0);
    this->top = ((class text *) dataObject)->CreateMark( 0, 0);
    this->recsearchpos = ((class text *) dataObject)->CreateMark( 0, 0);
    this->recsearchvalid = FALSE;
    this->frameDot = ((class text *) dataObject)->CreateMark( -1, 0);
    /* The frameDot must start at -1 because that is later interpreted
          (near the end of DoUpdate) as meaning that it has never been set */
    this->atMarker = ((class text *) dataObject)->CreateMark( 0, 0);

    this->predrawn = ((class text *) dataObject)->CreateMark( 0, 0);
    this->prepredrawn = ((class text *) dataObject)->CreateMark( 0, 0);
    (this->predrawn)->SetStyle( TRUE, FALSE);
    (this->prepredrawn)->SetStyle( TRUE, FALSE);

    (this->dot)->SetStyle( FALSE, FALSE);
    (this->top)->SetStyle( FALSE, FALSE);
    (this->recsearchpos)->SetStyle( FALSE, FALSE);
    (this->frameDot)->SetStyle( FALSE, FALSE);
    (this->atMarker)->SetStyle( FALSE, FALSE);
    this->lastStyleMenuVersion = -1;
    (this->menus)->SetMask( textview_NoMenus);
    this->movePosition = -1;

    this->insertEnvMark = ((class text *) dataObject)->CreateMark( 0, 0);
    (this->insertEnvMark)->SetStyle( FALSE, FALSE);

    (this)->WantUpdate( this);
}
/* This should be a private method.  It is used for hz scrolling. */
int textview_SizeOfLongestLineOnScreen(textview *tv)
{
    int i, w = 0;
    for (i = 0; i < tv->nLines; i++)
	if (w < tv->lines[i].nPixels)
	    w = tv->lines[i].nPixels;
    return w;
}

void textviewInterface::UpdateRegions(class scroll &scrollview) {
    class ScrollRegion *er=scrollview.Region(scroll_Elevator);
    class ScrollRegion *dr=scrollview.Region(scroll_Dot);
    if(er) {
	struct linedesc *last;
	long tl = (Text(tv))->GetLength();
	long lastpos=tv->GetTopPosition();
	if (tv->nLines > 0) {
	    last = &tv->lines[tv->nLines - 1];
	    lastpos = (last->data)->GetPos();
	    if (((tv)->GetLogicalHeight() - (2*BY)) >= (last->y + last->height)) {
		lastpos += last->nChars;
		if (lastpos > tl) {
		    lastpos = tl;
		}
	    }
	}
	/* Compute horizontal max size...should skip this if hz scrollbars are off (common) */
	int viewwidth = tv->GetLogicalWidth() - ((tv->hasApplicationLayer) ? tv->bx : tv->ebx) * 2;
	int txtwidth = textview_SizeOfLongestLineOnScreen(tv);
	if (txtwidth < viewwidth)
	    txtwidth = viewwidth;

	er->SetRanges(txtwidth,-tv->hz_offset,-tv->hz_offset+viewwidth,Text(tv)->GetLength(), tv->GetTopPosition(), lastpos);

    }
    if(dr) {
	dr->SetRanges(1,0,0, Text(tv)->GetLength(), tv->GetDotPosition(), tv->GetDotPosition()+tv->GetDotLength());
    }
}

void textviewInterface::Absolute(long totalx, long x, long totaly, long y) {
    if(totaly>0) {
	double pos=((double)y/totaly)*(Text(tv)->GetLength());
	long tpos, rpos;
	long lend=0;
	long curx, cury, xs, ys, off, height=0;
	struct rectangle lb;
	if(pos<0.0) pos=0.0;
	tpos=(long)pos;
	tv->GetLogicalBounds(&lb);
	textview_ComputeViewArea(tv, lb, curx, cury, xs, ys);
	rpos=textview_LineStart(tv, curx, cury, xs, ys, tpos, &lend, &height);
	if(y<=0) off=0;
	else if(y>=totaly) {
	    if(height>=ys/2) off=height-ys/2;
	    else off=0;
	} else if(lend-rpos>0) {
	    off=(long)(height *(pos-rpos)/(lend-rpos));
	    if(lend>=Text(tv)->GetLength() && height-off<ys/2) {
		if(height<ys/2) off=0;
		else off=height-ys/2;
	    }
	} else off=0;
	tv->SetTopOffTop(rpos,off);
    }
    if (totalx>0) {
	/* Compute horizontal max size...should skip this if hz scrollbars are off (common) */
	int old_offset = tv->hz_offset;
	int txtwidth = textview_SizeOfLongestLineOnScreen(tv);
	int viewwidth = tv->GetLogicalWidth() - ((tv->hasApplicationLayer) ? tv->bx : tv->ebx) * 2;
	if (txtwidth < viewwidth)
	    txtwidth = viewwidth;

	tv->hz_offset = -txtwidth * x / totalx;
	if (viewwidth - tv->hz_offset > txtwidth)
	    tv->hz_offset = viewwidth - txtwidth;
	if (tv->hz_offset > 0)
	    tv->hz_offset = 0;
	if (old_offset != tv->hz_offset) {
	    tv->force = TRUE;		/* probably could optimize this like v scrolling */
	    tv->WantUpdate(tv);
	}
    }
}

#define SHIFTMIN 20
#define MINSHIFT(dh) ((dh==0)?SHIFTMIN:dh)

void textviewInterface::Shift(scroll_Direction dir) {
    if(tv->nLines<1) return;
    struct linedesc *d;
    if (dir == scroll_Up) {
	long dist=0;
	long newpos=tv->GetTopPosition();
	if(tv->pixelsReadyToBeOffTop) {
	    d=&tv->lines[0];
	    if(tv->pixelsReadyToBeOffTop<MINSHIFT(d->textheight)) {
		tv->scrollLine=0;
		tv->scrollDist=tv->pixelsReadyToBeOffTop;
		tv->SetTopPosition(newpos);
	    }
	    else {
		scrollback(tv,MINSHIFT(d->textheight));
	    }
	}
	else {
	    newpos = tv->MoveBack( newpos, 1, textview_MoveByLines, &dist, 0);
	    struct formattinginfo info;
	    class mark *tob=new mark;
	    if (tob==NULL) return;
	    tob->SetPos(newpos);
	    long th=0;
	    tv->LineRedraw( textview_GetHeight, tob, 0, 0, 0, 0, FALSE, NULL, &th, &info);
	    delete tob;
	    if(dist>th && dist>30) {
		scrollback(tv, th);
	    } else {
	    tv->scrollLine=1;
	    tv->scrollDist=dist;
	    tv->SetTopPosition(newpos);
	}
	}
	/* setframe(tv, newpos<<FINESCROLL, 0, GetLogicalHeight()); */
    } else if(dir==scroll_Down) {
	long height=0;
	d=&tv->lines[tv->nLines-1];
	if(d->y-d->height <= 1) return;	// last line already at top.
	if(d->y+d->height>tv->GetLogicalHeight()) {
	    height=d->y+d->height-tv->GetLogicalHeight();
	    if(height>=MINSHIFT(d->textheight)) {
		scrollforward(tv,MINSHIFT(d->textheight));
		return;
	    }
	}
	{
	    struct formattinginfo info;
	    class mark *tob=new mark;
	    if (tob==NULL) return;
	    tob->SetPos(d->data->GetPos()+d->nChars);
	    long th=0;
	    height = tv->LineRedraw( textview_GetHeight, tob, 0, 0, 0, 0, FALSE, NULL, &th, &info);
	    delete tob;
	    if(height>th && height>30) height=th;
	    scrollforward(tv,height);
	}
    } else if (dir == scroll_Left || dir == scroll_Right) {
	int dx = textview_get_hshift_interval(tv);
	int viewwidth = tv->GetLogicalWidth() - ((tv->hasApplicationLayer) ? tv->bx : tv->ebx) * 2;
	int txtwidth = textview_SizeOfLongestLineOnScreen(tv);
	if (txtwidth < viewwidth)
	    txtwidth = viewwidth;
	if (dir == scroll_Left)
	    tv->hz_offset += dx;
	else
	    tv->hz_offset -= dx;
	if (viewwidth - tv->hz_offset > txtwidth)
	    tv->hz_offset = viewwidth - txtwidth;
	if (tv->hz_offset > 0)
	    tv->hz_offset = 0;
	tv->force = TRUE;
	tv->WantUpdate(tv);
    }
}

void textviewInterface::Extreme(scroll_Direction dir) { 
    if (dir == scroll_Up) {
	tv->SetTopPosition(0);
    } else if(dir == scroll_Down) {
	long len=Text(tv)->GetLength();
	long lend, lheight;
	long curx, cury, xs, ys;
	struct rectangle lb;
	tv->GetLogicalBounds(&lb);
	textview_ComputeViewArea(tv, lb, curx, cury, xs, ys);
	long pos=textview_LineStart(tv, curx, cury, xs, ys, len?len-1:0, &lend, &lheight);
	tv->SetTopOffTop(pos, lheight);
	scrollback(tv,ys/2);
    } else if (dir == scroll_Left) {
	tv->hz_offset = 0;
	tv->force = TRUE;
	tv->WantUpdate(tv);
    } else if (dir == scroll_Right) {
	/* Calc length of longest line on screen. */
	int txtwidth = textview_SizeOfLongestLineOnScreen(tv);
	int viewwidth = tv->GetLogicalWidth() - ((tv->hasApplicationLayer) ? tv->bx : tv->ebx) * 2;
	tv->hz_offset = viewwidth - txtwidth;
	tv->force = TRUE;
	tv->WantUpdate(tv);
    }
}


class view *textview::GetApplicationLayer()
    {
    long scrollpos=scroll_LEFT;
    const char *pos=environ::GetProfile("ScrollbarPosition");
    /* The horizontal scrollbar preference is mainly for testing. */
    boolean hzscroll = environ::GetProfileSwitch("TextHorizontalScrollbar", FALSE);
    class scroll *s;
    this->hasApplicationLayer = TRUE;

    if(pos) {
	if(!strcmp(pos,"right")) scrollpos=scroll_RIGHT;
    } else if(environ::GetProfileSwitch("MotifScrollbars", FALSE)) scrollpos=scroll_RIGHT;

    if (hzscroll)
	scrollpos |= scroll_BOTTOM;

    s=scroll::CreateScroller(this, scrollpos);
    if(s==NULL) return NULL;
    
    return (class view *) s;
}

void textview::DeleteApplicationLayer(class view  *scrollbar)
        {
    class scroll *s=((class scroll *) scrollbar);
    this->hasApplicationLayer = FALSE;
    s->Destroy();
}

static void EnsureSize(class textview  *self, long  lines)
        {
    int i, j;
    long nSize;

    if (lines < self->aLines) return;
    i = (lines > 10) ? (lines<<1) : lines + 2;
    nSize = i * (sizeof(struct linedesc));
    self->lines = (struct linedesc *) ((self->lines != NULL) ? realloc(self->lines, nSize) : malloc(nSize));
    for(j=self->aLines; j<i; j++)  {
	memset(&self->lines[j], 0, sizeof(struct linedesc));
	// self->lines[j].y = 0;
	// self->lines[j].height = 0;
	// self->lines[j].containsView = FALSE;
	self->lines[j].data = (Text(self))->CreateMark(0,0);
	self->lines[j].spots=NULL;
    }
    self->aLines = i;
}


/* Updates the cursor.  If oldcursor is true then it adds the elements to the old rectangle list.  If it is false then it adds them to the new rectangle list and then at the end does an invert rectangle.
*/

static int idebug = 0;
static void InvertRectangles(class textview *self) {
    spotcolor *spots=NULL;
    spotcolor **last=&spots;
    rectangle logical;
    self->GetLogicalBounds(&logical);
    long curx, cury, xs, ys;
    textview_ComputeViewArea(self, logical, curx, cury, xs, ys); 
    for(int i=0;i<self->nLines;i++) {
	linedesc *ld=&self->lines[i];
	spotcolor *p=ld->spots;
	// if(ld->y+ld->height>ys) break;
	while(p) {
	    *last=p;
	    if(idebug && ld->y+ld->height>ys) p->height=ys-ld->y;
	    else p->height=ld->height;
	    p->top=ld->y;
	    p->ox=0;
	    p->oy=0;
	    last=&p->nextgroup;
	    p=p->next;
	}
    }
    *last=NULL;
    rectlist::InvertRectangles(self, spots);
    return;
}

static void UpdateCursor(class textview  *self, boolean  oldCursor)
        {
    int csx, csy, csh, csb, cex, cey, ceh, ceb, by;

    csx = self->csxPos; csy = self->csyPos;
    csh = self->cshPos; csb = self->csbPos;
    cex = self->cexPos; cey = self->ceyPos;
    ceh = self->cehPos; ceb = self->cebPos;

    if (csx == BADCURPOS && cex == BADCURPOS && (((self->top)->GetPos() < (self->dot)->GetPos() || (self->top)->GetPos() >= (self->dot)->GetPos() + (self->dot)->GetLength()) || (!self->hasInputFocus && !self->alwaysDrawDot))) {
	if (oldCursor)
	    rectlist::ResetList();
	else {
	    InvertRectangles(self);
	}
	return;
    }

    /* Handle start and end positions that lie offscreen. */

    if (csx == BADCURPOS)  {
	csx = self->para_width;
	if (cey-ceh > 0)  {
	    /* ending is not on first line, fake a big first line. */

	    csy = cey-ceh;
	    csh = csy;
	    csb = 0;
	}
	else  {
	    /* ending is on first line, put beginning there too */

	    csy = cey;
	    csh = ceh;
	    csb = ceb;
	}
    }

    if (cex == BADCURPOS)  { 
	cex = (self)->GetLogicalWidth() - 1;
	if (csy == (self)->GetLogicalHeight())  {
	    /* beginning is at bottom, put ending there too-> */

	    cey = csy;
	    ceh = csh;
	    ceb = csb;
	}
	else  {
	    /* beginning not at bottom, fake big second line. */

	    cey = (self)->GetLogicalHeight() - 1;
	    ceh = cey-csy;
	    ceb = 0;
	}
    }

    (self)->SetFont(iconFont);
    (self)->SetTransferMode(graphic_XOR);

    by = (self->hasApplicationLayer) ? self->by : self->eby;

    if (oldCursor)
	rectlist::ResetList();

    if (csx != cex || csy != cey)  {
	/* Fix up sometimes bogus x values.  We move them in an extra pixel if necessary to show an empty rectangle. */

	if (csx < self->para_width) csx = self->para_width;
	if (csx > (self)->GetLogicalWidth() - 1) csx = (self)->GetLogicalWidth()-1;
	if (cex < 0) cex = self->para_width;
	if (cex > (self)->GetLogicalWidth() - 1) cex = (self)->GetLogicalWidth() - 1;
	if (cex == 0 && cey != csy)  {
	    ceh = ((cey = (cey - ceh)) == csy) ? csh : cey - csy;
	    cex = (self)->GetLogicalWidth() - 1;
	}
	if (cey <= by)  {
	    self->cexPos = self->csxPos = BADCURPOS;
	    if (oldCursor)
		rectlist::ResetList();
	    else {
		InvertRectangles(self);
	    }
	    return;
	}

	/* Now figure out how to draw it. */

	if (csy == cey)  {
	    /* start and end are on same line */

	    if (oldCursor)
		rectlist::AddOldRectangle(csy, csy - csh, csx, cex);
	    else
		rectlist::AddNewRectangle(csy, csy - csh, csx, cex, 0);
	}
	else  { 
	    /* Box first line */

	    if (oldCursor)
		rectlist::AddOldRectangle(csy, csy - csh, csx, (self)->GetLogicalWidth() - 1);
	    else
		rectlist::AddNewRectangle(csy, csy - csh, csx, (self)->GetLogicalWidth() - 1, 0);

	    if (cey-ceh!=csy)  {
		/* Box middle lines */

		if (oldCursor)
		    rectlist::AddOldRectangle(cey - ceh, csy, self->para_width,(self)->GetLogicalWidth() - 1);
		else
		    rectlist::AddNewRectangle(cey - ceh, csy, self->para_width,(self)->GetLogicalWidth() - 1, 0);
	    }

	    /* Box last line */

	    if (oldCursor)
		rectlist::AddOldRectangle(cey, cey - ceh, self->para_width, cex);
	    else
		rectlist::AddNewRectangle(cey, cey - ceh, self->para_width, cex, 0);
	}
    }
    else if (csx >= self->para_width) {
	(self)->MoveTo(csx,csy-csb);
	(self)->DrawString("|",graphic_NOMOVEMENT);
    }
    if (! oldCursor)  {
	InvertRectangles(self);
	self->csxPos = csx;
	self->csyPos = csy;
	self->cshPos = csh;
	self->csbPos = csb;
	self->cexPos = cex;
	self->ceyPos = cey;
	self->cehPos = ceh;
	self->cebPos = ceb;
    }
}

static void XorCursor(class textview  *self)
    {
    rectlist::ResetList();
    UpdateCursor(self, FALSE);
}

static void CopyLineInfo(class textview  *self, struct linedesc  *newline, struct linedesc  *line, int  movement)
                {
    if (line != newline) {
        (newline->data)->SetPos(  (line->data)->GetPos());
        (newline->data)->SetLength( (line->data)->GetLength());
        (newline->data)->SetStyle( (line->data)->IncludeBeginning(), (line->data)->IncludeEnding());
        newline->nChars = line->nChars;
        (newline->data)->SetModified( FALSE);
        (newline->data)->SetObjectFree( (line->data)->ObjectFree());
        newline->height = line->height;
	// not used: newline->xMax = line->xMax;
	// maybe we should do a view_remove if newline contained a view -rr2b 3/9/95
	newline->containsView = line->containsView;
	newline->childwantsupdate=line->childwantsupdate;
	DeleteSpots(newline->spots);
	newline->spots=line->spots;
	line->spots=NULL;
    }
    newline->y = line->y + movement;
    if (newline->containsView) {
        /* Put code for moving inset here */
        (self)->ViewMove( newline, movement);
    }
}

/* This routine redraws that which must change.  The key concept is this: marks have a modified flag that can be cleared, and which is set whenever the text within a marker is modified.  A view consists of a set of lines and the marks representing the corresponding text.  When we are supposed to update a view, we redraw the lines which correspond to modified marks. *//* 

 *//* For each line that needs to be redrawn, we call the type-specific redrawing routine.  For now, we will call specific routines in the document handler, but this will change soon. */

static void DoUpdate(class textview  *self, boolean  reformat)
{
    class mark *lob, *tob;
    struct linedesc *tl;
    long lobChars;
    int line;
    int textLength = (Text(self))->GetLength();
    int curx, cury, height, force, cStart, cEnd, mStart, mLength, csx;
    int csy, csh, csb, cex, cey, ceh, ceb, ysleft, xs, ys, stopline,redrawline;
    boolean cursorVisible = 0;
    boolean changed;
    int cont;		/* force continuation of redraw */
    /* set to v->scroll if this is a scrolling update */
    enum textview_ScrollDirection scrolling = textview_NoScroll;
    boolean zapRest;
    struct rectangle tempSrcRect;
    struct point tempDstOrigin;	/* Temps for graphics operations */
    struct formattinginfo info;
    long textheight;
    long bx, by;
    style *globalStyle;

    if(self->force) {
	if(self->predrawn) {
	    self->predrawn->SetPos(-1);
	    self->predrawn->SetLength(0);
	}
	if(self->prepredrawn) {
	    self->prepredrawn->SetPos(-1);
	    self->prepredrawn->SetLength(0);
	}
    }

    if (self->show_para_display &&
	self->scroll != textview_ScrollForward &&
	self->scroll != textview_ScrollBackward) {
	long top_pos = self->top->GetPos();
	if (top_pos != self->para_last_top_pos) {
	    /* Top position has moved.  Check the line number. */
	    line = Text(self)->GetLineForPos(top_pos);
	    if (line != self->para_last_top_linenum) {
		/* Line number has changed.  Remember it for next time and
		 * mark all lines as modified so they will be redrawn.
		 * We could force a full redraw, but we might benefit by a bitblt
		 * if this is a partial scroll.
		 */
		self->para_last_top_pos = top_pos;
		self->para_last_top_linenum = line;
		for (line = 0; line < self->nLines; line++)
		    self->lines[line].data->SetModified(TRUE);
	    } else {
		/* Line number is the same even though the top pos has changed. */
		self->para_last_top_pos = top_pos;
	    }
	}
    }

    if (self->lastStyleMenuVersion != Text(self)->styleSheet->version)  {
	/* Have to recompute the size of the tab character */
	class menulist *styleMenus;

	/* InitStateVector will fill in the tabs field
	 with a new tabs object, up to here the state vector is random garbage. */
	text::InitStateVector(&info.sv);

	text::ApplyEnvironment(&info.sv, self->defaultStyle, Text(self)->rootEnvironment);
	info.sv.CurCachedFont = fontdesc::Create(info.sv.CurFontFamily,
						 info.sv.CurFontAttributes, info.sv.CurFontSize);
	info.myWidths = (info.sv.CurCachedFont)->WidthTable( 	    (self)->GetDrawable());
	self->tabWidth = info.myWidths[TABBASE];

	/* Now reset the menus for the new styles */

	if (self->styleMenus != NULL)
	    delete self->styleMenus;
	styleMenus = (((class text *) Text(self))->styleSheet)->GetMenuList(textview_LookCmd, &textview_ATKregistry_ );
	self->styleMenus = (styleMenus)->DuplicateML( self);
	self->lastStyleMenuVersion = ((class text *) Text(self))->styleSheet->version;
	(self->menus)->SetMask( textview_NoMenus);   
	/* we're done with this state vector, all the other
	 calls in this function are being RETURNED state vector information in info.sv, but the tabs field will NOT be valid after this next line */
	text::FinalizeStateVector(&info.sv);
    }

    /* Set background color based on "bgcolor" attribute of the global
     * style--if such an attribute is found.  Otherwise, leave the
     * background color as-is.
     *
     * NOTE:  We should be working with color objects, not color string names here.
     */
    globalStyle = Text(self)->GetGlobalStyle();
    if (globalStyle) {
	char *bgcolor = globalStyle->GetAttribute("bgcolor");
	char *robgcolor, *fgcolor;

	if (Text(self)->GetReadOnly()) {
	    robgcolor = globalStyle->GetAttribute("robgcolor");
	    if (robgcolor)
		bgcolor = robgcolor;
	}
	if (bgcolor && bgcolor != self->cur_bgcolor &&
	    (self->cur_bgcolor == NULL || strcmp(bgcolor, self->cur_bgcolor) != 0)) {
	    self->SetBackgroundColor(bgcolor, 0, 0, 0);
	    if (self->cur_bgcolor)
		free(self->cur_bgcolor);
	    self->cur_bgcolor = strdup(bgcolor);
	    self->force = TRUE;
	}
	fgcolor = globalStyle->GetAttribute("color");
	if (fgcolor && (self->cur_fgcolor == NULL || strcmp(fgcolor, self->cur_fgcolor) != 0)) {
	    /* Set foreground color now so the cursor will pick it up. */
	    self->cur_fgcolor = strdup(fgcolor);
	    self->SetForegroundColor(fgcolor, 0, 0, 0);
	}
    }

    /* Get current bg/fg colors if still undefined. */
    if (self->cur_bgcolor == NULL) {
	const char *s;
	long r, g, b;
	char buf[16];
	self->GetBackgroundColor(&s, &r, &g, &b);
	if (s == NULL) {
	    sprintf(buf, "#%04lx%04lx%04lx", r, g, b);
	    s = buf;
	}
	self->cur_bgcolor = strdup(s);
    }
    if (self->cur_fgcolor == NULL) {
	const char *s;
	long r, g, b;
	char buf[16];
	self->GetForegroundColor(&s, &r, &g, &b);
	if (s == NULL) {
	    sprintf(buf, "#%04lx%04lx%04lx", r, g, b);
	    s = buf;
	}
	self->cur_fgcolor = strdup(s);
    }

    self->needUpdate = FALSE;
    if (iconFont == NULL)
	iconFont = fontdesc::Create("Icon",fontdesc_Plain,12);

    /* copy dimensions out of the view */

    by = cury = (self->hasApplicationLayer) ? self->by : self->eby;
    bx = (self->hasApplicationLayer) ? self->bx : self->ebx;
    curx = bx + self->para_width + self->hz_offset;
    xs = (self)->GetLogicalWidth() - bx - curx;
    ys = (self)->GetLogicalHeight() - 2 * by;

    cury -= self->pixelsReadyToBeOffTop;

    lob = self->top;	/* the first char on the screen */
    lobChars = 0;
    (lob)->SetLength(0);	/* ignore the length part, which can grow at random */
    force = self->force;

    cursorVisible = self->csxPos != BADCURPOS || self->cexPos != BADCURPOS;
    if (self->scroll == textview_ScrollForward || self->scroll == textview_ScrollBackward)  {
	for (line = 0; line < self->nLines && ! ((self->lines[line].data)->GetModified() || self->lines[line].childwantsupdate); line++);
	if (line == self->nLines && line >= 1)  { /* was > */
	    /* At this point we know that only scrolling has occurred and we can just do the bit blt and continue formatting */

	    int sy, dy, h, newLine, movement, lasty, yoff, extendedTop = 0;

	    scrolling = self->scroll;

	    if (self->scroll == textview_ScrollForward)  {
		/* Scrolling toward the end of the document */

		/* Determine area that has to be moved */

		if (self->lines[self->nLines - 1].height > self->lines[self->nLines - 1].textheight) {
		    /* last line contains a view that will probably need a full redraw */
		    stopline = self->nLines - 2;
		    redrawline = stopline + 1;
		    (self->lines[redrawline].data)->SetModified( TRUE); 
		}
		else {
		    stopline = self->nLines - 1;
		    redrawline = -1;
		}

		sy = self->lines[self->scrollLine].y;
		dy = cury;
		if(stopline>=0) lasty = self->lines[stopline].y + self->lines[stopline].height;
		else {
		    lasty=self->lines[redrawline].y;
		}
#if 0
		if (self->ceyPos >= lasty && self->ceyPos == self->csyPos && self->cexPos == self->csxPos)  {
		    /* We are viewing the end of the file with the carat at the end. */

		    lasty = cury + ys;
		}
#endif
		h = lasty - sy;
		movement = sy - dy;

		/* Remove Cursor if it is on the scroll-1 line */

		if (cursorVisible && (self->dot)->GetLength() == 0 && self->scrollLine > 0 && 
		    (self->dot)->GetPos() >= (self->lines[self->scrollLine-1].data)->GetPos() && (self->dot)->GetPos() < (self->lines[self->scrollLine].data)->GetPos()) {
		    (self)->SetTransferMode(graphic_XOR);
		    XorCursor(self);
		}

		(self)->SetTransferMode( graphic_COPY);
		yoff = (dy < 0) ? dy : 0;/* clip */
		rectangle_SetRectSize(&tempSrcRect, 0, sy - yoff, (self)->GetLogicalWidth(), h - yoff );
		point_SetPt(&tempDstOrigin, 0, dy  - yoff );
		if(tempDstOrigin.y<0) {
		    tempSrcRect.height+=tempDstOrigin.y;
		    tempSrcRect.top-=tempDstOrigin.y;
		    tempDstOrigin.y=0;
		}
		if(tempSrcRect.top<0) {
		    tempSrcRect.height+=tempSrcRect.top;
		    tempDstOrigin.y-=tempSrcRect.top;
		    tempSrcRect.top=0;
		}
		if(tempSrcRect.top + tempSrcRect.height>(self)->GetLogicalHeight()) {
		    tempSrcRect.height = (self)->GetLogicalHeight() - tempSrcRect.top;
		}
		if(tempDstOrigin.y + tempSrcRect.height > (self)->GetLogicalHeight()) {
		    tempSrcRect.height = (self)->GetLogicalHeight() - tempDstOrigin.y;
		}
		if (self->show_para_display)
		    self->ClearClippingRect();
		if (tempSrcRect.height <= 0) {
		    /* zero or negative height */
		}
		else {
		    (self)->BitBlt( &tempSrcRect, self, &tempDstOrigin, (struct rectangle *) NULL);
		}

		rectangle_SetRectSize(&tempSrcRect, 0, dy+h, (self)->GetLogicalWidth(), (self)->GetLogicalHeight() - (dy+h));
		pat = (self)->WhitePattern();
		(self)->FillRect( &tempSrcRect, pat);
		if (self->show_para_display) {
		    /* Need to update the bottom of the para number display.
		     * Just update the background...the line redraws will handle the rest.
		     */
		    static char oldbgcbuf[64];	/* jeez we need GC's! */
		    static char oldfgcbuf[64];
		    const char *oldbgcolor, *oldfgcolor;
		    long bcr, bcb, bcg, fcr, fcb, fcg;
		    struct rectangle clipRect;

		    clipRect.left = self->para_width;
		    clipRect.top = 0;
		    clipRect.width = self->GetLogicalWidth()-clipRect.left;
		    clipRect.height = self->GetLogicalHeight();

		    tempSrcRect.top -= 2;
		    tempSrcRect.width = self->para_width-6;
		    tempSrcRect.left = 3;
		    tempSrcRect.height -= by;
		    /* Save old fg/bg colors (need GC's!!!!) */
		    self->GetBackgroundColor(&oldbgcolor,&bcr,&bcb,&bcg);
		    if (oldbgcolor != NULL) {
			strncpy(oldbgcbuf, oldbgcolor,64);
			oldbgcbuf[63] = '\0';
			oldbgcolor = oldbgcbuf;
		    }
		    self->GetForegroundColor(&oldfgcolor,&fcr,&fcb,&fcg);
		    if (oldfgcolor != NULL) {
			strncpy(oldfgcbuf, oldfgcolor,64);
			oldfgcbuf[63] = '\0';
			oldfgcolor = oldfgcbuf;
		    }
		    if (self->three_d_display) {
			/* Draw 3D line number area. */
			if (self->para_bgcolor)
			    self->SetBackgroundColor(self->para_bgcolor, 0, 0, 0);
			pat = self->WhitePattern();
			self->FillRect(&tempSrcRect, pat);
			self->GetDrawable()->SetFGToShadow(shadows_TOPSHADOW);
			/* Draw top shadow. (way at the top, which has been damaged during the scroll) */
			self->MoveTo(tempSrcRect.left-2, by);
			self->DrawLine(tempSrcRect.width+3, 0);
			self->MoveTo(tempSrcRect.left-2, by+1);
			self->DrawLine(tempSrcRect.width+2, 0);
			/* Draw left shadow. (just at the bottom which is blank) */
			self->MoveTo(tempSrcRect.left-2, tempSrcRect.top);
			self->DrawLine(0, tempSrcRect.height+1);
			self->MoveTo(tempSrcRect.left-1, tempSrcRect.top);
			self->DrawLine(0, tempSrcRect.height);
			self->GetDrawable()->SetFGToShadow(shadows_BOTTOMSHADOW);
			/* Draw bottom shadow. */
			self->MoveTo(tempSrcRect.left-1, tempSrcRect.top+tempSrcRect.height+1);
			self->DrawLine(tempSrcRect.width+2, 0);
			self->MoveTo(tempSrcRect.left, tempSrcRect.top+tempSrcRect.height);
			self->DrawLine(tempSrcRect.width+1, 0);
			/* Draw right shadow. (just at the bottom which is blank) */
			self->MoveTo(tempSrcRect.left+tempSrcRect.width, tempSrcRect.top);
			self->DrawLine(0, tempSrcRect.height);
			self->MoveTo(tempSrcRect.left+tempSrcRect.width+1, tempSrcRect.top-1);
			self->DrawLine(0, tempSrcRect.height+1);

			/* Restore old fg/bg colors. */
			self->SetBackgroundColor(oldbgcolor, bcr, bcb, bcg);
			self->SetForegroundColor(oldfgcolor, fcr, fcb, fcg);
		    } else {
			/* Draw a simple line using the line number background color. */
			if (self->para_bgcolor)
			    self->SetForegroundColor(self->para_bgcolor, 0, 0, 0);
			self->MoveTo(tempSrcRect.left+tempSrcRect.width+1, tempSrcRect.top-1);
			self->DrawLine(0, tempSrcRect.height+1);
			if (self->para_bgcolor)
			    self->SetForegroundColor(oldfgcolor, fcr, fcb, fcg);
		    }
		    self->SetClippingRect(&clipRect);
		}

		/* if there was something going off the top, 
		 and now there isn't, clear that little bit of spoog */
		if (self->pixelsReadyToBeOffTop == 0 && self->pixelsShownOffTop != 0) {
		    rectangle_SetRectSize(&tempSrcRect, 0, 0, (self)->GetLogicalWidth(), by);
		    pat = (self)->WhitePattern();
		    (self)->FillRect( &tempSrcRect, pat);
		}

		/* remove scrolled off views */
		for (line = 0; line < self->scrollLine; line++) {
		    if (self->lines[line].containsView) {
			(self)->ViewMove(&self->lines[line], textview_REMOVEVIEW);
			/*                      mark_SetModified(self->lines[line].data, TRUE); */
		    }
		}

		for (newLine = 0, line = self->scrollLine; line <= stopline; line++, newLine++)  {
		    CopyLineInfo(self, &self->lines[newLine], &self->lines[line], -movement);
		}

		self->nLines -= self->scrollLine;

		if (self->csxPos != BADCURPOS)  {
		    extendedTop = (self->csyPos - self->cshPos) < by;
		    if (self->ceyPos > lasty)  {
			self->cehPos -= self->ceyPos - lasty;
			self->ceyPos = lasty;

			rectangle_SetRectSize(&tempSrcRect, 0, lasty, (self)->GetLogicalWidth(), (self)->GetVisualHeight() - lasty);
			pat = (self)->WhitePattern();
			(self)->FillRect(&tempSrcRect, pat);
		    }
		    self->csyPos -= movement;
		    self->ceyPos -= movement;
		    if (self->ceyPos - self->cehPos < dy)
			self->cehPos = self->ceyPos - dy;
		    if (self->ceyPos <= by)  {
			if (extendedTop)  {
			    /* clear out the top part of the selection box */

			    rectangle_SetRectSize(&tempSrcRect, 0, 0, (self)->GetLogicalWidth(), by);
			    pat = (self)->WhitePattern();
			    (self)->FillRect( &tempSrcRect, pat);
			}
			self->csyPos = BADCURPOS;
			self->csxPos = BADCURPOS;
			self->ceyPos = BADCURPOS;
			self->cexPos = BADCURPOS;
		    }
#if 0
		    /* This code seems suspicious... What if there is a partial line extending into the border? -rr2b */
		    else if ((self->csyPos - self->cshPos) < by)  {
			if (! extendedTop)  {
			    /* Just moved into the border, so color it black */
			    rectangle_SetRectSize(&tempSrcRect, 0, 0, (self)->GetLogicalWidth(), by);
			    pat = (self)->BlackPattern();
			    (self)->FillRect( &tempSrcRect, pat);
			}
			self->csyPos = by;
			self->csxPos = 0;
			self->cshPos = by;
		    }
#endif
		}
		if (cursorVisible && redrawline >=0 && (self->dot)->GetPos() >= (self->lines[redrawline].data)->GetPos()){
		    /* if cursor is on the view line that need updated */
		    rectlist::ResetList();
		    self->csyPos = BADCURPOS;
		    self->csxPos = BADCURPOS;
		    self->ceyPos = BADCURPOS;
		    self->cexPos = BADCURPOS;
		}
		cursorVisible = self->csxPos != BADCURPOS || self->cexPos != BADCURPOS;
	    }
	    else  {
		/* Scrolling toward the beginning of the document */

		int lastLine;
		sy = self->lines[0].y;
		movement = self->scrollDist;
		dy = sy + self->scrollDist;
		lasty = (self)->GetLogicalHeight() - by;
		h = 0;

		if(cursorVisible && (self->csyPos<self->scrollDist || self->ceyPos<self->scrollDist || self->csyPos-self->cshPos<self->scrollDist)) {
		   (self)->SetTransferMode(graphic_XOR);
		   XorCursor(self);
		   self->csyPos = BADCURPOS;
		   self->csxPos = BADCURPOS;
		   self->ceyPos = BADCURPOS;
		   self->cexPos = BADCURPOS;
		   cursorVisible=FALSE;
		}
		/* determine the height of the area to raster op and the number of lines that will be moved. */

		for (lastLine = 0; lastLine < self->nLines; lastLine++)  {
		    if (dy + h + self->lines[lastLine].height > lasty) break;
		    h += self->lines[lastLine].height;
		}
		
		(self)->SetTransferMode( graphic_COPY);
		yoff = (sy < 0) ? sy : 0; /* clip */
		rectangle_SetRectSize(&tempSrcRect, 0, sy - yoff, (self)->GetLogicalWidth(), h - yoff);
		point_SetPt(&tempDstOrigin, 0, dy - yoff);
		if(tempDstOrigin.y<0) {
		    tempSrcRect.height+=tempDstOrigin.y;
		    tempSrcRect.top-=tempDstOrigin.y;
		    tempDstOrigin.y=0;
		}
		if(tempSrcRect.top<0) {
		    tempSrcRect.height+=tempSrcRect.top;
		    tempDstOrigin.y-=tempSrcRect.top;
		    tempSrcRect.top=0;
		}
		if(tempSrcRect.top + tempSrcRect.height>(self)->GetLogicalHeight()) {
		    tempSrcRect.height = (self)->GetLogicalHeight() - tempSrcRect.top;
		}
		if(tempDstOrigin.y + tempSrcRect.height > (self)->GetLogicalHeight()) {
		    tempSrcRect.height = (self)->GetLogicalHeight() - tempDstOrigin.y;
		}
		if (self->show_para_display)
		    self->ClearClippingRect();

		(self)->BitBlt( &tempSrcRect, self, &tempDstOrigin, (struct rectangle *) NULL);

		rectangle_SetRectSize(&tempSrcRect, 0, 0, (self)->GetLogicalWidth(), (sy < 0) ? self->scrollDist : dy);
		pat = (self)->WhitePattern();
		(self)->FillRect( &tempSrcRect, pat);

		if (self->show_para_display) {
		    /* Need to update the top of the para number display.
		     * Just update the background...the line redraws will handle the rest.
		     */
		    static char oldbgcbuf[64];	/* jeez we need GC's! */
		    static char oldfgcbuf[64];
		    const char *oldbgcolor, *oldfgcolor;
		    long bcr, bcb, bcg, fcr, fcb, fcg;
		    struct rectangle clipRect;

		    clipRect.left = self->para_width;
		    clipRect.top = 0;
		    clipRect.width = self->GetLogicalWidth()-clipRect.left;
		    clipRect.height = self->GetLogicalHeight();

		    tempSrcRect.width = self->para_width-6;
		    tempSrcRect.top = by+2;
		    tempSrcRect.left = 3;
		    tempSrcRect.height -= by;
		    /* Save old fg/bg colors (need GC's!!!!) */
		    self->GetBackgroundColor(&oldbgcolor,&bcr,&bcb,&bcg);
		    if (oldbgcolor != NULL) {
			strncpy(oldbgcbuf, oldbgcolor,64);
			oldbgcbuf[63] = '\0';
			oldbgcolor = oldbgcbuf;
		    }
		    self->GetForegroundColor(&oldfgcolor,&fcr,&fcb,&fcg);
		    if (oldfgcolor != NULL) {
			strncpy(oldfgcbuf, oldfgcolor,64);
			oldfgcbuf[63] = '\0';
			oldfgcolor = oldfgcbuf;
		    }
		    if (self->three_d_display) {
			/* Draw 3D line number area. */
			if (self->para_bgcolor)
			    self->SetBackgroundColor(self->para_bgcolor, 0, 0, 0);
			pat = self->WhitePattern();
			self->FillRect(&tempSrcRect, pat);
			self->GetDrawable()->SetFGToShadow(shadows_TOPSHADOW);
			/* Draw top shadow. */
			self->MoveTo(tempSrcRect.left-2, tempSrcRect.top-2);
			self->DrawLine(tempSrcRect.width+3, 0);
			self->MoveTo(tempSrcRect.left-2, tempSrcRect.top-1);
			self->DrawLine(tempSrcRect.width+2, 0);
			/* Draw left shadow. (just at the top which is blank) */
			self->MoveTo(tempSrcRect.left-2, tempSrcRect.top);
			self->DrawLine(0, tempSrcRect.height+1);
			self->MoveTo(tempSrcRect.left-1, tempSrcRect.top);
			self->DrawLine(0, tempSrcRect.height);
			self->GetDrawable()->SetFGToShadow(shadows_BOTTOMSHADOW);
			/* Draw right shadow. (just at the top which is blank) */
			self->MoveTo(tempSrcRect.left+tempSrcRect.width, tempSrcRect.top);
			self->DrawLine(0, tempSrcRect.height);
			self->MoveTo(tempSrcRect.left+tempSrcRect.width+1, tempSrcRect.top-1);
			self->DrawLine(0, tempSrcRect.height+1);

			/* Restore old fg/bg colors. */
			self->SetBackgroundColor(oldbgcolor, bcr, bcb, bcg);
			self->SetForegroundColor(oldfgcolor, fcr, fcb, fcg);
		    } else {
			/* Draw a simple line using the line number background color. */
			if (self->para_bgcolor)
			    self->SetForegroundColor(self->para_bgcolor, 0, 0, 0);
			self->MoveTo(tempSrcRect.left+tempSrcRect.width+1, tempSrcRect.top-1);
			self->DrawLine(0, tempSrcRect.height+1);
			if (self->para_bgcolor)
			    self->SetForegroundColor(oldfgcolor, fcr, fcb, fcg);
		    }
		    self->SetClippingRect(&clipRect);
		}

		rectangle_SetRectSize(&tempSrcRect, self->para_width, dy+h, self->GetLogicalWidth()-self->para_width, self->GetLogicalHeight() - (dy + h));
		pat = (self)->WhitePattern();
		(self)->FillRect(&tempSrcRect,pat);

		newLine = (self->aLines > self->nLines)? self->nLines + 1 :self->nLines ;
		self->nLines = self->scrollLine + lastLine;
		EnsureSize(self, self->nLines);

		/* remove scrolled off views */
		for(line = lastLine + 1; line < newLine ; line++)
		    if(self->lines[line].containsView){
			(self)->ViewMove( &self->lines[line], textview_REMOVEVIEW);
		    }

		for (newLine = self->nLines - 1, line = lastLine-1; line >= 0; line--, newLine--)  {
		    CopyLineInfo(self, &self->lines[newLine], &self->lines[line], movement);
		}
		
		if (self->csxPos != BADCURPOS)  {
		    if (self->csyPos - self->cshPos < by)
			self->cshPos = self->csyPos - by;
		    self->csyPos += movement;
		    self->ceyPos += movement;
		    if (self->csyPos > (dy + h))  {
			if (self->csyPos - self->cshPos >= dy + h)  {
			    self->csxPos = BADCURPOS;
			    self->csyPos = BADCURPOS;
			    self->cexPos = BADCURPOS;
			    self->ceyPos = BADCURPOS;
			}
			else  {
			    self->cshPos -= self->csyPos - (dy + h);
			    self->csyPos = dy + h;
			}
		    }
		    if (self->ceyPos > dy + h)  {
			if (self->csxPos != BADCURPOS)  {
			    self->cexPos = (self)->GetLogicalWidth() - 1;
			    self->ceyPos = dy + h;
			    self->cehPos = (self->ceyPos == self->csyPos) ? self->cshPos : (self->ceyPos - self->csyPos);
			}
		    }
		}
	    }
	}
	else  {
	    /* Both scrolling and some text has changed.  Have no choice but to redraw the entire view. */
	    self->force = 1;
	}
    }
    self->scroll = textview_NoScroll;
    self->scrollDist = -1;

    cursorVisible = self->csxPos != BADCURPOS || self->cexPos != BADCURPOS;

    if (reformat && self->force)  {
	/* 	the reason for doing a full-screen clear is that redrawing the
	 screen with one clear is about twice as fast as doing it line by line->  Sigh. */

	self->force = 0;

	/* Zap cursor since it may spill into other insets. */

	if (cursorVisible)  { 
	    XorCursor(self);
	    cursorVisible = FALSE;
	}
	line = 0;
	while (line < self->nLines)  {
	    if(self->lines[line].containsView) {
		(self)->ViewMove( &self->lines[line], textview_REMOVEVIEW);
	    }
	    (self->lines[line++].data)->SetModified( TRUE);
	} 

	(self)->SetTransferMode( graphic_COPY);
	(self)->GetVisualBounds( &tempSrcRect);
	pat = (self)->WhitePattern();
	(self)->FillRect( &tempSrcRect, pat);

	if (self->show_para_display) {
	    static char oldbgcbuf[64];	/* jeez we need GC's! */
	    static char oldfgcbuf[64];
	    const char *oldbgcolor, *oldfgcolor;
	    long bcr, bcb, bcg, fcr, fcb, fcg;
	    struct rectangle clipRect;

	    clipRect.left = self->para_width;
	    clipRect.top = 0;
	    clipRect.width = tempSrcRect.width-clipRect.left;
	    clipRect.height = tempSrcRect.height;

	    /* First clear the view.  We should do this more efficiently. */
	    self->ClearClippingRect();

	    tempSrcRect.width = self->para_width-6;
	    tempSrcRect.left = 3;
	    tempSrcRect.top = by+2;
	    tempSrcRect.height -= 2*by+4;

	    /* Save old fg/bg colors (need GC's!!!!) */
	    self->GetBackgroundColor(&oldbgcolor,&bcr,&bcb,&bcg);
	    if (oldbgcolor != NULL) {
		strncpy(oldbgcbuf, oldbgcolor,64);
		oldbgcbuf[63] = '\0';
		oldbgcolor = oldbgcbuf;
	    }
	    self->GetForegroundColor(&oldfgcolor,&fcr,&fcb,&fcg);
	    if (oldfgcolor != NULL) {
		strncpy(oldfgcbuf, oldfgcolor,64);
		oldfgcbuf[63] = '\0';
		oldfgcolor = oldfgcbuf;
	    }
	    if (self->three_d_display) {
		/* Draw 3D line number area. */
		if (self->para_bgcolor)
		    self->SetBackgroundColor(self->para_bgcolor, 0, 0, 0);
		else if (bcr == 65535 && bcb == 65535 && bcg == 65535) {
		    self->SetBackgroundColor("gray75", 0, 0, 0);	/* White looks bad. */
		    self->para_bgcolor = "gray75";
		} else if (bcr == 0 && bcb == 0 && bcg == 0) {
		    self->SetBackgroundColor("gray25", 0, 0, 0);	/* Black looks bad. */
		    self->para_bgcolor = "gray25";
		}
		pat = self->WhitePattern();
		self->FillRect(&tempSrcRect, pat);
		self->GetDrawable()->SetFGToShadow(shadows_TOPSHADOW);
		/* Draw top shadow. */
		self->MoveTo(tempSrcRect.left-2, tempSrcRect.top-2);
		self->DrawLine(tempSrcRect.width+3, 0);
		self->MoveTo(tempSrcRect.left-2, tempSrcRect.top-1);
		self->DrawLine(tempSrcRect.width+2, 0);
		/* Draw left shadow. */
		self->MoveTo(tempSrcRect.left-2, tempSrcRect.top);
		self->DrawLine(0, tempSrcRect.height+1);
		self->MoveTo(tempSrcRect.left-1, tempSrcRect.top);
		self->DrawLine(0, tempSrcRect.height);
		self->GetDrawable()->SetFGToShadow(shadows_BOTTOMSHADOW);
		/* Draw bottom shadow. */
		self->MoveTo(tempSrcRect.left-1, tempSrcRect.top+tempSrcRect.height+1);
		self->DrawLine(tempSrcRect.width+2, 0);
		self->MoveTo(tempSrcRect.left, tempSrcRect.top+tempSrcRect.height);
		self->DrawLine(tempSrcRect.width+1, 0);
		/* Draw right shadow. */
		self->MoveTo(tempSrcRect.left+tempSrcRect.width, tempSrcRect.top);
		self->DrawLine(0, tempSrcRect.height);
		self->MoveTo(tempSrcRect.left+tempSrcRect.width+1, tempSrcRect.top-1);
		self->DrawLine(0, tempSrcRect.height+1);

		/* Restore old fg/bg colors. */
		self->SetBackgroundColor(oldbgcolor, bcr, bcb, bcg);
		self->SetForegroundColor(oldfgcolor, fcr, fcb, fcg);
	    } else {
		/* Draw a simple line using the line number background color. */
		if (self->para_bgcolor)
		    self->SetForegroundColor(self->para_bgcolor, 0, 0, 0);
		self->MoveTo(tempSrcRect.left+tempSrcRect.width+1, tempSrcRect.top-1);
		self->DrawLine(0, tempSrcRect.height+1);
		if (self->para_bgcolor)
		    self->SetForegroundColor(oldfgcolor, fcr, fcb, fcg);
	    }
	    self->SetClippingRect(&clipRect);
	}
    }

    cStart = (self->dot)->GetPos();
    cEnd = cStart + (self->dot)->GetLength();
    (self)->SetTransferMode( graphic_COPY);
    csx = BADCURPOS;
    csy = BADCURPOS;
    cex = BADCURPOS;
    cey = BADCURPOS;
    csh = 0; csb = 0; ceh = 0; ceb = 0;
    ysleft = ys + self->pixelsReadyToBeOffTop;
    zapRest = TRUE;
    line = 0;
    cont = 0;

    while (line < NLINES) {	/* for each potential line on the screen */
	EnsureSize(self, line);
	tl = &(self->lines[line]);
	tob = tl->data;
	
	/* move next mark to follow last redisplayed line, if necessary
	    this sets the modified flag on the mark iff a change was performed
	    inline expansion of doc_makefollow(lob, tob) */

	if (reformat && (tob)->GetPos() != (lob)->GetPos() + lobChars)  {
	    /*            if(tl->containsView)
		textview_ViewMove(self,tl,textview_REMOVEVIEW); */
	    (tob)->SetPos( (lob)->GetPos() + lobChars);
	    (tob)->SetLength( 0);
	    (tob)->SetModified( 1);
	}

	/* next check to see if the line must be redrawn */

	if (reformat &&
	    ((cont && scrolling != textview_ScrollBackward)
	     || line >= self->nLines
	     || (tob)->GetModified()
	     || cury != tl->y
	     || (scrolling == textview_ScrollBackward
		 && (line < self->scrollLine
		     || (line == self->scrollLine && self->pixelsShownOffTop!=0))))) {

	    /* zap cursor if need to */
	    if (cursorVisible && self->ceyPos >= cury &&
		(! scrolling || (self->csxPos == self->cexPos &&
				 self->csyPos == self->ceyPos &&
				 self->ceyPos == self->lines[self->nLines - 1].y +
				 self->lines[self->nLines - 1].height))) {
		XorCursor(self);
		cursorVisible = FALSE;
	    }

	    tl->childwantsupdate=FALSE;
	    DeleteSpots(tl->spots);
	    info.searchinset=NULL;
	    height = (self)->LineRedraw( textview_FullLineRedraw, tob, curx, cury, xs, ysleft, force, &cont, &textheight, &info, &tl->spots);
	    tl->containsView = (info.foundView != NULL);
	    tl->height = height;	/* set the new length */
	    tl->textheight = textheight;
	    tl->nChars = info.lineLength;
	    tl->y = cury;
	    tl->nPixels = info.continued ? info.fullWidth : info.totalWidth;
	    ysleft -= height;
	    (tob)->SetModified(0);	/* clear the mod flag */
	}
	else  {
	    ysleft -= tl->height;
	    cont = 0;
	}
	mStart = (tob)->GetPos();
	mLength = tl->nChars;

	if (reformat && cury + tl->textheight > ys + by)  {
	    cury += tl->height;
	    zapRest = FALSE;
	    break;
	} 
	cury += tl->height;

	/* Now do the cursor computations.  Note that we try to
	      avoid recomputing the other end of the cursor if it is the same
		  point on the screen. */

	if (cStart >= mStart 
	    && (cStart < mStart+mLength 
		|| mStart + mLength == textLength) 
	    && (self->hasInputFocus || self->alwaysDrawDot))  {
	    if (csx < 0)  {
		csx = (self)->LineRedraw( textview_GetCoordinate, tob, 
					  curx, cury, xs, ys, cStart, NULL,NULL, &info);

		if (csx<0 && cEnd!=cStart) 
		    csx= (self)->GetLogicalWidth(); /*RSK92add*/

		/* If we seem to be at the beginning of the line, and we aren't */
		/* going to draw a caret, adjust back to the inset boundary. */
		/* I hate this. wjh */
		/* oh, yeah? Well, I hate it not being there. Now it's a preference. */
		if (highlightToBorders && cStart == mStart && cEnd != cStart)
		    csx = self->para_width;

		csy = cury - (info.below - info.lineBelow);
		csb = info.lineBelow;
		csh = tl->height - (info.below - info.lineBelow);
	    }
	}
	if (cEnd != cStart && (self->hasInputFocus || self->alwaysDrawDot))  {
	    if (cEnd >= mStart 
		&& (cEnd < mStart+mLength 
		    || mStart + mLength == textLength))  {
		if (cex < 0)  {
		    cex = (self)->LineRedraw( textview_GetCoordinate, tob,
					      curx, cury, xs, ys, cEnd,NULL,NULL, &info);

		    if (cex<0) cex= (self)->GetLogicalWidth(); /*RSK92add*/

		    /* If we seem to be at the beginning of the line, and we aren't */
		    /* going to draw a caret, adjust back to the inset boundary. */
		    /* I hate this. wjh */
		    /* oh, yeah? Well, I hate it not being there. Now it's a preference. */
		    if (highlightToBorders && cEnd == mStart && cEnd != cStart)
			cex = self->para_width;

		    cey = cury - (info.below - info.lineBelow);
		    ceb = info.lineBelow;
		    ceh = tl->height - (info.below - info.lineBelow);
		}
	    }
	}
	else  {
	    cex=csx;
	    cey=csy;
	}

	/* markatend is checked here, since one null line should be redrawn in all
	 redisplays, since this null line could contain the cursor, should it be at the end of the buffer. */

	line++;
	if ((tob)->GetPos() >= textLength) {
	    break;
	}
	lob = tob;		/* last mark object */
	lobChars = self->lines[line-1].nChars;
    }
    if (reformat && (self->zapRest = zapRest))  {
	/* consider zapping the rest of the screen */

	if (cury < self->clearFrom)  {
	    (self)->SetTransferMode( graphic_COPY);
	    rectangle_SetRectSize(&tempSrcRect, self->para_width, cury, (self)->GetLogicalWidth(), (self)->GetLogicalHeight() - cury);
	    pat = (self)->WhitePattern();
	    (self)->FillRect(&tempSrcRect,pat);
	    if (self->show_para_display) {
		/* Clean out the paragraph numbers to the end. */
		tempSrcRect.width = self->para_width-6;
		tempSrcRect.left = 3;
		tempSrcRect.height = tempSrcRect.height - 2 - ((self->hasApplicationLayer) ? self->by : self->eby);
		if (self->three_d_display) {
		    static char oldfgcbuf[64];
		    const char *oldfgcolor;
		    long fcr, fcb, fcg;
		    self->GetForegroundColor(&oldfgcolor,&fcr,&fcb,&fcg);
		    if (oldfgcolor != NULL) {
			strncpy(oldfgcbuf, oldfgcolor,64);
			oldfgcbuf[63] = '\0';
			oldfgcolor = oldfgcbuf;
		    }
		    if (self->para_bgcolor)
			self->SetForegroundColor(self->para_bgcolor, 0, 0, 0);
		    self->FillRect(&tempSrcRect,pat);
		    self->SetForegroundColor(oldfgcolor,fcr,fcb,fcg);
		} else
		    self->FillRect(&tempSrcRect,pat);
	    }
	}
    }
    self->clearFrom = cury;

    self->nLines = line;

    /* now draw the cursor */

    changed =  ! (self->csxPos == csx && self->csyPos == csy
		   && self->cshPos == csh && self->csbPos == csb
		   && self->cexPos == cex && self->ceyPos == cey
		   && self->cehPos == ceh && self->cebPos == ceb);

    if (changed || ! cursorVisible)  {
	if (cursorVisible)  {
	    UpdateCursor(self, TRUE);
	}
	else
	    rectlist::ResetList();
	self->csxPos = csx;
	self->csyPos = csy;
	self->cshPos = csh;
	self->csbPos = csb;
	self->cexPos = cex;
	self->ceyPos = cey;
	self->cehPos = ceh;
	self->cebPos = ceb;
	UpdateCursor(self, FALSE);
    }

    self->displayLength = textLength;

    /* now check if we're supposed to force the cursor to be visible
		     contains expansion of doc_setmarkpos */

    self->pixelsShownOffTop = self->pixelsReadyToBeOffTop;
    if ((line = (self->frameDot)->GetPos()) != -1) {
	/* line is now the position to start back from */

	if (! (self)->Visible( line)) {
	    (self->frameDot)->SetPos( -1);		/* prevent recursive loops */
	    struct rectangle logical, hit;
	    self->GetLogicalBounds(&logical);
	    long off, extra=(self->GetLogicalHeight() * (100-self->scroll_percent))/100;
	    long pos=textview_ExposeRegion(self, line, 1, NULL, logical, hit, off, extra);
	    self->SetTopOffTop(pos, off);
#if 0
	    if(self->pixelsReadyToBeOffTop) {
		self->SetTopPosition(line);
	    }
	    t = (self)->MoveBack( line, (self->GetLogicalHeight()*(100-self->scroll_percent))/100,
				  textview_MoveByPixels, 0, 0);			/* our new top */
	    (self)->SetTopOffTop( t, self->pixelsComingOffTop);			/* adjust the frame */
#endif
	}
	else {
	    (self->frameDot)->SetPos( -1);
	}
    }

    /* Not the greatest place for this but... */                                             
    if (self->hasInputFocus) {

	long mask;
	boolean readonly = ((self)->GetDotPosition() < (Text(self))->GetFence()) || (Text(self))->GetReadOnly();

	mask = (((self->dot)->GetLength() > 0) ? textview_SelectionMenus : textview_NoSelectionMenus) |
	  (readonly ? 0 : textview_NotReadOnlyMenus);

	if ((self->menus)->SetMask( mask)) {
	    if (readonly || !(alwaysDisplayStyleMenus || (mask & textview_SelectionMenus)))
		(self->menus)->UnchainML( textview_StyleMenusKey);
	    else
		(self->menus)->ChainBeforeML( self->styleMenus, textview_StyleMenusKey);
	    (self)->PostMenus( self->menus);
	}
    }
}

void textview::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
{
    long dotpos;
    long line = 0;
    if (this->showColorStyles == -1) {
	/* We defer this until now because we want the default to
	 * depend on the display class, which is not known until
	 * this point.
	 */
	this->showColorStyles = environ::GetProfileSwitch("showcolorstyles",
				!((this)->DisplayClass() & graphic_Monochrome));
	three_d_display = environ::GetProfileSwitch("LineNumbers3D",
				 !(DisplayClass() & graphic_Monochrome));
    }
    switch(type){
	case view_Remove:
	    while (line < this->nLines)  {
		if(this->lines[line].containsView)
		    (this)->ViewMove(&this->lines[line],textview_REMOVEVIEW); 
		line++;
	    } 
	    break;
	case view_MoveNoRedraw:
	    while (line < this->nLines)  {
		if(this->lines[line].containsView)
		    (this)->ViewMove(&this->lines[line],textview_MOVEVIEW); 
		line++;
	    }
	    break;
	default:
	    this->force = TRUE;
	    if ((this)->Visible( (dotpos = (this)->GetDotPosition())))  {
		(this)->FrameDot( dotpos); 
	    }
	    this->csxPos = BADCURPOS;		/* Indication that cursor is not visible */
	    this->cexPos = BADCURPOS;
	    DoUpdate(this, TRUE);
    }
}

void textview::WantNewSize(class view  *requestor)
{
    this->force = TRUE;
    this->csxPos = BADCURPOS;		/* Indication that cursor is not visible */
    this->cexPos = BADCURPOS;
    if((class view *)this!=requestor) { /* if a child wants a new size, just redraw */
	(this)->WantUpdate( this);
    } else (this)->view::WantNewSize( requestor); /* if self wants a new size pass up the request */
}

void textview::Update()
    {
    DoUpdate(this, TRUE);
}

class view *textview::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
                    {
    long newPos, oldPos, oldLen, oldMid;
    class view *vptr;
    static int leftStartPos;
    static int rightStartPos;
    int redrawCursor = 0;
    long newLeftPos ;
    long newRightPos;

    newPos = (this)->Locate(x,y, &vptr);
    oldPos = (this->dot)->GetPos();
    oldLen = (this->dot)->GetLength();
    oldMid = oldPos + (oldLen>>1);

    if (x < para_width && show_para_display) {
	if ( para_hit_callback ) {
	    /* Call user callback. */
	    int linenum = FindLineNumber(newPos);
	    if (linenum >= 0) {
		/* Should always get here...otherwise how did they click here? */
		(*para_hit_callback)(this, lines[linenum].data, action, x, y, numberOfClicks, para_hit_rock);
		return this;
	    }
	}
	/* By default simulate a triple click with the left button (line select). */
	if (action == view_LeftDown)
	    numberOfClicks = 3;
    }

    if(hitcallback) {
        avalueflex out;
        (*hitcallback)(GetDataObject(), avalueflex()|(ATK *)this|(long)action|newPos|numberOfClicks|(ATK *)vptr|x|y, out);
        if(out.GetN()==0 || out[0].Integer()==0) return this;
    }
    if (vptr != NULL)
	    return (vptr)->Hit( action, (vptr)->EnclosedXToLocalX( x), 
			(vptr)->EnclosedYToLocalY( y), numberOfClicks);
    if (action == view_UpMovement) return NULL;

    if (action == view_LeftDown || action == view_RightDown)  {
 	if (! this->hasInputFocus)
	    (this)->WantInputFocus( this);
	
	(this)->GetClickPosition( newPos, numberOfClicks, action, oldPos, oldPos + oldLen, &newLeftPos, &newRightPos);
	
	if (action == view_LeftDown)  {
	    (this->dot)->SetPos( newLeftPos);
	    HandleSelection(this,newRightPos - newLeftPos, TRUE);
	    /* (this->dot)->SetLength( newRightPos - newLeftPos); */
	    leftStartPos = newLeftPos;
	    rightStartPos = newRightPos;
	    this->extendDirection = textview_ExtendRight;
	}
	else  {
	    int lPos = oldPos;
	    int rPos = oldPos + oldLen;
	    
	    if (numberOfClicks == 1)  {
		if (newPos < oldMid)  {
		    leftStartPos = rightStartPos = rPos;
		    lPos = newLeftPos;
		}
		else  {
		    leftStartPos = rightStartPos = lPos;
		    rPos = newRightPos;
		}
	    }
	    else  {
		if (newPos < leftStartPos)
		    lPos = newLeftPos;
		else if (newPos >= leftStartPos)    /* with multiple right clicks left and right start pos are the same */
		    rPos = newRightPos;
	    }
	    if (lPos < oldPos)
		this->extendDirection = textview_ExtendLeft;
	    else
		this->extendDirection = textview_ExtendRight;
		
	    (this->dot)->SetPos(lPos);
	    HandleSelection(this, rPos - lPos, TRUE);
	    /* (this->dot)->SetLength( rPos - lPos); */
	}
    }
    else {
	if (action == view_LeftMovement || action == view_RightMovement || (numberOfClicks == 1 && (action == view_LeftUp || action == view_RightUp)))  {
	    int lPos = leftStartPos;
	    int rPos = rightStartPos;

	    (this)->GetClickPosition( newPos, numberOfClicks, action, leftStartPos, rightStartPos, &newLeftPos, &newRightPos);
	    redrawCursor = FALSE;

	    if (newPos <= leftStartPos)
		lPos = newLeftPos;
	    else if (newPos >= rightStartPos)  
		rPos = newRightPos;

	    if(rPos<lPos) {
		int t=rPos;
		rPos=lPos;
		lPos=t;
	    }
	    
	    if(action==view_LeftUp || action==view_RightUp) HandleSelection(this, rPos - lPos, FALSE);
	    
	    if (lPos != oldPos || rPos != oldPos + oldLen)  {
		(this->dot)->SetPos( lPos);
		(this->dot)->SetLength( rPos - lPos);
		redrawCursor = TRUE;
	    }
	    if (lPos < oldPos)
		this->extendDirection = textview_ExtendLeft;
	    else if (rPos > oldPos + oldLen)
		this->extendDirection = textview_ExtendRight;
	}
    }

    if (action == view_LeftMovement || action == view_RightMovement)  {
	if (redrawCursor)
	    DoUpdate(this, FALSE);
    }
    else
	(this)->WantUpdate( this);

    return (class view *) this;
}

void textview::ReceiveInputFocus()
    {
    this->hasInputFocus = TRUE;
    this->keystate->next = NULL;
    (this->menus)->SetMask( textview_NoMenus);
    (this)->PostKeyState( this->keystate);
    if ( this->editor == VI ) {
	if ( this->viMode == COMMAND )
	    message::DisplayString(this, 0, "Command Mode");
        else
	    message::DisplayString(this, 0, "Input Mode");
    }
    (this)->WantUpdate( this);
}

void textview::LoseInputFocus()
    {
    this->hasInputFocus = FALSE;
    (this)->WantUpdate( this);
}

void textview::SetDotPosition(long  newPosition)
        {
    long len;

    if (newPosition < 0)
	newPosition = 0;
    else  {
	if (newPosition > (len = (Text(this))->GetLength()))
	    newPosition = len;
    }
    if(newPosition!=dot->GetPos() && Text(this)->rootEnvironment) {
	environment *env=(environment *)Text(this)->rootEnvironment->GetInnerMost(newPosition);
        if(env && env->type==environment_View) {
            long fpos=env->Eval();
            long lpos=env->Eval()+env->GetLength();
            if(newPosition!=fpos) {
                if(newPosition < dot->GetPos()) newPosition=fpos;
                else newPosition=lpos;
            }
	}
    }
    (this->dot)->SetPos( newPosition);
    (this)->WantUpdate( this);
	
}

void textview::SetDotLength(long  newLength)
{
    if (newLength < 0)
	newLength = 0;
    HandleSelection(this, newLength, TRUE);
    /* (this->dot)->SetLength( newLength); */
    (this)->WantUpdate( this);

}

long textview::GetDotPosition()
    {
    return (this->dot)->GetPos();
}

long textview::GetDotLength()
    {
    return (this->dot)->GetLength();
}

long textview::GetTopPosition()
    {
    return (this->top)->GetPos();
}

void textview::SetTopOffTop(long  newTopPosition, long  pixelsOffTop)
            {
    long len;
    long curTop;
    long curPixel;

    if (newTopPosition < 0) {
        newTopPosition = 0;
    }
    else  {
	if (newTopPosition > (len = (Text(this))->GetLength())) {
            newTopPosition = len;
        }
    }
    
    if ((curTop = (this->top)->GetPos()) != newTopPosition || pixelsOffTop != this->pixelsReadyToBeOffTop)  {
	long line;

        if (curTop != newTopPosition) {
            (this->top)->SetPos( newTopPosition);
        }
        curPixel = this->pixelsReadyToBeOffTop;
        this->pixelsReadyToBeOffTop = pixelsOffTop;

	if (curTop < newTopPosition || ((curTop == newTopPosition) && (curPixel < pixelsOffTop)))  {
	    if (this->scroll != textview_ScrollBackward)   {
		line = (this)->FindLineNumber( newTopPosition);
		if (line != -1)  {
		    this->scroll = textview_ScrollForward;
//		    this->force = FALSE;
                    this->scrollLine = line;
		}
		else  {
                    this->force = TRUE;
		}
	    }
	    else {
                this->scroll = textview_MultipleScroll;
            }
	}
	else if (this->scroll != textview_ScrollForward)  {
	    if (this->scrollDist != -1)  {
		this->scroll = textview_ScrollBackward;
//		this->force = FALSE;
	    }
	    else {
		this->force = TRUE;
	    }
	}
	else {
            this->scroll = textview_MultipleScroll;
        }
    }
    if (this->scroll == textview_MultipleScroll)  {
	this->force = TRUE;
    }
    if(force) {
	scroll=textview_NoScroll;
	scrollDist=-1;
	scrollLine=-1;
    }	
    (this)->WantUpdate( this);
}

void textview::SetTopPosition(long  newTopPosition)
        {
    (this)->SetTopOffTop( newTopPosition, 0);
}

void textview::SetBorder(long  xBorder, long  yBorder)
            {
    this->bx = xBorder;
    this->by = yBorder;
    
    /* Have to do some update here for resetting the border. */
}

void textview::SetEmbeddedBorder(long  xBorder, long  yBorder)
            {
    this->ebx = xBorder;
    this->eby = yBorder;
    
    /* Have to do some update here for resetting the border. */
}

long textview::CollapseDot()
    {
    long pos;
    
    pos = (this->dot)->GetPos() + (this->dot)->GetLength();
    (this->dot)->SetPos( pos);
    HandleSelection(this, 0, TRUE);
    /* (this->dot)->SetLength( 0); */
    (this)->WantUpdate( this);
    return pos;
}

void textview::GetClickPosition(long  position, long  numberOfClicks, enum view_MouseAction  action, long  startLeft, long  startRight, long  *leftPos, long  *rightPos)
                                {
    int pos;
    int testType;
    class text *text = Text(this);
    int textLength = (text)->GetLength();
    int extEnd;

    switch (numberOfClicks % 3)  {
	case 1:
	    /* Single Click */

	    *leftPos = *rightPos = position;
	    break;
	    
	case 2:
	    /* Double Click - word select */

	    if (position < textLength && (testType = charType((text)->GetChar(position))) != WHITESPACE)  {
		/* Inside a word */

		pos = position - 1;
		for (pos = position - 1; pos >= 0 && charType((text)->GetChar(pos)) == testType; pos--);
		*leftPos = ++pos;
		for (pos = position + 1; pos < textLength && charType((text)->GetChar( pos)) == testType; pos++);
		*rightPos = pos;
	    }
	    else if (position > 0 && (testType = charType((text)->GetChar( position - 1))) != WHITESPACE)  {
		/* Right of Word */

		*rightPos = position;
		for (pos = position - 1; pos >= 0 && charType((text)->GetChar( pos)) == testType; pos--);
		*leftPos = pos + 1;
	    }
	    else  {
		/* No word either side */
		
		if (action == view_LeftDown)  {
		    *rightPos = *leftPos = position;
		}
		else  {
		    if (position <= startLeft)  {
			for (pos = position + 1; pos < textLength && charType((text)->GetChar( pos)) != WORD; pos++);
			for (pos -= 1; pos >= 0 && isspace((text)->GetChar( pos)) == 0; pos--);
			pos += 1;
		    }
		    else  {
			for (pos = position - 1; pos >= 0 && charType((text)->GetChar( pos)) != WORD; pos--);
			for (pos += 1; pos < textLength && isspace((text)->GetChar( pos)) == 0; pos++);
		    }
		    *rightPos = *leftPos = pos;
		}
	    }
	    break;

	case 0:
	    /* Triple Click - Paragraph select */

	extEnd = (action == view_LeftDown || action == view_LeftMovement);

	*leftPos = (text)->GetBeginningOfLine( position);
	pos = (text)->GetEndOfLine( position);
	*rightPos = (extEnd && pos < textLength) ? pos + 1 : pos;
	break;
    }
}

boolean textview::Visible(long  pos)
        {
    class mark *lineMark;
    long len = this->displayLength;
    long endMark;
    if (this->nLines <= 0 || pos < (this->top)->GetPos()) {
	return FALSE;
    }
    struct linedesc *fl=lines;
    lineMark = this->lines[this->nLines - 1].data;
    if(pos>=fl->data->GetPos() && pos<fl->data->GetPos()+fl->nChars && pixelsShownOffTop>0) return FALSE; 
    if (pos < (endMark = (lineMark)->GetPos() + this->lines[this->nLines-1].nChars))
	return TRUE;
    if (pos == len)  {
	if (pos == (lineMark)->GetPos())
	    return (this->nLines != 1 || textview_PrevCharIsNewline(Text(this), pos));
	else if (pos == endMark)
	    return  !textview_PrevCharIsNewline(Text(this), pos); 
    }
    return FALSE;
}

long textview::Locate(long  x, long  y, class view  **foundView)
                {
    long i;
    long textLength = (Text(this))->GetLength();
    long end = this->nLines;
    struct formattinginfo info;
    long pos;
    long by, bx, bxm;

    if (foundView)
        *foundView = NULL;

    if(!end)
	return textLength;

    if(this->aLines > end && this->lines[end].containsView) end++; 

    if (y < this->lines[0].y)   /* Tamper with clicks in top margin */
        y = this->lines[0].y;

    by = (this->hasApplicationLayer) ? this->by : this->eby;
    bx = (this->hasApplicationLayer) ? this->bx : this->ebx;
    bxm = bx + para_width + hz_offset;

    for (i = 0; i < end ; i++)  { 
	if (y >= this->lines[i].y && y < this->lines[i].y + this->lines[i].height)  {
	    info.searchinset=NULL;
	    pos = (this)->LineRedraw( textview_GetPosition, this->lines[i].data, bxm, by, (this)->GetLogicalWidth() - bx - bxm, (this)->GetLogicalHeight() - 2 * by, x, NULL, NULL, &info);
	    if (foundView)
	        *foundView = info.foundView;
	    return pos;
	}
	if ((this->lines[i].data)->GetPos() + this->lines[i].nChars >= textLength)
	    return textLength;
    }

    if (i == 0)
	return textLength;
    else
	return (this->lines[i - 1].data)->GetPos() + this->lines[i - 1].nChars;
}

void textview::GetTextSize(long  *width, long  *height)
            {
    long by = (this->hasApplicationLayer) ? this->by : this->eby;
    long bx = (this->hasApplicationLayer) ? this->bx : this->ebx;
    long bxm = bx + para_width + hz_offset;

    *width = (this)->GetLogicalWidth() - bx - bxm;
    *height = (this)->GetLogicalHeight() - 2 * by;
}

static long CalculateBltToTop(class textview  *self, long  pos, long  *distMoved, long  *linesAdded)
                {
    class mark *tm;
    class text *text = Text(self);
    long textLength = (text)->GetLength();
    long vxs, vys, length;
    long height, tpos;
    int tp;
    struct formattinginfo info;

    (self)->GetTextSize( &vxs, &vys);
    tm = new mark;
    tpos = (self)->GetTopPosition();

    if (pos > textLength) {
	pos = textLength;
    }

    tp = ReverseNewline(text, pos);

    /* tp is -1 if paragraph starts document */

    tp++;                   /* skip forward to first char in paragraph */

    while (TRUE)  {         /* scan lines in the paragraph */
	(tm)->SetPos( tp); /* start mark at paragraph start */
	height = (self)->LineRedraw( textview_GetHeight, tm, 0, 0, vxs, vys, 0, NULL, NULL, &info);
	length = info.lineLength;

	/* handle stopping prematurely at end of file, instead of at a newline */

	if (tp+length == textLength && !textview_PrevCharIsNewline(text, textLength)) length++;

	if (pos == tp || (pos >= tp && pos < tp+length))  {
	    /* we've stumbled over pos again */
	    long accumHeight = height;
	    long numLines = 1;
	    long newTop = tp;

	    while (TRUE)  {
		tp += length;
		(tm)->SetPos(tp);		/* start mark at paragraph start */
		height = (self)->LineRedraw( textview_GetHeight, tm, 0, 0, vxs, vys, 0, NULL,NULL, &info);
		length = info.lineLength;
		if (tp+length == textLength && !textview_PrevCharIsNewline(text, textLength)) length ++;
		if (tpos == tp || (tpos >= tp && tpos < tp+length))  {
		    if (distMoved != NULL)  {
			*distMoved = accumHeight + self->pixelsReadyToBeOffTop;
		    }
		    if (linesAdded != NULL)  {
			*linesAdded = numLines;
		    }
		    delete tm;
		    return newTop;
		}

		accumHeight += height;
		numLines++;

		if (accumHeight >= vys)  {
		    if (distMoved != NULL)  {
			*distMoved = -1;
		    }
		    if (linesAdded != NULL)  {
			*linesAdded = -1;
		    }
		    delete tm;
		    return newTop;
		}
	    }
	}
	tp += length;
    }
}

#define PSEUDOLINEHEIGHT 24
#define VIEWTOOSMALL 36
#define PLines(height) (((height) == 0) ? 0 : (((height) <= VIEWTOOSMALL) ? 1 : \
    ((height) + PSEUDOLINEHEIGHT - 1) / PSEUDOLINEHEIGHT))

/*
  Return the pos that would be at the top of the screen
  if we were to move pos down by units.
*/

static long BackSpace(class textview  *self, long  pos, long  units, enum textview_MovementUnits  type, long  *distMoved, long  *linesAdded)
                        {
    class mark *tm;
    long lastn [MAXPARA];
    long posn[MAXPARA];
    long pseudo[MAXPARA];
    class text *text = Text(self);
    long textLength = (text)->GetLength();
    long pp, vxs, vys, length;
    long totalHeight, height, accumHeight, numLines;
    long pseudoLines;
    long plines;
    int tp, px;
    struct formattinginfo info;
    long ounits = units;

    /* try to fail safe */
    if (linesAdded) {
	*linesAdded = 0;
    }

    if(distMoved) {
	*distMoved = 0;
    }

    if (type == textview_MoveByPseudoLines) {
	units -= PLines(self->pixelsReadyToBeOffTop);
	if (units <= 0) {
	    self->pixelsComingOffTop = self->pixelsReadyToBeOffTop - ounits * PSEUDOLINEHEIGHT;
	    if (self->pixelsComingOffTop < PSEUDOLINEHEIGHT / 2) {
		self->pixelsComingOffTop = 0;
	    }
	    if (distMoved) {
		*distMoved = self->pixelsReadyToBeOffTop - self->pixelsComingOffTop;
	    }
	    return pos;
	}
    }

    if (type == textview_MoveByPixels) {
	units -= (self->lines[0].y + self->lines[0].height);
	if (units <= 0) {
	    return pos;
	}
	units -= self->pixelsReadyToBeOffTop;
	if (units <= 0) {
	    self->pixelsComingOffTop = self->pixelsReadyToBeOffTop - ounits;
	    if (distMoved) {
		*distMoved = self->pixelsReadyToBeOffTop - self->pixelsComingOffTop;
	    }
	    return pos;
	}
    }
    self->pixelsComingOffTop = 0;
    accumHeight = -self->pixelsReadyToBeOffTop;

    (self)->GetTextSize( &vxs, &vys);
    tm=new mark;
    pp = pos;
    numLines = 0;

    for (;;) {

	/* first move tp back to start of paragraph */
	if (pp > 0) {
	   tp=ReverseNewline(text, pp);
	}
        else tp = -1;   /* tp is -1 if paragraph starts document */

	tp++;		/* skip forward to first char in paragraph */
	pp = tp;	/* remember paragraph start */
	px = 0;
	totalHeight = 0;
        pseudoLines = 0;
	while (px < MAXPARA) {		/* scan lines in the paragraph */
	    lastn[px] = tp;
            posn[px] = totalHeight;
            pseudo[px] = pseudoLines;
	    (tm)->SetPos( tp);	/* start mark at paragraph start */
	    height = (self)->LineRedraw( textview_GetHeight, tm, 0, 0, vxs, vys, 0, NULL, NULL, &info);
	    length = info.lineLength;
            plines = PLines(height);

	    /* handle stopping prematurely at end of file, instead of at a newline */

	    if (tp+length == textLength && !textview_PrevCharIsNewline(text, textLength)) length++;
	    if (pos == tp || (pos >= tp && pos < tp+length)) {
		/* we've stumbled over pos again */

		if (type == textview_MoveByLines) {
		    if (units <= px)  {
			delete tm;
			if (distMoved) {
			    *distMoved = accumHeight + totalHeight - posn[px-units];
			}
			if (linesAdded) {
			    *linesAdded = numLines - (px - units);
                        }
			return lastn[px-units];
		    }
		    else break;
                }
                else if (type == textview_MoveByPseudoLines) {
                    if (units <= pseudoLines) {
                        int i;

			delete tm;

                        for (i = 0; (i <= px) && (pseudoLines - pseudo[i] >= units); i++) {
                        }

                        self->pixelsComingOffTop = (pseudoLines - pseudo[i-1] - units) * PSEUDOLINEHEIGHT;

			if (distMoved) {
			    *distMoved = accumHeight + totalHeight - posn[i-1] - self->pixelsComingOffTop;
			}
			if (linesAdded) {
                            *linesAdded = numLines - (i-1);
                        }
			return lastn[i-1];
                    }
                    else break;
                }
		else { /* textview_MoveByPixels */
		    if (units <= totalHeight) {
                        int i;

			delete tm;
                        for (i = 0; (i <= px) && (totalHeight - posn[i] >= units); i++) {
                        }
                        self->pixelsComingOffTop = totalHeight - posn[i-1] - units;
                        if (self->pixelsComingOffTop < PSEUDOLINEHEIGHT) {
                            self->pixelsComingOffTop = 0;
                        }
			if (distMoved) {
                            *distMoved = accumHeight + totalHeight - posn[i-1] - self->pixelsComingOffTop;
                        }
			if (linesAdded) {
                            *linesAdded = numLines - (i-1);
                        }
			return lastn[i-1];
		    }
		    else break;
		}
            }

	    if (tp >= textLength) break;
	    tp += length;
	    totalHeight += height;
	    numLines++;
            px++;
            pseudoLines += plines;
        }

	/*
          Here we've moved back px lines, but still haven't made it.
	  Move back one more paragraph and try again.
        */

	accumHeight += totalHeight;
	if (pp <= 0)  {
	    delete tm;
	    if (distMoved) {
                *distMoved = accumHeight;
            }
	    if (linesAdded) {
                *linesAdded = numLines;
            }
	    return 0;	/* can't go any farther */
	}

	if (type == textview_MoveByLines) {
            units -= px;
        }
        else if (type == textview_MoveByPseudoLines) {
            units -= pseudoLines;
        }
	else {
            units -= totalHeight;
        }
	pos = pp;
	pp--;	/* looking at LF that terminated preceding para */
    }
}

long textview::MoveBack(long  pos, long  units, enum textview_MovementUnits  type, long  *distMoved, long  *linesAdded)
                        {
    long accumHeight = 0;
    long numLines = 0;
    long newPos;

    if (units < 0) return pos;
    
    if (pos < (this)->GetTopPosition()) {
	/* Get the distance that we need to blt in order to move
	 pos to the top of the view.  This only needs to be 
	 calculated when we are above the top position. */

	pos = CalculateBltToTop(this, pos, &accumHeight, &numLines);
    }

    /* Now determine how much space we need to add in if we want to move
	pos down by units */

    newPos = BackSpace(this, pos, units, type, distMoved, linesAdded);

    if (distMoved) {
	if (accumHeight >= 0) {
	    *distMoved += accumHeight;
	}
	else {
	    *distMoved = -1;
	}
    }
    if (linesAdded) {
	if (accumHeight >= 0) {
	    *linesAdded += numLines;
	}
	else {
	    *linesAdded = -1;
	}
    }

    return newPos;
}

long textview::MoveForward(long  pos, long  units, enum textview_MovementUnits  type, long  *distMoved, long  *linesAdded)
                        {
    class mark *tm;
    long vxs, vys;
    int i;
    struct formattinginfo info;
    long viewHeight;
    long textlen = (Text(this))->GetLength();

    if (units < 0) return pos;
    tm = new mark;
    (this)->GetTextSize( &vxs, &vys);
    this->pixelsComingOffTop = this->pixelsReadyToBeOffTop;
    if (type == textview_MoveByPixels) {
        i = -this->pixelsReadyToBeOffTop;
    }
    else {
        i = 0;
    }

    while (i < units)  {
        (tm)->SetPos( pos);
        viewHeight = (this)->LineRedraw( textview_GetHeight, tm, 0, 0, vxs, vys, 0, NULL, NULL, &info);
        if (type == textview_MoveByPseudoLines && viewHeight > VIEWTOOSMALL) {
            long pixelsLeft = viewHeight - this->pixelsComingOffTop;
            long pseudoLines = (pixelsLeft + PSEUDOLINEHEIGHT - 1) / PSEUDOLINEHEIGHT;

            if (pos + info.lineLength == textlen && pixelsLeft <= VIEWTOOSMALL) {
                break; /* there's nothing left; give up */
            }

            if (pseudoLines <= units - i) {
                /* gobble up the whole line, and keep going */
                pos += info.lineLength;
                i += pseudoLines;
                this->pixelsComingOffTop = 0;
            }
            else {
                /* take what we need and stop */
                this->pixelsComingOffTop += (pixelsLeft / pseudoLines) * (units - i);
                break;
            }
        }
        else if (type == textview_MoveByPixels) {
            if (viewHeight < VIEWTOOSMALL || viewHeight <= units - i) {
                pos += info.lineLength;
                i += viewHeight;
                this->pixelsComingOffTop = 0;
                if (pos == textlen) break;
            }
            else {
                this->pixelsComingOffTop += units - i;
                break;
            }
        }
        else {
            pos += info.lineLength;
            i++;
            this->pixelsComingOffTop = 0;
            if (pos == textlen) break;
        }
    }
    delete tm;
    return pos;
}


static void scrollback(class textview *tv,long target) {
    long pos=tv->GetTopPosition();
    long dist=0, lines=0;
    long newpos=(-1);
    long off;
    if(tv->pixelsReadyToBeOffTop<target) {
	struct rectangle lb;
	tv->GetLogicalBounds(&lb);
	long curx, cury, xs, ys;
	textview_ComputeViewArea(tv, lb, curx, cury, xs, ys);
	target-=tv->pixelsReadyToBeOffTop;
	newpos=RealScrollBack(tv, curx, cury, xs, ys, pos, target, dist, lines);
	off=(-target);
	dist+=tv->pixelsReadyToBeOffTop;
    } else {
	newpos=pos;
	off=tv->pixelsReadyToBeOffTop-target;
	dist=target;
    }
    if(off<0) off=0;
    tv->scrollLine=lines;
    tv->scrollDist=dist;
    tv->SetTopOffTop(newpos,off);
}

static void scrollforward(class textview *tv, long target) {
    long pos=tv->GetTopPosition();
    long dist=0;
    long off;
    class mark *tob=new mark;
    struct formattinginfo info;
    long height, curx, cury, xs, ys, ysleft;
    class text *t=Text(tv);
    long /*textheight=0,*/textheight2;
    struct rectangle vb;
    tv->GetLogicalBounds(&vb);
    textview_ComputeViewArea(tv, vb, curx, cury, xs, ys);
    /* 
    cury = (tv->hasApplicationLayer) ? tv->by : tv->eby;
    curx = (tv->hasApplicationLayer) ? tv->bx : tv->ebx;
    xs = tv->GetLogicalWidth() - 2 * tv->bx;
    ys = tv->GetLogicalHeight() - 2 * tv->by;
*/
   /* cury -= tv->pixelsReadyToBeOffTop; */

    ysleft = ys /* + tv->pixelsReadyToBeOffTop */;

    if(tob==NULL) return;

    target+=tv->pixelsReadyToBeOffTop;
    while(pos<t->GetLength()) {
	tob->SetPos(pos);
	tob->SetLength(0);
	height = tv->LineRedraw( textview_GetHeight, tob, curx, cury, xs, ysleft, FALSE, NULL, &textheight2, &info);
	if(dist+height>target) break;
#if 0
	textheight=textheight2;
#endif
	dist+=height;
	pos+=info.lineLength;
    }
    delete tob;
    off=target-dist;
    if(off<0) off=0;
 //   Doesn't look like we can do this: if(off<textheight) off=0;
// It makes the Shift by line command fail to scroll down a whole
// line sometimes.
    tv->SetTopOffTop(pos,off);

}

void textviewInterface::ScreenDelta(long dx, long dy) {
    if(dy<0) {

	if(tv->nLines>0) {
	    struct linedesc *ll=&tv->lines[tv->nLines-1];
	    if(ll->data->GetPos() + ll->nChars >= Text(tv)->GetLength() - 1) {
		long diff=ll->y + dy - 1;
		if(diff<0) dy-=diff;
	    }
	}
	if(dy<0) {
	    int i;
	    for(i=0;i<tv->nLines && tv->lines[i].y<=-dy;i++) {
		if(-dy>=tv->lines[i].y && -dy<tv->lines[i].y+tv->lines[i].textheight) {
		    dy=(-tv->lines[i].y);
		    break;
		}
	    }
	    scrollforward(tv,-dy);
	}
    } else {
	 if(tv->nLines>0 && tv->lines[0].height>=tv->lines[0].textheight && dy>tv->lines[0].textheight/2) dy-=tv->lines[0].textheight/2;
	scrollback(tv,dy);
    }
    if (dx != 0) {
	int viewwidth = tv->GetLogicalWidth() - ((tv->hasApplicationLayer) ? tv->bx : tv->ebx) * 2;
	int txtwidth = textview_SizeOfLongestLineOnScreen(tv);
	if (txtwidth < viewwidth)
	    txtwidth = viewwidth;
	tv->hz_offset -= dx;
	if (viewwidth - tv->hz_offset > txtwidth)
	    tv->hz_offset = viewwidth - txtwidth;
	if (tv->hz_offset > 0)
	    tv->hz_offset = 0;
	tv->force = TRUE;
	tv->WantUpdate(tv);
    }
}

#define DEFAULTHEIGHT 20
static void CalculateLineHeight(class textview  *self, unsigned short *cw=NULL, unsigned short *lh=NULL)
    {

    class style *defaultStyle;
    class fontdesc *defaultFont;
    struct FontSummary *fontSummary;
    char fontFamily[256];
    long refBasis, fontSize;

    if(cw) *cw=10;
    if(lh) *lh=DEFAULTHEIGHT;
    if ((defaultStyle = (self)->GetDefaultStyle()) != NULL) {
        (defaultStyle)->GetFontFamily( fontFamily, sizeof (fontFamily));
        (defaultStyle)->GetFontSize( (enum style_FontSize * ) &refBasis, &fontSize);
        defaultFont = fontdesc::Create(fontFamily, defaultStyle->GetAddedFontFaces(), fontSize);
        if ((fontSummary = (defaultFont)->FontSummary( (self)->GetDrawable())) != NULL) {
            if(cw) *cw=fontSummary->maxSpacing;
            if(lh) *lh=fontSummary->maxHeight;
        }
    }
}

#define MAXWIDTH 2048
#define MINWIDTH 125
#define UNLIMITED 3000000
view_DSattributes textview::DesiredSize(long  width , long  height, enum view_DSpass  pass, long  *desiredwidth , long  *desiredheight)
{
    class mark *tm;
    long  len, txheight,totheight,curx,cury,xs,ys = 0,newwidth;
    struct formattinginfo info;
    long bx, by, bxm;
    unsigned short lh, cw;
    long fwidth=0, fheight=0;
    
    by = (this->hasApplicationLayer) ? this->by : this->eby;
    bx = (this->hasApplicationLayer) ? this->bx : this->ebx;
    
    CalculateLineHeight(this, &cw, &lh);
    if(cw==0) cw=10;
        
    bxm = bx + para_width + hz_offset;
    
    if(rows!=0) {
        fheight=lh*rows + 2 *by;
        //  this is the height we will request.
    }
    if(columns!=0) {
        fwidth=cw*columns + bx + bxm;
        // this is the width we will request.
    }
    

    if(fwidth!=0 && fheight!=0) {
        // if both the width and height desired are fixed by row column
        // settings just return now..
        *desiredheight=fheight;
        *desiredwidth=fwidth;
        return (view_HeightFlexible | view_WidthFlexible);
    }
    
    if(Text(this) == NULL || ((len = (Text(this))->GetLength())== 0)) {
        // there is no text to base our decision on, so we request
        // space for one line of 30 characters in the default font.
        *desiredwidth = fwidth?fwidth:(cw*30 + bx + bxm);
        *desiredheight = fheight?fheight:(lh+2*by);
	return(view_HeightFlexible | view_WidthFlexible);
    }

    if(len>32768) len=32768; // limit the size computation to the first 32K
    
    if(pass==view_HeightSet) {
        // accept the imposed height
        fheight=height;
    } else if(fheight!=0) {
        // if height was set by a row request and not overridden
        // by view_HeightSet in pass then fix height to the
        // height needed for the requested number of rows.
        height=fheight;
    }

    if(pass==view_WidthSet) {
        // accept the imposed width
        fwidth=width;
    } else if(fwidth!=0) {
        // if width was set by a column request and not overridden
        // by view_WidthSet in pass then fix width to the
        // width needed for the requested number of columns.
        width=fwidth;
    }
    
    totheight = 0;
    cury = by;
    curx = bxm;
    tm = new mark;
    
    xs= width - bx - bxm;
    if(fheight==0) ys = UNLIMITED;
    else ys = height - 2 * by;

    newwidth=fwidth;
    retry: 
    boolean wrapped=FALSE;
    long pos = 0;
    totheight=0;
    while (pos < len)  {
	(tm)->SetPos(pos);
        txheight = (this)->LineRedraw( textview_GetHeight, tm, curx,cury,xs,ys, 0, NULL,NULL, &info);
        if(pos!=0 && !textview_PrevCharIsNewline(Text(this), pos)) wrapped=TRUE;
        if(info.lineLength == 0){
            if(newwidth>0 && newwidth!=width) width=newwidth;
             if(*desiredwidth==0) *desiredwidth = fwidth>0?fwidth:width;
             if(*desiredheight==0) *desiredheight = lh+2*by;
	     return(view_HeightFlexible | view_WidthFlexible);
         }
         if( fwidth==0 && newwidth<MAXWIDTH && (info.totalWidth > newwidth - bx - bxm)) {
             // width is not explicitly limited and it looks like we could use more...
             // set newwidth to trigger a recomputation with enough
             // width to accomodate this total width.
             if(info.totalWidth + bx + bxm>newwidth) newwidth=info.totalWidth + bx + bxm;
         }
         if(fheight!=0 && totheight + txheight > ys ){
             // ran out of vertical space
             // give up if this is the first line, we have hit the maximum
             // width,  if the width is fixed, or we didn't wrap any lines
             if(pos == 0 || width >= MAXWIDTH || fwidth!=0 || !wrapped) break;
             // if we're here we need more width.  If the totalWidth field
             // has indicated a larger width try that first, then step up by 10
             // pixels per iteration.
             // this could be better & faster by watching the total area used
             // by wrapped lines and adding enough width to reduce the height.
             if(newwidth>width && newwidth<MAXWIDTH) width=newwidth;
             else width += 10;
             if(width > MAXWIDTH) break;
             lh=totheight;
	     pos = 0;	
	     xs = width - bx - bxm;
             totheight = 0;
             newwidth=0;
	     continue;
	 }
	pos += info.lineLength;
	totheight += txheight;
    }
    if(fwidth==0 && width!=newwidth && newwidth>0 && newwidth<MAXWIDTH) {
        // even if we had enough room the first time around
        // we'll try again if we might be able to get away with less.
        xs = newwidth - bx - bxm;
        width=newwidth;
        goto retry;
    }
    delete tm;
    *desiredwidth = width;
    *desiredheight = totheight + by + by;
    return(view_HeightFlexible | view_WidthFlexible);
}

void textview::GetOrigin(long  width, long  height, long  *originX, long  *originY)
{
    struct formattinginfo info;
    /* long cury=(hasApplicationLayer ? by : eby); */
    (this)->LineRedraw( textview_GetHeight, this->top, 0, 0, width,
                        height, 0, NULL, NULL, &info);
    *originY = info.lineAbove + (hasApplicationLayer ? by : eby);
    /* *originY = height - txheight + info.lineAbove - cury; */
      
    *originX = 0;
}

void textview::FrameDot(long  pos)
        {
    (this->frameDot)->SetPos( pos);
}

void textview::LineNumberDisplay(boolean on_or_off)
{
    if (show_para_display != on_or_off) {
	show_para_display = on_or_off;
	para_width = on_or_off ? para_width_default+8 : 0;
	force = TRUE;
	if (GetIM())
	    ClearClippingRect();
    }
}

struct textv_viewfind {
    class view *self;
    class view *v;
    long pos;
};

static boolean findviewsplot(struct textv_viewfind *result, class text *text, long pos, class environment *env)
{
    class viewref *vr;
    class view *v;

    if (env->type == environment_View) {
	vr = env->data.viewref;
	v = (class view *)dictionary::LookUp(result->self, (char *)vr);
	if (result->v==v) {
	    result->pos = pos;
	    return TRUE;
	}
    }
    return FALSE;
}

void textview::ExposeChild(class view *v)
{
    struct textv_viewfind result;
    class text *txt = Text(this);

    result.self = this;
    result.v = v;

    if (txt->EnumerateEnvironments(0, (txt)->GetLength(), (text_eefptr)findviewsplot, (long)(&result))) {
	this->FrameDot(result.pos);
    }
}

long textview::FindLineNumber(long  pos)
{
    struct linedesc *tl = this->lines;    
    int i;
    long len = (Text(this))->GetLength();
    long endMark;
    
    for (i = 0; i < this->nLines; i++, tl++)  {
	if (pos >= (tl->data)->GetPos())  {
	    if (pos < (endMark = (tl->data)->GetPos() + tl->nChars))
		return i;
	    if (pos == len)  {
		if (pos == (tl->data)->GetPos())  {
		    if (i != 0 || textview_PrevCharIsNewline(Text(this), pos))
			return i;
		}
		else if (pos == endMark && !textview_PrevCharIsNewline(Text(this), pos))
		    return i;
	    }
	}
    }
    return -1;
}

static long position(long  pos, struct linedesc  *theline, long  coord)
            {
    long off;

    if (theline == NULL || !theline->containsView) {
        off = 0;
    }
    else {
	off = coord - theline->y;
	if(off</*=*/0) off= -(theline->y+theline->height);
    }
    /* with FINEGRID at 12, this will break with
       views taller than 1524 pixels */
    off = (off + FINEGRID/2) / FINEGRID;

    /* off has to fit in 7 bits */
    if (off > FINEMASK) {
        off = FINEMASK;
    }
    return (pos << FINESCROLL) + off;
}

static void getinfo(class textview  *self, struct range  *total , struct range  *seen , struct range  *dot)
{
    struct linedesc *last;
    long lastpos;
    long lastcoord;
    long tl = (Text(self))->GetLength();

    total->beg = 0;
    total->end = (tl << FINESCROLL);

    if (self->nLines > 0) {
        seen->beg = position((self)->GetTopPosition(), &self->lines[0], BY/*0*/);
        last = &self->lines[self->nLines - 1];
        lastpos = (last->data)->GetPos();
        if (((self)->GetLogicalHeight() - (2*BY)) >= (last->y + last->height)) {
            lastcoord = last->y;
            lastpos += last->nChars;
            if (lastpos > tl) {
                lastpos = tl;
            }
        }
        else {
            lastcoord = (self)->GetLogicalHeight() - (2*BY);
        }
        seen->end = position(lastpos, last, lastcoord);
    }
    else {
        seen->beg = ((self)->GetTopPosition() << FINESCROLL);
        seen->end = seen->beg;
    }

    dot->beg = ((self)->GetDotPosition() << FINESCROLL);
    dot->end = dot->beg + ((self)->GetDotLength() << FINESCROLL);
}

static long whatisat(class textview  *self, long  numerator , long  denominator)
        {
    long coord;
    long pos;
    long linenum;

    coord = numerator * (self)->GetLogicalHeight();
    coord /= denominator;

    pos = (self)->Locate( 0, coord, NULL);
    linenum = (self)->FindLineNumber( pos);

    return position(pos, (linenum >= 0) ? &self->lines[linenum] : NULL, coord);
}

/* move the text at position to be on screen at numerator/denominator */

static void setframe(class textview  *self, long  position , long  numerator , long  denominator)
        {
    long dist, lines, coord;
    long newpos;
    long off;
    boolean forceup = FALSE;

    coord = numerator * (self)->GetLogicalHeight();
    coord /= denominator;

    off = (position & FINEMASK) * FINEGRID;
    position >>= FINESCROLL;

    newpos = (self)->MoveBack( position, 0, textview_MoveByLines, 0, 0);
    if (newpos != position) {
        forceup = TRUE;
    }
    newpos = (self)->MoveBack( newpos, coord, textview_MoveByPixels, &dist, &lines);
    if (newpos < (self)->GetTopPosition() && self->scroll != textview_ScrollForward && self->scroll != textview_MultipleScroll)  {
	if (dist == -1)  {
	    self->scrollDist = -1;
	    self->scroll = textview_MultipleScroll;
	}
	if (self->scrollDist == -1)  {
	    self->scrollDist = dist;
	    self->scrollLine = lines;
	}
	else  {
	    self->scrollDist += dist;
	    if (self->scrollDist >= (self)->GetLogicalHeight())
		self->scrollDist = -1;
	    else
		self->scrollLine += lines;
	}
    }
    if (numerator != 0) {
        off = self->pixelsComingOffTop;
    }
    else {
        if (off != 0) {
            long line = (self)->FindLineNumber( newpos);
            long height;

            if (line >= 0) {
                height = self->lines[line].height;
            }
            else {
                class mark *tm = new mark;
                long vxs, vys;
                struct formattinginfo info;

                (tm)->SetPos( newpos);
                (self)->GetTextSize( &vxs, &vys);
		height = (self)->LineRedraw( textview_GetHeight, tm, 0, 0, vxs, vys, 0, NULL, NULL, &info);
		delete tm;
            }

            if (height < VIEWTOOSMALL) {
                off = 0;
            }
            else if (forceup || off > height) {
                off = height;
            }
        }
    }
    (self)->SetTopOffTop( newpos, off);
}

static void endzone(class textview  *self, int  end, enum view_MouseAction  action)
{
    if(action != view_LeftDown && action != view_RightDown) return;
    
    if (action == view_LeftDown &&
         (end == scroll_TOPENDZONE || end == scroll_BOTTOMENDZONE)) {
	    if (end == scroll_TOPENDZONE)
		setframe(self, 0, 0, (self)->GetLogicalHeight());
	    else
		setframe(self, (Text(self))->GetLength()<<FINESCROLL, (self)->GetLogicalHeight()>>2, (self)->GetLogicalHeight());
    }
    else {
        if (end == scroll_TOPENDZONE || end == scroll_MOTIFTOPENDZONE) {
		long newpos=(self)->GetTopPosition();
		newpos = (self)->MoveBack( newpos, 1, textview_MoveByLines, 0, 0);
		setframe(self, newpos<<FINESCROLL, 0, (self)->GetLogicalHeight());
	    } else {
                if (self->nLines<2) return;
		setframe(self, (self->lines[1].data)->GetPos()<<FINESCROLL, 0, (self)->GetLogicalHeight());
	    }
    }
}


/* The h* routines are for horizontal scrolling. */
static void hgetinfo(textview *self, struct range *total, struct range *seen, struct range *dot)
{
    int width = self->GetLogicalWidth() - ((self->hasApplicationLayer) ? self->bx : self->ebx) * 2;
    int i, txtwidth;

    /* Find length of longest line visible in the view. */
    txtwidth = 0;
    for (i = 0; i < self->nLines; i++)
	if (txtwidth < self->lines[i].nPixels)
	    txtwidth = self->lines[i].nPixels;
    if (width <= 0)
	width = 1;
    total->beg = 0;
    total->end = txtwidth;
    seen->beg = -self->hz_offset;
    seen->end = width - self->hz_offset;
    if (seen->end > total->end)
	seen->end = total->end;
    dot->beg = dot->end = -1;	/* no dot */
}

static long hwhatisat(textview *self, long numerator, long denominator)
{
    int width = self->GetLogicalWidth() - ((self->hasApplicationLayer) ? self->bx : self->ebx) * 2;
    int pos;

    if (width <= 0)
	width = 1;
    pos = width * numerator / denominator - self->hz_offset;
    return pos;
}

static void hsetframe(textview *self, long position, long numerator, long denominator)
{
    int width = self->GetLogicalWidth() - ((self->hasApplicationLayer) ? self->bx : self->ebx) * 2;
    int old_offset = self->hz_offset;

    if (width <= 0)
	width = 1;
    self->hz_offset = width * numerator / denominator - position;
    if (self->hz_offset > 0)
	self->hz_offset = 0;
    if (old_offset != self->hz_offset) {
	self->force = TRUE;		/* probably could optimize this like v scrolling */
	self->WantUpdate(self);
    }
}

static void hendzone(textview *self, int end, enum view_MouseAction action)
{
    int width = self->GetLogicalWidth() - ((self->hasApplicationLayer) ? self->bx : self->ebx) * 2;

    if(action != view_LeftDown && action != view_RightDown) return;
    
    if (action == view_LeftDown &&
         (end == scroll_TOPENDZONE || end == scroll_BOTTOMENDZONE)) {
	/* Non-Motif endzone.  Jump to end. */
	if (end == scroll_TOPENDZONE)
	    hsetframe(self, 0, 0, width);
	else {
	    int i, txtwidth;

	    txtwidth = 0;
	    for (i = 0; i < self->nLines; i++)
		if (txtwidth < self->lines[i].nPixels)
		    txtwidth = self->lines[i].nPixels;
	    hsetframe(self, txtwidth, width, width);
	}
    }
    else {
	/* Either right button or motif style.  Scroll toward end. */
        if (end == scroll_TOPENDZONE || end == scroll_MOTIFTOPENDZONE)
	    /* Click on left endzone.  */
	    hsetframe(self,  -(self->hz_offset+textview_get_hshift_interval(self)), 0, width);
	else
	    /* Click on right endzone. */
	    hsetframe(self, textview_get_hshift_interval(self) - self->hz_offset, 0, width);
    }
}

ScrollInterface *textview::GetScrollInterface() {
    // sigh, backward compatibility hack.
    // If a derived class has supplied its own old style scrolling,
    // and not overridden this function, then this function should
    // return NULL so that scroll will know to do everything the
    // old way.
    const void *v=GetInterface("scroll,vertical");
    const void *h=GetInterface("scroll,horizontal");
    if(v!=&scrollInterface || h!=&hscrollInterface) return NULL;
    return new textviewInterface(this);
}

const void *textview::GetInterface(const char  *interfaceName)
        {

    if (strcmp(interfaceName, "scroll,vertical") == 0)
        return &scrollInterface;
    else if (strcmp(interfaceName, "scroll,horizontal") == 0)
	return &hscrollInterface;
    return NULL;
}

long textview::GetPrintOption(const class atom *popt)
{
    long value;
    short gotit;

    if (popt==textview_printoptels[0].name) {
	gotit = this->GetDataObject()->Get(popt, &A_printoption, &value);
	if (!gotit) {
	    const char *str;
	    if ((str = environ::Get("PrintContents"))) {
		value = !(*str == 'n' || *str == 'N');
	    }
	    else
		value = environ::GetProfileSwitch("PrintContents", FALSE);
	}
	return value;
    }
    else if (popt==textview_printoptels[1].name) {
	gotit = this->GetDataObject()->Get(popt, &A_printoption, &value);
	if (!gotit) {
	    const char *str;
	    if ((str = environ::Get("PrintIndex"))) {
		value = !(*str == 'n' || *str == 'N');
	    }
	    else
		value = environ::GetProfileSwitch("PrintIndex", FALSE);
	}
	return value;
    }
    else if (popt==textview_printoptels[2].name) {
	gotit = this->GetDataObject()->Get(popt, &A_printoption, &value);
	if (!gotit) {
	    const char *str;
	    if ((str = environ::Get("Endnotes"))) {
		value = !(*str == 'n' || *str == 'N');
	    }
	    else
		value = environ::GetProfileSwitch("Endnotes", FALSE);
	}
	return value;
    }
    else if (popt==textview_printoptels[3].name) {
	gotit = this->GetDataObject()->Get(popt, &A_printoption, &value);
	if (!gotit) {
	    const char *str;
	    if ((str = environ::Get("Duplex"))) {
		value = !(*str == 'n' || *str == 'N');
	    }
	    else
		value = environ::GetProfileSwitch("Duplex", FALSE);
	}
	return value;
    }
    else if (popt==textview_printoptels[4].name) {
	gotit = this->GetDataObject()->Get(popt, &A_printoption, &value);
	if (!gotit) 
	    value = (environ::Get("AutoEnumerate") != NULL ||
		     environ::GetProfileSwitch("AutoEnumerate", FALSE) ||
		     environ::Get("InitialChapNumber") != NULL ||
		     environ::GetProfileSwitch("InitialChapNumber", FALSE));
	return value;
    }
    else if (popt==textview_printoptels[5].name) {
	gotit = this->GetDataObject()->Get(popt, &A_printoption, &value);
	if (!gotit) 
	    value = environ::GetProfileSwitch("TwoColumns", FALSE);
	return value;
    }
    else {
	/* default */
	return this->view::GetPrintOption(popt);
    }
}

struct view_printoptlist *textview::PrintOptions()
{
    textview_printopts.parent = this->view::PrintOptions();
    return &textview_printopts;
}

/* SetPrintOptions is handled by the default method (in view.C) */

boolean  textview::InitializeClass()
    {
    A_boolean = atom::Intern("boolean");
    A_printoption = atom::Intern("printoption");

    textview_printoptels[0].name = atom::Intern("docontents");
    textview_printoptels[0].type = A_boolean;
    textview_printoptels[0].label = "Print table of contents with document";
    textview_printoptels[1].name = atom::Intern("doindex");
    textview_printoptels[1].type = A_boolean;
    textview_printoptels[1].label = "Print index with document";
    textview_printoptels[2].name = atom::Intern("endnotes");
    textview_printoptels[2].type = A_boolean;
    textview_printoptels[2].label = "Print footnotes at end of document";
    textview_printoptels[3].name = atom::Intern("swapheaders");
    textview_printoptels[3].type = A_boolean;
    textview_printoptels[3].label = "Swap right and left headers on even pages";
    textview_printoptels[4].name = atom::Intern("enumcontents");
    textview_printoptels[4].type = A_boolean;
    textview_printoptels[4].label = "Enumerate contents automatically";
    textview_printoptels[5].name = atom::Intern("twocolumns");
    textview_printoptels[5].type = A_boolean;
    textview_printoptels[5].label = "Print two columns";

    drawtxtv_tabscharspaces = environ::GetProfileInt("TabsCharSpaces", 8);
    /* these init functions should be called in this specific order */
    textviewEmacsKeymap = textview_InitEmacsKeyMap(&textview_ATKregistry_ , &viewMenus);
    textviewViCommandModeKeymap = textview_InitViCommandModeKeyMap(&textview_ATKregistry_ , NULL);
    textviewViInputModeKeymap = textview_InitViInputModeKeyMap(&textview_ATKregistry_ , NULL);
    initialExposeStyles =
	environ::GetProfileSwitch("ExposeStylesOnStartup", FALSE);
    alwaysDisplayStyleMenus = environ::GetProfileSwitch("AlwaysDisplayStyleMenus", TRUE);
    highlightToBorders = environ::GetProfileSwitch("HighlightToBorders", FALSE);
    InitializeMod();
    textview_InitializePS();
    
    return TRUE;
}

void textview::SetDefaultStyle(class style  *styleptr)
        {
    this->defaultStyle = styleptr;
}

class style *textview::GetDefaultStyle()
    {
    return this->defaultStyle;
}

void textview::Print(FILE  *f, const char  *process, const char  *final, int  toplevel)
                    {
    /* This is really screwed. This should return an error and the guy above this */
    /* layer should handle it. */
    if (ATK::LoadClass("texttroff") == NULL) {
	message::DisplayString(this, 0, "Print aborted: could not load class \"texttroff\".");
        return;
    }

    if (strcmp(process, "troff") == 0 && strcmp(final, "PostScript") == 0)
	texttroff::WriteTroff(this, Text(this), f, toplevel); 
}

long textview::GetUniquePlanId()
{
    ATKinit;

    static long val = 1;

    return ++val;
}

static void CreateMatte(class textview  *self, class viewref  *vr)
{
    class view *v =
      (class view *) dictionary::LookUp(self, (char *) vr);
    if(v==NULL && vr->viewType) ATK::LoadClass(vr->viewType);
    if (v == NULL && ATK::IsTypeByName(vr->viewType, "view")) { 
        v = (class view *) matte::Create(vr, (class view *) self);
        if (v != NULL) {
            (vr)->AddObserver( self);
            dictionary::Insert(self, (char *) vr, (char *) v);
        }
    }

    if (v != NULL) {
        if (v->parent != (class view *) self)
            (v)->LinkTree( (class view *) self);
        (v)->InitChildren();
    }
}

void textview::InitChildren()
{
    class text *d = Text(this);
    long pos = 0;

    while (pos < (d)->GetLength()) {
        long gotlen;
        char *s = (d)->GetBuf( pos, 10240, &gotlen);
        while (gotlen--) {
            if (*s == TEXT_VIEWREFCHAR) {
		/* Need to look here - ajp */
                class environment *env = (this)->GetStyleInformation( NULL, pos, NULL);

                if (env != NULL && env->type == environment_View)
                    CreateMatte(this, env->data.viewref);

		(this)->ReleaseStyleInformation( env);
            }
            s++, pos++;
	}
    }
}

boolean textview::CanView(const char  *TypeName)
{
    ATK::LoadClass(TypeName);
    return ATK::IsTypeByName(TypeName, "text");
}

/* NOTE: sv (if non-NULL) is assumed to be an UN-initialized statevector, either previously initialize and finalized, or never initialized. */
class environment *textview::GetStyleInformation(struct text_statevector  *sv, long  pos, long  *length)
{
    class environment *env;

    env = (class environment *)(Text(this)->rootEnvironment)->GetInnerMost( pos);
    if (sv != NULL) {
	text::InitStateVector(sv);
	text::ApplyEnvironment(sv, this->defaultStyle, env);
    }
    if (length != NULL) {
	*length = (Text(this)->rootEnvironment)->GetNextChange( pos);
    }
    return env;
}

class environment *textview::GetEnclosedStyleInformation(long  pos, long  *length)
{
    class environment *env;

    env = (class environment *)(Text(this)->rootEnvironment)->GetEnclosing( pos);
    if (length != NULL) {
	*length = (Text(this)->rootEnvironment)->GetNextChange( pos);
    }
    return env;
}

void textview::ReleaseStyleInformation(class environment  *env)
{
}

/* functions added to support VI interface - FAS */

void textview::ToggleVIMode()
{
    /* switch between VI input/command mode */

    this->viMode = this->viMode ^ 1;
    if ( this->viMode == COMMAND )
	this->keystate = this->viCommandModeKeystate;
    else
	this->keystate = this->viInputModeKeystate;
    (this)->ReceiveInputFocus();
}

void textview::ToggleEditor()
{
    /* switch between emacs and vi editors */
    this->editor = this->editor ^ 1;
    if ( this->editor == EMACS )
    {
    	this->keystate = this->emacsKeystate;
	message::DisplayString(this, 0, "EMACS interface in effect.");
    }
    else
    {
	this->keystate = this->viCommandModeKeystate;
	this->viMode = COMMAND;
	message::DisplayString(this, 0, "VI interface in effect.");
    }
    (this)->ReceiveInputFocus();
}

/* Stubs for selection code. */
void textview::LoseSelectionOwnership()
{
    (this->dot)->SetLength( 0);
    (this)->WantUpdate( this);
}

static int stringmatch(class text  *d,long  pos,const char  *c)
{
    /* Tests if the text begins with the given string */
    while(*c != '\0') {
        if((d)->GetChar(pos) != *c) return FALSE;
        pos++; c++;
    }
    return TRUE;
}

static void DoCopySelection(class textview  *self, FILE  *cutFile, long  pos , long  len)
{
    class text *d;
    long nextChange;
    int UseDataStream;

    d = Text(self);
    (d->rootEnvironment)->GetInnerMost( pos);
    nextChange = (d->rootEnvironment)->GetNextChange( pos);

    if ((UseDataStream = ((nextChange <= len|| stringmatch(d,pos,"\\begindata")) && (d)->GetExportEnvironments())))
	fprintf(cutFile, "\\begindata{%s, %d}\n",
		(d)->GetCopyAsText() ? "text": (d)->GetTypeName(),
		/* d->header.dataobject.id */ 999999);
    d->writeID = im::GetWriteID();
    (d)->WriteSubString( pos, len, cutFile, UseDataStream);
    
    if (UseDataStream)
	fprintf(cutFile, "\\enddata{%s,%d}\n", 
		(d)->GetCopyAsText() ? "text": (d)->GetTypeName(), /* d->header.dataobject.id */ 999999);
}

long textview::WriteSelection(FILE  *out)
{
    DoCopySelection(this, out, (this->dot)->GetPos(), (this->dot)->GetLength());
    /* if this is called on a view which doesn't override it there is an error.*/
    return 0;
}
