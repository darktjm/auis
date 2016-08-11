/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/txtvps.C,v 1.27 1996/05/23 14:01:56 robr Exp $";
#endif

#include <andrewos.h>
#include <ctype.h>
#include <time.h>    

#include <rect.h>
#include <util.h>

#include <environ.H>
#include <atom.H>
#include <buffer.H>
#include <proctable.H>
#include <environment.H>
#include <viewref.H>
#include <fontdesc.H>
#include <view.H>
#include <style.H>
#include <stylesheet.H>
#include <textview.H>
#include <text.H>
#include <message.H>
#include <dictionary.H>
#include <matte.H>
#include <print.H>
#include <bp.H>
#include <bpv.H>
#include <headrtv.H>
#include <fnote.H>
#include <fnotev.H>
#include <content.H>
#include <tindex.H>

#include "txtvps.h"
#include "txtvpse.h"

#define DEBUG 1 /* ### */

static void DumpStyle(struct textps_style  *ts);
static long ParseUnit(long  oper, enum style_Unit  unit);
static struct textps_style *AllocStyle(struct textps_slurp  *slurp);
static void FreeStyle(struct textps_style  *ts);
static void PurgeStyleStack(struct textps_slurp  *slurp, boolean  all);
static void DefaultStyle(struct textps_style  *ts);
static int tabsortsplot(struct textps_tabentry  *e1 , struct textps_tabentry  *e2);
static void copy_textps_style(struct textps_style  *tsnew , struct textps_style  *ts);
static void PushStyle(struct textps_slurp  *slurp, void  *pushval, int  pushtype);
static void PopStyle(struct textps_slurp  *slurp);
static void SetStyleRun(struct textps_stylerun  *srun, struct textps_style  *ts);
static struct textps_lexstate_text *AddNewLexer(struct textps_slurp *slurp);
static void InitLexer(struct textps_slurp  *slurp, struct textps_lexstate_text  *clex);
static void LexerGetNext(struct textps_slurp  *slurp, struct textps_lexstate_text  *clex, long  startpos);
static int LexerGetChar(struct textps_lexstate_text  *clex);
static void LexerCopySubString(struct textps_lexstate_text  *clex, char  *dest, long  pos , long  len);
static struct textps_slurp *InitWordSlurper(class textview  *txtv);
static void SlurpWord(struct textps_slurp  *slurp);
static void SplitSlurped(struct textps_slurp  *slurp, int  split, long  width);
static int PurgeSlurpedWords(struct textps_slurp  *slurp, int  eattoword);
static void FinalizeWordSlurper(struct textps_slurp  *slurp);
static struct textps_line *LayoutOneLine(struct textps_slurp  *slurp, long  columnwidth /* in PS units */, int  *wordatptr, boolean  firstlineinpara, boolean  *lastlineinparaptr, int  *breakableptr, int  *eattowordptr, struct textps_simple_layout_state *stat);
static void DestroyLine(struct textps_line  *ln);
static void InitLayoutState(struct textps_layout_state *lstat);
static void LayoutText(class textview  *txtv, struct textps_layout_history  *history, struct textps_layout_state *lstat);
static struct textps_simple_layout_state *SimpleLayoutTextInit();
static void SimpleLayoutTextFinal(struct textps_simple_layout_state *stat);
static void SimpleLayoutText(struct textps_simple_layout_state *stat, class textview  *txtv, long layoutwidth, char *prependstring, class style *prependstyle);
static void DumpLayout(struct textps_layout  *ly);
static struct textps_layout_plan *CreateRectLayoutPlan(long width, long height);
static void CopyLinesToLayout(struct textps_layout **destptr, struct textps_srunlist *destrunlist, struct textps_layout *src, struct textps_srunlist *srcrunlist, long startline, long endline, long colnum, long trimpt);
static void DeleteLinesFromLayout(struct textps_layout *dest, long endln);
static void DeleteLinesFromFootThing(struct textps_simple_layout_state *stat, long endln);
static void DeleteWordsFromFootThing(struct textps_simple_layout_state *stat, long startln);
static void AddSpecialLineToLayout(struct textps_layout *dest, long lineflag, long lineheight, long colnum);
static class style *MagicFootnoteStyle(struct textps_slurp *slurp);
static void PrintPSText(class textview *self, FILE *outfile, long width, long height, long firstpagenum, boolean allowsubs);
static void DeleteTextviewProc(void *rock);
static void DeleteLayoutPlanProc(void *rock);

void textview_DestroyPrintingLayout(class textview  *txtv);
void textview_DestroyPrintingLayoutPlan(class textview  *txtv);

static struct font_afm *symbolfont = NULL;
static class atom *A_twocolumns, *A_swapheaders, *A_endnotes, *A_docontents, *A_doindex;

/* called from InitializeClass */
void textview_InitializePS()
{
    A_twocolumns = atom::Intern("twocolumns");
    A_swapheaders = atom::Intern("swapheaders");
    A_endnotes = atom::Intern("endnotes");
    A_docontents = atom::Intern("docontents");
    A_doindex = atom::Intern("doindex");
}

/* chi is an AGP-encoded value; result is in local (RESOLUTION) units */
#define ComputeCharWidth(chi, afm, sz) ((long)(print::ComputePSCharWidth(chi, afm, sz)*dRESOLUTION))

static void DumpStyle(struct textps_style  *ts)
{
    if (ts->pushtype==textps_pushtype_View) {
	printf("  View envrment\n");
    }
    else {
	printf("  Style envrment, afm 0x%p\n", ts->afm);
	/*printf("  Font |%s|, %d pt\n", ts->fontfamily, ts->fontsize);
	 printf("  linespacing: %d; paraspacing: %d\n", ts->linespacing, ts->paraspacing);
	 printf("  Face: %d; flags: %d\n", ts->fontfaces, ts->miscflags);*/
    }
}

/* compute RESOLUTION units */
static long ParseUnit(long  oper, enum style_Unit  unit)
{
    if (oper==0) return 0;

    switch (unit) {
	case style_Points:
	case style_RawDots:
	    return oper*RESOLUTION;
	case style_Inches:
	    /* factor out the 8 */
	    return (oper * (long)(9*RESOLUTION)) / 8192; 
	case style_CM:
	    /* 28/64735 is the correct value to within 1 part in 10e6. I expect no complaints. */
	    return (oper * (long)(28*RESOLUTION)) / 64735;
	default:
	    return 0;
    }
}

/* this puts in the styleheap (linked list). The caller must put it in the stylestack (array) and allocate all fields. */
static struct textps_style *AllocStyle(struct textps_slurp  *slurp)
{
    struct textps_style *res;

    res = (struct textps_style *)malloc(sizeof(struct textps_style));
    res->usage = 0;
    res->next = slurp->styleheap;
    slurp->styleheap = res;
    return res;
}

static void FreeStyle(struct textps_style  *ts)
{
    if (ts->fontfamily)
	free(ts->fontfamily);
    if (ts->tabfill)
	free(ts->tabfill);
    if (ts->tabs) {
	ts->tabs->refcount--;
	if (ts->tabs->refcount <= 0) {
	    free(ts->tabs->list);
	    ts->tabs->list = NULL;
	    free(ts->tabs);
	}
    }
    /* kill some pointers to chase out bugs */
    ts->fontfamily = NULL;
    ts->tabfill = NULL;
    ts->afm = NULL;
    ts->encoding = NULL;
    ts->leftmargin = 0;
    ts->tabs = NULL;
    ts->next = NULL;
    free(ts);
}

static void PurgeStyleStack(struct textps_slurp  *slurp, boolean  all)
{
    struct textps_style *ts, **pt;
    int killcount = 0;

    pt = (&slurp->styleheap);
    while (*pt) {
	if (all || !(*pt)->usage) {
	    ts = (*pt);
	    *pt = ts->next;
	    FreeStyle(ts);
	    killcount++;
	}
	else {
	    pt = (&((*pt)->next));
	}
    }

    /*printf("### PurgeStyleStack killed %d\n", killcount);*/
}

static void DefaultStyle(struct textps_style  *ts)
{
    char namebuf[256];
    int ix;
    short *enc;

    /* these values are based on the text printing defaults. */
    ts->leftmargin = 0*RESOLUTION; 
    ts->rightmargin = 0*RESOLUTION;
    ts->paraindent = 0;
    ts->baselineoffset = 0;
    ts->linespacing = 0;
    ts->paraspacing = 0;
    if (environ::GetProfileSwitch("justified", TRUE))
	ts->justification = style_LeftAndRightJustified;
    else
	ts->justification = style_LeftJustified;
    ts->miscflags = 0;

    {
	long fontSize;
	char *font = NULL;
	long fontStyle;

	if (environ::ProfileEntryExists("print.bodyfont", FALSE))
	    font = environ::GetProfile("print.bodyfont");
	else
	    font = environ::GetProfile("bodyfont");

	if (font == NULL
	    || !fontdesc::ExplodeFontName(font, namebuf, (long)sizeof(namebuf), &fontStyle, &fontSize)) {
	    strcpy(namebuf, "Andy");
	    fontSize = 12;
	    fontStyle = fontdesc_Plain;
	}

	ts->fontfamily = NewString(namebuf);
	ts->fontsize = fontSize;
	ts->realfontsize = ts->fontsize;
	ts->fontfaces = fontStyle;
    }

    ts->tabfill = NULL;

    ts->tabs = (struct textps_tablist *)malloc(sizeof(struct textps_tablist));
    ts->tabs->refcount = 1;
    ts->tabs->listsize = 32; /* 16 inches, at a tab every half-inch */
    ts->tabs->list = (struct textps_tabentry *)malloc(ts->tabs->listsize * sizeof(struct textps_tabentry));
    for (ix=0; ix<ts->tabs->listsize; ix++) {
	ts->tabs->list[ix].pos = (ix+1) * 36*RESOLUTION;
	ts->tabs->list[ix].kind = style_LeftAligned;
    }

    print::LookUpPSFont(namebuf, &enc, NULL, ts->fontfamily, ts->fontsize, ts->fontfaces);
    ts->afm = print::GetPSFontAFM(namebuf);
    ts->fontindex = print::PSRegisterFont(namebuf);
    ts->encoding = enc;
}

static int tabsortsplot(struct textps_tabentry  *e1 , struct textps_tabentry  *e2)
{
    int res = (e1)->pos - (e2)->pos;
    if (res)
	return res;
    if (e1->kind == style_TabClear)
	return (-1);
    if (e2->kind == style_TabClear)
	return (1);
    return 0;
}

static void copy_textps_style(struct textps_style  *tsnew , struct textps_style  *ts)
{
    tsnew->fontfamily = NewString(ts->fontfamily);
    tsnew->realfontsize = ts->realfontsize;
    tsnew->fontfaces = ts->fontfaces;
    if (tsnew->realfontsize < 1)
	tsnew->fontsize = 1;
    else
	tsnew->fontsize = tsnew->realfontsize;
    tsnew->afm = ts->afm;
    tsnew->fontindex = ts->fontindex;
    tsnew->encoding = ts->encoding;
    tsnew->miscflags = ts->miscflags;
    tsnew->justification = ts->justification;
    tsnew->paraindent = ts->paraindent;
    tsnew->baselineoffset = ts->baselineoffset;
    tsnew->linespacing = ts->linespacing;
    tsnew->paraspacing = ts->paraspacing;
    tsnew->leftmargin = ts->leftmargin;
    tsnew->rightmargin = ts->rightmargin;
    if (ts->tabfill)
	tsnew->tabfill = NewString(ts->tabfill);
    else
	tsnew->tabfill = NULL;
    tsnew->tabs = ts->tabs;
    tsnew->tabs->refcount++;
}

/* pushes a style or environment onto the stylestack, and sets slurp->curstyle to the new value. slurp->lexernum should already be set to the current lexer value.
 pushtype can be textps_pushtype_Env for an environment, textps_pushtype_Style for a style, textps_pushtype_View for a view, textps_pushtype_None for nothing. Note that View and None do about the same thing, since the view-handling part of the layout code is in SlurpWord. (Env's length is taken from the environment; Style and None are assumed to cover the whole lexer object) */
static void PushStyle(struct textps_slurp  *slurp, void  *pushval, int  pushtype)
{
    struct textps_style *ts, *tsnew;
    char *cx;
    enum style_MarginValue RefBasis;
    long RefOperand;
    enum style_Unit RefUnit;
    enum style_Justification Just;
    enum style_FontSize RefBasisFS;
    enum style_ScriptMovement RefBasisSM;
    enum style_SpacingValue RefBasisSV;
    long RefNumTabChanges;
    struct tabentry **RefTabChanges;
    boolean afm_changed;
    char namebuf[256];
    short *enc;
    class style *sty = NULL;

    if (slurp->stylestack_num+1 >= slurp->stylestack_size) {
	slurp->stylestack_size *= 2;
	slurp->stylestack = (struct textps_style **)realloc(slurp->stylestack, sizeof(struct textps_style *) * slurp->stylestack_size);
    }

    ts = slurp->stylestack[slurp->stylestack_num];
    slurp->stylestack_num++;
    tsnew = AllocStyle(slurp);
    slurp->stylestack[slurp->stylestack_num] = tsnew;
    tsnew->lexlevel = slurp->lexernum;
    tsnew->usage |= textps_style_instack;

    slurp->curstyle = tsnew;

    switch (pushtype) {
	case textps_pushtype_Env: {
	    class environment *env = (class environment *)pushval;
	    tsnew->env = env;
	    tsnew->endpos = (env)->Eval() + (env)->GetLength();
	    if (env->type == environment_View) {
		pushtype = textps_pushtype_View;
	    }
	    else if (env->type == environment_Style) {
		pushtype = textps_pushtype_Style;
		sty = env->data.style;
	    }
	    else {
		pushtype = textps_pushtype_None;
	    }
	    }
	    break;
	case textps_pushtype_Style:
	    tsnew->endpos = slurp->lexers[slurp->lexernum].len;
	    tsnew->env = NULL;
	    sty = (class style *)pushval;
	    break;
	case textps_pushtype_View:
	    tsnew->endpos = slurp->lexers[slurp->lexernum].len;
	    tsnew->env = NULL;
	    break;
	case textps_pushtype_None:
	    tsnew->endpos = slurp->lexers[slurp->lexernum].len;
	    tsnew->env = NULL;
	    break;
    }

    /* pushtype should now be View, Style, or None */
    tsnew->pushtype = pushtype;
    /*printf("### pushing; type=%d (sty=0x%p) endpos=%d\n", pushtype, sty, tsnew->endpos);*/

    if (pushtype == textps_pushtype_None) {
	copy_textps_style(tsnew, ts);
    }
    else if (pushtype == textps_pushtype_View) {
	copy_textps_style(tsnew, ts);
    }
    else {
	/* sty should be set */

	afm_changed = FALSE;

	(sty)->GetFontSize( &RefBasisFS, &RefOperand);
	if (RefBasisFS==style_PreviousFontSize) {
	    tsnew->realfontsize = ts->realfontsize + RefOperand;
	}
	else {
	    tsnew->realfontsize = RefOperand;
	};
	if (tsnew->realfontsize < 1)
	    tsnew->fontsize = 1;
	else
	    tsnew->fontsize = tsnew->realfontsize;
	/* but we don't need to change the AFM */

	(sty)->GetFontFamily( namebuf, 254);
	if (namebuf[0]) {
	    tsnew->fontfamily = NewString(namebuf);
	    afm_changed = TRUE;
	}
	else {
	    tsnew->fontfamily = NewString(ts->fontfamily);
	}

	tsnew->fontfaces = (ts->fontfaces | (sty)->GetAddedFontFaces()) & (sty)->GetRemovedFontFaces();
	if (tsnew->fontfaces != ts->fontfaces)
	    afm_changed = TRUE;

	if (afm_changed) {
	    print::LookUpPSFont(namebuf, &enc, NULL, tsnew->fontfamily, tsnew->fontsize, tsnew->fontfaces);
	    tsnew->afm = print::GetPSFontAFM(namebuf);
	    tsnew->fontindex = print::PSRegisterFont(namebuf);
	    tsnew->encoding = enc;
	}
	else {
	    tsnew->afm = ts->afm;
	    tsnew->fontindex = ts->fontindex;
	    tsnew->encoding = ts->encoding;
	}

	tsnew->miscflags = (ts->miscflags | (sty->AddMiscFlags)) & (sty->OutMiscFlags);

	Just = (sty)->GetJustification();
	if (Just == style_PreviousJustification)
	    tsnew->justification = ts->justification;
	else
	    tsnew->justification = Just;

	(sty)->GetNewLeftMargin( &RefBasis, &RefOperand, &RefUnit);
	switch (RefBasis) {
	    case style_LeftMargin:
		tsnew->leftmargin = ts->leftmargin + ParseUnit(RefOperand, RefUnit);
		break;
	    case style_LeftEdge:
		tsnew->leftmargin = ParseUnit(RefOperand, RefUnit);
		break;
	    default:
		tsnew->leftmargin = ts->leftmargin;
		break;
	}
	(sty)->GetNewRightMargin( &RefBasis, &RefOperand, &RefUnit);
	switch (RefBasis) {
	    case style_RightMargin:
		tsnew->rightmargin = ts->rightmargin + ParseUnit(RefOperand, RefUnit);
		break;
	    case style_RightEdge:
		tsnew->rightmargin = ParseUnit(RefOperand, RefUnit);
		break;
	    default:
		tsnew->rightmargin = ts->rightmargin;
		break;
	}
	(sty)->GetNewIndentation( &RefBasis, &RefOperand, &RefUnit);    
	switch (RefBasis) {
	    case style_LeftMargin:
		/*tsnew->paraindent = ts->leftmargin + ParseUnit(RefOperand, RefUnit);*/ /* note that this is not a delta movement, but an offset from the previously-computed left margin position. */
		/* ap1i: I don't know why I did that. It's wrong. The following is what happens on screen, and I don't know what LeftEdge is supposed to look like, so I guess they're the same. */
		tsnew->paraindent = ParseUnit(RefOperand, RefUnit);
		break;
	    case style_LeftEdge:
		tsnew->paraindent = ParseUnit(RefOperand, RefUnit);
		break;
	    default:
		tsnew->paraindent = ts->paraindent;
		break;
	}
	(sty)->GetFontScript( &RefBasisSM, &RefOperand, &RefUnit);
	switch (RefBasisSM) {
	    case style_PreviousScriptMovement:
		tsnew->baselineoffset = ts->baselineoffset + ParseUnit(RefOperand, RefUnit);
		break;
	    case style_ConstantScriptMovement:
		tsnew->baselineoffset = ParseUnit(RefOperand, RefUnit);
		break;
	    default:
		tsnew->baselineoffset = ts->baselineoffset;
		break;
	};
	(sty)->GetNewInterlineSpacing( &RefBasisSV, &RefOperand, &RefUnit);
	switch (RefBasisSV) {
	    case style_ConstantSpacing:
		tsnew->linespacing = ParseUnit(RefOperand, RefUnit);
		break;
	    case style_InterlineSpacing:
		tsnew->linespacing = ts->linespacing + ParseUnit(RefOperand, RefUnit);
		break;
	    default:
		tsnew->linespacing = ts->linespacing;
		break;
	}

	(sty)->GetNewInterparagraphSpacing( &RefBasisSV, &RefOperand, &RefUnit);
	switch (RefBasisSV) {
	    case style_ConstantSpacing:
		tsnew->paraspacing = ParseUnit(RefOperand, RefUnit);
		break;
	    case style_InterparagraphSpacing:
		tsnew->paraspacing = ts->paraspacing + ParseUnit(RefOperand, RefUnit);
		break;
	    default:
		tsnew->paraspacing = ts->paraspacing;
		break;
	}

	cx = (sty)->GetAttribute("tabfill");
	if (cx) {
	    tsnew->tabfill = NewString(cx);
	}
	else {
	    if (ts->tabfill) {
		tsnew->tabfill = NewString(ts->tabfill);
	    }
	    else {
		tsnew->tabfill = NULL;
	    }
	}

	(sty)->GetTabChangeList( &RefNumTabChanges, &RefTabChanges);
	if (RefNumTabChanges==0) {
	    tsnew->tabs = ts->tabs;
	    tsnew->tabs->refcount++;
	}
	else {
	    int numold = ts->tabs->listsize;
	    int oldpos, newpos;
	    struct textps_tabentry *baby, *newlist;
	    int babysize;
	    long val;

	    newlist = (struct textps_tabentry *)malloc((RefNumTabChanges) * sizeof(struct textps_tabentry));
	    for (newpos=0; newpos<RefNumTabChanges; newpos++) {
		if (RefTabChanges[newpos]->TabOpcode == style_AllClear)
		    val = -999999;
		else
		    val = ParseUnit(RefTabChanges[newpos]->Location, RefTabChanges[newpos]->LocationUnit);
		newlist[newpos].pos = val;
		newlist[newpos].kind = RefTabChanges[newpos]->TabOpcode;
	    }
	    free(RefTabChanges); /* GetTabChangeList mallocs this for us; we get to throw it away */
	    RefTabChanges = NULL;
	    qsort((void *)newlist, RefNumTabChanges, sizeof(struct textps_tabentry), QSORTFUNC(tabsortsplot)); /* use QSORTFUNC? */

	    oldpos = 0;
	    newpos = 0;
	    while (newpos < RefNumTabChanges && newlist[newpos].kind == style_AllClear) {
		newpos++;
		numold = 0;
	    }
	    baby = (struct textps_tabentry *)malloc((numold + RefNumTabChanges) * sizeof(struct textps_tabentry));
	    babysize = 0;
	    while (oldpos < numold || newpos < RefNumTabChanges) {
		if (oldpos >= numold || (newpos < RefNumTabChanges && newlist[newpos].pos <= ts->tabs->list[oldpos].pos)) {
		    val = newlist[newpos].pos;
		    if (newlist[newpos].kind == style_TabClear) {
			/* do nothing; this position will be skipped */
		    }
		    else {
			baby[babysize] = newlist[newpos];
			babysize++;
		    }
		    /* jump over old and new tabs at this pos. */
		    while (oldpos < numold && ts->tabs->list[oldpos].pos <= val) {
			oldpos++;
		    }
		    while (newpos < RefNumTabChanges && newlist[newpos].pos <= val) {
			newpos++;
		    }
		}
		else {
		    baby[babysize] = ts->tabs->list[oldpos];
		    babysize++;
		    oldpos++;
		}
	    }

	    free(newlist); /* that was temporary */
	    tsnew->tabs = (struct textps_tablist *)malloc(sizeof(struct textps_tablist));
	    tsnew->tabs->list = baby;
	    tsnew->tabs->listsize = babysize;
	    tsnew->tabs->refcount = 1;
	}

	/* all style changes in place */

    }

    if (slurp->runs.num >= slurp->runs.size) {
	slurp->runs.size *= 2;
	slurp->runs.r = (struct textps_stylerun *)realloc(slurp->runs.r, sizeof(struct textps_stylerun) * slurp->runs.size);
    }

    SetStyleRun(&(slurp->runs.r[slurp->runs.num]), tsnew);
    tsnew->runnum = slurp->runs.num;
    slurp->runs.num++;

    /*DumpStyle(tsnew);*/ /* ### */

}

