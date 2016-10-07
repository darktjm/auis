/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* This code is taken , in part, from the switcher inset of N. Borenstein's
Andrew Toolkit Book . It has been modified and used with the permission
of the author */

#include <andrewos.h>
ATK_IMPL("page.H")
#include <page.H>
#include <dataobject.H>
#include <cel.H>

#define page_BYDATAOBJECT -10


ATKdefineRegistry(page, dataobject, NULL);
static struct page_switchee *FindSwitchee(class page  *self, class dataobject  *d,long  which);
static boolean SetSwitchee(class page  *self,struct page_switchee  *sw);


page::page()
{
    this->FirstSwitchee = NULL;
    this->NowPlaying = NULL;
    this->PostMenusFlag = TRUE;
    THROWONFAILURE((TRUE));
}

page::~page()
{
    struct page_switchee *sw;
    for (sw = this->FirstSwitchee; sw; sw = sw->next) {
	(sw->d)->Destroy();
	free(sw->label);
	free(sw->viewname);
    }
}
static struct page_switchee *FindSwitchee(class page  *self, class dataobject  *d,long  which)
{
    struct page_switchee *sw = NULL;
    struct page_switchee *Now = self->NowPlaying;
    switch( which){
	case page_CURRENT:
	    sw = self->FirstSwitchee;
	    break;
	case page_AFTERCURRENT:
	    if(Now != NULL){
		if((sw = Now->next) == NULL)
		    sw = self->FirstSwitchee;
	    }
	    break;
	case page_BEFORECURRENT :
	    if(Now != NULL){
		if(Now == self->FirstSwitchee)
		    Now = NULL;
		for (sw = self->FirstSwitchee; sw; sw = sw->next) {
		    if (sw->next == Now) break;
		}
	    }
	    break;
	case page_ATEND :
	    for (sw = self->FirstSwitchee; sw; sw = sw->next) {
		if (sw->next == NULL) break;
	    }
	    break;
	case page_ATBEGINING :
	    sw = self->FirstSwitchee;
	    break;
	case page_BYDATAOBJECT:
	    for (sw = self->FirstSwitchee; sw; sw = sw->next) {
		if (sw->d == d) break;
	    }
	    break;
	default:
	    for (sw = self->FirstSwitchee; sw; sw = sw->next) {
		if (--which <= 0) break;
	    }
	    break;
    }
    return sw;
}
static boolean SetSwitchee(class page  *self,struct page_switchee  *sw)
{
    if(sw == self->NowPlaying) return(FALSE); /* no change */
    self->NowPlaying = sw;
    (self)->NotifyObservers(observable::OBJECTCHANGED);
    return(TRUE);
}

long page::Write(FILE  *fp ,long  writeid,int  level)
{
    struct page_switchee *sw;
    if ((this)->GetWriteID() != writeid) {
	(this)->SetWriteID( writeid);

	fprintf(fp, "\\begindata{page,%ld}\n",
		(this)->GetID());
	for (sw = this->FirstSwitchee; sw; sw=sw->next) {
	    if (sw == this->NowPlaying) {
		fprintf(fp, "*%s\n", sw->label);
	    } else {
		fprintf(fp, "%s\n", sw->label);
	    }
	    (sw->d)->Write( fp, writeid, level+1);
	    fprintf(fp, "\\view{%s}\n", sw->viewname);
	}
	fprintf(fp,"\\enddata{page,%ld}\n",
		(this)->GetID());
    }
    return (this)->GetID();
}

