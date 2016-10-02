/* File findfile.c created by Myrl Youngman on Fri Aug  4 1995.
*/
 
#include <andrewos.h>    /* types */
#include <util.h>	/* strdup */

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


