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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/folders.C,v 1.8 1995/11/07 20:17:10 robr Stab74 $";
#endif


 

#include <andrewos.h> /* sys/file.h */
#include <textview.H>
#include <cui.h>
#include <fdphack.h>
#include <hdrparse.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <netinet/in.h>  /* for htonl, etc. */
#include <errprntf.h>
#include <ctype.h>
#include <rect.h>

#include <text.H>
#include <message.H>
#include <environment.H>
#include <bind.H>
#include <proctable.H>
#include <sbutton.H>
#include <sbuttonv.H>
#include <environ.H>
#include <lpair.H>
#include <keystate.H>
#include <menulist.H>
#include <style.H>
#include <fontdesc.H>

#include <ams.H>
#include <amsutil.H>
#include <sendmessage.H>
#include <captions.H>
#include <nbutterv.H>
#include <folders.H>
#include <msgsvers.h>

int foldersDebugging=0;
#define mdebug(n, x) ((foldersDebugging & (n)) ? printf x : 0)
#ifdef DOTIMING
extern int GlobalTransactionCount;
#else /* DOTIMING */
#define Logstat(x,y,z)
#endif /* DOTIMING */

static class keymap *folders_standardkeymap;
static class menulist *folders_standardmenulist;

static char *ForgetItString = "Forget it -- do nothing.";



ATKdefineRegistry(folders, messages, folders::InitializeClass);

static char * WhichIcon(int  substatus);
void folders_ClearFolders(class folders  *ci);
void folders_DoClick(class folders  *folders, boolean  IsLeftClick , boolean  IgnorePosition);
static void UpdateBEDirCachePositions(class folders  *ci, struct BEDirCache  *entry, int  addedlen);
static void RemoveFromBEDirCache(class folders  *ci, char  *longname , char  *shortname);
static int AddToBEDirCache(class folders  *ci, char  *longname , char  *shortname, int  substatus);
static int BEDC_AddComment(class folders  *ci, struct BEDirCache  *bdc, char  *comm, int  *addedlen);
static void InsertFolderNameInText(class folders  *f, struct BEDirCache  *bdcent, char  *comm);
static void AddSetupItem(class folders  *ci, char  *longname , char  *shortname, int  substatus , int  showingnewstuff , int  HasNew, boolean  *HasCleared);
int folders_SetupList(class folders  *ci, int  code, char  *thingstoread[]);
static void AlterSubStatus(class folders  *ci, char  *dir , int  status, char  *shortname);
static void UnhighlightFolderName(class folders  *ci);
static void HighlightSpecificFolderName(class folders  *ci, char  *name , char  *CommText, int  i);
static void HighlightFolderName(class folders  *ci, char  *name , char  *CommText);
static void DoButton(class sbutton  *b, enum view_MouseAction  action, int  ind, class folders  *self);
void folders_ExposeCap(class folders  *self);
static void InitKeyMenusStyles(class folders  *folders);
static void OneTimeInitKeyMenus();

extern void folders_ForceUpdate();
extern void folders_ConsiderResettingDescription(class folders  *ci, int  code, Boolean  FirstTime);
extern void folders_CreateFoldersCursor(class folders  *self);
extern void folders_Warp(class im  *im);
extern void folders_Expose(class im  *im);
extern void folders_Hide(class im  *im);
extern void folders_Vanish(class im  *im);
extern void folders_FinalizeProcStyleStuff(class folders  *self);
extern void folders_DownFocus(class folders  *self);
extern void folders_UpFocus(class folders  *self);
extern void folders_SimulateLeftClick(class folders  *self);
extern void folders_TextviewCompound(class textview  *tv, char  *cmds);
extern void folders_FoldersCompound(class folders  *self, char  *cmds);
extern void FoldersTextviewCommand(class folders  *self, char  *cmds);
extern void FoldersMessagesCommand(class folders  *self, char  *cmds);
extern void FoldersSendmessageCommand(class folders  *self, char  *cmds);
extern void FoldersCaptionsCommand(class folders  *self, char  *cmds);


void folders::HandleAsyncPrefetch()
{
#ifndef RCH_ENV
    int which, skipped;

    if (this->CurrentConfiguration != LIST_ALL_FOLDERS) {
	(this)->FindNextFolder( &which, &skipped);
	if (which < this->MainDirCacheCount) {
	    char Text[60+MAXPATHLEN];

	    sprintf(Text, "Starting pre-fetch of %s... ", this->MainDirCache[which].ShortName);
	    message::DisplayString(NULL, 10, Text);
	    folders_ForceUpdate();
	    (ams::GetAMS())->MS_PrefetchMessage( this->MainDirCache[which].FullName, NULL, FALSE);
	    strcat(Text, "done.");
	    message::DisplayString(NULL, 10, Text);
	    folders_ForceUpdate();
	}
    }
#endif /*RCH_ENV*/
    this->NeedsToPrefetch = FALSE;
}


static char *
WhichIcon(int  substatus)
{
	char *whichicon;

	switch (substatus) {
	    case AMS_ASKSUBSCRIBED:
		whichicon = SICON_SUB_ASK;
		break;
	    case AMS_ALWAYSSUBSCRIBED:
		whichicon = SICON_SUB_NORM;
		break;
	    case AMS_SHOWALLSUBSCRIBED:
		whichicon = SICON_SUB_ALL;
		break;
	    case AMS_PRINTSUBSCRIBED:
		whichicon = SICON_SUB_PRINT;
		break;
	    default:
	    case AMS_UNSUBSCRIBED:
		whichicon = SICON_SUB_NONE;
		break;
	}
	return(whichicon);
}

void folders_ClearFolders(class folders  *ci)
{
    int i;
    class text *d;

    d = (class text *) (ci)->GetDataObject();
    (d)->ClearCompletely();
    (d)->SetGlobalStyle( ci->GlobalCapStyle);
    if (ci->MainDirCacheCount <= 0) {
	return;
    }
    for (i=0; i<ci->MainDirCacheCount; ++i) {
	free(ci->MainDirCache[i].FullName);
	free(ci->MainDirCache[i].ShortName);
    }
    ci->MainDirCacheCount = 0;
    ci->HighlightEnv = NULL;
    (ci)->SetDotPosition( 0);
    (ci)->SetDotLength( 0);
    (ci)->PostMenus( NULL);
    (ci)->WantUpdate( ci);
}

