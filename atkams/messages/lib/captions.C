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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/captions.C,v 1.11 1995/11/07 20:17:10 robr Stab74 $";
#endif


 

#include <textview.H>
#include <cui.h>
#include <fdphack.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <netinet/in.h>  /* for htonl, etc. */
#include <errprntf.h>
#include <ctype.h>
#include <rect.h>

#include <hdrparse.h>

#include <t822view.H>
#include <text822.H>
#include <message.H>
#include <environment.H>
#include <scroll.H>
#include <style.H>
#include <fontdesc.H>
#include <environ.H>
#include <search.H>
#include <text.H>
#include <textview.H>
#include <ams.H>
#include <amsutil.H>
#include <sendmessage.H>
#include <folders.H>
#include <keystate.H>
#include <menulist.H>
#include <cursor.H>
#include <captions.H>

ATKdefineRegistry(captions, messages, captions::InitializeClass);

void GetInfo(class captions  *ci, struct range  *total , struct range  *seen , struct range  *dot);
static void SetFrame(class captions  *ci, long  p , long  n , long  d);
static long WhatIsAt(class captions  *ci, long  n, long  d);
void ResetCaptionNotBody(class captions  *ci);
void RemoveHighlighting(class captions  *h);
void AddCaptionToCacheEntry(struct CaptionCache  **ccache, int  *ct , int  *size, int  cuid , int  offset, class environment  *env , class environment  *iconenv, Boolean  MayModify , char  *snapshot, Boolean  IsDup);
void MergeTwoCacheEntries(class captions  *ci, struct CaptionCache  *ccache, int  cct , int  csize , int  prefixend);
int GetSouthernmostPoint(class captions  *ci);
void SetSouthernmostPoint(class captions  *ci, int  pos);
void MarkVisibleMessageSeen(class captions  *ci);
int GetCUIDFromPosition(class captions  *ci, int  pos);
int GetPositionFromCUID(class captions  *ci, int  cuid);
void CreateCaptionsCursor(class captions  *self);
void NextTextviewScreen(class textview  *tv);
void RSearchTextview(class textview  *tv);
void bcopyfromback(char  *from , char  *to, int  length);
void MakeCaptionLine(char  **Buf , int  cuid , char  *RawSnapshot, int  Fixed , int  *HighStart , int  *HighLen, Boolean  IsMail , Boolean  IsDup , Boolean  IsRead);
void SetLastClassification(class captions  *self, char  *lc);
void InitKeysMenus(class captions  *captions);
void FinalizeProcStuff(class captions  *self);

extern proctable_fptr captextv_PreviousLineCmd,
    captextv_ReverseSearchCmd,
    captextv_ScrollScreenFwdCmd,
    captextv_ScrollScreenBackCmd,
    captextv_BeginningOfTextCmd,
    captextv_BeginningOfLineCmd,
    captextv_EndOfLineCmd,
    captextv_GlitchDownCmd;

class keymap *captions_privkeymap;
class menulist *captions_privmenulist;
static char *ForwardString = "---------- Forwarded message begins here ----------";
  
extern int captions_InsertCaptions(class captions  *ci, char  *shortname , char  *dname , char  *StartTime, Boolean  ShowAll);
extern void ClassifyMarkedByName(class captions  *self, char  *NameGiven);
extern void OneTimeInitKeyMenus(struct ATKregistryEntry   *ci);

void captions::ShowHelp()
{
    int doprefix = 0;
    char *motdfile;
    FILE *fp;

    (this->CaptText)->ClearCompletely();
    motdfile = (char *) environ::GetProfile("messages.motdfile");
    if (!motdfile) {
	doprefix = 1;
	motdfile = "/etc/motd";
    }
    fp = fopen(motdfile, "r");
    if (!fp) {
	char ErrMsg[100];

	sprintf(ErrMsg, "Cannot open file %s (%d).", motdfile, errno);
	(this->CaptText)->AlwaysInsertCharacters( 0, ErrMsg, strlen(ErrMsg));
    } else {
	int inslen;
	class environment *et;

	if (doprefix) {
	    time_t clock;
	    char *timestr, InsStr[500];
	    struct stat SBlock;

	    if (fstat(fileno(fp), &SBlock) == 0) {
		clock = (unsigned long) SBlock.st_mtime;
	    } else {
		clock = osi_GetSecs();
	    }
	    timestr = ctime(&clock);
	    inslen = strlen(timestr);
	    timestr[inslen-1] = '\0';
	    sprintf(InsStr, "Message of the day as of %s:\n\n", timestr);
	    inslen = strlen(InsStr);
	    (this->CaptText)->AlwaysInsertCharacters( 0, InsStr, inslen);
	    et = (this->CaptText->rootEnvironment)->InsertStyle( 0, this->ActiveCaptionStyle, 1);
	    (et)->SetLength( inslen - 1);
	} else {
	    inslen = 0;
	}
	(this->CaptText)->SetReadOnly( FALSE); /* BOGUS -- AlwaysInsertFile doesn't work for read-only text, 12/28/88 */
	(this->CaptText)->AlwaysInsertFile( fp, motdfile, inslen);
	(this->CaptText)->SetReadOnly( TRUE); /* BOGUS -- DITTO */
	fclose(fp);
    }
    (this)->WantUpdate( this);
}    

void captions::SetBodies(class t822view  *bv)
{
    this->BodView = bv;
    if (bv) (bv)->SetCaptions( this);
}

class view *
captions::Hit(enum view_MouseAction  action, long  x , long  y , long  nclicks)
{
    int thisdot, linelen, whichthis, whichdown;
    class environment *env;

    if (action == view_LeftDown || action == view_RightDown) {
	(this)->messages::Hit( view_LeftDown, x, y, 1);
	this->downdot = (this)->GetDotPosition();
    } else if (action == view_LeftUp) {
	(this)->messages::Hit( view_LeftUp, x, y, 1);
	(this)->SimulateClick( TRUE);
    } else if (action == view_RightUp)  {
	(this)->messages::Hit( view_LeftUp, x, y, 1);
	if (amsutil::GetOptBit(EXP_MARKING)) {
	    thisdot = (this)->GetDotPosition();
	    if (thisdot == this->downdot) thisdot += (this)->GetDotLength();
	    if (thisdot != this->downdot) {
		(this)->FindCUIDByDocLocation( &thisdot, &linelen, &env, &whichthis);
		(this)->FindCUIDByDocLocation( &this->downdot, &linelen, &env, &whichdown);
		if (whichthis != whichdown) {
		    int i, low, high;

		    if (whichthis < whichdown) {
			low = whichthis;
			high = whichdown;
		    } else {
			low = whichdown;
			high = whichthis;
		    }
		    for (i=low; i<= high; ++i) {
			if (!this->capcache[i].IsMarked) {
			    (this)->ToggleMark( &this->capcache[i], this->capcache[i].offset);
			}
		    }
		    (this)->PostMenus( NULL);
		    (this)->ReportMarkedMessageCount();
		    return((class view *)this);
		}
	    }
	}
	(this)->SimulateClick( FALSE);
    } else if (action == view_LeftMovement || action == view_RightMovement) {
	(this)->messages::Hit( view_LeftMovement, x, y, 1);
    }
    return((class view *) this);
}

void captions::SimulateClick(boolean  IsLeftClick)
{
    int linestart, linelen, thisCUID, whichcaption;
    class environment *env;

    if (this->captioncachecount <= 0) {
	/* This code is kind of silly; it says if there are no captions on display, but there is some kind of text in the captions area, we will search through the bodies area for the contents of the current line and will scroll the body to start with that same line.  This is currently used only by the Set Options code, but it could in theory be used to make Messages able to view all sorts of things besides mail and bulletin boards... */
	class textview *tv = (class textview *)this;
	class text *t = (class text *) (this)->GetDataObject();
	long dot, len, tmpdot, retlen;
	char *str, *tp, BodyBuf[1000];
	struct SearchPattern *Pattern = NULL;
	class text *bod;

	ams::WaitCursor(TRUE);
	captextv_BeginningOfLineCmd(tv,0);
	dot = (this)->GetDotPosition();
	captextv_EndOfLineCmd(tv,0);
	tmpdot = (this)->GetDotPosition();
	len = tmpdot - dot;
	if (len > 0) {
	    str = (t)->GetBuf( dot, len, &retlen);
	    if (retlen >= sizeof(BodyBuf)) retlen = sizeof(BodyBuf) - 1;
	    strncpy(BodyBuf, str, retlen);
	    BodyBuf[retlen] = '\0';
	    tp = search::CompilePattern(BodyBuf, &Pattern);
	    if (tp) {
		message::DisplayString(NULL, 10, tp);
	    } else {
		bod = (this)->GetBodDoc();
		if ((tmpdot = search::MatchPattern(bod, dot, Pattern)) < 0) {
		    tmpdot = search::MatchPattern(bod, 0, Pattern);
		}
		if (tmpdot < 0) {
		    message::DisplayString(NULL, 10, "Option not found.");
		} else {
		    ((this)->GetBodView())->SetTopPosition( tmpdot);

		}
	    }
	}
	ams::WaitCursor(FALSE);
	return;
    }
    linestart = (this)->GetDotPosition();
    thisCUID =  (this)->FindCUIDByDocLocation( &linestart, &linelen, &env, &whichcaption);
    (this)->SetDotPosition( linestart);
    (this)->SetDotLength( 0);

    if (IsLeftClick || (!amsutil::GetOptBit(EXP_MARKING))) { 
	(this)->DisplayNewBody( thisCUID, linestart, linelen, env);
    } else {
	(this)->ToggleMark( &this->capcache[whichcaption], linestart);
	(this)->PostMenus( NULL);
	(this)->ReportMarkedMessageCount();
    }
}





