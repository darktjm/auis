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
#include <flexd.H>
#include <dataobject.H>
#include <message.H>
#include <lpair.H>




ATKdefineRegistry(flexd, dataobject, NULL);
static int ReadOneObject(class flexd  *self, FILE  *fp, boolean  IsLeft);
static void ResetToInitialState(class flexd *self);


flexd::flexd()
{
    this->left = NULL;
    this->right = NULL;
    this->lvname = NULL;
    this->rvname = NULL; 
    this->porf = lpair_PERCENTAGE;
    this->vorh = lpair_VERTICAL;
    this->pct = 50;
    this->movable = 1;
    THROWONFAILURE( TRUE);
}

flexd::~flexd()
{
    ResetToInitialState(this);
}

static void ResetToInitialState(class flexd *self)
{
    if (self->lvname) {
	free(self->lvname);
	self->lvname = NULL;
    }
    if (self->rvname) {
	free(self->rvname);
	self->rvname = NULL;
    }
    if (self->left) {
	(self->left)->Destroy();
	self->left = NULL;
    }
    if (self->right) {
	(self->right)->Destroy();
	self->right = NULL;
    }
}

void flexd::DeleteObjects()
{
    ResetToInitialState(this);
    /* reset everything to initial state */
    this->porf = lpair_PERCENTAGE;
    this->vorh = lpair_VERTICAL;
    this->pct = 50;
    this->movable = 1;
}

void flexd::SetDisplayParams(int  porf , int  vorh , int  movable , int  pct)
{
    if ((porf != this->porf) || (vorh != this->vorh)
	 || (movable != this->movable)
	 || (pct != this->pct)) {
	this->porf = porf;
	this->vorh = vorh;
	this->movable = movable;
	this->pct = pct;
    }
}

void flexd::ToggleParts()
{
    char *name;
    class dataobject *d;

    d = this->left;
    this->left = this->right;
    this->right = d;
    name = this->lvname;
    this->lvname = this->rvname;
    this->rvname = name;
}

boolean flexd::InsertObject (class dataobject  *d, const char  *viewname)
{
    class dataobject *d2;
    char *n1, *n2;

    d2 = (class dataobject *) ATK::NewObject("flexd");
    if (d == NULL || d2 == NULL) return(FALSE);
    n1 = strdup(viewname);
    n2 = strdup("flexview");
    if (n1 == NULL || n2 == NULL) return(FALSE);
    this->left = d;
    this->right = d2;
    this->lvname = n1;
    this->rvname = n2;
    return(TRUE);
}

long flexd::Write(FILE  *fp ,long  writeid,int  level)
{
    if ((this)->GetWriteID() != writeid) {
	(this)->SetWriteID( writeid);

	fprintf(fp,"\\begindata{%s,%ld}\n",
		(this)->GetTypeName(),
		(this)->GetID());
	fprintf(fp, "$ %d %d %d %d\n", this->porf,
		this->vorh, this->movable, this->pct);
	if(this->left == NULL){
	    fprintf(fp, "\\ObjectEmpty\n");
	} else {
	    (this->left)->Write(fp,writeid,level+1);
	    fprintf(fp, "\\view{%s}\n", this->lvname);
	}
	if(this->right == NULL){
	    fprintf(fp, "\\ObjectEmpty\n");
	} else {
	    (this->right)->Write(
			     fp, writeid,
			     level+1);
	    fprintf(fp, "\\view{%s}\n", this->rvname);
	}
	fprintf(fp,"\\enddata{%s,%ld}\n",
		(this)->GetTypeName(),
		(this)->GetID());
    }
    return (this)->GetID();
}

long flexd::Read(FILE  *fp, long  id)
{
    int status;
    char LineBuf[250];

    if (fgets(LineBuf, sizeof(LineBuf)-1, fp) == NULL) {
	return(dataobject::PREMATUREEOF);
    }
    sscanf(LineBuf, "$ %d %d %d %d\n", &(this->porf),
	    &(this->vorh), &(this->movable), &(this->pct));
    status = ReadOneObject(this, fp, TRUE);
    if (status != dataobject::NOREADERROR) return status;
    status = ReadOneObject(this, fp, FALSE);
    if (status != dataobject::NOREADERROR) return status;
    while (fgets(LineBuf, sizeof(LineBuf)-1, fp) != NULL) {
	if (!strncmp(LineBuf, "\\enddata{flexd", 13)) {
	    return dataobject::NOREADERROR;
	}
    }
    return(dataobject::PREMATUREEOF);
}

static int ReadOneObject(class flexd  *self, FILE  *fp, boolean  IsLeft)
{
    char LineBuf[250], *s, *obidstr, *thisname;
    int status, obid;
    class dataobject *newob = NULL;

    if (fgets(LineBuf, sizeof(LineBuf)-1, fp) == NULL) {
	return(dataobject::PREMATUREEOF);
    }
    if (strncmp(LineBuf, "\\begindata{", 11) != 0) {
	if (strncmp(LineBuf, "\\ObjectEmpty", 12) != 0) {
	    return(dataobject::BADFORMAT);
	}
	if (IsLeft) {
	    self->left = NULL;
	    self->lvname = NULL;
	} else {
	    self->right = NULL;
	    self->rvname = NULL;
	}
	return(dataobject::NOREADERROR);
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
	if (status != dataobject::NOREADERROR) return status;
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
    s = strdup(thisname);
    if (!s) return(dataobject::OBJECTCREATIONFAILED);
    if (IsLeft) {
	self->lvname = s;
	self->left = newob;
    } else {
	self->rvname = s;
	self->right = newob;
    }
    return(dataobject::NOREADERROR);
}

