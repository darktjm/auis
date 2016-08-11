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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/foldaux.C,v 1.7 1995/11/07 20:17:10 robr Stab74 $";
#endif


 

#include <andrewos.h>
#include <sys/param.h>
#include <errprntf.h>

#include <textview.H>

#include <keystate.H>
#include <proctable.H>
#include <menulist.H>
#include <im.H>
#include <frame.H>
#include <message.H>
#include <environ.H>
#include <style.H>
#include <fontdesc.H>
#include <cursor.H>
#include <text.H>
#include <filetype.H>
#include <environment.H>

#include <cui.h>
#include <fdphack.h>

#include <ams.H>
#include <amsutil.H>
#include <captions.H>

#define dontDefineRoutinesFor_sendmessage
#include <sendmessage.H>
#undef dontDefineRoutinesFor_sendmessage
#include <t822view.H>

#define AUXMODULE 1
#include <folders.H>
#undef AUXMODULE

#ifndef NORCSID
#endif
void folders_Warp(class im  *im);
void folders_Expose(class im  *im);
void folders_Hide(class im  *im);
void folders_Vanish(class im  *im);
void folders_ForceUpdate();
void folders_TextviewCompound(class textview  *tv, char  *cmds);
void folders_FoldersCompound(class folders  *self, char  *cmds);
void FoldersTextviewCommand(class folders  *self, char  *cmds);
void FoldersMessagesCommand(class folders  *self, char  *cmds);
void FoldersSendmessageCommand(class folders  *self, char  *cmds);
void FoldersCaptionsCommand(class folders  *self, char  *cmds);
void folders_DownFocus(class folders  *self);
void folders_UpFocus(class folders  *self);
void folders_SimulateLeftClick(class folders  *self);
void folders_FinalizeProcStyleStuff(class folders  *self);
void folders_CreateFoldersCursor(class folders  *self);
static void QAddToDoc(class text  *d, int  *pos, char  *text, int  tlen , class style  *ss, int  stylelen );
void folders_ConsiderResettingDescription(class folders  *ci, int  code, Boolean  FirstTime);

extern void folders_DoClick(class folders  *folders, boolean  IsLeftClick , boolean  IgnorePosition);
extern int folders_SetupList(class folders  *ci, int  code, char  *thingstoread[]);
extern void folders_ClearFolders(class folders  *ci);
extern void folders_ExposeCap(class folders  *self);

void folders_Warp(class im  *im)
{

    if(im) {
	(im)->SetWMFocus();
	im::SetLastUsed(im);
    }

}

void folders_Expose(class im  *im)
{
    if(im) (im)->ExposeWindow();
}

void folders_Hide(class im  *im)
{
    if(im) (im)->HideWindow();
}

void folders_Vanish(class im  *im)
{
    if(im) (im)->VanishWindow();
}

void folders_ForceUpdate()
{
    im::ForceUpdate();
}

void folders_TextviewCompound(class textview  *tv, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( tv, "textview", cmds);
}

void folders_FoldersCompound(class folders  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( self, "folders", cmds);
}

void FoldersTextviewCommand(class folders  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( self, "textview", cmds);
}

void FoldersMessagesCommand(class folders  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( self, "messages", cmds);
}

void FoldersSendmessageCommand(class folders  *self, char  *cmds)
{
    if (self->sm) {
	(ams::GetAMS())->GenericCompoundAction( self->sm, "sendmessage", cmds);
    }
}

void FoldersCaptionsCommand(class folders  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( (self)->GetCaptions(), "captions", cmds);
}

void folders_DownFocus(class folders  *self)
{
    ams::Focus((self)->GetCaptions());
}

void folders_UpFocus(class folders  *self)
{
    if (self->sm) {
	ams::Focus(self->sm->BodyTextview);
    } else {
	ams::Focus(((self)->GetCaptions())->GetBodView());
    }
}

void folders_SimulateLeftClick(class folders  *self)
{
    folders_DoClick(self, TRUE, TRUE);
}

