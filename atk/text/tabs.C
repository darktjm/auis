/* ********************************************************************** *\
 *         Copyright MIT 1990 - All Rights Reserved      *
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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/tabs.C,v 3.5 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 
ATK_IMPL("tabs.H")

#include <style.H>
#include <textview.H>
#include <txtstvec.h>

#include <tabs.H>

static class tabs *DefaultTabs = NULL;


ATKdefineRegistry(tabs, ATK, NULL);
int FindPrevTab(class tabs  *tabs, long  pos);


int
FindPrevTab(class tabs  *tabs, long  pos)
/* Post: returns...
 *	-1 if no tabs
 *	n if found
 *	NumTabs if not found in list
 */
{
    register int i;

    if (tabs->number == 0)
        return -1;

    for(i = 0; i < tabs->number; i++)
        if (pos < tabs->Positions[i])
            return i - 1;

    return tabs->number;
}

/* This is to take account of Andrew lying about the size of fonts */
#define	RealityHack(x) ((x)*14)


void
tabs::OutputTroff(long  indent, FILE  *file)
/* Output all tabs past indent, with a tab at indent */
{
    int i;

    if (indent < 0) {
	fprintf(file, "'ta %dp", -indent);
    }
    else{
	fprintf(file, "'ta");
    }

    for (i = 0; i < this->number; i++)
	if (this->Positions[i] > indent)
	    switch(this->Types[i]) {
		case style_LeftAligned:
		    fprintf(file, " %dp", this->Positions[i] - indent);
		    break;
		case style_RightAligned:
		    fprintf(file, " %dpR", this->Positions[i] - indent);
		    break;
		case style_CenteredOnTab:
		    fprintf(file, " %dpC", this->Positions[i] - indent);
		    break;
		default:
		    ; /* Ignoring all others for now... XXX */
	    }
    fprintf(file, "\n");
}


int
tabs::Different(class tabs  *b)
/* returns 1 if different, 0 if same */
{
    if (this->number == b->number) {
	int i;
	for (i = 0; i < this->number; i++)
	    if (!(this->Positions[i] == b->Positions[i] &&
		  this->Types[i] == b->Types[i]))
		return 1;
    } else
	return 1;
    return 0;
}


class tabs *
tabs::Delete(int  n)
/* delete the n'th tab */
/* precondition: n represents a valid tab: 0 <= n < CurNumTabs */
/* post: the original tabs are destroyed, if links > 1 */
{
    register class tabs *nt;

    register int num = this->number;

    if (num == 1) {
	/* Last tab in this list */
	/* Before we throw away the list, is it someone elses? */
	if (this->links > 1) {
	    nt = new tabs;
	    nt->links = 1;
	    this->links--;
	    return nt;
	} else {
	    free(this->Positions);
	    free(this->Types);
	    this->Positions = NULL;
	    this->Types = NULL;
	    this->number = 0;
	    return this;
	}
    } else {
	/* create new tablist..
	 * copy tabs from 0..n-1 into newlist[0..n-1] (That's n elements);
	 * copy	tabs from n+1..NumTabs into newlist[n..NumTabs-1]   
	 * free	the old	tab lists   
	 * dec(NumTabs)	
	 */ 
	nt = new tabs;
	nt->Positions = (long *) malloc(sizeof(long) * (num-1));
	nt->Types = (long *) malloc(sizeof(long) * (num-1));
    	if (n) {
	    memmove(nt->Positions, this->Positions, sizeof(long) * n);
	    memmove(nt->Types, this->Types, sizeof(long) * n);
	}
	if (n != num - 1) {
	    memmove(&(nt->Positions[n]), &(this->Positions[n+1]), sizeof(long) * (num-n-1));
	    memmove(&(nt->Types[n]), &(this->Types[n+1]), sizeof(long) * (num-n-1));
	}

	tabs::Death(this);

	nt->links = 1;
	nt->number = num-1;
	return nt;
    }
}