void GetInfo(class captions  *ci, struct range  *total , struct range  *seen , struct range  *dot)
{
    int pos, mylen, whichcaption, gap, mystart;
    class environment *envptr;

    ci->textscrollinterface->GetInfo(ci, total, seen, dot);
    if (ci->captioncachecount <= 0) return;
    gap = ci->FolderSize - ci->FetchedFromEnd - ci->FetchedFromStart;
    total->beg = 0;
    total->end = (ci)->EncodePosition( ci->FolderSize);
    pos = (ci)->DecodePosition( seen->beg);
    (ci)->FindCUIDByDocLocation( &pos, &mylen, &envptr, &whichcaption);
    if (pos < 0) {
	total->beg = total->end = 0;
	seen->beg = seen->end = 0;
	dot->beg = dot->end = 0;
	return;
    }
    mystart = (whichcaption >= ci->FetchedFromStart) ? (whichcaption + gap) : whichcaption;
    pos = (ci)->DecodePosition( seen->end) - 1;
    (ci)->FindCUIDByDocLocation( &pos, &mylen, &envptr, &whichcaption);
    seen->beg = (ci)->EncodePosition( mystart);
    seen->end = ((whichcaption >= ci->FetchedFromStart) ? (whichcaption + gap) : whichcaption) + 1;
    seen->end = (ci)->EncodePosition( seen->end);
    pos = ci->HighlightStartPos;
    (ci)->FindCUIDByDocLocation( &pos, &mylen, &envptr, &whichcaption);
    dot->beg = (whichcaption >= ci->FetchedFromStart) ? (whichcaption + gap) : whichcaption;
    dot->beg = (ci)->EncodePosition( dot->beg);
    dot->end = dot->beg;
}

static void SetFrame(class captions  *ci, long  p , long  n , long  d)
{
    int min, max, w, myp, outp;

    if (ci->captioncachecount <= 0) {
	ci->textscrollinterface->SetFrame(ci, p, n, d);
	return;
    }
    myp = (ci)->DecodePosition( p);
    w = (n * (ci)->GetLogicalHeight()) / d;
    /* The following conservatively assume a 6 pt font, 4 pts spacing */
    min = myp - (w / 10);
    max = myp + (((ci)->GetLogicalHeight() - w) / 10);
    if (min < 0) min = 0;
    if (max > ci->FolderSize) max = ci->FolderSize;
    if ((ci)->GuaranteeFetchedRange( min, max)) return; /* error reported */

    if (myp >= ci->FolderSize) myp = ci->FolderSize - 1;
    if (myp >= ci->FetchedFromStart) {
	myp -= ci->FolderSize - ci->FetchedFromEnd - ci->FetchedFromStart;
    }
    if (myp >= 0) {
        outp = (ci)->EncodePosition( ci->capcache[myp].offset);
    } else {
        outp = (ci)->EncodePosition( 0);
    }
    ci->textscrollinterface->SetFrame(ci, outp, n, d);
}

static long WhatIsAt(class captions  *ci, long  n, long  d)
{
    int pos, len, whichcaption;
    class environment *envptr;

    pos = ci->textscrollinterface->WhatIsAt(ci, n, d);
    if (ci->captioncachecount > 0) {
        pos = (ci)->DecodePosition( pos);
	(ci)->FindCUIDByDocLocation( &pos, &len, &envptr, &whichcaption);
        pos = (whichcaption >= ci->FetchedFromStart) ? (whichcaption + ci->FolderSize - ci->FetchedFromEnd - ci->FetchedFromStart) : whichcaption;
        pos = (ci)->EncodePosition( pos);
    }
    return(pos);
}

static const struct scrollfns capScrollInterface = {(scroll_getinfofptr)GetInfo, (scroll_setframefptr)SetFrame, NULL, (scroll_whatfptr)WhatIsAt};

const void * captions::GetInterface(const char  *interfaceName)
{
    if (strcmp(interfaceName, "scroll,vertical") == 0)
        return &capScrollInterface;
    return NULL;
}

class view *captions::GetApplicationLayer()
    {
    long scrollpos=scroll_LEFT;
    const char *pos=environ::GetProfile("ScrollbarPosition");
    class scroll *s;
    this->hasApplicationLayer = TRUE;

    if(pos) {
	if(!strcmp(pos,"right")) scrollpos=scroll_RIGHT;
    } else if(environ::GetProfileSwitch("MotifScrollbars", FALSE)) scrollpos=scroll_RIGHT;

    s=scroll::CreateScroller(this, scrollpos);
    if(s==NULL) return NULL;

    return (class view *) s;
}


void ResetCaptionNotBody(class captions  *ci)
{
    RemoveHighlighting(ci);
    ci->VisibleCUID = -1;
    (ci)->WantUpdate( ci);
    (ci)->PostMenus( NULL);
}

void RemoveHighlighting(class captions  *h) 
{
    if (h->HighlightEnv) {
	(h->CaptText)->SetEnvironmentStyle( h->HighlightEnv, AMS_GET_ATTRIBUTE(h->VisibleSnapshot, AMS_ATT_DELETED) ? h->DeletedStyle : h->NormalCaptionStyle);
	h->HighlightStartPos = 0;
	h->HighlightLen = 0;
	h->HighlightEnv = NULL;
    }
    return /* 0 */;
}

int captions::DeleteVisibleMessage(Boolean  Delete)
{
    int cuid;

    cuid = this->VisibleCUID;    
    if (cuid < 1 || !this->HighlightEnv) {
	message::DisplayString(NULL, 10, "There is no message on display.");
	return(0);
    }
    ams::WaitCursor(TRUE);
    if (Delete) {
	if ((ams::GetAMS())->CUI_DeleteMessage( cuid) == 0) { /* Everything worked Fine */
	    (this->CaptText)->SetEnvironmentStyle( this->HighlightEnv, this->ActiveDeletedStyle);
	    AMS_SET_ATTRIBUTE(this->VisibleSnapshot, AMS_ATT_DELETED);
	    (this)->WantUpdate( this);
	    (this)->AlterDeletedIcon( this->HighlightStartPos, TRUE);
	    message::DisplayString(NULL, 10, "Message deleted.");
	} /* The cui routine reported the errors itself */
    } else {
	if ((ams::GetAMS())->CUI_UndeleteMessage( cuid) == 0) { /* Everything worked Fine */
	    (this->CaptText)->SetEnvironmentStyle( this->HighlightEnv, this->ActiveCaptionStyle);
	    AMS_UNSET_ATTRIBUTE(this->VisibleSnapshot, AMS_ATT_DELETED);
	    (this)->WantUpdate( this);
	    (this)->AlterDeletedIcon( this->HighlightStartPos, FALSE);
	    message::DisplayString(NULL, 10, "Message undeleted.");
	} /* The cui routine reported the errors itself */
    }
    (this)->PostMenus( NULL);
    ams::WaitCursor(FALSE);
    return(0);
}

void captions::AlterDeletedIcon(int  pos, Boolean  Delete)
{
    char c;
    const char *newicon;
    char ErrorText[256];

    c = (this->CaptText)->GetChar( ++pos);
    switch(c) {
	case ICON_MAIL:
	case ICON_DELMAIL:
	    newicon = Delete ? SICON_DELMAIL : SICON_MAIL;
	    break;
	case ICON_READMAIL:
	case ICON_READDELMAIL:
	    newicon = Delete ? SICON_READDELMAIL : SICON_READMAIL;
	    break;
	case ICON_POST:
	case ICON_DELPOST:
	    newicon = Delete ? SICON_DELPOST : SICON_POST;
	    break;
	case ICON_READDELPOST:
	case ICON_READPOST:
	    newicon = Delete ? SICON_READDELPOST : SICON_READPOST;
	    break;
	default:
	    newicon = "X";
	    sprintf(ErrorText, "Found '%c' on '%sdelete' where a mail/post icon was expected", c, Delete ? "" : "un");
	    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, FALSE, 0);
	    break;
    }
    (this->CaptText)->AlwaysInsertCharacters( pos++, (char *)newicon, 1);
    (this->CaptText)->AlwaysDeleteCharacters( pos, 1);
}

int captions::FindCUIDByDocLocation(int  *position , int  *len, class environment  **envptr, int  *whichcaption)
{
    int top, bottom, split;

    bottom = 0;
    top = this->captioncachecount -1;
    if (top < 0) {
	*position = *len = *whichcaption = -1;
	*envptr = NULL;
	return(-1);
    }
    split = top/2;
    while (TRUE) {
	if (this->capcache[split].offset > *position) {
	    top = split;
	} else {
	    bottom = split;
	}
	split = bottom + (top-bottom)/2;
	if (top == bottom) break;
	if (top == bottom + 1) {
	    if (*position < this->capcache[top].offset) {
		split = bottom;
	    } else {
		split = top;
	    }
	    break;
	}
    }
    *position = this->capcache[split].offset;
    if (split == this->captioncachecount -1) {
	*len = (this->CaptText)->GetLength() - *position;
    } else {
	*len = this->capcache[split+1].offset - *position;
    }
    *envptr = this->capcache[split].env;
    *whichcaption = split;
    return(this->capcache[split].cuid);
}

