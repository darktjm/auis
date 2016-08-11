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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/table/RCS/tsearch.C,v 1.11 1996/10/26 16:48:13 robr Exp $";
#endif


#include <andrewos.h>
#include <simpletext.H>
#include <message.H>
#include <view.H>
#include <dataobject.H>
#include <rect.h>
#include <table.H>
#include <search.H>

#define AUXMODULE
#include <spread.H>

#include <shared.h>

/* support for nifty recursive search */

/* finds the first match, in a label or any child view, starting at pos. */
static boolean spread_RecSearchLoop(class spread *self, short posx, short posy, struct SearchPattern *pat)
{
    class table *d;
    short width, height;
    struct cell *curc;
    class view *v;
    char *ts;
    int substart;

    d = MyTable(self);
    height = d->NumberOfRows();
    width = d->NumberOfColumns();

    for (; posy<height; posy++, posx=0) {
	for (; posx<width; posx++) {
	    if (!d->IsJoinedToAnother(posy, posx)) {
		curc = d->GetCell(posy, posx);
		switch (curc->celltype) {
		    case table_TextCell:
			ts = curc->interior.TextCell.textstring;
			substart = search::MatchPatternStr((unsigned char *)ts, 0, strlen(ts), pat);
			if (substart>=0) {
			    self->recsearchchild = NULL;
			    self->recsearchposh = posx;
			    self->recsearchposv = posy;
			    self->recsearchvalid = TRUE;
			    self->recsearchsubstart = substart;
			    self->recsearchsublen = search::GetMatchLength();
			    return TRUE;
			}
			break;
		    case table_ImbeddedObject:
			v = spread_FindSubview(self, curc);
			if (v->RecSearch(pat, FALSE)) {
			    self->recsearchchild = v;
			    self->recsearchposh = posx;
			    self->recsearchposv = posy;
			    self->recsearchvalid = TRUE;
			    return TRUE;
			}
			break;
		    default:
			break;
		}
	    }
	}
    }

    self->recsearchvalid = FALSE;
    self->recsearchposh = 0;
    self->recsearchposv = 0;
    self->recsearchchild = NULL;
    return FALSE;
}


boolean spread::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    short posx, posy;

    /*this->InitChildren();*/

    if (!toplevel) {
	posx = 0;
	posy = 0;
    }
    else {
	posx = this->selection.LeftCol;
	posy = this->selection.TopRow;
    }

    return spread_RecSearchLoop(this, posx, posy, pat);
}

boolean spread::RecSrchResume(struct SearchPattern *pat)
{
    short posx, posy;

    if (!this->recsearchvalid)
	return FALSE;

    posx = this->recsearchposh;
    posy = this->recsearchposv;

    if (this->recsearchchild) {
	/* recsearchchild is the child that got the last success */
	if (this->recsearchchild->RecSrchResume(pat)) {
	    /* pos stays the same */
	    this->recsearchvalid = TRUE;
	    return TRUE;
	}

	/* ok, that child ran out of matches. Restart right after it. */
	posx++;
    }
    else {
	/* last success was a string. Restart right after it. Note that we only get one string match per cell for label cells. */
	posx++;
    }

    /* note, due to clever loop construction, that this works even if posx has gone past the end of the table. (It just restarts on the next line.) */
    return spread_RecSearchLoop(this, posx, posy, pat);
}

boolean spread::RecSrchReplace(class dataobject *srcdobj, long srcpos, long srclen)
{
    class table *d = MyTable(this);
    struct cell *cel;
    char *buf;
    char *ts, just;
    int substart, sublen;
    class simpletext *srctext;

    if (!this->recsearchvalid)
	return FALSE;

    if (this->recsearchchild) {
	/* hand it off to the child instead */
	return this->recsearchchild->RecSrchReplace(srcdobj, srcpos, srclen);
    }

    if (!srcdobj->IsType("simpletext")) {
	message::DisplayString(this, 0, "Replacement object is not text.");
	return FALSE;
    }
    srctext = (class simpletext *)srcdobj;

    cel = d->GetCell(this->recsearchposv, this->recsearchposh);
    if (cel->celltype != table_TextCell) {
	message::DisplayString(this, 0, "This cell is not text.");
	return FALSE;
    }

    ts = cel->interior.TextCell.textstring;
    just = (*ts);
    if (just == '\'' || just == '^' || just == '\"') {
	ts++;
	if (this->recsearchsubstart)
	    this->recsearchsubstart--;
    }
    else 
	just = '\'';
    substart = this->recsearchsubstart;
    sublen = this->recsearchsublen;
    buf = (char *)malloc(strlen(ts) - sublen + srclen + 3);
    buf[0] = just;
    strncpy(buf+1, ts, substart);
    srctext->CopySubString(srcpos, srclen, buf+1+substart, FALSE);
    strcpy(buf+1+substart+srclen, ts+substart+sublen);
    this->recsearchsublen = srclen;

    d->ParseCell(cel, buf);
    free(buf);

    return TRUE;
}
extern void spread_ComputeAnchorOffsets(class spread *V, struct rectangle *rect);

