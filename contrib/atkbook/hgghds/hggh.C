static char *hggh_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/atkbook/hgghds/RCS/hggh.C,v 1.2 1994/08/11 03:01:34 rr2b Stab74 $";

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
#include <stdio.h>
#include <butt.H>
#include <hggh.H>




ATKdefineRegistry(hggh, dataobject, NULL);


hggh::hggh()
{
    this->b = new butt;
    if (this->b == NULL) THROWONFAILURE((FALSE));
    (this->b)->SetText( "Toggle");
    THROWONFAILURE((TRUE));
}

void hggh::SetButtonText(char  *t)
{
    (this->b)->SetText( t);
    (this->b)->NotifyObservers( observable_OBJECTCHANGED);
}

long hggh::Write(FILE  *fp, long  id, int  level)
{
    long uniqueid = (this)->UniqueID();

    if (id != (this)->GetWriteID()) {
	/* New write operation */
	(this)->SetWriteID( id);
	fprintf(fp,
	  "\\begindata{%s,%ld}\n%s\n\\enddata{%s,%ld}\n",
	  (this)->GetTypeName(), uniqueid,
	  this->b->text ? this->b->text : "",
	  (this)->GetTypeName(), uniqueid);
    }
    return(uniqueid);
}

long hggh::Read(FILE  *fp, long  id)
{
    char LineBuf[250];

    if (fgets(LineBuf,sizeof(LineBuf), fp) == NULL) {
	return(dataobject_PREMATUREEOF);
    }
    /* should check for quoted backslashes first */
    (this)->SetButtonText( LineBuf); 
    /* Now read in the \enddata line */
    if (fgets(LineBuf,sizeof(LineBuf), fp) == NULL) {
	return(dataobject_PREMATUREEOF);
    }
    if (strncmp(LineBuf, "\\enddata", 8)) {
	return(dataobject_MISSINGENDDATAMARKER);
    }
    return(dataobject_NOREADERROR);
}
