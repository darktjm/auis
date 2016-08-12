/* Copyright 1993 Carnegie Mellon University All rights reserved.
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
#include <refs.h>
#include <util.h>

#ifndef FLEX_ENV
extern char yytext[];
#else
extern char *yytext;
#endif

static int done=0;
static struct clist {
    char *cname;
    char *super;
    struct clist *subs;
    struct clist *next;
} *classes=NULL;


static int NextToken()
{
    int ret;
    if(done) return -1;
    if(yyin && feof(yyin)) { /* 1st time around yyin not yet initialized */
	done=1;
	return -1;
    }
    ret=yylex();
    return ret;
}

static int NextNonWhiteSpace()
{
    int c=(-1);
    while(!done && ((c=NextToken())==refs_WhiteSpace));
    if(c==refs_WhiteSpace) return -1;
    return c;
}

static int entries=0;

static int AddClass(char *cname) {
    struct clist *result=NULL;
    int tok=refs_Name;
    char *super=NULL;
    super=NewString(yytext);
    tok=NextNonWhiteSpace();
    entries++;
    result=(struct clist *)malloc(sizeof(struct clist));
    if(result==NULL) {
	fprintf(stderr, "out of memory.\n");
	exit(-1);
    }
    result->cname=NewString(cname);
    if(result->cname==NULL) {
	fprintf(stderr, "out of memory.\n");
	exit(-1);
    }
    result->super=super;
    result->next=classes;
    classes=result;	
    return tok;
}

static void usage() {
    fprintf(stderr, "cat *.H|mkbrowse");
    exit(-1);
}

static int compareclasses(clist **x,clist **y)
{
    return strcmp((*x)->cname,(*y)->cname);
}

static int CheckSuperClass(clist **list,int count,const char *superclass)
{
    int top=0, bot=count-1;
    int cmp;
    
    while(bot!=top) {
	cmp=strcmp(list[(top+bot)/2]->cname, superclass);
	if(!cmp) return (top+bot)/2;
	if(cmp>0) bot=(top+bot-1)/2;
	else top=(top+bot+1)/2;
    }
    if(!strcmp(list[top]->cname, superclass)) return top;
    else return -1;
}

static void ShowSubClasses(clist *t,int depth)
{
    struct clist *TempEntry;
    int i=depth;
    if(depth>0) while(i--) putchar(' ');
    puts(t->cname);
    if(depth<0) printf("{\n");
    for(TempEntry=t->subs;TempEntry;TempEntry=TempEntry->next) {
	ShowSubClasses(TempEntry, depth<0?-1:depth+4);
    }
    if(depth<0) printf("}\n");
}

int main(int argc, char **argv) {
    char classname[1024];
    int tok;
    int i;
    char *p;
    tok=NextToken();
    while(!done) {
	switch(tok) {
	    case refs_Define:
		tok=NextNonWhiteSpace();
		if(tok!=refs_ATKdefineRegistry) continue;
		break;
	    case refs_Extern:
		tok=NextNonWhiteSpace();
		if(tok!=refs_ATKregistryEntry) continue;
		break;
	    case refs_ATKregistryEntry:
		tok=NextNonWhiteSpace();
		if(tok!=refs_ATKregistry_) continue;
		p=strchr(yytext, '_');
		if(p) *p='\0';
		strcpy(classname, yytext);
		tok=NextNonWhiteSpace();
		if(tok!=refs_Equal) continue;
		break;
	    case refs_ATKdefineRegistry:
		tok=NextNonWhiteSpace();
		if(tok!=refs_LeftParen) continue;
		tok=NextNonWhiteSpace();
		if(tok!=refs_Name) continue;
		break;
	    case refs_Include:
		break;
	    case refs_Class:
		tok=NextNonWhiteSpace();
		if(tok!=refs_Name) continue;
		if(strlen(yytext)>sizeof(classname)-1) {
		    fprintf(stderr, "Identifier %s too long, skipped...\n", yytext);
		    break;
		}
		strcpy(classname, yytext);
		tok=NextNonWhiteSpace();
		if(tok!=refs_Colon) continue;
		tok=NextNonWhiteSpace();
		while(tok==refs_Inheritance) tok=NextNonWhiteSpace();
		if(tok!=refs_Name) continue;
		tok=AddClass(classname);
		if(tok!=refs_LeftBrace) continue;
		break;
	    default:
		break;
	}
	tok=NextToken();
    }
    {
	clist **sortedlist=new clist *[entries], *TempEntry, *lasttop=NULL;
	int i=entries, index;
	for(TempEntry = classes; TempEntry;) {
	    i--;
	    sortedlist[i]=TempEntry;
	    TempEntry = TempEntry->next;
	    sortedlist[i]->subs=NULL;
	    sortedlist[i]->next=NULL;
	}
	qsort(sortedlist, entries, sizeof(struct clist *), QSORTFUNC(compareclasses));
	for(i=0;i<entries;i++) {
	    int index;
	    if(strcmp(sortedlist[i]->super,"ATK")==0) {
		sortedlist[i]->next=lasttop;
		lasttop=sortedlist[i];
		continue;
	    }
	    index=CheckSuperClass(sortedlist, entries, sortedlist[i]->super);
	    if(index<0) {
		fprintf(stderr,"Superclass %s of %s doesn't exist!\n",sortedlist[i]->cname, sortedlist[i]->super);
		continue;
	    }
	    sortedlist[i]->next=sortedlist[index]->subs;
	    sortedlist[index]->subs=sortedlist[i];
	}
	printf("\\begindata{org,42}\n");
	printf("ATK\n{");
	while(lasttop) {
	    ShowSubClasses(lasttop, -1);
	    lasttop=lasttop->next;
	}
	printf("}\n\\enddata{org,42}\n");
    }
    return 0;
}
