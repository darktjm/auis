/**********************************************************************
 * File Exchange client module to get sorted array from Paperlist
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

 

#include <fxcl.h>
#include <memory.h>

/* Another client module must define comparator for sorting */
int compar();

/*
 * get_array -- fill in the array of papers to collect
 */

get_array(list, ppp)
     Paperlist list;
     Paper ***ppp;
{
  Paperlist node;
  int i = 0, count = 0;

  /* Count papers in list */
  for(node = list; node; node = node->next) i++;

  if (i == 0) return(0);

  *ppp = NewArray(Paper *, i);
  if (!*ppp) return(0);

  /* Put papers into sorted array */
  for(node = list; node; node = node->next)
    (*ppp)[count++] = &node->p;
  qsort((char *) *ppp, count, sizeof(Paper *), compar);

  return(count);
}
