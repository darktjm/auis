/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *	Copyright Carnegie Mellon Univesity 1996
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
ATK_IMPL("linkview.H")
#include <sys/param.h>	/* for MAXPATHLEN */
#include <linkview.H>
#include <link.H>
#include <atom.H>
#include <buffer.H>
#include <completion.H>
#include <path.H>
#include <fontdesc.H>
#include <frame.H>
#include <im.H>
#include <menulist.H>
#include <message.H>
#include <observable.H>
#include <proctable.H>
#include <pshbttnv.H>
#include <textview.H>
#include <text.H>
#include <view.H>
#include <environ.H>

#ifdef ATTBL_ENV
#include <bind.H>
#include "shellesc.ih"
#endif

/* Defined constants and macros */
#define DBG(x) fprintf(stderr, "\nDebug: %s.", x);fflush(stderr);

#define PROMPTFONT "andy12b"
#define FONT "andy"
#define FONTSIZE 12
#define BUTTONDEPTH 4
#define BUTTONPRESSDEPTH 2
#define TEXTPAD 2

  

/* Global Variables */
static class menulist *linkview_menulist = NULL;
static class linkview *autolink_source = NULL;
static struct ATKregistryEntry  *proctype = NULL;
static struct ATKregistryEntry  *textviewtype = NULL;


#ifdef ATTBL_ENV
static struct bind_Description frameBindings[] = {
    {"linkview-set-target", "", 0, 
		"Inset~33,Autolink target here~12", 0, 0, 
		(proctable_fptr)TargetProc, 
		"Execute this proc from the frame of the the buffer for the target file of a link.  To be called after linkview-autolink."},
    NULL
};

static struct bind_Description textviewBindings[] = {
    {"linkview-insert-link", "", 0, 
		"Inset~33,Insert Link~13", 0, 0, 
		(proctable_fptr)InsertProc, 
		"Insert a link object in the current document."},
    NULL
};
#endif


ATKdefineRegistry(linkview, pushbuttonview, linkview::InitializeClass);

static void FileLinkProc(class ATK  *self, char  *param);
static void RelLinkProc(class ATK  *self, char  *param);
static void URLLinkProc(class ATK  *self, char  *param);
static void VarLinkProc(class ATK  *self, char  *param);
static boolean FindBuffer(class frame  *f,class buffer  *b);
static class view * FindViewofBuffer(class buffer  *b);
static void WarpLink(class linkview  *self, class observable  *triggerer, long  rock);
static char * FileName(char  *path);
static void InsertProc(class ATK  *tv,long  l);
static void TargetProc(class ATK  *v, long  param);
static void AutolinkProc(class ATK  *self, long  param);


#define DEFINEPROC(name, proc, registry, module, doc) proctable::DefineProc(name, (proctable_fptr)proc, registry, module, doc)

/* 
  Initialize all the class data:   set up the proc table entries 
  and the menu list (which is cloned for each instance of this class).
*/
	boolean
linkview::InitializeClass() {
	struct proctable_Entry *proc = NULL;

	proctype = ATK::LoadClass("frame");
	textviewtype = ATK::LoadClass("textview");
	linkview_menulist = new menulist;

#ifndef ATTBL_ENV
	proc = DEFINEPROC("linkview-set-target", TargetProc, 
		proctype, NULL, 
		"Execute this proc from the frame of the the buffer for the target file of a link.  To be called after linkview-autolink.");

	proc = DEFINEPROC("linkview-insert-link", InsertProc, 
		textviewtype, NULL, 
		"Insert a link object in the current document.");
#endif

	proc = DEFINEPROC("linkview-autolink", AutolinkProc, 
		&linkview_ATKregistry_ , NULL, 
		"Starts the autolink process.  Waits for linkview-set-target to be invoked, which tells this link what file to link to.");
	(linkview_menulist)->AddToML("Link~1,Start Autolink~1", 
		proc, 0, 0);

	proc = DEFINEPROC("linkview-set-link", 
		FileLinkProc, &linkview_ATKregistry_ , NULL, 
		"Prompts user for target filename of the link button.");
	(linkview_menulist)->AddToML(
		"Link~1,Link to File~11", proc, 0, 0);

	proc = DEFINEPROC("linkview-set-rel-link",
		RelLinkProc, &linkview_ATKregistry_ , NULL, 
		"Prompts user for a relative filename to link to.");
	(linkview_menulist)->AddToML(
		"Link~1,Link to Relative File~12", proc, 0, 0);

	proc = DEFINEPROC("linkview-set-var-link", 
		VarLinkProc, &linkview_ATKregistry_ , NULL, 
		"Prompts user for an uninterpreted link value (may have ${...}).");
	(linkview_menulist)->AddToML(
		"Link~1,Link to ${...}/...~13",proc, 0, 0);
	proc = DEFINEPROC("linkview-set-url-link", 
		URLLinkProc, &linkview_ATKregistry_ , NULL, 
		"Prompts user for a URL link value.");
	(linkview_menulist)->AddToML(
		"Link~1,Link to URL~14", proc, 0, 0);

#ifdef ATTBL_ENV
  	attachBindings(frameBindings, TRUE,
		environ::GetProfileSwitch("LinkMenus", TRUE),
		"frame");
	attachBindings(textviewBindings, TRUE, 
		environ::GetProfileSwitch("LinkMenus", TRUE), 
		"textview");
#endif

	return(TRUE);
}

