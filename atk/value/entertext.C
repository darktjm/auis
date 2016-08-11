/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/value/RCS/entertext.C,v 1.4 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 


#include <andrewos.h>
ATK_IMPL("entertext.H")
#include <entertext.H>
#include <stylesheet.H>
#include <style.H>
#include <environment.H>
#include <fontdesc.H>
#define INITIALSIZE 64

ATKdefineRegistry(entertext, text, NULL);
#ifndef NORCSID
#endif
static void checkstyles(class entertext  *self);


entertext::entertext()
{
    if((this->buf = (char *)malloc(INITIALSIZE)) == NULL)THROWONFAILURE( FALSE);
    this->buflenval = 0;
    this->realbuflen = INITIALSIZE;
    *(this->buf) = '\0';
    this->mod = -1;
    this->needswrap = TRUE;
    this->Style = NULL;
    THROWONFAILURE( TRUE);
}
entertext::~entertext()
{
    free(this->buf);
}
void entertext::updatebuf()
{
    long len = (this)->GetLength() + 1;
    this->needswrap = FALSE;
    if(this->realbuflen < len){
	this->buf = (char *) realloc(this->buf,len);
	this->realbuflen = len;
    }
    this->buflenval = len;
    this->mod = (this)->GetModified();
    len--;
    if(len > 0) (this)->CopySubString(0,len,this->buf,FALSE);
    this->buf[len] = '\0';
    (this)->NotifyObservers(entertext_BUFCHANGEDFLAG);
    if( (this)->GetGlobalStyle() != NULL)
	(this)->SetGlobalStyle( NULL);
    this->needswrap = TRUE;
}
boolean entertext::Changed()
{
    return (boolean)(this->mod != (this)->GetModified());
}
void entertext::SetChars(char  *str,int  len)
{
    this->needswrap = FALSE;
    (this)->Clear();
    if(len && str && *str) (this)->InsertCharacters(0,str,len);
    this->needswrap = TRUE;
}
static void checkstyles(class entertext  *self)
{
    
    if(self->needswrap && (self)->GetGlobalStyle() == NULL){
	if ((self->Style = (self->styleSheet)->Find( "italic")) == NULL){
		self->Style = new style;
		(self->Style)->SetName( "italic");
		(self->styleSheet)->Add( self->Style);
		(self->Style)->AddNewFontFace( fontdesc_Italic);
	    }
	(self)->SetGlobalStyle( self->Style);
    }
}
boolean entertext::InsertCharacters(long  pos, char  *str, long  len  )
{
    if(!(this)->text::InsertCharacters( pos, str, len)) return FALSE;
    checkstyles(this);
    return TRUE;
}
boolean entertext::DeleteCharacters(long  pos, long  len  )
{
    if(!(this)->text::DeleteCharacters( pos, len)) return FALSE;
    if((this)->GetLength() > 0)
	checkstyles(this);
    return TRUE;
}

char *entertext::ViewName()
{
    return ("etextview");
}