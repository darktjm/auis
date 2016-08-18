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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/stubs.C,v 1.15 1995/11/07 20:17:10 robr Stab74 $";
#endif

#include <stdio.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <pwd.h>
#ifdef AFS_ENV
#include <netinet/in.h>
#include <afs/param.h>
#include <sys/ioctl.h>
#include <rx/xdr.h>
#include <afs/vice.h>
#include <afs/errors.h>
#include <afs/prs_fs.h>
#include <afs/venus.h>
#include <afs/afsint.h>
#endif /* AFS_ENV */
#include <util.h>
#ifdef WHITEPAGES_ENV
#include <wp.h>
#endif /* WHITEPAGES_ENV */
#include <errprntf.h>
#include <mailconf.h>
#include <andrdir.h>

#include <im.H>
#include <proctable.H>
#include <event.H>
#include <environ.H>
#include <message.H>
#include <frame.H>

#include <cui.h>

#include <fdphack.h>
#include <ams.H>
#include <amsutil.H>


#include <msgsvers.h>

#include <cuiprotos.h>
#include <msprotos.h>
#include <msshrprotos.h>
#include <stubs.h>

void SubtleDialogs(int  Really);
int ChooseFromList(char  **QVec, int  def);
int HandleTimeout(char  *name, int  retries , int  restarts);
void DidRestart();
void SetTerminalParams(int  h,int  w);
#if defined(AMS_DEBUG_MALLOC_ENV) || defined(ELI_DEBUG_MALLOC_ENV)
void plumber(FILE  *fp);
void SetMallocCheckLevel();
#endif
void SubscriptionChangeHook(char  *name , char  *nick, int  status, class messages  *mess);
void DirectoryChangeHook(char  *adddir , char  *deldir, class messages  *mess);
void SetProgramVersion();
static int SendBug(char  *text , char  *moretext, int  code);
static int PrepareAutoBugFile(char  *text , char  *moretext , int  code, char  *FileName);
void WriteOutUserEnvironment(FILE  *fp, Boolean  IsAboutMessages);
static void DescribeLink(FILE  *fp, const char  *name);
static int SnarfFile(FILE  *fp, const char  *fname);
static void ReportOptionState(FILE  *fp);
int TildeResolve(const char  *old , char  *new_c);
void ReportError(char  *text, int  level, int  Decode);
void RealReportError(char  *text, int  level, int  Decode);
void ReportFailure(char  *text , char  *moretext, int  fmask);
void ReportSuccessNoLogging(char  *text);
void ReportSuccess(char  *text);
void RealReportSuccess(char  *text);
static void RememberMessage(char  *text , char  *moretext);
static void ReportErrorHistory(FILE  *fp);
int GenericCompoundAction(class view  *v, char  *prefix , char  *orgcmds);
Boolean GetBooleanFromUser(char  *prompt, int  defaultans);
int GetStringFromUser(char    *prompt , char    *buf, int  len , int  IsPassword);
char *BalancedQuote(char  *qstring);
void GetSeparators(char  *cmds , char  **argsep , char  **cmdsep);
char * DescribeProt(int  ProtCode);
void SnarfCommandOutputToFP(char  *cmd, FILE  *fp);

extern "C" {
    extern int pioctl(char *path, int id, struct ViceIoctl *vi_struct, int follow); 
} /* prototype of vice pioctl function */

extern char   *CUI_ClientVersion;
extern char CUI_MailDomain[], *CUI_WhoIAm;

extern int CUI_SnapIsRunning, CUI_LastCallFinished;

extern char
	*ms_errlist[], 
	*ms_errcauselist[], 
	*ms_errvialist[], 
	*rpc_errlist[];

extern int
	ms_nerr, 
	ms_nerrcause, 
	ms_nerrvia, 
	rpc_nerr;

static int Messages_Global_Error_Count = 0;

/* This stuff is here mostly to satisfy various linkers */

int Interactive = 1;

/* Masks for failure codes */

#define FMASK_BUG 1
#define FMASK_QUIT 2
#define FMASK_CONT 4
#define FMASK_COREBUG 8 /* Ignored in ReportFailure parameters, only used internally */
#define FMASK_QUIETCOREBUG 16 /* Ignored in ReportFailure parameters, only used internally */

/* these are used to pop up an error dialog if there
is no im up yet. */
static class im *errim=NULL;
static class view *errv=NULL;
static class frame *errf=NULL;

void ams_RemoveErrorWindow() {
    if(errim) errim->SetView(NULL);
    if(errf) errf->Destroy();
    if(errv) errv->Destroy();
    if(errim) errim->Destroy();
    errim=NULL;
    errf=NULL;
    errv=NULL;
}

static class view *GetIM() {
#ifdef RCH_ENV
    // It is ok to return NULL here.
    return (class view *)im::GetLastUsed();
#else
    if(im::GetLastUsed()) return (class view *)im::GetLastUsed();
    if(errim==NULL) {
	const char *errgeom=environ::GetProfile("ErrorDialogGeometry");
	if(errgeom==NULL) errgeom="400x300+1+1";
	im::SetGeometrySpec(errgeom);
	errim=im::Create(NULL);
	if(errim==NULL) {
	    return NULL;
	}
	errf=new frame;
	if(errf==NULL) {
	    errim->Destroy();
	    errim=NULL;
	    return NULL;
	}
	errv=new view();
	if(errv==NULL) {
	    errim->Destroy();
	    errf->Destroy();
	    errv->Destroy();
	    return NULL;
	}
	errf->SetView(errv);
	errim->SetView(errf);
	errv->WantInputFocus(errv);
	errf->PostDefaultHandler("message", errf->WantHandler("message"));
	im::ForceUpdate();
    }
    return (class view *)errv;
#endif
}

