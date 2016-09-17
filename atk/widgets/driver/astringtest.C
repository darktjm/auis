/* ********************************************************************** *\
 *	Copyright Carnegie Mellon University 1995 - All Rights Reserved
 *	For full copyright information see:'andrew/doc/COPYRITE'
\* ********************************************************************** */

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
