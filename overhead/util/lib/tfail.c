/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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
