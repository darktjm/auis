/***********************************************************************

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

/*
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/gestures/gestsrc/gdp/RCS/wm.c,v 1.3 1992/12/15 21:50:19 rr2b Stab74 $";
#endif

/*
  interface from objects to window manager
*/

#include <gestures/bool.h>
#include <gestures/util.h>
#include <gestures/gdev.h>
#include <stdio.h>

void
WmClear()
{
	GDEVstart();
}

void
WmInit(program)
char *program;
{
	GDEVinit(NULL);
	GDEVsets("currentfunction", "xor");
	GDEVseti("thickness", 0);	/* gives fast line drawing */
	GDEVsets("program", program ? program : "Dean's window");
	WmClear();
}

WmFlush()
{
	GDEVflush();
}

void
EraseOn()
{ /* GDEVsets("currentcolor", "white"); */
}

void
EraseOff()
{
	/* GDEVsets("currentcolor", "black"); */
}