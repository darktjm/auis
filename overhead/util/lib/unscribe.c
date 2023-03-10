/**********************************************************************\
 * 	Copyright IBM Corporation 1988,1995 - All Rights Reserved
 * 	For full copyright information see:'andrew/doc/COPYRITE'    
\**********************************************************************/

/*
	unscribe.c -- Remove Andrew datastream formatting.

Call UnScribeInit at the start, UnScribe for each bufferfull of text, and
UnScribeFlush at the end.

Function PrintQuotingFormatting can be used to take unformatted text 
(e.g. headers you are forwarding) and quote it past a given
version of the datastream interpretation.  

*/

#include <andrewos.h> /* strings.h */
#include <stdio.h>
#include <ctype.h>
#include <util.h>
#include <unscribe.h>

/* #define SPECIALFACES 1 */

#define Version1			0
#define Version2			1
#define Version10			10
#define Version12			12

#define v1StateInit			0
#define v1StateSeenAt		1

#define v2StateInit     		0
#define v2StateSeenAt		1
#define v2StateSeenNL		2
#define v2StateSeen2NL		3

#define v10StateInit   		0
#define v10StateSeenBS		1   /* BackSlash */
#define v10StateInKeyword   	2
#define v10StateNukeStyle   	3
#define v10StateBSview		4   /* process after \view */
#define v10StateDropChar   	5
#define v10StateSeenNL		6   /* Newline */
#define v10StateSeen2NL		7   /* Second newline */
#define v10State2Deep		8   /* begindatacount >=2 */
#define v10State2DeepSeenBS	9
#define v10State2DeepInKeyword	10
#define v12StateSeenSpc		11
#define v12StateSeenSpcNL   	12
#define v10StateBeginData   	13  /* within {} after begindata */
#define v10StateDataObj    	14  /* within embedded datobject */

#define LINEMAX  72
#define LINEFUDGE  7
#define MAXNETMAILLINE 255 /* Things longer than this will be 
					crudely wrapped by unscribe */
#define MAXLONGLINE 1000 /* ... unless in a literal style, which will 
					wrap lines at this length */
#define TABINTERVAL 8

#define INDENT_NOCHANGE (-LINEMAX)
#define JUSTIFY_NOCHANGE -1
#define JUSTIFY_LEFT 0
#define JUSTIFY_CENTER 1
#define JUSTIFY_RIGHT 2
#define PLAIN_SAME INDENT_NOCHANGE, 0, JUSTIFY_NOCHANGE

/* This macro exists to avoid a function call on every character */
#define ADDCHAR(State, fp, chr) do {\
    if ((State)->lmargin + SLEN(&(State)->linefrag) \
	+ (State)->specials > (State)->rmargin) \
		WriteCount += WriteFrag((State), (fp), (chr)); \
    else { \
	if (SLEN(&(State)->linefrag) == 0) StartFrag(State); \
	SCHAPP(&(State)->linefrag, (chr)); \
    } \
} while(0)


struct USString {
	char *s;
	int used;	/* length of string, excluding trailing \0 */
	int avail;	/* one less than size of *s */
};

struct ScribeState {
    int	statecode;		/* current state */
    int	previous_state;	/* last state, before \ processing */
    int 	writecount;		/* # chars written on current line */

    /* The following are only used by Version 10 and higher */
    int	begindatacount;	/* depth in dataobject tree. */
    struct USString linefrag;	/* Line buffer. */
    struct USString keyword;	/* Keyword buffer. */
    struct USString tempbuf;	/* Buffer for {} contents. Used to name views */
    short lmargin;	/* left margin of current line */
    short rmargin;	/* right margin of current line */
    short justify;	/* justification for this line */
    short indent;	/* indentation for this line */
    short specials;	/* count of extra characters needed at end of line */
    short face;		/* face state from previous line */
    short newpara;    /* flag: indicates that the first line of the paragraph is yet to be printed */
    struct StateVector *vector;	/* Stack of style states kept as a linked list */

    /* fields for dataobject processing */
    const char *((*dobjrcvr)(struct ScribeState *, char));	/* function to process characters */
    long oid;  		/* object id from \begindata */
    short sawcomma;  	/* used in v10StateBeginData */
    int rcvrstate;  	/* secondary state maintained by rcvr */
    void *objdata;  	/* pointer to struct for the object */

};

struct StateVector {
    short lmargin;
    short rmargin;
    short indent;     /* how much to indent (outdent if neg) the first line */
    short face;       /* 0=plain, 1=italics, 2=bold */
    short justify;    /*-1=don't change, 0=leftj, 1=center, 2=rightj */
    short quotedepth;
    short style;
    short newpara;    /* flag: T iff 1st line of para is yet to be printed */
    struct StateVector *next;
};

