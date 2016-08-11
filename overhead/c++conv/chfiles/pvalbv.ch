/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/pvalbv.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/pvalbv.ch,v $ */

class pvalbv : pvalsbv {
classprocedures:
    InitializeObject(struct pvalbv *self) returns boolean;
    FinalizeObject(struct pvalbv *self);
overrides:
    Hit (enum view_MouseAction action, long x, long y, long numberOfClicks) returns struct view *;
    UpdateSButton();
data:
    int activated;
};

