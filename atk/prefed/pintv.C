/* Copyright Carnegie Mellon University 1992 All rights reserved. */

#include <andrewos.h>
ATK_IMPL("pintv.H")


#define dontDefineRoutinesFor_observable
#define dontDefineRoutinesFor_view
#include "pintv.H"

#include <event.H>
#include <observable.H>
#include <text.H>
#include <textview.H>
#include <environment.H>
#include <stylesheet.H>
#include <style.H>
#include <sbuttonv.H>
#include <sbutton.H>
#include <lpair.H>
#include <label.H>
#include <labelview.H>
#include <view.H>
#include <rect.h>
#include <util.h>
#include <message.H>
#include <mark.H>
#include <environ.H>
#include <viewref.H>
#include <completion.H>
#include <dataobject.H>

#include <prefs.H>
#include <prefval.H>
#include <strcache.H>

#include <phelpv.H>

#include <pintv.h>

#define TEXT_VIEWREFCHAR '\377'

#define SaveStr strcache::SaveStr
#define SAVESTR(str) (str?SaveStr(str):NULL)
#define TEXT(tv) ((class text *)(tv)->GetDataObject())
#define PREFS(pv) ((class prefs *)(pv)->GetDataObject())
#define RFOLDEDEQ(x,y) ((x)==(y))

static const char texteditatomstr[]="TextEdit";

static const struct sbutton_list blist[]={
    {"Edit As Text", 0, texteditatomstr, FALSE},
    {NULL, 0, NULL, FALSE}
};


ATKdefineRegistry(pintv, lpair, pintv::InitializeClass);

static class labelview *MakeLabel(const char  *str);
static void DestroyLabel(class labelview  *lv);
static class textview *MakeText();
static void DestroyText(class textview  *tv, class view  *tva);
static void DestroyButtons(class sbuttonv  *sbv);
static void DestroyLpair(class lpair  *lp);
static void ClearLpair(class lpair  *lp);
static class environment *SelectLine(class textview  *tv, class environment  *oldsel, boolean  donew);
static class environment *AddView(class text  *self, long  pos, const char  *viewtype, class dataobject  *dataobject);
static class viewref *InsertObject(class text  *self, long  pos, const char  *name, const char  *viewname);
static class environment *AddStyle(class text  *self, long  pos , long  len, class style  *style);
static struct dstyle *GetDStyle(class text  *pt);
static void PushStyle(class text  *pt, long  pos , long  len, class style  *style);
static void PushView(class text  *pt, long  pos, const char  *view, class dataobject  *data);
static void PushObject(class text  *pt, long  pos, const char  *objname, const char  *dummy);
static void PushMark(class text  *pt, struct prefdesc  *pd, long  pos , long  len);
static void DoStyles(class pintv  *self);
static long AddVal(class text  *ct, long pos, const char  *label, const char  *val, boolean  bolditalic);
static boolean AddViews(char  *name, struct addrock  *rock);
static boolean AddCommentLines(char  *line, struct addrock  *rock);
static boolean getcondition(class pintv  *self, char  **current);
static boolean checkuniq(struct prefdesc  *pd, struct uniqrock  *ur);
static void FixPref(class pintv  *self, struct prefdesc  *pd);
static void AddPref(class pintv  *self, struct prefdesc  *pd, struct prefdesc  *pd2);
static void changecondition(class pintv  *self, class sbutton  *sb, struct prefdesc  *pd);
static void duplicate(class pintv  *self, class sbutton  *sb, struct prefdesc  *pd);
static void changeapp(class pintv  *self, class sbutton  *sb, struct prefdesc  *pd);
static void delete_c(class pintv  *self, class sbutton  *sb, struct prefdesc  *pd);
static void reset(class pintv  *self, class sbutton  *sb, struct prefdesc  *pd);
static void SetHelp(class pintv  *self, struct prefdesc  *pd);
static boolean AddInstances(struct prefdesc  *pd, struct addrock  *rock);
static boolean AddPreferences(struct prefdesc  *pd, class pintv  *self);
static boolean CheckPrefName(struct prefdesc  *name1 , struct prefdesc  *name2);
static boolean AddPreferencesToList(struct prefdesc  *pd, class pintv  *self);
static boolean RemoveMarks(struct prefdesc  *pd, struct addrock  *rock);
static int mycmp(struct prefdesc  *n1 , struct prefdesc  *n2);
static void UpdatePrefs(class pintv  *self);
static void ClearCPref(class pintv  *self);
static void UpdateCPref(class pintv  *self);
static boolean LocatePref(struct prefdesc  *pd, class pintv  *self);
static void MoveCPref(class pintv  *self);
static boolean FindNewHelp(struct prefdesc  *pd, class pintv  *self);
static void ReProcess(class pintv  *self);
static boolean VerifyPrefSanity(struct prefdesc  *pd2, class pintv  *self);
static boolean CheckSanity(struct prefdesc  *pd, class pintv  *self);
static boolean AddCategories(struct prefgroup  *pg, class pintv  *self);


static class labelview *MakeLabel(const char  *str)
{
    class label *l=new label;
    class labelview *lv=new labelview;

    if(l==NULL) {
	if(lv) (lv)->Destroy();
	return NULL;
    } else if(lv==NULL) {
	if(l) (l)->Destroy();
	return NULL;
    }

    (lv)->SetDataObject( l);

    (l)->SetFlags( label_CENTERED | label_BOXED);

    (l)->SetText( str);

    return lv;
}

static void DestroyLabel(class labelview  *lv)
{
    class label *l=(class label *)(lv)->GetDataObject();
    (l)->Destroy();
    (lv)->Destroy();
}

static class textview *MakeText()
{
    class text *t;
    class textview *tv;
    
    t=new text;
    
    if(t==NULL) return NULL;

    (t)->ReadTemplate( "prefs", FALSE);

    (t)->SetReadOnly( TRUE);
    
    tv=new textview;
    if(tv==NULL) {
	(t)->Destroy();
	return NULL;
    } 
    (tv)->SetDataObject( t);
    return tv;
}

