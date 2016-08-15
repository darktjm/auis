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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/amsn.C,v 1.6 1994/04/28 19:34:30 rr2b Stab74 $";
#endif

/* Until I come up with a better scheme, new functions here have to be added to SIX files -- ams.ch, amss.ch, amsn.ch (all identical specs) and the corresponding c files */ 


#include <ams.H>
#include <im.H>
#include <message.H>
#include <amsn.H>
#include <mailconf.h>
#include <cui.h>

#include <cuiprotos.h>
#include <msprotos.h>
#include <mail.h>
#include <util.h>
#include <unscribe.h>

#include <stubs.h>

extern char ProgramName[];	/* Icky-poo! */


ATKdefineRegistry(amsn, ams, amsn::InitializeClass);
static int TimerInitFunc(char *rock);


extern char *CUI_MachineName, CUI_MailDomain[], *CUI_Rock, CUI_VersionString[], *CUI_WhoIAm   ;

extern long CUI_DeliveryType, CUI_LastCallFinished, CUI_OnSameHost, CUI_SnapIsRunning, CUI_UseAmsDelivery, CUI_UseNameSep, mserrcode;

void amsn::RemoveErrorDialogWindow()
{
    extern void ams_RemoveErrorWindow();
    ams_RemoveErrorWindow();
}

boolean amsn::InitializeClass()
{
    CheckAMSConfiguration();
    SetProgramVersion();
    ::MS_SetCleanupZombies(0);
    return(TRUE);
}

amsn::amsn() {};


void amsn::CUI_BuildNickName(char  *shortname , char  *longname)
{
    ::CUI_BuildNickName(shortname, longname);
}

int amsn::CUI_CheckMailboxes(char  *forwhat) 
{
    return(::CUI_CheckMailboxes(forwhat));
}

long amsn::CUI_CloneMessage(int  cuid, char  *DirName, int  code) 
{
    return(::CUI_CloneMessage(cuid, DirName, code));
}

long amsn::CUI_CreateNewMessageDirectory(char  *dir , char  *bodydir) 
{
    return(::CUI_CreateNewMessageDirectory(dir, bodydir));
}

long amsn::CUI_DeleteMessage(int  cuid) 
{
    return(::CUI_DeleteMessage(cuid));
}

long amsn::CUI_DeliveryType() 
{
    return(::CUI_DeliveryType);
}

long amsn::CUI_DirectoriesToPurge() 
{
    return(::CUI_DirectoriesToPurge());
}

long amsn::CUI_DisambiguateDir(char  *shortname , char  **longname) 
{
    return(::CUI_DisambiguateDir(shortname, longname));
}

long amsn::CUI_DoesDirNeedPurging(char  *name) 
{
    return(::CUI_DoesDirNeedPurging(name));
}

void amsn::CUI_EndConversation()
{
    ::CUI_EndConversation();
}

long amsn::CUI_GenLocalTmpFileName(char  *name) 
{
    return(::CUI_GenLocalTmpFileName(name));
}

long amsn::CUI_GenTmpFileName(char  *name) 
{
    return(::CUI_GenTmpFileName(name));
}

long amsn::CUI_GetFileFromVice(char  *tmp_file , char  *vfile) 
{
    return(::CUI_GetFileFromVice(tmp_file, vfile));
}

long amsn::CUI_GetHeaderContents(int  cuid, char  *hdrname, int  hdrnum, char  *hdrbuf, int  lim)
{
    return(::CUI_GetHeaderContents(cuid, hdrname, hdrnum, hdrbuf, lim));
}

long amsn::CUI_GetHeaders(char  *dirname , char  *date64 , char  *headbuf, int  lim , int  startbyte , long  *nbytes , long  *status , int  RegisterCuids)
{
    return(::CUI_GetHeaders(dirname, date64, headbuf, lim, startbyte, nbytes, status, RegisterCuids));
}

long amsn::CUI_GetSnapshotFromCUID(int  cuid, char  *Sbuf) 
{
    return(::CUI_GetSnapshotFromCUID(cuid, Sbuf));
}

