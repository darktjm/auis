/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
	writeall.c -- Do write, resuming if interrupted
*/

#include <util.h>
#include <errno.h>
#include <unistd.h>

int writeall(int fd, const char *Buf, int NBytes)
{
    int Code, ToWrite;

    Code = 0;
    ToWrite = NBytes;
    errno = 0;
    while (ToWrite > 0) {
	Code = write(fd, Buf, ToWrite);
	if (Code < 0) return(Code);
	if (Code == ToWrite) return(NBytes);
	if (Code > ToWrite || errno != 0) return(Code + NBytes - ToWrite);
	ToWrite -= Code;
	Buf += Code;
    }
    return(Code);
}
