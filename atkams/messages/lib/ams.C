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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/ams.C,v 1.11 1994/08/15 03:52:52 rr2b Stab74 $";
#endif

/* Until I come up with a better scheme, new functions here have to be added to SIX files -- ams.ch, amss.ch, amsn.ch (all identical specs) and the corresponding c files */ 

#include <sys/param.h>
#include <util.h>
#include <ctype.h>
#include <errprntf.h>

#include <textview.H>

#include <cursor.H>
#include <im.H>
#include <event.H>
#include <frame.H>
#include <view.H>
#include <rect.h>

#include <init.H>
#include <cui.h>
#include <fdphack.h>
#include <ams.H>
#ifdef SNAP_ENV
#include <amss.H>
#endif /* SNAP_ENV */
#include <amsn.H>
#include <sendmessage.H>
#include <message.H>
#include <amsutil.H>
#include <t822view.H>
#include <captions.H>
#include <folders.H>
#include <environ.H>
static int IWantSnap = 0;


ATKdefineRegistry(ams, ATK, NULL);
#ifndef NORCSID
#endif
#ifdef SNAP_ENV
#endif /* SNAP_ENV */
static void ReportMissing(char  *s);
int AddToClassList(char  *TempName , char  *FullName, Boolean  CheckDups);
static void HandleInitProblem(long  rock, char  *err);
static class init *ReadInitFile(char  *fakeprogname , char  *realprogname);
static void DisplayAMS_ERRNO(char  *prefix);
static int UpdateServerState();
static void HandleTimer(void *pdrock, long timerock);
static void RestartTimer(long datarock);
static void TimerReport(int  code);
static FILE *GetCompletionFP(char  *partial);
static void FolderHelp(char  *partial, long  rock, message_workfptr helpfunc, long  helprock);
static enum message_CompletionCode FolderComplete(char  *part , long  dummy , char  *buf, long  size);


void ams::RemoveErrorDialogWindow()
{
}

void ams::SetWantSnap(int  wantsnap)
{
    IWantSnap = wantsnap;
}

static char * MyRock;

void ams::SetCUIRock(long  r)
{
    MyRock = (char *)r;
}

static class ams *myamsp=NULL;

class ams *ams::GetAMS()
{
    if(myamsp==NULL) {
	myamsp=ams::MakeAMS();
	return myamsp;
    }
    return myamsp;
}

class ams *ams::MakeAMS() {
    if (!myamsp) {
	message::DisplayString(NULL, 10, "Loading message server libraries...");
	im::ForceUpdate();
#ifdef SNAP_ENV
	if (IWantSnap) {
	    myamsp = (class ams *) new amss;
	} else {
	    myamsp = (class ams *) new amsn;
	}
#else /* SNAP_ENV */
	if (IWantSnap) {
	    fprintf(stderr, "No separate message server available: SNAP_ENV was not set at system build time.\n");
	    exit(-1);
	}
	myamsp = (class ams *) new amsn;
#endif /* SNAP_ENV */
	if (!myamsp || (myamsp)->CUI_Initialize( NULL, MyRock)) {
	    if(myamsp) {
		(myamsp)->ReportError( "Error initializing; program terminated.", ERR_FATAL, FALSE, 0);
	    } else fprintf(stderr, "Error initializing; program terminated.");
	    exit(-1); /* not reached */
	}
	}
    return(myamsp);
}

static void ReportMissing(char  *s)
{
    fprintf(stderr, "The %s function is not included in the skeleton ams class.\n", s);
}

void ams::CUI_BuildNickName(char  *shortname , char  *longname)
{
    ReportMissing("CUI_BuildNickName");
}

int ams::CUI_CheckMailboxes(char  *forwhat) 
{
    ReportMissing("CUI_CheckMailboxes");
    return(0);
}

long ams::CUI_CloneMessage(int  cuid, char  *DirName, int  code) 
{
    ReportMissing("CUI_CloneMessage");
    return(0);
}

long ams::CUI_CreateNewMessageDirectory(char  *dir , char  *bodydir) 
{
    ReportMissing("CUI_CreateNewMessageDirectory");
    return(0);
}

long ams::CUI_DeleteMessage(int  cuid) 
{
    ReportMissing("CUI_DeleteMessage");
    return(0);
}

long ams::CUI_DeliveryType() 
{
    ReportMissing("CUI_DeliveryType");
    return(0);
}

long ams::CUI_DirectoriesToPurge() 
{
    ReportMissing("CUI_DirectoriesToPurge");
    return(0);
}

long ams::CUI_DisambiguateDir(char  *shortname , char  **longname) 
{
    ReportMissing("CUI_DisambiguateDir");
    return(0);
}

long ams::CUI_DoesDirNeedPurging(char  *name) 
{
    ReportMissing("CUI_DoesDirNeedPurging");
    return(0);
}

void ams::CUI_EndConversation()
{
    ReportMissing("CUI_EndConversation");
}

long ams::CUI_GenLocalTmpFileName(char  *name) 
{
    ReportMissing("CUI_GenLocalTmpFileName");
    return(0);
}

long ams::CUI_GenTmpFileName(char  *name) 
{
    ReportMissing("CUI_GenTmpFileName");
    return(0);
}

long ams::CUI_GetFileFromVice(char  *tmp_file , char  *vfile) 
{
    ReportMissing("CUI_GetFileFromVice");
    return(0);
}

long ams::CUI_GetHeaderContents(int  cuid, char  *hdrname, int  hdrnum, char  *hdrbuf, int  lim)
{
    ReportMissing("CUI_GetHeaderContents");
    return(0);
}

long ams::CUI_GetHeaders(char  *dirname , char  *date64 , char  *headbuf, int  lim , int  startbyte , long  *nbytes , long  *status , int  RegisterCuids)
{
    ReportMissing("CUI_GetHeaders");
    return(0);
}

long ams::CUI_GetSnapshotFromCUID(int  cuid, char  *Sbuf) 
{
    ReportMissing("CUI_GetSnapshotFromCUID");
    return(0);
}

long ams::CUI_HandleMissingFolder(char  *dname) 
{
    ReportMissing("CUI_HandleMissingFolder");
    return(0);
}

long ams::CUI_Initialize(int  (*TimerFunction)(char *rockme), char  *rock) 
{
    ReportMissing("CUI_Initialize");
    return(0);
}

long ams::CUI_LastCallFinished() 
{
    ReportMissing("CUI_LastCallFinished");
    return(0);
}

