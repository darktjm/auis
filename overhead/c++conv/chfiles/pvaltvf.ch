/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/pvaltvf.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/pvaltvf.ch,v $ */

class pvaltvf : pvaltvl {
classprocedures:
    InitializeClass() returns boolean;
    InitializeObject(struct pvaltvf *self) returns boolean;
    FinalizeObject(struct pvaltvf *self);
methods:
overrides:
    Keys() returns struct keystate *;
macromethods:
data:
    struct keystate *ks;
    struct menulist *menulist;
};