static void PopStyle(struct textps_slurp  *slurp)
{
    struct textps_style *ts = slurp->stylestack[slurp->stylestack_num];

    ts->usage &= (~textps_style_instack);

    slurp->stylestack_num--;
    slurp->curstyle = slurp->stylestack[slurp->stylestack_num];
}

static void SetStyleRun(struct textps_stylerun  *srun, struct textps_style  *ts)
{
    if (ts->pushtype == textps_pushtype_View) {
	srun->miscflags = 0;
	srun->afm = NULL;
	srun->encoding = NULL;
	srun->fontindex = 0;
	srun->baselineoffset = ts->baselineoffset;
	srun->fontsize = ts->fontsize; /* keep this around so that we can measure how far down to translate the inset (ideally, it starts at the top of the text line) */
	strcpy(srun->tabfill, "");
	srun->tabfillwidth = 0;
    }
    else {
	srun->afm = ts->afm;
	srun->encoding = ts->encoding;
	srun->fontindex = ts->fontindex;
	srun->fontsize = ts->fontsize;
	srun->baselineoffset = ts->baselineoffset;
	srun->miscflags = ts->miscflags;
	if (ts->tabfill) {
	    long lx;
	    char *cx;
	    unsigned char let;
	    strncpy(srun->tabfill, ts->tabfill, MAXTABFILLLEN);
	    srun->tabfill[MAXTABFILLLEN] = '\0';
	    for (lx=0, cx=srun->tabfill; let=(unsigned char)(*cx); cx++) {
		lx += ComputeCharWidth(srun->encoding[let], srun->afm, srun->fontsize);
	    }
	    srun->tabfillwidth = lx;
	}
	else {
	    strcpy(srun->tabfill, "");
	    srun->tabfillwidth = 0;
	}
    }
    srun->locatag = NULL;
}

static struct textps_locatag *AllocLocatag(struct textps_slurp *slurp)
{
    struct textps_locatag *res;
    res = (struct textps_locatag *)malloc(sizeof(struct textps_locatag));
    res->next = NULL;
    res->type = locatag_Undef;
    res->linenum = locatag_Undef;
    res->pagenum = locatag_Undef;
    res->name = NULL;
    (*slurp->locatags) = res;
    slurp->locatags = (&(res->next));
    return res; 
}

static void DestroyLocatags(struct textps_locatag *root)
{
    struct textps_locatag *next;

    while (root) {
	next = root->next;
	root->next = NULL;
	if (root->name)
	    free(root->name);
	root->name = NULL;
	free(root);
	root = next;
    }
}

/* slight misnomer -- it resolves layoutplans also */
static void ResolveLocatags(struct textps_slurp *slurp, struct textps_line *ln, long linenum)
{
    int wx;
    struct textps_stylerun *srun;

    for (wx=0; wx<ln->numwords; wx++) {
	if (ln->words[wx].start < 0) {
	    if (ln->words[wx].start == wordflag_Spec
		&& ln->words[wx].inset.type == textview_Layout
		&& slurp->history) {
		struct textview_getlayout *tmp = (&ln->words[wx].inset.u.Layout);
		struct textps_layout_plan *newplan;
		if (tmp->func == slurp->history->initiallayoutfunc && tmp->rock == slurp->history->initiallayoutrock) {
		    /* do nothing -- already installed this one, before parsing began. */
		}
		else {
		    newplan = tmp->func(tmp->rock, slurp->history->pagew, slurp->history->pageh);
		    print::PSRegisterCleanup(DeleteLayoutPlanProc, (void *)newplan);
		    slurp->history->PushNewPlan(newplan);
		}
	    }
	}
	else {
	    srun = (&slurp->runs.r[ln->words[wx].run]);
	    if (srun->locatag && srun->locatag->linenum == locatag_Undef)
		srun->locatag->linenum = linenum;
	}
    }

/*### for all (real) words, check the style. If the style is linked to a locatag, and the locatag is not yet given a line number, give it this one.
### for all texttag words, ditto.*/
}

/* add a locatag to the list, using the given text. Chop off leading whitespace, trailing whitespace, and anything after a newline. */
static void AddContentLabel(struct textps_slurp *slurp, int lev, struct text *txt, long pos, long len, int labeltype)
{
    char *str, *cx, *cx2;
    struct textps_locatag *res;

    str = (char *)malloc(sizeof(char) * len+1);
    txt->CopySubString(pos, len, str, FALSE);
    str[len] = '\0';

    for (cx=str;
	 *cx==' ' || *cx=='\t' || *cx=='\n' || *cx=='\r';
	 cx++);
    for (cx2 = cx;
	 !(*cx2=='\0' || *cx2=='\n' || *cx2=='\r');
	 cx2++);
    for (cx2--;
	 *cx2==' ' || *cx2=='\t';
	 cx2--);
	 
    *(cx2+1) = '\0';

    if (cx != str) {
	for (cx2=str; *cx; cx++, cx2++)
	    *cx2 = *cx;
	*cx2 = '\0';
    }

    res = AllocLocatag(slurp);
    res->name = str;
    res->type = labeltype;
    slurp->runs.r[slurp->runs.num-1].locatag = res;
}

static struct textps_lexstate_text *AddNewLexer(struct textps_slurp *slurp)
{
    struct textps_lexstate_text  *clex;

    slurp->lexernum++;
    if (slurp->lexernum >= slurp->lexers_size) {
	while (slurp->lexernum >= slurp->lexers_size)
	    slurp->lexers_size *= 2;
	slurp->lexers = (struct textps_lexstate_text *)realloc(slurp->lexers, sizeof(struct textps_lexstate_text) * slurp->lexers_size);
    }
    clex = (&(slurp->lexers[slurp->lexernum]));

    clex->type = 0;
    clex->txt = NULL;
    clex->txtv = NULL;
    clex->str = NULL;
    clex->sty = NULL;
    clex->len = 0;
    clex->pos = 0;
    clex->nextenvpos = 0;
    clex->nextenvlen = 0;
    return clex;
}

static void InitLexer(struct textps_slurp  *slurp, struct textps_lexstate_text  *clex)
{
    class environment *ex, *curenv;
    class environment *nextenv;

    clex->pos = 0;

    switch (clex->type) {
	case textps_lexer_Text:
	    curenv = clex->txt->rootEnvironment;
	    PushStyle(slurp, curenv, textps_pushtype_Env);
	    ex = (class environment *)(curenv)->GetChild( clex->pos);
	    while (ex) {
		PushStyle(slurp, ex, textps_pushtype_Env);
		if (slurp->contents && ex->type==environment_Style) {
		    char *nm = ex->data.style->GetName();
		    int lev;
		    lev = content::StyleNameContentLevel(nm);
		    if (lev) {
			AddContentLabel(slurp, lev, clex->txt, ex->Eval(), ex->GetLength(), locatag_Content);
		    }
		    lev = tindex::StyleNameIndexLevel(nm);
		    if (lev) {
			AddContentLabel(slurp, lev, clex->txt, ex->Eval(), ex->GetLength(), locatag_Index);
		    }
		}
		curenv = ex;
		ex = (class environment *)(curenv)->GetChild( clex->pos);
	    }

	    nextenv = (class environment *)(curenv)->GetNextChild( NULL, clex->pos);
	    if (nextenv)
		clex->nextenvpos = (nextenv)->Eval();
	    else
		clex->nextenvpos = clex->len+2;
	    /*printf("### nextenvpos = %d; cur->end = %d\n", clex->nextenvpos, slurp->curstyle->endpos);*/
	    break;
	case textps_lexer_CharArray:
	    if (clex->sty)
		PushStyle(slurp, clex->sty, textps_pushtype_Style);
	    else
		PushStyle(slurp, NULL, textps_pushtype_None);
	    clex->nextenvpos = clex->len+2;
	    /*printf("### nextenvpos = %d; cur->end = %d\n", clex->nextenvpos, slurp->curstyle->endpos);*/
	    break;
    }
}

static void LexerGetNext(struct textps_slurp  *slurp, struct textps_lexstate_text  *clex, long  startpos)
{
    class environment *nextenv = NULL;

    switch (clex->type) {
	case textps_lexer_Text:
	    if (slurp->curstyle->env)
		nextenv = (class environment *)(slurp->curstyle->env)->GetNextChild( NULL, startpos);
	    if (nextenv)
		clex->nextenvpos = (nextenv)->Eval();
	    else
		clex->nextenvpos = clex->len+2;
	    /*printf("### clex->nextenvpos = %d; cur->end = %d\n", clex->nextenvpos, slurp->curstyle->endpos);*/
	    break;
	case textps_lexer_CharArray:
	    clex->nextenvpos = clex->len+2;
	    break;
    }
}

static int LexerGetChar(struct textps_lexstate_text  *clex)
{
    switch (clex->type) {
	case textps_lexer_Text:
	    return (clex->txt)->GetUnsignedChar( clex->pos);

	case textps_lexer_CharArray:
	    return clex->str[clex->pos];

	default:
	    return '@';
    }
}

static void LexerCopySubString(struct textps_lexstate_text  *clex, char  *dest, long  pos , long  len)
{
    switch (clex->type) {
	case textps_lexer_Text:
	    (clex->txt)->CopySubString( pos, len, dest, FALSE);
	    break;

	case textps_lexer_CharArray:
	    memcpy(dest, clex->str+pos, len);
	    break;

	default:
	    memset(dest, '@', len);
	    break;
    }
}

/* this is called before the new lexer is created and pushed in */
static class style *MagicFootnoteStyle(struct textps_slurp *slurp)
{
    static class style *sty = NULL;
    struct textps_style *tmpsty;
    long val;
    enum style_FontSize RefBasis;
    class style *prevsty;

    if (!sty) {
	sty = new style;
	sty->SetName("footnotebody");
	sty->SetFontScript(style_PreviousScriptMovement, -6, style_Points);
    }

    /* now, the current style is the footnote view env, so we're gonna look at the style before that. If that's a style called "footnote" which is exactly length==1, we'll cancel any attributes it has. If not, use the default. */
    tmpsty = NULL;
    if (slurp->stylestack_num > 0)
	tmpsty = slurp->stylestack[slurp->stylestack_num-1];
    if (tmpsty
	&& tmpsty->pushtype == textps_pushtype_Style
	&& tmpsty->env->type == environment_Style
	&& tmpsty->env->GetLength() == 1
	&& !strcmp(tmpsty->env->data.style->GetName(), "footnote")) {
	prevsty = tmpsty->env->data.style;
	if (prevsty->IsOverBarAdded())
	    sty->RemoveOverBar();
	else
	    sty->UseOldOverBar();
	prevsty->GetFontSize(&RefBasis, &val);
	val = (-val) - 4;
	sty->SetFontSize(style_PreviousFontSize, val);
    }
    else {
	// printf("not cancelling footnote style\n");
	sty->UseOldOverBar();
	sty->SetFontSize(style_PreviousFontSize, -4);
    }

    return sty;
}

/* txtv may be NULL, in which case add no lexer. */
static struct textps_slurp *InitWordSlurper(class textview  *txtv)
{
    class environment *ex;
    struct textps_slurp *slurp;
    struct textps_lexstate_text *clex;

    slurp = (struct textps_slurp *)malloc(sizeof(struct textps_slurp));

    slurp->tmpwords_size = 20; 
    slurp->tmpwords = (struct textps_tmp_word *)malloc(sizeof(struct textps_tmp_word) * slurp->tmpwords_size);
    slurp->tmpletters_size = 80; 
    slurp->tmpletters = (char *)malloc(slurp->tmpletters_size);
    slurp->stylestack_size = 4; 
    slurp->stylestack = (struct textps_style **)malloc(sizeof(struct textps_style *) * slurp->stylestack_size);
    slurp->styleheap = NULL;
    slurp->numwords = 0;
    slurp->wordseaten = 0;
    slurp->letters_pt = 0;

    slurp->runs.size = 8; 
    slurp->runs.r = (struct textps_stylerun *)malloc(sizeof(struct textps_stylerun) * slurp->runs.size);
    slurp->runs.num = 0;

    slurp->stylestack_num = 0;
    slurp->stylestack[slurp->stylestack_num] = AllocStyle(slurp);
    DefaultStyle(slurp->stylestack[slurp->stylestack_num]);
    slurp->stylestack[slurp->stylestack_num]->usage = textps_style_instack;

    slurp->lexers_size = 2;
    slurp->lexers = (struct textps_lexstate_text *)malloc(sizeof(struct textps_lexstate_text) * slurp->lexers_size);
    slurp->lexernum = (-1);

    slurp->contents = TRUE; /* until we think of a better way */
    slurp->locatags_root = NULL;
    slurp->locatags = (&slurp->locatags_root);

    if (txtv) {
	clex = AddNewLexer(slurp);
	clex->type = textps_lexer_Text;
	clex->txt = (class text *)(txtv)->GetDataObject();
	clex->txtv = txtv;
	clex->len = (clex->txt)->GetLength();
	InitLexer(slurp, clex);
    }

    slurp->footnotenumber = 0;
    slurp->history = NULL;

    return slurp;
}

