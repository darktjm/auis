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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/amss.C,v 1.5 1994/04/28 19:34:30 rr2b Stab74 $";
#endif

/* Until I come up with a better scheme, new functions here have to be added to SIX files -- ams.ch, amss.ch, amsn.ch (all identical specs) and the corresponding c files */ 


#include <andrewos.h>
#include <ams.H>
#include <amss.H>
#include <im.H>
#include <message.H>
#include <mailconf.h>
#include <cui.h>

#include <cuiprotos.h>
#include <msprotos.h>
#include <mail.h>
#include <util.h>
#include <unscribe.h>

#include <stubs.h>

ATKdefineRegistry(amss, ams, amss::InitializeClass);
#ifndef NORCSID
#endif
static int TimerInitFunc();


extern char *CUI_MachineName, CUI_MailDomain[], *CUI_Rock, *CUI_VersionString, *CUI_WhoIAm   ;

extern long CUI_DeliveryType, CUI_LastCallFinished, CUI_OnSameHost, CUI_SnapIsRunning, CUI_UseAmsDelivery, CUI_UseNameSep, mserrcode;

void amss::RemoveErrorDialogWindow()
{
    extern void ams_RemoveErrorWindow();
    ams_RemoveErrorWindow();
}

boolean amss::InitializeClass()
{
    CheckAMSConfiguration();
    SetProgramVersion();
    return(TRUE);
}

void amss::CUI_BuildNickName(char  *shortname , char  *longname)
{
    ::CUI_BuildNickName(shortname, longname);
}

int amss::CUI_CheckMailboxes(char  *forwhat) 
{
    return(::CUI_CheckMailboxes(forwhat));
}

long amss::CUI_CloneMessage(int  cuid, char  *DirName, int  code) 
{
    return(::CUI_CloneMessage(cuid, DirName, code));
}

long amss::CUI_CreateNewMessageDirectory(char  *dir , char  *bodydir) 
{
    return(::CUI_CreateNewMessageDirectory(dir, bodydir));
}

long amss::CUI_DeleteMessage(int  cuid) 
{
    return(::CUI_DeleteMessage(cuid));
}

long amss::CUI_DeliveryType() 
{
    return(::CUI_DeliveryType);
}

long amss::CUI_DirectoriesToPurge() 
{
    return(::CUI_DirectoriesToPurge());
}

long amss::CUI_DisambiguateDir(char  *shortname , char  **longname) 
{
    return(::CUI_DisambiguateDir(shortname, longname));
}

long amss::CUI_DoesDirNeedPurging(char  *name) 
{
    return(::CUI_DoesDirNeedPurging(name));
}

void amss::CUI_EndConversation()
{
    ::CUI_EndConversation();
}

long amss::CUI_GenLocalTmpFileName(char  *name) 
{
    return(::CUI_GenLocalTmpFileName(name));
}

long amss::CUI_GenTmpFileName(char  *name) 
{
    return(::CUI_GenTmpFileName(name));
}

long amss::CUI_GetFileFromVice(char  *tmp_file , char  *vfile) 
{
    return(::CUI_GetFileFromVice(tmp_file, vfile));
}

long amss::CUI_GetHeaderContents(int  cuid, char  *hdrname, int  hdrnum, char  *hdrbuf, int  lim)
{
    return(::CUI_GetHeaderContents(cuid, hdrname, hdrnum, hdrbuf, lim));
}

long amss::CUI_GetHeaders(char  *dirname , char  *date64 , char  *headbuf, int  lim , int  startbyte , long  *nbytes , long  *status , int  RegisterCuids)
{
    return(::CUI_GetHeaders(dirname, date64, headbuf, lim, startbyte, nbytes, status, RegisterCuids));
}

long amss::CUI_GetSnapshotFromCUID(int  cuid, char  *Sbuf) 
{
    return(::CUI_GetSnapshotFromCUID(cuid, Sbuf));
}

