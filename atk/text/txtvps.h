#ifndef _txtvps_H
#define _txtvps_H
/* lots and lots of definitions used by txtvps.C. Probably nothing else will use them, but who knows. */

#define textps_DefaultMargin	(90) /* in PS units */
#define textps_FootnoteGap	(18) /* in PS units */

struct textps_layout_history {
    long pagew, pageh; /* dimensions of the page in PS units */

    struct textps_layout_column **cols; /* array of columns used, in the order that they're used */
    long numcols;
    long cols_size;

    /* values that produced the initial layout plan -- we need to avoid calling these when we encounter the first layout inset, since they were called before parsing started. Kind of a hack, but it'll work. */
    void *initiallayoutrock;
    textview_getlayout_fptr initiallayoutfunc;

    /* stack of plans in use (not perfectly efficient, since if you push a plan with repeating columns, you'll never pop it -- but who cares) */
    struct textps_layout_history_unit {
	struct textps_layout_plan *plan;
	long coltyp, colnum;
	long pagebreaks;
    } *plans; 
    long numplans;
    long plans_size;

    textps_layout_history();
    ~textps_layout_history();
    void PushNewPlan(struct textps_layout_plan *newplan);
    struct textps_layout_column *NextColumn(void);
    inline struct textps_layout_history_unit *getcurplan(void) {
	if (this->numplans == 0)
	    return NULL;
	else
	    return (&this->plans[numplans-1]);
    };
};

/* all sizes and positions in the following structures are stored as integers, in units of RESOLUTION * (PS units). This is 1/720 inch at normal scaling, which is good enough. */
#define RESOLUTION 10
#define dRESOLUTION 10.0
#define NULLINT (-9999999)
#define ToPS(val) (((double)(val)) / dRESOLUTION)
#define AFMToPS(val, size) (((val) * (size)) / 1000.0)
#define MAXTABFILLLEN (8)

/* special line types */
#define lineflag_Footnote (-1) /* that horizontal line that separates footnotes from the rest of the page */

/* special word types */
#define wordflag_Tab  (-1) /* a tab */
#define wordflag_View (-2) /* a generic view (rectangular inset) */
#define wordflag_Spec (-3) /* special text-layout inset (one of enum textview_insettype) */

struct textps_word {
    int start, len; /* in this line's letters array. start may be wordflag_*, in which case it's a special "word" with no actual letters to its name. */
    long xpos;
    long width; /* not including spaces */
    int run; /* number of style run */

    /* stuff for wordflag_View: */
    struct textview_insetdata inset; /* ### ought to merge the following fields into the union, as well */
    class view *v;
    long vheight; /* view height */
    /* width serves as vwidth */
    long voff; /* portion of the view above the baseline */
};

struct textps_tmp_word {
    int start, len; /* again, may be wordflag_* */
    int run; /* number of style run */
    struct textps_style *style;
    long width; /* width of word, including spaces. (Meaning differs if special word) */
    long spaces; /* amount of space after word (Meaning differs if special word) */
    long tmppos, tmptabnum; /* temporary storage used by the line-layout loop */
    int breakable; /* 0: no; 1: possible; 2: must (end of line); 3: must (end of line and column); 4: must (end of line, column, and page); 5: must (end of file!) */

    /* stuff for wordflag_View: */
    struct textview_insetdata inset; /* ### ought to merge the following fields into the union, as well */
    class view *v;
    long deswidth, desheight; /* view's desired width, height */
    long vheight; /* view height, final value */
    /* width serves as vwidth */
    long footnotenumber; /* used in footnote words */
    long voff; /* portion of the view above the baseline */
};

struct textps_line {
    long colnum; /* column that this line belongs to */
    int lineheight; /* total */
    int baseline;  /* distance from the *top* of the lineheight */
    long clipline; /* if positive or zero, we should set up a clipping rectangle (of height clipline, RESOLUTION units, measured from the top edge of the line) before drawing this line. If negative, don't. */

