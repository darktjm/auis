/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * strinput.c
 *
 * String input dialogue box thingie.
*/

#include <andrewos.h>
ATK_IMPL("strinput.H")
#include <stdio.h>


#include <environ.H>
#include <fontdesc.H>
#include <observable.H>
#include <strinput.H>
#include <style.H>
#include <text.H>
#include <textview.H>
#include <view.H>

#define debug(x) /* printf x ; fflush(stdin); */
static class style *promptStyle = NULL;


ATKdefineRegistry(strinput, view, strinput::InitializeClass);


boolean strinput::InitializeClass()
{
    if (!promptStyle && !(promptStyle = new style))
	return FALSE;
    (promptStyle)->AddNewFontFace( fontdesc_Bold);
    promptStyle->SetName("bold");
    return TRUE;
}


strinput::strinput()
{
	ATKinit;

    this->textobj  = new text;
    this->textv    = new textview;
    this->prompt   = NULL;
    (this->textv)->SetDataObject( this->textobj);
    THROWONFAILURE( TRUE);
}

strinput::~strinput()
{
	ATKinit;

    if (this->textv) (this->textv)->Destroy();
    if (this->textobj) (this->textobj)->Destroy();
    if (this->prompt) free(this->prompt);
}

char *strinput::GetInput(int  length)
{
    int len,pos;
    char *string;

    len = (this->textobj)->GetLength();
    pos = (this->textobj)->GetFence();
    len = len - pos;
    if (length <= 0) return NULL;

    if (length < len) len = length;
    string = (char *) malloc(len + 1);

    (this->textobj)->CopySubString( pos, len, string, TRUE);
    
    return string;
}

void strinput::SetPrompt(char  *string)
{
    int fence, len;

    len = (string ? strlen(string) : 0);
    if (string != this->prompt) {
	if(this->prompt)
	    free(this->prompt);
	this->prompt = (char *) malloc((len+1));
	strcpy(this->prompt, (string ? string : ""));
    }

    /* Now to change the text */
    fence = (this->textobj)->GetFence();
    (this->textobj)->ClearFence();
    (this->textobj)->ReplaceCharacters( 0, fence, string, len);
    (this->textobj)->SetFence( len);
    (this->textobj)->AlwaysAddStyle( 0, len - 1, promptStyle);
    /* Now to change the view, iff it's linked into the viewTree */
    (this->textv)->SetDotPosition( (this->textobj)->GetLength());
    (this)->WantUpdate( this->textv);
}	  

void strinput::ClearInput()
{
    (this->textobj)->Clear();
    (this)->SetPrompt( this->prompt);
    /* Now to change the view, iff it's linked into the viewTree */
    (this->textv)->SetDotPosition( (this->textobj)->GetLength());
}

void strinput::SetInput(char  *string)
{
    (this->textobj)->Clear();
    (this)->SetPrompt( this->prompt);

    (this->textobj)->InsertCharacters( (this->textobj)->GetLength(), 
			   (string ? string : ""), 
			   (string ? strlen(string) : 0));
    /* Now to change the view, iff it's linked into the viewTree */
    (this->textv)->SetDotPosition( (this->textobj)->GetLength());
}

void strinput::FullUpdate(enum view::UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle child;

    (this)->GetLogicalBounds( &child);
    (this->textv)->InsertView( this, &child);
    (this->textv)->FullUpdate( type, 0, 0, child.width, child.height);
}

class view *strinput::Hit(enum view::MouseAction  action, long  x , long  y , long  clicks)
{
    return (this->textv)->Hit( action, x, y, clicks);
}

boolean strinput::HaveYouGotTheFocus()
{
    return(this->textv->hasInputFocus);
}

void strinput::ReceiveInputFocus()
{
    (this)->view::ReceiveInputFocus();
    (this)->WantInputFocus( this->textv);
}

void
strinput::LinkTree( class view  *parent )
{
    (this)->view::LinkTree( parent);
    if(this->textv)
	(this->textv)->LinkTree( this);
}
