/* Copyright 1994 Carnegie Mellon University All rights reserved. */

 // ATKDynImpl: the system specific code to adapt the link line for a dynamic object.

#include <ATKOLinkI.H>
#include <iostream>

using namespace std;

#include <ATKDoLoad.H>

ATKOLinkI::ATKOLinkI() {
}

ATKOLinkI::~ATKOLinkI() {
}

void ATKOLinkI::ProcessArgument(char *arg) {
    char buf[MAXPATHLEN];
    if(strlen(arg)>sizeof(buf)-4) {
	cerr<<"Argument too long, ignored..."<<endl<<arg<<endl;
	return;
    }
    if(arg[0]=='-') switch(arg[1]) {
	case 'D':
	case 'I':
	case 'g':
	    return;
	default:
	    break;
    }

    InsertArguments(arg);
}

void ATKOLinkI::ArgumentsDone() {
}

int ATKOLinkI::Test() {
#if 0
    char *p;

    /* since this process doesn't have all the needed symbols we can't test the object just by loading it.  Probably the only real test would be to do a static link of all the objects and libraries (including crt0.o).... -rr2b */
    ResetIterator();
    while((p=NextArgument())) {
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
#endif
    return 0;
}

