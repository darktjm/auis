/*  Copyright 1995 Carnegie Mellon University All rights reserved. */

#include <andrewos.h>
ATK_IMPL("amenu.H")

#include <amenu.H>
#include <sbuttonv.H>
#include <util.h>
#include <fontdesc.H>
#include <atom.H>
#include <mflex.H>
#include <im.H>
#include <shadows.h>
#include <popupwin.H>

AmenuGraphic::AmenuGraphic() {
}

AmenuGraphic::~AmenuGraphic() {
}

void AmenuSep::EnsurePrefs() {
    static sbutton_prefs *asep=NULL;
    if(this->prefs) return;
    if(asep==NULL) {
	asep=sbutton::GetNewPrefs("AmenuSep");
	if(asep) {
	    sbutton::InitPrefs(asep, "AmenuSep");
	    if(sbutton::GetDepth(asep)<0) sbutton::GetDepth(asep)=2;
	}
    }
    this->prefs=asep;
    sbutton::GetRefCount(asep)++;
}

void AmenuOption::EnsurePrefs() {
    static sbutton_prefs *aopt=NULL;
    if(this->prefs) return;
    if(aopt==NULL) {
	aopt=sbutton::GetNewPrefs("AmenuOption");
	if(aopt) {
	    sbutton::InitPrefs(aopt, "AmenuOption");
	    if(sbutton::GetDepth(aopt)<0) sbutton::GetDepth(aopt)=2;
	    if(sbutton::GetStyle(aopt)<0) sbutton::GetStyle(aopt)=3;
	}
    } 	
    this->prefs=aopt;
    sbutton::GetRefCount(aopt)++;
}

void AmenuLabel::EnsurePrefs() {
    struct sbutton_prefs *alabel=NULL;
    if(this->prefs) return;
    if(alabel==NULL) {
	alabel=sbutton::GetNewPrefs("AmenuLabel");
	if(alabel) {
	    sbutton::InitPrefs(alabel, "AmenuLabel");
	}
	if(sbutton::GetFont(alabel)==NULL) sbutton::GetFont(alabel)=fontdesc::Create("andy", fontdesc_Bold, 12);
    }
    this->prefs=alabel;
    sbutton::GetRefCount(alabel)++;
}

void AmenuSep::GetModPrefs() {
    EnsurePrefs();
    if(sbutton::GetRefCount(this->prefs)>1) {
	struct sbutton_prefs *p=this->prefs;
	this->prefs=sbutton::DuplicatePrefs(this->prefs, NULL);
	sbutton::FreePrefs(p);
    }
}


void AmenuLabel::GetModPrefs() {
    EnsurePrefs();
    if(sbutton::GetRefCount(this->prefs)>1) {
	struct sbutton_prefs *p=this->prefs;
	this->prefs=sbutton::DuplicatePrefs(this->prefs, NULL);
	sbutton::FreePrefs(p);
    }
}

AmenuLabel::AmenuLabel(const char *label, struct sbutton_prefs *sprefs, unsigned short spos, boolean atomize) {
    pos=spos;
    astr=NULL;
    str=NULL;
    if(label==NULL) {
	astr=atom::Intern("none");
	str=astr->Name();
    } else SetLabel(label,atomize);
    if(sprefs) {
	prefs=sprefs;
	sbutton::GetRefCount(sprefs)++;
    } else EnsurePrefs();
}

AmenuLabel::AmenuLabel(const char *label, const char *f, const char *col, unsigned short spos, boolean atomize) {
    pos=spos;
    astr=NULL;
    str=NULL;
    if(label==NULL) {
	astr=atom::Intern("none");
	str=astr->Name();
    } else SetLabel(label,atomize);
    if(col || f) {
	prefs=NULL;
	if(col) {
	    GetModPrefs();
	    sbutton::GetLabelFG(prefs)=atom::Intern(col)->Name();
	}
	if(f) {
	    char buf[1024];
	    long style, size;
	    if(f==NULL) return;
	    if(fontdesc::ExplodeFontName((char *)f, buf, sizeof(buf), &style, &size)) {
		GetModPrefs();
		sbutton::GetFont(prefs)=fontdesc::Create(buf, style, size);
	    }
	}
    } else prefs=NULL;
}

