/* Copyright 1994, Carnegie Mellon University, All rights reserved.
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

// ATKDynLink: the machine independent part of constructing a link line for dynamic objects.

#include <ATKOLinkI.H>
#include <iostream>

using namespace std;

void ATKOLink::ProcessArgument(char *arg) {
    InsertArguments(arg);
}

void ATKOLink::ArgumentsDone() {
} 

int ATKOLink::Test() {
    return 0;
}

int main(int argc, char **argv) {
    char *cmd;
    ATKOLink *adl=new ATKOLinkI;
    if(adl==NULL) {
	cerr<<"adynlink: Couldn't create argument list."<<endl;
	exit(-1);
    }
    argc--;
    argv++;
    for(int i=0;i<argc;i++) {
	adl->ProcessArgument(argv[i]);
    }
    adl->ArgumentsDone();
    cmd=adl->SystemString();
    if(cmd==NULL) {
	cerr<<"adynlink: Couldn't create command to link dynamic object."<<endl;
	exit(-1);
    }
    int result=system(cmd);
    if(result!=0) {
	cerr<<"adynlink: Link command failed with error code "<<result<<"."<<endl;
	exit(-1);
    }
    result=adl->Test();
    if(result!=0) {
	cerr<<"adynlink: Dynamic object test failed with error code "<<result<<"."<<endl;
	exit(-1);
    }
    exit(0);
}
    
    
	
