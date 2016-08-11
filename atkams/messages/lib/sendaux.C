/***********************************************************
                Copyright IBM Corporation 1991

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/sendaux.C,v 1.11 1995/11/07 20:17:10 robr Stab74 $";
#endif


                                 

#include <andrewos.h>                  /* sys/file.h */
#include <stdio.h>
#include <sys/param.h>
#include <util.h>
#include <pwd.h>
#include <ctype.h>


#include <proctable.H>
/* #include <keymap.ih> */
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <style.H>
#include <stylesheet.H>
#define dontDefineRoutinesFor_fontdesc
#include <fontdesc.H>
#undef dontDefineRoutinesFor_fontdesc
#include <text.H>
#include <environment.H>
#include <textview.H>
#include <environ.H>
#include <message.H>
#include <sbuttonv.H>
#include <im.H>
#include <frame.H>
#include <tree23int.H>
#include <completion.H>
#define dontDefineRoutinesFor_nestedmark
#include <nestedmark.H>
#undef dontDefineRoutinesFor_nestedmark

#include <cui.h>
#include <fdphack.h>
#include <ams.H>
#include <amsutil.H>
#include <folders.H>
#define AUXMODULE 1
#include <sendmessage.H>

static class keymap *smkm, *smhkm;
static class menulist *sm_menulist, *sm_hmenulist;

/* values for sendmessage menu mask */
#define SMMASK_FEWSTYLES 1
#define SMMASK_HASMESS 2
#define SMMASK_FORCESEND 4
#define SMMASK_CHECKRECIP 8
#define SMMASK_INSERTHEAD 16
#define SMMASK_FILEINTO 32

#ifndef NORCSID
#endif

extern void DirectlyInsertFile(class textview  *tv, class text  *t, char  *fname, int  pos);
extern void HandleButton(class sbutton  *self, long sendmessage, int  in, int  whichbut);
extern int WriteOneFile(class sendmessage  *sendmessage, char  *ViceFileName, Boolean  OnVice , Boolean  MayOverwrite , int  Version , Boolean  TrustDelivery , Boolean  UseMultipartFormat, int  *EightBitText);
extern int ProduceUnscribedVersion(char  *FileName, FILE  *OutputFP);
extern void SetNotModified(class sendmessage  *sendmessage);
extern void BSSM_HeadersFocus(class sendmessage  *sm);
extern void BSSM_BodyFocus(class sendmessage  *sm);
extern void BSSM_DownFocus(class sendmessage  *sm);
extern void BSSM_UpFocus(class sendmessage  *sm);
extern void BSSM_FakeBug(class sendmessage  *sm, char  *txt);
extern void BSSM_SendmessageCompound(class sendmessage  *sm, char  *cmds);
extern void BSSM_SendmessageFoldersCompound(class sendmessage  *sm, char  *cmds);
extern void BSSM_SendmessageMessagesCompound(class sendmessage  *sm, char  *cmds);
extern void SBSSM_TextviewCompound(class textview  *tv, char  *cmds);
extern void SBSSM_DoBodiesCommand(class sendmessage  *sm, char  *cmds);
extern void SBSSM_DoHeadersCommand(class sendmessage  *sm, char  *cmds);
extern int sendmessage_wrap_AppendBugInfoToBody(class sendmessage *self, Boolean  IsMessagesBug);
extern void sendmessage_wrap_Reset(class sendmessage *self);
extern int sendmessage_wrap_CheckRecipients(class sendmessage *self);
extern void ForceSending(class sendmessage  *sendmessage);
extern void ForceStripping(class sendmessage  *sendmessage);
extern void sendmessage_DuplicateWindow(class sendmessage  *self);
extern int FileIntoFolder(class sendmessage  *sm, char  *name);
extern void sendmessage_DoDelivery(class sendmessage  *sendmessage);
extern void RestoreFromPS(class sendmessage  *self);
extern int ComposeBugReport(class sendmessage  *sm);


static long AbsentProcedure(class view     *self, long rock);
static int AddSpecialHeaders(class sendmessage  *sm);
void QuoteProperly(char *str);
void QuitMessages(class sendmessage  *self);
static void     DeleteWindow(class sendmessage  *self);
void delete_sendmsg_win(class im  *im, class sendmessage  *self);
void BSSM_DummyQuit(class sendmessage  *self);
void ToggleClearButt(class sendmessage  *self);
void ToggleSignButt(class sendmessage  *self);
void ToggleHideButt(class sendmessage  *self);
void ToggleBlindButt(class sendmessage  *self);
void SimulateResetButton(class sendmessage  *self);
void BSSM_Preview(class sendmessage  *sm);
void DoPreview(class sendmessage  *sm, int   code);
void ReadDraft(char  *File, class sendmessage  *sendmessage);
void sendmessage_SaveDraft(class sendmessage  *sendmessage);
void sendmessage_RestoreDraft(class sendmessage  *sendmessage, char  *dfile);
Boolean OneTimeProcInit(struct ATKregistryEntry  *c);
void InitProcStuff(class sendmessage  *sendmessage);
void DestroyProcStuff(class sendmessage  *self);
void InitStylesAndFonts(class sendmessage  *sendmessage);
void DestroyStyles(class sendmessage  *self);
void UserWantsAHeader(class sendmessage  *self, char *head);
void BeginLine(class sendmessage  *sm);
void NextLine(class sendmessage  *sm, int   IsNewline);
void PreviousLine(class sendmessage  *sm);
void sendmessage_InsertFile(class sendmessage  *sendmessage, char *fname);
void PrepareBodyForSignature(class sendmessage  *self);
void SetMyFrameTitle(class sendmessage  *sm, char *tit);
static boolean  AddIfView(class environment  *env, int  *ct);
int EnvViewCt(class environment  *env);
void sendmessage_wrap_QuoteBody(class sendmessage *self);

static long AbsentProcedure(class ATK *self, long)
{
    message::DisplayString((view *)self, 75, "Absent procedure - did not find a normal ATK command in the proctable!");
    return 0;
}

