/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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

#include <andrewos.h>
ATK_IMPL("hlptextview.H")
#include <ctype.h>


#include <style.H>
#include <environment.H>
#include <text.H>
#include <hlptextview.H>


ATKdefineRegistry(hlptextview, textview, hlptextview::InitializeClass);
static char *TrimWhiteSpace(char  *buf, int  pos);


boolean hlptextview::InitializeClass()
{
    return TRUE;
}

hlptextview::hlptextview()
{
	ATKinit;

    (this)->SetHyperlinkCheck( FALSE);
    THROWONFAILURE( TRUE);
}

hlptextview::~hlptextview()
{
	ATKinit;

    return;
}

/* given a buffer and a position, return the largest string containing that position which includes no whitespace. The buffer may be mutilated as a side effect.
If the character at pos and pos+1 are both whitespace, return NULL. */
static char *TrimWhiteSpace(char  *buf, int  pos)
{
    int val;

    /* set buf to the start of the desired string */
    val = pos;
    while (val > 0 && (buf[val]=='\0' || isspace(buf[val]))) {
	if (buf[val] == '\n')  {
	    return NULL;
	}
	val--;
    }

    if (isspace(buf[val])) {
	return NULL;
    }

    while (val > 0 && !isspace(buf[val-1])) {
	val--;
    }

    buf = (buf+val);
    val = 0;
    while (buf[val] && !isspace(buf[val]))
	val++;
    buf[val] = '\0';
    return buf;
}

/* override */
void hlptextview::GetClickPosition(long  position , long  numberOfClicks, enum view_MouseAction  action, long  startLeft , long  startRight , long  *leftPos , long  *rightPos)
{
    (this)->textview::GetClickPosition( position, numberOfClicks, action, startLeft, startRight, leftPos, rightPos);
    if ((this)->GetHyperlinkCheck()) {
	/* Check for a hypertext style.  For now, we check for a style named
	 * 'helptopic', although we should create a new attribute called
	 * 'hyperlink' and look for any style with that attribute.
	 */
	class environment *env;
	class style *style;
	char *stylename;
	char *buf;
	class text *text;
	long textstart, textlen;
	static char topic[200];

	env = (this)->GetEnclosedStyleInformation( position, &textlen);
	while (env) {
	    style = env->data.style;
	    if (style) {
		stylename = style->name;
		if (stylename && strcmp(stylename, "helptopic") == 0) {
		    textstart = (env)->Eval();
		    textlen = (env)->GetLength();
		    text = (class text *)(this)->GetDataObject();
		    if (text) {
			buf = (text)->GetBuf( textstart, textlen, &textlen);
			if (buf && textlen < (int)sizeof(topic)) {
			    strncpy(topic, buf, textlen);
			    topic[textlen] = '\0';
			    this->hyperTopic = TrimWhiteSpace(topic, position-textstart);
			    this->action = action;
			    return;
			}
		    }
		}
	    }
	    env = (class environment *) env->parent;
	}
    }
    this->hyperTopic = NULL;	/* no topic hit */
}
