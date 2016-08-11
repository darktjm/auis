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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/capaux.C,v 1.9 1996/08/27 22:19:24 robr Exp $";
#endif


 

#include <andrewos.h>
#include <sys/param.h>

#include <textview.H>
   
#include <im.H>
#include <frame.H>
#include <environment.H>
#include <message.H>
#include <bind.H>
#include <proctable.H>
#include <keystate.H>
#include <menulist.H>
#include <text.H>
#include <textview.H>
#define dontDefineRoutinesFor_scroll
#include <scroll.H>
#undef dontDefineRoutinesFor_scroll
#include <search.H>
#include <style.H>
#include <stylesheet.H>


#include <cui.h>
#include <fdphack.h>
#include <errprntf.h>

#include <ams.H>
#include <text822.H>
#include <t822view.H>
#include <amsutil.H>
#include <folders.H>
#define AUXMODULE 1
#include <captions.H>


extern class keymap *captions_privkeymap;
extern class menulist *captions_privmenulist;

static long AbsentProcedure(class ATK  *self, long rock);
void captions_CaptionsCompound(class captions  *self, char  *cmds);
void captions_CaptionsTextviewCommand(class captions  *self, char  *cmds);
void captions_CaptionsFoldersCommand(class captions  *self, char  *cmds);
void captions_CaptionsBodiesCommand(class captions  *self, char  *cmds);
void captions_DownFocus(class captions  *self);
void captions_UpFocus(class captions  *self);
void ClassifyMarkedByName(class captions  *self, char  *NameGiven);
void captions_SimulateLeftClick( class captions  *self, long  rock );
void captions_SimulateRightClick( class captions  *self, long  rock );
void CapBeginText(class captions  *self);
void CapScrollBack(class captions  *self);
void PreviousCaptionLine(class captions  *self);
void CapGlitchDown(class captions  *self);
void captions_PurgeDeletions(class captions  *ci);
int captions_InsertCaptions(class captions  *ci, char  *shortname , char  *dname , char  *StartTime, Boolean  ShowAll);
void text_CleanUpGlitches(class text  *self);
void OneTimeInitKeyMenus(struct ATKregistryEntry   *ci);

extern int GetSouthernmostPoint(class captions  *ci);
extern void SetSouthernmostPoint(class captions  *ci, int  pos);
extern void ResetCaptionNotBody(class captions  *ci);
extern void MakeCaptionLine(char  **Buf , int  cuid , char  *RawSnapshot, int  Fixed , int  *HighStart , int  *HighLen, Boolean  IsMail , Boolean  IsDup , Boolean  IsRead);
extern void AddCaptionToCacheEntry(struct CaptionCache  **ccache, int  *ct , int  *size, int  cuid , int  offset, class environment  *env , class environment  *iconenv, Boolean  MayModify , char  *snapshot, Boolean  IsDup);
extern void MergeTwoCacheEntries(class captions  *ci, struct CaptionCache  *ccache, int  cct , int  csize , int  prefixend);
extern void MarkVisibleMessageSeen(class captions  *ci);
extern void RemoveHighlighting(class captions  *h);

proctable_fptr captextv_PreviousLineCmd = AbsentProcedure,
    captextv_ReverseSearchCmd = AbsentProcedure,
    captextv_ScrollScreenFwdCmd = AbsentProcedure,
    captextv_ScrollScreenBackCmd = AbsentProcedure,
    captextv_BeginningOfTextCmd = AbsentProcedure,
    captextv_BeginningOfLineCmd = AbsentProcedure,
    captextv_EndOfLineCmd = AbsentProcedure,
    captextv_GlitchDownCmd = AbsentProcedure;


static long AbsentProcedure(class ATK  *, long) 
{
    message::DisplayString(NULL, 75, "Absent procedure - did not find a normal BE2 command in the proctable!");
    return 0;
}

void captions_CaptionsCompound(class captions  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( self, "captions", cmds);
}

void captions_CaptionsTextviewCommand(class captions  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( self, "textview", cmds);
}

void captions_CaptionsFoldersCommand(class captions  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( (self)->GetFolders(), "folders", cmds);
}

void captions_CaptionsBodiesCommand(class captions  *self, char  *cmds)
{
    (ams::GetAMS())->GenericCompoundAction( (self)->GetBodView(), "t822view", cmds);
}

void captions_DownFocus(class captions  *self)
{
    ams::Focus((self)->GetBodView());
}

void captions_UpFocus(class captions  *self)
{
    ams::Focus((self)->GetFolders());
}

void captions::ClearMarks()
{
    int j, whicholdmark = 0;
    struct CaptionCache *hc;

    if (this->MarkCount <= 0) {
	message::DisplayString(NULL, 10, "There are no marked messages");
	return;
    }
    ams::WaitCursor(TRUE);
    if (this->OldMarks) free(this->OldMarks);
    this->OldMarks = (int *) malloc((1+this->MarkCount)*sizeof(long));
    if (this->OldMarks) this->OldMarkCount = this->MarkCount;
    for (j = 0; j < this->captioncachecount; ++j) {
	hc = &this->capcache[j];
	if (hc->IsMarked) {
	    if (this->OldMarks) this->OldMarks[whicholdmark++] = hc->cuid;
	    (this)->ToggleMark( hc, hc->offset);
	}
    }
    this->MarkCount = 0;
    (this)->PostMenus( NULL);
    message::DisplayString(NULL, 10, "All marks cleared; there are now no messages marked.");
    (this)->WantUpdate( this);
    ams::WaitCursor(FALSE);
}

