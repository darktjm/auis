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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/mail/lib/RCS/sysexits.c,v 2.6 1992/12/15 21:04:17 rr2b Stab74 $";
#endif

/*
		sysexits.c -- Text messages for sysexits.
*/


char *EX_Messages[] = {

/*			 	 1234567891123456789212345	*/

	/* EX_USAGE */		"Command line usage error",
	/* EX_DATAERR */	"Data format error",
	/* EX_NOINPUT */	"Cannot open input",
	/* EX_NOUSER */		"Unknown recipient",
	/* EX_NOHOST */		"Unknown host",
	/* EX_UNAVAILABLE */	"Service unavailable",
	/* EX_SOFTWARE */	"Internal software error",
	/* EX_OSERR */		"Operating system error",
	/* EX_OSFILE */		"System file missing",
	/* EX_CANTCREAT */	"Can't create file",
	/* EX_IOERR */		"I/O error",
	/* EX_TEMPFAIL */	"Temporary failure",
	/* EX_PROTOCOL */	"Remote protocol error",
	/* EX_NOPERM */		"Permission denied"

};

int EX_Nerr = (sizeof EX_Messages / sizeof EX_Messages[0]);
