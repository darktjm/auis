/* Copyright 1992 by Carnegie Mellon University. All rights Reserved. $Disclaimer: 
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

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/prefs.C,v 1.10 1994/11/30 20:42:06 rr2b Stab74 $ */

#ifndef NORCSID
static char *rcsid_prefs_c = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/prefs.C,v 1.10 1994/11/30 20:42:06 rr2b Stab74 $";
#endif /* NORCSID */

#include <andrewos.h>
ATK_IMPL("prefs.H")
#include <math.h>

#include <util.h>
#include <ctype.h>

#include <text.H>

#include "prefs.H"
#include "prefval.H"
#include <dictionary.H>
#include <environ.H>
#include <stylesheet.H>
#include <sbutton.H>
#include <im.H>
#include <strcache.H>
#include <environment.H>


ATKdefineRegistry(prefs, text, NULL);
#ifndef NORCSID
#endif /* NORCSID */
static int FindCat(struct prefgroup  *a, char  *b);
static void FreeHelpStyles(class prefs  *self);
static class environment *AddStyle(class text  *self, long  pos , long  len, class style  *style);
static void ApplyHelpStyles(class prefs  *self);
static boolean freestring(char  *name, long  rock);
static void freestringlist(class list  *vl);
static boolean freeall(struct prefdesc  *pd, class prefs  *rock);
static boolean freegroups(struct prefgroup  *pg, class prefs  *self);
static boolean AddViews(char  *vname, struct therock  *tr);
static struct prefdesc *NewPrefDesc(class prefs  *self, char  *p);
static boolean CopyViewName(char  *name, class list  *nl);
static class list *CopyViewList(class list  *vl);
static boolean CopyStr(char  *name, class list  *nl);
static class list *CopyList(class list  *vl);
static struct prefdesc *DuplicateDesc(struct prefdesc  *pd);
static boolean FixOrderValues(struct prefdesc  *pd, long  val);
static class list *HandleViewList(class prefs  *self, char  *buf);
static boolean FindDesc(struct prefdesc  *pd, struct thedescrock  *rock);
static boolean ListsEqual(class list  *l1 , class list  *l2);
static boolean InsertPref(struct prefdesc  *pd, class prefs  *self);
static void HandlePref(class prefs  *self, struct prefline  *line, int  *order);
static char *myindex(char  *p, char  ch);
static char * SetBuf(struct wbuf  *wb, char  *str);
#ifndef MAX
#endif
static void AddHelpStyle(class prefs  *self, long  pos, long  len, enum style_type  type);
static void AddCategory(class prefs  *self, char  *group, long  pos);
static boolean ReadPrefLine(class prefs  *self,struct prefline  *line);
static int sortbyname(struct prefdesc  *pd1 , struct prefdesc  *pd2);
static int sortbyapp(struct prefdesc  *pd1 , struct prefdesc  *pd2);
static int sortbygroup(struct prefdesc  *pd1 , struct prefdesc  *pd2);
static int sortbyorder(struct prefdesc  *pd1 , struct prefdesc  *pd2);
static int catsort(struct prefgroup  *pg1 , struct prefgroup  *pg2);
static boolean ResetOrder(struct prefdesc  *pd, class prefs  *self);
static boolean ResetValues(struct prefdesc  *pd, class prefs  *self);
static boolean PrefvalP(struct prefdesc  *pd, class prefval  *obj);
static boolean PrintPrevLines(char  *line, struct writerock  *rock);
static boolean writeprefs(struct prefdesc  *pd, struct writerock  *rock);
static void twrite(struct writerock  *rock, struct prefdesc  *pd);
static void filewrite(struct writerock  *rock, struct prefdesc  *pd);
static char *ReadLine(class text *self);

static int FindCat(struct prefgroup  *a, char  *b)
{
    return a->name==b?0:1;
}

#define STRDUP(x) (x?NewString(x):NULL)
#define SAVESTR(x) (x?strcache::SaveStr(x):NULL)
#define zfree(xxx) do { if(xxx) { free(xxx); (xxx)=NULL;}} while (0)

	

static void FreeHelpStyles(class prefs  *self)
{
    zfree(self->llist);
    self->llistsize=self->llistcnt=0;
}


static class environment *AddStyle(class text  *self, long  pos , long  len, class style  *style)
{
    class environment *newenv;
    newenv = (self->rootEnvironment)->InsertStyle( pos, style, TRUE);
    (newenv)->SetLength( len);
    (newenv)->SetStyle( FALSE, FALSE);
    return newenv;
}


static void ApplyHelpStyles(class prefs  *self)
{
    long i;
    class stylesheet *ss=(self->help)->GetStyleSheet();
    class style *b=NULL, *bb, *s;
    if(ss) {
	b=(ss)->Find( "subsection");
	bb=(ss)->Find( "groupname");
    }
    for(i=0;i<self->llistcnt;i++) {
	switch(self->llist[i].type) {
	    case PrefsSubsection:
		s=b;
		break;
	    case PrefsGroupname:
		s=bb;
		break;
	    default:
		continue;
	}
	AddStyle(self->help, self->llist[i].pos, self->llist[i].len, s);
    }
    FreeHelpStyles(self);
}

