/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

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
