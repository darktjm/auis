/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           * 
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/supportviews/RCS/sbutton.C,v 3.6 1995/04/17 15:40:04 rr2b Stab74 $";
#endif


 
#include <andrewos.h>
ATK_IMPL("sbutton.H")
#include <stdio.h>
#include <sys/param.h>
#include <atom.H>
#include <environ.H>
#include <fontdesc.H>
#include <observable.H>
#include <owatch.H>
#include <util.h>
#include <rect.h>

#include "sbutton.H"

/* Defined constants and macros */

#define MAX_LINE_LENGTH 70  /* can't be less than 6 */

/* External declarations */

/* Forward Declarations */

  

/* Global variables */
static class atom *buttonpushed=NULL;
static boolean linehascontrol=FALSE;
static char *True="True";
static char *False="False";
static char *EmptyString="";


ATKdefineRegistry(sbutton, dataobject, sbutton::InitializeClass);
#ifndef NORCSID
#endif
static char *Intern(char  *str);
static void init(class sbutton  *self, int  i , int  j);
static boolean SetupInitialState(class sbutton  *self);
static void sbutton__WriteDataPart(class sbutton  *self, FILE  *fp);
static boolean newprefsproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean labelproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean litpproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean styleproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean fontproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean bgproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean fgproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean tsproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean topproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean bsproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean lfgproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean lbgproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean ilfgproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean ilbgproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean nameproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean prefsproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean doneproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static boolean triggerproc(class sbutton  *self, struct read_status  *rock, char  *buf);
static long dostuff(class sbutton  *self, FILE  *fp, struct read_status *rock, struct dataprocs  *procs);
static long sbutton__ReadDataPart(class sbutton  *self, FILE  *fp, int  dsversion);
static long sbutton_SanelyReturnReadError(class sbutton  *self, FILE  *fp, long  id, long  code);
static void WriteLine(FILE  *f, char  *l);
static char *GlomStrings(char  *s , char  *t);
static char *ReadLine(FILE  *f);
static char *EncodeFont(class fontdesc  *font);


static char *Intern(char  *str)
{
    class atom *a;
    if(str==NULL) return NULL;
    a=atom::Intern(str);
    if(a!=NULL) return (a)->Name();
    else return NULL;
}

boolean sbutton::InitializeClass()
{
    /* 
      Initialize all the class data.
      */
    buttonpushed=atom::Intern("buttonpushed");
    if(buttonpushed==NULL) return FALSE;
    observable::DefineTrigger(&sbutton_ATKregistry_ , buttonpushed);
    return(TRUE);
}


static void init(class sbutton  *self, int  i , int  j)
{
    while(i<=j)	{
	self->buttons[i].rock=0;
	self->buttons[i].lit=FALSE;
	self->buttons[i].sensitive=TRUE;
	self->buttons[i].label=NULL;
	self->buttons[i].trigger=NULL;
	self->buttons[i].prefs=self->prefs;
	self->prefs->refcount++;
	i++;
    }
}

static boolean SetupInitialState(class sbutton  *self)
{
    self->prefs=sbutton::GetNewPrefs(NULL);
    if(self->prefs==NULL) return FALSE;
    self->prefs->name=NewString("Default");
    self->prefs->writeid= (self)->GetWriteID();
    return TRUE;
}

boolean sbutton::EnsureSize(int  ind)
{
    int newsize,i=this->maxcount;
    if(ind<0) return FALSE;
    if(this->prefs==NULL && !SetupInitialState(this)) return FALSE;
    if(ind>=i) {
	if(ind<i*2) newsize=i*2;
	else newsize=ind+1;
	if(this->buttons) this->buttons=(struct sbutton_info *)realloc(this->buttons, sizeof(struct sbutton_info)*newsize);
	else this->buttons=(struct sbutton_info *)malloc(sizeof(struct sbutton_info)*newsize);
	if(!this->buttons) {
	    this->count=0;
	    this->maxcount=0;
	    return FALSE;
	}

	init(this, i, newsize-1);
	this->maxcount=newsize;
    }
    
    if(ind>=this->count) {
	this->count=ind+1;
	(this)->SetChangeFlag( sbutton_SIZECHANGED);
	if(this->rows*this->cols<this->count) {
	    switch(this->sizepolicy) {
		case sbutton_SizeFixed:
		    break;
		case sbutton_GrowRows:
		    this->rows=this->count/this->cols;
		    if(this->count%this->cols) this->rows++;
		    break;
		case sbutton_GrowColumns:
		    this->cols=this->count/this->rows;
		    if(this->count%this->rows) this->cols++;
		    break;
	    }
	}

    }
    return TRUE;
}