/* pull in some more words */
static void SlurpWord(struct textps_slurp  *slurp)
{
    long origpos, origlet, endpos;
    int thisword;
    long wordwidth, wix;
    struct textps_tmp_word *wordpt;
    int thech;
    boolean nextcharnewline;
    int breakable;
    class environment *ex, *nextenv;
    struct textps_lexstate_text *clex;

    if (slurp->lexernum < 0) {
	/*printf("### uh, no lexer in place.\n");*/
	return;
    }

    clex = (&(slurp->lexers[slurp->lexernum]));

    if (slurp->numwords>0 && slurp->tmpwords[slurp->numwords-1].breakable == 5) {
	/* end of file, you turkey! */
	/*printf("### (3) hit the turkey-marker, slurp->numwords=%d.\n", slurp->numwords);*/
	return;
    }

    if (slurp->numwords >= slurp->tmpwords_size) {
	slurp->tmpwords_size *= 2;
	slurp->tmpwords = (struct textps_tmp_word *)realloc(slurp->tmpwords, sizeof(struct textps_tmp_word) * slurp->tmpwords_size);
    }

    if (clex->pos == slurp->curstyle->endpos) {
	while (clex->pos == slurp->curstyle->endpos) {
	    PopStyle(slurp);
	    /*printf("##  popped; endpos now %d\n", slurp->curstyle->endpos); fflush(stdout);*/
	}
	LexerGetNext(slurp, clex, clex->pos-1);
    }

    if (clex->pos == clex->nextenvpos) {
	switch (clex->type) {
	    case textps_lexer_Text:
		if (slurp->curstyle->env) {
		    ex = (class environment *)(slurp->curstyle->env)->GetChild( clex->pos);
		    while (ex) {
			PushStyle(slurp, ex, textps_pushtype_Env);
			if (slurp->contents && ex->type==environment_Style) {
			    char *nm = ex->data.style->GetName();
			    int lev;
			    lev = content::StyleNameContentLevel(nm);
			    if (lev) {
				AddContentLabel(slurp, lev, clex->txt, ex->Eval(), ex->GetLength(), locatag_Content);
			    }
			    lev = tindex::StyleNameIndexLevel(nm);
			    if (lev) {
				AddContentLabel(slurp, lev, clex->txt, ex->Eval(), ex->GetLength(), locatag_Index);
			    }
			}
			ex = (class environment *)(slurp->curstyle->env)->GetChild( clex->pos);
		    }
		}
		/* scan for next environment after the ones we just sucked in. */
		LexerGetNext(slurp, clex, clex->pos);
		break;
	    case textps_lexer_CharArray:
		clex->nextenvpos = clex->len+2;	
		break;
	}
    }

    thisword = slurp->numwords;
    wordpt = (&slurp->tmpwords[thisword]);
    
    if (slurp->curstyle->lexlevel == slurp->lexernum
	&& slurp->curstyle->pushtype == textps_pushtype_View) {
	/* "view" case */
	class view *v;
	class viewref *vref;
	class textview *parview;
	class style *styx;
	struct textview_insetdata *insdat;

	clex->pos = slurp->curstyle->endpos; /* skip length of view environment */
	slurp->curstyle->usage |= textps_style_inline;

	vref = NULL;
	v = NULL;
	if (slurp->curstyle->env && slurp->curstyle->env->type == environment_View)
	    vref = slurp->curstyle->env->data.viewref;
	if (vref) {
	    parview = (&(slurp->lexers[slurp->lexernum]))->txtv;
	    v = (class view *)dictionary::LookUp(parview, (char *) vref);

	    if (v == NULL) {
		ATK::LoadClass(vref->viewType);
		if (ATK::IsTypeByName(vref->viewType, "view")
		    && (v = (class view *)
			matte::Create(vref, (class view *) parview)) != NULL) {
		    (vref)->AddObserver( parview);
		    dictionary::Insert(parview, (char *) vref, (char *) v);
		}
	    }
	    else {
		if (v == (class view *) textview_UNKNOWNVIEW)
		    v = NULL;
	    }
	}

	if (slurp->curstyle->miscflags & style_Hidden) {
	    /* this ensures that the view is ignored, and SlurpWord re-called. */
	    v = NULL; 
	}

	if (v && (insdat =
		  (struct textview_insetdata *)v->GetPSPrintInterface("text"))) {
	    /* note that in this case, we cannot query v using the methods implied by insdat->type! This is because v itself might be a wrapper which just passed insdat up from inside it. */
	    switch (insdat->type) {
		case textview_StitchText:
		case textview_StitchString:
		    /* stitching inset  */
		    /* push a new lexer! */
		    clex = AddNewLexer(slurp);
		    if (insdat->type==textview_StitchString) {
			textview_getstring_fptr fpt = insdat->u.StitchString.func;
			void *rock = insdat->u.StitchString.rock;
			class style *strsty;
			clex->type = textps_lexer_CharArray;
			/* call the function and get a string. Note that the string must be a malloc()ed block, and the caller (us) will take care of free()ing it. */
			clex->str = (*fpt)(rock, &strsty);
			clex->sty = strsty;
			clex->len = strlen(clex->str);
		    }
		    else {
			clex->type = textps_lexer_Text;
			clex->txtv = insdat->u.StitchText;
			clex->txt = (class text *)(clex->txtv)->GetDataObject();
			clex->len = (clex->txt)->GetLength();
		    }
		    InitLexer(slurp, clex);
		    SlurpWord(slurp);
		    return;
		case textview_Layout:
		case textview_NewPage:
		case textview_Header:
		case textview_Footnote:
		    /* create a special word */
		    wordpt->start = wordflag_Spec;
		    wordpt->len = 0;
		    wordpt->spaces = 0;
		    wordpt->run = slurp->curstyle->runnum;
		    wordpt->style = slurp->curstyle;
		    wordpt->inset = (*insdat);
		    slurp->curstyle->usage |= textps_style_inline;
		    switch (insdat->type) {
			case textview_NewPage:
			    breakable = 4;
			    /* if we ever make a column-break option, it should set breakable = 3 here */
			    break;
			case textview_Footnote:
			    styx = MagicFootnoteStyle(slurp);
			    slurp->footnotenumber++;
			    wordpt->footnotenumber = slurp->footnotenumber;
			    /* do some severe hackery to get the superscripted number out (into the main text). */
			    wordpt->breakable = 0;
			    slurp->numwords++;
			    /* push a new lexer! */
			    clex = AddNewLexer(slurp);
			    /*class style *strsty;*/
			    clex->type = textps_lexer_CharArray;
			    clex->str = (char *)malloc(12); /* this must be a non-empty string */
			    sprintf(clex->str, "%d", wordpt->footnotenumber);
			    clex->sty = styx;
			    clex->len = strlen(clex->str);
			    InitLexer(slurp, clex);
			    return;
			case textview_Header:
			    breakable = 1;
			    break;
			case textview_Layout:
			    breakable = 1;
			    break;
			default:
			    break;
		    }
		    break;
		default:
		    /* Unknown inset type. We do nothing; just re-call SlurpWord to bring in another word. */
		    SlurpWord(slurp);
		    return;
	    }
	}
	else if (v && v->GetPSPrintInterface("generic")) {
	    /* generic inset (in a rectangle) */

	    long in_w, in_h, out_w, out_h;
	    in_w = 72; /* we ask for one inch square, PS units */
	    in_h = 72;
	    v->DesiredPrintSize(in_w, in_h, view_NoSet, &out_w, &out_h);
	    /*printf("### ...asked %d,%d; replied %d,%d (PS units)\n", in_w, in_h, out_w, out_h);*/
	    /*voffset = vheight - (ts->fontsize * RESOLUTION) + ts->baselineoffset; ### */

	    wordpt->deswidth = out_w * RESOLUTION;
	    wordpt->desheight = out_h * RESOLUTION;
	    wordpt->width = 0; /* to be set later, during LayoutOneLine */
	    wordpt->vheight = 0;
	    wordpt->voff=0;
	    wordpt->v = v;

	    wordpt->start = wordflag_View;
	    wordpt->len = 0;
	    wordpt->spaces = 0;
	    wordpt->run = slurp->curstyle->runnum;
	    wordpt->style = slurp->curstyle;
	    slurp->curstyle->usage |= textps_style_inline;
	    breakable = 1;
	}
	else {
	    /* no view, or no known print interface, or hidden inset. We do nothing; just re-call SlurpWord to bring in another word. */
	    SlurpWord(slurp);
	    return;
	}
    } /* end of "view" case */
    else if (LexerGetChar(clex) == '\t') {
	/* "tab" case */
	/* break after tab. This covers all cases like new style runs, etc. */
	clex->pos++;
	wordpt->start = wordflag_Tab;
	wordpt->len = 0;
	wordpt->spaces = 0; 
	wordpt->width = 0;
	wordpt->run = slurp->curstyle->runnum;
	wordpt->style = slurp->curstyle;
	slurp->curstyle->usage |= textps_style_inline;
	breakable = 1;
    } /* end of "tab" case */
    else {
	/* "no tab" case */
	origpos = clex->pos;
	wordpt->start = slurp->letters_pt;
	wordwidth = 0;
	breakable = 0;
	while (clex->pos < slurp->curstyle->endpos
	       && clex->pos < clex->nextenvpos
	       && clex->pos < clex->len) {
	    thech = LexerGetChar(clex);
	    if (thech == ' ' || thech == '\t' || thech == '\n' || thech == '\r')
		break;
	    wix = ComputeCharWidth(slurp->curstyle->encoding[thech], slurp->curstyle->afm, slurp->curstyle->fontsize);
	    wordwidth += wix;
	    clex->pos++;
	} 
	/* break on a space, newline, tab, the first char of a new style run, or the char after the last char in the file. */

	wordpt->len = clex->pos - origpos;
	origlet = slurp->letters_pt;
	slurp->letters_pt += wordpt->len;
	if (slurp->letters_pt >= slurp->tmpletters_size) {
	    while (slurp->letters_pt >= slurp->tmpletters_size)
		slurp->tmpletters_size *= 2;
	    slurp->tmpletters = (char *)realloc(slurp->tmpletters, slurp->tmpletters_size);
	}
	LexerCopySubString(clex, slurp->tmpletters+origlet, origpos, wordpt->len);
	endpos = clex->pos;
	while (clex->pos<clex->len && !(clex->pos == slurp->curstyle->endpos || clex->pos == clex->nextenvpos) && LexerGetChar(clex) == ' ') {
	    breakable = 1;
	    clex->pos++;
	}
	wordpt->spaces = (clex->pos - endpos) * ComputeCharWidth(slurp->curstyle->encoding[' '], slurp->curstyle->afm, slurp->curstyle->fontsize); /* may be 0 */
	wordpt->width = (wordwidth + wordpt->spaces);
	wordpt->run = slurp->curstyle->runnum;
	wordpt->style = slurp->curstyle;
	slurp->curstyle->usage |= textps_style_inline;
    } /* end of "no tab" case */

    /* if we get here, we have a word. *Except* that if it was a footnote, we did the slurp->numwords++ already and returned, so we *didn't* get here. Sucks, donit? */
    slurp->numwords++;

    thech = LexerGetChar(clex);
    nextcharnewline = (thech == '\n' || thech == '\r');
    if (!(clex->pos == slurp->curstyle->endpos || clex->pos == clex->nextenvpos) && nextcharnewline) {
	breakable = 2;
	clex->pos++;
    }
    wordpt->breakable = breakable;

    if (clex->pos >= clex->len) {
	/* end of this lexer object */
	if (clex->str)
	    free(clex->str);
	if (slurp->lexernum == 0) {
	    /* neatness would indicate that we should pop the remaining styles off in this case as well. However, it doesn't make any difference, and it would require special-casing to avoid reading slurp->stylestack[-1]. So forget it. */
	    wordpt->breakable = 5;
	    return; /* ### This ignores the hidden flag on the last word. This is bad. I can't think of a better solution, though; if we called back, we'd just get the turkey-marker. Maybe create a fake zero-length word? */
	}
	else {
	    int popcount = 0;
	    while (slurp->curstyle->lexlevel == slurp->lexernum) {
		PopStyle(slurp);
		popcount++;
	    };
	    /*printf("### (3) popped off %d styles\n", popcount);*/

	    slurp->lexernum--;
	    wordpt->breakable = 1;
	}
    }

    if (wordpt->style->miscflags & style_Hidden) {
	slurp->numwords--;
	/* pitch it, start all over! Yuk. */
	SlurpWord(slurp);
	return;
    }
}

/* split word split into two parts, so that no more than width of it is in the first part. Do not try to split a wordflag_ word. */
/* note: this can realloc slurp->tmpwords! */
static void SplitSlurped(struct textps_slurp  *slurp, int  split, long  width)
{
    long wordwidth, wix;
    int thech, chpos;
    int ix;
    struct textps_tmp_word *wd = &(slurp->tmpwords[split]);

    if (wd->start < 0) {
	fprintf(stderr, "text formatter: request to split a special word (%d).\n", wd->start);
	return;
    }

    wordwidth = 0;
    chpos = 0;

    do {
	thech = slurp->tmpletters[wd->start+chpos];
	wix = ComputeCharWidth(wd->style->encoding[thech], wd->style->afm, wd->style->fontsize);
	if (wordwidth+wix > width && chpos > 0) {
	    /* if possible, cut at least one letter */
	    break;
	}
	wordwidth += wix;
	chpos++;
    } while (chpos<wd->len);

    if (slurp->numwords >= slurp->tmpwords_size) {
	slurp->tmpwords_size *= 2;
	slurp->tmpwords = (struct textps_tmp_word *)realloc(slurp->tmpwords, sizeof(struct textps_tmp_word) * slurp->tmpwords_size);
    }

    for (ix = slurp->numwords-1; ix >= split; ix--) {
	slurp->tmpwords[ix+1] = slurp->tmpwords[ix]; /* copy whole structure */
    }
    slurp->numwords++;

    /* now words split and split+1 are identical structures; we have to diddle them. */
    wd = &(slurp->tmpwords[split]);
    wd[0].len = chpos;
    wd[0].width = wordwidth;
    wd[0].spaces = 0;
    wd[0].breakable = 1;
    wd[1].start += chpos;
    wd[1].len -= chpos;
    wd[1].width -= wordwidth;
}

/* take num words off the beginning of the slurp buffer. Returns the number of words actually purged. */
static int PurgeSlurpedWords(struct textps_slurp  *slurp, int  eattoword)
{
    long eattoletter;
    int ix;

    if (eattoword <= 0)
	return 0;

    if (eattoword > slurp->numwords) {
	fprintf(stderr, "text formatter: request to purge %d more words than there are buffered.\n", eattoword - slurp->numwords);
	eattoword = slurp->numwords;
    }

    eattoletter = 0;
    for (ix=eattoword-1; ix>=0; ix--)
	if (slurp->tmpwords[ix].start >= 0) {
	    eattoletter = slurp->tmpwords[ix].start + slurp->tmpwords[ix].len;
	    break;
	}

    /* shift words down */
    for (ix=0; ix<slurp->numwords-eattoword; ix++) {
	/* copy whole structure */
	slurp->tmpwords[ix] = slurp->tmpwords[eattoword+ix];
	if (slurp->tmpwords[ix].start >= 0)
	    slurp->tmpwords[ix].start -= eattoletter;
    }
    slurp->numwords -= eattoword;
    slurp->wordseaten += eattoword;

    /* shift letters down */
    if (eattoletter) {
	for (ix=0; ix<slurp->letters_pt-eattoletter; ix++)
	    slurp->tmpletters[ix] = slurp->tmpletters[eattoletter+ix];
	slurp->letters_pt -= eattoletter;
    }

    return eattoword;
}

static void FinalizeWordSlurper(struct textps_slurp  *slurp)
{
    if (slurp->tmpwords)
	free(slurp->tmpwords);
    if (slurp->tmpletters)
	free(slurp->tmpletters);
    PurgeStyleStack(slurp, TRUE);
    if (slurp->stylestack)
	free(slurp->stylestack);
    if (slurp->runs.r)
	free(slurp->runs.r);
    if (slurp->locatags_root)
	DestroyLocatags(slurp->locatags_root);

    free(slurp);
}

static void DeleteTextviewProc(void *rock)
{
    class textview *tv;
    tv = (class textview *)rock;
    tv->Destroy();
}

static void DeleteLayoutPlanProc(void *rock)
{
    class textps_layout_plan *pl;
    pl = (class textps_layout_plan *)rock;
    delete pl;
}

static struct textps_line *LayoutOneLine(struct textps_slurp  *slurp, long  columnwidth /* in PS units */, int  *wordatptr, boolean  firstlineinpara, boolean  *lastlineinparaptr, int  *breakableptr, int  *eattowordptr, struct textps_simple_layout_state *stat)
{
    int wordat;
    boolean lastlineinpara;

    int ix, jx, lix;
    int tabnum;
    long copyletstart, copyletend;
    struct textps_line *ln;
    struct textps_tmp_word *thiswd;
    int firstwordat, eattoword, lasttabpos;
    long leftmargin, rightmargin;
    long lineheight, baseline;
    long wordwidth, wix, linewidth, unbrokenlength;
    long widthsofar, extraspace, totalspace;
    enum style_Justification just;
    boolean wordovermargin, nextcharnewline;
    struct textps_style *linestyle;
    int breakable;

    /* localize vars */
    /* breakable and eattoword are output-only */
    firstwordat = (*wordatptr);
    lastlineinpara = (*lastlineinparaptr);

    if (columnwidth<0)
	columnwidth = 0;

    /* begin munching */
    wordat = firstwordat;
    widthsofar = 0;
    tabnum = 0;
    eattoword = firstwordat;

    while (wordat >= slurp->numwords) {
	SlurpWord(slurp);
    }
    linestyle = slurp->tmpwords[firstwordat].style; /* will definitely be there */
    linestyle->usage |= textps_style_inlinestart; /* .. because of this */

    linewidth = (columnwidth*RESOLUTION) - (linestyle->leftmargin+linestyle->rightmargin);
    if (firstlineinpara)
	linewidth -= linestyle->paraindent;

    if (linewidth <= 0) 
	linewidth = 30;	/* give it something if the margins squeezed it to
	    nothing. */ 

    if (linewidth <= 0)
	linewidth = 72*RESOLUTION;	/* give it an inch if the margins squeezed it to nothing. */

    /* measure */

    /* tabs are measured from the beginning of the line (including margin and para-indent adjustments). This follows a patch that was checked in to drawtxtv.c rev 2.52, Tue Mar 23 17:34:30 1993 (V6 tree).
     Fortunately, this is the easy way to do it. */