AmenuLabel::AmenuLabel() {
    char buf[16];
    sprintf(buf, "%p\n", this);
    astr=atom::Intern(buf);
    str=astr->Name();
    printf("str:%s\n", str);
    prefs=NULL;
    pos= graphic::BETWEENTOPANDBOTTOM|graphic::BETWEENLEFTANDRIGHT;
}

AmenuLabel::~AmenuLabel() {
    if(astr==NULL && str) free((char *)str);
    if(prefs) sbutton::FreePrefs(prefs);
}

AmenuLabel &AmenuLabel::SetLabel(const char *label, boolean atomize) {
    if(label==NULL) return *this;
    if(astr==NULL && str) free((char *)str);
    if(atomize) {
	astr=atom::Intern(label);
	if(astr) str=astr->Name();
	else str=NULL;
    } else {
	str=strdup((char *)label);
	astr=NULL;
    }
    return *this;
}

AmenuLabel &AmenuLabel::SetColor(const char *c) {
    if(c==NULL) return *this;
    GetModPrefs();
    sbutton::GetLabelFG(prefs)=atom::Intern(c)->Name();
    return *this;
}

AmenuLabel &AmenuLabel::SetFont(const char *f) {
    if(f==NULL) return *this;
    char buf[1024];
    long style, size;
    if(f==NULL) return *this;
    if(fontdesc::ExplodeFontName((char *)f, buf, sizeof(buf), &style, &size)) {
	GetModPrefs();
	sbutton::GetFont(prefs)=fontdesc::Create(buf, style, size);
    }
    return *this;
}
    
AmenuLabel &AmenuLabel::SetPrefs(struct sbutton_prefs *p) {
    if(p==NULL) return *this;
    sbutton::GetRefCount(p)++;
    prefs=p;
    return *this;
}

void AmenuLabel::ComputeSize(view *vw, long *w, long *h) {
    FontSummary *summary;
    graphic *g=vw->GetDrawable();
    EnsurePrefs();
    if(g==NULL || sbutton::GetFont(prefs)==NULL || str==NULL) {
	*w=*h=0;
	return;
    }
    fontdesc *font=sbutton::GetFont(prefs);
    struct fontdesc_charInfo ci;
    if(strlen(str)==1) {
	(font)->CharSummary( g, *str, &ci);
	*w=ci.width+4;
	*h=ci.height+4;
    } else {
	(font)->StringSize(g, str, w, h);
	summary =  (font)->FontSummary(g);
	if(summary) *h=summary->maxHeight;
    }
}

AmenuLabel &AmenuLabel::SetPosition(unsigned short spos) {
    pos=spos;
    return *this;
}

void AmenuLabel::Draw(view *vw, const rectangle *bounds, boolean sensitive, boolean selected, const rectangle *damaged) {
    long x=0, y=0;
    if(pos&graphic::BETWEENTOPANDBOTTOM) y=bounds->top+bounds->height/2;
    else if(pos&graphic::ATTOP) y=bounds->top;
    else if(pos&graphic::ATBOTTOM) y=bounds->top+bounds->height-1;
    if(pos&graphic::BETWEENLEFTANDRIGHT) x=bounds->left+bounds->width/2;
    else if(pos&graphic::ATLEFT) x=bounds->left;
    else if(pos&graphic::ATRIGHT) x=bounds->left+bounds->width-1;
    struct sbuttonv_view_info vi;
    sbuttonv::SaveViewState(vw, &vi);
    sbuttonv::DrawLabel(vw, str, x , y, prefs, selected, pos, sensitive);
    sbuttonv::RestoreViewState(vw, &vi);
}

AmenuSep::AmenuSep(struct sbutton_prefs *p) {
    prefs=p;
}

AmenuSep::AmenuSep(const char *topshadow, const char *bottomshadow) {
    prefs=NULL;
    SetShadows(topshadow, bottomshadow);
}

void AmenuSep::SetShadows(const char *topshadow, const char *bottomshadow) {
    GetModPrefs();
    sbutton::GetTopShadow(prefs)=atom::Intern(topshadow)->Name();
    sbutton::GetBottomShadow(prefs)=atom::Intern(bottomshadow)->Name();
}
    
AmenuSep::~AmenuSep() {
    if(prefs) sbutton::FreePrefs(prefs);
}

void AmenuSep::SetPrefs(struct sbutton_prefs *p) {
    if(p==NULL) return;
    sbutton::GetRefCount(p)++;
    prefs=p;
}