char * ams::CUI_MachineName() 
{
    ReportMissing("CUI_MachineName");
    return(NULL);
}

char * ams::CUI_MailDomain() 
{
    ReportMissing("CUI_MailDomain");
    return(NULL);
}

long ams::CUI_MarkAsRead(int  cuid) 
{
    ReportMissing("CUI_MarkAsRead");
    return(0);
}

long ams::CUI_MarkAsUnseen(int  cuid) 
{
    ReportMissing("CUI_MarkAsUnseen");
    return(0);
}

long ams::CUI_NameReplyFile(int  cuid , int  code, char  *fname) 
{
    ReportMissing("CUI_NameReplyFile");
    return(0);
}

long ams::CUI_OnSameHost() 
{
    ReportMissing("CUI_OnSameHost");
    return(0);
}

long ams::CUI_PrefetchMessage(int  cuid , int  ReallyNext) 
{
    ReportMissing("CUI_PrefetchMessage");
    return(0);
}

long ams::CUI_PrintBodyFromCUIDWithFlags(int  cuid , int  flags, char  *printer )
{
    ReportMissing("CUI_PrintBodyFromCUIDWithFlags");
    return(0);
}

void ams::CUI_PrintUpdates(const char  *dname , const char  *nickname)
{
    ReportMissing("CUI_PrintUpdates");
}

long ams::CUI_ProcessMessageAttributes(int  cuid, char  *snapshot) 
{
    ReportMissing("CUI_ProcessMessageAttributes");
    return(0);
}

long ams::CUI_PurgeDeletions(char  *dirname) 
{
    ReportMissing("CUI_PurgeDeletions");
    return(0);
}

long ams::CUI_PurgeMarkedDirectories(boolean  ask , boolean  OfferQuit) 
{
    ReportMissing("CUI_PurgeMarkedDirectories");
    return(0);
}

long ams::CUI_ReallyGetBodyToLocalFile(int  cuid, char  *fname, int  *ShouldDelete , int  MayFudge) 
{
    ReportMissing("CUI_ReallyGetBodyToLocalFile");
    return(0);
}

long ams::CUI_RemoveDirectory(char  *dirname)
{
    ReportMissing("CUI_RemoveDirectory");
    return(0);
}
long ams::CUI_RenameDir(char  *oldname , char  *newname) 
{
    ReportMissing("CUI_RenameDir");
    return(0);
}

void ams::CUI_ReportAmbig(char  *name , char  *atype)
{
    ReportMissing("CUI_ReportAmbig");
}

long ams::CUI_ResendMessage(int  cuid, char  *tolist) 
{
    ReportMissing("CUI_ResendMessage");
    return(0);
}

long ams::CUI_RewriteHeaderLine(char  *addr , char  **newaddr) 
{
    ReportMissing("CUI_RewriteHeaderLine");
    return(0);
}

long ams::CUI_RewriteHeaderLineInternal(char  *addr , char  **newaddr, int  maxdealiases , int  *numfound , int  *externalcount , int  *formatct , int  *stripct , int  *trustct)
{
    ReportMissing("CUI_RewriteHeaderLineInternal");
    return(0);
}

char *ams::CUI_Rock()
{
    ReportMissing("CUI_Rock");
    return(NULL);
}

void ams::CUI_SetClientVersion(char  *vers)
{
    ReportMissing("CUI_SetClientVersion");
}

long ams::CUI_SetPrinter(char  *printername) 
{
    ReportMissing("CUI_SetPrinter");
    return(0);
}

long ams::CUI_SnapIsRunning() 
{
    ReportMissing("CUI_SnapIsRunning");
    return(0);
}

long ams::CUI_StoreFileToVice(char  *localfile , char  *vicefile) 
{
    ReportMissing("CUI_StoreFileToVice");
    return(0);
}

long ams::CUI_SubmitMessage(char  *infile, long  DeliveryOpts) 
{
    ReportMissing("CUI_SubmitMessage");
    return(0);
}

long ams::CUI_UndeleteMessage(int  cuid) 
{
    ReportMissing("CUI_UndeleteMessage");
    return(0);
}

long ams::CUI_UseAmsDelivery() 
{
    ReportMissing("CUI_UseAmsDelivery");
    return(0);
}

long ams::CUI_UseNameSep() 
{
    ReportMissing("CUI_UseNameSep");
    return(0);
}

char * ams::CUI_VersionString() 
{
    ReportMissing("CUI_VersionString");
    return(NULL);
}

char* ams::CUI_WhoIAm() 
{
    ReportMissing("CUI_WhoIAm");
    return(NULL);
}

int ams::CUI_GetCuid(char  *id , char  *fullname, int  *isdup)
{
    ReportMissing("CUI_GetCuid");
    return(0);
}

long ams::MS_AppendFileToFolder(char  *filename , char  *foldername) 
{
    ReportMissing("MS_AppendFileToFolder");
    return(0);
}

long ams::MS_CheckAuthentication(int  *auth) 
{
    ReportMissing("MS_CheckAuthentication");
    return(0);
}

long ams::MS_DebugMode(int  mslevel , int  snaplevel , int  malloclevel) 
{
    ReportMissing("MS_DebugMode");
    return(0);
}

long ams::MS_DisambiguateFile(char  *source , char  *target, long  MustBeDir) 
{
    ReportMissing("MS_DisambiguateFile");
    return(0);
}

long ams::MS_FastUpdateState() 
{
    ReportMissing("MS_FastUpdateState");
    return(0);
}

long ams::MS_GetDirInfo(char  *dirname, int  *protcode , int  *msgcount) 
{
    ReportMissing("MS_GetDirInfo");
    return(0);
}

long ams::MS_GetNewMessageCount(char  *dirname , int  *numnew , int  *numtotal , char  *lastolddate, int  InsistOnFetch)
{
    ReportMissing("MS_GetNewMessageCount");
    return(0);
}

long ams::MS_GetNthSnapshot(char  *dirname, long  which, char  *snapshotbuf)
{
    ReportMissing("MS_GetNthSnapshot");
    return(0);
}

long ams::MS_GetSearchPathEntry(long  which, char  *buf, long  buflim) 
{
    ReportMissing("MS_GetSearchPathEntry");
    return(0);
}

long ams::MS_GetSubscriptionEntry(char  *fullname, char  *nickname, int  *status)
{
    ReportMissing("MS_GetSubscriptionEntry");
    return(0);
}

