/* C++ prototypes for C++ functions in stubs.C */

#ifndef _STUBS_H_
#define _STUBS_H_ 1

#include <atkproto.h>
BEGINCPLUSPLUSPROTOS

void SubtleDialogs(int  Really);
int ChooseFromList(const char  * const *QVec, int  def);
int HandleTimeout(const char  *name, int  retries , int  restarts);
void DidRestart();
extern void SetTerminalParams(int  h,int  w);
void SubscriptionChangeHook(char  *name , char  *nick, int  status, class messages  *mess);
void DirectoryChangeHook(char  *adddir , char  *deldir, class messages  *mess);
void SetProgramVersion();
void WriteOutUserEnvironment(FILE  *fp, Boolean  IsAboutMessages);
int TildeResolve(const char  *old , char  *new_c);
void ReportError(const char  *text, int  level, int  Decode);
void RealReportError(const char  *text, int  level, int  Decode);
void ReportFailure(const char  *text , const char  *moretext, int  fmask);
void ReportSuccessNoLogging(const char  *text);
void ReportSuccess(const char  *text);
void RealReportSuccess(const char  *text);
int GenericCompoundAction(class view  *v, const char  *prefix , const char  *orgcmds);
Boolean GetBooleanFromUser(const char  *prompt, int  defaultans);
int GetStringFromUser(const char    *prompt , char    *buf, int  len , int  IsPassword);
char *BalancedQuote(const char  *qstring);
void GetSeparators(char  *cmds , const char  **argsep , char  **cmdsep);
const char * DescribeProt(int  ProtCode);
void SnarfCommandOutputToFP(const char  *cmd, FILE  *fp);

ENDCPLUSPLUSPROTOS

#endif /* _STUBS_H_ */