static void DestroyText(class textview  *tv, class view  *tva)
{
    class text *t=TEXT(tv);
    (tv)->DeleteApplicationLayer( tva);
    (t)->Destroy();
    (tv)->Destroy();
}

static void DestroyButtons(class sbuttonv  *sbv)
{
    class sbutton *sb=(class sbutton *)(sbv)->GetDataObject();
    (sb)->Destroy();
    (sbv)->Destroy();
}

static const char currlab[]="---Current---";
static const char *currlabp=NULL;



pintv::pintv()
{
	ATKinit;

    struct sbutton_prefs *prefs=sbutton::GetNewPrefs("PrefEdButtons");
    class sbutton *sb;

    if(prefs==NULL) THROWONFAILURE( FALSE);

    this->autolist=environ::GetProfileSwitch("PrefEdAutoList", FALSE);
    sbutton::InitPrefs(prefs, "PrefEd");
    sbutton::InitPrefs(prefs, "PrefEdButtons");
  
    this->lockdown=FALSE;
    
    this->prefs = prefs;
    
    this->errors=NULL;
    this->reportevent=NULL;
    this->oevent=NULL;
    
    this->prefslist=NULL;
    
    this->cat_sel=this->pref_sel=NULL;

    currlabp=SaveStr(currlab);
    
    this->category=currlabp;
    if(this->category==NULL) {
	sbutton::FreePrefs(prefs);
	THROWONFAILURE( FALSE);
    }

    this->pref=NULL;

    this->helpv=NULL;
    this->helpva=NULL;
      
    this->top=new lpair;
    this->labelpair=new lpair;
    this->leftpair=new lpair;
    this->rightpair=new lpair;
    this->blr=new lpair;
    
    this->catlabel=MakeLabel("Categories");
    this->plabel=MakeLabel("Preferences");
    this->categories=MakeText();
    this->preferences=MakeText();
    this->cpref=MakeText();
    
    this->categories_al = (this->categories)->GetApplicationLayer();
    this->preferences_al = (this->preferences)->GetApplicationLayer();

    this->cpref_al = (this->cpref)->GetApplicationLayer();
    
    (this->labelpair)->HSplit( this->leftpair, this->rightpair, 60, TRUE);

    (this->leftpair)->VTFixed( this->catlabel, this->categories_al,  40, TRUE);
    (this->rightpair)->VTFixed( this->plabel, this->preferences_al, 40, TRUE);
    
    this->buttons = sbuttonv::CreateFilledSButtonv("sbuttonv", prefs, blist);
   
    sb=(class sbutton *)(this->buttons)->GetDataObject();
    (sb)->SetLayout( 1, 1, sbutton_GrowColumns);

    sbutton::FreePrefs(prefs);

    (sb)->AddRecipient( atom::Intern(texteditatomstr), this, (observable_fptr)pintv_EditAsText, 0);
    (this->top)->VSplit( this->labelpair, this->blr, 80, TRUE);
    (this->blr)->VSplit( this->cpref_al, NULL, 30, TRUE);
    (this)->VFixed( this->top, this->buttons, 40, TRUE);
    
    this->redosizes=TRUE;
    
    THROWONFAILURE( TRUE);
}

static void DestroyLpair(class lpair  *lp)
{
    (lp)->Destroy();
}

static void ClearLpair(class lpair  *lp)
{
    (lp)->SetNth( 0, NULL);
    (lp)->SetNth( 1, NULL);
}






void pintv::ObservedChanged(class observable  *changed, long  value)
{
    if((class observable *)this->errors==changed && value==-1) this->errors=NULL;
    if((class observable *)PREFS(this)==changed && value!=(-1) && !PREFS(this)->selfmod) {
	this->lockdown=TRUE;
	this->category=NULL;
	if(this->cat_sel) this->cat_sel=SelectLine(this->categories, this->cat_sel, FALSE);
	if(this->pref_sel) this->pref_sel=SelectLine(this->preferences, this->pref_sel, FALSE);
	UpdatePrefs(this);
	this->pref=NULL;
	if(this->autolist) UpdateCPref(this);
	else ClearCPref(this);
    }
    (this)->lpair::ObservedChanged( changed, value);
}

pintv::~pintv()
{
	ATKinit;

    if(this->errors) (this->errors)->RemoveObserver( this);
    ClearLpair((class lpair *)this);
    ClearLpair(this->top);
    ClearLpair(this->labelpair);
    ClearLpair(this->leftpair);
    ClearLpair(this->rightpair);
    ClearLpair(this->blr);
    if(this->reportevent) (this->reportevent)->Cancel();
    if(this->oevent) (this->oevent)->Cancel();
    if(this->errors) (this->errors)->Destroy();
    DestroyText(this->cpref, this->cpref_al);
    DestroyText(this->preferences, this->preferences_al);
    DestroyText(this->categories, this->categories_al);
    DestroyLabel(this->plabel);
    DestroyLabel(this->catlabel);
    DestroyButtons(this->buttons);
    DestroyLpair(this->top);
    DestroyLpair(this->labelpair);
    DestroyLpair(this->leftpair);
    DestroyLpair(this->rightpair);
    DestroyLpair(this->blr);
    if(this->helpva) (this->helpv)->DeleteApplicationLayer( this->helpva);
    if(this->helpv) (this->helpv)->Destroy();
}

boolean pintv::InitializeClass()
{
    return TRUE;
}

void pintv::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle bounds;
    (this)->GetVisualBounds( &bounds);
    if(this->redosizes) {
	long dw=bounds.width, dh=40;
	(this->buttons)->DesiredSize( bounds.width, bounds.height, view_WidthSet, &dw, &dh);
	this->redosizes=FALSE;
	if(dh<bounds.height) {
	    (this)->GetObjSize( 1)=dh;	
	    ((class lpair *)this)->needsfull=TRUE;
	}
	(this->catlabel)->DesiredSize( bounds.width, bounds.height, view_WidthSet, &dw, &dh);
	if(dh<bounds.height-(this)->GetObjSize( 1)) {
	    (this->leftpair)->GetObjSize( 0)=dh;
	    (this->rightpair)->GetObjSize( 0)=dh;
	    ((class lpair *)this)->needsfull=TRUE;
	}	    
    }
    (this)->lpair::FullUpdate( type, left, top, width, height);
}

