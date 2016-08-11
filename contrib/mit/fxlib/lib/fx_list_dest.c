/**********************************************************************
 * File Exchange client library
 *
 * $Author: sa3e $
 * $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_list_dest.c,v $
 * $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_list_dest.c,v 1.5 1994/02/01 21:35:08 sa3e Stab74 $
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_list_dest.c,v 1.5 1994/02/01 21:35:08 sa3e Stab74 $";
#endif

#include <mitcopyright.h>

 

#include "fxcl.h"

/*
 * fx_list_destroy -- free memory allocated for paper list
 */

void
fx_list_destroy(plist)
     Paperlist_res **plist;
{
  Paperlist node;

  if (plist && *plist) {

    for (node = (*plist)->Paperlist_res_u.list; node; node = node->next) {
#ifdef KERBEROS
    /* be cautious; restore names to original length */
      _fx_unshorten(node->p.owner);
      _fx_unshorten(node->p.author);
#endif
      xdr_free(xdr_Paperlist_res, (char *) *plist);
      *plist = NULL;
    }
  }
}
