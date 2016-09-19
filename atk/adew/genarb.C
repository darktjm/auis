/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <stdio.h>
#define CMD_STR "#!/bin/sh\n" \
                "exec ez \"$@\" \"${ANDREWDIR:-%s}/lib/arbiters/Arb\"\n"

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