void AmenuSep::ComputeSize(view *vw, long *w, long *h) {
    EnsurePrefs();
    *h=sbutton::GetDepth(prefs);
    if(*h<0) *h=2;
    *w=0;
}

void AmenuSep::Draw(view *vw, const rectangle *bounds, boolean sensitive, boolean selected, const rectangle *damaged) {
    EnsurePrefs();
    if(sbutton::GetBottomShadow(prefs)==NULL) {
	vw->GetDrawable()->SetFGToShadow(shadows_BOTTOMSHADOW);
    } else vw->SetForegroundColor(sbutton::GetBottomShadow(prefs),0,0,0);
    long h=sbutton::GetDepth(prefs)/2;
    if(h<0) h=1;
    long topoff=(bounds->height-h*2)/2;
    vw->FillRectSize(bounds->left, bounds->top+topoff, bounds->width, h, NULL);
    if(sbutton::GetTopShadow(prefs)==NULL) {
	vw->GetDrawable()->SetFGToShadow(shadows_TOPSHADOW);
    } else vw->SetForegroundColor(sbutton::GetTopShadow(prefs),0,0,0); 
    vw->FillRectSize(bounds->left, bounds->top+topoff+h, bounds->width, h, NULL);
}	
    
struct AmenuGraphicdata {
    AmenuGraphic *ptr;
    int pos;
};

DEFINE_MFLEX_CLASS(AmenuAuxlabels_base,AmenuGraphicdata,1);
class AmenuAuxlabels : public AmenuAuxlabels_base {
    boolean sorted;
  public:
    AmenuAuxlabels() : sorted(0) {}
    void Sort();
    boolean Sorted();
    void MarkChanged();
};

DEFINE_MFLEX_SORTER(auxlabel_sort,AmenuAuxlabels,AmenuGraphicdata,((long)e1.pos)-((long)e2.pos));

void AmenuAuxlabels::Sort() {
    if(!sorted) {
	auxlabel_sort(*this);
	sorted=TRUE;
    }
}

boolean AmenuAuxlabels::Sorted() {
    return sorted;
}

void AmenuAuxlabels::MarkChanged() {
    sorted=FALSE;
}

AmenuEntry::AmenuEntry() {
    label=NULL;
    sensitive=TRUE;
    obj=NULL;
    act=NULL;
    left = new AmenuAuxlabels();
    right = new AmenuAuxlabels();
}

AmenuEntry &AmenuEntry::SetLabel(const char *label, struct sbutton_prefs *prefs, unsigned short pos) {
    AmenuLabel *l=new AmenuLabel(label, prefs, pos);
    SetKey(atom::Intern(label));
    SetLabel(l);
    return  *this;
}

AmenuEntry &AmenuEntry::SetLabel(const char *label, const char *font, const char *color, unsigned short pos) {
    AmenuLabel *l=new AmenuLabel(label, font, color, pos);
    SetKey(atom::Intern(label));
    SetLabel(l);
    return  *this;
}

AmenuEntry::~AmenuEntry() {
    size_t i;
    delete label;
    for(i=0;i<left->GetN();i++) delete (*left)[i].ptr;
    delete left;
    for(i=0;i<right->GetN();i++) delete (*right)[i].ptr;
    delete right;
}

struct AmenuEntrydata {
    AmenuEntry *ptr;
    int pos;
};

DEFINE_MFLEX_CLASS(AmenuEntrylist_base,AmenuEntrydata,5);
class AmenuEntrylist : public AmenuEntrylist_base {
    boolean sorted;
  public:
    AmenuEntrylist() : sorted(FALSE) {
    }
    void Sort();
    boolean Sorted();
    void MarkChanged();
};

DEFINE_MFLEX_SORTER(entrylist_sort,AmenuEntrylist,AmenuEntrydata,((long)e1.pos)-((long)e2.pos));

void AmenuEntrylist::Sort() {
    if(!sorted) {
	entrylist_sort(*this);
	sorted=TRUE;
    }
}

boolean AmenuEntrylist::Sorted() {
    return sorted;
}

void AmenuEntrylist::MarkChanged() {
    sorted=FALSE;
}

void AmenuCard::Sort() {
    entries->Sort();
}

AmenuEntry &AmenuCard::operator[](size_t i) {
    return *(*entries)[i].ptr;
}