char *colorprefs[sbutton_COLORS]={
    "background",
    "topshadow",
    "top", 
    "bottomshadow",
    "foreground",
    "labelforeground",
    "labelbackground",
    "insensitivelabelforeground",
    "insensitivelabelbackground",
    "insensitivetop"
};

static char *defprefname="sbutton";

void sbutton::InitPrefs(struct sbutton_prefs  *prefs, char  *name)
{
	ATKinit;

    char buf[1024], *font;
    long style, size;
    int i;
    for(i=0;i<sbutton_COLORS;i++) {
	char *value;
	strncpy(buf, name, sizeof(buf)-strlen(colorprefs[i])-1);
	strcat(buf, colorprefs[i]);
	value=environ::GetProfile(buf);
	if(value != NULL) prefs->colors[i] = Intern(value);
    }
    strncpy(buf, name, sizeof(buf)-6); /* reserve space for the word style too. */
    strcat(buf,"style");
    prefs->style=environ::GetProfileInt(buf, prefs->style);
    strncpy(buf, name, sizeof(buf)-6); /* reserve space for the word style too. */
    strcat(buf,"Depth");
    prefs->bdepth=environ::GetProfileInt(buf, prefs->bdepth);
    strncpy(buf, name, sizeof(buf)-5); /* reserve space for the word font too. */
    strcat(buf,"font");
    font=environ::GetProfile(buf);
    if (fontdesc::ExplodeFontName(font,buf,sizeof(buf), &style, &size)) prefs->font=fontdesc::Create(buf,style,size);
}

struct sbutton_prefs *sbutton::GetNewPrefs(char  *name)
{
	ATKinit;

    char buf[1024];
    struct sbutton_prefs *new_c=(struct sbutton_prefs *)malloc(sizeof(struct sbutton_prefs));
    if(new_c==NULL) return NULL;
    memset(new_c, 0, sizeof(struct sbutton_prefs));
    new_c->style=(-1);
    new_c->refcount++;
    new_c->name=NULL;
    new_c->bdepth=(-1);
    
    if(name) {
	strncpy(buf, name, sizeof(buf)-8);
	strcat(buf, "Inherit");
    }
    if(name==NULL || environ::GetProfileSwitch("SbuttonAlwaysInherit", FALSE) ||  environ::GetProfileSwitch(buf, FALSE)) {
	sbutton::InitPrefs(new_c, defprefname);
    }
    return new_c;
}

void sbutton::WritePrefs(FILE  *fp, struct sbutton_prefs  *prefs)
{
	ATKinit;

    int i;
    char buf[1024];
    fprintf(fp, "\\newprefs\n\n");
    if(sbutton::GetStyle(prefs)>0) {
	fprintf(fp, "\\style\n");
	sprintf(buf, "%d", sbutton::GetStyle(prefs));
	WriteLine(fp, buf);
    }
    if(sbutton::GetFont(prefs)) {
	char *encfont;
	encfont =  EncodeFont(sbutton::GetFont(prefs));
	if (encfont) {
	    fprintf(fp, "\\font\n");
	    WriteLine(fp, encfont);
	    free(encfont);
	}
    }
    if(sbutton::GetDepth(prefs)>=0) {
	fprintf(fp, "\\depth\n%d\n", sbutton::GetDepth(prefs));
    }
    for(i=0;i<sbutton_COLORS;i++) {
	if(prefs->colors[i]) {
	    fprintf(fp, "\\%s\n",colorprefs[i]);
	    WriteLine(fp, prefs->colors[i]);
	}
    }
    fprintf(fp, "\\name\n");
    WriteLine(fp, prefs->name?prefs->name:EmptyString);
    fprintf(fp,"\\endprefs\n\n");
}

void sbutton::FreePrefs(struct sbutton_prefs  *prefs)
{
	ATKinit;

    prefs->refcount--;
    if(prefs->refcount<=0) {
	if(prefs->name!=NULL) free(prefs->name);
	free(prefs);
    }
}

struct sbutton_prefs *sbutton::DuplicatePrefs(struct sbutton_prefs  *prefs, char  *name)
{
	ATKinit;

    struct sbutton_prefs *result=(struct sbutton_prefs *)malloc(sizeof(struct sbutton_prefs));
    if(result==NULL) return result;
    *result=(*prefs);
    result->refcount=0;
    if(name!=NULL) result->name=NewString(name);
    else result->name=NULL;
    return result;
}