static struct styletable {
    const char *name;
    long hash; /* initialized by Init */
    struct StateVector vec;
} Styles[] =
{
    { "define",		0, { 0,	0, 0, 0, 0, 0,} },	/* 1st pseudo-style */
    { "template",   	0, { 0, 0, 0, 0, 0, 0,} },
    { "textdsversion",	0, { 0, 0, 0, 0, 0, 0,} },
    { "enddata",   	0, { 0, 0, 0, 0, 0, 0,} },
    { "dsversion", 	0, { 0, 0, 0, 0, 0, 0,} },
    { "begindata",	0, { 0, 0, 0, 0, 0, 0,} },
    { "view",		0, { 0,	0, 0, 0, 0, 0,} },	/* last pseudo-style */
    { "literal",       	0, { 0, MAXLONGLINE, 0, 0, 0, 0,} },    /* no fmting */
    { "italic",		0, { 0, 0, INDENT_NOCHANGE,
				 1, JUSTIFY_NOCHANGE, 0,} },
    { "bold",		0, { 0, 0, INDENT_NOCHANGE,
				 2, JUSTIFY_NOCHANGE, 0,} },
    { "chapter",   	0, { 0, 0, 0, 2, JUSTIFY_LEFT, 0,} },
    { "section",    	0, { 0, 0, 0, 2, JUSTIFY_LEFT, 0,} },
    { "subsection",	0, { 0, 0, 0, 2, JUSTIFY_LEFT, 0,} },
    { "paragraph",	0, { 0, 0, 0, 1, JUSTIFY_LEFT, 0,} },
    { "bigger",		0, { 0, 0, PLAIN_SAME, 0,} },
    { "indent",		0, { 4, 4, 0, 0, JUSTIFY_NOCHANGE, 0,} },
    { "typewriter",	0, { 0, 0, PLAIN_SAME, 0,} },
    { "display",    	0, { 4, 4, 0, 0, JUSTIFY_NOCHANGE, 0,} },
    { "example",   	0, { 4, 0, 0, 0, JUSTIFY_NOCHANGE, 0,} },
    { "itemize",    	0, { 4, 0, 0, 0, JUSTIFY_NOCHANGE, 0,} },
    { "description",	0, { 4, 0,-4, 0, JUSTIFY_NOCHANGE, 0,} },
    { "enumerate",	0, { 4, 0, 0, 0, JUSTIFY_NOCHANGE, 0,} },
    { "programexample", 0, { 4, 0, 0, 0, JUSTIFY_NOCHANGE, 0,} },
    { "quotation",	0, { 0, 0, PLAIN_SAME, 1,} },
    { "subscript", 	0, { 0, 0, PLAIN_SAME, 0,} },
    { "superscript",	0, { 0, 0, PLAIN_SAME, 0,} },
    { "smaller",    	0, { 0, 0, PLAIN_SAME, 0,} },
    { "heading",    	0, { 0, 0, 0, 1, JUSTIFY_LEFT, 0,} },
    { "majorheading",	0, { 0, 0, 0, 0, JUSTIFY_CENTER, 0,} },
    { "formatnote",	0, { 0, 0, 0, 0, JUSTIFY_LEFT, 0,} },
    { "subheading",	0, { 0, 0, 0, 2, JUSTIFY_LEFT, 0,} },
    { "center",		0, { 0, 0, 0, 0, JUSTIFY_CENTER, 0,} },
    { "flushleft",  	0, { 0, 0, 0, 0, JUSTIFY_LEFT, 0,} },
    { "flushright",	0, { 0, 0, 0, 0, JUSTIFY_RIGHT, 0,} },
    { "underline",	0, { 0, 0, INDENT_NOCHANGE,
				 1, JUSTIFY_NOCHANGE, 0,} },
    { "leftindent",	0, { 4, 0, 0, 0, JUSTIFY_NOCHANGE, 0,} },
    { "verbatim",  	0, { 0, MAXLONGLINE, 0, 0, 0, 0,} },
    { "inputoutput",	0, { 4, MAXLONGLINE, 0, 0, 0, 0,} },
    { "syntaxexample",	0, { 4, MAXLONGLINE, 0, 0, 0, 0,} },
    { "caption",    	0, { 0, 0, 0, 0, JUSTIFY_CENTER, 0,} },
    { 0,   		0, { 0, 0, 0, 0, 0, 0,} }
};


/* Inset table - how to treat insets  */

static void DummyInit(struct ScribeState *state), gofigInit(struct ScribeState *state);
static const char *DummyRcvr(struct ScribeState *state, char ch), *gofigRcvr(struct ScribeState *state, char ch);

struct InsetS {
	const char *dobjname;
	void (*dobjinit)(struct ScribeState *);
	const char *((*dobjrcvr)(struct ScribeState *, char));
};

static const struct InsetS InsetTable[] = {
	{"gofig", gofigInit, gofigRcvr},
	{NULL, DummyInit, DummyRcvr}
};



/* flexible-string functions */

/* SINIT initializes struct USString varables to empty */
#define SINIT(sptr) {(sptr)->used=(sptr)->avail=0; (sptr)->s=NULL;}

/*SFREE will deallocate the USString */
#define SFREE(sptr) {if ((sptr)->s) free((sptr)->s); SINIT(sptr);}

/* STEXT and SLEN return the character string and current length, resp. */
#define STEXT(sptr) ((sptr)->s)
#define SLEN(sptr)  ((sptr)->used)

/* SCLEAR sets the queue to empty */
#define SCLEAR(sptr) ((sptr)->used = 0, ((sptr)->s) ? (*((sptr)->s) = '\0') : 0)

/* append a string to a struct USString */
#define SAPPEND(sptr, text)  \
	{char c; const char *tp = (text); while ((c = *(tp)++)) SCHAPP((sptr), c);}

/* SCHAPP appends a character to a struct USString */
#define SCHAPP(sptr, ch) {  \
	if ((sptr)->used >= (sptr)->avail)   \
		schappend((sptr), (ch));          \
	else {char *where = &(sptr)->s[(sptr)->used++];   \
		*where++  = (ch);  *where = '\0';  }}

/*  append a character to a struct USString
*/
	static void
schappend(struct USString *sptr, char ch)
{
	if ( ! sptr->s) {
		sptr->s = malloc(sptr->avail += 100);
		sptr->used = 0;
	}
	else if (sptr->used >= sptr->avail) 
		sptr->s = realloc(sptr->s, sptr->avail += 200);
	if ( ! sptr->s) {   /* alloc failed */
		SINIT(sptr);   /* BOGUS xxx */
		fprintf(stderr, "malloc failed in unscribe.c\n");
	}
	else SCHAPP(sptr, ch);
}



/* Return the VersionXXX value corresponding to 'val', 
	or -1 if there's no match. 
*/
	static int 
usVersion(const char *val)
{
    int numVal;
    const char *Src;

    while (*val != '\0' && strchr(" \t\r\n", *val) != NULL) ++val;
    if (ULstrcmp(val, "Yes") == 0) {
	return Version1;
    } else if (ULstrcmp(val, "2") == 0) {
	return Version2;
    } else if (ULstrcmp(val, "10") == 0) {
	return Version10;
    } else if (ULstrcmp(val, "12") == 0) {
	return Version12;	/* ***Check next clause when adding a new type*** */
    } else {
	numVal = 0;	/* Interpret any type >= 12 as Version12 */
	for (Src = val; *Src != '\0'; ++Src) if (! isdigit(*Src)) {numVal = 1; break;}
	if (numVal == 0) {
		numVal = atoi(val);
		if (numVal >= 12) return Version12;
	}
	return -1;		/* don't understand header value */
    }
}

	static long
hash(const char *str)
{
    unsigned long h = 0;
    unsigned long g;

    for ( ; *str; str++) {
	h = (h << 4) + *str;
	if ((g = h & 0xf0000000)) {
	    h = h ^ (g >> 24);
	    h = h ^ g;
	}
    }
    return h;
}

	static const struct styletable *
findstyle(const char *str)
{
    const struct styletable *stp;
    long strhash;

    strhash = hash(str);
    for (stp = Styles; stp->name; stp++) {
	if (strhash == stp->hash && !strcmp(stp->name, str)) return stp;
    }
    return 0;
}


/* values for state->rcvrstate */
#define RcvrInitial 	0   /* process most chars */
#define RcvrBS  	1   /* process 1st char after \ */
#define RcvrInKeywd	2   /* process keyword after \ */
#define RcvrDataLine	3   /* accumulate a line of input */
#define RcvrError		9   /* skip to \enddata */

