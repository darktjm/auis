/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* style code for be2roff */

extern NO_DLL_EXPORT int BeginStyle(class rofftext  *self,const char  *st);
extern NO_DLL_EXPORT void EndStyle(class rofftext  *self,int  ID);
extern NO_DLL_EXPORT int  InitText();
extern NO_DLL_EXPORT int WriteText(struct rofftext  *self);
/* MISSING DEFINITION TextPut() */
extern NO_DLL_EXPORT int  TextPut();
extern NO_DLL_EXPORT int ChangeStyle(struct rofftext  *self,int  id,const char  *st);
extern NO_DLL_EXPORT int CloseStyle(struct rofftext  *self);
extern NO_DLL_EXPORT int CloseAllStyles(struct rofftext  *self);

 
