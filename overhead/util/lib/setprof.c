/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* 
 * WARNING WARNING WARNING WARNING WARNING WARNING 
 * 
 * This code contains a possible filename length bug (appending .NEW 
 * to the profileFileName).  This isn't worth fixing now because 
 * this code will soon be revamped for the port to SysV.
 *
 *							-t
 *
 * WARNING WARNING WARNING WARNING WARNING WARNING 
 */





/* ************************************************************ *\
	setprof.c -- routines to set preferences.
	    (routines to read preferences are in profile.c) 
\* ************************************************************ */
#include <andrewos.h>		/* sys/file.h */
#include <system.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <pwd.h>
#include <errno.h>
#include <util.h>

#define BIGPREF 2000

int setprofilestring(const char *prog, const char *pref, const char *val) 
{
    FILE *oldR;
    FILE *newR;
    FILE *newW;
    char newProfileFileName[MAXPATHLEN+20]; /* the extra because we */
					    /* tack extensions onto */
					    /* existing filenames */
    const char *finalProfileFileName;
    char LineBuf[BIGPREF], *real = NULL;
    const char *program;
    const char *key;
    const char *condition;
    const char *profileFileName;
    char k[BIGPREF];
    char pgm[BIGPREF]; 
    int MatchAllProgs;
    int programLength;
    int keyLength;
    int conditionLength;
    int retVal;

    if (prog == NULL)
	prog = ProgramName;

    MatchAllProgs = !strcmp(prog, "*");


    if (!(profileFileName = GetProfileFileName()) ||
	strcmp(profileFileName, AndrewDir("/lib/global.prf")) == 0 ||
	 access(profileFileName, W_OK))
	if (!(profileFileName = GetFirstProfileFileName()))  /* ~/preferences */
	    return -1;

    /* 
     * If the profile file is a symbolic link, we want to deal with 
     * the physical file underlying it since we'll be renaming on top 
     * of it and want to preserve the link.
     */

    real = realpath(profileFileName, NULL);
    if(!real)
	finalProfileFileName = profileFileName;
    else
	finalProfileFileName = real;


    /* Strictly speaking, this test just saves time.  It could be omitted. */
    if ((access(finalProfileFileName, W_OK) != 0) && (errno != ENOENT)) {
	fprintf(stderr,
		"<error:setprofile>No write access on file '%s'.\n",
		finalProfileFileName);
	if(real)
	    free(real);
	return -1;
    }

#ifdef USESHORTFILENAMES
    sprintf(newProfileFileName, "%s+", profileFileName);
#else
    sprintf(newProfileFileName, "%s.NEW", profileFileName);
#endif

    /* Open preference file for locking */
    oldR = fopen(profileFileName, osi_F_READLOCK);
    if (oldR == NULL) {
	if (errno == ENOENT) {
	    fprintf(stderr,
		    "<warning:setprofile>You have no '%s' file; creating one.\n",
		    profileFileName); 
	    newW = fopen(profileFileName, "w");
	    if (newW == NULL) {
		if(real)
		    free(real);
		return(-2);
	    }
	    fclose(newW);
	    oldR = fopen(profileFileName, osi_F_READLOCK);
	    if (oldR == NULL) {
		if(real)
		    free(real);
		return(-3);
	    }
	}
    }

    if (osi_ExclusiveLockNoBlock(fileno(oldR))){
	fclose(oldR);
	if(real)
	    free(real);
	return(-4);
    }

    /* And we open the write copy */
    newW = fopen(newProfileFileName, "w");
    if (newW == NULL) {
	fclose(oldR);
	if(real)
	    free(real);
	return(-5);
    }

    /* Now we open and lock a copy for reading, in order to keep 
	the lock when we close the write copy */
    newR = fopen(newProfileFileName, osi_F_READLOCK);
    if (newR == NULL) {
	fclose(oldR);
	fclose(newW);
	if(real)
	    free(real);
	return(-6);
    }
    if (osi_ExclusiveLockNoBlock(fileno(newR))){
	fclose(newR);
	fclose(oldR);
	fclose(newW);
	if(real)
	    free(real);
	return(-7);
    }

    refreshprofile();  /* Force re-reading on next getprofilexxx call */

    fprintf(newW, "%s.%s:%s\n", prog, pref, val);
    while ((retVal = ReadConfigureLine(oldR, LineBuf, BIGPREF,
				       &program, &programLength, &key,
				       &keyLength, NULL, NULL,
				       &condition, &conditionLength))
	   != CONFIG_EOF) {
	if (retVal == CONFIG_FOUNDENTRY &&
	    (condition == NULL || conditionLength == 0) &&
	    program != NULL && programLength != 0) { 

	    strncpy(pgm, program, programLength);
	    pgm[programLength] = '\0';
	    strncpy(k, key, keyLength);
	    k[keyLength] = '\0';
	    if ((!MatchAllProgs && !FOLDEDEQ(prog, pgm)) ||
		!FOLDEDEQ(pref, k))
		fputs(LineBuf, newW);
	} else
	    fputs(LineBuf, newW);
    }

    if (fclose(newW)) {
	fclose(oldR);
	fclose(newR);
	remove(newProfileFileName);
	if(real)
	    free(real);
	return(-8);
    }

    if (rename(newProfileFileName, finalProfileFileName)) {
	fclose(newR);
	fclose(oldR);
	if(real)
	    free(real);
	return(-9);
    }

    fclose(oldR);
    fclose(newR);
    if(real)
	free(real);
    return(0);
}

#ifdef TESTINGONLYTESTING
/* 
Needs to compile with:
  cc -DTESTINGONLYTESTING setprof.c /usr/andrew/lib/libutil.a -o setprof 
*/
char ProgramName[100] = "foobar";
int main(int argc, char **argv)
{
    int x;

    printf("Setting preference %s.%s: %s\n", argv[1], argv[2], argv[3]);
    x = setprofilestring(argv[1], argv[2], argv[3]);
    printf("setprofilestring() returned %d (errno %d)\n", x, errno);
}
#endif /* TESTINGONLYTESTING */