void captions::ToggleMark(struct CaptionCache  *hc,int  linestart)
{
    (this->CaptText)->AlwaysInsertCharacters( linestart+1, (char *)(hc->IsMarked ? " " : SICON_MARK), 1);
    (this->CaptText)->AlwaysDeleteCharacters( linestart, 1);
    this->MarkCount += hc->IsMarked ? -1 : 1;
    hc->IsMarked = hc->IsMarked ? 0 : 1;
    (this)->WantUpdate( this);
}


void AddCaptionToCacheEntry(struct CaptionCache  **ccache, int  *ct , int  *size, int  cuid , int  offset, class environment  *env , class environment  *iconenv, Boolean  MayModify , char  *snapshot, Boolean  IsDup)
{
    long dumint;
    struct CaptionCache *cc = &((*ccache)[*ct]);

    cc->offset = offset;
    cc->cuid = cuid;
    cc->env = env;
    cc->iconenv = iconenv;
    cc->IsMarked = 0;
    cc->MayModify = MayModify ? 1 : 0;
    cc->IsDup = IsDup ? 1 : 0;
    bcopy(AMS_CHAIN(snapshot), &dumint, sizeof(long));
    cc->Chain = ntohl(dumint);
    bzero(cc->Date, AMS_DATESIZE);
    strcpy(cc->Date, AMS_DATE(snapshot));
    bzero(cc->Attributes, AMS_ATTRIBUTESIZE);
    bcopy(AMS_ATTRIBUTES(snapshot), cc->Attributes, AMS_ATTRIBUTESIZE);
    if (++*ct >= *size) {
	int newsize;

	newsize = *size * 2;
	*ccache = (struct CaptionCache *) realloc(*ccache, newsize * sizeof(struct CaptionCache));
	*size = newsize;
    }
}

void MergeTwoCacheEntries(class captions  *ci, struct CaptionCache  *ccache, int  cct , int  csize , int  prefixend)
{
    int totalct, i;

    totalct = ci->captioncachecount + cct;
    if (totalct >= ci->captioncachesize) {
	ci->capcache = (struct CaptionCache *) realloc(ci->capcache, (1+totalct) * sizeof(struct CaptionCache));
	ci->captioncachesize = totalct+1;
    }
    bcopyfromback((char *)ci->capcache, (char *)&ci->capcache[cct], ci->captioncachecount * sizeof(struct CaptionCache));
    bcopy(ccache, ci->capcache, cct * sizeof(struct CaptionCache));
    for (i = cct; i<totalct; ++i) {
	ci->capcache[i].offset += prefixend;
    }
    ci->captioncachecount = totalct;
}

int GetSouthernmostPoint(class captions  *ci)
{
    if (ci->SouthPoint) {
	return((ci->SouthPoint)->Eval());
    } else {
	return(0);
    }
}

void SetSouthernmostPoint(class captions  *ci, int  pos)
{
    int i;

    if (pos >= 0 && pos < GetSouthernmostPoint(ci)) return;
    if (ci->SouthPoint) {
	(ci->CaptText)->SetEnvironmentStyle( ci->SouthPoint, ci->IconicStyle);
	ci->SouthPoint = NULL;
    }
    if (pos < 0) return;
    for (i=0; i<ci->captioncachecount; ++i) {
	if (ci->capcache[i].offset > pos) break;
    }
    if (--i < 0) return;
    ci->SouthPoint = ci->capcache[i].iconenv;
    (ci->CaptText)->SetEnvironmentStyle( ci->SouthPoint, ci->UnderlinedIconicStyle);

    (ci)->WantUpdate( ci);
}


void MarkVisibleMessageSeen(class captions  *ci)
{
    if (AMS_GET_ATTRIBUTE(ci->VisibleSnapshot, AMS_ATT_MAYMODIFY) && AMS_GET_ATTRIBUTE(ci->VisibleSnapshot, AMS_ATT_UNSEEN) && (ams::GetAMS())->CUI_MarkAsRead( ci->VisibleCUID)) {
	return /* (-1) */; /* error was reported */
    }
    AMS_UNSET_ATTRIBUTE(ci->VisibleSnapshot, AMS_ATT_UNSEEN);
    (ci)->MarkVisibleMessageStateofSeeing( TRUE);
}


void captions::MarkVisibleMessageStateofSeeing(Boolean  HasSeen)
{
    int c, pos;
    class text *d;
    const char *newicon;
    char ErrorText[256];

    d = this->CaptText;
    pos = this->HighlightStartPos + 1;
    c = (d)->GetChar(  pos);
    switch(c) {
	case ICON_MAIL:
	case ICON_READMAIL:
	    newicon = HasSeen ? SICON_READMAIL : SICON_MAIL;
	    break;
	case ICON_READDELMAIL:
	case ICON_DELMAIL:
	    newicon = HasSeen ? SICON_READDELMAIL : SICON_DELMAIL;
	    break;
	case ICON_POST:
	case ICON_READPOST:
	    newicon = HasSeen ? SICON_READPOST : SICON_POST;
	    break;
	case ICON_READDELPOST:
	case ICON_DELPOST:
	    newicon = HasSeen ? SICON_READDELPOST : SICON_DELPOST;
	    break;
	default:
	    newicon = "X";
	    sprintf(ErrorText, "Found '%c' on 'mark %sseen' where a mail/post icon was expected", c, HasSeen ? "" : "un");
	    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, FALSE, 0);
	    break;
    }
    (d)->AlwaysInsertCharacters( pos++, (char *)newicon, 1);
    (d)->AlwaysDeleteCharacters( pos, 1);
    (this)->WantUpdate( this);
    (this)->PostMenus( NULL);
    if (!HasSeen) message::DisplayString(NULL, 10, "Marked message as unread");
}


int captions::ShowMore(Boolean  MayScroll , Boolean  MayGoOn , Boolean  InsistOnMark)
{
    int whichcaption, pos, len, thisCUID;
    class environment *env;

    if ((this)->GetDotLength() > 0) {
	pos = (this)->GetDotPosition();
	thisCUID = (this)->FindCUIDByDocLocation( &pos, &len, &env, &whichcaption);
	if (pos > 0) {
	    (this)->SetDotPosition( pos - 1);
	}
	(this)->SetDotLength( 0);
    }
    if (MayScroll && (len=((this)->GetBodDoc())->GetLength()) > 0 && !((this)->GetBodView())->Visible( len)  && (len != ((this)->GetBodView())->GetTopPosition())) {
	/* Scroll it forward */
	NextTextviewScreen((this)->GetBodView());
	return(0);
    } else {
	/* Go on to the next one */
	if (this->HighlightLen > 0) {
	    pos = this->HighlightStartPos;
	} else {
	    pos = (this)->GetDotPosition();
	}
	if (pos <= 0 && this->HighlightLen <= 0) {
	    whichcaption = -1;
	} else {
	    thisCUID = (this)->FindCUIDByDocLocation( &pos, &len, &env, &whichcaption);
	}
	++whichcaption;
	while (whichcaption < this->captioncachecount && this->capcache[whichcaption].IsDup) {
	    ++whichcaption;
	}
	if (InsistOnMark) {
	    while (whichcaption < this->captioncachecount && !this->capcache[whichcaption].IsMarked) {
		++whichcaption;
	    }
	}
	if (whichcaption >= this->captioncachecount) {
	    if (MayGoOn) {
		/* Have to set southernmost point in case skipped duplicates */
		if (this->captioncachecount > 0) {
		    SetSouthernmostPoint(this, this->capcache[this->captioncachecount-1].offset);
		}
		((this)->GetFolders())->NextFolder( TRUE);
	    } else {
		if (this->captioncachecount > 0) {
		    message::DisplayString(NULL, 10, "You are at the end of the captions");
		}
		/* Else we assume another error is displayed already... */
	    }
	} else {
		pos = this->capcache[whichcaption].offset;
		(this)->SetDotPosition( pos);
		thisCUID = (this)->FindCUIDByDocLocation( &pos, &len, &env, &whichcaption);
		(this)->DisplayNewBody( thisCUID, pos, len, env);
		return(0);
	}
    }
    return 0; /* ? */
}



