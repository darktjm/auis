/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/t822view.C,v 1.5 1995/11/07 20:17:10 robr Stab74 $";
#endif

#include <andrewos.h>
#include <sys/param.h>

#include <textview.H>

#include <cui.h>
#include <fdphack.h>

#include <amsutil.H>
#include <text.H>
#include <environ.H>
#include <environment.H>
#include <captions.H>
#include <im.H>
#include <folders.H>
#include <proctable.H>
#include <menulist.H>
#include <keystate.H>
#include <keymap.H>
#include <message.H>
#include <bind.H>
#include <style.H>
#include <scroll.H>
#include <t822view.H>
#define dontDefineRoutinesFor_sendmessage
#include <sendmessage.H>
#undef dontDefineRoutinesFor_sendmessage
#include <ams.H>

#define Text(self) ((class text *) (self)->GetDataObject())

static class keymap *t822view_standardkeymap;
static class menulist *t822view_standardmenulist;


ATKdefineRegistry(t822view, messages, t822view::InitializeClass);
#ifndef NORCSID
#endif
void BodiesCompound(class t822view  *self, char  *cmds);
void BodiesTextviewCommand(class t822view  *self, char  *cmds);
void BodiesCaptionsCommand(class t822view  *self, char  *cmds);
static void DownFocus(class t822view  *self);
static void UpFocus(class t822view  *self);


void BodiesCompound(class t822view  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( self, "t822view", cmds);
}

void BodiesTextviewCommand(class t822view  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( self, "textview", cmds);
}

void BodiesCaptionsCommand(class t822view  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( (self)->GetCaptions(), "captions", cmds);
}

static void DownFocus(class t822view  *self)
{
    class folders *f = ((self)->GetCaptions())->GetFolders();

    if (f->sm) {
	ams::Focus(f->sm->HeadTextview);
    } else {
	ams::Focus(f);
    }
}

static void UpFocus(class t822view  *self)
{
    ams::Focus((self)->GetCaptions());
}

