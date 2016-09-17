/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/* A hash table */

#include <andrewos.h>
ATK_IMPL("ghash.H")
#include <ghash.H>
#include <glist.H>
#include <util.h>

struct egg {
    const char *key;
    char *value;
};




ATKdefineRegistry(ghash, ATK, ghash::InitializeClass);
static int DefaultHash(const char  *key);
static int safestrcmp(const char  *a ,const char  *b);
static int FindEgg(struct egg  *egg,const char  *key);
static boolean EnumProc(struct egg  *e, struct enumerate  *rock);
static int PrintAll(struct egg  *egg,int  nothing);


static int DefaultHash(const char  *key)
{
    char c;
    int index=0;

    while ((c = *key++) != '\0') {
        index += c;
    }
    index &= (ghash_BUCKETS-1);
    return index;
}


boolean ghash::InitializeClass()
{
    return TRUE;
}

static int safestrcmp(const char  *a ,const char  *b)
{
    if(a==NULL && b==NULL) return 0;
    if(a==NULL) return -1;
    if(b==NULL) return 1;
    return strcmp(a, b);
}

ghash::ghash()
{
	ATKinit;

    int i;
    for(i=0;i<ghash_BUCKETS;i++)
        this->buckets[i] = NULL;
    this->hash = DefaultHash;
    (this)->SetFreeKey( (ghash_freekeyfptr)free);
    (this)->SetCopyKey( NewString);
    (this)->SetCompareKey( safestrcmp);
    (this)->SetCopyVal( NULL);
    (this)->SetFreeVal( NULL);
    THROWONFAILURE( TRUE);
}

ghash::~ghash()
{
	ATKinit;

    int i;
    for (i=0;i<ghash_BUCKETS;i++)
        if (this->buckets[i] != NULL)
            delete this->buckets[i];
}

boolean ghash::Store(const char  *key,const char  *value)
{
    int bucket = (*this->hash)(key);
    struct egg *egg = (struct egg *)malloc(sizeof(struct egg));

    if(egg==NULL) return FALSE;
    
    if((this)->GetCopyKey()) key = (this)->GetCopyKey()(key);
    egg->key = key;
    
    if((this)->GetCopyVal()) value = (this)->GetCopyVal()(value);
    egg->value = (char *)value;

    if (this->buckets[bucket] == NULL)
        this->buckets[bucket] = glist::Create(NULL);

    (this->buckets[bucket])->Insert(egg);
    return TRUE;
}

static ghash_comparekeyfptr compkey=NULL;

static int FindEgg(struct egg  *egg,const char  *key)
{

    if (compkey) {
	if(compkey(egg->key, key)==0) return TRUE;
	else return FALSE;

    } else if(egg->key==key)
        return TRUE;
    else
        return FALSE;
}



char *ghash::Lookup(const char  *key)
{
    int bucket = (*this->hash)(key);
    struct egg *egg;

    if (this->buckets[bucket] == NULL) {
        return NULL;
    }
    else {
	compkey=(this)->GetCompareKey();
	egg = (struct egg *) (this->buckets[bucket])->Find((glist_findfptr)FindEgg,key);
        if (egg != NULL) {
            return egg->value;
        }
        else {
            return NULL;
        }
    }
}

const char *ghash::LookupKey(const char  *key)
{
    int bucket = (*this->hash)(key);
    struct egg *egg;

    if (this->buckets[bucket] == NULL) {
        return NULL;
    }
    else {
	compkey=(this)->GetCompareKey();
	egg = (struct egg *) (this->buckets[bucket])->Find((glist_findfptr)FindEgg,key);
        if (egg != NULL) {
            return egg->key;
        }
        else {
            return NULL;
        }
    }
}

char * ghash::Delete(const char  *key)
{
    int bucket = (this->hash)(key);
    struct egg *egg;
    if (this->buckets[bucket] == NULL)
	return NULL;
    compkey=(this)->GetCompareKey();
    egg = (struct egg *)(this->buckets[bucket])->Find((glist_findfptr)FindEgg,key);
    if (egg != NULL) {
	char *val = egg->value;
        (this->buckets[bucket])->Delete(egg,FALSE);
	if((this)->GetFreeVal()) (this)->GetFreeVal()(egg->value);
	if((this)->GetFreeKey()) (this)->GetFreeVal()((char *)egg->key);
        free(egg);
        return val; /* which may be freed - be wary! */
    }
    else
        return NULL;
    
}

struct enumerate {
    boolean found;
    ghash_efptr proc;
    long rock;
    class ghash *self;
};

static boolean EnumProc(struct egg  *e, struct enumerate  *rock)
{
    boolean result;
    result=rock->proc(rock->rock, e->value,  e->key, rock->self);
    rock->found=result;
    return result;
}

char *ghash::Enumerate(ghash_efptr  proc,long  rock)
{
    char *result = NULL;
    struct enumerate r;
    int i;
    if(this->buckets==NULL) return NULL;
    r.found=FALSE;
    r.proc=proc;
    r.rock=rock;
    r.self=this;
    for(i=0;i<ghash_BUCKETS;i++) {
	if(this->buckets[i]) result=(char *)(this->buckets[i])->Find((glist_findfptr) EnumProc, &r);
	if(r.found) return result;
    }
    return NULL;
}
    

char *ghash::Rename(const char  *key,char  *new_c)
{
    int bucket = (*this->hash)(key);
    struct egg *egg;
    if (this->buckets[bucket] == NULL)
	return NULL;
    compkey=(this)->GetCompareKey();
    egg = (struct egg *)(this->buckets[bucket])->Find((glist_findfptr)FindEgg,key);
    if (egg == NULL)
        return NULL;
    else {
        (this->buckets[bucket])->Delete(egg,FALSE);
	if((this)->GetFreeKey()) (this)->GetFreeKey()((char *)egg->key);
	if((this)->GetCopyKey()) key = (this)->GetCopyKey()(key);
	egg->key = key; /* tjm - wtf?  shouldn't this be new_c? */
        bucket = (*this->hash)(new_c);
        if (this->buckets[bucket] == NULL)
            this->buckets[bucket] = glist::Create(NULL);
        (this->buckets[bucket])->Insert(egg);
        return egg->value;
    }
}

void ghash::Clear()
{
    int i;
    struct egg *egg;
    class glist *gl;

    for (i=0;i<ghash_BUCKETS;i++) {
	if ((gl = this->buckets[i]) != NULL)  {
	    while ((egg = (struct egg *) (gl)->Pop()) != NULL)  {
		if (egg->key != NULL)  {
		    if((this)->GetFreeKey()) (this)->GetFreeKey()((char *)egg->key);
		}
		if ((this)->GetFreeVal())  {
		    (this)->GetFreeVal()(egg->value);
		}
		free(egg);
	    }
	    delete this->buckets[i];
	    this->buckets[i] = NULL;
	}
    }
}


static int PrintAll(struct egg  *egg,int  nothing)
{
    printf("Egg (%s) contains (%s)\n",egg->key,egg->value);
    return FALSE;
}



void ghash::Debug()
{
    int i;
    for (i=0;i<ghash_BUCKETS;i++) {
        if (this->buckets[i] == NULL)
            printf("Bucket %d is NULL\n",i);
        else {
            printf("Bucket %d ------------------>\n",i);
            (this->buckets[i])->Find((glist_findfptr)PrintAll,NULL);
        }
    }
}
