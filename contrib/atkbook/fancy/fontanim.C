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
#include <fontanim.H>
#include <im.H>
#include <stylesheet.H>
#include <style.H>
#include <environment.H>
#include <event.H>



static char *InsertionText = "\n\n\n\nHello, world!\n\nYou are viewing a demonstration of how the Andrew Toolkit can be used to produce interesting and complex multi-font formatted text.\n\nThis should prove useful in understanding how to make fancy text.\n(At least, that's the intent.)\n";


ATKdefineRegistry(fontanim, text, NULL);
void ToggleStyle(class fontanim  *self, long time);


fontanim::fontanim()
{
    class style *styletmp;
    class stylesheet *ss = (this)->GetStyleSheet();

    (this)->InsertCharacters( 0,
		InsertionText, strlen(InsertionText));
    (this)->ReadTemplate( "help", FALSE);
    styletmp = (ss)->Find( "center");
    (this)->SetGlobalStyle( styletmp);
    this->italic = (ss)->Find( "italic");
    this->bold = (ss)->Find( "bold");
    this->currentstyle = this->italic;
    this->myenv = (this)->AddStyle( 4, 5,
			this->currentstyle);
    (this->myenv)->SetStyle( TRUE, TRUE);

    this->myevent = im::EnqueueEvent((event_fptr)ToggleStyle, (char *)this,
				     event_MSECtoTU(1000)); 
    THROWONFAILURE((TRUE));
}

fontanim::~fontanim()
{
    (this->myevent)->Cancel();
}

void ToggleStyle(class fontanim  *self, long time)
{
    if (self->currentstyle == self->italic) {
	self->currentstyle = self->bold;
    } else {
	self->currentstyle = self->italic;
    }
    (self)->SetEnvironmentStyle( self->myenv,
				       self->currentstyle);
    (self)->NotifyObservers(
		observable_OBJECTCHANGED);
    self->myevent = im::EnqueueEvent((event_fptr)ToggleStyle, (char *)self,
				     event_MSECtoTU(1000));
}

char *fontanim::ViewName()
{
    return("textview");
}
