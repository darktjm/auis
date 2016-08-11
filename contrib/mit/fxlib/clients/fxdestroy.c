/**********************************************************************
 * File Exchange fxdestroy client
 *
 * $Author: sa3e $
 * $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/fxdestroy.c,v $
 * $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/fxdestroy.c,v 1.4 1994/01/31 22:36:06 sa3e Stab74 $
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/fxdestroy.c,v 1.4 1994/01/31 22:36:06 sa3e Stab74 $";
#endif

#include <mitcopyright.h>

 

#include <stdio.h>
#include <fxcl.h>

main(argc, argv)
  int argc;
  char *argv[];
{
  FX *fxp;
  long code;

  if (argc == 2) {
    if ((fxp = fx_open(argv[1], &code)) == NULL) {
      com_err(argv[0], code, "while connecting");
      exit(1);
    }
    if (code = fx_destroy(fxp, argv[1])) {
      com_err(argv[0], code, "trying to destroy %s", argv[1]);
      exit(1);
    }
    printf("Destroyed %s.\n", argv[1]);
    exit(0);
  }
  fprintf(stderr, "Usage: %s <course name>\n", argv[0]);
  exit(1);
}
