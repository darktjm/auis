/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
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
