/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/mit/RCS/icon.C,v 1.2 1993/05/30 14:47:01 rr2b Stab74 $";
#endif


 

#include "dataobject.H"
#include "dictionary.H"
#include "icon.H"


ATKdefineRegistry(icon, dataobject, icon::InitializeClass);
#ifndef NORCSID
#endif
static long check_for_title(class icon  * self, FILE  * file);


boolean
icon::InitializeClass()
    {
    return TRUE;
}


icon::icon()
        {
	ATKinit;

    this->child = (class dataobject *)0;
    this->width = 200;
    this->height = 100;
    this->title = "";
    THROWONFAILURE( TRUE);
}




void
icon::SetSize(long  x,long  y)
            {
    if ((this->width != x) || (this->height != y)) {
	this->width = x;
	this->height = y;
	(this)->NotifyObservers( icon_SizeChanged);
    }
}


void
icon::GetSize(long  * x,long  * y)
            {
    *x = this->width;
    *y = this->height;
}


void
icon::SetChild(class dataobject  * dobj)
        {
    if (this->child != (class dataobject *)0)
	(this->child)->Destroy();
    this->child = dobj;
    (this)->NotifyObservers( icon_ChildChanged);
}

class dataobject *
icon::GetChild()
    {
    return this->child;
}

void
icon::SetTitle(const char  * title)
{
    this->title = strdup(title); /* leak */
    (this)->NotifyObservers( icon_TitleChanged);
}   

const char *
icon::GetTitle()
{
    return this->title;
}


long
icon::Write(FILE  *file, long  writeID, int  level)
{   
    int  haschild = 0;
    const char * title = this->title;
    if ((this)->GetWriteID() != writeID)  {
	(this)->SetWriteID(writeID);
	if (this->child != (class dataobject *)0)
	    haschild = 1;
	fprintf(file, "\\begindata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
	fprintf(file, "%ld %ld %d \n",this->width,this->height, haschild);

	fprintf(file,"\\title{");
	while (*title != '\0') {
	    if (*title == '\\')
		fputs("\\\\", file);
	    else if (*title == '}')
		fputs("\\}", file);
	    else
		fputc(*title, file);
	    title++;
	}
	fputs("}\n",file);

	if (haschild) 
	    (this->child)->Write(file,
			     this->writeID,
			     2);
	fprintf(file, "\\enddata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
    }

    return (this)->GetID();
}

static long check_for_title(class icon  * self, FILE  * file)
{
    char * match = "title{";
    char title[1024];
    int c;
    int x;

    while (*match != '\0') {
	c = fgetc(file);
	if (c == EOF) 
	    return dataobject_PREMATUREEOF;
	if (c != *match){
	    ungetc(c,file); 
	    (self)->SetTitle("");
	    return dataobject_NOREADERROR;
	}
	++match;
    }

    if((c = fgetc(file))!= '\n')
	ungetc(c,file);
    x = 0;
    while ((c = fgetc(file)) != '}') {
	if (c == EOF)
	    return dataobject_PREMATUREEOF;
	if (c == '\\')
	    if ((c = fgetc(file)) != EOF)
		title[x] = c;
	    else return dataobject_PREMATUREEOF;
	else
	    title[x] = c;
	if (++x == 1024)
	    return dataobject_BADFORMAT;
    }
    title[x] = '\0';
    (self)->SetTitle(title);

    while ((c = fgetc(file)) != '\\')
	if (c == EOF)
	    return dataobject_PREMATUREEOF;
    return dataobject_NOREADERROR;
}

long
icon::Read(FILE  * file,long  id)
            {
    long x,y,haschild;
    long objectid;
    int c;
    char * match;
    char datatype[1024];
    class dataobject * newobject;
    long status;

    if (this->child != (class dataobject *)0)
	(this->child)->Destroy();
    this->child = (class dataobject *)0;
    (this)->SetID((this)->UniqueID());

    fscanf(file,"%ld %ld %ld", &x, &y, &haschild);
    this->width = x;
    this->height = y;

    while ((c = fgetc(file)) != '\\')
	if (c == EOF)
	    return dataobject_PREMATUREEOF;

    if (check_for_title(this, file) != dataobject_NOREADERROR)
	return dataobject_BADFORMAT;

    newobject = (class dataobject *)0;
    if (haschild) {
	match = "begindata{";
	while (*match != '\0') {
	    c = fgetc(file);
	    if (c == EOF) 
		return dataobject_PREMATUREEOF;
	    if (c != *match)
		return dataobject_BADFORMAT;
	    ++match;
	}
	if((c = fgetc(file))!= '\n')
	    ungetc(c,file);
	x = 0;
	while ((c = fgetc(file)) != ',') {
	    if (c == EOF)
		return dataobject_PREMATUREEOF;
	    datatype[x] = c;
	    if (++x == 1024)
		return dataobject_BADFORMAT;
	}
	datatype[x] = '\0';
	objectid = 0;
	while ((c = fgetc(file)) != '}') {
	    if (c == EOF)
		return dataobject_PREMATUREEOF;
	    if(c >= '0'	&& c <=	'9')
		objectid = objectid * 10 + c - '0';
	}

	if((c = getc(file))!= '\n')
	    ungetc(c,file);

	newobject = (class dataobject *)ATK::NewObject(datatype);
	if (newobject == (class dataobject *)0)
	    return dataobject_OBJECTCREATIONFAILED;
	dictionary::Insert(NULL,(char *)objectid, (char *)newobject);
	status = (newobject)->Read( file, objectid);
	if (status != dataobject_NOREADERROR) 
	    return status;
    }
    /* adapt a cavalier attitude towards enddata */
    while ((c = fgetc(file)) != '\n')
	if (c == EOF)
	    break;

    (this)->SetChild(newobject);  /* might be null */

    return dataobject_NOREADERROR;
}

