/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/prefed/RCS/prefval.C,v 1.6 1994/11/30 20:42:06 rr2b Stab74 $";
#endif



 


#include <andrewos.h>
ATK_IMPL("prefval.H")
#include <math.h>

#include <ctype.h>

#include <observable.H>

#include <util.h>

#include "prefval.H"

#define zfree(x) do { if(x) { free(x); (x)=NULL;}} while (0)

#define MAX_LINE_LENGTH 70

 


ATKdefineRegistry(prefval, dataobject, NULL);
#ifndef NORCSID
#endif
static char *strsave(char  *str);
static boolean EnsureListSize(class prefval  *self, int  n);
static boolean EnsureChoiceListSize(class prefval  *self, int  n);
static boolean appproc(class prefval  *self, FILE  *fp, char  *buf);
static boolean prefproc(class prefval  *self, FILE  *fp, char  *buf);
static boolean listproc(class prefval  *self, FILE  *fp, char  *buf);
static boolean clistproc(class prefval  *self, FILE  *fp, char  *buf);
static boolean typeproc(class prefval  *self, FILE  *fp, char  *buf);
static boolean condproc(class prefval  *self, FILE  *fp, char  *buf);
static boolean sepproc(class prefval  *self, FILE  *fp, char  *buf);
static boolean listmaxproc(class prefval  *self, FILE  *fp, char  *buf);
static boolean doneproc(class prefval  *self, FILE  *fp, char  *buf);
static long dostuff(class prefval  *self, FILE  *fp, long  rock, struct dataprocs  *procs);
static char *PreferenceString(class prefval  *self);
static long prefval_SanelyReturnReadError(class prefval  *self, FILE  *fp, long  id, long  code);
static int compare_choices(int  *i1 , int  *i2);
static char *GlomStrings(char  *s , char  *t);
static char *ReadLine(FILE  *f);


static char *strsave(char  *str)
{
    char *result;

    if(str==NULL) return NULL;

    result=(char *)malloc(strlen(str)+1);
    
    if(result==NULL) return NULL;

    strcpy(result, str);

    return result;
}

void prefval::FreeValue(struct prefval_value  *v)
{
    switch((this)->GetType()) {
	case prefval_Filename:
	    zfree(v->v.fval);
	    break;
	case prefval_Directory:
	    zfree(v->v.dval);
	    break;
	case prefval_String:
	    zfree(v->v.sval);
	    break;
	case prefval_Font:
	    zfree(v->v.fontval);
	    break;
	case prefval_Color:
	    zfree(v->v.cval);
	    break;
	default: ;
    }
}

/* copy v2 to v1. */
void prefval::CopyValue(struct prefval_value  *v1 , struct prefval_value  *v2)
{
    (this)->FreeValue( v1);
    switch((this)->GetType()) {
	case prefval_Filename:
	    v1->v.fval=strsave(v2->v.fval);
	    break;
	case prefval_Directory:
	    v1->v.dval=strsave(v2->v.dval);
	    break;
	case prefval_String:
	    v1->v.sval=strsave(v2->v.sval);
	    break;
	case prefval_Font:
	    v1->v.fontval=strsave(v2->v.fontval);
	    break;
	case prefval_Color:
	    v1->v.cval=strsave(v2->v.cval);
	    break;
	default: 
	    *v1 = (*v2);
	    break;
    }
    v1->set=v2->set;
}

void prefval::InitValue(struct prefval_value  *v)
{
    switch((this)->GetType()) {
	case prefval_Filename:
	    v->v.fval=NULL;
	    break;
	case prefval_Directory:
	    v->v.dval=NULL;
	    break;
	case prefval_String:
	    v->v.sval=NULL;
	    break;
	case prefval_Font:
	    v->v.fontval=NULL;
	    break;
	case prefval_Color:
	    v->v.cval=NULL;
	    break;
	case prefval_Boolean:
	case prefval_Integer:
	case prefval_Real:
	    break;
	default:
	    fprintf(stderr, "WARNING: prefval_InitValue called on prefval of unkown type %d.\n", (this)->GetType());
	    break;
    }
    v->set=TRUE;
}

