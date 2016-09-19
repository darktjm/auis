/*
 * ATKdebug -- Class for helping debug ATK classes.
 *
  */
/* Copyright 1995 Carnegie Mellon University All rights reserved. */
#ifndef _ATKDEBUG_H
#define _ATKDEBUG_H

class ATKdebug {
  public:
    static void Traceback();	// dump traceback to stderr

    // Memory checking.  These funcs are only valid if the
    // "ATKMALLOC" environment var is set.  This var can be
    // set to the numbers 1..5 where 1 is the default (if set).
    static int SetMallocCheckLevel(int newlevel);
    static int MallocCheck();	// check memory...return TRUE if ok.
    static int MallocDump();	// dump addresses of active blocks of memory by caller.
    static int MallocFullDump();// dump addresses of all active blocks of memory.

    // This is a helper function not intended for mortals to use...
    static void MallocInit();
};

#endif /* _ATKDEBUG_H */