void folders::ShowHelp()
{
    char InitString[50];
    int pos, len;
    static char *blurb1 = "by N. Borenstein\n";
    static char *altblurb1 = "by N. Borenstein and R. Glickstein\n";
    static char *blurb2 = "a multi-media interface to the Andrew Message System\n";
    static char *blurb3 = "also by J. Rosenberg, C. Everhart, A. Stoller, and R. Glickstein.\n\n";
    static char *altblurb3 = "also by J. Rosenberg, C. Everhart, and A. Stoller.\n\n";
    static int bobgvanity = -1;
    class environment *et;
    class text *mytext = (class text *) (this)->GetDataObject();

    folders_ClearFolders(this);
    sprintf(InitString, "Messages %d.%d\n", MESSAGES_MAJOR_VERSION, MESSAGES_MINOR_VERSION);
    len = strlen(InitString);
    (mytext)->AlwaysInsertCharacters( 0, InitString, len);
    et = (mytext->rootEnvironment)->InsertStyle( 0, this->BigCenterStyle, 1);
    (et)->SetLength( len-1);
    pos = len;
    if (bobgvanity < 0) {
        bobgvanity = environ::GetProfileInt("bobgvanity", 0);
    }
    len = strlen(bobgvanity ? altblurb1 : blurb1);
    (mytext)->AlwaysInsertCharacters( pos, bobgvanity ? altblurb1 : blurb1, len);
    et = (mytext->rootEnvironment)->InsertStyle( pos, this->SmallCenterStyle, 1);
    (et)->SetLength( len-1);
    pos += len;
    len = strlen(blurb2);
    (mytext)->AlwaysInsertCharacters( pos, blurb2, len);
    et = (mytext->rootEnvironment)->InsertStyle( pos, this->CenterStyle, 1);
    (et)->SetLength( len-1);
    pos += len;
    len = strlen(bobgvanity ? altblurb3 : blurb3);
    (mytext)->AlwaysInsertCharacters( pos, bobgvanity ? altblurb3 : blurb3, len);
    et = (mytext->rootEnvironment)->InsertStyle( pos, this->SmallCenterStyle, 1);
    (et)->SetLength( len-1);

    (this)->WantUpdate( this);
}

class view *
folders::Hit(enum view_MouseAction  action, long  x , long  y , long  nclicks  )
{
    if (action != view_LeftUp && action != view_RightUp) return((class view *) this);
    (this)->messages::Hit( view_LeftDown, x, y, 1);
    (this)->SimulateClick( (action == view_LeftUp));
    return((class view *) this);
}

static void folders_wrap_SimulateClick(struct folders *self, boolean  IsLeftClick)
{
    self->SimulateClick(IsLeftClick);
}

void folders::SimulateClick(boolean  IsLeftClick)
{
    folders_DoClick(this, IsLeftClick, FALSE);
}

void folders_DoClick(class folders  *folders, boolean  IsLeftClick , boolean  IgnorePosition)
{
    int linestart, hitpos, i;

    (folders)->WantInputFocus( folders);
    if (folders->MainDirCacheCount<=0) return;
    if (((class text *) (folders)->GetDataObject())->GetLength() <= 0) return;
    hitpos = (folders)->GetDotPosition();
    for (i=0; i<folders->MainDirCacheCount; ++i) {
	if (folders->MainDirCache[i].pos > hitpos) break;
    }
    --i;
    linestart = folders->MainDirCache[i].pos;
    (folders)->SetDotPosition( linestart);
    (folders)->SetDotLength( 0);
    if ((!IgnorePosition && (hitpos - linestart) < 4) || !IsLeftClick) {
	int myoffset;

	myoffset = hitpos - linestart;
	if ((myoffset > 1) && (myoffset < 5) && ((class text *) (folders)->GetDataObject())->GetChar( linestart + 2) == ICON_FOLDER) {
	    if (IsLeftClick || !amsutil::GetOptBit(EXP_MARKING)) {
		((folders)->GetCaptions())->FileCurrent( folders->MainDirCache[i].FullName, folders->MainDirCache[i].ShortName);
	    } else {
		((folders)->GetCaptions())->FileMarked( folders->MainDirCache[i].FullName, folders->MainDirCache[i].ShortName);
	    }
	} else {
	    (folders)->ActionHit( folders->MainDirCache[i].substatus, folders->MainDirCache[i].FullName, folders->MainDirCache[i].ShortName);
	}
	return;
    }
    ams::WaitCursor(TRUE);
    ((folders)->GetCaptions())->InsertUpdatesInDocument( folders->MainDirCache[i].ShortName, folders->MainDirCache[i].FullName, FALSE);
    folders_ExposeCap(folders);
    ams::WaitCursor(FALSE);
}

static void UpdateBEDirCachePositions(class folders  *ci, struct BEDirCache  *entry, int  addedlen)
{
    Boolean FoundIt = FALSE;
    int i;

    for (i=0; i<ci->MainDirCacheCount; ++i) {
	if (FoundIt) {
	    ci->MainDirCache[i].pos += addedlen;
	    ci->MainDirCache[i].commentstart += addedlen;
	} else if (&ci->MainDirCache[i] == entry) {
	    entry->len += addedlen;
	    FoundIt = TRUE;
	}
    }
}

static void RemoveFromBEDirCache(class folders  *ci, char  *longname , char  *shortname)
{
    int i;

    for (i=0; i<ci->MainDirCacheCount; ++i) {
	if (!strcmp(longname, ci->MainDirCache[i].FullName)) {
	    struct BEDirCache *bdc;

	    /* found it */
	    bdc = &ci->MainDirCache[i];
	    if (ci->HighlightEnv == ci->MainDirCache[i].env) {
		ci->HighlightEnv = NULL;
	    }
	    if (bdc->FullName) free (bdc->FullName);
	    if (bdc->ShortName) free (bdc->ShortName);
	    if (bdc->len) {
		((class text *) (ci)->GetDataObject())->AlwaysDeleteCharacters( bdc->pos, bdc->len);
	    }
	    UpdateBEDirCachePositions(ci, bdc, -bdc->len);
	    while (i< ci->MainDirCacheCount - 1) {
		bcopy(&ci->MainDirCache[i+1], &ci->MainDirCache[i], sizeof(struct BEDirCache));
		++i;
	    }
	    --ci->MainDirCacheCount;
	    return;
	}
    }
}

static int AddToBEDirCache(class folders  *ci, char  *longname , char  *shortname, int  substatus)
{
    int i = ci->MainDirCacheCount, j;

    ci->MainDirCache[i].pos = ((class text *) (ci)->GetDataObject())->GetLength();
    if (longname == NULL) longname = "";
    ci->MainDirCache[i].FullName = (char *)malloc(1+strlen(longname));
    strcpy(ci->MainDirCache[i].FullName, longname);
    ci->MainDirCache[i].ShortName = (char *)malloc(1+strlen(shortname));
    strcpy(ci->MainDirCache[i].ShortName, shortname);
    ci->MainDirCache[i].substatus = substatus;
    if (++ci->MainDirCacheCount >= ci->MainDirCacheSize) {
	j = (ci->MainDirCacheSize * 2) - (ci->MainDirCacheSize / 2);
	ci->MainDirCache = (struct BEDirCache *) realloc(ci->MainDirCache, j * sizeof(struct BEDirCache));
	bzero(&ci->MainDirCache[ci->MainDirCacheSize], (j-ci->MainDirCacheSize) * sizeof(struct BEDirCache));
	ci->MainDirCacheSize = j;
    }
    return(i);
}


static int BEDC_AddComment(class folders  *ci, struct BEDirCache  *bdc, char  *comm, int  *addedlen)
{
    int newlen;

    if (ci->VeryNarrow) return(bdc->pos);
    if (!comm) comm = "";
    if (!bdc->commentstart) bdc->commentstart = ((class text *) (ci)->GetDataObject())->GetLength();
    if (bdc->commentlen) ((class text *) (ci)->GetDataObject())->AlwaysDeleteCharacters( bdc->commentstart, bdc->commentlen);
    newlen = strlen(comm);
    *addedlen += newlen - bdc->commentlen;
    bdc->commentlen = newlen;
    ((class text *) (ci)->GetDataObject())->AlwaysInsertCharacters( bdc->commentstart, comm, newlen);
    return(bdc->commentstart + bdc->commentlen);
}