    while (TRUE) {
	/* first, measure out words, assuming left-flush, and getting the tabs right. */

	/* do an unbreakable word string */

	breakable = 0;
	unbrokenlength = 0;
	while (breakable==0 && widthsofar+unbrokenlength < linewidth) {
	    while (wordat >= slurp->numwords) {
		SlurpWord(slurp);
	    }
	    thiswd = &(slurp->tmpwords[wordat]);
	    breakable = thiswd->breakable;
	    thiswd->tmppos = widthsofar+unbrokenlength;
	    if (thiswd->start >= 0)
		wordwidth = thiswd->width - thiswd->spaces;
	    else {
		struct textps_tablist *tabtab;
		switch (thiswd->start) {
		    case wordflag_View:
			thiswd->width = thiswd->deswidth;
			wordwidth = thiswd->width;
			break;
		    case wordflag_Tab:
			tabtab = linestyle->tabs;
			/* run tabnum up to the first tab which is past (not equal to) thiswd->tmppos */
			while (tabnum < tabtab->listsize && tabtab->list[tabnum].pos <= thiswd->tmppos)
			    tabnum++;
			if (tabnum >= tabtab->listsize) {
			    /* ran out */
			    thiswd->tmptabnum = (-1);
			    wordwidth = 0;
			}
			else {
			    thiswd->tmptabnum = tabnum;
			    switch (tabtab->list[tabnum].kind) {
				default:
				case style_LeftAligned:
				    wix = tabtab->list[tabnum].pos;
				    wordwidth = wix - thiswd->tmppos;
				    break;
				case style_RightAligned:
				    if (tabnum==0)
					wix = 0;
				    else
					wix = tabtab->list[tabnum-1].pos;
				    wordwidth = wix - thiswd->tmppos;
				    if (wordwidth<0)
					wordwidth = 0;
				    /* if tab starts before beginning of tab column, extend to it. if not, width is 0. */
				    tabnum++; /* ensure we won't get this tab again */
				    break;
				case style_CenteredOnTab:
				    wordwidth = 0;
				    tabnum++; /* ensure we won't get this tab again */
				    break;
			    }
			}
			break;
		    default:
			wordwidth = 0;
			break;
		}
	    }
	    unbrokenlength += wordwidth;
	    wordat++;
	    /*printf("  # wordat=%d, breakable=%d, unbrokelen=%d\n", wordat, breakable, unbrokenlength);*/
	}

	if (widthsofar+unbrokenlength >= linewidth) {

	    if (eattoword==firstwordat) {
		/* break this word, here */
		if (slurp->tmpwords[wordat-1].start == wordflag_Tab) {
		    /* it was a tab that overflowed. Break after it. */
		    eattoword = wordat;
		    lastlineinpara = FALSE;
		    breakable = 1;
		    widthsofar = linewidth; /* This tab ends at the right margin, ipso facto id est erg. */
		    break;
		}
		else if (slurp->tmpwords[wordat-1].start == wordflag_View) {
		    /* generic view overflowed. Cut it down. */
		    long splitlen = linewidth-(widthsofar+unbrokenlength-wordwidth);
		    slurp->tmpwords[wordat-1].width = splitlen;
		    eattoword = wordat;
		    lastlineinpara = FALSE;
		    breakable = 1;
		    widthsofar = linewidth; /* This view ends at the right margin, ipso facto id est erg. */
		    break;
		}
		else {
		    long splitlen = linewidth-(widthsofar+unbrokenlength-wordwidth);
		    SplitSlurped(slurp, wordat-1, splitlen);
		    eattoword = wordat;
		    lastlineinpara = FALSE;
		    breakable = 1;
		    widthsofar -= slurp->tmpwords[wordat].width; /* cut out the second half of the split */
		    break; /* eattoword is set */
		}
	    }
	    else {
		/* break at eattoword */
		lastlineinpara = FALSE;
		breakable = 1;
		if (slurp->tmpwords[eattoword-1].start >= 0)
		    widthsofar -= slurp->tmpwords[eattoword-1].spaces; /* cut out spaces from last word that will go on line */
		else {
		    /* do nothing; a tab at the end of a line has no spaces anyway. */
		}
		break; /* eattoword is set */
	    }
	}

	widthsofar += unbrokenlength + thiswd->spaces;
	eattoword = wordat;

	/*printf(" ## wordat=%d, breakable=%d, widthsofar=%d\n", wordat, breakable, widthsofar);*/
	if (breakable >= 2) {
	    /* end of line (or col, page, or document) */
	    lastlineinpara = TRUE;
	    /* widthsofar is correct -- should include spaces on end */
	    break;
	}
    }
    /* widthsofar should now be total width (wrt justification) */

    wordat = eattoword;

    /*printf("### line has words %d..%d; breakable = %d\n", firstwordat, eattoword-1, breakable);*/

    /* adjust right-tabs and center-tabs. */
    {
	long thistword, nexttword; /* the word which is a tab, or eattoword */
	long thistab, nexttab; /* a tab number, or -1 */
	long nextpos, thispos, thisend, thiscenter, moveforward;
	enum style_TabAlignment kind; /* style_{{Left,Right}Aligned,CenteredOnTab} */
	thistword=firstwordat;
	for (; thistword<eattoword && slurp->tmpwords[thistword].start != wordflag_Tab; thistword++);
	for (; thistword<eattoword; thistword = nexttword) {
	    for (nexttword=thistword+1; nexttword<eattoword && slurp->tmpwords[nexttword].start != wordflag_Tab; nexttword++);
	    /* now thistword is on a tab word, and nexttword is on the next (or eattoword). */
	    if (thistword+1==nexttword || thistword+1>=eattoword)
		continue; /* no adjustment for two adjacent tabs, or a tab at the end of a line */
	    thistab = slurp->tmpwords[thistword].tmptabnum;
	    if (thistab >= 0) {
		kind = linestyle->tabs->list[thistab].kind;
		thispos = linestyle->tabs->list[thistab].pos;
		if (kind==style_RightAligned) {
		    thisend = slurp->tmpwords[nexttword-1].tmppos + slurp->tmpwords[nexttword-1].width;
		    /* thisend is the end of this tab-delimited field */
		    /* now, we shove all words between thistword and nexttword (exclusive) forward. */
		    moveforward = (thispos) - (thisend);
		    /*printf("  ## we move wds %d..%d forward %d-%d=%d\n", thistword+1, nexttword-1, thispos, thisend, moveforward);*/
		    if (moveforward > 0) {
			for (ix=thistword+1; ix<nexttword; ix++)
			    slurp->tmpwords[ix].tmppos += moveforward;
		    }
		}
		else if (kind==style_CenteredOnTab) {
		    thisend = slurp->tmpwords[nexttword-1].tmppos + slurp->tmpwords[nexttword-1].width;
		    thiscenter = (slurp->tmpwords[thistword+1].tmppos + thisend) / 2;
		    /* thisend is the end of this tab-delimited field; thiscenter is the center */
		    /* now, we shove all words between thistword and nexttword (exclusive) forward. */
		    moveforward = thispos - thiscenter;
		    /*printf("  ## we move wds %d..%d forward (%d-%d)=%d\n", thistword+1, nexttword-1, thispos, thiscenter, moveforward);*/
		    if (moveforward > 0) {
			for (ix=thistword+1; ix<nexttword; ix++)
			    slurp->tmpwords[ix].tmppos += moveforward;
		    }
		}
	    }
	}
    }

    /* adjust justification */
    just = linestyle->justification;
    if (just==style_LeftAndRightJustified && lastlineinpara)
	just = style_LeftJustified;
    if (just==style_LeftThenRightJustified)
	just = firstlineinpara ? style_LeftJustified : style_RightJustified;

    leftmargin = linestyle->leftmargin;
    rightmargin = linestyle->rightmargin;
    if (firstlineinpara) {
	switch (just) {
	    case style_RightJustified:
		rightmargin -= linestyle->paraindent;
		break;
	    case style_Centered:
		rightmargin -= linestyle->paraindent/2;
		leftmargin += linestyle->paraindent/2;
		break;
	    default:
		leftmargin += linestyle->paraindent;
		break;
	}
    }

    /* compute line height stuff */
    if (eattoword > firstwordat) {
	long distabove=0, distbelow=0, maxspacing=(-999999);
	long tmpabove, tmpbelow;
	struct textps_style *wsty;
	for (ix=firstwordat; ix<eattoword; ix++) {
	    wsty = slurp->tmpwords[ix].style;
	    if (slurp->tmpwords[ix].start < 0) {
		switch (slurp->tmpwords[ix].start) {
		    case wordflag_View:
			slurp->tmpwords[ix].vheight = slurp->tmpwords[ix].desheight;
			{
			long xoff=0, yoff= (wsty->fontsize * RESOLUTION);
			if(slurp->tmpwords[ix].v) slurp->tmpwords[ix].v->GetPrintOrigin(slurp->tmpwords[ix].width, slurp->tmpwords[ix].vheight, &xoff, &yoff);
			slurp->tmpwords[ix].voff=yoff*RESOLUTION;
			tmpbelow = slurp->tmpwords[ix].vheight - (yoff * RESOLUTION) + wsty->baselineoffset;
			tmpabove = slurp->tmpwords[ix].vheight - tmpbelow;

			}
			break;
		    default:
			tmpabove = 0;
			tmpbelow = 0;
			break;
		}
	    }
	    else {
		tmpabove = wsty->fontsize * RESOLUTION - wsty->baselineoffset;
		tmpbelow = (int)((double)(wsty->fontsize) * 0.1667 * dRESOLUTION) + wsty->baselineoffset;
	    }

	    if (wsty->linespacing > maxspacing) 
		maxspacing = wsty->linespacing;
	    if (lastlineinpara && wsty->paraspacing > maxspacing)
		maxspacing = wsty->paraspacing;
	    if (tmpabove > distabove)
		distabove = tmpabove;
	    if (tmpbelow > distbelow)
		distbelow = tmpbelow;
	} /* end for loop */
	lineheight = distabove + distbelow + maxspacing;
	baseline = distabove;
    }
    else {
	lineheight = (int)((double)(linestyle->fontsize) * 1.1667 * dRESOLUTION) + linestyle->linespacing;
	baseline = linestyle->fontsize * RESOLUTION;
    };
    /*printf("###lineheight, baseline = %d, %d\n", lineheight, baseline);*/

    /* stick the line in */
    ln = (struct textps_line *)malloc(sizeof(struct textps_line));
    ln->words = (struct textps_word *)malloc((eattoword-firstwordat)*sizeof(struct textps_word));
    /*widthsofar = 0;*/ /* already computed, above */
    totalspace = 0;
    lasttabpos = firstwordat;
    for (ix=firstwordat; ix<eattoword; ix++) {
	if (slurp->tmpwords[ix].start >= 0) {
	    /*widthsofar += slurp->tmpwords[ix].width;*/
	    totalspace += slurp->tmpwords[ix].spaces;
	}
	else {
	    if (slurp->tmpwords[ix].start == wordflag_Tab) {
		lasttabpos = ix;
		totalspace = 0;
	    }
	}
    }
    if (eattoword > firstwordat) {
	if (slurp->tmpwords[eattoword-1].start >= 0) {
	    /*widthsofar -= slurp->tmpwords[eattoword-1].spaces;*/
	    totalspace -= slurp->tmpwords[eattoword-1].spaces;
	}
    }

    extraspace = linewidth - widthsofar;
    if (just==style_RightJustified)
	wix = leftmargin + extraspace;
    else if (just==style_Centered)
	wix = leftmargin + extraspace/2;
    else /* left or full-justified */
	wix = leftmargin;

    /* copylet* refer to tmpletters string */
    copyletstart = 0;
    copyletend = 0;
    if (eattoword > firstwordat) {
	for (ix=firstwordat; ix < eattoword && slurp->tmpwords[ix].start < 0; ix++);
	for (jx=eattoword-1; jx >= firstwordat && slurp->tmpwords[jx].start < 0; jx--);
	if (ix <= jx) {
	    copyletstart = slurp->tmpwords[ix].start;
	    copyletend = slurp->tmpwords[jx].start + slurp->tmpwords[jx].len;
	}
    }

    /* copy words over, being careful about special words */
    for (ix=firstwordat, lix=0; ix<eattoword; ix++) {
	struct textps_tmp_word *wd = &(slurp->tmpwords[ix]);
	wd->style->usage &= (~textps_style_inline);

	if (wd->start >= 0) {
	    ln->words[lix].start = wd->start - copyletstart;
	    ln->words[lix].len = wd->len;
	    ln->words[lix].xpos = wix + wd->tmppos;
	    ln->words[lix].run = wd->run;
	    ln->words[lix].width = wd->width - wd->spaces;
	    if (just==style_LeftAndRightJustified && totalspace && ix >= lasttabpos) {
		wix += (extraspace * wd->spaces) / (totalspace);
	    }
	    lix++;
	}
	else {
	    switch (wd->start) {
		case wordflag_Tab:
		    ln->words[lix].start = wd->start;
		    ln->words[lix].len = 0;
		    ln->words[lix].xpos = wix + wd->tmppos;
		    ln->words[lix].run = wd->run;
		    ln->words[lix].v = wd->v;
		    ln->words[lix].width = wd->width; /* no spaces on a tab word */
		    lix++;
		    break;
		case wordflag_View:
		    ln->words[lix].start = wd->start;
		    ln->words[lix].len = 0;
		    ln->words[lix].xpos = wix + wd->tmppos;
		    ln->words[lix].run = wd->run;
		    ln->words[lix].v = wd->v;
		    ln->words[lix].vheight = wd->vheight;
		    ln->words[lix].voff = wd->voff;
		    ln->words[lix].width = wd->width; /* no spaces on a view word */
		    lix++;
		    break;
		case wordflag_Spec:
		    ln->words[lix].start = wd->start;
		    ln->words[lix].len = 0;
		    ln->words[lix].xpos = wix + wd->tmppos;
		    ln->words[lix].run = wd->run;
		    ln->words[lix].v = NULL;
		    ln->words[lix].width = wd->width; /* no spaces on a view word */
		    ln->words[lix].inset = wd->inset;
		    if (wd->inset.type==textview_Footnote && stat) {
			/* note that we ignore footnotes if we're not given a footnotes thing to put them in. */
			/* ### do a rectangle layout, store the result somewhere! bleah. */
			class fnotev *v = wd->inset.u.Footnote;
			class fnote *fn;
			class textview *tv;
			/*struct textps_layout_plan *layoutplan;*/
			char *numbuf;
			static class style *fstyle = NULL;

			fn = (class fnote *)(v->GetDataObject());
			numbuf = (char *)malloc(20 * sizeof(char));
			sprintf(numbuf, "%d", wd->footnotenumber);
			if (!fstyle) {
			    fstyle = new style;
			    fstyle->SetName("footnotetag");
			    fstyle->SetFontSize(style_PreviousFontSize, -4);
			    fstyle->SetFontScript( style_PreviousScriptMovement, -6, style_Points);
			}
			tv = new textview;
			tv->SetDataObject(fn);

			/*layoutplan = CreateRectLayoutPlan(columnwidth, 999999);*/
			fnote::CloseAll(fn);
			SimpleLayoutText(stat, tv, columnwidth, numbuf, fstyle);

			/*CopyLinesToLayout(footnotes, NULL, stat->footnotes, NULL, 0, tv->cachedlayout->numlines, 0, -1);*/

			print::PSRegisterCleanup(DeleteTextviewProc, (void *)tv);
			/* the footnotes layout now contains pointers to subviews of tv, if there are any. Therefore, we cannot destroy tv until the printing is done. */
		    }
		    lix++;
		    break;
		default:
		    break;
	    }
	}
    }
    ln->firstword = slurp->wordseaten + firstwordat;
    ln->numwords = lix;
    ln->lineheight = lineheight;
    ln->baseline = baseline;

    if (copyletstart < copyletend) {
	ln->letters = (char *)malloc(copyletend-copyletstart);
	for (lix=0, ix=copyletstart; ix<copyletend; ix++, lix++)
	    ln->letters[lix] = slurp->tmpletters[ix];
    }
    else {
	ln->letters = (char *)malloc(1); /* dummy */
    }

    /* delocalize vars */
    *breakableptr = breakable;
    *eattowordptr = eattoword;
    *wordatptr = wordat;
    *lastlineinparaptr = lastlineinpara;
    return ln;
}

static void DestroyLine(struct textps_line  *ln)
{
    if (ln->words) {
	free(ln->words);
	ln->words = NULL;
    }
    if (ln->letters) {
	free(ln->letters);
	ln->letters = NULL;
    }
    ln->colnum = (-1);
    free(ln);
}

static void copy_textps_srunlist(struct textps_srunlist *dest, struct textps_srunlist *src)
{
    size_t len = sizeof(struct textps_stylerun) * src->num;

    dest->num = src->num;
    dest->size = src->num; /* let's be efficient */
    if (len) {
	dest->r = (struct textps_stylerun *)malloc(len);
	memcpy(dest->r, src->r, len);
    }
    else {
	/* what? src->num == 0. */
	dest->size = 1;
	dest->r = (struct textps_stylerun *)malloc(sizeof(struct textps_stylerun));
    }
}

static struct textps_simple_layout_state *SimpleLayoutTextInit()
{
    struct textps_simple_layout_state *stat;

    stat = (struct textps_simple_layout_state *)malloc(sizeof(struct textps_simple_layout_state));

    stat->slurp = InitWordSlurper(NULL);
    stat->footnotes = NULL;

    return stat;
}

/* If txtv != NULL, this is a new footnote (or whatever) -- stuff it onto the layout.
 If txtv == NULL, there's no new text; instead, we must re-lay the old text out in a new width. Trash the old layout and start over.
 prependstring and prependstyle are hacks to get footnotes to work. Note that prependstring will be free()d in the course of LayoutText, but prependstyle will not. */