class tabs *
tabs::Add(long  pos, enum style_TabAlignment  op)
{
    /* Add tab into the list */
    /* The original lists ARE DESTROYED */
    register class tabs *nt;
    register int PrevTab;
    register int num = this->number;

    /* Find out what tab is before the destination spot */
    PrevTab = FindPrevTab(this, pos);
    /* If there is already a tab at pos, then let's just overwrite it */
    if (PrevTab >= 0 && this->Positions[PrevTab] == pos)
	/* There is one at this spot */ {
	if (this->Types[PrevTab] == (long) op) {
	    /* But no change... */
	    return this;
	}

	if (this->links > 1) { /* But someone else is using this list, so we need to copy */
	    nt = new tabs;
	    nt->Positions = (long *) malloc(sizeof(long) * num);
	    nt->Types = (long *) malloc(sizeof(long) * num);
	    memmove(nt->Positions, this->Positions, sizeof(long)*num);
	    memmove(nt->Types, this->Types, sizeof(long)*num);
	    nt->Types[PrevTab] = (long) op;
	    this->links--;
	    nt->links = 1;
	    nt->number = num;
	    return nt;
	} else {
	    this->Types[PrevTab] = (long) op;
	    return this;
	}
    } else {
	/* newpos is where in the array to place the new tabstop - it
	 * is similar to PrevTab, but takes into account the case
	 * where PrevTab indicates the new pos is outside the old array bounds
	 */
	register int newpos = PrevTab;
	if (newpos < num)
	    newpos++;

	/* malloc new arrays */
	nt = new tabs;
	nt->Positions = (long *) malloc(sizeof(long) * (num+1));
	nt->Types = (long *) malloc(sizeof(long) * (num+1));

	/* Copy all the old tabs before this new one */
	if (PrevTab >= 0) {
	    memmove(nt->Positions, this->Positions, sizeof(long) * (newpos));
	    memmove(nt->Types, this->Types, sizeof(long) * (newpos));
	}

	/* Put in the new one */
	nt->Positions[newpos] = pos;
	nt->Types[newpos] = (long) op;
	
	/* Put in all the old tabs after the new one */
	if (PrevTab != num) {
	    memmove(&(nt->Positions[newpos+1]), &(this->Positions[newpos]), sizeof(long) * (num-newpos));
	    memmove(&(nt->Types[newpos+1]), &(this->Types[newpos]), sizeof(long) * (num-newpos));
	}
	/* And get rid off all the old tabs */
	tabs::Death(this);
	nt->number = num+1;
	nt->links = 1;
	return nt;
    }
}

class tabs *
tabs::Clear()
/* 
 * Post: if links == 1, then list is destroyed, else links is kept
 */
{
    if (this->number == 0)
	/* Tabs are already cleared */
	return this;

    if (this->links == 1) {
	free(this->Positions);
	free(this->Types);
	this->number = 0;
	this->Positions = NULL;
	this->Types = NULL;
	return this;
    } else {
	class tabs *nt = new tabs;
	nt->links = 1;
	return nt;
    }
}


class tabs *
tabs::Create()
{
    register long x;
    register int i;
    /*
     * We are using:
     * Default Tabs, every 1/2 inch, for the first 10inches;
     * equiv: every 36 pts, from 36 .. 720
     */
    
    if (DefaultTabs) {
	DefaultTabs->links++;
	return DefaultTabs;
    }

    DefaultTabs = new tabs;
    DefaultTabs->links = 2;
    DefaultTabs->Positions = (long *) malloc(sizeof(long)*19);
    DefaultTabs->Types = (long *) malloc(sizeof(long)*19);

    for (x = 36,i=0; x < 720; x+=36, i++) {
	DefaultTabs->Positions[i] = x;
	DefaultTabs->Types[i] = (long) style_LeftAligned;
    }
    DefaultTabs->number = i;
    
    return DefaultTabs;
}

tabs::tabs()
{
    this->Positions = NULL;
    this->Types = NULL;
    this->links = 0;
    this->number = 0;
    THROWONFAILURE( TRUE);
}

tabs::~tabs()
{
    if (this->Positions) free(this->Positions);
    if (this->Types) free(this->Types);
    this->Positions=NULL;
    this->Types=NULL;
}

void
tabs::Death(class tabs  *self)
{
    if (--self->links > 0)
	return;
    else if(self==DefaultTabs) {
	fprintf(stderr, "tabs: WARNING: would have destroyed default tabs object!\n");
#ifdef DEBUG
	abort();
#endif	
	self->links=1;
	return;
    } else delete self; 
}


class tabs *
tabs::ApplyChange(struct tabentry  *tabChange)
{
    long Pos;	    /* Position for current tab */
    int PrevTab;   /* Tab location at or immediately before proposed tab */
    class tabs *tabs = this;

    Pos = tabChange->DotLocation;
    /* DotLocation is in the units style_RawDots... (calculated by style.c)
     */

    switch(tabChange->TabOpcode) {
        case style_LeftAligned:
        case style_RightAligned:
        case style_CenteredOnTab:
        case style_CenteredBetweenTab:
	case style_CharAligned:
	    tabs = (this)->Add( Pos, tabChange->TabOpcode);
	    break;
        case style_TabDivide:
	    /* First get rid of old tabs */
	    tabs = (this)->Clear();
	    /* Calculate where new tab stops should go */
	    /* XXX - what is TabDivide supposed to *DO* ???!! */
	    break;
        case style_TabClear:
	    PrevTab = FindPrevTab(this, Pos);
	    if (PrevTab >= 0 && PrevTab < number && tabChange->DotLocation == this->Positions[PrevTab])
		/* There is a tab at this postion, so we get rid of it */
		tabs = (this)->Delete( PrevTab);
	    /* If there is no tab at the precise position specified, then
	     * this operation is ignored. Perhaps it should find another
	     * tab within a certain tolerance?
	     */
	    break;
	case style_AllClear:
	    tabs = (this)->Clear();
	    break;
	default:
	    /* Unknown -- should return error */
	    return tabs;
    }
    return tabs;
}

class tabs *
tabs::Copy()
{
    this->links++;
    return this;
}
