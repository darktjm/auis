/***********************************************************************

region.h - some window-related stuff for mrecord

Copyright (C) 1991 Dean Rubine

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

#define	MOUSE_CHAR	001

#define    LEFT_DOWN			001
#define    LEFT_MOVE		  	002
#define    LEFT_UP		  	003

#define    RIGHT_DOWN			011
#define    RIGHT_MOVE		  	012
#define    RIGHT_UP		  	013

extern void REGIONinit(void);
extern void REGIONinit2(void);
