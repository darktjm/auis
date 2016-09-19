/* ********************************************************************** *\
 *	Copyright Carnegie Mellon University 1995 - All Rights Reserved
 *	For full copyright information see:'andrew/doc/COPYRITE'
\* ********************************************************************** */

/* astringtest.C

	test the astring object

 *    $Log: astringtest.C,v $
 * Revision 1.1  1995/09/23  15:18:47  wjh
 * Initial revision
 *
 * Copied from /usr/andrew/lib/genericinset
 */

#include <andrewos.h>
#include <astring.H>

extern void doStaticLoads();


void main(int argc, char **argv) {
    	printf("Start\n"); fflush(stdout);
	doStaticLoads(); 
    	printf("Loaded\n"); fflush(stdout);


	AString title, name;

// usage:
	title = "Duchess";
		printf("title is %s\n", (char *)title);
	title.Append(" of Kent");
		printf("title is now %s\n", (char *)title);
	name = "Clara";
	name.Append(", ").Append(title);	// Clara, Duchess of Kent
		printf("name is %s\n", (char *)name);
	int i = name.Search("of");		// 15
		printf("index of 'of' in %s is %d\n", (char *)name, i);
	name.Replace(i, 2, "for");
		printf("name is %s\n", (char *)name);
	name.ReplaceN(3, 4, name+4, 6);
		printf("name is %s\n", (char *)name);
	name.ReplaceN(3, 4, name+1, 6);
		printf("name is %s\n", (char *)name);
	name.ReplaceN(3, 4, name+4, 2);
		printf("name is %s\n", (char *)name);
	title = "Moby Dick";
		printf ("%s\n", (char *)title);	// prints "Moby Dick\n"
	boolean foo = (title == "Moby Dick");	// TRUE
		printf("title %s Moby Dick\n", (foo)?"IS":"ISNT");
	name.Sprintf("any%s %d u\n", "thing", 4); // "anything 4 u\n"
		printf(name);
}