long amsn::CUI_HandleMissingFolder(char  *dname) 
{
    return(::CUI_HandleMissingFolder(dname));
}


long amsn::CUI_Initialize(int  (*TimerFunction)(char *rock), char  *rock) 
{
    message::DisplayString(NULL, 10, "Initializing Internal Message Server...");
    im::ForceUpdate();
    if (!TimerFunction) TimerFunction = TimerInitFunc;
    strcpy(ProgramName, im::GetProgramName());
    return(::CUI_Initialize(TimerFunction, rock));
}

long amsn::CUI_LastCallFinished() 
{
    return(::CUI_LastCallFinished);
}

char * amsn::CUI_MachineName() 
{
    return(::CUI_MachineName);
}

char * amsn::CUI_MailDomain() 
{
    return(::CUI_MailDomain);
}

long amsn::CUI_MarkAsRead(int  cuid) 
{
    return(::CUI_MarkAsRead(cuid));
}

long amsn::CUI_MarkAsUnseen(int  cuid) 
{
    return(::CUI_MarkAsUnseen(cuid));
}

long amsn::CUI_NameReplyFile(int  cuid , int  code, char  *fname) 
{
    return(::CUI_NameReplyFile(cuid, code, fname));
}

long amsn::CUI_OnSameHost() 
{
    return(::CUI_OnSameHost);
}

long amsn::CUI_PrefetchMessage(int  cuid , int  ReallyNext) 
{
    return(::CUI_PrefetchMessage(cuid, ReallyNext));
}

long amsn::CUI_PrintBodyFromCUIDWithFlags(int  cuid , int  flags, char  *printer )
{
    return(::CUI_PrintBodyFromCUIDWithFlags(cuid, flags, printer));
}

void amsn::CUI_PrintUpdates(const char  *dname , const char  *nickname)
{
    ::CUI_PrintUpdates(dname, nickname);
}

long amsn::CUI_ProcessMessageAttributes(int  cuid, char  *snapshot) 
{
    return(::CUI_ProcessMessageAttributes(cuid, snapshot));
}

long amsn::CUI_PurgeDeletions(char  *dirname) 
{
    return(::CUI_PurgeDeletions(dirname));
}

long amsn::CUI_PurgeMarkedDirectories(boolean  ask , boolean  OfferQuit) 
{
    return(::CUI_PurgeMarkedDirectories(ask, OfferQuit));
}

long amsn::CUI_ReallyGetBodyToLocalFile(int  cuid, char  *fname, int  *ShouldDelete , int  MayFudge) 
{
    return(::CUI_ReallyGetBodyToLocalFile(cuid, fname, ShouldDelete, MayFudge));
}

long amsn::CUI_RemoveDirectory(char  *dirname)
{
    return(::CUI_RemoveDirectory(dirname));
}
long amsn::CUI_RenameDir(char  *oldname , char  *newname) 
{
    return(::CUI_RenameDir(oldname, newname));
}

void amsn::CUI_ReportAmbig(char  *name , char  *atype)
{
    ::CUI_ReportAmbig(name, atype);
}

long amsn::CUI_ResendMessage(int  cuid, char  *tolist) 
{
    return(::CUI_ResendMessage(cuid, tolist));
}

long amsn::CUI_RewriteHeaderLine(char  *addr , char  **newaddr) 
{
    return(::CUI_RewriteHeaderLine(addr, newaddr));
}

long amsn::CUI_RewriteHeaderLineInternal(char  *addr , char  **newaddr, int  maxdealiases , int  *numfound , int  *externalcount , int  *formatct , int  *stripct , int  *trustct)
{
    return(::CUI_RewriteHeaderLineInternal(addr, newaddr, maxdealiases, numfound, externalcount, formatct, stripct, trustct));
}

char *amsn::CUI_Rock()
{
    return(::CUI_Rock);
}

void amsn::CUI_SetClientVersion(char  *vers)
{
    ::CUI_SetClientVersion(vers);
}