static class environment *SelectLine(class textview  *tv, class environment  *oldsel, boolean  donew)
{
    class text *t=TEXT(tv);
    long line=(t)->GetLineForPos( (tv)->GetDotPosition());
    long pos=(t)->GetPosForLine( line);
    long lend=(t)->Index( pos, '\n', (t)->GetLength()-pos);
    class stylesheet *ss=(t)->GetStyleSheet();
    class style *bold;
    if(oldsel) {
	(t)->RegionModified( (oldsel)->Eval(), (oldsel)->GetLength());
	(oldsel)->Remove( (oldsel)->Eval(), (oldsel)->GetLength(), environment_Style, FALSE);
    }
    if(!donew) {
	(t)->NotifyObservers( observable::OBJECTCHANGED);
	return NULL;
    }
    if(lend<0 || ss==NULL ) return NULL;
    bold=(ss)->Find( "bold");
    if(bold==NULL) return NULL;
    oldsel=(t)->AlwaysAddStyle( pos, lend-pos, bold);
    (t)->NotifyObservers( observable::OBJECTCHANGED);
    return oldsel;
}


struct addrock {
    class pintv *self;
    class style *bold;
    class style *prefstyle;
    struct sbutton_prefs *prefs;
    long lastpos, npos;
    struct prefdesc *cpd;
};

static class environment *AddView(class text  *self, long  pos, const char  *viewtype, class dataobject  *dataobject)
{
    class environment *newenv=(self)->AlwaysWrapViewChar( pos, viewtype, dataobject);
  /* yes, this looks weird, but the viewref takes ownership of the object.... so all this does is decrement the reference count the code which adds prefvals to the queue grabs an extra reference so that this destroy won't remove the prefs ownership of the prefval */
    (dataobject)->Destroy();
    return newenv;
}

static class viewref *InsertObject(class text  *self, long  pos, const char  *name, const char  *viewname)
{
    class dataobject *newobject;
    class environment *env;

    if((newobject = (class dataobject *) ATK::NewObject(name)))  {
        /* Register the object with the dictionary */
        /* is this needed? dictionary_Insert(NULL, (char *) newobject->id, (char *) newobject); */
        if (viewname == NULL || *viewname == '\0')
	    viewname = (newobject)->ViewName();
        if ((env = AddView(self, pos, viewname, newobject)))
            return env->data.viewref;
    }
    return NULL;
}

static class environment *AddStyle(class text  *self, long  pos , long  len, class style  *style)
{
    class environment *newenv;
    newenv = (self->rootEnvironment)->InsertStyle( pos, style, TRUE);
    (newenv)->SetLength( len);
    (newenv)->SetStyle( FALSE, FALSE);
    return newenv;
}


static struct dstyle {
    long pos, len;
    class dataobject *data;
    class style *style;
    const char *view;
    struct prefdesc *pd;
    class text *pt;
    struct dstyle *next;
} *firststyle=NULL, *freedstyle=NULL;

static struct dstyle *GetDStyle(class text  *pt)
{
    struct dstyle *result;
    if(freedstyle) {
	result=freedstyle;
	freedstyle=freedstyle->next;
    } else {
	result=(struct dstyle *)malloc(sizeof(struct dstyle));
    }
    result->pt=pt;
    result->next=firststyle;
    firststyle=result;
    return result;
}

static void PushStyle(class text  *pt, long  pos , long  len, class style  *style)
{
    struct dstyle *n=GetDStyle(pt);
    if(n==NULL) return;
    n->view=NULL;
    n->data=NULL;
    n->pd=NULL;
    n->pos=pos;
    n->len=len;
    n->style=style;
}

static char viewrefchar='\377';
static void PushView(class text  *pt, long  pos, const char  *view, class dataobject  *data)
{
    struct dstyle *n=GetDStyle(pt);
    if(n==NULL) return;
    (pt)->AlwaysInsertCharacters( pos, &viewrefchar, 1);
    n->pos=pos;
    n->view=view;
    n->data=data;
    n->style=NULL;
    n->pd=NULL;
}

static void PushObject(class text  *pt, long  pos, const char  *objname, const char  *dummy)
{
    struct dstyle *n=GetDStyle(pt);
    if(n==NULL) return;
    (pt)->AlwaysInsertCharacters( pos, &viewrefchar, 1);
    n->pos=pos;
    n->view=objname;
    n->data=NULL;
    n->style=NULL;
    n->pd=NULL;
}

static void PushMark(class text  *pt, struct prefdesc  *pd, long  pos , long  len)
{
    struct dstyle *n=GetDStyle(pt);
    if(n==NULL) return;
    n->data=NULL;
    n->pd=pd;
    n->pos=pos;
    n->len=len;
    n->style=NULL;
    n->data=NULL;
    n->view=NULL;
}

static void DoStyles(class pintv  *self)
{
    struct dstyle *d=firststyle, *next;
    while(d) {
	next=d->next;
	if(d->pd) {
	    struct prefdesc *pd=d->pd;
	    if(pd->mark==NULL) {
		pd->mark=(d->pt)->CreateMark( d->pos, d->len);
		if(pd->mark) (pd->mark)->SetStyle( FALSE, FALSE);
	    } else {
		(pd->mark)->SetPos( d->pos);
		(pd->mark)->SetLength( d->len);
		(pd->mark)->SetStyle( FALSE, FALSE);
	    }
	} else if(d->style) {
	    AddStyle(d->pt, d->pos, d->len, d->style);
	} else if(d->data) {
	    AddView(d->pt, d->pos, d->view, d->data);
	} else if(d->view) {
	    InsertObject(d->pt, d->pos, d->view, NULL);
	}
	d->next=freedstyle;
	freedstyle=d;
	d=next;
    }
    firststyle=NULL;
}

