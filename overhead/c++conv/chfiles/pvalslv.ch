/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/pvalslv.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/pvalslv.ch,v $ */

class pvalslv : ssliderv {
classprocedures:
    InitializeObject(struct pvalslv *self) returns boolean;
    FinalizeObject(struct pvalslv *self);
overrides:
    WhatIsAt(long num, long denom) returns long;
    Endzone(int end, enum view_MouseAction act);
    SetFrame(long pos, long num, long denom);
    GetInfo(struct range *total, struct range *seen, struct range *dot);
macromethods:
macros:
data:
};