long ams::MS_NameChangedMapFile(char  *mapfile, int  mailonly, int  listall, int  *numchanged, int  *numunavailable, int  *nummissing, int  *numslowpokes, int  *numfastfellas)
{
    ReportMissing("MS_NameChangedMapFile");
    return(0);
}

long ams::MS_NameSubscriptionMapFile(char  *root, char  *mapfile) 
{
    ReportMissing("MS_NameSubscriptionMapFile");
    return(0);
}

long ams::MS_MatchFolderName(char  *pat, char  *filename) 
{
    ReportMissing("MS_MatchFolderName");
    return(0);
}

long ams::MS_ParseDate(char  *indate, int  *year, int  *month, int  *day, int  *hour, int  *min, int  *sec, int  *wday, int  *gtm)
{
    ReportMissing("MS_ParseDate");
    return(0);
}

long ams::MS_PrefetchMessage(char  *dirname, char  *id, long  getnext) 
{
    ReportMissing("MS_PrefetchMessage");
    return(0);
}

long ams::MS_SetAssociatedTime(char  *fullname, char  *newvalue) 
{
    ReportMissing("MS_SetAssociatedTime");
    return(0);
}

void ams::MS_SetCleanupZombies(long  doclean)
{
    ReportMissing("MS_SetCleanupZombies");
}

long ams::MS_SetSubscriptionEntry(char  *fullname, char  *nickname, long  status)
{
    ReportMissing("MS_SetSubscriptionEntry");
    return(0);
}

long ams::MS_UnlinkFile(char  *filename) 
{
    ReportMissing("MS_UnlinkFile");
    return(0);
}

long ams::MS_UpdateState()
{
    ReportMissing("MS_UpdateState");
    return(0);
}

long ams::MS_DomainHandlesFormatting(char  *domname, int  *retval)
{
    ReportMissing("MS_DomainHandlesFormatting");
    return(0);
}

void ams::ReportSuccess(char  *s)
{
    ReportMissing("ReportSuccess");
}


void ams::ReportError(char  *s, int  level , int  decode, long  mserrcode)
{
    ReportMissing("ReportError");
}


int ams::GenericCompoundAction(class view  *v, char  *prefix, char  *cmds)
{
    ReportMissing("GenericCompoundAction");
    return(0);
}

int ams::GetBooleanFromUser(char  *prompt, int  defaultans)
{
    ReportMissing("GetBooleanFromUser");
    return(0);
}

int ams::GetStringFromUser(char  *prompt , char  *buf, int  len , int  ispass)
{
    ReportMissing("GetStringFromUser");
    return(0);
}

int ams::TildeResolve(const char  *in , char  *out)
{
    ReportMissing("TildeResolve");
    return(0);
}

int ams::OnlyMail()
{
    ReportMissing("OnlyMail");
    return(0);
}

char *ams::ap_Shorten(char  *fname)
{
    ReportMissing("ap_Shorten");
    return(NULL);
}

int ams::fwriteallchars(char  *s, int  len, FILE  *fp)
{
    ReportMissing("fwriteallchars");
    return(0);
}

long ams::MSErrCode()
{
    ReportMissing("MSErrCode");
    return(0);
}

int ams::vdown(int  Errno)
{
    ReportMissing("vdown");
    return(0);
}

int ams::AMS_ErrNo()
{
    ReportMissing("AMS_ErrNo");
    return(0);
}

void ams::SubtleDialogs(boolean  besubtle)
{
    ReportMissing("SubtleDialogs");
}

char *ams::DescribeProt(int  code)
{
    ReportMissing("DescribeProt");
    return(NULL);
}

int ams::ChooseFromList(char  **QVec, int  defans)
{
    ReportMissing("ChooseFromList");
    return(0);
}
int ams::CUI_GetAMSID(int  cuid, char  **id , char  **dir)
{
    ReportMissing("ChooseFromList");
    return(0);
}

char *ams::MessagesAutoBugAddress()
{
    ReportMissing("MessagesAutoBugAddress");
    return(NULL);
}

int ams::UnScribe(int  ucode, struct ScribeState  **ss, char  *LineBuf, int  ct, FILE  *fout)
{
    ReportMissing("UnScribe");
    return(0);
}

int ams::UnScribeFlush(int  ucode, struct ScribeState  **ss, FILE  *fout)
{
    ReportMissing("UnScribeFlush");
    return(0);
}

int ams::UnScribeInit(char  *vers, struct ScribeState  **ss)
{
    ReportMissing("UnScribeInit");
    return(0);
}

void ams::WriteOutUserEnvironment(FILE  *fp, boolean  IsAboutMessages)
{
    ReportMissing("WriteOutUserEnvironment");
}

int ams::CheckAMSUseridPlusWorks(char  *dom)
{
    ReportMissing("CheckAMSUseridPlusWorks");
    return(0);
}

char *ams::ams_genid(boolean  isfilename)
{
    ReportMissing("ams_genid");
    return(NULL);
}

static int CheckpointFrequency = -1;

/* We keep a list of all the sendmessage, captions, and folders views in creation.
  Currently, however, the captions list is not used. */

static struct smlist {
    class sendmessage *s;
    struct smlist *next;
} *SmlistRoot = NULL;

static struct clist {
    class captions *c;
    struct clist *next;
} *ClistRoot = NULL;

static struct flist {
    class folders *f;
    struct flist *next;
} *FlistRoot = NULL;

static struct blist {
    class t822view *b;
    struct blist *next;
} *BlistRoot = NULL;

void ams::SetCheckpointFrequency(int  n)
{
    CheckpointFrequency = n;
    if (CheckpointFrequency < 0) {
	CheckpointFrequency = 60;
    } else if (CheckpointFrequency > 1000) {
	CheckpointFrequency = 1000;
    }
}

void ams::AddCheckpointCaption(class captions  *c)
{
    struct clist *ctmp;
    ctmp = (struct clist *) malloc(sizeof (struct clist));
    if (ctmp) {
	ctmp->next = ClistRoot;
	ctmp->c = c;
	ClistRoot = ctmp;
    }
}

void ams::AddCheckpointBodies(class t822view  *b)
{
    struct blist *btmp;
    btmp = (struct blist *) malloc(sizeof (struct blist));
    if (btmp) {
	btmp->next = BlistRoot;
	btmp->b = b;
	BlistRoot = btmp;
    }
}

void ams::AddCheckpointFolder(class folders  *f)
{
    struct flist *ctmp;
    ctmp = (struct flist *) malloc(sizeof (struct flist));
    if (ctmp) {
	ctmp->next = FlistRoot;
	ctmp->f = f;
	FlistRoot = ctmp;
    }
}

