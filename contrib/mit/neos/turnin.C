/* $Author: wjh $ */

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/neos/RCS/turnin.C,v 1.5 1996/06/11 01:28:57 wjh Exp $";
#endif


 
/*
 * turnin.c
 *
 * This does the work for turnin within the EOS applications.
*/

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *  For full copyright information see:'mit-copyright.h'     *
 *************************************************************/

#include <andrewos.h>
#include <mit-copyright.h>

#ifdef _IBMR2
	/* Kludge: sys/rpc/types.h declares malloc as extern char * */
#endif


#include <bind.H>
#include <buffer.H>
#include <cursor.H>
#include <environ.H>
#include <eos.h>
#include <frame.H>
#include <im.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <message.H>
#include <point.h>
#include <strinput.H>
#include <turnin.H>
#include <value.H>
#include <view.H>
#include <sys/errno.h>

/* sys/types.h in AIX PS2 defines "struct label",  causing a type name clash.
  Avoid this by temporarily redefining "label" to be something else. */
#define label gezornenplatz
#include <eosfx.H>	/* eosfx.ih includes sys/types.h */
#undef label

#include <eos.H>   /* eos.ih uses struct label */

#define debug(x)  /* printf x ; fflush(stdin);  */
/* #include <stdio.h> */







static class cursor *clockcursor;
class keymap *turnin_kmap;
class menulist *turnin_menus;


ATKdefineRegistry(turnin, view, turnin::InitializeClass);
#ifndef NORCSID
#endif
#ifdef _IBMR2
#endif
void CancelOperation(class turnin  *self, FX  **fxp);
void turnin_Hide( class turnin  *self, class observable  *triggerer, long  rock);
void turnin_TurninGo(class turnin  *self, class observable  *triggerer, long  rock);
void MoveOn(class turnin  *self, long  rock);


void CancelOperation(class turnin  *self, FX  **fxp)
{
    eosfx::Close(fxp);
    (self)->WantInputFocus( self->name);
    StopWaitCursor();
}

void turnin_Hide( class turnin  *self, class observable  *triggerer, long  rock)
{
    if (!self) return;
    self->IDoNotExist = TRUE;
    /* "If I close my eyes, then you can't see me!" */
    ((self)->GetIM())->VanishWindow();
}

void turnin_TurninGo(class turnin  *self, class observable  *triggerer, long  rock)
/* This routine sends the paper specified by the strinput fields
   The name and number must be specified, else the routine fails
   If any errors occur during the sending of the file, then the routine fails
   If the routine fails, an error message is reported and nothing changes
   If it succeeds, then it gives a MESSAGE to self->daddy->frame
     and to self saying that turnin succeeded.
    Then it acts like we hit the ok button and hides the turnin window.
 */
{
    char filename[128], papername[128], assignment[6], *errormsg;
    class buffer *buf;
    Paper paper;

    strcpy(filename, (self->name)->GetInput( 120));
    if (!strcmp(filename, "")) {
	message::DisplayString(self, DIALOG, "You must specify a filename");
	(self)->WantInputFocus( self->name);
	CancelOperation(self, NULL);
	return;
    }

    strcpy(assignment, (self->number)->GetInput( 5));
    if (!strcmp(assignment, "")) {
	message::DisplayString(self, DIALOG, "You must specify an assignment number");
	(self)->WantInputFocus( self->number);
	CancelOperation(self, NULL);
	return;
    }

    message::DisplayString(self, MESSAGE, "Please wait: Turning in paper....");
    StartWaitCursor();
    im::ForceUpdate();

    eosfx::PaperClear(&paper);
    paper.assignment = atoi(assignment);
    paper.type = TURNEDIN;
    strcpy (papername, eosfx::PathTail(filename));
    paper.filename = papername;

    if (self->daddy == NULL) {
	message::DisplayString(self, DIALOG, "Program bug - I am an orphan!");
	CancelOperation(self, NULL);
	return;
    }

    if ((self->onoff)->GetValue()) {
	char *home;
	debug(("Turning in buffer\n"));
	self->turninfromfile = FALSE;
	/* Save temporarily in home dir of user */
	home = environ::Get("HOME");
	strcpy(filename, home);
	strcat(filename, "/");
	strcat(filename, papername);
	strcpy(filename, eosfx::LocalUnique(filename));
	buf = (self->daddy->frame)->GetBuffer();
	(buf)->WriteToFile( filename, buffer_ReliableWrite);
    } else {
	debug(("Turning in file\n"));
	self->turninfromfile = TRUE;
    }

    errormsg = eosfx::SendFile(self->course, filename, &paper, FALSE);
    if (!self->turninfromfile) unlink (filename);
    if (errormsg != NULL) {
	message::DisplayString(self, DIALOG, errormsg);
	message::DisplayString(self, MESSAGE, "Turn in failed.");
	CancelOperation(self, NULL);
	return;
    }
    message::DisplayString(self->daddy->frame, MESSAGE, "Assignment has been turned in.");
    message::DisplayString(self, MESSAGE, "Assignment has been turned in.");
    StopWaitCursor();
    self->IDoNotExist = TRUE;
    ((self)->GetIM())->VanishWindow();
    return;
}

void MoveOn(class turnin  *self, long  rock)
/* This is the routine bound to the Return key - it moves the focus from
  self->name to self->number, or, if self->number already has the focus,
      then it calls turnin_TurninGo
      */
{
    if ((self->name)->HaveYouGotTheFocus())
	(self)->WantInputFocus( self->number);
    else if ((self->number)->HaveYouGotTheFocus())
	turnin_TurninGo(self, NULL, 0);
}

static struct bind_Description turnin_bindings[] =
{
    {"turnin-move-on", "\015", 0, NULL, 0, 0, MoveOn, NULL},
    {"turnin-move-on", "\012", 0, NULL, 0, 0, MoveOn, NULL},
    {"turnin-cancel", NULL, 0, "Cancel", 0, 0, turnin_Hide, NULL},
    NULL
};

boolean turnin::InitializeClass()
{
    turnin_menus = new class menulist;
    turnin_kmap = new class keymap;
    bind::BindList(turnin_bindings, turnin_kmap, turnin_menus, &turnin_ATKregistry_ );
    clockcursor = cursor::Create(NULL);
    (clockcursor)->SetStandard( Cursor_Wait);
    return TRUE;
}

void turnin::ReceiveInputFocus()
{
    debug(("ReceiveFocus(turnin)\n"));
    (this)->WantInputFocus( this->whole);
}

void
turnin::PostMenus(class menulist  *ml)
{
    (this)->view::PostMenus( this->menus);
}

void turnin::PostKeyState(class keystate  *ks)
/* Want to add our own keybindings into the chain that gets passed to us */
{
    if (!ks) return;

    this->kstate->next = NULL;
    (this->kstate)->AddBefore( ks); 
    (this)->view::PostKeyState( this->kstate);
}

void
turnin::GoForIt()
{
    (this->name)->SetInput( ((this->daddy->frame)->GetBuffer())->GetName());
    (this->number)->ClearInput(); 
    (this->onoff)->SetValue( 1);
    (this)->WantUpdate( this->onoffv);
    message::DisplayString(this, MESSAGE, "");
    (this)->WantInputFocus( this->name);
}
