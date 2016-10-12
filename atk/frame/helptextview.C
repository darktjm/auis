/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#define Text(self) ((class text *) ((self)->GetDataObject()))

#include <andrewos.h>
ATK_IMPL("helptextview.H")
#include <text.H>
#include <message.H>
#include <view.H>
#include <cursor.H>
#include <helptextview.H>


ATKdefineRegistry(helptextview, textview, NULL);
static char *helptextview_getstartstring(class helptextview  *self,char  *buf);


void helptextview::WantInputFocus(class view  *vw)
{   /* ignore requests for input focus */
}
class view *helptextview::Hit(enum view::MouseAction  action, long  x, long  y, long  numberOfClicks)
{
    if (action == view::LeftDown  || action == view::LeftUp) 
	return (this)->textview::Hit( action, x, y, numberOfClicks);
    return NULL;
}
static char *helptextview_getstartstring(class helptextview  *self,char  *buf)
{
    /* finds the starting string for the completion, sticks it in buf, and returns
	a pointer to the end of the string */
    /* currently assumes
      1. The completion string will be found as the first quoted string in the document.
      2. portions of the string following the last '/' are to be discarded.
      */
    /* assumes too much */
    char *bp;
    int i,len;
    class text *doc = Text(self);
    len = (doc)->GetLength();
    for(i = 0;i < len; i++) if((doc)->GetChar(i) == '`') break;
    if(i < len) i++;
    for(i++, bp = buf;i < len && ((*bp = (doc)->GetChar(i)) != '\''); i++)  bp++;
   if(i >= len) { *buf = '\0'; return buf; }
    while (*bp != '/' && bp != buf) bp--;
   if(*bp == '/') bp++;
    *bp = '\0';
    return bp;
}
void helptextview::GetClickPosition(long  position, long  numberOfClicks, enum view::MouseAction  action, long  startLeft, long  startRight, long  *leftPos, long  *rightPos)
{   
    char buf[512], *bp;
    int pos;
    class text *doc = Text(this);
    (this)->textview::GetClickPosition( position,3, action, startLeft, startRight, leftPos, rightPos);
    if (action != view::LeftDown ) return;
    *buf = '\0';
    message::GetCurrentString(this,buf,512) ;
    if(*buf)message::DeleteCharacters(this, 0, strlen(buf));
    bp = helptextview_getstartstring(this,buf);
    /* grab the first unbroken word on the current line */
    for(pos = *leftPos; pos < *rightPos; pos++,bp++) {
	*bp = (doc)->GetChar(pos);
	if(*bp == '\n' || *bp == ' ' || *bp == '\t'){
	    break;
	}
    }
    *bp = '\0';
    message::InsertCharacters(this, 0, buf, bp - buf);
    message::SetCursorPos(this,bp - buf);
}
void helptextview::FullUpdate(enum view::UpdateType  type, long  left, long  top, long  width, long  height)
{
    struct rectangle tr;
    (this)->textview::FullUpdate( type, left, top, width, height);
    (this)->GetVisualBounds(&tr);
    (this)->PostCursor(&tr,this->myCursor);
}
helptextview::helptextview()
{
    this->myCursor = cursor::Create(this);
    (this->myCursor)->SetStandard(Cursor_LeftPointer);
    THROWONFAILURE( TRUE);
}
helptextview::~helptextview()
{
    delete this->myCursor;
}
