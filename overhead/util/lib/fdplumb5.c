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

#include <andrewos.h>
#include <stdio.h>
#include <fdplumbi.h>
#include <util.h>


int dbg_t2open(const char *name, char * const argv[], FILE **r, FILE **w)
{
    int code;

    code = t2open(name, argv, r, w);
    if (!code) {
	RegisterOpenFile(fileno(*w), "t2-r", FDLEAK_OPENCODE_T2OPEN);
	RegisterOpenFile(fileno(*r), "t2-w", FDLEAK_OPENCODE_T2OPEN);
    }
    return(code);
}


int dbg_t2close(FILE *fp, int seconds, int *timedout)
{
    RegisterCloseFile(fileno(fp));
    return(t2close(fp, seconds, timedout));
}