static long AddVal(class text  *ct, long pos, const char  *label, const char  *val, boolean  bolditalic)
{
    class stylesheet *ss=(ct)->GetStyleSheet();
    class style *bold=(ss)->Find( bolditalic?"bold":"italic");
    long len=strlen(label);
    (ct)->AlwaysInsertCharacters( pos, label, len);
    (ct)->AlwaysInsertCharacters( pos+len, ": ", 2);
#ifdef HIGHLIGHT_LABELS    
    if(bold) PushStyle(ct, pos, len, bold);
#endif /* HIGHLIGHT_LABELS */
    pos+=len+2;
    len=strlen(val);
    (ct)->AlwaysInsertCharacters( pos, val, len);
    (ct)->AlwaysInsertCharacters( pos+len, "\n", 1);
#ifndef HIGHLIGHT_LABELS /* HIGHLIGHT_VALUES */
    if(bold) PushStyle(ct, pos, len, bold);
#endif /* ndef HIGHLIGHT_LABELS */
    return pos+len+1;
}

static boolean AddViews(char  *name, struct addrock  *rock)
{
    struct prefdesc *pd=rock->cpd;
    class text *ct=TEXT(rock->self->cpref);
    (ct)->AlwaysInsertCharacters( rock->lastpos, "\n", 1);
    /* for convenience dealing with the other objects
     AddView assumes the owner doesn't want the object any
     more, for the prefvals we do want them, so we grab extra
     ownership of the object here. */
    (pd->obj)->Reference();
    PushView(ct, rock->lastpos+1, name, (class dataobject *)pd->obj);
    rock->lastpos+=2;
    return TRUE;
}

static boolean AddCommentLines(char  *line, struct addrock  *rock)
{
    int len=strlen(line);
    class text *pt=TEXT(rock->self->cpref);
    (pt)->AlwaysInsertCharacters( rock->lastpos, line, strlen(line));
    (pt)->AlwaysInsertCharacters( rock->lastpos+len, "\n", 1);
    rock->lastpos+=len+1;
    return TRUE;
}

static const char deleteatomstr[]="Delete";
static const char changeatomstr[]="ChangeCondition";
static const char duplicateatomstr[]="Duplicate";
static const char changeappatomstr[]="ChangeApp";
static const char resetatomstr[]="Reset";


static struct sbutton_list blistt[]={
    {
	"Duplicate",
	0,
	duplicateatomstr,
	FALSE
    },
    {
	NULL, /* this will be filled in or not based on whether
	       a deletion of the given preference should be allowed */
	0,
	deleteatomstr,
	FALSE
    },
    {
	NULL,
	0,
	NULL,
	FALSE
    }

};

static struct sbutton_list blistc[]={
     {
	"Change",
	0,
	changeappatomstr,
	FALSE
    },
    {
	NULL,
	0,
	NULL,
	FALSE
    }
};


static struct sbutton_list blistc2[]={
    {
	"Change",
	0,
	changeatomstr,
	FALSE
    },
    {
	NULL,
	0,
	NULL,
	FALSE
    }
};

static struct sbutton_list blistd[]=
{
    {
	"Reset",
	0,
	resetatomstr,
	FALSE
    },
    {
	NULL,
	0,
	NULL,
	FALSE
    }
};


enum Choices {
    None,
    CPUType,
    MachName,
    EnvVar,
    Cancel
};

static const char * const choices[]={
    "Always",
    "CPU Type",
    "Machine Name",
    "Environment Variable",
    "Cancel",
    NULL
};


static boolean getcondition(class pintv  *self, char  **current)
{
    long result;
    Choices num=None;
    const char *prompt = NULL;
    char *def=NULL;
    char buf[1024];
    static char defbuf[1024];

    if(*current) {
	switch(*current[0]) {
	    case 'C':
		num=CPUType;
		break;
	    case 'M':
		num=MachName;
		break;
	    case 'E':
		num=EnvVar;
		break;
	}
    }
    if(message::MultipleChoiceQuestion(self, 100, "Type of Conditional?", num, &result, choices, NULL)<0 || result==Cancel) {
	message::DisplayString(self, 0, "Cancelled!");
	return TRUE;
    }

    if(*current) def= *current+2;
    switch(result) {
	case CPUType: 
	    prompt="CPU Type?";
	    break;
	case MachName:
	    prompt="Machine Name?";
	    break;
	case EnvVar:
	    prompt="Expression like ENV=b, or ENV!=b.";
	    def=NULL;
	    if(*current) {
		if(*((*current)+1)=='!') {
		    char *c= *current+2;
		    if(strlen(c)>sizeof(defbuf)-1) break;
		    strcpy(defbuf, c);
		    c=strchr(defbuf, '=');
		    if(c==NULL) break;
		    *c='\0';
		    c=strchr(*current+2, '=');
		    strcat(defbuf, "!");
		    strcat(defbuf, c);
		    def=defbuf;
		} else def= *current+2;
	    } 
	    break;
	case None:
	    *current=NULL;
	    return FALSE;
    }

    if(message::AskForString(self, 100, prompt, (num==result)?def:NULL, buf, sizeof(buf))<0) {
	message::DisplayString(self, 0, "Cancelled!");
	return TRUE;
    }

    switch(result) {
	case CPUType:
	case MachName:
	    defbuf[0]=(result==CPUType)?'C':'M';
	    defbuf[1]='=';
	    strcpy(defbuf+2, buf);
	    break;
	case EnvVar: {
	    char *ne=strchr(buf, '!');
	    defbuf[0]='E';
	    defbuf[2]='\0';
	    if(ne) {
		defbuf[1]='!';
		if(ne[1]=='=') {
		    ne[0]='\0';
		    strcat(defbuf, buf);
		    strcat(defbuf, "=");
		    strcat(defbuf, ne+2);
		} else {
		    message::DisplayString(self, 0, "Invalid expression for environment variable test.");
		    return TRUE;
		}
	    } else {
		ne=strchr(buf, '=');
		defbuf[1]='=';
		if(ne==NULL) {
		    message::DisplayString(self, 0, "Invalid expression for environment variable test.");
		    return TRUE;
		}
		strcat(defbuf, buf);
	    }
	    }	    
    }
    *current=defbuf;
    return FALSE;
}

struct uniqrock {
    const char *prefname;
    const char *appname;
    char *cond;
};


