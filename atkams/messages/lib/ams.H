#ifndef _ams_H_
#define _ams_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

/* conversion notes: the AMS_ErrNo method used to be called AMS_ERRNO. MSErrCode used to be mserrcode. */


/* Until I come up with a better scheme, new functions here have to be added to SIX files -- ams.ch, amss.ch, amsn.ch (all identical specs) and the corresponding c files */ 

#include <ATK.H>
class ams : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      static void Focus(class view *v);
      static class frame * InstallInNewWindow(class view *v, char *programname, char *windowname, int width, int ht, class view *focusv)  ;
      static void SetWantSnap(int wantsnap);
      static class ams * GetAMS()  ;
      static void SetCheckpointFrequency(int freq);
      static void AddCheckpointCaption(class captions *c);
      static void AddCheckpointBodies(class t822view *b);
      static void AddCheckpointFolder(class folders *f);
      static void AddCheckpointSendmessage(class sendmessage *s);
      static void RemoveCheckpointCaption(class captions *c);
      static void RemoveCheckpointBodies(class t822view *b);
      static void RemoveCheckpointFolder(class folders *f);
      static void RemoveCheckpointSendmessage(class sendmessage *s);
      static void PlanFolderPrefetch(class folders *f);
      static void TimerInit();
      static void WaitCursor(boolean IsWait);
      static void SubscriptionChangeHook(char *name, char * nick, int status, class messages *mess);
      static void DirectoryChangeHook(char *adddir, char *deldir, class messages *mess);
      static int InitializeClassList()  ;
      static void ResetClassList();
      static char * GetMailPath()  ;
      static char * GetFirstMailClass()  ;
      static int TryDelayedUpdates()  ;
      static void CacheDelayedUpdate(char *fullname, char *updatedate);
      static int GetLastMenuClass()  ;
      static char* GetClassListEntry(int which)  ;
      static int GetClassListCount()  ;
      static void SetCUIRock(long r);
      static void CommitState(boolean DoQuit, boolean MustQuit, boolean MayPurge, boolean OfferQuit);
      static int CountAMSViews()  ;
      static int GetFolderName(char *prompt, char *buf, int buflen, char *defaultname, boolean MustMatch)  ;
      static class ams *MakeAMS();
      
      virtual int GenericCompoundAction(class view *v, char *prefix, char *cmds)  ;
      virtual int GetBooleanFromUser(char *prompt, int defaultans)  ;
      virtual int GetStringFromUser(char *prompt, char *buf, int len, int IsPassword)  ;
      virtual int ChooseFromList(char **QVec, int defans)  ;
      virtual void ReportError(char *text, int level, int Decode, long mserrcode);
      virtual void ReportSuccess(char *text);
      virtual int TildeResolve(const char *in, char *out)  ;
      virtual int OnlyMail()  ;
      virtual char* ap_Shorten(char *fname)  ;
      virtual int fwriteallchars(char *s, int len, FILE *fp)  ;
      virtual long MSErrCode()  ;
      virtual int vdown(int Errno)  ;
      virtual void CUI_BuildNickName(char *shortname, char *longname);
      virtual int CUI_CheckMailboxes(char *forwhat)  ;
      virtual long CUI_CloneMessage(int cuid, char *DirName, int code)  ;
      virtual long CUI_CreateNewMessageDirectory(char *dir, char *bodydir)  ;
      virtual long CUI_DeleteMessage(int cuid)  ;
      virtual long CUI_DeliveryType()  ;
      virtual long CUI_DirectoriesToPurge()  ;
      virtual long CUI_DisambiguateDir(char *shortname, char **longname)  ;
      virtual long CUI_DoesDirNeedPurging(char *name)  ;
      virtual void CUI_EndConversation();
      virtual long CUI_GenLocalTmpFileName(char *name)  ;
      virtual long CUI_GenTmpFileName(char *name)  ;
      virtual long CUI_GetFileFromVice(char *tmpfile, char *vfile)  ;
      virtual long CUI_GetHeaderContents(int cuid, char *hdrname, int hdrnum, char *hdrbuf, int lim)  ;
      virtual long CUI_GetHeaders(char *dirname, char *date64, char *headbuf, int lim, int startbyte, long *nbytes, long *status, int RegisterCuids)  ;
      virtual long CUI_GetSnapshotFromCUID(int cuid, char *Sbuf)  ;
      virtual long CUI_HandleMissingFolder(char *dname)  ;
      virtual long CUI_Initialize(int  (*TimerFunction)(char *rock), char *rock)  ;
      virtual long CUI_LastCallFinished()  ;
      virtual char * CUI_MachineName()  ;
      virtual char * CUI_MailDomain()  ;
      virtual long CUI_MarkAsRead(int cuid)  ;
      virtual long CUI_MarkAsUnseen(int cuid)  ;
      virtual long CUI_NameReplyFile(int cuid, int code, char *fname)  ;
      virtual long CUI_OnSameHost()  ;
      virtual long CUI_PrefetchMessage(int cuid, int ReallyNext)  ;
      virtual long CUI_PrintBodyFromCUIDWithFlags(int cuid, int flags, char *printer)  ;
      virtual void CUI_PrintUpdates(const char *dname, const char *nickname);
      virtual long CUI_ProcessMessageAttributes(int cuid, char *snapshot)  ;
      virtual long CUI_PurgeDeletions(char *dirname)  ;
      virtual long CUI_PurgeMarkedDirectories(boolean ask, boolean OfferQuit)  ;
      virtual long CUI_ReallyGetBodyToLocalFile(int cuid, char *fname, int *ShouldDelete, int MayFudge)  ;
      virtual long CUI_RemoveDirectory(char *dirname)  ;
      virtual long CUI_RenameDir(char *oldname, char *newname)  ;
      virtual void CUI_ReportAmbig(char *name, char *atype);
      virtual long CUI_ResendMessage(int cuid, char *tolist)  ;
      virtual long CUI_RewriteHeaderLine(char *addr, char **newaddr)  ;
      virtual long CUI_RewriteHeaderLineInternal(char *addr, char **newaddr, int maxdealiases, int *numfound, int *externalcount, int *format, int *strip, int *trust)  ;
      virtual char * CUI_Rock()  ;
      virtual void CUI_SetClientVersion(char *vers);
      virtual long CUI_SetPrinter(char *printername)  ;
      virtual long CUI_SnapIsRunning()  ;
      virtual long CUI_StoreFileToVice(char *localfile, char *vicefile)  ;
      virtual long CUI_SubmitMessage(char *infile, long DeliveryOpts)  ;
      virtual long CUI_UndeleteMessage(int cuid)  ;
      virtual long CUI_UseAmsDelivery()  ;
      virtual long CUI_UseNameSep()  ;
      virtual char * CUI_VersionString()  ;
      virtual char* CUI_WhoIAm()  ;
      virtual int CUI_GetCuid(char *id, char *fullname, int *isdup)  ;
      virtual int CUI_GetAMSID(int cuid, char **id, char **dir)  ;
      virtual long MS_AppendFileToFolder(char *filename, char *foldername)  ;
      virtual long MS_CheckAuthentication(int *auth)  ;
      virtual long MS_DebugMode(int mslevel, int snaplevel, int malloclevel)  ;
      virtual long MS_DisambiguateFile(char *source, char *target, long MustBeDir)  ;
      virtual long MS_FastUpdateState()  ;
      virtual long MS_GetDirInfo(char *dirname, int *protcode, int *msgcount)  ;
      virtual long MS_GetNewMessageCount(char *dirname, int *numnew, int *numtotal, char *lastolddate, int InsistOnFetch)  ;
      virtual long MS_GetNthSnapshot(char *dirname, long which, char *snapshotbuf)  ;
      virtual long MS_GetSearchPathEntry(long which, char *buf, long buflim)  ;
      virtual long MS_GetSubscriptionEntry(char *fullname, char *nickname, int *status)  ;
      virtual long MS_NameChangedMapFile(char *mapfile, int mailonly, int listall, int *numchanged, int *numunavailable, int * nummissing, int *numslowpokes, int *numfastfellas)  ;
      virtual long MS_NameSubscriptionMapFile(char *root, char *mapfile)  ;
      virtual long MS_ParseDate(char *indate, int *year, int *month, int *day, int *hour, int *min, int *sec, int *wday, int *gtm)  ;
      virtual long MS_PrefetchMessage(char *dirname, char *id, long getnext)  ;
      virtual long MS_SetAssociatedTime(char *fullname, char *newvalue)  ;
      virtual void MS_SetCleanupZombies(long doclean);
      virtual long MS_SetSubscriptionEntry(char *fullname, char *nickname, long status)  ;
      virtual long MS_UnlinkFile(char *filename)  ;
      virtual long MS_UpdateState()  ;
      virtual long MS_MatchFolderName(char *pattern, char *filename)  ;
      virtual long MS_DomainHandlesFormatting(char *domname, int *retval)  ;
      virtual int AMS_ErrNo()  ;
      virtual void SubtleDialogs(boolean besubtle);
      virtual char * DescribeProt(int code)  ;
      virtual char * MessagesAutoBugAddress()  ;
      virtual int UnScribe(int ucode, struct ScribeState **ss, char *LineBuf, int ct, FILE *fout)  ;
      virtual int UnScribeFlush(int ucode, struct ScribeState **ss, FILE *fout)  ;
      virtual int UnScribeInit(char *vers, struct ScribeState **ss)  ;
      virtual void WriteOutUserEnvironment(FILE *fp, boolean IsAboutMessages);
      virtual char * ams_genid(boolean isfilename)  ;
      virtual int CheckAMSUseridPlusWorks(char *dom)  ;
      virtual void RemoveErrorDialogWindow();
};

#endif /* _ams_H_ */
