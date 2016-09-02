/*LIBS: -lbasics -lclass -lerrors -lutil
*/

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

 


/* testdo.c
	test the lookz data object
*/


#include <andrewos.h>
#include <stdio.h>

#include <observable.H>
#include <proctable.H>
#include <dataobject.H>

#include <lookz.H>

main( int	   argc, char   **argv );
printdata(class lookz  *st);


main( int	   argc, char   **argv )
		{
	class lookz *st, *st2;
	FILE *f;

	printf("Start\n"); fflush(stdout);
	ATK_Init(".");		/* use current directory for dataobject path (???) */
	printf("Init done\n"); fflush(stdout);

	observable_StaticEntry;
	proctable_StaticEntry;
	dataobject_StaticEntry;
/*
	lookz_StaticEntry;
*/
	printf("About to New\n"); fflush(stdout);
	st = new lookz;

	printdata(st);
	(st)->SetVisibility( ! (st)->GetVisibility());
	printdata(st);

	(st)->Destroy();

	printf("\n Phase II\n");  fflush(stdout);
	st = new lookz;

	printdata(st);

	printf("\nWriting plain data stream\n");  fflush(stdout);
	f = fopen("/tmp/lookzfoo", "w");
	(st)->Write( f, 0, 0);
	fclose(f);

	printf("Reading\n");  fflush(stdout);
	f = fopen("/tmp/lookzfoo", "r");
	st2 = new lookz;
	(st2)->Read( f, 0);
	fclose(f);

	printdata(st2);
	(st2)->Destroy();

	/* the next file should be empty because the
		WriteID is the same as above */
	printf("\nReWriting with same WriteID\n");  fflush(stdout);
	f = fopen("/tmp/lookzfoo2", "w");
	(st)->Write( f, 0, 2);
	fclose(f);

	(st)->SetVisibility( FALSE);
	printf("\nWriting with headers\n");  fflush(stdout);
	f = fopen("/tmp/lookzfoo3", "w");
	(st)->Write( f, 1, 2);
	fclose(f);


	printf("Reading\n");  fflush(stdout);
	f = fopen("/tmp/lookzfoo3", "r");
	while (TRUE) 
		if((fgetc(f)) == '}') break;
	st2 = new lookz;
	(st2)->Read( f, 3);
	fclose(f);

	printdata(st2);
}

printdata(class lookz  *st)
	{
	printf("Image is %s\n", ((st)->GetVisibility() ? "visible" : "hidden"));
	fflush(stdout);
}
