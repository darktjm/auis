#ifndef _rofftext_H_
#define _rofftext_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup librofftext Object rofftext
 *  A dynamically loadable ATK class file.  Contains rofftext, rofftextapp,
 *  mmtext, mantext.
 * @{ */
/* rofftext - translate troff/nroff to ATK text object
 *
 */

#include <table.H>
#include <fnote.H>
#define MAX_COLS 40
#define MAX_TAG 200
struct tag_tbl {
    char *tag;	/* tag value */
    int	def;	/* TRUE if def, FALSE if ref */
    long pos;	/* position of definition */
    class link	*l; /* for ref only */
};

/* environment:
point size
font
fill, adjust, center
line spacing, line length
indent, temporary indent
input trap
tabs
underline
control character
nobreak control character
hyphenation
title length
number mode
margin character
*/



enum RegFmt {
    reg_Single=0,     /* 0,1,2...*/
    reg_Triple,     /* 000,001,002,...*/
    reg_LCRoman,    /* 0,i,ii,iii,...*/
    reg_UCRoman,    /* 0,I,II,III,...*/
    reg_LCaz,       /* 0,a,b,c,...*/
    reg_UCaz        /* 0,A,B,C,...*/
};

struct reg {
    enum RegFmt format;
    int value;
    int autoinc;
};
typedef struct reg *Reg;

struct trickle;
typedef struct trickle *Trickle;

struct MagicBuf {
    char *begin, *end, *ptr;
    int size;
    int blocksize;
};
typedef struct MagicBuf *BUF;
#define ClearBuf(b) ((b)->ptr = (b)->begin)
#define Buf2Str(b) ((b)->begin)

struct InputContext {
    int argc;
    const char **argv;
};
typedef struct InputContext *IC;


struct trap {
    int loc;
    char *macro;
};

#define MAXARGS 24
#define NewObj(type) ((type *)malloc(sizeof(type)))

/* reserve the use of 200 - 210 */

#define NULCHAR 0200
#define ESCAPE 0201

enum scale_type {
    scale_u = 0,
    scale_i = 1,
    scale_c = 2,
    scale_P = 3,
    scale_m = 4,
    scale_n = 5,
    scale_p = 6,
    scale_v = 7,
    scale_none = 8
};

#define INDENTLEVELS 5

struct roffEnviron{
    int pointSize;
    boolean fill;
    const char *font; /* name of the font */
    const char *prevFont;
    int fontStyle; /* style ID for font */
    int indent;     /* number of basic units to indent */
    int prevIndent;
    int tempIndent; /* number of basic units for temp. indent */
    int NextInputTrap;  /* next input line to spring input trap */
    char *InputTrapCmd; /* name of input trap cmd */
    char controlChar; /* '.' */
    char NBControlChar; /* '\'' */
};

/* for debugging (obviously) */

#ifdef DEBUGGING
extern int ROFFDEBUG;
#define DEBUG(lev, arg) if (ROFFDEBUG > (lev)) fprintf arg
#else /* DEBUGGING */
#define DEBUG(lev, arg)
#endif /* DEBUGGING */


/* diversions */

struct diversionLevel {
    int NextDiversionTrap; /* line which springs next diversion trap */
    char *DiversionTrapCmd; /* command on sprung trap */
    char *name; /* diversion name */
    int level; /* diversion level */
    boolean NoSpaceMode;
    int OutputDone;
    BUF SnarfOutput;
};


struct stackelt {
    int level;
    long pos;
    class environment *env;
    int ID;
};

struct tempelt {
    int level;
    long pos;
    class style *style;
    int ID;
};


#define Newelt() ((struct stackelt *)malloc(sizeof(struct stackelt)))
#define STACKSIZE 48


#include <text.H>
class rofftext: public  text {
public:
	TRACED_CLASS(rofftext);


    
	rofftext();
    
	static boolean InitializeClass();

    
    static long ReadRoffIntoText(class text *t, FILE *fp, long pos, const char * const *resources)  ;

    virtual void SetText(class text *t); /* must be used before any styles started */

    inline void SetInputFiles(const char * const *files) { ((this)->inputfiles = files); }
    inline void SetInitialPos(long n) { ((this)->pos = n); }

     long Read(FILE *file, long id)  ;
     long Write(FILE *file, long writeID, int level)  ;
     const char * ViewName()  ;
     void SetAttributes(struct attributes *atts);

    /* global state variables */

    int v_InCond;                   /* inside conditional input */
    boolean v_TrapBlown; /* TRUE after trap is sprung */
    boolean v_CopyMode;         /* inside macro definition */
    int v_TblMode;
    char v_TblTab;
    int colWidth[MAX_COLS];
    class table *Tbl;
    int baseline;
    int basestyle;
    class fnote *Fn;
    int fnpos;
    int gc_mode;
    int out_column;
    long picBegin;
    class link *picButton;
    int tag_count;
    struct tag_tbl tags[MAX_TAG];
    boolean v_ReadEscapes;       /* escape mechanism is turned on */
    boolean v_RawMode;          /* interpret only \{  and  \} */
    boolean v_LastIfResult;      /* for .el requests */
    boolean v_BOL;        /* the last char read was beginning of line */
    boolean v_NextBOL;    /* the next char read will be beginning of line */
    boolean v_MultiSpace;
    Trickle Input;                      /* input for put() to push a command! */
    char *Output;                       /* string used to snarf output */
    char EscChar;                       /* the current escape char */
    boolean RoffType;           /* TRUE for troff, FALSE for nroff */
    char *v_Trap;                       /* trap for beginning of page */
    int v_InputLineCount;           /* non-blank input line number */
    int v_InputLineNumber;          /* real input line number */
    int v_ErrorCount;               /* number of errors for sanity check */
    int v_TempIndentTrap;               /* line number which springs temp. indent */
    boolean v_BlankLine;                 /* current line is blank */
    struct roffEnviron *CurrentEnviron; /* current environment */
    class glist *EnvironStack;
    struct roffEnviron *Environs[4]; /* environments -- last one is dummy */
    int v_DiversionLevel;
    struct diversionLevel *CurrentDiversion;
    class glist *DiversionStack;

    unsigned char Berliz[256]; /* translation table */

    /* indenting */


    int IndentStyle[INDENTLEVELS]; /* style ID's for indentation */
    char *IndentName[INDENTLEVELS]; /* name of styles for indent */
    int TempIndentStyle[INDENTLEVELS];
    char *TempIndentName[INDENTLEVELS];
    int MinusTempIndentStyle[INDENTLEVELS];
    char *MinusTempIndentName[INDENTLEVELS];


    class glist *ArgStack;
    class hash *Registers;
    class hash *Commands;
    class hash *Macros;
    class hash *Goofy; /* commands with special syntax */
    class hash *SpecialChars; /* for \(XX */
    char current;
    const char *Nullarg[10];
    IC CurrentContext;


    int ScaleFactor[8];
    /* Fonts */

    const char *Fonts[5];/* = {"roman","italic","bold","special","typewriter"};*/
    /*int CurrentFont = 0;*/
    int CurrentFontSize;


    struct stackelt *stack;
    struct tempelt *tempstack;
    long pos;
    int styleID;

    const char *macrofile;
    const char * const *inputfiles;

    boolean HelpMode;
    boolean BeCompletelyBogus;

    class glist *EndMacros;
    const char *filename;
    boolean PrintWarnings;
    class text *textm;  /* text to use for actually inserting chars and etc */
};
/** @} */
#endif /* _rofftext_H_ */
