/*
 * ATKdebug -- Class for helping debug ATK classes.
 *
  */
/* Copyright 1995 Carnegie Mellon University All rights reserved.
  $Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/
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