static boolean checkuniq(struct prefdesc  *pd, struct uniqrock  *ur)
{
    if(strcmp(pd->name, ur->prefname)==0 && strcmp(pd->app, ur->appname)==0 && ((pd->cond==ur->cond) || (pd->cond && ur->cond && strcmp(pd->cond, ur->cond)==0))) return FALSE;
    return TRUE;
}




static void FixPref(class pintv  *self, struct prefdesc  *pd)
{
    class stylesheet *ss;
    struct addrock rock;
    
    if(pd->mark==NULL) {
	message::DisplayString(self, 100, "Couldn't find area in prefs text to delete!");
	return;
    }

    (TEXT(self->cpref))->AlwaysDeleteCharacters( (pd->mark)->GetPos(), (pd->mark)->GetLength());


    ss=(TEXT(self->cpref))->GetStyleSheet();
    if(ss==NULL) return;

    rock.self=self;
    rock.bold = (ss)->Find( "bold");
    rock.prefstyle = (ss)->Find( "prefname");
    rock.prefs=self->prefs;
    rock.lastpos = 0;
    rock.npos = (pd->mark)->GetPos();
    AddInstances(pd, &rock);
    DoStyles(self);
    (TEXT(self->cpref))->NotifyObservers( observable::OBJECTCHANGED);
}

static void AddPref(class pintv  *self, struct prefdesc  *pd, struct prefdesc  *pd2)
{
    class stylesheet *ss;
    struct addrock rock;
    
    if(pd->mark==NULL) {
	message::DisplayString(self, 100, "Couldn't find area in prefs text to insert duplicate!");
	return;
    }

    ss=(TEXT(self->cpref))->GetStyleSheet();
    if(ss==NULL) return;

    rock.self=self;
    rock.bold = (ss)->Find( "bold");
    rock.prefstyle = (ss)->Find( "prefname");
    rock.prefs=self->prefs;
    rock.lastpos = 0;
    rock.npos = (pd->mark)->GetPos()+(pd->mark)->GetLength();
    AddInstances(pd2, &rock);
    DoStyles(self);
    if(self->cpref && pd2->mark) (self->cpref)->SetTopPosition( (pd2->mark)->GetPos());
    (TEXT(self->cpref))->NotifyObservers( observable::OBJECTCHANGED);
}

static void changecondition(class pintv  *self, class sbutton  *sb, struct prefdesc  *pd)
{
    struct uniqrock ur;
    char *newcond=pd->cond;
    struct prefdesc *pd2;
    
    if(getcondition(self, &newcond)) return;

    ur.cond=newcond;
    ur.appname=pd->app;
    ur.prefname=pd->name;
    
    pd2=(struct prefdesc *)(PREFS(self)->prefsp)->Enumerate( (list_efptr)checkuniq, (char *)&ur);
    if(pd2) {
	message::DisplayString(self, 30, "Such a preference for that application already exists!");
	return;
    }

    pd->cond=newcond?strdup(newcond):NULL;
    (pd->obj)->SetCondition( pd->cond);
    (pd->obj)->NotifyObservers( observable::OBJECTCHANGED);
    FixPref(self, pd);
    message::DisplayString(self, 0, "Condition changed.");
}

static void duplicate(class pintv  *self, class sbutton  *sb, struct prefdesc  *pd)
{
    struct uniqrock ur;
    class prefs *prefs=PREFS(self);
    char buf[1024];
    struct prefdesc *pd2;
    
    if(message::AskForString(self, 100,"New preference for which application?", NULL, buf, sizeof(buf))<0) {
	message::DisplayString(self, 0, "Cancelled!");
	return;
    }
    
    ur.cond=pd->cond;
    if(getcondition(self, &ur.cond)) return;
    ur.prefname=pd->name;
    ur.appname=buf;

    pd2=(struct prefdesc *)(PREFS(self)->prefsp)->Enumerate( (list_efptr)checkuniq, (char *)&ur);
    if(pd2) {
	message::DisplayString(self, 30, "Such a preference for that application already exists!");
	return;
    }
    pd2=(prefs)->DuplicatePref( pd, buf, ur.cond);
    PREFS(self)->selfmod=TRUE;
    (PREFS(self))->NotifyObservers( observable::OBJECTCHANGED);
    PREFS(self)->selfmod=FALSE;
    AddPref(self, pd, pd2);
    message::DisplayString(self, 0, "Preference duplicated.");
}

static void changeapp(class pintv  *self, class sbutton  *sb, struct prefdesc  *pd)
{
    struct uniqrock ur;
    char buf[1024];
    struct prefdesc *pd2;
    if(message::AskForString(self, 100,"Apply this preference to which application?", NULL, buf, sizeof(buf))<0) {
	message::DisplayString(self, 0, "Cancelled!");
	return;
    }
    
    ur.prefname=pd->name;
    ur.appname=buf;
    ur.cond=pd->cond;
    pd2=(struct prefdesc *)(PREFS(self)->prefsp)->Enumerate( (list_efptr)checkuniq, (char *)&ur);
    if(pd2) {
	message::DisplayString(self, 30, "Such a preference for that application already exists!");
	return;
    }
    pd->app=SaveStr(buf);
    (pd->obj)->SetAppName( pd->app);

    FixPref(self, pd);
    message::DisplayString(self, 0, "Application changed.");
}


static void delete_c(class pintv  *self, class sbutton  *sb, struct prefdesc  *pd)
{
    long dotpos=(self->preferences)->GetDotPosition();
    long toppos=(self->preferences)->GetTopPosition();
    if(pd->mark==NULL) {
	message::DisplayString(self, 100, "Couldn't find area in prefs text to delete!");
	return;
    }
    
    (TEXT(self->cpref))->AlwaysDeleteCharacters( (pd->mark)->GetPos(), (pd->mark)->GetLength());
    
    (TEXT(self->cpref))->RemoveMark( pd->mark);
    (TEXT(self->cpref))->NotifyObservers( observable::OBJECTCHANGED);
    
    (PREFS(self))->DeletePref( pd);

    UpdatePrefs(self);
    
    if(dotpos>(TEXT(self->preferences))->GetLength()) {
	dotpos=(TEXT(self->preferences))->GetLength();
    }
	
    if(toppos>(TEXT(self->preferences))->GetLength()) {
	toppos=(TEXT(self->preferences))->GetLength();
    }
    (self->preferences)->SetTopPosition( toppos);
    (self->preferences)->SetDotPosition( dotpos);
    (self->preferences)->WantUpdate( self->preferences);
    
    PREFS(self)->selfmod=TRUE;
    (PREFS(self))->NotifyObservers( observable::OBJECTCHANGED);
    PREFS(self)->selfmod=FALSE;
    message::DisplayString(self, 0, "Preference deleted.");
}

