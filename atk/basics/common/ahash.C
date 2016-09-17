/* ********************************************************************** *\
 *	   Copyright Carnegie Mellon, 1996 - All Rights Reserved
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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
ATK_IMPL("ahash.H")
#include <ahash.H>

AHash::~AHash() {
}

size_t AHash::BucketN(size_t) {
    return 0;
}

size_t AHash::FirstBucket(size_t i) {
    while(i<maxhash) {
	if(BucketN(i)) return i;
	i++;
    }
    return i;
}

#ifdef TESTING
 DEFINE_AMHASH_CLASS(tester,AHashStringKey,long);
 DEFINE_AHASH_CLASS_ITER(tester,AHashStringKey,long);
 int main() {
    tester grn;
    grn.Insert("foo", 9);
    grn.Insert("Bar", 4);
    long *r=grn.Fetch("foo");
    printf("r:%x\t", r);
    fflush(stdout);
    printf("*r:%d\n", *r);
    r=grn.Fetch("Bar");
    printf("r:%x\t", r);
    fflush(stdout);
    printf("*r:%d\n", *r);
    r=grn.Fetch("BAZ");
    
    printf("r:%x\t", r);
    fflush(stdout);
    if(r) printf("*r:%d\n", *r);
    fflush(stdout);
    tester_iter g(grn);
    printf("starting iteration...\n");
    fflush(stdout);
    while(!g.done()) {
	printf("%s:%d\n", g.Key().CStr(), *g);
	fflush(stdout);
	++g;
    }
    printf("done iterating...\n");
    fflush(stdout);
}
    
#endif
