/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#include <andrewos.h>
ATK_IMPL("mentertext.H")

#include <stylesheet.H>
#include <style.H>
#include <environment.H>
#include <fontdesc.H>
#include <mentertext.H>
#define INITIALSIZE 64

#define Root(self) (((class text *)self)->rootEnvironment)
#define MyEnvinfo(text,pos) (Root(this))->GetInnerMost( pos)
#define Enclosed(text,pos) Root(this) != MyEnvinfo(text,pos)


ATKdefineRegistry(mentertext, text, NULL);
static void checkstyles(class mentertext  *self);
int mentertext_GetBufP(class mentertext  *self,char  *buf);


mentertext::mentertext()
{
    if((this->buf = (char *)malloc(INITIALSIZE)) == NULL)THROWONFAILURE( FALSE);
    this->buflenval = 0;
    this->realbuflen = INITIALSIZE;
    *(this->buf) = '\0';
    this->mod = -1;
    this->needswrap = TRUE;
    this->Style = NULL;
    this->src = NULL;
    this->bufp[0] = "";
    THROWONFAILURE( TRUE);
}
static void checkstyles(class mentertext  *self)
{

    if ((self->Style = (self->styleSheet)->Find( "italic")) == NULL){
	self->Style = new style;
	(self->Style)->SetName( "bold");
	(self->styleSheet)->Add( self->Style);
	(self->Style)->AddNewFontFace( fontdesc_Bold);
    }
}
int mentertext_GetBufP(class mentertext  *self,char  *buf)
{
    long i,fin,elen,count;
    class environment *newenv;
    boolean foundlast = FALSE;
    char *c;
    count = i = 0;
    fin = (self)->GetLength();
    while(i < fin && count < 127){ 
	newenv = (class environment *)(Root(self))->GetInnerMost( i);
	elen = (Root(self))->GetNextChange( i);
	if(elen + i > fin) elen = (fin  /* +1 */)- i;
	if(newenv == Root(self)){
	    foundlast = TRUE;
	    self->bufp[count++] = buf;
	    (self)->CopySubString(i,elen,buf,FALSE);
	    if((c = strchr(buf,'\n')) != NULL){
		*c = '\0';
		buf = ++c;
	    }
	    else {
		buf += elen ;
		*buf++ = '\0';
	    }
	}
	else{
	    foundlast = FALSE;
	}
	i += elen;
    }
    if(!foundlast) self->bufp[count++] = "";
    return count;
}
mentertext::~mentertext()
{
    free(this->buf);
}
void mentertext::updatebuf()
{
    long len = (this)->GetLength() + 1;
    if(this->realbuflen < len){
	this->buf = (char *)realloc(this->buf,len);
	this->realbuflen = len;
    }
    this->buflenval = len;
    this->mod = (this)->GetModified();
    len--;
    if(len > 0) {
	this->bufpcount = mentertext_GetBufP(this,this->buf);
	(this)->NotifyObservers(mentertext_BUFCHANGEDFLAG);
    }
}
boolean mentertext::Changed()
{
    return (boolean)(this->mod != (this)->GetModified());
}
void mentertext::SetChars(const char  *str,int  len)
{
    this->src = str;
    this->bufpcount = 0;
    (this)->Clear();
    if(len && str && *str){
	long last , i;
	class environment *te = NULL;
	boolean founddef;
	(this)->InsertCharacters(0,str,len);
	checkstyles(this);
	founddef = FALSE;
	for(i = 0 ,last = 0; i < len; i++,str++){
	    if(*str == '@' || *str == '\n'){
		if(!founddef){
		    te = (Root(this))->InsertStyle( last, this->Style, TRUE);
		    (te)->SetStyle( TRUE, FALSE);
		    (te)->SetLength( i - last);
		}
		if(*str == '@'){
		    if(!founddef){
			(this)->text::DeleteCharacters( i, 1);
			(this)->text::InsertCharacters( i, " ",1);
			(te)->SetLength( i + 1 - last);
		    }
		    founddef = TRUE;
		}
		else founddef = FALSE;
		last = i + 1;
	    }
	}
	(this)->text::DeleteCharacters(len - 1,1);
    }
    (this)->NotifyObservers(0);
}

boolean mentertext::InsertCharacters(long  pos, const char  *str, long  len  )
{
    if(Enclosed(this,pos)){
/*
    long n;
	if( (n = environment_GetNextChange(Root(self), pos)) >  100000) return;
	pos += n;
*/
	return FALSE; /* Is this the right return value? --ghoti */
    }
    if(!(this)->text::InsertCharacters( pos, str, len)) return FALSE;
    return TRUE;
}
boolean mentertext::DeleteCharacters(long  pos, long  len  )
{
    long n;
    if(Enclosed(this,pos) || Enclosed(this,pos + 1)) return FALSE;
    if( (n = (Root(this))->GetNextChange( pos)) > len + pos && n < 100000) return FALSE; /* Is this the right return value? --ghoti */
    if(!(this)->text::DeleteCharacters( pos, len)) return FALSE;
    return TRUE;
}
void mentertext::ClearLine(long  pos)
{
    long i,lasti;
    long len = (this)->GetLength();
    for(i = 0 ,lasti = 0; (i = (this)->Index(i,'\n',len)) != EOF; lasti =i){
	if(i >= pos) break;
    }
    i++;
    if(Enclosed(this,lasti)){
	lasti += (Root(this))->GetNextChange( lasti);
    }
    if(lasti < i)
	(this)->DeleteCharacters( i, lasti - i);
}


const char *mentertext::ViewName()
{
    return ("etextview");
}