static void SimpleLayoutText(struct textps_simple_layout_state *stat, class textview  *txtv, long layoutwidth, char *prependstring, class style *prependstyle)
{
    int ix;
    struct textps_slurp *slurp;
    struct textps_layout *ly;
    struct textps_line *ln;
    boolean lastlineinpara, firstlineinpara;
    int wordat, breakable, eattoword;
    int newwordat;
    boolean newlastline;
    long colypos;

    /*printf("\n### [simple] recomputing layout\n");*/

    if (!txtv && stat->slurp->numwords==0) {
	/*printf("### re-laying zero words. Punt.\n");*/
	delete stat->footnotes;
	stat->footnotes = NULL;
	return;
    }

    if (!txtv || !stat->footnotes) {
	/* if this is a relayout, or the first layout, create a layout. */
	delete stat->footnotes;

	ly = new textps_layout;
	ly->planid = 0;
	ly->runs.r = NULL;
	ly->runs.size = 0;
	ly->runs.num = 0;
	ly->numlines = 0;
	ly->lines_size = 32;
	ly->lines = (struct textps_line **)malloc(sizeof(struct textps_line *) * ly->lines_size);
	stat->footnotes = ly;
    }
    else {
	/* continue old layout */
	ly = stat->footnotes; 
    }

    colypos = 0;

    slurp = stat->slurp;

    if (txtv) {
	/*printf("### Adding some text.\n");*/

	firstlineinpara = TRUE;
	wordat = slurp->numwords;

	if (wordat > 0) {
	    /* Sneakily declare that the previous last word just ended a line. */
	    slurp->tmpwords[wordat-1].breakable = 2;
	}

	/* always throw the textview lexer in */
	{
	    struct textps_lexstate_text  *clex;
	    clex = AddNewLexer(slurp);
	    clex->type = textps_lexer_Text;
	    clex->txt = (class text *)(txtv)->GetDataObject();
	    clex->txtv = txtv;
	    clex->len = (clex->txt)->GetLength();
	    InitLexer(slurp, clex);
	}

	if (prependstring) {
	    /* throw on an initial lexer, to make life fun */
	    struct textps_lexstate_text  *clex;
	    clex = AddNewLexer(slurp);
	    clex->type = textps_lexer_CharArray;
	    clex->str = prependstring;
	    clex->sty = prependstyle;
	    clex->len = strlen(clex->str);
	    InitLexer(slurp, clex);
	}
    }
    else {
	/*printf("### re-laying %d words.\n", slurp->numwords);*/

	firstlineinpara = TRUE;
	wordat = 0;
    }

    while (TRUE) {
	/* do a line */

	/*PurgeStyleStack(slurp, FALSE); ### not really worth the effort */

	newwordat = wordat;
	newlastline = lastlineinpara; /* what? apparently undefined value the first time through ### */
	ln = LayoutOneLine(slurp, layoutwidth, &newwordat, firstlineinpara, &newlastline, &breakable, &eattoword, NULL);
	/* in the next section, we should make sure usethisline and breakable are set nicely. breakable values of 3, 4, or 5 determine whether we jump to the next column, page, or exit the layout loop. */

	if (TRUE) {
	    /* the line will be used. */
	    ln->clipline = (-1);
	    ln->colnum = 0;
	    /*for (ix=0; ix<ln->numwords; ix++) {
		ln->words[ix].xpos += partstart;
	    }*/
	    colypos += ln->lineheight;
	    wordat = newwordat;
	    lastlineinpara = newlastline;
	    /*printf("### line %d result: wordat=%d, lastline?=%d, breakable=%d, eattoword=%d\n", ly->numlines, wordat, lastlineinpara, breakable, eattoword);*/

	    if (ly->numlines >= ly->lines_size) {
		ly->lines_size *= 2;
		ly->lines = (struct textps_line **)realloc(ly->lines, sizeof(struct textps_line *) * ly->lines_size);
	    }

	    ResolveLocatags(slurp, ln, ly->numlines);
	    ly->lines[ly->numlines] = ln;
	    ly->numlines++;

	    /*wordat -= PurgeSlurpedWords(slurp, eattoword);*/

	    firstlineinpara = lastlineinpara; /* if was last line, we're now in a new one. If not, not. */
	}
	else {
	    /* we will not use this line. */
	    DestroyLine(ln);
	}

	if (breakable>=3) {
	    /* end of document, or page break, or column break -- doesn't matter, they all end a footnote. */
	    break; /* exit loop */
	} 
    }; /* end while (TRUE) */

    /*copy_textps_srunlist(&ly->runs, &slurp->runs);*/ /* don't damage slurper */

    /* stat->slurp is ready to go again */
}

static void SimpleLayoutTextFinal(struct textps_simple_layout_state *stat)
{
    FinalizeWordSlurper(stat->slurp);
    stat->slurp = NULL;
    if (stat->footnotes) {
	delete stat->footnotes;
	stat->footnotes = NULL;
    }

    free(stat);
}

static void InitLayoutState(struct textps_layout_state *lstat)
{
    lstat->prependstring = NULL;
    lstat->prependstyle = NULL;
    lstat->locatags = NULL;
}

static void FinalizeLayoutState(struct textps_layout_state *lstat)
{
    if (lstat->locatags) {
	DestroyLocatags(lstat->locatags);
	lstat->locatags = NULL;
    }
}

/* prependstring and prependstyle are hacks to get footnotes to work. Note that prependstring will be free()d in the course of LayoutText, but prependstyle will not. */
static void LayoutText(class textview  *txtv, struct textps_layout_history  *history, struct textps_layout_state *lstat)
{
    class text *txt = (class text *)(txtv)->GetDataObject();
    int ix;
    struct textps_slurp *slurp;
    struct textps_layout *ly;
    struct textps_line *ln;
    boolean lastlineinpara, firstlineinpara;
    int wordat, breakable, eattoword;
    int newwordat;
    boolean newlastline;
    long partnum, partstart, extraparts;
    struct textps_layout_column *thiscol;
    long colwidth, colheight, colypos, partheightsofar, partheightabove;
    long thisx1, thisx2;
    struct textps_layout_block *tmppart;
    long clipline;
    boolean usethisline;
    long getnextpart;
    struct textps_simple_layout_state *stat;
    boolean useendnotes, footsthispage;
    long footlines, footheight, oldfootlines, oldfootheight;
    long footlimnum;

#if 0
    if (txtv->cachedlayout
	&& txtv->cachedlayout->planid == layoutplan->planid
	&& !lstat->prependstring) {
	/* the one hanging out is still good */
	return;
    }
#endif

    /*printf("\n### recomputing layout\n");*/

    textview_DestroyPrintingLayout(txtv);
    useendnotes = txtv->GetPrintOption(A_endnotes);

    ly = new textps_layout;
    /*ly->planid = layoutplan->planid;*/
    ly->runs.r = NULL;
    ly->runs.size = 0;
    ly->runs.num = 0;
    ly->numlines = 0;
    ly->lines_size = 32;
    ly->lines = (struct textps_line **)malloc(sizeof(struct textps_line *) * ly->lines_size);
    txtv->cachedlayout = ly;
 
    history->NextColumn(); /* set up the first column */
    partnum = 0;
    partheightabove = 0; /* maintained in synch with partnum */
    colypos = 0;
    extraparts = 0;

    firstlineinpara = TRUE;
    wordat = 0;

    slurp = InitWordSlurper(txtv);
    /* footnotes and footlines grow forever. footheight, and footlimnum are reset every time a bunch of footnotes is dumped. */
    stat = SimpleLayoutTextInit();
    footlines = 0;
    footheight = 0;
    footlimnum = (-1);
    slurp->history = history;

    if (lstat->prependstring) {
	/* throw on an initial lexer, to make life fun */
	struct textps_lexstate_text  *clex;
	clex = AddNewLexer(slurp);
	clex->type = textps_lexer_CharArray;
	clex->str = lstat->prependstring;
	clex->sty = lstat->prependstyle;
	clex->len = strlen(clex->str);
	InitLexer(slurp, clex);
    }

    while (TRUE) {
	/* do a line */

	/*PurgeStyleStack(slurp, FALSE); ### not really worth the effort */

	/*thiscol = (&layoutplan->cols[coltyp]);*/
	thiscol = history->cols[history->numcols-1];

	if (useendnotes)
	    footsthispage = FALSE;
	else
	    footsthispage = (thiscol->coltags & textview_coltag_FootNotes);
	colheight = RESOLUTION * thiscol->h;
	tmppart = (&(thiscol->parts[partnum]));
	partheightsofar = partheightabove + RESOLUTION*tmppart->height;
	thisx1 = tmppart->x1;
	thisx2 = tmppart->x2;
	if (extraparts) {
	    /* special case; we're allowing this line to zog through multiple columns. So we slim down (thisx1,thisx2) to fit the intersection. Note that partheightsofar is still the height of the first part (partnum), since we want to fall into the "overflow" section. */
	    for (ix=1; ix<=extraparts; ix++) {
		tmppart = (&thiscol->parts[partnum+ix]);
		if (tmppart->x1 > thisx1)
		    thisx1 = tmppart->x1;
		if (tmppart->x2 < thisx2)
		    thisx2 = tmppart->x2;
	    }
	}
	colwidth = thisx2 - thisx1;
	partstart = thisx1 * RESOLUTION;

	newwordat = wordat;
	newlastline = lastlineinpara; /* what? apparently undefined value the first time through ### */
	clipline = (-1);
	ln = LayoutOneLine(slurp, colwidth, &newwordat, firstlineinpara, &newlastline, &breakable, &eattoword, stat);
	oldfootlines = footlines;
	oldfootheight = footheight;
	if (!stat->footnotes) {
	    footlines = 0;
	}
	else {
	    footlines = stat->footnotes->numlines;
	}
	if (footlines > oldfootlines) {
	    for (ix=oldfootlines; ix<footlines; ix++) {
		if (ix==0)
		    footheight += textps_FootnoteGap * RESOLUTION; /* the space for the division line */
		footheight += stat->footnotes->lines[ix]->lineheight;
	    }
	}

	usethisline = TRUE;
	getnextpart = 0;
	extraparts = 0;
	/* in the next section, we should make sure usethisline and breakable are set nicely. breakable values of 3, 4, or 5 determine whether we jump to the next column, page, or exit the layout loop. */

	if (colypos+ln->lineheight >= partheightsofar) {
	    /* the line don't fit vertically in this part. Or maybe we just want to end the page. */
	    long partheightplus;
	    struct textps_layout_block *tmppart;
	    long sectx1, sectx2;
	    long numnewpart;
	    boolean overend;

	    /* partheightplus = partheightsofar + all the parts the line spills into */
	    partheightplus = partheightsofar;
	    sectx1 = thisx1;
	    sectx2 = thisx2;
	    for (numnewpart = partnum+1; numnewpart < thiscol->numparts; numnewpart++) {
		tmppart = (&thiscol->parts[numnewpart]);
		partheightplus += RESOLUTION*tmppart->height;
		if (tmppart->x1 > sectx1)
		    sectx1 = tmppart->x1;
		if (tmppart->x2 < sectx2)
		    sectx2 = tmppart->x2;
		if (colypos+ln->lineheight < partheightplus)
		    break;
	    }
	    /* overend is TRUE if the line goes off the end of the column (ie, every part in the column */
	    overend = (numnewpart >= thiscol->numparts);

	    /* things that can happen now:
	     line doesn't fit horizontally into all the parts it crosses. Pitch it, try again with more parts. (This cannot go on forever; eventually every part in the column will be involved, and we'll fall into the next cases.)
	     line is overend, but it's the first line in this column (not just this part). Leave it in (and go on to next part). It will leak off the bottom edge; too bad.
	     line overends the column. Throw out line, go on to next column. (This can't go on forever either, since the next try will be the first line in the column)
	     line does fit. keep it, but go on to next part.
	     */

	    if ((sectx1 > thisx1) || (sectx2 < thisx2)) {
		/* it doesn't fit horizontally -- intersect. Overend may be TRUE, in which case we should only go as many extra parts as there are. */
		usethisline = FALSE;
		if (overend)
		    extraparts = (thiscol->numparts-1)-partnum;
		else
		    extraparts = numnewpart-partnum;
		getnextpart = 0;
		breakable = 0;
	    }
	    else if (overend && partnum==0 && colypos==0) { 
		/* overend is TRUE, but this is first line in column */
		/* we do nothing. breakable will be set to page-break, so we'll jump to the next column after this line. */
		if (breakable < 3)
		    breakable = 3;
		/* set clipline so that we don't draw outside the column limits */
		clipline = colheight;
		usethisline = TRUE;
	    }
	    else if (overend) {
		/* overend is TRUE; head for the next column */
		breakable = 3;
		usethisline = FALSE;
	    }
	    else if (TRUE) {
		/* it fits in new part(s), horizontally and vertically. */
		getnextpart = numnewpart-partnum;
		usethisline = TRUE;
	    }
	} /* end of "overflowed the part" case */

	/* footnotes overflow check */
	if (footsthispage
	    && usethisline
	    && footlines
	    && colypos+ln->lineheight >= colheight-footheight) {
	    /* remember that this might still be an overended line (if it's the first line in column) or some other weird case. */
	    if ((partnum==0 && colypos==0) /* first-line case */
		|| (footlimnum < 0 /* skanky case */ 
		    && (colheight-(colypos+oldfootheight) > 2*72*RESOLUTION)
		    && (colypos + ln->lineheight + textps_FootnoteGap*RESOLUTION + stat->footnotes->lines[0]->lineheight) < colheight)) {
		/* first line in column, again. Let grace = max {3 inches, line height}; then hack off enough footnote to allow grace between the footnote and the top of the line. Except that we leave one line (minimum) of footnote. */
		long grace, footsofar;
		int flx;
		if (partnum==0 && colypos==0) {
		    grace = 3*72*RESOLUTION; /* 3 inches */
		}
		else {
		    /* gonna skankify */
		    grace = 1*72*RESOLUTION; /* 1 inches */
		}
		if (grace < ln->lineheight)
		    grace = ln->lineheight;
		grace = colypos+grace; /* now at boundary */
		footsofar = textps_FootnoteGap * RESOLUTION; /* the space for the division line */
		for (flx=0;
		     flx<footlines && grace+footsofar+stat->footnotes->lines[flx]->lineheight < colheight;
		     flx++) {
		    footsofar += stat->footnotes->lines[flx]->lineheight;
		}
		if (flx==0) {
		    /* ickiness. We want to force in the first footnote line, but there's a possibility that'll overflow the (one?) line on the page. Which is a certainty if the line overended. In which case we'll have to clip the line (further?) */
		    footsofar += stat->footnotes->lines[flx]->lineheight;
		    flx++;
		    if (colypos+ln->lineheight >= colheight-footsofar) {
			/* Oh rapture. */
			/*printf("### The immovable text has been hit with the unstoppable footnote. Now what?\n");*/
			/* answer: screw over the footnotes. This will be done later, when the end-of-column stuff sees the lines overending and clips them. */
		    }
		}
		/* hack at flx. If the footnotes are already hacked, do not advance the hack-point. (Not a problem in the first-line case, but possibly in the skanky case.) */
		if (footlimnum < 0) {
		    footheight = footsofar;
		    footlimnum = flx;
		}
	    }
	    else {
		/* pitch line */
		breakable = 3;
		usethisline = FALSE;
	    }

	} /* end of "overkicked footnotes" case */ 

	if (usethisline) {
	    /* the line will be used. */
	    ln->clipline = clipline;
	    ln->colnum = history->numcols-1;
	    for (ix=0; ix<ln->numwords; ix++) {
		ln->words[ix].xpos += partstart;
	    }
	    colypos += ln->lineheight;
	    wordat = newwordat;
	    lastlineinpara = newlastline;
	    /*printf("### line %d result: wordat=%d, lastline?=%d, breakable=%d, eattoword=%d\n", ly->numlines, wordat, lastlineinpara, breakable, eattoword);*/

	    if (ly->numlines >= ly->lines_size) {
		ly->lines_size *= 2;
		ly->lines = (struct textps_line **)realloc(ly->lines, sizeof(struct textps_line *) * ly->lines_size);
	    }

	    ResolveLocatags(slurp, ln, ly->numlines);
	    ly->lines[ly->numlines] = ln;
	    ly->numlines++;

	    wordat -= PurgeSlurpedWords(slurp, eattoword);

	    firstlineinpara = lastlineinpara; /* if was last line, we're now in a new one. If not, not. */
	}
	else {
	    /* we will not use this line. */
	    DestroyLine(ln);
	    footlines = oldfootlines;
	    footheight = oldfootheight;
	    DeleteLinesFromFootThing(stat, footlines);
	}

	if (breakable>=3 || getnextpart>0) {
	    if (breakable < 3
		&& partnum+getnextpart < thiscol->numparts) {
		/* if we're only supposed to go on to the next part, *and* there is a next part, just do that. */
		for (ix=0; ix<getnextpart; ix++) {
		    partheightabove += RESOLUTION * thiscol->parts[partnum].height;
		    partnum++;
		}
		/* partnum has now been increased by getnextpart */
	    }
	    else {
		/* end-of-column processing; do footnotes if appropriate, or pass them on to the next column. */
		if (footsthispage && footheight > 0) {
		    long gapwid, trimpt;

		    if (footlimnum < 0)
			footlimnum = stat->footnotes->numlines;

		    /*printf("### end of col: shoving out %d..%d of footnotes\n", 0, footlimnum-1);*/
		    /* compute height of footnotes *without* gap line */
		    footheight = 0;
		    for (ix=0; ix<footlimnum; ix++) {
			footheight += stat->footnotes->lines[ix]->lineheight;
		    }
		    /*printf("### used-now footheight = %d\n", footheight);*/
		    gapwid = (colheight - colypos) - footheight;
		    if (gapwid < textps_FootnoteGap*RESOLUTION) {
			gapwid = textps_FootnoteGap*RESOLUTION;
		    }
		    AddSpecialLineToLayout(ly, lineflag_Footnote, gapwid, history->numcols-1);
		    trimpt = colheight-(gapwid+colypos);
		    if (trimpt < 0)
			trimpt = 0;
		    CopyLinesToLayout(&ly, &(slurp->runs), stat->footnotes, &(stat->slurp->runs), 0, footlimnum, history->numcols-1, trimpt);
		    DeleteWordsFromFootThing(stat, footlimnum);
		}

		if (breakable==5) {
		    /* end of document */
		    break; /* exit loop */
		}

		/* go on to next column */
		do {
		    /* advance one. If breakable==4, keep advancing until you hit a coltag_PageBreak column. If you run out of layout (and there's no repeat), drop out here. */
		    thiscol = history->NextColumn();
		    if (!thiscol) {
			/* end of layout */
			breakable = 5;
		    }
		    /*
		    colnum++;
		    coltyp++;
		    if (coltyp >= layoutplan->numcols) {
			if (!layoutplan->repeat) {
			    breakable = 5;
			}
			else {
			    coltyp -= layoutplan->repeat;
			}
		    }
		    */
		} while (breakable == 4 && !(thiscol->coltags & textview_coltag_PageBreak));
		if (breakable == 5)
		    break; /* exit loop -- out of layout */

		/* there's gonna be a new page */
		partnum = 0;
		partheightabove = 0;
		colypos = 0;

		SimpleLayoutText(stat, NULL, thiscol->w, NULL, NULL);
		footlimnum = (-1);
		if (stat->footnotes) {
		    footlines = stat->footnotes->numlines;
		    /* reset footheight, but add on hacked lines */
		    footheight = 0;
		    for (ix=0; ix<stat->footnotes->numlines; ix++) {
			if (ix==0)
			    footheight += RESOLUTION*textps_FootnoteGap;
			footheight += stat->footnotes->lines[ix]->lineheight;
		    }
		}
		else {
		    footlines = 0;
		    footheight = 0;
		}

		/*printf("### hacked-over footheight = %d, lines = %d\n", footheight, footlines);*/

	    }
	    extraparts = 0;
	} /* if breakable >= 3 || getnextpart */
    }; /* end while (TRUE) */

    /* and now we stick in all the footnotes that were hacked over from the last page. ### and endnotes, too */
    /*printf("### there are %d words still in the footnotes\n", stat->slurp->numwords);*/
    while (stat->slurp->numwords > 0) {
	thiscol = history->NextColumn();
	if (!thiscol) {
	    /* end of layout. Note that we will immediately fall into this case if we did earlier */
	    /*printf("### but we're out of layout, so forget it.\n");*/
	    break;
	}
	/*colnum++;
	coltyp++;
	if (coltyp >= layoutplan->numcols) {
	    if (!layoutplan->repeat) {
		break;
	    }
	    else {
		coltyp -= layoutplan->repeat;
	    }
	}
	thiscol = history->cols[history->numcols-1];*/
	SimpleLayoutText(stat, NULL, thiscol->w, NULL, NULL);

	if (stat->footnotes && stat->footnotes->numlines) {
	    long footypos;
	    colheight = RESOLUTION * thiscol->h;
	    footypos = 0;
	    for (footlimnum = 0; footlimnum < stat->footnotes->numlines; footlimnum++) {
		if (footypos+stat->footnotes->lines[footlimnum]->lineheight >= colheight)
		    break;
		footypos += stat->footnotes->lines[footlimnum]->lineheight;
	    }
	    if (footlimnum == 0) {
		footlimnum++;
	    }
	    CopyLinesToLayout(&ly, &(slurp->runs), stat->footnotes, &(stat->slurp->runs), 0, footlimnum, history->numcols-1, colheight);
	    DeleteWordsFromFootThing(stat, footlimnum);
	}
	/*printf("### there are %d words still in the footnotes\n", stat->slurp->numwords);*/
    }

    ly->runs = slurp->runs; /* copy structure, including pointer to actual list */
    slurp->runs.r = NULL;
    slurp->runs.num = 0;
    slurp->runs.size = 0;
    lstat->locatags = slurp->locatags_root;
    slurp->locatags_root = NULL;
    slurp->locatags = NULL;
    FinalizeWordSlurper(slurp);
    SimpleLayoutTextFinal(stat);
}

