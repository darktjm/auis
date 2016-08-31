/**********************************************************************
 * File Exchange client
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

 

#include <stdio.h>
#include <fxcl.h>
#include <ss/ss.h>

/*** Global variables ***/

char Course[COURSE_NAME_LEN];
Paperlist_res *Plist = NULL;
Paper Criterion;

main(argc, argv)
  int argc;
  char *argv[];
{
  long code;
  extern ss_request_table fx_ct;
  int idx;
  char *course;

  course = (char *) getenv("COURSE");
  if (course) (void) strcpy(Course, course);

  idx = ss_create_invocation("fx", "0.1", 0, &fx_ct, &code);
  if (code) {
    com_err(argv[0], code, "");
    exit(1);
  }

  paper_clear(&Criterion);

  printf("This program is still experimental.  Type 'q' to exit.\n");
  ss_listen(idx, &code);
  if (code) {
    com_err(argv[0], code, "");
    exit(1);
  }
  exit(0);
}