long amsn::CUI_SetPrinter(char  *printername) 
{
    return(::CUI_SetPrinter(printername));
}

long amsn::CUI_SnapIsRunning() 
{
    return(::CUI_SnapIsRunning);
}

long amsn::CUI_StoreFileToVice(char  *localfile , char  *vicefile) 
{
    return(::CUI_StoreFileToVice(localfile, vicefile));
}

long amsn::CUI_SubmitMessage(char  *infile, long  DeliveryOpts) 
{
    return(::CUI_SubmitMessage(infile, DeliveryOpts));
}

long amsn::CUI_UndeleteMessage(int  cuid) 
{
    return(::CUI_UndeleteMessage(cuid));
}

long amsn::CUI_UseAmsDelivery() 
{
    return(::CUI_UseAmsDelivery);
}

long amsn::CUI_UseNameSep() 
{
    return(::CUI_UseNameSep);
}

char * amsn::CUI_VersionString() 
{
    return(::CUI_VersionString);
}

char* amsn::CUI_WhoIAm() 
{
    return(::CUI_WhoIAm);
}

int amsn::CUI_GetCuid(char  *id , char  *fullname, int  *isdup)
{
return(::CUI_GetCuid(id, fullname, isdup));
}

long amsn::MS_AppendFileToFolder(char  *filename , char  *foldername) 
{
    return(::MS_AppendFileToFolder(filename, foldername));
}

long amsn::MS_CheckAuthentication(int  *auth) 
{
    return(::MS_CheckAuthentication(auth));
}

long amsn::MS_DebugMode(int  mslevel , int  snaplevel , int  malloclevel) 
{
    return(::MS_DebugMode(mslevel, snaplevel, malloclevel));
}

long amsn::MS_DisambiguateFile(char  *source , char  *target, long  MustBeDir) 
{
    return(::MS_DisambiguateFile(source, target, MustBeDir));
}

long amsn::MS_FastUpdateState() 
{
    return(::MS_FastUpdateState());
}

long amsn::MS_GetDirInfo(char  *dirname, int  *protcode , int  *msgcount) 
{
    return(::MS_GetDirInfo(dirname, protcode, msgcount));
}

long amsn::MS_GetNewMessageCount(char  *dirname , int  *numnew , int  *numtotal , char  *lastolddate, int  InsistOnFetch)
{
    return(::MS_GetNewMessageCount(dirname, numnew, numtotal, lastolddate, InsistOnFetch));
}

long amsn::MS_GetNthSnapshot(char  *dirname, long  which, char  *snapshotbuf)
{
    return(::MS_GetNthSnapshot(dirname, which, snapshotbuf));
}

long amsn::MS_GetSearchPathEntry(long  which, char  *buf, long  buflim) 
{
    return(::MS_GetSearchPathEntry(which, buf, buflim));
}

long amsn::MS_GetSubscriptionEntry(char  *fullname, char  *nickname, int  *status)
{
    return(::MS_GetSubscriptionEntry(fullname, nickname, status));
}

long amsn::MS_NameChangedMapFile(char  *mapfile, int  mailonly, int  listall, int  *numchanged, int  *numunavailable, int  *nummissing, int  *numslowpokes, int  *numfastfellas)
{
    return(::MS_NameChangedMapFile(mapfile, mailonly, listall, numchanged, numunavailable, nummissing, numslowpokes, numfastfellas));
}

long amsn::MS_NameSubscriptionMapFile(char  *root, char  *mapfile) 
{
    return(::MS_NameSubscriptionMapFile(root, mapfile));
}

long amsn::MS_MatchFolderName(char  *pat, char  *filename) 
{
    return(::MS_MatchFolderName(pat, filename));
}

long amsn::MS_ParseDate(char  *indate, int  *year, int  *month, int  *day, int  *hour, int  *min, int  *sec, int  *wday, int  *gtm)
{
    return(::MS_ParseDate(indate, year, month, day, hour, min, sec, wday, gtm));
}

long amsn::MS_PrefetchMessage(char  *dirname, char  *id, long  getnext) 
{
    return(::MS_PrefetchMessage(dirname, id, getnext));
}