static void InsertFolderNameInText(class folders  *f, struct BEDirCache  *bdcent, char  *comm)
{
    class environment *et;
    char IconBuf[10];
    char FoldChar = (amsutil::GetOptBit(EXP_FILEICONCAPTIONS)) ? ICON_FOLDER : ' ';
    int addlen, inspos = bdcent->pos;
    class text *d = (class text *) (f)->GetDataObject();

    sprintf(IconBuf, "%s %c ", WhichIcon(bdcent->substatus), FoldChar);
    (d)->AlwaysInsertCharacters( inspos, IconBuf, 4);
    et = (d->rootEnvironment)->InsertStyle( inspos, f->IconicStyle, 1);
    (et)->SetLength( 3);
    inspos += 4;
    addlen = strlen(bdcent->ShortName);
    (d)->AlwaysInsertCharacters( inspos, bdcent->ShortName, addlen);
    et = (d->rootEnvironment)->InsertStyle( inspos, f->Normalfolderstyle, 1);
    bdcent->env = et;
    bdcent->commentstart = inspos+addlen;
    bdcent->commentlen = 0;
    BEDC_AddComment(f, bdcent, comm, &addlen);
    (d)->AlwaysInsertCharacters( bdcent->commentstart+bdcent->commentlen, " \n", 2);
    (et)->SetLength( addlen);
    bdcent->len = addlen + 6;
}


static void AddSetupItem(class folders  *ci, char  *longname , char  *shortname, int  substatus , int  showingnewstuff , int  HasNew, boolean  *HasCleared)
{
    struct BEDirCache *bdcent;
    char *comm;
    int whichcache;

    if (!*HasCleared) {
	folders_ClearFolders(ci);
	*HasCleared = TRUE;
    }
    whichcache = AddToBEDirCache(ci, longname, shortname, substatus);
    bdcent = &ci->MainDirCache[whichcache];
    if (substatus == AMS_ASKSUBSCRIBED || substatus == AMS_PRINTSUBSCRIBED || !HasNew) {
	bdcent->SkipMe = TRUE;
    } else {
	bdcent->SkipMe = FALSE;
    }
    if (substatus == AMS_PRINTSUBSCRIBED && showingnewstuff) {
	(ams::GetAMS())->CUI_PrintUpdates( longname, shortname);
    }
    if (!ci->VeryNarrow && showingnewstuff) {
	switch(substatus) {
	    case AMS_SHOWALLSUBSCRIBED:
		comm = HasNew ? " (Show-All Subscribed) " : " (Show-All Subscribed; Empty) ";
		break;
	    case AMS_ASKSUBSCRIBED:
		comm = HasNew ? " (Ask-Subscribed; Has New) " : " (Ask-Subscribed; No New) ";
		break;
	    default:
		comm = HasNew ? " (Has New Messages) " : " (Nothing New) ";
		break;
	}
    } else {
	comm = NULL;
    }
    InsertFolderNameInText(ci, bdcent, comm);
}


