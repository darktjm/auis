/**********************************************************************
 * File Exchange pickup client
 *
 * $Author: sa3e $
 * $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/pickup.c,v $
 * $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/pickup.c,v 1.4 1994/01/31 22:36:06 sa3e Stab74 $
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/pickup.c,v 1.4 1994/01/31 22:36:06 sa3e Stab74 $";
#endif

#include <mitcopyright.h>

 

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fxmain.h"

/*** Global variables ***/
long do_dump();
extern int verbose, errno;
int kbytes = 0;

/*
 * pickup_arg checks to see if the current argument indicates
 * a type for which PRESERVE should be set or a type for which
 * we pickup papers authored by others, e.g. handouts.
 */

/*ARGSUSED*/
int
pickup_arg(argc, argv, ip, p, flagp)
     int argc;
     char *argv[];
     int *ip;
     Paper *p;
     int *flagp;
{
  static int initialized = 0;

  if (!initialized) {
    *flagp |= ONE_AUTHOR;
    initialized = 1;
  }
  if (argv[*ip][0] == '-' && index("*TtPheAH", argv[*ip][1])) {
    *flagp |= PRESERVE;
    *flagp &= ~ONE_AUTHOR;
  }
  return(0);
}

/*
 * adjust_criterion -- put filename in criterion
 */

void
adjust_criterion(p, s)
     Paper *p;
     char *s;
{
  p->filename = s;
  return;
}

/*
 * compar -- compares two papers, used by qsort
 */

compar(p1, p2)
     Paper **p1, **p2;
{
  register int ret;

  ret = strcmp((*p1)->filename, (*p2)->filename);
  if (ret) return(ret);
  ret = (int) (*p1)->modified.tv_sec - (*p2)->modified.tv_sec;
  if (ret) return(ret);
  return((int) ((*p1)->modified.tv_usec - (*p2)->modified.tv_usec));
}

/*
 * prep_paper -- gets filename, disk usage for paper
 */

/*ARGSUSED*/
long
prep_paper(p, f, flags)
     Paper *p;			/* paper to retrieve */
     char *f;			/* filename (modified) */
     int flags;			/* flags (unused) */
{
  (void) strcpy(f, p->filename);
  kbytes += ((p->size + 1023) >> 10);
  return(0L);
}

empty_list(criterion)
     Paper *criterion;
{
  printf("No papers to pick up\n");
}

mark_retrieved(fxp, p)
     FX *fxp;
     Paper *p;
{
  Paper pickedup;
  static int warned = 0;
  long code;

  if (!warned) {
    /******** mark file on server as PICKEDUP ********/
    paper_copy(p, &pickedup);
    pickedup.type = PICKEDUP;
    if (!warned && (code = fx_move(fxp, p, &pickedup))) {
      com_err("Warning", code, "-- files not marked PICKEDUP on server.", "");
      warned = 1;
    }
  }
  return;
}

/*
 * main pickup procedure
 */

main(argc, argv)
  int argc;
  char *argv[];
{
  Paper p;

  paper_clear(&p);
  p.type = GRADED;

  if (fxmain(argc, argv,
	     "Usage: %s [options] [assignment] [filename ...]\n",
	     &p, pickup_arg, do_dump)) exit(1);
  if (verbose && kbytes) printf("%d kbytes total\n", kbytes);
  exit(0);
}
