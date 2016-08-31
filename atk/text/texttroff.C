/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

/*
 * Rofftext: Write ATK multimedia text document to file in troff
 *
 * Bugs:
 *
 * The PB/PE macros do not work at a diversion level greater than 0.
 * This means they do not work in tables.
 * Does not handle hyphenation properly - will have to be fixed when
 * scanning the profile routines are put into camlib.
 *
 * Fixed:
 *
 * Uses \s<SIZE>  handles need new line right
 * Changes \ to \\  sub and superscripts
 * Passthru not handled (should set needNewLine )
 */

#include <andrewos.h>
ATK_IMPL("texttroff.H")
#include <system.h>

#include <ctype.h>

#include <atom.H>
#include <text.H>
#include <fontdesc.H>
#include <environ.H>
#include <print.H>
#include <dictionary.H>
#include <viewref.H>
#include <view.H>
#include <tabs.H>
#include <textview.H>
#include <txtstvec.h>
#include <environment.H>
#include <style.H>
#include <stylesheet.H>
#include <matte.H>
#include <fnote.H>
#include <fnotev.H>
#include <proctable.H>
#include <pcompch.H>

#define BOGUS3812 1 /* avoids new bug in psdit */

static int tabscharspaces = 8;

struct tran
{
    char out[3];
};

/*        The following two translation tables use the array index
           as the character code. The key xx is a place holder for no char.
      These tables translate between the characters in the andysymbol 
      and andysymbola fonts and the escape sequences used by troff 
      to represent these symbols */


static struct tran symtran[128] =
{
"xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx",
"xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx",
"xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","**","pl","xx","mi","xx","sl",
"xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx",
"~=","*A","*B","*X","*D","*E","*F","*G","*Y","*I","xx","*K","*L","*M","*N","*O",
"*P","*H","*R","*S","*T","*U","xx","*W","*C","*Q","*Z","xx","xx","xx","xx","ul",
"rn","*a","*b","*x","*d","*e","*f","*g","*y","*i","xx","*k","*l","*m","*n","*o",
"*p","*h","*r","*s","*t","*u","xx","*w","*c","*q","*z","xx","or","xx","ap","xx"
};

static struct tran symatran[128] =
{
"xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx",
"xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx","xx",
"xx","xx","aa","<=","sl","if","xx","xx","xx","xx","xx","xx","<-","ua","->","da",
"de","+-","xx",">=","mu","pt","pd","bu","di","!=","==","xx","xx","br","xx","xx",
"xx","xx","xx","xx","xx","xx","es","ca","cu","sp","ip","xx","sb","ib","mo","xx",
"xx","gr","rg","co","xx","*P","sr","xx","no","xx","xx","xx","<-","ua","->","da",
"xx","xx","rg","co","xx","lt","bv","lb","lc","bv","lf","lt","lk","lb","bv","xx",
"xx","is","xx","xx","xx","rt","bv","rb","rc","bv","rf","rt","rk","rb","xx","xx"
};


#define ENDNOTESONLY FALSE /* Administrators should define as TRUE if local troff can't support footnotes */
/* The following default values are now set by the defaults in textview::GetPrintOption() (textview.C). */
/*#define	CONTENTSBYDEFAULT FALSE*/	/* define the default behavior 
   regarding the printing of tables
   of  contents if the appropriate styles are present */
/*#define	DUPLEXBYDEFAULT	FALSE*/	/* default behavior: simplex printing or duplex printing */

#define ENUMERATE 
#define INDENTSPACE 6
#ifdef ENUMERATE
#include <content.H>
static boolean enumerate;
#endif

#include <texttroff.H>

static FILE *troffFile;
static int addNewLine;      /* True if \n should be added to keep lines from getting */
                            /* Too long, and a space has been found to insert it */
static boolean needNewLine;     /* True if must put out a new line before troff cmd */
static boolean passthru;
/* static boolean HideText; 	*/ /* True if text should be hidden (not completed) */
static boolean underlining;     /* True if text in underlining mode */
static boolean changebar;       /* True if text in changebar mode */
static boolean strikethrough;   /* True if text in strikethrough mode */
static boolean overbar;         /* True if text in overbar mode */
static long dFont;		/* Desired index of name of the font */
static long dSize;		/* Desired size of the font */
static long dFace;		/* Desired faces of the font */
static long cSize;		/* Current size of the font */
static long PageOffset;         /* Faked pg. offset (left marg.) for troff calcs. */
static long LineLength;         /* Faked line length for troff calculations */
static boolean InlineMacros;	/* TRUE if macro files should be included inline */
static int symbola;		/* >0  if the current font is a special symbol font */
static int leadingwhitespace;   /* counts space-char equivalent of leading whitespace that was ignored for ContinueIndent*/ /*RSKadd*/
#define NegOffset       36      /* Negative offset for use in faking troff */

static boolean resetTabs = FALSE;
static long currentVS;
static long latestVS;
static long extraVS;
static long currentSpread;
static long latestSpread;
struct content_chapentry *lastcentry;

static class style *defaultStyle = NULL;
static int textLevel = -1;	/* For generating proper .ev argument */

static struct text_statevector sv, nsv;     /* Current and new state vectors */
static boolean printContents; /* Flag to indicate if we are printing a table of contents */
static boolean printDuplex; /* Flag to indicate if we process for duplex printing. */
static const struct {
    const char *fontname;
    const char * const fontcodes[9];
    /* Fontcodes are in this order:  */
    /*  plain, italic, bold, bolditalic,  fixed-plain, fixed-italic, */
    /* fixed-bold, fixed-bolditalic, shadow. */
    /* All shadowface is bold for now */ 
} fonttable[] = {
#if !defined(TROFF_FONTS_ENV) || defined(EROFF_ENV)
    {"timesroman", {"R", "I", "B", "BI", "C", "CO", "CB", "CD", "B"}},
    {"helvetica", {"H", "HO", "HB", "HD", "C", "CO", "CB", "CD", "B"}},
    {"andy", {"R", "I", "B", "BI", "C", "CO", "CB", "CD", "B"}},
    {"andysans", {"H", "HO", "HB", "HD", "C", "CO", "CB", "CD", "B"}},
    {"andytype", {"C", "CO", "CB", "CD", "C", "CO", "CB", "CD", "C"}},
    {"gacha", {"C", "CO", "CB", "CD", "C", "CO", "CB", "CD", "C"}},
    {0, {"R", "I", "B", "BI", "C", "CO", "CB", "CD", "B"}}
#else
    {"timesroman", {"R",  "I",  "B",  "BI", "CO", "CI", "CB", "CX", "B"}},
    {"helvetica",  {"H",  "HI", "HB", "HX", "CO", "CI", "CB", "CX", "B"}},
    {"andy",       {"R",  "I",  "B",  "BI", "CO", "CI", "CB", "CX", "B"}},
    {"andysans",   {"H",  "HI", "HB", "HX", "CO", "CI", "CB", "CX", "B"}},
    {"andytype",   {"CO", "CI", "CB", "CX", "CO", "CI", "CB", "CX", "CO"}},
    {"gacha",      {"CO", "CI", "CB", "CX", "CO", "CI", "CB", "CX", "CO"}},
    {0,            {"R",  "I",  "B",  "BI", "CO", "CI", "CB", "CX", "B"}}
#endif /* !defined(TROFF_FONTS_ENV) || defined(EROFF_ENV) */
  }; 

static struct {
    const char *fontname;
    int tablenumber;
} specfonttable[] = {
    {"symbol", 1},
    {"andysymbol", 1},
    {"symbola", 2},
    {"andysymbola", 2},
    {0,0}
};

