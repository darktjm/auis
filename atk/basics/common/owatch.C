/* Copyright 1992 Carnegie Mellon University, All rights reserved.
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
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/owatch.C,v 3.4 1994/11/30 20:42:06 rr2b Stab74 $ */

#ifndef NORCSID
char *owatch_c_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/owatch.C,v 3.4 1994/11/30 20:42:06 rr2b Stab74 $";
#endif

#include <andrewos.h>
ATK_IMPL("owatch.H")


#include <observable.H>
#include "owatch.H"

static struct owatch_data *freedata=NULL;
static struct owatch_data *useddata=NULL;
static class owatch *owo=NULL;

#define owatch_BLOCKSIZE 8160
#define owatch_WATCHESPERBLOCK (owatch_BLOCKSIZE/sizeof(struct owatch_data))


ATKdefineRegistry(owatch, observable, NULL);
#ifndef NORCSID
#endif


void owatch::ObservedChanged(class observable  *changed, long  value  )
{
    struct owatch_data *o=useddata;
    if(value!=observable_OBJECTDESTROYED) return;
    while(o) {
	if(o->obj==changed && o->alive) {
	    o->alive=FALSE;
	    return;
	}
	o=o->next;
    }
}

struct owatch_data *owatch::Create(class observable  *obj)
{
    int i;
    struct owatch_data *o=useddata;
    if(obj==NULL) return NULL;
    
    if(owo==NULL) {
	owo=new owatch;
	if(owo==NULL) return NULL;
    }
    
    while(o) {
	if(o->obj==obj && o->alive) {
	    o->refs++;
	    return o;
	}
	o=o->next;
    }
    if(freedata==NULL) {
	struct owatch_data *oarray;
	oarray=(struct owatch_data *)malloc(owatch_WATCHESPERBLOCK*sizeof(struct owatch_data));
	if(oarray==NULL) return NULL;
	for(i=0;i<owatch_WATCHESPERBLOCK-1;i++) {
	    oarray[i].refs=0;
	    oarray[i].obj=NULL;
	    oarray[i].alive=FALSE;
	    oarray[i].next=oarray+i+1;
	    oarray[i].prev=oarray+i-1;
	}
	oarray[0].prev=NULL;
	oarray[owatch_WATCHESPERBLOCK-1].next=NULL;
	freedata=oarray;
    }
    if(freedata==NULL) return NULL;
    o=freedata;
    freedata=freedata->next;
    o->obj=obj;
    o->alive=TRUE;
    o->next=useddata;
    o->prev=NULL;
    o->refs++;
    if(useddata) {
	useddata->prev=o;
    }
    useddata=o;
    (o->obj)->AddObserver( owo);
    return o;
}

void owatch::Delete(struct owatch_data  *owd)
{
    if(owd==NULL) return;
    if(owd->refs<1) {
	fprintf(stderr, "owatch: Attempt to delete an already free owatch_data structure.");
	return;
    }
    if(--(owd->refs)>0) return;
    if(owd->alive) {
	(owd->obj)->RemoveObserver( owo);
    }
    if(owd->next) owd->next->prev=owd->prev;
    if(owd->prev) owd->prev->next=owd->next;
    else useddata=owd->next;
    owd->next=freedata;
    owd->prev=NULL;
    freedata=owd;
}

boolean owatch::CheckAndDelete(struct owatch_data  *owd)
{
    boolean result;
    if(owd==NULL) return FALSE;
    result=owatch::Check(owd);
    owatch::Delete(owd);
    return result;
}
    