void captions::MakeCachedUpdates() 
{
    char ErrorText[500], UpdateDate[AMS_DATESIZE];
    int south, j;
    long mserrcode;

    ams::TryDelayedUpdates();
    if (!this->FullName) return;
    south = GetSouthernmostPoint(this);
    if (south <= 0 && (this->FolderSize > (this->FetchedFromEnd + this->FetchedFromStart))) {
	(this)->GuaranteeFetchedRange( this->FolderSize - this->FetchedFromEnd - 1, this->FolderSize);
	south = GetSouthernmostPoint(this);
    }
    for(j=0; j<this->captioncachecount; ++j) {
	if (this->capcache[j].offset > south) break;
    }
    if (j<=0) {
	((this)->GetFolders())->SetSkip( this->FullName, TRUE);
	return;
    }
    strcpy(UpdateDate, this->capcache[--j].Date);
    mserrcode = (ams::GetAMS())->MS_SetAssociatedTime( this->FullName, UpdateDate);
    if (mserrcode) {
	if ((ams::GetAMS())->AMS_ErrNo() == ENOENT) {
	    sprintf(ErrorText, "Folder %s has recently been deleted -- profile not set", this->ShortName);
	    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, FALSE, mserrcode);
	} else if ((ams::GetAMS())->vdown( (ams::GetAMS())->AMS_ErrNo()) || ((ams::GetAMS())->AMS_ErrNo() == EWOULDBLOCK)) {
	    sprintf(ErrorText, "Could not set profile for %s; will try again later.", this->ShortName);
	    (ams::GetAMS())->ReportSuccess( ErrorText);
	    ams::CacheDelayedUpdate(this->FullName, UpdateDate);
	} else {
	    sprintf(ErrorText, "Could not set profile for %s (%s, %d, %ld, %ld)", this->ShortName, this->FullName, (ams::GetAMS())->AMS_ErrNo(), AMS_ERRCAUSE, AMS_ERRVIA);
	    /* UGH!  Above line only works because we've defined mserrcode locally to be the right thing... */
	    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, TRUE, mserrcode);
	}
	return;
    }
    if (j >= (this->captioncachecount-1)) {
	((this)->GetFolders())->SetSkip( this->FullName, TRUE);
    }
}


int GetCUIDFromPosition(class captions  *ci, int  pos)
{
    int j;

    for(j=0; j<ci->captioncachecount; ++j) {
	if (ci->capcache[j].offset > pos) break;
    }
    if (j<=0) {
	return(-1);
    } else {
	return(ci->capcache[j].cuid);
    }
}

int GetPositionFromCUID(class captions  *ci, int  cuid)
{
    int j;

    for(j=0; j<ci->captioncachecount; ++j) {
	if (ci->capcache[j].cuid == cuid) return(ci->capcache[j].offset);
    }
    return(-1);
}


int captions::GuaranteeFetchedRange(int  min , int  max)
{
    int frontgap, backgap, oldsize, olddot, added, oldtop, south, southcuid, dotcuid, topcuid, pos;
    char TimeBuf[AMS_DATESIZE+1], SBuf[AMS_SNAPSHOTSIZE], ErrorText[500];
    long errcode;
    int RetryCount = 0;
#define RETRY_MAX 5 /* Maximum number of times to believe the folder is changing underneath me in rapid sequence. */

    if (this->FolderSize <= 0) return(0);
    if (min < 0) min = 0;
restart:
    frontgap = max - this->FetchedFromStart;
    backgap = (this->FolderSize - min) - this->FetchedFromEnd;
    if (frontgap < 0 || backgap < 0) return(0); /* already there */
    oldtop = (this)->GetTopPosition();
    ams::WaitCursor(TRUE);
    if (errcode = (ams::GetAMS())->MS_GetNthSnapshot( this->FullName, min, SBuf)) {
	if ((RetryCount < RETRY_MAX) && ((ams::GetAMS())->AMS_ErrNo() == EINVAL)) {
	    /* This is the case where you have had a folder purged underneath you! */
	    char nickname[1+MAXPATHLEN], fullname[1+MAXPATHLEN];

	    strcpy(nickname, this->ShortName);
	    strcpy(fullname, this->FullName);
	    southcuid = GetCUIDFromPosition(this, GetSouthernmostPoint(this));
	    topcuid = GetCUIDFromPosition(this, (this)->GetTopPosition());
	    dotcuid = GetCUIDFromPosition(this, (this)->GetDotPosition());
	    (this)->ClearAndUpdate( FALSE, FALSE); /* Both false to inhibit recursion into GuaranteeFetchedRange again */
	    (this)->InsertUpdatesInDocument( nickname, fullname, FALSE);
	    RetryCount++;
	    goto restart;
	} else {
	    sprintf(ErrorText, "Could not get %dth snapshot in folder %s (size %d)", min, this->FullName, this->FolderSize);
	    (ams::GetAMS())->ReportError( ErrorText, ERR_CRITICAL, TRUE, errcode);
	    ams::WaitCursor(FALSE);
	    return(-1);
	}
    }
    if (strcmp(AMS_DATE(SBuf), "zzzzzz")) {
	strcpy(TimeBuf, amsutil::convlongto64(amsutil::conv64tolong(AMS_DATE(SBuf)) -1, 0));
    } else {
	if (min <= 0) {
	    sprintf(ErrorText, "Bad first date for %s; probably needs reconstruction.", this->ShortName);
	} else {
	    sprintf(ErrorText, "Bad %dth date for %s; probably needs reconstruction.", min, this->ShortName);
	}
	(ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, FALSE, 0);
	strcpy(TimeBuf, "000000");
    }
    oldsize = (this->CaptText)->GetLength();
    olddot = (this)->GetDotPosition();
    south = GetSouthernmostPoint(this);
    captions_InsertCaptions(this, this->ShortName, this->FullName, TimeBuf, FALSE);
    added = (this->CaptText)->GetLength() - oldsize;
    (this)->SetDotPosition( olddot + added);
    (this)->SetTopPosition( oldtop + added);
    if (this->HighlightLen > 0) {
	this->HighlightStartPos += added;
    }
    SetSouthernmostPoint(this, south+added);
    if (RetryCount > 0) {
	pos = GetPositionFromCUID(this, southcuid);
	if (pos >= 0) SetSouthernmostPoint(this, pos);
	pos = GetPositionFromCUID(this, dotcuid);
	if (pos >= 0) (this)->SetDotPosition( pos);
	pos = GetPositionFromCUID(this, topcuid);
	if (pos >= 0) (this)->SetTopPosition( pos);
    }
    ams::WaitCursor(FALSE);
    return (0);
}

void captions::FileCurrent(char  *FullName , char  *nickname)
{
    int	    cuid, OpCode;
    Boolean DoAppend = FALSE;

    if (*FullName == '*') {
	++FullName;
	DoAppend = TRUE;
    }
    cuid = this->VisibleCUID;
    if (cuid < 1) {
	message::DisplayString(NULL, 10, "There is nothing to classify.");
	return;
    }
    ams::WaitCursor(TRUE);
    if (DoAppend) {
	OpCode = AMS_GET_ATTRIBUTE(this->VisibleSnapshot, AMS_ATT_MAYMODIFY) ? MS_CLONE_APPENDDEL : MS_CLONE_APPEND;
    } else {
	OpCode = AMS_GET_ATTRIBUTE(this->VisibleSnapshot, AMS_ATT_MAYMODIFY) ? MS_CLONE_COPYDEL : MS_CLONE_COPY;
    }
    if (!(ams::GetAMS())->CUI_CloneMessage( cuid, FullName, OpCode)) {
	/* cuilib reports errors, here we deal with success */
	if (OpCode == MS_CLONE_COPYDEL || OpCode == MS_CLONE_APPENDDEL) {
	    if (this->HighlightEnv) (this->CaptText)->SetEnvironmentStyle( this->HighlightEnv, this->ActiveDeletedStyle);
	    AMS_SET_ATTRIBUTE(this->VisibleSnapshot, AMS_ATT_DELETED);
	    (this)->PostMenus( NULL);
	}
	(this)->WantUpdate( this);
    }
    ams::WaitCursor(FALSE);
    return;
}

void captions::FileMarked(char  *FullName , char  *nickname)
{
    ClassifyMarkedByName(this, FullName);
    return;
}

void captions::AlterPrimaryFolderName(char  *addname , char  *delname)
{
    char Nick[1+MAXPATHLEN], *s;

    if (strcmp(delname, this->FullName)) return;

    if (addname) {
	s = (char *)malloc(1+strlen(addname));
	if (!s) return;
	free (this->FullName);
	this->FullName = s; 
	strcpy(this->FullName, addname);

	(ams::GetAMS())->CUI_BuildNickName( addname, Nick);
	s = (char *)malloc(1+strlen(Nick));
	if (!s) return;
	free (this->ShortName);
	this->ShortName = s;
	strcpy(this->ShortName, Nick);
    } else {
	/* Just Deleting the sucker on display */
	(this)->ClearAndUpdate( FALSE, FALSE);
    }
}

void CreateCaptionsCursor(class captions  *self)
{
        class fontdesc *fd;

	fd = fontdesc::Create("icon", 0, 12);
	self->mycursor = cursor::Create(self);
	(self->mycursor)->SetGlyph( fd, 'R');
}

void captions::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle Rect;

    (this)->GetLogicalBounds( &Rect);
    (this)->messages::FullUpdate( type, left, top, width, height);
    if (!this->mycursor) {
	CreateCaptionsCursor(this);
    }
    (this)->PostCursor( &Rect, this->mycursor);
}

void captions::ReportMarkedMessageCount()
{
    char ErrorText[100];
    if (this->MarkCount <=0) {
	strcpy(ErrorText, "There are now no messages marked.");
    } else if (this->MarkCount == 1) {
	strcpy(ErrorText, "There is now one message marked.");
    } else {
	sprintf(ErrorText, "There are now %s messages marked.", amsutil::cvEng(this->MarkCount, 0, 1000));
    }
    message::DisplayString(NULL, 10, ErrorText);
}

void NextTextviewScreen(class textview  *tv)
{
    captextv_ScrollScreenFwdCmd(tv,0);
}