void ClassifyMarkedByName(class captions  *self, char  *NameGiven)
{
    int code;

    if (*NameGiven == '*') {
	++NameGiven;
	code = MARKACTION_APPENDBYNAME;
    } else {
	code = MARKACTION_CLASSIFYBYNAME;
    }
    (self)->ActOnMarkedMessages( code, NameGiven);
}

void captions_SimulateLeftClick( class captions  *self, long  rock )
        {
    (self)->SimulateClick( TRUE);
}

void captions_SimulateRightClick( class captions  *self, long  rock )
        {
    (self)->SimulateClick( FALSE);
}

void captions::PrintVisibleMessage() 
{
    int flags = 0;

    if (this->VisibleCUID <1) {
	message::DisplayString(NULL, 10, "There is nothing to print.");
	return;
    }
    ams::WaitCursor(TRUE);
    if (this->CurrentFormatting & MODE822_ROT13) {
	flags |= AMS_PRINT_ROT13;
    }
    if (this->CurrentFormatting & MODE822_FIXEDWIDTH) {
	flags |= AMS_PRINT_FIXED;
    }
    message::DisplayString(NULL, 10, "Printing message; please wait...");
    im::ForceUpdate();
    if ((ams::GetAMS())->CUI_PrintBodyFromCUIDWithFlags( this->VisibleCUID, flags, NULL) == 0) {
	message::DisplayString(NULL, 10, "Message queued for printing.");
    }
    ams::WaitCursor(FALSE);
}


void captions::PuntCurrent(Boolean  GoToNext)
{
    int loops = 1;
    class im *im = (this)->GetIM();
    char Scratch[100+MAXPATHLEN];

    if (im && (im)->ArgProvided()) {
	loops = (im)->Argument();
	if (loops < 0) {
	    loops = 1;
	} else if (loops > 1) {
	    sprintf(Scratch, "Do you really want to punt the next %d folders", loops);
	    if (!(ams::GetAMS())->GetBooleanFromUser( Scratch, FALSE)) {
		return;
	    }
	}
    }
    while (loops--) {
	if (this->captioncachecount > 0) {
	    SetSouthernmostPoint(this, this->capcache[this->captioncachecount-1].offset);
	}
	if (GoToNext) {
	    ((this)->GetFolders())->NextFolder( FALSE);
	}
	if (loops>0) im::ForceUpdate();
    }
}

void captions::ThisIsFlorida()
{
    int pos, len, whichcaption;
    class environment *env;

    im::ForceUpdate();
    /* The following line makes things work better if this caption is the first one fetched into the caption cache but not the first in the folder */
    (this)->GuaranteeFetchedRange( this->FolderSize - this->FetchedFromEnd - 1, this->FolderSize);
    pos = this->HighlightStartPos;
    (void) (this)->FindCUIDByDocLocation( &pos, &len, &env, &whichcaption);
    SetSouthernmostPoint(this, -1); /* Always needed */
    if (--whichcaption >= 0) {
	pos = this->capcache[whichcaption].offset;
	(void) (this)->FindCUIDByDocLocation( &pos, &len, &env, &whichcaption);
	SetSouthernmostPoint(this, pos);
    }
    im::ForceUpdate();
}

void CapBeginText(class captions  *self)
{
    (self)->GuaranteeFetchedRange( 0, self->FolderSize);
    im::ForceUpdate();
    (*captextv_BeginningOfTextCmd)(self,0);
}

void CapScrollBack(class captions  *self)
{
    int min, pos, mylen, whichcaption;
    struct range total, seen, dot;
    class environment *envptr;

    self->textscrollinterface->GetInfo(self, &total, &seen, &dot);
    pos = (self)->DecodePosition( seen.beg);
    /* The following conservatively assume a 6 pt font, 4 pts spacing, maximum of one screen of scrolling */
    (self)->FindCUIDByDocLocation( &pos, &mylen, &envptr, &whichcaption);
    min = self->FolderSize - self->FetchedFromEnd - ((self)->GetLogicalHeight()/10) + whichcaption - 1;
    if (min < 0) min = 0;
    (self)->GuaranteeFetchedRange( min, self->FolderSize);
    im::ForceUpdate();
    captextv_ScrollScreenBackCmd((class textview *) self,0);
}

void captions::CapReverseSearch()
{
    (this)->GuaranteeFetchedRange( 0, this->FolderSize);
    im::ForceUpdate();
    captextv_ReverseSearchCmd((class textview *) this,0);
    (this)->WantInputFocus( this);
}

void PreviousCaptionLine(class captions  *self)
{
    int backupto = self->FolderSize - self->FetchedFromEnd -2;
    class im *im = (self)->GetIM();

    if (im && (im)->ArgProvided()) {
	backupto -= (im)->Argument();
    }
    if ((self)->GetDotPosition() < 150) { /* poor heuristic */
	if ((self)->GuaranteeFetchedRange( backupto, self->FolderSize)) return; /* error reported */
    }
    captextv_PreviousLineCmd((class textview *) self, 0);
}

