/* Copyright 1994 Carnegie Mellon University All rights reserved. */

 // ATKDynImpl: the system specific code to adapt the link line for a dynamic object.

#include <ATKOLink.H>
#include <ATKSymTab.H>

class ATKOLinkI : public ATKOLink {
  private:
  public:
    virtual void ProcessArgument(char *arg);
    virtual void ArgumentsDone();
    virtual int Test();
    ATKOLinkI();
    ~ATKOLinkI();
};

