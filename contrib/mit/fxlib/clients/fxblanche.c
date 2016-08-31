/**********************************************************************
 * File Exchange access control list client
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
#include <ctype.h>
#include <strings.h>

FX *
open_course(course, module)
     char *course, *module;
{
  FX *fxp;
  long code;

  if (!course) {
    fprintf(stderr, "%s: No course specified.\n", module);
    exit(1);
    }
  fxp = fx_open(course, &code);
  if (code) com_err(module, code, "(%s)", course);
  if (fxp == NULL) exit(1);
  return(fxp);
}

show_list(fxp, course, module, acl, verbose)
     FX *fxp;
     char *course, *module, *acl;
     int verbose;
{
  stringlist_res *sr;
  stringlist l;
  long code;

  if (!fxp) fxp = open_course(course, module);
  code = fx_acl_list(fxp, acl, &sr);
  if (code) {
    com_err(module, code, "(%s)", acl);
    exit(1);
  }
  if (verbose) printf("Members of the %s list for %s:\n", acl, course);
  l = sr->stringlist_res_u.list;
  while(l) {
    if (verbose) printf("\t%s\n", full_name(l->s));
    else printf("%s\n", l->s);
    l = l->next;
  }
  return;
}

main(argc, argv)
  int argc;
  char *argv[];
{
  FX *fxp = NULL;
  long code;
  char *acl;
  int i;
  char *course;
  int specified = 0, verbose = 1;
  static const char USAGE[] = "Usage: %s course [options]\n";

  course = (char *) getenv("COURSE");
  acl = ACL_GRADER;

  for (i=1; i<argc; i++) {
    if (argv[i][0] == '-') {
      switch(argv[i][1]) {
      case 'c':
	course = argv[++i];
	if (fxp) {
	  fx_close(fxp);
	  fxp = NULL;
	}
	break;
      case 'a':
	specified++;
	if (!fxp) fxp = open_course(course, argv[0]);
	code = fx_acl_add(fxp, acl, argv[++i]);
	if (code) {
	  com_err(argv[0], code, "(%s %s)", acl, argv[i]);
	  exit(1);
	}
	if (verbose)
	  printf("Added %s to the %s list for %s.\n",
		 full_name(argv[i]), acl, course);
	break;
      case 'd':
	specified++;
	if (!fxp) fxp = open_course(course, argv[0]);
	code = fx_acl_del(fxp, acl, argv[++i]);
	if (code) {
	  com_err(argv[0], code, "(%s %s)", acl, argv[i]);
	  exit(1);
	}
	if (verbose)
	  printf("Deleted %s from the %s list for %s.\n",
		 full_name(argv[i]), acl, course);
	break;
      case 'm':
	specified++;
	show_list(fxp, course, argv[0], acl, verbose);
	break;
      case 'l':
	acl = argv[++i];
	break;
      case 'q':
        verbose = 0;
        break;
      case 'v':
        verbose = 1;
        break;
      default:
	fprintf(stderr, USAGE, argv[0]);
	break;
      }
      continue;
    }
    course = argv[i];
  }
  if (!specified) show_list(fxp, course, argv[0], acl, verbose);
  exit(0);
}
