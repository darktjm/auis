/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/bush/RCS/bushapp.C,v 1.6 1995/11/07 20:17:10 robr Stab74 $";
#endif

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	Bush Application

MODULE	bushapp.c

VERSION	1.0

AUTHOR	TC Peters & GW Keim
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the Application layer for the Bush Inset.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  08/21/85	Created (TCP)
  01/15/89	Convert to ATK (GW Keim)
  05/01/89	Revise Initializing msg to conform with Majro/Minor Version msg (TCP)
  06/06/89	Added call to chdir() to set the current-working-directory to the 
	               path specified on the command line; (GW Keim)
  07/05/89	Use super_ParseArgs (TCP)
  07/28/89	Changed all occurances of AptTree to just Tree; (GW Keim)
  09/07/89	Set V1.0 msg (TCP)

END-SPECIFICATION  ************************************************************/


#include <andrewos.h>
ATK_IMPL("bushapp.H")
#include <im.H>
#include <frame.H>
#include <bush.H>
#include <bushv.H>
#include <bushapp.H>

#define	Bush		((self)->bushp)
#define	Bushv		((self)->bushview)
#define	Debug		((self)->debug)
#define imPtr		((self)->imp)
#define framePtr	((self)->framep)
#define	ArgC		((self)->argc)
#define	ArgV		((self)->argv)

/* The following defines are for when there are seperate windows for the tree-view, listing-view, and entry-view. */
#define	nodes_object    1
#define	entries_object  2
#define	entry_object    4


ATKdefineRegistry(bushapp, application, NULL);
#ifndef NORCSID
#endif


bushapp::bushapp( )
    {
  class bushapp *self = this;
  Bush = NULL;
  Bushv = NULL;
  imPtr = NULL;
  framePtr = NULL;
  ArgC = 0;
  ArgV = NULL;
  Debug = 0;
  (this)->SetMajorVersion(1);
  (this)->SetMinorVersion(0);
  THROWONFAILURE((TRUE));
}

bushapp::~bushapp( )
    {
  class bushapp *self = this;
  if(imPtr) {
    (imPtr)->Destroy();
    imPtr = NULL;
  }
  if(framePtr) {
    (framePtr)->Destroy();
    framePtr = NULL;
  }
  if(Bush) {
    (Bush)->Destroy();
    Bush = NULL;
  }
  if(Bushv) {
    (Bushv)->Destroy();
    Bushv = NULL;
  }
}

boolean
bushapp::ParseArgs( register int		     argc, register char		   **argv )
      {
  class bushapp *self = this;
  char **args = argv;

  if(!(this)->application::ParseArgs(argc,argv))
    return(FALSE);
  ArgV = argv;
  while(args && *args++) ArgC++;
  if(ArgC > 2) {
      printf("usage: bush [directory]\n");
      printf("bush: using '%s' as root directory.\n", ArgV[1]);
  }
  return(TRUE);
}

boolean
bushapp::Start( )
  {
  class bushapp *self = this;
  register char		    *startDir = NULL;
	
  if(this->argc > 1) startDir = this->argv[1];
  if(!((this)->application::Start()) ||
     !(imPtr = im::Create(NULL)) ||
     !(framePtr = new frame) ||
     !(Bush = bush::Create(startDir)) ||
     !(Bushv = bushv::Create(nodes_object))) 
	return(FALSE);
  (Bushv)->SetDataObject(Bush);
  (framePtr)->SetView(Bushv);
  (imPtr)->SetView(framePtr);
  (framePtr)->PostDefaultHandler("message",
    (framePtr)->WantHandler("message"));
  if((Bush)->TreeRoot()) 
    (imPtr)->SetTitle((Bush)->DirName((Bush)->TreeRoot()));
  chdir((Bush)->DirPath((Bush)->TreeRoot()));
  (Bushv)->WantInputFocus(Bushv);
  return(TRUE);
}
