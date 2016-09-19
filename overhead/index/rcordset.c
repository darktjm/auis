/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h> /* sys/types.h sys/file.h */
#include <stdio.h>

#include "index.h"

/*
 * Internal routine: return a new, empty hash list.  Empty hash lists are not represented
 * by null pointers, but rather a hash list with a zero entry count.
 */
struct hashList *index_NewHL(void)
{
    struct hashList *th;
    th = (struct hashList *) malloc (sizeof (struct hashList));
    th->next = (struct hashList *) 0;
    th->nentries = 0;
    return th;
}

/*
  * Internal routine: given a hash list and a hash value, return true if the value is
      * contained in the list, and false otherwise.
      */
int index_HashPresent(struct hashList *alist, long ahash)
{
    int i;
    for(;alist;alist=alist->next) {
	for(i=0;i<alist->nentries;i++) {
	    if (ahash == alist->entries[i]) return 1;
	}
    }
    return 0;
}

/*
  * Internal routine: given a hash list and a hash value, delete the value from
  * the hash list.
  */
int index_HashRemove(struct hashList *alist, long ahash)
{
    int i;
    for(;alist;alist=alist->next) {
	for(i=0;i<alist->nentries;i++) {
	    if (ahash == alist->entries[i]) alist->entries[i] = 0xffff;
	}
    }
    return 0;
}

/*
  * Internal routine: given a hash list and a hash value, add the value to the list.
  */
void index_HashAdd(struct hashList *alist, long ahash)
{
    struct hashList *tlist;
    for(tlist=alist;tlist;tlist=tlist->next) {
	if (tlist->nentries < MAXHL) {
	    tlist->entries[tlist->nentries++] = ahash;
	    return;
	}
    }
    tlist = (struct hashList *) malloc(sizeof(struct hashList *));
    tlist->nentries = 1;
    tlist->next = alist->next;
    alist->next = tlist;
    tlist->entries[0] = ahash;
}

/*
  * Internal routine: given a hash list, free it.
  */
void index_FreeHL(struct hashList *alist)
{
    struct hashList *next;
    for(;alist;alist=next) {
	next = alist->next;
	free(alist);
    }
}

/*
  * Create a new, empty record set.
  */
struct recordSet *recordset_New(int asize)
{
    struct recordSet *tr;
    if (asize <= 0) asize = 1;
    tr = (struct recordSet *) malloc(sizeof(struct recordSet));
    tr->count = 0;
    tr->acount = asize;
    tr->data = (struct recordID *) malloc(asize * sizeof(struct recordID));
    return tr;
}

/*
  * Free a record set.
  */
void recordset_Free(struct recordSet *aset)
{
    free(aset->data);
    free(aset);
}

/*
  * Add a record id to a record set.
  */
void recordset_Add(struct recordSet *aset, struct recordID *arid)
{
    long c;
    struct recordID *tid;
    long newSize;
    for(tid=aset->data,c=0; c<aset->count; tid++,c++) {
	if (req(*tid, *arid)) return;
    }
    c = aset->count;
    if (c>= aset->acount) {
	newSize = 2*c;
	aset->data = (struct recordID *) realloc(aset->data, newSize*sizeof(struct recordID));
	aset->acount = newSize;
    }
    rset(aset->data[c], *arid);
    aset->count = c+1;
}

/* more recordset operations to come as they become necessary */
