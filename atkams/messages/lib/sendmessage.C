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

#include <andrewos.h> /* sys/file.h */

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/sendmessage.C,v 1.9 1995/11/07 20:17:10 robr Stab74 $";
#endif



#include <stdio.h>
#include <sys/param.h>
#include <util.h>
#include <pwd.h>
#include <ctype.h>

#include <fontdesc.H>
#include <sbutton.H>
#include <sbuttonv.H>
#include <text.H>
#include <textview.H>
#include <lpair.H>
#include <search.H>
#include <im.H>
#include <environment.H>
#include <errprntf.h>
#include <rect.h>

#ifdef ibm032
#include <ams.h>
typedef short Boolean;
#else
#include <cui.h>
#endif
#include <fdphack.h>
#include <mailconf.h>
#include <mail.h>
#include <dropoff.h>
#include <environ.H>
#include <message.H>

#include <ams.H>
#include <amsutil.H>
#include <text822.H>
#include <folders.H>
#define dontDefineRoutinesFor_captions
#include <captions.H>
#undef dontDefineRoutinesFor_captions
#include <sendmessage.H>

#include <unscribe.h>

/* constants for the Deliver() subroutine */
#define FORCE_ASK_ABOUT_FORMATTING 0
#define FORCE_SEND_FORMATTED 1
#define FORCE_SEND_UNFORMATTED 2



ATKdefineRegistry(sendmessage, view, sendmessage::InitializeClass);
#ifndef NORCSID
#endif
#ifdef ibm032
#else
#endif
void sendmessage_SetButtonFont(class sendmessage  *self, class fontdesc  *font);
void BSSM_HeadersFocus(class sendmessage  *sm);
void BSSM_BodyFocus(class sendmessage  *sm);
void BSSM_DownFocus(class sendmessage  *sm);
void BSSM_UpFocus(class sendmessage  *sm);
void HandleButton(class sbutton  *self, long Asendmessage, int  in, int  whichbut);
void ReadTemplate(class sendmessage  *sm);
void BSSM_FakeBug(class sendmessage  *sm, char  *txt);
int ComposeBugReport(class sendmessage  *sm);
void DirectlyInsertFile(class textview  *tv, class text  *t, char  *fname, int  pos);
void ForceSending(class sendmessage  *sendmessage);
void ForceStripping(class sendmessage  *sendmessage);
void sendmessage_DoDelivery(class sendmessage  *sendmessage);
int Deliver(class sendmessage  *sendmessage, int  formathandlingcode);
int SetSendingDot(class textview  *v, class text  *d);
static int Submit(class sendmessage  *sendmessage, Boolean  Unformat , int  Version , int  TrustDelivery, Boolean  UseMultipartFormat);
void SetNotModified(class sendmessage  *sendmessage);
static void UnlinkCKPFile(class sendmessage  *sendmessage);
void MakeHeaderFieldsBold(class sendmessage  *self);
void MakeOneHeaderFieldBold(class sendmessage  *self, int  pos);
int CheckAndCountRecipients(class sendmessage  *sm, int  *tot , int  *ext , int  *totformat , int  *totstrip , int  *tottrust);
int ValidateHeader(class sendmessage  *sm, char  *lookfor, int  *externalct , int  *totct , int  *formatct , int  *stripct , int  *trustct);
int RemoveUselessHeaderLines(class sendmessage  *sm);
int FileIntoFolder(class sendmessage  *sm, char  *name);
int ProduceUnscribedVersion(char  *FileName, FILE  *OutputFP);
void BSSM_SendmessageCompound(class sendmessage  *sm, char  *cmds);
void BSSM_SendmessageFoldersCompound(class sendmessage  *sm, char  *cmds);
void BSSM_SendmessageMessagesCompound(class sendmessage  *sm, char  *cmds);
void SBSSM_TextviewCompound(class textview  *tv, char  *cmds);
void SBSSM_DoBodiesCommand(class sendmessage  *sm, char  *cmds);
void SBSSM_DoHeadersCommand(class sendmessage  *sm, char  *cmds);
void RestoreFromPS(class sendmessage  *self);
int AlreadyPS(char  *subj);
void SaveForPS(class sendmessage  *self);
void sendmessage_DuplicateWindow(class sendmessage  *self);

extern Boolean OneTimeProcInit(struct ATKregistryEntry  *c);
extern void InitProcStuff(class sendmessage  *sendmessage);
extern void InitStylesAndFonts(class sendmessage  *sendmessage);
extern void DestroyProcStuff(class sendmessage  *self);
extern void DestroyStyles(class sendmessage  *self);
extern void SetMyFrameTitle(class sendmessage  *sm, char *tit);
extern void PrepareBodyForSignature(class sendmessage  *self);
extern int EnvViewCt(class environment  *env);
extern int WriteOneFile(class sendmessage  *sendmessage, char  *ViceFileName, Boolean  OnVice , Boolean  MayOverwrite , int  Version , Boolean  TrustDelivery , Boolean  UseMultipartFormat, int  *EightBitText);
extern int delete_sendmsg_win(class im  *im, class sendmessage  *self);

void sendmessage_SetButtonFont(class sendmessage  *self, class fontdesc  *font)
{
    sbutton::GetFont((self->buttons)->ButtonData() ->prefs)=font;
}

void sendmessage::LinkTree(class view  *parent)
{
    (this)->view::LinkTree(parent);
    if (this->SendLpair != NULL)  {
	(this->SendLpair)->LinkTree( this);
    }
}

void sendmessage::UnlinkTree()
{
    (this)->view::UnlinkTree();
    if (this->SendLpair) {
	(this->SendLpair)->UnlinkTree();
    }
}

void sendmessage::AddHeaderLine(char  *headerline)
{
    class textview *v = this->HeadTextview;
    int len;

    len = strlen(headerline);
    (v)->SetDotPosition(0);
    (this->HeadText)->InsertCharacters( 0, headerline, len);
    (this->HeadText)->InsertCharacters( len, "\n", 1);
    (v)->SetDotPosition( len);
    (v)->FrameDot( len);
    (v)->WantUpdate( v);
    MakeOneHeaderFieldBold(this, 0);
}

void BSSM_HeadersFocus(class sendmessage  *sm)
{
    (sm->HeadTextview)->WantInputFocus( sm->HeadTextview);
}

void BSSM_BodyFocus(class sendmessage  *sm)
{
    (sm->BodyTextview)->WantInputFocus( sm->BodyTextview);
}

void BSSM_DownFocus(class sendmessage  *sm)
{
    class im *myim = (sm)->GetIM();

    if (myim && ((myim)->GetInputFocus() == (class view *) sm->HeadTextview)) {
	(sm->BodyTextview)->WantInputFocus( sm->BodyTextview);
    } else if (sm->foldersp) {
	ams::Focus(sm->foldersp);
    } else {
	ams::Focus((sm)->NewFoldersInNewWindow());
    }
}

void BSSM_UpFocus(class sendmessage  *sm)
{
    class im *myim = (sm)->GetIM();

    if (myim && ((myim)->GetInputFocus() == (class view *) sm->BodyTextview)) {
	(sm->HeadTextview)->WantInputFocus( sm->HeadTextview);
    } else if (sm->foldersp) {
	class view *v;

	if (sm->foldersp->mycaps) {
	    if (sm->foldersp->mycaps->BodView) {
		v = (class view *) sm->foldersp->mycaps->BodView;
	    } else {
		v = (class view *) sm->foldersp->mycaps;
	    }
	} else {
	    v = (class view *) sm->foldersp;
	}
	ams::Focus(v);
    } else {
	ams::Focus((sm)->NewFoldersInNewWindow());
    }
}

static char dummyname[]="";

struct sbutton_list blist[]={
    {dummyname, (long)0, NULL, FALSE}, /* Reset */

    {dummyname, (long)EXP_SIGNMAIL, NULL, FALSE}, /* Will/Won't sign */

    {dummyname, (long)EXP_HIDEAFTER, NULL, FALSE}, /* Will/Won't hide */

    {dummyname, (long)EXP_CLEARAFTER, NULL, FALSE}, /* Will/Won't clear */

