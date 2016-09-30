#ifndef _attlist_H_
#define _attlist_H_
/* File attlist.H created by Ward Nelson
   (c) Copyright IBM Corp 1995.  All rights reserved.  */

#include <andrewos.h>
/** \addtogroup libhtmltext
 * @{ */
struct htmlatts {
    const char *name;
    const char *value;
    boolean quoted;
};

#include <list.H>
class attlist : public list {
    unsigned long refs;
  public:
    virtual ATKregistryEntry *ATKregistry();

    virtual void DebugPrint();
    virtual void AddAttribute(const char *name, const char *value, boolean quoted);
    virtual struct htmlatts *GetAttribute(const char *name);
		// returns null if attr not found
    virtual void ClearAttributes();
    virtual void MakeFromString(const char *str);
    virtual char *MakeIntoString();
    virtual class attlist *CopyAttributes();

    attlist();
    virtual ~attlist();
    static boolean InitializeClass();

    static class attlist * ParseAttributes(const char *str);
    static void Copy(class attlist *dst, class attlist *src);
    inline void Reference() {
	refs++;
    }
    inline void Dereference() {
	refs--;
    }
    inline unsigned long ReferenceCount() {
	return refs;
    }
    virtual void Destroy();
};


/* cistrcmp is a case-insensitive strcmp, used mainly to match up attribute names (which are not case-sensitive) */
#define cistrcmp strcasecmp
/** @} */
#endif /* _attlist_H_ */
