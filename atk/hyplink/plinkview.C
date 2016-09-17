/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#include <andrewos.h>
ATK_IMPL("plinkview.H")
#include <sys/param.h>	/* for MAXPATHLEN */

#include <plinkview.H>

#include <plink.H>
#include <linkview.H>
#include <proctable.H>
#include <menulist.H>
#include <message.H>
#include <observable.H>
#include <frame.H>
#include <buffer.H>
#include <view.H>
#include <environment.H>
#include <viewref.H>
#include <text.H>
#include <textview.H>
#include <fnote.H>
#include <texttag.H>
#include <im.H>





static class menulist *plinkview_menulist = NULL;


ATKdefineRegistry(plinkview, linkview, plinkview::InitializeClass);
static void SetTagProc(class plinkview  *self, char  *rock);
static boolean HuntCheck(class plinkview  *self, class text  *t, long  pos, class environment  *env);
static void HuntForTag(class plinkview  *self, class textview  *tv);
static void NewWarpLink(class plinkview  *self, class observable  *triggerer, long  rock);
static boolean FindBuffer(class frame  *f,class buffer  *b);
static class view *FindViewofBuffer(class buffer  *b);


plinkview::plinkview()
{
	ATKinit;

    /* Inititialize the object instance data. */
    boolean res;

    this->pml = (plinkview_menulist)->DuplicateML( this);
    (this)->DeleteRecipient ( atom::Intern("buttonpushed"), this); 
    res = (this)->AddRecipient( atom::Intern("buttonpushed"), this, (observable_fptr)NewWarpLink, 0);

    if (!res) 
	THROWONFAILURE( FALSE); /* couldn't make button work */

    THROWONFAILURE( TRUE);
}

plinkview::~plinkview()
{
	ATKinit;

    /* Finalize the object instance data. */
    delete this->pml;
    return;
}

boolean plinkview::InitializeClass()
{
    struct proctable_Entry *proc = NULL;

    plinkview_menulist = new menulist;

    proc = proctable::DefineProc("plinkview-set-tag", (proctable_fptr)SetTagProc, &plinkview_ATKregistry_ , NULL, "Prompts for user to set search tag of the link button.");
    (plinkview_menulist)->AddToML( "Link~1,Set Tag~15", proc, 0, 0);

    return TRUE;
}

void plinkview::PostMenus(class menulist  *ml)
{
/* Enable the menus for this object. */

    (this->pml)->ClearChain();
    if (ml) (this->pml)->ChainAfterML( ml, (long)ml);
    (this)->linkview::PostMenus( this->pml);
}

static void SetTagProc(class plinkview  *self, char  *rock)
{
    char buffer[80];
    int res;
    class plink *pl = (class plink *)(self)->GetDataObject();

    res = message::AskForString (self, 10, "New search tag for this link:  ", (pl)->GetTag(), buffer, 79);

    if (res<0) {
	message::DisplayString(self, 10, "Cancelled.");
	return;
    }

    if (strlen(buffer)==0) {
	message::DisplayString(self, 10, "Tag cleared (link will go to top of file).");
	(pl)->SetTag( NULL);
    }
    else {
	(pl)->SetTag( buffer);

	if ((pl)->GetText()!=NULL) {
	    message::DisplayString(self, 10, "Tag set.");
	}
	else {
	    (pl)->SetText( buffer);
	    message::DisplayString(self, 10, "Tag (and label) set.");
	}
    }
}

static boolean HuntCheck(class plinkview  *self, class text  *t, long  pos, class environment  *env)
{
    class viewref *vref;
    enum environmenttype etype;
    class dataobject *d;
    class texttag *ttag;
    class plink *pl;
    char buf[128];
    char *mytag;

    /* get data from structure. Akhhhh, blthhhhpppch. */
    etype = env->type;

    if (etype != environment_View) {
	return FALSE;
    }

    vref = env->data.viewref;
    d = vref->dataObject;
    if (!ATK::IsTypeByName((d)->GetTypeName(), "texttag")) {
	return FALSE;
    }

    ttag = (class texttag *)d;
    (ttag)->GetTag( 128, buf);
    pl = (class plink *)(self)->GetDataObject();
    mytag = (pl)->GetTag();

    if (mytag==NULL || !strncmp(mytag, buf, strlen(mytag))) {
	return TRUE;
    }

    return FALSE;
}