sbutton::sbutton()
{
	ATKinit;

    /*
      Inititialize the object instance data.
      */
    this->singleactivation=TRUE;

    this->activated=(-1);
    
    
    this->triggerchange=(-1);
    
    
    this->hitfunc=NULL;
    this->resizefunc=NULL;
    
    this->sizepolicy=sbutton_GrowRows;
    this->maxcount=0;
    this->count=0;
    this->buttons=NULL;
   
    this->prefs=NULL;

    /* self->prefs=sbutton_GetNewPrefs(NULL);
    if(self->prefs==NULL) return FALSE;
    self->prefs->name=NewString("Default");
    self->prefs->writeid= sbutton_GetWriteID(self);
*/
    this->matteprefs = NULL;
      
    this->rows=1;
    this->cols=1;
    
    this->change=0;
    
    THROWONFAILURE((TRUE));
}

sbutton::~sbutton()
{
	ATKinit;

    /*
      Finalize the object instance data.
      */
    int i=this->count;
    if(this->prefs) sbutton::FreePrefs(this->prefs);
    if(this->matteprefs) sbutton::FreePrefs(this->matteprefs);
    
    while(--i>=0) {
	struct sbutton_info *bi=this->buttons+i;
	if(bi->label) free(bi->label);
	if(bi->prefs) sbutton::FreePrefs(bi->prefs);
    }
    if(this->buttons) {
	free(this->buttons);
	this->buttons=NULL;
    }
    return;
}

void sbutton::SetChangeFlag(long  on)
{
    this->change|=on;
}


long sbutton::GetChangeType()
{
    return this->change;
}

static char *sizepolicies[]={
    "NoneSet",
    "Fixed",
    "Rows",
    "Columns",
    "Proc"
};

static void sbutton__WriteDataPart(class sbutton  *self, FILE  *fp)
{
    /*
      Write the object data out onto the datastream.
      */
    char buf[1024];

    int j, c=0;
    int i=self->count;

    for(j=0;j<self->count;j++) {
	self->buttons[j].prefs->writeid=(-1);
    }

    for(j=0;j<self->count;j++) {
	if(self->buttons[j].prefs->writeid==-1 && self->buttons[j].prefs!=self->prefs) {
	    c++;
	    self->buttons[j].prefs->writeid=(-2);
	}
    }
    
    sprintf(buf, "%d %d %d %s %d", i, self->rows, self->cols, sizepolicies[(unsigned int)self->sizepolicy], c+1);
    WriteLine(fp, buf);

    if(self->prefs) {
	sbutton::WritePrefs(fp, self->prefs);
	self->prefs->writeid=0;
    }
    j=0;
    while(--i>=0) {
	if(self->buttons[i].prefs && self->buttons[i].prefs->writeid<0) {
	    self->buttons[i].prefs->writeid=(++j);
	    sbutton::WritePrefs(fp, self->buttons[i].prefs);
	}
	if((self)->GetLabel( i)) {
	    fprintf(fp, "\\label\n");
	    WriteLine(fp, (self)->GetLabel( i));
	} else fprintf(fp, "\\label\n\n");
	fprintf(fp,"\\litp\n");
	WriteLine(fp, self->buttons[i].lit?True:False);
	fprintf(fp,"\\sensitive\n");
	WriteLine(fp, self->buttons[i].sensitive?True:False);
	fprintf(fp,"\\prefs\n");
	sprintf(buf,"%d", self->buttons[i].prefs->writeid);
	WriteLine(fp, buf);
	if(self->buttons[i].trigger) {
	    fprintf(fp, "\\trigger\n");
	    WriteLine(fp, (self->buttons[i].trigger)->Name());
	}
	
	if(i>0) fprintf(fp,"\\end\n\n");
    }
    fprintf(fp, "\\done\n\n");
}


long sbutton::Write(FILE  *fp, long  id, int  level)
{
    long uniqueid = (this)->UniqueID();

    if (id != (this)->GetWriteID()) {
	/* New Write Operation */
	(this)->SetWriteID( id);

	fprintf(fp, "\\begindata{%s,%d}\nDatastream version: %d\n",
		(this)->GetTypeName(), 
		uniqueid, DS_VERSION);

	sbutton__WriteDataPart(this, fp);

	fprintf(fp, "\\enddata{%s,%d}\n", (this)->GetTypeName(), uniqueid);
    }
    return(uniqueid);
}
struct read_status {
    int lastbutton;
    int lastprefs, maxprefs;
    struct sbutton_prefs **prefs;
};

static boolean newprefsproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    if(rock->prefs==NULL) return TRUE;
    rock->lastprefs++;
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) {
	rock->lastprefs=rock->maxprefs-1;
	return TRUE;
    }
    return FALSE;
}

static boolean labelproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    rock->lastbutton--;
    if(rock->lastbutton<0) return TRUE;
    if(*buf!='\0') (self)->SetLabel( rock->lastbutton, buf);
    return FALSE;
}

static boolean litpproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    (self)->SetLit( rock->lastbutton, *buf=='T');
    return FALSE;
}