int folders_SetupList(class folders  *ci, int  code, char  *thingstoread[])
{
    char *whattofree;
    char PathElt[MAXPATHLEN+1], MapFile[MAXPATHLEN+1], RemoteMapFile[MAXPATHLEN+1], ErrorText[256], *s=NULL, *shortname, *longname, *nextline, *t;
    int spcode = 0, substatus = 0, ShowUnsubscribed = 0, pathindex = 0;
    int HasNew;
    FILE *mfp;
    struct stat stbuf;
    long errcode;
    boolean HasCleared = FALSE;

    if (ci->HasSetUp) {
	folders_ConsiderResettingDescription(ci, code, FALSE);
	return(0);
    }
    switch(code) {
	case LIST_ALL_FOLDERS:
	    ShowUnsubscribed = 1;
	    pathindex = 0;
	    break;
	case LIST_SUBSCRIBED:
	    ShowUnsubscribed = 0;
	    pathindex = 0;
	    break;
	case LIST_MAIL_FOLDERS:
	    ams::InitializeClassList();
	    ShowUnsubscribed = 1;
	    pathindex = AMS_MAILPATH;
	    break;
	case LIST_AS_REQUESTED:
	    break;
	case LIST_NEWONLY:
	    message::DisplayString(NULL, 10, "Checking to see which folders have new messages...");
	    folders_ForceUpdate();
	    break;
	default:
	    (ams::GetAMS())->ReportError( "Internal error --invalid parameter to setup folder list", ERR_CRITICAL, FALSE, 0);
	    return(-1);
    }

    do {
	if (code == LIST_NEWONLY) {
	    int changed, unavail, missing, slow, fast;

	    errcode = (ams::GetAMS())->MS_NameChangedMapFile( MapFile, ci->MailOnlyMode, TRUE, &changed, &unavail, &missing, &slow, &fast);
	    if (errcode) {
		(ams::GetAMS())->ReportError( "Cannot get list of changed folders", ERR_WARNING, TRUE, errcode);
		return(-1);
	    }
	    if (unavail > 0) {
		char Foobar[100];
		sprintf(ErrorText, "%s of ", amsutil::cvEng(changed, 1, 100));
		sprintf(Foobar, "%s subscriptions changed (%d unavailable, %d unchanged.).", amsutil::cvEng(slow+fast+unavail, 0, 100), unavail, slow+fast-changed);
		strcat(ErrorText, Foobar);
	    } else {
		char Foobar[100];
		sprintf(ErrorText, "%s of your ", amsutil::cvEng(changed, 1, 100));
		sprintf(Foobar, "%s subscriptions have changed (%d have nothing new).", amsutil::cvEng(slow+fast, 0, 100), slow+fast-changed);
		strcat(ErrorText, Foobar);
	    }
	    message::DisplayString(ci, 10, ErrorText); /* Force this message in right window */
	} else if (code == LIST_AS_REQUESTED) {
	    if (!thingstoread) break;
	} else {
	    if (code == LIST_SUBSCRIBED) {
		PathElt[0] = '\0';
	    } else {
		spcode = (ams::GetAMS())->MS_GetSearchPathEntry( pathindex, PathElt, MAXPATHLEN);
		if (pathindex != AMS_MAILPATH) ++pathindex;
		if (spcode) break;
	    }
	    if ((errcode = (ams::GetAMS())->MS_NameSubscriptionMapFile( PathElt, MapFile)) != 0) {
		sprintf(ErrorText, "MS can not generate subscription map file for %s", PathElt[0] ? PathElt : "subscribed list");
		if ((ams::GetAMS())->AMS_ErrNo() == ENOENT) {
		    (ams::GetAMS())->ReportSuccess( ErrorText);
		    continue; /* user may not have his own message dir, for example */
		}
		(ams::GetAMS())->ReportError( ErrorText, ERR_CRITICAL, TRUE, errcode);
		continue;
	    }
	}
	if (code == LIST_AS_REQUESTED) {
	    int i;
	    for (i=0; thingstoread[i]; ++i) {
		AddSetupItem(ci, NULL, thingstoread[i], -1, FALSE, 1, &HasCleared);
	    }
	} else {
	    if (!(ams::GetAMS())->CUI_OnSameHost() && code != LIST_AS_REQUESTED) {
		strcpy(RemoteMapFile, MapFile);
		(ams::GetAMS())->CUI_GenTmpFileName( MapFile);
		if ((errcode = (ams::GetAMS())->CUI_GetFileFromVice( MapFile, RemoteMapFile)) != 0) {
		    sprintf(ErrorText, "Cannot copy map file %s from server file %s", MapFile, (ams::GetAMS())->ap_Shorten( RemoteMapFile));
		    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, TRUE, errcode);
		}
		(ams::GetAMS())->MS_UnlinkFile( RemoteMapFile);
	    }
	    if ((mfp = fopen(MapFile, "r")) == NULL) {
		sprintf(ErrorText, "Cannot open map file %s (for %s - error %d)", MapFile, PathElt[0] ? PathElt : "subscribed list", errno);
		(ams::GetAMS())->ReportError( ErrorText, ERR_CRITICAL, FALSE, 0);
		unlink(MapFile);
		continue;
	    }
	    mdebug(4, ("Opened map file %s, reading it...\n", MapFile));
	    if (fstat(fileno(mfp), &stbuf)) {
		sprintf(ErrorText, "Cannot stat map file %s (for %s) - error %d", MapFile, PathElt[0] ? PathElt : "subscribed list", errno);
		(ams::GetAMS())->ReportError( ErrorText, ERR_CRITICAL, FALSE, 0);
		fclose(mfp);
		unlink(MapFile);
		continue;
	    }
	    s = (char *)malloc(1+stbuf.st_size);
	    if (!s) {
		sprintf(ErrorText, "Cannot allocate memory for map file %s (for %s)", MapFile, PathElt[0] ? PathElt : "subscribed list");
		(ams::GetAMS())->ReportError( ErrorText, ERR_CRITICAL, FALSE, 0);
		fclose(mfp);
		unlink(MapFile);
		continue;
	    }
	    whattofree = s;
	    if (read(fileno(mfp), s, stbuf.st_size) != stbuf.st_size) {
		sprintf(ErrorText, "Cannot read map file %s (for %s - error %d)", MapFile, PathElt[0] ? PathElt : "subscribed list", errno);
		(ams::GetAMS())->ReportError( ErrorText, ERR_CRITICAL, FALSE, 0);
		free(whattofree);
		fclose(mfp);
		unlink(MapFile);
		continue;
	    }
	    s[stbuf.st_size] = '\0';
	    mdebug(4, ("Read map file %s, parsing it...\n", MapFile));
	    for ( ; *s; s = nextline) {
		nextline = index(s, '\n');
		if (nextline) *nextline++ = '\0';
		mdebug(4, ("Parsing %s\n", s));
		longname = index(s, ':');
		if (longname) {
		    *longname ++ = '\0';
		} else {
		    longname = s;
		}
		shortname = s;
		HasNew = 1;
		s = index(longname, ' ');
		if (s) {
		    *s++ = '\0';
		    t = index(s, ' ');
		    if (t) {
			*t++ = '\0';
			HasNew = atoi(t);
		    }
		    substatus = atoi(s);
		} else {
		    substatus = AMS_UNSUBSCRIBED;
		}
		if (substatus == AMS_UNSUBSCRIBED && !ShowUnsubscribed) continue;
		if (!HasNew && substatus != AMS_ASKSUBSCRIBED && substatus != AMS_SHOWALLSUBSCRIBED) continue;
		AddSetupItem(ci, longname, shortname, substatus, (code == LIST_NEWONLY), HasNew, &HasCleared);
	    }
	    free(whattofree);
	    fclose(mfp);
	    unlink(MapFile);
	    mdebug(4, ("Done with map file %s\n", MapFile));
	    (ci)->WantUpdate( ci);
	}
    } while (!spcode && pathindex != AMS_MAILPATH && code != LIST_SUBSCRIBED && code != LIST_NEWONLY && code != LIST_AS_REQUESTED);
    ci->HasSetUp = 1;
    folders_ConsiderResettingDescription(ci, code, TRUE);
    return(0);
}