void folders::PostKeyState(class keystate  *ks)
{
    this->mykeys->next = NULL;
    if (amsutil::GetOptBit(EXP_KEYSTROKES)) {
	if (ks) (ks)->AddAfter( this->mykeys);
	(this)->messages::PostKeyState( this->mykeys);
    } else {
	(this)->messages::PostKeyState( ks);
    }
}

void folders::PostMenus(class menulist  *ml)
{
    (this->mymenulist)->ClearChain();
    if (ml) (this->mymenulist)->ChainAfterML( ml, (long)ml);
    (this)->messages::PostMenus( this->mymenulist);
}

void folders_FinalizeProcStyleStuff(class folders  *self)
{
    delete self->mykeys;
    delete self->mymenulist;
    delete self->Activefolderstyle;
    delete self->Normalfolderstyle;
    delete self->GlobalCapStyle;
    delete self->IconicStyle;
    delete self->BoldStyle;
    delete self->ItalicStyle;
    delete self->CenterStyle;
    delete self->BigCenterStyle;
    delete self->mycursor;
}

void folders_CreateFoldersCursor(class folders  *self)
{
    class fontdesc *fd;

    fd = fontdesc::Create("icon", 0, 12);
    self->mycursor = cursor::Create(self);
    (self->mycursor)->SetGlyph( fd, 'R');
}

static int lastconfiguration = -999;

void folders::Reconfigure(int  listcode)
{
#ifndef RCH_ENV
    /* Our user's don't like this because Exposed Changed breaks. */
    if (lastconfiguration == listcode) return;
#endif
    ams::WaitCursor(TRUE);
    this->HasSetUp = 0;
    folders_SetupList(this, listcode, NULL);
    lastconfiguration = this->CurrentConfiguration = listcode;
    (this)->PostMenus( NULL);
    ams::WaitCursor(FALSE);
}

void folders::UpdateMsgs(int  mailonly, char  *thingstoread[], boolean  ShowHelp) 
{
    if (ShowHelp) {
	if (this->mycaps) {
	    if (this->mycaps->BodView) {
		(this->mycaps->BodView)->ShowHelp( NULL);
	    }
	    (this->mycaps)->ClearAndUpdate( FALSE, TRUE);
	    (this->mycaps)->ShowHelp();
	}
	(this)->ShowHelp();
	im::ForceUpdate();
    }
    this->HasSetUp = 0;
    if ((ams::GetAMS())->OnlyMail()) mailonly = 1;
    this->MailOnlyMode = mailonly;
    ams::WaitCursor(TRUE);
    if (thingstoread) {
	this->ShowingAsRequested = 1;
	folders_SetupList(this, LIST_AS_REQUESTED, thingstoread);
	lastconfiguration = LIST_AS_REQUESTED;
    } else {
	(ams::GetAMS())->CUI_CheckMailboxes( mailonly ? ams::GetMailPath() : NULL);
	this->ShowingAsRequested = 0;
	folders_SetupList(this, LIST_NEWONLY, NULL);
	lastconfiguration = LIST_NEWONLY;
    }
    if (!amsutil::GetOptBit(EXP_NOFIRSTFOLDER)) {
	if (this->MainDirCacheCount > 0) {
	    int which, lim;

	    lim = this->MainDirCacheCount;
	    for(which=0; which<lim && this->MainDirCache[which].SkipMe; ++which) {
		;
	    }
	    if (which < lim) {
		((this)->GetCaptions())->InsertUpdatesInDocument( this->MainDirCache[which].ShortName, this->MainDirCache[which].FullName, FALSE);
	    }
	} else {
	    folders_ClearFolders(this);
	    (this)->ReadMail( FALSE);
	}
    }
    (this)->WantInputFocus( this);
    ams::WaitCursor(FALSE);
}

static void QAddToDoc(class text  *d, int  *pos, char  *text, int  tlen , class style  *ss, int  stylelen )
{
    class environment *et;

    (d)->AlwaysInsertCharacters( *pos, text, tlen);
    if (ss) {
	et = (((class text *)d)->rootEnvironment)->InsertStyle( *pos, ss, 1);
	(et)->SetLength( stylelen);
    }
    *pos += tlen;
}

