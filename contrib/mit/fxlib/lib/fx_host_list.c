/**********************************************************************
 * File Exchange client library
 *
 * $Author: sa3e $
 * $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_host_list.c,v $
 * $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_host_list.c,v 1.5 1994/02/01 21:35:03 sa3e Stab74 $
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_host_list.c,v 1.5 1994/02/01 21:35:03 sa3e Stab74 $";
#endif

#include <mitcopyright.h>

 

#include <strings.h>
#ifdef HESIOD
#include <hesiod.h>
#endif /* HESIOD */
#include <fxcl.h>
#include <memory.h>

char *getenv();			/* this ought to be in a system .h file */

stringlist
fx_host_list()
{
  stringlist ret = NULL;
  stringlist node, newret;
  char *fxpath, *s;
  int i, count=0;
#ifdef HESIOD
  char **hesinfo;
#endif /* HESIOD */

  /* allocate first node of stringlist */
  if ((ret = New(stringnode)) == NULL) return(NULL);
  ret->next = NULL;
  ret->s = NULL;
  
  /* if FXPATH environment variable is set, use it */
  if ((fxpath = getenv(ENV_FXPATH)) != NULL) {

    for (node = ret; node; node = node->next = New(stringnode)) {
      node->next = NULL;

      /* find the next host */
      if (*fxpath == '\0') break;
      s = index(fxpath, ':');
      if (s) i = (int) (s - fxpath);
      else i = strlen(fxpath);

      /* allocate space to copy it into */
      if ((node->s = NewArray(char, i+1)) == NULL) {
	fx_host_list_destroy(ret);
	return(NULL);
      }

      /* copy it */
      (void) strncpy(node->s, fxpath, i);
      node->s[i] = '\0';
      count++;

      /* get ready to copy the next name */
      if (!s) break;
      fxpath = 1 + s;
    }
  }
#ifdef HESIOD
  else {
    /* Get server locations from hesiod */
    if ((hesinfo = hes_resolve("turnin", "sloc")) == NULL) {
      fx_host_list_destroy(ret);
      return(NULL);
    }

    for (node = ret; node; node = node->next = New(stringnode)) {
      node->next = NULL;

      if ((node->s = newstring(*hesinfo)) == NULL) {
	fx_host_list_destroy(ret);
	return(NULL);
      }
      (void) strcpy(node->s, *hesinfo++);
      count++;
      if (*hesinfo == NULL) break;
    }
  }
#endif /* HESIOD */
  /*
   * Now we rotate the list of server hosts based on pid.  This will
   * cause fxopen to choose a server approximately at random, evening
   * the load among servers.
   */
  newret = ret;
  for(i = getpid()%count ; i>0 ; i--) {
    node = newret;
    if (newret->next) newret = newret->next;
    if (i == 1) node->next = NULL;
  }
  if (newret != ret) {
    node = newret;
    while (node->next) node = node->next;
    node->next = ret;
    ret = newret;
  }
  return(ret);
}