void CapGlitchDown(class captions  *self)
{
    int backupto = self->FolderSize - self->FetchedFromEnd -2;
    class im *im = (self)->GetIM();
    
    if (im && (im)->ArgProvided()) {
	backupto -= (im)->Argument();
    }
    if ((self)->GuaranteeFetchedRange( backupto, self->FolderSize)) return;
    im::ForceUpdate();
    captextv_GlitchDownCmd((class textview *) self,0);
}

void captions_PurgeDeletions(class captions  *ci)
{
    message::DisplayString(NULL, 10, "Purging deletions; please wait...");
    im::ForceUpdate();
    ams::WaitCursor(TRUE);
    if ((ams::GetAMS())->CUI_PurgeDeletions( ci->FullName) == 0) {
	message::DisplayString(NULL, 10, "Purging complete.");
    }
    ams::WaitCursor(FALSE);
}

void captions::ClearAndUpdate(int  ConsiderPurging , int  SaveState) 
{
    class text *d;

    d = this->CaptText;
    if (SaveState) (this)->MakeCachedUpdates();
    if (ConsiderPurging
	 && (ams::GetAMS())->CUI_DoesDirNeedPurging( this->FullName)
	 && (amsutil::GetOptBit(EXP_PURGEONQUIT)
	     || (ams::GetAMS())->GetBooleanFromUser( "Do you want to purge the deleted messages", FALSE))) {
	captions_PurgeDeletions(this);
    }
    ResetCaptionNotBody(this);
    if (this->MarkCount > 0) {
	/* We clear the marks this way so that they can be restored later if so desired.  */
	(this)->ClearMarks();
	this->MarkCount = 0;
    }
    if (this->FullName) {
	free(this->FullName);
	this->FullName = NULL;
    }
    if (this->ShortName) {
	free(this->ShortName);
	this->ShortName = NULL;
    }
    this->captioncachecount = 0;
    this->FetchedFromStart = this->FetchedFromEnd = this->FolderSize = 0;
    SetSouthernmostPoint(this, -1);
    (d)->ClearCompletely();
    (d)->SetGlobalStyle( this->GlobalCapStyle);
    (this)->SetDotPosition( 0);
    (this)->SetDotLength( 0);
    (this)->PostMenus( NULL);
    (this)->WantUpdate( this);
    im::ForceUpdate();
}

