/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

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
