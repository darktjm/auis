/* Copyright  Carnegie Mellon University 1995 -- All rights reserved

"C" interface to sockets for web code

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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>

#include <webparms.h>

	void 
webaccept(f, viewforclient)
	FILE  *f;
	void (*viewforclient)();
{
	char c;
	FILE *fp, *ofp;
	int fromlen, cnt;
	char hostname[64];
	struct hostent *hp;
	char buf[2560], *cp;
	int i,  s,  ns,  len;
	struct sockaddr_un saun,  fsaun;

	s = fileno(f);
/*	im::RemoveFileHandler(f);		??? WHY ??? xxx  */

	ns = accept(s,  &fsaun,  &fromlen);
	if (ns < 0) {
		perror("server: accept");
		return;  
	}
/*	unlink(ADDRESS);		??? WHY ??? xxx  */

	/*
	 * We'll use stdio for reading the socket.
	 */
	fp = fdopen(ns,  "r");

	/*
	 * Then we read some strings from the client
	 * and print them out.
	 */
	printf("waiting to read\n"); fflush(stdout);

	for(cp = buf, cnt = 0; ; cp++){
		int ch = fgetc(fp);
		if (ch == EOF) break;
		*cp = ch;
		putchar(*cp);fflush(stdout);
		if (*cp == '\n'){
			*cp = '\0';
			if ( cp == buf || *(cp -1) == '\0')
				break;
			cnt++;
		}
	}
	*cp = '\0';
	printf("Got here %s %d \n", buf, cnt);

/*	weblisten();		 ??? WHY ??? xxx  
		(can't do this here because 
		need to call im::AddFileHandler)
*/

	(*viewforclient)(buf);

	/*
	 * We can simply use close() to terminate the
	 * connection,  since we're done with both sides.
	 */
	close(s);
}

	int 
weblisten() {
	char c;
	FILE *fp, *ofp;
	int fromlen, cnt;
	char hostname[64];
	struct hostent *hp;
	char buf[2560], *cp;
	int i,  s,  ns,  len;
	struct sockaddr_un saun,  fsaun;

	/*
	 * Get a socket to work with.  This socket will
	 * be in the UNIX domain,  and will be a
	 * stream socket.
	 */
	if ((s = socket(AF_UNIX,  SOCK_STREAM,  0)) < 0) {
		perror("server: socket");
		return(1);
	}

	/*
	 * Create the address we will be binding to.
	 */
	saun.sun_family = AF_UNIX;
	strcpy(saun.sun_path,  ADDRESS);

	/*
	 * Try to bind the address to the socket.  We
	 * unlink the name first so that the bind won't
	 * fail.
	 *
	 * The third argument indicates the "length" of
	 * the structure,  not just the length of the
	 * socket name.
	 */
	unlink(ADDRESS);
	len = sizeof(saun.sun_family) + strlen(saun.sun_path) + 1;

	if (bind(s,  &saun,  len) < 0) {
		perror("server: bind");
		return(1);
	}

	/*
	 * Listen on the socket.
	 */
	if (listen(s,  5) < 0) {
		perror("server: listen");
		return(1);
	}

	/*
	 * Accept connections.  When we accept one,  ns
	 * will be connected to the client.  fsaun will
	 * contain the address of the client.
	 */
	return s;
}

	int 
webclient(char  *buf, int  timeout, char  *rbuf, int  size)  {
	char c, *eb;
	FILE *fp, *ofp;
	int i,  s,  len, res, del;
	struct sockaddr_un saun;
	
	/*
	 * Get a socket to work with.  This socket will
	 * be in the UNIX domain,  and will be a
	 * stream socket.
	 */
	for(i = 0; 1; i++) {
		s = socket(AF_UNIX,  SOCK_STREAM,  0);
		if (s >= 0) break;
		if (i == timeout)
			return(-1);
		sleep(1);
	}

	/*
	 * Create the address we will be connecting to.
	 */
	saun.sun_family = AF_UNIX;
	strcpy(saun.sun_path,  ADDRESS);

	/*
	 * Try to connect to the address.  For this to
	 * succeed,  the server must already have bound
	 * this address,  and must have issued a listen()
	 * request.
	 *
	 * The third argument indicates the "length" of
	 * the structure,  not just the length of the
	 * socket name.
	 */
	len = sizeof(saun.sun_family) + strlen(saun.sun_path) + 1;
	i = i * 100;
	del = 0;
	for(i = 0; 1; i++) {
		res = connect(s,  &saun,  len);
		if (res >= 0) break;
		if (i == timeout)
			return (-2);
		/*printf("connect res = %d %d\n", res, errno); */
		usleep(10000);
		del++;
	}

	/*
	 * We'll use stdio for reading
	 * the socket.
	 */
	ofp = fdopen(s,  "w");
	fwrite(buf, 1, strlen(buf), ofp);
	fwrite("\n\n", 2, 1, ofp);
	fflush(ofp);
	/*
	 * We can simply use close() to terminate the
	 * connection,  since we're done with both sides.
	 */
	close(s);
	return del;
}