static boolean EnsureListSize(class prefval  *self, int  n)
{
    struct prefval_value *result=self->values;
    
    if(n<self->vlistsize) return TRUE;
    
    if(result) {
	result=(struct prefval_value *)realloc(result, sizeof(struct prefval_value)*(n+1));
    } else result=(struct prefval_value *)malloc(sizeof(struct prefval_value)*(n+1));
    if(result) {
	int i;
	for(i=self->vlistsize;i<=n;i++) {
	    (self)->InitValue( result+i);
	}
	self->vlistsize=n+1;
	self->values=result;
	return TRUE;
    } else return FALSE;
}

static boolean EnsureChoiceListSize(class prefval  *self, int  n)
{
    struct prefval_value *cvalues=self->cvalues;
    char **choices=self->choices;
    int i;

    if(n<self->clistsize) return TRUE;

    if(cvalues) {
	cvalues=(struct prefval_value *)realloc(cvalues, sizeof(struct prefval_value)*(n+1));
    } else cvalues=(struct prefval_value *)malloc(sizeof(struct prefval_value)*(n+1));
    
    if(cvalues==NULL) return FALSE;

    if(choices) {
	choices=(char **)realloc(choices, sizeof(char *)*(n+1));
    } else choices=(char **)malloc(sizeof(char *)*(n+1));

    if(choices==NULL) {
	free(cvalues);
	return FALSE;
    }

    for(i=self->clistsize;i<=n;i++) {
	choices[i]=NULL;
	(self)->InitValue( cvalues+i);
    }
    self->clistsize=n+1;
    self->choices=choices;
    self->cvalues=cvalues;
    return TRUE;
}

prefval::prefval()
{
    this->listmax=1;
    this->valueset=FALSE;
    this->prefname=NULL;
    this->appname=NULL;
    this->condition=NULL;
    this->type=prefval_Integer;
    this->vlistsize=0;
    this->values=NULL;
    this->clistsize=0;
    this->choices=NULL;
    this->cvalues=NULL;
    this->separator=NULL;
    this->curitem=0;
    THROWONFAILURE( TRUE);
}

prefval::~prefval()
{
    zfree(this->separator);
    zfree(this->prefname);
    zfree(this->appname);
    if(this->values) {
	int i;
	for(i=0;i<this->vlistsize;i++) {
	    (this)->FreeValue( this->values+i);
	}
	zfree(this->values);
    }
    if(this->choices) {
	int i;
	for(i=0;i<this->clistsize;i++) {
	    if(this->cvalues) (this)->FreeValue( this->cvalues+i);
	    zfree(this->choices[i]);
	}
	zfree(this->choices);
    }
}

static boolean appproc(class prefval  *self, FILE  *fp, char  *buf)
{
    (self)->SetAppName( buf);
    return FALSE;
}

static boolean prefproc(class prefval  *self, FILE  *fp, char  *buf)
{
    (self)->SetPrefName( buf);
    return FALSE;
}

static boolean listproc(class prefval  *self, FILE  *fp, char  *buf)
{
    int i=atoi(buf);
    
    if((self)->GetType()==prefval_None) return TRUE;

    while(--i>=0) {
	char *tbuf=ReadLine(fp);
	struct prefval_value *pv;

	if(tbuf==NULL) return FALSE;
	
	pv = (self)->StringToValue( tbuf);
	
	(self)->SetIndexValue( i, pv);
	free(tbuf);
    }
    return FALSE;
}

static boolean clistproc(class prefval  *self, FILE  *fp, char  *buf)
{
    int i=atoi(buf);
    
    if((self)->GetType()==prefval_None) return TRUE;
    
    while(--i>=0) {
	char *tbuf=ReadLine(fp), *tbuf2;
	struct prefval_value *pv;

	if(tbuf==NULL) return TRUE;
	
	pv = (self)->StringToValue( tbuf);
	
	tbuf2=ReadLine(fp);
	if(tbuf2==NULL) {
	    free(tbuf);
	    return TRUE;
	}
	(self)->SetChoice( i, tbuf2, pv);
	free(tbuf);
	free(tbuf2);
    }
    return FALSE;
}


static boolean typeproc(class prefval  *self, FILE  *fp, char  *buf)
{
    (self)->SetType( prefval::StringToType(buf));
    return FALSE;
}

static boolean condproc(class prefval  *self, FILE  *fp, char  *buf)
{
    (self)->SetCondition( buf);
    return FALSE;
}

static boolean sepproc(class prefval  *self, FILE  *fp, char  *buf)
{
    (self)->SetSeparator( buf);
    return FALSE;
}

static boolean listmaxproc(class prefval  *self, FILE  *fp, char  *buf)
{
    (self)->SetListMax( atoi(buf));
    return FALSE;
}

