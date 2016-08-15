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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/support/RCS/strcache.C,v 3.3 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 

/* A string cache */

ATK_IMPL("strcache.H")

#include <strcache.H>
#include <util.h>
#include <ctype.h>

static class strcache *gcache=NULL;


ATKdefineRegistry(strcache, ghash, strcache::InitializeClass);
#ifndef NORCSID
#endif
static int lchash(char  *key);
static int lccomp(char  *a ,char  *b);
static boolean EnumProc(struct egg  *e, struct enumerate  *rock);
char *Enumerate(class ghash  *self,procedure  proc,long  rock);
boolean DumpStr(long  rock, char  *val, char  *key, class strcache  *self);


static int lchash(char  *key)
{
    char c;
    int index=0;

    while ((c = *key++) != '\0') {
	index += isupper(c)?tolower(c):c;
    }
    index &= (ghash_BUCKETS-1);
    return index;
}

static int lccomp(char  *a ,char  *b)
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

char *strcache::SaveStr(char  *str)
{
	ATKinit;

    char *result;
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

