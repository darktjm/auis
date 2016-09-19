/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>

/*
	sys.c		
	Print name that describes current machine and operating
	system.
*/

#ifndef NULL
#define NULL (char *)0
#endif

int main (int argc,const char **argv)
{
  char retval[1000], *s;

  strcpy(retval, SYS_NAME);

  while (*++argv!=NULL)
    if (**argv=='-' && (*argv)[1]=='c' && (*argv)[2]=='\0') {
      if ((s=index(retval, '_')) != NULL)
	*s = '\0';
      break;
    }
  printf("%s\n", retval);
  exit(0); /* Don't return bogoid status... -zs */
}