void RSearchTextview(class textview  *tv)
{
    captextv_ReverseSearchCmd(tv,0);
}

/* This is the same as bcopy, but copies from the back to be safe when you're 
    really just extending an array by pushing back elements */

void bcopyfromback(char  *from , char  *to, int  length)
{
    register char *f, *t;

    f = from + length-1;
    t = to+length-1;
    while (f >=from) {
	*t-- = *f--;
    }
}


/* The next two used to be defines, but things are getting tight again... */
static int PADTOCOLUMNA = 11;
static int PADTOCOLUMNB = 45;
static char *LOTSASPACE="                                                                ";

/* The next call returns a pointer to a static area, overwritten
	on each call */
static char CaptionBuf[AMS_SNAPSHOTSIZE];

void MakeCaptionLine(char  **Buf , int  cuid , char  *RawSnapshot, int  Fixed , int  *HighStart , int  *HighLen, Boolean  IsMail , Boolean  IsDup , Boolean  IsRead)
{
    char *s, *t, *RawCap;
    int len, len2, IconCode, IconCode2;

    *HighStart = *HighLen = 0;
    if (AMS_GET_ATTRIBUTE(RawSnapshot, AMS_ATT_DELETED)) {
	IconCode = IsMail ? (IsRead ? ICON_READDELMAIL : ICON_DELMAIL) : (IsRead ? ICON_READDELPOST : ICON_DELPOST);
    } else {
	IconCode = IsMail ? (IsRead ? ICON_READMAIL : ICON_MAIL) : (IsRead ? ICON_READPOST : ICON_POST);
    }
    IconCode2 = IsDup ? ICON_DUP : ' ';
    RawCap = AMS_CAPTION(RawSnapshot);
    *Buf = CaptionBuf;
    s = index(RawCap, '\t');
/* IF BOGUS ATK tabbing is fixed, the following line should be all we ever need. */
    if (!s || ((s-RawCap)>PADTOCOLUMNA)) {
	sprintf(CaptionBuf, " %c%c%s\n", IconCode, IconCode2, RawCap);
	return /* 0 */;
    }
    *s = '\0';
    sprintf(CaptionBuf, " %c%c%s ", IconCode, IconCode2, RawCap);
    if (Fixed) {
	len2 = strlen(CaptionBuf);
	len = PADTOCOLUMNA - len2;
	if (len > 0) {
	    strncat(CaptionBuf, LOTSASPACE, len);
	    CaptionBuf[len+len2] = '\0';
	}
    } else { /* This is bogus -- can NOT get tabbing right in ATK */
	strcat(CaptionBuf, " ");
    }
    t = index(++s, '\t');
    if (!t || ((t-s)>PADTOCOLUMNB)) {
	strcat(CaptionBuf, s);
	strcat(CaptionBuf, "\n");
	return /* 0 */;
    }
    *t = '\0';

    if (Fixed) {
	strcat(CaptionBuf, s);
	len2 = strlen(CaptionBuf);
	len = PADTOCOLUMNB - len2;
	if (len > 0) {
	    strncat(CaptionBuf, LOTSASPACE, len);
	    CaptionBuf[len+len2] = '\0';
	}
    } else { /* This is bogus -- can NOT get tabbing right in ATK */
	*HighStart = strlen(CaptionBuf);
	strcat(CaptionBuf, s);
	*HighLen = strlen(CaptionBuf) - *HighStart;
	strcat(CaptionBuf, " - ");
    }
    strcat(CaptionBuf, t+1);
    strcat(CaptionBuf, "\n");
    return /* 0 */;
}

void captions::SearchAll()
{
    char shortname[1+MAXPATHLEN], *tp, ErrorText[256];
    struct SearchPattern *Pattern = NULL;
    int pos, numfound, len, orgpos, whichcaption, oldpos;
    class environment *envdum;
    
    if (message::AskForString(NULL, 50, "Find all occurrences of: ", "", shortname, sizeof(shortname))< 0) return;
    (this)->GuaranteeFetchedRange( 0, this->FolderSize);
    tp = search::CompilePattern(shortname, &Pattern);
    if (tp) {
	message::DisplayString(NULL, 10, tp);
	return;
    }
    if (this->MarkCount > 0) {
	(this)->ClearMarks();
    }
    numfound = 0;
    orgpos = (this)->GetDotPosition();
    pos = 0;
    (this)->SetDotPosition( 0);
    (this)->SetDotLength( 0);
    while ((pos = search::MatchPattern(this->CaptText, pos, Pattern)) >= 0) {
	oldpos = pos;
	if ((this)->FindCUIDByDocLocation( &pos, &len, &envdum, &whichcaption)) {
	    if (numfound++ <= 0) {
		orgpos = pos;
	    }
	    if (!this->capcache[whichcaption].IsMarked) {
		(this)->ToggleMark( &this->capcache[whichcaption], pos);
	    }
	    pos += len;
	} else {
	    pos = oldpos + 1;
	}
    }
    (this)->SetDotPosition( orgpos);
    (this)->FrameDot( orgpos);
    (this)->PostMenus( NULL);
    sprintf(ErrorText, "Marked %s captions containing '%s'", amsutil::cvEng(numfound, 0, 1000), shortname);
    message::DisplayString(NULL, 10, ErrorText);
    (this)->WantInputFocus( this);
}

void captions::FindRelatedMessages()
{
    char ErrorText[256];
    long numfound, i, j, mainchain, orgpos;
    struct CaptionCache *cc;
    
    if (this->VisibleCUID <= 0) {
	message::DisplayString(NULL, 10, "There is nothing on display.");
	return;
    }
    (this)->GuaranteeFetchedRange( 0, this->FolderSize);
    if (this->MarkCount > 0) {
	(this)->ClearMarks();
    }
    orgpos = (this)->GetDotPosition();
    numfound = 0;
    bcopy(AMS_CHAIN(this->VisibleSnapshot), &i, sizeof(long));
    mainchain = ntohl(i);
    if (mainchain) {
	for (j = 0; j < this->captioncachecount; ++j) {
	    cc = &this->capcache[j];
	    if (cc->Chain == mainchain) {
		if (numfound++ <= 0) {
		    orgpos = cc->offset;
		}
		(this)->ToggleMark( cc, cc->offset);
	    }
	}
    }
    (this)->SetDotPosition( orgpos);
    (this)->FrameDot( orgpos);
    (this)->PostMenus( NULL);
    if (numfound) {
	sprintf(ErrorText, "Marked %s related messages.", amsutil::cvEng(numfound, 0, 1000));
    } else {
	strcpy(ErrorText, "There are no related messages.");
    }
    message::DisplayString(NULL, 10, ErrorText);
    (this)->WantInputFocus( this);
}

void captions::MarkRangeOfMessages()
{
    char ErrorText[256], Sdate[1+AMS_DATESIZE], Edate[1+AMS_DATESIZE], DBuf[400];
    long numfound, j, orgpos;
    struct CaptionCache *cc;
    int year, month, day, hour, min, sec, wday, gtm;

    (this)->GuaranteeFetchedRange( 0, this->FolderSize);
    if (this->MarkCount > 0) {
	(this)->ClearMarks();
    }
    if (message::AskForString(NULL, 50, "Mark messages since date [beginning of time]: ", "", DBuf, sizeof(DBuf)) < 0) {
	return;
    }
    if (DBuf[0]) {
	long code;
	code = (ams::GetAMS())->MS_ParseDate( DBuf, &year, &month, &day, &hour, &min, &sec, &wday, &gtm);
	if (code) {
	    message::DisplayString(NULL, 25, "Sorry; I don't understand the date you entered.");
	    return;
	}
	strcpy(Sdate, amsutil::convlongto64(gtm, 0));
    } else {
	strcpy(Sdate, "000000");
    }
    if (message::AskForString(NULL, 50, "Mark messages through date [end of time]: ", "", DBuf, sizeof(DBuf)) < 0) {
	return;
    }
    if (DBuf[0]) {
	long code;
	code = (ams::GetAMS())->MS_ParseDate( DBuf, &year, &month, &day, &hour, &min, &sec, &wday, &gtm);
	if (code) {
	    message::DisplayString(NULL, 25, "Sorry; I don't understand the date you entered.");
	    return;
	}
	strcpy(Edate, amsutil::convlongto64(gtm, 0));
    } else {
	strcpy(Edate, "zzzzzz");
    }
    orgpos = (this)->GetDotPosition();
    numfound = 0;
    for (j = 0; j < this->captioncachecount; ++j) {
	cc = &this->capcache[j];
	if (strcmp(cc->Date, Sdate) > 0 && strcmp(cc->Date, Edate) <= 0) {
	    if (numfound++ <= 0) {
		orgpos = cc->offset;
	    }
	    (this)->ToggleMark( cc, cc->offset);
	}
    }
    (this)->SetDotPosition( orgpos);
    (this)->FrameDot( orgpos);
    (this)->PostMenus( NULL);
    if (numfound) {
	sprintf(ErrorText, "Marked %s messages in the date range requested.", amsutil::cvEng(numfound, 0, 1000));
    } else {
	strcpy(ErrorText, "There are no messages in that date range.");
    }
    message::DisplayString(NULL, 10, ErrorText);
    (this)->WantInputFocus( this);
}