size_t AmenuCard::GetN() {
    return entries->GetN();
}

static void ComputeGraphicSize(AmenuGraphic *g, view *vw, long *w, long *h) {
    long lw, lh;
    g->ComputeSize(vw, &lw, &lh);
    *w+=lw;
    if(lh>*h) *h=lh;
}

void AmenuEntry::ComputeSize(view *vw, long *labelw, long *leftw, long *rightw, long *h) {
    *h=0;
    if(label) {
	long lh;
	label->ComputeSize(vw, labelw, &lh);
	if(lh>*h) *h=lh;
    } else *labelw=*h=0;
    *leftw=0;
    unsigned int i;
    for(i=0;i<left->GetN();i++) ComputeGraphicSize((*left)[i].ptr, vw, leftw, h);
    for(i=0;i<right->GetN();i++) ComputeGraphicSize((*right)[i].ptr, vw, rightw, h);
    
}

static void DrawLeft(AmenuGraphic *g, view *vw, const rectangle *bounds, const rectangle *damaged, long *right, boolean sensitive, boolean selected) {
    long w, h;
    g->ComputeSize(vw, &w,&h);
    rectangle agr;
    agr.left=(*right)-w;
    agr.width=w;
    agr.top=bounds->top;
    agr.height=bounds->height;
    if(damaged) {
	rectangle ld;
	rectangle_IntersectRect(&ld, &agr, (rectangle *)damaged);
	g->Draw(vw, &agr, sensitive, selected, &ld);
    } else g->Draw(vw, &agr, sensitive, selected);
    (*right)-=w;
}

static void DrawRight(AmenuGraphic *g, view *vw, const rectangle *bounds, const rectangle *damaged, long *left, boolean sensitive, boolean selected) {
    long w, h;
    g->ComputeSize(vw, &w,&h);
    rectangle agr;
    agr.left=(*left);
    agr.width=w;
    agr.top=bounds->top;
    agr.height=bounds->height;
    if(damaged) {
	rectangle ld;
	rectangle_IntersectRect(&ld, &agr, (rectangle *)damaged);
	g->Draw(vw, &agr, sensitive, selected, &ld);
    } else g->Draw(vw, &agr, sensitive, selected);
    (*left)+=w;
}

void AmenuEntry::Draw(view *vw, const rectangle *bounds, const rectangle *labelbounds, boolean sensitive, boolean selected, const rectangle *damaged) {
    if(label) {
	if(damaged) {
	    rectangle labeldamage;
	    rectangle_IntersectRect( &labeldamage, (struct rectangle *)labelbounds, (rectangle *)damaged);
	    label->Draw(vw, labelbounds, sensitive, selected, &labeldamage);
	} else label->Draw(vw, labelbounds, sensitive, selected, NULL);
    }
    {
	long right=labelbounds->left;
	left->Sort();
	for(unsigned int i=0;i<left->GetN();i++) DrawLeft((*left)[i].ptr, vw, bounds, damaged, &right, sensitive, selected);
    }
    
    {
	long left=labelbounds->left + labelbounds->width;
	right->Sort();
	for(unsigned int i=0;i<right->GetN();i++) DrawRight((*right)[i].ptr, vw, bounds, damaged,  &left, sensitive, selected);
    }
}


AmenuEntry &AmenuEntry::AddLeftGraphic(AmenuGraphic *g, int priority) {
    AmenuGraphicdata *ag=left->Append();
    if(ag) {
	ag->ptr=g;
	ag->pos=priority;
	left->MarkChanged();
    }
    return *this;
}

AmenuEntry &AmenuEntry::AddLeftIcon(const char *icon, const char *color, int priority, unsigned short pos) {
    AmenuLabel *l=new AmenuLabel(icon, "icon12", color, pos);
    AddLeftGraphic(l, priority);
    return *this;
}

AmenuEntry &AmenuEntry::AddRightGraphic(AmenuGraphic *g, int priority) {
    AmenuGraphicdata *ag=right->Append();
    if(ag) {
	ag->ptr=g;
	ag->pos=priority;
	right->MarkChanged();
    }
    return *this;
}

AmenuEntry &AmenuEntry::AddRightString(const char *str, const char *font, const char *color, int priority, unsigned short pos) {
    AmenuLabel *l=new AmenuLabel(str, font, color, pos);
    AddRightGraphic(l,priority);
    return *this;
}

