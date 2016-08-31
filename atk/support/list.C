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
 * Generic Linked List / Stack / Queue datatype
 *
 * Although char *'s are used for data, pointers to any type
 * of object or structure may be used.  Minimal casting may
 * be required by some compilers.
 */

#include <andrewos.h>
ATK_IMPL("list.H")
#include "list.H"

#define new_c() \
  (struct list_Entry *) malloc(sizeof (struct list_Entry))

/*
 * Class procedures
 */


ATKdefineRegistry(list, ATK, NULL);
static int CopyEntry(char  *value, class list  *dst);
static boolean MoveNew(char  *data, struct arg  *ap);
static int rcompare(char  **d1, char  **d2);


list::list()
{
    this->head = NULL;
    this->tail = NULL;
    this->size = 0;

    THROWONFAILURE( TRUE);
}

list::~list()
{
    (this)->Clear();
}

static int CopyEntry(char  *value, class list  *dst)
{
    (dst)->InsertEnd( value);

    return TRUE;
}

void list::Merge(class list  *dst , class list  *src)
{
    (src)->Enumerate( (list_efptr)CopyEntry, (char *) dst);
}

void list::Append(class list  *dst , class list  *src)
{
    if (dst->size == 0) {
        dst->head = src->head;
        dst->tail = src->tail;
        dst->size = src->size;
    } else {
        dst->tail->next = src->head;
        dst->size += src->size;
    }

    src->head = NULL;
    src->tail = NULL;
    src->size = 0;
}

/*
 * Methods
 */

void list::InsertFront(char  *data)
{
    register struct list_Entry *p;

    p = new_c();
    p->data = data;
    p->next = this->head;
    this->head = p;
    this->size++;
}

void list::InsertEnd(char  *data)
{
    register struct list_Entry *p;

    p = new_c();
    p->data = data;
    p->next = NULL;

    if (this->size == 0)
        this->head = p;
    else
        this->tail->next = p;
    this->tail = p;
    this->size++;
}

boolean list::InsertUnique(char  *data)
{
    if ((this)->Member( data))
        return FALSE;

    (this)->InsertEnd( data);
    return TRUE;
}

/* Compare takes two arguments, data1 and data2. */
/* It should return a positive number if data1 is greater than */
/* data2, a negative number if data1 is less than data2, */
/* or zero if they are equal. */

void list::InsertSorted(char  *data, list_greaterfptr  compare)
{
    register struct list_Entry *n, *p, **pp;

    n = new_c();
    n->data = data;

    pp = &this->head;
    for (p = *pp; p != NULL &&
      (*compare)(data, p->data) > 0; p = *pp)
        pp = &p->next;

    if (p == NULL)
        this->tail = n;

    n->next = *pp;
    *pp = n;

    this->size++;
}

char *list::RemoveFront()
{
    register struct list_Entry *p = this->head;
    register char *data;

    if (this->size == 0)
	return NULL;

    if (--this->size == 0)
        this->head = this->tail = NULL;
    else
        this->head = p->next;

    data = p->data;
    free(p);
    return data;
}

/*
 * Find and delete
 */

boolean list::Delete(char  *data)
{
    register struct list_Entry *p, **pp, *lp=NULL;

    pp = &this->head;
    for (p = *pp; p != NULL && p->data != data; lp=p, p = *pp)
        pp = &p->next;

    if (p == NULL)
        return FALSE;

    if (this->tail == p)
        this->tail = lp;

    *pp = p->next;
    free(p);

    this->size--;
    return TRUE;
}

boolean list::Member(char  *data)
{
    register struct list_Entry *p;

    for (p = this->head; p != NULL; p = p->next)
        if (p->data == data)
              return TRUE;

    return FALSE;
}

/*
 * Simple sort, using qsort, wind out this list into
 * an array, sort it and make the list again.
 * Compare routine is same as that taken by InsertSorted.
 */

struct arg { char **list; int ind;};

static boolean MoveNew(char  *data, struct arg  *ap)
{
    ap->list[ap->ind]=data;
    ap->ind++;
    return TRUE;
}

static list_greaterfptr tcompare=NULL;

static int rcompare(char  **d1, char  **d2)
{
    list_greaterfptr lcompare=tcompare;
    int result=tcompare?tcompare(*d1, *d2):0;
    tcompare=lcompare;
    return result;
}

boolean list::Sort(list_greaterfptr  compare)
{
    struct arg a;
    int i;
    /* Create new list, inserting old elements in sorted order */

    if(this->size<=0) return FALSE;
    
    a.list = (char **)malloc(sizeof(char *)*this->size);
    tcompare = compare;
    a.ind=0;

    if(a.list == NULL) return FALSE;
    
    (this)->Enumerate( (list_efptr)MoveNew, (char *) &a);

    qsort(a.list, this->size, sizeof(char *), QSORTFUNC(rcompare));

    (this)->Clear();

    for(i=0;i<a.ind;i++) (this)->InsertEnd( a.list[i]);

    free(a.list);
    
    return TRUE;
}

/*
 * Enumerate runs proc on each entry in a list.
 * If proc returns FALSE, the enumeration terminates
 * and the piece of data responsible is returned.
 * Otherwise, the enumeration completes and NULL is returned.
 */

char *list::Enumerate(list_efptr  proc, const char  *rock)
{
    register struct list_Entry *p;

    for (p = this->head; p != NULL; p = p->next)
        if ((*proc)(p->data, (char *)rock) == FALSE)
            return p->data;

    return NULL;
}

void list::Clear()
{
    register struct list_Entry *p, *n;

    for (p = this->head; p != NULL; p = n) {
	n = p->next;
        free(p);
    }

    this->head = NULL;
    this->tail = NULL;
    this->size = 0;
}
