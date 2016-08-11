/*
  * tmpfile -- Open temporary file
  *	This is different fromt the emx one in that $TMP is used if set.  Would be nice if
  *	emx's tmpfile would do this.  We don't support multiple threads here.
  *	tmpfile is used by the vfile code in im to store the cut/paste buffer.
  * 
  */

#include <sys/emx.h>
#include <stdio.h>

static char tempDir[FILENAME_MAX] = "";

/*Temporary files created by emx's tmpfile don't always get deleted, because fclose is looking in current directory only, meaning if you change the working directory before quitting they won't get deleted.  Only the _tmpidx is remembered by emx's tmpfile - not the directory where stored.
 */

static void closeupTime() {
    /*Called when process exits.  We need to clean up any temp files we created that still exist.
     */

    /*Change the current working directory to $TMP and call _rmtmp to clean up any temp files*/
    if(strlen(tempDir)!=0) {
	char *currentDir = (char *)_getcwd2(0, 0);

	_chdir2(tempDir);
	_rmtmp();

	if(currentDir) {
	    _chdir2(currentDir); /*restore previous working directory*/
	    free(currentDir);
	}
    }
}

FILE *tmpfile() {
    char *name;
    FILE *f;
    char *tmpidx; /*pointer into temp name at start of first digit*/
    int idx;	/*integer value of tmpidx*/

    /*temp file names are of the form DIRECTORY+tmpidx+".tmp"  */
    if(strlen(tempDir)==0) {/*initialize things on first call*/
	char *tmp;
	if((tmp = (char *)getenv("TMP"))==NULL)
	    tmp = (char *)_getcwd2(&tempDir, sizeof(tempDir));
	strcpy(tempDir, tmp);
	if(atexit(closeupTime)==-1) /*register our "temp file cleanup" function*/
	    fprintf(stderr, "No atexit() spots left.\n");/*guess we won't be cleaning up temp files*/
    }

    if((name = tempnam(tempDir, ""))==NULL)
	return NULL;
    f = fopen(name, "w+b");
    /*extract out tmpidx so that we can remember it in stream structure*/
    for(tmpidx=name+strlen(tempDir);tmpidx && !isdigit(*tmpidx);++tmpidx);
    idx = atoi(tmpidx);
    free(name);
    if(f==NULL)
	return NULL;
    f->tmpidx = idx;
    f->flags |= _IOTMP; /*mark it as a temporary file*/
    return f;
}