textps_layout::textps_layout()
{
    this->lines = NULL;
    this->numlines = 0;
    this->runs.r = NULL;
}

textps_layout::~textps_layout()
{
    if (this->lines) {
	DeleteLinesFromLayout(this, 0);
	free(this->lines);
	this->lines = NULL;
    }
    if (this->runs.r) {
	free(this->runs.r);
	this->runs.r = NULL;
    }
}

static void DumpLayout(struct textps_layout  *ly)
{
    int lx, wx;
    struct textps_line *tlin;
    int buflen;
    char buf[1000];

    for (lx=0; lx<ly->numlines; lx++) {
	printf("line %d: ", lx);
	tlin = ly->lines[lx];
	if (tlin->numwords < 0) {
	    printf("<special line>");
	}
	else for (wx=0; wx<tlin->numwords; wx++) {
	    buflen = tlin->words[wx].len;
	    if (buflen>998) buflen=998;
	    strncpy(buf, tlin->letters+tlin->words[wx].start, buflen);
	    buf[buflen] = '\0';
	    printf("|%s| [%d], ", buf, tlin->words[wx].xpos);
	}
	printf("\n");
    }
}

textps_layout_history::textps_layout_history()
{
    this->cols_size = 8;
    this->numcols = 0;
    this->cols = (struct textps_layout_column **)malloc(this->cols_size * sizeof(struct textps_layout_column *));

    this->numplans = 0;
    this->plans_size = 2;
    this->plans = (struct textps_layout_history_unit *)malloc(this->plans_size * sizeof(struct textps_layout_history_unit));

    this->pagew = 0;
    this->pageh = 0;
}

textps_layout_history::~textps_layout_history()
{
    if (this->cols) {
	free(this->cols);
	this->cols = NULL;
    }
    if (this->plans) {
	free(this->plans);
	this->plans = NULL;
    }
}

void textps_layout_history::PushNewPlan(struct textps_layout_plan *newplan)
{
    struct textps_layout_history_unit *curplan;

    if (this->numplans == this->plans_size) {
	this->plans_size *= 2;
	this->plans = (struct textps_layout_history_unit *)realloc(this->plans, this->plans_size * sizeof(struct textps_layout_history_unit));
    }
    this->numplans++;

    curplan = this->getcurplan();
    curplan->plan = newplan;
    curplan->coltyp = -1;
    curplan->colnum = -1;
    curplan->pagebreaks = 0;
}

struct textps_layout_column *textps_layout_history::NextColumn(void)
{
    struct textps_layout_history_unit *curplan;
    struct textps_layout_column *newcol;

    curplan = this->getcurplan();
    if (!curplan)
	return NULL;

    curplan->colnum++;
    curplan->coltyp++;
    if (curplan->coltyp >= curplan->plan->numcols) {
	if (!curplan->plan->repeat) {
	    /* end of layout; try to pop */
	    this->numplans--;
	    curplan = this->getcurplan();
	    if (!curplan)
		return NULL;
	    curplan->coltyp = 0;
	    curplan->colnum = 0;
	}
	else {
	    curplan->coltyp -= curplan->plan->repeat;
	}
    }

    newcol = (&curplan->plan->cols[curplan->coltyp]);
    if (this->numcols >= this->cols_size) {
	this->cols_size *= 2;
	this->cols = (struct textps_layout_column **)realloc(this->cols, this->cols_size * sizeof(struct textps_layout_column *));
    }
    this->cols[this->numcols] = newcol;
    this->numcols++;

    if (newcol->numparts == 0) {
	return this->NextColumn();
    }
    else
	return newcol;
}

/* clever utility to set up the height field of the columns */
static void SumColumnSizes(struct textps_layout_plan *lp)
{
    int ix, jx;
    long hgt;
    struct textps_layout_column *col;
    for (ix=0; ix<lp->numcols; ix++) {
	col = &(lp->cols[ix]);
	hgt = 0;
	for (jx=0; jx<col->numparts; jx++) {
	    hgt += col->parts[jx].height;
	}
	col->h = hgt;
    }
}

textps_layout_plan::textps_layout_plan()
{
    this->cols = NULL;
    this->numcols = 0;
    this->planid = 0;
}

textps_layout_plan::~textps_layout_plan()
{
    int ix;
    if (this->cols) {
	for (ix=0; ix<this->numcols; ix++) {
	    if (this->cols[ix].parts)
		free(this->cols[ix].parts);
	    this->cols[ix].parts = NULL;
	}
	free(this->cols);
	this->cols = NULL;
    }
    this->numcols = 0;
}

void textview_DestroyPrintingLayout(class textview  *txtv)
{
    if (txtv->cachedlayout) {
	delete txtv->cachedlayout;
	txtv->cachedlayout = NULL;
    }
}

void textview_DestroyPrintingLayoutPlan(class textview  *txtv)
{
    if (txtv->cachedlayoutplan) {
	delete txtv->cachedlayoutplan;
	txtv->cachedlayoutplan = NULL;
    }
    txtv->cachedplanwid = 0;
    txtv->cachedplanhgt = 0;
}

/* remove lines from endln on. Note that this produces a less efficient layout, since no sruns are removed. */
static void DeleteLinesFromLayout(struct textps_layout *dest, long endln)
{
    long lx;
    struct textps_line *ln;

    if (endln >= dest->numlines)
	return;
    if (endln < 0)
	endln = 0;

    for (lx=endln; lx<dest->numlines; lx++) {
	ln = dest->lines[lx];
	if (ln->words) {
	    free(ln->words);
	    ln->words = NULL;
	}
	if (ln->letters) {
	    free(ln->letters);
	    ln->letters = NULL;
	}
	free(ln);
	dest->lines[lx] = NULL;
    }
    dest->numlines = endln;
}

/* delete lines from endln to end, and the tmpwords for them. */
static void DeleteLinesFromFootThing(struct textps_simple_layout_state *stat, long endln)
{
    struct textps_slurp  *slurp = stat->slurp;
    struct textps_line *ln;
    int eatat;
    long eatletterat;
    int ix;

    if (!stat->footnotes || endln >= stat->footnotes->numlines)
	return;

    ln = stat->footnotes->lines[endln];
    eatat = ln->firstword - slurp->wordseaten;
    if (eatat < 0)
	eatat = 0;

    eatletterat = (-1);
    for (ix=eatat; ix<slurp->numwords; ix++)
	if (slurp->tmpwords[ix].start >= 0) {
	    eatletterat = slurp->tmpwords[ix].start;
	    break;
	}

    if (eatletterat >= 0) {
	slurp->letters_pt = eatletterat;
    }

    slurp->numwords = eatat;
    if (eatat > 0) {
	slurp->tmpwords[eatat-1].breakable = 5;
    }

    DeleteLinesFromLayout(stat->footnotes, endln);
}

/* startln is the line to delete up to. This just wipes out the tmpwords in question */
static void DeleteWordsFromFootThing(struct textps_simple_layout_state *stat, long startln)
{
    struct textps_line *ln;
    int numtoeat;

    if (startln >= stat->footnotes->numlines) {
	numtoeat = stat->slurp->numwords;
    }
    else {
	ln = stat->footnotes->lines[startln];
	numtoeat = ln->firstword - stat->slurp->wordseaten;
    }
    PurgeSlurpedWords(stat->slurp, numtoeat);
}

/* add a special line to the end of the layout. lineflag may be 0, for an empty line, or lineflag_*. */
static void AddSpecialLineToLayout(struct textps_layout *dest, long lineflag, long lineheight, long colnum)
{
    struct textps_line *destln;
    long lx2;

    destln = (struct textps_line *)malloc(sizeof(struct textps_line));
    destln->colnum = colnum;
    destln->lineheight = lineheight;
    destln->baseline = 0;
    destln->clipline = (-1);
    destln->numwords = lineflag;
    destln->words = NULL;
    destln->letters = NULL;

    lx2 = dest->numlines;
    if (lx2 >= dest->lines_size) {
	dest->lines_size *= 2;
	dest->lines = (struct textps_line **)realloc(dest->lines, dest->lines_size * sizeof(struct textps_line *));
    }
    dest->numlines++;
    dest->lines[lx2] = destln;
}

/* copy lines from src to *destptr. If destptr is NULL, create a new layout and return it. Otherwise, *destptr will not change (whew).
 if destrunlist is NULL, use it. Otherwise, use the runlist in the dest layout. Ditto for srcrunlist. */
static void CopyLinesToLayout(struct textps_layout **destptr, struct textps_srunlist *destrunlist, struct textps_layout *src, struct textps_srunlist *srcrunlist, long startline, long endline, long colnum, long trimpt)
{
    struct textps_layout *dest;
    long runnew;
    long lx, lx2, wx, sx, letsize, letend;
    struct textps_line *destln, *srcln;
    long hgtsofar;
    struct textps_textviewlist *tvl;

    int ix;

    if (*destptr) {
	dest = (*destptr);
    }
    else {
	dest = new textps_layout;
	dest->lines_size = 4;
	dest->lines = (struct textps_line **)malloc(dest->lines_size * sizeof(struct textps_line *));
	dest->numlines = 0;
	dest->planid = textview::GetUniquePlanId();
	dest->runs.r = NULL;
	dest->runs.size = 0;
	dest->runs.num = 0;
	
	*destptr = dest;
    }

    if (!destrunlist)
	destrunlist = (&dest->runs);
    if (!srcrunlist)
	srcrunlist = (&src->runs);

    /* ### be inefficient about which sruns to copy */
    if (destrunlist->size==0) {
	destrunlist->size = 4;
	destrunlist->r = (struct textps_stylerun *)malloc(destrunlist->size * sizeof(struct textps_stylerun));
	destrunlist->num = 0;
    }
    runnew = destrunlist->num;

    hgtsofar = 0;
    for (lx=startline; lx<endline; lx++) {
	lx2 = dest->numlines;
	if (lx2 >= dest->lines_size) {
	    dest->lines_size *= 2;
	    dest->lines = (struct textps_line **)realloc(dest->lines, dest->lines_size * sizeof(struct textps_line *));
	}
	dest->numlines++;
	destln = (struct textps_line *)malloc(sizeof(struct textps_line));
	dest->lines[lx2] = destln;
	srcln = (src->lines[lx]);
	*destln = (*srcln); /* copy struct */
	destln->colnum = colnum;
	if (trimpt >= 0 && hgtsofar+destln->lineheight > trimpt) {
	    destln->clipline = trimpt - hgtsofar;
	    if (destln->clipline < 0)
		destln->clipline = 0;
	}
	hgtsofar += destln->lineheight;
	if (srcln->numwords > 0) {
	    destln->words = (struct textps_word *)malloc(srcln->numwords * sizeof(struct textps_word));
	    letsize = 0;
	    for (wx=0; wx<srcln->numwords; wx++) {
		destln->words[wx] = srcln->words[wx]; /* copy struct */
		destln->words[wx].run += runnew;
		if (destln->words[wx].start >= 0) {
		    letend = destln->words[wx].start + destln->words[wx].len;
		    if (letend > letsize)
			letsize = letend;
		}
	    }
	    if (letsize) {
		destln->letters = (char *)malloc(letsize);
		memcpy(destln->letters, srcln->letters, letsize);
	    }
	    else {
		destln->letters = (char *)malloc(1);
	    }
	}
	else {
	    /* case of zero words, *or* lineflag_* */
	    destln->words = NULL;
	    destln->letters = NULL;
	}
    }

    for (sx=0; sx<srcrunlist->num; sx++) {
	if (sx+runnew >= destrunlist->size) {
	    destrunlist->size *= 2;
	    destrunlist->r = (struct textps_stylerun *)realloc(destrunlist->r, destrunlist->size * sizeof(struct textps_stylerun));
	}
	destrunlist->r[sx+runnew] = srcrunlist->r[sx];
	destrunlist->num++;
    }
}

#define usedef_tU	(1)
#define usedef_tBox	(2)

/* all the stuff that you need to know when laying out a column in a text document, but not when just drawing a rectangle (or header or footnote) */
struct textps_layout_blonk {
    struct textps_header head[2];
    long nextpagenum;
    long nextline;
};
/* all the stuff you need to know to print a column, mostly */
struct textps_layout_place {
    long colnum;
    struct textps_layout_column *col;
};