    {dummyname, (long)EXP_KEEPBLIND, NULL, FALSE}, /* Will/Won't keep copy */

    {NULL, (long)0, NULL, FALSE}
};

void HandleButton(class sbutton  *self, long Asendmessage, int  in, int  whichbut)
{
    class sendmessage *sendmessage=(class sendmessage *)Asendmessage;
    char *Yes, *No;

    switch (whichbut) {
	case 0:
	    amsutil::SetOptBit(EXP_CLEARAFTER, amsutil::GetPermOptBit(EXP_CLEARAFTER));
	    amsutil::SetOptBit(EXP_SIGNMAIL, amsutil::GetPermOptBit(EXP_SIGNMAIL));
	    amsutil::SetOptBit(EXP_HIDEAFTER, amsutil::GetPermOptBit(EXP_HIDEAFTER));
	    amsutil::SetOptBit(EXP_KEEPBLIND, amsutil::GetPermOptBit(EXP_KEEPBLIND));
	    (sendmessage)->CheckButtons();
	    (sendmessage)->Reset();
	    return /* 0 */;
	case EXP_CLEARAFTER:
	    Yes = "Will Clear";
	    No = "Won't Clear";
	    break;
	case EXP_SIGNMAIL:
	    Yes = "Will Sign";
	    No = "Won't Sign";
	    break;
	case EXP_HIDEAFTER:
	    Yes = "Will Hide";
	    No = "Won't Hide";
	    break;
	case EXP_KEEPBLIND: 
	    Yes = "Will Keep Copy";
	    No = "Won't Keep Copy";
	    break;
	default: 
	    return /* 0 */;
    }
    if (amsutil::GetOptBit(whichbut)) {
	amsutil::SetOptBit(whichbut, 0);
	(self)->SetLabel( in, No);
    } else {
	amsutil::SetOptBit(whichbut, 1);
	(self)->SetLabel( in, Yes);
    }
    return /* 0 */;
}


void ReadTemplate(class sendmessage  *sm)
{
    if (sm->NeedsTemplate) {
	if ((sm->BodyText)->ReadTemplate( "messages", FALSE)) {
	    if (errno == ENOENT) {
		message::DisplayString(sm, 99, "Warning: No messages template on your template path; no styles available!");
	    } else {
		(ams::GetAMS())->ReportError( "Cannot open messages template.", ERR_WARNING, FALSE, 0);
	    }
	} else {
	    sm->NeedsTemplate = 0;
	}
    }
}

void sendmessage::SetFoldersView(class folders  *fold)
{
    this->foldersp = fold;
}

sendmessage::sendmessage()
{
	ATKinit;

    int lpaircount=0;
    class sbutton *bs;
    class lpair *headeroptlp;

    InitProcStuff(this);
    this->HasSigned = 0;
    this->myframe = NULL;
    this->CKPFileName = NULL;
    this->PSMsg = NULL;
    this->CurrentState = SM_STATE_NOSTATE;
    this->HeadModified = this->BodyModified = 0;
    this->foldersp = NULL;
    this->HeadText = new text;
    this->BodyText = new text;
    this->NeedsTemplate = 1;
    this->HeadTextview = new textview;
    (this->HeadTextview)->SetDataObject( this->HeadText);
    this->HeadScroll = (class scroll *) (this->HeadTextview)->GetApplicationLayer();
    this->BodyTextview = new textview;
    (this->BodyTextview)->SetDataObject( this->BodyText);
    this->BodyScroll = (class scroll *) (this->BodyTextview)->GetApplicationLayer();

    this->prefs = sbutton::GetNewPrefs("sendoptions");
    if(this->prefs) {
	if(sbutton::GetFont(this->prefs)==NULL) sbutton::GetFont(this->prefs)=fontdesc::Create("andy", fontdesc_Bold, 10);
	sbutton::InitPrefs(this->prefs, "sendoptions");
	this->buttons = sbuttonv::CreateFilledSButtonv("sbuttonv", this->prefs, blist);
	if(!this->buttons) THROWONFAILURE( FALSE);
	bs=(this->buttons)->ButtonData();

	(this->buttons)->vborder = environ::GetProfileInt("sendoptionsborder", 0);
	(this->buttons)->hborder = (this->buttons)->GetVBorder();
	(this->buttons)->vspacing = environ::GetProfileInt("sendoptionspadding", 0);
	(this->buttons)->hspacing = (this->buttons)->GetVSpacing();

	(bs)->GetMattePrefs() = sbutton::DuplicatePrefs(this->prefs, "sendoptionsmatte");
	if((bs)->GetMattePrefs()) {
	    sbutton::GetStyle((bs)->GetMattePrefs())=0;
	    sbutton::InitPrefs((bs)->GetMattePrefs(), "sendoptionsmatte");
	}

    } else THROWONFAILURE( FALSE);


    (bs)->SetLabel( SM_BLIND, (char *)(amsutil::GetOptBit(EXP_KEEPBLIND) ? "Will Keep Copy" : "Won't Keep Copy"));
    (bs)->SetLabel( SM_CLEAR,  (char *)(amsutil::GetOptBit(EXP_CLEARAFTER) ?  "Will Clear" : "Won't Clear"));
    (bs)->SetLabel( SM_SIGN, (char *)(amsutil::GetOptBit(EXP_SIGNMAIL) ?  "Will Sign" : "Won't Sign"));
    (bs)->SetLabel( SM_HIDE, (char *)(amsutil::GetOptBit(EXP_HIDEAFTER) ? "Will Hide" : "Won't Hide"));
    (bs)->SetLabel( SM_RESET, "Reset");
    (bs)->GetHitFunc()=HandleButton;
    (bs)->GetHitFuncRock()=(long)this;

#define HACKMAXLPAIRS 5
    this->randomlpairs=(class lpair **)malloc(sizeof(class lpair *)*(HACKMAXLPAIRS+1));

#define ADDLPAIR(l) do {\
    if(this->randomlpairs && lpaircount<HACKMAXLPAIRS) {\
    this->randomlpairs[lpaircount++]=(l);\
    this->randomlpairs[lpaircount]=NULL;\
    }\
} while(FALSE)

    ADDLPAIR(headeroptlp = new lpair);

#undef ADDLPAIR
#undef HACKMAXLPAIRS

    (headeroptlp)->HFixed( (class view *)this->HeadScroll, this->buttons, 115, 1);

    this->SendLpair = new lpair;
    (this->SendLpair)->VSplit( headeroptlp, (class view *)this->BodyScroll, 75, 1);

    InitStylesAndFonts(this);
    SetNotModified(this);

    (this->HeadTextview)->WantInputFocus( this->HeadTextview);
    ams::AddCheckpointSendmessage(this);

    THROWONFAILURE((TRUE));
}

sendmessage::~sendmessage()
{
	ATKinit;
   /* This is a bit bogus, because there isn't really a sendmessage dataobject/view split */
    class lpair **lps=this->randomlpairs;


    /* OK, OK, I give up... hack around lpair's desire to unlinktree its children when it is finalized.... rr2b */
    if(lps) while(*lps) {
	(*lps)->SetNth( 0, NULL);
	(*lps)->SetNth( 1, NULL);
	lps++;
    }


    ams::RemoveCheckpointSendmessage(this);
    if (this->PSMsg) {
	free(this->PSMsg);
	this->PSMsg = NULL;
    }
    if (this->CKPFileName) {
	free(this->CKPFileName);
	this->CKPFileName = NULL;
    }
    if (this->foldersp) {
	(this->foldersp)->SetSendmessage( NULL);
    }
    DestroyProcStuff(this);
    DestroyStyles(this);
    (this->SendLpair)->Destroy();

    lps=this->randomlpairs;
    if(lps) while(*lps) {
	(*lps)->Destroy();
	lps++;
    }

    if(this->randomlpairs) {
	free(this->randomlpairs);
	this->randomlpairs=NULL;
    }
    (this->HeadTextview)->DeleteApplicationLayer( (class view *)this->HeadScroll);
    (this->BodyTextview)->DeleteApplicationLayer( (class view *)this->BodyScroll);
    if(this->buttons) {
	class sbutton *bs=(this->buttons)->ButtonData();
	if(bs) (bs)->Destroy();
	(this->buttons)->Destroy();
    }
    if(this->prefs) sbutton::FreePrefs(this->prefs);
    /*   this is NOT currently needed:
      if(self->matteprefs) sbutton_FreePrefs(self->matteprefs); */

    (this->HeadText)->Destroy();
    (this->BodyText)->Destroy();
    (this->HeadTextview)->Destroy();
    (this->BodyTextview)->Destroy();
}