void ams::AddCheckpointSendmessage(class sendmessage  *sm)
{
    struct smlist *smtmp;
    smtmp = (struct smlist *) malloc(sizeof (struct smlist));
    if (smtmp) {
	smtmp->next = SmlistRoot;
	smtmp->s = sm;
	SmlistRoot = smtmp;
    }
}

void ams::RemoveCheckpointFolder(class folders  *f)
{
    struct flist *ctmp = FlistRoot, *ctmpprev = NULL;
    while (ctmp) {
	if (ctmp->f != f) {
	    ctmpprev = ctmp;
	    ctmp = ctmp->next;
	} else {
	    if (ctmpprev) {
		ctmpprev->next = ctmp->next;
	    } else {
		FlistRoot = ctmp->next;
	    }
	    free(ctmp);
	    ctmp = ctmpprev ? ctmpprev->next : FlistRoot;
	}
    }
}

void ams::RemoveCheckpointBodies(class t822view  *b)
{
    struct blist *btmp = BlistRoot, *btmpprev = NULL;
    while (btmp) {
	if (btmp->b != b) {
	    btmpprev = btmp;
	    btmp = btmp->next;
	} else {
	    if (btmpprev) {
		btmpprev->next = btmp->next;
	    } else {
		BlistRoot = btmp->next;
	    }
	    free(btmp);
	    btmp = btmpprev ? btmpprev->next : BlistRoot;
	}
    }
}

void ams::RemoveCheckpointCaption(class captions  *c)
{
    struct clist *ctmp = ClistRoot, *ctmpprev = NULL;
    while (ctmp) {
	if (ctmp->c != c) {
	    ctmpprev = ctmp;
	    ctmp = ctmp->next;
	} else {
	    if (ctmpprev) {
		ctmpprev->next = ctmp->next;
	    } else {
		ClistRoot = ctmp->next;
	    }
	    free(ctmp);
	    ctmp = ctmpprev ? ctmpprev->next : ClistRoot;
	}
    }
}

void ams::RemoveCheckpointSendmessage(class sendmessage  *sm)
{
    struct smlist *stmp = SmlistRoot, *stmpprev = NULL;
    while (stmp) {
	if (stmp->s != sm) {
	    stmpprev = stmp;
	    stmp = stmp->next;
	} else {
	    if (stmpprev) {
		stmpprev->next = stmp->next;
	    } else {
		SmlistRoot = stmp->next;
	    }
	    free(stmp);
	    stmp = stmpprev ? stmpprev->next : SmlistRoot;
	}
    }
}

void ams::WaitCursor(boolean  IsWait)
{
    static class cursor *waitcursor = NULL;
    if (IsWait) {
	if (!waitcursor) {
	    waitcursor = cursor::Create(im::GetLastUsed());
	    (waitcursor)->SetStandard( Cursor_Wait);
	}
	im::SetProcessCursor(waitcursor);
    } else {
	im::SetProcessCursor(NULL);
    }
}

void ams::SubscriptionChangeHook(char  *name , char  *nick, int  status, class messages  *mess)
{
    struct flist *ctmp;

    for (ctmp = FlistRoot; ctmp; ctmp = ctmp->next) {
	(ctmp->f)->AlterSubscriptionStatus( name, status, nick);
    }
}


static char **ClassList = NULL, **ClassListDirs = NULL;
static int ClassListSize = 0, ClassListCount = 0, LastMenuClass = 0, HasInitializedClassList = 0;
static char *FirstMailClass = NULL;

int ams::GetLastMenuClass()
{
    return(LastMenuClass);
}

int ams::GetClassListCount()
{
    return(ClassListCount);
}

char *ams::GetClassListEntry(int  i)
{
    if (ClassList && i < ClassListCount) {
	return(ClassList[i]);
    } 
    return(NULL);
}

void ams::DirectoryChangeHook(char  *adddir , char  *deldir, class messages *rock)
{
    char Nick[1+MAXPATHLEN], *mp;
    int i, mplen;
    struct flist *ftmp;
    struct clist *ctmp;

    mp = ams::GetMailPath();
    mplen = strlen(mp);
    if (mplen == 0) return;
    if (adddir) {
	(ams::GetAMS())->CUI_BuildNickName( adddir, Nick);
	for (ftmp = FlistRoot; ftmp; ftmp = ftmp->next) {
	    (ftmp->f)->AlterFolderNames( adddir, Nick, TRUE);
	}
	if (!strncmp(mp, adddir, mplen)) {
	    AddToClassList(Nick, adddir, TRUE);
	}
    }
    if (deldir) {
	for (ctmp = ClistRoot; ctmp; ctmp = ctmp->next) {
	    (ctmp->c)->AlterPrimaryFolderName( adddir, deldir);
	}
	(ams::GetAMS())->CUI_BuildNickName( deldir, Nick);
	if (!strncmp(mp, deldir, mplen)) {
	    /* Delete from ClassList array */
	    for (i = 0; i < ClassListCount; ++i) {
		if (!strcmp(Nick, ClassList[i])) {
		    break;
		}
	    }
	    if (i < ClassListCount) {
		free(ClassList[i]);
		while (i+1 < ClassListCount) {
		    ClassList[i] = ClassList[i+1];
		    ++i;
		}
		--ClassListCount;
	    }
	}
	for (ftmp = FlistRoot; ftmp; ftmp = ftmp->next) {
	    (ftmp->f)->AlterFolderNames( deldir, Nick, FALSE);
	}
    }
    ams::ResetClassList();
}

char *ams::GetFirstMailClass()
{
    return(FirstMailClass);
}

