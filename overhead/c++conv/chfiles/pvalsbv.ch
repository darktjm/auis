/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/pvalsbv.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ */
/* $ACIS:eza.ch 1.4$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/pvalsbv.ch,v $ */

class pvalsbv : wrapv {
classprocedures:
    InitializeObject(struct pvalsbv *self) returns boolean;
    FinalizeObject(struct pvalsbv *self);
overrides:
    ObservedChanged(struct observable *changed, long val);
    SetDataObject(struct dataobject *prefval);
macromethods:
    GetSButtonView() ((struct sbuttonv *)((struct wrapv *)self)->tv)
    GetSButton() ((struct sbutton *)((struct wrapv *)self)->t)
methods:
    UpdateSButton();
};

