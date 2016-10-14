#ifndef _eq_H_
#define _eq_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libeq Object eq
 *  A dynamically loadable ATK class file.  Contains eq, eqview.
 * @{ */
/*
 * eq.H
 * class header file for eq.
 */

/* Enums--see struct symbol below. */
enum align {HLIST, LPILE, CPILE, RPILE, MAX_align };
enum eqstyle { D_EQSTYLE, T_EQSTYLE, S_EQSTYLE, SS_EQSTYLE, MAX_eqstyle };
enum script {SUP, SUB, ABOVE, BELOW, MAX_script };
enum simple {ITALIC, ROMAN, BOLD, SYMBOL, SYMBOLA, SYM, MAX_simple };
enum genre { ORD, OP, BIN, REL, OPEN, CLOSE, PUNCT, INNER, MAX_genre };
 enum type {
	BEGIN, END,				/* control tokens */
	ALIGN,					/* set alignment type */
	EQSTYLE,					/* set eqstyle */
	SCRIPT,					/* script follows */
	SIMPLE,					/* real symbols */
	EXTEND,					/* extendables */
	MAX_type
    };

typedef void (*eq_enumfptr)(const char *alias, const char *orig, void *rock);

#include <mark.H>

#include <dataobject.H>
class eq : public  dataobject {
public:
	TRACED_CLASS(eq);


     long Read(FILE *file, long id)  ;
     long Write(FILE *file, long writeid, int level)  ;


/*
 * The array of struct formulas in an equation is contained in a
 * struct eq, which is maintained by the following
 * routines.  The data structure itself is private.
 */

    virtual void Insert(long pos, struct formula *f);
    virtual long InsertTokens(long pos, const char *s)  ;
    virtual long InsertTokensCarefully(long pos, const char *s)  ;
    virtual void Delete(long pos);
    virtual long DeleteCarefully(long start, long stop)  ;
    virtual long DoScript(long pos, enum script script, const char *string)  ;
    virtual class mark * CreateMark(long pos, long length)  ;
    virtual void RemoveMark(class mark *mark);
    virtual void Erase();
    virtual struct formula * Access(long n)  ;
    virtual long Size()  ;
    virtual struct formula * NextFormula(struct formula *f)  ;
    virtual long GetTokens(long *startp, long stop, char *string, long size)  ;   /* fix -- make static? */
    virtual long FindEndGroup(long i)  ;
    virtual long FindBeginGroup(long i)  ;
    virtual long FindLeftSibling(long i)  ;
    virtual long FindRightSibling(long i)  ;
    virtual void WriteFILE(FILE *f, long start, long stop, char sep);
    virtual long ReadFILE(FILE *f, long start)  ;
    virtual void Dump(const char *name);
    virtual void Parse(FILE *file, long whch, long *width=NULL, long *height=NULL);
    static int Lex(void *rock, void *pyylval);

    eq();

    static const struct symbol * Lookup(const char *name)  ;
    static void EnumerateSymbols(eq_enumfptr proc, void *rock);
    static boolean InitializeClass();



/*
 * An equation.  .formula points to an array of struct formulas.  This
 * array has p1 + gap + p2 entries. The first p1 elements of
 * the array contain valid information; the next gap elements
 * of the array contain garbage; and the final p2 elements of
 * the array contain the remainder of the valid information.
 * Thus p1 + p2 elements of the array are in use, and gap elements
 * of the array are free.
 */
    struct formula *f;
    short p1, gap, p2;
    class mark *markList;
    class mark *fence;
};


/*
 * Symbol table.  Each symbol has a general type (.type) and
 * a more specific type (.what).  This two-level description
 * makes certain formatting operations easier.
 */
struct symbol {
    const char *name;				/* print name */
    enum type type;
    short what;					/* should be a union of: */
	/* enum align if type is ALIGN */
	/* enum eqstyle if type is EQSTYLE */
	/* enum script if type is SCRIPT */
	/* enum simple if type is SIMPLE */
    enum genre genre;				/* spacing genre */
    const char *string;				/* for real symbols */
    long y_genre;				/* parse genre */
    const char *doc;				/* online documentation string */
};

/*
 * An equation is (conceptually) an array of struct formula.
 * Each formula contains among other things a pointer to its symbol.
 */

struct pos {
	short x, y;
 } ;
 
struct formula {
    struct pos min, max;					/* ul and lr corners */
    short sup_y, sub_y, kern;			/* script positioning info */
    struct pos posp;				/* where relative to group */
    struct pos hot;				/* absolute hot spot */
    class fontdesc *font;			/* for drawing */
    const struct symbol *symbol;		/* symbol for this formula */
    char *string;				/* string for extendables */
    boolean has_hot_spot;			/* whether dot can be here */
    boolean transparent;
    boolean deletable;
    boolean verticalExtend;			/* TRUE if it extends vertically */
};

/* Miscellaneous */
#undef NIL
#undef MAX
#undef MIN
#undef ABS

#define NIL 0
#define MAX(x,y) ((x)>(y)? (x) : (y))
#define MIN(x,y) ((x)<(y)? (x) : (y))
#define ABS(x)   ((x)>0? (x) : -(x))


/* 
 * Knuth's spacing classifications for symbols.  See The TeXbook p. 170.
 * This is in eq.ch only so that symbols.c can be separately compiled.
 */
enum space {		/* Knuth:	Description:			     */
    EQBAD,		/*  *		 Can't happen			     */
    NOSPC,		/*  0		 No space 			     */
    CTHIN,		/* (1)		 Thin space only in t and d eqstyles   */
    THIN,		/*  1		 Thin space in any eqstyle	     */
    CMED,		/* (2)		 Medium space only in t and d eqstyles */
    MED,		/*  2		 Medium space in any eqstyle	     */
    CTHICK,		/* (3)		 Thick space only in t and d eqstyles  */
    THICK,		/*  3		 Thick space in any eqstyle	     */
    MAX_space
};


/* generally useful.  Initialized in eqview_InitializeClass. */
extern const struct symbol *eq_zilch;
extern const struct symbol *eq_root;

/** @} */
#endif /* _eq_H_ */