static void reset(class pintv  *self, class sbutton  *sb, struct prefdesc  *pd)
{
    if(pd->indefs) {
	(PREFS(self))->DeletePref( pd);
	return;
    }
    
    if(pd->val) {
	(pd->obj)->SetFromPreferenceString( pd->val);
    } else {
	(pd->obj)->SetFromPreferenceString( "");
    }
    (pd->obj)->NotifyObservers( observable::OBJECTCHANGED);
    message::DisplayString(self, 0, "Preference reset to default.");
}


static void SetHelp(class pintv  *self, struct prefdesc  *pd)
{
    if(pd->helppos>=0 && self->helpv) {
	(self->helpv)->SetTopPosition( pd->helppos);
	(self->helpv)->SetDotPosition( pd->helppos);
	(self->helpv)->SetDotLength( 0);
	(self->helpv)->FrameDot( pd->helppos);
    }
}


#define INCLUDE(self, pd, autolist) ((autolist || pd->name==self->pref) && (pd->group==self->category || (self->category==currlabp && pd->order>=0)))

static boolean sethelp=FALSE;

static boolean AddInstances(struct prefdesc  *pd, struct addrock  *rock)
{
    class pintv *self=rock->self;

    if(INCLUDE(self, pd, self->autolist)) {
	static const char editors[]="Current Setting (";
	static const char comments[]="Comments";
	class text *pt=TEXT(self->cpref);
	class style *bold=rock->bold;
	struct sbutton_prefs *prefs=rock->prefs;
	class sbutton *sb;
	long pos,len;
	class style *prefstyle=rock->prefstyle;
	const char *val;

	if(sethelp) {
	    sethelp=FALSE;
	    SetHelp(self, pd);
	}

	/* if this entry is from the defaults then
	    it cannot be deleted. */
	
	if(pd->indefs) {
	    blistt[1].label=NULL;
	} else blistt[1].label="Delete";
	
	sb=sbutton::CreateFilledSButton(prefs, blistt);
	if(sb==NULL) return TRUE;
	(sb)->SetLayout( 1, 1, sbutton_GrowColumns);
	(sb)->AddRecipient( atom::Intern(duplicateatomstr), self, (observable_fptr)duplicate, (long)pd);
	(sb)->AddRecipient( atom::Intern(deleteatomstr), self, (observable_fptr)delete_c, (long)pd);
	pos=rock->npos;
	PushView(pt, pos, "sbuttonv", (class dataobject *)sb);
	pos++;
	(pt)->AlwaysInsertCharacters( pos, "\n", 1);
	pos++;
	len=strlen(pd->name);
	(pt)->AlwaysInsertCharacters( pos, pd->name, len);
	(pt)->AlwaysInsertCharacters( pos+len, "\n\n", 2);
	if(prefstyle!=NULL) {
	   PushStyle(pt, pos, len,  prefstyle);
	}
	pos+=len+2;
	
	sb=sbutton::CreateFilledSButton(prefs, blistc);
	if(sb==NULL) return TRUE;

	(sb)->SetLayout( 1, 1, sbutton_GrowRows);
	(sb)->AddRecipient( atom::Intern(changeappatomstr), self, (observable_fptr)changeapp, (long)pd);
	PushView(pt, pos, "sbuttonv", (class dataobject *)sb);
	pos++;
	pos=AddVal(pt, pos, "Application", pd->app, TRUE);
	
	
	sb=sbutton::CreateFilledSButton(prefs, blistc2);
	if(sb==NULL) return TRUE;

	(sb)->SetLayout( 1, 1, sbutton_GrowRows);
	(sb)->AddRecipient( atom::Intern(changeatomstr), self, (observable_fptr)changecondition, (long)pd);
	PushView(pt, pos, "sbuttonv", (class dataobject *)sb);
	pos++;
	pos=AddVal(pt, pos, "Conditional", pd->cond?pd->cond:"Always", TRUE);


	if(pd->val) {
	    (pt)->AlwaysInsertCharacters( pos, editors, sizeof(editors)-1);

	    sb=sbutton::CreateFilledSButton(prefs, blistd);
	    if(sb==NULL) return TRUE;

	    (sb)->SetLayout( 1, 1, sbutton_GrowRows);
	    (sb)->AddRecipient( atom::Intern(resetatomstr), self, (observable_fptr)reset, (long)pd);
	    PushView(pt, pos+sizeof(editors)-1, "sbuttonv", (class dataobject *)sb);
	    PushStyle(pt, pos, sizeof(editors)-2, bold);
	    pos+=sizeof(editors) /* - 1  REMOVE -1 WHEN doing views */;
	    (pt)->AlwaysInsertCharacters( pos, " to ", 4);
	    val=pd->val?pd->val:"<None>";
	    len=strlen(val);
	    (pt)->AlwaysInsertCharacters( pos+4, val, len);

	    (pt)->AlwaysInsertCharacters( pos+4+len, ")", 1);
	    pos+=5+len;
	}
	if(pd->defviews) {
	    rock->lastpos=pos;
	    rock->cpd=pd;
	    (pd->defviews)->Enumerate((list_efptr)AddViews, (char *)rock);
	    pos=rock->lastpos;
	}
	(pt)->AlwaysInsertCharacters( pos, "\n", 1);
	pos++;
	
	if(pd->prevlines && (pd->prevlines)->Size()>0) {
	    (pt)->AlwaysInsertCharacters( pos, comments, sizeof(comments)-1);
	    (pt)->AlwaysInsertCharacters( pos+sizeof(comments)-1, "\n", 1);
	    PushStyle(pt, pos, sizeof(comments)-1, bold);
	    rock->lastpos=pos+sizeof(comments);
	    (pd->prevlines)->Enumerate( (list_efptr)AddCommentLines, (char *)rock);
	    pos=rock->lastpos;
	}
	PushObject(pt, pos, "bp", NULL);
	pos++;
	(pt)->AlwaysInsertCharacters( pos, "\n", 1);
	pos++;
	PushMark(pt, pd, rock->npos, pos-rock->npos);
	rock->npos=pos;
    }
    return TRUE;
}