static int SubtleDialogFlag = 0;

void SubtleDialogs(int  Really) 
{
    SubtleDialogFlag = Really;
}

int ChooseFromList(char  **QVec, int  def)
{
    long myans;
    class view *v;

    v = GetIM();
    im::ForceUpdate(); /* reduce flashing */
    if (SubtleDialogFlag) {
	message::Advice(v, message_NoBlock);
	message::Advice(v, message_OnTop);
    }
    if(v && v->GetIM()) v->GetIM()->SetProcessCursor(NULL);
    if (message::MultipleChoiceQuestion(v, 50, QVec[0], def-1, &myans, &QVec[1], NULL)) {
	return(def);
    } 
    return(myans+1);
}

int HandleTimeout(char  *name, int  retries , int  restarts)
{
/*    if (retries < 2) { */ /* Not needed in SNAP 2 */
    if (retries < 0) {
	ReportSuccess("Connection to message server timed out; retrying...");
	im::ForceUpdate();
	return(CUI_RPC_RETRY);
    }
    if (restarts < 2) {
	ReportSuccess("Reconnecting to message server; please wait...");
	im::ForceUpdate();
	return(CUI_RPC_RESTART);
    }
    return(CUI_RPC_BUGOUT);  /* No message needed here -- will propogate error */
}

void DidRestart() {
    ReportSuccess("Reconnected to Message Server!");
}

void SetTerminalParams(int  h,int  w)
{
}

#if defined(AMS_DEBUG_MALLOC_ENV) || defined(ELI_DEBUG_MALLOC_ENV)
void plumber(FILE  *fp)
{
    im::plumber(fp); /* silly, but it helps with the messageserver signal handling */
}
void SetMallocCheckLevel(){} /* BOGUS -- satisfies linker, but doesn't work. */

#endif

void SubscriptionChangeHook(char  *name , char  *nick, int  status, class messages  *mess)
{
    ams::SubscriptionChangeHook(name, nick, status, mess);
}

void DirectoryChangeHook(char  *adddir , char  *deldir, class messages  *mess)
{
    ams::DirectoryChangeHook(adddir, deldir, mess);
}

char ProgramVersion[70];

void SetProgramVersion()
{
    sprintf(ProgramVersion, "((prog messages %d %d))", MESSAGES_MAJOR_VERSION, MESSAGES_MINOR_VERSION);
}

static int SendBug(char  *text , char  *moretext, int  code)
{
    char FileName[1+MAXPATHLEN], Msg[100+MAXPATHLEN];
    int DumpingCore;

    if (PrepareAutoBugFile(text, moretext, code, FileName)) {
	errprintf(im::GetProgramName(), ERR_CRITICAL, 0, 0, "Cannot open temporary file to send bug mail!");
	ams::WaitCursor(FALSE);
	return(-1);
    }
    if (code & (FMASK_COREBUG | FMASK_QUIETCOREBUG)) {
	DumpingCore = TRUE;
    } else {
	DumpingCore = FALSE;
    }
    if (CUI_SubmitMessage(FileName, AMS_SEND_ILLEGAL)) {

	errprintf(im::GetProgramName(), ERR_CRITICAL, 0, 0, "Automatic bug mail failed--sorry! (%s!)", DumpingCore ? "Not dumping core, either" : "Really");
    } else {
	char Buf[1000];

	sprintf(Buf, "Sent automatic bug mail to %s", MessagesAutoBugAddress);
	ReportSuccess(Buf);
	if (DumpingCore) {
	    char CoreDirectory[1+MAXPATHLEN];

	    CoreDirectory[0] = '\0';
	    getwd(CoreDirectory);
	    sprintf(Msg, "Dumping core in directory %s", CoreDirectory);
	    errprintf(im::GetProgramName(), ERR_CRITICAL, 0, 0, Msg);
	    ReportSuccess(Msg);
	    kill(getpid(), SIGBUS);
	    return(-1); /* Hah!  That's what you think, you stupid compiler. */
	}
    }
    ams::WaitCursor(FALSE);
    return(0);
}

static int PrepareAutoBugFile(char  *text , char  *moretext , int  code, char  *FileName)
{
    char Buf[2000], Subj[50], CoreDirectory[1+MAXPATHLEN];
    FILE *fp;
    int IsQuiet, IsCore;

    IsCore = (code & (FMASK_COREBUG | FMASK_QUIETCOREBUG)) ? TRUE : FALSE;
    if (code & FMASK_QUIETCOREBUG) {
	IsQuiet = TRUE;
    } else {
	if (GetStringFromUser("Describe bug briefly: ", Buf, sizeof(Buf), FALSE)) {
	    IsQuiet = TRUE;
	} else {
	    IsQuiet = FALSE;
	    strncpy(Subj, Buf, sizeof(Subj) - 6);
	    Subj[sizeof(Subj) -6] = '\0';
	    strcat(Subj, " ...");
	}
    }
    CUI_GenLocalTmpFileName(FileName);
    fp = fopen(FileName, "w");
    if (!fp) {
	return -1;
    }
    ams::WaitCursor(TRUE);
    fputs("To: ", fp);
    fputs(MessagesAutoBugAddress, fp);
    fputs(IsCore ? "\nSubject: CORE BUG -- " : "\nSubject: BUG -- ", fp);
    fputs(IsQuiet ? "Unreported Messages Error" : Subj, fp);
    fputs("\n\nThis bug report was generated automatically as the result of an AMS error message.", fp);
    if (IsQuiet) {
	fputs("\n\nThis error could not be reported to the user, so he did not describe it.", fp);
    } else {
	fputs("\n\nThe user's description of this message follows:\n\n", fp);
	fputs(Buf, fp);
    }
    if (text) {
	fputs("\n\nThe primary error reported was:\n\t", fp);
	fputs(text, fp);
    } else {
	fputs("\n\nThere was no primary error text reported.", fp);
    }
    if (moretext) {
	fputs("\n\nThe secondary explanatory text was:\n\t", fp);
	fputs(moretext, fp);
    } else {
	fputs("\n\nThere was no secondary explanatory text reported.", fp);
    }
    if (IsCore) {
	getwd(CoreDirectory);
	fputs("\n\nA core file will be dumped in the directory:\n\t", fp);
	fputs(CoreDirectory, fp);
    } else {
	fputs("\n\nNo core file is being dumped (I hope).", fp);
    }
    fputs("\n\n", fp);
    WriteOutUserEnvironment(fp, TRUE);
    fclose(fp);
    return(0);
}