    int firstword; /* this is the actual number of the first word, regardless of how many have been eaten */
    int numwords; /* may be lineflag_*, in which case words and letters are NULL. May also be 0, in which case the same may be true. */
    char *letters;
    struct textps_word *words;
};

struct textps_srunlist {
    struct textps_stylerun *r;
    int num, size;
};

struct textps_layout {
    long planid; /* layout_plan id that this layout is good for */

    int numlines;
    int lines_size;
    struct textps_srunlist runs;
    struct textps_line **lines;

    textps_layout();
    ~textps_layout();
};

struct textps_tabentry {
    long pos;
    enum style_TabAlignment kind; /* style_{{Left,Right}Aligned,CenteredOnTab} */
};
struct textps_tablist {
    int refcount;
    int listsize;
    struct textps_tabentry *list;
};

/* all measurements in RESOLUTION PS */
#define textps_style_instack 1
#define textps_style_inline 2
#define textps_style_inlinestart 4
#define textps_pushtype_None 0
#define textps_pushtype_Env 1
#define textps_pushtype_Style 2
#define textps_pushtype_View 3

struct textps_style {
    int pushtype; /* textps_pushtype_{View,Style,None} */
    class environment *env; /* this may be NULL, in which case endpos will be after the end of the lexer object */
    long endpos;
    long runnum;

    struct font_afm *afm;
    short *encoding;
    int fontindex; /* returned by RegisterFont */
    int fontsize;
    int realfontsize; /* not coerced to be positive */
    char *fontfamily;
    long leftmargin, rightmargin, paraindent;
    long baselineoffset, linespacing, paraspacing;
    enum style_Justification justification;
    long fontfaces, miscflags;
    struct textps_tablist *tabs;
    char *tabfill;

    struct textps_style *next; /* for heap list */
    int lexlevel; /* which lexer put this on the stack */
    short usage;
};

struct textps_stylerun {
    /*    stuff used in style runs*/
    struct font_afm *afm;
    short *encoding;
    int fontindex; /* returned by RegisterFont */
    int fontsize;
    long baselineoffset; /* distance from line's baseline (+ is down) */
    long miscflags; /* underline, etc */
    /* shouldn't need fontfaces */
    char tabfill[MAXTABFILLLEN+1];
    long tabfillwidth;
    struct textps_locatag *locatag;
};

/* the state of a word-slurp operation. Includes words, letters, runs, and the styleheap */
struct textps_slurp {
    struct textps_tmp_word *tmpwords;
    int tmpwords_size, numwords;
    int wordseaten;
    char *tmpletters;
    long tmpletters_size;
    long letters_pt;
    struct textps_style *curstyle;
    struct textps_style **stylestack;
    int stylestack_num, stylestack_size;
    struct textps_style *styleheap;
    struct textps_srunlist runs;

    struct textps_lexstate_text *lexers; 
    int lexernum, lexers_size;

    boolean contents; /* TRUE if content styles are to be put in locatags list */
    struct textps_locatag *locatags_root;
    struct textps_locatag **locatags;

    int footnotenumber;
    struct textps_layout_history *history;
};

#define textps_lexer_Text 1
#define textps_lexer_CharArray 2

/* everything we need to know about the state of the text object we're slurping words out of */
struct textps_lexstate_text {
    int type;
    long len, pos;
    long nextenvpos, nextenvlen;

    /* for type = textps_lexer_Text */
    class text *txt;
    class textview *txtv;

    /* for type = textps_lexer_CharArray */
    char *str;
    class style *sty;
};

struct textps_layout_state {
    char *prependstring;
    class style *prependstyle;
    struct textps_locatag *locatags;
};

struct textps_header {
    char *string[header_TEXTS];
    class text *t;
    class textview *tv;
    boolean dirty;
    short haspage; /* 3-bit field */
};

struct textps_simple_layout_state {
    struct textps_slurp *slurp;
    struct textps_layout *footnotes;
};

extern void textview_InitializePS(void);

#endif   /* _txtvps_H */