/* The first character a receiver will see is the newline 
after the \begindata{...}.  The last character it must read is the
left brace '{' after \enddata. */


/* initialize dummy rcvr */
	static void
DummyInit(struct ScribeState *state)
{
	state->rcvrstate = RcvrInitial;
}

/* dummy rcvr - just discard chars (and keep track of nesting) */
	static const char *
DummyRcvr(struct ScribeState *state, char ch)
{
	switch(state->rcvrstate) {
	case RcvrInitial:
		if (ch == '\\')
			state->rcvrstate = RcvrBS;
		break;
	case RcvrBS:
		/* process 1st char after \ */
		switch (ch) {
		case '\\':  
		case '\n':
		case '{':
		case '}':
			state->rcvrstate = RcvrInitial;
			break;
		default: 
			/* 1st char in keyword */
			SCLEAR(&state->keyword);
			SCHAPP(&state->keyword, ch);
			state->rcvrstate = RcvrInKeywd;
			break;
		}
		break;
	case RcvrInKeywd:
		switch(ch) {
		case '{':
			state->rcvrstate = RcvrInitial;
			if (strcmp("begindata", STEXT(&state->keyword)) == 0)
				state->begindatacount ++;
			else if (strcmp("enddata",STEXT(&state->keyword))==0) {
				state->begindatacount --;
				if (state->begindatacount < 2)
					return "";  /* finish with no result */
			}
			break;
		case '}': case '\n': 
			/* error in keyword */
			state->rcvrstate = RcvrInitial;
			break;
		case '\\':
			/* error in keyword */
			state->rcvrstate = RcvrBS;
			break;
		default:
			SCHAPP(&state->keyword, ch);
			break;
		}
		break;
	}
	return NULL;
}


/* GOFIG dataobject receiver */

#define LEFTedge 1
#define RIGHTedge 2
#define TOPedge 4
#define BOTTOMedge 8

struct gofigS {
	int width, height, edges;
	char grid[25][25];		/* board places */
	struct USString Xat, Oat;	/* where X(,O) pieces appear */
		/* Entries are made in Xat and Oat each time a stone 
		should appear in the grid with some other mark.  
		That other mark is put in the grid and entered in
		either Xat or Oat, depending on what color stone is needed.
		The entry is a character if 126 or less, and is 128+i
		for the integer i. */
};

static void gofigAppendMark(struct USString *str, int entry)
{
	if (entry & 0x80) {
		char buf[5];
		sprintf(buf, "%d", entry & 0x7F);
		SAPPEND(str, buf);
	}
	else if (entry != '.')
		SCHAPP(str, entry);
}

/* free up storage malloc'ed for this data object scan
*/
	static void
gofigFree(struct ScribeState *state)
{
	struct gofigS *gd = (struct gofigS *)state->objdata;
	SFREE(&gd->Oat);
	SFREE(&gd->Xat);
	free(state->objdata);
}

/* enter error state
*/
	static char *
gofigError(struct ScribeState *state)
{
	state->rcvrstate = RcvrError;
	SCLEAR(&state->tempbuf);
	return NULL;
}

	static void
gofigInit(struct ScribeState *state)
{
	struct gofigS *gd = (struct gofigS *)malloc(sizeof (struct gofigS));
	SINIT(&gd->Xat);
	SINIT(&gd->Oat);
	state->objdata = (void *)gd;

	SCLEAR(&state->tempbuf);	/* collect 1st line */
	state->rcvrstate = RcvrInitial;
}

	static const char *
