/**********************************************************************
 * File Exchange client library
 *
 * $Author: sa3e $
 * $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_dir_dest.c,v $
 * $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_dir_dest.c,v 1.5 1994/02/01 21:35:06 sa3e Stab74 $
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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/fxlib/lib/RCS/fx_dir_dest.c,v 1.5 1994/02/01 21:35:06 sa3e Stab74 $";
#endif

#include <mitcopyright.h>

 

#include "fxcl.h"

/*
 * fx_directory_destroy -- free memory allocated for directory
 */

void
fx_directory_destroy(list)
     stringlist_res **list;
{
  if (list && *list) xdr_free(xdr_stringlist_res, (char *) *list);
  *list = NULL;
}
