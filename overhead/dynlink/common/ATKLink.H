/* Copyright 1994 Carnegie Mellon University All rights reserved. */

/** \addtogroup libATKLink libATKLink.a
 * @{ */

// ATKLink: a base class for classes manipulating link commands.

#include "ATKArgv.H"

class ATKLink : public ATKArgv {
  public:
    int FirstObjectFile();
    int LastObjectFile();
};

/** @} */