static boolean sensitiveproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    (self)->SetSensitivity( rock->lastbutton, *buf=='T');
    return FALSE;
}
static boolean styleproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) return TRUE;
    sbutton::GetStyle(rock->prefs[rock->lastprefs]) = atoi(buf);
    return FALSE;
}

static boolean fontproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    class fontdesc *font=NULL;
    char buf2[1024];
    long style, size;
    
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) return TRUE;
    
    if(*buf=='\0') return FALSE;
    
    if(fontdesc::ExplodeFontName(buf, buf2, sizeof(buf2), &style, &size))	{
	font=fontdesc::Create(buf2,style,size);
	sbutton::GetFont(rock->prefs[rock->lastprefs]) = font;
    }
    return FALSE;
}

static boolean bgproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) return TRUE;
    if(*buf!='\0') sbutton::GetBackground(rock->prefs[rock->lastprefs]) = Intern(buf);
    return FALSE;
}

static boolean fgproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) return TRUE;
    if(*buf!='\0') sbutton::GetForeground(rock->prefs[rock->lastprefs]) = Intern(buf);
    return FALSE;
}


static boolean tsproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) return TRUE;
    if(*buf!='\0') sbutton::GetTopShadow(rock->prefs[rock->lastprefs]) = Intern(buf);
    return FALSE;
}

static boolean topproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) return TRUE;
    if(*buf!='\0') sbutton::GetTop(rock->prefs[rock->lastprefs]) = Intern(buf);
    return FALSE;
}


static boolean bsproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) return TRUE;
    if(*buf!='\0') sbutton::GetBottomShadow(rock->prefs[rock->lastprefs]) = Intern(buf);
    return FALSE;
}


static boolean lfgproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) return TRUE;
    if(*buf!='\0') sbutton::GetLabelFG(rock->prefs[ rock->lastprefs]) = Intern(buf);
    return FALSE;
}


static boolean lbgproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) return TRUE;
   if(*buf!='\0') sbutton::GetLabelBG(rock->prefs[rock->lastprefs]) = Intern(buf);
   return FALSE;
}

static boolean itopproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) return TRUE;
    if(*buf!='\0') sbutton::GetInsensitiveTop(rock->prefs[ rock->lastprefs]) = Intern(buf);
    return FALSE;
}

static boolean ilfgproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) return TRUE;
    if(*buf!='\0') sbutton::GetInsensitiveLabelFG(rock->prefs[ rock->lastprefs]) = Intern(buf);
    return FALSE;
}


static boolean ilbgproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) return TRUE;
   if(*buf!='\0') sbutton::GetInsensitiveLabelBG(rock->prefs[rock->lastprefs]) = Intern(buf);
   return FALSE;
}

static boolean depthproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    sbutton::GetDepth(rock->prefs[rock->lastprefs])=atoi(buf);
    return FALSE;
}

static boolean nameproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    if(rock->lastprefs<0 || rock->lastprefs>=rock->maxprefs) return TRUE;
    if(*buf!='\0') sbutton::GetName(rock->prefs[rock->lastprefs]) = NewString(buf);
    else {
	char buf2[256];
	sprintf(buf2, rock->lastprefs?"Default %d":"Default", rock->lastprefs);
	sbutton::GetName(rock->prefs[rock->lastprefs]) = NewString(buf2);
    }
    return FALSE;}

static boolean prefsproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    int i=atoi(buf);
    if(i<0 || i>=rock->maxprefs) return TRUE;
    (self)->SetPrefs( rock->lastbutton, rock->prefs[i]);
    return FALSE;
}

static boolean doneproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    return TRUE;
}

static boolean triggerproc(class sbutton  *self, struct read_status  *rock, char  *buf)
{
    (self)->SetTrigger( rock->lastbutton, buf);
    return FALSE;
}
typedef boolean (*dostufffptr)(class sbutton *self, struct read_status *rock, char *buf);

static struct dataprocs {
    char *name;
    dostufffptr func;
} sprocs[]={
    {"newprefs", (dostufffptr)newprefsproc},
    {"label", (dostufffptr)labelproc},
    {"litp", (dostufffptr)litpproc},
    {"sensitive", (dostufffptr)sensitiveproc},
    {"prefs", (dostufffptr)prefsproc},
    {"style", (dostufffptr)styleproc},
    {"font", (dostufffptr)fontproc},
    {"background", (dostufffptr)bgproc},
    {"topshadow", (dostufffptr)tsproc},
    {"top", (dostufffptr)topproc},
    {"bottomshadow", (dostufffptr)bsproc},
    {"foreground", (dostufffptr)fgproc},
    {"labelforeground", (dostufffptr)lfgproc},
    {"labelbackground", (dostufffptr)lbgproc},
    {"insensitivelabelforeground", (dostufffptr)ilfgproc},
    {"insensitivelabelbackground", (dostufffptr)ilbgproc},
    {"insensitivetop", (dostufffptr)itopproc},
    {"depth", (dostufffptr)depthproc},
    {"name", (dostufffptr)nameproc},
    {"trigger", (dostufffptr)triggerproc},
    {"end", (dostufffptr) NULL},
    {"done", (dostufffptr)doneproc},
    {NULL, NULL}
};

