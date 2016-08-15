/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/adew/RCS/genarb.C,v 1.2 1993/05/10 16:44:29 rr2b Stab74 $";
#endif

#include <stdio.h>
#define CMD_STR "#! /bin/csh -f\n\
if ($?ANDREWDIR) then\n\
exec ez $1  $ANDREWDIR/lib/arbiters/Arb\n\
else\n\
exec ez $1  %s/lib/arbiters/Arb\n\
endif\n"

#ifndef NORCSID
#endif
int main(int  argc, char  *argv[]);


int
main(int  argc, char  *argv[])

{
    /*
     * Check command line.
     */
    if (argc != 2) {
	(void) fprintf(stderr, "\ngenarb:  genarb ANDREWDIR\n");
	exit(1);
    }

    /*
     * Write out the script contents.
     */
    printf(CMD_STR, argv[1]);

    /*
     * We must check flush because flushs can fail with remote filesystems.
     */
    if (0 != fflush(stdout)) {
	(void) fprintf(stderr, "\ngenhdr:  error fflush'ing stdout\n");
	exit(1);
    }

    /*
     * All done!
     */
    exit(0);

}