int
captions_InsertCaptions(class captions  *ci, char  *shortname , char  *dname , char  *StartTime, Boolean  ShowAll)
{
    int totalbytes, cuid, addlen, highstart, highlen, envstart, inspos, insertct = 0, IsDup, myfirstcuid = 0;
    long numbytes, status;
    char date64[AMS_DATESIZE+1], olddate64[AMS_DATESIZE+1], newdate[AMS_DATESIZE+1], firstdate[AMS_DATESIZE+1], captionbuf[100*AMS_SNAPSHOTSIZE], ErrorText[256], *DirName, *ThisCaption, *s;
    Boolean UseHighDensity, MayModify, IsRead;
    class environment *et, *et2;
    int PositionDot = 1, NewCt, TotalCt, ProtCode;
    long errcode;
    struct CaptionCache *tempcache = NULL;
    int tempcachesize, tempcachecount;

    if ((errcode = (ams::GetAMS())->CUI_DisambiguateDir( dname, &DirName)) != 0) {
	if (*dname == '/' && (ams::GetAMS())->AMS_ErrNo() == ENOENT) {
	    (ams::GetAMS())->CUI_HandleMissingFolder( dname);
	    return(-1);
	} else if ((ams::GetAMS())->vdown( (ams::GetAMS())->AMS_ErrNo())) {
	    sprintf(ErrorText, "%s: temporarily unavailable (net/server problem)", shortname);
	} else if ((ams::GetAMS())->AMS_ErrNo() == EACCES) {
	    sprintf(ErrorText, "'%s' is private; you don't have read-access or are unauthenticated.", shortname);
	} else {
	    sprintf(ErrorText, "The folder %s is not readable.", shortname);
	}
	message::DisplayString(NULL, 75, ErrorText);
	return(-1);
    }
    UseHighDensity = ! (amsutil::GetOptBit(EXP_WHITESPACE));
    if (!StartTime) {
	long mytime;

	im::ForceUpdate();
	errcode = (ams::GetAMS())->MS_GetDirInfo( DirName, &ProtCode, &TotalCt);
	if (errcode) {
	    if ((ams::GetAMS())->AMS_ErrNo() == ENOENT) {
		(ams::GetAMS())->CUI_HandleMissingFolder( dname);
		return(-1);
	    }
	    sprintf(ErrorText, "Cannot look up information about %s", DirName);
	    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, TRUE, errcode);
	    return(-1);
	}
	ci->IsFullMail =  (ProtCode == AMS_DIRPROT_FULLMAIL) ? TRUE : FALSE;
	ci->FolderSize = TotalCt;
	if (errcode = (ams::GetAMS())->MS_GetNewMessageCount( DirName, &NewCt, &TotalCt, olddate64, TRUE)) {
	    sprintf(ErrorText, "Couldn't get date information for %s", DirName);
	    (ams::GetAMS())->ReportError( ErrorText, ERR_CRITICAL, TRUE, errcode);
	    return(-1);
	}
	if (NewCt == TotalCt) ShowAll = TRUE;
	if (ShowAll) {
	    date64[0] = '\0';
	    NewCt = TotalCt;
	} else {
	    mytime = amsutil::conv64tolong(olddate64);
	    if (mytime>0) {
		strcpy(date64, amsutil::convlongto64(mytime-1, 0));
	    }
	}
	inspos = insertct;
   } else {
	strcpy(date64, StartTime);
    }
    insertct = 0;
    inspos = 0;
    newdate[0] = '\0';
    totalbytes = 0;
    s = NULL;
    if (StartTime) {
	tempcache = (struct CaptionCache *) malloc(25 * sizeof(struct CaptionCache));
	tempcachesize = 25;
	tempcachecount = 0;
    }
    do {
	if ((errcode = (ams::GetAMS())->CUI_GetHeaders( DirName, date64, captionbuf, sizeof(captionbuf), totalbytes, &numbytes, &status, FALSE)) != 0) {
	    (ci)->WantUpdate( ci);
	    if ((ams::GetAMS())->vdown( (ams::GetAMS())->AMS_ErrNo())) {
		sprintf(ErrorText, "%s: temporarily unavailable (net/server problem)", DirName);
		(ams::GetAMS())->ReportSuccess( ErrorText);
	    } else {
	        sprintf(ErrorText, "Could not get captions for %s", dname);
		(ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, FALSE, 0);
	    }
	    if (tempcache) free(tempcache);
	    return(-1);
	}
	if (numbytes <= 0) break;
	totalbytes += numbytes;
	for (s=captionbuf; s-captionbuf  < numbytes; s+= AMS_SNAPSHOTSIZE) {
	    cuid = (ams::GetAMS())->CUI_GetCuid( AMS_ID(s), DirName, &IsDup);
	    if (ci->firstcuid == cuid) {
		/* All done -- we are back to where we started */
		status = 0;
		break;
	    }
	    MayModify = AMS_GET_ATTRIBUTE(s, AMS_ATT_MAYMODIFY) ? 1 : 0;
	    if (MayModify) {
		IsRead = AMS_GET_ATTRIBUTE(s, AMS_ATT_UNSEEN) ? FALSE : TRUE;
	    } else {
		IsRead = (!StartTime && (myfirstcuid || ShowAll)) ? FALSE : TRUE;
	    }
	    if (!myfirstcuid) {
		myfirstcuid = cuid;
		strcpy(firstdate, AMS_DATE(s));
	    }
	    MakeCaptionLine(&ThisCaption, cuid, s, amsutil::GetOptBit(EXP_FIXCAPTIONS), &highstart, &highlen, ci->IsFullMail, IsDup, IsRead);
	    if (!UseHighDensity) {
		strcat(ThisCaption, "\n");
	    }
	    addlen = strlen(ThisCaption);
	    (ci->CaptText)->AlwaysInsertCharacters( inspos, ThisCaption, addlen);
	    if (!StartTime && PositionDot) {
		(ci)->SetDotPosition( inspos+addlen-1);
		(ci)->FrameDot( inspos);
		PositionDot = 0;
	    }
	    insertct += addlen;
	    et2 = (ci->CaptText->rootEnvironment)->InsertStyle( inspos, ci->IconicStyle, 1);
	    (et2)->SetLength( 3);
	    envstart = inspos + 2;
	    et = (ci->CaptText->rootEnvironment)->InsertStyle( envstart, AMS_GET_ATTRIBUTE(s, AMS_ATT_DELETED) ? ci->DeletedStyle : ci->NormalCaptionStyle, 1);
	    (et)->SetLength( addlen -3);
	    if (StartTime) {
		AddCaptionToCacheEntry(&tempcache, &tempcachecount, &tempcachesize, cuid, inspos, et, et2, MayModify, s, IsDup);
	    } else {
		AddCaptionToCacheEntry(&ci->capcache, &ci->captioncachecount, &ci->captioncachesize, cuid, inspos, et, et2, MayModify, s, IsDup);
	    }
	    ++ci->FetchedFromEnd; /* sometimes from start? */
	    if (ci->IsFullMail) {
		et = (et)->InsertStyle( 0, ci->MailStyle, 1);
		(et)->SetLength( addlen - 3); 
	    }
	    if (highlen) {
		et = (et)->InsertStyle( inspos+highstart - envstart, ci->HighlightStyle, 1);
		(et)->SetLength( highlen);
	    }
	    inspos += addlen;
	}
    } while (status > 0);
    if (myfirstcuid) ci->firstcuid = myfirstcuid;
    if (GetSouthernmostPoint(ci) == 0) { /* No southernmost point yet */
	int newsouth;

	if ((ci->substatus == AMS_SHOWALLSUBSCRIBED)
	    || ShowAll
	    || (ci->captioncachecount <= 0)
	    || (!StartTime && myfirstcuid && (strcmp(firstdate, olddate64)< 0))) {
	    newsouth = -1;
	    (ci)->SetDotPosition( 0);
	} else {
	    newsouth = ci->capcache[0].offset;
	}
	SetSouthernmostPoint(ci, newsouth);
    }
    if (status < 0) {
	sprintf(ErrorText, "Couldn't read all of the captions successfully");
	(ams::GetAMS())->ReportError( ErrorText, ERR_CRITICAL, TRUE, errcode);
	if (tempcache) free(tempcache);
	return(-1);
    }
    if (ci->firstcuid) {
	if (s && !StartTime) {
	    strcpy(newdate, AMS_DATE(s - AMS_SNAPSHOTSIZE));
	}
    }
    (ci)->WantUpdate( ci);
    if (StartTime) {
	if (tempcache) {
	    MergeTwoCacheEntries(ci, tempcache, tempcachecount, tempcachesize, inspos);
	    free(tempcache);
	}
    } else {
	char CommText[500];

	if (ci->substatus == AMS_SHOWALLSUBSCRIBED) {
	    sprintf(CommText, " (%s; All %d shown) ", (ams::GetAMS())->DescribeProt( ProtCode), TotalCt);
	} else {
	    sprintf(CommText, " (%s; %d new of %d) ", (ams::GetAMS())->DescribeProt( ProtCode), NewCt, TotalCt);
	}
	ci->CommentText = (char *)malloc(1+strlen(CommText));
	if (ci->CommentText) {
	    strcpy(ci->CommentText, CommText);
	}
    }
    return(0);
}

