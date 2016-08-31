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

static char buf[16384];
#ifdef ANSI_COMPILER
int dbgprintf(char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    va_end(ap);
    fprintf(stdout, "%s", buf);
    if(ferror(stdout)) {
	FILE *fp=freopen("/dev/console", "w", stdout);
	if(fp==NULL) fp=freopen("/dev/tty", "w", stdout);
	fprintf(fp, "%s", buf);
    }
}
#else
/*VARARGS*/
dbgprintf(format, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20)
char   *format,
       *s1,
       *s2,
       *s3,
       *s4,
       *s5,
       *s6,
       *s7,
       *s8,
       *s9,
       *s10,
       *s11,
       *s12,
       *s13,
       *s14,
       *s15,
       *s16,
       *s17,
       *s18,
       *s19,
       *s20;
{
    fprintf(stdout, format, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20);
    if(ferror(stdout)) {
	FILE *fp=freopen("/dev/console", "w", stdout);
	if(fp==NULL) fp=freopen("/dev/tty", "w", stdout);
	fprintf(stdout, format, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14, s15, s16, s17, s18, s19, s20);
    }
}
#endif