static char *SepLine = "\n\n----------------------------------------\n\n";

void WriteOutUserEnvironment(FILE  *fp, Boolean  IsAboutMessages)
{
    const char *adir, *ldir;
    struct CellAuth *ca;
    int i, RC;
    time_t t;

    t = time(0);
    fprintf(fp, "\n\nThe following bug report information was prepared automatically by the program `%s', %s, at %s\n(More information on version number and CPU type should be visible in the Received: headers.)", im::GetProgramName(), CUI_ClientVersion, ctime(&t));

    if (IsAboutMessages) {
	fputs(SepLine, fp);
	ReportErrorHistory(fp);
    }
#ifdef AFS_ENV
    if (AMS_ViceIsRunning) {
	char FBuf[1+MAXPATHLEN];
	struct ViceIoctl blob;
	char space[2000];
	int code;
	struct VolumeStatus *status;

	FBuf[0] = '\0';
	code = GetCurrentWSCell(FBuf, sizeof(FBuf));
	if (code == 0) {
	    fprintf(fp, "\nThis workstation is in the AFS Cell named %s.\n\n", FBuf);
	} else {
	    fprintf(fp, "\nCan't determine workstation cell: errno %d\n\n", code);
	}

	TildeResolve("~", FBuf);
	blob.out_size = sizeof(space);
	blob.in_size = 0;
	blob.out = space;
	code = pioctl(FBuf, VIOCGETVOLSTAT, &blob, 1);
	if (code) {
	    fprintf(fp, "Attempt to check the user's AFS Quota failed (%d)\n\n", code);
	} else {
	    status = (struct VolumeStatus *) space; 
	    fprintf(fp, "Quota used for user's home AFS volume (containing %s): %6.0f%%\n\n", FBuf, (((1.0 * status->BlocksInUse)/status->MaxQuota) * 100.0));
	    fprintf(fp, "Utilization of AFS disk partition containing that volume: %6.0f%%\n\n", (100.0 - (((1.0 * status->PartBlocksAvail)/status->PartMaxBlocks) * 100.0)));
	}
    } else
#endif /* AFS_ENV */
    {
	fputs("This workstation is apparently not running the Andrew File System.\n", fp);
    }
    fprintf(fp, "The current user is %s in mail domain %s.\n", CUI_WhoIAm, CUI_MailDomain);
    fprintf(fp, "Known tokens:\n");
    (void) ca_UpdateCellAuths();
    ca = NULL; i = 1;
    for (RC = FindNextCell(&ca); RC == 0 && ca != NULL; RC = FindNextCell(&ca)) {
	fprintf(fp, "(%d) ``%s'', %s%svid %d", i, ca->CellName,
	       (ca->IsPrimary < 0 ? "" : ca->IsPrimary ? "primary, " : "non-primary, "),
	       (ca->IsLocal < 0 ? "" : ca->IsLocal ? "local, " : "non-local, "),
	       ca->ViceID);
	if (ca->UserName != NULL) fprintf(fp, " (user %s)", ca->UserName);
	if (ca->homeDir != NULL) fprintf(fp, " (home %s)", ca->homeDir);
	if (ca->WpError > 0) fprintf(fp, " (wperr %d)", ca->WpError);
	fprintf(fp, ", expires %s.\n", NiceTime(ca->ExpireTime));
	++i;
    }
    if (RC != 0) fprintf(fp, "Can't find a next cell: %d\n", RC);
    /* Include as much as possible of the environment here.  Of course, things like
      the machine name & program version numbers will show up in the Received header */
    fputs(SepLine, fp);
    adir = environ::AndrewDir(NULL);
    fprintf(fp, "This program is configured with ANDREWDIR %s\n", adir ? adir : "<NULL>");
    if (strcmp(adir, "/usr/andrew") && strcmp(adir, "/usr/andy")) {
	DescribeLink(fp, adir);
    }
    DescribeLink(fp, "/usr/andrew");
    DescribeLink(fp, "/usr/andy");
    ldir = LocalDir(NULL);
    fprintf(fp, "This program is configured with LOCALDIR %s\n", ldir ? ldir : "<NULL>");
    if (ldir && strcmp(ldir, "/usr/local")) {
	DescribeLink(fp, ldir);
    }
    DescribeLink(fp, "/usr/local");
    fputs(SepLine, fp);

    for (i=0; conf_ConfigNames[i] != NULL; ++i) {
	if (SnarfFile(fp, conf_ConfigNames[i]) == 0) break;
    }
    if (conf_ConfigUsed >= 0) {
	if (conf_ConfigUsed < conf_NumConfigNames) {
	    fprintf(fp, "The AndrewSetup file used may have been [%d] %s\n", conf_ConfigUsed, conf_ConfigNames[conf_ConfigUsed]);
	} else {
	    fprintf(fp, "Probably no AndrewSetup file was found; last index %d, errno %d\n", conf_ConfigUsed, conf_ConfigErrno);
	}
    }
    SnarfFile(fp, "~/preferences");
    SnarfFile(fp, "~/.preferences");
    SnarfFile(fp, "~/.Xdefaults");
    SnarfFile(fp, "~/.AMS.flames");
    SnarfFile(fp, "~/.MS.spec");
    SnarfFile(fp, "~/MS-Errors"); /* This string also appears in ams/libs/errmsgs.c */
    ReportOptionState(fp);

    SnarfCommandOutputToFP("xset q", fp);
    fputs(SepLine, fp);
    fputs("Can't get list of the environment variables...", fp);
   /* for (i=0; environ[i]; ++i) {
	fprintf(fp, "%s\n", environ[i]);
    } */
    fputs(SepLine, fp);

    if (IsAboutMessages) {
#ifdef PLUMBFDLEAKS
	fprintf(fp, "Here are the open files known to the file descriptor plumber:%s", SepLine);
	fdplumb_SpillGutsToFile(fp, 1);
	fputs(SepLine, fp);
#endif /* PLUMBFDLEAKS */

#if defined(DEBUG_MALLOC_ENV) && defined(AMS_DEBUG_MALLOC_ENV)
	fprintf(fp, "Here are the malloc plumber statistics:%s", SepLine);
	plumber(fp);
#else /* #if defined(DEBUG_MALLOC_ENV) && defined(AMS_DEBUG_MALLOC_ENV) */
	fprintf(fp, "(No malloc plumber statistics without DEBUG_MALLOC_ENV and AMS_DEBUG_MALLOC_ENV.)\n");
#endif /* #if defined(DEBUG_MALLOC_ENV) && defined(AMS_DEBUG_MALLOC_ENV) */
    } else {
	fputs("This is not a messages bug report; the plumber statistics are omitted.\n", fp);
    }
}