int captions::InsertUpdatesInDocument(char  *shortname , char  *dname, Boolean  ShowFirst)
{
    int code, substatus;
    long errcode;
    char DumBuf[1+MAXPATHLEN];
    boolean HadFullName = TRUE;

    ams::WaitCursor(TRUE);
    (this)->ClearAndUpdate( FALSE, TRUE);
    if (!dname || !*dname) {
	HadFullName = FALSE;
	errcode = (ams::GetAMS())->CUI_DisambiguateDir( shortname, &dname);
	if (errcode) {
	    if ((ams::GetAMS())->vdown( (ams::GetAMS())->AMS_ErrNo())) {
		sprintf(DumBuf, "%s: temporarily unavailable (net/server problem)", shortname);
	    } else if ((ams::GetAMS())->AMS_ErrNo() == EACCES) {
		sprintf(DumBuf, "'%s' is private; you don't have read-access or are unauthenticated.", shortname);
	    } else {
		sprintf(DumBuf, "The folder %s is not readable.", shortname);
	    }
	    message::DisplayString(NULL, 75, DumBuf);
	    return(-1);
	}
    }
    errcode = (ams::GetAMS())->MS_GetSubscriptionEntry( dname, DumBuf, &substatus);
    if (errcode) {
	ams::WaitCursor(FALSE);
	(ams::GetAMS())->ReportError( "Could not get subscription entry", ERR_WARNING, TRUE, errcode);
	return(-1);
    }
    if (!HadFullName && this->myfold) {
	(this->myfold)->AlterSubscriptionStatus( dname, substatus, shortname);
    }
    if (substatus == AMS_PRINTSUBSCRIBED) {
	(ams::GetAMS())->CUI_PrintUpdates( dname, shortname);
    }
    this->CommentText = NULL;
    this->firstcuid = -1;
    this->FullName = (char *)malloc(1+strlen(dname));
    if (this->FullName) strcpy(this->FullName, dname);
    this->ShortName = (char *)malloc(1+strlen(shortname));
    if (this->ShortName) strcpy(this->ShortName, shortname);
    this->substatus = substatus;
    (this)->SetLabel( shortname);
    code = captions_InsertCaptions(this, shortname, dname, NULL, substatus == AMS_SHOWALLSUBSCRIBED ? TRUE : FALSE);
    if (this->CommentText) {
	((this)->GetFolders())->HighlightFolder( dname, this->CommentText); 
	free(this->CommentText);
	this->CommentText = NULL;
    }

    im::ForceUpdate();
    if (ShowFirst) {
	if (substatus == AMS_SHOWALLSUBSCRIBED) {
	    int len, pos;

	    pos = this->capcache[0].offset;
	    if (this->captioncachecount > 1) {
		len = this->capcache[1].offset - pos;
	    } else if (this->captioncachecount > 0) {
		len = (this->CaptText)->GetLength() - pos;
	    } else {
		len = 0;
	    }
	    if (len) (this)->DisplayNewBody( this->capcache[0].cuid, pos, len, this->capcache[0].env);
	} else {
	    (this)->ShowMore( FALSE, FALSE, FALSE);
	}
    } else {
	/* Pre-fetch the first message */
	if (this->captioncachecount > 1) {
	    (ams::GetAMS())->CUI_PrefetchMessage( this->capcache[1].cuid, 0);
	} else if (this->captioncachecount > 0) {
	    (ams::GetAMS())->CUI_PrefetchMessage( this->capcache[0].cuid, 0);
	}
    }
    (this)->WantInputFocus( this);
    ams::WaitCursor(FALSE);
    return(code);
}

void captions::ResetVisibleCaption()
{
    class t822view *bv = (this)->GetBodView();

    ResetCaptionNotBody(this);
    ((this)->GetBodDoc())->ClearCompletely();
    text822::ResetGlobalStyle(((this)->GetBodDoc()));
    if ((bv)->GetIM() && ((bv)->GetIM())->GetInputFocus() == NULL) (bv)->WantInputFocus( bv);
    (bv)->SetDotPosition( 0);
    (bv)->SetDotLength( 0);
    (bv)->FrameDot( 0);
    (bv)->WantUpdate( bv);
}

/* The following routine tries to make a text object "prettier" by transforming _\010x into and underlined x, and by extracting literal ATK data streams (e.g. from rejected mail) */