static struct bind_Description t822view_standardbindings [] = {
    /* procname, keysequence, key rock, menu string, menu rock, proc, docstring, dynamic autoload */
    {"bodies-down-focus", "\030n", 0, NULL, 0, 0, (proctable_fptr)DownFocus, "Move input focus to folders or sendmessage"},
    {"bodies-up-focus", "\030p", 0, NULL, 0, 0, (proctable_fptr)UpFocus, "Move input focus to captions"},
    {"bodies-down-focus", "\030\016", 0, NULL, 0, 0, (proctable_fptr)DownFocus, "Move input focus to folders or sendmessage"},
    {"bodies-up-focus", "\030\020", 0, NULL, 0, 0, (proctable_fptr)UpFocus, "Move input focus to captions"},
    {"bodies-compound-operation", NULL, 0, NULL, 0, 0, (proctable_fptr)BodiesCompound, "Execute a compound bodies operation"},
    {"bodies-textview-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)BodiesTextviewCommand, "Execute a compound 'textview' operation on the bodies"},
    {"bodies-captions-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)BodiesCaptionsCommand, "Execute a compound 'folders' operation."},
    {NULL, NULL, 0, NULL, 0, 0, NULL, NULL, NULL}
};

static class style *InstructionsStyle = NULL;

boolean t822view::InitializeClass()
{
    t822view_standardkeymap = new keymap;
    t822view_standardmenulist = new menulist;
    bind::BindList(t822view_standardbindings, t822view_standardkeymap, t822view_standardmenulist, &t822view_ATKregistry_ );
    InstructionsStyle = new style;
    (InstructionsStyle)->SetFontSize( style_ConstantFontSize, 16);
    (InstructionsStyle)->SetJustification( style_LeftJustified);
    (InstructionsStyle)->SetFontFamily( amsutil::GetDefaultFontName());
    return(TRUE);
}

t822view::t822view()
{
	ATKinit;

    (this)->SetWhatIAm( WHATIAM_BODIES);
    ams::AddCheckpointBodies(this);
    this->myks = keystate::Create(this, t822view_standardkeymap);
    this->myml = (t822view_standardmenulist)->DuplicateML( this);
    this->mycaps = NULL;
    this->PriorReadOnliness = FALSE;
    THROWONFAILURE((TRUE));
}

void t822view::SetCaptions(class captions  *cap)
{
    this->mycaps = cap;
}

void t822view::ShowHelp(char  *text)
{
    int len;
    static char *InitialHelpString = "To see the captions for messages in a folder, click with the left mouse button on the folder name.\n\nTo see a message, click on its caption in the middle region of the window.\n\nTo perform actions on a folder, or get information about it, click on the help icon next to its name.";
    char *SurrogateHelpFile, *s;
    char ErrMsg[500+MAXPATHLEN];
    class environment *et;
    extern int errno;
    class text *t = Text(this);
    class im *im;

    (this)->SetDotPosition( 0);
    (this)->SetTopPosition( 0);
    s = (char *) environ::GetProfile("messages.surrogatehelpfile");
    if (s) {
	if(SurrogateHelpFile = (char *)malloc(1+strlen(s)))
	    strcpy(SurrogateHelpFile, s);
	else s = SurrogateHelpFile;
    } else SurrogateHelpFile = s;
    (t)->ClearCompletely();
    im = (this)->GetIM();
    if (im && (im)->GetInputFocus() == NULL) (this)->WantInputFocus( this);
    if (!text) {
	if (SurrogateHelpFile && *SurrogateHelpFile) {
	    FILE *fp;
	    char FName[1+MAXPATHLEN];

	    (ams::GetAMS())->TildeResolve( SurrogateHelpFile, FName);
	    fp = fopen(FName, "r");
	    if (!fp) {
		sprintf(ErrMsg, "Cannot read surrogate help file %s (%d).", FName, errno);
		text = ErrMsg;
	    } else {
		(t)->AlwaysInsertFile( fp, FName, 0);
		fclose(fp);
	    }
	} else {
	    text = InitialHelpString;
	}
    }
    if (text) {
	len = strlen(text);
	(t)->AlwaysInsertCharacters( 0, text, len);
	et = (t->rootEnvironment)->InsertStyle( 0, InstructionsStyle, 1);
	(et)->SetLength( len);
    }
    (this)->WantUpdate( this);
}


/* we override ObservedChanged so we can be sure the keystate is posted
	when there is a change in the read-onliness */
void t822view::ObservedChanged(class observable  *changed, long  value)
            {
    class text *text = Text(this);
    boolean RO;
    (this)->messages::ObservedChanged( changed, value);
    if (value == observable_OBJECTDESTROYED)  
	return;
    RO = (text)->GetReadOnly();
    if (this->PriorReadOnliness != RO
         && ((class textview *) this)->hasInputFocus) {
	(this)->WantInputFocus( this);	/* post key state */
	this->PriorReadOnliness = RO;
    }
}



void t822view::PostMenus(class menulist  *ml)
{
    (this->myml)->ClearChain();
    if (ml) (this->myml)->ChainAfterML( ml, (long)ml);
    (this)->messages::PostMenus( this->myml);
}

void t822view::PostKeyState(class keystate  *ks)
{
    this->myks->next = NULL;
    if (amsutil::GetOptBit(EXP_KEYSTROKES) 
		&& (Text(this))->GetReadOnly()) {
	if (ks) (ks)->AddAfter( this->myks);
	(this)->messages::PostKeyState( this->myks);
    } else {
	(this)->messages::PostKeyState( ks);
    }
}

/*
class view *t822view::GetApplicationLayer()
{
     return (class view *) scroll::Create(this, scroll_LEFT);
}
*/

void t822view::DeleteApplicationLayer(class view  *scrollbar)
{
    ams::RemoveCheckpointBodies(this);
    (scrollbar)->Destroy();
}

void t822view::SetDataObject(class dataobject  *dat)
{
    (this)->messages::SetDataObject( dat);
    (this)->ShowHelp( NULL);
    this->PriorReadOnliness = (Text(this))->GetReadOnly();
}

class captions *
t822view::NewCaptionsInNewWindow()
{
    class captions *cap = new captions;

    (this)->SetCaptions( cap);
    (cap)->SetBodies( this);
    cap->myframe = ams::InstallInNewWindow((cap)->GetApplicationLayer(), "messages-captions", "Message Captions", environ::GetProfileInt("captions.width", 600), environ::GetProfileInt("captions.height", 250), cap);
    return(cap);
}

t822view::~t822view()
{
	ATKinit;

    ams::RemoveCheckpointBodies(this);
    if (this->mycaps) {
	(this->mycaps)->SetBodies( NULL);
    }
    if (this->myks) delete this->myks;
    if (this->myml) delete this->myml;
}

class captions *t822view::GetCaptions()
{
    if (!this->mycaps) {
	(this)->NewCaptionsInNewWindow();
    }
    return(this->mycaps);
}

void t822view::PrintPSDoc(FILE  *outfile, long  width, long  height)
{
    /* Peform hackery to eliminate unimportant headers and widen the margins. */

    // First widen the margins for printing (perhaps this can be the default for on screen?)
    text *txt = (text *)GetDataObject();
    style *glob = txt->GetGlobalStyle();
    enum style_MarginValue basis_left, basis_right;
    long op_left, op_right;
    enum style_Unit unit_left, unit_right;
    if (glob) {
	glob->GetNewLeftMargin(&basis_left, &op_left, &unit_left);
	glob->GetNewRightMargin(&basis_right, &op_right, &unit_right);
	glob->SetNewLeftMargin(style_LeftMargin, -18, style_Points);
	glob->SetNewRightMargin(style_RightMargin, -36, style_Points);
    }
    // Now, delete the unimportant headers.  Save them so we can put them back!
    text *saved_hdrs = new text;
    int pos = 0;
    int endpos = txt->GetLength();
    int ch = 0;
    while (ch != '\n' && pos < endpos) {
	while (ch != '\n' && pos < endpos) {
	    ch = txt->GetChar(pos++);
	}
	ch = txt->GetChar(pos++);
    }
    if (pos < endpos) {
	// pos is now the position of the newline at the end of the headers.
	saved_hdrs->AlwaysCopyText(0, txt, 0, pos);
	txt->AlwaysDeleteCharacters(0, pos);
    }

    messages::PrintPSDoc(outfile, width, height);

    /* Unperform the hackery from above. */
    if (glob) {
	glob->SetNewLeftMargin(basis_left, op_left, unit_left);
	glob->SetNewRightMargin(basis_right, op_right, unit_right);
    }
    // restore headers here.
    if (pos < endpos) {
	txt->AlwaysCopyText(0, saved_hdrs, 0, saved_hdrs->GetLength());
	SetTopPosition(pos);
    }
}
