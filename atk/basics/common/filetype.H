#ifndef _filetype_H_
#define _filetype_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
/* filetype.h
 * Declarations for the filetype package.
 *
 */

#include <stdio.h>
#include <ATK.H>

class filetype : public ATK {
  public:
    virtual ATKregistryEntry *ATKregistry();

    static void AddEntry(const char *extesion, const char *dataName, const char *attributes);
    static void AddExistingAttributes(const char *extension, const char *dataName, const char *attributes);
    static int DeleteEntry(char *extension); /* The return value here is dubious... */
    static void FreeAttributes(struct attributes *attributes);
    static struct attributes *ParseAttributes(const char *attributes);

    /* NOTE: filetype_Lookup.
     *    The value returned through the attributes parameter to this function
     *    contains pointers to static storage. Its value should be used before
     *    the next call to this routine. If this can't be guaranteed, the
     *    programmer must copy the attributes list.
     */
    static char *Lookup(FILE *file, const char *filename, /* OUT parameter */ long *objectID,
			/* OUT parameter */ struct attributes **attributes);

    static void CanonicalizeFilename(char *canonicalName, const char *name, int maxSize);
};
/** @} */
#endif /* _filetype_H_ */