gofigRcvr(struct ScribeState *state, char ch)
{
	int v, w, h, p, e,   r, c, inx;
	char piece, mark;
	struct gofigS *gd = (struct gofigS *)state->objdata;

	switch(state->rcvrstate) {
	case RcvrInitial:
		if (ch == '\\') {
			gofigError(state);
			SCHAPP(&state->tempbuf, ch);
			break;
		}
		if (ch != '\n') {
			SCHAPP(&state->tempbuf, ch);
			break;
		}
		if (SLEN(&state->tempbuf) == 0) 
			break;	/* newline after \begindata{...}  */
		/* we have the newline at the end of the header line */
		if (sscanf(STEXT(&state->tempbuf), " %d %d %d %d %d ",
				&v, &w, &h, &p, &e) != 5)
			return gofigError(state);

		if (v != 1 || w < 1 || w > 25 || h < 1 || h > 25 
				|| p < 2000 || p > 200000 || e < 0 || e > 31) 
			return gofigError(state);

		gd->width = w;  gd->height = h;  gd->edges = e;
		for (r = 0; r < h; r++)
			for (c = 0; c < w; c++)
				gd->grid[r][c] = ' ';

		state->rcvrstate = RcvrDataLine;
		SCLEAR(&state->tempbuf);
		break;

	case RcvrDataLine:
		switch (ch) {
		default:     	/* absorb character */
			SCHAPP(&state->tempbuf, ch);
			break;
		case '\n':	/* end of line; process the line */
			if (sscanf(STEXT(&state->tempbuf), " %d %d %c %d %c ",
					&r, &c, &piece, &inx, &mark) != 5) 
				return gofigError(state);
			if (r < 0 || r >= gd->height || c < 0 || c >= gd->width
					|| (piece != 'W' && piece != 'B' 
						&& piece != '/')
					|| (mark&0x80) || inx > 127)
				return gofigError(state);
			SCLEAR(&state->tempbuf);
			if (inx == 0 && mark == '.')
				/* no mark or integer:  put piece in grid */
				switch (piece) {
				case 'W':  gd->grid[r][c] = 'O';   break;
				case 'B':  gd->grid[r][c] = 'X';   break;
				}
			else {
				/* mark or integer: put it in grid
					and append piece to Oat or Xat */
				gd->grid[r][c] = 
					(inx != 0) ? (inx|0x80) : mark;
				switch (piece) {
				case 'W': 
					SCHAPP(&gd->Oat, gd->grid[r][c]);
					break;
				case 'B': 
					SCHAPP(&gd->Xat, gd->grid[r][c]);
					break;
				}
			}
			break;

		case '\\':  {  /* must be \enddata;  create result string */
			int j, braces;
			char *oat, *xat;
			SCLEAR(&state->tempbuf);
			if (gd->edges&TOPedge) {
				if (gd->edges&LEFTedge) 
					SCHAPP(&state->tempbuf, '+');
				for (j = 2*gd->width - 1; j>0; j--)
					SCHAPP(&state->tempbuf, '-');
				if (gd->edges&RIGHTedge) 
					SCHAPP(&state->tempbuf, '+');
				SCHAPP(&state->tempbuf, '\n');
			}
			for (r = 0; r < gd->height; r++) {
				if (gd->edges&LEFTedge) 
					SCHAPP(&state->tempbuf, '|');
				for (c = 0; c < gd->width-1; c++) {
					gofigAppendMark(&state->tempbuf, 
						gd->grid[r][c]);
					SCHAPP(&state->tempbuf, ' ');
				}
				gofigAppendMark(&state->tempbuf, 
					gd->grid[r][c]);
				if (gd->edges&RIGHTedge)
					SCHAPP(&state->tempbuf, '|');
				SCHAPP(&state->tempbuf, '\n');
			}
			if (gd->edges&BOTTOMedge) {
				if (gd->edges&LEFTedge) 
					SCHAPP(&state->tempbuf, '+');
				for (j = 2*gd->width - 1; j>0; j--)
					SCHAPP(&state->tempbuf, '-');
				if (gd->edges&RIGHTedge) 
					SCHAPP(&state->tempbuf, '+');
				SCHAPP(&state->tempbuf, '\n');
			}

			xat = STEXT(&gd->Xat);
			oat = STEXT(&gd->Oat);
			braces = ((xat && *xat) || (oat && *oat));
			if (braces)  SCHAPP(&state->tempbuf, '<');
			if (xat && *xat) {
				SAPPEND(&state->tempbuf, "X at ");
				while (*xat) {
					gofigAppendMark(&state->tempbuf, 
						*xat++);
					if (*xat) SCHAPP(&state->tempbuf, ' ');
				}
				if (oat && *oat)
					SAPPEND(&state->tempbuf, ";  ");
			}
			if (oat && *oat) {
				SAPPEND(&state->tempbuf, "O at ");
				while (*oat) {
					gofigAppendMark(&state->tempbuf, 
						*oat++);
					if (*oat) SCHAPP(&state->tempbuf, ' ');
				}
			}
			if (braces)  SAPPEND(&state->tempbuf, ">\n");

			gofigFree(state);
		}	return strdup(STEXT(&state->tempbuf));
		} /* end switch(ch) */
		break;
	case RcvrError: 
		switch (ch) {
		case '{':
			if (strcmp(STEXT(&state->tempbuf), "\\enddata") != 0) {
				/* not enddata */
				SCLEAR(&state->tempbuf);
				break;
			}
			/* is enddata for error */
			SCLEAR(&state->tempbuf);
			SAPPEND(&state->tempbuf, 
					"[A go diagram at this point was invalid ");
			SAPPEND(&state->tempbuf, 
					"and could not be converted to ASCII.]");
			gofigFree(state);

			return strdup(STEXT(&state->tempbuf));
		case '\n':
			SCLEAR(&state->tempbuf);
			break;
		default:
			SCHAPP(&state->tempbuf, ch);
		}
		break;
	} /* end of switch (rcvrstate) */
	return NULL;
}	


 /* UnScribeInit(fieldvalue, refstate)
	Pass in the value of the X-Andrew-ScribeFormat: header to see if the
	package can handle this format.  Returns a code value >= 0 for OK, < 0
	for error.  Remember this code value to pass to UnScribe.  In addition,
	pass the address of an integer variable to hold the UnScribe state between
	calls.  This variable will be initialized with the UnScribeInit call.
 */
	int 
UnScribeInit(const char *fieldvalue, struct ScribeState **refstate)
{
   
    int Vers;

    /* Initialize keyword table, but only once */
    if (Styles->hash == 0) {
	struct styletable *stp;

	for (stp = Styles; stp->name; stp++) stp->hash = hash(stp->name);
    }

    *refstate = (struct ScribeState *) malloc(sizeof(struct ScribeState));
    if (!*refstate) return -2;
    (*refstate)->begindatacount = 0;
    SINIT(&(*refstate)->linefrag);;
    SINIT(&(*refstate)->keyword);;
    SINIT(&(*refstate)->tempbuf);
    (*refstate)->vector = NULL;
    Vers = usVersion(fieldvalue);
    switch (Vers) {
	case Version1:
	    (*refstate)->statecode = v1StateInit;	/* initialize it */
	    break;
	case Version2:
	    (*refstate)->statecode = v2StateInit;
	    break;
	case Version10:
	case Version12:
	    (*refstate)->statecode = v10StateInit;
	    (*refstate)->lmargin = 0;
	    (*refstate)->rmargin = LINEMAX;
	    (*refstate)->indent = 0;
	    (*refstate)->justify = JUSTIFY_LEFT;
	    (*refstate)->specials = 0;
	    (*refstate)->face = 0;
	    (*refstate)->newpara = 1;
	    (*refstate)->vector = (struct StateVector *) malloc(sizeof(struct StateVector));
	    if ((*refstate)->vector == NULL) {
		free(*refstate);
		return -2;
	    }
	    (*refstate)->vector->next = NULL;
	    (*refstate)->vector->lmargin = 0;
	    (*refstate)->vector->rmargin = LINEMAX;
	    (*refstate)->vector->indent = 0;
	    (*refstate)->vector->justify = JUSTIFY_LEFT;
	    (*refstate)->vector->face = 0;
	    (*refstate)->vector->newpara = 1;
	    (*refstate)->vector->quotedepth = 0;
	    break;
	default:
	    free(*refstate);
	    *refstate = NULL;
	    return -1;		/* don't understand header value */
    }
    return Vers;
}


	static void
StartFrag(struct ScribeState *State)
{
#ifdef SPECIALFACES
    int oldfaces;
    int newfaces;
#endif /* SPECIALFACES */

    /* set current state from stack */
    State->lmargin = State->vector->lmargin;
    State->rmargin = State->vector->rmargin;
    State->justify = State->vector->justify;
    State->indent = State->vector->indent;

#ifdef SPECIALFACES
    oldfaces = State->face & State->vector->face;
    newfaces = State->vector->face ^ oldfaces;

    /* incorporate specials into start of string */
    if (oldfaces & 1) SCHAPP(&State->linefrag,  '_');
    if (oldfaces & 2) SCHAPP(&State->linefrag, '*');
/*
    if (oldfaces & 4) SCHAPP(&State->linefrag, '-');
    if (oldfaces & 8) SCHAPP(&State->linefrag, '+');
*/

    if (newfaces & 1) SCHAPP(&State->linefrag, '_');
    if (newfaces & 2) SCHAPP(&State->linefrag, '*');
/*
    if (newfaces & 4) SCHAPP(&State->linefrag, '-');
    if (newfaces & 8) SCHAPP(&State->linefrag, '+');
*/
#endif /* SPECIALFACES */
}


	static int
