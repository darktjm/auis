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
#include <fstream.h>

#include <ATKDoLoad.H>

ATKOLinkI::ATKOLinkI() {
}

ATKOLinkI::~ATKOLinkI() {
}

void ATKOLinkI::ProcessArgument(char *arg) {
    InsertArguments(arg);
   
}

void ATKOLinkI::ArgumentsDone() {
}

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
    int (*entry)(int argc, char **argv)=ATKDoLoad(p,0);
    if(entry==NULL) {
	cerr<<"ATKOLinkI: couldn't dynamically load file "<<p<<"."<<endl;
	return 3;
    }
    return 0;
}