static void HuntForTag(class plinkview  *self, class textview  *tv)
{
    class text *t = (class text *)(tv)->GetDataObject();
    long pos, len;
    long doclen = (t)->GetLength();
    class environment *env;
    class plink *b = (class plink *)(self)->GetDataObject();

    if ((b)->GetTag()==NULL) {
	pos = 0;
	len = 0;
    }
    else {
	fnote::CloseAll(t);
	env = (t)->EnumerateEnvironments( 0, doclen, (text_eefptr)HuntCheck, (long)self);
	if (env==NULL) {
	    pos = 0;
	    len = 0;
	}
	else {
	    pos = (env)->Eval();
	    len = 1;
	}
    }

    (tv)->SetDotPosition( pos);
    (tv)->SetDotLength( len);
    (tv)->FrameDot( pos);
    (tv)->WantInputFocus( tv);
}

static void NewWarpLink(class plinkview  *self, class observable  *triggerer, long  rock)
{
    char temp[MAXPATHLEN];
    class buffer *buffer;
    class view *view;
    class im *im;
    const char *filename;
    class plink *b = (class plink *)(self)->GetDataObject();

    filename = (b)->GetResolvedLink();
    if (!filename) {
	message::DisplayString(self, 10, "No link");
	return;
    }

    if ((buffer = buffer::FindBufferByFile(filename)) == NULL) {
	/* don't have the file in memory, get it */
	if ((buffer = buffer::GetBufferOnFile(filename, 0)) == NULL) {
	    sprintf(temp, "Couldn't get file %s.", filename);
	    message::DisplayString(self, 50, temp);
	    return;
	}
    }
    /* have the file in buffer buffer */

    /* get a view on the buffer */
    view = FindViewofBuffer(buffer);

    if ((view == NULL) || ((im = (view)->GetIM()) == NULL)) {
	message::DisplayString(self, 50, "Couldn't find window.");
	return;
    }

    (view)->WantInputFocus(view);

    /* If view is a textview, set dot position */
    if (ATK::IsTypeByName((view)->GetTypeName(), "textview")) {
	class textview *tv = (class textview *)view;
	HuntForTag(self, tv);
    }

    /* pop to top window */
    (im)->ExposeWindow();
    /* warp cursor there */
    (im)->SetWMFocus();
}

static boolean FindBuffer(class frame  *f,class buffer  *b)
{
    /* Little, dippy routine passed to frame_Enumerate to find the frame which contains the buffer we want. */

    return((f)->GetBuffer()==b);
}

static class view *FindViewofBuffer(class buffer  *b)
{
    /* I don't know why *I* have to do this, it should be a buffer method. Anyways, this tries to find the frame of our buffer.  If there is no such frame, make one, make a new IM for it (new window), and put the buffer in the frame in the IM.  *phew* */

    class frame *f;
    class im *im;

    if ((f = frame::Enumerate((frame_effptr)FindBuffer, (long)b))==NULL) {
	/* No frame--need to map buffer to new window */

	if((f = new frame) == NULL) {
	    fprintf(stderr,"hyplink: Could not allocate enough memory.\n");
	    return((class view*) NULL);
	}
	if((im = im::Create(NULL)) == NULL) {
	    fprintf(stderr,"hyplink: Could not create new window.\n");
	    if(f) (f)->Destroy();
	    return((class view*) NULL);
	}
	(im)->SetView( f);

	(f)->SetCommandEnable( TRUE);
	(f)->PostDefaultHandler( "message", (f)->WantHandler( "message"));
	(f)->SetBuffer( b, TRUE);
    }
    return((f)->GetView());
}

