#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/systeminfo.h>
#ifndef RUSAGE_SELF
#include <sys/procfs.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

int
osi_ExclusiveLockNoBlock( fd )
int  fd;
{
    flock_t wL;

    wL.l_type = F_WRLCK;
    wL.l_whence = SEEK_SET;
    wL.l_start = wL.l_len = 0;
    return(fcntl(fd, F_SETLK, &wL));
}

int
osi_UnLock( fd )
int  fd;
{
    flock_t wL;

    wL.l_type = F_UNLCK;
    wL.l_whence = SEEK_SET;
    wL.l_start = wL.l_len = 0;
    return(fcntl(fd, F_SETLK, &wL));
}


sigset_t
sigblock(new)
sigset_t new;
{
    sigset_t s_new, s_old;
    s_new = new;
    sigprocmask(SIG_BLOCK,&s_new,&s_old);
    return s_old;
}

sigsetmask(new)
sigset_t new;
{
    sigset_t s_new = new;

    sigprocmask(SIG_SETMASK,&s_new,0);
}

char *
getwd(path)
char *path;
{
    return(getcwd(path, MAXPATHLEN));
}

int sigvec(Signal, Invec, Outvec)
int Signal;
struct sigaction *Invec, *Outvec; {
    return((int)sigaction(Signal, Invec, Outvec));
}




int
setreuid(ruid, euid)
int ruid, euid;
{
    setuid(ruid);
}

long
random()
{
    return(rand());
}

long
gethostid() {

    char buf[128];

    if (sysinfo(SI_HW_SERIAL, buf, 128) == -1) {
	return((long) -1);
    }
    return(strtoul(buf,NULL,0));
}

int
getdtablesize() {
    return(sysconf(_SC_OPEN_MAX));
}

int
setlinebuf(f)
    FILE *f;
{
    return(setvbuf(f, NULL, _IOLBF, BUFSIZ));
}