static int endnotes;
class content *con;
static boolean addindex;
#define ModBits 4
#define FaceCodeBits 4
#ifndef text_TMACBASE
#define text_TMACBASE "/lib/tmac/tmac.atk"
#endif /* text_TMACBASE */

#ifndef text_TMACFILE
#define text_TMACFILE environ::AndrewDir(text_TMACBASE)
#endif/* text_TMACFILE */

#ifndef text_TMACACCENTSFILE
#define text_TMACACCENTSFILE environ::AndrewDir("/lib/tmac/tmac.acc")
#endif /* text_TMACACCENTSFILE */

#ifndef text_TMACGROFFILE
#define text_TMACGROFFFILE environ::AndrewDir("/lib/tmac/tmac.psatk")
#endif

#ifndef text_INLINEMACROS
#define text_INLINEMACROS FALSE
#endif


/* C O L O R  -- this code has been moved to print_LookUpColor()*/

#define COLORPTN "'if  \\n(zT  \\{\\\n\\X'%f %f %f setrgbcolor'\n\\}\n"
#define PUTCOLOR(R, G, B, F)   fprintf(F, COLORPTN, R, G, B )

static class atom *A_swapheaders, *A_enumcontents, *A_docontents, *A_endnotes;

ATKdefineRegistry(texttroff, ATK, texttroff::InitializeClass);

static void setcolor(char  *color,FILE  *f);
static char *speclookup(long  c,long  f);
static void PutNewlineIfNeeded();
static void ComputeTroffFont(const char  *name, long  FaceCodemodifier , long  FontSize);
static void ChangeFont();
static void ChangeJustification(enum style_Justification  old , enum style_Justification  new_c,boolean  putbreak);
static void ChangeState();
static void setdefaultstate();
static void InitializeStyle();
static void handlemac(FILE  *f,const char  *s);
static void OutputInitialTroff(FILE  *f, class view *vw, boolean  toplevel, class environment  *cenv);
static int FlushBars(FILE  *f);
static void FlushLineSpacing(int  cs, int  hitchars, boolean  needbreak);
static int findinlist(char  **lst ,int  cnt,char  *str);
static int appendlist(char  **lst,int  cnt,const char  *ostr,int  TEST);
static int lookup(const char  *s);
static void endspecialformating();
static void deletenewlines(char  *buf);
static void deletechapnumbers(char  *buf);
static void insert(const char  *src,char  *c);
static void quote(char  *buf,char  c,int  len);
static void outputendnote();
static int handlespecialformating(class text  *d,class environment  *env,long  pos,long  len);
#if 0
class text *texttroff__CompileNotes(struct classheader  *classID,class text  *txt);
static class text *CompileNotes(class text  *srctext,  class environment  *env, long  startpos, int  topLevel		/* top level call is slightly different */);
#endif /* 0 */


static void
setcolor(char  *color,FILE  *f)
		{
	double r, g, b;
	print::LookUpColor(color, &r, &g, &b);
	PUTCOLOR(r, g, b, f);
}

static char *speclookup(long  c,long  f)
{
    static char foo[6];
    *foo = 0;
    switch(f){
	case 1: 
	    if ( symtran[c].out[0] == 'x' && symtran[c].out[1] == 'x')
		return NULL;
	    sprintf(foo,"\\(%c%c",symtran[c].out[0],symtran[c].out[1]);
	    break;
	case 2:
	    if ( symatran[c].out[0] == 'x' && symatran[c].out[1] == 'x')
		return NULL;
	    sprintf(foo,"\\(%c%c",symatran[c].out[0],symatran[c].out[1]);
	    break;
    }
    return foo;
}

static void PutNewlineIfNeeded()
{
    if (needNewLine) {
        putc('\n', troffFile);
        needNewLine = 0;
    }
}

static void ComputeTroffFont(const char  *name, long  FaceCodemodifier , long  FontSize)
{
    int family, mod,specfamily;

    symbola = 0;
    for (family = 0; fonttable[family].fontname; family++) {
	const char *s, *t;

	for (s = name, t = fonttable[family].fontname; *s && *t; s++, t++) {
	    if (*s != *t && *s != (*t - 32) && *s != (*t + 32))
                break;
	}
	if (*s == '\0' && *t == '\0')
            break;
    }
    if(!fonttable[family].fontname){
	/* try to look up symbol table font */
	for (specfamily = 0; specfonttable[specfamily].fontname; specfamily++) {
	    const char *s, *t;

	    for (s = name, t = specfonttable[specfamily].fontname; *s && *t; s++, t++) {
		if (*s != *t && *s != (*t - 32) && *s != (*t + 32))
		    break;
	    }
	    if (*s == '\0' && *t == '\0')
		break;
	}
	symbola = specfonttable[specfamily].tablenumber;
    }
    /* Take the first modifier we find (italic-bold will be italic) */

    mod = (FaceCodemodifier & (long) fontdesc_Italic) ? 1 : 0;
    if (FaceCodemodifier & (long) fontdesc_Bold)
        mod += 2;
    if (FaceCodemodifier & (long) fontdesc_Fixed)
        mod += 4;
    if (FaceCodemodifier & (long) fontdesc_Shadow)
        mod = 8;
    dFace = mod;
    dFont = family;
    dSize = FontSize;
}

static void ChangeFont()
{
    const char *code = fonttable[dFont].fontcodes[dFace];

    if (needNewLine)
	fprintf(troffFile, code[1] ? "\\&\\f(%s" : "\\&\\f%s", code);
    else
	fprintf(troffFile, "'ft %s\n", code);
}

static void ChangeJustification(enum style_Justification  old , enum style_Justification  new_c,boolean  putbreak)
{
    if (old != new_c) {
	PutNewlineIfNeeded();
	if(putbreak && !(old == style_LeftJustified && new_c == style_LeftAndRightJustified))
	    fputs(".br\n", troffFile);
	switch (new_c) {
	    case style_Centered:
		fputs(".ad c\n", troffFile);
		break;
	    case style_LeftJustified:
	    case style_LeftThenRightJustified:
		fputs(".ad l\n", troffFile);
		break;
	    case style_RightJustified:
		fputs(".ad r\n", troffFile);
		break;
	    case style_LeftAndRightJustified:
		fputs(".ad b\n", troffFile);
		break;
	    default:
                /* Unknown justification code */;
		break;
	}
    }
}