/*
  Set up the data for each instance of the object (i.e.: clone the menu
  list, add the trigger recipient).
*/
linkview::linkview() {
	ATKinit;
	this->ml = (linkview_menulist)->DuplicateML( this);
	THROWONFAILURE(((this)->AddRecipient( atom::Intern("buttonpushed"), 
			this, (observable_fptr)WarpLink, 0L)));
}

linkview::~linkview() {
	ATKinit;
	return;
}

/*
  Enable the menus for this object.
*/
	void
linkview::PostMenus(class menulist  *ml) {
  (this->ml)->ClearChain();
  if (ml) (this->ml)->ChainAfterML( ml, (long)ml);
  (this)->pushbuttonview::PostMenus( this->ml);
}

	static void
DisplayLinkMsg(linkview *self) {
	link *dobj = (link *)self->GetDataObject();
	char buf[MAXPATHLEN+50];
	switch (dobj->GetTypeCode()) {
	case NoLink:  sprintf(buf,  "Not Linked"); break;
	case FileLink:  sprintf(buf, "Linked to File %s",
				dobj->GetResolvedLink(self));  break;
	case RelLink:  sprintf(buf, "Linked to Relative File %s",
				dobj->GetRawLink());  break;
	case URLLink:  sprintf(buf, "Linked to URL %s",
				dobj->GetRawLink());  break;
	case VarFileLink:  sprintf(buf, "$ Linked to File %s",
				dobj->GetRawLink());  break;
	}
	message::DisplayString(self, 10, buf);
}

	class view *
linkview::Hit (enum view_MouseAction action, 
			long x, long y, long numberOfClicks)  {
	if (action == view_RightDown) 
		DisplayLinkMsg(this);
	return pushbuttonview::Hit(action, x, y, numberOfClicks);
}

	static void
FinishLink(linkview *self, enum link_type code, char *linkdest) {
	link *l = (link *)(self)->GetDataObject();
	(l)->SetLink(code, linkdest);
	DisplayLinkMsg(self);
	(l)->SetText(FileName(linkdest));
}

/*
  Prompt the user for a file name.
  Uses the (amazingly quick) completion package.
*/
	static void
FileLinkProc(class ATK  *aself, char  *param)
{
	class linkview *self=(class linkview *)aself;

	char buf[MAXPATHLEN];
	class link *l = (class link *)(self)->GetDataObject();

	if (param != NULL) 
	    strcpy(buf, param);
	else {
	    const char *initval = (l)->GetRawLink();
	    if ( ! initval ||  ! *initval) {
		initval = (self)->WantInformation("filename");
		if (initval) {
			strcpy(buf, initval);
			initval = buf;
			// N.B.:  sometimes initval refers to buf
			char *slash = strrchr(buf, '/');
			if (slash) slash[1] = '\0';	// retain slash at end
		}
	    }
	    if (completion::GetFilename(self, "New target file for link:  ",
				       initval, buf, sizeof(buf), 0, 0) < 0)
		return;
	    if (strchr(buf, '$')) {
		char tmpbuf[1 + MAXPATHLEN];

		(void) im::GetDirectory(tmpbuf);
		strcat(tmpbuf, "/$");
		if (strncmp(buf, tmpbuf, strlen(tmpbuf)) == 0) {

		    /* Okay, here's a really gross hack.

			Retained for compatibility, but should not be. xxx

		       This hack remedies the following problem:
		         completion_GetFilename will prepend the current
			 directory to the resulting string when the user's
			 input looks like a relative pathname.  Unfortunately,
			 strings of the form "$FOO/whatever" look like
			 relative pathnames, and so get changed to
			 "/xyz/abc/$FOO/whatever", which is wrong.  On the
			 other hand, if the user's input is
			 "jkl/$FOO/pqr" then it is correct to prepend the
			 working directory.
		       The solution is to look at the result of
		       completion_GetFilename and check whether it's of
		       the form "workingdirectory/$anything".  If it is,
		       we strip the "workingdirectory/" off it.
		       At this point in the code, we know it's of that form.
		     */
		    strcpy(tmpbuf, buf+strlen(tmpbuf)-1);
		    strcpy(buf, tmpbuf);
		}
	    }
	}
	FinishLink(self, FileLink, buf);
}

	static void
