/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/time/RCS/writestmpv.C,v 1.4 1994/08/11 03:03:29 rr2b Stab74 $";
#endif


#include <andrewos.h>
#include <writestmpv.H>
#include <writestamp.H>
#include <proctable.H>
#include <util.h>

/* Defined constants and macros */

/* External Declarations */

/* Forward Declarations */

/* Global Variables */
static class menulist *writestampview_menulist = NULL;

static char *formats[] = {
  "Default~1", NULL,
  "H:MM AM/PM~10", "%u:%M %P",
  "Month DD YYYY~11", "%o %A, %Y",
  NULL, NULL};



ATKdefineRegistry(writestampview, timeodayview, writestampview::InitializeClass);
#ifndef NORCSID
#endif
static void MenuSetFormat(class writestampview  *self, char  *format);


static void MenuSetFormat(class writestampview  *self, char  *format)
          {
  class writestamp *b = (class writestamp *) (self)->GetDataObject();

  (b)->SetFormat( format);
  (b)->SetModified();
}


boolean
writestampview::InitializeClass()
{
/* 
  Initialize all the class data.
*/
  int i;
  char *menuname;
  char temp[250];
  struct proctable_Entry *proc = NULL;
  char menutitlefmt[200];
  char procname[200];

  writestampview_menulist = new menulist;
  
  sprintf(procname, "%s-set-format", "writestamp");
  proc = proctable::DefineProc(procname, (proctable_fptr)MenuSetFormat, &writestampview_ATKregistry_ , NULL, "Set the writestamp's inset's format.");
  
  sprintf(menutitlefmt, "%s,%%s", "Write Stamp");
  
  for (i=0; formats[i]; i+=2) {
    sprintf(temp, menutitlefmt, formats[i]);
    menuname = NewString(temp);
    (writestampview_menulist)->AddToML( menuname, proc, (long)formats[i+1], 0);
  }
  
  return(TRUE);
}


writestampview::writestampview()
{
	ATKinit;

/*
  Set up the data for each instance of the object.
*/
  class menulist *ml;

  if ((this)->GetMenuList()) delete (this)->GetMenuList();
  ml = (writestampview_menulist)->DuplicateML( this);
  (this)->SetMenuList( ml);

  THROWONFAILURE((TRUE));
}


writestampview::~writestampview()
{
	ATKinit;

  return;
}