void folders::ActionHit(int  substatus, char  *FullName , char  *nickname)
{
    char    ErrorText[256], *ActionVector[10], Question[100+MAXPATHLEN];
    int result, quicknewstatus, vecsize;
    Boolean HasCurrent = FALSE, NowPlaying;
    class captions *mainci;
    class sendmessage *sendm;
    long errcode;

    sprintf(Question, "What do you want to do with '%s'?", nickname);
    ActionVector[0] = Question;
    ActionVector[1] = ForgetItString;
    ActionVector[2] = "Explain what it is";
    if (substatus != AMS_UNSUBSCRIBED) {
	ActionVector[3] = "Unsubscribe";
	quicknewstatus = AMS_UNSUBSCRIBED;
    } else {
	ActionVector[3] = "Subscribe";
	quicknewstatus = AMS_ALWAYSSUBSCRIBED;
    }
    ActionVector[4] = "Alter subscription status";
    ActionVector[5] = "Post a message on it";
    mainci = (this)->GetCaptions();
    if (mainci->FullName && !strcmp(FullName, mainci->FullName)) {
	NowPlaying = TRUE;
	ActionVector[6] = "Update it (Find New Messages; Purge)";
    } else {
	NowPlaying = FALSE;
	ActionVector[6] = "See the messages";
    }
    vecsize = 7;
    if (!NowPlaying && amsutil::GetOptBit(EXP_FILEINTO)) {
	if (mainci->VisibleCUID > 0) {
	    ActionVector[vecsize++] = "File current message into it";
	    HasCurrent = TRUE;
	}
	if (mainci->MarkCount > 0) {
	    ActionVector[vecsize++] = "File marked messages into it";
	}
    }
    ActionVector[vecsize] = NULL;
    result = (ams::GetAMS())->ChooseFromList( ActionVector, 1);
    switch(result) {
	case 1:
	    break;
	case 2:
	    (this)->ExplainDir( FullName, nickname);
	    break;
	case 3:
	    substatus = quicknewstatus;
	    ams::WaitCursor(TRUE);
	    if ((errcode = (ams::GetAMS())->MS_SetSubscriptionEntry( FullName, nickname, substatus)) != 0) {
		ams::WaitCursor(FALSE);
		if ((ams::GetAMS())->AMS_ErrNo() == EACCES) {
		    sprintf(ErrorText, "'%s' is private; you don't have read-access or are unauthenticated.", nickname);
		} else if ((ams::GetAMS())->vdown( (ams::GetAMS())->AMS_ErrNo())) {
		    sprintf(ErrorText, "%s: temporarily unavailable (net/server problem)", nickname);
		} else if ((ams::GetAMS())->AMS_ErrNo() == ENOENT) {
		    sprintf(ErrorText, "Sorry; %s no longer exists so you cannot subscribe to it.", nickname);
		} else {
		    sprintf(ErrorText, "Cannot set subscription entry to %s", nickname);
		    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, TRUE, errcode);
		    return;
		}
		message::DisplayString(NULL, 75, ErrorText);
		return;
	    }
	    ams::WaitCursor(FALSE);
	    (this)->AlterSubscriptionStatus( FullName, quicknewstatus, nickname);
	    break;
	case 4:
	    substatus = amsutil::ChooseNewStatus(nickname, substatus, TRUE);
	    ams::WaitCursor(TRUE);
	    if ((errcode = (ams::GetAMS())->MS_SetSubscriptionEntry( FullName, nickname, substatus)) != 0) {
		ams::WaitCursor(FALSE);
		if ((ams::GetAMS())->AMS_ErrNo() == EACCES) {
		    sprintf(ErrorText, "'%s' is private; you don't have read-access or are unauthenticated.", nickname);
		} else if ((ams::GetAMS())->vdown( (ams::GetAMS())->AMS_ErrNo())) {
		    sprintf(ErrorText, "%s: temporarily unavailable (net/server problem)", nickname);
		} else if ((ams::GetAMS())->AMS_ErrNo() == ENOENT) {
		    sprintf(ErrorText, "Sorry; %s no longer exists so you cannot subscribe to it.", nickname);
		} else {
		    sprintf(ErrorText, "Cannot set subscription entry to %s", nickname);
		    (ams::GetAMS())->ReportError( ErrorText, ERR_WARNING, TRUE, errcode);
		    return;
		}
		message::DisplayString(NULL, 75, ErrorText);
		return;
	    }
	    ams::WaitCursor(FALSE);
	    (this)->AlterSubscriptionStatus( FullName, substatus, nickname);
	    break;
	case 5:
	    /* Post on it */
	    sendm = (this)->ExposeSend();
	    if (!sendm) return;
	    (sendm)->ResetFromParameters( nickname, NULL, NULL, NULL, 0);
	    (sendm)->CheckRecipients();
	    break;
	case 6:
	    /* see the messages */
	    ams::WaitCursor(TRUE);
	    if (NowPlaying) { /* Force to consider purge */
		char *s, *t;

		s = (char *)malloc(1+strlen(nickname));
		t = (char *)malloc(1+strlen(FullName));
		strcpy(s, nickname);
		strcpy(t, FullName);
		nickname = s;
		FullName = t;
		/* In this case, nickname and FullName will be
		   freed by the following call */
		(mainci)->ClearAndUpdate( TRUE, TRUE);
		(ams::GetAMS())->CUI_CheckMailboxes( FullName);
	    }
	    (mainci)->InsertUpdatesInDocument( nickname, FullName, FALSE);
	    ams::WaitCursor(FALSE);
	    break;
	case 7:
	    if (HasCurrent) {
		/* file current */
		(mainci)->FileCurrent( FullName, nickname);
		break;
	    }
	    /* drop through */
	case 8:
	    /* file marked */
	    (mainci)->FileMarked( FullName, nickname);
	    break;
	default:
	    (ams::GetAMS())->ReportError( "Unexpected return value from message handler.", ERR_WARNING, FALSE, 0);
	    break;
    }
    return;
}

static void AlterSubStatus(class folders  *ci, char  *dir , int  status, char  *shortname)
{
    int i;
    struct BEDirCache *bdcent = NULL;

    if (!ci->HasSetUp) return /* 0 */;
    for (i=0; i<ci->MainDirCacheCount; ++i) {
	if (!strcmp(dir, ci->MainDirCache[i].FullName)
	    || ((ci->MainDirCache[i].FullName[0] == '\0')
		&& !strcmp(shortname, ci->MainDirCache[i].ShortName))) {
	    bdcent = &ci->MainDirCache[i];
	    break;
	}
    }
    if (!bdcent) return /* 0 */; /* Not in here */
    ((class text *) (ci)->GetDataObject())->AlwaysInsertCharacters( bdcent->pos+1, WhichIcon(status), 1);
    ((class text *) (ci)->GetDataObject())->AlwaysDeleteCharacters( bdcent->pos, 1);
    bdcent->substatus = status;
    (ci)->WantUpdate( ci);
    return /* 0 */;
}

int folders::AlterSubscriptionStatus(char  *dir , int  status, char  *shortname)
{
    char *StatusString, ErrorText[256];

    AlterSubStatus(this, dir, status, shortname);
    switch (status) {
	case AMS_ALWAYSSUBSCRIBED:
	    StatusString = "You are now subscribed to %s.";
	    break;
	case AMS_ASKSUBSCRIBED: 
	    StatusString = "You are now 'Ask' subscribed to %s.";
	    break;
	case AMS_SHOWALLSUBSCRIBED: 
	    StatusString = "You are now 'ShowAll' subscribed to %s.";
	    break;
	case AMS_PRINTSUBSCRIBED: 
	    StatusString = "You are now 'Print' subscribed to %s.";
	    break;
	case AMS_UNSUBSCRIBED:
	default:
	    StatusString = "You are now not subscribed to %s.";
	    break;
    }
    sprintf(ErrorText, StatusString, shortname);
    message::DisplayString(NULL, 10, ErrorText);
    return(0);
}



void folders::AlterFolderNames(char  *name , char  *Nickname, Boolean  DoInsert)
{
    char DumBuf[1+MAXPATHLEN];
    int substatus, whichcache;
    struct BEDirCache *bdcent;
    long errcode;

    if (!this->HasSetUp) return;
    if (DoInsert) {
	if (errcode = (ams::GetAMS())->MS_GetSubscriptionEntry( name, DumBuf, &substatus)) {
	    (ams::GetAMS())->ReportError( "Cannot get subscription entry", ERR_WARNING, TRUE, errcode);
	    substatus = AMS_UNSUBSCRIBED;
	}
	whichcache = AddToBEDirCache(this, name, Nickname, substatus);
	bdcent = &this->MainDirCache[whichcache];
	InsertFolderNameInText(this, bdcent, NULL);
    } else {
	RemoveFromBEDirCache(this, name, Nickname);
    }
    (this)->WantUpdate( this);
}

static void UnhighlightFolderName(class folders  *ci)
{
    if (ci->HasSetUp && ci->HighlightEnv) {
	((class text *) (ci)->GetDataObject())->SetEnvironmentStyle( ci->HighlightEnv, ci->Normalfolderstyle);
    }
}


static void HighlightSpecificFolderName(class folders  *ci, char  *name , char  *CommText, int  i)
{
    char Label[500];
    int insertct = 0, fpos;

    ((class text *) (ci)->GetDataObject())->SetEnvironmentStyle( ci->MainDirCache[i].env, ci->Activefolderstyle);
    ci->HighlightEnv = ci->MainDirCache[i].env;
    BEDC_AddComment(ci, &ci->MainDirCache[i], CommText, &insertct);
    sprintf(Label, "%s %s", ci->MainDirCache[i].ShortName, CommText);
    UpdateBEDirCachePositions(ci, &ci->MainDirCache[i], insertct);
    (ci)->SetDotPosition( ci->MainDirCache[i].pos);
    (ci)->WantUpdate( ci);
    if (i+1 < ci->MainDirCacheCount) {
	fpos = ci->MainDirCache[i+1].pos + 1;
    } else {
	fpos = ci->MainDirCache[i].pos + 1;
    }
    if (!(ci)->Visible( fpos)) {
	(ci)->SetTopPosition( ci->MainDirCache[i].pos);
    }
    (ci)->WantUpdate( ci);
}

