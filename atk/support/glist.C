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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/support/RCS/glist.C,v 3.4 1995/05/05 16:51:43 rr2b Stab74 $";
#endif


 

/* List object
 * for Graph Editor
 */



#include <andrewos.h>
ATK_IMPL("glist.H")
#include <glist.H>

#define newelt() (struct glistelt *) malloc(sizeof(struct glistelt))


ATKdefineRegistry(glist, ATK, NULL);
#ifndef NORCSID
#endif
static int copyElement(char  *value,class glist  *dest);
static int MoveNew(char  *listelt, struct glist_SortStruct  *ss);


glist::glist()

{
    this->head = this->tail = NULL;
    this->size = 0;
    this->DestroyProc = NULL;
    THROWONFAILURE( TRUE);
}

class glist *glist::Create(glist_destroyfptr  Destroy)
{
    class glist *list = new glist;
    list->DestroyProc = Destroy;
    return list;
}

static int copyElement(char  *value,class glist  *dest)
{
    (dest)->Insert(value);
    return FALSE;
}


void glist::Copy(class glist  *dest,class glist  *source)
{
    (source)->Find((glist_findfptr)copyElement,(char *)dest);
}

void glist::Clear(boolean  destroy)
{
    struct glistelt *item = this->head, *next;

    while(item) {
	next = item->next;
	if (destroy && this->DestroyProc)
	    (*this->DestroyProc)(item->this_c);
        free(item);
        item = next;
    }

    this->head = this->tail = NULL;
    this->size = 0;
}

glist::~glist()
{
    struct glistelt *item = this->head, *next;

    while(item) {
	next = item->next;
	if (this->DestroyProc != NULL)
	    (*this->DestroyProc)(item->this_c);
        free(item);
        item = next;
    }
    this->head = this->tail = NULL;
    this->size = 0;

}


char * glist::Find(glist_findfptr  filter,char  * rock)
{
    char *rvalue;
    struct glistelt *item = this->head;

    while(item) {
        if ((*filter)(item->this_c,rock)) {
              rvalue = item->this_c;
              return rvalue;
          }
        else
            item = item->next;
    }
    return NULL;
}

/* push - inserts element at head of list
 *
 */

boolean glist::Push(char  * element)
{
    struct glistelt *temp = newelt();

    temp->position=0;
    temp->next = this->head;
    temp->this_c = element;
    this->head = temp;
    if (this->size == 0)
      this->tail = temp;
    ++(this->size);
    return TRUE;
}

/* insert - inserts element at tail of list
 *
 */

boolean glist::Insert(char  *element, int eposition)
{
    struct glistelt *temp = newelt();

    temp->position=eposition;
    temp->this_c = element;
    temp->next = NULL;

    if (this->size == 0) {
        this->head = this->tail = temp;
    }
    else {
        this->tail->next = temp;
        this->tail = temp;
    }
    ++(this->size);
    return TRUE;
}

/* pop - removes first element from list
 *
 */

char * glist::Pop()
{
    char *rvalue;

    if (this->size == 0)
	return NULL;
    else {
	rvalue = this->head->this_c;
	if (this->size == 1) {
	    free(this->head);
	    this->head = this->tail = NULL;
	}
	else {
	    struct glistelt *temp = this->head;
	    this->head = this->head->next;
	    free(temp);
	}
	--(this->size);
	return rvalue;
    }
}

/***********************************************************************/

boolean glist::InsertSorted(char  * element,glist_greaterfptr  greater /* greater(element_1,element_2) */, int eposition)

{
    struct glistelt *temp = newelt();

    temp->position = eposition;
    temp->this_c = element;

    if (this->size == 0) {
        temp->next = NULL;
        this->head = this->tail = temp;
    }
    else {
        struct glistelt *i = this->head, *prev = this->head;

        /* move to the correct place in the list */
	while( (i != NULL)) {
	    int result=0;
	    if(greater) {
		  result=(*greater)(element,i->this_c);
		  if(result<0) break;
	    }
	    if(result==0 && eposition<=i->position) break;
            prev = i;
            i = i->next;
        }

        /* insert the element */
        if (i == prev) { /* at head of list */
            temp->next = this->head;
            this->head = temp;
        }
        else {
            if (i == NULL) { /* at tail of list */
                this->tail = temp;
            }
            prev->next = temp;
            temp->next = i;
        }
    }
    ++(this->size);
    return TRUE;
}


/* Insert an element only if it is not contained
 * (only tests pointers for equality)
 *
 */

boolean glist::InsertUnique(char  * element, int eposition)
{
    if ((this)->Contains(element))
        return FALSE;
    else
        return (this)->Insert(element, eposition);
}


/***********************************************************************/

struct glist_SortStruct {
    class glist *newlist;
    glist_greaterfptr compare;
};

static int MoveNew(char  *listelt, struct glist_SortStruct  *ss)
{
    (ss->newlist)->InsertSorted(listelt,ss->compare);
    return FALSE;
}

boolean glist::Sort(glist_greaterfptr  compare)
{
    class glist *temp = new glist;
    struct glist_SortStruct ss;
    struct glistelt *temphead,*temptail;
    int tempsize;

    /* create new list, inserting old list elements in sorted order */
    ss.newlist = temp;
    ss.compare = compare;
    (this)->Find((glist_findfptr)MoveNew,(char *)&ss);
    temphead = this->head;
    temptail = this->tail;
    tempsize = this->size;

    /* swap new and old lists */
    this->head = temp->head;
    this->tail = temp->tail;
    this->size = temp->size;
    temp->head = temphead;
    temp->tail = temptail;
    temp->size = tempsize;

    /* destroy old list */
    delete temp;
    return TRUE;
}

/***********************************************************************/


boolean glist::Delete(char  * element,boolean  destroy)
{
    struct glistelt *item = this->head;
    struct glistelt *prev = NULL;

    while (item) {
        if (item->this_c == element) {
            if (prev == NULL) {
                this->head = item->next;
		if (destroy && this->DestroyProc)
		    (*this->DestroyProc)(item->this_c);
		free(item);
                --(this->size);
                return TRUE;
            }
            else if (this->tail == item) {
                prev->next = NULL;
		if (destroy && this->DestroyProc)
		    (*this->DestroyProc)(item->this_c);
                free(item);
                --(this->size);
                this->tail = prev;
                return TRUE;
            }
            else {
                prev->next = item->next;
		if (destroy && this->DestroyProc)
		    (*this->DestroyProc)(item->this_c);
                free(item);
                --(this->size);
                return TRUE;
            }
        }
        else {
            prev = item;
            item = item->next;
        }
    }
    return FALSE;
}

/* check to see if a list contains a given element
 * (only checks pointer value)
 *
 */

boolean glist::Contains(char  * element)
{
    struct glistelt *item = this->head;

    while(item) {
        if (item->this_c == element)
              return TRUE;
        else
            item = item->next;
    }
    return FALSE;
}

void glist::Enumerate(glist_efptr proc, unsigned long  rock)
{
  struct glistelt *elt;

  if(proc == NULL) return;
  for(elt = this->head; elt != NULL; elt = elt->next)
    (*proc)(elt->this_c,rock);
}