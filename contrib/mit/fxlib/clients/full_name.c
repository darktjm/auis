/**********************************************************************
 * full_name module
 *
 * $Author
 * $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/full_name.c,v $
 * $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/full_name.c,v 1.4 1994/01/31 22:36:06 sa3e Stab74 $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mitcopyright.h>.
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/full_name.c,v 1.4 1994/01/31 22:36:06 sa3e Stab74 $";
#endif

#include <mitcopyright.h>

#include <stdio.h>
#include <strings.h>
#include <hesiod.h>

/**** borrowed from eos sources ****/
/*
  Takes a username and tries to find a password entry for it via Hesiod;
  If the resolve fails or the password entry cannot be parsed, then the
  original name is returned, else the name given in passwd is returned,
  with the parameter name following in parentheses;
  e.g. RealName("jsmith") == "jsmith" || "John Smith (jsmith)";
*/

char *full_name(name)
     char *name;
{
  char **namelist, *realname, *tmp;
  static char finalname[256];
  char *index();
  int i;

  if ((namelist = hes_resolve(name, "passwd")) == NULL) {
    strcpy(finalname, name);
    strcat(finalname, " (no hesiod info)");
  } else {
    /* Extract name from password entry */
    realname = *namelist;
    for (i=0; i<4; i++)
      if ((realname = index(++realname, ':')) == NULL) {
	/* Password entry is screwy - so give up and return original */
	strcpy(finalname, name);
	return finalname;
      }
    /* Remove rest of password entry */
    if ((tmp = index(++realname,':')) != NULL)
      *tmp = '\0';
    /* Make sure this is just the name, no unneccassry junk */
    if ((tmp = index(realname, ',')) != NULL)
      *tmp = '\0';
    /* Just to be nice, add on the original name */
    strcpy(finalname, realname);
    strcat(finalname, " (");
    strcat(finalname, name);
    strcat(finalname, ")");
  }
  return finalname;
}
