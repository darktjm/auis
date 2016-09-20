/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
main(void)
{
	puts("/* andrdir.h - automatically generated */\n");
	puts("/** \\addtogroup libutil\n * @{ */\n");
	printf("#define QUOTED_DEFAULT_XLIBDIR_ENV \"%s\"\n", XLIBDIR_VAL);
	printf("#define QUOTED_DEFAULT_ANDREWDIR_ENV \"%s\"\n", ADIRDIR_VAL);
#ifdef DEFAULT_LOCALDIR_ENV
	printf("#define QUOTED_DEFAULT_LOCALDIR_ENV \"%s\"\n", LOCDIR_VAL);
#endif
	printf("#define QUOTED_DEFAULT_ANDREWDIR_ANDREWSETUP \"%s%s\"\n", 
		ADIRDIR_VAL, "/etc/AndrewSetup");
	puts("\n/** @} */");

    /*
     * We must check flush because flushs can fail with remote filesystems.
     * or if the local filesystem is full
     */
    if (0 != fflush(stdout)) {
	(void) fprintf(stderr, "\ngenhdr:  error fflush'ing stdout\n");
	exit(1);
    }
    exit(0);
}
