/* Copyright 1994, Carnegie Mellon University, All rights reserved. */

// ATKOLink: the machine independent part of constructing a link line for dynamic objects.

#include <ATKLink.H>

class ATKOLink : public ATKLink {
  public:
    virtual void ProcessArgument(char *arg);
    virtual void ArgumentsDone();
    virtual int Test();
};