static void DescribeLink(FILE  *fp, const char  *name)
{
    struct stat stbuf;
    char Buffer[1+MAXPATHLEN];

    fprintf(fp, "The file %s", name);
    errno = 0;
    if (lstat(name, &stbuf)) {
	if (errno == ENOENT) {
	    fputs(" does not exist.\n", fp);
	} else if (errno == EACCES) {
	    fputs(" is unreadable.\n", fp);
	} else {
	    fprintf(fp, " causes lstat to fail (%s).\n", strerror(errno));
	}
	return;
    }
#ifdef S_IFLNK
    if ((stbuf.st_mode & S_IFMT) == S_IFLNK) {
	int len;
	fputs(" is a symbolic link ", fp);
	len = readlink(name, Buffer, sizeof(Buffer));
	if (len < 0) {
	    fprintf(fp, "but the readlink call failed (%s).\n", strerror(errno));
	} else {
	    Buffer[len] = '\0';  /* I can't believe the lstat implementors didn't do this... */
	    fprintf(fp, "to the file %s.\n", Buffer);
	}
    } else
#endif	/* S_IFLNK */
    {
	if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
	    fputs(" is not a symbolic link.\n", fp);
	} else {
	    fputs(" is not a symbolic link.\n", fp);
	}
    }
}

static int SnarfFile(FILE  *fp, const char  *fname)
{
    struct stat stbuf;
    char LineBuf[1000], Buf[1+MAXPATHLEN];
    FILE *rfp;
    int RC;
    Boolean OnVice;
    char CellBuf[200];

    Buf[0] = '\0';
    RC = TildeResolve(fname, Buf);
    if (RC != 0) {
	fprintf(fp, "Could not resolve tilde in %s (%d).\n", fname, RC);
	return 1;
    }
    if (stat(Buf, &stbuf)) {
	fprintf(fp, "Could not stat file %s (%s).\n", Buf, strerror(errno));
	return 1;
    }
    if ((stbuf.st_mode & S_IFMT) != S_IFREG) {
	fprintf(fp, "File %s is not a regular file, but has mode %#o.\n", Buf, stbuf.st_mode);
	return 1;
    }
    errno = 0;
    rfp = fopen(Buf, "r");
    if (!rfp) {
	if (errno == 0) errno = ENOMEM;
	fprintf(fp, "Could not open file %s (%s).\n", Buf, strerror(errno));
	return 1;
    }	
    fprintf(fp, "File: %s\n", Buf);
    OnVice = FALSE;
    CellBuf[0] = '\0';
#ifdef AFS_ENV
    if (AMS_ViceIsRunning) {
	RC = GetCellFromFileName(Buf, CellBuf, sizeof(CellBuf));
	if (RC == 0) OnVice = TRUE;
    }
#endif /* AFS_ENV */
    fprintf(fp, "Protection Mode (octal): %#o\nOn Vice: %s\nOwner: User # %d", stbuf.st_mode, OnVice ? "YES" : "NO", stbuf.st_uid);
    if (CellBuf[0] != '\0') fprintf(fp, ", AFS Cell %s", CellBuf);
    fprintf(fp, "\nFile Size: %ld\nLast Modified: %s", stbuf.st_size, ctime(&(stbuf.st_mtime)));
    fprintf(fp, "The file contents are enclosed by separating lines:%s", SepLine);
    while (fgets(LineBuf, sizeof(LineBuf), rfp)) {
	fputs(LineBuf, fp);
    }
    fputs(SepLine, fp);
    fclose(rfp);
    return 0;
}