static long dostuff(class sbutton  *self, FILE  *fp, struct read_status *rock, struct dataprocs  *procs)
{
    char *buf, *buf2;
    boolean done=FALSE;
    while(!done) {
	struct dataprocs *dps;
	buf=ReadLine(fp);
	if(buf==NULL) return dataobject_PREMATUREEOF;
	if(!linehascontrol) {
	    free(buf);
	    continue;
	}
	buf2=ReadLine(fp);
	if(buf2==NULL) {
	    free(buf);
	    return dataobject_PREMATUREEOF;
	}
	for(dps=procs;dps->name;dps++) {
	    if(!strcmp(dps->name, buf+1)) {
		if(dps->func) done=(dps->func)(self, rock, buf2);
		break;
	    }
	}
	free(buf);
	free(buf2);
    }
    return dataobject_NOREADERROR;
}


 
static long sbutton__ReadDataPart(class sbutton  *self, FILE  *fp, int  dsversion)
{
    /*
      Read in the object from the file.
      */
    char *buf, *p;
    int rows, cols, count;
    enum sbutton_sizepolicy policy;
    long err=0;
    struct read_status rs;
    rs.maxprefs=0;
    rs.lastprefs=(-1);
    rs.lastbutton=(-1);
    rs.prefs=NULL;
    if ((buf = ReadLine(fp)) == NULL)
	    return(dataobject_PREMATUREEOF);
    count=atoi(buf);
    p=strchr(buf,' ');
    if(p) {
	rows=atoi(p+1);
	p=strchr(p+1, ' ');
	if(p) {
	   cols=atoi(p+1);
	   p=strchr(p+1, ' ');
	   if(p) {
	       switch(p[1]) {
		   case 'N':
		       policy=sbutton_NoneSet;
		       break;
		   case 'F':
		       policy=sbutton_SizeFixed;
		       break;
		   case 'R':
		       policy=sbutton_GrowRows;
		       break;
		   case 'C':
		       policy=sbutton_GrowColumns;
		       break;
		   case 'P':
		       policy=sbutton_ResizeFunction;
		       break;
		   default:
		       policy=sbutton_GrowRows;
	       }
	       (self)->SetLayout( rows, cols, policy);
	       p=strchr(p+1, ' ');
	       if(p) rs.maxprefs=atoi(p+1);
	   }
	}
    }
    free(buf);
    if(count<=0) return dataobject_PREMATUREEOF;

    rs.lastbutton=count;
    if(rs.maxprefs>0 && rs.maxprefs<=count+1) {
	int i;
	rs.prefs=(struct sbutton_prefs **)malloc(sizeof(struct sbutton_prefs *)*rs.maxprefs);
	if(rs.prefs==NULL) return dataobject_PREMATUREEOF;
	for(i=0;i<rs.maxprefs;i++) {
	    if(self->prefs==NULL) SetupInitialState(self);
	    rs.prefs[i] = sbutton::DuplicatePrefs(self->prefs, NULL);
	    if(rs.prefs[i]==NULL) return dataobject_PREMATUREEOF;
	    rs.prefs[i]->refcount=1;
	}
    } else return dataobject_NOREADERROR;
    err = dostuff(self, fp, &rs, sprocs);
    if(rs.prefs!=NULL) {
	int i;
	if(self->prefs) sbutton::FreePrefs(self->prefs);
	self->prefs=rs.prefs[0];
	for(i=1;i<rs.maxprefs;i++) {
	    if(rs.prefs[i]) sbutton::FreePrefs(rs.prefs[i]);
	}
    }
    return err;
}


static long sbutton_SanelyReturnReadError(class sbutton  *self, FILE  *fp, long  id, long  code)
{
    /*
      Suck up the file until our enddata, then return the error code.
      */
    char *buf, buf2[255];

    buf = NULL;
    sprintf(buf2, "\\enddata{%s,%ld}", (self)->GetTypeName(), id);
    do {
	if (buf != NULL) free(buf);
	if ((buf = ReadLine(fp)) == NULL)
	    return(dataobject_PREMATUREEOF);
    } while (strncmp(buf, "\\enddata{", 9) != 0); /* find an enddata */

    if (strcmp(buf, buf2) != 0) {
	free(buf);
	return(dataobject_MISSINGENDDATAMARKER); /* not ours! */
    }
    free(buf);

    return(code);
}


