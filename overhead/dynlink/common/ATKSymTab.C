/* Copyright 1994 Carnegie Mellon University All rights reserved.
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

#include <ATKSymTab.H>
#include <fstream.h>

const int ATKSymList::BumpSize=100;
const int ATKSymTab::HashSize=401;

ATKSymList::ATKSymList() {
    syms=NULL;
    size=0;
    used=0;
    extra=0;
}

ATKSymList::ATKSymList(int ex) {
    syms=NULL;
    size=0;
    used=0;
    extra=ex;
}

ATKSymList::~ATKSymList() {
    if(syms) free(syms);
}

#define BYTE(v,s) ((char)(((v)>>(s))&0xff))
void * ATKSymList::AddSymbol(const char *sym) {
    int len=strlen(sym);
    int newsize=used+len+1+extra;
    void *result;
    if(newsize>=size) {
	if(syms) syms=(char *)realloc(syms, newsize+BumpSize);
	else syms=(char *)malloc(newsize+BumpSize);
	if(syms==NULL) {
	    used=0;
	    size=0;
	    return NULL;
	}
	size=newsize+BumpSize;
    }
    result=syms+used;
    strcpy(syms+used+extra, sym);
    if(extra) used=((newsize+extra-1)/extra)*extra;
    else used=newsize;
    return result;
}

void *ATKSymList::FindSymbol(const char *sym) {
    char *p=syms;
    while(p-syms<used) {
	if(strcmp(p+extra, sym)==0) {
	    return p;
	}
	p+=extra+strlen(p+extra)+1;
	if(extra) {
	    int pos=(p-syms);
	    pos=((pos+extra-1)/extra)*extra;
	    p=syms+pos;
	}
    }
    return NULL;
}

void ATKSymList::SetExtra(int ex) {
    extra=ex;
}

ATKSymTab::ATKSymTab() {
    sl=new ATKSymList[HashSize];
}

ATKSymTab::ATKSymTab(int extra) {
    sl=new ATKSymList[HashSize];
    if(sl) {
	for(int i=0;i<HashSize;i++) sl[i].SetExtra(extra);
    }
}

ATKSymTab::~ATKSymTab() {
    delete [] sl;
}

int ATKSymTab::Hash(const char *sym) {
    const unsigned char *p=(const unsigned char *)sym;
    unsigned int hash=0;
    while(*p) {
	hash=hash*4301+*p;
	p++;
    }
    hash=hash%HashSize;
    return hash;
}

void *ATKSymTab::AddSymbol(const char *sym)
{
    if(sl==NULL) return FALSE;
    return sl[Hash(sym)].AddSymbol(sym);
}

void *ATKSymTab::FindSymbol(const char *sym)
{
    if(sl==NULL) return FALSE;
    return sl[Hash(sym)].FindSymbol(sym);
}

static char *sympos=NULL;
static int iterator=(-1);

void ATKSymList::ResetIterator() {
    sympos=syms;
    if(syms==NULL) return;
}

void ATKSymTab::ResetIterator() {
    iterator=0;
    sl[0].ResetIterator();
	
}

char *ATKSymList::NextSymbol() {

    char *sym=sympos;
    if(sympos==NULL) return NULL;
    if(sympos-syms>=used) return NULL;
    sympos+=extra+strlen(sym+extra)+1;
    if(extra) {
	int pos=(sympos-syms);
	pos=((pos+extra-1)/extra)*extra;
	sympos=syms+pos;
    }
    return sym+extra;
}

char *ATKSymTab::NextSymbol() {
    if(iterator<0) return NULL;
    char *p;
    while((p=sl[iterator].NextSymbol())==NULL) {
	iterator++;
	if(iterator>=HashSize) {
	    iterator=(-1);
	    return NULL;
	}
	sl[iterator].ResetIterator();
    }
    return p;
}