static void ChangeState()
{
    int recomputefont = 0;
    static int cstatus = 0;
    int changetabs = 0;

    /* Figure out what to do for each change in state vector */
    if (sv.CurLeftMargin != nsv.CurLeftMargin) {
        PutNewlineIfNeeded();
        fprintf(troffFile, "'in %ldp\n", NegOffset + nsv.CurLeftMargin);
	changetabs = 1;
    }

    if(sv.CurColor != nsv.CurColor){
	PutNewlineIfNeeded();
	setcolor(nsv.CurColor, troffFile);
    }

    if (sv.CurRightMargin != nsv.CurRightMargin) {
        PutNewlineIfNeeded();

        /* This is currently wrong, since it assumes that the page */
        /* width is 6 + NegOffset across, and since the real right margin */
        /* will be measured from the left edge, as apparently */
        /* troff does as well. See comment in troff init cmds */
        /* fprintf(troffFile, "'ll %dp\n",  LineLength  - nsv.CurRightMargin); */
        /* Fix For Above */

        fprintf(troffFile, "'ll \\n(.lu-(%ldp)\n",
          nsv.CurRightMargin - sv.CurRightMargin);
    }

    if (sv.CurRightEdge != nsv.CurRightEdge) {
    }

    if (sv.CurLeftEdge != nsv.CurLeftEdge) {
    }

    if (sv.CurTopMargin != nsv.CurTopMargin) {
    }

    if (sv.CurBottomMargin != nsv.CurBottomMargin) {
    }

    if (sv.CurFontAttributes != nsv.CurFontAttributes) {
	recomputefont = 1;
    }

    if (sv.CurScriptMovement != nsv.CurScriptMovement) {
        /* fprintf(stderr,"<%d>",nsv.CurScriptMovement); */
        fprintf(troffFile, "\\v'%ldp'", nsv.CurScriptMovement - cstatus);
	needNewLine = 1;
        cstatus = nsv.CurScriptMovement;
    }

    if (sv.CurFontSize != nsv.CurFontSize)
	recomputefont = 1;

    if (sv.CurIndentation != nsv.CurIndentation) {
        PutNewlineIfNeeded();
        if (nsv.CurIndentation < 0)
            fprintf(troffFile, ".ti %ldp\n", nsv.CurIndentation);
        else
	    fprintf(troffFile, ".ti +%ldp\n", nsv.CurIndentation);
	changetabs = TRUE;
    }

    if (sv.CurJustification != nsv.CurJustification)
	ChangeJustification(sv.CurJustification, nsv.CurJustification,TRUE);

    if (sv.SpecialFlags != nsv.SpecialFlags) {
	if(passthru && !(nsv.SpecialFlags & style_PassThru))
	    PutNewlineIfNeeded();
	if (changebar && !(nsv.SpecialFlags & style_ChangeBar)) {
	    /* ChangeBar was on, now off. */
	    fprintf(troffFile, "\n.cB\n\\&");
	    needNewLine = 1;
	} else if (!changebar && (nsv.SpecialFlags & style_ChangeBar)) {
	    /* ChangeBar was off, now on. */
	    fprintf(troffFile, "\n.Cb\n\\&");
	    needNewLine = 1;
	}
	passthru = (nsv.SpecialFlags & style_PassThru);
        underlining = (nsv.SpecialFlags & style_Underline);
        changebar = (nsv.SpecialFlags & style_ChangeBar);
        strikethrough = (nsv.SpecialFlags & style_StrikeThrough);
        overbar = (nsv.SpecialFlags & style_OverBar);
	/* HideText = (nsv.SpecialFlags & style_Hidden); */
    }

    if (sv.CurFontFamily != nsv.CurFontFamily)
	recomputefont = 1;

    /* Check for tab stop changes */
    /* The changetabs state variable is used to force
      reinitializing of tabs after the margin moved.  This is
      unnecessary if we are tabbing with spaces. -wdc */
    /* Actions:
      nsv.tabchars == 1 && sv.tabchars == 1: do nothing
      nsv.tabchars == 1 && sv.tabchars == 0: do space tabs
      nsv.tabchars == 0 && sv.tabchars == 1: force regular tabs
      nsv.tabchars == 0 && sv.tabchars == 0: do regular tabs if necessary */

    if (nsv.SpecialFlags & style_TabsCharacters) {
	if (!(sv.SpecialFlags & style_TabsCharacters)) {
	    /* only output tabstops if we havent done so already */
	    fprintf (troffFile, "'ta %dn\n", tabscharspaces);
	}
    } else {
	if (sv.SpecialFlags & style_TabsCharacters) changetabs = TRUE; /* Force change tabs if we just stopped tabbing with spaces */
	if (!changetabs)
	    changetabs = (sv.tabs)->Different( nsv.tabs);

	if (changetabs) {
            if (needNewLine) fprintf(troffFile,"\\c");
	    PutNewlineIfNeeded();
	    (nsv.tabs)->OutputTroff( nsv.CurIndentation, troffFile);
	    resetTabs = FALSE;
	}
    }
    
    tabs::Death(sv.tabs);
    sv = nsv;
    sv.tabs = (nsv.tabs)->Copy();

    if (recomputefont) {
        ComputeTroffFont(sv.CurFontFamily,
          sv.CurFontAttributes, sv.CurFontSize);    /* Sets dFont, dFace, dSize */
        ChangeFont();                               /* Set default font */
	if (cSize != dSize) {
            if (needNewLine)
                fprintf(troffFile, "\\s%ld\\&", dSize);
            else
                fprintf(troffFile, ".ps %ld\n", dSize);	/* set point size */
	    
	    cSize = dSize;
	}

    }

    latestVS = ((cSize <= 12) ? (cSize + 2) : (cSize * 14 / 12)) + sv.CurSpacing;

    if (currentVS < latestVS)  {
	extraVS = latestVS - currentVS;
    }
    else  {
	extraVS = 0;
    }

    if (sv.CurSpread > sv.CurSpacing) {
	latestSpread = sv.CurSpread - sv.CurSpacing;
    }
    else {
	latestSpread = 0;
    }
}

static void setdefaultstate()
{
    /*	Encounted a style that encompasses whole document */
    /*  dFont, dFace, dSize are already set at this point */
/*     code for this need to be written 
     printf("Setting default to %d, %d, %d\n",dFont,dSize,dFace);
     fflush(stdout);
*/
}
/* PSmacros define PB and PE to surround a postscript insertion.  */
/*
 * These macros now moved into tmac.atk */

static void InitializeStyle()
{
    long fontSize = 12;
    char bodyFont[100];
    const char *font;
    long fontStyle = fontdesc_Plain;
    boolean justify;

    defaultStyle = new style;
    (defaultStyle)->SetName( "printdefault");

    if (environ::ProfileEntryExists("print.bodyfont", FALSE))
	font = environ::GetProfile("print.bodyfont");
    else
	font = environ::GetProfile("bodyfont");

    if (font == NULL || ! fontdesc::ExplodeFontName(font, bodyFont,
      (long) sizeof(bodyFont), &fontStyle, &fontSize))
	strcpy(bodyFont, "Andy");

    justify = environ::GetProfileSwitch("justified", TRUE);

    (defaultStyle)->SetFontFamily( bodyFont);
    (defaultStyle)->SetFontSize( style_ConstantFontSize, (long) fontSize);
    (defaultStyle)->AddNewFontFace( fontStyle);

    if (! justify)
	(defaultStyle)->SetJustification( style_LeftJustified);
}

/* OutputInitialTroff(f, cenv) */
/* Generates the standard stuff at the beginning of the troff stream */
/* The current environment is used to set font, font size, and adjust mode. */
static void handlemac(FILE  *f,const char  *s)
{
    FILE *fi;
    int c;
    if(InlineMacros && ((fi = fopen(s,"r")) != NULL)){
	while((c = getc(fi)) != EOF) putc(c,f);
	fclose(fi);
    }
    else fprintf(f, ".so %s\n",s);
}

