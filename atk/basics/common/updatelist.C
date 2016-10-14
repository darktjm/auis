/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("updatelist.H")
#include <updatelist.H>
#include <view.H>

static struct updateitem *freeList = NULL;


ATKdefineRegistryNoInit(updatelist, ATK);

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

    struct updateitem *newui;
    struct updateitem *item, *last;

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