static atom_def object("object");

AmenuEntry &AmenuEntry::SetArgs(const avalueflex &a) {
    args=a;
    args.add(this,object);
    return *this;
}

AmenuEntry &AmenuEntry::SetAction(ATK *o, const aaction *a) {
    obj=o;
    act=(aaction *)a;
    if(!args.GetN())
	args.add(this,object);
    return *this;
}

AmenuEntry &AmenuEntry::SetAction(ATK *o, const aaction *a, const avalueflex &av) {
    obj=o;
    act=(aaction *)a;
    SetArgs(av);
    args.add(this,object);
    return *this;
}

void AmenuEntry::Dispatch() {
    if(obj && act) {
	avalueflex out;
	aaction &a=(*act);
	a(obj,args,out);
    }
}


ATKdefineRegistryNoInit(AmenuCard, dataobject);

AmenuCard::AmenuCard()
{
    entries = new AmenuEntrylist;
    prefs=sbutton::GetNewPrefs("AmenuCard");
    if(prefs==NULL) return;
    sbutton::InitPrefs(prefs, "AmenuCard");
}

#if 0
class NO_DLL_EXPORT democlass;
DEFINE_AACTION_FUNC_CLASS(democlass,AmenuCard);
static void demofunc(AmenuCard *obj, const avalueflex &aux, const avalueflex &in, avalueflex &out) {
    printf("obj:%p\n", obj);
    printf("aux:%ld\n",aux[0].Integer());
    printf("in:%ld\n",in[0].Integer());
}

static democlass demo(demofunc,9L);

void AmenuCard::Demo()
{
    AddEntry(10). SetAction(this,&demo,42L). AddLeftIcon("\100"). SetLabel("Foo", NULL, "pink");
    AddEntry(5).  SetAction(this,&demo,88L). AddRightString(NULL,"andy22i").AddLeftIcon("\047"). SetLabel("foo", NULL, "pink");
    AddSeparator(7);
}
#endif

AmenuCard::~AmenuCard() {
    if(prefs) sbutton::FreePrefs(prefs);
    for(size_t i=0;i<entries->GetN();i++) {
	delete (*entries)[i].ptr;
    }
    delete entries;
}

AmenuEntry &AmenuCard::AddEntry(AmenuEntry *e, int priority) {
    AmenuEntrydata *ae=entries->Append();
    if(ae) {
	ae->ptr=e;
	ae->pos=priority;
	entries->MarkChanged();
    }
    return *e;
}

AmenuEntry &AmenuCard::AddEntry(int priority) {
    AmenuEntrydata *ae=entries->Append();
    ae->ptr=new AmenuEntry;
    ae->pos=priority;
    entries->MarkChanged();
    return *ae->ptr;
}

AmenuEntry &AmenuCard::AddSeparator(int priority, struct sbutton_prefs *pref) {
    AmenuEntrydata *ae=entries->Append();
    ae->ptr=new AmenuEntry;
    ae->pos=priority;
    entries->MarkChanged();
    ae->ptr->SetSensitivity(FALSE);
    ae->ptr->SetLabel(new AmenuSep(pref));
    return *ae->ptr;
}

AmenuEntry &AmenuCard::AddSeparator(int priority, const char *topshadow, const char *bottomshadow) {
    AmenuEntrydata *ae=entries->Append();
    ae->ptr=new AmenuEntry;
    ae->pos=priority;
    entries->MarkChanged();
    ae->ptr->SetSensitivity(FALSE);
    ae->ptr->SetLabel(new AmenuSep(topshadow,bottomshadow));
    return *ae->ptr;
}

void AmenuCard::RemoveEntry(AmenuEntry *e) {
    for(size_t i=0;i<entries->GetN();i++) {
	if((*entries)[i].ptr==e) {
	    delete (*entries)[i].ptr;
	    entries->Remove(i,1);
	    return;
	}
    }
}

void AmenuCard::RemoveEntry(const atom *key) {
    for(size_t i=0;i<entries->GetN();i++) {
	if((*entries)[i].ptr->key==key) {
	    delete (*entries)[i].ptr;
	    entries->Remove(i,1);
	    return;
	}
    }
}

struct AmenuEntriesdata {
    struct rectangle r;
    AmenuEntry *e;
};

