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

#include <ATKELinkI.H>
#include <iostream>
#include <fstream>

using namespace std;

#include <doload.h>

ATKELinkI::ATKELinkI() {
    afile2[0]='\0';
    ofile2[0]='\0';
    ast=new ATKSymTab;
    if(ast==NULL) {
	cerr<<"ATKELinkI: Couldn't create new symtab."<<endl;
	exit(-1);
    }
}

ATKELinkI::~ATKELinkI() {
    delete ast;
    if(afile2[0]) unlink(afile2);
    if(ofile2[0]) unlink(ofile2);
}

void ATKELinkI::ProcessArgument(char *arg) {
    char buf[MAXPATHLEN];

    InsertArguments(arg);
    if(strlen(arg)>sizeof(buf)-3) {
	cerr<<"Argument too long, ignored..."<<endl<<arg<<endl;
	return;
    }
    strcpy(buf, arg);
    
    char *p=strrchr(buf, '.');
    if(p==NULL) return;
    
    if(strcmp(p, ".o")==0) {
	strcpy(p, ".exp");
    } else if(strcmp(p, ".a")==0) {
	strcpy(p, ".exp");
	char *q=strrchr(buf, '/');
	if(q==NULL) q=buf;
	else q++;
	memmove(q, q+3, strlen(q + 3) + 1);
    } else {
	return;
    }

    if(access(buf, R_OK)!=0) return;

    ifstream in(buf);
    if(!in.is_open()) {
	cerr<<"ATKELinkI: Warning: Couldn't open "<<buf<<'.';
	return;
    }
    while(!in.eof()) {
	in.getline(buf, sizeof(buf));
	if(!in.eof()) {
	    if(!ast->AddSymbol(buf)) {
		cerr<<"ATKELinkI: Couldn't add symbol to the symbol table."<<endl;
		return;
	    }
	}
    }
   
}

int ATKELinkI::ArgumentsDone() {
    char *afile=tmpnam(afile2);
    if(afile==NULL) return 1;
    ofstream out(afile2);
    if(!out.is_open()) {
	cerr<<"ATKELinkI: Couldn't open temporary file "<<afile2<<"."<<endl;
	return -1;
    }
    out<<".data"<<endl;
    int L=0;
    ast->ResetIterator();
    char *p;
    while(p=ast->NextSymbol()) {
	out<<'L'<<'C'<<L<<':'<<endl;
	out<<".ascii \""<<p<<'\\'<<'0'<<'"'<<endl;
	L++;
    }
    out<<".align 4"<<endl;
    out<<".globl _globals"<<endl;
    out<<"_globals:"<<endl;
    L=0;
    ast->ResetIterator();
    while(p=ast->NextSymbol()) {
	out<<".long "<<'L'<<'C'<<L<<endl;
	out<<".long "<<p<<endl;
	L++;
    }
    out<<".global _globalcount"<<endl;
    out<<"_globalcount:"<<endl;
    out<<".long "<<L<<endl;
    out.close();
    char *ofile=tmpnam(ofile2);
    if(ofile==NULL) return 2;
    strcat(ofile2, ".o");
    char cmd[1024];
    sprintf(cmd, "as %s -o %s", afile2, ofile2);
    afile2[0]='\0';
    int rc=system(cmd);
    if(rc!=0) {
	cerr<<"ATKELinkI: Couldn't assemble symbol table."<<endl;
	cerr<<"ATKELinkI: Command '"<<cmd<<"' failed."<<endl;
	return -2;
    }
    InsertArguments(ofile2, FirstObjectFile());
    return 0;
    
}
