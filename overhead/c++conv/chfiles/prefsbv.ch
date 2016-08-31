/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

class prefsbv: sbuttonv {
  classprocedures:
    InitializeObject(struct sbutton *self) returns boolean;
    FinalizeObject(struct sbutton *self);
  overrides:
    Touch(int ind, enum view_MouseAction act) returns boolean;
 };