long amss::CUI_HandleMissingFolder(char  *dname) 
{
    return(::CUI_HandleMissingFolder(dname));
}


long amss::CUI_Initialize(int  (*TimerFunction)(char *rock), char  *rock)
{

    message::DisplayString(NULL, 10, "Initializing Connection to Message Server...");
    im::ForceUpdate();
    if (!TimerFunction) TimerFunction = TimerInitFunc;
    return(::CUI_Initialize(TimerFunction, rock));
}

long amss::CUI_LastCallFinished() 
{
    return(::CUI_LastCallFinished);
}

char * amss::CUI_MachineName() 
{
    return(::CUI_MachineName);
}

char * amss::CUI_MailDomain() 
{
    return(::CUI_MailDomain);
}

long amss::CUI_MarkAsRead(int  cuid) 
{
    return(::CUI_MarkAsRead(cuid));
}

long amss::CUI_MarkAsUnseen(int  cuid) 
{
    return(::CUI_MarkAsUnseen(cuid));
}

long amss::CUI_NameReplyFile(int  cuid , int  code, char  *fname) 
{
    return(::CUI_NameReplyFile(cuid, code, fname));
}

long amss::CUI_OnSameHost() 
{
    return(::CUI_OnSameHost);
}

long amss::CUI_PrefetchMessage(int  cuid , int  ReallyNext) 
{
    return(::CUI_PrefetchMessage(cuid, ReallyNext));
}

long amss::CUI_PrintBodyFromCUIDWithFlags(int  cuid , int  flags, char  *printer )
{
    return(::CUI_PrintBodyFromCUIDWithFlags(cuid, flags, printer));
}

void amss::CUI_PrintUpdates(char  *dname , char  *nickname)
{
    ::CUI_PrintUpdates(dname, nickname);
}

long amss::CUI_ProcessMessageAttributes(int  cuid, char  *snapshot) 
{
    return(::CUI_ProcessMessageAttributes(cuid, snapshot));
}

long amss::CUI_PurgeDeletions(char  *dirname) 
{
    return(::CUI_PurgeDeletions(dirname));
}

long amss::CUI_PurgeMarkedDirectories(boolean  ask , boolean  OfferQuit) 
{
    return(::CUI_PurgeMarkedDirectories(ask, OfferQuit));
}

long amss::CUI_ReallyGetBodyToLocalFile(int  cuid, char  *fname, int  *ShouldDelete , int  MayFudge) 
{
    return(::CUI_ReallyGetBodyToLocalFile(cuid, fname, ShouldDelete, MayFudge));
}

long amss::CUI_RemoveDirectory(char  *dirname)
{
    return(::CUI_RemoveDirectory(dirname));
}
long amss::CUI_RenameDir(char  *oldname , char  *newname) 
{
    return(::CUI_RenameDir(oldname, newname));
}

void amss::CUI_ReportAmbig(char  *name , char  *atype)
{
    ::CUI_ReportAmbig(name, atype);
}

long amss::CUI_ResendMessage(int  cuid, char  *tolist) 
{
    return(::CUI_ResendMessage(cuid, tolist));
}

long amss::CUI_RewriteHeaderLine(char  *addr , char  **newaddr) 
{
    return(::CUI_RewriteHeaderLine(addr, newaddr));
}

long amss::CUI_RewriteHeaderLineInternal(char  *addr , char  **newaddr, int  maxdealiases , int  *numfound , int  *externalcount , int  *formatct , int  *stripct , int  *trustct)
{
    return(::CUI_RewriteHeaderLineInternal(addr, newaddr, maxdealiases, numfound, externalcount, formatct, stripct, trustct));
}

char *amss::CUI_Rock()
{
    return(::CUI_Rock);
}

void amss::CUI_SetClientVersion(char  *vers)
{
    ::CUI_SetClientVersion(vers);
}

long amss::CUI_SetPrinter(char  *printername) 
{
    return(::CUI_SetPrinter(printername));
}

