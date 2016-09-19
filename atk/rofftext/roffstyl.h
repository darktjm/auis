/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* style code for be2roff */

extern int BeginStyle(class rofftext  *self,const char  *st);
extern void EndStyle(class rofftext  *self,int  ID);
extern int  InitText();
extern int WriteText(struct rofftext  *self);
/* MISSING DEFINITION TextPut() */
extern int  TextPut();
extern int ChangeStyle(struct rofftext  *self,int  id,const char  *st);
extern int CloseStyle(struct rofftext  *self);
extern int CloseAllStyles(struct rofftext  *self);

 