void sendmessage::SetCurrentState(int state)
{
    char *tit;

    if (this->CurrentState == state) return;
    this->CurrentState = state;
    switch(state) {
	case SM_STATE_NOSTATE:
	    tit = "Uninitialized";
	    break;
	case SM_STATE_READY:
	    tit = "Starting Fresh";
	    break;
	case SM_STATE_INPROGRESS:
	    tit = "Composing";
	    break;
	case SM_STATE_SENDING:
	    tit = "Sending/Posting";
	    break;
	case SM_STATE_SENT:
	    tit = "Sent/Posted";
	    break;
	case SM_STATE_VALIDATING:
	    tit = "Validating";
	    break;
	case SM_STATE_VALIDATED:
	    tit = "Validated";
	    break;
	case SM_STATE_VALIDATEFAILED:
	    tit = "Validation Failed";
	    break;
	default:
	    tit = "Unknown state";
	    break;
    }
    if (this->myframe) SetMyFrameTitle(this, tit);
}

void sendmessage::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
{
    struct rectangle myrect;

    rectangle_SetRectSize(&myrect, 0, 0, (this)->GetLogicalWidth(), (this)->GetLogicalHeight());
    (this->SendLpair)->InsertView( this, &myrect);
    (this->SendLpair)->FullUpdate( type, left, top, width, height);
}

void sendmessage::Update()
{
    (this->SendLpair)->Update();
    if ((this->CurrentState != SM_STATE_INPROGRESS && this->CurrentState != SM_STATE_SENDING && this->CurrentState != SM_STATE_VALIDATING) && (this)->HasChanged()) {
	(this)->SetCurrentState( SM_STATE_INPROGRESS);
    }
}

class view *
sendmessage::Hit(enum view_MouseAction  action, long  x , long  y , long  NumberOfClicks)
{
    return((this->SendLpair)->Hit( action, x, y, NumberOfClicks));
}

void BSSM_FakeBug(class sendmessage  *sm, char  *txt)
{
    (ams::GetAMS())->ReportError( txt ? txt : (char *)"One of my bits is missing!  Call an ambulance!", ERR_CRITICAL, FALSE, 0);
}

int ComposeBugReport(class sendmessage  *sm)
{
    char FileName[1+MAXPATHLEN];
    FILE *fp;

    if ((sm)->HasChanged()) {
	if (!(sm)->AskEraseUnsentMail()) {
	    return(0);
	}
	(sm)->Clear();
    }
    (ams::GetAMS())->CUI_GenLocalTmpFileName( FileName);
    fp = fopen(FileName, "w");
    if (!fp) {
	return -1;
    }
    fputs("To: ", fp);
    fputs((ams::GetAMS())->MessagesAutoBugAddress(), fp);
    fputs("\nContent-Type: X-BE2; 12\nSubject: \nCC:\n\n\\begindata{text, 42}\n\\textdsversion{12}\n\\template{messages}\n\\majorheading{Messages/Sendmessage Bug Report Form}\n\n\nThe maintainers of this program are sincerely sorry if you have encountered a bug.  By filling out this report carefully and completely, you may be able to help us fix the bug relatively quickly.\n\n\n \\bold{Please enter a description of the bug}:\n\n\n\n \\bold{If the bug did not occur on this machine, please give the machine and CPU type, or enter 'uncertain'}:\n\n\n\n \\bold{If the bug did not occur using this version of the program, please state which version you were using below, or enter 'uncertain'}:\n\n\n\n \\bold{If you are running the Console program, please use the 'Write Log File' option and then insert the resulting file below}:\n\n\n\n \\italic{Thank you for your assistance and patience.  Below this point, you will see information that has been automatically added by the system in order to help understand the bug; please do not make any changes below this point.}.\n\n\\enddata{text, 42}", fp);
    fclose(fp);
    (sm)->ReadFromFile( FileName, TRUE);
    (sm)->AppendBugInfoToBody( TRUE);

    return 0;
}

int sendmessage_wrap_AppendBugInfoToBody(class sendmessage *self, Boolean  IsMessagesBug)
{
    return self->AppendBugInfoToBody(IsMessagesBug);
}


int sendmessage::AppendBugInfoToBody(Boolean  IsMessagesBug)
{
    char FileName[1+MAXPATHLEN];
    FILE *fp;

    ams::WaitCursor(TRUE);
    (ams::GetAMS())->CUI_GenLocalTmpFileName( FileName);
    fp = fopen(FileName, "w");
    if (!fp) {
	ams::WaitCursor(FALSE);
	message::DisplayString(this, 10, "Could not open temporary file to write out bug report statistics--sorry.");
	return -1;
    }
    (ams::GetAMS())->WriteOutUserEnvironment( fp, IsMessagesBug);
    fclose(fp);
    DirectlyInsertFile(this->BodyTextview, this->BodyText, FileName, (this->BodyText)->GetLength());
    unlink(FileName);
    ams::WaitCursor(FALSE);
    return(0);
}    

void DirectlyInsertFile(class textview  *tv, class text  *t, char  *fname, int  pos)
{
    FILE *fp;

    fp = fopen(fname, "r");
    if (!fp) {
	char ErrorText[100+MAXPATHLEN];
	sprintf(ErrorText, "The file %s could not be inserted.", (ams::GetAMS())->ap_Shorten( fname));
	message::DisplayString(tv, 50, ErrorText);
	im::ForceUpdate();
	return;
    }
    (t)->AlwaysInsertFile( fp, fname, pos);
    fclose(fp);
}


void ForceSending(class sendmessage  *sendmessage)
{
    /* return */ (Deliver(sendmessage, FORCE_SEND_FORMATTED));
}

void ForceStripping(class sendmessage  *sendmessage)
{
    /* return */ (Deliver(sendmessage, FORCE_SEND_UNFORMATTED));
}

void sendmessage_DoDelivery(class sendmessage  *sendmessage)
{
    /* return */ (Deliver(sendmessage, FORCE_ASK_ABOUT_FORMATTING));
}

/* Constants to declare use of ATK or new multipart datastream */
#define MAILFORMAT_UNDEFINED 0
#define	MAILFORMAT_ANDREW 1	/* use atk datastream */
#define	MAILFORMAT_MULTIPART 2	/* use new mail standard */
#define	MAILFORMAT_MULTIPARTNOASK 3 /* New mail standard for ALL recips, without even asking about their receiving capability */
#define	MAILFORMAT_ASK 4	/* ask which data format to use */

