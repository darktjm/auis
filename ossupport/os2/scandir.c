/* alphasort(...), scandir(...)
*/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <errno.h>


/* For some reason EMX doesn't have scandir?  This is snarfed from BSD. */
#define DIRSIZ(dp) \
    ((sizeof (struct dirent) - (MAXNAMLEN+1)) + (((dp)->d_namlen+1 + 3) &~ 3))
int scandir(char *dirname, struct dirent *(*namelist[]), int (*select)(struct dirent *), int (*dcomp)(struct dirent **, struct dirent **))
{
	register struct dirent *d, *p, **names;
	register int nitems;
	register char *cp1, *cp2;
	struct stat stb;
	long arraysz;
	DIR *dirp;

	if ((dirp = opendir(dirname)) == NULL)
	    return(-1);
	/*The fstat is failing sometimes - apparently a bad dd_id*/
	/*
	if (fstat(dirp->dd_id, &stb) < 0)
	    return(-1);
	 */

	/*
	 * estimate the array size by taking the size of the directory file
	 * and dividing it by a multiple of the minimum size entry. 
	 */
	arraysz = 150/*(stb.st_size / 24)*/;
	names = (struct dirent **)malloc(arraysz * sizeof(struct dirent *));
	if (names == NULL)
		return(-1);

	nitems = 0;
	while ((d = readdir(dirp)) != NULL) {
		if (select != NULL && !(*select)(d))
			continue;	/* just selected names */
		/*
		 * Make a minimum size copy of the data
		 */
		p = (struct dirent *)malloc(DIRSIZ(d));
		if (p == NULL)
			return(-1);
		p->d_ino = d->d_ino;
		p->d_reclen = d->d_reclen;
		p->d_namlen = d->d_namlen;
		for (cp1 = p->d_name, cp2 = d->d_name; *cp1++ = *cp2++; );
		/*
		 * Check to make sure the array has space left and
		 * realloc the maximum size.
		 */
		if (++nitems >= arraysz) {
		    /*The fstat is failing sometimes - apparently a bad dd_id*/
			/*if (fstat(dirp->dd_id, &stb) < 0)
				return(-1);*/	/* just might have grown */
			arraysz = arraysz + arraysz/*stb.st_size / 12*/;
			names = (struct dirent **)realloc((char *)names,
				arraysz * sizeof(struct dirent *));
			if (names == NULL)
				return(-1);
		}
		names[nitems-1] = p;
	}
	closedir(dirp);
	if (nitems && dcomp != NULL)
		qsort(names, nitems, sizeof(struct dirent *), dcomp);
	*namelist = names;
	return(nitems);
}

/*
 * Alphabetic order comparison routine for those who want it.
 */
int alphasort(struct dirent **d1, struct dirent **d2)
{
	return(strcmp((*d1)->d_name, (*d2)->d_name));
}