void text_CleanUpGlitches(class text  *self)
{
    class style *uss = 0, *bolds = 0;
    struct SearchPattern *Pattern = NULL;
    int loc = 0, tmploc=0;
    char *tp, c1, c2;

    /* CLEANUP PART ONE:  DEAL WITH _\010 -type underlining */
    tp = search::CompilePattern("\010", &Pattern);
    if (tp) {
	message::DisplayString(NULL, 10, tp);
    } else while (tmploc >= 0) {
	tmploc = search::MatchPattern(self, loc, Pattern);
	if (tmploc > 0) {
	    c1 = (self)->GetChar( tmploc-1);
	    c2 = (self)->GetChar( tmploc+1);
	    if (c1 == '_') {
		if (!uss) uss = (self->styleSheet)->Find( "underline");
		if (c2 != '\010') {
		    /* highlight next char */
		    (self->rootEnvironment)->WrapStyle( tmploc+1, 1, uss);
		    (self)->AlwaysDeleteCharacters( tmploc-1, 2);
		    tmploc--;
		} else {
		    /* Handle multiple backspaces here */
		    int underscores = 1, erases = 2, pos = tmploc-2;
		    while(pos >= 0 && (self)->GetChar( pos) == '_') {
			++underscores;
			--pos;
		    }
		    pos = tmploc + 2;
		    while ((self)->GetChar( pos) == '\010') {
			++erases;
			++pos;
		    }
		    /* Ignore extra underscores */
		    if (underscores > erases) underscores = erases;
		    
		    if (underscores == erases) {
			tmploc -= underscores;
			(self)->AlwaysDeleteCharacters( tmploc, erases+underscores);
			(self-> rootEnvironment)->WrapStyle( tmploc, erases, uss);
		    }
		}
	    } else {
		if (!bolds) bolds = (self->styleSheet)->Find( "bold");
		if (c2 == c1) {
		    /* highlight previous char
		     * We do the previous char because it may have already
		     * been italicised
		     */
		    (self->rootEnvironment)->WrapStyle( tmploc-1, 1, bolds);
		    (self)->AlwaysDeleteCharacters( tmploc, 2);
		    tmploc--;
		} else if (c2 == '\010') {
		    /* Handle multiple \010 here */
		    int len, erases = 2, pos = tmploc+2, pos2;
		    while ((self)->GetChar( pos) == '\010') {
			++erases;
			++pos;
		    }
		    pos2 = tmploc - erases;
		    while (pos2 > 0 && pos2 < tmploc) {
			if ((self)->GetChar( pos2) != (self)->GetChar( pos)) break;
			pos2++;
			pos++;
		    }
		    if (pos2 == tmploc) {
			(self)->AlwaysDeleteCharacters( tmploc, erases+erases);
			(self->rootEnvironment)->WrapStyle( tmploc-erases, erases, bolds);
			tmploc--;
		    }
		}
	    }
	}
	loc = tmploc + 1;
    }
    /* CLEANUP PART TWO:  DEAL WITH EMBEDDED ATK DATASTREAM, E.G. IN MAIL REJECTIONS */
    loc = tmploc = 0;
    tp = search::CompilePattern("\\\\begindata{text822", &Pattern);
    if (tp) {
	message::DisplayString(NULL, 10, tp);
    } else do {
	tmploc = search::MatchPattern(self, loc, Pattern);
	if (tmploc >= 0) {
	    char *Buf;
	    int i, endloc;
	    FILE *fp;

	    if(Buf = (char *) malloc((self)->GetLength() - tmploc)) {
		strcpy(Buf, "\\\\end");
		i=5; 
		while (TRUE) {
		    Buf[i] = (self)->GetChar( tmploc+1+i);
		    if (Buf[i] == '\n') break;
		    ++i;
		}
		Buf[i] = (char) 0;
		tp = search::CompilePattern(Buf, &Pattern);
		if (tp) {
		    message::DisplayString(NULL, 10, tp);
		} else {
		    endloc = search::MatchPattern(self, tmploc, Pattern);
		    if (endloc > tmploc) {
		    /* Found it!  Now we need to turn it into ATK! */
		     endloc += strlen(Buf);
		     sprintf(Buf, "/tmp/clean.%d", getpid());
		     fp = fopen(Buf, "w");
		     if (fp) {
			/* This next loop is dumb -- is there a way to get to simpletext_Write from here? */
			 for (i=tmploc; i<endloc; ++i) {
			     putc((self)->GetChar( i), fp);
			 }
			 fclose(fp);
			 (self)->AlwaysDeleteCharacters( tmploc, endloc-tmploc);
			 (self)->AlwaysInsertFile( NULL, Buf, tmploc);
			 unlink(Buf);
		     }
		    }
		}
		free(Buf);
	    }
	}
	loc = tmploc+1;
    } while (tmploc >= 0);
#if 0				/* Need to do this only in body */
    /* CLEANUP PART THREE:  DEAL WITH EMBEDDED MIME DATASTREAM, E.G. IN  MAIL REJECTIONS */ 
    loc = tmploc = 0; 
    tp = search::CompilePattern("\nContent-t", &Pattern); /* -type or  -transfer-encoding */ 
    if (tp) { 
	message::DisplayString(NULL, 10, tp); 
    } else do { 
	tmploc = search::MatchPattern(self, loc, Pattern); 
	if (tmploc >= 0) { 
	    char Buf[200]; 
	    int i, endloc, len, bs, ig; 
	    FILE *fp; 
 
	    endloc = (self)->GetLength(); 
	    ++tmploc; /* skip the newline */ 
	    sprintf(Buf, "/tmp/clean.%d", getpid()); 
	    fp = fopen(Buf, "w"); 
	    if (fp) { 
		/* This next loop is dumb -- is there a way to get to simpletext_Write  from here? */ 
		for (i=tmploc; i <endloc; ++i) { 
		    putc((self)->GetChar( i), fp); 
		}    
		fclose(fp); 
		(self)->AlwaysDeleteCharacters( tmploc, endloc-tmploc); 
		fp = fopen(Buf, "r"); 
		text822::ReadIntoText(self, fp, MODE822_NORMAL, NULL, &len, FALSE, &bs,   &ig, NULL); 
		fclose(fp); 
		unlink(Buf);  
		tmploc = -1; /* End the loop */ 
	    } 
	} 
	loc = tmploc+1; 
    } while (tmploc >= 0); 
#endif
}

