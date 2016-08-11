/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/


 

/* filetype.h
 * Declarations for the filetype package.
 *
 */


package filetype {
    classprocedures:
        AddEntry(char *extesion, char *dataName, char *attributes);
        AddExistingAttributes(char *extension, char *dataName, char *attributes);
        DeleteEntry(char *extension) returns int; /* The return value here is dubious... */
/* NOTE: filetype_Lookup.
 *    The value returned through the attributes parameter to this function
 *    contains pointers to static storage. Its value should be used before
 *    the next call to this routine. If this can't be guaranteed, the
 *    programmer must copy the attributes list.
 */
        Lookup(FILE *file, char *filename, /* OUT parameter */ long *objectID,
              /* OUT parameter */ struct attributes **attributes) returns char *;
        CanonicalizeFilename(char *canonicalName, char *name, int maxSize);
};
