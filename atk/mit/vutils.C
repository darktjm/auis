/*************************************************
 * View utility package
 * Contains stuff useful to all views.
 *************************************************/

/*************************************************
 * Copyright (C) 1990 by the Massachusetts Institute of Technology
 * Permission to use, copy, modify, distribute, and sell this
 * software and its documentation for any purpose is hereby
 * granted without fee, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.  M.I.T. makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *************************************************/

#include <andrewos.h>
#include <ctype.h>
#include <bind.H>
#include <view.H>
#include <im.H>
#include <message.H>
#include <environ.H>
#include <environment.H>

#include <vutils.H>

#include <util.h>

#define DIALOG 100
#define MESSAGE 0

/* Routines appear in this file in "bottom-up" order. */
/* This is so I don't have to deal with declaring forward */
/* references. */

/* Added friendly read-only behavior from txtvcmds.c */

#define H_D_STRING "Help Could not start. It was "


ATKdefineRegistry(vutils, ATK, vutils::InitializeClass);
static void helpDeath (int  pid, class view  *self, WAIT_STATUS_TYPE  *status);
static void forkhelpproc (class view  *self, long  key);


static void helpDeath (int  pid, class view  *self, WAIT_STATUS_TYPE  *status)
{
    char buf[128],*em;

    strcpy(buf, H_D_STRING);
    em=statustostr(status, buf+strlen(H_D_STRING), 128 - strlen(H_D_STRING));
    if (em == NULL) {
	/* successful completion */
	message::DisplayString(self, MESSAGE, "If you don't see a help window now, something is wrong.");
    } else {
	message::DisplayString(self, MESSAGE, buf);
    }
}

static void forkhelpproc (class view  *self, long  key)
{
    const char *helpname = environ::AndrewDir("/bin/help");
    int pid, fd;

    switch (pid = osi_vfork()) {
	case 0:
	    for (fd = FDTABLESIZE(); fd > 2; --fd) close(fd);
	    execl(helpname, helpname, im::GetProgramName(), (char *)NULL);
	    printf ("Exec of %s failed.\n", helpname);
	    fflush (stdout);
	    _exit(-1);
	case -1:
	    message::DisplayString(self, DIALOG, "Could not start the help!");
	    break;
	default:
	    message::DisplayString(self, MESSAGE, "A Help window should appear shortly.");
	    im::AddZombieHandler(pid, (im_zombiefptr)helpDeath, (long)self);
	    break;
    }
    return;
}

boolean vutils::InitializeClass()
{
    static struct bind_Description compat_fns[] = {
	{"vutils-fork-help", NULL, 0, NULL, 0, 0, (proctable_fptr)forkhelpproc, "Call Andrew help in a new process.", "vutils"},

        {NULL},
    };
    struct ATKregistryEntry  *viewClassinfo;

    viewClassinfo = ATK::LoadClass("view");
    if (viewClassinfo != NULL) {
        bind::BindList(compat_fns, NULL, NULL, viewClassinfo);
        return TRUE;
    }
    else
        return FALSE;
}
