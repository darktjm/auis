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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/textobjects/RCS/chlistview.C,v 1.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif

ATK_IMPL("chlistview.H")
#include "text.H"
#include "view.H"
#include "chlist.H"
#include "style.H"
#include "chlistview.H"

static boolean chlistview_Debug = FALSE;



ATKdefineRegistry(chlistview, textview, NULL);
#ifndef NORCSID
#endif


void chlistview::SetUpdateRegion(long  pos, long  len)
{
    class textview *tv = (class textview *) this;
    long i;

    for (i = 0; i < tv->nLines; i++) {
	if (pos < (tv->lines[i].data)->GetEndPos() && (pos + len) >= (tv->lines[i].data)->GetPos()) {
	    (tv->lines[i].data)->SetModified( TRUE);
	}
    }
    if (pos < (tv->predrawn)->GetEndPos() && (pos + len) >= (tv->predrawn)->GetPos()) {
	(tv->predrawn)->SetModified( TRUE);
    }
    if (pos < (tv->prepredrawn)->GetEndPos() && (pos + len) >= (tv->prepredrawn)->GetPos()) {
	(tv->prepredrawn)->SetModified( TRUE);
    }

    (this)->WantUpdate( this);
}


chlistview::chlistview()
{
    this->highlightedItem = -1;
    this->numStylesAllocated = 0;
    this->normalStyles = NULL;
    this->highlightedStyles = NULL;
    THROWONFAILURE((TRUE));
}

chlistview::~chlistview()
{
    if (this->normalStyles != NULL) {
	free(this->normalStyles);
    }
    if (this->highlightedStyles != NULL) {
	free(this->highlightedStyles);
    }
}

class view *chlistview::Hit(enum view_MouseAction  action, long  x , long  y , long  nclicks)
{
    long pos;
    long index, region;
    class chlist *l;

    (this)->textview::Hit( action, x, y, nclicks);
    l = (class chlist *) (this)->GetDataObject();
    pos = (this)->GetDotPosition();
    index = (l)->GetIndexByPosition( pos, &region, NULL, NULL);
    if (index >= 0) {
	(this)->HighlightItem( index);
	if (l->ItemList[index].proc) {
	    (l->ItemList[index].proc)(l->ItemList[index].rock, l, action, nclicks, index, region);
	}
    }
    return((class view *) this);
}

void chlistview::ActivateItem(int  pos)
{
    long region, index;
    class chlist *l;

    l = (class chlist *) (this)->GetDataObject();
    index = (l)->GetIndexByPosition( pos, &region, NULL, NULL);
    if (index >= 0) {
	(this)->HighlightItem( index);
	if (l->ItemList[index].proc) {
	    (l->ItemList[index].proc)(l->ItemList[index].rock, l, view_LeftDown, 1, index, region);
	}
    }
   (this)->WantInputFocus( this);
}

void chlistview::HighlightItem(long  index)
{
    if (this->highlightedItem != index) {
	class chlist *l;
	long len;
	struct listitem *item, *nextItem;

	(this)->UnhighlightItem( this->highlightedItem);

	if (index >= 0) {
	    l = (class chlist *) (this)->GetDataObject();
	    item = (l)->FindItemByIndex( index);
	    if (index == ((l)->GetNumItems() - 1)) {
		len = (l)->GetLength();
	    }
	    else {
		nextItem = (l)->FindItemByIndex( index + 1);
		len = nextItem->loc;
	    }
	    len -= (item->loc + 1);

	    (this)->SetUpdateRegion( item->loc, len);
	    (this)->WantUpdate( this);
	}

	this->highlightedItem = index;
    }
}

void chlistview::UnhighlightItem(long  index)
{
    if (this->highlightedItem >= 0) {
	class chlist *l;
	long len;
	struct listitem *item, *nextItem;

	l = (class chlist *) (this)->GetDataObject();
	item = (l)->FindItemByIndex( this->highlightedItem);
	nextItem = (l)->FindItemByIndex( this->highlightedItem + 1);

	len = (this->highlightedItem == (l)->GetNumItems() - 1) ? (l)->GetLength() : nextItem->loc;
	len -= (item->loc + 1);

	(this)->SetUpdateRegion( item->loc, len);

	this->highlightedItem = -1;
	(this)->WantUpdate( this);
    }
}

class environment *chlistview::GetStyleInformation(struct text_statevector  *sv, long  pos, long  *length)
{
    class environment *env;
    long index, regionID, size, offset;
    class style *style;
    class chlist *l;

    l = (class chlist *) (this)->GetDataObject();

    env = (this)->textview::GetStyleInformation( sv, pos, length);
    index = (l)->GetIndexByPosition( pos, &regionID, &size, &offset);

    if (chlistview_Debug) {
	long len = (length == NULL) ? -1 : *length;

	printf("pos: %d length: %d index: %d regionID: %d size: %d offset: %d\n", pos, len, index, regionID, size, offset);
    }

    if (index >= 0 && regionID >= 0) {
	style = (this)->GetRegionStyle( regionID, FALSE);
	if (style != NULL) {
	    text::ApplyEnvironment(sv, style, NULL);
	}
	if (index == this->highlightedItem) {
	    style = (this)->GetRegionStyle( regionID, TRUE);
	    if (style != NULL) {
		text::ApplyEnvironment(sv, style, NULL);
	    }
	}
	if (length != NULL && (size - offset) < *length) {
	    *length = size - offset;
	}
    }

    return env;
}

class style *chlistview::GetRegionStyle(long  regionID, boolean  highlighted)
{
    if (this->normalStyles != NULL) {
	if (highlighted) {
	    return this->highlightedStyles[regionID];
	}
	else {
	    return this->normalStyles[regionID];
	}
    }
    return NULL;
}

void chlistview::SetRegionStyles(long  regionID, class style  *normalStyle, class style  *highlightStyle)
{
    long oldSize;

    if (regionID >= (oldSize = this->numStylesAllocated)) {
	this->numStylesAllocated = regionID + 4;
	if (this->normalStyles == NULL) {
	    this->normalStyles = (class style **) calloc(this->numStylesAllocated, sizeof(class style *));
	    this->highlightedStyles = (class style **) calloc(this->numStylesAllocated, sizeof(class style *));
	}
	else {
	    this->normalStyles = (class style **) realloc(this->normalStyles, this->numStylesAllocated * sizeof(class style *));
	    this->highlightedStyles = (class style **) realloc(this->highlightedStyles, this->numStylesAllocated * sizeof(class style *));
	    while (oldSize < this->numStylesAllocated) {
		this->normalStyles[oldSize] = NULL;
		this->highlightedStyles[oldSize] = NULL;
		oldSize++;
	    }
	}
    }

    this->normalStyles[regionID] = normalStyle;
    this->highlightedStyles[regionID] = highlightStyle;
}
	    
		
