/***********************************************************************

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

/*
  interface from objects to window manager
*/

#include <gestures/bool.h>
#include <gestures/util.h>
#include <gestures/gdev.h>
#include <stdio.h>
#include "proto.h"

void
WmClear(void)
{
	GDEVstart();
}

void
WmInit(char *program)
{
	GDEVinit(NULL);
	GDEVsets("currentfunction", "xor");
	GDEVseti("thickness", 0);	/* gives fast line drawing */
	GDEVsets("program", program ? program : "Dean's window");
	WmClear();
}

void WmFlush(void)
{
	GDEVflush();
}

#if 0
void
EraseOn(void)
{ /* GDEVsets("currentcolor", "white"); */
}

void
EraseOff(void)
{
	/* GDEVsets("currentcolor", "black"); */
}
#endif
