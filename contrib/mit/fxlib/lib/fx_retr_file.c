/**********************************************************************
 * File Exchange client library
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

 

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fxcl.h"

/*
 * fx_retriv_file -- retrieve a file from the exchange
 */

long
fx_retriv_file(fxp, p, filename)
     FX *fxp;
     Paper *p;
     char *filename;
{
  FILE *fp;
  struct stat buf;
  long code;

  if ((fp = fopen(filename, "w")) == NULL) return((long) errno);
  code = fx_retrieve(fxp, p, fp);
  if (fclose(fp) == EOF && !code) code = (long) errno;

  /* If there's an error, don't leave a zero-length file. */
  if (code) {
    if (!stat(filename, &buf) && !buf.st_size) unlink(filename);
    return(code);
  }

  /* check file status */
  if (p->flags & PAPER_EXECUTABLE) {
    if (stat(filename, &buf)) return((long) errno);
    if (buf.st_mode & S_IEXEC) return(code);
    /* copy read permission to execute permission */
    if (chmod(filename, (int) (((0444 & buf.st_mode) >> 2) | buf.st_mode)))
      return((long) errno);
  }

  return(code);
}