static void PrintColumn(FILE *outfile, struct textps_layout_blonk *blonk, long startline, struct textps_layout *ly, struct textps_layout_place *place, long *defsused)
{
    long lx, wx;
    long tmp, tmp2, miscflags;
    double dtmp, dtmp2;
    long start_tBox, end_tBox; 
    struct textps_stylerun *srun_tBox;
    long start_tTab, end_tTab; 
    struct textps_stylerun *srun_tTab;
    int curfontindex, curfontsize, curbaseline;
    long pageposx, pageposy;
    int headnum, headflag;
    struct textps_line *tlin;
    struct textps_stylerun *srun, *tmpsrun;

    lx = startline;

    pageposx = place->col->x * RESOLUTION;
    pageposy = place->col->y * RESOLUTION;

#if 0  /* debugging code to draw outlines around the column parts */ 
    {
	int ix;
	long colypos;
	struct textps_layout_column *col = place->col;
	colypos = pageposy;
	for (ix=0; ix<col->numparts; ix++) {
	    fprintf(outfile, "gsave 0 setlinewidth\n");
	    fprintf(outfile, "%g %g moveto\n", ToPS(pageposx + col->parts[ix].x1 * RESOLUTION), ToPS(colypos));
	    fprintf(outfile, "%g %g lineto\n", ToPS(pageposx + col->parts[ix].x2 * RESOLUTION), ToPS(colypos));
	    colypos -= RESOLUTION * col->parts[ix].height;
	    fprintf(outfile, "%g %g lineto\n", ToPS(pageposx + col->parts[ix].x2 * RESOLUTION), ToPS(colypos));
	    fprintf(outfile, "%g %g lineto\n", ToPS(pageposx + col->parts[ix].x1 * RESOLUTION), ToPS(colypos));
	    fprintf(outfile, "closepath stroke grestore\n");
	}
    }
#endif

    curfontindex = NULLINT;
    curfontsize = NULLINT;
    curbaseline = NULLINT;
    while (lx<ly->numlines && ly->lines[lx]->colnum==place->colnum) {
	tlin = ly->lines[lx];
	curbaseline = NULLINT; /* always dump this at the beginning of a line (when pageposy or tlin changes) */
	start_tBox = NULLINT;
	srun_tBox = NULL;
	start_tTab = NULLINT;
	srun_tTab = NULL;
	/* beginning of line */
	if (tlin->clipline >= 0) {
	    /* clip the vertical extent of this line (horizontal is allowed to extend across the page) */
	    fprintf(outfile, "gsave\n");
	    fprintf(outfile, "%g %g moveto  0 %g rlineto  %g 0 rlineto  0 %g rlineto\n",
		    0.0, ToPS(pageposy),
		    ToPS(-tlin->clipline),
		    (double)(place->col->w),
		    ToPS(tlin->clipline));
	    fprintf(outfile, "closepath clip newpath\n");
	}
	switch (tlin->numwords) {
	    case lineflag_Footnote:
		/* draw the nifty horizontal line at one point above the bottom of the lineheight. It should be one point wide, and start at the default left margin and go for 1 inch. (is this correct?) */
		fprintf(outfile, "gsave 1 setlinewidth 0 setgray\n");
		fprintf(outfile, "%g %g moveto\n",
			ToPS(pageposx),
			ToPS(pageposy - tlin->lineheight) + 1.0);
		fprintf(outfile, "72 0 rlineto stroke grestore\n");
		break;
	    default:
		/* zero or any positive number */
		break;
	}
	/* really ought to do this next loop only when tlin->numwords > 0, but that's what happens anyway. */
	for (wx=0; wx<tlin->numwords; wx++) {
	    srun = (&ly->runs.r[tlin->words[wx].run]);
	    if (curbaseline != srun->baselineoffset) {
		curbaseline = srun->baselineoffset;
		fprintf(outfile, "%g tL\n", ToPS(pageposy-(tlin->baseline+curbaseline)));
	    }
	    miscflags = srun->miscflags; /* we pull this out so that views can zero it */
	    if (tlin->words[wx].start < 0) {
		switch (tlin->words[wx].start) {
		    case wordflag_Tab: {
			miscflags = 0; /* underlines break at tabs */
			}
			break;
		    case wordflag_View: {
			/* dump a generic view */
			long wid, hgt;
			struct rectangle visrect;
			wid = tlin->words[wx].width;
			hgt = tlin->words[wx].vheight;
			fprintf(outfile, "gsave\n");
			fprintf(outfile, "%g tY %g sub translate\n", ToPS(pageposx+tlin->words[wx].xpos), ToPS(hgt-tlin->words[wx].voff));
			fprintf(outfile, "0 0 moveto %g 0 lineto %g %g lineto 0 %g lineto closepath clip newpath\n", ToPS(wid), ToPS(wid), ToPS(hgt), ToPS(hgt));
			visrect.left = (long)ToPS(0);
			visrect.top = (long)ToPS(hgt);
			visrect.width = (long)ToPS(wid);
			visrect.height = (long)ToPS(hgt);
			tlin->words[wx].v->PrintPSRect(outfile, visrect.width, visrect.height, &visrect);
			fprintf(outfile, "grestore\n");
			curbaseline = NULLINT; /* because the view might have played with tY */
			miscflags = 0; /* no such thing as an underlined view */
			}
			break;
		    case wordflag_Spec: {
			switch (tlin->words[wx].inset.type) {
			    case textview_NewPage: {
				class bpv *v = tlin->words[wx].inset.u.NewPage;
				class bp *bp;
				if (!blonk)
				    break; /* pagebreaks have no effect in non-top-level text objects */
				bp = (class bp *)(v->GetDataObject());
				if (bp && bp->haspagenum) {
				    blonk->nextpagenum = bp->pagenum;
				}
				}
				break;
			    case textview_Footnote: {
				class fnotev *v = tlin->words[wx].inset.u.Footnote;
				class fnote *fn;
				if (!blonk)
				    break; /* footnotes have no effect in non-top-level text objects */
				fn = (class fnote *)(v->GetDataObject());
				}
				break;
			    case textview_Header: {
				class headrtv *v = tlin->words[wx].inset.u.Header;
				class header *hd;
				char *hstr;
				boolean haspage;
				if (!blonk)
				    break; /* headers have no effect in non-top-level text objects */
				hd = (class header *)(v->GetDataObject());
				headflag = hd->where;
				for (headnum=0; headnum<header_TEXTS; headnum++) {
				    hstr = v->GetExpandedString(headnum, &haspage);
				    if (hstr) {
					blonk->head[headflag].dirty = TRUE;
					if (blonk->head[headflag].string [headnum]) {
					    free(blonk->head[headflag].string [headnum]);
					}
					blonk->head[headflag].string [headnum] = hstr;
					if (haspage)
					    blonk->head[headflag].haspage |= (1<<headnum);
					else
					    blonk->head[headflag].haspage &= ~(1<<headnum);
				    }
				}
				}
				break;
			    default:
				break;
			}
			}
			break;
		    default:
			break;
		}
	    }
	    else {
		/* regular word */
		if (tlin->words[wx].len!=0) {
		    if (srun->fontindex!=curfontindex || srun->fontsize!=curfontsize) {
			curfontindex = srun->fontindex;
			curfontsize = srun->fontsize;
			fprintf(outfile, "%s%d %d scalefont setfont\n", print_PSFontNameID, curfontindex, curfontsize);
		    }
		    print::GeneratePSWord(outfile, tlin->letters+tlin->words[wx].start, tlin->words[wx].len, ToPS(pageposx+tlin->words[wx].xpos), srun->encoding, srun->afm, srun->fontsize);
		}
	    }
	    /* the following sections must be executed even for tab or view "words", but they will crash if the tab or view has any of the special features (underline, etc.) Well, only the DottedBox section actually needs to be executed. But I want to keep them together. */
	    if (miscflags & style_Underline) {
		tmp2 = tlin->words[wx].xpos;
		if (wx+1==tlin->numwords)
		    tmp = tlin->words[wx].xpos+tlin->words[wx].width;
		else
		    tmp = tlin->words[wx+1].xpos;
		if (tmp2 != tmp) {
		    *defsused |= usedef_tU;
		    dtmp = srun->afm->h.underlineposition; /* 0 if no underline position */
		    fprintf(outfile, "%g %g %g %g tU\n", ToPS(pageposx+tmp2), ToPS(pageposx+tmp), AFMToPS(dtmp, srun->fontsize), AFMToPS(srun->afm->h.underlinethickness, srun->fontsize));
		}
	    }
	    if (miscflags & style_OverBar) {
		tmp2 = tlin->words[wx].xpos;
		if (wx+1==tlin->numwords)
		    tmp = tlin->words[wx].xpos+tlin->words[wx].width;
		else
		    tmp = tlin->words[wx+1].xpos;
		if (tmp2 != tmp) {
		    *defsused |= usedef_tU;
		    if (srun->afm->ascender==0.0)
			dtmp = 1000.0 - srun->afm->h.underlinethickness; /* top of line */
		    else 
			dtmp = srun->afm->ascender + srun->afm->h.underlinethickness;
		    fprintf(outfile, "%g %g %g %g tU\n", ToPS(pageposx+tmp2), ToPS(pageposx+tmp), AFMToPS(dtmp, srun->fontsize), AFMToPS(srun->afm->h.underlinethickness, srun->fontsize));
		}
	    }
	    if (miscflags & style_StrikeThrough) {
		tmp2 = tlin->words[wx].xpos;
		if (wx+1==tlin->numwords)
		    tmp = tlin->words[wx].xpos+tlin->words[wx].width;
		else
		    tmp = tlin->words[wx+1].xpos;
		if (tmp2 != tmp) {
		    *defsused |= usedef_tU;
		    dtmp = srun->afm->xheight / 2.0;
		    if (dtmp==0.0)
			dtmp = 500.0; /* middle of line */
		    fprintf(outfile, "%g %g %g %g tU\n", ToPS(pageposx+tmp2), ToPS(pageposx+tmp), AFMToPS(dtmp, srun->fontsize), AFMToPS(srun->afm->h.underlinethickness, srun->fontsize));
		}
	    }
	    if (tlin->words[wx].start == wordflag_Tab) {
		if (start_tTab==NULLINT) {
		    start_tTab = tlin->words[wx].xpos;
		    srun_tTab = srun;
		    end_tTab = tlin->words[wx].xpos;
		}
		else {
		    end_tTab = tlin->words[wx].xpos;
		}
	    }
	    if ((start_tTab != NULLINT)
		&& ((wx+1 == tlin->numwords)
		    || (tlin->words[wx+1].start != wordflag_Tab)
		    || (srun != (&ly->runs.r[tlin->words[wx+1].run])))) {
		if ((wx+1 == tlin->numwords))
		    tmp = end_tTab;
		else
		    tmp = tlin->words[wx+1].xpos;
		tmp2 = start_tTab;
		/* now display the dots and turn off the box flag */
		start_tTab = NULLINT;
		if (srun_tTab->tabfillwidth && (tmp2 != tmp)) {
		    long reppos;
		    short tabfilllen = strlen(srun_tTab->tabfill);
		    if (srun_tTab->fontindex!=curfontindex || srun_tTab->fontsize!=curfontsize) {
			curfontindex = srun_tTab->fontindex;
			curfontsize = srun_tTab->fontsize;
			fprintf(outfile, "%s%d %d scalefont setfont\n", print_PSFontNameID, curfontindex, curfontsize);
		    }
		    for (reppos = tmp2; reppos+srun_tTab->tabfillwidth <= tmp; reppos += srun_tTab->tabfillwidth) {
			print::GeneratePSWord(outfile, srun_tTab->tabfill, tabfilllen, ToPS(pageposx+reppos), srun_tTab->encoding, srun_tTab->afm, srun_tTab->fontsize);
		    }
		}
	    }
	    if (miscflags & style_DottedBox) {
		if (start_tBox==NULLINT) {
		    start_tBox = tlin->words[wx].xpos;
		    srun_tBox = srun;
		    end_tBox = tlin->words[wx].xpos+tlin->words[wx].width;
		}
		else {
		    end_tBox = tlin->words[wx].xpos+tlin->words[wx].width;
		}
	    }
	    if ((start_tBox != NULLINT)
		&& ((wx+1 == tlin->numwords)
		    || (srun != (&ly->runs.r[tlin->words[wx+1].run])))) {
		if ((wx+1 == tlin->numwords))
		    tmp = end_tBox;
		else
		    tmp = tlin->words[wx+1].xpos;
		tmp2 = start_tBox;
		/* now display the box and turn off the box flag */
		start_tBox = NULLINT;
		if (tmp2 != tmp) {
		    *defsused |= usedef_tBox;
		    if (srun_tBox->afm->ascender==0.0)
			dtmp = 1000.0 - srun_tBox->afm->h.underlinethickness; /* top of line */
		    else 
			dtmp = srun_tBox->afm->ascender + srun_tBox->afm->h.underlinethickness;
		    dtmp2 = srun_tBox->afm->h.underlineposition; /* 0 if no underline position */
		    fprintf(outfile, "%g %g %g %g %g tBox\n", ToPS(pageposx+tmp2), ToPS(pageposx+tmp), AFMToPS(dtmp2, srun_tBox->fontsize), AFMToPS(dtmp, srun_tBox->fontsize), AFMToPS(srun_tBox->afm->h.underlinethickness, srun_tBox->fontsize));
		}
	    }
	}
	if (tlin->clipline >= 0) {
	    fprintf(outfile, "grestore\n");
	    curfontindex = NULLINT;
	    curfontsize = NULLINT;
	    curbaseline = NULLINT;
	}
	/* end of line */

	pageposy -= tlin->lineheight;
	lx++;
    }
    /* end of this column */

    if (blonk)
	blonk->nextline = lx;
}

static void register_text_defs(long defsused)
{
    print::PSRegisterDef("tL", "{/tY exch store} bind");
    print::PSRegisterDef("tY", "0");

    if (defsused & usedef_tU) {
	print::PSRegisterDef("tU", "{gsave setlinewidth tY add dup 4 -1 roll exch moveto lineto stroke grestore} bind"); /* x0 x1 yoff width tU */
    }
    if (defsused & usedef_tBox) {
	print::PSRegisterDef("tBox", "{gsave setlinewidth [1 2] 1 setdash \n dup tY add 3 index exch moveto \n sub 3 1 roll sub exch \n dup 0 exch rlineto  exch dup 0 rlineto  exch dup neg 0 exch rlineto  closepath pop pop \n stroke grestore} bind"); /* x0 x1 yoff0 yoff1 linewidth tBox */
    }
}

static void insert_header_line(class text *t, char *str, long pagenum)
{
    char *cx;
    static char numbuf[16];
    boolean inited = FALSE;

    cx = strstr(str, "\\\\n%");
    while (cx) {
	t->InsertCharacters(999999, str, (cx-str));
	if (!inited) {
	    sprintf(numbuf, "%d", pagenum);
	    inited = TRUE;
	}
	t->InsertCharacters(99999, numbuf, strlen(numbuf));
	str = (cx+4); /* skip the "\\n%" also */
	cx = strstr(str, "\\\\n%");
    }
    t->InsertCharacters(999999, str, strlen(str));
}

static void rebuild_header(struct textps_header *hx, long width, long pagenum, boolean swapflag)
{
    char *cx;
    class style *st;
    if (pagenum % 2 == 1)
	swapflag = FALSE; /* only swap headers on even pages */
    /* width is the width of the page. */
    if (width<0)
	width = 0;
    hx->t->Clear();
    st = new style;
    st->SetName("global");
    st->AddTabChange(style_AllClear, 0, style_Points);
    st->AddTabChange(style_CenteredOnTab, width/2, style_Points);
    st->AddTabChange(style_RightAligned, width, style_Points);
    hx->t->SetGlobalStyle(st);
    cx = hx->string[swapflag ? header_rtext : header_ltext];
    if (cx) {
	insert_header_line(hx->t, cx, pagenum);
    }
    hx->t->InsertCharacters(999999, "\t", 1);
    cx = hx->string[header_ctext];
    if (cx) {
	insert_header_line(hx->t, cx, pagenum);
    }
    hx->t->InsertCharacters(999999, "\t", 1);
    cx = hx->string[swapflag ? header_ltext : header_rtext];
    if (cx) {
	insert_header_line(hx->t, cx, pagenum);
    }
    hx->dirty = FALSE;
}

#ifdef RCH_ENV
void SetIBMClassFooter(struct textps_layout_blonk *blonk)
{
    char *dist = environ::Get("DIST");
    char dcode;
    int i;
    static char *classifications[] = {
	"Internal Use Only",
	"Confidential",
	"Restricted",
    };
    /* Corresponds to classifications.  Values used
     * in the DIST environment var. Keep this
     * list NULL terminated.
     */
    static char classificationsenv[] = {
	'i', 'c', 'z',
    };

    if (!dist || strlen(dist) < 4)
	return;
    dcode = tolower(dist[3]);
    for (i = 0; i < 6; i++)
	if (dcode == classificationsenv[i])
	    break;
    if (i < 6) {
	/* Found it.  Set the current value. */
	blonk->head[header_FOOTER].string[header_ctext] = NewString(classifications[i]);
	blonk->head[header_FOOTER].haspage |= (1<<header_ctext);
    }
}
#endif


void textview::PrintPSDoc(FILE *outfile, long width, long height)
{
    PrintPSText(this, outfile, width, height, 1, TRUE);
}

