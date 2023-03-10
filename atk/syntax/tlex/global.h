/* global.h  -  global variables for gentlex */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/

#ifndef boolean
#define boolean char
#define TRUE 1
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

/* severity values */
#define WARNING 1
#define ERROR	2
#define FATAL	3


/* add to the list of recognizers for processing purposes */
/* (see list of names in output.c) */
#define tlex_RESWD 8
#define tlex_GLOBAL 9
#define tlex_ERRHAND 10


struct symbol {
	const char *s;
	int n;
};


/* A struct line value is created for each non-trivial line of the .tlx
	an enum linetype value distinguishes the lines:
*/
enum linetype {
	Hdr, 	/* "tokenclass" */
	Elt, 	/* type var value */
	C, 	/* a C code line */
	Thong	/* value stored for a thong in the Action array */
};
struct line {
	int lineno;		/* linenumber */
	struct line *next;	/* if in a linked list of lines */
	enum linetype type;
	union {
		/* Hdr */
		struct {
			struct line *decls;
			struct line *Ccode;
			int tokennumber;
			char subtoken;		/* disambiguating letter */
			int recognizer; 	/* tlex_XXXX */
			char *structname;	/* gensym */
				/* for defaults: lineno will be zero,
				   decls and structname will be NULL */
			int action;	/* flag & value for actions array */
		} h;
		/* Elt */
		struct {
			const char *type;
			const char *var;
			const char *val;
			int toknum;	/* for action variables */
		} d;
		/* C */
		struct {
			char *text;	   /* line with newline */
		} c;
		/* Thong - dummy for Action array */
		struct {
			char *thong;
			struct line *hdr;
			int index;		/* location in output array */
		} g;
	} u;
};


/* gentlex.c */
extern struct line *Classes;		/* list of all Hdr lines */
extern struct line *Actions[256];	/* action for each char
						may be Hdr or Thong line */
extern char *Prefix;			/* prefix identifiers */
extern char *tlxFile;			/* name of .tlx file */
extern boolean linenos;			/* TRUE if #line lines are output */
extern struct line *GlobalHandler;
extern struct line *ErrorHandler;
extern struct line *ResWordHandler;
extern int MaxSeverity;   /* maximum severity passed to Error() */
extern int CurrSym;	  /* integer portion of next symbol from GenSym() */
extern int LineNo;	  /* current input file line number */
void Error(int  severity, const char  *msg);
void ErrorA(int  severity, const char  *msg, const char  *Arg);
char * GenSym();
char * freeze(const char  *sx , const char  *ex);
char * Escapify(const char  *s, int  *plen);

/* readtabc.c */
/* 'TokenNames' has all the token names parsed from ftabc
		each name is terminated with \0
	numtokens is the number of tokens (YYNTOKENS)
	Token[i] is a pointer to the i'th token name in TokenNames
*/
extern char *TokenNames;
extern int numtokens;
extern char **Token;
int TransEscape(char  *buf, int  *plen);
void ReadTabc(FILE  *f);
char * ScanToken(char  *cx, char  *sub);
int  ScanTokenToNumber(char  *cx, char  *sub);

/* readtlx.c */
void ReadTlx(FILE  *f);

/* defaults.c */
extern struct line *ResWords;
void ComputeDefaults();

/* output.c */
void WriteTlc(FILE  *fout, char  *fname);

/* thongs.c */
void ThongAdd(const char  *thong, struct line  *action, boolean  fromset);
void ThongReplaceNulls(struct line  *hdr);
void ThongOut(FILE  *f);
int ThongAction(struct line  *thong);

/* charset.c */
char * CharsetParse(char  *s);
char * CharsetValue(char  *set);
void CharsetOutputArrays(FILE  *f);