static char *E1 = "   (NOT the currently-displayed folder)";
static char *E2 = "Folder name: ";
static char *E3 = "\nFolder type: ";
static char *E4 = "\nNumber of messages: ";
static char *E5 = "\nYour subscription status: ";
static char *E6 = "\n\nExplanation of this message folder:\n\n";
static char *E7 = "\n\nNo explanation of this folder is available, but here is the first message:\n\n";
static char *E8 = "\n\nNo explanation of this folder is available.";

void folders::ExplainDir(char  *FullName , char  *nickname)
{
    int ProtCode, MsgCount;
    char ErrorText[100+MAXPATHLEN], *TypeStr, *SubsStr, ExpFileName[1+MAXPATHLEN], LocalFileName[1+MAXPATHLEN];
    class text *d;
    int pos = 0, substatus, ShouldDelete, fpos;
    long mcode;

    ((this)->GetCaptions())->ResetVisibleCaption();
    (((this)->GetCaptions())->GetBodView())->SetDotPosition( 0);
    (((this)->GetCaptions())->GetBodView())->SetDotLength( 0);
    (((this)->GetCaptions())->GetBodView())->FrameDot( 0);

    d = ((this)->GetCaptions())->GetBodDoc();

    mcode = (ams::GetAMS())->MS_GetDirInfo( FullName, &ProtCode, &MsgCount);
    if (mcode) {
	if ((ams::GetAMS())->AMS_ErrNo() == EACCES) {
	    sprintf(ErrorText, "'%s' is private; you don't have read-access or are unauthenticated.", nickname);
	} else if ((ams::GetAMS())->vdown( (ams::GetAMS())->AMS_ErrNo())) {
	    sprintf(ErrorText, "%s: temporarily unavailable (net/server problem)", nickname);
	} else if ((ams::GetAMS())->AMS_ErrNo() == ENOENT) {
	    (ams::GetAMS())->CUI_HandleMissingFolder( FullName);
	    return;
	} else {
	    sprintf(ErrorText, "Cannot look up information about %s", FullName);
	    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, TRUE, mcode);
	}
	message::DisplayString(NULL, 75, ErrorText);
	return;
    } else {
	TypeStr = (ams::GetAMS())->DescribeProt( ProtCode);
    }
    if (mcode = (ams::GetAMS())->MS_GetSubscriptionEntry( FullName, ErrorText, &substatus)) {
	(ams::GetAMS())->ReportError( "Cannot get subscription entry", ERR_WARNING, TRUE, mcode);
	SubsStr = "Lookup error";
    } else {
	switch(substatus) {
		case AMS_ALWAYSSUBSCRIBED:
		    SubsStr = "Subscribed";
		    break;
		case AMS_UNSUBSCRIBED:
		    SubsStr = "Not subscribed";
		    break;
		case AMS_ASKSUBSCRIBED:
		    SubsStr = "Ask-subscribed";
		    break;
		case AMS_SHOWALLSUBSCRIBED:
		    SubsStr = "Show-all subscribed";
		    break;
		case AMS_PRINTSUBSCRIBED:
		    SubsStr = "Print-subscribed";
		    break;
		default:
		    SubsStr = "unknown";
		    break;
	}
    }
    QAddToDoc(d, &pos, E2, strlen(E2), this->BoldStyle, strlen(E2) - 1);
    QAddToDoc(d, &pos, nickname, strlen(nickname), NULL, 0);
    if ((this)->GetCaptions()->FullName && strcmp(FullName, (this)->GetCaptions()->FullName)) {
	QAddToDoc(d, &pos, E1, strlen(E1), this->ItalicStyle, strlen(E1));
    }
    QAddToDoc(d, &pos, E3, strlen(E3), this->BoldStyle, strlen(E3) - 1);
    QAddToDoc(d, &pos, TypeStr, strlen(TypeStr), NULL, 0);
    QAddToDoc(d, &pos, E4, strlen(E4), this->BoldStyle, strlen(E4)-1);
    sprintf(ErrorText, "%d", MsgCount);
    QAddToDoc(d, &pos, ErrorText, strlen(ErrorText), NULL, 0);
    QAddToDoc(d, &pos, E5, strlen(E5), this->BoldStyle, strlen(E5)-1);
    QAddToDoc(d, &pos, SubsStr, strlen(SubsStr), NULL, 0);

    sprintf(ErrorText, "%s/%s", FullName, AMS_EXPLANATIONFILE);
    mcode = (ams::GetAMS())->MS_DisambiguateFile( ErrorText, ExpFileName, AMS_DISAMB_EXISTS);
    if (mcode) {
	if ((ams::GetAMS())->AMS_ErrNo() == ENOENT) {
	    if (MsgCount > 0) {
		FILE *fp;
		char SnapshotBuf[AMS_SNAPSHOTSIZE], LineBuf[2000], *objtype;
		int cuid, IsDup;
		long numbytes, bytesleft, myid = 0;

		QAddToDoc(d, &pos, E7, strlen(E7), this->ItalicStyle, strlen(E7)-1);
		if ((ams::GetAMS())->CUI_GetHeaders( FullName, "000000", SnapshotBuf, AMS_SNAPSHOTSIZE, 0, &numbytes, &bytesleft, TRUE)) {
		    (ams::GetAMS())->ReportError( "Could not get first notice text", ERR_WARNING, TRUE, mcode);
		    return;
		}
		cuid = (ams::GetAMS())->CUI_GetCuid( AMS_ID(SnapshotBuf), FullName, &IsDup);
		if ((ams::GetAMS())->CUI_ReallyGetBodyToLocalFile( cuid, LocalFileName, &ShouldDelete, !(ams::GetAMS())->CUI_SnapIsRunning())) {
		    return; /* error already reported */
		}
#ifdef RCH_ENV
                d->AlwaysInsertFile(NULL, LocalFileName, pos);
#else
		fp = fopen(LocalFileName, "r");
		if (!fp) {
		    (ams::GetAMS())->ReportError( "Could not open initial message to display it", ERR_WARNING, FALSE, 0);
		    return;
		}
		while (fgets(LineBuf, sizeof(LineBuf), fp)) {
		    if (LineBuf[0] == '\n') break;
		}
		fpos = ftell(fp);
		objtype = filetype::Lookup(fp, NULL, &myid, NULL);
		if (ftell(fp) == 0) {
		    fseek(fp, fpos, 0);
		}
		if (objtype && strcmp(objtype, "text")) {
		    myid = 0;
		    message::DisplayString(NULL, 80, "ATK message does not contain a top-level text object!");
		}
		(d)->SetReadOnly( FALSE);
		(d)->ReadSubString( pos, fp, 1);
		(d)->SetReadOnly( TRUE);
		fclose(fp);
#endif /* RCH_ENV */
		if (ShouldDelete) unlink(LocalFileName);
	    } else {
		QAddToDoc(d, &pos, E8, strlen(E8), this->ItalicStyle, strlen(E8)-1);
	    }
	} else {
	    (ams::GetAMS())->ReportError( "Cannot get explanation of messages folder", ERR_WARNING, TRUE, mcode);
	    QAddToDoc(d, &pos, E8, strlen(E8), this->ItalicStyle, strlen(E8)-1);
	}
    } else {
	int fd, bytes;
	char Splat[5000];

	QAddToDoc(d, &pos, E6, strlen(E6), this->ItalicStyle, strlen(E6)-1);
	(ams::GetAMS())->CUI_GenTmpFileName( LocalFileName);
	if ((ams::GetAMS())->CUI_GetFileFromVice( LocalFileName, ExpFileName)) {
	    (ams::GetAMS())->ReportError( "Cannot get explanation file from AFS", ERR_WARNING, TRUE, (ams::GetAMS())->MSErrCode());
	    return;
	}
#ifdef RCH_ENV
        d->AlwaysInsertFile(NULL, LocalFileName, pos);
#else
	fd = open(LocalFileName, O_RDONLY, 0644);
	if (fd<0) {
	    sprintf(ErrorText, "Cannot open local help file %s (%d)", LocalFileName, errno);
	    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, FALSE, 0);
	    unlink(LocalFileName);
	    return;
	}
	while((bytes = read(fd, Splat, sizeof(Splat))) > 0) {
	    (d)->AlwaysInsertCharacters( pos, Splat, bytes);
	    pos += bytes;
	}
	close(fd);
	if (bytes<0) {
	    sprintf(ErrorText, "Cannot read from local help file %s (%d)", LocalFileName, errno);
	    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, FALSE, 0);
	}
        unlink(LocalFileName);
