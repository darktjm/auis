/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/pvaltvfc.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/pvaltvfc.ch,v $ */

class pvaltvfc : pvaltvc {
classprocedures:
    InitializeClass() returns boolean;
    InitializeObject(struct pvaltvfc *self) returns boolean;
    FinalizeObject(struct pvaltvfc *self);
methods:
overrides:
    Select(int ind);
macromethods:
data:
};
