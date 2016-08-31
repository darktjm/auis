/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* 
 ***************************************************************
 * Routines for monitoring messages on console service
 * and (on some machines) /dev/console.
 * Some routines swiped from ttyscript.
 ****************************************************************
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <andrewos.h> /* sys/types.h sys/time.h sys/file.h */
#include <sys/ioctl.h>
#include <termios.h>

main()
{
    int SubChannel, tempfd;
    char ptyname[64];
    int ON = 1;
    char buf[1024];
    FILE *SubChannelFile;
    
    if (! GetPtyandName(&SubChannel, &tempfd, ptyname, sizeof(ptyname)))  {
        printf("getconsole: Incomplete error monitoring: Can't open pty %s %s\n", ptyname, strerror(errno));
	exit(1);
    }
    if (ioctl (tempfd, TIOCCONS, (char *) &ON) < 0) {
	printf("getconsole: Incomplete error monitoring: ioctl (TIOCCONS) failed (%s)\n", strerror(errno));
	exit(1);
    }
    SubChannelFile = fdopen(SubChannel, "r");

    while(fgets(buf, sizeof(buf), SubChannelFile)) {
	fputs(buf, stdout);
	fflush(stdout);
    }
    if (feof(SubChannelFile)) {
	printf("getconsole: EOF reading console output\n");
    }
    else {
	printf("getconsole: error reading console output: %s\n",
	       strerror(errno));
    }
    exit(1);
}

    
