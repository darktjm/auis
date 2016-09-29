/* Copyright 1993 Carnegie Mellon University All rights reserved. */

#include <andrewos.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#if 0
#include <arpa/inet.h>
#endif /* hpux */
#include <netdb.h>
/* #include <sys/tty.h> */
#include <sitevars.h>
#include <errno.h>
#include <util.h>
#include "socket.h"

#ifdef ANSI_COMPILER
extern char *inet_ntoa(unsigned long addr);
#endif
extern char ThisHostName[];
NO_DLL_EXPORT struct sockaddr_in ConsoleAddr;
NO_DLL_EXPORT u_long ThisHostAddr;

static void maptolower(char  *str)
{
    unsigned int i;
    for (i = 0; i < strlen(str); i++){
	if (isupper(str[i])){
	    str[i] = tolower(str[i]);
	}
    }
}

/* failures:
 -1 gethostname
	if (self){
	    ReportInternalError(self, "console:<OpenConsoleSocket> gethostname failed");
	}
	else{
	    arrgh(("console:<OpenConsoleSocket> gethostname failed"));
	}
 -2 gethostbyname
	if (self){
	    ReportInternalError(self, "console:<OpenConsoleSocket> gethostbyname failed");
	}
	else{
	    arrgh(("console:<OpenConsoleSocket> gethostbyname failed"));
	}
 -3
 if (self){
	    ReportInternalError(self, "console:<OpenConsoleSocket> Incomplete error monitoring: bind to ConsoleSocket failed");
	}
	else{
	    arrgh(("console:<OpenConsoleSocket> Incomplete error monitoring: bind to ConsoleSocket failed"));
	}
 -4
 if (self){
	    ReportInternalError(self, "console:<OpenConsoleSocket> fcntl on ConsoleSocket failed");
	}
	else {
	    arrgh(("console:<OpenConsoleSocket> fcntl on ConsoleSocket failed"));
	}
 -5
 
 -6 socket
 
	ReportInternalError(self, "console:<make_socket> socket failed");
 -7 bind
 
	ReportInternalError(self, "console:<make_socket> bind failed");
 */

static int make_socket(int  port)
{

    int desc, protonum;
    struct protoent *proto;
    struct sockaddr_in socketname;

    if ((proto = getprotobyname("udp")) == NULL) /* Some machines don't have /etc/protocols... */
        protonum = 6;
    else
        protonum = proto->p_proto;

    desc = socket(AF_INET, SOCK_DGRAM, protonum);
    if (desc < 0) {
	perror("socket");
        return(-6);
    }

    socketname.sin_family = AF_INET;
    /* Address info must be in network byte order! */
    socketname.sin_port = htons(port);
    socketname.sin_addr.s_addr = INADDR_ANY;

    if (bind(desc, (struct sockaddr *)&socketname, sizeof(socketname)) < 0) {
/*	perror("bind"); */
        return(-7);
    }
    return(desc);
}

int OpenConsoleSocket(void)
    {
    struct hostent *hent;
    int flags;
    int sock;
    errno = 0;
    if (gethostname(ThisHostName, 256) < 0){
	return -1;
    }
    maptolower(ThisHostName);
    hent = gethostbyname(ThisHostName);
    if (hent == 0) {
	return -2;
    }
    strcpy(ThisHostName, hent->h_name);
    memmove(&ThisHostAddr, hent->h_addr, sizeof(ThisHostAddr));

    if ((sock = make_socket(_SITE_CONSOLE_SOCKET)) < 0){
	return sock;/* should I be setting some global flag ? */
    }
    flags = fcntl(sock, F_GETFL, 0);
#if POSIX_ENV && !defined(sys_sun4_41)
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
#else
    if (fcntl(sock, F_SETFL, flags | FNDELAY) < 0)
#endif
    {
	return -4;
    }
    return(sock);
}

#define MESSAGESIZE 1500
NO_DLL_EXPORT boolean WasUDPAction = FALSE;
NO_DLL_EXPORT char ConsoleMessage[MESSAGESIZE+100] = "";
extern int ConsoleSocket;
int FakeCheckConsoleSocket(void)
{
    int j;
    socklen_t len;
    static char hostname[80];
    struct hostent *hp;
    static u_long lastHostAddr = 0;
    u_long newHostAddr;

    WasUDPAction = TRUE;
    len = sizeof(ConsoleAddr);
if ((j = recvfrom(ConsoleSocket, ConsoleMessage, MESSAGESIZE, 0, (struct sockaddr *)&ConsoleAddr, &len)) >= 0)  {
	ConsoleMessage[j] = '\0';
	for (; j >= 0 && (ConsoleMessage[j] == '\n' || ConsoleMessage[j] == ' ' || ConsoleMessage[j] == '\t' || ConsoleMessage[j] == '\0'); j--)
	    ;
	ConsoleMessage[++j] = '\0';

	newHostAddr = ConsoleAddr.sin_addr.s_addr;
	if (newHostAddr != ThisHostAddr)  {
	    if (newHostAddr != lastHostAddr)  {
		lastHostAddr = newHostAddr;
		hp = gethostbyaddr((const char *)&ConsoleAddr.sin_addr, sizeof(ConsoleAddr.sin_addr), AF_INET);
		if (hp == 0) {
		    strcpy(hostname, inet_ntoa(ConsoleAddr.sin_addr.s_addr));
		} else {
		    strcpy(hostname, hp->h_name);
		}
	    }
	    if (lc_strcmp(hostname, ThisHostName) && lc_strcmp(hostname, "localhost")) {
#ifdef RCH_ENV
		if (ConsoleMessage[j-1] == '\n')
		    sprintf(&(ConsoleMessage[j-1]), " [From host %s]\n", hostname);
		else
		    sprintf(&(ConsoleMessage[j]), " [From host %s]\n", hostname);
#else
		sprintf(&(ConsoleMessage[j]), " [From host %s]\n", hostname);
#endif
	    }
	}
	return 1;
    }

    if (j<0) {
	return(-1);
    }
    return 0;
}

#if 0
static int Bind (int  service, char  *host)
        {
    int s;
    char buf[100];
    struct sockaddr_in server;
    struct servent *sp;
    struct hostent *hp;

    if (host == NULL) {
        gethostname(buf, sizeof(buf));
        host = buf;
    }
    sp = getservbyport(htons(service), "tcp");
    if (sp == NULL){
	return -1;
    }
    hp = gethostbyname(host);
    if (hp == NULL){
	return -1;
    }
    memset((char *) &server, 0, sizeof(server));
    memmove((char *) &server.sin_addr, hp->h_addr, hp->h_length);
    server.sin_family = hp->h_addrtype;
    server.sin_port = sp->s_port;
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0){
	return -1;
    }
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        close(s);
        return -1;
    }
    return s;
}
#endif
