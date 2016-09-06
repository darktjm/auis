/* Copyright 1993 Carnegie Mellon University All rights reserved.
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
#include <netdb.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>

#include "config.h"
#include <helpsys.h>

#include <util.h>

#include "helpapp.h"

#ifdef MAXHOSTNAMELEN
#undef MAXHOSTNAMELEN
#endif /* MAXHOSTNAMELEN */
#define MAXHOSTNAMELEN 64	/* some people just don't */

#define	IPPORT_HELPNAME	"andrewhelp"

int helpSocket=(-1);
int helpPort=(-1);
extern struct helpDir *helpsearchDirs;
extern int help_newWindow;
extern char *help_indexName;
extern char *help_aliasName;
extern char *help_helpKey;
static struct servent *ts;
static struct hostent *th;
static struct sockaddr_in myaddr;
/*
 * send_pack: send a command packet to an existing help instance
 */
static int 
send_pack(
char  c,				/* the command char */
char  *s,			/* the string to send */
int  sock)			/* the socket to send to */
 {
     long len;
    /* buf needs to be an array of longs so that it will be
     * properly aligned for architectures like the HP-PA
     * that insist that longs be 4-byte aligned
     */
     long buf[(MAXPATHLEN + 1 + 2*sizeof(long))/sizeof(long)];

    /* length = string + NULL + sizeof(length) + command char */
     len = strlen(s) + sizeof(long) + 2;
     buf[0] = htonl(len);
     sprintf(((char *)buf)+sizeof(long), "%c%s", c, s);
     return write(sock, ((char *)buf), len);
 }

char *ncproc_AddSearchDir=NULL;
char *ncproc_SetIndex=NULL;
char *ncproc_SetAliasesFile=NULL;
char *ncproc_GetHelpOn=NULL;
/*
 * start up and start accepting connections and commands from other helps
 */
void 
helpapp_ncproc (void)
{
    int ns;
    struct sockaddr_in helpaddr;
    socklen_t addrlen;
    char buf[MAXPATHLEN+1];
    long len;
/* 
    char errbuf[HNSIZE + 50];
		help_SetAliasesFile(buf+1); 
		((help::GetInstance())->GetIM())->ExposeWindow(); 
		sprintf(errbuf, error, buf+1);
		help::HelpappGetHelpOn(buf+1, help_NEW, help_HIST_NAME, errbuf);*/
    ncproc_AddSearchDir=NULL;
    ncproc_SetIndex=NULL;
    ncproc_SetAliasesFile=NULL;
    ncproc_GetHelpOn=NULL;
    addrlen = sizeof(helpaddr);
    ns = accept(helpSocket, (struct sockaddr *)&helpaddr, &addrlen);
    if (ns >= 0) {
	while(1) {
	    addrlen = read(ns, &len, sizeof(long));
	    if (addrlen <= 0) {
		close(ns);
		return;
	    }
	    len = ntohl(len);
	    addrlen = read(ns, buf, len);
	    switch(buf[0]) {
		case 's':		/* add a search dir */
		    ncproc_AddSearchDir=NewString(buf+1);
		break;
	    case 'i':		/* change the index */
		ncproc_SetIndex=NewString(buf+1);
		break;
	    case 'a':		/* add an alias file */
		ncproc_SetAliasesFile=NewString(buf+1);
		break;
	    case 'h':		/* get help on a topic */
		ncproc_GetHelpOn=NewString(buf+1);
		break;
	    }
	}
    }
}

int 
help_unique(void)
{
    int i;
    char *wmHost = NULL, *dpyHost = NULL, displayHost[MAXHOSTNAMELEN], *colon;
    int runningLocally = 0;
    
    if(helpPort==-1) {
/* lookup the help port, default to the 'right' thing */
	if ((ts = getservbyname(IPPORT_HELPNAME, "tcp")) != (struct servent *)NULL)
	    helpPort = ts->s_port;
	else
	    helpPort = htons(HELPSOCK);
    }
    
#ifdef X11_ENV
    wmHost = (char *)malloc(MAXHOSTNAMELEN * sizeof(char));
    gethostname(wmHost, MAXHOSTNAMELEN);
    if((dpyHost = getenv("DISPLAY"))) {
	strcpy(displayHost, dpyHost);
	if (*displayHost && (colon = strchr(displayHost, ':')) != NULL) {
	    *colon = (char)0;
	    if(colon == displayHost || /* colon but no hostname */
	       strcmp(displayHost, wmHost) == 0 ||
	       strcmp(displayHost, "unix") == 0)
		runningLocally = 1;
	    else {
		/* here we might want to look for an existing help server that is servicing $DISPLAY */
		/* this case occurs when you have help running on another host and have your DISPLAY set back to where you are sitting; how do we determine if there is already a help proc for DISPLAY? */
		/* without doing anything special here, it'll bring up a new help window */
	    }
	    runningLocally = 1;
	    *colon = ':';
	}
    }
#endif /* X11_ENV */
    if (!help_newWindow && runningLocally) {
	/* see if we can find a help server already */
	th = gethostbyname(wmHost);
	if (th != NULL) {
	    memmove(&myaddr.sin_addr.s_addr, th->h_addr, sizeof(long));
	    myaddr.sin_family = AF_INET;
	    myaddr.sin_port = helpPort;
	    helpSocket = socket(AF_INET, SOCK_STREAM, 0);
	    if(helpSocket>=0) {
		i = connect(helpSocket, (struct sockaddr *)&myaddr, sizeof(myaddr));
		if (i >= 0) {
		    struct helpDir *thd, *nhd;

		    for (thd = helpsearchDirs; thd; thd = nhd) {
			send_pack('s', thd->dirName, helpSocket);
			nhd = thd->next;
			free(thd->dirName);
			free(thd);
		    }
		    if (help_indexName != NULL)
			send_pack('i', help_indexName, helpSocket);
		    if (help_aliasName != NULL)
			send_pack('a', help_aliasName, helpSocket);
		    send_pack('h', help_helpKey, helpSocket);
		    printf("Sent request to existing help window.\n");
		    close(helpSocket);
		    exit(0);
		}
		close(helpSocket);
	    }
	    helpSocket=(-1);
	} else
	    printf("No 'localhost' found in host table; creating new window.\n");
    }

    /*
     *setup the help server, but not if we're running on someone else's machine
     */
    if (runningLocally) {
	helpSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (helpSocket >= 0) {
	    int on;
	    
	    on = 1;
	    setsockopt(helpSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on) );
	    myaddr.sin_family = AF_INET;
	    myaddr.sin_port = helpPort;
	    myaddr.sin_addr.s_addr = 0;		/* me, me! */
	    i = bind(helpSocket, (struct sockaddr *)&myaddr, sizeof(myaddr));
	    if (i >= 0) {
		i = listen(helpSocket, 4);
		if (i == 0) {
		    return helpSocket;
		}
	    }
	}
    }
    return helpSocket;
}

