/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
	uerror.c -- Return a static string describing an errno value.
*/


#include <andrewos.h>
#include <util.h> 

#include <string.h>
#include <errno.h>
#ifndef NULL
#define NULL (char *) 0
#endif

const char *UnixError(int errorNumber)
{
/* Returns a pointer to a static buffer containing English text describing the same error condition that errorNumber describes (interpreted as a Unix error number).  The text has no newlines in it.  We contend that this is what ``perror'' should have been returning all along. */
    return strerror(errorNumber);
}
