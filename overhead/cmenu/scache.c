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
#include <stdio.h>
#include "scache.h"

/* This file (scache.c) and scache.h will implement a string cacheing mechanism so that even if menus are leaked they won't represent much of a drain... */

static struct scache_node scache[256];

#define HASH(str,len) (((*(str)<<2)+((str)[(len)>>1])+((str)[(len)-(len)?1:0]<<2)+((str)[len>>2]<<2)+(len))&0xff)
#define BLOCKSIZE 5
#define GROWTHFACTOR 2

void scache_Init()
{
    memset(scache,0, sizeof(scache));
}

const char *scache_Hold(str)
const char *str;
{
    unsigned long len;
    unsigned long hash;
    struct scache_node *s;
    struct scache_el *e=NULL;
    int indx=(-1);
    
    if(!str) str="";
    
    len=strlen(str);
    hash=HASH(str,len);
    s=scache+hash;

    if(s->els) {
	int i;
	struct scache_el **p=s->scache_el;
	for(i=0;i<s->els;i++, p++) {
	    if(*p && !strcmp((*p)->str, str)) {
		(*p)->refcount++;
		return (*p)->str;
            }
            if(indx<0 && (*p==NULL || (*p)->refcount<=0)) {
                if(*p) {
                    free(*p);
                    *p=NULL;
                }
                indx=i;
            }
	}
    }
    if(indx<0) {
        if(s->els>=s->maxels) {
            s->maxels=s->maxels*GROWTHFACTOR+BLOCKSIZE;
            if(s->scache_el) s->scache_el=(struct scache_el **)realloc(s->scache_el, s->maxels*sizeof(struct scache_el *));
            else s->scache_el=(struct scache_el **)malloc(s->maxels*sizeof(struct scache_el *));
            if(!s->scache_el) {
                s->els=0;
                s->maxels=0;
                return NULL;
            }
        }
        indx=s->els;
        s->els++;
    }
    e=(struct scache_el *)malloc(len+sizeof(struct scache_el));
    s->scache_el[indx]=e;
    if(!e) return NULL;
    strcpy(e->str,str);
    e->refcount=1;
    return e->str;
}

void scache_Free(str)
const char *str;
{
    scache_REFCOUNT(str)--;
}


void scache_Collect() {
    int i;
    for(i=0;i<256;i++) {
        int j;
        struct scache_el **p=scache[i].scache_el;
        for(j=0;j<scache[i].els;j++, p++) {
            if((*p)->refcount<=0) {
                free(*p);
                *p=NULL;
            }
        }
    }
}

void scache_Dump()
{
    int i;
    printf("doing scache dump\n");
    for(i=0;i<256;i++) {
	printf("------\n");
	if(scache[i].els) {
	    int j;
	    for(j=0;j<scache[i].els;j++) printf("%s\n",scache[i].scache_el[j]->str);
	}
    }
}