static char *OptDescriptions[] = {
    "EXP_FILEINTO",
    "OBSOLETE 1",
    "OBSOLETE 2",
    "EXP_FILEICONCAPTIONS",
    "EXP_FILEINTOMENU",
    "OBSOLETE 5",
    "EXP_SHOWCLASSES",
    "OBSOLETE 7",
    "OBSOLETE 8",
    "OBSOLETE 9",
    "OBSOLETE 10",
    "EXP_FIXCAPTIONS",
    "EXP_PURGEONQUIT",
    "EXP_SUBSEXPERT",
    "OBSOLETE 14",
    "EXP_WHITESPACE",
    "OBSOLETE 16",
    "OBSOLETE 17",
    "EXP_THREEREPLIES",
    "EXP_SHOWNOHEADS",
    "EXP_MARKING",
    "EXP_SETQUITHERE",
    "EXP_SHOWMORENEXT",
    "EXP_MARKASUNREAD",
    "EXP_APPENDBYNAME",
    "EXP_MARKEDEXTRAS",
    "EXP_CLEARAFTER",
    "EXP_HIDEAFTER",
    "EXP_KEEPBLIND",
    "EXP_INSERTHEADER",
    "EXP_KEYSTROKES",
    "OBSOLETE 31",
    "EXP_BIGSTYLES",
    "EXP_CHECKRECIP",
    "EXP_PUNTBUTT",
    "OBSOLETE 35",
    "EXP_SHOWALLBUTKEYS",
    "EXP_PUNTMENU",
    "EXP_SIDEBYSIDE",
    "OBSOLETE 39",
    "OBSOLETE 40",
    "EXP_DUMPCORE",
    "EXP_CKPONTMP",
    "EXP_FORCESEND",
    "EXP_FORMATMENUS",
    "OBSOLETE 45",
    "EXP_SENDEMPTY",
    "EXP_WARPWINDOW",
    "EXP_GROWFOLDS",
    "EXP_SIGNMAIL",
    "EXP_NOFIRSTFOLDER",
    "EXP_VANISH",
};

/* Changes to the above descriptions should be accompanied by changes to the definitions in amsutil.ch and to the EXP_MAXUSED constant there. */

static void ReportOptionState(FILE  *fp)
{
    int i;

    fprintf(fp, "Here are the current messages binary option settings:\n\n");
    for (i=0; i<EXP_MAXUSED; ++i) {
	fprintf(fp, "Option %s: permanent %s current %s\n", OptDescriptions[i], amsutil::GetPermOptBit(i) ? "ON" : "OFF", amsutil::GetOptBit(i) ? "ON" : "OFF");
    }
    fprintf(fp, "\n\n");
}

/* This routine is similar to one named ResolveTildes in util.c for the 
message server.  Having it exist twice is wasteful in standalone messages
(messagesn), but we don't want it to have to compile as part of the cui on
PC's, either. */

int TildeResolve(const char  *old , char  *new_c)
{
	static char *MyHomeDir = NULL, *udir;
	struct passwd *pw;
	char *t, user[100]; /* User names should be < 100 chars ? */
	int olen;

	debug(1, ("TildeResolve %s\n", old));
	
	old = amsutil::StripWhiteEnds(old, &olen);
	if (*old != '~') {
	    strncpy(new_c, old, olen);
	    new_c[olen] = 0;
	} else {
	    *new_c = '\0';
	    if (*++old == '/' || ! --olen) {
		if (!MyHomeDir) {
		    pw = getcpwnam((ams::GetAMS())->CUI_WhoIAm(), (ams::GetAMS())->CUI_MailDomain());
		    if (!pw) return(-1);
		    MyHomeDir = (char *)malloc(1+strlen(pw->pw_dir));
		    if (!MyHomeDir) return(-2);
		    strcpy(MyHomeDir, pw->pw_dir);
		}
		if (olen) {
		    sprintf(new_c, "%s/%.*s", MyHomeDir, --olen, ++old);
		} else {
		    strcpy(new_c, MyHomeDir);
		}
	    } else {
		for (t=user; olen && *old != '/'; ++old, --olen, ++t) {
		    *t = *old;
		}
		*t = '\0';
		if (olen){ ++old; --olen; }
		udir = FindUserDir(user, (ams::GetAMS())->CUI_MailDomain());
		if (udir == (char *) 0 || udir == (char *) -1) return(-3);
		sprintf(new_c, "%s/%.*s", udir, olen, old);
	    }
	}
	return(0);
}


void ReportError(char  *text, int  level, int  Decode)
{
    RealReportError(text, level, Decode);
}