static void HighlightFolderName(class folders  *ci, char  *name , char  *CommText)
{
    int i;
    char NickName[1+MAXPATHLEN];

    if (!ci->HasSetUp || !name) return;
    (ams::GetAMS())->CUI_BuildNickName( name, NickName);
    for (i=0; i<ci->MainDirCacheCount; ++i) {
	if (ci->MainDirCache[i].FullName[0] == '\0') {
	    if (!strcmp(NickName, ci->MainDirCache[i].ShortName)) {
		long errcode;
		char DumBuf[100+MAXPATHLEN];

		errcode = (ams::GetAMS())->CUI_DisambiguateDir( NickName, &name);
		if (errcode) {
		    if ((ams::GetAMS())->vdown( (ams::GetAMS())->AMS_ErrNo())) {
			sprintf(DumBuf, "%s: temporarily unavailable (net/server problem)", NickName);
		    } else if ((ams::GetAMS())->AMS_ErrNo() == EACCES) {
			sprintf(DumBuf, "'%s' is private; you don't have read-access or are unauthenticated.", NickName);
		    } else {
			sprintf(DumBuf, "The folder %s is not readable.", NickName);
		    }
		    message::DisplayString(NULL, 75, DumBuf);
		    return;
		}
		ci->MainDirCache[i].FullName = (char *)malloc(1+strlen(name));
		if (!ci->MainDirCache[i].FullName) return;
		strcpy(ci->MainDirCache[i].FullName, name);
	    }
	}
	if (!strcmp(name, ci->MainDirCache[i].FullName)) {
	    if (ci->MainDirCache[i].len == 0) {
		if (ci->MainDirCacheCount == 0) {
		    folders_ClearFolders(ci);
		}
		InsertFolderNameInText(ci, &ci->MainDirCache[i], NULL);
		UpdateBEDirCachePositions(ci, &ci->MainDirCache[i], ci->MainDirCache[i].len);
	    }
	    HighlightSpecificFolderName(ci, name, CommText, i);
	    return;
	}
    }
    /* Did not find it, have to ADD it */
    (ci)->AlterFolderNames( name, NickName, TRUE);
    /* I cannot believe my good fortune in having i already set right...  :-) */
    HighlightSpecificFolderName(ci, name, CommText, i);
}

void folders::HighlightFolder(char  *name , char  *CommText)
{
    if (this->CurrentConfiguration != LIST_ALL_FOLDERS) {	
	ams::PlanFolderPrefetch(this);
    }
    UnhighlightFolderName(this);
    HighlightFolderName(this, name, CommText);
}


void folders::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle Rect;
    static int narrowlim = -1;

    (this)->GetLogicalBounds( &Rect);
    if (narrowlim < 0) {
	narrowlim = environ::GetProfileInt("VeryNarrowFolders", 300);
    }
    (this)->messages::FullUpdate( type, left, top, width, height);
    if (!this->mycursor) folders_CreateFoldersCursor(this);
    (this)->PostCursor( &Rect, this->mycursor);
    (this->puntlp)->GetLogicalBounds( &Rect);
    if (Rect.width < narrowlim) {
	if (!this->VeryNarrow) (this)->SetVeryNarrow( TRUE);
    } else {
	if (this->VeryNarrow) (this)->SetVeryNarrow( FALSE);
    }
}


void folders::FindNextFolder(int  *which , int  *skipped)
{
    int pos, curr;

    *skipped = 0;
    if (this->HighlightEnv) {
	pos = (this->HighlightEnv)->Eval();
    } else {
	pos = (this)->GetDotPosition();
    }
    for (*which=0; *which<this->MainDirCacheCount && this->MainDirCache[*which].pos <= pos; ++*which) {
	;
    }
    curr = *which - 1;
    while (*which<this->MainDirCacheCount && this->MainDirCache[*which].SkipMe) {
	++*skipped;
	++*which;
    }
    if (*which >= this->MainDirCacheCount) {
	/* Wrap around to beginning and try once more before declaring "nothing new" */
	*which = 0;
	while (*which<curr && this->MainDirCache[*which].SkipMe) {
	    ++*skipped;
	    ++*which;
	}
	if (*which >= curr) *which = this->MainDirCacheCount;
    }
}

void folders::ReadMail(Boolean  CheckMailbox)
{
    char ShortName[1+MAXPATHLEN], *FullName, *fm;

    if ((ams::GetAMS())->CUI_DisambiguateDir( "mail", &FullName)
	 && (ams::GetAMS())->CUI_DisambiguateDir( "misc", &FullName)) {
	fm = ams::GetFirstMailClass();
	if (!fm) {
	    long mserrcode = (ams::GetAMS())->CUI_DisambiguateDir( fm, &FullName);
	    if (mserrcode) {
		(ams::GetAMS())->ReportError( "I don't know where your mail goes; sorry.", ERR_WARNING, TRUE, mserrcode);
		return;
	    }
	}
    }
    (ams::GetAMS())->CUI_BuildNickName( FullName, ShortName);
    if (CheckMailbox) {
	ams::WaitCursor(TRUE);
	(ams::GetAMS())->CUI_CheckMailboxes( FullName);
	ams::WaitCursor(FALSE);
    }
    ((this)->GetCaptions())->InsertUpdatesInDocument( ShortName, FullName, FALSE);
    return;
}

void folders::NextFolder(Boolean  ShowFirst)
{
    int which, skipped;

    (this)->FindNextFolder( &which, &skipped);
    if (which < this->MainDirCacheCount) {
	/* display this one */
	((this)->GetCaptions())->InsertUpdatesInDocument( this->MainDirCache[which].ShortName, this->MainDirCache[which].FullName, ShowFirst);
    } else {
	if (skipped) {
	    message::DisplayString(NULL, 10, "This is the last changed folder on the list.");
	} else {
	    message::DisplayString(NULL, 10, "This is the last folder on the list.");
	}
    }
}

void folders::SetCaptions(class captions  *c)
{
    this->mycaps = c;
}

void folders::SetSkip(char  *fname, boolean  doskip)
{
    int i;

    for (i=0; i<this->MainDirCacheCount; ++i) {
	if (!strcmp(fname, this->MainDirCache[i].FullName)) {
	    this->MainDirCache[i].SkipMe = doskip;
	    break;
	}
    }
}

class sendmessage *folders::ExposeSend()
{

    if (this->sm == NULL) {
	this->sm = new sendmessage;
	(this->sm)->SetFoldersView( this);
	
	this->sm->myframe = ams::InstallInNewWindow(this->sm, "messages-send", "Message Composition", environ::GetProfileInt("sendmessage.width", -1), environ::GetProfileInt("sendmessage.height", -1), this->sm);
	if (this->sm->myframe == NULL) {
	    (this->sm)->Destroy();
	    this->sm = NULL;
	    return(NULL);
	}
	(this->sm)->Reset();
    }
    folders_Expose((this->sm)->GetIM());
    if (amsutil::GetOptBit(EXP_WARPWINDOW)) {
	folders_Warp((this->sm)->GetIM());
    }
    folders_ForceUpdate();
    (this->sm)->WantInputFocus( this->sm);
    return(this->sm);
}