int ams::InitializeClassList()
{
    char TempName[1+MAXPATHLEN], LocalName[1+MAXPATHLEN], *s, *t, *t2;
    int MaxClassMenu;
    FILE *fp;

    if (HasInitializedClassList) return(0);
    ClassListSize = 20;
    strcpy(TempName, ams::GetMailPath());
    if (TempName[0] == '\0') return(-1);
    strcat(TempName, "/");
    strcat(TempName, AMS_SUBSCRIPTIONMAPFILE);
    FirstMailClass = NULL;

    (ams::GetAMS())->CUI_GenTmpFileName( LocalName);
    if ((ams::GetAMS())->CUI_GetFileFromVice( LocalName, TempName)) {
	return(-1);
    }
    fp = fopen(LocalName, "r");
    if (!fp) {
	unlink(LocalName);
	return(-1);
    }
    ClassList = (char **) malloc(ClassListSize * sizeof(char *));
    if (!ClassList) {
	unlink(LocalName);
	fclose(fp);
	return(-1);
    }
    ClassListDirs = (char **) malloc(ClassListSize * sizeof(char *));
    if (!ClassListDirs) {
	unlink(LocalName);
	fclose(fp);
	return(-1);
    }
    MaxClassMenu = environ::GetProfileInt("messages.maxclassmenu", 8);
    s = (char *) environ::GetProfile("messages.crucialclasses");
    if (s) {
	t = (char *)malloc(1+strlen(s));
	strcpy(t, s);
	s = t; /* permanent copy */
    }
    while (s && *s) {
	t = strchr(s, ',');
	if (t) *t++ = '\0';
	t2 = strchr(s, ':');
	if (t2) *t2++ = '\0';
	if (AddToClassList(s, t2, FALSE)) {
	    unlink(LocalName);
	    fclose(fp);
	    return(-1);
	}
	s = t;
    }
    LastMenuClass = ClassListCount;
    while (fgets(TempName, MAXPATHLEN, fp)) {
	s = strchr(TempName, ':');
	if (s) *s++ = '\0';
	if (!FirstMailClass) {
	    FirstMailClass = (char *)malloc(1+strlen(TempName));
	    if (!FirstMailClass) {
		unlink(LocalName);
		fclose(fp);
		return(-1);
	    }
	    strcpy(FirstMailClass, TempName);
	}
	if (AddToClassList(TempName, s, TRUE)) {
	    unlink(LocalName);
	    fclose(fp);
	    return(-1);
	}
    }
    unlink(LocalName);
    fclose(fp);
    LastMenuClass = ClassListCount;
    if (LastMenuClass > MaxClassMenu) LastMenuClass = MaxClassMenu;
    HasInitializedClassList = 1;
    return(0);
}


int AddToClassList(char  *TempName , char  *FullName, Boolean  CheckDups)
{
    int i;
    char *MyName;

    MyName = amsutil::StripWhiteEnds(TempName);
    if (CheckDups) {
	for (i = 0; i<LastMenuClass; ++i) {
	    if (!strcmp(MyName, ClassList[i])) {
		return(0);
	    }
	}
    }
    ClassList[ClassListCount] = (char *)malloc(1+strlen(MyName));
    if (!ClassList[ClassListCount]) {
	while (--ClassListCount >= 0) {
	    free(ClassList[ClassListCount]);
	    free(ClassListDirs[ClassListCount]);
	}
	free(ClassList);
	ClassList = NULL;
	return(-1);
    }
    if (FullName) {
	amsutil::StripWhiteEnds(FullName);
	ClassListDirs[ClassListCount] = (char *)malloc(1+strlen(FullName));
	if (!ClassListDirs[ClassListCount]) {
	    while (--ClassListCount >= 0) {
		free(ClassList[ClassListCount]);
		free(ClassListDirs[ClassListCount]);
	    }
	    free(ClassListDirs);
	    ClassListDirs = NULL;
	    return(-1);
	}
	strcpy(ClassListDirs[ClassListCount], FullName);
    } else {
	ClassListDirs[ClassListCount] = NULL;
    }
    strcpy(ClassList[ClassListCount], MyName);
    if (++ClassListCount>= ClassListSize) {
	ClassListSize += 20;
	ClassList = (char **) realloc(ClassList, ClassListSize * sizeof(char *));
	if (!ClassList) {
	    return(-1);
	}
	ClassListDirs = (char **) realloc(ClassListDirs, ClassListSize * sizeof(char *));
	if (!ClassListDirs) {
	    return(-1);
	}
    }
    return(0);
}

char *ams::GetMailPath()
{
    static char MailPath[1+MAXPATHLEN];
    static int hassetup = 0;
    long code;

    if (!hassetup) {
	code = (ams::GetAMS())->MS_GetSearchPathEntry( AMS_MAILPATH, MailPath, MAXPATHLEN);
	if (code) {
	    (ams::GetAMS())->ReportError( "Cannot get the name of your mail directory from the message server.", ERR_WARNING, TRUE, code);
	    MailPath[0] = '\0';
	} else {
	    hassetup = 1;
	}
    }
    return(MailPath);
}

void ams::ResetClassList()
{
    struct clist *ctmp;
    struct flist *ftmp;
    struct blist *btmp;

    while (--ClassListCount >= 0) {
	free(ClassList[ClassListCount]);
	if (ClassListDirs[ClassListCount]) {
	    free(ClassListDirs[ClassListCount]);
	}
    }
    if (ClassList) free(ClassList);
    if (ClassListDirs) free(ClassListDirs);
    if (FirstMailClass) free(FirstMailClass);
    ClassList = NULL;
    ClassListDirs = NULL;
    FirstMailClass = NULL;
    ClassListCount = 0;
    ClassListSize = 0;
    LastMenuClass = 0;
    HasInitializedClassList = 0;

    ams::InitializeClassList();
    for (ctmp = ClistRoot; ctmp; ctmp = ctmp->next) {
	(ctmp->c)->ResetFileIntoMenus();
    }
    for (ftmp = FlistRoot; ftmp; ftmp = ftmp->next) {
	(ftmp->f)->ResetFileIntoMenus();
    }
    for (btmp = BlistRoot; btmp; btmp = btmp->next) {
	(btmp->b)->ResetFileIntoMenus();
    }
}

static struct DelayedUpdate {
    char *name;
    char date[AMS_DATESIZE+1];
    struct DelayedUpdate *next;
} *DelayedUpdates = NULL;

int ams::TryDelayedUpdates()
{
    struct DelayedUpdate *dutmp;
    long mcode;

    while (DelayedUpdates) {
	mcode = (ams::GetAMS())->MS_SetAssociatedTime( DelayedUpdates->name, DelayedUpdates->date);
	if (mcode) return 1; /* Do not report these repeated errors */
	free(DelayedUpdates->name);
	dutmp = DelayedUpdates->next;
	free(DelayedUpdates);
	DelayedUpdates = dutmp;
    }
    return 0;
}

static char *NoCacheMem = "Out of memory; cannot save your unremembered profile information";