WriteFrag(struct ScribeState *State, FILE *fPtr, char Chr)
{
    int	NumWritten = 1;	/* at least a NewLine */
    char *tail, *frag;
#ifdef TRIMTRAILINGWHITESPACE
    char *c;
#endif /* TRIMTRAILINGWHITESPACE */
    int i;
    int len;
    int width;
    int quotedepth;

    if (Chr == '\n' && SLEN(&State->linefrag) == 0) {
	putc('\n', fPtr);
	State->newpara = 1;  /* Yes, this is a new paragraph */
	return 1;
    } 

    /* do we really _have_ to wrap it? */
    if (Chr != '\n' && State->newpara && 
		State->lmargin + SLEN(&State->linefrag) + State->specials 
			< State->rmargin + LINEFUDGE ) {
	/* if we're outputting the one and only line of a paragraph,
	   we can let the line grow a little longer. */
	SCHAPP(&State->linefrag, Chr);
	return 0;
    } /* if (State->newpara ... ) */

    /* find a break point in the string */
    frag = STEXT(&State->linefrag);
    len = SLEN(&State->linefrag);
    if (Chr != '\n') {
	tail = frag + MIN(len,
		(State)->rmargin - (State)->lmargin - (State)->specials + 1);

	while (tail-- > frag && *tail != ' ')  {}
	if (*tail != ' ') {
				/* couldn't find a break point */
	    tail = frag+len;
	} else {
				/* found a break point */
	    *tail++ = '\0';
	    len = tail - frag - 1;
	}
    } 
    else 
	tail = frag + len;

#ifdef TRIMTRAILINGWHITESPACE
    /* remove trailing white space */
    for (c = frag + len; *--c == ' '; len--) ;
    *++c = '\0';
#endif /* TRIMTRAILINGWHITESPACE */

    /* quote marks :-) */
    /* putting quote mark handling here means that quotes appear at the 
	left margin only--no more in-line quotes */
    for (quotedepth = State->vector->quotedepth; 
		quotedepth; 
		quotedepth--, NumWritten++) 
	putc('>', fPtr);
    if (State->vector->quotedepth) {
	putc(' ', fPtr);
	NumWritten++;
    }

    /* left margin */
    if (State->newpara) 
	for (i = State->lmargin + State->indent; i; i--, NumWritten++) 
		putc(' ', fPtr);
    else 
	for (i = State->lmargin; i; i--, NumWritten++) 
		putc(' ', fPtr);

    /* justification */
    width = State->rmargin - State->lmargin + 1;
    len += State->specials;
    if (State->justify == JUSTIFY_CENTER) i = (width - len) / 2;
    else if (State->justify == JUSTIFY_RIGHT) i = width - len;
    else i = 0;	    /* default to JUSTIFY_LEFT */
    while ((i--) > 0) { putc(' ', fPtr); NumWritten++; }

    /* shove out the actual text */
    if (len) {
	fputs(frag, fPtr);

#ifdef SPECIALFACES
/*
	if (State->vector->face & 8) putc('+', fPtr);
	if (State->vector->face & 4) putc('-', fPtr);
*/
	if (State->vector->face & 2) putc('*', fPtr);
	if (State->vector->face & 1) putc('_', fPtr);
#endif /* SPECIALFACES */
	NumWritten += len;
    }
    putc('\n', fPtr);
    SCLEAR(&State->linefrag);
	/* marginally BOGUS: 'tail' still points to part of STEXT(linefrag) */
    State->face = State->vector->face;
    if (Chr != '\n') {
	State->newpara = 0;		/* no longer */
	StartFrag(State);
	SAPPEND(&State->linefrag, tail);  /* use BOGUS */
	SCHAPP(&State->linefrag, Chr);
    } else {
	State->newpara = 1;  /* Yes, this is a new paragraph */
    }

    return NumWritten;
}


	static int
HandleKeyword(struct ScribeState *State, FILE *fPtr)
{
    const struct styletable *style;
    short index = -1;
    struct StateVector *vec;
    int WriteCount = 0;
#ifdef SPECIALFACES
    unsigned int bitvec;
#endif /* SPECIALFACES */

    style = findstyle(STEXT(&State->keyword));
    if (style) index = style - Styles;

    switch (index) {
	case -1:    /* unknown keyword, ignore */
	    State->statecode = State->previous_state;
	    vec = (struct StateVector *) malloc(sizeof(struct StateVector));
	    *vec = *(State->vector);
	    vec->next = State->vector;
	    State->vector = vec;
	    vec->style = index;
	    break;
	case 0:	case 1:	case 2:	case 3:	case 4:	/* garbage keywords */
	    State->statecode = v10StateNukeStyle;
	    break;
	case 5:	    /* begindata */
	    SCLEAR(&State->keyword);
	    SCLEAR(&State->tempbuf);
	    State->sawcomma = FALSE;
	    State->begindatacount++;
	    if (State->begindatacount > 1)
		    State->statecode = v10StateBeginData;
	    else State->statecode = v10StateNukeStyle;
	    break;
	case 6:	    /* view */
	    State->statecode = v10StateBSview;
	    SCLEAR(&State->tempbuf);
	    break;
	default:    /* a normal style */
	    State->statecode = State->previous_state;

	    /* do silly things to mark beginning of environment */
	    if (SLEN(&State->linefrag)) {
#ifdef INLINEQUOTING
		if (style->vec.quotedepth) {
		    ADDCHAR(State, fPtr, '>');
		    ADDCHAR(State, fPtr, ' ');
		}
#endif /* INLINEQUOTING */
#ifdef SPECIALFACES
		if (style->vec.face & 1) ADDCHAR(State, fPtr, '_');
		if (style->vec.face & 2) ADDCHAR(State, fPtr, '*');
/*
		if (style->vec.face & 4) ADDCHAR(State, fPtr, '-');
		if (style->vec.face & 8) ADDCHAR(State, fPtr, '+');
*/
#endif /* SPECIALFACES */
	    }

	    /* push to top of stack */
	    vec = (struct StateVector *) malloc(sizeof(struct StateVector));
	    *vec = *(State->vector);
	    vec->next = State->vector;
	    State->vector = vec;

	    /* play style over vector */
	    vec->style = index;
	    vec->lmargin += style->vec.lmargin;
	    if (style->vec.rmargin != MAXLONGLINE) {
		vec->rmargin -= style->vec.rmargin;
	    } else {
		vec->rmargin = MAXLONGLINE;
	    }
	    vec->face |= style->vec.face;
	    if ((style->vec.indent) != INDENT_NOCHANGE) 
		vec->indent = style->vec.indent;
	    if ((style->vec.justify) != JUSTIFY_NOCHANGE) 
		vec->justify = style->vec.justify;
	    vec->quotedepth += style->vec.quotedepth;

	    /* play style over current state */
	    State->specials = 0;
#ifdef SPECIALFACES
	    bitvec = vec->face;
	    while (bitvec) {
		if (bitvec & 1) State->specials++;
		bitvec >>= 1;
	    }
#endif /* SPECIALFACES */
    }   /* end switch (index) */
    return WriteCount;
}

