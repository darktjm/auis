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

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/cursor.C,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 


ATK_IMPL("cursor.H")
#include <fontdesc.H>
#include <graphic.H>
#include <view.H>
#include <im.H>

#include <cursor.H>


static char CursorTable[] = {  'a',
    'a','g','x','H','h','l','u','v','w','|',
    'J',':','C','R','M',';','/','^','s','U',
    'D','@','X','~','}' };
#define lookup(A) ((A >= 0 && A < Cursor_TABLELEN)?  CursorTable[A] :CursorTable[0])




ATKdefineRegistry(cursor, ATK, NULL);
#ifndef NORCSID
#endif


void cursor::ChangeShape()
{
    printf("cursor_ChangeShape: missing method\n");
}


void cursor::SetGlyph(class fontdesc  *fd,short  ch)
{
     if (this->fillFont != fd || this->fillChar != ch)  {
	this->fillFont = fd;
	this->fillChar = ch;
	(this)->ChangeShape();
     }
}

void cursor::SetStandard(short  ch)
{
    short c = lookup(ch);

     if (this->fillFont != NULL || this->fillChar != c)  {
	this->fillFont = NULL;
	this->fillChar = c;
	(this)->ChangeShape();
    }
}

cursor::cursor()
{
	this->view = NULL;
	this->posted = NULL;
	this->windowim = NULL;
	this->next = NULL;
	this->processC = FALSE;
	this->changed = FALSE;
        this->fillChar = 0;
        this->fillFont = NULL;
	THROWONFAILURE( TRUE);
}

class cursor *cursor::Create(class view  *view)
{

	class cursor *c = im::GetCursor();
	c->view = view;
	return(c);
}

cursor::~cursor()
{
	if((this)->IsPosted()) (this->posted)->RetractCursor(this);
	if((this)->IsWindowCursor()) (this->windowim)->SetWindowCursor(NULL);
	if((this)->IsProcessCursor()) im::SetProcessCursor(NULL);
}