static char LastClassification[1+MAXPATHLEN] = AMS_DEFAULTMAILDIR;

char *captions::GetLastClassification()
{
    if ((this->ShortName) && !strcmp(LastClassification, this->ShortName)) {
	return("");
    }
    return(LastClassification);
}

void SetLastClassification(class captions  *self, char  *lc)
{
    strncpy(LastClassification, lc, sizeof(LastClassification));
}


void captions::BackUpCheckingMarks(Boolean  InsistOnMark)
{
	int pos, len, thisCUID, whichcaption;
	class environment *env;
	Boolean FirstPass = TRUE;

restart:
	pos = this->HighlightEnv ? (this->HighlightEnv)->Eval() : -1;
        if (pos <= 0) pos = (this)->GetDotPosition();
	thisCUID = (this)->FindCUIDByDocLocation( &pos, &len, &env, &whichcaption);
	if ((this)->GetDotLength() > 0) {
	    if (pos > 0) {
		(this)->SetDotPosition( pos + len + 1);
	    }
	    (this)->SetDotLength( 0);
	}
	--whichcaption;
	if (InsistOnMark) {
	     while (whichcaption >= 0 && !this->capcache[whichcaption].IsMarked) {
		--whichcaption;
	     }
	}
	if (whichcaption < 0) {
	    if (FirstPass && ((this->FetchedFromStart + this->FetchedFromEnd) < this->FolderSize)) {
		(this)->GuaranteeFetchedRange( InsistOnMark ? 0 : (this->FolderSize - this->FetchedFromEnd-5), this->FolderSize);
		FirstPass = FALSE;
		goto restart;
	    } else {
		message::DisplayString(NULL, 10, "You are at the beginning of the captions");
		return;
	    }
	}
	pos = this->capcache[whichcaption].offset;
	(this)->SetDotPosition( pos);
	(this)->FrameDot( pos + 5);
	thisCUID = (this)->FindCUIDByDocLocation( &pos, &len, &env, &whichcaption);
	(this)->DisplayNewBody( thisCUID, pos, len, env);
	return;
}

void captions::AlterFileIntoMenus(boolean  Shrink)
{
    if (Shrink) {
	if (this->MenusExpanded) {
	    this->MenusExpanded = FALSE;
	    (this)->PostMenus( NULL);
	}
    } else {
	if (!this->MenusExpanded) {
	    this->MenusExpanded = TRUE;
	    (this)->PostMenus( NULL);
	}
    }
}

void captions::MarkCurrent()
{
    int whichcaption, linestart, linelen;
    char ErrorText[256];
    class environment *env;

    if (!this->HighlightEnv) {
	message::DisplayString(NULL, 10, "There is nothing to mark.");
	return;
    }
    linestart = (this->HighlightEnv)->Eval();
    (this)->FindCUIDByDocLocation( &linestart, &linelen, &env, &whichcaption);
    (this)->ToggleMark( &this->capcache[whichcaption], linestart);
    (this)->PostMenus( NULL);
    if (this->MarkCount <=0) {
	strcpy(ErrorText, "There are now no messages marked.");
    } else if (this->MarkCount == 1) {
	strcpy(ErrorText, "There is now one message marked.");
    } else {
	sprintf(ErrorText, "There are now %s messages marked.", amsutil::cvEng(this->MarkCount, 0, 1000));
    }
    message::DisplayString(NULL, 10, ErrorText);
}

void captions::Redisplay(int  Mode, char  *contenttype)
{
    int dot, top, len;
    class t822view *bod;

    if(contenttype && strcmp(contenttype,"text/plain")==0) rawText=TRUE;
    else rawText=FALSE;
    if (this->VisibleCUID < 0) return;
    ams::WaitCursor(TRUE);
    bod = (this)->GetBodView();
    dot = (bod)->GetDotPosition();
    top = (bod)->GetTopPosition();
    len = (bod)->GetDotLength();
    (this)->GetBodyFromCUID( this->VisibleCUID, Mode|MODE822_NOAUTOFIX, contenttype);
    (this)->PostMenus( NULL);
    (bod)->SetDotPosition( dot);
    (bod)->SetTopPosition( top);
    (bod)->SetDotLength( len);
    ams::WaitCursor(FALSE);
}

void captions::CloneMessage(int  Code)
{
    int cuid, MayModify;
    char NewDirName[1+MAXPATHLEN], SaveDirName[1+MAXPATHLEN], ErrorText[256];

    cuid = this->VisibleCUID;    
    if (cuid < 1) {
	message::DisplayString(NULL, 10, "There is no message on display.");
	return;
    }
    MayModify = AMS_GET_ATTRIBUTE(this->VisibleSnapshot, AMS_ATT_MAYMODIFY) ? 1 : 0;
    switch(Code) {
	case MS_CLONE_APPEND:
	case MS_CLONE_APPENDDEL:
	    sprintf(ErrorText, "Append this message to what folder [%s]:  ", (this)->GetLastClassification());
	    if (!MayModify) Code = MS_CLONE_APPEND;
	    break;
	case MS_CLONE_COPY:
	case MS_CLONE_COPYDEL:
	    sprintf(ErrorText, "File this message into which folder [%s]: ", (this)->GetLastClassification());
	    if (!MayModify) Code = MS_CLONE_COPY;
	    break;
	default:
	    (ams::GetAMS())->ReportError( "Invalid internal parameter to CloneMessage.", ERR_WARNING, FALSE, 0);
	    return;
    }
    if (ams::GetFolderName(ErrorText, NewDirName, MAXPATHLEN, "", FALSE)) return;
    if (NewDirName[0] == '\0') {
	strcpy(NewDirName, (this)->GetLastClassification());
	if (NewDirName[0] == '\0') return;
    }
    strcpy(SaveDirName, NewDirName); /* Cuilib fiddles with the string */
    ams::WaitCursor(TRUE);
    if (!(ams::GetAMS())->CUI_CloneMessage( cuid, NewDirName, Code)) {
	SetLastClassification(this, SaveDirName);
	/* cuilib reports errors, here we deal with success */
	if (Code == MS_CLONE_COPYDEL || Code == MS_CLONE_APPENDDEL) {
	    if (this->HighlightEnv) (this->CaptText)->SetEnvironmentStyle( this->HighlightEnv, this->ActiveDeletedStyle);
	    AMS_SET_ATTRIBUTE(this->VisibleSnapshot, AMS_ATT_DELETED);
	    (this)->AlterDeletedIcon( this->HighlightStartPos, TRUE);
	    (this)->PostMenus( NULL);
	}
	(this)->WantUpdate( this);
    }
    ams::WaitCursor(FALSE);
    return;
}

void captions::SendMessage(int  code )
{
    char FileName[MAXPATHLEN+1];
    class sendmessage *sm;

    if (code == AMS_REPLY_FRESH) {
	ams::WaitCursor(TRUE);
	sm = ((this)->GetFolders())->ExposeSend();
	(sm)->Reset();
	ams::WaitCursor(FALSE);
	return;
    }
    if (this->VisibleCUID <1) {
	message::DisplayString(NULL, 10, "There is no message on display.");
	return;
    }
    ams::WaitCursor(TRUE);
    if ((ams::GetAMS())->CUI_NameReplyFile( this->VisibleCUID, code, FileName) != 0) {
	ams::WaitCursor(FALSE);
	return;
    } 
    sm = ((this)->GetFolders())->ExposeSend();
    if (sm) {
	(sm)->ReadFromFile( FileName, TRUE);
	/* Insert ---- Forwarded Message ---- line. */
	if (code == AMS_REPLY_FORWARD_FMT && environ::GetProfileSwitch("ForwardMessageLine", TRUE)) {
	    long save_modified = sm->BodyText->GetModified();
	    sm->BodyText->InsertCharacters(0, ForwardString, strlen(ForwardString));
	    sm->BodyText->RestoreModified(save_modified);
	}
	/* Put in a newline so when we insert, it comes up 
	 in the plain style. */
#if 0	/* this results in bogus questions about deleting unsent mail, and extra newlines in messages if sending is aborted... users just have to remember to be careful and click on the TOP line... not the one below it, alternately we could try to move the style -rr2b 11/92*/
	(sm->BodyText)->InsertCharacters( 0, "\n", 1);
#endif	
    }
    if (code != AMS_REPLY_FRESH)
	(sm)->SetCurrentState( SM_STATE_INPROGRESS);
    ams::WaitCursor(FALSE);
    return;
}

void captions::SetFolders(class folders  *f)
{
    this->myfold = f;
}

class t822view *
captions::NewBodiesInNewWindow()
{
    class t822view *tv = new t822view;
    class text *t = new text;

    (t)->SetReadOnly( TRUE);	/* -wjh */
    (this)->SetBodies( tv);
    (tv)->SetCaptions( this);
    (tv)->SetDataObject( t);
    ams::InstallInNewWindow((tv)->GetApplicationLayer(), "messages-bodies", "Message Bodies", environ::GetProfileInt("bodies.width", 600), environ::GetProfileInt("bodies.height", 250), tv);
    return(tv);
}

class folders *
captions::NewFoldersInNewWindow()
{
    class folders *f = new folders;

    (this)->SetFolders( f);
    (f)->SetCaptions( this);
    ams::InstallInNewWindow((f)->GetApplicationLayer(), "messages-folders", "Message Folders", environ::GetProfileInt("folders.width", 600), environ::GetProfileInt("folders.height", 120), f);
    return(f);
}

