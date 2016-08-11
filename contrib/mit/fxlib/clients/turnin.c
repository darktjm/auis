/**********************************************************************
 * File Exchange turnin client
 *
 * $Author: sa3e $
 * $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/turnin.c,v $
 * $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/turnin.c,v 1.4 1994/01/31 22:36:06 sa3e Stab74 $
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/turnin.c,v 1.4 1994/01/31 22:36:06 sa3e Stab74 $";
#endif

#include <mitcopyright.h>
#include <stdio.h>
#include "fxmain.h"

/*
 * turnin_arg checks to see if the current argument is -g author,
 * which is an option unique to the turnin program.
 */

/*ARGSUSED*/
int
turnin_arg(argc, argv, ip, p, flagp)
     int argc;
     char *argv[];
     int *ip;
     Paper *p;
     int *flagp;
{
  if (argv[*ip][0] == '-' && argv[*ip][1] == 'g') {
    p->type = GRADED;
    p->author = argv[++(*ip)];
    return(1);
  }
  return(0);
}

long
do_turnin(fxp, p, flags, filename)
     FX *fxp;
     Paper *p;
     int flags;
     char *filename;
{
  long code;

  /* If no filename is specified, use stdin if it's not a terminal. */
  if (!filename) {
    if (isatty(0)) return(ERR_USAGE);
    if (!p->filename) p->filename = "stdin";
    if (code = fx_send(fxp, p, stdin))
      strcpy(fxmain_error_context, "while sending from stdin");
    return(code);
  }

  /* If a filename is specified, send that file. */
  if (code = fx_send_file(fxp, p, filename))
    sprintf(fxmain_error_context, "while sending %s", filename);
  else if (flags & VERBOSE) {
    if (p->type == GRADED) printf("Returned %s to %s in %s.", filename,
				  full_name(p->author), fxp->name);
    else printf("Turned in %s to %s on %s.\n",
		filename, fxp->name, fxp->host);
  }
  return(code);
}

main(argc, argv)
     int argc;
     char *argv[];
{
  Paper turnin_paper;

  paper_clear(&turnin_paper);
  turnin_paper.type = TURNEDIN;
  if (fxmain(argc, argv,
	     "USAGE: %s [options] assignment filename\n",
	     &turnin_paper, turnin_arg, do_turnin)) exit(1);
  exit(0);
}