long sbutton::Read(FILE  *fp, long  id)
{

  char *buf;
  int dsversion;
  long err=dataobject_NOREADERROR;
  
  (this)->SetID( (this)->UniqueID());
  
  if ((buf = ReadLine(fp)) == NULL) err=dataobject_PREMATUREEOF;
  else if (strncmp(buf,"Datastream version:",19)) {
      err=dataobject_BADFORMAT;
  } else if ((dsversion = atoi(buf+19)) > DS_VERSION)	{
      err=dataobject_BADFORMAT;
  }
  if(buf) free(buf);

  if(err==dataobject_NOREADERROR) {
      return sbutton_SanelyReturnReadError(this, fp, id, sbutton__ReadDataPart(this, fp, dsversion));
  } else return(sbutton_SanelyReturnReadError(this, fp, id, err));
}


char *sbutton::ViewName()
{
    return "sbttnav";
}

void sbutton::SetSensitivity(int ind, boolean sensitive) {
    if(ind<0 || ind>=this->count) return;
    if(this->buttons[ind].sensitive!=sensitive) {
	unsigned long changes=sbutton_SENSITIVITYCHANGED;
	this->buttons[ind].sensitive=sensitive;
	(this)->SetModified();
	if(this->buttons[ind].lit) {
	    changes|=sbutton_ACTIVATIONCHANGED;
	    this->buttons[ind].lit=FALSE;
	}
	(this)->SetChangeFlag(changes);
	(this)->NotifyObservers(ind+sbutton_CHANGEBASE);
    }
}

void sbutton::ActivateButton(int  ind)
{
    if(ind<0 || ind>=this->count || !this->buttons[ind].sensitive) return;
    if(this->singleactivation && this->activated>=0 && this->activated<this->count && this->activated!=ind) (this)->DeActivateButton( this->activated);
    if(this->buttons[ind].lit) return;
    this->activated=ind;
    this->buttons[ind].lit=TRUE;
    (this)->SetModified();
    (this)->SetChangeFlag( sbutton_ACTIVATIONCHANGED);
    (this)->NotifyObservers( ind+sbutton_CHANGEBASE);
}

void sbutton::DeActivateButton(int  ind)
{
    if(ind<0 || ind>=this->count) return;
    if(this->activated==ind) this->activated=(-1);
    if(!this->buttons[ind].lit) return;
    this->buttons[ind].lit=FALSE;
    (this)->SetModified();
    (this)->SetChangeFlag( sbutton_ACTIVATIONCHANGED);
    (this)->NotifyObservers( ind+sbutton_CHANGEBASE);
}

void sbutton::Actuate(int  ind)
{
    if(id<0 || ind>=this->count || !this->buttons[ind].sensitive) return;
    struct owatch_data *w1=owatch::Create(this);
    if(this->hitfunc) this->hitfunc(this, this->hitfuncrock, ind, this->buttons[ind].rock);
    if(owatch::CheckAndDelete(w1)) {
	if(this->buttons[ind].trigger) (this)->PullTrigger( this->buttons[ind].trigger);
	else (this)->PullTrigger( buttonpushed);
    }
}

void sbutton::SetRock(int  ind, long  rock)
{
    if(!(this)->EnsureSize( ind)) return;

    this->buttons[ind].rock=rock;
}

void sbutton::SetLit(int  ind, boolean  onoff)
{
    if(!(this)->EnsureSize( ind)) return;
    this->buttons[ind].lit=onoff;
    (this)->SetModified();
    (this)->SetChangeFlag( sbutton_ACTIVATIONCHANGED);
    (this)->NotifyObservers( ind+sbutton_CHANGEBASE);
}


void sbutton::SetPrefs(int  ind, struct sbutton_prefs  *prefs)
{
    if(!(this)->EnsureSize( ind) || prefs==NULL) return;
    sbutton::FreePrefs(this->buttons[ind].prefs);
    this->buttons[ind].prefs=prefs;
    prefs->refcount++;
    (this)->SetModified();
    (this)->SetChangeFlag( sbutton_FONTCHANGED|sbutton_STYLECHANGED|sbutton_SIZECHANGED);
    (this)->NotifyObservers( ind+sbutton_CHANGEBASE);
}

void sbutton::SetLayout(int  rows , int  cols, enum sbutton_sizepolicy  policy)
{
    if(rows>0) this->rows=rows;
    if(cols>0) this->cols=cols;
    if(policy!=sbutton_NoneSet) this->sizepolicy=policy;
    if(this->rows*this->cols<this->count) {
	switch(this->sizepolicy) {
	    case sbutton_SizeFixed:
		break;
	    case sbutton_GrowRows:
		this->rows=this->count/this->cols;
		if(this->count%this->cols) this->rows++;
		break;
	    case sbutton_GrowColumns:
		this->cols=this->count/this->rows;
		if(this->count%this->rows) this->cols++;
		break;
	    case sbutton_ResizeFunction:
		if(this->resizefunc) this->resizefunc(this, this->resizerock);
	}
    }
    (this)->SetModified();
    (this)->SetChangeFlag( sbutton_SIZECHANGED);
    (this)->NotifyObservers( observable_OBJECTCHANGED);
}