static void OutputInitialTroff(FILE  *f, class view *vw, boolean  toplevel, class environment  *cenv)
{
/*     char **mx; */
    int i;

    tabscharspaces = environ::GetProfileInt("TabsCharSpaces", 8);
    
    troffFile = f;
    needNewLine = 0;
    addNewLine = 0;

    /*
     * We don't want to use environments, troff uses themselves
     * fprintf(f, ".ev %d\n", (textLevel > 2) ? 2 : textLevel);
     *
     */

    if (toplevel) {
	const char *macfile;
	/*
	 * cleaning up
	 * everything done here before will no be done in a macro file
	 */

	/*
	 * Built in pathname for now. Needs to be cleaned up
	 */
	if((macfile = environ::GetProfile("tmacaccentsfile")) != NULL || (macfile = environ::GetConfiguration("tmacaccentsfile")) != NULL)
	    handlemac(f,macfile);
	else handlemac(f,text_TMACACCENTSFILE);
	
	if((macfile = environ::GetProfile("tmacfile")) != NULL ||
	   (macfile =environ::GetConfiguration("tmacfile")) != NULL)
	    handlemac(f,macfile);
	else handlemac(f,text_TMACFILE);

	fprintf(f, ".\\\" If \"\\*(.T\" is \"ps\", we're presumably using \"groff\"; set \"zT\" to 1,\n.\\\" and load up the \"groff\" version of the macros (which use \"groff\"\n.\\\" features, and, as such, might not be usable in a package that\n.\\\" Boring Old \"troff\" and \"ditroff\" also have to read).\n.\\\"\n.if \"\\*(.T\"ps\" \\{\\\n.	nr zT 1\n");
	if((macfile = environ::GetProfile("tmacgrofffile")) != NULL ||
	   (macfile =environ::GetConfiguration("tmacgrofffile")) != NULL || (macfile = environ::Get("ATKTMACGROFFFILE")) != NULL)
	    handlemac(f,macfile);
	else {
	    handlemac(f, text_TMACGROFFFILE);
	}
	fprintf(f, ".\\}\n");

	fprintf(f, ".IZ\n");	/*initialise random defaults */
        PageOffset = 54;                    /* Page offset in points */
        LineLength = 468;                   /* Line length in points */
        fprintf(f,".nr IN %dp\n",NegOffset);   /* Makes "0" indentation */

	/* Adjust the lengths of the title lens and margins for */
        /* headers (w/o phony left space for outdenting) */

	fprintf(f, ".nr LT %ldp\n", LineLength - NegOffset);

	/* Reset the left hand margin for the document */

	fprintf(f,".nr PO %ldp\n", PageOffset);

    }

    if (defaultStyle == NULL)
        InitializeStyle();

    text::InitStateVector(&sv);
    text::ApplyEnvironment(&sv, defaultStyle, cenv);
    ChangeJustification((enum style_Justification) - 1, sv.CurJustification,toplevel);

    ComputeTroffFont(sv.CurFontFamily, sv.CurFontAttributes,
      sv.CurFontSize);              /* Sets dFont, dFace, dSize */

    ChangeFont();                   /* Set default font */
    fprintf(f, ".nr PS %ld\n", dSize);  /* Set point size */
    fprintf(f, ".ps \\n(PS\n");
    cSize = dSize;

    currentVS = ((dSize <= 12) ? (dSize + 2) : (dSize * 14 / 12)) + sv.CurSpacing;
    latestVS = currentVS;
    extraVS = 0;
    if (sv.CurSpread > sv.CurSpacing) {
	currentSpread = sv.CurSpread - sv.CurSpacing;
    }
    else {
	currentSpread = 0;
    }

    fprintf(f, ".nr VS %ldp\n", currentVS);        /* Set interline spacing and tabs */
    fprintf(f, ".vs \\n(VSu\n");
    fprintf(f, ".nr EN %dn\n", tabscharspaces);


    if (toplevel) {
	char *val;

	fputs(".sp 0.5i\n", f);     /* Space down for start of page */

	if (vw)
	    printDuplex = vw->GetPrintOption(A_swapheaders);
	else
	    printDuplex = FALSE;

	if (printDuplex) fputs(".nr DP 0\n", f);
	else fputs(".nr DP 1\n", f);

	/* one or two char font name? */
	i = strlen(fonttable[dFont].fontcodes[dFace]);
	/* set font for footnote number in text, I assume here that
	    dFont & dFace are actually the typeface used for the body font
		(from the templates) but a quick test learned they are not
		  Aaaaargh */
	fprintf(f, ".ds Fn %s%s\n", i > 1 ? "\\f(" : "\\f",
		fonttable[dFont].fontcodes[dFace]);
	fprintf(f, ".ds HF %s\n", fonttable[dFont].fontcodes[dFace]);
	fputs(".nr HS \\n(.s\n", f);
	fprintf(f, ".ds FF %s\n", fonttable[dFont].fontcodes[dFace]);
	fputs(".nr FS \\n(.s\n", f);
	fprintf(f, ".RS\n");	/* init real defaults */
	if (sv.CurLeftMargin != 0) {
	    fprintf(troffFile, ".in %ldp\n", NegOffset + sv.CurLeftMargin);
	}
	if (sv.CurRightMargin != 0) {
	    fprintf(troffFile, ".ll \\n(.lu-(%ldp)\n", sv.CurRightMargin);
	}
	if (sv.SpecialFlags & style_TabsCharacters) {
#if 0
	    fprintf (troffFile, "'ta %dn\n", tabscharspaces);
#else
#if 0
	    fprintf (troffFile, "'ta \\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u\n");
#else

	    /* I don't remember why  'ta 8n  didn't work, but this is the same as its replacement except it takes the value of tabscharspaces into account. There are a couple other places  'ta 8n  is still used...shrug. -RSK*/
	    int tabstopnum, count;
	    fprintf(troffFile, "'ta");
	    for (tabstopnum=0; tabstopnum<12; ++tabstopnum) {
		fprintf(troffFile, tabstopnum?" +\\w'":" \\w'");
		for (count=0; count<tabscharspaces; ++count)
		    fprintf(troffFile, " ");
		fprintf(troffFile, "'u");
	    }
	    fprintf(troffFile, "\n");
#endif
#endif
	} else {
	    (sv.tabs)->OutputTroff( sv.CurIndentation, troffFile);
	}
	/*RSKadd:*/
	if (sv.SpecialFlags & style_ContinueIndent) {
	    /* there should be some tricky calculations in here to make the temporary indent check the difference between the style's leftmargin and paragraphindent.  This is mostly just to make source code print halfway decent, so screw it and hardcode it to +5 spaces to approximate what c.tpl does. -RSK*/
	    int i=leadingwhitespace+5;
	    fprintf(troffFile, "'in \\w'");
	    while (i--) fprintf(troffFile, " ");
	    fprintf(troffFile, "'u\n'ti -\\w'     'u\n");
	    /* This sucks.  The silly hack implementation of ContinueIndent screws up the left margin, thereby screwing up all the tab stops.  Compensate for it: -RSK*/
	    if (sv.SpecialFlags & style_TabsCharacters) {
		int tabstopnum, count;
		fprintf(troffFile, "'ta");
		for (tabstopnum=0; tabstopnum<12; ++tabstopnum) {
		    fprintf(troffFile, tabstopnum?" +\\w'":" \\w'");
		    for (count= (tabstopnum ? 0 : leadingwhitespace%tabscharspaces); count<tabscharspaces; ++count)
			fprintf(troffFile, " ");
		    fprintf(troffFile, "'u");
		}
		fprintf(troffFile, "\n");
	    }
	    leadingwhitespace= 0;
	} else
	    /*:RSKadd*/

	    if (sv.CurIndentation < 0)
		fprintf(troffFile, ".ti %ldp\n", sv.CurIndentation);
	    else if (sv.CurIndentation > 0)
		fprintf(troffFile, ".ti +%ldp\n", sv.CurIndentation);
    }

    if (environ::GetProfileSwitch("hyphenate", 0))
	fputs(".hy\n", f);
    else
	fputs(".nh\n", f);
}

static int barPending;

static int FlushBars(FILE  *f)
{
    if (barPending) {
        char buf[128];

        barPending = 0;

        /* Start position of bar(s) is currently in troff register X. */
        /* Find length of bar(s) in troff register Y */

        strcpy(buf, "\\kY");

        if (underlining)    /* Move back and draw underline */
            strcat(buf, "\\h'|\\nXu'\\l'|\\nYu\\(ul'");
        if (strikethrough)      /* Move back and draw change-bar */
            strcat(buf, "\\h'|\\nXu'\\u\\l'|\\nYu\\(ul'\\d");
        if (overbar)        /* Move back and draw over-bar */
            strcat(buf, "\\h'|\\nXu'\\l'|\\nYu\\(rn'");

        fputs(buf, f);
        return strlen(buf);
    } else
        return 0;
}

