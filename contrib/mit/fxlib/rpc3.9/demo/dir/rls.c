/* @(#)rls.c	1.1 87/11/04 3.9 RPCSRC */

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/rpc3.9/demo/dir/RCS/rls.c,v 1.3 1992/12/15 21:53:21 rr2b Stab74 $";
#endif

/*
 * rls.c: Remote directory listing client
 */
#include <stdio.h>
#include <rpc/rpc.h>		/* always need this */
#include "dir.h"		/* need this too: will be generated by rpcgen*/

main(argc, argv)
	int argc;
	char *argv[];
{
	CLIENT *cl;
	char *server;
	char *dir;
	readdir_res *result;
	namelist nl;
	

	if (argc != 3) {
		fprintf(stderr, "usage: %s host directory\n", argv[0]);
		exit(1);
	}

	/*
	 * Remember what our command line arguments refer to
	 */
	server = argv[1];
	dir = argv[2];

	/*
	 * Create client "handle" used for calling MESSAGEPROG on the
	 * server designated on the command line. We tell the rpc package
	 * to use the "tcp" protocol when contacting the server.
	 */
	cl = clnt_create(server, DIRPROG, DIRVERS, "tcp");
	if (cl == NULL) {
		/*
		 * Couldn't establish connection with server.
		 * Print error message and die.
		 */
		clnt_pcreateerror(server);
		exit(1);
	}
	
	/*
	 * Call the remote procedure "readdir" on the server
	 */
	result = readdir_1(&dir, cl);
	if (result == NULL) {
		/*
		 * An error occurred while calling the server. 
	 	 * Print error message and die.
		 */
		clnt_perror(cl, server);
		exit(1);
	}

	/*
	 * Okay, we successfully called the remote procedure.
	 */
	if (result->errno != 0) {
		/*
		 * A remote system error occurred.
		 * Print error message and die.
		 */
		errno = result->errno;
		perror(dir);
		exit(1);
	}

	/*
	 * Successfuly got a directory listing.
	 * Print it out.
	 */
	for (nl = result->readdir_res_u.list; nl != NULL; nl = nl->next) {
		printf("%s\n", nl->name);
	}
}