void ams::CacheDelayedUpdate(char  *FullName , char  *UpdateDate)
{
    struct DelayedUpdate *dutmp;

    dutmp = (struct DelayedUpdate *) malloc(sizeof(struct DelayedUpdate));
    if (!dutmp) {
	(ams::GetAMS())->ReportError( NoCacheMem, ERR_CRITICAL, FALSE, 0);
	return;
    }
    dutmp->name = (char *)malloc(1+strlen(FullName));
    if (!dutmp->name) {
	(ams::GetAMS())->ReportError( NoCacheMem, ERR_CRITICAL, FALSE, 0);
	free(dutmp);
	return;
    }
    strcpy(dutmp->name, FullName);
    strncpy(dutmp->date, UpdateDate, AMS_DATESIZE);
    dutmp->next = DelayedUpdates;
    DelayedUpdates = dutmp;
}

static void HandleInitProblem(long  rock, char  *err)
{
    fprintf(stderr,"%s\n",err);
}

static class init *ReadInitFile(char  *fakeprogname , char  *realprogname)
{
    char buffer[256], buffer1[256];
    const char *andrewDir, *sitename;
    class init *initp;
    boolean HadGlobalNameInit = FALSE;
    boolean siteinit = FALSE, sitegloinit = FALSE;
    const char *home;
    class init *oldinit=im::GetGlobalInit();
    
    if(!fakeprogname) return NULL;

    if(!strcmp(fakeprogname, realprogname)) return NULL;

    initp = new init;
    im::SetGlobalInit(initp);	/* tell im about it.    -wjh
			The init can then be modified by called procs.
			(retract below if no inits found)  */

    home = environ::Get("HOME");
    andrewDir = environ::AndrewDir(NULL);

    if((sitename = environ::GetConfiguration("SiteConfigName")) != NULL){
	sprintf(buffer, "%s/lib/%s.atkinit", andrewDir,sitename);
	sprintf(buffer1, "%s/lib/%s.%sinit", andrewDir,sitename, fakeprogname);
	if(access(buffer1, R_OK)>=0) {
	    sitegloinit = (initp)->Load( buffer, HandleInitProblem, (long) NULL,FALSE) >= 0;
	    siteinit = (initp)->Load( buffer1, HandleInitProblem, (long) NULL, FALSE) >= 0;
	}
    }
    
    /* try for .NAMEinit and quit if succeed */
    if (home != NULL)  {
	sprintf(buffer1, "%s/.%sinit", home, fakeprogname);
	if(access(buffer1, R_OK)>=0) {
	    if(sitename && !sitegloinit) {
		sitegloinit = (initp)->Load( buffer, HandleInitProblem, (long) NULL,FALSE) >= 0;
	    }

	    if (((initp)->Load( buffer1, HandleInitProblem, (long) NULL, FALSE) >= 0)) 
		goto done;
	}
    }
    
    /* try for andrew/lib/global.NAMEinit and continue even if succeed */
    sprintf(buffer1, "%s/lib/global.%sinit", andrewDir, fakeprogname);
    if(access(buffer1, R_OK)>=0) {

	if(sitename && !sitegloinit) {
	    sitegloinit = (initp)->Load( buffer, HandleInitProblem, (long) NULL,FALSE) >= 0;
	}

	HadGlobalNameInit = (initp)->Load( buffer1, HandleInitProblem, (long) NULL, FALSE) >= 0;
    }

	
    /* try for ~/.atkinit or ~/.be2init  extending
	global.NAMEinit quit if succeed */
    if(HadGlobalNameInit) {
	if (home != NULL) {
	    sprintf(buffer, "%s/.atkinit", home);
	    if((initp)->Load( buffer, HandleInitProblem, (long) NULL, FALSE) >= 0) 
		goto done;
	    sprintf(buffer, "%s/.be2init", home);
	    if ((initp)->Load( buffer, HandleInitProblem, (long) NULL, FALSE) >= 0 ) 
		goto done;
	}
    } else {
	/* If we failed to find initialization, discard the data structure we might have used */
	im::SetGlobalInit(oldinit);
	delete initp;
	return NULL;
    }
done:
    im::SetGlobalInit(oldinit);
    return initp;
}

class frame *ams::InstallInNewWindow(class view  *v, char  *programname , char  *windowname, int  w , int  h, class view  *focusv)
{
    class frame *myframe;
    class im *myim;
    char geompref[500];
    const char *geomspec;
    class init *init;
    char *realprogname, *pname=im::GetProgramName();

      
    if((myframe = new frame) == NULL) {
	fprintf(stderr,"Could not allocate enough memory.\n");
	return(NULL);
    }
    if (w >= 0 && h >= 0) {
	im::SetPreferedDimensions(0, 0, w, h);
    }
    
    realprogname=(char *)malloc(strlen(pname)+1);
    if(realprogname) strcpy(realprogname, pname);
    
    im::SetProgramName(programname);
    strcpy(geompref, programname);
    strcat(geompref, ".geometry");
    geomspec = environ::GetProfile(geompref);
    if (geomspec) im::SetGeometrySpec(geomspec);
    myim = im::Create(NULL);
    if (!myim) {
	fprintf(stderr, "Could not create new window.\n");
	if(myframe) (myframe)->Destroy();
	return(NULL);
    }
    init=ReadInitFile(programname, realprogname);
    if(init) {
	delete myim->init;
	myim->init=init;
    }
    im::SetProgramName(realprogname);
    free(realprogname);
    
    (myframe)->SetView( v);
    (myim)->SetView( myframe);
    (myframe)->SetTitle( windowname);
    (myframe)->PostDefaultHandler( "message", (myframe)->WantHandler( "message"));
    im::Interact(FALSE); /* force the redisplay */
    (focusv)->WantInputFocus( focusv);
    (focusv)->WantUpdate( focusv);
    (v)->WantUpdate( v);
    return(myframe);
}

void ams::Focus(class view  *v)
{
    class im *im;

    im = (v)->GetIM();
    if (im) {
#ifdef NOWAYJOSE
        (im)->ExposeWindow();
#endif /* NOWAYJOSE */
	(im)->SetWMFocus();
	im::SetLastUsed(im);
    }
    (v)->WantInputFocus( v);
}

static class event *NextCKP = NULL;

void ams::TimerInit()
{
    RestartTimer(0);
}

