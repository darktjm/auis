/* Copyright 1994, Carnegie Mellon University, All rights reserved. */

// ATKELink: the machine independent part of constructing a link line for base executables which will load dynamic objects.

#include <ATKLink.H>

class ATKELink : public ATKLink {
  public:
    virtual void ProcessArgument(char *arg);
    virtual int ArgumentsDone();
    virtual int PostProcess();
};
