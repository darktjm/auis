/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#undef _DEFAULT_SOURCE

#include <config.h>
#include <sys/types.h>
#include <errno.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#else
#endif

#if defined(STDC_HEADERS) || defined(USG) || defined(SYSV)
#include <string.h>
#else /* not (STDC_HEADERS or USG) */
#include <strings.h>
#endif /* STDC_HEADERS or USG */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef NULL
#define NULL 0
#endif

#if !__STDC__
#define const
#endif

extern char **environ;

/* Put STRING, which is of the form "NAME=VALUE", in the environment.  */
int
putenv (string)
     const char *string;
{
  char *name_end = index (string, '=');
  size_t size;
  char **ep;

  if (name_end == NULL)
    {
      /* Remove the variable from the environment.  */
      size = strlen (string);
      for (ep = environ; *ep != NULL; ++ep)
	if (!strncmp (*ep, string, size) && (*ep)[size] == '=')
	  {
	    while (ep[1] != NULL)
	      {
		ep[0] = ep[1];
		++ep;
	      }
	    *ep = NULL;
	    return 0;
	  }
    }

  size = 0;
  for (ep = environ; *ep != NULL; ++ep)
    if (!strncmp (*ep, string, name_end - string) &&
	(*ep)[name_end - string] == '=')
      break;
    else
      ++size;

  if (*ep == NULL)
    {
      static char **last_environ = NULL;
      char **new_environ = (char **) malloc ((size + 2) * sizeof (char *));
      if (new_environ == NULL)
	return -1;
      (void) bcopy ((char *) environ, (char *) new_environ, size * sizeof (char *));
      new_environ[size] = (char *) string;
      new_environ[size + 1] = NULL;
      if (last_environ != NULL)
	free ((char *) last_environ);
      last_environ = new_environ;
      environ = new_environ;
    }
  else
    *ep = (char *) string;

  return 0;
}
