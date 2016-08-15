#include <unistd.h>
#include <sys/param.h>

char *getwd(PathName)
char *PathName; {
    return(getcwd(PathName, MAXPATHLEN));
}
