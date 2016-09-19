/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* type checking */
extern int SetIndent(struct rofftext  *self,int  u);
extern int SetTempIndent(struct rofftext  *self,int  u);
extern int Is_BOL(struct rofftext  *self);
extern int  Set_BOL(class rofftext *self);
extern int DestroyContext(IC  c);
extern int ung(struct rofftext  *self,int  c,Trickle  t);
extern int tpush(struct rofftext  *self,Trickle  t,const char  *filename,FILE  *f,const char  *s,boolean  push,int  argc,const char  * const argv[]);
extern int putregister(struct rofftext  *self,const char *name,int  value,enum RegFmt  fmt,int  inc,boolean  relative);
extern const char *getstring(struct rofftext  *self,const char *name);
extern int putstring(struct rofftext  *self,const char *name,const char *value);
extern int getarg(struct rofftext  *self,Trickle  t,char  *buf,int  n,boolean  copymode);
extern int put(struct rofftext  *self,int c);
extern int DoBreak(struct rofftext  *self);
extern int get(struct rofftext  *self,Trickle  t);
extern int Scan(struct rofftext  *self,Trickle  t,const char  *cmd);
