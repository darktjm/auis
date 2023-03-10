/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <stdio.h>
#include "scache.h"

/* This file (scache.c) and scache.h will implement a string cacheing mechanism so that even if menus are leaked they won't represent much of a drain... */

struct scache_el {
    long refcount;
    char str[1];
};

#define scache_REFCOUNT(string) (*(long *)((string)-(long)((struct scache_el *)0)->str))

struct scache_node {
    unsigned long els, maxels;
    struct scache_el **scache_el;
};

static struct scache_node scache[256] = {};

/* this hash function is pretty poor as far as magic functions go, but it's not too bad... */
#define HASH(str,len) (((*(str)<<2)+((str)[(len)>>1])+((str)[(len)-(len)?1:0]<<2)+((str)[len>>2]<<2)+(len))&0xff)
#define BLOCKSIZE 5
#define GROWTHFACTOR 2

const char *scache_Hold(const char *str)
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
	unsigned long i;
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

void scache_Free(const char *str)
{
    scache_REFCOUNT(str)--;
}


void scache_Collect(void) {
    int i;
    for(i=0;i<256;i++) {
        unsigned long j;
        struct scache_el **p=scache[i].scache_el;
        for(j=0;j<scache[i].els;j++, p++) {
            if((*p)->refcount<=0) {
                free(*p);
                *p=NULL;
            }
        }
    }
}

#if 0 /* unused & undeclared in headers */
void scache_Dump(void)
{
    int i;
    printf("doing scache dump\n");
    for(i=0;i<256;i++) {
	printf("------\n");
	if(scache[i].els) {
	    unsigned long j;
	    for(j=0;j<scache[i].els;j++) printf("%s\n",scache[i].scache_el[j]->str);
	}
    }
}
#endif