void RealReportError(char  *text, int  level, int  Decode)
{
    static char LatestDisaster[400] = "";
    char    ErrorText[500],
        NumDum[10];
    int     errnum,
        errcause,
        errvia,
	mymask,
        LatestDisasterTime = 0,
        errrpc;

    debug(1,("ReportError %s (%d, %d)\n", text, level, Decode));

    ++Messages_Global_Error_Count;
    if (mserrcode == 0) {
	Decode = 0;
    } else if (!errno) {
	Decode = 1;
    }
    ErrorText[0] = '\0';
    if (Decode) {
	errnum = AMS_ERRNO;
	errcause = AMS_ERRCAUSE;
	errvia = AMS_ERRVIA;
	errrpc = AMS_RPCERRNO;
	if (errrpc) {
	    sprintf(ErrorText, "AMS RPC error: %s", rpc_errlist[errrpc]);
	    ReportFailure(text, ErrorText, FMASK_QUIT | FMASK_BUG | FMASK_CONT);
	}
	else {
	    if (errnum < 0 || errnum >= (EMSBASE + ms_nerr)
		    || (errnum < EMSBASE && errnum > sys_nerr)) {
		errprintf(im::GetProgramName(), ERR_WARNING, 0, 0, "errnum %d out of range", errnum);
		errnum = EMSUNKNOWN;
	    }
	    if (errcause < 0 || errcause >= ms_nerrcause) {
		errprintf(im::GetProgramName(), ERR_WARNING, 0, 0, "errcause %d out of range", errcause);
		errcause = EIN_UNKNOWN;
	    }
	    if (errvia < 0 || errvia >= ms_nerrvia) {
		errprintf(im::GetProgramName(), ERR_WARNING, 0, 0, "errvia %d out of range", errvia);
		errvia = EVIA_UNKNOWN;
	    }
	    if (errnum < EMSBASE) {
		    sprintf(ErrorText, "Error: %s (in ", strerror(errnum));
	    }
	    else {
		if (ms_errlist[errnum - EMSBASE]) {
		    sprintf(ErrorText, "Error: %s (in ", ms_errlist[errnum - EMSBASE]);
		}
		else {
		    sprintf(ErrorText, "Unknown error %d (in ", errnum);
		}
	    }
	    if (ms_errcauselist[errcause]) {
		strcat(ErrorText, ms_errcauselist[errcause]);
	    }
	    else {
		strcat(ErrorText, "(in unknown call ");
		sprintf(NumDum, "%d", errcause);
		strcat(ErrorText, NumDum);
	    }
	    strcat(ErrorText, " in ");
	    if (ms_errvialist[errvia]) {
		strcat(ErrorText, ms_errvialist[errvia]);
	    }
	    else {
		strcat(ErrorText, "in unknown caller ");
		sprintf(NumDum, "%d", errvia);
		strcat(ErrorText, NumDum);
	    }
	    strcat(ErrorText, ")");
	   
	}
    } else {
	errnum = errno;
	if (errnum != 0) {
	    sprintf(ErrorText, "Error %d (%s) *may* have been encountered internally.", errno, strerror(errno));
	}
    }

#ifdef PLUMBFDLEAKS
    if (errnum == EMFILE) {
	fdplumb_SpillGuts();
    }
#endif /* PLUMBFDLEAKS */
    mymask = FMASK_CONT | FMASK_BUG;
    if (vdown(errnum)) {
	text = "A file server or the network is down.";
	mymask &= ~FMASK_BUG;
    } else if (errnum == EACCES) {
	int Authenticated;

	mymask &= ~FMASK_BUG; /* Never report these as auto-errors */
	mserrcode = MS_CheckAuthentication(&Authenticated);
	if (mserrcode) {
	    if (vdown(AMS_ERRNO)) {
		text = "A file server or the network is down.";
	    }
	} else if (!Authenticated) {
	    text = "Your Vice authentication has apparently expired.";
	}
    } else if (errnum == ENOMEM) {
	text = "You are out of virtual memory!";
	mymask &= ~FMASK_BUG;
    } else if (errnum == EWOULDBLOCK) {
	text = "Someone else has a file or directory locked; try again soon.";
	mymask &= ~FMASK_BUG;
#ifdef EDQUOT
    } else if (errnum == EDQUOT) {
	text = "A file operation failed because it would overflow your disk quota.";
	mymask &= ~FMASK_BUG;
#endif /* EDQUOT */
    } else if (errnum == ENOSPC || errnum == EMSFASCISTSUBSCRIPTION || errnum == EMSNOVUID) {
	mymask &= ~FMASK_BUG;
    }
    if (level <= ERR_FATAL) {
	mymask = (mymask | FMASK_QUIT) & ~FMASK_CONT;
	ReportFailure(text, ErrorText, mymask);
	exit(-1); /* not reached, I hope */
    }
#define DECENTINTERVAL 30 /* Prevent the same horrible dialog boxes recurring endlessly */
    if (!strcmp(text, LatestDisaster) && ((time(0) - LatestDisasterTime) < DECENTINTERVAL)) {
	if (ErrorText[0]) {
	    ReportSuccess(ErrorText);
	    }
	ReportSuccess(text);
	return;
    }
    if (level <= ERR_CRITICAL && strcmp(text, LatestDisaster)) {
	mymask |= FMASK_QUIT;
	ReportFailure(text, ErrorText, mymask);
	strcpy(LatestDisaster, text);
	LatestDisasterTime = time(0);
    } else {
	ReportFailure(text, ErrorText, mymask);
    }	
}

