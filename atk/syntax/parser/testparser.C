/* testparser.c	- test the parser object */

#include <stdio.h>
#include <foo.H>


	static int
lex(void *ls, void *yylval) {
	int *lexstate=(int *)ls;

/* tokens: 
	3	"FOO"
	4	"'?'"
	5	"'a'"
	6	"'b'"
	7	"'/'"
*/
	static short dummy[] = {
		/* a  b  ?  / FOO a  b FOO b  a  /  ?  / FOO */
		   5, 6, 4, 7, 3, 5, 6, 3, 6, 5, 7, 4, 7, 3
	};
	int dummyinx = *lexstate;
	(*lexstate)++;
	return (dummyinx < sizeof(dummy)/sizeof(short)) ? dummy[dummyinx] : 0;
}

int main()
{
	int lexstate = 0;
	class foo *p = new foo;
	parser::SetDebug( 1 );
	printf("parser returns %d\n", (p)->Parse(lex, &lexstate));
	exit(0);	/* keep 'make' happy */
}