int Deliver(class sendmessage  *sendmessage, int  formathandlingcode)
{
    int code, total, external, ans, StreamVersion = 12, TrustDelivery = 0, nkids, format, strip, trust, ThisMailFormat;
    Boolean Unformat = FALSE, SendingWithAMSDel;
    static int MailFormatToUse = MAILFORMAT_UNDEFINED;
    static char *ExternalQVec[] = {
	"The readers of this message may not recognize Andrew formatting.",
	"Cancel sending",
	"Remove formatting & send",
	"Send with formatting",
	NULL,	/* qv[4] is *always* set below */
	NULL
    };

    if (!(sendmessage)->HasChanged() && !(ams::GetAMS())->GetBooleanFromUser( "Message unchanged from last delivery/draft.  Send anyway", FALSE)) {
	return(0);
    }
    message::DisplayString(sendmessage, 10, "Message delivery in progress");
    if (MailFormatToUse == MAILFORMAT_UNDEFINED) {
	int len;
	const char *fmt = environ::GetProfile("messages.mailsendingformat");
	if (!fmt) fmt = environ::GetConfiguration("mailsendingformat");
	if (!fmt) {
	    MailFormatToUse = MAILFORMAT_ASK;
	} else {
	    len = strlen(fmt);
	    while (*fmt && isspace(*fmt)) ++fmt;
	    if (!amsutil::lc2strncmp("atk", fmt, len)
		|| !amsutil::lc2strncmp("andrew", fmt, len)) {
		MailFormatToUse = MAILFORMAT_ANDREW;
	    } else if (!amsutil::lc2strncmp("mime", fmt, len)
		       || !amsutil::lc2strncmp("multipart", fmt, len)) {
		MailFormatToUse = MAILFORMAT_MULTIPART;
	    } else if (!amsutil::lc2strncmp("mime-force", fmt, len)
		       || !amsutil::lc2strncmp("multipart-force", fmt, len)) {
		MailFormatToUse = MAILFORMAT_MULTIPARTNOASK;
	    } else if (!amsutil::lc2strncmp("ask", fmt, len)) {
		MailFormatToUse = MAILFORMAT_ASK;
	    } else {
		message::DisplayString(sendmessage, 10, "Unrecognized 'mailsendingformat' configuration is being ignored.");

		MailFormatToUse = MAILFORMAT_ASK;
	    }
	}
    }
    ThisMailFormat = MailFormatToUse;
    total = external = format = strip = trust = 0;
    code = CheckAndCountRecipients(sendmessage, &total, &external, &format, &strip, &trust);
    if (code) {
	return(code);
    }
    if (amsutil::GetOptBit(EXP_SIGNMAIL) && !sendmessage->HasSigned) {
	FILE *fp;
	char fname[1+MAXPATHLEN], *fnamepattern;
	char home[1+MAXPATHLEN];
	
	strcpy(home,getenv("HOME"));

	nkids = (sendmessage->BodyText->rootEnvironment)->NumberOfChildren();
	/* Note that we will recalculate this value after inserting the signature file */
	fnamepattern = (char *)((nkids > 0) ? ".sig.fmt" : ".sig");
	sprintf(fname,"%s/%s",home,fnamepattern);
	fp = fopen(fname, "r");
	if (!fp) {
	sprintf(fname,"%s/.signature",home);
	    fp = fopen(fname, "r");
	}
	if (!fp) {
	    static char *NoSignatureQVec[] = {
		"The file '~/.signature' could not be read",
		"Send unsigned",
		"Do not send",
		NULL
	    };
	    if ((ams::GetAMS())->ChooseFromList( NoSignatureQVec, 2) != 1) return(-1);
	} else {
	    int len;
	    PrepareBodyForSignature(sendmessage);
	    len = (sendmessage->BodyText)->GetLength();
	    (sendmessage->BodyText)->AlwaysInsertFile( fp, fname, len);
	    (sendmessage->BodyTextview)->WantUpdate( sendmessage->BodyTextview);
	    fclose(fp);
	    sendmessage->HasSigned = 1;
	}
    }
    nkids = (sendmessage->BodyText->rootEnvironment)->NumberOfChildren();
    switch ((ams::GetAMS())->CUI_DeliveryType()) {
	case DT_AMS: case DT_AMSWAIT:
	    SendingWithAMSDel = TRUE; break;
	case DT_NONAMS:
	    SendingWithAMSDel = FALSE; break;
	default:
	    SendingWithAMSDel = ((ams::GetAMS())->CUI_UseAmsDelivery() >= 0);
    }
    if (formathandlingcode == FORCE_SEND_FORMATTED) {
	ans = 3;
    } else if (formathandlingcode == FORCE_SEND_UNFORMATTED) {
	ans = 2;
    } else if (nkids == 0) {
	ans = SendingWithAMSDel ? 4 : 2; /* trust delivery : unformatted */
	StreamVersion = 12;
    } else if (external > 0) {
	if (external == format) {
	    ans = 3;
	} else if (external == trust) {
	    ans = 4;
	} else if ((external == strip) && (external == total)) {
	    ans = 2;
	} else if (ThisMailFormat == MAILFORMAT_MULTIPARTNOASK) {
	    ans = 3;
	    ThisMailFormat = MAILFORMAT_MULTIPART; /* Probably redundant */
	} else if (ThisMailFormat == MAILFORMAT_ASK) {
	    static char *Question[] = {
		"The readers of this message may not recognize multimedia formatting.",
		"Cancel sending",
		"Remove formatting & send",
		"Send with formatting in Andrew Format",
		"Send with formatting in MIME (Internet standard) format",
		NULL
	    };
	    ans = (ams::GetAMS())->ChooseFromList( Question, 1);
	    if (ans == 4) {
		ans = 3;
		ThisMailFormat = MAILFORMAT_MULTIPART;
	    } else if (ans == 3) {
		ThisMailFormat = MAILFORMAT_ANDREW;
	    }
	} else {
#ifdef CMU_ENV
	    ans = 4;		/* Always trust the delivery system */
#else
	    ExternalQVec[4] = (char *)(SendingWithAMSDel ? "Trust the delivery system to remove it as needed" : NULL);
	    ans = (ams::GetAMS())->ChooseFromList( ExternalQVec, 1);
#endif
	}
    } else {
	ans = 4; /* Local recipients, Raw, raw, raw! */
	StreamVersion = 12;
    }
    switch(ans) {
	case 1:
	    message::DisplayString(sendmessage, 10, "Message not sent.");
	    return(-1);
	case 2:
	    /* send unformatted */
	    StreamVersion = 0;
	    ThisMailFormat = MAILFORMAT_ANDREW; /* makes unformat work right */
	    Unformat = TRUE;
	    break;
	case 3:
	    /* send raw */
	    break;
	case 4:
	    /* Trust delivery system */
	    TrustDelivery = 1;
	    break;
	default:
	    message::DisplayString(sendmessage, 10, "Unrecognized choice code; message not sent.");
	    return(-1);
    }
    if (nkids <= 0) ThisMailFormat = MAILFORMAT_ANDREW; /* handled right  there */
    if (!Unformat && ThisMailFormat == MAILFORMAT_ASK) {
	static char *whichfmt[] = {
	    "Which mail data format do you want to use?",
	    "Andrew 'native' data stream",
	    "MIME (Internet standard) format",
            "Cancel",
	    NULL
	};
	ans = (ams::GetAMS())->ChooseFromList( whichfmt, 1);
	if (ans == 3) {
	    message::DisplayString(sendmessage, 10, "Message not sent.");
	    return(-1);
	}
	ThisMailFormat = (ans == 1) ? MAILFORMAT_ANDREW : MAILFORMAT_MULTIPART;
    }
    ams::WaitCursor(TRUE);
    message::DisplayString(sendmessage, 10, "Sending message; please wait...");
    if (Submit(sendmessage, Unformat, StreamVersion, TrustDelivery, ((ThisMailFormat == MAILFORMAT_MULTIPART) || ThisMailFormat == MAILFORMAT_MULTIPARTNOASK)) == 0) {
	class im *myim = (sendmessage)->GetIM();

	if (myim && amsutil::GetOptBit(EXP_HIDEAFTER)) {
	    if (amsutil::GetOptBit(EXP_VANISH)) {
		(myim)->VanishWindow();
	    } else {
		(myim)->HideWindow();
	    }
	    if (sendmessage->foldersp) {
		(sendmessage->foldersp)->GrowWindow();
	    }
	}
	if (amsutil::GetOptBit(EXP_CLEARAFTER)) {
	    (sendmessage)->Reset();
	}
    }
    ams::WaitCursor(FALSE);
    im::ForceUpdate();
    return(0);
}

int sendmessage::Clear()
{
    (this->BodyText)->ClearCompletely();
    (this->HeadText)->ClearCompletely();
    (this->BodyText)->SetGlobalStyle( this->DefaultStyle);
    (this->HeadText)->SetGlobalStyle( this->DefaultHeadStyle);
    this->NeedsTemplate = TRUE;
    ReadTemplate(this);
    text822::ResetGlobalStyle((class text822 *)(this->BodyText));

    (this->BodyTextview)->WantUpdate( this->BodyTextview);
    (this->HeadTextview)->WantUpdate( this->HeadTextview);

    SetNotModified(this);
    (this)->SetCurrentState( SM_STATE_READY);
    message::DisplayString(this, 10, "Ready to send a new message");
    (this->HeadTextview)->WantInputFocus( this->HeadTextview);
    this->HasSigned = 0;
    return(0);
}

