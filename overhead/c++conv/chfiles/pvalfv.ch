/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

class pvalfv : pvalsbv {
classprocedures:
    InitializeObject(struct pvalfv *self) returns boolean;
    FinalizeObject(struct pvalfv *self);
overrides:
    UpdateSButton();
};

