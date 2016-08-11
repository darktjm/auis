/* ********************************************************************** *\
 *         Copyright Carnegie Mellon University 1993 - All Rights Reserved 
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
#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/gbuf.C,v 1.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif
#include <andrewos.h>

#include <gbuf.H>



ATKdefineRegistry(gbuf, ATK, gbuf::InitializeClass);

static void CheckHolds(register class gbuf  *self, register char  *where);

#ifdef TestingOnlyTesting
void main();
#endif


static void CheckHolds(register class gbuf  *self, register char  *where)
{
    if (self->holds > 0)
	fprintf(stderr, 
		"*** gbuf_%s: incorrect between %s\n",
		where, "GetBuffer and ReleaseBuffer");
}

boolean gbuf::InitializeClass()
{
    return TRUE;
}

gbuf::gbuf()
{
    ATKinit;

    this->size = 200;
    this->curlen = 0;
    this->holds = 0;
    this->buf = (char *)malloc(this->size);
    THROWONFAILURE( (this->buf != NULL));
}

gbuf::~gbuf()
{
    ATKinit;

    if (this->buf)
	free(this->buf);
}


/* gbuf__GetBuffer(self, pos, len, gotlen)
	Returns a pointer to the portion of the buffer 
	starting at pos and trying to extend for len
	length actually gotten is returned in gotlen
*/
char *gbuf::GetBuffer(register long  pos, register long  len, register long  *gotlen)
{
    this->holds++;
    *gotlen = this->curlen - pos;
    return this->buf + pos;
}

/* gbuf__Insert(self, pos, txt, txtlen)
	inserts txt (its length is txtlen) into self->buf at pos
*/
void gbuf::Insert(register long  pos, register char  *txt, register long  txtlen)
{
    char *tbuf = NULL;

    if (txtlen < 0) return;
    if (pos < 0) pos = 0;
    if (pos > this->curlen)  pos = this->curlen;
    if (pos == this->curlen && txtlen <= this->size - this->curlen) {
	memmove(this->buf+pos, txt, txtlen);
	this->curlen += txtlen;
	return;
    }
    CheckHolds(this, "Insert");

    /* copy text in case of overlap */
    if (txt+txtlen >= this->buf+pos && txt < this->buf+this->curlen) {
	tbuf = (char *)malloc(txtlen);
	memcpy(tbuf, txt, txtlen);
    }

    /* be sure space is big enough */
    if (txtlen > this->size - this->curlen) {
	/* need to make buffer bigger */
	this->size = txtlen + this->curlen;
	if (this->size < 1000) this->size = 2000;
	else this->size += this->size/2;
	this->buf = (char *)realloc(this->buf, this->size);
	if (this->buf == NULL) {
	    fprintf(stderr, 
		    "*** gbuf_Insert: buffer size failure");
	    return;
	}
    }

    /* make gap for the insertion */
    if (pos != this->curlen)
	memmove(this->buf+pos+txtlen, this->buf+pos, txtlen);

    /* now insert txt into gap */
    if (tbuf) {
	memcpy(this->buf+pos, tbuf, txtlen);
	free(tbuf);
    }
    else
	memmove(this->buf+pos, txt, txtlen);
}

/* gbuf__Delete(self, pos, len)
	deletes the len characters starting at pos from self->buf
*/
void gbuf::Delete(register long  pos, register long  len)
{
    CheckHolds(this, "Delete");
    if (len < 0) {
	/* assume delete backward from pos */
	len = -len;
	pos -= len;
    }
    if (pos < 0) {
	/* ignore portion before 0 */
	len -= -pos;
	pos = 0;
    }
    if (len <= 0 || pos > this->curlen) return;
    if (pos+len > this->curlen)
	len = this->curlen - pos;
    if (pos+len < this->curlen) {
	register long tx = pos + len;
	memmove(this->buf+pos, this->buf+tx, this->curlen - tx);
    }
    this->curlen -= len;

    /* XXX should decide whether to reduce space */
}

/* gbuf__AppendCharacter(self, c)
	appends c to the end of self->buf.
	Causes error message if there is an unreleased getbuffer.
	Usually gbuf_AppendChar is preferable 
	Returns the character
*/
char gbuf::AppendCharacter(register char  c)
{
    char ct = c;
    (this)->Insert( this->curlen, &ct, 1);
    return ct;
}


#ifdef TestingOnlyTesting
void main()
{
    class gbuf *gb;
    char *tb;
    long len;

    ATK_Init(".");
    gb = new gbuf;
    (gb)->Insert( 0, "this is a test", 14);
    (gb)->Delete( 8, 5);
    (gb)->Replace( 5, 3, "was");
    (gb)->AppendChar( ' ');
    (gb)->AppendChar( 's');
    (gb)->AppendChar( 't');
    (gb)->Insert( (gb)->Len()-2, "a te", 4);
    tb = (gb)->GetBuffer( 0, 10000, &len);
    printf("length %d   \"%s\"\n", len, tb);
}
#endif
