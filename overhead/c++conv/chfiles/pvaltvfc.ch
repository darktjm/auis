/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

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
