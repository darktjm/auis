/* **************************************************** *\
Copyright 1989 Nathaniel S. Borenstein
Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of 
Nathaniel S. Borenstein not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission. 

Nathaniel S. Borenstein DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
Nathaniel S. Borenstein BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
\* ***************************************************** */
#include <andrewos.h>
#include <switcher.H>
#include <dataobject.H>




ATKdefineRegistryNoInit(switcher, dataobject);


switcher::switcher()
{
    this->FirstSwitchee = NULL;
    this->NowPlaying = NULL;
    THROWONFAILURE((TRUE));
}

switcher::~switcher()
{
    struct switchee *sw;
    for (sw = this->FirstSwitchee; sw; sw = sw->next) {
	(sw->d)->Destroy();
	free(sw->label);
	free(sw->viewname);
    }
}

long switcher::Write(FILE  *fp ,long  writeid,int  level)
{
    struct switchee *sw;
    if ((this)->GetWriteID() != writeid) {
	(this)->SetWriteID( writeid);

	fprintf(fp, "\\begindata{switcher,%ld}\n",
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
	fprintf(fp,"\\enddata{switcher,%ld}\n",
		(this)->GetID());
    }
    return (this)->GetID();
}

long switcher::Read(FILE  *fp, long  id)
{
    char LineBuf[250], Label[250], *s, *obidstr, *thisname;
    int status, obid;
    class dataobject *newob = NULL;

    while (TRUE) {
	if (fgets(Label, sizeof(Label)-1, fp) == NULL) {
	    return(dataobject::PREMATUREEOF);
	}
	if (!strncmp(Label, "\\enddata{switcher", 16)) {
	    return dataobject::NOREADERROR;
	}
	if (fgets(LineBuf, sizeof(LineBuf)-1, fp) == NULL) {
	    return(dataobject::PREMATUREEOF);
	}
	if (strncmp(LineBuf, "\\begindata{", 11)) {
	    return(dataobject::BADFORMAT);
	}
	thisname = &LineBuf[11];
	obidstr = index(thisname, ',');
	if (!obidstr) return(dataobject::BADFORMAT);
	*obidstr++ = '\0';
	s = index(obidstr, '}');
	if (!s) return(dataobject::BADFORMAT);
	*s = '\0';
	obid = atoi(obidstr);
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
	thisname = &LineBuf[6];
	s = index(thisname, '}');
	if (s) *s = '\0';
	s = index(Label, '\n');
	if (s) *s = '\0';
	if (Label[0] == '*') {
	    if (!(this)->AddObject( newob,
				    Label+1, thisname)) {
		return(dataobject::OBJECTCREATIONFAILED);
	    }
	    (this)->SetNowPlaying( newob);
	} else if (!(this)->AddObject( newob,
				       Label, thisname)) {
		return(dataobject::OBJECTCREATIONFAILED);
	}
    }
}


boolean switcher::AddObject(class dataobject  *d, const char  *label , const char  *viewname)
{
    struct switchee *sw, *swtmp;

    sw = (struct switchee *) malloc(sizeof(struct switchee));
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

    /* find right place to put it */
    for (swtmp = this->FirstSwitchee;
	  (swtmp != NULL) && swtmp->next;
	  swtmp = swtmp->next) {
	;
    }
    if (swtmp != NULL) {
	swtmp->next = sw;
    } else { /* first one ever */
	this->FirstSwitchee = sw;
    }
    if (this->NowPlaying == NULL) {
	this->NowPlaying = sw;
	(this)->NotifyObservers(
		observable::OBJECTCHANGED);
    }
    return(TRUE);
}

boolean switcher::DeleteObject(class dataobject  *d)
{
    struct switchee *sw, *prevsw;

    for(prevsw = NULL, sw = this->FirstSwitchee;
	 sw != NULL;
	 prevsw = sw, sw = sw->next) {
	if (sw->d == d) {
	    if (prevsw != NULL) {
		prevsw->next = sw->next;
	    } else {
		this->FirstSwitchee = sw->next;
	    }
	    (sw->d)->Destroy();
	    free(sw->label);
	    free(sw->viewname);
	    return(TRUE);
	}
    }
    return(FALSE);
}

boolean switcher::SetNowPlaying(class dataobject  *d)
{
    struct switchee *sw;

    for (sw = this->FirstSwitchee; sw; sw = sw->next) {
	if (sw->d == d) {
	    if (this->NowPlaying == sw) {
		return(TRUE); /* no change */
	    }
	    this->NowPlaying = sw;
	    (this)->NotifyObservers(
			observable::OBJECTCHANGED);
	    return(TRUE);
	}
    }
    return(FALSE);
}

const char *switcher::ViewName()
{
    return("switview"); 
    /* Two cheers for short file names */
}