prefs::prefs()
{
    char *file;
    FILE *fp2;
    long err, errs=0;
    char *files, filenamebuf[1025];

    this->selfmod=FALSE;
    this->llistsize=this->llistcnt=0;
    this->llist=NULL;
    
    this->version=(-1);
    
    this->sane=FALSE;

    this->pstyle=this->hstyle=NULL;
    
    this->prefsp=new list;
    if(this->prefsp==NULL) {

	THROWONFAILURE( FALSE);
    }
    this->categories=new list;
    if(this->categories==NULL) {
	delete this->prefsp;
	THROWONFAILURE( FALSE);
    }

    
    this->help=new text;
    if(this->help==NULL) {
	delete this->prefsp;
	delete this->categories;
	THROWONFAILURE( FALSE);
    } else {
	(this->help)->ReadTemplate( "prefhelp", FALSE);
	(this->help)->ReadTemplate( "default", FALSE);
    }

    this->readingdefaults=TRUE;

    files=environ::GetProfile("PrefEdPath");

    if(files==NULL) {
	files=environ::Get("PREFEDPATH");
	if(files==NULL) files=environ::AndrewDir("/lib/prefs.def");
    }

    (this)->ReadTemplate( "default", FALSE);
    while (files && *files) {
	int len;
	file = strchr(files, ':');
	if(file==NULL) len=strlen(files);
	else len = file-files;
	strncpy(filenamebuf, files, len);
	filenamebuf[len]='\0';
	err=(this)->AlwaysInsertFile( NULL, filenamebuf, (this)->GetLength());
	if(err<=0) {
	    fprintf(stderr, "prefs WARNING: couldn't read preferences definitions file %s!\n", filenamebuf);
	    errs++;
	}
	if(file) files=file+1;
	else files=NULL;
    }
    files=environ::AndrewDir("/lib/global.prf");
    if(files && access(files, R_OK)==0) {
	err=this->AlwaysInsertFile(NULL, files, this->GetLength());
	if(err<=0) {
	    fprintf(stderr, "prefs WARNING: couldn't read preferences definitions file %s!\n", files);
	    errs++;
	}
	
    }
    if(errs) err=1;
    else err=(this)->ReadDataPart();
    if(err!=dataobject_NOREADERROR) {
	FreeHelpStyles(this);
	delete this->prefsp;
	delete this->categories;
	fprintf(stderr, "prefs WARNING: couldn't read preferences definitions file(s)!\n");
	THROWONFAILURE( FALSE);
    }
    ApplyHelpStyles(this);
    (this)->SetCopyAsText( TRUE);
    this->lastheader=NULL;
    this->lastgroup=NULL;
    
    THROWONFAILURE( TRUE);
}


static boolean freestring(char  *name, long  rock)
{
    zfree(name);
    return TRUE;
}

static void freestringlist(class list  *vl)
{
    (vl)->Enumerate((list_efptr)freestring, NULL);
    delete vl;
}

static boolean freeall(struct prefdesc  *pd, class prefs  *rock)
{
    zfree(pd->cond);
    zfree(pd->val);
    if(pd->mark) {
	delete pd->mark;
    }
    if(pd->pm) {
	(rock)->RemoveMark( pd->pm);
	delete pd->pm;
    }
    if(pd->defviews) {
	delete pd->defviews;
    }
    if(pd->prevlines) {
	freestringlist(pd->prevlines);
    }
    if(pd->freeobject && pd->obj) (pd->obj)->Destroy();
    free(pd);
    return TRUE;
}


static boolean freegroups(struct prefgroup  *pg, class prefs  *self)
{
    zfree(pg);
    return TRUE;
}

prefs::~prefs()
{
    if(this->categories) {
	(this->categories)->Enumerate((list_efptr)freegroups, (char *)this);
	delete this->categories;
    }
    if(this->prefsp) {
	(this->prefsp)->Enumerate((list_efptr)freeall, (char *)this);
	delete this->prefsp;
    }
    if(this->lastline.vl) {
	delete this->lastline.vl;
    }
    if(this->lastline.prevlines) {
	freestringlist(this->lastline.prevlines);
    }
    (this->help)->Destroy();
}

static boolean endflag=TRUE;

static char *trans[]={
    "file", "pvaltvf",
    "string", "pvaltv",
    "integer", "pvaltv",
    "boolean", "pvalbv",
    "slider", "pvalslv",
    "color", "pvalcv",
    "font", "pvalfv",
    "filename", "pvaltvf",
    "filechoices", "pvaltvfc",
    "directory", "pvaltvf",
    "filelist", "pvaltvf",
    "directorylist", "pvaltvf",
    "stringlist", "pvaltvl",
    "slider", "pvalslv",
    NULL, NULL
};

char *prefs::TranslateViewName(char  *name)
{
    char **p=trans;
    while(p[0] && !FOLDEDEQ(p[0],name)) {
	p+=2;
    }
    if(p[0]) return p[1];
    else {
	struct ATKregistryEntry  *ci=ATK::LoadClass(name);
	static struct ATKregistryEntry  *vci=NULL;
	if(vci==NULL) vci=ATK::LoadClass("view");
	if(ci && (ci)->IsType( vci)) return name;
	else return "pvaltv";
    }
}

struct thedescrock {
    struct prefdesc *pd;
    char *name;
    char *app;
    char *cond;
};

struct therock {
    class prefs *self;
    long pos;
    class prefval *obj;
};

static boolean AddViews(char  *vname, struct therock  *tr)
{
    class prefs *self=tr->self;
    class prefval *obj=tr->obj;
    long pos=tr->pos;
    (self)->AlwaysAddView( pos, vname, obj);
    (self)->AlwaysInsertCharacters( pos+1, "\n", 1);
    tr->pos+=2;
    return TRUE;
}

static struct prefdesc *NewPrefDesc(class prefs  *self, char  *p)
{
    struct prefdesc *result=(struct prefdesc *)malloc(sizeof(struct prefdesc));
    char *q;
    enum prefval_Type type;
    char separators[3];
    separators[0]=separators[1]=separators[2]='\0';
    result->app=NULL;
    result->name=NULL;
    result->group=NULL;
    result->seps=NULL;
    result->cond=NULL;
    result->val=NULL;
    result->defviews=NULL;
    result->low=(0);
    result->high=(-1);
    result->listsize=1;
    result->writetype=FALSE;
    result->writeviews=FALSE;
    result->views=NULL;
    result->mark=NULL;
    result->indefs=FALSE;
    result->shadow=FALSE;
    result->order=(-1);
    result->helppos=(-1);
    result->freehelp=FALSE;
    result->expert=FALSE;
    result->prevlines=NULL;
    result->pm=NULL;
    q=strchr(p, ' ');
    if(q) *q='\0';
    result->type=prefval::StringToType(p);
    if(q==NULL) return result;
    q++;
    p=strchr(q, ' ');
    if(p) *p='\0';
    if(FOLDEDEQ(q, "List")) {
	result->listsize=65536;
	if(p) {
	    p++;
	    strncpy(separators, p, 2);
	} else strcpy(separators, ":");

	result->seps=SAVESTR(separators);
    }
    return result;
}

