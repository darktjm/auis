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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/eli/lib/RCS/bucket.c,v 2.8 1993/06/30 21:59:00 rr2b Stab74 $";
#endif

#include  <bucket.h>

/* Initialize a bucket */

void            eliBucket_Init(b)
eliBucket_t    *b;
{
    b->head = b->tail = NULL;
}

/* Insert a bucketnode into the given bucket (insertion at HEAD) */

void            eliBucket_Insert(b, bnode)
eliBucket_t    *b;
eliBucketNode_t *bnode;
{
    eliBucketNode_SetPrev(bnode, NULL);
    if (!(b->head)) {		/* Empty bucket */
	eliBucketNode_SetNext(bnode, NULL);
	b->head = b->tail = bnode;
    }
    else {
	eliBucketNode_SetPrev(b->head, bnode);	/* Prev points toward head */
	eliBucketNode_SetNext(bnode, b->head);	/* Next points toward tail */
	b->head = bnode;
    }
    eliBucketNode_IncrRefcount(bnode);	/* Should be its only reference */
}

/* Auxiliary function -- finds a bucketnode with a key & returns it */

eliBucketNode_t *eliBucket_Find_aux(b, key)
eliBucket_t    *b;
char           *key;
{
    eliBucketNode_t *tmp = b->head;
    int             found = FALSE;

    while (!found) {
	if (!tmp)
	    found = TRUE;	/* End search with failure */
	else if (!(found = !strcmp(key, EliStr_GetString(eliBucketNode_GetKey(tmp)))))
	    tmp = eliBucketNode_GetNext(tmp);
    }
    return (tmp);
}

/* Traverse bucket, find bucketnode with given key, return bucketnode's datum */

EliSexp_t      *eliBucket_Find(b, key)
eliBucket_t    *b;
char           *key;
{
    eliBucketNode_t *tmp;

    return ((tmp = eliBucket_Find_aux(b, key)) ? eliBucketNode_GetSexp(tmp) : NULL);
}

/*
 * Delete bucketnode with given key 
 */

void            eliBucket_Delete(st, b, key)
EliState_t     *st;
eliBucket_t    *b;
char           *key;
{
    eliBucketNode_t *tmp3, *tmp2, *tmp;

    if (tmp = eliBucket_Find_aux(b, key)) {
	if (b->head == tmp) {
	    if (b->tail == tmp)	/* Single element bucket */
		b->head = b->tail = NULL;
	    else {		/* Mult. elements, tmp at head */
		eliBucketNode_SetPrev(tmp2 = eliBucketNode_GetNext(tmp), NULL);
		b->head = tmp2;
	    }
	}
	else {
	    if (b->tail == tmp) {	/* Multiple elements, tmp at tail */
		eliBucketNode_SetNext(tmp2 = eliBucketNode_GetPrev(tmp), NULL);
		b->tail = tmp2;
	    }
	    else {		/* Mult elts, tmp not at head or tail */
		eliBucketNode_SetNext(tmp2 = eliBucketNode_GetPrev(tmp),
				      tmp3 = eliBucketNode_GetNext(tmp));
		eliBucketNode_SetPrev(tmp3, tmp2);
	    }
	}
	eliBucketNode_DecrRefcount(st, tmp);
    }
}
