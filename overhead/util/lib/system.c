/* Copyright 1992 Carnegie Mellon University, All rights reserved. */

#include <andrewos.h>
#include <util.h>

/* work around a bug in system where if the parent process catches sigchld system returns a bogus exit status. */
#ifdef hpux
#include <stdlib.h>
#include <errno.h>
int os_system(const char *cmd)
{
    int result;
    errno=0;
    result=system(cmd);
    if(result==-1 && errno==EINTR) return 0;
    return result;
}
#else
int os_system(const char *cmd)
{
    return system(cmd);
}
#endif

