/* ********************************************************************** *\
 *     Copyright Carnegie Mellon University 1993 - All Rights Reserved    *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
#include <andrewos.h>
#include <gbuf.H>



ATKdefineRegistry(gbuf, ATK, gbuf::InitializeClass);

static void CheckHolds(class gbuf  *self, const char  *where);

#ifdef TestingOnlyTesting
void main();
#endif


static void CheckHolds(class gbuf  *self, const char  *where)
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
char *gbuf::GetBuffer(long  pos, long  len, long  *gotlen)
{
    this->holds++;
    *gotlen = this->curlen - pos;
    return this->buf + pos;
}

/* gbuf__Insert(self, pos, txt, txtlen)
	inserts txt (its length is txtlen) into self->buf at pos
*/
void gbuf::Insert(long  pos, char  *txt, long  txtlen)
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
void gbuf::Delete(long  pos, long  len)
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
	long tx = pos + len;
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
char gbuf::AppendCharacter(char  c)
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