static void DisplayAMS_ERRNO(char  *prefix)
{
    long myerrno;
    char ErrorText[1000];
    char *msg2;

    myerrno = (ams::GetAMS())->AMS_ErrNo();
    switch (myerrno) {
#ifdef AFS_ENV
#ifdef EDQUOT
	case EDQUOT:
	    msg2 = "Volume over quota";
	    break;
#endif /* EDQUOT */
	case EACCES:
	    msg2 = "Permission denied; maybe authentication failure";
	    break;
#endif /* AFS_ENV */
	default:
	    if ((ams::GetAMS())->vdown( myerrno)) {
		msg2 = "Network/server failure";
	    } else {
		msg2 = (char *) UnixError(myerrno);
	    }
    }
    sprintf(ErrorText, "%s: %s.", prefix, msg2);
    message::DisplayString(NULL, 10, ErrorText);
}

static int UpdateServerState() {
    static int updatect = 0; /* a hack to make sure we close everything eventually */

    message::DisplayString(NULL, 10, "Checkpointing message server state...");
    im::ForceUpdate();
    if (++updatect > 20) { /* every 20 minutes or so, do it the long way */
	updatect = 0;
	if ((ams::GetAMS())->MS_UpdateState()) {
	    DisplayAMS_ERRNO("Checkpoint failed");
	    return(-1);
	}
    } else if ((ams::GetAMS())->MS_FastUpdateState()) {
	DisplayAMS_ERRNO("Checkpoint failed");
	return(-1);
    }
    message::DisplayString(NULL, 10, "Checkpointing message server state... done.");
    return(0);
}

static struct im_InteractionEvent *MyInteractionEvent = NULL;

static void HandleTimer(void *pdrock, long timerock)
{
    struct flist *ctmp;
    struct smlist *smtmp;
    boolean DidSomething = FALSE;
    static int CheckpointCounter = 0;
    static boolean DoCheckpoints = TRUE, DoPrefetch = TRUE;

    ams::WaitCursor(TRUE);
    if (CheckpointFrequency < 0) { /* First time */
	ams::SetCheckpointFrequency(environ::GetProfileInt("messages.checkpointfrequency", 30));
	DoCheckpoints = ! environ::GetProfileSwitch("messages.TurnOffCheckpointingAndIUnderstandTheDangersForMessages", 0);
	DoPrefetch = environ::GetProfileSwitch("messages.doprefetch", 1);
    }
    /* Make sure there is an absolute cap on intervals between ms checkpointing.    */
    if (DoCheckpoints && ++CheckpointCounter >= 5) {
	TimerReport(2);
	UpdateServerState(); /* errors irrelevant here */
	DidSomething = TRUE; /* Definitely want to wake up again soon */
	CheckpointCounter = 0;
    }
    if (DoPrefetch && !DidSomething) {
	for (ctmp = FlistRoot; ctmp; ctmp = ctmp->next) {
	    if (ctmp->f->NeedsToPrefetch) {
		TimerReport(0);
		(ctmp->f)->HandleAsyncPrefetch();
		DidSomething = TRUE;
		break;
	    }
	}
    }
    if (DoCheckpoints && !DidSomething) {
	for (smtmp = SmlistRoot; smtmp; smtmp = smtmp->next) {
	    if ((smtmp->s)->NeedsCheckpointing()) {
		TimerReport(1);
		(smtmp->s)->Checkpoint();
		DidSomething = TRUE;
		break;
	    }
	}
    }
    if (DoCheckpoints && !DidSomething) {
	TimerReport(2);
	if (UpdateServerState()) {
	    DidSomething = TRUE; /* An error occurred -- try again sooner this way */
	}
	CheckpointCounter = 0;
    }
    if (DidSomething) {
	RestartTimer(0);
	MyInteractionEvent = 0;
    } else {
	NextCKP = NULL;
	MyInteractionEvent = (im::GetLastUsed())->SetInteractionEvent((im_interactionfptr)RestartTimer, 0, 0);
	TimerReport(4);
    }
    ams::WaitCursor(FALSE);
    im::ForceUpdate();
}

static void RestartTimer(long datarock) {
    int freq = (CheckpointFrequency > 0) ? CheckpointFrequency : 60;

    NextCKP = im::EnqueueEvent((event_fptr)HandleTimer, 0, event_SECtoTU(freq));
    TimerReport(3);
}

void ams::PlanFolderPrefetch(class folders  *f)
{
    f->NeedsToPrefetch = TRUE;
    if (NextCKP) {
	(NextCKP)->Cancel();
    } else if (MyInteractionEvent) {
	(im::GetLastUsed())->CancelInteractionEvent( MyInteractionEvent);
	MyInteractionEvent = NULL;
    }
    NextCKP = im::EnqueueEvent((event_fptr)HandleTimer, 0, event_SECtoTU(5));
    TimerReport(5);
}

static void TimerReport(int  code)
{
    static int ReportTimers = -1;

    if (ReportTimers < 0) {
	ReportTimers = environ::GetProfileSwitch("reporttimers", 0) ? 1 : 0;
    }
    if (ReportTimers) {
	time_t clock = time(0);

	switch(code) {
	    case 0:
		printf("AMS: prefetching");
		break;
	    case 1:
		printf("AMS: checkpointing sendmessage");
		break;
	    case 2:
		printf("AMS: checkpointing messageserver");
		break;
	    case 3:
		printf("AMS: re-enqueueing for %d seconds", CheckpointFrequency);
		break;
	    case 4:
		printf("AMS: waiting for next interaction event");
		break;
	    case 5:
		printf("AMS: Prefetch expected; rescheduling timer event SOON");
		break;
	    default:
		printf("AMS:  BOGUS TimerReport code");
	}
	printf(" at %s", ctime(&clock));
    }
}