void folders::GrowWindow() 
{
    struct rectangle Rect;
    class im *im;

    (this)->GetLogicalBounds( &Rect);
    im = (this)->GetIM();
    if (im && amsutil::GetOptBit(EXP_GROWFOLDS) && (Rect.height > 0 && Rect.width > 0)) {
	if (amsutil::GetOptBit(EXP_VANISH)) {
	    folders_Vanish(im);
	} else {
	    folders_Hide(im);
	}
	folders_ForceUpdate();
	folders_Expose(im);
    }
}

void folders::FolderOnDisplay(char  *nick , char  *full)
{
    if (nick) {
	if ((this)->GetCaptions()->ShortName) {
	    strcpy(nick, (this)->GetCaptions()->ShortName);
	} else {
	    *nick = '\0';
	}
    }
    if (full) {
	if ((this)->GetCaptions()->FullName) {
	    strcpy(full, (this)->GetCaptions()->FullName);
	} else {
	    *full = '\0';
	}
    }
}

void folders::SetVeryNarrow(boolean  vn)
{
    this->VeryNarrow = vn;
    if (this->buttons) {
	boolean WantsPunt = amsutil::GetOptBit(EXP_PUNTBUTT);
	boolean WantsCommit = environ::GetProfileSwitch("messages.TurnOffCheckpointingAndIUnderstandTheDangersForMessages", 0) || environ::GetProfileSwitch("messages.CommitButton", 0);
	int buttpct;
	int ind=0;
	class sbutton *bs=(this->buttons)->ButtonData();

	if(WantsCommit || WantsPunt) {
	    buttpct = vn ? 30 : 100;
	} else buttpct=0;

	
    /* sorry about the hack (see DoButton), don't change the names of the buttons! */
	if(WantsPunt) {
	    (bs)->SetLabel( 0, "Punt!");
	    (bs)->SetRock( 0, (long)this);
	}
	if(WantsCommit) {
	    (bs)->SetLabel( WantsPunt?1:0, "Commit");
	    (bs)->SetRock( WantsPunt?1:0, (long)this);
	}

	if (vn) {
	    (this->puntlp)->VTFixed( this->buttons, (class view *)this->myscroll, buttpct, 0);
	} else {
	    (this->puntlp)->HFixed( (class view *)this->myscroll, this->buttons, buttpct, 0);
	}
    }
}

static void DoButton(class sbutton  *b, enum view_MouseAction  action, int  ind, class folders  *self)
{
    /* sorry about the hack, don't change the names of the buttons! */

    switch(*(b)->GetLabel( ind)) {
	case 'P':
	    if (action == view_RightUp) {
		((self)->GetCaptions())->PuntCurrent( FALSE);
	    } else if (action == view_LeftUp) {
		((self)->GetCaptions())->PuntCurrent( TRUE);
            }
            break;
	case 'C':
	    if (action == view_LeftUp || action == view_RightUp) {
		ams::CommitState(FALSE, FALSE, TRUE, FALSE);
            }
            break;
	default: ;
    }
}

static struct sbutton_list blist[]={
    {"Punt!", 0, NULL, FALSE},
    {NULL, 0, NULL, FALSE}
};

class view *folders::GetApplicationLayer()
{
    class sbutton *bs;
    this->myscroll = (class scroll *) (this)->messages::GetApplicationLayer();

    if(!this->myscroll) return NULL;

    this->puntlp=new lpair;
    if(!this->puntlp) return NULL;
    
    this->prefs = sbutton::GetNewPrefs("folderoptions");
    if(this->prefs) {
	if(sbutton::GetFont(this->prefs)==NULL) sbutton::GetFont(this->prefs)=fontdesc::Create("andy", fontdesc_Bold, 12);
	sbutton::InitPrefs(this->prefs, "folderoptions");
	this->buttons = sbuttonv::CreateFilledSButtonv("nbutterv", this->prefs, blist);
	if(!this->buttons) return FALSE;
	bs=(this->buttons)->ButtonData();
	(bs)->GetHitFunc()=(sbutton_hitfptr)DoButton;
	(this->buttons)->vborder=environ::GetProfileInt("folderoptionsborder", 0);
	(this->buttons)->hborder=(this->buttons)->GetVBorder();
	(this->buttons)->vspacing=environ::GetProfileInt("folderoptionspadding", 0);
	(this->buttons)->hspacing=(this->buttons)->GetVSpacing();
	
	(bs)->GetMattePrefs()=sbutton::DuplicatePrefs(this->prefs, "folderoptionsmatte");
	if((bs)->GetMattePrefs()) {
	    sbutton::GetStyle((bs)->GetMattePrefs())=0;
	    sbutton::InitPrefs((bs)->GetMattePrefs(), "folderoptionsmatte");
	}
	
    } else return NULL;
    (this)->SetVeryNarrow( this->VeryNarrow); /* Forces lpair to be set right */
    return (class view	*) this->puntlp;
}

void folders::DeleteApplicationLayer(class view *ignored)
{
    
    if (this->puntlp) (this->puntlp)->Destroy();
    if(this->prefs) sbutton::FreePrefs(this->prefs);
    if(this->buttons) {
	if((this->buttons)->ButtonData()) ((this->buttons)->ButtonData())->Destroy();
	(this->buttons)->Destroy();
    }
    (this)->messages::DeleteApplicationLayer( (class view *)this->myscroll);
    this->prefs=NULL;
    this->buttons=NULL;
    this->myscroll = NULL;
    this->puntlp = NULL;
}

class captions *
folders::NewCaptionsInNewWindow()
{
    class captions *cap = new captions;

    (this)->SetCaptions( cap);
    (cap)->SetFolders( this);
    cap->myframe = ams::InstallInNewWindow((cap)->GetApplicationLayer(), "messages-captions", "Message Captions", environ::GetProfileInt("captions.width", 600), environ::GetProfileInt("captions.height", 250), cap);
    return(cap);
}

void folders::SetSendmessage(class sendmessage  *sm)
{
    this->sm = sm;
}

class captions *folders::GetCaptions()
{
    if (!this->mycaps) {
	(this)->NewCaptionsInNewWindow();
    }
    return(this->mycaps);
}

void folders_ExposeCap(class folders  *self)
{
#ifdef NOWAYJOSE
    class im *myim;

    myim = ((self)->GetCaptions())->GetIM();
    if (myim) {
	folders_ExposeWindow(myim);
    }
#endif /* NOWAYJOSE */
}

