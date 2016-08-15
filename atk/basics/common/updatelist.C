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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/updatelist.C,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 


ATK_IMPL("updatelist.H")
#include <updatelist.H>
#include <view.H>

static struct updateitem *freeList = NULL;


ATKdefineRegistry(updatelist, ATK, NULL);
#ifndef NORCSID
#endif


updatelist::updatelist()
        {
    this->updates = NULL;
    THROWONFAILURE( TRUE);
}

updatelist::~updatelist()
        {

    struct updateitem *item;

    if (this->updates != NULL) {
        for (item = this->updates; item->next != NULL; item = item->next)
            ;
        item->next = freeList;
        freeList = this->updates;
    }
}

void updatelist::AddTo(class view  *view)
        {
  /* modified to 1) add the view to the end of the list, and 2) remove earlier occurrence of the view from the list */

    register struct updateitem *newui;
    register struct updateitem *item, *last;

    /* if view appears on list already, remove it */
    for (item = this->updates, last = NULL;  item != NULL;  last = item, item = item->next)
      {
	if (item->view == view)
	  {
	    if (last == NULL)
	      this->updates = item->next;
	    else
	      last->next = item->next;
	    if (item->next != NULL)
	      last = item->next;	/* save that ptr for a moment... */
	    item->view = NULL;
	    item->next = freeList;
	    freeList = item;

	    /* now run to the end of the list using saved pointer */
	    while (last != NULL && last->next != NULL)
	      last = last->next;
	    break;
	  }
      }

    if (freeList != NULL)  {
	newui = freeList;
	freeList = freeList->next;
    }
    else  {
	newui = (struct updateitem *) malloc(sizeof(struct updateitem));
    }
    
    newui->view = view;
    newui->next = NULL;
    if (last == NULL)
      this->updates = newui;
    else
      last->next = newui;
}

void updatelist::DeleteTree(class view  *tree)
        {
    struct updateitem *item;
    struct updateitem *next;
    struct updateitem **previous;

    previous = &this->updates;
    for (item = this->updates; item != NULL; item = next) {
        next = item->next;
	if ((item->view)->IsAncestor( tree)) {
            *previous = item->next;
            item->next = freeList;
            freeList = item;
        }
        else
            previous = &item->next;
    }
}


void updatelist::Clear()
    {
    struct updateitem *item;
    struct updateitem *lastitem = NULL;
    struct updateitem *pendingUpdates;

    pendingUpdates = this->updates;
    this->updates = NULL;
    for (item = pendingUpdates; item != NULL ; item = item->next) {
        (item->view)->Update();
        lastitem = item;
    }
    if (lastitem != NULL) {
        lastitem->next = freeList;
        freeList = pendingUpdates;
    }
}
