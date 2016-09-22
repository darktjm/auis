/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* type checking */
extern NO_DLL_EXPORT int SetIndent(struct rofftext  *self,int  u);
extern NO_DLL_EXPORT int SetTempIndent(struct rofftext  *self,int  u);
extern NO_DLL_EXPORT int Is_BOL(struct rofftext  *self);
extern NO_DLL_EXPORT int  Set_BOL(class rofftext *self);
extern NO_DLL_EXPORT int DestroyContext(IC  c);
extern NO_DLL_EXPORT int ung(struct rofftext  *self,int  c,Trickle  t);
extern NO_DLL_EXPORT int tpush(struct rofftext  *self,Trickle  t,const char  *filename,FILE  *f,const char  *s,boolean  push,int  argc,const char  * const argv[]);
extern NO_DLL_EXPORT int putregister(struct rofftext  *self,const char *name,int  value,enum RegFmt  fmt,int  inc,boolean  relative);
extern NO_DLL_EXPORT const char *getstring(struct rofftext  *self,const char *name);
extern NO_DLL_EXPORT int putstring(struct rofftext  *self,const char *name,const char *value);
extern NO_DLL_EXPORT int getarg(struct rofftext  *self,Trickle  t,char  *buf,int  n,boolean  copymode);
extern NO_DLL_EXPORT int put(struct rofftext  *self,int c);
extern NO_DLL_EXPORT int DoBreak(struct rofftext  *self);
extern NO_DLL_EXPORT int get(struct rofftext  *self,Trickle  t);
extern NO_DLL_EXPORT int Scan(struct rofftext  *self,Trickle  t,const char  *cmd);
