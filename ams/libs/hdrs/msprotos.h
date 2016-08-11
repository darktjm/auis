/* C++ified by magic !@#%&@#$ */

#ifndef _msprotos_H_
#define _msprotos_H_ 1

#include <atkproto.h>
BEGINCPLUSPLUSPROTOS

/* C++ prototypes for functions in message server */

int MS_AppendFileToFolder(char  *FileName , char *FolderName);
int MS_CheckAuthentication(int  *Authenticated);
int MS_DebugMode(int  level , int  snap , int  malloc);
int MS_DisambiguateFile(char  *source , char  *target, int  AccessCode);
int MS_FastUpdateState();
long    MS_GetDirInfo (char    *DirName, int     *ProtCode, int     *MsgCount);
int MS_GetNewMessageCount(char  *FullDirName , int  *numnew , int  *numtotal , char  *LastOldDate, int  InsistOnFetch);
int MS_GetNthSnapshot(char  *DirName , int  n, char  *SnapshotBuf);
int MS_GetSearchPathEntry(int      which, char    *buf, int      lim);
int MS_GetSubscriptionEntry(char  *FullName , char  *NickName , int  *status );
int MS_NameChangedMapFile(char  *MapFile , int  MailOnly , int  ListAll  , int  *NumChanged , int  *NumUnavailable , int  *NumMissingFolders , int  *NumSlowpokes , int  *NumFastFellas );
int MS_NameSubscriptionMapFile(char  *Root  , char  *MapFile );
long MS_MatchFolderName(char  *pat , char  *filename);
int MS_ParseDate(char  *indate, int  *year , int  *month , int  *day , int  *hour , int  *min , int  *sec , int  *wday , int  *gtm);
int MS_PrefetchMessage(char  *DirName , char  *id, int  GetNext);
int MS_SetAssociatedTime(char  *FullName , char  *newvalue);
int MS_SetCleanupZombies(int  value);
int MS_SetSubscriptionEntry(char  *FullName , char  *NickName , int  status );
long    MS_UnlinkFile (char    *FileName);
int MS_UpdateState();
int MS_DomainHandlesFormatting(char  *domname , int  *codeP);

ENDCPLUSPLUSPROTOS
 
#endif /* _msprotos_H_ */
