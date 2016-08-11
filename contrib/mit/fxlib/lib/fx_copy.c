/**********************************************************************
 * File Exchange client library
 *
 * $Author: sa3e $
 * $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_copy.c,v $
 * $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_copy.c,v 1.5 1994/02/01 21:34:40 sa3e Stab74 $
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_copy.c,v 1.5 1994/02/01 21:34:40 sa3e Stab74 $";
#endif

#include <mitcopyright.h>

 

#include "fxcl.h"

/*
 * fx_copy -- copy file in exchange to another file in exchange
 */

long
fx_copy(fxp, src, dest)
     FX *fxp;
     Paper *src, *dest;
{
  TwoPaper pp;
  long *ret, code;
  char src_owner[FX_UNAMSZ], src_author[FX_UNAMSZ];
  char dest_owner[FX_UNAMSZ], dest_author[FX_UNAMSZ];

  paper_copy(src, &pp.src);
  paper_copy(dest, &pp.dest);

#ifdef KERBEROS
  /* lengthen usernames to kerberos principals */
  pp.src.owner = _fx_lengthen(fxp, src->owner, src_owner);
  pp.src.author = _fx_lengthen(fxp, src->author, src_author);
  pp.dest.owner = _fx_lengthen(fxp, dest->owner, dest_owner);
  pp.dest.author = _fx_lengthen(fxp, dest->author, dest_author);
#endif

  ret = copy_1(&pp, fxp->cl);
  if (!ret) return(_fx_rpc_errno(fxp->cl));
  code = *ret;
  xdr_free(xdr_long, (char *) ret);
  return(code);
}