static boolean CopyViewName(char  *name, class list  *nl)
{
    (nl)->InsertEnd( name);
    return TRUE;
}

static class list *CopyViewList(class list  *vl)
{
    class list *nl;

    if(vl==NULL) return NULL;

    nl=new list;
    if(nl==NULL) return NULL;

    (vl)->Enumerate((list_efptr)CopyViewName, (char *)nl);
    return nl;
}


static boolean CopyStr(char  *name, class list  *nl)
{
    (nl)->InsertEnd( STRDUP(name));
    return TRUE;
}

static class list *CopyList(class list  *vl)
{
    class list *nl;

    if(vl==NULL) return NULL;

    nl=new list;
    if(nl==NULL) return NULL;

    (vl)->Enumerate((list_efptr)CopyStr, (char *)nl);
    return nl;
}

static struct prefdesc *DuplicateDesc(struct prefdesc  *pd)
{
    struct prefdesc *result=(struct prefdesc *)malloc(sizeof(struct prefdesc));
    if(result==NULL) return NULL;
    result->app=pd->app;
    result->name=pd->name;
    result->group=pd->group;
    result->seps=pd->seps;
    result->cond=STRDUP(pd->cond);
    result->val=STRDUP(pd->val);
    result->views=pd->views;
    result->prevlines=CopyList(pd->prevlines);
    result->defviews=CopyViewList(pd->defviews);
    result->low=pd->low;
    result->high=pd->high;
    result->type=pd->type;
    result->listsize=pd->listsize;
    result->writetype=FALSE;
    result->writeviews=FALSE;
    result->mark=NULL;
    result->indefs=FALSE;
    result->shadow=FALSE;
    result->order=(-1);
    result->obj=NULL;
    result->freeobject=FALSE;
    result->expert=pd->expert;
    result->helppos=pd->helppos;
    result->freehelp=FALSE;
    result->pm=NULL;
    return result;
}

static boolean FixOrderValues(struct prefdesc  *pd, long  val)
{
    if(pd->order>val) pd->order++;
    return TRUE;
}

struct prefdesc *prefs::DuplicatePref(struct prefdesc  *pd, char  *newapp , char  *newcond)
{
    struct prefdesc *result=DuplicateDesc(pd);
    char *v;
    static char *star;

    if(star==NULL) star=SAVESTR("*");
    
    if(result==NULL) return NULL;
    zfree(result->cond);
    result->cond=STRDUP(newcond);
    result->app=SAVESTR(newapp);
    result->obj=new prefval;
    if(result->obj==NULL) {
	freeall(result, this);
	return NULL;
    }
    result->freeobject=TRUE;
    (result->obj)->SetAppName( newapp);
    (result->obj)->SetPrefName( pd->name); 
    (result->obj)->SetType( pd->type);
    (result->obj)->SetListMax( pd->listsize);
    (result->obj)->SetSeparator( pd->seps);
    (result->obj)->SetCondition( result->cond);
    (result->obj)->SetRangeLow( result->low);
    (result->obj)->SetRangeHigh( result->high);
    v=(pd->obj)->PreferenceString();
    if(v) {
	v=strchr(v, ':');
	if(v) {
	    v++;
	    while(isspace(*v)) v++;
	}
    }
    if(v) (result->obj)->SetFromPreferenceString( v);
    (this->prefsp)->InsertEnd((char *)result);
    (result->obj)->AddObserver( this);
    if(pd->order>=0) {
	long ipos=(-1);
	(this->prefsp)->Enumerate((list_efptr)FixOrderValues, (char *)pd->order);
	this->maxorder++;
	if(pd->app==star || pd->app==NULL) {
	    result->order=pd->order;
	    pd->order--;
	    if(pd->pm) {
		ipos=(pd->pm)->GetPos();
	    }
	} else {
	    result->order=pd->order+1;
	    ipos=(pd->pm)->GetEndPos()+1;
	}
	if(ipos>=0) {
	    (this)->InsertCharacters( ipos, " \n", 2);
	    result->pm=(this)->CreateMark( ipos, 1);
	    (result->pm)->SetStyle( FALSE, FALSE);
	}
    } else {
	long pos=(this)->GetLength();
	result->order= ++this->maxorder;
	(this)->InsertCharacters( pos, " \n", 2);
	result->pm=(this)->CreateMark( pos, 1);
	(result->pm)->SetStyle( FALSE, FALSE);
    }
    if(result->pm) 
    (this)->UpdateOneInText( result);
    return result;
}

void prefs::DeletePref(struct prefdesc  *pd)
{
    if(pd->pm) {
	long pos=(pd->pm)->GetPos()-2, lpos=(-1);
	while(pos>=0) {
	    if((this)->GetChar( pos)=='\n') {
		if((this)->GetChar( pos+1)=='#') lpos=pos+1;
		else break;
	    }
	    pos--;
	}
	(this)->DeleteCharacters( (pd->pm)->GetPos(), (pd->pm)->GetLength()+1);
	if(lpos>=0) (this)->DeleteCharacters( lpos, (pd->pm)->GetPos()-lpos);
	(this)->RemoveMark( pd->pm);
	delete pd->pm;
	pd->pm=NULL;
    }
    if(!pd->indefs) {
	(this->prefsp)->Delete((char *)pd);
	freeall(pd, this);
    } else {
	if(pd->val) {
	    (pd->obj)->SetFromPreferenceString( pd->val);
	} else {
	    (pd->obj)->SetFromPreferenceString( "");
	}
	(pd->obj)->SetDefault();
	(pd->obj)->NotifyObservers( observable_OBJECTCHANGED);
    }
}