DEFINE_MFLEX_CLASS(AmenuEntries, AmenuEntriesdata, 5);

ATKdefineRegistryNoInit(AmenuCardv, view);
AmenuCardv::AmenuCardv() {
    ed = new AmenuEntries;
    newhighlight=highlight=NULL;
}

AmenuCardv::~AmenuCardv() {
    delete ed;
}

view::DSattributes AmenuCardv::DesiredSize(long  width, long  height, enum view::DSpass  pass, long  *desired_width, long  *desired_height)
{
    AmenuCard &cr=*(AmenuCard *)GetDataObject();
    size_t i;
    long mlw=0, maxleftw=0, maxrightw=0, mh=0, mw=0;
    for(i=0;i<cr.GetN();i++) {
	long labelw=0, leftw=0, rightw=0, lh=0, tw=0;
	cr[i].ComputeSize(this, &labelw, &leftw, &rightw, &lh);
	if(labelw>mlw) mlw=labelw;
	if(leftw>maxleftw) maxleftw=leftw;
	if(rightw>maxrightw) maxrightw=rightw;
	sbuttonv::SizeForBorder(this, sbuttonv_Enclosing, cr.prefs, TRUE, mlw+maxleftw+maxrightw, lh, &tw, &lh);
	if(tw>mw) mw=tw;
	mh+=lh;
    }
    *desired_width=mw;
    *desired_height=mh;
    sbuttonv::SizeForBorder(this, sbuttonv_Enclosing, cr.prefs, TRUE, *desired_width, *desired_height, desired_width, desired_height);
    return (view::DSattributes)(view::HeightFlexible|view::WidthFlexible);
}
    
void AmenuCardv::FullUpdate(enum view::UpdateType type, long left, long top, long width, long height) {
    if(type!=view::FullRedraw && type!=view::LastPartialRedraw) return;
    AmenuCard *c=(AmenuCard *)GetDataObject();
    if(c==NULL) return;
    AmenuCard &cr=*c;
    rectangle bounds, enclosed, label;
    GetLogicalBounds(&bounds);
    sbuttonv::DrawRectBorder(this, &bounds, cr.prefs, FALSE, TRUE, &enclosed);

    long mlw=0, mh=0, count=0, maxleftw=0, maxrightw=0;
    cr.Sort();
    ed->Clear();
    {
	size_t i;
	for(i=0;i<cr.GetN();i++) {
	    AmenuEntriesdata *ae=ed->Append();
	    long labelw=0, leftw=0, rightw=0, lh=0;
	    cr[i].ComputeSize(this, &labelw, &leftw, &rightw, &lh);
	    if(labelw>mlw) mlw=labelw;
	    if(leftw>maxleftw) maxleftw=leftw;
	    if(rightw>maxrightw) maxrightw=rightw;
	    long tw=0;
	    sbuttonv::SizeForBorder(this, sbuttonv_Enclosing, cr.prefs, TRUE, tw, lh, &tw, &lh);
	    ae->r.height=lh;
	    ae->e=&cr[i];
	    mh+=lh;
	    count++;
	}
    }
    long tw=0;
    rectangle wenclosed;
    sbuttonv::DrawRectBorder(this, &enclosed, cr.prefs, TRUE, FALSE, &wenclosed);
    tw=wenclosed.width-maxleftw-maxrightw;
    /* if(tw>=mlw) { */
	label.left=wenclosed.left+maxleftw;
	label.width=tw;
    /* } else {
	label.left=wenclosed.left+wenclosed.width/2-mlw/2;
	label.width=mlw;
    } */
    highlight=newhighlight;
    enclosed.top=enclosed.top + (enclosed.height - mh) / 2;
    double bg[3];
    sbuttonv::InteriorBGColor(this, cr.prefs, FALSE, bg);
    SetBGColor(bg[0], bg[1], bg[2]);
    {
	for(size_t i=0;i<cr.GetN();i++) {
	    enclosed.height=(*ed)[i].r.height;
	    (*ed)[i].r=enclosed;
	    rectangle localenclosed;
	    sbuttonv::DrawRectBorder(this, &enclosed, cr.prefs, FALSE,&cr[i]==highlight, &localenclosed, FALSE);
	    label.top=localenclosed.top;
	    label.height=localenclosed.height;
	    cr[i].Draw(this, &localenclosed, &label, cr[i].sensitive, &cr[i]==highlight);
	    enclosed.top+=enclosed.height;
	}
    }
    FlushGraphics();
}