static int
HandleClose(struct ScribeState *State, FILE *fPtr)
{
    struct StateVector *temp;
    int WriteCount = 0;
#ifdef SPECIALFACES
    struct styletable *style;
    int bitvec;
#endif /* SPECIALFACES */

    if (State->vector->next) {
	/* pop top of stack */
	temp = State->vector;
	State->vector = State->vector->next;

	/* play new top over current state */
	State->specials = 0;
#ifdef SPECIALFACES
	bitvec = State->vector->face;
	while (bitvec) {
	    if (bitvec & 1) State->specials++;
	    bitvec >>= 1;
	}
#endif /* SPECIALFACES */

	/* close the old environment and nuke it */
#ifdef SPECIALFACES
	if (temp->style >= 0 && SLEN(&State->linefrag)) {
	    style = Styles + temp->style;
/*
	    if (style->vec.face & 8) ADDCHAR(State, fPtr, '+');
	    if (style->vec.face & 4) ADDCHAR(State, fPtr, '-');
*/
	    if (style->vec.face & 2) ADDCHAR(State, fPtr, '*');
	    if (style->vec.face & 1) ADDCHAR(State, fPtr, '_');
	}
#endif /* SPECIALFACES */
	free(temp);
    } else {
	fprintf(stderr, "Popped too far!\n");
	ADDCHAR(State, fPtr, '}');
    }
    return WriteCount;
}


 /* UnScribe(code, refstate, text, textlen, fileptr)
	Pass in the initial UnScribeInit return value and the address of the integer 
	state variable.  Also pass the address of the text to be unscribed, and the 
	number of bytes at that address.  The unscribed text will be written onto 
	stdio file *fileptr.  Return values are something like fwrite: >= 0 for OK, 
	< 0 for errors, with a code either in the return value or in errno.  Basically, if 
	-1 is returned, errno is valid; otherwise, the code is specific to this
	procedure.
*/
	int 