void ReportFailure(char  *text , char  *moretext, int  fmask)
{
#define MAXFAILCHOICES 5
    char *QVec[MAXFAILCHOICES+1];
    int ct, Codes[MAXFAILCHOICES+1], ans;

    if (moretext && !*moretext) moretext = NULL; /* get it over with */
    RememberMessage(text, moretext);
restart:
    ct = 1;
    QVec[0] = text;
    if (MessagesAutoBugAddress[0] && (fmask & FMASK_BUG)) {
	QVec[ct] = "Send automatic bug report";
	Codes[ct++] = FMASK_BUG;
	if (amsutil::GetOptBit(EXP_DUMPCORE)) {
	    QVec[ct] = "Send automatic bug report & dump core";
	    Codes[ct++] = FMASK_COREBUG;
	}
    }
    if (fmask & FMASK_QUIT) {
	QVec[ct] = (char *)((fmask & FMASK_CONT) ? "Quit the program" : "Quit the program (cannot continue)");
	Codes[ct++] = FMASK_QUIT;
    }
    /* This one should remain last if you add other things */
    if (fmask & FMASK_CONT) {
	QVec[ct] = "Continue";
	Codes[ct++] = FMASK_CONT;
    }
    if (ct <= 1) {
	fmask = FMASK_CONT;
	goto restart;
    }
    if (moretext) ReportSuccessNoLogging(moretext);
    QVec[ct] = NULL;
    SubtleDialogs(FALSE);
    ans = ChooseFromList(QVec, ct-1);
    if ((ans < 1) || (ans >= ct)) {
	errprintf(im::GetProgramName(), ERR_CRITICAL, 0, 0, "Help!  I have an error and cannot ask a question!  %s (%s)", text, moretext ? moretext : "");
	ans = FMASK_QUIETCOREBUG;
    } else {
	ans = Codes[ans];
    }
    switch(ans) {
	case FMASK_BUG:
	case FMASK_COREBUG:
	case FMASK_QUIETCOREBUG:
	    SendBug(text, moretext, ans);
	    fmask &= ~(FMASK_BUG | FMASK_COREBUG | FMASK_QUIETCOREBUG);
	    if (fmask & ~FMASK_CONT) goto restart;
	    break;
	case FMASK_QUIT:
	    if (!(ams::GetAMS())->CUI_SnapIsRunning()) {
		ams::CommitState(TRUE, TRUE, FALSE, FALSE); /* Might help, could hang on snapified version */
	    }
	    exit(-1);
	case FMASK_CONT:
	default:
	    break;
    }
}

void ReportSuccessNoLogging(char  *text)
{
    message::DisplayString(GetIM(), 10, text);
    im::ForceUpdate();
}

void ReportSuccess(char  *text)
{
    RealReportSuccess(text);
}

void RealReportSuccess(char  *text)
{
    debug(1, ("ReportSuccess %s\n", text));
    RememberMessage(text, NULL);
    ReportSuccessNoLogging(text);
    /*return(0);*/
}


#define ERRHISTSIZE 25
static char *ErrorHistory[ERRHISTSIZE];
static time_t ErrHistTimes[ERRHISTSIZE];
static int DidInitErrHist = 0;
static int ErrHistStart = 0;

static void RememberMessage(char  *text , char  *moretext)
{
    char *SavedCopy;

    SavedCopy = (char *)malloc(5+strlen(text)+(moretext ? strlen(moretext) : 0));
    if (!SavedCopy) {
	message::DisplayString(GetIM(), 10, "OUT OF MEMORY IN TRYING TO SAVE ERROR MESSAGE HISTORY!");
	return;
    }
    strcpy(SavedCopy, text);
    if (moretext) {
	strcat(SavedCopy, " (");
	strcat(SavedCopy, moretext);
	strcat(SavedCopy, ")");
    }
    if (!DidInitErrHist) {
	int i;
	for (i=0; i<ERRHISTSIZE; ++i) {
	    ErrorHistory[i] = NULL;
	}
	DidInitErrHist = TRUE;
    }
    if (ErrorHistory[ErrHistStart]) free(ErrorHistory[ErrHistStart]);
    ErrHistTimes[ErrHistStart] = time(0);
    ErrorHistory[ErrHistStart] = SavedCopy;
    if (++ErrHistStart >= ERRHISTSIZE) ErrHistStart = 0;
}

static void ReportErrorHistory(FILE  *fp)
{
    int i, numinhist = 0, which;
    if (!DidInitErrHist) {
	fprintf(fp, "There have been NO user messages.\n");
	return;
    }
    for (i=0; i<ERRHISTSIZE; ++i) {
	if (ErrorHistory[i]) ++numinhist;
    }
    fprintf(fp, "Here are the %d most recent user messages:\n\n", numinhist);
    i = ErrHistStart;
    which = 1;
    do {
	if (ErrorHistory[i]) {
	    fprintf(fp, "%d: %s -- %s", which++,  ErrorHistory[i], ctime(&ErrHistTimes[i]));
	}
	if (++i >= ERRHISTSIZE) i = 0;
    } while (i!=ErrHistStart);
}

int GenericCompoundAction(class view  *v, char  *prefix , char  *orgcmds)
{
    char *nextcmd, *args, ErrorText[1000], *cmds, *cmdstofree;
    struct proctable_Entry *ptent;
    int len = 0, numdone = 0, priorerrors;
    proctable_fptr thisproc;

    if (!orgcmds) return -1;
    cmds = cmdstofree = (char *)malloc(1+strlen(orgcmds));
    if (!cmds) return -1;
    strcpy(cmds, orgcmds);
    if (prefix) len = strlen(prefix);
    priorerrors = Messages_Global_Error_Count;
    if (cmds) cmds = amsutil::StripWhiteEnds(cmds);
    while (cmds && *cmds) {
	GetSeparators(cmds, &args, &nextcmd);
	if (nextcmd) {
	    *nextcmd++ = '\0';
	    nextcmd = amsutil::StripWhiteEnds(nextcmd);
	}
	if (args) {
	    *args++='\0';
	    args = amsutil::StripWhiteEnds(args);
	}
	cmds = amsutil::StripWhiteEnds(cmds);
	if (prefix) {
	    if (!strncmp(cmds, "untyped-", 8)) {
		cmds += 8;
	    } else if (strncmp(prefix, cmds, len)) {
		sprintf(ErrorText, "The compound command component '%s' is not a '%s' command", cmds, prefix);
		message::DisplayString(v, 75, ErrorText);
		++Messages_Global_Error_Count;
		free(cmdstofree);
		return(-1);
	    }
	}
	ptent = proctable::Lookup(cmds);
	if (!ptent) {
	    char *s = index(cmds, '-');
	    if (s) *s = '\0';
	    ATK::LoadClass(cmds);
	    if (s) *s = '-';
	    ptent = proctable::Lookup(cmds);
	    if (!ptent) {
		sprintf(ErrorText, "The name %s is not in the proc table.", cmds);
		message::DisplayString(v, 75, ErrorText);
		++Messages_Global_Error_Count;
		free(cmdstofree);
		return(-1);
	    }
	}
	proctable::ForceLoaded(ptent);
	thisproc = proctable::GetFunction(ptent);
	if (!thisproc) {
	    sprintf(ErrorText, "The name %s is in the proc table but the function is undefined.", cmds);
	    message::DisplayString(v, 75, ErrorText);
	    ++Messages_Global_Error_Count;
	    free(cmdstofree);
	    return(-1);
	}
	thisproc(v, (long)args);
	if (priorerrors != Messages_Global_Error_Count) {
	    message::DisplayString(v, 25, "Error in compound command component; compound command terminated.");
	    ++Messages_Global_Error_Count; /* helps in the recursive case */
	    free(cmdstofree);
	    return(-1);
	}
	++numdone;
	cmds = nextcmd;
    }
    if (numdone <= 0) {
	message::DisplayString(v, 10, "Executed a null compound operation.");
    }
    free(cmdstofree);
    return(0);
}