#endif /* RCH_ENV */
    }
    folders_ExposeCap(this);
}

int folders::WriteFormattedBodyFile(char  *fname , char  *captbuf)
{
    FILE *fp;
    class text *t, *new_c = new text;
    class t822view *bv;
    class captions *mycap;

    mycap = (this)->GetCaptions();
    if (mycap->VisibleCUID >= 1) {
	strcpy(captbuf, AMS_CAPTION(mycap->VisibleSnapshot));
    } else {
	*captbuf = '\0';
    }
    (ams::GetAMS())->CUI_GenTmpFileName( fname);
    fp = fopen(fname, "w");
    if (!fp) return(-1);
    t = (mycap)->GetBodDoc();
    bv = (mycap)->GetBodView();
    (t)->SetWriteID( im::GetWriteID());
    if ((bv)->GetDotLength() > 0) {
	(new_c)->AlwaysCopyText( 0, t, (bv)->GetDotPosition(), (bv)->GetDotLength());
    } else {
	(new_c)->AlwaysCopyText( 0, t, mycap->StartOfRealBody, (t)->GetLength() - mycap->StartOfRealBody);
    }
    (new_c)->Write( fp, im::GetWriteID(), 0);
    (new_c)->Destroy();
    return(fclose(fp));
}

void folders_ConsiderResettingDescription(class folders  *ci, int  code, Boolean  FirstTime)
{
    char Label[256], MessageText[256];
    char *PluralString;

    PluralString = (ci->MainDirCacheCount == 1) ? "" : "s";
    switch(code) {
	case LIST_ALL_FOLDERS:
	    sprintf(MessageText, "Exposed a list of all %s folders on your search path.", amsutil::cvEng(ci->MainDirCacheCount, 0, 1000));
	    sprintf(Label, "All %d Folders", ci->MainDirCacheCount);
	    break;
	case LIST_SUBSCRIBED:
	    sprintf(MessageText, "Exposed a list of the %s folder%s you subscribe to.", amsutil::cvEng(ci->MainDirCacheCount, 0, 1000), PluralString);
	    sprintf(Label, "%d Subscribed Folder%s", ci->MainDirCacheCount, PluralString);
	    break;
	case LIST_MAIL_FOLDERS:
	    sprintf(MessageText, "Exposed a list of %s personal mail folder%s.", amsutil::cvEng(ci->MainDirCacheCount, 0, 1000), PluralString);
	    sprintf(Label, "%d Mail Folder%s", ci->MainDirCacheCount, PluralString);
	    break;
	case LIST_AS_REQUESTED:
	    sprintf(MessageText, "Exposed a list of the %s folder%s you requested.", amsutil::cvEng(ci->MainDirCacheCount, 0, 1000), PluralString);
	    sprintf(Label, "%d Requested Folder%s", ci->MainDirCacheCount, PluralString);
	    break;
	case LIST_NEWONLY:
	    if (FirstTime) {
		/* printed a more detailed message elsewhere */
		MessageText[0] = '\0'; 
	    } else {
		if (ci->ShowingAsRequested) {
		    sprintf(MessageText, "Exposed a list of %s folder%s.", amsutil::cvEng(ci->MainDirCacheCount, 0, 1000), PluralString);
		} else {
		    sprintf(MessageText, "Exposed a list of your %s subscription%s with new messages.", amsutil::cvEng(ci->MainDirCacheCount, 0, 1000), PluralString);
		}
	    }
	    if (ci->ShowingAsRequested) {
		sprintf(Label, "%d Requested Folder%s", ci->MainDirCacheCount, PluralString);
	    } else {
		sprintf(Label, "%d Changed Folder%s", ci->MainDirCacheCount, PluralString);
	    }
	    break;
	default:
	    MessageText[0] = '\0';
	    Label[0] = '\0';
    }
    if (ci->myframe) (ci->myframe)->SetTitle( Label);
    if (MessageText[0]) {
	message::DisplayString(NULL, 10, MessageText);
    }
}

