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


/* These routines are in an include file instead of a libary because they
 * depend on the internal structure of the doload_environment which may or
 * may not be the same between various machine types. In short, it would
 * have required rewriting more code than I have time to in order to do
 * this right. This solution gets source code sharing if nothing else...
 * Also this way, the safe routines are not exposedas externals in doload.
 */

static void doload_punt(struct doload_environment *e, char *message)
{
    fprintf(stderr, "doload:  %s\n", message);
    fflush(stderr);
    longjmp(e->errorJump, 1);
    /*NOTREACHED*/
}

static char *safe_malloc(struct doload_environment *e, long size)
{
    char *result;
    
    result = (char *)malloc((unsigned)size);
    if (result == NULL)
	doload_punt(e, "insufficient memory");
    return result;
}

static char *safe_realloc(struct doload_environment *e, char *ptr, long size)
{
    char *result;

    if (ptr == NULL)
	result = (char *)malloc((unsigned)size);
    else
	result = (char *)realloc(ptr, (unsigned)size);
    if (result == NULL)
	doload_punt(e, "insufficient memory");
    return result;
}

/* Not static since this is used by dofix. */
static void safe_free(char *thing)
{
    if (thing != NULL)
	free(thing);
    return ;
}

static safe_read(struct doload_environment *e, char *thing, long size)
{	
    if ((unsigned)read(e->fd, thing, (int)size) < size)
	doload_punt(e, "incomplete load file");
}

/* Not static since this is used by dofix. */
static void safe_write(struct doload_environment *e, int fd, char *thing, long size)
{
    int n;

    n = write(fd, thing, (int)size);
    if (n < size) {
	perror("dofix");
	doload_punt(e, "problems writing .do file");
    }
    return;
}

static safe_lseek(struct doload_environment *e, long offset, int how)
{	
    if (lseek(e->fd, offset, how) == -1)
	doload_punt(e, "seek failed");
}
