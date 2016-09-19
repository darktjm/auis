/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* A string cache */

#include <andrewos.h>
ATK_IMPL("strcache.H")

#include <strcache.H>
#include <util.h>
#include <ctype.h>

static class strcache *gcache=NULL;


ATKdefineRegistry(strcache, ghash, strcache::InitializeClass);
static int lchash(const char  *key);
static int lccomp(const char  *a ,const char  *b);
char *Enumerate(class ghash  *self,procedure  proc,long  rock);
boolean DumpStr(long  rock, char  *val, char  *key, class strcache  *self);


static int lchash(const char  *key)
{
    char c;
    int index=0;

    while ((c = *key++) != '\0') {
	index += isupper(c)?tolower(c):c;
    }
    index &= (ghash_BUCKETS-1);
    return index;
}

static int lccomp(const char  *a ,const char  *b)
{
    if(a==NULL && b==NULL) return 0;
    if(a==NULL) return -1;
    if(b==NULL) return 1;
    return lc_strcmp(a, b);
}

boolean strcache::InitializeClass()
{
    gcache=new strcache;
    if(gcache==NULL) return FALSE;
    (gcache)->SetCompareKey( lccomp);
    (gcache)->SetHash( lchash);
    return TRUE;
}

strcache::strcache()
{
	ATKinit;

    THROWONFAILURE( TRUE);
}

const char *strcache::SaveStr(const char  *str)
{
	ATKinit;

    const char *result;
    if(gcache==NULL) return NULL;
    result=(gcache)->LookupKey( str);
    if(result) return result;
    if(!(gcache)->Store( str, NULL)) return NULL;
    result=(gcache)->LookupKey( str);
    return result;
}


strcache::~strcache()
{
	ATKinit;


}

char *strcache::Delete(char  *key)
{
    return NULL;
}
    

char *strcache::Rename(char  *key,char  *new_c)
{
    return NULL;
}