class folders *captions::GetFolders()
{
    if (!this->myfold) {
	(this)->NewFoldersInNewWindow();
    }
    return(this->myfold);
}

class t822view *captions::GetBodView()
{
    if (!this->BodView) {
	(this)->NewBodiesInNewWindow();
    }
    return(this->BodView);
}


void InitKeysMenus(class captions  *captions)
{
    captions->privkeys = keystate::Create(captions, captions_privkeymap);
    captions->privmenus = (captions_privmenulist)->DuplicateML( captions);
}

boolean captions::InitializeClass() 
{
    ATK::LoadClass("textview"); /* make sure the textview is loaded first */
    OneTimeInitKeyMenus(&captions_ATKregistry_ );
    return(TRUE);
}

captions::captions()
{
	ATKinit;

    const char *fontname;
    int fontsize, mailfontbloat = (amsutil::GetOptBit(EXP_WHITESPACE)) ? 2 : 0;

    rawText=FALSE;
    (this)->SetWhatIAm( WHATIAM_CAPTIONS);
    InitKeysMenus(this);
    this->CommentText = NULL;
    this->myframe = NULL;
    this->MenusExpanded = FALSE;
    this->mycursor = NULL;
    this->MarkCount = 0;
    this->FolderSize = this->FetchedFromStart = this->FetchedFromEnd = 0;

    this->CaptText = new text;
    (this)->SetDataObject( this->CaptText);
    this->textscrollinterface = (const struct scrollfns *) (this)->messages::GetInterface( "scroll,vertical");

    this->ActiveCaptionStyle = new style;
    this->ActiveDeletedStyle = new style;
    this->DeletedStyle = new style;
    this->FixedStyle = new style;
    this->GlobalCapStyle = new style;
    this->HighlightStyle = new style;
    this->IconicStyle = new style;
    this->UnderlinedIconicStyle = new style;
    this->MailStyle = new style;
    this->NormalCaptionStyle = new style;

    (this->FixedStyle)->AddNewFontFace( (long) fontdesc_Fixed);
    (this->HighlightStyle)->AddNewFontFace( (long) fontdesc_Italic);
    (this->FixedStyle)->SetFontFamily( "andytype");
    (this->IconicStyle)->SetFontFamily( "msgs");
    (this->UnderlinedIconicStyle)->SetFontFamily( "msgs");
    (this->IconicStyle)->SetFontSize( style_ConstantFontSize, 10);
    (this->UnderlinedIconicStyle)->SetFontSize( style_ConstantFontSize, 10);
    (this->MailStyle)->SetFontSize( style_PreviousFontSize, mailfontbloat);
    (this->GlobalCapStyle)->SetJustification( style_LeftJustified);
    (this->HighlightStyle)->SetJustification( style_LeftJustified);
    (this->FixedStyle)->SetName( "Typewriter");
    (this->ActiveCaptionStyle)->SetNewIndentation( style_ConstantMargin, -10, style_RawDots);
    (this->ActiveDeletedStyle)->SetNewIndentation( style_ConstantMargin, -10, style_RawDots);
    (this->GlobalCapStyle)->SetNewIndentation( style_ConstantMargin, -10, style_RawDots);
    (this->ActiveCaptionStyle)->SetNewLeftMargin( style_ConstantMargin, 20, style_RawDots);
    (this->ActiveDeletedStyle)->SetNewLeftMargin( style_ConstantMargin, 20, style_RawDots);
    (this->GlobalCapStyle)->SetNewLeftMargin( style_ConstantMargin, 20, style_RawDots);
    (this->UnderlinedIconicStyle)->AddFlag( style_Underline);
    if (amsutil::GetOptBit(EXP_FIXCAPTIONS)) {
	(this->ActiveCaptionStyle)->AddNewFontFace( (long) fontdesc_Bold | fontdesc_Fixed);
	(this->ActiveDeletedStyle)->AddNewFontFace( (long) fontdesc_Bold | fontdesc_Fixed);
	(this->DeletedStyle)->AddNewFontFace( (long) fontdesc_Fixed);
	(this->GlobalCapStyle)->AddNewFontFace( (long) fontdesc_Fixed);
	(this->HighlightStyle)->AddNewFontFace( (long) fontdesc_Fixed);
	(this->MailStyle)->AddNewFontFace( (long) fontdesc_Fixed);
	(this->NormalCaptionStyle)->AddNewFontFace( (long) fontdesc_Fixed);
    } else {
	(this->ActiveCaptionStyle)->AddNewFontFace( (long) fontdesc_Bold);
	(this->ActiveDeletedStyle)->AddNewFontFace( (long) fontdesc_Bold);
    }
    (this->CaptText)->SetGlobalStyle( this->GlobalCapStyle);

    this->SouthPoint = NULL;

    this->VisibleCUID = -1;
    bzero(this->VisibleSnapshot, AMS_SNAPSHOTSIZE);
    this->HighlightStartPos = 0;
    this->HighlightLen = 0;
    this->HighlightEnv = NULL;
    this->OldMarkCount = 0;
    this->OldMarks = NULL;
    this->capcache = (struct CaptionCache *) malloc(25 *sizeof(struct CaptionCache));
    this->captioncachesize = 25;
    this->captioncachecount = 0;
    this->FullName = NULL;
    this->ShortName = NULL;
    this->firstcuid = -1;
    ams::AddCheckpointCaption(this);

    this->myfold = NULL;

    fontsize = environ::GetProfileInt("messages.fontsize", 12);
    (this->DeletedStyle)->SetFontSize( style_ConstantFontSize, fontsize - 4);
    (this->ActiveDeletedStyle)->SetFontSize( style_ConstantFontSize, fontsize - 4);
    (this->GlobalCapStyle)->SetFontSize( style_ConstantFontSize, fontsize);

    fontname = amsutil::GetDefaultFontName();
    if (amsutil::GetOptBit(EXP_FIXCAPTIONS)) fontname = "andytype";
    (this->ActiveCaptionStyle)->SetFontFamily( fontname);
    (this->ActiveDeletedStyle)->SetFontFamily( fontname);
    (this->DeletedStyle)->SetFontFamily( fontname);
    (this->GlobalCapStyle)->SetFontFamily( fontname);
    (this->HighlightStyle)->SetFontFamily( fontname);
    (this->MailStyle)->SetFontFamily( fontname);
    (this->NormalCaptionStyle)->SetFontFamily( fontname);

    this->BodView = NULL;

    (this)->ShowHelp();
    (this->CaptText)->SetReadOnly( TRUE);
    THROWONFAILURE((TRUE));
}

void FinalizeProcStuff(class captions  *self)
{
    delete self->privkeys;
    delete self->privmenus;
    delete self->mycursor;
}

captions::~captions()
{
	ATKinit;

    ams::RemoveCheckpointCaption(this);
    (this->CaptText)->Destroy();
    if (this->CommentText) {
	free(this->CommentText);
	this->CommentText = NULL;
    }
    if (this->myfold) {
	(this->myfold)->SetCaptions( NULL);
    }
    if (this->BodView) {
	(this->BodView)->SetCaptions( NULL);
    }
    delete this->ActiveCaptionStyle;
    delete this->NormalCaptionStyle;
    delete this->HighlightStyle;
    delete this->GlobalCapStyle;
    delete this->DeletedStyle;
    delete this->ActiveDeletedStyle;
    delete this->IconicStyle;
    delete this->UnderlinedIconicStyle;
    delete this->MailStyle;
    delete this->FixedStyle;
    FinalizeProcStuff(this);
    if (this->OldMarks) {
	free(this->OldMarks);
	this->OldMarks = NULL;
    }
    if (this->capcache) {
	free(this->capcache);
	this->capcache = NULL;
    }
    if (this->FullName) {
	free(this->FullName);
	this->FullName = NULL;
    }
    if (this->ShortName) {
	free(this->ShortName);
	this->ShortName = NULL;
    }
}


void captions::PostMenus(class menulist  *ml)
{
    (this->privmenus)->ClearChain();
    if (ml) (this->privmenus)->ChainAfterML( ml, (long)ml);
    (this)->messages::PostMenus( this->privmenus);
}

void captions::PostKeyState(class keystate  *ks)
{
    this->privkeys->next = NULL;
    if (amsutil::GetOptBit(EXP_KEYSTROKES)) {
	if (ks) (ks)->AddAfter( this->privkeys);
	(this)->messages::PostKeyState( this->privkeys);
    } else {
	(this)->messages::PostKeyState( ks);
    }
}