static void FlushLineSpacing(int  cs, int  hitchars, boolean  needbreak)
{
    /* Put out .sp for subsequent new lines  */

    PutNewlineIfNeeded();

    if (needbreak) {
        fputs(".OC\n", troffFile);
    }
    if (cs == 1) {
	if (currentSpread != 0) {
	    fprintf(troffFile, ".sp %ldp\n", currentSpread);
	}
    }
    else if (cs > 1) {
	cs--;
	if(hitchars == 0) /* space past trap */
	    fprintf(troffFile, ".sv %d\n", cs);
	else fprintf(troffFile, ".sp %ldp\n", cs * (currentVS + currentSpread) + currentSpread);
    }
    currentSpread = latestSpread;

    /* Restore tabs after temporary setting of indent to 0.
      If we're doing character spaced tabs, resetTabs wasnt set by
      us.  We save it for who ever DID set it. 
      I'm unwilling to say that it will never be set under these
      conditions. -wdc */
    if (sv.CurIndentation != 0) {
	if (resetTabs  && !(sv.SpecialFlags & style_TabsCharacters)) {
	    (sv.tabs)->OutputTroff( sv.CurIndentation, troffFile);
	    resetTabs = FALSE;
	}

	/*RSKadd:*/
	if (sv.SpecialFlags & style_ContinueIndent) {
	    int i=leadingwhitespace+5;
	    fprintf(troffFile, "'in \\w'");
	    while (i--) fprintf(troffFile, " ");
	    fprintf(troffFile, "'u\n'ti -\\w'     'u\n");
	    leadingwhitespace= 0;
	} else
	    /*:RSKadd*/

	    if (sv.CurIndentation < 0) 
		fprintf(troffFile, "'ti %ldp\n", sv.CurIndentation);
	    else 
		fprintf(troffFile, "'ti +%ldp\n", sv.CurIndentation);
    }
    if (latestVS != currentVS)  {
	fprintf(troffFile, ".vs %ld\n", latestVS);
	currentVS = latestVS;
	extraVS = 0;
    }
}
#define UNMATCHED -1
#define FOOTNOTE 0
#define INDEX 1
#define INVINDEX 2
#define NORMAL 0
#define NOFORMAT 1
#define NOPRINT 2
static char **namelist;
static const char listheader[] = 
    "footnote,index,indexi"
   ;
static const char defaultlist[] = 
/*    "majorheading,heading,subheading,chapter,section,subsection,paragraph,function" */
"chapter,section,subsection,paragraph"
;
static int formatnote;
static int findinlist(char  **lst ,int  cnt,char  *str)
{
    int i;
    for(i = 0; i < cnt; i++,lst++){
	if(*lst == NULL || str == NULL) return -1;
	if(**lst == *str && (*lst == str || strcmp(*lst,str) == 0)){
	    return i;
	}
    }
    return -1;
}
static int appendlist(char  **lst,int  cnt,const char  *ostr,int  TEST)
{   /* BUG -- OVERFLOWS NOT DETECTED */

    char *str;
    long len;
    int next = 1;
    ;
    if(ostr == NULL || (len = strlen(ostr)) == 0 || ((str = (char *)malloc(len + 1)) == NULL)) return 0;
/* NEED A REASONABLE WAY TO NOTE str TO FREE LATER */
    strcpy(str,ostr);
    if(TEST){
	if(findinlist(lst,cnt,str) != -1) return cnt;
    }
    while(*str){
	if(*str == ',' || *str == '\n') {
	    *str = '\0';
	    next++;
	}
	else if(*str == '\0') break;
/*	else if(*str == ' ') ; */
	else if(next){
	    lst[cnt++] = str;
	    next = 0;
	}
	str++;
    }
    lst[cnt] = NULL;
    return cnt;
}
static int lookup(const char  *s)
{
    char **p;
    int i = 0;
    for(p = namelist ; p && *p; p++,i++){
/* fprintf(stderr,"Testing %s %s\n",*p,s); */
	if(**p == *s && strcmp(*p,s) == 0) return i;
    }
    return UNMATCHED;
}
/* Writes the document out in troff format into a file  */
static void endspecialformating()
{
    PutNewlineIfNeeded();
    fprintf(troffFile,".FE\n");
    formatnote = -1;
}
static void deletenewlines(char  *buf)
{
    char *c;
    for(c = buf; *c != '\0'; c++){
	if(*c == '\n') *c = ' ';
    }
    for(c--; c > buf; c--){
	if(isspace(*c))  *c = '\0';
	else break;
    }
}
static void deletechapnumbers(char  *buf)
{
    char *c,*s;
    s = buf;
    if(*s <= '9' && *s >= '0' && (c = strchr(buf,'\t')) != NULL){
	c++;
	do{
	    *s++ = *c;
	} while (*c++ != '\0');
    }
}
static void insert(const char  *src,char  *c)
{   /* inserts string src into the begining of string c , assumes enough space */
    char *p,*enddest;
    const char *cp;
    enddest = c + strlen(c);
    p = enddest + strlen(src);
    while(enddest >= c) *p-- = *enddest-- ;
    for(cp = src; *cp != '\0';cp++)
	*c++ = *cp;
}
static void quote(char  *buf,char  c,int  len)
{
    char *ebuf ;
    int cfree;
    int clen = strlen(buf);
    ebuf = buf +  clen - 1;
    cfree = len - clen;
    if(cfree <= 0) return;
    while(ebuf >= buf){
	if(*ebuf == c){    
	    insert("\\",ebuf);
	    if(--cfree == 0) return;
	}
	ebuf--;
    }
}
static void outputendnote()
{
    fprintf(troffFile,"%d ",endnotes++);
}
static int handlespecialformating(class text  *d,class environment  *env,long  pos,long  len)
{
    class style *st;
    struct content_chapentry *centry;
    char buf[256],*bbf,*sbuf;
    const char *sn;
    long type;
    int ch;
/* printf("pos = %d formatnore = %d\n",pos,formatnote);
fflush(stdout); */
    if(pos == formatnote){
	if(endnotes == FALSE){
	    endspecialformating();
	}
    }
    else if(pos < formatnote){
	if(endnotes == TRUE){
	    return NOPRINT;
	}
	else {
	    return NOFORMAT; /* could probably handle allowing bold and italic here */
	}
    }
    if(env->type == environment_View){
	class viewref *vr;
	vr = env->data.viewref;
	if(*(vr->viewType) == 'f' && strcmp(vr->viewType,"fnotev") == 0)
	    return NOFORMAT;
    }
    if(env->type != environment_Style) return NORMAL;
    st = env->data.style;
    if(st == NULL || ((sn = (st)->GetName()) == NULL)){
/* fprintf(stderr,"Null style\n"); */
	return NORMAL;
    }
    type = lookup(sn);
    switch(type){
	case UNMATCHED :
 /* fprintf(stderr,"Returning Normal\n"); */
	    return NORMAL;
#ifdef REFER
	case REFER:
	    PutNewlineIfNeeded();
	    if(len > 255) len = 255;
	    (d)->CopySubString(pos,len,buf,TRUE);
	    deletenewlines(buf);
	    quote(buf,'"',255);
	    fprintf(troffFile,".[ [\n%s\n.]]\n",buf);
	    return NOPRINT ;
#endif
	case FOOTNOTE :
	    fprintf(troffFile, "\\**\n");	/* automatic footnote counter string */
	    needNewLine = 0;
	    if(endnotes == TRUE){
/*		outputendnote(); */
		formatnote = pos + (env)->GetLength() - 1;
		return NOPRINT;
	    }
	    else {
		fprintf(troffFile,".FS\n");
		formatnote = pos + (env)->GetLength() - 1;
		return NOFORMAT;
	    }
	case INDEX :
	case INVINDEX :
	    if(addindex) {
		PutNewlineIfNeeded();
		if(len > 255) len = 255;
		(d)->CopySubString(pos,len,buf,TRUE);
		deletenewlines(buf);
		deletechapnumbers(buf);
		quote(buf,'"',255);
		fprintf(troffFile,".ix \"%s\"\n",buf);
	    }
	    if(type == INVINDEX) return NOPRINT ;
	default:    /* table of contents entry */
	    if(con && ((centry = (con)->CopyEntry(pos,len,buf,255)) != NULL)) {
		if(centry == lastcentry){
		    return NORMAL;
		}
		lastcentry = centry;
		PutNewlineIfNeeded();
		/*
		 printf("Copy Entry returned %s<\n",buf);
		 fflush(stdout);
		 */
	    }
	    else {
		if(type == INDEX) return NORMAL;
		PutNewlineIfNeeded();
		if(len > 255) len = 255;
		(d)->CopySubString(pos,len,buf,FALSE);
		deletenewlines(buf);
		centry = NULL;
	    }
	    
	    quote(buf,'"',255);
	    for(sbuf = buf;*sbuf != '\0' && isspace(*sbuf);sbuf++) ;
	    bbf = strchr(sbuf,'\t');
	    if(bbf && isdigit(*sbuf) && (centry == NULL || centry->space == -1)) {
		char *c;
		long n;
		*bbf++ = '\0';
		for(c = sbuf,n = 1; *c; c++)
		    if(*c == '.') n++;
		for(ch = (d)->GetChar(pos); isspace(ch); ch = (d)->GetChar(pos))
		    pos++;
		if(con != NULL && *sbuf != (d)->GetChar(pos) && (d)->Strncmp(pos,sbuf,strlen(sbuf)) != 0){
		    fprintf(troffFile,".HE\n");
		    if(printContents)fprintf(troffFile,".IC %ld \"%s\" NO\n",n,bbf);
		}
		else {
		    fprintf(troffFile,".HE\n");
		    if(printContents)fprintf(troffFile,".IC %ld \"%s\" %s\n",n,bbf,sbuf);
		    fprintf(troffFile,".iw %s \"%s\"\n",sbuf,bbf);
		}
	    }
	    else if(*sbuf){
		fprintf(troffFile,".HE\n");
		if(printContents){
		    long n;
		    if(centry != NULL) n = centry->space;
		    else n = -1;
		    fprintf(troffFile,".IC %ld \"",n + 1);
		    n = n *INDENTSPACE;
		    while(n-- > 0) putc(' ',troffFile);
		    fprintf(troffFile,"%s\" NO\n",sbuf);

		}
	    }
	    return NORMAL;
    }

}
#if 0
class text *texttroff__CompileNotes(struct classstyle;
	    if(st != NULL && ((sn = (st)->GetName()) != NULL) &&
	       *sn == 'f' && strcmp(sn,"footnote") == 0){
		sprintf(foo,"\n%d   ",count++);
		(txt)->InsertCharacters((txt)->GetLength(),foo, strlen(foo));
		place = (txt)->GetLength();
		(txt)->CopyText(place,srctext ,pos,(env)->GetLength());
		(txt->rootEnvironment)->Remove( place, (env)->GetLength(), environment_Style, FALSE);
	    }
	}
	else  /* look for footnote object */
	    if(env->type == environment_View){
		vr = env->data.viewref;
		if(*(vr->viewType) == 'f' && strcmp(vr->viewType,"fnotev") == 0){
		    newtxt = (class text *) vr->dataObject;
		    sprintf(foo,"\n%d   ",count++);
		    (txt)->InsertCharacters((txt)->GetLength(),foo, strlen(foo));
		    (txt)->CopyText((txt)->GetLength(), newtxt,0,(newtxt)->GetLength());
		}
	    }
	pos = cpos;
    }
    return txt;
}
  
