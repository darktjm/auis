/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("entertext.H")
#include <entertext.H>
#include <stylesheet.H>
#include <style.H>
#include <environment.H>
#include <fontdesc.H>
#define INITIALSIZE 64

ATKdefineRegistry(entertext, text, NULL);
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
void entertext::SetChars(const char  *str,int  len)
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
boolean entertext::InsertCharacters(long  pos, const char  *str, long  len  )
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

const char *entertext::ViewName()
{
    return ("etextview");
}
