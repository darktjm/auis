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

/*
   A nop snap client, just created to test authentication
*/

#include <andrewos.h> /* sys/types.h sys/time.h */
#include <stdio.h>
#include <netinet/in.h>
#include <gasp.h>
#include <snap.h>

extern int SNAP_debugmask;
main(argc, argv)
int argc;
char *argv[];
{
    char *client;
    int clientfd;
    SNAP_CPARMS connparms;
    int auth;
    int rc;

    connparms.maxtime = 30;
    connparms.timeout = 1;
    connparms.maxmsgsize = 1000;
    connparms.encryptlevel = SNAP_ENCRYPT;
    rc=GASP_ServerInit(argc, argv, &connparms, &client, &clientfd, &auth);
    if(rc!=0) {
	printf ("snapfakeclient:GASP_ServerInit failed %d\n",rc);
	exit(1);
    }
    {
	char *buffer;
	int msgtype;
	int cid;
	rc=SNAP_Accept(&buffer,&msgtype,&cid,50);
	if(rc<0) {
	    printf("snapfakeclient:accept failed %d",rc);
	    exit(1);
	}
    }
    exit(0);
}
