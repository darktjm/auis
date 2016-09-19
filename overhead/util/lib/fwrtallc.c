/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
	fwrtallc.c
	fwriteallchars() -- Do fwrite, resuming if interrupted
*/


 

#include <util.h>
#include <stdio.h>
#include <errno.h>

int fwriteallchars(const char *Thing, int NItems, FILE *stream)
{
    int Code, ToWrite;

    Code = 0;
    ToWrite = NItems;
    errno = 0;
    while (ToWrite > 0) {
	Code = fwrite(Thing, sizeof(char), ToWrite, stream);
	if (Code < 0) return(Code);
	if (Code == 0 && (errno != 0 || ferror(stream) || feof(stream)))
	    return(Code);
	if (Code == ToWrite) return(NItems);
	if (Code > ToWrite || errno != 0 || ferror(stream) || feof(stream))
	    return(Code + NItems - ToWrite);
	ToWrite -= Code;
	Thing += Code;
    }
    return(Code);
}