static void PrintPSText(class textview *self, FILE *outfile, long width, long height, long firstpagenum, boolean allowsubs)
{
    long colnum;
    long lx, wx;
    struct textps_layout *ly;
    struct textps_layout_plan *layoutplan;
    struct textps_layout_history *history;
    int headnum, headflag;
    struct textps_layout_blonk blonk;
    long thispagenum;
    boolean firstpage;
    long defsused = 0;
    class text *t;
    boolean swapheaders;
    struct textps_layout_state lstat;
    struct textps_locatag *lub;
    /* stuff for subcalls */
    boolean docontents, doindex;
    class text *thistext, *contentstext, *indextext;
    class viewref *vref;
    void *initiallayoutrock;
    textview_getlayout_fptr initiallayoutfunc;
    boolean suppressmaintext = FALSE;

    contentstext = NULL;
    if (allowsubs && self->GetPrintOption(A_docontents)) {
	docontents = TRUE;
    }
    else {
	docontents = FALSE;
    }
    indextext = NULL;
    if (allowsubs && self->GetPrintOption(A_doindex)) {
	doindex = TRUE;
	if (environ::Get("IndexOnly") != NULL) {
	    suppressmaintext = TRUE;
	}
    }
    else {
	doindex = FALSE;
    }
    
    textview_DestroyPrintingLayout(self); /* ### really only need to do this if the file has changed since the last time it was printed */

    /* check if the damn text object starts with a layout plan. Wouldn't that be sweet? */
    initiallayoutrock = NULL;
    initiallayoutfunc = NULL;
    thistext = (class text *)(self->GetDataObject());
    if (thistext && (vref = thistext->FindViewreference(0, 1))) {
	class view *v;
	class view *parview = self;
	v = (class view *)dictionary::LookUp(parview, (char *) vref);
	if (v == NULL) {
	    ATK::LoadClass(vref->viewType);
	    if (ATK::IsTypeByName(vref->viewType, "view")
		&& (v = (class view *)
		    matte::Create(vref, (class view *) parview)) != NULL) {
		(vref)->AddObserver( parview);
		dictionary::Insert(parview, (char *) vref, (char *) v);
	    }
	}
	else {
	    if (v == (class view *) textview_UNKNOWNVIEW)
		v = NULL;
	}

	if (v) {
	    struct textview_insetdata *tmpdat;
	    tmpdat = (struct textview_insetdata *)v->GetPSPrintInterface("text");
	    if (tmpdat && tmpdat->type == textview_Layout) {
		initiallayoutrock = tmpdat->u.Layout.rock;
		initiallayoutfunc = tmpdat->u.Layout.func;
	    }
	}
    }

    /* make a new layout plan and clobber it at the end of the printing process. It's not worth the effort to keep it around in case the printing conditions haven't changed next time */
    if (initiallayoutfunc) {
	layoutplan = initiallayoutfunc(initiallayoutrock, width, height);
    }
    else if (!self->GetPrintOption(A_twocolumns)) {
	/* one column per page, one part per column -- the usual */
	layoutplan = new textps_layout_plan;
	layoutplan->planid = textview::GetUniquePlanId();
	layoutplan->numcols = 1;
	layoutplan->repeat = 1;
	layoutplan->cols = (struct textps_layout_column *)malloc(sizeof(struct textps_layout_column) * layoutplan->numcols);
	layoutplan->cols[0].coltags = textview_coltag_NewPage | textview_coltag_FootNotes | textview_coltag_PageBreak;
	layoutplan->cols[0].numparts = 1;
	layoutplan->cols[0].background = NULL;
	layoutplan->cols[0].x = textps_DefaultMargin;
	layoutplan->cols[0].w = width - 2*textps_DefaultMargin;
	layoutplan->cols[0].y = height - 72; /* allow 1-inch top margin */
	layoutplan->cols[0].parts = (struct textps_layout_block *)malloc(sizeof(struct textps_layout_block) * layoutplan->cols[0].numparts);
	layoutplan->cols[0].parts[0].x1 = 0;
	layoutplan->cols[0].parts[0].x2 = width - 2*textps_DefaultMargin;
	layoutplan->cols[0].parts[0].height = (height - 2*72); /* pull off top and bottom margins */

	SumColumnSizes(layoutplan);
    }
    else {
#if 1
	/* two columns per page */
	layoutplan = new textps_layout_plan;
	layoutplan->planid = textview::GetUniquePlanId();
	layoutplan->numcols = 2;
	layoutplan->repeat = 2;
	layoutplan->cols = (struct textps_layout_column *)malloc(sizeof(struct textps_layout_column) * layoutplan->numcols);
	layoutplan->cols[0].coltags = textview_coltag_NewPage | textview_coltag_FootNotes | textview_coltag_PageBreak;
	layoutplan->cols[0].numparts = 1;
	layoutplan->cols[0].background = NULL;
	layoutplan->cols[0].x = textps_DefaultMargin;
	layoutplan->cols[0].w = width/2 - textps_DefaultMargin - 18;
	layoutplan->cols[0].y = height - 72; /* allow 1-inch top margin */
	layoutplan->cols[0].parts = (struct textps_layout_block *)malloc(sizeof(struct textps_layout_block) * layoutplan->cols[0].numparts);
	layoutplan->cols[0].parts[0].x1 = 0;
	layoutplan->cols[0].parts[0].x2 = width/2 - textps_DefaultMargin - 18;
	layoutplan->cols[0].parts[0].height = (height - 2*72); 
	layoutplan->cols[1].coltags = textview_coltag_FootNotes;
	layoutplan->cols[1].numparts = 1;
	layoutplan->cols[1].background = NULL;
	layoutplan->cols[1].x = width/2 + 18;
	layoutplan->cols[1].w = width/2 - textps_DefaultMargin - 18;
	layoutplan->cols[1].y = height - 72; /* allow 1-inch top margin */
	layoutplan->cols[1].parts = (struct textps_layout_block *)malloc(sizeof(struct textps_layout_block) * layoutplan->cols[1].numparts);
	layoutplan->cols[1].parts[0].x1 = 0;
	layoutplan->cols[1].parts[0].x2 = width/2 - textps_DefaultMargin - 18;
	layoutplan->cols[1].parts[0].height = (height - 2*72); 

	SumColumnSizes(layoutplan);
#else
	/* two columns per page */
	layoutplan = new textps_layout_plan;
	layoutplan->planid = textview::GetUniquePlanId();
	layoutplan->numcols = 3;
	layoutplan->repeat = 3;
	layoutplan->cols = (struct textps_layout_column *)malloc(sizeof(struct textps_layout_column) * layoutplan->numcols);
	layoutplan->cols[0].coltags = textview_coltag_NewPage | textview_coltag_PageBreak;
	layoutplan->cols[0].numparts = 1;
	layoutplan->cols[0].background = NULL;
	layoutplan->cols[0].x = textps_DefaultMargin;
	layoutplan->cols[0].w = width - 2*textps_DefaultMargin;
	layoutplan->cols[0].y = height - 72; /* allow 1-inch top margin */
	layoutplan->cols[0].parts = (struct textps_layout_block *)malloc(sizeof(struct textps_layout_block) * layoutplan->cols[0].numparts);
	layoutplan->cols[0].parts[0].x1 = 0;
	layoutplan->cols[0].parts[0].x2 = width - 2*textps_DefaultMargin;
	layoutplan->cols[0].parts[0].height = (2*72); /* pull off top and bottom margins */
	layoutplan->cols[1].coltags = textview_coltag_FootNotes;
	layoutplan->cols[1].numparts = 1;
	layoutplan->cols[1].background = NULL;
	layoutplan->cols[1].x = textps_DefaultMargin;
	layoutplan->cols[1].w = width/2 - textps_DefaultMargin - 18 - 72;
	layoutplan->cols[1].y = height - 72 - 2*72; /* allow 1-inch top margin */
	layoutplan->cols[1].parts = (struct textps_layout_block *)malloc(sizeof(struct textps_layout_block) * layoutplan->cols[1].numparts);
	layoutplan->cols[1].parts[0].x1 = 0;
	layoutplan->cols[1].parts[0].x2 = width/2 - textps_DefaultMargin - 18 - 72;
	layoutplan->cols[1].parts[0].height = (height - 2*72 - 2*72); 
	layoutplan->cols[2].coltags = textview_coltag_FootNotes;
	layoutplan->cols[2].numparts = 1;
	layoutplan->cols[2].background = NULL;
	layoutplan->cols[2].x = width/2 + 18 - 72;
	layoutplan->cols[2].w = width/2 - textps_DefaultMargin - 18 + 72;
	layoutplan->cols[2].y = height - 72 - 2*72; /* allow 1-inch top margin */
	layoutplan->cols[2].parts = (struct textps_layout_block *)malloc(sizeof(struct textps_layout_block) * layoutplan->cols[2].numparts);
	layoutplan->cols[2].parts[0].x1 = 0;
	layoutplan->cols[2].parts[0].x2 = width/2 - textps_DefaultMargin - 18 + 72;
	layoutplan->cols[2].parts[0].height = (height - 2*72 - 2*72); 

	SumColumnSizes(layoutplan);
#endif
    }
    /* the layoutplan is done. */

    history = new textps_layout_history;
    history->pagew = width;
    history->pageh = height;
    history->PushNewPlan(layoutplan);
    history->initiallayoutrock = initiallayoutrock;
    history->initiallayoutfunc = initiallayoutfunc;
    print::PSRegisterCleanup(DeleteLayoutPlanProc, (void *)layoutplan);

    swapheaders = self->GetPrintOption(A_swapheaders);

    InitLayoutState(&lstat);
    t = (class text *)self->GetDataObject();
    fnote::CloseAll(t);
    LayoutText(self, history, &lstat);

    /*DumpLayout(self->cachedlayout);*/

    /* Let's play mash-the-potato. The potato, in this case, is all the information we have about what things are on what page. We have to mash all the way through and assign page numbers. This would be trivial, except that we have to scan for number-setting page breaks. */
    lub = lstat.locatags;
    ly = self->cachedlayout;
    lx = 0;
    colnum = 0;
    blonk.nextpagenum = firstpagenum;
    while (lx<ly->numlines && colnum < history->numcols) {
	thispagenum = blonk.nextpagenum;
	blonk.nextpagenum = thispagenum+1; /* may be updated later by a pagebreak inset on this page */
	/* handle all the columns on this page (up to textview_coltag_NewPage) */
	while (lx<ly->numlines && colnum < history->numcols) {
	    while (lx<ly->numlines && ly->lines[lx]->colnum==colnum) {
		struct textps_line *tlin = ly->lines[lx];
		/* beginning of line */
		/* really ought to do this next loop only when tlin->numwords > 0, but that's what happens anyway. */
		for (wx=0; wx<tlin->numwords; wx++) {
		    if (tlin->words[wx].start < 0) {
			switch (tlin->words[wx].start) {
			    case wordflag_Spec: {
				switch (tlin->words[wx].inset.type) {
				    case textview_NewPage: {
					class bpv *v = tlin->words[wx].inset.u.NewPage;
					class bp *bp;
					bp = (class bp *)(v->GetDataObject());
					if (bp && bp->haspagenum) {
					    blonk.nextpagenum = bp->pagenum;
					}
					}
					break;
				    default:
					break;
				}
				}
				break;
			    default:
				break;
			}
		    }
		    else {
			/* regular word */
		    }
		}
		/* end of line */
		lx++;
	    }
	    /* end of this column */

	    /* stuff in the page number stuff */
	    while (lub && lub->linenum<lx) {
		lub->pagenum = thispagenum;
		lub = lub->next;
	    }

	    colnum++;
	    if (colnum < history->numcols && ((history->cols[colnum]->coltags) & textview_coltag_NewPage)) {
		break;
	    }
	    /*coltyp++;
	    if (coltyp >= layoutplan->numcols) {
		coltyp -= layoutplan->repeat;
	    }
	    if (coltyp<layoutplan->numcols && (layoutplan->cols[coltyp].coltags & textview_coltag_NewPage)) {
		break;
	    }  ### */
	}
	/* finished handling columns on this page. */
    };

    /*{
	printf("Tag list:\n");
	for (lub = lstat.locatags; lub; lub=lub->next) {
	    printf(" %d %d %d <%s>\n", lub->type, lub->linenum, lub->pagenum, lub->name);
	}
    }*/

    /* Lo, the potato is mashed. */

    if (docontents) {
	char buf[16];
	contentstext = new text;
	contentstext->ReadTemplate("contenttab", TRUE);
	for (lub = lstat.locatags; lub; lub=lub->next) {
	    if (lub->type == locatag_Content) {
		contentstext->InsertCharacters(999999, lub->name, strlen(lub->name));
		sprintf(buf, "\t%d\n", lub->pagenum);
		contentstext->InsertCharacters(999999, buf, strlen(buf));
	    }
	}
    }
    if (doindex) {
	indextext = tindex::BuildIndexText(lstat.locatags);
    }

    if (!suppressmaintext) { 
	if (self->cachedlayout->numlines <= 0) {
	    /* zero lines in file. print one blank page and return. */
	    print::PSNewPage(print_UsualPageNumbering);
	    return;
	}

	if (allowsubs) {
	    fprintf(outfile, "%% This document was generated by an AUIS text object.\n");
	}
	else {
	    fprintf(outfile, "%% Additional AUIS text object.\n");
	}

	ly = self->cachedlayout;
	lx = 0;
	colnum = 0;
	firstpage = TRUE;
	blonk.nextpagenum = firstpagenum;
	for (headflag=0; headflag<2; headflag++) {
	    for (headnum=0; headnum<header_TEXTS; headnum++) {
		blonk.head[headflag].string[headnum] = NULL;
	    }
	    blonk.head[headflag].t = new text;
	    blonk.head[headflag].tv = new textview;
	    blonk.head[headflag].tv->SetDataObject(blonk.head[headflag].t);
	    blonk.head[headflag].haspage = 0;
	    blonk.head[headflag].dirty = TRUE;
	}
	blonk.head[header_HEADER].string[header_ctext] = NewString("- \\\\n% -");
	blonk.head[header_HEADER].haspage |= (1<<header_ctext);
#ifdef RCH_ENV
    SetIBMClassFooter(&blonk);
#endif
    
	while (lx<ly->numlines && colnum < history->numcols) {
	    thispagenum = blonk.nextpagenum;
	    blonk.nextpagenum = thispagenum+1; /* may be updated later by a pagebreak inset on this page */
	    print::PSNewPage(thispagenum);
	    if (blonk.head[header_HEADER].dirty || blonk.head[header_HEADER].haspage) {
		rebuild_header(&(blonk.head[header_HEADER]), width-2*textps_DefaultMargin, thispagenum, swapheaders);
	    }

	    if (!firstpage && blonk.head[header_HEADER].t->GetLength() > 2) {
		struct textps_header *hx = &(blonk.head[header_HEADER]);
		struct rectangle visrect;
		fprintf(outfile, "gsave %d %d translate\n", textps_DefaultMargin, height-(36+36)); /* half-inch header zone, half-inch from top, at default left margin */
		fprintf(outfile, "0 0 moveto %d 0 rlineto 0 %d rlineto %d 0 rlineto closepath clip newpath\n", width-2*textps_DefaultMargin, 36, -(width-2*textps_DefaultMargin));
		rectangle_SetRectSize(&visrect, 0, 36, width-2*textps_DefaultMargin, 36);
		hx->tv->PrintPSRect(outfile, width-2*textps_DefaultMargin, 36, &visrect);
		fprintf(outfile, "grestore\n");
	    }

	    /* print all the columns on this page (up to textview_coltag_NewPage) */
	    while (lx<ly->numlines && colnum < history->numcols) {
		struct textps_layout_place place;
		place.colnum = colnum;
		place.col = history->cols[colnum];
		if (place.col->background) {
		    rectangle bkrect;
		    rectangle_SetRectSize(&bkrect, 0, height, width, height);
		    fprintf(outfile, "gsave %% page background\n");
		    /* we don't bother to check GetPSPrintInterface; PrintPSRect defaults to no action. */
		    place.col->background->PrintPSRect(outfile, width, height, &bkrect);
		    fprintf(outfile, "grestore\n");
		}
		PrintColumn(outfile, &blonk, lx, ly, &place, &defsused);
		lx = blonk.nextline;

		colnum++;
		if (colnum < history->numcols && ((history->cols[colnum]->coltags) & textview_coltag_NewPage)) {
		    break;
		}
		/*coltyp++;
		 if (coltyp >= layoutplan->numcols) {
		 coltyp -= layoutplan->repeat;
		 }
		 if (coltyp<layoutplan->numcols && (layoutplan->cols[coltyp].coltags & textview_coltag_NewPage)) {
		 break;
		 }*/
	    }

	    if (blonk.head[header_FOOTER].dirty || blonk.head[header_FOOTER].haspage) {
		rebuild_header(&(blonk.head[header_FOOTER]), width - 2*textps_DefaultMargin, thispagenum, swapheaders);
	    }

	    if (blonk.head[header_FOOTER].t->GetLength() > 2) {
		struct textps_header *hx = &(blonk.head[header_FOOTER]);
		struct rectangle visrect;
		fprintf(outfile, "gsave %d %d translate\n", textps_DefaultMargin, 36); /* half-inch header zone, half-inch from bottom, at default left margin */
		fprintf(outfile, "0 0 moveto %d 0 rlineto 0 %d rlineto %d 0 rlineto closepath clip newpath\n", width-2*textps_DefaultMargin, 36, -(width-2*textps_DefaultMargin));
		rectangle_SetRectSize(&visrect, 0, 36, width-2*textps_DefaultMargin, 36);
		hx->tv->PrintPSRect(outfile, width-2*textps_DefaultMargin, 36, &visrect);
		fprintf(outfile, "grestore\n");
	    }
	    firstpage = FALSE;
	};

	for (headflag=0; headflag<2; headflag++) {
	    for (headnum=0; headnum<header_TEXTS; headnum++) {
		if (blonk.head[headflag].string[headnum])
		    free(blonk.head[headflag].string[headnum]);
		blonk.head[headflag].string[headnum] = NULL;
	    }
	    if (blonk.head[headflag].tv) {
		blonk.head[headflag].tv->Destroy();
	    }
	    if (blonk.head[headflag].t)
		blonk.head[headflag].t->Destroy();
	}
    }

    register_text_defs(defsused);
    delete history;

    FinalizeLayoutState(&lstat);

    if (contentstext) {
	class textview *contentstextv = new textview;
	contentstextv->SetDataObject(contentstext);
	PrintPSText(contentstextv, outfile, width, height, 1, FALSE);
	delete contentstextv;
	delete contentstext;
    }
    if (indextext) {
	class textview *indextextv = new textview;
	indextextv->SetDataObject(indextext);
	PrintPSText(indextextv, outfile, width, height, 1, FALSE);
	delete indextextv;
	delete indextext;
    }
}

static struct textps_layout_plan *CreateRectLayoutPlan(long width, long height)
{
    struct textps_layout_plan *layoutplan;

    layoutplan = new textps_layout_plan;
    layoutplan->planid = textview::GetUniquePlanId();
    layoutplan->numcols = 1;
    layoutplan->repeat = 0;
    layoutplan->cols = (struct textps_layout_column *)malloc(sizeof(struct textps_layout_column) * layoutplan->numcols);
    layoutplan->cols[0].coltags = textview_coltag_NewPage | textview_coltag_FootNotes | textview_coltag_PageBreak;
    layoutplan->cols[0].numparts = 1;
    layoutplan->cols[0].background = NULL;
    layoutplan->cols[0].x = 0;
    layoutplan->cols[0].w = width;
    layoutplan->cols[0].y = height; 
    layoutplan->cols[0].h = height;
    layoutplan->cols[0].parts = (struct textps_layout_block *)malloc(sizeof(struct textps_layout_block) * layoutplan->cols[0].numparts);
    layoutplan->cols[0].parts[0].x1 = 0;
    layoutplan->cols[0].parts[0].x2 = width;
    layoutplan->cols[0].parts[0].height = (height);

    return layoutplan;
}

void textview::PrintPSRect(FILE *outfile, long width, long height, struct rectangle *visrect)
{
    long defsused = 0;
    struct textps_layout *ly;
    struct textps_layout_place place;
    class text *t;
    struct textps_layout_state lstat;
    struct textps_layout_history *history;
    
    textview_DestroyPrintingLayout(this); /* ### really only need to do this if the file has changed since the last time it was printed */

    if (this->cachedlayoutplan && (this->cachedplanwid != width || this->cachedplanhgt != height)) {
	textview_DestroyPrintingLayoutPlan(this);
    }

    if (!this->cachedlayoutplan) {
	this->cachedplanwid = width;
	this->cachedplanhgt = height;
	this->cachedlayoutplan = CreateRectLayoutPlan(width, height);
    }

    history = new textps_layout_history;
    history->pagew = width;
    history->pageh = height;
    history->PushNewPlan(this->cachedlayoutplan);

    InitLayoutState(&lstat);
    t = (class text *)this->GetDataObject();
    fnote::CloseAll(t);
    LayoutText(this, history, &lstat);
    ly = this->cachedlayout;
    place.colnum = 0;
    place.col = history->cols[place.colnum];
    /*place.layoutplan = this->cachedlayoutplan;*/
    PrintColumn(outfile, NULL, 0, ly, &place, &defsused);
   
    register_text_defs(defsused);
    delete history;
}

void *textview::GetPSPrintInterface(char *printtype)
{
    /* ### we might not want the "text" stitching in general -- mostly I stuck it in for testing purposes. */
    /*if (!strcmp(printtype, "text")) {
	static struct textview_insetdata dat;
	dat.type = textview_StitchText;
	dat.u.StitchText = this;
	return (void *)(&dat);
    }*/

    if (!strcmp(printtype, "generic")) {
	return (void *)this;
    }

    return NULL;
}