void AmenuCardv::Update() {
    size_t i;
    AmenuCard *c=(AmenuCard *)GetDataObject();
    if(c==NULL) return;
    if(highlight!=newhighlight) {
	if(highlight) {
	    for(i=0;i<ed->GetN() && (*ed)[i].e!=highlight;i++);
	    if(i<ed->GetN()) sbuttonv::EraseRectBorder(this, &(*ed)[i].r, c->prefs);
	}
	if(newhighlight) {
	    for(i=0;i<ed->GetN() && (*ed)[i].e!=newhighlight;i++);
	    if(i<ed->GetN()) sbuttonv::DrawRectBorder(this, &(*ed)[i].r, c->prefs,  FALSE, TRUE, NULL, FALSE); 
	}
	highlight=newhighlight;
	FlushGraphics();
    } // else FullUpdate(view::FullRedraw, 0, 0, 0, 0);
}

AmenuEntry *AmenuCardv::FindHit(long x, long y) {
    for(size_t i=0;i<this->ed->GetN();i++) {
	if((*this->ed)[i].e->sensitive && y>=(*this->ed)[i].r.top && y<(*this->ed)[i].r.top+(*this->ed)[i].r.height && x>=(*this->ed)[i].r.left && x<(*this->ed)[i].r.left+(*this->ed)[i].r.width) {
	    return (*this->ed)[i].e;
	}
    }
    return NULL;
}

class view * AmenuCardv::Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  {
    AmenuEntry *e=FindHit(x, y);
    newhighlight=e;
    if(action==view::LeftUp || action==view::RightUp) {
	if(e) e->Dispatch();
    }
    if(action==view::LeftUp || action==view::RightUp) newhighlight=NULL;
    
    if(newhighlight!=highlight) {
	WantUpdate(this);
	im::ForceUpdate();
    }
    return this;
}

ATKdefineRegistryNoInit(AmenuOption,AmenuCard);
ATKdefineRegistryNoInit(AmenuOptionv,view);

AmenuOption::AmenuOption() {
    selected=NULL;
    prefs=NULL;
}

AmenuOption::~AmenuOption() {
}

AmenuOptionv::AmenuOptionv() {
    card=NULL;
    win=NULL;
}
class NO_DLL_EXPORT AmenuOptionCardv : public AmenuCardv {
  public:
//    virtual ATKregistryEntry *ATKregistry();
    class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
};

AmenuOptionv::~AmenuOptionv() {
    win->Destroy();
    card->Destroy();
}

// There is no way to make this "private" using the macro
// The only reason to use this at all is in case someone inspects the
// type, and expects the ATK type info to be accurate.
//ATKdefineRegistryNoInit(AmenuOptionCardv,AmenuCardv);

class view *AmenuOptionCardv::Hit(enum view::MouseAction action, long x, long y, long numberOfClicks) {
    view *ret=this;
    AmenuEntry *e=FindHit(x, y);
    if(action==view::LeftUp || action==view::RightUp) {
	AmenuOption *ao=(AmenuOption *)GetDataObject();
	if(e) ao->selected=e;
	ao->NotifyObservers(observable::OBJECTCHANGED);
    }
    ret=AmenuCardv::Hit(action,x,y,numberOfClicks);
    if(action==view::LeftUp || action==view::RightUp) GetIM()->VanishWindow();
    return ret;
}


view::DSattributes AmenuOptionv::DesiredSize(long  width, long  height, enum view::DSpass  pass, long  *desired_width, long  *desired_height) {
    AmenuOption &cr=*(AmenuOption *)GetDataObject();
    size_t i;
    cr.EnsurePrefs();
    long mlw=0, maxleftw=0, maxrightw=0, mh=0, mw=0;
    for(i=0;i<cr.GetN();i++) {
	long labelw=0, leftw=0, rightw=0, lh=0, tw=0;
	cr[i].ComputeSize(this, &labelw, &leftw, &rightw, &lh);
	if(labelw>mlw) mlw=labelw;
	if(leftw>maxleftw) maxleftw=leftw;
	if(rightw>maxrightw) maxrightw=rightw;
	sbuttonv::SizeForBorder(this, sbuttonv_Enclosing, cr.prefs, TRUE, mlw+maxleftw+maxrightw, lh, &tw, &lh);
	if(tw>mw) mw=tw;
	if(lh>mh) mh=lh;
    }
    *desired_width=mw;
    *desired_height=mh;
    sbuttonv::SizeForBorder(this, sbuttonv_Enclosing, cr.prefs, TRUE, *desired_width, *desired_height, desired_width, desired_height);
    return (view::DSattributes)(view::HeightFlexible|view::WidthFlexible);
}

