/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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
ATK_IMPL("plink.H")
#include <plink.H>

#include <link.H>




#define MAX_LINE_LENGTH 70


ATKdefineRegistry(plink, link, NULL);
static void WriteLine(FILE  *f, const char  *l);
static char *GlomStrings(char  *s , char  *t);
static char *ReadLine(FILE  *f);


plink::plink()
{
    /* Inititialize the object instance data. */
    this->tag = NULL;
    THROWONFAILURE( TRUE);
}

plink::~plink()
{
    /* Finalize the object instance data. */
    if (this->tag)
	free(this->tag);
    return;
}

void plink::SetTag(char  *newtag)
{
    /* set the tag field */

    if (this->tag) free(this->tag);
    if (newtag) {
	this->tag = strdup(newtag);
    } else {
	this->tag = NULL;
    }
    (this)->SetModified();
    (this)->NotifyObservers( 0);
}

long plink::Write(FILE  *fp, long  wid, int  level)
{
    long unid = (this)->UniqueID();

    if (wid!=(this)->GetWriteID()) {
	/* this dataobj has not been written yet */

	fprintf(fp, "\\begindata{%s,%ld}\n", (this)->GetTypeName(), unid);
	if (this->tag)
	    WriteLine(fp, this->tag);
	else
	    WriteLine(fp, "\\NoTag");
	(this)->link::Write( fp, wid, level);
	fprintf(fp, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), unid);

	(this)->SetWriteID( wid);
    }
    return unid;
}

long plink::Read(FILE  *fp, long  id)
{
    long res;
    char *buf;
    
    buf = ReadLine(fp);
    if (buf==NULL) {
	return dataobject_PREMATUREEOF;
    }

    if (!strcmp(buf, "\\NoTag")) {
	this->tag = NULL;
	free(buf);
    }
    else {
	this->tag = buf;
    }

    buf = ReadLine(fp);
    if (buf==NULL) {
	return dataobject_PREMATUREEOF;
    }
    if (strncmp(buf, "\\begindata", 10)) {
	free(buf);
	return dataobject_BADFORMAT;
    }
    free(buf);

    res = (this)->link::Read( fp, id);
    if (res!=dataobject_NOREADERROR)
	return res;

    buf = ReadLine(fp);
    if (buf==NULL) {
	return dataobject_PREMATUREEOF;
    }
    if (strncmp(buf, "\\enddata", 8)) {
	free(buf);
	return dataobject_MISSINGENDDATAMARKER;
    }
    free(buf);

    return dataobject_NOREADERROR;
}

static void WriteLine(FILE  *f, const char  *l)
{
    /* Output a single line onto the data stream, quoting back slashes and staying within line length limits. Warning:  this routine wasn't meant to handle embedded newlines.*/

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
    /* Safely (allocs more memory) concatenates the two strings, freeing the first.  Meant to build a new string of unknown length. */

    char *r;

    r = (char *)malloc(strlen(s)+strlen(t)+1);
    strcpy(r,s);
    free(s);
    strcat(r,t);
    return r;
}

static char *ReadLine(FILE  *f)
{
    /* Reads from the datastream, attempting to return a single string. Undoes quoting and broken lines. Warning:  this routine wasn't meant to handle embedded newlines.
	Warning:  possible source of memory leaks;  remember to	free the returned string when finished with it! */

    char buf[MAX_LINE_LENGTH], /* (BUG) What if the datastream is broken? */
    buf2[MAX_LINE_LENGTH],
    *result;
    int i,j;


    result = (char *)malloc(1);
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
  return(NULL);
}

