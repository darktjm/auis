/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include "dataobject.H"
#include "dictionary.H"
#include "icon.H"


ATKdefineRegistry(icon, dataobject, icon::InitializeClass);
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
    this->title = strdup("");
    THROWONFAILURE( TRUE);
}


icon::~icon()
{
    free(this->title);
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
    free(this->title);
    this->title = strdup(title);
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
			     this->GetWriteID(),
			     2);
	fprintf(file, "\\enddata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
    }

    return (this)->GetID();
}

static long check_for_title(class icon  * self, FILE  * file)
{
    const char * match = "title{";
    char title[1024];
    int c;
    int x;

    while (*match != '\0') {
	c = fgetc(file);
	if (c == EOF) 
	    return dataobject::PREMATUREEOF;
	if (c != *match){
	    ungetc(c,file); 
	    (self)->SetTitle("");
	    return dataobject::NOREADERROR;
	}
	++match;
    }

    if((c = fgetc(file))!= '\n')
	ungetc(c,file);
    x = 0;
    while ((c = fgetc(file)) != '}') {
	if (c == EOF)
	    return dataobject::PREMATUREEOF;
	if (c == '\\')
	    if ((c = fgetc(file)) != EOF)
		title[x] = c;
	    else return dataobject::PREMATUREEOF;
	else
	    title[x] = c;
	if (++x == 1024)
	    return dataobject::BADFORMAT;
    }
    title[x] = '\0';
    (self)->SetTitle(title);

    while ((c = fgetc(file)) != '\\')
	if (c == EOF)
	    return dataobject::PREMATUREEOF;
    return dataobject::NOREADERROR;
}

long
icon::Read(FILE  * file,long  id)
            {
    long x,y,haschild;
    long objectid;
    int c;
    const char * match;
    char datatype[1024];
    class dataobject * newobject;
    long status;

    if (this->child != (class dataobject *)0)
	(this->child)->Destroy();
    this->child = (class dataobject *)0;

    fscanf(file,"%ld %ld %ld", &x, &y, &haschild);
    this->width = x;
    this->height = y;

    while ((c = fgetc(file)) != '\\')
	if (c == EOF)
	    return dataobject::PREMATUREEOF;

    if (check_for_title(this, file) != dataobject::NOREADERROR)
	return dataobject::BADFORMAT;

    newobject = (class dataobject *)0;
    if (haschild) {
	match = "begindata{";
	while (*match != '\0') {
	    c = fgetc(file);
	    if (c == EOF) 
		return dataobject::PREMATUREEOF;
	    if (c != *match)
		return dataobject::BADFORMAT;
	    ++match;
	}
	if((c = fgetc(file))!= '\n')
	    ungetc(c,file);
	x = 0;
	while ((c = fgetc(file)) != ',') {
	    if (c == EOF)
		return dataobject::PREMATUREEOF;
	    datatype[x] = c;
	    if (++x == 1024)
		return dataobject::BADFORMAT;
	}
	datatype[x] = '\0';
	objectid = 0;
	while ((c = fgetc(file)) != '}') {
	    if (c == EOF)
		return dataobject::PREMATUREEOF;
	    if(c >= '0'	&& c <=	'9')
		objectid = objectid * 10 + c - '0';
	}

	if((c = getc(file))!= '\n')
	    ungetc(c,file);

	newobject = (class dataobject *)ATK::NewObject(datatype);
	if (newobject == (class dataobject *)0)
	    return dataobject::OBJECTCREATIONFAILED;
	dictionary::Insert(NULL,(char *)objectid, (char *)newobject);
	status = (newobject)->Read( file, objectid);
	if (status != dataobject::NOREADERROR) 
	    return status;
    }
    /* adapt a cavalier attitude towards enddata */
    while ((c = fgetc(file)) != '\n')
	if (c == EOF)
	    break;

    (this)->SetChild(newobject);  /* might be null */

    return dataobject::NOREADERROR;
}

