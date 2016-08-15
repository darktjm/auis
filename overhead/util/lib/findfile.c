/* File findfile.c created by Myrl Youngman on Fri Aug  4 1995.
*/
/* $Disclaimer: 
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
 *  $ */
 
#include <andrewos.h>    /* types */
#include <util.h>	/* NewString */

#ifdef DOS_STYLE_PATHNAMES
#define PATH_DELIMITER ';'
#define DIR_SEPARATOR '\\'
#else
#define PATH_DELIMITER ':'
#define DIR_SEPARATOR '/'
#endif

/* findFileInPath: Searches thru each dir in the init file
 * path (ifp) for a file (fname). 
 * Returns: Full path name of the init file or NULL if not
 *          found.
 */
/* tjm: bugs:
 *   doesn't allow spaces in paths (fixed)
 *   doesn't skip empty elements correctly (fixed)
 *   creates memory duplicate of ifpath for no good reason (fixed)
 *     frees that copy from address in its middle (fixed, obviously)
 *   copies out path elements into separate buffer (fixed)
 *   copies result into unsized output buffer (not fixed)
 */
void findfileinpath(char *retbuf, const char *ifpath, const char *fname)
{
    const char *tp, *ip;

    tp = ifpath;

    do {
	while(/* (*tp == ' ') || */ (*tp == PATH_DELIMITER)) tp++;
	ip = tp;
	while(/* (*ip != ' ') && */ (*ip != PATH_DELIMITER) && (*ip != '\0'))
	    ip++;
	sprintf(retbuf, "%.*s%c%s", (int)(ip - tp), tp, DIR_SEPARATOR, fname);
	if (!access(retbuf,R_OK)) {	/* file found */
	    return;
	}
	tp = ++ip;
   } while (*ip != '\0');
   /* If we got here, the file was not found */
   *retbuf = '\0';
   return;
}


