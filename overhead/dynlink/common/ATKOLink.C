/* Copyright 1994, Carnegie Mellon University, All rights reserved. */

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
    
    
	