static struct bind_Description folders_standardbindings [] = {
    /* procname, keysequence, key rock, menu string, menu rock, proc, docstring, dynamic autoload */
    {"folders-left-click-here", "!", TRUE, NULL, 0, 0, (proctable_fptr)folders_SimulateLeftClick, "Display what I am pointing at"},
    {"folders-right-click-here", "@", FALSE, NULL, 0, 0, (proctable_fptr)folders_wrap_SimulateClick, "Simulate right click on what I am pointing at"},
    {"folder-down-focus", "\030n", 0, NULL, 0, 0, (proctable_fptr)folders_DownFocus, "Move input focus to captions"},
    {"folders-up-focus", "\030p", 0, NULL, 0, 0, (proctable_fptr)folders_UpFocus, "Move input focus to bodies or sendmessage"},
    {"folder-down-focus", "\030\016", 0, NULL, 0, 0, (proctable_fptr)folders_DownFocus, "Move input focus to captions"},
    {"folders-up-focus", "\030\020", 0, NULL, 0, 0, (proctable_fptr)folders_UpFocus, "Move input focus to bodies or sendmessage"},
    {"textview-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)folders_TextviewCompound, "Execute a compound textview operation"},
    {"folders-compound-operation", NULL, 0, NULL, 0, 0, (proctable_fptr)folders_FoldersCompound, "Execute a compound folders operation"},
    {"folders-textview-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)FoldersTextviewCommand, "Execute a compound 'textview' operation on the folders"},
    {"folders-sendmessage-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)FoldersSendmessageCommand, "Execute a compound 'sendmessage' operation."},
    {"folders-captions-compound", NULL, 0, NULL, 0, 0, (proctable_fptr)FoldersCaptionsCommand, "Execute a compound 'captions' operation."},
    {NULL, "\033~", 0, NULL, 0, 0, NULL, NULL, NULL}, /* Preserve read onliness */
    {NULL, NULL, 0, NULL, 0, 0, NULL, NULL, NULL}
};

static void InitKeyMenusStyles(class folders  *folders)
{
    char *fontname;
    int fontsize;

    folders->mykeys = keystate::Create(folders, folders_standardkeymap);
    folders->mymenulist = (folders_standardmenulist)->DuplicateML( folders);

    folders->BigCenterStyle = new style;
    (folders->BigCenterStyle)->SetFontSize( style_ConstantFontSize, 22);
    (folders->BigCenterStyle)->SetJustification( style_Centered);

    folders->CenterStyle = new style;
    (folders->CenterStyle)->SetJustification( style_Centered);
    (folders->CenterStyle)->AddNewFontFace( (long) fontdesc_Italic);

    folders->SmallCenterStyle = new style;
    (folders->SmallCenterStyle)->SetJustification( style_Centered);
    (folders->SmallCenterStyle)->SetFontSize( style_ConstantFontSize, 8);

    folders->BoldStyle = new style;
    (folders->BoldStyle)->SetName( "Bold");
    (folders->BoldStyle)->AddNewFontFace( (long) fontdesc_Bold);

    folders->ItalicStyle = new style;
    (folders->ItalicStyle)->SetFontSize( style_PreviousFontSize, 2);
    (folders->ItalicStyle)->AddNewFontFace( (long) fontdesc_Italic);

    folders->IconicStyle = new style;
    (folders->IconicStyle)->SetFontFamily( "msgs");
    (folders->IconicStyle)->SetFontSize( style_ConstantFontSize, 10);

    folders->Activefolderstyle = new style;
    if (amsutil::GetOptBit(EXP_FIXCAPTIONS)) {
	(folders->Activefolderstyle)->AddNewFontFace( (long) fontdesc_Bold | fontdesc_Fixed);
    } else {
	(folders->Activefolderstyle)->AddNewFontFace( (long) fontdesc_Bold);
    }
    (folders->Activefolderstyle)->SetNewLeftMargin( style_ConstantMargin, 20, style_RawDots);
    (folders->Activefolderstyle)->SetNewIndentation( style_ConstantMargin, -10, style_RawDots);

    folders->Normalfolderstyle = new style;

    folders->GlobalCapStyle = new style;
    (folders->GlobalCapStyle)->SetJustification( style_LeftJustified);
    (folders->GlobalCapStyle)->SetNewLeftMargin( style_ConstantMargin, 20, style_RawDots);
    (folders->GlobalCapStyle)->SetNewIndentation( style_ConstantMargin, -10, style_RawDots);

    fontsize = environ::GetProfileInt("messages.fontsize", 12);
    fontname = amsutil::GetDefaultFontName();
    (folders->GlobalCapStyle)->SetFontSize( style_ConstantFontSize, fontsize);
    (folders->Activefolderstyle)->SetFontFamily( fontname);
    (folders->Normalfolderstyle)->SetFontFamily( fontname);
    (folders->GlobalCapStyle)->SetFontFamily( fontname);
    (folders->BoldStyle)->SetFontFamily( fontname);
    (folders->ItalicStyle)->SetFontFamily( fontname);
}

static void OneTimeInitKeyMenus()
{
    folders_standardkeymap = new keymap;
    folders_standardmenulist = new menulist;
    bind::BindList(folders_standardbindings, folders_standardkeymap, folders_standardmenulist, &folders_ATKregistry_ );
}

boolean folders::InitializeClass() 
{
    OneTimeInitKeyMenus();
    return(TRUE);
}

folders::folders()
{
	ATKinit;

    class text *mytext;

    (this)->SetWhatIAm( WHATIAM_FOLDERS);
    InitKeyMenusStyles(this);
    this->prefs=NULL;
    this->buttons=NULL;
    this->NeedsToPrefetch = FALSE;
    this->myframe = NULL;
    this->puntlp=NULL;
    this->buttons = NULL;
    this->myscroll = NULL;
    this->sm = NULL;
    this->mycursor = NULL;
    this->HasSetUp = 0;
    this->CurrentConfiguration = LIST_NEWONLY;
    this->MailOnlyMode = 0;
    this->VeryNarrow = 0;
    /* Initialize the BEDirCache */
    this->MainDirCache = (struct BEDirCache *) malloc(25*sizeof(struct BEDirCache));
    bzero(this->MainDirCache, 25 * sizeof(struct BEDirCache));
    this->MainDirCacheSize = 25;
    this->MainDirCacheCount = 0;

    mytext = new text;
    (mytext)->SetGlobalStyle( this->GlobalCapStyle);
    (mytext)->SetReadOnly( TRUE);
    (this)->SetDataObject( mytext);

    this->HighlightEnv = NULL;
    ams::AddCheckpointFolder(this);
    this->mycaps = NULL;

    (this)->ShowHelp();
    THROWONFAILURE((TRUE));
}

folders::~folders()
{
	ATKinit;

    ams::RemoveCheckpointFolder(this);
    if (this->MainDirCache) {
	free(this->MainDirCache);
	this->MainDirCache = NULL;
    }
    if (this->sm) {
	(this->sm)->SetFoldersView( NULL);
    }
    if (this->mycaps) {
	(this->mycaps)->SetFolders( NULL);
    }
    folders_FinalizeProcStyleStuff(this);
    if (this->puntlp) (this->puntlp)->Destroy();
    if(this->prefs) sbutton::FreePrefs(this->prefs);
    if(this->buttons) {
	if((this->buttons)->ButtonData()) ((this->buttons)->ButtonData())->Destroy();
	(this->buttons)->Destroy();
    }
    (this)->messages::DeleteApplicationLayer( (class view *)this->myscroll);
}
