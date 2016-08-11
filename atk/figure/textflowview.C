/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
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
char *textflowview_c_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/textflowview.C,v 1.2 1995/03/29 20:39:30 Zarf Stab74 $";
#endif

#include <andrewos.h>
ATK_IMPL("textflowview.H")
#include <textflowview.H>

#include <textview.H>
#include <figobj.H>
#include <figogrp.H>
#include <textflow.H>
#include <figtoolview.H>

ATKdefineRegistry(textflowview, figview, NULL);

textflowview::textflowview()
{
    ATKinit;

    THROWONFAILURE( TRUE);
}

textflowview::~textflowview()
{
    ATKinit;
}

void textflowview::BuildToolList(struct figtoolview_tool_t *&list, int &listnum, int &listsize)
{
#define FIGOBJS_NUM (1)
    static struct figtoolview_tool_t objectlayout[FIGOBJS_NUM] = {
	{"figoflow", 0},
    };
    int ix;

    this->figview::BuildToolList(list, listnum, listsize);

    if (listnum+FIGOBJS_NUM > listsize) {
	while (listnum+FIGOBJS_NUM > listsize)
	    listsize *= 2;
	list = (struct figtoolview_tool_t *)realloc(list, listsize * sizeof(struct figtoolview_tool_t));
    }

    for (ix=0; ix<FIGOBJS_NUM; ix++) {
	list[listnum+ix] = objectlayout[ix];
    }
    listnum += FIGOBJS_NUM;
}

void textflowview::PrintPSDoc(FILE *outfile, long pagew, long pageh)
{
    /* do nothing. Since this does not call print::PSNewPage, the print package will know that this inset type does not print as a top-level document. */
}

struct printlump {
    class textflowview *self;
    struct textps_layout_plan *plan;
    struct rectangle allbounds;
    long pagewidth, pageheight;
};

/* add some columns to the plan in the printlump */
static boolean PrintSplot(class figobj *o, long  ref, class figure *fig, struct printlump *lump)
{
    struct textps_layout_column *col;
    struct rectangle *bbox;
    class textflowview *self = lump->self;

    if (o->IsType("figoflow")) {
	bbox = o->GetBounds(self);

	if (lump->plan->numcols==0) {
	    lump->plan->cols = (struct textps_layout_column *)malloc(sizeof(struct textps_layout_column) * (lump->plan->numcols+1));
	}
	else {
	    lump->plan->cols = (struct textps_layout_column *)realloc(lump->plan->cols, sizeof(struct textps_layout_column) * (lump->plan->numcols+1));
	}
	col = (&(lump->plan->cols[lump->plan->numcols]));
	lump->plan->numcols++;

	col->coltags = textview_coltag_FootNotes;

	col->x = self->ToPrintPixX(bbox->left);
	col->y = self->ToPrintPixY(bbox->top);
	col->w = self->ToPrintPixW(bbox->width);
	col->h = self->ToPrintPixH(bbox->height);

	col->y = lump->pageheight - col->y; /* cuz the result has to be in PS coords */

	col->numparts = 1;
	col->parts = (struct textps_layout_block *)malloc(sizeof(struct textps_layout_block) * col->numparts);
	col->parts[0].x1 = 0;
	col->parts[0].x2 = col->w;
	col->parts[0].height = col->h;

	col->background = NULL;
    }

    return FALSE;
}

long textflowview::BuildPlan(struct textps_layout_plan **planptr, long pagewidth, long pageheight)
{
    class textflow *fig = (class textflow *)this->dataobject;
    class figobj *foc;
    struct printlump lump;
    int startcol;

    this->FlushDataChanges();

    /* perform page-size adjustment */
    foc = (fig)->FindObjectByRef( (fig)->RootObjRef());
    if (foc 
	&& (foc)->IsGroup() 
	&& ((class figogrp *)foc)->doconstraints) {
	long x1 = (this)->ToDefFigX( 0);
	long y1 = (this)->ToDefFigY( 0);
	long x2 = (this)->ToDefFigX( 0+pagewidth);
	long y2 = (this)->ToDefFigY( 0+pageheight);

	((class figogrp *)foc)->MoveHandle( x1, y1, 3);
	((class figogrp *)foc)->MoveHandle( x2, y2, 7);
	this->lastheight--; /* tell view that it should resize the root group, by convincing it that it has changed size. */
    }
   

    lump.self = this;
    lump.plan = (*planptr);
    lump.allbounds = (*(fig->GetOverallBounds()));
    lump.pagewidth = pagewidth;
    lump.pageheight = pageheight;

    startcol = lump.plan->numcols;

    fig->EnumerateObjectTree( figure_NULLREF, NULL, FALSE, (figure_eofptr)PrintSplot, (long)&lump);
    if (lump.plan->numcols == startcol) {
	/* no columns? we'll add an empty column so that it can have a background. */
	struct textps_layout_column *col;
	if (lump.plan->numcols==0) {
	    lump.plan->cols = (struct textps_layout_column *)malloc(sizeof(struct textps_layout_column) * (lump.plan->numcols+1));
	}
	else {
	    lump.plan->cols = (struct textps_layout_column *)realloc(lump.plan->cols, sizeof(struct textps_layout_column) * (lump.plan->numcols+1));
	}
	col = (&(lump.plan->cols[lump.plan->numcols]));
	lump.plan->numcols++;
	col->coltags = 0;
	col->x = 0;
	col->y = 0;
	col->w = 0;
	col->h = 0;
	col->numparts = 0;
	col->parts = NULL;
	col->background = NULL;
    }
    if (lump.plan->numcols > startcol) {
	/* there better be one now */
	lump.plan->cols[startcol].coltags |= (textview_coltag_NewPage | textview_coltag_PageBreak);
	lump.plan->cols[startcol].background = this;
    }

    *planptr = lump.plan;
    return startcol;
}

void *textflowview::GetPSPrintInterface(char *printtype)
{
    static struct textview_insetdata dat;

    /* this thing doesn't print at all, by itself. */

    return NULL;
}