void sbutton::Delete(int  ind)
{
    int i;
    struct sbutton_prefs *prefs;
    char *label;
    if(ind<0 || ind>=this->count) return;
    
    prefs=(this)->GetPrefs( ind);
    label=(this)->GetLabel( ind);
    
    if(prefs) sbutton::FreePrefs(prefs);
    if(label) free(label);
    
    for(i=ind;i<this->count-1;i++) {
	this->buttons[i]=this->buttons[i+1];
    }
    
    this->count--;
    init(this, this->count, this->count);
    
    (this)->EnsureSize( 0);
    
    (this)->SetModified();
    (this)->SetChangeFlag( sbutton_SIZECHANGED);
    (this)->NotifyObservers( observable_OBJECTCHANGED);
    return;
}

void sbutton::Swap(int  i1 , int  i2)
{
    struct sbutton_info si;
    if(i1<0 || i1>=this->count) return;
    if(i2<0 || i2>=this->count) return;

    si=this->buttons[i1];
    this->buttons[i1]=this->buttons[i2];
    this->buttons[i2]=si;
    (this)->SetModified();
    (this)->SetChangeFlag( sbutton_ALLCHANGED);
    (this)->NotifyObservers( observable_OBJECTCHANGED);
}
	
void sbutton::SetLabel(int  ind, char  *txt)
{
/*
  Set the text label for this object.
*/
    if(!(this)->EnsureSize( ind)) return;
    
    if (this->buttons[ind].label) {
      if(txt && !strcmp(this->buttons[ind].label, txt)) return;
      free(this->buttons[ind].label);
      this->buttons[ind].label = NULL;
    }
    
    if (txt)this->buttons[ind].label = NewString(txt);
    (this)->SetModified();
    (this)->SetChangeFlag( sbutton_LABELCHANGED);
    (this)->NotifyObservers( ind+sbutton_CHANGEBASE);
}

void sbutton::SetTrigger(int  ind, char  *txt)
{
/*
  Set the text label for this object.
*/
    if(!(this)->EnsureSize( ind)) return;
    this->triggerchange=ind;
    if(*txt=='\0') {
	this->buttons[ind].trigger=NULL;
    } else {
	this->buttons[ind].trigger=atom::Intern(txt);
	if(this->buttons[ind].trigger!=NULL) {
	    observable::DefineTrigger(this->ATKregistry(), this->buttons[ind].trigger);
	}
    }
    (this)->SetModified();
    (this)->SetChangeFlag( sbutton_TRIGGERCHANGED);
    (this)->NotifyObservers( ind+sbutton_CHANGEBASE);
}

int sbutton::Enumerate(sbutton_efptr func, long  rock)
{
    int i;
    for(i=0;i<this->count;i++) {
	if(func(this, i, &this->buttons[i], rock)) return i;
    }
    return -1;
}

static void WriteLine(FILE  *f, char  *l)
{
    /* 
      Output a single line onto the data stream, quoting
      back slashes and staying within line length limits.
      Warning:  this routine wasn't meant to handle embedded
  newlines.
  */

    char buf[MAX_LINE_LENGTH];
    int i = 0;

    for (;*l != '\0'; ++l) {
	if (i > (MAX_LINE_LENGTH - 5)) {
	    buf[i++] = '\\';  /* signal for line continuation */
	    buf[i++] = '\n';
	    buf[i++] = '\0';
	    fputs(buf,f);
	    i = 0;
	} /* if (i > ...) */
	switch (*l) {
	    case '\\': 
		/* if a backslash, quote it. */
		buf[i++] = '\\';
		buf[i++] = *l;
		break;
	    default:
		buf[i++] = *l;
	} /* switch (*l) */
    } /* for (; *l != ... ) */

    /* Need to empty buffer */
    if ((i > 0) && (buf[i-1]==' ')) {
	/* don't allow trailing whitespace */
	buf[i++] = '\\';
	buf[i++] = '\n';
	buf[i++] = '\0';
	fputs(buf,f);
	fputs("\n",f);
    } else {
	buf[i++] = '\n';
	buf[i++] = '\0';
	fputs(buf,f);
    }
}


