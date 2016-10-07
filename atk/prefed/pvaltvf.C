/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

#include <andrewos.h>
ATK_IMPL("pvaltvf.H")
#include <math.h>


#include "pvaltvf.H"

#include <prefval.H>

#include <proctable.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <text.H>
#include <textview.H>
#include <observable.H>
#include <message.H>
#include <completion.H>
#include <filetype.H>

#define DATA(self) ((class prefval *)(self)->GetDataObject())
#define TEXT(self) ((self)->GetText())

static class menulist *pvaltvfMenus=NULL;
static class keymap *pvaltvfKeymap=NULL;


static struct helprock {
    class pvaltvf *self;
    char *path;
    long ind;
} thelprock;


ATKdefineRegistry(pvaltvf, pvaltvl, pvaltvf::InitializeClass);
static void helpfunc(struct helprock  *rock, enum message_HelpItem  type, char  *item, long  null);
static void pvaltvfHelp(class pvaltvf  *self, long  rock);
static void pvaltvfComplete(class pvaltvf  *self, long  rock);


static void helpfunc(struct helprock  *rock, enum message_HelpItem  type, char  *item, long  null)
{
    char buf[1024];
    class pvaltvf *self=rock->self;
    if(type!=message_HelpListItem) return;

    if(rock->ind==0) (DATA(self))->ClearChoices();

    if(item[0]!='/') {
	char *p;
	strcpy(buf, rock->path);
	p=strrchr(buf, '/');
	if(p==NULL) p=buf;
	else *p='\0';
	strcat(p, "/");
	strcat(p, item);
    } else strcpy(buf, item);
    
    (DATA(self))->SetChoice( rock->ind, item, (DATA(self))->StringToValue( buf));
    rock->ind++;
}

static void pvaltvfHelp(class pvaltvf  *self, long  rock)
{
    char buf[1024], buf2[1024];
    class textview *tv=(self)->GetTextView();
    long pos=(tv)->GetDotPosition();
    long end;
    
    if(pos>0) {
	while(--pos>=0 && (TEXT(self))->GetChar( pos)!='\n');
	pos++;
    }
    
    end=(TEXT(self))->Index( pos, '\n', (TEXT(self))->GetLength()-pos);
    if(end<0) end=(TEXT(self))->GetLength();
    
    if(end-pos>=(int)sizeof(buf)-1) return;
    
    (TEXT(self))->CopySubString( pos, end-pos, buf, FALSE);

    if(buf[0]=='\0') strcpy(buf, "./");
    filetype::CanonicalizeFilename(buf2, buf, sizeof(buf2)-1);
    thelprock.self=self;
    thelprock.ind=0;
    thelprock.path=buf2;
    completion::FileHelp(buf2, 0, (message_workfptr)helpfunc, (long)&thelprock);
    (DATA(self))->SortChoices();
    (DATA(self))->NotifyObservers( prefval_ChoicesChanged);
}


#if 0 /* use commented out below */
static void pvaltvfNop(class pvaltvf  *self, long  rock)
{
}
#endif

static void pvaltvfComplete(class pvaltvf  *self, long  rock)
{
    char buf[1024], buf2[1024];
    boolean good=TRUE;
    class textview *tv=(self)->GetTextView();
    long pos=(tv)->GetDotPosition();
    long end;
    if(pos>0) {
	while(--pos>=0 && (TEXT(self))->GetChar( pos)!='\n');
	pos++;
    }
    end=(TEXT(self))->Index( pos, '\n', (TEXT(self))->GetLength()-pos);
    if(end<0) end=(TEXT(self))->GetLength();
    
    if(end-pos>=(int)sizeof(buf)-1) return;
    
    (TEXT(self))->CopySubString( pos, end-pos, buf, FALSE);

    switch(completion::FileComplete(buf, FALSE, buf2, sizeof(buf2))) {
	case message_Complete:
	    message::DisplayString(self, 0, "Unique.");
	    break;
	case message_CompleteValid:
	    message::DisplayString(self, 0, "Others.");
	    break;
	case message_Valid:
	    break;
	case message_Invalid:
	    good=FALSE;
	    message::DisplayString(self, 0, "No match.");
	    break;
    }

    if(good) {
	int len=strlen(buf2);
	(TEXT(self))->ReplaceCharacters( pos, end-pos, buf2, len);
	(TEXT(self))->NotifyObservers( observable::OBJECTCHANGED);
	(tv)->SetDotPosition( pos+len);
	(tv)->SetDotLength( 0);
	(tv)->FrameDot( pos+len);
    }
    
}

static struct bind_Description pvaltvfBindings[]={
    {
	"pvaltvf-complete",
	" ",
	' ',
	0,
	0,
	0,
	(proctable_fptr)pvaltvfComplete,
	"Attempts to complete the filename before the cursor.",
	"pvaltvf"
    },/* {
	"pvaltvf-nop",
	"\r",
	'\r',
	0,
	0,
	0,
	(proctable_fptr)pvaltvfNop,
	"Does nothing.",
	"pvaltvf"
    }, */
    {
	"pvaltvf-help",
	"?",
	'?',
	0,
	0,
	0,
	(proctable_fptr)pvaltvfHelp,
	"Provides a list of possible completions in the choices list.",
	"pvaltvf"
    },
    {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
    }
};
    
    
boolean pvaltvf::InitializeClass()
{
    pvaltvfMenus=new menulist;
    pvaltvfKeymap=new keymap;

    if(pvaltvfMenus == NULL || pvaltvfKeymap==NULL) {
	if(pvaltvfMenus != NULL) delete pvaltvfMenus;
	if(pvaltvfKeymap != NULL) delete pvaltvfKeymap;
	return FALSE;
    }
    
    bind::BindList(pvaltvfBindings, pvaltvfKeymap, pvaltvfMenus, &pvaltvf_ATKregistry_ );
    return TRUE;
}

pvaltvf::pvaltvf()
{
	ATKinit;

    this->ks=keystate::Create(this, pvaltvfKeymap);
    if(this->ks==NULL) THROWONFAILURE( FALSE);

    this->menulistp=(pvaltvfMenus)->DuplicateML( this);

    if(this->menulistp==NULL) {
	delete this->ks;
	THROWONFAILURE( FALSE);
    }
    
    THROWONFAILURE( TRUE);
}

pvaltvf::~pvaltvf()
{
	ATKinit;

    if(this->ks) delete this->ks;
    if(this->menulistp) delete this->menulistp;
}


class keystate *pvaltvf::Keys()
{
    return this->ks;
}