RelLinkProc(class ATK  *aself, char  *param) {
	class linkview *self=(class linkview *)aself;
	class link *l = (class link *)(self)->GetDataObject();

	const char *filename = self->WantInformation("filename");
	char buf[MAXPATHLEN], initialpath[MAXPATHLEN];

	// The original link and resulting link are both relative,
	// but the prompt and file completed values are absolute.
	//  UnfoldFileName combines its 1st and 3rd arguments,
	//  putting them in its 2nd.
	//  RelativizePathname removes the common prefix.

	if (param != NULL) 
		strcpy(buf, param);
	else {
		if (completion::GetFilename(self, 
				"FULL file name for path-relative link:  ",
				path::UnfoldFileName((l)->GetRawLink(),
						initialpath, filename), 
				buf, sizeof(buf), 0, 0) < 0)
		return;
	}
	path::RelativizePathname(buf, filename, 2);

	FinishLink(self, RelLink, buf);
}

	static void
URLLinkProc(class ATK  *aself, char  *param) {
	class linkview *self=(class linkview *)aself;

	char buf[MAXPATHLEN];
	class link *l = (class link *)(self)->GetDataObject();

	if (param != NULL) 
		strcpy(buf, param);
	else {
		if (message::AskForString(self, 50,  
				"Link to URL: ",
				(l)->GetRawLink(),
				buf, sizeof(buf)) < 0)
		return;
	}
	FinishLink(self, URLLink, buf);
}

	static void
VarLinkProc(class ATK  *aself, char  *param) {
	class linkview *self=(class linkview *)aself;

	char buf[MAXPATHLEN];
	class link *l = (class link *)(self)->GetDataObject();

	if (param != NULL) 
		strcpy(buf, param);
	else {
		if (message::AskForString(self, 50,  
				"Link to raw string:  ",
				(l)->GetRawLink(),
				buf, sizeof(buf)) < 0)
		return;
	}
	FinishLink(self, VarFileLink, buf);
}



static boolean
FindBuffer(class frame  *f,class buffer  *b)
{
/*
  Little, dippy routine passed to frame_Enumerate to find the
  frame which contains the buffer we want.
*/

  return((f)->GetBuffer()==b);
}