static class list *HandleViewList(class prefs  *self, char  *buf)
{
    char *p=buf;
    class list *vl;
    vl=new list;
    if(vl==NULL) return NULL;
    while(p) {
	char *q=strchr(p, ' ');
	char *vname;
	long pos=(self)->GetLength();
	if(q) *q='\0';
	if(*p!='\0') {
	    vname=SAVESTR(prefs::TranslateViewName(p));
	    if(vname==NULL) continue;
	    (vl)->InsertEnd( vname);
	}
	if(q) p=q+1;
	else break;
    }
    return vl;
}


static boolean FindDesc(struct prefdesc  *pd, struct thedescrock  *rock)
{
    char *app=rock->app;
    char *name=rock->name;
    char *cond=rock->cond;

    if(pd->shadow) return TRUE;
    
    if(!(name==pd->name)) return TRUE;

    if((pd->app==NULL || (pd->app[0]=='*' && pd->app[1]=='\0')) && pd->cond==NULL && rock->pd==NULL) {
	rock->pd=pd;
	return TRUE;
    }

    if(pd->app && !(app==pd->app)) return TRUE;
    
    if(pd->cond==NULL) {
	rock->pd=pd;
	return TRUE;
    }
    if((cond==pd->cond || (cond && pd->cond && strcmp(cond, pd->cond)==0))) rock->pd=pd;
    
    return TRUE;
}


static boolean ListsEqual(class list  *l1 , class list  *l2)
{
    if(l1==l2) return TRUE;
    
    if(l1==NULL || l2==NULL) return TRUE;
    
    if((l1)->Size()!=(l2)->Size()) return FALSE;
    (l1)->Start();
    (l2)->Start();
    
    do {
	if((l1)->Data()!=(l2)->Data()) return FALSE;
    } while((l1)->Advance() && (l2)->Advance());
    return TRUE;
}


static boolean InsertPref(struct prefdesc  *pd, class prefs  *self)
{
    if(pd->shadow && self->sortby!=prefs_Group) return TRUE;
 
    if(self->lastgroup==NULL || (self->lastgroup!=pd->group)) {
	struct prefgroup *pg;
	self->lastgroup=pd->group;
	if((self->categories)->Enumerate((list_efptr)FindCat, (char *)pd->group)) return TRUE;
	pg=(struct prefgroup *)malloc(sizeof(struct prefgroup));
	if(pg==NULL) return FALSE;
	pg->name=pd->group;
	pg->grouphelp=0;
	(self->categories)->InsertEnd((char *)pg);
    }
    return TRUE;
/* see code at end of this file... it doesn't get used right now, but might be useful someday... */
}

static void HandlePref(class prefs  *self, struct prefline  *line, int  *order)
{
    class prefval *obj=NULL;
    int len;
    char *p;
    long lpos;
    struct prefdesc *desc;
    struct thedescrock otherrock;
    boolean newobj=FALSE;
    char *app=SAVESTR(line->app), *name=SAVESTR(line->name);
    char *cond=line->cond;
    char *val=line->val;
    char *group=SAVESTR(line->group);

    otherrock.app=app;
    otherrock.name=name;
    otherrock.cond=cond;
    otherrock.pd=NULL;
    /* if a type was specified don't bother with the lookup */
   if(!line->type || self->readingdefaults) (self->prefsp)->Enumerate((list_efptr)FindDesc, (char *)&otherrock);
    if(otherrock.pd==NULL) {
	if(line->shadow) fprintf(stderr, "prefs WARNING: no match found for shadow preference %s.%s\n", app, name);
	desc=NewPrefDesc(self, line->type?line->type:"String");
	desc->helppos=line->helppos;
	if(line->expert>=0) desc->expert=line->expert;
	if(line->vl && (line->vl)->Size()>0) desc->defviews=CopyViewList(line->vl);
	else {
	    desc->defviews=new list;
	    (desc->defviews)->InsertEnd( SAVESTR("pvaltv"));
	}
	if(line->views) desc->views=SAVESTR(line->views);
	else desc->views=SAVESTR("pvaltv");
	if(!self->readingdefaults) {
	    if(line->type) desc->writetype=TRUE;
	    if(line->vl) desc->writeviews=TRUE;
	}
	newobj=TRUE;
    } else {
	desc=otherrock.pd;
	if(!line->shadow && (app==desc->app) && (cond==desc->cond || (cond && desc->cond && strcmp(cond, desc->cond)==0)) && ListsEqual(line->vl, desc->defviews)) {
	    obj=desc->obj;
	    if(!self->readingdefaults) {
		if(desc->prevlines) freestringlist(desc->prevlines);
		if(line->prevlines) {
		    desc->prevlines=CopyList(line->prevlines);
		} else desc->prevlines=NULL;
	    }
	} else {
	    group=desc->group;
	    desc=DuplicateDesc(desc);
	    if(line->expert>=0) desc->expert=line->expert;
	    if(line->helppos>=0) {
		desc->helppos=line->helppos;
	    }
	    if(line->shadow) {
		desc->shadow=TRUE;
		desc->group=SAVESTR(line->group);
		desc->obj=otherrock.pd->obj;
		desc->freeobject=FALSE;
		(self->prefsp)->InsertEnd((char *)desc);

		if(!self->readingdefaults) {
		    if(desc->prevlines) freestringlist(desc->prevlines);
		    if(line->prevlines) {
			desc->prevlines=CopyList(line->prevlines);
		    } else desc->prevlines=NULL;
		}
	    } else newobj=TRUE;
	    if(line->vl) {
		desc->views=SAVESTR(line->views);
		
		if(desc->defviews) {
		    delete desc->defviews;
		}
		desc->defviews=CopyViewList(line->vl);
		if(!self->readingdefaults) desc->writeviews=TRUE;
	    }
	}
    }
    
    if(newobj) {
	long spos;
	obj=new prefval;
	if(obj==NULL) return;
	if(desc->app!=app) {
	    desc->app=app;
	}
	if(desc->name!=name) {
	    desc->name=name;
	}
	if(desc->group!=group) {
	    desc->group=group;
	}
	zfree(desc->cond);
	desc->cond=STRDUP(cond);
	if(!self->readingdefaults) {
	    if(desc->prevlines) freestringlist(desc->prevlines);
	    if(line->prevlines) {
		desc->prevlines=CopyList(line->prevlines);
	    } else desc->prevlines=NULL;
	}
	desc->obj=obj;
	desc->freeobject=TRUE;
	if(line->low<=line->high) {
	    desc->low=line->low;
	    desc->high=line->high;
	}
	(self->prefsp)->InsertEnd((char *)desc);
	(obj)->SetAppName( app);
	(obj)->SetPrefName( name); 

	(obj)->SetType( desc->type);
	(obj)->SetListMax( desc->listsize);
	(obj)->SetSeparator( desc->seps);
	(obj)->SetCondition( cond);
	(obj)->SetRangeLow( desc->low);
	(obj)->SetRangeHigh( desc->high);
	(obj)->AddObserver( self);
    }
    if(self->readingdefaults) {
	zfree(desc->val);
	desc->val=STRDUP(val);
    }
    p=val;
    if(!desc->indefs) desc->indefs=self->readingdefaults;
    if(p!=NULL && !desc->shadow) {
	if(!self->readingdefaults && desc->order>=0) {
	    fprintf(stderr, "Preferences Warning: multiple settings for %s.%s, first setting taken.\nFirst setting was: %s\nLater settings for %s.%s will be removed when you save your preferences.\n", desc->app, desc->name, (desc->obj)->PreferenceString(), desc->app, desc->name);
	    return;
	}
	while(isspace(*p)) p++;
	(desc->obj)->SetFromPreferenceString( p);
	if(self->readingdefaults) (desc->obj)->SetDefault();
	else {
	    if(self->maxorder<*order) self->maxorder=(*order);
	    desc->order=(*order)++;
	}
    }
}