static char *GlomStrings(char  *s , char  *t)
{
    /* 
      Safely (allocs more memory) concatenates the two strings, 
      freeing the first.  Meant to build a new string of unknown length.
      */

    char *r;

    if (r = (char *)malloc(strlen(s)+strlen(t)+1)) {
	*r = '\0';
	strcpy(r,s);
	free(s);
	strcat(r,t);
	return(r);
    } else {
	free(s);
	return(NULL);
    }
}

static char *ReadLine(FILE  *f)
{
    /* 
      Reads from the datastream, attempting to return a single string.
      Undoes quoting and broken lines.
      Warning:  this routine wasn't meant to handle embedded
	 newlines.
	 Warning:  possible source of memory leaks;  remember to 
	   free the returned string when finished with it!
	   */

    char buf[MAX_LINE_LENGTH], /* (BUG) What if the datastream is broken? */
    buf2[MAX_LINE_LENGTH],
    *result;
    int i,j;

    linehascontrol=FALSE;
    if (result = (char *)malloc(1)) {
	*result = '\0';

	while (fgets(buf,sizeof(buf),f)) {
	    for (i = 0, j = 0; buf[i] != '\0'; ++i) {
		switch (buf[i]) {
		    case '\\':
			/* Unquote backslash or splice line */
			switch (buf[++i]) {
			    case '\\':
				/* Unquote the backslash */
				buf2[j++] = buf[i];
				break;
			    case '\n':
				/* broke long line */
				break;
			    default:
				/* things like \enddata come through here */
				linehascontrol=TRUE;
				buf2[j++] = '\\';
				buf2[j++] = buf[i];
				break;
			} /* switch (buf[++i]) */
			break;
		    case '\n':
			/* An unquoted newline means end of string */
			buf2[j++] = '\0';
			result = GlomStrings(result, buf2);
			return(result);
		    default:
			buf2[j++] = buf[i];
			break;
		} /* switch (buf[i]) */
	    } /* for (i = 0, ...) */
	    buf2[j++] = '\0';
	    result = GlomStrings(result, buf2);
	} /* while (fgets...) */
	/* Should not get here... it means we went off the end
	 of the data stream.  Ooops. */
    } /* if (result = ... ) */
    return(NULL);
}

static char *EncodeFont(class fontdesc  *font)
{
    /*
      Returns a string representing the name of the font for this object.
	  (BUG) I shouldn't have to do this function, it should be a method
		of the fontdesc object.  In any case, I handle only Bold, Italic,
		    and fixed styles.
		    */

    char *buf, type[15];
    long myfonttype, myfontsize;
    char *myfontname;

    *type = '\0';
    myfontname = (font)->GetFontFamily();
    myfontsize = (font)->GetFontSize();
    myfonttype = (font)->GetFontStyle();
    if (myfonttype & fontdesc_Bold) strcpy(type,"b");
    if (myfonttype & fontdesc_Italic) strcpy(type,"i");
    if (myfonttype & fontdesc_Fixed) strcpy(type,"f");
    if (buf = (char *)malloc(strlen(myfontname)+25)) {
	sprintf(buf,"%s%d%s", myfontname, myfontsize, type);
	return (buf);
    } else {
	return(NULL);
    }
}


class sbutton *sbutton::CreateSButton(struct sbutton_prefs  *prefs)
{
	ATKinit;

    class sbutton *self=new sbutton;
    int i;
    if(!self) return self;

    /* get rid of default contents... */

    if(self->prefs) sbutton::FreePrefs(self->prefs);
    
    i=self->count;
    if(self->buttons) {
	while(--i>=0) {
	    struct sbutton_info *bi=self->buttons+i;
	    if(bi->prefs) sbutton::FreePrefs(bi->prefs);
	    if(bi->label) free(bi->label);

	}
	free(self->buttons);
	self->buttons=NULL;
    }

    self->prefs=prefs;
    prefs->refcount++;
    
    self->count=0;
    self->maxcount=0;


    (void) (self)->EnsureSize( 0);

    self->change=0;
    
    return self;
}


class sbutton *sbutton::CreateFilledSButton(struct sbutton_prefs  *prefs, struct sbutton_list  *blist)
{
	ATKinit;

    class sbutton *self=sbutton::CreateSButton(prefs);
    int count=0;
    struct sbutton_list *bl=blist;

    if(!self) return self;

    if(bl) while(bl->label) {
	count++;
	bl++;
    }
    while(--count>=0) {
	(self)->SetLabel( count, blist[count].label);
	if(blist[count].trigger) (self)->SetTrigger( count, blist[count].trigger);
	(self)->SetRock( count, blist[count].rock);
	(self)->SetLit( count, blist[count].lit);
    }
    return self;
}

void sbutton::NotifyObservers(long  v)
{
    /* Do all the notifications, then reset the change flag. */
    (this)->dataobject::NotifyObservers( v);
    this->change=0;
}