void AmenuOptionv::FullUpdate(enum view::UpdateType type, long left, long top, long width, long height) {
    if(type!=view::FullRedraw && type!=view::LastPartialRedraw) return;
    AmenuOption *c=(AmenuOption *)GetDataObject();
    if(c==NULL) return;
    AmenuOption &cr=*c;
    rectangle bounds, enclosed, label;
    GetLogicalBounds(&bounds);
    c->EnsurePrefs();
    sbuttonv::DrawRectBorder(this, &bounds, cr.prefs, FALSE, TRUE, &enclosed);

    if(cr.selected) {
	long labelw=0, leftw=0, rightw=0, lh=0;
	cr.selected->ComputeSize(this, &labelw, &leftw, &rightw, &lh);

	long tw=0;
	sbuttonv::SizeForBorder(this, sbuttonv_Enclosing, cr.prefs, TRUE, tw, lh, &tw, &lh);
	rectangle wenclosed;
	sbuttonv::DrawRectBorder(this, &enclosed, cr.prefs, TRUE, FALSE, &wenclosed);
	label.left=wenclosed.left+leftw;
	label.width=wenclosed.width-leftw-rightw;
	// wenclosed.top=wenclosed.top + (wenclosed.height - lh) / 2;
	double bg[3];
	sbuttonv::InteriorBGColor(this, cr.prefs, FALSE, bg);
	SetBGColor(bg[0], bg[1], bg[2]);
	rectangle localenclosed;
	sbuttonv::DrawRectBorder(this, &wenclosed, cr.prefs, FALSE, FALSE, &localenclosed, FALSE);
	label.top=localenclosed.top;
	label.height=localenclosed.height;
	cr.selected->Draw(this, &localenclosed, &label, TRUE, TRUE);
    }
    FlushGraphics();
}

void AmenuOptionv::Update() {
    FullUpdate(view::FullRedraw,0,0,0,0);
}

   
class view *AmenuOptionv::Hit (enum view::MouseAction action, long x, long y, long numberOfClicks) {
    if(action==view::LeftDown || action==view::RightUp) {
	if(card==NULL) card=new AmenuOptionCardv;
	if(card)  {
	    card->SetDataObject(GetDataObject());
	    if(win==NULL) win=popupwin::Create(GetIM(), card);
	    if(win) {
		AmenuOption &cr=*(AmenuOption *)GetDataObject();
		size_t i;
		cr.EnsurePrefs();
		long my=0;
		long mlw=0, maxleftw=0, maxrightw=0, mh=0, mw=0;
		card->newhighlight=cr.selected;
		cr.Sort();
		for(i=0;i<cr.GetN();i++) {
		    long labelw=0, leftw=0, rightw=0, lh=0, tw=0;
		    cr[i].ComputeSize(card, &labelw, &leftw, &rightw, &lh);
		    if(labelw>mlw) mlw=labelw;
		    if(leftw>maxleftw) maxleftw=leftw;
		    if(rightw>maxrightw) maxrightw=rightw;
		    sbuttonv::SizeForBorder(card, sbuttonv_Enclosing, cr.prefs, TRUE, mlw+maxleftw+maxrightw, lh, &tw, &lh);
		    if(cr.selected==&cr[i]) {
			my=mh+lh/2;
		    }
		    if(tw>mw) mw=tw;
		    mh+=lh;
		}
		sbuttonv::SizeForBorder(card, sbuttonv_Enclosing, cr.prefs, TRUE, mw, mh, &mw, &mh);
		rectangle enclosed, wenclosed;
		enclosed.left=enclosed.top=0;
		enclosed.width=mw;
		enclosed.height=mh;
		sbuttonv::DrawRectBorder(card, &enclosed, cr.prefs, FALSE, FALSE, &wenclosed);
		win->Post(this, x, y, action, mw/2, my + wenclosed.top, mw, mh);
		return win;
	    }

	}
    }
    return this;
}