int captions::GetBodyFromCUID(int  cuid , int  Mode, char  *ContentTypeOverride)
{
    class text *d;
    class t822view *bv;
    char FileName[1+MAXPATHLEN], ErrorText[500];
    int ShouldDelete, IgnorePos, len;
    FILE *fp;
    class im *im;

    d = (this)->GetBodDoc();
    bv = (this)->GetBodView();
    (d)->ClearCompletely();
    text822::ResetGlobalStyle(d);
    im = (bv)->GetIM();
    if (im && (im)->GetInputFocus() == NULL) (bv)->WantInputFocus( bv);
    if ((ams::GetAMS())->CUI_ReallyGetBodyToLocalFile( cuid, FileName, &ShouldDelete, !(ams::GetAMS())->CUI_SnapIsRunning())
    || (ams::GetAMS())->CUI_GetSnapshotFromCUID( cuid, this->VisibleSnapshot)) {
	return(-1); /* Error message already reported */
    }
    fp = fopen(FileName, "r");
    if (!fp) {
	(ams::GetAMS())->ReportError( "Cannot read message body file", ERR_WARNING, FALSE, 0);
	return(-1);
    }
    if (!text822::ReadIntoText(d, fp, Mode, ContentTypeOverride, &len, TRUE, &this->StartOfRealBody, &IgnorePos, NULL)) {
	(ams::GetAMS())->ReportError( "Cannot read text822 object", ERR_WARNING, FALSE, 0);
	fclose(fp);
	return(-1);
    }
    fclose(fp);
    this->CurrentFormatting = Mode;
    if(d->IsType("text822")) {
        // ARGH, *IS* the text ever actually a text822? -robr
        if (((text822*)d)->autofix) {
            ((text822*)d)->autofix = FALSE;	/* only check this on first read! */
            CurrentFormatting |= MODE822_FIXEDWIDTH;
        }
    }
    if (ShouldDelete) {
	if (unlink(FileName)) {
	    sprintf(ErrorText, "Cannot unlink local file %s (%d)", FileName, errno);
	    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, FALSE, 0);
	}
    }
    (bv)->SetTopPosition( IgnorePos+1);
    (bv)->SetDotPosition( IgnorePos+1);
    (bv)->SetDotLength( 0);

    if (!ContentTypeOverride) text_CleanUpGlitches(d);
    (d)->SetReadOnly( TRUE);
    MarkVisibleMessageSeen(this);
    (bv)->WantUpdate( bv);
    (this)->WantUpdate( this);
    im = (bv)->GetIM();
#ifdef NOWAYJOSE
    if (im) {
	(im)->ExposeWindow();
    }
#endif /* NOWAYJOSE */
    im::ForceUpdate();
    (ams::GetAMS())->SubtleDialogs( TRUE);
    (ams::GetAMS())->CUI_ProcessMessageAttributes( cuid, this->VisibleSnapshot);
    (ams::GetAMS())->SubtleDialogs( FALSE);
    return (0);
}

