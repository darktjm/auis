/* Copyright 1992 Carnegie Mellon University All rights reserved.
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


#include <pintv.H>
#include <im.H>
#include <event.H>
#include <buffer.H>
#include <text.H>
#include <frame.H>
#include <textview.H>
#include <cursor.H>
#include <prefs.H>
#include <completion.H>

#include <pintv.h>
#include <util.h>
#include <errno.h>

#define TEXT(tv) ((class text *)(tv)->GetDataObject())
#define PREFS(pv) ((class prefs *)(pv)->GetDataObject())
#define RFOLDEDEQ(x,y) ((x)==(y))


static char explanation[]="In the following cases a general preference of the form \"*.prefname:...\" or \"prefname:...\" was found to precede a preference of the form \"appname.prefname:...\"\nIn cases like this the general preference overrides the specific preference, which probably isn't what was intended.  If you save your preferences they will be re-ordered so that this conflict does not occur.  This may cause a CHANGE in behavior.\n\n";


static void ReportErrors(class pintv  *self, long  curtime);
class event *pintv_GetReportEvent(long  rock);
static void KeepOldCopy(class pintv  *self, long  curtime);
class event *pintv_GetKeepEvent(long  rock);
static boolean FindTextBuffer(class buffer  *b, class prefs  *data);
void pintv_EditAsText(class pintv  *self, class sbutton  *sb, long  rock);


static void ReportErrors(class pintv  *self, long  curtime)
{
    class buffer *b;
    class frame *f;
    char bufname[256];

    self->reportevent=NULL;
    
    if(self->errors==NULL) {
	fprintf(stderr, "Preferences Warning: errors object disappeared before errors could be reported.\n");
	return;
    }
    buffer::GetUniqueBufferName("Preferences Errors", bufname, sizeof(bufname));
    (self->errors)->AlwaysInsertCharacters( 0, explanation, sizeof(explanation)-1);
    b=buffer::Create(bufname, NULL, NULL, self->errors);
    if(b==NULL) {
	fprintf(stderr, "Preferences Warning: error buffer could not be created.\n");
	return;
    }
    (b)->SetDestroyData(TRUE);
    self->errors->RemoveObserver(self);
    self->errors=NULL;
    f=frame::GetFrameInWindowForBuffer(b);
    if(f==NULL) {
	fprintf(stderr, "Preferences Warning: couldn't get a window for the error buffer %s\n", bufname);
	fprintf(stderr, "Use switch buffer to examine the error list.\n");
    }
    ((class textview *)(f)->GetView())->WantInputFocus( (class textview *)(f)->GetView());
}

class event *pintv_GetReportEvent(long  rock)
{
    return im::EnqueueEvent((event_fptr)ReportErrors, (char *)rock, 0);
}

static void KeepOldCopy(class pintv  *self, long  curtime)
{
    class prefs *prefs=(class prefs *)(self)->GetDataObject();
    char buf[1024];
    char error[1500];
    class cursor *busy, *old=NULL;
    int close_err=0;
    FILE *fp=NULL;
    old=im::GetProcessCursor();
    message::DisplayString(self, 100, "Make sure you have a backup copy of your preferences.\nYou may not like how the preferences\n editor re-formats them.\n The preferences editor is still experimental,\nit may corrupt your preferences under some situations.\nIf this happens please let us know.");
    strcpy(buf, "~/keep.prf");
    while(1) {
	if (completion::GetFilename(self, "File to save old preferences to:", buf, buf, sizeof(buf), FALSE, FALSE) == -1) {
	    errno = 0;
	    message::DisplayString(self, 100, "Cancelled.  If you save you will overwrite your old preferences.");
	    break;
	}
	busy=cursor::Create(self);
	if(busy) {
	    (busy)->SetStandard( Cursor_Wait);
	    im::SetProcessCursor(busy);
	}
	fp=fopen(buf, "w");
	if(fp==NULL) {
	    sprintf(error, "Couldn't open file %s for writing.", buf);
	    message::DisplayString(self, 100, error);
	    im::SetProcessCursor(old);
	    continue;
	}	    
	errno=0;
	(prefs)->WritePlain( fp, im::GetWriteID(), 0);
#ifdef AFS_ENV
	if(ferror(fp)) close_err=(-1);
	else close_err=vclose(fileno(fp));
	fclose(fp);
	if(close_err!=0)
#else
	if((fclose(fp))!=0)
#endif
	{
	    sprintf(error, "Failed to save old preferences in %s.", buf);
	    message::DisplayString(self, 100, error);
	    im::SetProcessCursor(old);
	    continue;
	} else {
	    sprintf(error, "Saved old preferences in %s.", buf);
	    message::DisplayString(self, 0, error);
	    break;
	}
    }
    (prefs)->UpdateText();
    im::SetProcessCursor(old);
}

class event *pintv_GetKeepEvent(long  rock)
{
    return im::EnqueueEvent((event_fptr)KeepOldCopy, (char *)rock, 0);
}

static boolean FindTextBuffer(class buffer  *b, class prefs  *data)
{
    if((b)->GetData()==(class dataobject *)data && (b)->GetDefaultViewname() && strcmp((b)->GetDefaultViewname(), "textview")==0) return TRUE;
    else return FALSE;
}

void pintv_EditAsText(class pintv  *self, class sbutton  *sb, long  rock)
{
    char bufname[1024];
    class buffer *b;
    class frame *f;
    class prefs *prefs=PREFS(self);
    (prefs)->UpdateText();
    b=buffer::Enumerate((bufferlist_efptr)FindTextBuffer, (long)prefs);
    if(b==NULL) {
	buffer::GetUniqueBufferName("PrefsText", bufname, sizeof(bufname));
	b=buffer::Create(bufname, NULL, NULL, prefs);
	if(b==NULL) {
	    fprintf(stderr, "Preferences Warning: text buffer could not be created.\n");
	    return;
	}
	(b)->SetDefaultViewname( "textview");
    }
    f=frame::GetFrameInWindowForBuffer(b);
    if(f==NULL) {
	fprintf(stderr, "Preferences Warning: couldn't get a window for the error buffer %s\n", bufname);
	fprintf(stderr, "Use switch buffer to examine the error list.\n");
    }
    ((class textview *)(f)->GetView())->WantInputFocus( (class textview *)(f)->GetView());
}