static boolean doneproc(class prefval  *self, FILE  *fp, char  *buf)
{
    return TRUE;
}


typedef boolean (*readfptr)(class prefval *self, FILE *fp, char *buf);

static struct dataprocs {
    char *name;
    readfptr func;
} sprocs[]={
    {"appname", appproc},
    {"prefname", prefproc},
    {"list", listproc},
    {"clist", clistproc},
    {"type", typeproc},
    {"separator", sepproc},
    {"listmax", listmaxproc},
    {"cond", condproc},
    {"done", doneproc},
    {NULL, NULL}
};

/* set in ReadLine if a line has a control directive */
static boolean linehascontrol;

static long dostuff(class prefval  *self, FILE  *fp, long  rock, struct dataprocs  *procs)
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
		if(dps->func) done=(dps->func)(self, (FILE *)rock, buf2);
		break;
	    }
	}
	free(buf);
	free(buf2);
    }
    
    return dataobject_NOREADERROR;
}


 
 long prefval::ReadDataPart(FILE  *fp, int  dsversion)
{
    /*
      Read in the object from the file.
      */
    long err;
    err = dostuff(this, fp, (long)fp, sprocs);
    return err;
}

 struct prefval_value *prefval::StringToValue(char  *str)
{
    static struct prefval_value sv;
    if((this)->GetType()==prefval_None) return NULL;
    sv.set=TRUE;
    switch((this)->GetType()) {
	case prefval_Integer:
	    sv.v.ival=atoi(str);
	    break;
	case prefval_Real:
	    sv.v.rval=(float)atof(str);
	    break;
	case prefval_Filename:
	    sv.v.fval=str;
	    break;
	case prefval_Directory:
	    sv.v.dval=str;
	    break;
	case prefval_Font:
	    sv.v.fontval=str;
	    break;
	case prefval_String:
	    sv.v.sval=str;
	    break;
	case prefval_Boolean:
	    sv.v.bval=(*str=='T' || *str=='t' || *str=='y' || *str=='Y' || (*str=='o' && str[1]=='n'));
	    break;
	case prefval_Color:
	    sv.v.sval=str;
	    break;
	default:
	    fprintf(stderr, "prefval: WARNING: unkown type for preference %s\n", (this)->GetPrefName()?(this)->GetPrefName():"(NULL)");
	    return NULL;
    }
    return &sv;
}

static struct prefvaltypes {
    char *name;
    enum prefval_Type type;
} types[] = {
    {"Integer", prefval_Integer},
    {"Real", prefval_Real},
    {"Boolean", prefval_Boolean},
    {"String", prefval_String},
    {"Filename", prefval_Filename},
    {"Directory", prefval_Directory},
    {"Font", prefval_Font},
    {"Color", prefval_Color},
    {NULL, prefval_None}
};


enum prefval_Type prefval::StringToType(char  *str)
{
    struct prefvaltypes *t=types;

    while(t->name) {
	if(FOLDEDEQ(t->name, str)) {
	    return t->type;
	}
	t++;
    }
    fprintf(stderr, "prefval WARNING: couldn't map '%s' to a type specifier!\n", str);
    return prefval_None;
}

char *prefval::TypeString()
{
    
    struct prefvaltypes *t=types;

    while(t->name) {
	if(t->type==(this)->GetType()) {
	    return t->name;
	}
	t++;
    }
    return "Unkown";
}

char *prefval::IndexValueString(int  which)
{
    static char buf[1024];
    if(which>=this->vlistsize) return "NO VALUE 3!";
    if(!this->values[which].set) {
	return NULL;
    }
    switch((this)->GetType()) {
	case prefval_None:
	    return "NO VALUE 2!";
	case prefval_Filename:
	case prefval_Directory:
	case prefval_Font:
	case prefval_Color:
	case prefval_String:
	    return this->values[which].v.sval;
	case prefval_Boolean:
	    return (char *)(this->values[which].v.bval?"Yes":"No");
	case prefval_Integer:
	    sprintf(buf, "%d", this->values[which].v.ival);
	    return buf;
	case prefval_Real:
	    sprintf(buf, "%f", this->values[which].v.rval);
	    return buf;
	default:
	    ;
    }
    return "NO VALUE 1!";
}

