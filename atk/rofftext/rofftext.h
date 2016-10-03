/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

enum trickle_type {
    trickle_File,
    trickle_String
};

struct trickle {
    struct _trickle *t;
};

struct _trickle {
    enum trickle_type type;
    union {
        FILE *f;
        const char *s;
    }u;
    const char *filename;
    int LineNumber;
    char *buf;
    boolean pop;
    struct _trickle *prev;
};

extern NO_DLL_EXPORT void SetIndent(struct rofftext  *self,int  u);
extern NO_DLL_EXPORT void SetTempIndent(struct rofftext  *self,int  u);
extern NO_DLL_EXPORT void  Set_BOL(class rofftext *self);
extern NO_DLL_EXPORT void ung(struct rofftext  *self,int  c,Trickle  t);
extern NO_DLL_EXPORT void tpush(struct rofftext  *self,Trickle  t,const char  *filename,FILE  *f,const char  *s,boolean  push,int  argc,const char  * const argv[]);
extern NO_DLL_EXPORT void putregister(struct rofftext  *self,const char *name,int  value,enum RegFmt  fmt,int  inc,boolean  relative);
extern NO_DLL_EXPORT const char *getstring(struct rofftext  *self,const char *name);
extern NO_DLL_EXPORT void putstring(struct rofftext  *self,const char *name,const char *value);
extern NO_DLL_EXPORT void getarg(struct rofftext  *self,Trickle  t,char  *buf,int  n,boolean  copymode);
extern NO_DLL_EXPORT void put(struct rofftext  *self,int c);
extern NO_DLL_EXPORT void DoBreak(struct rofftext  *self);
extern NO_DLL_EXPORT int get(struct rofftext  *self,Trickle  t);
extern NO_DLL_EXPORT void Scan(struct rofftext  *self,Trickle  t,const char  *cmd);
