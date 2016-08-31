/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

class pvalcv : pvalsbv {
classprocedures:
    InitializeObject(struct pvalcv *self) returns boolean;
    FinalizeObject(struct pvalcv *self);
overrides:
    UpdateSButton();
};