#define MAXHEADERLINE 1000

int sendmessage::ReadFromFile(char  *SourceFile, Boolean  DeleteIt)
{
    FILE *fp;
    int len, start, ig;
    char ErrorText[256], MyName[1+MAXPATHLEN];

    if ((this)->HasChanged() && !(this)->AskEraseUnsentMail()) return(0);
    if (!(ams::GetAMS())->CUI_OnSameHost() && access(SourceFile, R_OK)) {
	(ams::GetAMS())->CUI_GenTmpFileName( MyName);
	if ((ams::GetAMS())->CUI_GetFileFromVice( MyName, SourceFile)) {
	    return(-1);
	}
    } else {
	strcpy(MyName, SourceFile);
    }
    if ((fp = fopen(MyName, "r")) == NULL) {
	if (errno == ENOENT) {
	    sprintf(ErrorText, "There is no file named '%s'.", (ams::GetAMS())->ap_Shorten( MyName));
	    message::DisplayString(this, 10, ErrorText);
	} else {
	    sprintf(ErrorText, "Cannot open source file %s", (ams::GetAMS())->ap_Shorten( MyName));
	    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, FALSE, 0);
	}
	return(-1);
    }
    if ((this)->Clear()) {
	fclose(fp);
	return(-1);
    }
    if (!text822::ReadIntoText((class text822 *)(this->BodyText), fp, MODE822_NORMAL|MODE822_NOAUTOFIX, NULL, &len, TRUE, &start, &ig, this->HeadText)) {
	(ams::GetAMS())->ReportError( "Could not read in text822 draft message", ERR_WARNING, FALSE, 0);
	fclose(fp);
	return(-1);
    }
    fclose(fp);
    if (!(ams::GetAMS())->CUI_OnSameHost() && access(SourceFile, R_OK)) {
	unlink(MyName);
    }
    if (DeleteIt) {
	/* Try local delete first */
	if (unlink(SourceFile)) {
	    long mserrcode; 
	    if (mserrcode = (ams::GetAMS())->MS_UnlinkFile( SourceFile)) {
		(ams::GetAMS())->ReportError( "Could not unlink a temporary file", ERR_WARNING, TRUE, mserrcode);
	    }
	}
    }
    (this)->ResetSendingDot();
    SetNotModified(this);
    MakeHeaderFieldsBold(this);
    (this->HeadTextview)->WantUpdate( this->HeadTextview);
    (this->BodyTextview)->WantUpdate( this->BodyTextview);
    return(0);
}

/* The following function returns 1 if all headers are filled in, zero
    otherwise.  It also sets the dot at the first empty header.  */

int SetSendingDot(class textview  *v, class text  *d)
{
    register int pos, len, c, lastnewline = 0;
    Boolean FoundIt = FALSE, JustSawAColon = FALSE;
    int numcolons = 0;

    for (pos = 0, len = (d)->GetLength(); !FoundIt && pos < len; ++pos) {
	c = (d)->GetChar( pos);
	switch(c) {
	    case ':':
		JustSawAColon = TRUE;
		++numcolons;
		break;
	    case '\012':
	    case '\015':
		if ((numcolons == 1) && JustSawAColon
		    /* Special hack to ignore empty CC header */
		    && ((pos - lastnewline) < 4 
			|| (d)->GetChar( ++lastnewline) != 'C'
			|| (d)->GetChar( ++lastnewline) != 'C'
			|| (d)->GetChar( ++lastnewline) != ':'))
		{
		    FoundIt = TRUE;
		}
		lastnewline = pos;
		numcolons = 0;
		JustSawAColon = FALSE;
		break;
	    case ' ':
	    case '\t':
		break;
	    default: 
		JustSawAColon = FALSE;
		break;
	}
    }
    if (pos-- < len) {
	(v)->SetDotPosition( pos);
	(v)->FrameDot( pos); 
	return(0);
    } else {
	(v)->SetDotPosition( 0);
	(v)->FrameDot( 0);
	return(1);
    }
}

static int Submit(class sendmessage  *sendmessage, Boolean  Unformat , int  Version , int  TrustDelivery, Boolean  UseMultipartFormat)
{
    static int flags = 0;
    char FileName[MAXPATHLEN+1];
    int EightBitText=0;

    if (((sendmessage->BodyText)->GetLength() < 2) && !amsutil::GetOptBit(EXP_SENDEMPTY)) {
	int kids;

	kids = EnvViewCt(sendmessage->BodyText->rootEnvironment);
	if (kids <= 0) {
	    message::DisplayString(sendmessage, 10, "You need at least 2 characters in the body of your message");
	    return(-1);
	}
    }
    (ams::GetAMS())->CUI_GenTmpFileName( FileName);

    RemoveUselessHeaderLines(sendmessage);
    if (WriteOneFile(sendmessage, FileName, TRUE, FALSE, Version, TrustDelivery, UseMultipartFormat, &EightBitText)) return(-1);
    UnlinkCKPFile(sendmessage);
    if (Unformat && !EightBitText) {
	int code = ProduceUnscribedVersion(FileName, NULL);
	if (code) {
	    char ErrorText[256];

	    sprintf(ErrorText, "Unformatting error (%d, %d); cannot produce unformatted text", code, errno);
	    message::DisplayString(sendmessage, 10, ErrorText);
	    return(-1);
	}
    }
    (sendmessage)->SetCurrentState( SM_STATE_SENDING);
    flags = amsutil::GetOptBit(EXP_KEEPBLIND) ? AMS_SEND_BLINDYES : AMS_SEND_BLINDNO;
    im::ForceUpdate();
    flags |= AMS_SEND_ILLEGAL; /* checking for illegal chars takes too long on multimedia stuff -- we now trust the _WriteOther routines instead */
    if ((ams::GetAMS())->CUI_SubmitMessage( FileName, flags)) {
	(sendmessage)->SetCurrentState( SM_STATE_INPROGRESS);
	return(-1);
    }
    (sendmessage)->SetCurrentState( SM_STATE_SENT);
    SaveForPS(sendmessage);
    SetNotModified(sendmessage);
    im::ForceUpdate();
    return(0);
}

void SetNotModified(class sendmessage  *sendmessage)
{
    sendmessage->HeadModified = sendmessage->HeadCheckpoint = (sendmessage->HeadText)->GetModified();
    sendmessage->BodyModified = sendmessage->BodyCheckpoint = (sendmessage->BodyText)->GetModified();
    UnlinkCKPFile(sendmessage);
}

int sendmessage::WriteFile(char  *ViceFileName)
{
    int code;

    code = WriteOneFile(this, ViceFileName, TRUE, FALSE, 12, 1, FALSE, NULL);
    if (!code) SetNotModified(this);
    return(code);
}

static void UnlinkCKPFile(class sendmessage  *sendmessage)
{
    if (sendmessage->CKPFileName) {
	if (unlink(sendmessage->CKPFileName)) { /* Try local unlink first */
	    (ams::GetAMS())->MS_UnlinkFile( sendmessage->CKPFileName);
	}
    }
}

void sendmessage_wrap_Reset(class sendmessage *self)
{
    self->Reset();
}

void sendmessage::Reset()
{
    static char *InitialText = "To: \nSubject: \nCC: ";

    if ((this)->HasChanged()) {
	if (!(this)->AskEraseUnsentMail()) {
	    return;
	}
    }
    (this)->Clear();
    (this->HeadText)->InsertCharacters( 0, InitialText, strlen(InitialText));
    SetSendingDot(this->HeadTextview, this->HeadText);
    SetNotModified(this);
    MakeHeaderFieldsBold(this);
    ((this)->GetIM())->SetDeleteWindowCallback( (im_deletefptr)delete_sendmsg_win, (long)this);
}


