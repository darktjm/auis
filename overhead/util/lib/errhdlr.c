/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>		/* <strings.h> */
#include <util.h>
#include <errno.h>

char EH_Error_Msg[EH_ERR_MSG_BUF_SIZE];
EH_environment *_error_handler_env;

void *emalloc(long size)
{
  /* Returns a pointer to a hunk of memory of size. 
     Errors are signalled. */
  void *tmp_ptr;

  EH_cond_error_on(!(tmp_ptr = malloc(size)),
		   EH_ret_code(EH_module_system,ENOMEM),
		   "Malloc failed");
  return(tmp_ptr);
}

void *erealloc(void *old, long newsize)
{
  /* Returns a pointer to a hunk of memory of newsize, with
     the contents of old (trunc) copied into it. 
     Errors are signalled.  */
  void *tmp_ptr;

  EH_cond_error_on(!(tmp_ptr = realloc(old, newsize)),
		   EH_ret_code(EH_module_system,ENOMEM),
		   "Realloc failed");
  return(tmp_ptr);
}
