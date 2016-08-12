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

#include <ATKELinkI.H>
#include <iostream>

using namespace std;

void ATKELink::ProcessArgument(char *arg) {
    // massage the argument list for use in the link commnad
    InsertArguments(arg);
}

int ATKELink::ArgumentsDone() {
    // perform any processing on the completed argument list needed before the
    // link command is run.
    return 0;
} 

int ATKELink::PostProcess() {
    // perform any processing needed after the executable has been linked.
    // This could include linking again...
    return 0;
}

int main(int argc, char **argv) {
    ATKELinkI adl;
    argc--;
    argv++;
    for(int i=0;i<argc;i++) {
	adl.ProcessArgument(argv[i]);
    }
    int result=adl.ArgumentsDone();
    if(result!=0) {
	cerr<<"aexelink: Argument processing failed with error code "<<result<<endl;
	exit(-1);
    }
    char *cmd=adl.SystemString();
    if(cmd==NULL) {
	cerr<<"aexelink: Couldn't create command to link dynamic object."<<endl;
	exit(-1);
    }
    result=system(cmd);
    if(result!=0) {
	cerr<<"aexelink: Link command failed with error code "<<result<<"."<<endl;
	exit(-1);
    }
    result=adl.PostProcess();
    if(result!=0) {
	cerr<<"aexelink: Link post-processing failed with error code "<<result<<"."<<endl;
	exit(-1);
    }
    return 0;
}
   
    
	
