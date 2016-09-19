/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* evalstrdata is passed as the rock to parser and lexer */
struct evalstrdata {
	int nextdbl;			/* which element of dbls to use next */
	double dbls[20];		/* pointed to from value stack */
	double dummydbl;		/* value to point to if none left */
	class rofftext *rt;		/* the rofftext being parsed */
	class text *numtext ;		/* text passed to numlex */
	class num *numparse;		/* parser */
	class tlex *numlex;		/* lexer object */
	double v_Result;		/* numeric result of parse */
	int v_DefaultScale;		/* default scaling factor */
	int *SF;			/* vector of scale values */
};

/* #ifndef AIX */
/* utilities for rofftext */
extern void Add2Buf(BUF b, int c);
extern BUF NewBuf();
extern void CreateEnvirons(struct rofftext  *self);
extern void DestroyEnvirons(struct rofftext  *self);
extern void PushEnviron(struct rofftext  *self,int  env);
extern void PopEnviron(struct rofftext  *self);
extern struct diversionLevel *CreateDiversion(struct rofftext  *self,struct diversionLevel  *c);
extern void DestroyDiversion(struct rofftext  *self,struct diversionLevel  *d);
extern void PushDiversion(struct rofftext  *self);
extern void PopDiversion(struct rofftext  *self);
extern const char *getstring(struct rofftext  *self,const char  *name);
/* #endif // AIX */

extern void FreeBuf(BUF  b);
extern void EvalString(class rofftext *self, char *str, 
	int *result, int scale, boolean *absolute, boolean *relative);
extern void InitChars(class rofftext *self);
