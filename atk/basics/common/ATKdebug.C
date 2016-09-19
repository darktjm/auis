/*
 * ATKdebug -- Class for helping debug ATK classes.
 *
 */
/* Copyright 1995 Carnegie Mellon University All rights reserved. */
#include <andrewos.h>

#if SY_AIXx && defined (RCH_ENV)
extern "C" {
#include "/usr/local/include/traceback.h"
};
#include "/usr/contrib/lib/malloc/dbmalloc.h"
#endif

#include <ATKdebug.H>

void ATKdebug::Traceback()
{
#if SY_AIXx && defined (RCH_ENV)
    traceback(NULL);
#endif
}

int ATKdebug::SetMallocCheckLevel(int newlevel)
{
#if SY_AIXx && defined (RCH_ENV)
    return dbmalloc_checklevel(newlevel);
#else
    return -1;	// not implemented on this architecture.
#endif
}

int ATKdebug::MallocCheck()
{
#if SY_AIXx && defined (RCH_ENV)
    return dbmalloc_check();
#else
    return -1;	// not implemented on this architecture.
#endif
}

int ATKdebug::MallocDump()
{
#if SY_AIXx && defined (RCH_ENV)
    return dbmalloc_dump();
#else
    return -1;	// not implemented on this architecture.
#endif
}

int ATKdebug::MallocFullDump()
{
#if SY_AIXx && defined (RCH_ENV)
    return dbmalloc_fulldump();
#else
    return -1;	// not implemented on this architecture.
#endif
}

void ATKdebug::MallocInit()
{
#if SY_AIXx && defined (RCH_ENV)
#if SY_AIX4
    // malloc debugging is broken in AIX 4.1 :-(.
#else
    static int dbmalloc_checking = -1;
    if (dbmalloc_checking == -1) {
	// See if we should do dbmalloc_checking.
	int level;
	char *env = getenv("ATKMALLOC");
	if (dbmalloc_checking = (env != NULL)) {
	    level = atoi(env);
	    dbmalloc_checklevel(level);
	    if (level >= 2)
		printf("ATK Malloc checking at level %d\n", level);
	}
    }
    if (dbmalloc_checking)
	dbmalloc_init();
#endif
#endif
}


