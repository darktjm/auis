/* Copyright 1994 Carnegie Mellon University All rights reserved. */

 // ATKDynImpl: the system specific code to adapt the link line for a dynamic object.

#include <ATKELinkI.H>

ATKELinkI::ATKELinkI() {
}

ATKELinkI::~ATKELinkI() {
}

void ATKELinkI::ProcessArgument(char *arg) {
    InsertArguments(arg);
}

int ATKELinkI::ArgumentsDone() {
    return 0;
    
}
