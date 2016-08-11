/**********************************************************************
 * File Exchange fxls client
 *
 * $Author: sa3e $
 * $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/fxls.c,v $
 * $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/fxls.c,v 1.4 1994/01/31 22:36:06 sa3e Stab74 $
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/clients/RCS/fxls.c,v 1.4 1994/01/31 22:36:06 sa3e Stab74 $";
#endif

#include <mitcopyright.h>

 

#include <stdio.h>
#include <fxcl.h>

main(argc, argv)
  int argc;
  char *argv[];
{
  FX *fxp;
  stringlist_res *sr;
  stringlist l;
  long code;

  if ((fxp = fx_open("", &code)) == NULL) {
    com_err(argv[0], code, "while connecting.");
    exit(1);
  }

  code = fx_directory(fxp, &sr);
  fx_close(fxp);
  if (code) {
    com_err(argv[0], code, "");
    exit(1);
  }

  for(l = sr->stringlist_res_u.list; l != NULL; l = l->next)
    printf("%s\n", l->s);

  exit(0);
}