UnScribe(int code, struct ScribeState **refstate, const char *text, int textlen, FILE *fileptr)
{
   
    int Char;
    struct ScribeState *MyState;
    int	ReadCount;
    int WriteCount;
    const char *Src;

    MyState = *refstate;
    ReadCount = textlen;
    Src = text;
    WriteCount = 0;
    if (ReadCount < 0) return -2;

    switch (code) {
	case Version1:
	    if (MyState->statecode != v1StateInit && MyState->statecode != v1StateSeenAt) return -2;
	    while (--ReadCount >= 0) {
		Char = *Src++;
		switch (MyState->statecode) {
		    case v1StateInit:
			if (Char == '@') MyState->statecode = v1StateSeenAt;
			else {
			    WriteCount++;
			    if (putc(Char, fileptr) == EOF) return -1;
			    if (Char == '\n') MyState->writecount = -1;
			    if (++MyState->writecount > MAXNETMAILLINE) {
				MyState->writecount = 0; WriteCount++;
				if(putc('\n', fileptr) == EOF) return -1;
			    }
			}
			break;
		    case v1StateSeenAt:
			if (Char == '@') {		/* write a single at-sign for ``@@'' */
			    WriteCount++;
			    if (putc(Char, fileptr) == EOF) return -1;
			    if (++MyState->writecount > MAXNETMAILLINE) {
				MyState->writecount = 0; WriteCount++;
				if(putc('\n', fileptr) == EOF) return -1;
			    }
			    /* write nothing for ``@*'' */
			} else if (Char != '*') {	/* write the at-sign and the character */
			    WriteCount += 2;
			    if (putc('@', fileptr) == EOF) return -1;
			    if (putc(Char, fileptr) == EOF) return -1;
			    if (Char == '\n') MyState->writecount = -1;
			    if (++MyState->writecount > MAXNETMAILLINE) {
				MyState->writecount = 0; WriteCount++;
				if(putc('\n', fileptr) == EOF) return -1;
			    }
			}
			MyState->statecode = v1StateInit;
			break;
		    default:
			return -3;
		}
	    }
	    break;

	case Version2:
	    if (MyState->statecode != v2StateInit &&
		MyState->statecode != v2StateSeenAt &&
		MyState->statecode != v2StateSeenNL &&
		MyState->statecode != v2StateSeen2NL) return -2;
	    while (--ReadCount >= 0) {
		Char = *Src++;
		switch (MyState->statecode) {
		    case v2StateInit:
			WriteCount++;
			if (putc(Char, fileptr) == EOF) return -1;
			if (Char == '\n') MyState->writecount = -1;
			if (++MyState->writecount > MAXNETMAILLINE) {
			    MyState->writecount = 0; WriteCount++;
			    if(putc('\n', fileptr) == EOF) return -1;
			}
			if (Char == '@') MyState->statecode = v2StateSeenAt;
			else if (Char == '\n') MyState->statecode = v2StateSeenNL;
			break;
		    case v2StateSeenAt:
			if (Char == '@') {
				/* write a single at-sign for ``@@'' */
			    MyState->statecode = v2StateInit;
			} else {	/* write the at-sign and the character */
			    WriteCount++;
			    if (putc(Char, fileptr) == EOF) return -1;
			    if (Char == '\n') MyState->writecount = -1;
			    if (++MyState->writecount > MAXNETMAILLINE) {
				MyState->writecount = 0; WriteCount++;
				if(putc('\n', fileptr) == EOF) return -1;
			    }
			    if (Char == '\n') MyState->statecode = v2StateSeenNL;
			    else MyState->statecode = v2StateInit;
			}
			break;
		    case v2StateSeenNL:
			if (Char == '\n') 
				/* consume NL */
				MyState->statecode = v2StateSeen2NL;
			else {
			    WriteCount++;
			    if (putc(Char, fileptr) == EOF) return -1;
			    if (Char == '\n') MyState->writecount = -1;
			    if (++MyState->writecount > MAXNETMAILLINE) {
				MyState->writecount = 0; WriteCount++;
				if(putc('\n', fileptr) == EOF) return -1;
			    }
			    MyState->statecode 
				= (Char == '@' ? v2StateSeenAt : v2StateInit);
			}
			break;
		    case v2StateSeen2NL:
			WriteCount++;
			if (putc(Char, fileptr) == EOF) return -1;
			if (Char == '\n') MyState->writecount = -1;
			if (++MyState->writecount > MAXNETMAILLINE) {
			    MyState->writecount = 0; WriteCount++;
			    if(putc('\n', fileptr) == EOF) return -1;
			}
			if (Char != '\n') 
				MyState->statecode  = 
					(Char == '@' ? v2StateSeenAt : v2StateInit);
			break;
		    default:
			return -3;
		}
	    }
	    break;

	case Version12:	/* Slight modification to V10 */
	case Version10:
	    if (MyState->vector == NULL) 
		return -5;	/* not initialized (I think) */

	    while (--ReadCount >= 0) {
		Char = *Src++;
		switch (MyState->statecode) {


/* outdent for switch body */

case v12StateSeenSpcNL:
case v12StateSeenSpc:
case v10StateSeen2NL:
case v10StateSeenNL:
case v10StateInit:
	switch (Char) {
	case '\n':
		switch(MyState->statecode) {
		case v10StateSeen2NL:
			WriteCount += WriteFrag(MyState, fileptr, Char);
			break;
		case v10StateSeenNL:
		case v12StateSeenSpcNL:
			WriteCount += WriteFrag(MyState, fileptr, Char);
			MyState->statecode = v10StateSeen2NL;
			break;
		case v12StateSeenSpc:
			if (code == Version12) 
				MyState->statecode = v12StateSeenSpcNL;
			else 
				WriteCount += WriteFrag(MyState, fileptr, Char);
			break;
		case v10StateInit:
			if (code == Version12) {
				/* Sigh... can't add the space yet... 
					have to wait and make sure this isn't a \n\n...
					ADDCHAR(MyState, fileptr, ' '); */
				MyState->statecode = v10StateSeenNL;
			} 
			else 
				WriteCount += WriteFrag(MyState, fileptr, Char);
			break;
		} /* end switch(MyState->statecode) in case '\n' */
		break;

	case '\\':
		if(code==Version12 && MyState->statecode==v10StateSeenNL)
			ADDCHAR(MyState, fileptr, ' ');
		MyState->previous_state = MyState->statecode;
		MyState->statecode = v10StateSeenBS;
		break;
	case '}':
		if(code==Version12 && MyState->statecode==v10StateSeenNL)
			ADDCHAR(MyState, fileptr, ' ');
		WriteCount += HandleClose(MyState, fileptr);
		break;
	case '\t':
		if(code==Version12 && MyState->statecode==v10StateSeenNL)
			ADDCHAR(MyState, fileptr, ' ');
		if (MyState->lmargin > 0) {
			do {
				ADDCHAR(MyState, fileptr, ' ');
			} while ((MyState->lmargin 
						+ SLEN(&MyState->linefrag))
					% TABINTERVAL);
			break;
		}
		/* DROP THRU */
	default:
		if(code==Version12 && MyState->statecode==v10StateSeenNL) 
			ADDCHAR(MyState, fileptr, ' ');
		ADDCHAR(MyState, fileptr, Char);
		break;
	} /* end switch (Char) */

	/* turn off NL flags */
	if (Char != '\n' && Char != '}' &&
				(MyState->statecode == v10StateSeenNL
				|| MyState->statecode == v12StateSeenSpcNL
				|| MyState->statecode == v10StateSeen2NL)  ) 
		MyState->statecode = v10StateInit;
	if (MyState->statecode == v10StateInit 
				|| MyState->statecode == v12StateSeenSpc)
		MyState->statecode = 
			(Char == ' ') ? v12StateSeenSpc : v10StateInit;

	break;

case v10StateSeenBS:
	switch (Char) {
	case '\\': case '{': case '}':
		MyState->statecode = v10StateInit;
		ADDCHAR(MyState, fileptr, Char);
		break;
	case '\n':	/* only present in version12 */
		MyState->statecode = v10StateInit;
		break;
	default:
		MyState->statecode = v10StateInKeyword;
		SCLEAR(&MyState->keyword);
		SCHAPP(&MyState->keyword, Char);
	}
	break;

case v10StateInKeyword:
	if (Char != '{')  {
		SCHAPP(&MyState->keyword, Char);
	}   /* braces needed because SCHAPP is {}-bounded */
	else
		WriteCount += HandleKeyword(MyState, fileptr);
	break;

case v10StateBSview:
	if (Char != '}') {
		/* accumulate view name and parameters */
		SCHAPP(&MyState->tempbuf, Char);
	}
	else {
		/* process view */

		/* xxx utilize dataobject from cache */

		char *_c;
		const char *c, *d;
		
		for (_c = STEXT(&MyState->tempbuf); *_c && *_c != ','; _c++) {}
		*_c = '\0';
		c = STEXT(&MyState->tempbuf);
		if (!strcmp(c, "bpv")) {
			ADDCHAR(MyState, fileptr, '\f');
		}
		else if (!strcmp(c, "gofigview")) {
			/* gofig was (BOGUSly) written for the dataobj */
		}
		else {
			d = "[An Andrew ToolKit view (";
			while (*d) { ADDCHAR(MyState, fileptr, *d); d++; }

			if (!strcmp(c, "fadview")) c = "an animated drawing";
			else if (!strcmp(c, "rasterview")) c = "a raster image";
			else if (!strcmp(c, "zipview")) c = "a drawing";
			else if (!strcmp(c, "figview")) c = "a drawing";
			else if (!strcmp(c, "textview")) c = "embedded text";
			else if (!strcmp(c, "spread")) c = "a spreadsheet";
			else if (!strcmp(c, "fnotev")) c = "a footnote";
			else if (!strcmp(c, "linkview")) c = "a hyperlink";
			while (*c) { ADDCHAR(MyState, fileptr, *c); c++; }

			c = ") was included here, but could not be displayed.]";
			while (*c) { ADDCHAR(MyState, fileptr, *c); c++; }
		}
		MyState->statecode = v10StateInit;
	}
	break;

#if 1
case v10StateNukeStyle:
	if (Char == '}') MyState->statecode = v10StateDropChar;
	break;

case v10StateDropChar:
	/* ignore the newline after `}` */
	MyState->statecode = v10StateInit;
	break;

#else

case v10StateNukeStyle:
	if (Char == '}') {
		MyState->statecode = v10StateDropChar;
		if (!strcmp(STEXT(&MyState->keyword), "enddata")) 
			MyState->begindatacount--;
	}
	break;

case v10StateDropChar:
	if (MyState->begindatacount < 2) 
		MyState->statecode = v10StateInit;
	else MyState->statecode = v10State2Deep;
	break;

case v10State2Deep:
	if (Char == '\\') 
		MyState->statecode = v10State2DeepSeenBS;
	break;

case v10State2DeepSeenBS:
	switch (Char) {
	case 'b': case 'e':	    /* begindata/enddata */
		MyState->statecode = v10State2DeepInKeyword;
		SCLEAR(&MyState->keyword);
		SCHAPP(&MyState->keyword, Char);
		break;
	default:
		MyState->statecode = v10State2Deep;
	}
	break;

case v10State2DeepInKeyword:
	if (Char != '{') {
		SCHAPP(&MyState->keyword, Char);
		if (SLEN(&MyState->keyword) > sizeof("begindata") + 2)
			MyState->statecode = v10State2Deep;
	} 
	else {
		if (!strcmp(STEXT(&MyState->keyword), "enddata")) {
			MyState->statecode = v10StateNukeStyle;
		} 
		else if (!strcmp(STEXT(&MyState->keyword), "begindata")) {
			MyState->statecode = v10StateNukeStyle;
			MyState->begindatacount++;
		} 
		else
			MyState->statecode = v10State2Deep;
	}
	break;
#endif

case v10StateBeginData:
	/* this state is entered from HandleKeyword.
		It processes the text within {} after \begindata{.
		On seeing '}',  transit to v10StateDataObj.  
	*/
	if (Char == '}') {
		char *s = STEXT(&MyState->tempbuf);	/* dobj class */
		const struct InsetS *insetentry = InsetTable;
		while (insetentry->dobjname != NULL 
				&& strcmp(s, insetentry->dobjname) != 0)
			insetentry++;
		MyState->dobjrcvr = insetentry->dobjrcvr;
		insetentry->dobjinit(MyState);
	
		MyState->oid = atoi(STEXT(&MyState->keyword));
		MyState->statecode = v10StateDataObj;
	}
	else if (Char == ',') 
		MyState->sawcomma = TRUE;
	else if ( ! MyState->sawcomma) {
		/* append to dataobject class name */
		SCHAPP(&MyState->tempbuf, Char);
	}
	else 
		/* append to identifier number */
		SCHAPP(&MyState->keyword, Char);
	break;

case v10StateDataObj: {
	/* this state is entered when v10StateBeginData has
		processed the '}'.  It passes each character to 
		the receiver for the dataobject class identifed in
		v10StateBeginData.  The receiver returns a string
		when it is done and has processed \enddata{.
		MyState->keyword has text of integer under which
		to store the returned string.
	*/
		const char *s = MyState->dobjrcvr(MyState, Char);
		int prefix = SLEN(&MyState->linefrag);

		if ( ! s) break;
		
		/* discard rest of enddata line */
		MyState->statecode = v10StateNukeStyle;

		/* if result is an empty string, ignore it */
		if ( ! *s) break;

		/* we have a result string.  
			xxx should store it under the value in MyState->oid */

		/* instead we print it immediately */
		while (*s) { 
			if (*s == '\n') {
				int j;
				WriteCount += WriteFrag(MyState, fileptr, *s);
				if (*(s+1)) for (j=prefix; j--; )
					ADDCHAR(MyState, fileptr, ' '); 
			}
			else
				ADDCHAR(MyState, fileptr, *s); 
			s++; 
		}
		/* free(s); */ /* BAD IDEA (middle of string; may not be malloc'd */
		break;
	} break;

default:
	return -3;

/* resume normal indent after switch body */


		} /* end switch (MyState->statecode) */
	    }  /* end while (--ReadCount >= 0) */
	    break;	/* end of Version10: Version12: */

	default:  /* no known data stream version */
	    return -4;
    }
    return WriteCount;
}