static char *myindex(char  *p, char  ch)
{
    boolean haveslash=FALSE;
    while(*p) {
	if(!haveslash && *p==ch) return p;
	if(*p=='\\')  haveslash=!haveslash;
	else haveslash=FALSE;
	p++;
    }
    return NULL;
}

static struct wbuf {
    char *buf;
    int size;
} typebuf={
    NULL, 0
};

static struct wbuf viewsbuf={
    NULL, 0
};

static struct wbuf groupbuf={
    NULL,
    0
};

static struct wbuf appbuf={
    NULL,
    0
};
static struct wbuf namebuf={
    NULL,
    0
};
static struct wbuf valbuf={
    NULL,
    0
};

static struct wbuf condbuf={
    NULL,
    0
};

static char * SetBuf(struct wbuf  *wb, char  *str)
{
    if(str==NULL) str="";
    if(wb->buf==NULL) {
	wb->size=strlen(str)+1;
	wb->buf=(char *)malloc(wb->size);
    } else {
	int len=strlen(str)+1;
	if(len>wb->size) {
	    wb->buf=(char *)realloc(wb->buf, len);
	    if(wb->buf==NULL) {
		wb->size=0;
		return NULL;
	    }
	    wb->size=len;
	}
    }
    strcpy(wb->buf, str);
    return wb->buf;
}

#ifndef MAX
#define MAX(x,y) ((x)<(y)?y:x)
#endif

static void AddHelpStyle(class prefs  *self, long  pos, long  len, enum style_type  type)
{
    if(self->llistcnt>=self->llistsize) {
	if(self->llist==NULL) {
	    self->llistsize=MAX(300, self->llistcnt+1);
	    self->llist=(struct hstyles *)malloc(self->llistsize*sizeof(struct hstyles));
	    if(self->llist==NULL) return;
	} else {
	    self->llistsize+=100;
	    self->llist=(struct hstyles *)realloc(self->llist, self->llistsize*sizeof(struct hstyles));
	    if(self->llist==NULL) return;
	}
    }
    self->llist[self->llistcnt].pos=pos;
    self->llist[self->llistcnt].len=len;
    self->llist[self->llistcnt].type=type;
    self->llistcnt++;
}

#define TypeBufSet(str) SetBuf(&typebuf, str)
#define ViewsBufSet(str) SetBuf(&viewsbuf, str)
#define GroupBufSet(str) SetBuf(&groupbuf, str)
#define AppBufSet(str) SetBuf(&appbuf, str)
#define NameBufSet(str) SetBuf(&namebuf, str)
#define ValBufSet(str) SetBuf(&valbuf, str)
#define CondBufSet(str) SetBuf(&condbuf, str)

static void AddCategory(class prefs  *self, char  *group, long  pos)
{
    struct prefgroup *pg=(struct prefgroup *)(self->categories)->Enumerate((list_efptr)FindCat, (char *)group);
    if(pg==NULL) {
	pg=(struct prefgroup *)malloc(sizeof(struct prefgroup));
	if(pg==NULL) return;
	pg->grouphelp=(-1);
	(self->categories)->InsertEnd((char *)pg);
	pg->name=SAVESTR(group);
    }
    if(pos>=0) pg->grouphelp=pos;
}