void MakeHeaderFieldsBold(class sendmessage  *self)
{
    int i, len;
    class text *t = self->HeadText;
    Boolean SawNewline = TRUE;
    char c;

    for (i=0, len = (t)->GetLength(); i < len; ++i) {
	c = (t)->GetChar( i);
	if (SawNewline) {
	    if (!isspace(c)) {
		MakeOneHeaderFieldBold(self, i);
	    }
	}
	SawNewline = (c == '\n');
    }
}

void MakeOneHeaderFieldBold(class sendmessage  *self, int  pos)
{
    int i, len;
    class text *t = self->HeadText;

    for (i=pos, len = (t)->GetLength(); i < len; ++i) {
	if ((t)->GetChar( i) == ':') { /* Found the end point */
	    class environment *et;

	    et = (t->rootEnvironment)->WrapStyle( pos, i-pos, self->BoldStyle);
	    (et)->SetStyle( (pos > 0), TRUE);
	    return;
	}
    }
}

void sendmessage::AddToToHeader(char  *line)
{
    static char ToStates[] = "\nTo:";
    int pos, len, state = 1;
    class textview *v;
    class text *d;

    v = this->HeadTextview;
    d = this->HeadText;
    for (pos = 0, len = (d)->GetLength(); pos < len; ++pos) {
	if ((d)->GetChar( pos) == ToStates[state]) {
	    ++state;
	} else {
	    state = 0;
	}
	if (state == 4) break;
    }
    for (++pos; pos < len && (d)->GetChar( pos) != '\n'; ++pos) {
    }
    (d)->InsertCharacters( pos, line, strlen(line));
    (v)->WantUpdate(v);
}

void sendmessage::ResetSendingDot()
{
    if (SetSendingDot(this->HeadTextview, this->HeadText)) {
	(this->BodyTextview)->WantInputFocus( this->BodyTextview);
    } else {
	(this->HeadTextview)->WantInputFocus( this->HeadTextview);
    }
}

int sendmessage::ResetFromParameters(char  *ToName , char  *Subject , char  *CC , char  *IncludeFile, int  Delete)
{
    FILE *fp;
    class text *d;
    class textview *bv, *hv;
    char ErrorText[256], BigBuf[1000];

    if ((this)->HasChanged()) {
	if (!(this)->AskEraseUnsentMail()) {
	    return 0;
	}
    }
    hv = this->HeadTextview;
    bv = this->BodyTextview;
    if ((this)->Clear()) {
	return(-1);
    }
    d = this->HeadText;
    sprintf(BigBuf, "To: %s\nSubject: %s\nCC: %s",
	     ToName ? ToName : "", Subject ? Subject : "", CC ? CC : "");
    (d)->InsertCharacters( 0, BigBuf, strlen(BigBuf));
    d = this->BodyText;
    if (IncludeFile && *IncludeFile) {
	if ((fp = fopen(IncludeFile, "r")) == NULL) {
	    if (errno == ENOENT) {
		sprintf(ErrorText, "There is no file named '%s'.", (ams::GetAMS())->ap_Shorten( IncludeFile));
		message::DisplayString(this, 10, ErrorText);
	    } else {
		sprintf(ErrorText, "Cannot open source file %s", (ams::GetAMS())->ap_Shorten( IncludeFile));
		(ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, FALSE, 0);
	    }
	    return(-1);
	}
	(d)->Read( fp, 0);
	fclose(fp);
    }
    (this)->ResetSendingDot();
    MakeHeaderFieldsBold(this);
    (hv)->WantUpdate( hv);
    (bv)->WantUpdate( bv);
    if (Delete && IncludeFile && *IncludeFile) unlink(IncludeFile);
    return(0);
}

int sendmessage_wrap_CheckRecipients(class sendmessage *self)
{
    return self->CheckRecipients();
}

int sendmessage::CheckRecipients()
{
    int tot, ext, format, strip, trust;

    return(CheckAndCountRecipients(this, &tot, &ext, &format, &strip, &trust));
}

int CheckAndCountRecipients(class sendmessage  *sm, int  *tot , int  *ext , int  *totformat , int  *totstrip , int  *tottrust)
{
    int code, code2, extct, extct2, totct, totct2, formatct, stripct, trustct, formatct2, stripct2, trustct2;
    char ErrorText[256];

    ams::WaitCursor(TRUE);
    message::DisplayString(sm, 10, "Validating recipient names; please wait.");
    (sm)->SetCurrentState( SM_STATE_VALIDATING);
    im::ForceUpdate();
    extct = extct2 = totct = totct2 = formatct = formatct2 = stripct = stripct2 = trustct = trustct2 = 0;
    code = ValidateHeader(sm, "\nTo:", &extct, &totct, &formatct, &stripct, &trustct);
    code2 = ValidateHeader(sm, "\nCC:", &extct2, &totct2, &formatct2, &stripct2, &trustct2);
    ams::WaitCursor(FALSE);
    im::ForceUpdate();
    *tot = totct + totct2;
    *ext = extct + extct2;
    *totformat = formatct + formatct2;
    *totstrip = stripct + stripct2;
    *tottrust = trustct + trustct2;
    (sm)->SetCurrentState( SM_STATE_VALIDATED);
    if (code || code2) {
	if (code >= 0 && code2 >= 0) {
	    code += code2;
	    if (*tot > 1) {
		if (*tot == code) {
		    strcpy(ErrorText, (code == 2) ? "Both" : "All");
		} else {
		    strcpy(ErrorText, amsutil::cvEng(code, 1, 1000));
		}
		strcat(ErrorText, " of the ");
		strcat(ErrorText, amsutil::cvEng(*tot, 0, 1000));
		strcat(ErrorText, " names you typed ");
		strcat(ErrorText, (code > 1) ? "are" : "is");
		strcat(ErrorText, " invalid.");
	    } else {
		strcpy(ErrorText, "The name you typed is invalid");
	    }
	    message::DisplayString(sm, 25, ErrorText);
	} else {
	    message::DisplayString(sm, 25, "Cannot parse the headers to validate them.");
	    code = -1;
	}
	(sm)->SetCurrentState( SM_STATE_VALIDATEFAILED);
	im::ForceUpdate();
	return(code);
    }
    if (*tot > 2) {
	sprintf(ErrorText, "All %s recipient names seem to be OK.", amsutil::cvEng(*tot, 0, 1000));
    } else if (*tot > 1) {
	strcpy(ErrorText, "Both recipient names seem to be OK.");
    } else {
	strcpy(ErrorText, "The recipient name you typed seems to be OK.");
    }
    message::DisplayString(sm, 10, ErrorText);
    im::ForceUpdate();
    return(0);
}

int ValidateHeader(class sendmessage  *sm, char  *lookfor, int  *externalct , int  *totct , int  *formatct , int  *stripct , int  *trustct)
{
    char *tp, *old, *new_c, *s, *realname;
    int tpos, pos, newpos, len, i, errct, c;
    class text *d;
    Boolean Searching = FALSE, SeeingAt = FALSE;
    class textview *v;
    struct SearchPattern *pat;

    v = sm->HeadTextview;
    d = sm->HeadText;
    pat = NULL;
    tp = search::CompilePattern(lookfor, &pat);
    if (tp != 0) {
	return(-1);
    }
    (d)->InsertCharacters( 0, "\n", 1);
    pos = search::MatchPattern(d, 0, pat);
    (d)->DeleteCharacters( 0, 1);
    if (pos-- < 0) {
	return(0);
    }
    pos += strlen(lookfor);
    while ((d)->GetChar( pos) == ' ') ++pos;
    tp = search::CompilePattern("\n", &pat);
    Searching = TRUE;
    tpos = pos;
    while (Searching) {
	newpos = search::MatchPattern(d, tpos, pat);
	if (newpos < 0) {
	    newpos = (d)->GetLength();
	    Searching = FALSE;
	}
	c = (d)->GetChar( newpos+1);
	if (c != ' ' && c != '\t') {
	    Searching = FALSE;
	}
	tpos = newpos+1;
    }
    len = newpos -pos;
    s = old = (char *)malloc(2+len);
    new_c = (char *)malloc(4*len);
    for (i = pos; i<newpos; ++i) {
	c = (d)->GetChar( i);
       /* this was a work around for an old be1 bug it should not be needed anymore
	   if (c == '@') {
	    if (SeeingAt) continue;
	    SeeingAt = TRUE;
	} else {
	    SeeingAt = FALSE;
	}
      */
	*s++ = c;
    }
    *s = '\0';
    s = (char *) amsutil::StripWhiteEnds(old);    
    if (!*s) {
	free(old);
	free(new_c);
	return(0);
    }
    *totct = *externalct = 0;
    errct = (ams::GetAMS())->CUI_RewriteHeaderLineInternal( s, &realname, 25, totct, externalct, formatct, stripct, trustct);
    if (realname) {
	(d)->DeleteCharacters( pos, len);
	(d)->InsertCharacters( pos, realname, strlen(realname));
	free(realname);
    }
    (v)->WantUpdate(v);
    free(old);
    free(new_c);
    return(errct);
}