long page::Read(FILE  *fp, long  id)
{
    char LineBuf[250], Label[250], *s, *obidstr;
    const char *thisname;
    char *lp;
    int status, obid;
    class dataobject *newob = NULL;

    while (TRUE) {
	if (fgets(Label, sizeof(Label)-1, fp) == NULL) {
	    return(dataobject::PREMATUREEOF);
	}
	if (!strncmp(Label, "\\enddata{page", 13)) {
	    break;
	}
	if (fgets(LineBuf, sizeof(LineBuf)-1, fp) == NULL) {
	    return(dataobject::PREMATUREEOF);
	}
	if (strncmp(LineBuf, "\\begindata{", 11)) {
	    return(dataobject::BADFORMAT);
	}
	thisname = lp = &LineBuf[11];
	obidstr = strchr(lp, ',');
	if (!obidstr) return(dataobject::BADFORMAT);
	*obidstr++ = '\0';
	s = strchr(obidstr, '}');
	if (!s) return(dataobject::BADFORMAT);
	*s = '\0';
	obid = atoi(obidstr);
	if(!ATK::LoadClass(thisname)) {
	    thisname="unknown";
	}
	if ((newob = (class dataobject *)
	     ATK::NewObject(thisname)))  {
	    status = (newob)->Read( fp, obid);
	    if (status != dataobject::NOREADERROR) {
		return status;
	    }
	} else {
	    return(dataobject::OBJECTCREATIONFAILED);
	}
	if (fgets(LineBuf, sizeof(LineBuf)-1, fp) == NULL) {
	    return(dataobject::PREMATUREEOF);
	}
	if (strncmp(LineBuf, "\\view{", 6)) {
	    return(dataobject::BADFORMAT);
	}
	thisname = lp = &LineBuf[6];
	s = strchr(lp, '}');
	if (s) *s = '\0';
	s = strchr(Label, '\n');
	if (s) *s = '\0';
	if (Label[0] == '*') {
	    if (!(this)->AddObject( newob,
				    Label+1, thisname,page_ATEND)) {
		return(dataobject::OBJECTCREATIONFAILED);
	    }
	    (this)->SetNowPlaying( newob);
	} else if (!(this)->AddObject( newob,
				       Label, thisname,page_ATEND)) {
		return(dataobject::OBJECTCREATIONFAILED);
	}
    }
    if(this->NowPlaying == NULL && this->FirstSwitchee != NULL)
	SetSwitchee(this,this->FirstSwitchee);
    return dataobject::NOREADERROR;

}


boolean page::AddObject(class dataobject  *d, const char  *label , const char  *viewname,long which)
{
    struct page_switchee *sw, *swtmp;

    sw = (struct page_switchee *) malloc(sizeof(struct page_switchee));
    if (sw == NULL) return(FALSE);
    if (label == NULL) label = "Next object";
    if (viewname == NULL) viewname = (d)->ViewName();
    if (viewname == NULL) viewname = "view";
    sw->d = d;
    sw->label = strdup(label);
    if (sw->label == NULL) return(FALSE);
    sw->viewname = strdup(viewname);
    if (sw->viewname == NULL) return(FALSE);
    sw->next = NULL;

    /* find right place to put it 
      swtmp is set to the existing object that should be made to point to the new object 
      or NULL if the new object should be first */
    switch(which){
	case page_CURRENT:
	    /* inserting as the current object is the same as 
	     inserting after the current object , except that the current object will be updated */
	case page_AFTERCURRENT:
	    swtmp = this->NowPlaying;
	    break;	
	case page_BEFORECURRENT:
	case page_ATEND:
	    swtmp = FindSwitchee(this, NULL ,which);
	    break;
	case page_ATBEGINING:
	case 0:
	case 1:
	    /* insert as first object */
	    swtmp = NULL;
	    break;
	default:
	    /* insert as numbered object */
	    swtmp = FindSwitchee(this, NULL ,--which);
	    break;
    }
    if(swtmp){
	/* insert after swtmp */
	sw->next = swtmp->next;
	swtmp->next = sw;
    }
    else {
	/* insert as first object */
	sw->next = this->FirstSwitchee;
	this->FirstSwitchee = sw;
	}
    if (this->NowPlaying == NULL || which == page_CURRENT) {
	SetSwitchee(this,sw);
    }
    return(TRUE);
}