static boolean ReadPrefLine(class prefs  *self,struct prefline  *line)
{
    char *buf=NULL, *pp, *app, *name, *val;
    boolean errorflag=FALSE;
    boolean seentype=FALSE;
    boolean seenviews=FALSE;
    boolean seengroup=FALSE;
    boolean seenhelp=FALSE;
    boolean addedcategory=FALSE;
    while(1) {
	pp=buf=ReadLine(self);
	if(buf==NULL) {
	    return FALSE;
	}
	if(buf[0]=='#' && buf[1]!='~') {
	    if(!self->readingdefaults) {
		if(line->prevlines==NULL) {
		    line->prevlines=new list;
		    if(line->prevlines==NULL) return FALSE;
		}
		(line->prevlines)->InsertEnd( STRDUP(buf));
	    }
	    continue;
	} else if(buf[0]=='#') {
	    char *p=strchr(buf+2, ' ');
	    if(p!=NULL) p++;
	    if(strncmp(buf+2, "type ", 5)==0) {
		if(p==NULL) continue;
		if(seentype) {
		    fprintf(stderr, "prefs WARNING: already saw type!\n");
		    errorflag=TRUE;
		}
		seentype=TRUE;
		line->type=TypeBufSet(p);
		continue;
	    } else if(strncmp(buf+2, "views ", 6)==0 || strncmp(buf+2, "view ", 5)==0) {
		if(p==NULL) continue;
		if(seenviews) {
		    fprintf(stderr, "prefs WARNING: already saw views!\n");
		    errorflag=TRUE;
		}
		seenviews=TRUE;
		line->views=ViewsBufSet(p);
		if(line->vl) delete line->vl;
		line->vl=HandleViewList(self, p);
		continue;
	    } else if(strncmp(buf+2, "group ", 6)==0) {
		if(p==NULL) continue;
		if(seengroup) {
		    fprintf(stderr, "prefs WARNING: already saw group!\n");
		    errorflag=TRUE;
		}
		seengroup=TRUE;
		line->group=GroupBufSet(p);
		continue;
	    } else if(strcmp(buf+2, "shadow")==0) {
		line->shadow=TRUE;
		continue;
	    } else if(strncmp(buf+2, "range ", 6)==0) {
		if(p==NULL) continue;
		line->low=atol(p);
		p=strchr(p, ' ');
		if(p==NULL) continue;
		line->high=atol(p+1);
		continue;
	    } else if(strncmp(buf+2, "help", 4)==0) {
		long pos=(self)->GetFence();
		long epos=pos;
		long hpos=(self->help)->GetLength();
		if(seenhelp) {
		    fprintf(stderr, "prefs WARNING: already saw help!\n");
		    errorflag=TRUE;
		}
		seenhelp=TRUE;
		while(buf) {
		    epos=(self)->GetFence();
		    buf=ReadLine(self);
		    if(strcmp(buf, "#~endhelp")==0) break;
		}
		(self->help)->CopyText( hpos, self, pos, epos-pos);
		(self->help)->InsertCharacters( hpos+epos-pos, "\n", 1);
		line->helppos=hpos;
		continue;
	    } else if(strncmp(buf+2, "grouphelp", 9)==0) {
		long pos=(self)->GetFence();
		long epos=pos;
		long hpos=(self->help)->GetLength();
		while(buf) {
		    epos=(self)->GetFence();
		    buf=ReadLine(self);
		    if(strcmp(buf, "#~endhelp")==0) break;
		}
		(self->help)->CopyText( hpos, self, pos, epos-pos);
		(self->help)->InsertCharacters( hpos+epos-pos, "\n", 1);
		if(line->group) {
		    addedcategory=TRUE;
		    AddCategory(self, line->group, hpos);
		}
		continue;
	    } else if(strncmp(buf+2, "expert", 5)==0) {
		line->expert=1;
		continue;
	    } else if(strncmp(buf+2, "novice", 5)==0) {
		line->expert=0;
		continue;
	    } else if(strncmp(buf+2, "PrefEdVersion ", 14)==0) {
		long version=atol(p);
		if(version>prefs_DS_VERSION) {
		    fprintf(stderr, "prefs WARNING: file was last written by a later version of the preferences editor.\n");
		}
		self->version=version;
		continue;
	    }
	} else if(buf[0]=='?') {
	    char *p=myindex(buf, ':'); 
	    if(p) {
		class prefval *obj;
		*p='\0';
		line->cond=CondBufSet(buf+1);
		pp=p+1;
	    } else fprintf(stderr, "prefs: Warning malformed conditional preference:\n%s\n",buf);
	}
	while(isspace(*pp)) pp++;
	if(*pp==0) {
	    if(!self->readingdefaults) {
		if(line->prevlines==NULL) {
		    line->prevlines=new list;
		    if(line->prevlines==NULL) return FALSE;
		}
		(line->prevlines)->InsertEnd( STRDUP(buf));
	    }
	    continue;
	}
	
	val=strchr(pp, ':');
	if(val) {
	    *val='\0';
	    val++;
	    while(isspace(*val)) val++;
	}
	name=strchr(pp, '.');
	if(name) {
	    *name='\0';
	    name++;
	    app=pp;
	} else {
	    app="*";
	    name=pp;
	};
	if(line->group) {
	    self->lastgroup=SAVESTR(line->group);
	    if(!addedcategory) AddCategory(self, line->group, -1);
	} else line->group=self->lastgroup;
	line->app=AppBufSet(app);
	line->name=NameBufSet(name);
	if(errorflag) {
	    fprintf(stderr, "prefs WARNING: error in description for preference: %s.%s\n", line->app?line->app:"(NULL)", line->name?line->name:"(NULL)");
	}
	if(name && line->helppos>=0) {
	    long len=strlen(name), len2;
	    long pos=line->helppos;
	    AddHelpStyle(self, pos, len, PrefsSubsection);
	    (self->help)->AlwaysInsertCharacters( pos, name, len);
	    pos+=len;
	    if(self->lastgroup) {
		(self->help)->AlwaysInsertCharacters( pos, " (", 2);
		pos+=2;
		len=strlen(self->lastgroup);
		AddHelpStyle(self, pos, len, PrefsGroupname);
		(self->help)->AlwaysInsertCharacters( pos, self->lastgroup, len);
		pos+=len;
		(self->help)->AlwaysInsertCharacters( pos, ")", 1);
		pos++;
	    }	    
	    (self->help)->AlwaysInsertCharacters( pos, "\n", 1);
	}
	line->val=ValBufSet(val);
	return TRUE;
    }
}

#define SAFESTR(x) (x?x:"")

static int sortbyname(struct prefdesc  *pd1 , struct prefdesc  *pd2)
{
    int result=lc_strcmp((pd1)->name, (pd2)->name);
    if(result!=0) return result;
    else {
	result=lc_strcmp((pd1)->app, (pd2)->app);
	if(result!=0) return result;
	else return lc_strcmp(SAFESTR((pd1)->group), SAFESTR((pd2)->group));
    }
}

