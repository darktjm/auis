/**********************************************************************
 * File Exchange purge module
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

#include <mitcopyright.h>

 

#include <stdio.h>
#include <sys/time.h>
#include <memory.h>
#include <ctype.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include "fxmain.h"

/*** Global variables ***/
Paper **paperv;
char *full_name();
char purge_err_context[1024];
int compar();
long prep_paper();

/*
 * do_dump -- dumps papers into files
 */

long
do_purge(fxp, criterion, flags, user)
     FX *fxp;
     Paper *criterion;
     int flags;
     char *user;
{
  long code;
  Paperlist_res *plist;
  int count, i, found = 0;

  criterion->author = user;
  /******** get list of papers from server ********/
  code = fx_list(fxp, criterion, &plist);
  if (code) {
    strcpy(purge_err_context, "while retrieving list");
    return(code);
  }

  count = get_array(plist->Paperlist_res_u.list, &paperv);

  /******** deal with empty list ********/
  if (count == 0) {
    if (flags & VERBOSE)
      empty_list(criterion);
    return(0L);
  }

  /******** main loop through list ********/
  for (i=0; i<count; i++) {

    /******* Skip papers not in time range ********/
    if (paperv[i]->modified.tv_sec < criterion->created.tv_sec ||
	paperv[i]->modified.tv_sec > criterion->modified.tv_sec) continue;

    found = 1;

    prep_paper(paperv[i], flags);

    if (flags & VERBOSE) {
      /******** print information about file ********/
      printf("%5d %-9s %9d  %-16.16s  %s\n", paperv[i]->assignment,
	     paperv[i]->owner, paperv[i]->size,
	     ctime(&(paperv[i]->modified.tv_sec)), paperv[i]->filename);
    }

    if (!(flags & LISTONLY)) {
      /******** delete file from server ********/
      code = fx_delete(fxp, paperv[i]);
      if (code) {
	strcpy(purge_err_context, "while deleting.");
	return(code);
      }
    }
  }

  /******** clean up ********/
  if (!found && (flags & VERBOSE)) empty_list(criterion);
  fx_list_destroy(&plist);
  free((char *) paperv);
  return(0L);
}
