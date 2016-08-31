/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

class pvalbv : pvalsbv {
classprocedures:
    InitializeObject(struct pvalbv *self) returns boolean;
    FinalizeObject(struct pvalbv *self);
overrides:
    Hit (enum view_MouseAction action, long x, long y, long numberOfClicks) returns struct view *;
    UpdateSButton();
data:
    int activated;
};