static boolean AddPreferences(struct prefdesc  *pd, class pintv  *self)
{
    const char *name=pd->name;
    class text *pt=TEXT(self->preferences);
    (pt)->AlwaysInsertCharacters( (pt)->GetLength(), name, strlen(name));
    (pt)->AlwaysInsertCharacters( (pt)->GetLength(), "\n", 1);
    return TRUE;
}

static boolean CheckPrefName(struct prefdesc  *name1 , struct prefdesc  *name2)
{
    return !RFOLDEDEQ(name1->name, name2->name);
}

static boolean AddPreferencesToList(struct prefdesc  *pd, class pintv  *self)
{
    if(INCLUDE(self, pd, TRUE) && !pd->shadow) {
	if((self->prefslist)->Enumerate((list_efptr)CheckPrefName, (char *)pd)) return TRUE;
	(self->prefslist)->InsertEnd((char *)pd);
    }
    return TRUE;
}

static boolean RemoveMarks(struct prefdesc  *pd, struct addrock  *rock)
{
    class pintv *self=rock->self;
    if(pd->mark) {
	(TEXT(self->cpref))->RemoveMark( pd->mark);
	delete pd->mark;
	pd->mark=NULL;
    }
    return TRUE;
}

static int mycmp(struct prefdesc  *n1 , struct prefdesc  *n2)
{
    int result=ULstrcmp(n1->name, n2->name);
    return result;
}

static void UpdatePrefs(class pintv  *self)
{

    self->pref_sel=NULL;
    (TEXT(self->preferences))->AlwaysDeleteCharacters( 0, (TEXT(self->preferences))->GetLength());

    if(self->prefslist) delete self->prefslist;

    self->prefslist=new list;
    if(self->prefslist==NULL) return;
    (PREFS(self)->prefsp)->Enumerate((list_efptr)AddPreferencesToList, (char *)self);
    (self->prefslist)->Sort((list_greaterfptr) mycmp);
    (self->prefslist)->Enumerate((list_efptr)AddPreferences, (char *)self);
    (TEXT(self->preferences))->NotifyObservers( 0);
}

static void ClearCPref(class pintv  *self)
{
    class text *ct=TEXT(self->cpref);
   /* class environment *rt;
    rt = ct->rootEnvironment;
    ct->rootEnvironment = NULL;
    (rt)->FreeTree();
    ct->rootEnvironment = environment::GetRootEnvironment();
*/
    (ct)->AlwaysDeleteCharacters( 0, (ct)->GetLength());

    /* horrible hack... please forgive me. -rr2b
     this convinces textview to remove most of its marks
     from the text, thus speeding up the text changes
     which are about to be done.
     */
    (self->cpref)->SetDataObject( ct);
}

static void UpdateCPref(class pintv  *self)
{
    class text *ct=TEXT(self->cpref);
    class stylesheet *ss;
    struct addrock rock;
#ifdef TIMEY
    struct timeval a,b;

    gettimeofday(&b, NULL);
    printf("time before:%lf\n", (((double)b.tv_sec)*1000000.0+(double)b.tv_usec)/1000000);
#endif /* TIMEY */
    ClearCPref(self);
    
    ss=(ct)->GetStyleSheet();
    if(ss==NULL) return;
    
    rock.self=self;
    rock.bold = (ss)->Find( "bold");
    rock.prefstyle = (ss)->Find( "prefname");
    rock.prefs=self->prefs;
    rock.lastpos = rock.npos = 0;
    
    sethelp=TRUE;
    (PREFS(self)->prefsp)->Enumerate((list_efptr)RemoveMarks, (char *)&rock);
    (PREFS(self)->prefsp)->Enumerate((list_efptr)AddInstances, (char *)&rock);
    DoStyles(self);
    (ct)->NotifyObservers( observable::OBJECTCHANGED);
#ifdef TIMEY
    gettimeofday(&a, NULL);
    printf("time after:%lf\n", (((double)a.tv_sec)*1000000.0+(double)a.tv_usec)/1000000);
    printf("time:%lf\n", ((double)(((double)a.tv_sec)*1000000+(double)a.tv_usec)-(((double)b.tv_sec)*1000000+(double)b.tv_usec))/1000000);
#endif /* TIMEY */
}


static boolean LocatePref(struct prefdesc  *pd, class pintv  *self)
{
    if(INCLUDE(self, pd, FALSE)) return FALSE;
    else return TRUE;
}

static void MoveCPref(class pintv  *self)
{
    struct prefdesc *pd=(struct prefdesc *)(PREFS(self)->prefsp)->Enumerate((list_efptr)LocatePref, (char *)self);
    if(pd && pd->mark && (pd->mark)->GetPos()>=0) {
	(self->cpref)->SetTopPosition( (pd->mark)->GetPos());
	(self->cpref)->SetDotPosition( (pd->mark)->GetPos());
	(self->cpref)->SetDotLength( 0);
	SetHelp(self, pd);
    } else {
	char buf[1024];
	sprintf(buf, "Couldn't locate instance of preference %s.\n", self->pref);
	message::DisplayString(self, 0, buf);
    }
}

static boolean FindNewHelp(struct prefdesc  *pd, class pintv  *self)
{
    long pos=(self->cpref)->GetDotPosition();
    if(pd->mark && pos >= (pd->mark)->GetPos() && pos<=(pd->mark)->GetEndPos()) {
	SetHelp(self, pd);
	return FALSE;
    }
    return TRUE;
}

