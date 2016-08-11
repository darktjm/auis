/**********************************************************************
 * File Exchange client library
 *
 * $Author: sa3e $
 * $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_retrieve.c,v $
 * $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_retrieve.c,v 1.5 1994/02/01 21:34:53 sa3e Stab74 $
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_retrieve.c,v 1.5 1994/02/01 21:34:53 sa3e Stab74 $";
#endif

#include <mitcopyright.h>

 

#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include "fxcl.h"
#include "memory.h"
#define brst(res) (res->retrieve_res_u.burst)

/*
 * fx_retrieve -- retrieve a stream from the exchange
 */

long
fx_retrieve(fxp, p, fp)
     FX *fxp;
     Paper *p;
     FILE *fp;
{
  FX *f;
  long *res1, code = 0L;
  init_res *ires;
  retrieve_res *res = NULL;
  int dummy;
  Paper to_retrieve;
  char new_owner[FX_UNAMSZ], new_author[FX_UNAMSZ];

  f = fxp;
  if (strcasecmp(fxp->host, p->location.host)) {
    if ((f = New(FX)) == NULL) return((long) errno);
    (void) strcpy(f->host, p->location.host);
    (void) strcpy(f->name, fxp->name);
    code = fx_init(f, &ires);
    if (code && code != ERR_NOT_SYNC) {
      free((char *)f);
      return(code);
    }
  }

  paper_copy(p, &to_retrieve);

#ifdef KERBEROS
  /* lengthen usernames to kerberos principals */
  to_retrieve.owner = _fx_lengthen(fxp, p->owner, new_owner);
  to_retrieve.author = _fx_lengthen(fxp, p->author, new_author);
#endif

  res1 = retrieve_file_1(&to_retrieve, f->cl);
  if (!res1) {
    code = _fx_rpc_errno(f->cl);
    goto FX_RETRIEVE_CLEANUP;
  }
  code = *res1;
  xdr_free(xdr_long, (char *) res1);
  if (code) goto FX_RETRIEVE_CLEANUP;

  do {
    if (res) xdr_free(xdr_retrieve_res, (char *) res);
    res = retrieve_burst_1(&dummy, f->cl);
    if (!res) {
      code = _fx_rpc_errno(f->cl);
      goto FX_RETRIEVE_CLEANUP;
    }
    if (code = res->errno)
      goto FX_RETRIEVE_CLEANUP;

    if (fwrite(brst(res).data, 1, brst(res).size, fp) < brst(res).size) {
      code = (long) errno;
      goto FX_RETRIEVE_CLEANUP;
    }
  } while (brst(res).size == MAX_BURST_SIZE);

 FX_RETRIEVE_CLEANUP:
  if (res) xdr_free(xdr_retrieve_res, (char *) res);
  if (f != fxp) fx_close(f);
  return(code);
}