void ams::CommitState(boolean  DoQuit , boolean  MustQuit , boolean  MayPurge , boolean  OfferQuit)
{
    int ChangedSendmessages = 0, purgecode;
    boolean SafeToExit = TRUE;
    struct clist *ctmp;
    struct smlist *smtmp;

    if (!DoQuit && !MustQuit) SafeToExit = FALSE;
    if (DoQuit) {
	for (smtmp = SmlistRoot; smtmp; smtmp = smtmp->next) {
	    if ((smtmp->s)->HasChanged()) {
		++ChangedSendmessages;
	    }
	}
	if (ChangedSendmessages) {
	    char Question[400], *quest;

	    if (ChangedSendmessages > 1) {
		sprintf(Question, "Do you want to erase the %d pieces of mail you have not yet sent", ChangedSendmessages);
		quest = Question;
	    } else {
		quest = "Do you want to erase the mail you have not yet sent";
	    }
	    if (!(ams::GetAMS())->GetBooleanFromUser( quest, FALSE)) {
		SafeToExit = FALSE;
	    }
	}
    }
    for (ctmp = ClistRoot; ctmp; ctmp = ctmp->next) {
	(ctmp->c)->MakeCachedUpdates(); /* Never returns errors, delays them */
    }
    if (MayPurge && (ams::GetAMS())->CUI_DirectoriesToPurge()) {
	ams::WaitCursor(TRUE);
	if (amsutil::GetOptBit(EXP_PURGEONQUIT)) {
	    purgecode = (ams::GetAMS())->CUI_PurgeMarkedDirectories( FALSE, OfferQuit);
	} else {
	    purgecode = (ams::GetAMS())->CUI_PurgeMarkedDirectories( TRUE, OfferQuit);
	}
	ams::WaitCursor(FALSE);
	if (purgecode > 0) {
	    SafeToExit = FALSE;
	} else if (purgecode < 0) {
	    if (!MustQuit) DisplayAMS_ERRNO("Cannot purge deletions");
	    if (SafeToExit && !(ams::GetAMS())->GetBooleanFromUser( "Could not purge deletions; quit anyway", FALSE)) {
		SafeToExit = FALSE;
	    }
	}
    }
    if (ams::TryDelayedUpdates()) {
	if (!MustQuit) DisplayAMS_ERRNO("Cannot update your profile");
	if (SafeToExit && !(ams::GetAMS())->GetBooleanFromUser( "Could not update your profile; quit anyway", FALSE)) {
	    SafeToExit = FALSE;
	}
    }
    if ((ams::GetAMS())->MS_UpdateState()) {
	if (!MustQuit) {
	    DisplayAMS_ERRNO("Cannot update state");
	    if (SafeToExit && !(ams::GetAMS())->GetBooleanFromUser( "Could not update messageserver state; quit anyway", FALSE)) {
		SafeToExit = FALSE;
	    }
	}
    }
    if (MustQuit || (DoQuit && SafeToExit)) {
	(ams::GetAMS())->CUI_EndConversation();
	exit(0);
    }
}

int ams::CountAMSViews() {
    struct clist *ctmp;
    struct smlist *smtmp;
    struct flist *ftmp;
    struct blist *btmp;
    int ct = 0;

    for (smtmp = SmlistRoot; smtmp; smtmp = smtmp->next) {
	++ct;
    }
    for (ctmp = ClistRoot; ctmp; ctmp = ctmp->next) {
	++ct;
    }
    for (ftmp = FlistRoot; ftmp; ftmp = ftmp->next) {
	++ct;
    }
    for (btmp = BlistRoot; btmp; btmp = btmp->next) {
	++ct;
    }
    return(ct);
}

static FILE *GetCompletionFP(char  *partial)
{
    char fname[1+MAXPATHLEN];
    long errcode;
    class ams *myams = ams::GetAMS();
    FILE *fp;

    errcode = (myams)->MS_MatchFolderName( partial, fname);
    if (errcode) {
	(myams)->ReportError( "Could not get list of matching folders", ERR_CRITICAL, TRUE, errcode);
	return(NULL);
    }
    if (!(myams)->CUI_OnSameHost()) {
	char TmpName[1+MAXPATHLEN];

	(myams)->CUI_GenLocalTmpFileName( TmpName);
	if (errcode = (myams)->CUI_GetFileFromVice( TmpName, fname)) {
	    (myams)->ReportError( "Could not copy file from AFS", ERR_CRITICAL, TRUE, errcode);
	    return(NULL);
	}
	(myams)->MS_UnlinkFile( fname);
	strcpy(fname, TmpName);
    }
    fp = fopen(fname, "r");
    if (!fp) {
	(myams)->ReportError( "Could not open temporary file", ERR_CRITICAL, FALSE, 0);
    }
    unlink(fname); /* file remains around until closed */
    return(fp);
}

static void FolderHelp(char  *partial, long  rock, message_workfptr helpfunc, long  helprock)
{
    char LineBuf[1000], *s;
    FILE *fp;
    int inlen;

    ams::WaitCursor(TRUE);
    fp = GetCompletionFP(partial);
    if (!fp) {
	ams::WaitCursor(FALSE);
	return;
    }
    inlen = strlen(partial);
    while (fgets(LineBuf, sizeof(LineBuf), fp)) {
	int len=strlen(LineBuf)-1;
	if(len>=0 && LineBuf[len]=='\n') LineBuf[len] = '\0';
	s = &LineBuf[inlen+1];
	if (strchr(s, '.')) continue;
	(*helpfunc)(helprock, message_HelpListItem, LineBuf, NULL);
    }
    fclose(fp);
    ams::WaitCursor(FALSE);
}

static enum message_CompletionCode FolderComplete(char  *part , long  dummy , char  *buf, long  size)
{
    char LineBuf[1000], AnsBuf[1+MAXPATHLEN];
    FILE *fp;
    int ct = 0, i;
    boolean incomplete = FALSE;

    ams::WaitCursor(TRUE);
    fp = GetCompletionFP(part);
    if (!fp) {
	ams::WaitCursor(FALSE);
	return(message_Invalid);
    }
    AnsBuf[0] = '\0';
    while (fgets(LineBuf, sizeof(LineBuf), fp)) {
	LineBuf[strlen(LineBuf)-1] = '\0';
	++ct;
	if (AnsBuf[0]) {
	    for (i=0; AnsBuf[i] && LineBuf[i] && (AnsBuf[i] == LineBuf[i]); ++i) {
		;
	    }
	    if (AnsBuf[i]) {
		AnsBuf[i] = '\0';
		incomplete = LineBuf[i] ? TRUE : FALSE;
	    }
	} else {
	    strcpy(AnsBuf, LineBuf);
	}
    }
    ams::WaitCursor(FALSE);
    if (ct == 0) {
	strncpy(buf, part, size);
	return(message_Invalid);
    }
    strncpy(buf, AnsBuf, size);
    fclose(fp);
    if (ct == 1) return(message_Complete);
    return(incomplete ? message_Valid : message_CompleteValid);
}

int ams::GetFolderName(char  *prompt , char  *buf , int  buflen, char  *defaultname, boolean  MustMatch)
{
    if (message::AskForStringCompleted(NULL, 25, prompt, defaultname, buf, buflen, NULL, (message_completionfptr)FolderComplete, (message_helpfptr)FolderHelp, 0 /* rock */, MustMatch ? message_MustMatch : 0)< 0) return(-1);
    return(0);
}