static char prefvalbuf[1024];
static char *PreferenceString(class prefval  *self)
{
    char *buf=prefvalbuf;
    int i;
    if((self)->GetAppName()) {
	strcat(buf, (self)->GetAppName());
	strcat(buf, ".");
    }
    if((self)->GetPrefName()) {
	strcat(buf, (self)->GetPrefName());
	strcat(buf,": ");
    }
    for(i=(self)->GetListSize()-1;i>=0;i--) {
	char *vs=(self)->IndexValueString( i);
	strcat(buf, vs?vs:"");
	if(i && (self)->GetSeparator()) strcat(buf, (self)->GetSeparator());
    }
    return buf;
}


char *prefval::PreferenceString()
{
    prefvalbuf[0]='\0';
    return ::PreferenceString(this);
}

char *prefval::FullPreferenceString()
{
    if((this)->GetCondition()) {
	sprintf(prefvalbuf, "?%s:", (this)->GetCondition());
    } else prefvalbuf[0]='\0';
    return ::PreferenceString(this);
}


void prefval::SetFromPreferenceString(char  *str)
{
    (this)->ClearValues();
    if(this->separator) {
	char *b=str;
	char *p;
	int i=0;
	
	do {
	    p=strchr(b, this->separator[0]);
	    if(p) *p='\0';
	    i++;
	    if(p) *p=this->separator[0];
	    b=p+1;
	    if(p && this->separator[0] && this->separator[1]==' ') {
		while(isspace(*b)) b++;
	    }
	} while (p);
	
	b=str;
	
	do {
	    p=strchr(b, this->separator[0]);
	    if(p) *p='\0';
	    i--;
	    if(*b) (this)->SetIndexValue( i, (this)->StringToValue( b));
	    if(p) *p=this->separator[0];
	    b=p+1;
	    if(p && this->separator[0] && this->separator[1]==' ') {
		while(isspace(*b)) b++;
	    }
	} while (p);
    } else {
	(this)->SetIndexValue( 0, (this)->StringToValue( str));
    }
}


static long prefval_SanelyReturnReadError(class prefval  *self, FILE  *fp, long  id, long  code)
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


long prefval::Read(FILE  *fp, long  id)
{

  char *buf;
  int dsversion;
  long err=dataobject_NOREADERROR;
  
  (this)->SetID( (this)->UniqueID());

  if ((buf = ReadLine(fp)) == NULL) err=dataobject_PREMATUREEOF;
  else if (strncmp(buf,"Datastream version:",19)) {
      err=dataobject_BADFORMAT;
  } else if ((dsversion = atoi(buf+19)) > prefval_DS_VERSION)	{
      err=dataobject_BADFORMAT;
  }
  if(buf) free(buf);

  (this)->SetModified();
  (this)->NotifyObservers( prefval_Generic);
  if(err==dataobject_NOREADERROR) {
      return prefval_SanelyReturnReadError(this, fp, id, (this)->ReadDataPart( fp, dsversion));
  } else return(prefval_SanelyReturnReadError(this, fp, id, err));
}



