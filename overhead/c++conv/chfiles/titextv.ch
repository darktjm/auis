/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

class titextv : textview [textv] {
classprocedures:
    InitializeObject(struct textintv *self) returns boolean;
    FinalizeObject(struct textintv *self);
overrides:
    Update();
macromethods:
    SetTextIntv(v) (self->intv=(v))
    GetTextIntv() (self->intv)
data:
   struct textintv *intv;
   long lastpos;
};