long amsn::MS_SetAssociatedTime(char  *fullname, char  *newvalue) 
{
    return(::MS_SetAssociatedTime(fullname, newvalue));
}

void amsn::MS_SetCleanupZombies(long  doclean)
{
    ::MS_SetCleanupZombies(doclean);
}

long amsn::MS_SetSubscriptionEntry(char  *fullname, char  *nickname, long  status)
{
    return(::MS_SetSubscriptionEntry(fullname, nickname, status));
}

long amsn::MS_UnlinkFile(char  *filename) 
{
    return(::MS_UnlinkFile(filename));
}

long amsn::MS_UpdateState()
{
    return(::MS_UpdateState());
}

void amsn::ReportSuccess(char  *s)
{
    ::ReportSuccess(s);
}


long amsn::MS_DomainHandlesFormatting(char  *domname, int  *retval)
{
    return(::MS_DomainHandlesFormatting(domname, retval));
}

void amsn::ReportError(char  *s, int  level , int  decode, long  err)
{
    if (decode) ::mserrcode = err;
    ::ReportError(s, level, decode);
}

int amsn::GenericCompoundAction(class view  *v, char  *prefix, char  *cmds)
{
    return(::GenericCompoundAction(v, prefix, cmds));
}

int amsn::GetBooleanFromUser(char  *prompt, int  defaultans)
{
    return(::GetBooleanFromUser(prompt, defaultans));
}

int amsn::GetStringFromUser(char  *prompt , char  *buf, int  len , int  ispass)
{
    return(::GetStringFromUser(prompt, buf, len, ispass));
}

int amsn::TildeResolve(const char  *in , char  *out)
{
    return(::TildeResolve(in, out));
}

int amsn::OnlyMail()
{
    return(AMS_OnlyMail);
}

const char *amsn::ap_Shorten(const char  *fname)
{
    return(::ap_Shorten(fname));
}

int amsn::fwriteallchars(char  *s, int  len, FILE  *fp)
{
    return(::fwriteallchars(s, len, fp));
}

long amsn::MSErrCode()
{
    return(::mserrcode);
}

int amsn::vdown(int  errno)
{
    return(::vdown(errno));
}

int amsn::AMS_ErrNo()
{
    return(AMS_ERRNO);
}

void amsn::SubtleDialogs(boolean  besubtle)
{
    ::SubtleDialogs(besubtle);
}

char *amsn::DescribeProt(int  code)
{
    return(::DescribeProt(code));
}

int amsn::ChooseFromList(char  **QVec, int  defans)
{
    return(::ChooseFromList(QVec, defans));
}
int amsn::CUI_GetAMSID(int  cuid, char  **id , char  **dir)
{
    return(::CUI_GetAMSID(cuid, id, dir));
}

char *amsn::MessagesAutoBugAddress()
{
    return(::MessagesAutoBugAddress);
}

int amsn::UnScribe(int  ucode, struct ScribeState  **ss, char  *LineBuf, int  ct, FILE  *fout)
{
    return(::UnScribe(ucode, ss, LineBuf, ct, fout));
}

int amsn::UnScribeFlush(int  ucode, struct ScribeState  **ss, FILE  *fout)
{
    return(::UnScribeFlush(ucode, ss, fout));
}

int amsn::UnScribeInit(char  *vers, struct ScribeState  **ss)
{
    return(::UnScribeInit(vers, ss));
}

void amsn::WriteOutUserEnvironment(FILE  *fp, boolean  IsAboutMessages)
{
    ::WriteOutUserEnvironment(fp, IsAboutMessages);
}

char *amsn::ams_genid(boolean  isfilename)
{
    return(::ams_genid(isfilename));
}

int amsn::CheckAMSUseridPlusWorks(char  *dom)
{
    return(::CheckAMSUseridPlusWorks(dom));
}

static int TimerInitFunc(char *rock) {
    ams::TimerInit();
    return 0; /* cuilib doesn't seem to use the return value for anything */
}
