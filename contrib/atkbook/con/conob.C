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
#include <conob.H>
#include <dataobject.H>
#include <fontdesc.H>

 


ATKdefineRegistry(conob, dataobject, NULL);


conob::conob()
{
    this->numval = 42;
    this->strval = NULL;
    this->displaymin = 0;
    this->displaymax = 100;
    this->Boxed = TRUE;
    this->DisplayTemplate = NULL;
    THROWONFAILURE((TRUE));
}

conob::~conob()
{
    if (this->DisplayTemplate) {
	free(this->DisplayTemplate);
	this->DisplayTemplate = NULL;
    }
}

void conob::GetStringToDisplay(char  *buf, int  len, boolean  IsClick)
{
    const char *templateptr;
    char *end, NumBuf[25];
    const char *str_val, *t;

    if (this->strval == NULL) {
	str_val = "<no value>";
    } else {
	str_val = this->strval;
    }
    end = buf+len-1; /* leave room for null */
    if (this->DisplayTemplate == NULL) {
	templateptr = "$ (*)";
    } else {
	templateptr = this->DisplayTemplate;
    }
    while(*templateptr && (buf < end)) {
	switch (*templateptr) {
	    case '\\':
		/* Force copy of next char */
		*buf++ = *++templateptr;
		break;
	    case '$':
		/* copy over numeric value */
		sprintf(NumBuf, "%ld", this->numval);
		for (t=NumBuf; *t && (buf<end); ++t) {
		    *buf++ = *t;
		}
		break;
	    case '*':
		/* copy over string value */
		for (t=str_val; *t && (buf<end); ++t) {
		    *buf++ = *t;
		}
		break;
	    default:
		*buf++ = *templateptr;
	}
	++templateptr;
    }
    *buf = '\0';
}

long conob::Write(FILE  *fp, long  writeID, int  level)
{
    if ((this)->GetWriteID() != writeID)  {
        (this)->SetWriteID(writeID);
	fprintf(fp, "\\begindata{%s,%ld}\n",
		(this)->GetTypeName(),
		(this)->GetID());
	(this)->WriteState( fp);
	fprintf(fp,
		"\\enddata{%s,%ld}\n",
		(this)->GetTypeName(),
		(this)->GetID());
    }

    return (this)->GetID();
}

void conob::WriteState(FILE  *fp)
{
    fprintf(fp, "$a %d\n$b %d\n$c %d\n", this->displaymin,
	     this->displaymax, this->Boxed);
    if (this->DisplayTemplate) {
	fprintf(fp, "$d %s\n", this->DisplayTemplate);
    }
}

long conob::Read(FILE  *fp, long  id)
{
    char LineBuf[250];

    while (fgets(LineBuf, sizeof(LineBuf)-1, fp) != NULL) {
	if (strncmp(LineBuf, "\\enddata{", 9) == 0) {
	    return(dataobject_NOREADERROR);
	}
	(this)->HandleDataLine( LineBuf);
    }
    return dataobject_PREMATUREEOF;
}

void conob::HandleDataLine(char  *line)
{
    if (*line == '$' && (*(line+2) == ' ')) {
	switch (*(line+1)) {
	    case 'a':
		this->displaymin = atoi(line+3);
		return;
	    case 'b':
		this->displaymax = atoi(line+3);
		return;
	    case 'c':
		this->Boxed = atoi(line+3);
		return;
	    case 'd':
		(this)->SetDisplayTemplate( line+3);
		return;
	}
    }
    fprintf(stderr, "Ignoring unrecognized data: %s\n", line);
}

const char *conob::ViewName()
{
    return("conview"); /* overrides default for subclasses */
}

void conob::SetNumval(long  num)
{
    this->numval = num;
}

void conob::SetStrval(const char  *str)
{
    this->strval = str;
}

void conob::SetDisplayTemplate(const char  *dt)
{
    char *s;

    if (dt == NULL ) {
	if (this->DisplayTemplate != NULL) {
	    free(this->DisplayTemplate);
	}
	this->DisplayTemplate = NULL;
	return;
    }
    if (this->DisplayTemplate == NULL) {
	this->DisplayTemplate = (char *)malloc(1+strlen(dt));
    } else {
	this->DisplayTemplate = (char *)realloc(this->DisplayTemplate,
					1+strlen(dt));
    }
    if (this->DisplayTemplate != NULL) {
	strcpy(this->DisplayTemplate, dt);
	s = index(this->DisplayTemplate, '\n');
	if (s) *s = NULL; /* End templates at newlines */
    }
}

void conob::SetDisplayMin(int  min)
{
    this->displaymin = min;
}

void conob::SetDisplayMax(int  max)
{
    this->displaymax = max;
}

void conob::SetBoxed(boolean  boxed)
{
    this->Boxed = boxed;
}

