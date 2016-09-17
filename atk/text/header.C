/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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
ATK_IMPL("header.H")
#include <text.H>
#include <dictionary.H>
#include <dataobject.H>
#include <environment.H>
#include <fontdesc.H>
#include <style.H>
#include <stylesheet.H>

#include "header.H"

static const char * const header_prompts[] = {
    "  Left\t\t:  ",
    "    Center\t:  ",
    "          Right\t:  "
};

static class style *header_promptStyle;


ATKdefineRegistry(header, dataobject, header::InitializeClass);
void header_SetPrompt(class text  *textobj, const char  *string);
static long header_FencedWrite(class text  *textobj, FILE  *file, long  writeID, int  level);


boolean header::InitializeClass()
{
    if (!(header_promptStyle = new style))
	return FALSE;
    
    (header_promptStyle)->AddNewFontFace( fontdesc_Bold);
    return TRUE;
}



void header::ObservedChanged(class observable  *t,long  value)
{
    if(value==0) (this)->SetModified();
}

const char *header::ViewName()
{
    return "headrtv";
}

void header_SetPrompt(class text  *textobj, const char  *string)
{
    int x, l;
    class environment *newenv;

    l = strlen(string);

    /* Now to change the view */
    x = (textobj)->GetFence();
    (textobj)->ClearFence();
    (textobj)->ReplaceCharacters( 0, x, string, l);
    (textobj)->SetFence( l);

    newenv = (textobj->rootEnvironment)->InsertStyle( 0, header_promptStyle, TRUE);
    (newenv)->SetLength( l - 1);
}	  

/* header_Read: the basic philosophy of this routine is to be completely forward compatible.
  Restrictions:
  In future versions  the first line in the data must always be type:[header|footer] or this routine will leave the object empty.
  Also the texts must always occur in order from left to right.
  */
long header::Read(FILE  *file, long  id)
{

    /* this is take mostly from the template in dataobj.c */

    long endcount = 1;
    boolean begindata;
    char *s;
    long c;
    long status;
    char objectname[200];
    char buf[256];
    long objectid;
    int texts=header_ltext,i;
    class dataobject *newobject;

    (this)->SetID((this)->UniqueID());/* change id to unique number */
    if(!fgets(buf,sizeof(buf),file)) return dataobject_PREMATUREEOF;
    s=strrchr(buf,':');
    if(s) *s++='\0';
    else return dataobject_PREMATUREEOF;
    if(strcmp(buf,"where")) return dataobject_NOREADERROR;
    else {
	if(!strcmp(s,"header\n")) this->where=header_HEADER;
	else if(!strcmp(s,"footer\n")) this->where=header_FOOTER;
    }
    if(!fgets(buf,sizeof(buf),file)) return dataobject_PREMATUREEOF;
    s=strrchr(buf,':');
    if(s) *s++='\0';
    else return dataobject_PREMATUREEOF;
    if(!strcmp(buf,"active")) {
	for(i=header_ltext;*s && i<header_TEXTS;i++,s++) {
	    /* Currently ALWAYS_ACTIVE_MODE is defined in header.ch */
#ifdef ALWAYS_ACTIVE_MODE
	    this->active[i] = TRUE;	    
#else /* ALWAYS_ACTIVE_MODE */
	    this->active[i]=(*s)-'0';
#endif /* ALWAYS_ACTIVE_MODE */
	}
    }
    while (endcount != 0)  {
        while ((c = getc(file)) != EOF && c != '\\')  {
	    if(endcount == 1){
		return dataobject_NOREADERROR;
	    }
        }
        if (c == EOF) return dataobject_NOREADERROR;
        if ((c = getc(file)) == EOF)
            return dataobject_PREMATUREEOF;
	const char *be;
        if (c == 'b')  {
            begindata = TRUE;
            be = "egindata";
        }
        else if (c == 'e')  {
            begindata = FALSE;
            be = "nddata";
        }
        else  {
	    if(endcount == 1){
		/* Place handling of \x characters here */
	    }
            continue;
        }
        while ((c = getc(file)) != EOF && c == *be) be++;
        if (c == '{' && *be == '\0')  {
            if (begindata) {
                s = objectname;
                while ((c = getc(file)) != EOF && c != ',')
                    *s++ = c;
                if (c == EOF) return dataobject_PREMATUREEOF;
                *s = '\0';
		objectid = 0;
		while ((c = getc(file)) != EOF && c != '}')
		    if(c >= '0' && c <= '9')objectid = objectid * 10 + c - '0';
		if (c == EOF) return dataobject_PREMATUREEOF;
		if((c = getc(file))!= '\n') ungetc(c,file);
		/* Call the New routine for the object */
		if(!strcmp(objectname,"text")) {
		    if(texts>=header_TEXTS) continue;
		    status = (this->texts[texts])->Read( file, objectid);
		    if (status != dataobject_NOREADERROR) return status;
		    dictionary::Insert(NULL,(char *)objectid, (char *)this->texts[texts]);
		    header_SetPrompt(this->texts[texts], header_prompts[texts]);
		    (this->texts[texts])->SetObjectInsertionFlag( FALSE);
		    texts++;
		} else {
		    if ((newobject = (class dataobject *) ATK::NewObject(objectname)))  {
			/* Call the read routine for the object */
			status = (newobject)->Read( file, objectid);
			if (status != dataobject_NOREADERROR) return status;
			/* We don't know this object so ignore it */
			(newobject)->Destroy();

		    }
		    else {
			endcount += 1;
			/* return dataobject_OBJECTCREATIONFAILED; */
		    }
		}

	    }
	    else  {
		endcount -= 1;
		while ((c = getc(file)) != EOF && c != '}');
		if((c = getc(file))!= '\n') ungetc(c,file);
            }
        }
        else if(endcount == 1){
	    
        /* 	    Place Handling of characters following \  
           */	}
    }
    return dataobject_NOREADERROR;
}

