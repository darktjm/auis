/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
	Network monitoring routines for Instrument Console.

 */

#include <andrewos.h>
#include <stdio.h>
#include <console.h>

void CheckNet();
void InitNet();


void CheckNet()
{
    short i;

    mydbg(("entering: CheckNet\n"));
    i = NETRESPONSES;
    if (Numbers[i].IsDisplaying) {
#if 0
	NewValue(self, &Numbers[i], 5, NULL, FALSE);
#else
	printf("Network monitoring doesn't work.\n");
#endif
    }
}

void InitNet()
{
    mydbg(("entering: InitNet\n"));
    Numbers[NETRESPONSES].RawText = (char *)malloc(20);
    strcpy(Numbers[NETRESPONSES].RawText, "Foobar");
}
