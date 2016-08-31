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

#include  <ht.h>

/* Default hashing function */

int             eliHT_Hash(key)
char           *key;
{
    int             result = 0;
    char           *p = key;

    do
	result += *p;
    while (*(p++));
    return (result % NUMBUCKETS);
}

/* Initialize a hash table */

void            eliHT_Init(h)
eliHashTable_t *h;
{
    int             i;

    h->hash = eliHT_Hash;
    for (i = 0; i < NUMBUCKETS; ++i)
	eliBucket_Init(&(h->buckets[i]));
}

/*
 * Create a bucketnode to hold datum, and insert into the hash table with the
 * given key 
 */

void            eliHT_Insert(st, h, datum, key)
EliState_t     *st;
eliHashTable_t *h;
EliSexp_t      *datum;
char           *key;
{
    eliBucketNode_t *tmp;

    if (!(tmp = eliBucketNode_GetNew_trace(st, EliTraceStk(st), datum, key)))
	return;
    eliBucket_Insert(&(h->buckets[(*(h->hash)) (key)]), tmp);
}

/* Return the datum within the proper bucketnode whose key is key */

EliSexp_t      *eliHT_Find(h, key)
eliHashTable_t *h;
char           *key;
{
    return (eliBucket_Find(&(h->buckets[(*(h->hash)) (key)]), key));
}

/* Remove the bucketnode whose key is key */

void            eliHT_Delete(st, h, key)
EliState_t     *st;
eliHashTable_t *h;
char           *key;
{
    eliBucket_Delete(st, &(h->buckets[(*(h->hash)) (key)]), key);
}

/* Some may never wish to use this */

void            eliHT_SetHashFn(h, fn)
eliHashTable_t *h;
int             (*fn) ();

{
    h->hash = fn;
}
