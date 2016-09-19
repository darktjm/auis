/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>		/* sys/time.h */
#include <stdio.h>
#include <util.h>

static int NOFILES=(-1);

int fselect(int nfds, FILE **rfiles, FILE **wfiles, FILE **xfiles, struct timeval *timeout)
{
    fd_set rmask, wmask, xmask;
    int ret=0;
    int	i;

    FD_ZERO(&rmask);
    FD_ZERO(&wmask);
    FD_ZERO(&xmask);
    if (NOFILES < 0) {
	NOFILES = FDTABLESIZE();
    }
    for (i = nfds; --i >= 0;) {
	int fd;
	if (rfiles && rfiles[i] != NULL && (fd = fileno(rfiles[i])) >= 0 && fd < NOFILES) {
	    if (FILE_HAS_IO(rfiles[i]) > 0)
		ret++;
	    else
		FD_SET(fd, &rmask);
	}
	if (wfiles && wfiles[i] != NULL && (fd = fileno(wfiles[i])) >= 0 && fd < NOFILES)
	    FD_SET(fd, &wmask);
	if (xfiles && xfiles[i] != NULL && (fd = fileno(xfiles[i])) >= 0 && fd < NOFILES)
	    FD_SET(fd, &xmask);
    }
    if (ret==0) 
	ret = select(NOFILES, &rmask, &wmask, &xmask, timeout);
    else {
	FD_ZERO(&rmask);
	FD_ZERO(&wmask);
	FD_ZERO(&xmask);
    }
    for (i = nfds; --i >= 0;) {
	if (rfiles && rfiles[i] != NULL &&
	    FILE_HAS_IO(rfiles[i]) <= 0 &&
	    (!FD_ISSET(fileno(rfiles[i]), &rmask)))
	    rfiles[i] = NULL;
	if (wfiles && wfiles[i] != NULL && (!FD_ISSET(fileno(wfiles[i]), &wmask)))
	    wfiles[i] = NULL;
	if (xfiles && xfiles[i] != NULL && (!FD_ISSET(fileno(xfiles[i]), &xmask)))
	    xfiles[i] = NULL;
    }
    return (ret);
}