#endif /* 0 */

void texttroff::WriteSomeTroff(class view  *view, class dataobject  *dd, FILE  * f, int  toplevel, unsigned long  flags)
{
    int elen, cs, ln , flag,count,indexfontface,hitchars;
    long i, doclen;
    class text *d,*ttxt;
    boolean quotespace; 
    class environment *cenv, *nenv;
    char *list[64];
    const char *p,*val;
    class style *IndexStyle;
    struct text_statevector safesv;
#ifdef ENUMERATE
    const char *ChapNumber;
    if(toplevel){
	enumerate = view->GetPrintOption(A_enumcontents);
	lastcentry = NULL;
    }
#endif /* ENUMERATE */
    ttxt = NULL;
    
    if(toplevel){
	count = appendlist(list,0,listheader,FALSE);
	if((p = environ::Get("ContentsList")) == NULL){
	    if((p = environ::GetProfile("ContentsList"))== NULL)
		p = defaultlist;
	}
	appendlist(list,count,p,FALSE);
	namelist = list;
	printContents = view->GetPrintOption(A_docontents);
	if((val = environ::Get("InlineMacros")) != NULL ){
	    InlineMacros = (*val == 'n' || *val == 'N')? FALSE:TRUE;
	}
	else {
	    if((val = environ::GetConfiguration("InlineMacros")) != NULL){
		InlineMacros = (*val == 'n' || *val == 'N')? FALSE:TRUE;
	    }
	    else InlineMacros = text_INLINEMACROS;
	    InlineMacros = environ::GetProfileSwitch("InlineMacros",InlineMacros);
	}
	fflush(stderr);
    }
    passthru = 0;
    underlining = 0;
    changebar = 0;
    strikethrough = 0;
    overbar = 0;
    if(toplevel){
	fnote::CloseAll((class text *)dd);
	if (!ENDNOTESONLY)
	    endnotes = view->GetPrintOption(A_endnotes);
	else
	    endnotes = TRUE;
    }
    /* fprintf(stderr,"endnotes = %s\n",(endnotes == FALSE) ? "FALSE":"TRUE");fflush(stderr); */  
    textLevel++;
    d = (class text *) dd;
    cenv = d->rootEnvironment;  /* Initial environment */
    /* The changes in templates (aka stylesheets, ~june 15-20, 1995) change the effect of this next section.
     Writing into the index style will most likely cause ALL documents using that template
     to be affected.  I suspect that this is reasonable, but it is a CHANGE from the old
     way. For now I will leave this code as is. -Rob Ryan. */
    if(toplevel && (IndexStyle = (d->styleSheet)->Find("index" )) != NULL){
	indexfontface = (IndexStyle)->GetAddedFontFaces();
	(IndexStyle)->ClearNewFontFaces();
    }
    if(toplevel) addindex = (environ::Get("IndexOnly") != NULL);

    if(toplevel == TRUE || (ATK::IsTypeByName((dd)->GetTypeName(),"fnote") == FALSE)){
	if(toplevel != TRUE) {
	    /* don't kill safesv tabs - safesv is uninitialized */
	    safesv = sv;
	    /* Clever optimization:
	     We're done with sv's tabs.  We could:
	    safesv.tabs = tabs_Copy(sv.tabs);
	    text_FinalizeStateVector(&sv);
	     but since that just has the effect of an increment
	     and a decrement of the link count, we cleverly
	     do nothing -wdc */
	}
	OutputInitialTroff(f, view, toplevel, cenv);
    }
    else {
	/* don't kill safesv tabs - safesv is uninitialized */
	safesv = sv;
	/* Clever optimization:
	 We're done with sv's tabs.  We could:
	safesv.tabs = tabs_Copy(sv.tabs);
	text_FinalizeStateVector(&sv);
	 but since that just has the effect of an increment
	 and a decrement of the link count, we cleverly
	 do nothing -wdc */

	if (defaultStyle == NULL)
	    InitializeStyle();

	text::InitStateVector(&sv);
	text::ApplyEnvironment(&sv, defaultStyle, cenv);
    }
    ln = 0;
    i = 0;
    cs = 0;                     /* start w/ .br or proper line spacing */
    doclen = (d)->GetLength();
#ifdef ENUMERATE
    if(toplevel){
	ChapNumber =  environ::Get("InitialChapNumber");
	con	= new content;
	(con)->SetSourceText(d);
	if(enumerate){
	    (con)->Enumerate(0,doclen,ChapNumber);
	    (con)->UpdateSource(0,doclen);
	    doclen = (d)->GetLength();
	}
    }
#endif /* ENUMERATE */
    formatnote = -1;
    if(toplevel){
	int lastnote;
	ttxt = new text;
	lastnote = fnote::CopyAll(d,ttxt,1,TRUE);
	if(lastnote == 1){ /* no footnotes */
	    (ttxt)->Destroy();
	    ttxt = NULL;
	}
	else{
#ifdef NOCHILDREN
	    if((ttxt->rootEnvironment)->NumberOfChildren() > 0)
		/* footnotes can't be displayed with styles, 
		    so process as endnotes */
		endnotes = TRUE;
	    else 
#endif
		if(endnotes == FALSE){
		    (ttxt)->Destroy();
		    ttxt = NULL;
	    }
	}
	fnotev::SetEndnote(endnotes);
    }

/*fprintf(stderr,"::endnotes = %s\n",(endnotes == FALSE) ? "FALSE":"TRUE");fflush(stderr); */
    hitchars = 0;
    quotespace = TRUE;
    while (i < doclen) {
	nenv = (class environment *)(d->rootEnvironment)->GetInnerMost( i);
	elen = (d->rootEnvironment)->GetNextChange( i);
	if (elen > doclen)
	    elen = doclen - i;
	if (cenv != nenv) {	/* change environment */
	    text::InitStateVector(&nsv);
	    text::ApplyEnvironment(&nsv, defaultStyle, nenv);
	    if(toplevel)
		flag = handlespecialformating(d,nenv,i,elen); /* flag = Normal,NoFormat, or NoPrint */
	    else{
		flag = NORMAL;
	    }
	    cenv = nenv;
	    if(flag == NORMAL){ 
		ChangeState();
		if(i == 0){
		    if((nenv)->GetLength() == doclen) setdefaultstate();
		    FlushLineSpacing(0,hitchars,FALSE); /* sets vertical spacing */
		}
	    }
	    else if(flag == NOPRINT){
		i += elen ;
		continue;
	    }
            if (nenv->type == environment_View) {
		boolean needta;
                class viewref *vr = nenv->data.viewref;
                if ((sv.CurView = (class view *)
		     dictionary::LookUp(view, (char *) vr)) == NULL) {
		    ATK::LoadClass(vr->viewType);
	            if (ATK::IsTypeByName(vr->viewType, "view")
                      && (sv.CurView = (class view *)
                      matte::Create(vr, (class view *) view)) != NULL) {
                        (vr)->AddObserver( view);
			dictionary::Insert(view, (char *) vr, (char *) sv.CurView);
                    }
                } else
                    if (sv.CurView == (class view *) textview_UNKNOWNVIEW)
                        sv.CurView = NULL;

		 if (sv.CurView != NULL) {
		     if(strcmp(vr->viewType,"fnotev") == 0){
			 if (cs) {
			     FlushLineSpacing(cs,hitchars,TRUE);
			     cs = 0;
			 }
			 needNewLine = 0;
			 needta = FALSE;
		     }
		     else if(strcmp(vr->viewType,"bpv") == 0){
			 if(cs){
			     FlushLineSpacing(1,hitchars,TRUE);/* throw away unneeded newlines */
			     cs = 0;
			 }
			 needta = FALSE;
			 hitchars = 0;
			 PutNewlineIfNeeded();
		     }
		     else {
			 if (cs) {
			     FlushLineSpacing(cs,hitchars,TRUE);
			     cs = 0;
			 }
			 needta = TRUE;
			 PutNewlineIfNeeded();
		     }
#ifdef VIEWREF_DESIREDSIZE_SET	
		     /* I would use this code, but at the moment the desired width and height don't get set in the viewref. */	     
		     if(vr->desw>0 && vr->desh>0) {
			 /* set up the desired width and height in troff registers,
			  this isn't really sensible, to do it right requires that we know the
			  desired physical size. not pixel size. oh well, individual insets
			  can make whatever assumptions they like. */
			 fprintf(".nr vw %d\n", vr->desw); /* put the desired width and height in regs */
			 fprintf(".nr vh %d\n", vr->desh);
			 fprintf(".nr ds 1\n"); /* flag that the vh and vw regs are set. */
		     } else fprintf(".nr ds 0\n"); /* flag that the vh and vw regs are nonsense */
#endif /* VIEWREF_DESIREDSIZE_SET */		     
		     (sv.CurView)->Print( f, "troff", "PostScript", 0);
		     if(needta)
			 /* reset tab stops, as table is want to mess them up */
			 if (sv.SpecialFlags & style_TabsCharacters) {
#if 0
			     fprintf (troffFile, "'ta %dn\n", tabscharspaces);
#else
#if 0
	    fprintf (troffFile, "'ta \\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u +\\w'        'u\n");
#else

	    /* I don't remember why  'ta 8n  didn't work, but this is the same as its replacement except it takes the value of tabscharspaces into account. There are a couple other places  'ta 8n  is still used...shrug. -RSK*/
	    int tabstopnum, count;
	    fprintf(troffFile, "'ta");
	    for (tabstopnum=0; tabstopnum<12; ++tabstopnum) {
		fprintf(troffFile, tabstopnum?" +\\w'":" \\w'");
		for (count=0; count<tabscharspaces; ++count)
		    fprintf(troffFile, " ");
		fprintf(troffFile, "'u");
	    }
	    fprintf(troffFile, "\n");
#endif
#endif
			 } else {
			     (sv.tabs)->OutputTroff( (resetTabs) ? 0 : sv.CurIndentation, troffFile);
			 }
		 }
	    }
	    text::FinalizeStateVector(&nsv);
	}	/* End change environment */

	elen += i;		/* Bump by current position */

	if (! needNewLine){
            ln = 0;
	    quotespace = TRUE;
	}

	if (nenv->type != environment_View) {
            int c, insideWord = 0;

            barPending = 0;
	    if(!needNewLine && i > 0 && (d)->GetChar( i) == ' '){
		/* Fix bug with space following style change forceing newline*/
		i++;
		if(i < elen && (d)->GetChar( i) == ' '){
		    fputs("\\c\n",f);
		}
	    }
	    if(toplevel && i == formatnote && endnotes == FALSE)
		endspecialformating();
            while (i < elen) {
		if (passthru){
		    if (cs) {
			FlushLineSpacing(cs,hitchars,TRUE);
			cs = 0;
		    }
		    /* Put out passthru stuff as we see it */
		    if ((c = (d)->GetChar( i)) == '\n') {
			needNewLine = 0;
			ln = 0;
		    } else {
			needNewLine = 1;
			ln++;
		    }
		    fputc(c, f);
		    i++;
		    continue;
		}  /* end passthru */

		if ((c = (d)->GetChar( i)) == '\n') {
		    cs++;	/* count line Spacing */
		    i++;
                    insideWord = 0;
                    ln = FlushBars(f);

		    /*RSKadd:*/
		    if (sv.SpecialFlags & style_ContinueIndent) {
			int ch;
			for (leadingwhitespace=0; (ch=(d)->GetChar( i))==' ' || ch=='\t'; i++)
			    if (ch=='\t')
				/* just hope nobody's ever gonna use ContinueIndent WITHOUT TabsCharacters.  If they do, this calculation is totally bogus: */
				/* leadingwhitespace= (leadingwhitespace+8)&~7; */
				leadingwhitespace = ((leadingwhitespace+tabscharspaces)/tabscharspaces)*tabscharspaces;
			    else
				leadingwhitespace++;
		    }
		    /*:RSKadd*/

		    continue;
		}

		if (cs) {
                    FlushLineSpacing(cs,hitchars,TRUE);
		    cs = 0;
		    quotespace = TRUE;
                }
		hitchars++;
                /* The bar style is broken up into a separate region */
                /* surrounding each word and each intervening */
                /* group of white spaces.  This is because we cannot */
                /* know when the filled troff output will wrap and */
                /* mess up the drawing. */

                if (insideWord) {
                    if (c == ' ' || c == '\t') {
                        insideWord = 0;
			ln += FlushBars(f);
                    }
                } else {
                    if (c != ' ' && c != '\t') {
			insideWord = 1;
			if (extraVS != 0)  {
			    fprintf(f, "\\x'-%ldp'", extraVS);
			    ln += 7;
			}
                        ln += FlushBars(f);
                    }
		    else leadingwhitespace++; /*RSKadd*/
                }

                if (! barPending && (i > formatnote) && (underlining || strikethrough || overbar)) {
                    /* Use backslash to start a new line; */
                    /* the bar-generating troff code can get pretty wide. */
                    fputs("\\\n\\kX", f);       /* Save start pos of item */
                    ln = 3;
                    barPending = 1;
                }

		i++;

		if (ln++ > 80 && (c == ' ' || c == '\t')) {
		    /* Don't let lines get too long */
		    addNewLine++;
		} else if (addNewLine) {
		    /* Add the newline before the first */
                    /* non-blank character (if still needed) */
		    if (ln > 80) {
			fputc('\\', f); /* troff is going to ignore the trailing whitespace if we don't quote this newline! */ /*RSKadd*/
			fputc('\n', f);
			ln = 0;
			needNewLine = 0;
		    }
		    addNewLine = 0;
		}
		if(symbola != 0){
		    /* handle special characters in the symbol fonts 
		     by inserting troff escape codes */
		    char *outst; 
		    outst = speclookup(c,symbola);
		    if(outst){
			if (ln > 80) {
			    fputc('\\', f);
			    fputc('\n', f);
			    ln = 0;
			}
			fputs(outst,f);
			ln += strlen(outst);
		    }
		   else {
		       /* code here for unknown symbol */
		   }
		}
		else {
		    if ((c == '\\') || (! needNewLine && (c == '\'' || c == '.'))) {
			/* quote special characters */
			fputc('\\', f);
			if (c == '.') {
			    fputc('&', f);
			    ln++;
			}
			ln++;
		    }
		    if (c == '\r') {
			fputs("\n.br\n", f);
			if (sv.CurIndentation != 0) {
			    if (sv.SpecialFlags & style_TabsCharacters) {
				fprintf (troffFile, "'ta %dn\n", tabscharspaces);
			    } else {
				(sv.tabs)->OutputTroff( 0, f);
				resetTabs = TRUE;
			    }
			}
		    }
		    else if((!isascii(c) || !isprint(c)) && c != '\t') {
			char *ccp= pcompch::CharacterToTroff(c,nenv,NULL);
			if(ccp) {
			    while(*ccp) {
				fputc(*ccp,f);
				ccp++;
				ln++;
			    }
			} else {
			    if (ln > 80) {
				fputc('\n', f);
				ln = 0;
			    }
			    fprintf(f,"\\\\%3.3o",c);
			    ln += 3;
			}
		    }
		    else {
#ifdef BOGUS3812
			/* this code isn't quite right, but some 3812 printers can't handle hard spaces  */
			if(quotespace){			
			    if(c == ' ') {
				fputc('\\',f);
				fputc('&',f);
				ln++;
			    }
			    quotespace = FALSE;
			}
#else /*BOGUS3812 */
			/* put hard spaces at the beginning of lines so that initial spacings are consistent 
			 and no extra newlines are added */
			if(quotespace){
			    if(c == ' ') {
				fputc('\\',f);
				ln++;
			    }
			    else quotespace = FALSE;
			}
#endif /*BOGUS3812 */
			fputc(c, f);
		    }
		}
		needNewLine = 1;
	    }
            FlushBars(f);
	    
        } else      /* nenv->type ==  environment_View */
	    i = elen; 
	if(i == formatnote && endnotes == FALSE)
	    endspecialformating();
   }

    PutNewlineIfNeeded();

    /*
     * troff ueses envs itself
     * fputs(".ev\n", f);
     *
     */
    textLevel--;
#ifdef ENUMERATE
    if(toplevel){
	if(IndexStyle != NULL){
	    (IndexStyle)->AddNewFontFace(indexfontface);
	}
	if(enumerate && con){
	    (con)->Denumerate(0,doclen);
	    (con)->UpdateSource(0,doclen);
	}
	if(con) (con)->Destroy();
    }
    else {
	/* write troff to return to parents state */

        /*
         * nsv may not have been initialized.
         * (if we're not toplevel and there are no style changes?)
         * Until this is figured out, don't garbage collect. -dba
         */
#if 0
	tabs::Death(nsv.tabs);
#endif

	nsv = safesv;
	nsv.tabs = (safesv.tabs)->Copy();

	if(flags&texttroff_Revert) ChangeState();
	tabs::Death(sv.tabs);
	sv = safesv;

	/* Clever optimization:
	 We're done with safesv's tabs.  We could:
	sv.tabs = tabs_Copy(safesv.tabs);
	tabs_Death(safesv.tabs);
	 but since that just has the effect of an increment
	 and a decrement of the link count, we cleverly
	 do nothing -wdc */

    }
#endif  /* ENUMERATE */

    if(ttxt){	/* print the endnotes */
	class textview *tv;
	tv = new textview;
	(tv)->SetDataObject(ttxt);
	(tv)->LinkTree(view);
	PutNewlineIfNeeded();
	fputs(".bp\n", f);
	(tv)->Print(f,"troff","PostScript",0);
	(tv)->UnlinkTree();
	(tv)->Destroy();
	(ttxt)->Destroy();
    }
}

