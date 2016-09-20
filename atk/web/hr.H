#ifndef _hr_H_
#define _hr_H_
/** \addtogroup libhtmltext
 * @{ */
/* File hr.H created by Ward Nelson
     (c) Copyright IBM Corp 1995.  All rights reserved. 

   hr, an inset that draws and prints a window-width horizontal line */

#include <dataobject.H>
class hr : public dataobject {
  public:
    virtual ATKregistryEntry *ATKregistry();

    const char *ViewName();
};
/** @} */
#endif /* _hr_H_ */
