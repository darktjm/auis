/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
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
