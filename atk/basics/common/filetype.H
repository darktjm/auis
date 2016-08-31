ATK_INTER
#ifndef _filetype_H_
#define _filetype_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


 

/* filetype.h
 * Declarations for the filetype package.
 *
 */


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

#endif /* _filetype_H_ */