int sendmessage::Checkpoint()
{
    static char *VCKPFileName = NULL, *TCKPFileName = NULL;
    char *ckpfile; /* was static??? */
    char Msg[100+MAXPATHLEN], TmpFileName[1+MAXPATHLEN];

    if (this->HeadCheckpoint == (this->HeadText)->GetModified() && this->BodyCheckpoint == (this->BodyText)->GetModified()) {
	return(0);
    }
    strcpy(Msg, "CKP: ");
    message::DisplayString(this, 10, Msg);
    im::ForceUpdate();
    if (amsutil::GetOptBit(EXP_CKPONTMP)) {
	if (!TCKPFileName) {
	    (ams::GetAMS())->CUI_GenTmpFileName( TmpFileName);
	    TCKPFileName = (char *)malloc(1+strlen(TmpFileName));
	    if (!TCKPFileName) {
		message::DisplayString(this, 99, "Out of memory!");
		return(-1);
	    }
	    strcpy(TCKPFileName, TmpFileName);
	}
	ckpfile = TCKPFileName;
    } else {
	if (!VCKPFileName) {
	    const char *s = environ::GetProfile("messages.checkpointdir");
	    if (!s) s = "~";
	    (ams::GetAMS())->TildeResolve( s, TmpFileName);
	    strcat(TmpFileName, "/");
	    strcat(TmpFileName, (ams::GetAMS())->ams_genid( 1));
	    strcat(TmpFileName, ".CKP");
	    VCKPFileName = (char *)malloc(1+strlen(TmpFileName));
	    if (!VCKPFileName) {
		message::DisplayString(this, 99, "Out of memory!");
		return(-1);
	    }
	    strcpy(VCKPFileName, TmpFileName);
	}
	ckpfile = VCKPFileName;
    }
    this->CKPFileName = (char *)malloc(1+strlen(ckpfile));
    if (!this->CKPFileName) {
	message::DisplayString(this, 99, "Out of memory!");
	return(-1);
    }
    strcpy(this->CKPFileName, ckpfile);
    if (!(WriteOneFile(this, this->CKPFileName, FALSE, TRUE, 12, 1, FALSE, NULL))) {
	this->HeadCheckpoint = (this->HeadText)->GetModified();
	this->BodyCheckpoint = (this->BodyText)->GetModified();
    }
/*    strcat(Msg, " done.  Wrote CKP file in ");*/
    strcat(Msg, (ams::GetAMS())->ap_Shorten( this->CKPFileName));
    strcat(Msg, ".");
    message::DisplayString(this, 10, Msg);
    im::ForceUpdate();
    return(0);
}

int RemoveUselessHeaderLines(class sendmessage  *sm)
{
    class text *d;
    char *tp;
    struct SearchPattern *pat;
    int pos, orgpos, c;

    d = sm->HeadText;
    /* First we get rid of the CC line if it is empty */
    pat = NULL;
    tp = search::CompilePattern("CC:", &pat);
    if (tp != 0) {
	return(-1);
    }
    orgpos = pos = search::MatchPattern(d, 0, pat);
    if (pos >= 0) {
	pos += 3;
	while ((c = (d)->GetChar( pos)) == ' ' || c == '\t') ++pos;
	if (c=='\n' || c == -1 || c == 255) {
	    /* Aha!  We need to remove this sucker! */
	    (d)->DeleteCharacters( orgpos, pos-orgpos+1);
	    (sm->HeadTextview)->WantUpdate( sm->HeadTextview);
	}
    }
    /* Now we get rid of any blank lines */
    pat = NULL;
    tp = search::CompilePattern("\n\n", &pat);
    if (tp != 0) {
	return(-1);
    }
    while (TRUE) {
	pos = search::MatchPattern(d, 0, pat);
	if (pos < 0) {
	    return(0);
	}
	(d)->DeleteCharacters( pos, 1);
    }
}

int FileIntoFolder(class sendmessage  *sm, char  *name)
{
    char ShortName[1+MAXPATHLEN], *FullName, FileName[1+MAXPATHLEN], TmpFileName[1+MAXPATHLEN];
    long mserrcode;

    if (name && *name != '?') {
	strcpy(ShortName, name);
    } else {
	if (ams::GetFolderName("Save draft in what folder? ", ShortName, sizeof(ShortName), name ? ++name : (char *)"", FALSE)) return(-1);
    }
    ams::WaitCursor(TRUE);
    if ((ams::GetAMS())->CUI_DisambiguateDir( ShortName, &FullName)) {
	ams::WaitCursor(FALSE);
	(ams::GetAMS())->CUI_ReportAmbig( ShortName, "folder");
	return(-1);
    }
    sprintf(TmpFileName, "~/SMTMP.%d", getpid());
    (ams::GetAMS())->TildeResolve( TmpFileName, FileName);
    (ams::GetAMS())->MS_UnlinkFile( FileName); /* Just to be sure */
    if ((sm)->WriteFile( FileName)) {
	ams::WaitCursor(FALSE);
	return(-1); /* error was reported */
    }
    mserrcode = (ams::GetAMS())->MS_AppendFileToFolder( FileName, FullName);
    ams::WaitCursor(FALSE);
    if (mserrcode) {
	(ams::GetAMS())->ReportError( "Could not append message draft to folder", ERR_WARNING, TRUE, mserrcode);
	(ams::GetAMS())->MS_UnlinkFile( FileName); /* ignore errors */
	return(-1);
    }
    /* Use FileName as scratchpad for message now */
    sprintf(FileName, "Filed message draft into folder: '%s'.", ShortName);
    message::DisplayString(sm, 10, FileName);
    SetNotModified(sm);
    return(0);
}


void sendmessage::CheckButtons()
{
    class sbutton *bs=(this->buttons)->ButtonData();
    (bs)->SetLabel( SM_CLEAR, (char *)(amsutil::GetOptBit(EXP_CLEARAFTER) ? "Will Clear" : "Won't Clear"));
    (bs)->SetLabel( SM_SIGN, (char *)(amsutil::GetOptBit(EXP_SIGNMAIL) ? "Will Sign" : "Won't Sign"));
    (bs)->SetLabel( SM_HIDE,  (char *)(amsutil::GetOptBit(EXP_HIDEAFTER) ? "Will Hide" : "Won't Hide"));
    (bs)->SetLabel( SM_BLIND, (char *)(amsutil::GetOptBit(EXP_KEEPBLIND) ? "Will Keep Copy" : "Won't Keep Copy"));
}


