#ifndef _hidden_H_
#define _hidden_H_
/** \addtogroup libhtmltext
 * @{ */
/* File hidden.H created by Ward Nelson
      (c) Copyright IBM Corp 1995.  All rights reserved. 

   hidden, an inset for stashing non-displayed text inside of */

enum visibility { hidden_HIDDEN, hidden_EXPOSED, hidden_FULLSCREEN };

#include <htmltext.H>
class hidden : public htmltext {
  public:
    virtual ATKregistryEntry *ATKregistry();

    const char *ViewName();
    inline boolean IsVisible() { return ((this)->Visibility != hidden_HIDDEN); }
    inline boolean IsFullScreen() { return ((this)->Visibility == hidden_FULLSCREEN); }
    inline void SetVisibility(enum visibility v) { ((this)->Visibility = ((this)->Visibility == hidden_FULLSCREEN)?hidden_FULLSCREEN:v); }

    hidden();
    virtual ~hidden();
    static boolean InitializeClass();

    enum visibility Visibility;
};
/** @} */
#endif /* _hidden_H_ */
