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
#include <andrewos.h>
#include <util.h>
#include <ATKELinkI.H>
#include <fstream.h>

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
    if(p==NULL) {
	return;
    }
    if(strcmp(p, ".o")==0) {
	strcpy(p, ".exp");
    } else if(strcmp(p, ".a")==0) {
	strcpy(p, ".exp");
	char *q=strrchr(buf, '/');
	if(q==NULL) q=buf;
	else q++;
	strcpy(q, q+3);
    } else {
	return;
    }

    if(access(buf, R_OK)!=0) return;

    ifstream in(buf);
    if(!in.is_open()) {
	cerr<<"ATKELinkI: Warning: Couldn't open "<<buf<<'.'<<endl;
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

    strcpy(buf, AndrewDir("/etc/globalrefs._"));
    ifstream in2(buf);
    if(!in2.is_open()) {
	cerr<<"ATKELinkI: Warning: Couldn't open "<<buf<<'.'<<endl;
	return;
    }
    while(!in2.eof()) {
	in2.getline(buf, sizeof(buf));
	if(!in2.eof()) {
	    if(!ast->AddSymbol(buf)) {
		cerr<<"ATKELinkI: Couldn't add symbol to the symbol table."<<endl;
		return;
	    }
	}
    }
}

int ATKELinkI::PostProcess() {
    char exec[MAXPATHLEN];
    char buf[MAXPATHLEN];
    char cmd[MAXPATHLEN+1024];
    char *p;
    int i=0;
    int gsize=0;
    ResetIterator();
    while(p=NextArgument()) {
	if(strcmp(p,"-o")==0) {
	    i++;
	    p=NextArgument();
	    if(p==NULL) return -1;
	    strcpy(buf,p);
	    strcpy(exec,buf);
	    p=strrchr(exec,'.');
	    if(p) *p='\0';
	    DeleteArguments(i,1);
	    InsertArguments(exec,i);
	    break;
	}
	i++;
    }
    // logically the 55536 below should be 65536, but extra space in the gp table might be
    // needed when some of the libraries were compiled without -G 0
    sprintf(cmd, "size -A %s|awk '/sdata|sbss|lit8|lit4/ { sum += $2 }\\\nEND { print 55536-sum}'", buf);
    FILE *fp=popen(cmd, "r");
    if(fp==NULL) return -2;
    if(fscanf(fp, "%d", &gsize)!=1) return -3;
    pclose(fp);
    unlink(buf);
    if(gsize<=0) {
	gsize=0;
	cerr<<"ATKELink: warning: no space for gp data in dynamic objects."<<endl;
    }
    sprintf(cmd, "cc -c -G %d -DMIPS_GLOBAL_SPACE_SIZE=%d %s", gsize, gsize, AndrewDir("/etc/mips_global_space.c"));
    if(system(cmd)!=0) return -3;
    p=SystemString();
    if(p==NULL) return -4;
    return system(p);
}

int ATKELinkI::ArgumentsDone() {
    char *exec;
    char *p;
    int i=0;
    ResetIterator();
    while(p=NextArgument()) {
	if(strcmp(p,"-o")==0) {
	    char buf[MAXPATHLEN];
	    i++;
	    exec=NewString(NextArgument());
	    strcpy(buf,exec);
	    strcat(buf,".tmp");
	    DeleteArguments(i,1);
	    InsertArguments(buf,i);
	    break;
	}
	i++;
    }
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
    while(p=ast->NextSymbol()) {
	out<<'L'<<'C'<<L<<':'<<endl;
	out<<".ascii \""<<p<<'\\'<<'0'<<'"'<<endl;
	L++;
    }
    out<<".align 4"<<endl;
    out<<".globl globals"<<endl;
    out<<"globals:"<<endl;
    L=0;
    ast->ResetIterator();
    while(p=ast->NextSymbol()) {
	out<<".word "<<p<<endl;
	out<<".word "<<'L'<<'C'<<L<<endl;
	L++;
    }
    out<<".globl globalcount"<<endl;
    out<<"globalcount:"<<endl;
    out<<".word "<<L<<endl;
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
    sprintf(cmd, "cc -c -DMIPS_GLOBAL_SPACE_SIZE=0 %s",AndrewDir("/etc/mips_global_space.c"));
    rc=system(cmd);
    if(rc!=0) {
	cerr<<"ATKELinkI: Couldn't compile mips_global_space.o"<<endl;
	cerr<<"ATKELinkI: Command '"<<cmd<<"' failed."<<endl;
	return -3;
    }
    InsertArguments("mips_global_space.o", FirstObjectFile());
    return 0;
    
}