/* Convert the named file into an unscribed file */
int ProduceUnscribedVersion(char  *FileName, FILE  *OutputFP)
{
    char TmpFileName[1+MAXPATHLEN], LineBuf[5000];
    FILE *fin, *fout;
    int ct, ucode;
    struct ScribeState *ScribeState;
    char MyName[1+MAXPATHLEN];
    int sameHost = (ams::GetAMS())->CUI_OnSameHost();

    if (!OutputFP) {
	(ams::GetAMS())->CUI_GenLocalTmpFileName( TmpFileName);
	if (!sameHost) {
	    (ams::GetAMS())->CUI_GenTmpFileName( MyName);
	    if ((ams::GetAMS())->CUI_GetFileFromVice( MyName, FileName)) {
		return(-1);
	    }
	}
	else strcpy(MyName, FileName);
	fout = fopen(TmpFileName, "w");
	if (!fout) {
	    return(-2);
	}
    } else {
	fout = OutputFP;
	strcpy(MyName, FileName);
    }
    fin = fopen(MyName, "r");
    if (!fin) {
	if (fout != OutputFP) fclose(fout);
	return(-1);
    }
    if (!OutputFP) {
	while (fgets(LineBuf, sizeof(LineBuf), fin)) {
	    fputs(LineBuf, fout);
	    if (LineBuf[0] == '\n') break;
	}
    }
    ucode = (ams::GetAMS())->UnScribeInit( "12", &ScribeState);
    while ((ct = fread(LineBuf, sizeof(char), sizeof(LineBuf), fin)) > 0) {
	if ((ams::GetAMS())->UnScribe( ucode, &ScribeState, LineBuf, ct, fout) < 0) {
	    fclose(fin);
	    if (!sameHost)
		unlink(MyName);
	    if (fout != OutputFP) fclose(fout);
	    unlink(TmpFileName);
	    return(-3);
	}
    }
    if ((ams::GetAMS())->UnScribeFlush( ucode, &ScribeState, fout)) {
	fclose(fin);
	if (!sameHost)
	    unlink(MyName);
	if (fout != OutputFP) fclose(fout);
	unlink(TmpFileName);
	return(-4);
    }
    fclose(fin);
    if (!OutputFP) {
	if (vfclose(fout)) {
	    unlink(TmpFileName);
	    return(-5);
	}
	if (!sameHost) {
	    /* should do some more error checking here */
	    (ams::GetAMS())->MS_UnlinkFile(FileName);
	    (ams::GetAMS())->CUI_StoreFileToVice(TmpFileName,FileName);
	    unlink(MyName);
	    unlink(TmpFileName);
	} else {
	    rename(TmpFileName, FileName);
	}
    }
    return(0);
}

void BSSM_SendmessageCompound(class sendmessage  *sm, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( sm, "sendmessage", cmds);
}

void BSSM_SendmessageFoldersCompound(class sendmessage  *sm, char  *cmds)
{
    if (sm->foldersp) {
	(ams::GetAMS())->GenericCompoundAction( sm->foldersp, "folders", cmds);
    } else {
	message::DisplayString(sm, 25, "There is no related folders view.");
    }
}

void BSSM_SendmessageMessagesCompound(class sendmessage  *sm, char  *cmds)
{
    if (sm->foldersp) {
	(ams::GetAMS())->GenericCompoundAction( sm->foldersp, "messages", cmds);
    } else {
	message::DisplayString(sm, 25, "There is no related messages view.");
    }
}

void SBSSM_TextviewCompound(class textview  *tv, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( tv, "textview", cmds);
}

void SBSSM_DoBodiesCommand(class sendmessage  *sm, char  *cmds)
{
    SBSSM_TextviewCompound(sm->BodyTextview, cmds);
}

void SBSSM_DoHeadersCommand(class sendmessage  *sm, char  *cmds)
{
    SBSSM_TextviewCompound(sm->HeadTextview, cmds);
}

void RestoreFromPS(class sendmessage  *self)
{
    char *ExpandedPS, *PStext;

    if (self->PSMsg == NULL) {
	message::DisplayString(self, 10, "There is nothing to which to add a PS.");
	return;
    }
    if ((self)->HasChanged()) {
	if (!(self)->AskEraseUnsentMail()) {
	    return;
	}
    } 
    (self)->Clear();
    /*    text_ClearCompletely(self->HeadText);
      text_SetGlobalStyle(self->HeadText, self->DefaultHeadStyle); */
    ExpandedPS = (char *)malloc(10+strlen(self->PSMsg));
    if (ExpandedPS) {
	char *s, c;
	for (s=self->PSMsg; *s; ++s) {
	    if (!strncmp(s, "\nSubject: ", 10)) {
		break;
	    }
	}
	if (*s) {
	    s += 10;
	    c = *s;
	    *s = '\0';
	    strcpy(ExpandedPS, self->PSMsg);
	    *s = c;
	    if (AlreadyPS(s)) {
		strcat(ExpandedPS, "P");
	    } else {
		strcat(ExpandedPS, "PS -- ");
	    }
	    strcat(ExpandedPS, s);
	} else {
	    free(ExpandedPS);
	    ExpandedPS = NULL;
	}
    }
    PStext = ExpandedPS ? ExpandedPS : self->PSMsg;
    (self->HeadText)->InsertCharacters( 0, PStext, strlen(PStext));
    if (ExpandedPS) free(ExpandedPS);
    MakeHeaderFieldsBold(self);
    (self)->ResetSendingDot();
    (self->HeadTextview)->WantUpdate( self->HeadTextview);
}

int AlreadyPS(char  *subj)
{
    Boolean UpToS = FALSE, SawP = FALSE;
    char *s;

    for (s=subj; *s; ++s) {
	if (UpToS) return(isspace(*s));
	if (*s == 'P') {
	    SawP = TRUE;
	    continue;
	}
	if (*s != 'S') return(0);
	if (!SawP) return(0);
	UpToS = TRUE;
    }
    return(0);
}

boolean sendmessage::InitializeClass() 
{
    return (OneTimeProcInit(&sendmessage_ATKregistry_ ));
}

int sendmessage::HasChanged()
{
    if (this->HeadModified != (this->HeadText)->GetModified()) {
	return 1;
    }
    if (this->BodyModified != (this->BodyText)->GetModified()) {
	return 1;
    }
    return 0;
}

int sendmessage::NeedsCheckpointing()
{
    if (this->HeadCheckpoint != (this->HeadText)->GetModified()) {
	return 1;
    }
    if (this->BodyCheckpoint != (this->BodyText)->GetModified()) {
	return 1;
    }
    return 0;
}

void sendmessage::WantUpdate(class view  *v)
{
    (this)->view::WantUpdate( v);
    if (this->CurrentState != SM_STATE_INPROGRESS) {
	(this)->view::WantUpdate( this);
    }
}

/* This should probably be a method. */
void sendmessage_DuplicateWindow(class sendmessage  *self)
{
    class sendmessage *s = new sendmessage;

    s->foldersp = self->foldersp;	// reference same folders if defined.
    s->myframe = ams::InstallInNewWindow(s, "messages-send", "Message composition", environ::GetProfileInt("sendmessage.width", -1), environ::GetProfileInt("sendmessage.height", -1), s);
    ((self)->GetIM())->SetDeleteWindowCallback( (im_deletefptr)delete_sendmsg_win, (long)self);
    (s)->Reset();
}

int sendmessage::AskEraseUnsentMail()
{
    char *ActionVector[10];
    int result;

    ActionVector[0] = "You have unsent mail. What do you want to do?";
    ActionVector[1] = "Cancel";
    ActionVector[2] = "Create a new SendMessage window";
    ActionVector[3] = "Erase unsent mail";
    ActionVector[4] = NULL;

    im::SetLastUsed(this->GetIM());	// hint for dialog.
    result = ams::GetAMS()->ChooseFromList(ActionVector, 1);

    switch(result) {
	case 1:
	    return(FALSE);
	case 2:
	    sendmessage_DuplicateWindow(this);
	    return(FALSE);
	case 3:
	    return(TRUE);
    }
    return(FALSE);
}

void SaveForPS(class sendmessage  *self)
{
    int len = (self->HeadText)->GetLength();
    if (self->PSMsg) free(self->PSMsg);
    self->PSMsg = (char *)malloc(1+len);
    (self->HeadText)->CopySubString( 0, len, self->PSMsg, FALSE);
    self->PSMsg[len] = '\0';
}

class folders *sendmessage::NewFoldersInNewWindow()
{
    class folders *f = new folders;

    (this)->SetFoldersView( f);
    (f)->SetSendmessage( this);
    ams::InstallInNewWindow((f)->GetApplicationLayer(), "messages-folders", "Message Folders", environ::GetProfileInt("folders.width", 600), environ::GetProfileInt("folders.height", 120), f);
    ((this)->GetIM())->SetDeleteWindowCallback( (im_deletefptr)delete_sendmsg_win, (long)this);
    return(f);
}