Boolean GetBooleanFromUser(char  *prompt, int  defaultans)
{
    static char *BooleanQVec[4] = {"", "Yes", "No", NULL};
    int ans;
    char MyQuest[500];

    strcpy(MyQuest, prompt);
    strcat(MyQuest, "?");
    BooleanQVec[0] = MyQuest;
    ans = ChooseFromList(BooleanQVec, defaultans ? 1 : 2);
    return (ans == 1);
}

int GetStringFromUser(char    *prompt , char    *buf, int  len , int  IsPassword)
{
    char *new_prompt = (char *)malloc(strlen(prompt) + 3);
    int retval = 0;

    strcpy(new_prompt,prompt);
    strcat(new_prompt," ? ");
    if (IsPassword) {
        if (message::AskForPasswd(GetIM(), 50, new_prompt, NULL, buf, len) < 0) {
	    *buf = '\0';
	    retval = -1;
        }
    } else {
        if (message::AskForString(GetIM(), 50, new_prompt, NULL, buf, len) < 0) {
	    *buf = '\0';
	    retval = -1;
        }
    }
    free(new_prompt);
    return(retval);
}

char *BalancedQuote(char  *qstring)
{
    int qct = 0;
    char *s;

    if (!qstring) return(NULL);
    for (s=qstring; *s; ++s) {
	if (*s == '`') {
	    ++qct;
	} else if (*s == '\'') {
	    if (--qct <= 0) {
		return(qct ? NULL : s);
	    }
	}
    }
    return(NULL);
}

void GetSeparators(char  *cmds , char  **argsep , char  **cmdsep)
{
    char *secondquote, *firstspace, *firstsemi;

    if (!cmds) {
	*argsep = *cmdsep = NULL;
	return;
    }
    firstspace = index(cmds, ' ');
    firstsemi = index(cmds, ';');
    if (!firstsemi) {
	/* simple command */
	*argsep = firstspace;
	*cmdsep = NULL;
	return;
    }
    if (!firstspace || (firstsemi < firstspace)) {
	/* first cmd has no args */
	*argsep = NULL;
	*cmdsep = firstsemi;
	return;
    }
    *argsep = firstspace; /* this much is for sure by now */
    if (*++firstspace != '`') {
	/* argument not quoted */
	*cmdsep = firstsemi;
	return;
    }
    /* firstspace now inappropriately points at the first open quote */
    secondquote = BalancedQuote(firstspace);
    if (!secondquote) {
	/* syntax error?  Leave it up to the interface and treat as 1 cmd. */
	*cmdsep = NULL;
	return;
    }
    /* Now get rid of the quote marks to pass things on */
    *firstspace = ' ';
    *secondquote = ' ';
    *cmdsep = index(secondquote, ';');
    return;
}    

char *
DescribeProt(int  ProtCode)
{
      switch(ProtCode) {
	case AMS_DIRPROT_READ:
	    return("Private BB");
	case AMS_DIRPROT_LOCALBB:
	    return("Local BB");
	case AMS_DIRPROT_EXTBB:
	    return("External BB");
	case AMS_DIRPROT_OFFBB:
	    return("Official BB");
	case AMS_DIRPROT_MODIFY:
	    return("Editable BB");
	case AMS_DIRPROT_MBOX:
	    return("BB you administer");
	case AMS_DIRPROT_FULLMAIL:
	    return("Mail");
	default: /* Should not happen */
	    return("Unreadable(?)");
      }
}

void SnarfCommandOutputToFP(char  *cmd, FILE  *fp)
{
    FILE *myfp;
    char LineBuf[2000], *cmdv[5];
    int pgrp, timedout;

    fprintf(fp, "Here is the output of the command '%s':\n", cmd);
    fprintf(fp, "The output is enclosed by separating lines:%s", SepLine);
    cmdv[0] = "/bin/sh";
    cmdv[1] = "-c";
    cmdv[2] = cmd;
    cmdv[3] = NULL;
    myfp = topen(cmdv[0], cmdv, "r", &pgrp);
    if (!myfp) {
	fprintf(fp, "topen failed!\n");
    } else {
	while(fgets(LineBuf, sizeof(LineBuf), myfp)) {
	    fputs(LineBuf, fp);
	}
	tclose(myfp, 20, &timedout);
	if (timedout) fputs("The tclose call timed out!\n", fp);
    }
    fputs(SepLine, fp);
}
