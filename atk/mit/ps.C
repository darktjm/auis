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
#include "dataobject.H"
#include "dictionary.H"
#include "ps.H"
#include "text.H"
#define WIDTH 438
#define HEIGHT 244

/* image dimensions in 72 dot/inch pixels */
#define HALF_PAGE_WIDTH 432
#define HALF_PAGE_HEIGHT 324


/****************************************************************/
/*		private functions				*/
/****************************************************************/


ATKdefineRegistry(ps, icon, ps::InitializeClass);
static long check_for_title(class ps  * self, FILE  * file);


static long check_for_title(class ps  * self, FILE  * file)
{
    const char * match = "title{";
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



/****************************************************************/
/*		class procedures				*/
/****************************************************************/
boolean
ps::InitializeClass()
    {
    return TRUE;
}

ps::ps()
{
	ATKinit;

    class text * to;

    to = new text;
    (to)->SetReadOnly(0);
    (this)->SetChild(to);
    (this)->SetTitle("PostScript");
    (this)->SetSize( WIDTH, HEIGHT);
    (this)->SetPixelWidth( HALF_PAGE_WIDTH);
    (this)->SetPixelHeight( HALF_PAGE_HEIGHT);
    THROWONFAILURE( TRUE);
}

/****************************************************************/
/*		instance methods				*/
/****************************************************************/

void
ps::SetChild(class dataobject  * child)
        {
	(this)->icon::SetChild(child);
	if (child != (class dataobject *) 0)
	    ((class text *) child)->SetReadOnly( 0);
}

long
ps::Write(FILE  *file, long  writeID, int  level)
{   
    int  haschild = 0;
    long w, h, pw, ph;
    const char * title = (this)->GetTitle();
    class dataobject *childob = (this)->GetChild();

    if ((this)->GetWriteID() != writeID)  {
	(this)->SetWriteID(writeID);
	if (childob != (class dataobject *)0)
	    haschild = 1;
	(this)->GetSize( &w, &h);
	pw = (this)->GetPixelWidth();
	ph = (this)->GetPixelHeight();
	fprintf(file, "\\begindata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
	fprintf(file, "%ld %ld %d %ld %ld \n", w, h, haschild, pw, ph);

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
	    (childob)->Write(file,
			     this->writeID,
			     2);
	fprintf(file, "\\enddata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
    }

    return (this)->GetID();
}

long
ps::Read(FILE  * file,long  id)
            {
    long x, y, haschild, width, height;
    long objectid;
    int c;
    const char *match;
    char datatype[1024];
    class dataobject * newobject;
    long status;

    newobject = (this)->GetChild();

    if (newobject != (class dataobject *)0)
	/* has the effect of destroying the child */
	(this)->SetChild( (class dataobject *)0);

    (this)->SetID((this)->UniqueID());

    fscanf(file,"%ld %ld %ld %ld %ld", &x, &y, &haschild, &width, &height);
    (this)->SetSize( x, y);
    (this)->SetPixelWidth( width);
    (this)->SetPixelHeight( height);

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