void texttroff::WriteTroff(class view  *view, class dataobject  *dd, FILE  * f, int  toplevel)
{
    texttroff::WriteSomeTroff(view,dd,f,toplevel,texttroff_Revert);
}

void texttroff::BeginDoc(FILE  *f)
{
    textLevel++;
    OutputInitialTroff(f, NULL, TRUE, NULL);
    fputs(".br\n", f);
}

void texttroff::EndDoc(FILE  *f)
{
    /*
     *fputs(".ev\n",f);
     *
     */
    textLevel--;
}

void texttroff::BeginPS(FILE  *f, long  width , long  height)
{
    fprintf(f, "'PB %ld %ld\n", width, height);
    fprintf(f, "'if  \\n(zT  \\{\\\n");
    fprintf(f, "\\!    %ld troffadjust %ld neg translate\n", width, height);
}

void texttroff::EndPS(FILE  *f, long  width , long  height)
{
    fprintf(f, "\\}\n");
    fprintf(f, "'PE %ld %ld\n", width, height);
}

boolean texttroff::InitializeClass()
{
    A_swapheaders = atom::Intern("swapheaders");
    A_enumcontents = atom::Intern("enumcontents");
    A_docontents = atom::Intern("docontents");
    A_endnotes = atom::Intern("endnotes");

    return TRUE;
}