static int sortbyapp(struct prefdesc  *pd1 , struct prefdesc  *pd2)
{
    int result=lc_strcmp((pd1)->app, (pd2)->app);
    if(result!=0) return result;
    else {
	result=lc_strcmp(SAFESTR((pd1)->group), SAFESTR((pd2)->group));
	if(result!=0) return result;
	else return lc_strcmp((pd1)->name, (pd2)->name);
    }
}


static int sortbygroup(struct prefdesc  *pd1 , struct prefdesc  *pd2)
{
    int result=lc_strcmp(SAFESTR((pd1)->group), SAFESTR((pd2)->group));
    if(result!=0) return result;
    else {
	result=lc_strcmp((pd1)->app, (pd2)->app);
	if(result!=0) return result;
	else return lc_strcmp((pd1)->name, (pd2)->name);
    }
}

static boolean sane=FALSE;
#define ISWILD(p) (p==NULL || (p[0]=='*' && p[1]=='\0'))

static int sortbyorder(struct prefdesc  *pd1 , struct prefdesc  *pd2)
{
    if(::sane) {
	if((pd1->name==pd2->name) && ISWILD(pd1->app) && !ISWILD(pd2->app)) return 1;
	if((pd1->name==pd2->name) && ISWILD(pd2->app) && !ISWILD(pd1->app)) return -1;
    }
    if((pd1)->order==-1 && (pd2)->order!=-1) return 1;
    if((pd2)->order==-1 && (pd1)->order!=-1) return -1;
    return (pd1)->order-(pd2)->order;
}

typedef int (*sortfptr)(struct prefdesc *p1, struct prefdesc *p2);

static sortfptr sortprocs[]={
    sortbyname,
    sortbyapp,
    sortbygroup,
    sortbyorder
};

static int catsort(struct prefgroup  *pg1 , struct prefgroup  *pg2)
{
    return ULstrcmp(pg1->name, pg2->name);
}

void prefs::Sort(enum prefs_SortType  sortby, boolean  perm)
{
    if((int)sortby>=(int)prefs_MaxSortType) return;

    if(perm) this->sortby=sortby;

    ::sane=this->sane;
    
    (this->prefsp)->Sort((list_greaterfptr)sortprocs[(int)sortby]);
    (this->categories)->Sort((list_greaterfptr)catsort);
}

static boolean ResetOrder(struct prefdesc  *pd, class prefs  *self)
{
    pd->order=(-1);
    return TRUE;
}

long prefs::ReadDataPart()
{
    /*
      Read in the object from the file.
      */
    long err=dataobject_NOREADERROR;
    struct prefline line;
    boolean cont=TRUE;
    int order=0;
    if(!this->readingdefaults) {
	(this->prefsp)->Enumerate((list_efptr)ResetOrder, (char *)this);
	this->maxorder=(-1);
    }
    this->lastgroup=SAVESTR("---Unkown---");
    do {
	memset(&line, 0, sizeof(line));
	line.low=line.high+1;
	line.expert=(-1);
	line.helppos=(-1);
	cont=ReadPrefLine(this, &line);
	if(!cont) {
	    this->lastline=line;
	    continue;
	}	    
	HandlePref(this, &line, &order);
	if(line.vl) {
	    delete line.vl;
	    line.vl=NULL;
	}
	if(line.prevlines) {
	    freestringlist(line.prevlines);
	    line.prevlines=NULL;
	}
    } while (cont);
    if(!this->readingdefaults) {
	(this)->Sort( prefs_Order, TRUE);
    }
    if(this->readingdefaults || this->version!=-1 || !environ::GetProfileSwitch("KeepOrigPrefs", TRUE)) {
	(this)->UpdateText();
    }
    return err;
}

static boolean ResetValues(struct prefdesc  *pd, class prefs  *self)
{

    if(pd->val) {
	(pd->obj)->SetFromPreferenceString( pd->val);
    } else {
	(pd->obj)->SetFromPreferenceString( "");
    }
    (pd->obj)->SetDefault();
    return TRUE;
}

long prefs::ReScan()
{
    long err;
    (this->prefsp)->Enumerate((list_efptr)ResetValues, (char *)this);
    (this)->SetReadingDefaults( FALSE);
    err=(this)->ReadDataPart();
    ApplyHelpStyles(this);
    return err;
}

long prefs::Read(FILE  *fp, long  id)
{
    long err=dataobject_NOREADERROR;
    endflag=TRUE;
    err=(this)->text::Read( fp, id);
    if(err<0) return err;
    (this)->ReScan();
    return err;
}

static boolean PrefvalP(struct prefdesc  *pd, class prefval  *obj)
{
    if(pd->obj==obj) return FALSE;
    else return TRUE;
}

void prefs::ObservedChanged(class observable  *changed, long  val)
{
    static struct ATKregistryEntry  *prefval_ci=NULL;
    struct prefdesc *pd=NULL;
    if(prefval_ci==NULL) prefval_ci=ATK::LoadClass("prefval");
    if(val!=observable_OBJECTDESTROYED && (changed)->ATKregistry()==prefval_ci && (pd=(struct prefdesc *)(this->prefsp)->Enumerate((list_efptr)PrefvalP, (char *)changed))) {
	(this)->UpdateOneInText( pd);
	this->selfmod=TRUE;
	(this)->NotifyObservers( observable_OBJECTCHANGED);
	this->selfmod=FALSE;
    }
    (this)->text::ObservedChanged( changed, val);
}

typedef void (*writefptr)(struct writerock *rock, struct prefdesc *pd);
struct writerock {
    class prefs *self;
    FILE *fp;
    writefptr writefunc;
    char *buf;
    long pos, len;
};

static boolean PrintPrevLines(char  *line, struct writerock  *rock)
{
    sprintf(rock->buf, "%s\n", line);
    rock->writefunc(rock, NULL);
    return TRUE;
}