void
captions::ActOnMarkedMessages(int  Code, char  *GivenName /* Not always supplied */)
{
    char ErrorText[256];
    struct CaptionCache *hc;
    int j, k, resultcode, OpCode, goodct = 0, len;
    Boolean IsActiveCaption, HadDisaster, errct;
    char HeaderBuf[2000];
    char *HeadAccum = NULL;
    class sendmessage *sm = NULL;

    if ((this->MarkCount <= 0) && (Code != MARKACTION_RESTORE)) {
	message::DisplayString(NULL, 10, "There are no marked messages");
	return;
     }
    ams::WaitCursor(TRUE);
    HadDisaster = FALSE;
    errct = 0;
    for (j = 0; j < this->captioncachecount && !HadDisaster; ++j) {
	hc = &this->capcache[j];
	if (hc->IsMarked || (Code == MARKACTION_RESTORE)) {
	    if (hc->env->data.style == this->ActiveDeletedStyle || hc->env->data.style == this->ActiveCaptionStyle) {
		IsActiveCaption = TRUE;
	    } else {
		IsActiveCaption = FALSE;
	    }
	    switch(Code) {
		case MARKACTION_APPENDTOFILE:
		case MARKACTION_APPENDTOFILERAW:
		    if ((this)->AppendOneMessageToFile( hc->cuid, GivenName, (Code == MARKACTION_APPENDTOFILERAW) ? 1 : 0)) {
			errct++;
			HadDisaster = TRUE;
		    } else ++goodct;
		    break;
		case MARKACTION_CLASSIFYBYNAME:
		case MARKACTION_APPENDBYNAME:
		case MARKACTION_COPYBYNAME:
		    if (Code == MARKACTION_APPENDBYNAME) {
			OpCode = hc->MayModify ? MS_CLONE_APPENDDEL : MS_CLONE_APPEND;
		    } else if (Code == MARKACTION_COPYBYNAME) {
			OpCode = MS_CLONE_COPY;
		    } else {
			OpCode = hc->MayModify ? MS_CLONE_COPYDEL : MS_CLONE_COPY;
		    }
		    resultcode = (ams::GetAMS())->CUI_CloneMessage( hc->cuid, GivenName, OpCode);
		    if (resultcode) {
			errct++;
			HadDisaster = TRUE;
			break;
		    } 		    
		    if (OpCode == MS_CLONE_APPEND || OpCode == MS_CLONE_COPY) {
			++goodct;
			break;
		    }
		    /* FALL THROUGH to delete */
		case MARKACTION_DELETE:
		    if ((ams::GetAMS())->CUI_DeleteMessage( hc->cuid) == 0) {
			(this->CaptText)->SetEnvironmentStyle( hc->env, IsActiveCaption ? this->ActiveDeletedStyle : this->DeletedStyle);
			if (IsActiveCaption) {
			    AMS_SET_ATTRIBUTE(this->VisibleSnapshot, AMS_ATT_DELETED);
			}
			(this)->AlterDeletedIcon( hc->offset, TRUE);
			++goodct;
		    } else {
			++errct;
			HadDisaster = TRUE;
			/* Errors were reported by cui routine */
		    }
		    break;
		case MARKACTION_UNDELETE:
		    if ((ams::GetAMS())->CUI_UndeleteMessage( hc->cuid) == 0) {
			(this->CaptText)->SetEnvironmentStyle( hc->env, IsActiveCaption ? this->ActiveCaptionStyle : this->NormalCaptionStyle);
			if (IsActiveCaption) {
			    AMS_UNSET_ATTRIBUTE(this->VisibleSnapshot, AMS_ATT_DELETED);
			}
			(this)->AlterDeletedIcon( hc->offset, FALSE);
			++goodct;
		    } else {
			++errct;
			HadDisaster = TRUE;
			/* Errors were reported by cui routine */
		    }
		    break;
		case MARKACTION_PRINT:
		    if ((ams::GetAMS())->CUI_PrintBodyFromCUIDWithFlags( hc->cuid, 0, NULL)) {
			HadDisaster = TRUE;
			++errct;
		    } else {
			++goodct;
		    }
		    /* Errors were reported by cui routine */
		    break;
		case MARKACTION_RESTORE:
		    for (k=0; k<this->OldMarkCount; ++k) {
			if (hc->cuid == this->OldMarks[k] && (!hc->IsMarked)) {
			    (this)->ToggleMark( hc, hc->offset);
			    ++goodct;
			    break;
			}
		    }
		    break;
		case MARKACTION_EXCERPT:
		    if (!sm) {
			sm = ((this)->GetFolders())->ExposeSend();
			if (!sm) return;
		    }
		    if (j == (this->captioncachecount - 1)) {
			len = (this->CaptText)->GetLength() - hc->offset;
		    } else {
			len = this->capcache[j+1].offset - hc->offset;
		    }
		    (this)->DisplayNewBody( hc->cuid, hc->offset, len, hc->env);
		    (sm->BodyTextview)->SetDotPosition( ((class text *) (sm->BodyTextview)->GetDataObject())->GetLength());
		    (sm)->QuoteBody();
		    ++goodct;
		    break;
		case MARKACTION_REPLYALL:
		case MARKACTION_REPLYSENDERS:
		    HeaderBuf[0] = '\0';
		    (ams::GetAMS())->CUI_GetHeaderContents( hc->cuid, NULL, (Code == MARKACTION_REPLYSENDERS) ? HP_REPLY_TO : HP_ALLREPLY, HeaderBuf, sizeof(HeaderBuf) - 2);
		    if (HeadAccum) {
			HeadAccum = (char *)realloc(HeadAccum, strlen(HeadAccum) + strlen(HeaderBuf) + 5);
			strcat(HeadAccum, ",\n\t");
			strcat(HeadAccum, HeaderBuf);
		    } else {
			HeadAccum = (char *)malloc(1+strlen(HeaderBuf));
			strcpy(HeadAccum, HeaderBuf);
		    }
		    ++goodct;
		    break;
		case MARKACTION_RESEND:
		    if ((ams::GetAMS())->CUI_ResendMessage( hc->cuid, GivenName)) {
			HadDisaster = TRUE;
		    } else {
			++goodct;
		    }
		    break;
	    }
	    if (HadDisaster) {
		if ((ams::GetAMS())->GetBooleanFromUser( "Do you want to continue with the other marked messages", FALSE)) {
		    HadDisaster = FALSE;
		}
	    }
	}
    }
    if (HeadAccum) {
	if (!sm) {
	    sm = ((this)->GetFolders())->ExposeSend();
	    if (!sm) return;
	}
	(sm)->AddToToHeader( HeadAccum);
	free(HeadAccum);
	HeadAccum = NULL;
    }
    if (errct) {
	char Foobar[100];
	sprintf(ErrorText, "Errors were encountered on %s ", amsutil::cvEng(errct, 0, 1000));
	sprintf(Foobar, "of the %s marked messages.", amsutil::cvEng(this->MarkCount, 0, 1000));
	strcat(ErrorText, Foobar);
	message::DisplayString(NULL, 50, ErrorText);
    } else {
	switch (Code) {
	    case MARKACTION_RESTORE:
		sprintf(ErrorText, "Restored %s old marks.", amsutil::cvEng(goodct, 0, 1000));
		break;
	    case MARKACTION_RESEND:
		sprintf(ErrorText, "Resent %s messages to %s.", amsutil::cvEng(goodct, 0, 1000), GivenName);
		break;
	    case MARKACTION_DELETE:
		sprintf(ErrorText, "Deleted %s messages.", amsutil::cvEng(goodct, 0, 1000));
		break;
	    case MARKACTION_UNDELETE:
		sprintf(ErrorText, "Undeleted %s messages.", amsutil::cvEng(goodct, 0, 1000));
		break;
	    case MARKACTION_CLASSIFYBYNAME:
		sprintf(ErrorText, "Classified %s messages into %s.", amsutil::cvEng(goodct, 0, 1000), GivenName);
		break;
	    case MARKACTION_PRINT:
		sprintf(ErrorText, "Printed %s messages.", amsutil::cvEng(goodct, 0, 1000));
		break;
	    case MARKACTION_APPENDBYNAME:
		sprintf(ErrorText, "Appended %s messages to folder %s.", amsutil::cvEng(goodct, 0, 1000), GivenName);
		break;
	    case MARKACTION_EXCERPT:
		sprintf(ErrorText, "Excerpted %s messages.", amsutil::cvEng(goodct, 0, 1000));
		break;
	    case MARKACTION_REPLYSENDERS:
		sprintf(ErrorText, "Replying to senders of %s messages.", amsutil::cvEng(goodct, 0, 1000));
		break;
	    case MARKACTION_REPLYALL:
		sprintf(ErrorText, "Replying widely to %s messages.", amsutil::cvEng(goodct, 0, 1000));
		break;
	    case MARKACTION_COPYBYNAME:
		sprintf(ErrorText, "Copied %s messages into folder %s.", amsutil::cvEng(goodct, 0, 1000), GivenName);
		break;
	    case MARKACTION_APPENDTOFILE:
	    case MARKACTION_APPENDTOFILERAW:
		sprintf(ErrorText, "Appended %s messages to file %s.", amsutil::cvEng(goodct, 0, 1000), GivenName);
		break;
	    default:
		sprintf(ErrorText, "Did something to %s messages.", amsutil::cvEng(goodct, 0, 1000));
		break;
	}
	message::DisplayString(NULL, 10, ErrorText);
	(this)->PostMenus( NULL);
	if (Code == MARKACTION_CLASSIFYBYNAME
		|| Code ==  MARKACTION_APPENDBYNAME
	    || Code ==  MARKACTION_COPYBYNAME) {
	    char Nick[1+MAXPATHLEN];
	    (ams::GetAMS())->CUI_BuildNickName( GivenName, Nick);
	    SetLastClassification(this, Nick);
	}
    }
    (this)->WantUpdate( this);
    ams::WaitCursor(FALSE);
    return;
}
