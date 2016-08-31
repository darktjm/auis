/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

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