long amss::CUI_SnapIsRunning() 
{
    return(::CUI_SnapIsRunning);
}

long amss::CUI_StoreFileToVice(char  *localfile , char  *vicefile) 
{
    return(::CUI_StoreFileToVice(localfile, vicefile));
}

long amss::CUI_SubmitMessage(char  *infile, long  DeliveryOpts) 
{
    return(::CUI_SubmitMessage(infile, DeliveryOpts));
}

long amss::CUI_UndeleteMessage(int  cuid) 
{
    return(::CUI_UndeleteMessage(cuid));
}

long amss::CUI_UseAmsDelivery() 
{
    return(::CUI_UseAmsDelivery);
}

long amss::CUI_UseNameSep() 
{
    return(::CUI_UseNameSep);
}

char * amss::CUI_VersionString() 
{
    return(::CUI_VersionString);
}

char* amss::CUI_WhoIAm() 
{
    return(::CUI_WhoIAm);
}

int amss::CUI_GetCuid(char  *id , char  *fullname, int  *isdup)
{
    return(::CUI_GetCuid(id, fullname, isdup));
}

long amss::MS_AppendFileToFolder(char  *filename , char  *foldername) 
{
    return(::MS_AppendFileToFolder(filename, foldername));
}

long amss::MS_CheckAuthentication(int  *auth) 
{
    return(::MS_CheckAuthentication(auth));
}

long amss::MS_DebugMode(int  mslevel , int  snaplevel , int  malloclevel) 
{
    return(::MS_DebugMode(mslevel, snaplevel, malloclevel));
}

long amss::MS_DisambiguateFile(char  *source , char  *target, long  MustBeDir) 
{
    return(::MS_DisambiguateFile(source, target, MustBeDir));
}

long amss::MS_FastUpdateState() 
{
    return(::MS_FastUpdateState());
}

long amss::MS_GetDirInfo(char  *dirname, int  *protcode , int  *msgcount) 
{
    return(::MS_GetDirInfo(dirname, protcode, msgcount));
}

long amss::MS_GetNewMessageCount(char  *dirname , int  *numnew , int  *numtotal , char  *lastolddate, int  InsistOnFetch)
{
    return(::MS_GetNewMessageCount(dirname, numnew, numtotal, lastolddate, InsistOnFetch));
}

long amss::MS_GetNthSnapshot(char  *dirname, long  which, char  *snapshotbuf)
{
    return(::MS_GetNthSnapshot(dirname, which, snapshotbuf));
}

long amss::MS_GetSearchPathEntry(long  which, char  *buf, long  buflim) 
{
    return(::MS_GetSearchPathEntry(which, buf, buflim));
}

long amss::MS_GetSubscriptionEntry(char  *fullname, char  *nickname, int  *status)
{
    return(::MS_GetSubscriptionEntry(fullname, nickname, status));
}

long amss::MS_NameChangedMapFile(char  *mapfile, int  mailonly, int  listall, int  *numchanged, int  *numunavailable, int  *nummissing, int  *numslowpokes, int  *numfastfellas)
{
    return(::MS_NameChangedMapFile(mapfile, mailonly, listall, numchanged, numunavailable, nummissing, numslowpokes, numfastfellas));
}

long amss::MS_NameSubscriptionMapFile(char  *root, char  *mapfile) 
{
    return(::MS_NameSubscriptionMapFile(root, mapfile));
}

long amss::MS_MatchFolderName(char  *pat, char  *filename) 
{
    return(::MS_MatchFolderName(pat, filename));
}

long amss::MS_ParseDate(char  *indate, int  *year, int  *month, int  *day, int  *hour, int  *min, int  *sec, int  *wday, int  *gtm)
{
    return(::MS_ParseDate(indate, year, month, day, hour, min, sec, wday, gtm));
}

long amss::MS_PrefetchMessage(char  *dirname, char  *id, long  getnext) 
{
    return(::MS_PrefetchMessage(dirname, id, getnext));
}

