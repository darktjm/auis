#ifndef _htmlenv_H_
#define _htmlenv_H_
/* File htmlenv.H created by Ward Nelson
      (c) Copyright IBM Corp 1995.  All rights reserved.

   htmlenv, an environment that can also store a list of HTML+ tag attributes */

#include <environment.H>
class htmlenv : public environment {
  public:
    virtual ATKregistryEntry *ATKregistry();

    virtual void ClearAttribs();
    virtual void SetAttribs(class attlist *atts);
    virtual char *GetAttribValue(char *name);
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

static char *skipwhitespace(char *s)
{
    while (*s && isspace(*s)) ++s;
    return s;
}

static char *skiptokenchars(char *s)
{
    while (*s && !isspace(*s) && *s != '=') ++s;
    return s;
}

#endif /* _htmlenv_H_ */