static proctable_fptr
                textv_EndOfLineCmd = AbsentProcedure,
                textv_OpenLineCmd = AbsentProcedure,
                textv_InsertNewlineCmd = AbsentProcedure,
                textv_PreviousLineCmd = AbsentProcedure,
                textv_InsertFileCmd = AbsentProcedure,
                textv_BeginningOfLineCmd = AbsentProcedure,
                textv_PlainestCmd = AbsentProcedure,
                textv_NextLineCmd = AbsentProcedure;

                            


static int AddSpecialHeaders(class sendmessage  *sm)
{
    class textview *tv;
    int             ans, pos, oldpos, oldlen, useplus;
    char            buf[1000], head[2000], ShortName[1 + MAXPATHLEN], *FullName, *md;
    static char    *SVec[] = {
        "What kind of `special' action should this message take?",
        "Cancel",
        "Request a return-receipt",
        "Request a vote",
        "Announce an Enclosure",
        "Invite folder subscriptions",
        "Invite further redistribution",
        NULL
    };
    static char    *EVec[] = {
        "What do you want to call an 'enclosure'?",
        "Cancel",
        "The whole message",
        "A certain file",
        NULL
    };
    static char    *EnclosureString = "---- Enclosure ----";
    char            EnclosureBuf[50];

    ans = (ams::GetAMS())->ChooseFromList( SVec, 1);
    switch (ans) {
        case 1:
            /* nothing */
            message::DisplayString(sm, 10, "No special headers added.");
            return (-1);
        case 2:
            /* return receipt */
            md = (ams::GetAMS())->CUI_MailDomain();
            useplus = (ams::GetAMS())->CUI_UseAmsDelivery();
            if (useplus <= 0) useplus = (ams::GetAMS())->CheckAMSUseridPlusWorks( md);
            sprintf(head, "%s%s@%s", (ams::GetAMS())->CUI_WhoIAm(), useplus > 0 ? "+" : "", md);
            if (message::AskForString(sm, 50, "Request acknowledgement to: ", head, buf, sizeof(buf)) >= 0) {
                char            Msg[500], *valaddr = NULL;

                sprintf(Msg, "Validating %s; please wait...", buf);
                message::DisplayString(sm, 10, Msg);
                im::ForceUpdate();
                ams::WaitCursor(TRUE);
                if ((ams::GetAMS())->CUI_RewriteHeaderLine( buf, &valaddr) || !valaddr) {
                    message::DisplayString(sm, 10, "Bad address specified -- not return receipt request.");
                    ams::WaitCursor(FALSE);
                    return (-1);
                }
                ams::WaitCursor(FALSE);
                strcpy(head, "Ack-to: ");
                strcat(head, valaddr);
                (sm)->AddHeaderLine( head);
                free(valaddr);
            }
            break;
        case 3:
            {
                /* vote */
                char            Question[500], Answer[100], ID[100], Addr[300], Prompt[100], DefaultID[100], *valaddr = NULL, *md;
                int             index = 1, HasWriteIn = 0, useplus;
                static int      votectr = 0;

                md = (ams::GetAMS())->CUI_MailDomain();
                useplus = (ams::GetAMS())->CUI_UseAmsDelivery();
                if (useplus <= 0) useplus = (ams::GetAMS())->CheckAMSUseridPlusWorks( md);
                sprintf(head, "%s%s@%s", (ams::GetAMS())->CUI_WhoIAm(), useplus > 0 ? "+" : "", md);
                if (message::AskForString(sm, 50, "Send votes to: ", head, Addr, sizeof(Addr)) < 0) {
                    return (-1);
                }
                sprintf(Question, "Validating %s; please wait...", Addr);
                message::DisplayString(sm, 10, Question);
                im::ForceUpdate();
                ams::WaitCursor(TRUE);
                if ((ams::GetAMS())->CUI_RewriteHeaderLine( Addr, &valaddr) || !valaddr) {
                    message::DisplayString(sm, 10, "Bad address specified -- not composing vote request.");
                    ams::WaitCursor(FALSE);
                    return (-1);
                }
                ams::WaitCursor(FALSE);
                if (message::AskForString(sm, 50, "Please enter the vote question: ", NULL, Question, sizeof(Question)) < 0) {
                    free(valaddr);
                    return (-1);
                }
                QuoteProperly(Question);
                buf[0] = '\0';
                while (TRUE) {
                    sprintf(Prompt, "Please enter vote choice #%d [default: all done] : ", index);
                    if (message::AskForString(sm, 50, Prompt, NULL, Answer, sizeof(Answer)) < 0) {
                        free(valaddr);
                        return (-1);
                    }
                    if (Answer[0] == '\0')
                        break;
                    if (index++ > 1)
                        strcat(buf, ", ");
                    if (index > 8) {
                        message::DisplayString(sm, 10, "That's enough choices.");
                        break;
                    }
                    QuoteProperly(Answer);
                    strcat(buf, Answer);
                    if (Answer[0] == '*' && Answer[1] == '\0')
                        HasWriteIn = 1;
                }
                if (!HasWriteIn) {
                    if ((ams::GetAMS())->GetBooleanFromUser( "Do you want to allow write-in votes", FALSE)) {
                        if (index++ > 1)
                            strcat(buf, ", ");
                        strcat(buf, "*");
                    }
                }
                sprintf(DefaultID, "%s.%d.%d", (ams::GetAMS())->CUI_MachineName() ? (ams::GetAMS())->CUI_MachineName() : "unknown-host", getpid(), votectr++);
                if (message::AskForString(sm, 50, "Your ID for this vote: ", DefaultID, ID, sizeof(ID)) < 0) {
                    free(valaddr);
                    return (-1);
                }
                QuoteProperly(ID);
                strcpy(head, "Vote-Request: ");
                strcat(head, ID);
                strcat(head, ", ");
                strcat(head, Question);
                (sm)->AddHeaderLine( head);
                strcpy(head, "Vote-To: ");
                strcat(head, valaddr);
                (sm)->AddHeaderLine( head);
                free(valaddr);
                strcpy(head, "Vote-Choices: ");
                strcat(head, buf);
                (sm)->AddHeaderLine( head);
                break;
            }
        case 4:
            /* enclosure */
            ans = (ams::GetAMS())->ChooseFromList( EVec, 1);
            switch (ans) {
                case 1:
                    /* Nothing */
                    break;
                case 2:
                    /* the whole message */
                    (sm)->AddHeaderLine( "Enclosure: ");
                    break;
                case 3:
                    /* a certain file */
                    sprintf(EnclosureBuf, "Enclosure: %s", EnclosureString);
                    (sm)->AddHeaderLine( EnclosureBuf);
                    tv = sm->BodyTextview;
                    oldpos = (tv)->GetDotPosition();
                    oldlen = (tv)->GetDotLength();
                    pos = (sm->BodyText)->GetLength();
                    sprintf(EnclosureBuf, "\n%s\n", EnclosureString);
                    (sm->BodyText)->InsertCharacters( pos, EnclosureBuf, strlen(EnclosureBuf));
                    pos = (sm->BodyText)->GetLength();
                    (tv)->SetDotPosition( pos);
                    textv_InsertFileCmd(tv,0);
                    pos = (sm->BodyText)->GetLength();
                    (sm->BodyText)->InsertCharacters( pos, EnclosureBuf, strlen(EnclosureBuf));
                    (tv)->SetDotPosition( pos);
                    (tv)->SetDotLength( strlen(EnclosureBuf));
                    textv_PlainestCmd(tv,0);
                    (tv)->SetDotPosition( oldpos);
                    (tv)->SetDotLength( oldlen);
                    break;
            }
            break;
        case 5:
            /* subscription */
            if (message::AskForString(sm, 50, "Invite subscriptions to what folder? ", NULL, ShortName, sizeof(ShortName)) < 0)
                return (-1);
            if ((ams::GetAMS())->CUI_DisambiguateDir( ShortName, &FullName)) {
                (ams::GetAMS())->CUI_ReportAmbig( ShortName, "folder");
                return (-1);
            }
            strcpy(head, "X-Andrew-DirectoryCreation: ");
            strcat(head, FullName);
            (sm)->AddHeaderLine( head);
            break;
        case 6:
            /* Invite redistribution */
            {
                char            Question[500], Addr[1000], *valaddr = NULL, head[1100];

                if (message::AskForString(sm, 50, "Invite redistribution to: ", "", Addr, sizeof(Addr)) < 0) {
                    return (-1);
                }
                sprintf(Question, "Validating %s; please wait...", Addr);
                message::DisplayString(sm, 10, Question);
                im::ForceUpdate();
                ams::WaitCursor(TRUE);
                if ((ams::GetAMS())->CUI_RewriteHeaderLine( Addr, &valaddr) || !valaddr) {
                    message::DisplayString(sm, 10, "Bad address specified -- not composing redistribution request.");
                    ams::WaitCursor(FALSE);
                    return (-1);
                }
                ams::WaitCursor(FALSE);
                strcpy(head, "X-Andrew-Redistribution-To: ");
                strcat(head, valaddr);
                (sm)->AddHeaderLine( head);
                free(valaddr);
                break;
            }
        default:
            message::DisplayString(sm, 10, "Unrecognized answer - no headers added.");
            return (-1);
    }
    message::DisplayString(sm, 10, "Done.");
    return (0);
}

