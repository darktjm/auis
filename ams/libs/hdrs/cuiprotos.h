/* C++ified by magic !@#%&@#$ */

#ifndef _cuiprotos_H_
#define _cuiprotos_H_ 1

#include <atkproto.h>
BEGINCPLUSPLUSPROTOS

/* C++ prototypes for functions in cuilib */

int CUI_BuildNickName(char    *FullName, char    *NickName);
int CUI_CheckMailboxes(char    *ForWhat);
int CUI_CloneMessage(int  cuid , char  *OrigDirName, int  Code);
int CUI_CreateNewMessageDirectory(char    *dir, char    *bodydir );
int CUI_DeleteMessage(int  cuid);
int CUI_DirectoriesToPurge();
long CUI_DisambiguateDir(const char    *shortname, char   **longname);
int CUI_DoesDirNeedPurging(char  *Dname);
int CUI_GenLocalTmpFileName(char    *nmbuf);
int CUI_GenTmpFileName(char    *nmbuf);
int CUI_GetAMSID(int  cuid, char   **id, char   **dir);
int CUI_GetFileFromVice(char  *LocalFile , char  *ViceFile);
int CUI_GetHeaderContents(int  cuid, char    *HeaderName, int  HeaderTypeNumber, char    *HeaderBuf, int  lim);
long CUI_GetHeaders(const char    *DirName, const char    *date64, char    *headbuf, int  limit , long  startbyte, long  *nbytes, long  *status, int  RegisterCuids);
int CUI_GetSnapshotFromCUID(int  cuid, char    *SnapshotBuf);
int CUI_HandleMissingFolder(char  *OldName);
long CUI_Initialize(int  (*TimerFunction)(char *rock), char  *rock);
int CUI_MarkAsRead(int  cuid);
int CUI_MarkAsUnseen(int  cuid);
int CUI_NameReplyFile(int  cuid, int  code, char    *FileName);
int CUI_PrefetchMessage(int  cuid , int  ReallyNext);
int CUI_PrintBodyFromCUIDWithFlags(int  cuid, int  flags, char  *printer);
int CUI_PrintUpdates(const char  *dname , const char  *nickname);
int CUI_ProcessMessageAttributes(int  cuid, char  *Snapshot);
int CUI_PurgeDeletions(char   *arg);
int CUI_PurgeMarkedDirectories(Boolean  Ask , Boolean  OfferQuit);
int CUI_ReallyGetBodyToLocalFile(int  cuid, char  *FileName, int  *ShouldDelete, int  MayFudge);
int CUI_RemoveDirectory(const char  *DirName);
int CUI_RenameDir(const char  *old , const char  *new_c);
int CUI_ReportAmbig(const char  *name , const char  *atype);
int CUI_ResendMessage(int  cuid, char  *Tolist);
int CUI_RewriteHeaderLine(char  *text , char  **newtext);
int CUI_RewriteHeaderLineInternal(char  *text , char  **newtext, int  maxdealiases , int  *numfound , int  *externalct , int  *formatct , int  *stripct , int  *trustct);
int CUI_SetClientVersion(char  *Vers);
int CUI_SetPrinter(char  *printername);
int CUI_StoreFileToVice(char  *LocalFile , char  *ViceFile);
int CUI_SubmitMessage(char    *InFile, int  DeliveryOpts);
int CUI_UndeleteMessage(int  cuid);

void CUI_EndConversation(); /* this is defined either in cuisnap.c or nosnap.c, but the prototype is the same. */
int CUI_GetCuid(char *amsid, char *dirname, int *IsDup);
ENDCPLUSPLUSPROTOS
 
#endif /* _cuiprotos_H_ */