static long header_FencedWrite(class text  *textobj, FILE  *file, long  writeID, int  level)
{
    int len, pos;

    if (textobj->writeID != writeID)  {
	textobj->writeID = writeID;
	fprintf(file, "\\begindata{%s,%ld}\n", 		
		(textobj->WriteAsText)?"text": (textobj)->GetTypeName(),
		textobj->UniqueID());
	fprintf(file, "\\textdsversion{%d}\n", 12);
	if (textobj->styleSheet->templateName)
	    fprintf(file, "\\template{%s}\n", textobj->styleSheet->templateName);
	(textobj->styleSheet)->Write( file);
	len = (textobj)->GetLength();
	pos = (textobj)->GetFence();
	len = len - pos;
	(textobj)->WriteSubString( pos, len, file, 1);
	fprintf(file, "\\enddata{%s,%ld}\n",
		(textobj->WriteAsText)?"text": (textobj)->GetTypeName(),
		textobj->id);
	fflush(file);
    }
    return textobj->id;
}


#define printbool(x) ((x)?"1":"0")
long header::Write(FILE  *file, long  writeID, int  level)
{
    if ((this)->GetWriteID() != writeID)  {
	(this)->SetWriteID(writeID);
	fprintf(file, "\\begindata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
	fprintf(file,"where:");
	switch(this->where) {
	    case header_FOOTER:
		fprintf(file,"footer\n");
		break;
	    default:
	    case header_HEADER:
		fprintf(file,"header\n");
	}
	fprintf(file,"active:%s%s%s\n", printbool(this->active[header_ltext]), printbool(this->active[header_ctext]), printbool(this->active[header_rtext]));
	header_FencedWrite(this->texts[header_ltext], file,(this)->GetWriteID(),1);
	header_FencedWrite(this->texts[header_ctext], file,(this)->GetWriteID(),1);
	header_FencedWrite(this->texts[header_rtext], file,(this)->GetWriteID(),1);
	fprintf(file, "\\enddata{%s,%ld}\n", (this)->GetTypeName(),(this)->GetID());
    }

    return (this)->GetID();
}

#define HEADERTXTHELPSTRING "Click on 'Left', 'Center', or 'Right' Above"

header::header()
{
	ATKinit;

    int i;
    for(i=header_ltext;i<header_TEXTS;i++) {
	this->texts[i]=new text;
	if(!this->texts[i]) {
	    for(i--;i>=header_ltext;i--) (this->texts[i])->Destroy();
	    THROWONFAILURE( FALSE);
	}
	header_SetPrompt(this->texts[i], header_prompts[i]);
	(this->texts[i])->SetObjectInsertionFlag( FALSE);
	(this->texts[i])->AddObserver(this);
	this->active[i]=TRUE;
    }
    this->where=header_HEADER;
    THROWONFAILURE( TRUE);
}

void header::SetHeader(int  which, const char  *str)
{
    long pos;
    if(which<0 || which>=header_TEXTS) return;
    pos=(this->texts[which])->GetFence();
    if(!str) str="";
    (this->texts[which])->ReplaceCharacters( pos, (this->texts[which])->GetLength()-pos, str, strlen(str));
}

class header *header::Create(int  type, char  *left , char  *center , char  *right)
{
	ATKinit;

    if(type==header_FOOTER || type==header_HEADER) {
	class header *h=new header;
	if(h) {
	    h->where=type;
	    (h)->SetHeader( header_ltext, left);
	    (h)->SetHeader( header_ctext, center);
	    (h)->SetHeader( header_rtext, right);
	    return h;
	}
    }
    return NULL;
}

header::~header()
{
	ATKinit;

    long i;
    for(i=header_ltext;i<header_TEXTS;i++)
	if(this->texts[i]) (this->texts[i])->Destroy();
}
