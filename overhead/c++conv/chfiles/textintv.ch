/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

class textintv : wrapv {
classprocedures:
    InitializeObject(struct textintv *self) returns boolean;
    FinalizeObject(struct textintv *self);
macromethods:
    GetTextView() ((struct textview *)((struct wrapv *)self)->tv)
    GetText() ((struct text *)((struct wrapv *)self)->t)
methods:
    SetDotPosition(long pos);
data:
};