boolean page::DeleteObject(class dataobject  *d)
{
    struct page_switchee *sw, *prevsw;

    for(prevsw = NULL, sw = this->FirstSwitchee;
	 sw != NULL;
	  sw = sw->next) {
	if (sw->d == d) {
	    if (prevsw != NULL) {
		prevsw->next = sw->next;
	    } else {
		this->FirstSwitchee = sw->next;
	    }
/*	    dataobject::Destroy(sw->d); */
	    free(sw->label);
	    free(sw->viewname);
	    if(this->NowPlaying == sw){
		this->NowPlaying = this->FirstSwitchee;
		(this)->NotifyObservers(0);
	    }
	    return(TRUE);
	}
	prevsw = sw;
    }
    return(FALSE);
}
const char * page::GetSwitcheeName(struct page_switchee  *sw)
{

    if(ATK::IsTypeByName((sw->d)->GetTypeName(),"cel")){
	return ((class cel *) sw->d)->GetRefName();
    }
    return(sw->label);
}
const char * page::GetNowPlayingName()
{
    struct page_switchee *sw;

    sw = this->NowPlaying;
    if(ATK::IsTypeByName((sw->d)->GetTypeName(),"cel")){
	return ((class cel *) sw->d)->GetRefName();
    }
    return(sw->label);
}

boolean page::SetNowPlaying(class dataobject  *d)
{
    struct page_switchee *sw;

    sw = FindSwitchee(this,d,page_BYDATAOBJECT);
    if(sw == NULL) return(FALSE);
    SetSwitchee(this,sw);
    return(TRUE);

}
boolean page::SetNowPlayingByName(char  *name)
{
    struct page_switchee *sw;

    for (sw = this->FirstSwitchee; sw; sw = sw->next) {
	if (strcmp(name,(this)->GetSwitcheeName(sw)) == 0) {
	    SetSwitchee(this,sw);
	    return(TRUE);
	}
    }
    return(FALSE);
}
boolean page::SetNowPlayingByPosition(long  number)
{
    struct page_switchee *sw;
    sw = FindSwitchee(this,NULL,number);
    if(sw == NULL) return(FALSE);
    SetSwitchee(this,sw);
    return(TRUE);
}
class dataobject *page::GetObjectAtPosition(long  number)
{
    struct page_switchee *sw;
    sw = FindSwitchee(this,NULL,number);
    if(sw == NULL) return NULL;
    return sw->d;
}
class dataobject *page::GetObjectByName(char  *name)
{
    struct page_switchee *sw;
    for (sw = this->FirstSwitchee; sw; sw = sw->next) {
	if (strcmp(name,(this)->GetSwitcheeName(sw)) == 0) {
	    return sw->d;
	}
    }
    return NULL;
}
const char  *page::GetNameOfObject(class dataobject  *d)
{
    struct page_switchee *sw;
    sw = FindSwitchee(this,d,page_BYDATAOBJECT);
    if(sw == NULL) return NULL;
    return (this)->GetSwitcheeName(sw);
}
long page::GetPositionOfObject(class dataobject  *d)
{
    struct page_switchee *sw;
    long count = 0L;
    for (sw = this->FirstSwitchee; sw; sw = sw->next) {
	count++;
	if(sw->d == d) return count;
    }
    return 0l;
}
long page::GetObjectCount()
{
    struct page_switchee *sw;
    long count = 0L;
    for (sw = this->FirstSwitchee; sw; sw = sw->next) {
	count++;
    }
    return count;
}
const char *page::ViewName()
{
    return("pagev"); 
}
long page::GetModified()
{
#if 0
    long maxSoFar,x;
    struct page_switchee *sw;

    maxSoFar = (this)->dataobject::GetModified();
    if (this->executingGetModified)
	return 0;
    this->executingGetModified = TRUE;

    for (sw = this->FirstSwitchee; sw; sw = sw->next) {
	if(sw->d){
	    x = (sw->d)->GetModified();
	    if (x > maxSoFar)
		maxSoFar = x;
	}
    }
    this->executingGetModified = FALSE;

    return maxSoFar;
#else 
    return (this)->dataobject::GetModified();
#endif
}
