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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/hdrs/RCS/genhdr.c,v 1.8 1996/06/14 23:49:35 wjh Exp $";
#endif

#include <andrewos.h>

/**
 **
 **  Create the file andrdir.h which gives the defined
 **  QUOTED_DEFAULT_XLIBDIR_ENV
 **  QUOTED_DEFAULT_ANDREWDIR_ENV and possibly the 
 **  QUOTED_DEFAULT_LOCALDIR_ENV.
 **  Also, define QUOTED_DEFAULT_ANDREWDIR_ANDREWSETUP.
 **
 **/
int
main()
{
	printf("/* andrdir.h - automatically generated */\n\n");
	printf("#define QUOTED_DEFAULT_XLIBDIR_ENV \"%s\"\n", XLIBDIR_VAL);
	printf("#define QUOTED_DEFAULT_ANDREWDIR_ENV \"%s\"\n", ADIRDIR_VAL);
#ifdef DEFAULT_LOCALDIR_ENV
	printf("#define QUOTED_DEFAULT_LOCALDIR_ENV \"%s\"\n", LOCDIR_VAL);
#endif
	printf("#define QUOTED_DEFAULT_ANDREWDIR_ANDREWSETUP \"%s%s\"\n", 
		ADIRDIR_VAL, "/etc/AndrewSetup");

    /*
     * We must check flush because flushs can fail with remote filesystems.
     */
    if (0 != fflush(stdout)) {
	(void) fprintf(stderr, "\ngenhdr:  error fflush'ing stdout\n");
	exit(1);
    }
    exit(0);
}
