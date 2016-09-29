/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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

static const char * const formats[] = {
  "Default~1", NULL,
  "H:MM AM/PM~10", "%u:%M %P",
  "Month DD YYYY~11", "%o %A, %Y",
  NULL, NULL};



ATKdefineRegistry(writestampview, timeodayview, writestampview::InitializeClass);
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
  char menuname[250];
  struct proctable_Entry *proc = NULL;
  char menutitlefmt[200];
  static char procname[200];

  writestampview_menulist = new menulist;
  
  sprintf(procname, "%s-set-format", "writestamp");
  proc = proctable::DefineProc(procname, (proctable_fptr)MenuSetFormat, &writestampview_ATKregistry_ , NULL, "Set the writestamp's inset's format.");
  
  sprintf(menutitlefmt, "%s,%%s", "Write Stamp");
  
  for (i=0; formats[i]; i+=2) {
    sprintf(menuname, menutitlefmt, formats[i]);
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