static class view *
FindViewofBuffer(class buffer  *b)
{
/*
  I don't know why *I* have to do this, it should be a buffer method.
  Anyways, this tries to find the frame of our buffer.  If there is no
  such frame, make one, make a new IM for it (new window), and put the
  buffer in the frame in the IM.  *phew*
*/



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

/*
  Do the actual "warp" in response to a button press.  
  Open the file in a buffer in the SAME window.  (The old semantics used 
  another window.)	
*/
	static void
WarpLink(class linkview  *self, class observable  *triggerer, long  rock) {
  char temp[MAXPATHLEN];
  class buffer *buffer;
  class view *view;
  class im *im;
  const char *filename;
  class link *b = (class link *)(self)->GetDataObject();

  filename = (b)->GetResolvedLink(self);
  if ( ! filename) {
      message::DisplayString(self, 10, "No link");
      return;
  }

  // check for URL and just display message (XXX later do the warp)
  if (b->GetTypeCode() == URLLink) {
	char buf[MAXPATHLEN+50];
	sprintf(buf, "Link to URL %s", b->GetRawLink());
	message::DisplayString(self, 60, buf);
	return;
  }

#ifdef ATTBL_ENV
  /* extend so that hyplink can execute a command */
  if (filename[0] == '!') {
      /* execute command and capture output in buffer */
      shellesc_RunAsync(&filename[1], "Link Endpoint", &buffer, NULL, NULL, 0, 0, FM_CREATE);
  }
  else 
#endif

  // filename must name a file
  if (access(filename, R_OK) < 0) {
	char buf[MAXPATHLEN+50];
	sprintf(buf, "Cannot find file %s", filename);
	message::DisplayString(self, 60, buf);
	return;
  }

  if (environ::GetProfileSwitch("LinkReuseWindow", TRUE)) {
	// use same window as at present
	im = self->GetIM();
	view = im->topLevel;	// reach inside im
	if (view->IsType("frame")) {
		frame *frm = (frame *)view;
		frm->VisitNamedFile(filename, FALSE, FALSE);
	}
  }

  // read file into buffer 
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

  /* If view is a textview, set dot pos-len */
  if (ATK::IsTypeByName((view)->GetTypeName(), "textview")) {
      class textview *tv = (class textview *)view;
      class text *t = (class text *)(tv)->GetDataObject();
      long pos = (b)->GetPos();
      long len = (b)->GetLen();
      long docLen = (t)->GetLength();
      if (pos > docLen) pos = docLen;
      if (pos + len > docLen) len = docLen - pos;
      (tv)->SetDotPosition( pos);
      (tv)->SetDotLength( len);
      (tv)->FrameDot( pos);
      (tv)->WantInputFocus( tv);
  }

  /* pop to top window */
  (im)->ExposeWindow();
  /* warp cursor there */
  (im)->SetWMFocus();
}


static char *
FileName(char  *path)
{
/*
  Returns the filename portion of path (i.e.: strips leading
  directories and any extensions--unless the filename begins
  with a '.', in which case it strips the extensions after the 
  first '.' [so: ".cshrc" and ".cshrc.BAK" maps to ".cshrc"]);
  Warning:  destructively modifies the string path!
*/
  char *r, *s;

  if ((s = strrchr(path, '/'))==NULL) {
    s = path;
  } else {
    ++s;
  }
  if ((r = strchr(s+1, '.'))!=NULL) *r = '\0';

  return(s);
}


static void
InsertProc(class ATK  *atv,long  l)
{
    class textview *tv=(class textview *)atv;
    long pos;
    class text *t = (class text *) (tv)->GetDataObject();
    pos = (tv)->GetDotPosition() + (tv)->GetDotLength();
    tv->currentViewreference = (t)->InsertObject( pos,"link", NULL);
    (t)->NotifyObservers(0);
}


static void
TargetProc(class ATK  *av, long  param)
{
    class view *v=(class view *)av;
/*
  First, checks to see if there is a link object waiting for an
  autolink.  Then, if checks to make sure this frame has a buffer,
  and that the buffer has a filename.
*/
  class buffer *fb;
  char *fn, name[255];
  class link *l;

  if (!autolink_source) {
    message::DisplayString(v, 50, "Please set autolink source first.");
    return;
  }

  if (!ATK::IsTypeByName((v)->GetTypeName(),"frame")) {
    message::DisplayString(v, 10, "Autolink works on frames only");
    return;
  }
  if ((fb = (class buffer *)(((class frame *)v))->GetBuffer())==NULL) {
    message::DisplayString(v, 10, "Autolink works with buffers only");
    return;
  }

  if ((fn = ((class buffer *)fb)->GetFilename())==NULL) {
    message::DisplayString(v, 50, "Can only autolink to buffers on files.");
  } else {
    class view *vw;
    strcpy(name, fn);
    l = (class link *)(autolink_source)->GetDataObject();
    (l)->SetLink( name);
    /* if textview, also set pos and len */
    vw = (((class frame *)v))->GetView();

    if (vw != NULL && ATK::IsTypeByName((vw)->GetTypeName(), "textview")) {
	class textview *tv = (class textview *)vw;
	(l)->SetPos( (tv)->GetDotPosition());
	(l)->SetLen( (tv)->GetDotLength());
    }
    /* Now set the label if needed */
    if ((l)->GetText() == NULL) {
	(l)->SetText(FileName(name));
	message::DisplayString(autolink_source, 10, "Changed link target (and label).");
    } else {
      message::DisplayString(autolink_source, 10, "Changed link target.");
    } /* if (!link_LabelSetP(...)) */
  } /* if (!(fn = ...)) */
  autolink_source = NULL;

  return;
}


static void
AutolinkProc(class ATK  *aself, long  param)
{
    class linkview *self=(class linkview *)aself;
/*
  Start the autolink process.  Check to make sure we're not trouncing
  on another autolink first though....
*/
  static const char * const conflict[] = {
    "Use new autolink source",
    "Use old autolink source",
    "Cancel autolink",
    NULL
  };
  long answer;

  if (autolink_source) {
    if (message::MultipleChoiceQuestion(self,99,"Already autolinking!", 2, &answer, conflict, NULL)>= 0) {
      switch(answer) {
      case 0:
	message::DisplayString(self, 10, "Please indicate autolink target buffer.");
	message::DisplayString(autolink_source, 10, "Cancelled.  Using new source.");
	autolink_source = self;
	return;
      case 1:
	message::DisplayString(self, 10, "Cancelled.  Using old source");
	message::DisplayString(autolink_source, 10, "Please indicate autolink target buffer with the linkview-set-target proc.");
	return;
      case 2:
	message::DisplayString(self, 10, "Autolink cancelled.");
	message::DisplayString(autolink_source, 10, "Autolink cancelled.");
	autolink_source = NULL;
	return;
      }
    } else {
	message::DisplayString(self, 10, "Autolink cancelled.");
	message::DisplayString(autolink_source, 10, "Autolink cancelled.");
	autolink_source = NULL;
	return;
    }
  }
  message::DisplayString(self, 10, "Please indicate autolink target buffer.");
  autolink_source = self;

  return;
}

 void linkview::Link()
{
    FileLinkProc(this, 0);
}

 void linkview::AutoLink()
{
    AutolinkProc(this, 0);
}

 void linkview::LinkFile(char  *dest )
{
    FileLinkProc(this, dest);
}
