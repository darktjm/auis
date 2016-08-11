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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/support/RCS/hash.C,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 

/* A hash table */


#include <andrewos.h>
ATK_IMPL("hash.H")

#include <hash.H>
#include <glist.H>

struct egg {
    char *key,*value;
};




ATKdefineRegistry(hash, ATK, hash::InitializeClass);
#ifndef NORCSID
#endif
static int DefaultHash(char  *key);
static int FindEgg(struct egg  *egg,char  *key);
static PrintAll(struct egg  *egg,int  nothing);


static int DefaultHash(char  *key)
{
    char c;
    int index=0;

    while ((c = *key++) != '\0') {
        index += c;
    }
    index &= (hash_BUCKETS-1);
    return index;
}


boolean hash::InitializeClass()
{
    return TRUE;
}

hash::hash()
{
	ATKinit;

    int i;
    for(i=0;i<hash_BUCKETS;i++)
        this->buckets[i] = NULL;
    this->hashfunc = DefaultHash;
    THROWONFAILURE( TRUE);
}

hash::~hash()
{
	ATKinit;

    int i;
    for (i=0;i<hash_BUCKETS;i++)
        if (this->buckets[i] != NULL)
            delete this->buckets[i];
}

void hash::Store(char  *key,char  *value)
{
    int bucket = (*this->hashfunc)(key);
    struct egg *egg = (struct egg *)malloc(sizeof(struct egg));

    egg->key = (char *)malloc(strlen(key)+1);
    strcpy(egg->key,key);
    egg->value = value;

    if (this->buckets[bucket] == NULL)
        this->buckets[bucket] = glist::Create(NULL);

    (this->buckets[bucket])->Insert((char *)egg);
}

static int FindEgg(struct egg  *egg,char  *key)
{

    if (strcmp(egg->key,key)==0)
        return TRUE;
    else
        return FALSE;
}



char *hash::Lookup(char  *key)
{
    int bucket = (*this->hashfunc)(key);
    struct egg *egg;

    if (this->buckets[bucket] == NULL) {
        return NULL;
    }
    else {
	egg = (struct egg *) (this->buckets[bucket])->Find((glist_findfptr)FindEgg,key);
        if (egg != NULL) {
            return egg->value;
        }
        else {
            return NULL;
        }
    }
}

char * hash::Delete(char  *key)
{
    char *value;
    int bucket = (*this->hashfunc)(key);
    struct egg *egg;
    if (this->buckets[bucket] == NULL)
        return NULL;
    egg = (struct egg *)(this->buckets[bucket])->Find((glist_findfptr)FindEgg,key);
    if (egg != NULL) {
	value=egg->value;
        (this->buckets[bucket])->Delete((char *)egg,FALSE);
        free(egg->key);
        free(egg);
        return value;
    }
    else
        return NULL;
    
}

char *hash::Rename(char  *key,char  *new_c)
{
    int bucket = (*this->hashfunc)(key);
    struct egg *egg;
    if (this->buckets[bucket] == NULL)
        return NULL;
    egg = (struct egg *)(this->buckets[bucket])->Find((glist_findfptr)FindEgg,key);
    if (egg == NULL)
        return NULL;
    else {
        (this->buckets[bucket])->Delete((char *)egg,FALSE);
        egg->key = (char *)realloc(egg->key,strlen(new_c)+1);
        strcpy(egg->key,new_c);
        bucket = (*this->hashfunc)(new_c);
        if (this->buckets[bucket] == NULL)
            this->buckets[bucket] = glist::Create(NULL);
        (this->buckets[bucket])->Insert((char *)egg);
        return egg->value;
    }
}

void hash::Clear(hash_freefptr valFree)
{
    int i;
    struct egg *egg;
    class glist *gl;

    for (i=0;i<hash_BUCKETS;i++) {
	if ((gl = this->buckets[i]) != NULL)  {
	    while ((egg = (struct egg *) (gl)->Pop()) != NULL)  {
		if (egg->key != NULL)  {
		    free(egg->key);
		}
		if (egg->value != NULL && valFree != NULL)  {
		    (*valFree)(egg->value);
		}
		free(egg);
	    }
	    delete this->buckets[i];
	    this->buckets[i] = NULL;
	}
    }
}


static PrintAll(struct egg  *egg,int  nothing)
{
    printf("Egg (%s) contains (%s)\n",egg->key,egg->value);
    return FALSE;
}



void hash::Debug()
{
    int i;
    for (i=0;i<hash_BUCKETS;i++) {
        if (this->buckets[i] == NULL)
            printf("Bucket %d is NULL\n",i);
        else {
            printf("Bucket %d ------------------>\n",i);
            (this->buckets[i])->Find((glist_findfptr)PrintAll,NULL);
        }
    }
}