void spread::RecSrchExpose(const struct rectangle &logical, struct rectangle &hit) {
    /* ComputeSizes will do the LinkTree and InsertView on any children. */
    ComputeSizes();
    struct chunk chunk;
    
    chunk.TopRow = this->recsearchposv;
    chunk.LeftCol = this->recsearchposh;
    chunk.BotRow = chunk.TopRow;
    chunk.RightCol = chunk.LeftCol;

    SetCurrentCell (this, &chunk, FALSE);


    long c=this->recsearchposh;
    long r=this->recsearchposv;
    long kvo=vOffset, kho=hOffset;
    vOffset=hOffset=0;
    spread_ScrollLogically(this, r, c, r, c, (struct rectangle *)&logical);
    if(recsearchchild) {
	long rr, cc, xth, yth;
	struct rectangle childrect;
	for (rr = r + 1, yth = this->rowInfo[r].computedHeight - spread_SPACING - 2 * spread_CELLMARGIN;
	     MyTable(this)->IsJoinedAbove( rr, c);
	     NextY(this, rr, yth)) ;
	for (cc = c + 1, xth = colInfo[c].computedWidth - 2 * spread_CELLMARGIN;
	     MyTable(this)->IsJoinedToLeft( r, cc);
	     NextX(this, cc, xth)) ;
	rectangle_SetRectSize(&childrect, CtoX(this,c) +spread_SPACING+ spread_CELLMARGIN, RtoY(this,r) + spread_SPACING+ spread_CELLMARGIN, xth, yth);
	if(!recsearchchild->GetIM()) recsearchchild->LinkTree(this);
	recsearchchild->RecSrchExpose(childrect, hit);
    } else {
	hit.left=CtoX(this, c);
	hit.width=CtoX(this,c+1)-hit.left;
	hit.top=RtoY(this,r);
	hit.height=RtoY(this,r+1)-hit.top;
    }
    hit.left+=logical.left;
    hit.top+=logical.top;
    long k;
    struct rectangle res;
    rectangle_IntersectRect(&res, (struct rectangle *)&logical, &hit);
    if(!rectangle_IsEqualRect(&res,&hit)) {
	if ((k = rectangle_Right(&hit) + spread_SPACING - rectangle_Right(&logical)) > 0) {
	    hOffset += k;
	    hit.left-=k;
	    lastTime = -1;
	}
	if ((k = rectangle_Left(&hit) - spread_BORDER(this) - rectangle_Left(&logical)) < 0) {
	    hOffset += k;
	    lastTime = -1;
	}
	if ((k = CtoX(this, (MyTable(this))->NumberOfColumns()+1)+logical.left - rectangle_Right(&logical) ) < 0 && hOffset) {
	    hOffset += k;
	    hit.left-=k;
	    lastTime = -1;
	}

	if ((k = CtoX(this, 0) - spread_BORDER(this)) < 0) {
	    long l = rectangle_Right(&hit) + spread_SPACING;
	    if(rectangle_Right(&logical)-l>=-k) {
		hOffset += k;
		hit.left-=k;
		lastTime = -1;
	    }
	}
	if (hOffset < 0)
	    hOffset = 0;
	if ((k = rectangle_Bottom(&hit) + spread_SPACING - rectangle_Bottom(&logical)) > 0) {
	    vOffset += k;
	    hit.top-=k;
	    lastTime = -1;
	}
	if ((k = rectangle_Top(&hit) - spread_BORDER(this) -rectangle_Top(&logical)) < 0) {
	    vOffset += k;
	    hit.top-=k;
	    lastTime = -1;
	}
	if ((k = RtoY(this, (MyTable(this))->NumberOfRows()+1) +rectangle_Top(&logical)- rectangle_Bottom(&logical)) < 0 && vOffset) {
	    vOffset += k;
	    hit.top-=k;
	    lastTime = -1;
	}
	if (vOffset < 0)
	    vOffset = 0;
    }
    TellFormula(this);
    if(!recsearchchild) this->WantInputFocus(this);
    if(kho!=hOffset || kvo!=vOffset) lastTime = -1;
    this->WantUpdate(this);
}