static void ReProcess(class pintv  *self)
{
    message::DisplayString(self, 0, "Processing changes from text.");
    (PREFS(self))->ReScan();
    PREFS(self)->selfmod=TRUE;
    (PREFS(self))->NotifyObservers( observable::OBJECTCHANGED);
    PREFS(self)->selfmod=FALSE;
    self->lockdown=FALSE;
}

class view *pintv::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
{
    char buf[1024];
    class view *result;
    if(this->lockdown) {
	ReProcess(this);
    }
    result=(this)->lpair::Hit( action, x, y, numberOfClicks);
    if(result==(class view *)this->categories) {
	this->cat_sel=SelectLine(this->categories, this->cat_sel, TRUE);
	if(this->cat_sel==NULL) return (class view *)this;
	(TEXT(this->categories))->CopySubString( (this->cat_sel)->Eval(), (this->cat_sel)->GetLength(), buf, FALSE);
	this->category=SaveStr(buf);	
	UpdatePrefs(this);
	this->pref=NULL;
	if(this->autolist) UpdateCPref(this);
	else ClearCPref(this);
	return (class view *)this;
    } else if(result==(class view *)this->preferences) {
	this->pref_sel=SelectLine(this->preferences, this->pref_sel, TRUE);
	if(this->pref_sel==NULL) return (class view *)this;
	(TEXT(this->preferences))->CopySubString( (this->pref_sel)->Eval(), (this->pref_sel)->GetLength(), buf, FALSE);
	this->pref=SaveStr(buf);
	if(this->autolist) {
	    MoveCPref(this);
	} else UpdateCPref(this);
	return (class view *)this;
    }
    return result;
}

static boolean VerifyPrefSanity(struct prefdesc  *pd2, class pintv  *self)
{
    struct prefdesc *pd=self->cpd;
    if(RFOLDEDEQ(pd2->name, pd->name) && pd2->order>pd->order) {
	pd->order=pd2->order;
	self->newerrors=TRUE;
	if(self->errors==NULL) {
	    self->errors=new text;
	}
	if(self->errors==NULL) {
	    fprintf(stderr, "prefs: Failed to create error buffer for sanity check output!\n");
	    fprintf(stderr, "prefs: sanity check failed on preference %s\n", pd->name);
	} else {
	    (self->errors)->AddObserver( self);
	    (self->errors)->AlwaysInsertCharacters( (self->errors)->GetLength(), "\t", 1);
	    (self->errors)->AlwaysInsertCharacters( (self->errors)->GetLength(), pd2->app, strlen(pd2->app));
	    (self->errors)->AlwaysInsertCharacters( (self->errors)->GetLength(), "\n", 1);
	}
    }
   return TRUE;
}

static boolean CheckSanity(struct prefdesc  *pd, class pintv  *self)
{
    if(pd->order>=0 && (pd->app==NULL || pd->app[0]=='\0' || (pd->app[0]=='*' && pd->app[1]=='\0'))) {
	self->cpd=pd;
	self->newerrors=FALSE;
	(PREFS(self)->prefsp)->Enumerate((list_efptr)VerifyPrefSanity, (char *)self);
	if(self->newerrors && self->errors) {
	    (self->errors)->AlwaysInsertCharacters( 0, "\n", 1);
	    (self->errors)->AlwaysInsertCharacters( 0, pd->name, strlen(pd->name));
	}
    }
    return TRUE;
}

static boolean AddCategories(struct prefgroup  *pg, class pintv  *self)
{
    class text *cat=TEXT(self->categories);
    (cat)->AlwaysInsertCharacters( (cat)->GetLength(), pg->name, strlen(pg->name));
    (cat)->AlwaysInsertCharacters( (cat)->GetLength(), "\n", 1);
    return TRUE;
}

 

void pintv::SetDataObject(class dataobject  *d)
{
    class prefs *prefs=(class prefs *)d;
    struct prefdesc *pd;
    class text *t=TEXT(this->categories);
    class stylesheet *ss=(t)->GetStyleSheet();
    class style *bold;
    struct prefgroup pg;

    pg.name=currlabp;
    pg.grouphelp=0;
    
    (this)->lpair::SetDataObject( d);
    
    if((!(prefs)->GetReadOnly()) && prefs->version==-1 && environ::GetProfileSwitch("KeepOrigPrefs", TRUE)) this->oevent=pintv_GetKeepEvent((long)this);
    if(ss==NULL) return;
    
    bold=(ss)->Find( "bold");
    if(bold==NULL) return;

    
    AddCategories(&pg, this);
    (prefs->categories)->Enumerate((list_efptr)AddCategories, (char *)this);
    pd=(struct prefdesc *)(prefs->prefsp)->Enumerate((list_efptr) CheckSanity, (char *)this);
    if(this->errors) {
	this->reportevent=pintv_GetReportEvent((long)this);
    }
    if(pd) {
	fprintf(stderr, "Preferences Warning: sanity check violation!\n");
	fprintf(stderr, "Sanity check failed on %s.\n", pd->name);
    }
    PREFS(this)->sane=TRUE;
    if(prefs->help) {
	this->helpv=new phelpv;
	if(this->helpv!=NULL) {
	    this->helpva = (this->helpv)->GetApplicationLayer();
	    (this->helpv)->SetDataObject( prefs->help);
	    (this->blr)->SetNth( 1, this->helpva);
	    (this->helpv)->SetPrefs( PREFS(this));
	}
    }
    this->cat_sel=AddStyle(t, 0, strlen(currlabp), bold);
    UpdatePrefs(this);
    if(this->autolist) UpdateCPref(this);
    else ClearCPref(this);
}

void pintv::WantInputFocus(class view  *requestor)
{
    (this)->lpair::WantInputFocus( requestor);
}

void pintv::WantUpdate(class view  *requestor)
{
    if((requestor==(class view *)this->cpref || requestor==(class view *)this->cpref_al)) {
	(PREFS(this)->prefsp)->Enumerate((list_efptr)FindNewHelp, (char *)this);
    }
    (this)->lpair::WantUpdate( requestor);
}
