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

#include <iostream>
#include <fstream>
#include <ATKELinkI.H>

using namespace std;

ATKELinkI::ATKELinkI() {
    ast=new ATKSymTab;
    afile2[0]=ofile2[0]='\0';
}

ATKELinkI::~ATKELinkI() {
    delete ast;
    unlink(afile2);
    unlink(ofile2);
}

void ATKELinkI::ProcessArgument(char *arg) {
    InsertArguments(arg);
    char buf[MAXPATHLEN+1];
    if(strlen(arg)>=MAXPATHLEN-2) return;
    strcpy(buf, arg);
    char *p=strrchr(buf, '.');
    if(p==NULL) return;
    p++;
    if(strcmp(p, "a")!=0 && strcmp(p, "+")!=0) return;
    strcpy(p, "exp");
    p=strrchr(buf, '/');
    if(p==NULL) return;
    p++;
    if(strncmp(p, "lib", 3)==0) strcpy(p, p+3);
    
    ifstream in(buf);
    if(!in.is_open()) {
	return;
    }
    while(!in.eof()) {
	in.getline(buf, sizeof(buf));
	for(p=buf;*p && !isspace(*p);p++);
	if(*p && isspace(*p)) *p='\0';
	if(!in.eof()) {
	    if(strncmp(buf, "_GLOBAL_.", 9)==0 && (buf[9]=='I' || buf[9]=='D') && buf[10]=='.') {
		if(!ast->AddSymbol(buf)) {
		    cerr<<"ATKELinkI: Couldn't add symbol "<<buf<<" to the symbol table."<<endl;
		    in.close();
		    exit(-1);
		}
	    }
	}
    }
    in.close();
}

int ATKELinkI::ArgumentsDone() {
    char *p;
    char *afile=tmpnam(afile2);
    if(afile==NULL) return 1;
    char *ofile=tmpnam(ofile2);
    if(ofile==NULL) return 2;
    ofstream out(afile2);
    if(!out.is_open()) {
	cerr<<"ATKELinkI: Couldn't open temporary file "<<afile2<<"."<<endl;
	unlink(afile2);
	unlink(ofile2);
	return -1;
    }
    out<<".file \"ATKIniFini.c\""<<endl;
    out<<".toc"<<endl;
    out<<".csect .text[PR]"<<endl;
    out<<"gcc2_compiled.:"<<endl;
    out<<"__gnu_compiled_c:"<<endl;
    out<<".globl ATK_IniList"<<endl;
    out<<".globl ATK_FiniList"<<endl;
    out<<".csect .data[RW]"<<endl;
    out<<".align 2"<<endl;
    out<<"ATK_IniList:"<<endl;
    ast->ResetIterator();
    while(p=ast->NextSymbol()) {
	if(p[9]=='I') {
	    out<<".extern "<<p<<"[DS]"<<endl;
	    out<<".extern ."<<p<<endl;
	    out<<".long "<<p<<"[DS]"<<endl;
	}
    }
    out<<".long 0"<<endl;
    out<<"ATK_FiniList:"<<endl;
    ast->ResetIterator();
    while(p=ast->NextSymbol()) {
	if(p[9]=='D') {
	    out<<".extern "<<p<<"[DS]"<<endl;
	    out<<".extern ."<<p<<endl;
	    out<<".long "<<p<<"[DS]"<<endl;
	}
    }
    out<<".long 0"<<endl;
    out.close();
    char cmd[MAXPATHLEN+1024];
    strcat(ofile2, ".o");
    sprintf(cmd, "as -o %s %s", ofile2, afile2);
    if(system(cmd)!=0) {
	cerr<<"ATKELinkI: Command '"<<cmd<<"' failed."<<endl;
	unlink(afile2);
	unlink(ofile2);
	return 3;
    }
    unlink(afile2);
    InsertArguments(ofile2);
    return 0;
    
}
