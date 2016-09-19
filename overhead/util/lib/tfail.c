/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
	tfail.c
	tfail() -- Return a Boolean indicating whether the errno value represents a temporary failure.
*/

#include <andrewos.h>
#include <stdio.h>
#include <errno.h>
#include <util.h>

int tfail(int errorNumber)
{
/* Returns a Boolean indicating whether the errno value errorNumber is probably a temporary failure condition, i.e., one that might succeed if tried again later.  Returns 1 (true) on a temporary failure, 0 (false) on a permanent failure.
Admittedly, for most of the error conditions described, we can make only a guess about the temporary-ness of an error (EIO? EMFILE? EROFS? EMLINK?), but this is only a rough guess.
*/

    switch (errorNumber) {
	case EINTR:
	case EIO:
	case EAGAIN:
	case ENOMEM:
	case ENODEV:
	case ENFILE:
#ifdef ETXTBSY
	case ETXTBSY:
#endif
	case ENOSPC:
	case ENETDOWN:
	case ENETUNREACH:
	case ENETRESET:
	case ECONNABORTED:
	case ECONNRESET:
	case ENOBUFS:
	case ESHUTDOWN:
	case ETIMEDOUT:
	case ECONNREFUSED:
	case EHOSTDOWN:
	case EHOSTUNREACH:
#ifdef EDQUOT
	case EDQUOT:
#endif /* EDQUOT */
	    return 1;		/* temporary failures */
	default:
	    return 0;		/* all others are permanent failures */
    }
}
