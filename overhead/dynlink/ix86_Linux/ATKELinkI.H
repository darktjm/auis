/* Copyright 1994 Carnegie Mellon University All rights reserved. */

 // ATKDynImpl: the system specific code to adapt the link line for a dynamic object.

#include <ATKELink.H>
#include <ATKSymTab.H>

class ATKELinkI : public ATKELink {
  private:
  public:
    virtual void ProcessArgument(char *arg);
    virtual int ArgumentsDone();
    ATKELinkI();
    virtual ~ATKELinkI();
};