void QuoteProperly(char            *str)
{
    if (strchr(str, '"') || strchr(str, '\\') || strchr(str, ',')) {
        char            MyBuf[2000];
        register char  *s, *t;

        /* Needs quoting */
        MyBuf[0] = '"';
        for (s = str, t = &MyBuf[1]; *s; ++s, ++t) {
            if ((*s == '"') || (*s == '\\')) {
                *t++ = '\\';
            }
            *t = *s;
        }
        *t++ = '"';
        *t = '\0';
        strcpy(str, MyBuf);
    }
}

void            QuitMessages(class sendmessage  *self)
{
    ams::CommitState(TRUE, FALSE, TRUE, TRUE);
}

static char     lastWindowWarning[] =
"This is the last window.";
static char    *lastWindowChoices[] = {
    "Continue Running",
    "Quit Messages",
NULL};

#define lastWindow_CANCEL 0
#define lastWindow_QUIT   1

static void     DeleteWindow(class sendmessage  *self)
{
    if ((self)->HasChanged()) {
	if (!ams::GetAMS()->GetBooleanFromUser("Do you want to erase the mail you have not yet sent", FALSE)) {
            return;
        }
        (self)->Clear();
    }
    if (ams::CountAMSViews() > 1) {
	ams::CommitState(FALSE, FALSE, FALSE, FALSE);
	if ((self)->GetIM()) {
	    class im *im=(self)->GetIM();
	    im->UnlinkTree();
	    if(self->myframe) (self->myframe)->SetView( NULL);
	    (im)->SetView(NULL);
	    if(self->myframe) (self->myframe)->Destroy();
	    (im)->Destroy();
	}
        (self)->Destroy();
    }
    else {
        long            answer;

        if (message::MultipleChoiceQuestion(NULL, 0,
                                        lastWindowWarning, lastWindow_CANCEL,
                                           &answer, lastWindowChoices, NULL)
            == -1)
            return;
        switch (answer) {
            case lastWindow_CANCEL:
                return;

            case lastWindow_QUIT:
                QuitMessages(self);
        }
    }
}

void delete_sendmsg_win(class im  *im, class sendmessage  *self)
{
    DeleteWindow(self);
}

void            BSSM_DummyQuit(class sendmessage  *self)
{
    message::DisplayString(NULL, 10, "Use ^X^C to quit.");
}

void            ToggleClearButt(class sendmessage  *self)
{
    HandleButton(self->buttons->ButtonData(), (long)self, SM_CLEAR, EXP_CLEARAFTER);
}

void            ToggleSignButt(class sendmessage  *self)
{
    HandleButton(self->buttons->ButtonData(), (long)self, SM_SIGN, EXP_SIGNMAIL);
}

