/**********************************************************************
 * File Exchange purge client
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

#include <mitcopyright.h>

 

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sys/errno.h>
#include "fxmain.h"

#define DAY (86400L)		/* number of seconds in one day */

/*** Global variables ***/
long do_purge();
char *full_name();

/*
 * compar -- compares two papers, used by qsort
 */

compar(p1, p2)
     Paper **p1, **p2;
{
  register int ret;

  ret = strcmp((*p1)->author, (*p2)->author);
  if (ret) return(ret);
  ret = strcmp((*p1)->filename, (*p2)->filename);
  if (ret) return(ret);
  ret = (int) (*p1)->modified.tv_sec - (*p2)->modified.tv_sec;
  if (ret) return(ret);
  return((int) ((*p1)->modified.tv_usec - (*p2)->modified.tv_usec));
}

/*
 * prep_paper -- prints info before paper is purged
 */

long
prep_paper(p, flags)
     Paper *p;			/* paper to purge */
     int flags;
{
  static char *old_author = NULL;

  if (flags & VERBOSE) {
    if (!old_author || strcmp(p->author, old_author))
      printf("%s:\n", full_name(p->author));
    old_author = p->author;
  }

  return(0L);
}

empty_list(criterion)
     Paper *criterion;
{
  if (criterion->author)
    printf("%s:\n", full_name(criterion->author));
  printf("No papers to purge\n");
}

/*
 * main purge procedure
 */

main(argc, argv)
  int argc;
  char *argv[];
{
  Paper p;

  paper_clear(&p);
  p.type = PICKEDUP;
  if (fxmain(argc, argv,
             "Usage: %s [-c course] [options] [assignment] [student ...]\n",
             &p, NULL, do_purge)) exit(1);
  exit(0);
}
