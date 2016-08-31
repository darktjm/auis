/* Copyright 1993 Carnegie Mellon University All rights reserved.
  $Disclaimer: 
 Permission to use, copy, modify, and distribute this software and its 
 documentation for any purpose and without fee is hereby granted, provided 
 that the above copyright notice appear in all copies and that both that 
 copyright notice and this permission notice appear in supporting 
 documentation, and that the name of IBM not be used in advertising or 
 publicity pertaining to distribution of the software without specific, 
 written prior permission. 
                         
 THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 
  $
 */

#include <andrewos.h> /* sys/time.h sys/types.h sys/file.h */
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* Define USEGETSERV if you want wmclient to look up service names
   in /etc/services.  We wire them in here for speed. */
#ifndef USEGETSERV
#define CONSPORT	2018		/* to avoid getservbyname calls */
#endif /* USEGETSERV */
  
#ifdef ANSI_COMPILER
void xim_EstablishConsole(char  * xhost, char *progname)
#else
void xim_EstablishConsole(xhost, progname)
char *xhost;
char *progname;
#endif
{
    int i;
    char tmpHostName[255];
    unsigned long hostaddr;
    register int fd;
    struct sockaddr_in sin;
    struct hostent *hostent;
#ifdef USEGETSERV
    struct servent *servent;
#endif /* USEGETSERV */
    static boolean established = FALSE;
    char *consolehost;
    boolean haveport=FALSE;

    if (! established)  {

	consolehost=getenv("ATKCONSOLEHOST");
	
	/* Create a name with screen number */
	strncpy(tmpHostName, consolehost?consolehost:xhost, sizeof(tmpHostName));
	for (i=0;i<255 &&(tmpHostName[i] != 0)&& (tmpHostName[i] != ':');i++);
	if (i<255) {
	    if(tmpHostName[i]==':') haveport=TRUE;
	    tmpHostName[i] =	0;  /* Truncate at the : */
	}
	/* get address of host */
	if ((i==0) || (strcmp("unix",tmpHostName)==0)) {
	    hostaddr = getaddr();
	}
	else {
	    if (isdigit(*tmpHostName)) {
		int ints[4];

		if (4 == sscanf(tmpHostName, "%d.%d.%d.%d",
				ints, ints+1, ints+2, ints+3)) {
		    char addr[4];

		    addr[0] = ints[0];
		    addr[1] = ints[1];
		    addr[2] = ints[2];
		    addr[3] = ints[3];
		    hostent = gethostbyaddr(addr, 4, AF_INET);
		}
		else {
		    hostent = gethostbyname(tmpHostName);
		}
	    }
	    else 
		hostent = gethostbyname(tmpHostName);
	    if (hostent == 0) {
		fprintf (stderr, "Could not find host %s in %s(xim). Error messages may not go to console.\n", tmpHostName, progname);
		fflush (stderr);
		return;
	    }
	    memmove(&hostaddr, hostent->h_addr, sizeof(hostaddr));
	}

	/* Have the address, try to open the special connection */
	/* establish a connection from stderr to console error log */
	fd = socket(AF_INET, SOCK_DGRAM, 0);
#ifdef USEGETSERV
	servent = getservbyname("console", 0);
	if (servent != NULL && fd >= 0)
#else /* USEGETSERV */
        if (fd >= 0)
#endif /* USEGETSERV */
	{
	    memset((char *) &sin, 0, sizeof(sin));
	    sin.sin_family = AF_INET;
#ifdef USEGETSERV
	    sin.sin_port = servent->s_port;
#else /* USEGETSERV */
	    sin.sin_port = htons(CONSPORT);
#endif /* USEGETSERV */
	    if(haveport && consolehost) {
		sin.sin_port = htons(atoi(tmpHostName+i+1));
	    }
	    sin.sin_addr.s_addr = hostaddr;
	    if (connect(fd, (struct sockaddr *)&sin, sizeof(sin)) >= 0) {
		/*
		 Ignore SIGPIPE since writes on a connected udp
		 socket can generate SIGPIPE if there is no
		     receiver on the other side.
		     */

		signal(SIGPIPE, SIG_IGN);

		fclose(stderr);
		dup2(fd, 2);
#ifdef hpux
	    setvbuf(fdopen(2, "w"), NULL, _IOLBF, BUFSIZ);
#else /* hpux */
		setlinebuf(fdopen(2, "w"));
#endif /* hpux */
	    }
	}
	if (fd >= 0)
	    close(fd);
	established = TRUE;
    }
}