void            ToggleHideButt(class sendmessage  *self)
{
    HandleButton(self->buttons->ButtonData(), (long)self, SM_HIDE, EXP_HIDEAFTER);
}

void            ToggleBlindButt(class sendmessage  *self)
{
    HandleButton(self->buttons->ButtonData(), (long)self, SM_BLIND, EXP_KEEPBLIND);
}

void            SimulateResetButton(class sendmessage  *self)
{
    HandleButton(self->buttons->ButtonData(), (long)self, SM_RESET, 0);
}

#define PREVIEW_ASK 0
#define PREVIEW_UNSCRIBE 1
#define PREVIEW_RAW 2

static char    *PreviewCmdVector[] = {NULL, "-p", "-z", NULL, NULL};
static char     PreviewProg[1 + MAXPATHLEN] = "";

void            BSSM_Preview(class sendmessage  *sm)
{
    DoPreview(sm, PREVIEW_ASK);
}

static char    *PrevQVec[] = {
    "Which kind of preview do you want?",
    "Cancel",
    "Show with formatting removed",
    "Show with raw formatting information",
    NULL
};

void DoPreview(class sendmessage  *sm, int              code)
{
    char            FileName[1 + MAXPATHLEN];
    int             ans, rcode;

    if (code == PREVIEW_ASK) {
        ans = (ams::GetAMS())->ChooseFromList( PrevQVec, 1);
        if (ans != 2 && ans != 3)
            return;
    }
    else {
        if (code == PREVIEW_UNSCRIBE) {
            ans = 2;
        }
        else {
            ans = 3;
        }
    }
    (ams::GetAMS())->CUI_GenLocalTmpFileName( FileName);
    if (WriteOneFile(sm, FileName, FALSE, TRUE, (ans == 2) ? 0 : 12, (code == PREVIEW_UNSCRIBE) ? 1 : 0, FALSE, NULL)) {
        return;                        /* error was reported */
    }
    if (!PreviewProg[0]) {
        strcpy(PreviewProg, environ::AndrewDir("/bin/ezprint"));
        PreviewCmdVector[0] = PreviewProg;
    }
    PreviewCmdVector[3] = FileName;
    if (ans == 2) {
        rcode = ProduceUnscribedVersion(FileName, NULL);
        if (rcode) {
            char            ErrorText[256];

            sprintf(ErrorText, "Unformatting error (%d, %d); cannot produce unformatted text", rcode, errno);
            message::DisplayString(sm, 10, ErrorText);
        }
    }
    rcode = osi_vfork();
    if (rcode == 0) {
        int             fd;

        /* I am a child */
        for (fd = FDTABLESIZE(); fd > 2; --fd)
            close(fd);
        execv(PreviewCmdVector[0], PreviewCmdVector);
        _exit(-1);                     /* not reached */
    }
    if (rcode > 0) {
        message::DisplayString(sm, 10, "Preview window should appear soon.");
    }
    else {
        message::DisplayString(sm, 10, "Preview fork failed!");
    }
}
static struct bind_Description smheadbindings[] = {
    {"sendmessage-next-line", "\015", 1, NULL, 0, 0, (proctable_fptr)NextLine, "control-n function", NULL},
    {"sendmessage-next-line", "\012", 1, NULL, 0, 0, (proctable_fptr)NextLine, "control-n function", NULL},
    {"sendmessage-add-header", "\017", 0, "Other~42,Insert Header~88", 0, SMMASK_INSERTHEAD, (proctable_fptr)UserWantsAHeader, "control-o function", NULL},
    {NULL, NULL, 0, NULL, 0, 0, NULL, NULL, NULL}
};


static char DraftFileNameBuf[MAXPATHLEN+1];
static void GetDraftDefaultName()
{
    char *name;

    if (DraftFileNameBuf[0] != '\0')
	return;	/* already got it */
    name = environ::GetProfile("DraftMailFile");
    strcpy(DraftFileNameBuf, name ? name : "~/Draft.mail");
}

void ReadDraft(char  *File, class sendmessage  *sendmessage)
{
    char FileBuf[1+MAXPATHLEN];

    (ams::GetAMS())->TildeResolve( File, FileBuf);
    (sendmessage)->ReadFromFile( FileBuf, FALSE);
}

void sendmessage_SaveDraft(class sendmessage  *sendmessage)
{
    GetDraftDefaultName();
    if (completion::GetFilename(sendmessage, "Draft file to save: ", DraftFileNameBuf, DraftFileNameBuf, sizeof(DraftFileNameBuf), FALSE, FALSE) == -1 ) {
	return;
    }
    if (!(sendmessage)->WriteFile( DraftFileNameBuf)) {
	SetNotModified(sendmessage);
    }
}

void sendmessage_RestoreDraft(class sendmessage  *sendmessage, char  *dfile)
{
    int NeedReset =0;

    if (dfile && *dfile) {
	strcpy(DraftFileNameBuf, dfile);
    } else {
	GetDraftDefaultName();
	if ((sendmessage)->HasChanged()) {
	    if (!(sendmessage)->AskEraseUnsentMail()) {
		return /* 0 */;
	    }
	    NeedReset = 1;
	    (sendmessage)->Clear();
	} 
	if (completion::GetFilename(sendmessage, "Draft file to restore: ", DraftFileNameBuf, DraftFileNameBuf, sizeof(DraftFileNameBuf), FALSE, TRUE) == -1 ) {
	    if (NeedReset) (sendmessage)->Reset();
	    return;
	}
    }
    ReadDraft(DraftFileNameBuf, sendmessage);
}

static struct bind_Description smbindings[] = {

