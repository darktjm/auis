#ifndef _amsn_H_
#define _amsn_H_
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


/* Until I come up with a better scheme, new functions here have to be added to SIX files -- ams.ch, amss.ch, amsn.ch (all identical specs) and the corresponding c files */ 

#include <ams.H>
class amsn: public  ams {
public:
	virtual ATKregistryEntry *ATKregistry();

        amsn();
      
	static boolean InitializeClass();
	static boolean atk_inited;

    
       int GenericCompoundAction(class view *v, const char *prefix, const char *cmds)  ;
       int GetBooleanFromUser(const char *prompt, int defaultans)  ;
       int GetStringFromUser(const char *prompt, char *buf, int len, int IsPassword)  ;
       int ChooseFromList(const char * const *QVec, int defans)  ;
       void ReportError(const char *text, int level, int Decode, long mserrcode);
       void ReportSuccess(const char *text);
       int TildeResolve(const char *in, char *out)  ;
       int OnlyMail()  ;
       const char* ap_Shorten(const char *fname)  ;
       int fwriteallchars(char *s, int len, FILE *fp)  ;
       long MSErrCode()  ;
       int vdown(int errno)  ;
       void CUI_BuildNickName(char *shortname, char *longname);
       int CUI_CheckMailboxes(char *forwhat)  ;
       long CUI_CloneMessage(int cuid, char *DirName, int code)  ;
       long CUI_CreateNewMessageDirectory(char *dir, char *bodydir)  ;
       long CUI_DeleteMessage(int cuid)  ;
       long CUI_DeliveryType()  ;
       long CUI_DirectoriesToPurge()  ;
       long CUI_DisambiguateDir(const char *shortname, char **longname)  ;
       long CUI_DoesDirNeedPurging(char *name)  ;
       void CUI_EndConversation();
       long CUI_GenLocalTmpFileName(char *name)  ;
       long CUI_GenTmpFileName(char *name)  ;
       long CUI_GetFileFromVice(char *tmpfile, char *vfile)  ;
       long CUI_GetHeaderContents(int cuid, char *hdrname, int hdrnum, char *hdrbuf, int lim)  ;
       long CUI_GetHeaders(const char *dirname, const char *date64, char *headbuf, int lim, int startbyte, long *nbytes, long *status, int RegisterCuids)  ;
       long CUI_GetSnapshotFromCUID(int cuid, char *Sbuf)  ;
       long CUI_HandleMissingFolder(char *dname)  ;
       long CUI_Initialize(int  (*TimerFunction)(char *rock), char *rock)  ;
       long CUI_LastCallFinished()  ;
       char * CUI_MachineName()  ;
       char * CUI_MailDomain()  ;
       long CUI_MarkAsRead(int cuid)  ;
       long CUI_MarkAsUnseen(int cuid)  ;
       long CUI_NameReplyFile(int cuid, int code, char *fname)  ;
       long CUI_OnSameHost()  ;
       long CUI_PrefetchMessage(int cuid, int ReallyNext)  ;
       long CUI_PrintBodyFromCUIDWithFlags(int cuid, int flags, char *printer)  ;
       void CUI_PrintUpdates(const char *dname, const char *nickname);
       long CUI_ProcessMessageAttributes(int cuid, char *snapshot)  ;
       long CUI_PurgeDeletions(char *dirname)  ;
       long CUI_PurgeMarkedDirectories(boolean ask, boolean OfferQuit)  ;
       long CUI_ReallyGetBodyToLocalFile(int cuid, char *fname, int *ShouldDelete, int MayFudge)  ;
       long CUI_RemoveDirectory(const char *dirname)  ;
       long CUI_RenameDir(const char *oldname, const char *newname)  ;
       void CUI_ReportAmbig(const char *name, const char *atype);
       long CUI_ResendMessage(int cuid, char *tolist)  ;
       long CUI_RewriteHeaderLine(char *addr, char **newaddr)  ;
       long CUI_RewriteHeaderLineInternal(char *addr, char **newaddr, int maxdealiases, int *numfound, int *externalcount, int *format, int *strip, int *trust)  ;
       char * CUI_Rock()  ;
       void CUI_SetClientVersion(char *vers);
       long CUI_SetPrinter(char *printername)  ;
       long CUI_SnapIsRunning()  ;
       long CUI_StoreFileToVice(char *localfile, char *vicefile)  ;
       long CUI_SubmitMessage(char *infile, long DeliveryOpts)  ;
       long CUI_UndeleteMessage(int cuid)  ;
       long CUI_UseAmsDelivery()  ;
       long CUI_UseNameSep()  ;
       char * CUI_VersionString()  ;
       char* CUI_WhoIAm()  ;
       int CUI_GetCuid(char *id, char *fullname, int *isdup)  ;
       int CUI_GetAMSID(int cuid, char **id, char **dir)  ;
       long MS_AppendFileToFolder(char *filename, char *foldername)  ;
       long MS_CheckAuthentication(int *auth)  ;
       long MS_DebugMode(int mslevel, int snaplevel, int malloclevel)  ;
       long MS_DisambiguateFile(char *source, char *target, long MustBeDir)  ;
       long MS_FastUpdateState()  ;
       long MS_GetDirInfo(char *dirname, int *protcode, int *msgcount)  ;
       long MS_GetNewMessageCount(char *dirname, int *numnew, int *numtotal, char *lastolddate, int InsistOnFetch)  ;
       long MS_GetNthSnapshot(char *dirname, long which, char *snapshotbuf)  ;
       long MS_GetSearchPathEntry(long which, char *buf, long buflim)  ;
       long MS_GetSubscriptionEntry(char *fullname, char *nickname, int *status)  ;
       long MS_NameChangedMapFile(char *mapfile, int mailonly, int listall, int *numchanged, int *numunavailable, int * nummissing, int *numslowpokes, int *numfastfellas)  ;
       long MS_NameSubscriptionMapFile(char *root, char *mapfile)  ;
       long MS_ParseDate(char *indate, int *year, int *month, int *day, int *hour, int *min, int *sec, int *wday, int *gtm)  ;
       long MS_PrefetchMessage(char *dirname, char *id, long getnext)  ;
       long MS_SetAssociatedTime(char *fullname, char *newvalue)  ;
       void MS_SetCleanupZombies(long doclean);
       long MS_SetSubscriptionEntry(char *fullname, char *nickname, long status)  ;
       long MS_UnlinkFile(char *filename)  ;
       long MS_UpdateState()  ;
       long MS_MatchFolderName(char *pattern, char *filename)  ;
       long MS_DomainHandlesFormatting(char *domname, int *retval)  ;
       int AMS_ErrNo()  ;
       void SubtleDialogs(boolean besubtle);
       const char * DescribeProt(int code)  ;
       char * MessagesAutoBugAddress()  ;
       int UnScribe(int ucode, struct ScribeState **ss, char *LineBuf, int ct, FILE *fout)  ;
       int UnScribeFlush(int ucode, struct ScribeState **ss, FILE *fout)  ;
       int UnScribeInit(const char *vers, struct ScribeState **ss)  ;
       void WriteOutUserEnvironment(FILE *fp, boolean IsAboutMessages);
       char * ams_genid(boolean isfilename)  ;
       int CheckAMSUseridPlusWorks(char *dom)  ;
       void RemoveErrorDialogWindow();
};


#endif /* _amsn_H_ */
