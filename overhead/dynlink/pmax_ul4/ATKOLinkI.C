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

 // ATKDynImpl: the system specific code to adapt the link line for a dynamic object.

#include <ATKOLinkI.H>
#include <iostream>
#include <fstream>

#include <doload.h>
#include <util.h>

using namespace std;

ATKOLinkI::ATKOLinkI() {
    skipobject=FALSE;
    dyndebug=FALSE;
    ast=new ATKSymTab;
    if(ast==NULL) {
	cerr<<"ATKOLinkI: Couldn't create new symtab."<<endl;
	exit(-1);
    }
}

ATKOLinkI::~ATKOLinkI() {
    delete ast;
}

void ATKOLinkI::ProcessArgument(char *arg) {
    char buf[MAXPATHLEN];
    if(skipobject) {
	skipobject=FALSE;
	InsertArguments(arg);
	return;
    }
    if(strcmp(arg, "-o")==0) {
	skipobject=TRUE;
	InsertArguments(arg);
	return;
    }
    if(strcmp(arg, "-g")==0) {
	dyndebug=TRUE;
    }
    if(strlen(arg)>sizeof(buf)-3) {
	cerr<<"Argument too long, ignored..."<<endl<<arg<<endl;
	return;
    }
    strcpy(buf, arg);

    char *q=strrchr(buf, '/');
    if(q==NULL) q=buf;
    else q++;

    char *r=strrchr(q, '.');
    if(r && r[1]=='+') {
	char abuf[MAXPATHLEN];
	*r='\0';
	sprintf(abuf,"-Wl,-u,%s_ATKregistry_",q);
	InsertArguments(abuf);
	*r='.';
    } else {
	if(strncmp(q, "lib", 3)!=0) {
	    InsertArguments(arg);
	    return;
	}
	strcpy(q, q+3);
    }
    char *p=strrchr(buf, '.');
    if(p==NULL) {
	InsertArguments(arg);
	return;
    }

    strcpy(p, ".exp");

    if(access(buf, R_OK)!=0) {
	InsertArguments(arg);
	return;
    }

    ifstream in(buf);
    if(!in.is_open()) {
	cerr<<"ATKOLinkI: Warning: Couldn't open "<<buf<<'.';
    }
    while(!in.eof()) {
	in.getline(buf, sizeof(buf));
	if(!in.eof()) {
	    if(!ast->AddSymbol(buf)) {
		cerr<<"ATKOLinkI: Couldn't add symbol to the symbol table."<<endl;
		exit(-1);
	    }
	}
    }   
}

boolean ATKOLinkI::FindSymbol(const char *sym)
{
    return ast?(ast->FindSymbol(sym)?TRUE:FALSE):FALSE;
}

void ATKOLinkI::ArgumentsDone() {
    char buf[MAXPATHLEN];
   
    ifstream in(AndrewDir("/etc/globalrefs._"));
    if(!in.is_open()) {
	cerr<<"ATKOLinkI: Warning: Couldn't open "<<buf<<'.'<<endl;
    }
    while(!in.eof()) {
	in.getline(buf, sizeof(buf));
	if(!in.eof()) {
	    if(!ast->AddSymbol(buf)) {
		cerr<<"ATKOLinkI: Couldn't add symbol to the symbol table."<<endl;
		exit(-1);
	    }
	}
    }
}

extern char *ATKDoFix(char *path, ATKSymTab *ast, boolean dyndebug);
int ATKOLinkI::Test() {
    char *p, *base;
    long len;
    char buf[MAXPATHLEN+1];
    ResetIterator();
    while(p=NextArgument()) {
	if(p[0]=='-' && p[1]=='o' && p[2]=='\0') {
	    p=NextArgument();
	    break;
	}
    }
    if(p==NULL) {
	cerr<<"ATKOLinkI: couldn't find object filename on the link line."<<endl;
	return 1;
    }
    p=ATKDoFix(p, ast, dyndebug);
    if(p==NULL) {
	cerr<<"ATKOLinkI: couldn't fix object file for dynamic loading."<<endl;
	return 2;
    }
    return 0;
}

