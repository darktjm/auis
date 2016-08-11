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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/errhdlr.c,v 1.7 1993/05/19 23:46:56 gk5g Stab74 $";
#endif

#include <andrewos.h>		/* <strings.h> */
#include <util.h>
#include <errno.h>

char EH_Error_Msg[EH_ERR_MSG_BUF_SIZE];
EH_environment *_error_handler_env;

char *emalloc(size)
long size;
{
  /* Returns a pointer to a hunk of memory of size. 
     Errors are signalled. */
  char *tmp_ptr;

  EH_cond_error_on(!(tmp_ptr = (char *)malloc(size)),
		   EH_ret_code(EH_module_system,ENOMEM),
		   "Malloc failed");
  return(tmp_ptr);
}

char *erealloc(old, newsize)
char *old;
long newsize;
{
  /* Returns a pointer to a hunk of memory of newsize, with
     the contents of old (trunc) copied into it. 
     Errors are signalled.  */
  char *tmp_ptr;

  EH_cond_error_on(!(tmp_ptr = (char *)realloc(old, newsize)),
		   EH_ret_code(EH_module_system,ENOMEM),
		   "Realloc failed");
  return(tmp_ptr);
}


char *CopyString(old)
char *old;
{
  /* Returns a newly emalloc'd copy of the string.
     Errors from emalloc are passed up. */

  return(strcpy(emalloc(strlen(old)+1), old));
}

