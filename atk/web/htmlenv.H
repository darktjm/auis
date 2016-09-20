#ifndef _htmlenv_H_
#define _htmlenv_H_
/** \addtogroup libhtmltext
 * @{ */
/* File htmlenv.H created by Ward Nelson
      (c) Copyright IBM Corp 1995.  All rights reserved.

   htmlenv, an environment that can also store a list of HTML+ tag attributes */

#include <environment.H>
class htmlenv : public environment {
  public:
    virtual ATKregistryEntry *ATKregistry();

    virtual void ClearAttribs();
    virtual void SetAttribs(class attlist *atts);
    virtual const char *GetAttribValue(const char *name);
    class nestedmark *NewButSimilar();
    class environment *Wrap(long pos, long length, 
		enum environmenttype type, 
		union environmentcontents data);
    static htmlenv *GetTextRootEnvironment(class text *txt);
    inline class attlist *GetAttribs() { return ((this)->attribs); }

    htmlenv();
    virtual ~htmlenv();

    class attlist* attribs;
};
/** @} */
#endif /* _htmlenv_H_ */