static boolean writeprefs(struct prefdesc  *pd, struct writerock  *rock)
{
    class prefs *self=rock->self;
    FILE *fp=rock->fp;
    char buff[10240];
    rock->buf=buff;
    if(pd->shadow) return TRUE;
    if(pd->obj && !(pd->obj)->GetIsDefault()) {
	if(pd->prevlines) {
	    (pd->prevlines)->Enumerate((list_efptr)PrintPrevLines, (char *)rock);
	}
	if(pd->writetype) {
	    sprintf(buff, "##type %s%s%s%s%s\n", (pd->obj)->TypeString(), ((pd->obj)->GetListMax() > 1) ? " " : "",
		((pd->obj)->GetListMax() > 1) ? "List" : "",
		    ((pd->obj)->GetSeparator()) ? " " : "",
		    (pd->obj)->GetSeparator()?(pd->obj)->GetSeparator():"");
	    rock->writefunc(rock, NULL);
	}
	
	if(pd->writeviews && pd->views) {
	    sprintf(buff, "##view %s\n", pd->views);
	    rock->writefunc(rock, NULL);
	}
	
	if(pd->cond) {
	    sprintf(buff, "?%s:", pd->cond);
	} else buff[0]=0;
	
	sprintf(buff + strlen(buff), "%s\n", (pd->obj)->PreferenceString());
	
	rock->writefunc(rock, pd);
    } else {
	if(pd->pm) {
	    (rock->self)->RemoveMark( pd->pm);
	    delete pd->pm;
	    pd->pm=NULL;
	}
    }
    return TRUE;
}

long prefs::WritePlain(FILE  *file, long  writeID, int  level)
{
    return (this)->text::Write( file, writeID, level);
}


static void twrite(struct writerock  *rock, struct prefdesc  *pd)
{
    int len=strlen(rock->buf);
    (rock->self)->AlwaysReplaceCharacters( rock->pos, rock->len, rock->buf, len);
    if(pd) {
	if(pd->pm) {
	    (rock->self)->RemoveMark( pd->pm);
	    delete pd->pm;
	}
	pd->pm=(rock->self)->CreateMark( rock->pos, len-1);
	(pd->pm)->SetStyle( FALSE, FALSE);
    }
    rock->pos+=len;
}

void prefs::UpdateOneInText(struct prefdesc  *pd)
{
    char *nval=(pd->obj)->FullPreferenceString();
    int len;
    nval=nval?nval:"";
    len=strlen(nval);
    if(pd->pm==NULL) {
	long pos=(this)->GetLength();
	if((pd->obj)->GetIsDefault()) return;
	(this)->InsertCharacters( pos, nval, len);
	(this)->InsertCharacters( (this)->GetLength(), "\n", 1);
	pd->pm=(this)->CreateMark( pos, len);
	(pd->pm)->SetStyle( FALSE, FALSE);
	pd->order= ++this->maxorder;
	return;
    }
    if((pd->pm)->GetLength()==0) {
	fprintf(stderr, "Preference %s.%s has been deleted from text.\n", pd->app?pd->app:"(NULL)", pd->name?pd->name:"(NULL)");
	return;
    }
    (this)->AlwaysReplaceCharacters( (pd->pm)->GetPos(), (pd->pm)->GetLength(), nval, len);
    (pd->pm)->SetLength( len);    
}

void prefs::UpdateText()
{
    struct writerock thisrock;
    char buf[1024];
    (this)->AlwaysDeleteCharacters( 0, (this)->GetLength());
    (this)->SetWriteID(im::GetWriteID());
    thisrock.self=this;
    thisrock.fp=NULL;
    thisrock.writefunc=(writefptr)twrite;
    thisrock.len=0;
    (this)->Sort( prefs_Order, FALSE);
    sprintf(buf, "#~PrefEdVersion %d\n", prefs_DS_VERSION);
    (this)->AlwaysInsertCharacters( 0, buf, strlen(buf));
    thisrock.pos=(this)->GetLength();
    (this->prefsp)->Enumerate((list_efptr)writeprefs, (char *)&thisrock);
    (this)->Sort( this->sortby, FALSE);
    if(this->lastline.prevlines) (this->lastline.prevlines)->Enumerate((list_efptr)PrintPrevLines, (char *)&thisrock);
}

static void filewrite(struct writerock  *rock, struct prefdesc  *pd)
{
    fprintf(rock->fp, "%s", rock->buf);
}

long prefs::Write(FILE  *file, long  writeID, int  level)
{
    struct writerock thisrock;
    if ((this)->GetWriteID() != writeID)  {
	if(level) (this)->text::Write( file, writeID, level);
	else {
	    (this)->SetWriteID(writeID);
	    thisrock.self=this;
	    thisrock.fp=file;
	    thisrock.writefunc=(writefptr)filewrite;
	    (this)->Sort( prefs_Order, FALSE);
	    fprintf(file, "#~PrefEdVersion %d\n", prefs_DS_VERSION);
	    (this->prefsp)->Enumerate((list_efptr)writeprefs, (char *)&thisrock);
	    (this)->Sort( this->sortby, FALSE);
	    if(this->lastline.prevlines) (this->lastline.prevlines)->Enumerate((list_efptr)PrintPrevLines, (char *)&thisrock);
	}
    }
    return (this)->GetID();
}


#define BUFSTEP 400
static char *ReadLine(class text *self)
{
    static int bufsize=0;
    int bp=0;
    static char *buf=NULL;
    
    long pos=(self)->GetFence(); /* use the fence to track our position. */
    long epos=0;

   
    if(self==NULL) {
	zfree(buf);
	return NULL;
    }

    if(buf==NULL) {
	bufsize=BUFSTEP;
	buf=(char *)malloc(bufsize);
    }

    if(buf==NULL) return NULL;

    if(pos>=(self)->GetLength()) return NULL;
    
    epos=(self)->Index( pos,  '\n', (self)->GetLength()-pos);

    if(epos<0 || epos>(self)->GetLength()) epos=(self)->GetLength()-1;

    if(epos-pos>bufsize) {
	buf=(char *)realloc(buf, epos-pos+1);
	if(buf==NULL) return NULL;
    }
    
    (self)->CopySubString( pos, epos-pos, buf, FALSE);

    (self)->SetFence( epos+1);

    return buf;
}


char *prefs::ViewName()
{
    return "pintv";
}

