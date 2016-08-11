#include <andrewos.h>

#define MAXTRIES 15

int lockFile(fn)
char *fn;
{
    int triesLeft=MAXTRIES;
    char buf[MAXPATHLEN];

    strcpy(buf,fn);
    strcat(buf,".LOCK");

    while(triesLeft-->0){
	int fd=open(buf,O_CREAT|O_EXCL,0600);
	if(fd>=0){
	    close(fd);
	    return 1;
	}
	sleep(1);
    }

    return 0;
}

int unlockFile(fn)
char *fn;
{
    char buf[MAXPATHLEN];

    strcpy(buf,fn);
    strcat(buf,".LOCK");

    unlink(buf);
}