long amss::MS_SetAssociatedTime(char  *fullname, char  *newvalue) 
{
    return(::MS_SetAssociatedTime(fullname, newvalue));
}

void amss::MS_SetCleanupZombies(long  doclean)
{
    ::MS_SetCleanupZombies(doclean);
}

long amss::MS_SetSubscriptionEntry(char  *fullname, char  *nickname, long  status)
{
    return(::MS_SetSubscriptionEntry(fullname, nickname, status));
}

long amss::MS_UnlinkFile(char  *filename) 
{
    return(::MS_UnlinkFile(filename));
}

long amss::MS_UpdateState()
{
    return(::MS_UpdateState());
}

long amss::MS_DomainHandlesFormatting(char  *domname, int  *retval)
{
    return(::MS_DomainHandlesFormatting(domname, retval));
}

void amss::ReportSuccess(char  *s)
{
    ::ReportSuccess(s);
}


void amss::ReportError(char  *s, int  level , int  decode, long  err)
{
    if (decode) ::mserrcode = err;
    ::ReportError(s, level, decode);
}


int amss::GenericCompoundAction(class view  *v, char  *prefix, char  *cmds)
{
    return(::GenericCompoundAction(v, prefix, cmds));
}

int amss::GetBooleanFromUser(char  *prompt, int  defaultans)
{
    return(::GetBooleanFromUser(prompt, defaultans));
}

int amss::GetStringFromUser(char  *prompt , char  *buf, int  len , int  ispass)
{
    return(::GetStringFromUser(prompt, buf, len, ispass));
}

int amss::TildeResolve(const char  *in , char  *out)
{
    return(::TildeResolve(in, out));
}

int amss::OnlyMail()
{
    return(AMS_OnlyMail);
}

char *amss::ap_Shorten(char  *fname)
{
    return(::ap_Shorten(fname));
}

int amss::fwriteallchars(char  *s, int  len, FILE  *fp)
{
    return(::fwriteallchars(s, len, fp));
}

long amss::MSErrCode()
{
    return(::mserrcode);
}

int amss::vdown(int  errno)
{
    return(::vdown(errno));
}

int amss::AMS_ErrNo()
{
    return(AMS_ERRNO);
}

void amss::SubtleDialogs(boolean  besubtle)
{
    ::SubtleDialogs(besubtle);
}

char *amss::DescribeProt(int  code)
{
    return(::DescribeProt(code));
}

int amss::ChooseFromList(char  **QVec, int  defans)
{
    return(::ChooseFromList(QVec, defans));
}
int amss::CUI_GetAMSID(int  cuid, char  **id , char  **dir)
{
    return(::CUI_GetAMSID(cuid, id, dir));
}

char *amss::MessagesAutoBugAddress()
{
    return(::MessagesAutoBugAddress);
}

int amss::UnScribe(int  ucode, struct ScribeState  **ss, char  *LineBuf, int  ct, FILE  *fout)
{
    return(::UnScribe(ucode, ss, LineBuf, ct, fout));
}

int amss::UnScribeFlush(int  ucode, struct ScribeState  **ss, FILE  *fout)
{
    return(::UnScribeFlush(ucode, ss, fout));
}

int amss::UnScribeInit(char  *vers, struct ScribeState  **ss)
{
    return(::UnScribeInit(vers, ss));
}

void amss::WriteOutUserEnvironment(FILE  *fp, boolean  IsAboutMessages)
{
    ::WriteOutUserEnvironment(fp, IsAboutMessages);
}

char *amss::ams_genid(boolean  isfilename)
{
    return(::ams_genid(isfilename));
}

int amss::CheckAMSUseridPlusWorks(char  *dom)
{
    return(::CheckAMSUseridPlusWorks(dom));
}

static int TimerInitFunc() {
    ams::TimerInit();
    return 0; /* cuilib doesn't seem to use the return value for anything */
}