/* Unbuffer data from an UnScribe sequence.  Return just like fflush(). 
*/
	int 
UnScribeFlush(int code, struct ScribeState **refstate, 	FILE *fileptr)
{
    int Res = 0;

    if (refstate != NULL) {
	if (*refstate != NULL) {
	    if (STEXT(&(*refstate)->linefrag) != NULL) {
		if (SLEN(&(*refstate)->linefrag))
			WriteFrag((*refstate), fileptr, '\n');
		Res = fflush(fileptr);
		SFREE(&(*refstate)->linefrag);
	    }
	    SFREE(&(*refstate)->keyword);
	    SFREE(&(*refstate)->tempbuf);
	    while ((*refstate)->vector != NULL) {
		struct StateVector *vp;

		vp = (*refstate)->vector;
		(*refstate)->vector = (*refstate)->vector->next;
		free(vp);
	    }
	    free(*refstate);
	    *refstate = NULL;
	}
    }
    return Res;
}


/* Close off the UnScribeInit state without needing a valid file to send to. 
*/
	int 
UnScribeAbort(int code, struct ScribeState **refstate)
{

    if (refstate != NULL) {
	if (*refstate != NULL) {
	    SFREE(&(*refstate)->linefrag);
	    SFREE(&(*refstate)->keyword);
	    SFREE(&(*refstate)->tempbuf);
	    while ((*refstate)->vector != NULL) {
		struct StateVector *vp;

		vp = (*refstate)->vector;
		(*refstate)->vector = (*refstate)->vector->next;
		free(vp);
	    }
	    free(*refstate);
	    *refstate = NULL;
	}
    }
    return 0;
}



#ifdef TESTINGONLYTESTING
	int
main(int argc, char *argv[])
{
  int version, err, outcode;
  char buf[500];
  struct ScribeState *ussp;
  FILE *fptr;

  if (argc == 1) {
    fptr = stdin;
  } else {
    if ((fptr = fopen(argv[1], "r")) == NULL) {
      fprintf(stderr, "Could not open file '%s'.\n", argv[1]);
      exit(1);
    }
  }
  outcode = 0;
  version = UnScribeInit(" 12", &ussp);
  while (fgets(buf,sizeof(buf)-1,fptr)) {
    err = UnScribe(version, &ussp, buf, strlen(buf), stdout);
    if (err < 0) {
	fprintf(stderr, "Error: UnScribe == %d.\n", err);
	outcode = 1;
    }
  }
  if (err = UnScribeFlush(0, &ussp, stdout)) {
      fprintf(stderr, "UnScribeFlush error: %d.\n", err);
      outcode = 1;
  }
  return outcode;
}
#endif /* TESTINGONLYTESTING */