    /*
     * procname, keysequenece, key rock, menu string, menu rock, proc,
     * docstring, dynamic autoload
     */
    {"sendmessage-toggle-clear-button", NULL, 0, NULL, 0, 0, (proctable_fptr)ToggleClearButt, "simulate the 'clear' button"},
    {"sendmessage-toggle-hide-button", NULL, 0, NULL, 0, 0, (proctable_fptr)ToggleHideButt, "simulate the 'hide' button"},
    {"sendmessage-toggle-sign-button", NULL, 0, NULL, 0, 0, (proctable_fptr)ToggleSignButt, "simulate the 'sign' button"},
    {"sendmessage-toggle-blind-button", NULL, 0, NULL, 0, 0, (proctable_fptr)ToggleBlindButt, "simulate the 'blind' button"},
    {"sendmessage-simulate-reset-button", NULL, 0, NULL, 0, 0, (proctable_fptr)SimulateResetButton, "simulate the 'reset' button"},
    {"sendmessage-dummy-quit", "\003", 0, NULL, 0, 0, (proctable_fptr)BSSM_DummyQuit, "dummy quit command"},
    {"sendmessage-quit", "\030\003", 0, "Quit~99", 0, 0, (proctable_fptr)QuitMessages, "exit messages"},
    {"sendmessage-delete-window", "\030\004", 0, "Delete Window~89", 0, 0, (proctable_fptr)DeleteWindow, "Delete sendmessage window"},
    {"sendmessage-duplicate", "\0302", 0, NULL, 0, 0, (proctable_fptr)sendmessage_DuplicateWindow, "Open another sendmessage window"},
    {"sendmessage-force-send", NULL, 0, "Send Formatted~23", 0, SMMASK_FORCESEND, (proctable_fptr)ForceSending, "Send mail even if formatted", NULL},
    {"sendmessage-force-strip", NULL, 0, "Send Unformatted~24", 0, SMMASK_FORCESEND, (proctable_fptr)ForceStripping, "Send mail stripping all formatting", NULL},
    {"sendmessage-add-header", NULL, 0, "Other~42,Insert Header~88", 0, SMMASK_INSERTHEAD, (proctable_fptr)UserWantsAHeader, "control-o function", NULL},
    {"sendmessage-check-recipients", NULL, 0, "Other~42,Check Recipients~75", 0, SMMASK_CHECKRECIP, (proctable_fptr)sendmessage_wrap_CheckRecipients, "Check recipient list", NULL},
    {"sendmessage-file-into-folder", NULL, 0, "Other~42,File Into Folder~55", 0, SMMASK_FILEINTO, (proctable_fptr)FileIntoFolder, "File draft message into a folder"},
    {"sendmessage-excerpt-body", "\030q", 0, "Other~42,Excerpt Body~36", 0, SMMASK_HASMESS, (proctable_fptr)sendmessage_wrap_QuoteBody, "Quote message on display"},
    {"sendmessage-PS", NULL, 0, "Other~42,Send PS~97", 0, 0, (proctable_fptr)RestoreFromPS, "Try to send another piece of mail to the same address(es)"},
    {"sendmessage-append-bug-info", NULL, 0, NULL, 0, 0, (proctable_fptr)sendmessage_wrap_AppendBugInfoToBody, "Append bug report information to the message being composed."},
    {"sendmessage-fake-bug", NULL, 0, NULL, 0, 0, (proctable_fptr)BSSM_FakeBug, "Fake a bug report"},
    {"sendmessage-down-input-focus", "\030n", 0, NULL, 0, 0, (proctable_fptr)BSSM_DownFocus, "Move input focus down"},
    {"sendmessage-up-input-focus", "\030p", 0, NULL, 0, 0, (proctable_fptr)BSSM_UpFocus, "Move input focus up"},
    {"sendmessage-down-input-focus", "\030\016", 0, NULL, 0, 0, (proctable_fptr)BSSM_DownFocus, "Move input focus down"},
    {"sendmessage-up-input-focus", "\030\020", 0, NULL, 0, 0, (proctable_fptr)BSSM_UpFocus, "Move input focus up"},
    {"sendmessage-focus-on-headers", NULL, 0, NULL, 0, 0, (proctable_fptr)BSSM_HeadersFocus, "Input focus on headers"},
    {"sendmessage-focus-on-body", NULL, 0, NULL, 0, 0, (proctable_fptr)BSSM_BodyFocus, "Input focus on body of outgoing message"},
    {"sendmessage-send-bug-report", NULL, 0, "Other~42,Compose Bug Report~99", 0, 0, (proctable_fptr)ComposeBugReport, "Begin composing a messages/sendmessage bug report"},
    {"sendmessage-preview", NULL, 0, "Other~42,Preview Non-Andrew~77", 0, 0, (proctable_fptr)BSSM_Preview, "Preview how a message will look to non-Andrew users"},
    {"sendmessage-special-headers", NULL, 0, "Other~42,Add Special Headers~98", 0, 0, (proctable_fptr)AddSpecialHeaders, "Add special headers to the message"},
    {"sendmessage-clear-message", NULL, 0, "Clear~60", 0, 0, (proctable_fptr)sendmessage_wrap_Reset, "Clear the message composition area"},
    {"sendmessage-send-message", "\030s", 0, "Send/Post~70", 0, 0, (proctable_fptr)sendmessage_DoDelivery, "Send/Post the current message"},
    {"sendmessage-save-draft", NULL, 0, "Other~42,Save Draft~60", 0, 0, (proctable_fptr)sendmessage_SaveDraft, "Save the draft of a message"},
    {"sendmessage-restore-draft", NULL, 0, "Other~42,Restore Draft~61", 0, 0, (proctable_fptr)sendmessage_RestoreDraft, "Restore the draft of a message"},
    {"sendmessage-insert-file", "\030\011", 0, "File,Insert File~35", 0, 0, (proctable_fptr)sendmessage_InsertFile, "Insert a file in the message header or body area"},

    {"sendmessage-beginning-of-header", "\001", 0, NULL, 0, 0, (proctable_fptr)BeginLine, "control-a function", NULL},
    {"sendmessage-next-line", "\016", 0, NULL, 0, 0, (proctable_fptr)NextLine, "control-n function", NULL},
    {"sendmessage-previous-header-line", "\020", 0, NULL, 0, 0, (proctable_fptr)PreviousLine, "control-p function", NULL},

