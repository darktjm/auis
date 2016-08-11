/* C++ prototypes for C++ functions in stubs.C */

#ifndef _STUBS_H_
#define _STUBS_H_ 1

#include <atkproto.h>
BEGINCPLUSPLUSPROTOS

void SubtleDialogs(int  Really);
int ChooseFromList(char  **QVec, int  def);
int HandleTimeout(char  *name, int  retries , int  restarts);
void DidRestart();
extern void SetTerminalParams(int  h,int  w);
void SubscriptionChangeHook(char  *name , char  *nick, int  status, class messages  *mess);
void DirectoryChangeHook(char  *adddir , char  *deldir, class messages  *mess);
void SetProgramVersion();
void WriteOutUserEnvironment(FILE  *fp, Boolean  IsAboutMessages);
int TildeResolve(char  *old , char  *new_c);
void ReportError(char  *text, int  level, int  Decode);
void RealReportError(char  *text, int  level, int  Decode);
void ReportFailure(char  *text , char  *moretext, int  fmask);
void ReportSuccessNoLogging(char  *text);
void ReportSuccess(char  *text);
void RealReportSuccess(char  *text);
int GenericCompoundAction(class view  *v, char  *prefix , char  *orgcmds);
Boolean GetBooleanFromUser(char  *prompt, int  defaultans);
int GetStringFromUser(char    *prompt , char    *buf, int  len , int  IsPassword);
char *BalancedQuote(char  *qstring);
void GetSeparators(char  *cmds , char  **argsep , char  **cmdsep);
char * DescribeProt(int  ProtCode);
void SnarfCommandOutputToFP(char  *cmd, FILE  *fp);

ENDCPLUSPLUSPROTOS

#endif /* _STUBS_H_ */
