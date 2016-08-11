/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/prefsbv.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/prefsbv.ch,v $ */

class prefsbv: sbuttonv {
  classprocedures:
    InitializeObject(struct sbutton *self) returns boolean;
    FinalizeObject(struct sbutton *self);
  overrides:
    Touch(int ind, enum view_MouseAction act) returns boolean;
 };