int captions::DisplayNewBody(int  thisCUID , int  linestart , int  linelen, class environment  *env)
{
    char *id, *dir;
    int WasDeleted, IsDeleted, checkvis;

    if (this->captioncachecount <= 0) return 0;
    ams::WaitCursor(TRUE);
    /* Want to increase the likelihood the next line is visible unless this is the last line, in which case we simply want to ensure this line is visible. */
    if ((linestart + linelen + 1) < (this->CaptText)->GetLength()) {
	checkvis = linestart+linelen+1;
    } else {
	checkvis = linestart;
    }
    if (!(this)->Visible( checkvis)) {
	(this)->SetTopPosition( linestart);
    }
    if (this->HighlightLen) {
	RemoveHighlighting(this);
    }
    this->HighlightStartPos = linestart;
    this->HighlightLen = linelen;
    this->HighlightEnv = env;
    if (this->HighlightEnv->data.style == this->ActiveDeletedStyle
    || this->HighlightEnv->data.style == this->DeletedStyle) {
	(this->CaptText)->SetEnvironmentStyle( this->HighlightEnv, this->ActiveDeletedStyle);
	WasDeleted = 1;
    } else {
	(this->CaptText)->SetEnvironmentStyle( this->HighlightEnv, this->ActiveCaptionStyle);
	WasDeleted = 0;
    }
/* Why is this being done when below a NEW body is being loaded and displayed?  RJB
    ((this)->GetBodView())->WantUpdate( (this)->GetBodView());
    (this)->WantUpdate( this);
    im::ForceUpdate();
*/
    if (thisCUID == this->VisibleCUID) {
	SetSouthernmostPoint(this, linestart);
	ams::WaitCursor(FALSE);
	return(0);
    }
    this->VisibleCUID = thisCUID;
    if ((this)->GetBodyFromCUID( thisCUID, MODE822_NORMAL, NULL)) {
	/* error was reported */
	ams::WaitCursor(FALSE);
	ResetCaptionNotBody(this);
	return(-1);
    }
    IsDeleted = AMS_GET_ATTRIBUTE(this->VisibleSnapshot, AMS_ATT_DELETED) ? 1 : 0;
    if (IsDeleted != WasDeleted) {
	/* Someone else just deleted or undeleted it! */
	if (IsDeleted) {
	    message::DisplayString(NULL, 25, "Someone has just deleted this message, but not yet purged it.");
	    (this->CaptText)->SetEnvironmentStyle( this->HighlightEnv, this->ActiveDeletedStyle);
	} else {
	    (this->CaptText)->SetEnvironmentStyle( this->HighlightEnv, this->ActiveCaptionStyle);
	    message::DisplayString(NULL, 25, "Someone has just undeleted this message.");
	}	
	(this)->AlterDeletedIcon( this->HighlightStartPos, IsDeleted);
	(this)->WantUpdate( this);
    }
    SetSouthernmostPoint(this, linestart);
    im::ForceUpdate();
    if (!(ams::GetAMS())->CUI_GetAMSID( thisCUID, &id, &dir)) {
	(ams::GetAMS())->MS_PrefetchMessage( dir, id, 1);
    }
    rawText = FALSE;
    GetBodView()->WantInputFocus(GetBodView());
    ams::WaitCursor(FALSE);
    return(0);
}

void captions::SetLabel(char  *label)
{
    if (this->myframe) {
	(this->myframe)->SetTitle( label);
    }
}

static struct bind_Description privbindings [] = {
    {"captions-previous-caption", "\020", 0, NULL, 0, 0, (proctable_fptr)PreviousCaptionLine, "Go to previous caption"},
    {"captions-scroll-back", "\033v", 0, NULL, 0, 0, (proctable_fptr)CapScrollBack, "Scroll back captions"},
    {"captions-glitch-down", "\033z", 0, NULL, 0, 0, (proctable_fptr)CapGlitchDown, "Glitch down captions"},
    {"captions-glitch-down", "\021", 0, NULL, 0, 0, (proctable_fptr)CapGlitchDown, "Glitch down captions"},
    {"captions-beginning-of-text", "\033<", 0, NULL, 0, 0, (proctable_fptr)CapBeginText, "Move to top of captions"},
    {"captions-left-click-here", "!", TRUE, NULL, 0, 0, (proctable_fptr)captions_SimulateLeftClick, "Display what I am pointing at"},
    {"captions-right-click-here", "@", FALSE, NULL, 0, 0, (proctable_fptr)captions_SimulateRightClick, "Simulate right click on what I am pointing at"},
    {"captions-compound-operation", NULL, 0, NULL, 0, 0, (proctable_fptr)captions_CaptionsCompound, "Execute a compound captions operation"},
    {"captions-textview-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)captions_CaptionsTextviewCommand, "Execute a compound 'textview' operation on the captions"},
    {"captions-folders-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)captions_CaptionsFoldersCommand, "Execute a compound 'folders' operation."},
    {"captions-bodies-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)captions_CaptionsBodiesCommand, "Execute a compound 't822view' operation."},
    {"captions-down-focus", "\030n", 0, NULL, 0, 0, (proctable_fptr)captions_DownFocus, "Move input focus to bodies"},
    {"captions-up-focus", "\030p", 0, NULL, 0, 0, (proctable_fptr)captions_UpFocus, "Move input focus to folders"},
    {"captions-down-focus", "\030\016", 0, NULL, 0, 0, (proctable_fptr)captions_DownFocus, "Move input focus to bodies"},
    {"captions-up-focus", "\030\020", 0, NULL, 0, 0, (proctable_fptr)captions_UpFocus, "Move input focus to folders"},
    {NULL, "\033~", 0, NULL, 0, 0, NULL, NULL, NULL}, /* Preserve read onliness */
    {NULL, NULL, 0, NULL, 0, 0, NULL, NULL, NULL}
};

void OneTimeInitKeyMenus(struct ATKregistryEntry   *ci)
{
    struct proctable_Entry *tempProc;

    if ((tempProc = proctable::Lookup("textview-previous-line")) != NULL) {
        captextv_PreviousLineCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-reverse-search")) != NULL) {
        captextv_ReverseSearchCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-next-screen")) != NULL) {
        captextv_ScrollScreenFwdCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-prev-screen")) != NULL) {
        captextv_ScrollScreenBackCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-beginning-of-text")) != NULL) {
        captextv_BeginningOfTextCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-beginning-of-line")) != NULL) {
        captextv_BeginningOfLineCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-end-of-line")) != NULL) {
        captextv_EndOfLineCmd = proctable::GetFunction(tempProc);
    }
    if ((tempProc = proctable::Lookup("textview-glitch-down")) != NULL) {
        captextv_GlitchDownCmd = proctable::GetFunction(tempProc);
    }

    captions_privkeymap = new keymap;
    captions_privmenulist = new menulist;

    bind::BindList(privbindings, captions_privkeymap, captions_privmenulist, ci);
}
