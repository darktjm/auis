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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/writeall.c,v 2.6 1992/12/15 21:11:36 rr2b Stab74 $";
#endif

/*
	writeall.c -- Do write, resuming if interrupted
*/

 

extern int errno;

int writeall(fd, Buf, NBytes)
int fd;
char *Buf;
int NBytes;
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