    /* These are just setting up some proc table entries */
    {"sendmessage-folders-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)BSSM_SendmessageFoldersCompound, "Execute a compound operation on the related folders view"},
    {"sendmessage-messages-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)BSSM_SendmessageMessagesCompound, "Execute a compound operation on the related messages view"},
    {"sendmessage-compound-operation", NULL, 0, NULL, 0, 0, (proctable_fptr)BSSM_SendmessageCompound, "Execute a compound sendmessage operation"},
    {"sendmessage-set-not-modified", NULL, 0, NULL, 0, 0, (proctable_fptr)SetNotModified, "Mark the message composition area as 'not modified'"},
    {"sendmessage-headers-textview-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)SBSSM_DoHeadersCommand, "Execute a compound textview command on the headers"},
    {"sendmessage-bodies-textview-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)SBSSM_DoBodiesCommand, "Execute a compound textview command on the body"},
    {"textview-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)SBSSM_TextviewCompound, "Execute a compound textview operation"},

    {NULL, NULL, 0, "Region,ProgramExample", 0, 0, NULL, NULL, NULL},
    {NULL, NULL, 0, "Region,Enumerate", 0, 0, NULL, NULL, NULL},
    {NULL, NULL, 0, "Region,Itemize", 0, 0, NULL, NULL, NULL},
    {NULL, NULL, 0, "Region,Display", 0, SMMASK_FEWSTYLES, NULL, NULL, NULL},
    {NULL, NULL, 0, "Region,FormatNote", 0, SMMASK_FEWSTYLES, NULL, NULL, NULL},
    {NULL, NULL, 0, "Region,Indent", 0, SMMASK_FEWSTYLES, NULL, NULL, NULL},
    {NULL, NULL, 0, "Title,MajorHeading", 0, SMMASK_FEWSTYLES, NULL, NULL, NULL},
    {NULL, NULL, 0, "Title,Heading", 0, SMMASK_FEWSTYLES, NULL, NULL, NULL},
    {NULL, NULL, 0, "Title,Subheading", 0, SMMASK_FEWSTYLES, NULL, NULL, NULL},
    {NULL, NULL, 0, "Title,Chapter", 0, SMMASK_FEWSTYLES, NULL, NULL, NULL},
    {NULL, NULL, 0, "Title,Section", 0, SMMASK_FEWSTYLES, NULL, NULL, NULL},
    {NULL, NULL, 0, "Title,Subsection", 0, SMMASK_FEWSTYLES, NULL, NULL, NULL},
    {NULL, NULL, 0, "Title,Paragraph", 0, SMMASK_FEWSTYLES, NULL, NULL, NULL},
    {NULL, NULL, 0, NULL, 0, 0, NULL, NULL, NULL}
};


Boolean OneTimeProcInit(struct ATKregistryEntry  *c)
{
    struct proctable_Entry *tempProc;

    ATK::LoadClass("textview");            /* make sure the textview is loaded first */
    if ((tempProc = proctable::Lookup("textview-insert-file")) != NULL) {
        textv_InsertFileCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-end-of-line")) != NULL) {
        textv_EndOfLineCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-open-line")) != NULL) {
        textv_OpenLineCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-insert-newline")) != NULL) {
        textv_InsertNewlineCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-beginning-of-line")) != NULL) {
        textv_BeginningOfLineCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-next-line")) != NULL) {
        textv_NextLineCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-plainest")) != NULL) {
        textv_PlainestCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-previous-line")) != NULL) {
        textv_PreviousLineCmd = proctable::GetFunction(tempProc);
    }
    smkm = (class keymap *)ATK::NewObject("keymap"); /*keymap_New(); */
    sm_menulist = new menulist;
    bind::BindList(smbindings, smkm, sm_menulist, c);
    smhkm = (class keymap *)ATK::NewObject("keymap"); /*keymap_New(); */
    sm_hmenulist = new menulist;
    bind::BindList(smheadbindings, smhkm, sm_hmenulist, c);
    return (TRUE);
}

void InitProcStuff(class sendmessage  *sendmessage)
{
    sendmessage->keys = keystate::Create(sendmessage, smkm);
    sendmessage->headkeys = keystate::Create(sendmessage, smhkm);
    sendmessage->mymenulist = (sm_menulist)->DuplicateML( sendmessage);
    sendmessage->myheadmenulist = (sm_hmenulist)->DuplicateML( sendmessage);
}

void DestroyProcStuff(class sendmessage  *self)
{
    delete self->keys;
    delete self->mymenulist;
}

void            sendmessage::PostKeyState(class keystate  *ks)
{
    this->keys->next = NULL;
    (this->keys)->AddBefore( ks);
    if (((this)->GetIM())->GetInputFocus() == (class view *) this->HeadTextview) {
	this->headkeys->next = NULL;
	(this->headkeys)->AddBefore( this->keys);
	(this)->view::PostKeyState( this->headkeys);
    }
    else {
	(this)->view::PostKeyState( this->keys);
    }
}

void            sendmessage::PostMenus(class menulist  *menulist)
{
    long            newmask;

    (this->mymenulist)->ClearChain();
    newmask = ((this->foldersp) ? SMMASK_HASMESS : 0)
          | ((amsutil::GetOptBit(EXP_FILEINTO)) ? SMMASK_FILEINTO : 0)
          | ((amsutil::GetOptBit(EXP_FORCESEND)) ? SMMASK_FORCESEND : 0)
          | ((amsutil::GetOptBit(EXP_CHECKRECIP)) ? SMMASK_CHECKRECIP : 0)
          | ((amsutil::GetOptBit(EXP_INSERTHEADER)) ? SMMASK_INSERTHEAD : 0)
          | ((amsutil::GetOptBit(EXP_BIGSTYLES)) ? 0 : SMMASK_FEWSTYLES);
    (this->mymenulist)->SetMask( newmask);
    (this->myheadmenulist)->SetMask( newmask);
    if (menulist != this->mymenulist) {
        if (menulist)
            (this->mymenulist)->ChainAfterML( menulist, (int)menulist);
    }
    if (((this)->GetIM())->GetInputFocus() == (class view *) this->HeadTextview) {
        (this->mymenulist)->ChainAfterML( this->myheadmenulist, (int)(this->myheadmenulist));
    }
    (this)->view::PostMenus( this->mymenulist);
}

void InitStylesAndFonts(class sendmessage  *sendmessage)
{
    int             fontsize = environ::GetProfileInt("messages.fontsize", 12);
    char           *fontname = amsutil::GetDefaultFontName();

    sendmessage->DefaultStyle = new style;
    (sendmessage->DefaultStyle)->SetFontFamily( fontname);
    (sendmessage->DefaultStyle)->SetFontSize( style_ConstantFontSize, fontsize);
    (sendmessage->DefaultStyle)->SetJustification( style_LeftJustified);
    (sendmessage->DefaultStyle)->SetNewLeftMargin( style_ConstantMargin, 25, style_RawDots);
    (sendmessage->DefaultStyle)->SetNewRightMargin( style_ConstantMargin, 25, style_RawDots);

    sendmessage->DefaultHeadStyle = new style;
    (sendmessage->DefaultHeadStyle)->SetFontFamily( fontname);
    (sendmessage->DefaultHeadStyle)->SetFontSize( style_ConstantFontSize, fontsize);
    (sendmessage->DefaultHeadStyle)->SetJustification( style_LeftJustified);
    (sendmessage->DefaultHeadStyle)->SetNewLeftMargin( style_ConstantMargin, 25, style_RawDots);
    (sendmessage->DefaultHeadStyle)->SetNewRightMargin( style_ConstantMargin, 25, style_RawDots);
    (sendmessage->DefaultHeadStyle)->SetNewIndentation( style_LeftMargin, -20, style_RawDots);

    (sendmessage->BodyText)->SetGlobalStyle( sendmessage->DefaultStyle);
    (sendmessage->HeadText)->SetGlobalStyle( sendmessage->DefaultHeadStyle);

    sendmessage->BoldStyle = new style;
    (sendmessage->BoldStyle)->SetName( "Bold");
    (sendmessage->BoldStyle)->AddNewFontFace( (long) fontdesc_Bold);
    (sendmessage->BoldStyle)->SetFontFamily( fontname);

/*    tmpfontdesc = fontdesc_Create("andy", fontdesc_Bold, 10);
    sendmessage_SetButtonFont(sendmessage, tmpfontdesc); */
}

void DestroyStyles(class sendmessage  *self)
{
    delete self->DefaultStyle;
    delete self->BoldStyle;
    delete self->DefaultHeadStyle;
}

void sendmessage_wrap_QuoteBody(class sendmessage *self)
{
    self->QuoteBody();
}

void  sendmessage::QuoteBody()
{
    char            TempFile[1 + MAXPATHLEN], CaptBuf[AMS_CAPTIONSIZE + 1], *mycaps, BBName[1 + MAXPATHLEN];
    int             pos, len, orgpos, orglen, endpt, caplen;
    class text    *t;
    class style   *qss, *bss;

#define XCERPT "Excerpts from "

    if (!this->foldersp) {
        message::DisplayString(this, 10, "There is no associated messages object.");
        return;
    }
    ams::WaitCursor(TRUE);
    if ((this->foldersp)->WriteFormattedBodyFile( TempFile, CaptBuf)) {
        message::DisplayString(this, 10, "Could not write out body to temp file.");
        ams::WaitCursor(FALSE);
        return;
    }
    t = this->BodyText;
    orgpos = pos = (this->BodyTextview)->GetDotPosition();
    orglen = (t)->GetLength();
    amsutil::ReduceWhiteSpace(CaptBuf);
    mycaps = amsutil::StripWhiteEnds(CaptBuf);
    len = strlen(mycaps);
    (t)->InsertCharacters( pos, XCERPT, sizeof(XCERPT) - 1);
    pos += sizeof(XCERPT) - 1;
    (this->foldersp)->FolderOnDisplay( BBName, NULL);
    if (BBName[0]) {
        (t)->InsertCharacters( pos, BBName, strlen(BBName));
        pos += strlen(BBName);
    }
    (t)->InsertCharacters( pos, ": ", 2);
    pos += 2;
    (t)->InsertCharacters( pos, mycaps, len);
    pos += len;
    caplen = pos - orgpos;
    (t)->InsertCharacters( pos, "\n\n", 2);
    pos += 2;
    (t)->AlwaysInsertFile( NULL, TempFile, pos);
    len = (t)->GetLength() - orglen;
    endpt = orgpos + len;
    if ((t)->GetChar( endpt) != '\n') {
        (t)->InsertCharacters( endpt++, "\n\n", 2);
    }
    else
        if ((t)->GetChar( endpt) != '\n') {
            (t)->InsertCharacters( endpt, "\n", 1);
        }
        else {
            --endpt;
        }

    /*
     * Note that endpt is actually one prior to the last character inserted.
     * This makes the styles work better below.
     */
    qss = (t->styleSheet)->Find( "quotation");
    bss = (t->styleSheet)->Find( "excerptedcaption");
    if (bss) {
        (t->rootEnvironment)->WrapStyle( orgpos, caplen, bss);
    }
    if (qss) {
        int             envstart = -1;
        char            c;
        class environment *et;

        for (pos = orgpos + caplen + 1; pos < endpt; ++pos) {
            c = (t)->GetChar( pos);
            if (c == '\n') {
                if (envstart >= 0) {
                    et = (t->rootEnvironment)->WrapStyle( envstart, pos - envstart + 1, qss);
                    (et)->SetStyle( FALSE, FALSE);
                    envstart = -1;
                }
            }
            else {
                if (envstart < 0) {
                    envstart = pos;
                }
            }
        }
        if (envstart >= 0) {
            et = (t->rootEnvironment)->WrapStyle( envstart, pos - envstart + 1, qss);
            (et)->SetStyle( FALSE, FALSE);
        }
    }
    else {
        message::DisplayString(this, 10, "There is no quotation style in the stylesheet!");
    }
    unlink(TempFile);
    (this->BodyTextview)->WantUpdate( this->BodyTextview);
    ams::WaitCursor(FALSE);
}

void            UserWantsAHeader(class sendmessage  *self, char            *head)
{
    char            newheader[150], *s;
    class textview *hv;
    class im      *myim = (self)->GetIM();

    hv = self->HeadTextview;
    if (head || myim) {
	if((myim)->GetInputFocus() != (class view *) hv)
	    (self)->WantInputFocus((class view*) hv);
        if (head && *head != '?') {
            strcpy(newheader, head);
        }
        else {
            if (message::AskForString(hv, 50, "Enter the name of the header you want to add: ", head ? ++head : "", newheader, sizeof(newheader)) < 0)
                return;
        }
        if (newheader[0] == '\0')
            return;
        s = strchr(newheader, ':');
        if (!s)
            strcat(newheader, ": ");
        (self)->AddHeaderLine( newheader);
        (hv)->SetDotLength( 0);
        (hv)->WantUpdate( hv);
        message::DisplayString(hv, 10, "Done.");
    }
    else {
        textv_OpenLineCmd(self->BodyTextview,0);
    }
}

void  BeginLine(class sendmessage  *sm)
{
    class textview *v;
    int   dot, len;
    char   c;
    class im   *myim = (sm)->GetIM();

    v = sm->HeadTextview;
    if (myim && ((myim)->GetInputFocus() == (class view *) v)) {
        textv_BeginningOfLineCmd(sm->HeadTextview, 0);
        len = (sm->HeadText)->GetLength();
        dot = (v)->GetDotPosition();
        c = (sm->HeadText)->GetChar( dot);
        if ((c == ' ') || (c == '\t')) {
            ++dot;
        }
        else {
            while ((c != ':') && (c != '\n') && (dot <= len)) {
                c = (sm->HeadText)->GetChar( ++dot);
            }
            c = (sm->HeadText)->GetChar( ++dot);
            while ((c == ' ') || (c == '\t') && (dot <= len)) {
                c = (sm->HeadText)->GetChar( ++dot);
            }
        }
        (v)->SetDotPosition( dot);
        (v)->WantUpdate( v);
    }
    else {
        textv_BeginningOfLineCmd(sm->BodyTextview,0);
    }
}

void  NextLine(class sendmessage  *sm, int  IsNewline)
{
    class textview *v;
    int             dot;
    class im      *myim = (sm)->GetIM();

    v = sm->HeadTextview;
    if (myim && ((myim)->GetInputFocus() == (class view *) v)) {
        dot = (v)->GetDotPosition();
        textv_NextLineCmd(v,0);
        textv_EndOfLineCmd(v,0);
        if (dot == (v)->GetDotPosition()) {
            (sm->BodyTextview)->WantInputFocus( sm->BodyTextview);
        }
        else {
            (v)->WantUpdate( v);
        }
    }
    else {
        if (IsNewline) {
            textv_InsertNewlineCmd(sm->BodyTextview,0);
        }
        else {
            textv_NextLineCmd(sm->BodyTextview,0);
        }
    }
}

void  PreviousLine(class sendmessage  *sm)
{
    int   dot;
    class textview *v;
    class im      *myim = (sm)->GetIM();

    v = sm->HeadTextview;
    if (myim && ((myim)->GetInputFocus() == (class view *) v)) {
        textv_PreviousLineCmd(v,0);
        textv_EndOfLineCmd(v,0);
        (v)->WantUpdate( v);
    }
    else {
        v = sm->BodyTextview;
        dot = (v)->GetDotPosition();
        textv_PreviousLineCmd(v,0);
        if (dot == (v)->GetDotPosition()) {
            (sm->HeadTextview)->SetDotPosition( (sm->HeadText)->GetLength());
            (sm->HeadTextview)->WantInputFocus( sm->HeadTextview);
        }
        else {
            (v)->WantUpdate( v);
        }
    }
}

void            sendmessage_InsertFile(class sendmessage  *sendmessage, char            *fname)
{
    class textview *tv;
    class text    *t;
    class im      *myim = (sendmessage)->GetIM();

    if (myim && ((myim)->GetInputFocus() == (class view *) sendmessage->HeadTextview)) {
        tv = sendmessage->HeadTextview;
        t = sendmessage->HeadText;
    }
    else {
        tv = sendmessage->BodyTextview;
        t = sendmessage->BodyText;
    }
    if (fname) {
        int             pos;

        pos = (tv)->GetDotPosition();
        DirectlyInsertFile(tv, t, fname, pos);
    }
    else {
        textv_InsertFileCmd(tv,0);
    }
}

void PrepareBodyForSignature(class sendmessage  *self)
{
    class text    *t = self->BodyText;
    class textview *tv = self->BodyTextview;
    int             len = (t)->GetLength();
    char            c = (t)->GetChar( len - 1);
    int             dotpos = (tv)->GetDotPosition();
    int             dotlen = (tv)->GetDotLength();

    if (c != '\n') {
        (t)->AlwaysInsertCharacters( len, "\n", 1);
        ++len;
    }
    (tv)->SetDotPosition( len - 1);
    (tv)->SetDotLength( 1);
    textv_PlainestCmd(tv,0);
    (tv)->SetDotPosition( dotpos);
    (tv)->SetDotLength( dotlen);
}

void SetMyFrameTitle(class sendmessage  *sm, char            *tit)
{
    (sm->myframe)->SetTitle( tit);
}

/*static tree23int_efptr AddIfView;*/

static boolean  AddIfView(class environment  *env, int *ct)
{
    if (env) {
        *ct += EnvViewCt(env);
        if (env->type == environment_View)
            ++(*ct);
    }
    return (FALSE);
}

int EnvViewCt(class environment  *env)
{
    int  total = 0;
    class nestedmark *nm;

    nm = (class nestedmark *) env;
    if (nm->children) {
	(nm->children)->Enumerate( (tree23int_efptr)AddIfView, (char *)(&total));
    }
    return (total);
}
