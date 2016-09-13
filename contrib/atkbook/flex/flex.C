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
#include <flex.H>
#include <dataobject.H>
#include <message.H>
#include <lpair.H>




ATKdefineRegistry(flex, dataobject, NULL);
static int ReadOneObject(class flex  *self, FILE  *fp, boolean  IsLeft);
static void ResetToInitialState(class flex *self);


flex::flex()
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

flex::~flex()
{
    ResetToInitialState(this);
}

static void ResetToInitialState(class flex *self)
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

void flex::DeleteObjects()
{
    ResetToInitialState(this);
    /* reset everything to initial state */
    this->porf = lpair_PERCENTAGE;
    this->vorh = lpair_VERTICAL;
    this->pct = 50;
    this->movable = 1;
}

void flex::SetDisplayParams(int  porf , int  vorh , int  movable , int  pct)
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

void flex::ToggleParts()
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

boolean flex::InsertObject (class dataobject  *d, const char  *viewname)
{
    class dataobject *d2;
    char *n1, *n2;

    d2 = (class dataobject *) ATK::NewObject("flex");
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

long flex::Write(FILE  *fp ,long  writeid,int  level)
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

long flex::Read(FILE  *fp, long  id)
{
    int status;
    char LineBuf[250];

    if (fgets(LineBuf, sizeof(LineBuf)-1, fp) == NULL) {
	return(dataobject_PREMATUREEOF);
    }
    sscanf(LineBuf, "$ %d %d %d %d\n", &(this->porf),
	    &(this->vorh), &(this->movable), &(this->pct));
    status = ReadOneObject(this, fp, TRUE);
    if (status != dataobject_NOREADERROR) return status;
    status = ReadOneObject(this, fp, FALSE);
    if (status != dataobject_NOREADERROR) return status;
    while (fgets(LineBuf, sizeof(LineBuf)-1, fp) != NULL) {
	if (!strncmp(LineBuf, "\\enddata{flex", 13)) {
	    return dataobject_NOREADERROR;
	}
    }
    return(dataobject_PREMATUREEOF);
}

static int ReadOneObject(class flex  *self, FILE  *fp, boolean  IsLeft)
{
    char LineBuf[250], *s, *obidstr, *thisname;
    int status, obid;
    class dataobject *newob = NULL;

    if (fgets(LineBuf, sizeof(LineBuf)-1, fp) == NULL) {
	return(dataobject_PREMATUREEOF);
    }
    if (strncmp(LineBuf, "\\begindata{", 11) != NULL) {
	if (strncmp(LineBuf, "\\ObjectEmpty", 12) != NULL) {
	    return(dataobject_BADFORMAT);
	}
	if (IsLeft) {
	    self->left = NULL;
	    self->lvname = NULL;
	} else {
	    self->right = NULL;
	    self->rvname = NULL;
	}
	return(dataobject_NOREADERROR);
    }
    thisname = &LineBuf[11];
    obidstr = index(thisname, ',');
    if (!obidstr) return(dataobject_BADFORMAT);
    *obidstr++ = '\0';
    s = index(obidstr, '}');
    if (!s) return(dataobject_BADFORMAT);
    *s = '\0';
    obid = atoi(obidstr);
    if ((newob = (class dataobject *)
	  ATK::NewObject(thisname)))  {
	status = (newob)->Read( fp, obid);
	if (status != dataobject_NOREADERROR) return status;
    } else {
	return(dataobject_OBJECTCREATIONFAILED);
    }
    if (fgets(LineBuf, sizeof(LineBuf)-1, fp) == NULL) {
	return(dataobject_PREMATUREEOF);
    }
    if (strncmp(LineBuf, "\\view{", 6)) {
	return(dataobject_BADFORMAT);
    }
    thisname = &LineBuf[6];
    s = index(thisname, '}');
    if (s) *s = '\0';
    s = strdup(thisname);
    if (!s) return(dataobject_OBJECTCREATIONFAILED);
    if (IsLeft) {
	self->lvname = s;
	self->left = newob;
    } else {
	self->rvname = s;
	self->right = newob;
    }
    return(dataobject_NOREADERROR);
}