long prefval::Write(FILE  *file, long  writeID, int  level)
{
    if ((this)->GetWriteID() != writeID)  {
	int i=(this)->GetListSize();
	(this)->SetWriteID(writeID);
	fprintf(file, "\\begindata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
	fprintf(file, "Datastream version:%d\n",prefval_DS_VERSION);

	/* write the type info */
	fprintf(file, "\\type\n%s\n", (this)->TypeString());
	if((this)->GetSeparator()) fprintf(file, "\\separator\n%s\n", (this)->GetSeparator());
	if((this)->GetCondition()) fprintf(file, "\\cond\n%s\n", (this)->GetCondition());
	/* and the list */
	if((this)->GetListMax()>1) fprintf(file, "\\listmax\n%d\n", (this)->GetListMax());
	fprintf(file, "\\list\n%d\n", i);
	while(--i>=0) {
	    char *vs=(this)->IndexValueString( i);
	    fprintf(file, "%s\n", vs?vs:"");
	}
	fprintf(file, "\\done\n\n");
	fprintf(file, "\\enddata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
    }

    return (this)->GetID();
}


void prefval::SetAppName(char  *name)
{
    zfree(this->appname);
    this->appname=strsave(name);
    (this)->SetModified();
}

void prefval::SetPrefName(char  *name)
{
    zfree(this->prefname);
    this->prefname=strsave(name);
    (this)->SetModified();
}

void prefval::SetSeparator(char  *name)
{
    zfree(this->separator);
    this->separator=strsave(name);
    (this)->SetModified();
}

void prefval::ClearChoices()
{
    int i;
    for(i=0;i<this->clistsize;i++) {
	if(this->choices) zfree(this->choices[i]);
	if(this->cvalues) (this)->FreeValue( this->cvalues+i);
    }
    zfree(this->choices);
    zfree(this->cvalues);
    this->clistsize=0;
}

void prefval::ClearValues()
{
    int i;
    for(i=0;i<this->vlistsize;i++) {
	if(this->values) (this)->FreeValue( this->values+i);
    }
    zfree(this->values);
    this->vlistsize=0;
}


void prefval::SetChoices(int  nchoices, char  **choices, struct prefval_value  *tvalues)
{
    int i;
    
    (this)->ClearChoices();

    if(!EnsureChoiceListSize(this, nchoices-1)) return;
    
    this->clistsize=nchoices;

    for(i=0;i<nchoices;i++) {
	this->choices[i]=(char *)malloc(strlen(choices[i])+1);
	if(this->choices[i]) strcpy(this->choices[i], choices[i]);
	(this)->CopyValue( this->cvalues+i, tvalues+i);
    }
    (this)->SetModified();
}

void prefval::SetChoice(int  which, char  *choice, struct prefval_value  *tvalue)
{
    if(!EnsureChoiceListSize(this, which)) return;
    
    if(choice) {
	this->choices[which]=(char *)malloc(strlen(choice)+1);
	if(this->choices[which]) strcpy(this->choices[which], choice);
    } else this->choices[which]=NULL;
    
    (this)->CopyValue( this->cvalues+which, tvalue);
    
    (this)->SetModified();
}

void prefval::SetValues(int  nvalues, struct prefval_value  *tvalues)
{
    int i;

    if(!EnsureListSize(this, nvalues-1)) return;

    for(i=0;i<nvalues;i++) (this)->CopyValue( this->values+i, tvalues+i);

    this->valueset=TRUE;
    this->vlistsize=nvalues;
    (this)->SetModified();
}

void prefval::SetIndexValue(int  which, struct prefval_value  *tvalue)
{
    if(!EnsureListSize(this, which)) return;
    if(tvalue) (this)->CopyValue( this->values+which, tvalue);
    if(tvalue) this->valueset=TRUE;
    (this)->SetModified();
}

void prefval::SetValue(struct prefval_value  *tvalue)
{
    (this)->SetIndexValue( 0, tvalue);
}

static char **choices=NULL;
static struct prefval_value *cvalues=NULL;

static int compare_choices(int  *i1 , int  *i2)
{
    if(::choices==NULL) return 0;
    
    return strcmp(::choices[*i2], ::choices[*i1]);
}
	     
void prefval::SortChoices()
{
    int *indices;
    int i;
    int listsize=this->clistsize;
    if(listsize==0) return;
    
    ::choices=(char **)malloc(sizeof(char *)*listsize);
    
    if(::choices==NULL) return;
    
    ::cvalues=(struct prefval_value *)malloc(sizeof(struct prefval_value)*listsize);

    if(::cvalues==NULL) {
	zfree(::choices);
	return;
    }

    indices=(int *)malloc(sizeof(int)*listsize);

    if(indices==NULL) {
	zfree(::choices);
	zfree(::cvalues);
	return;
    }

    for(i=0;i<listsize;i++) {
	::choices[i]=strsave(this->choices[i]);
	(this)->InitValue( ::cvalues+i);
	(this)->CopyValue( ::cvalues+i, this->cvalues+i);
	indices[i]=i;
    }

    qsort(indices, listsize, sizeof(int), QSORTFUNC(compare_choices));


    (this)->ClearChoices();

    for(i=listsize-1;i>=0;i--) {
	int ind=indices[i];
	(this)->SetChoice( i, ::choices[ind], ::cvalues+ind);
	zfree(::choices[ind]);
	(this)->FreeValue( ::cvalues+ind);
    }

    zfree(::choices);
    zfree(::cvalues);
    zfree(indices);
}


void prefval::SetCondition(char  *cond)
{
    zfree(this->condition);
    this->condition=strsave(cond);
    (this)->SetModified();
}

void prefval::SetModified()
{
    this->isdefault=FALSE;
    (this)->dataobject::SetModified();
}

void prefval::SetDefault()
{
    this->isdefault=TRUE;
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
