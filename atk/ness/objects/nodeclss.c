/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1993 - All Rights Reserved
\* ********************************************************************** */

/* nodeclss.c 
	translate .Hn files to .H files

	convert nodeclass declarations to struct declarations and
	define for each the macros  _New,  _Destroy,  and _Create

usage:
	nodeclss filename.Hn

result:
	file named filename.H


sample input section:

    nodeclass varnode {
	long loc, len;
	struct nesssym *sym;
	struct toksym *paren;
    };


corresponding output:

    struct varnode {
	long loc, len;
	struct nesssym *sym;
	struct toksym *paren;
    };

    #define varnode_New() ((struct varnode *)malloc(sizeof(struct varnode)))
    #define varnode_Destroy(n) (free(n))
    #define varnode_Create(QZloc, QZlen, QZsym, QZparen) \
	(QTnode = (struct node *)malloc(sizeof(struct varnode)), \
		((struct varnode *)QTnode)->loc = QZloc, \
		((struct varnode *)QTnode)->len = QZlen, \
		((struct varnode *)QTnode)->sym = QZsym, \
		((struct varnode *)QTnode)->paren = QZparen, \
	(struct varnode *)QTnode)

*/

#include <andrewos.h>
#include <ctype.h>

char filestem[100];		/* stem of input filename */



FILE *outf, *inf;

char CurTok[200];


static void SkipComment();
static void SkipString(char  d	/* the close delimiter */);
static void PutToken();
static boolean GetToken();
static void PutNodeClass();
int main(int argc, char **argv);


	static void 
SkipComment(void) {
	int c;
	boolean SawStar = FALSE;
	putc('*', outf);
	while ((c=getc(inf)) != EOF)  {
		putc(c, outf);
		if (c == '/'  &&  SawStar)
			return;
		else SawStar = (c == '*');
	}
}

	static void
SkipString(char  d	/* the close delimiter */) {
	int c;
	putc(d, outf);	/* open the string */
	while ((c=getc(inf)) != d)  {
		putc(c, outf);
		if (c == '\\') 
			putc(getc(inf), outf);
	}
	putc(d, outf);	/* close the string */
}


	static void
PutToken(void)  {
	fprintf(outf, "%s", CurTok);
}

/* GetToken gets a token to CurTok
	It also transfers to the output any strings, whitespace, and comments
	    it sees between tokens.
	Return TRUE except at end of file.
	Note that numeric values are treated with each digit as a token.
*/
	static boolean
GetToken(void) {
	int c;
	char *cx;
	while ((c = getc(inf)) != EOF)  {
		if (isspace(c)) putc(c, outf);
		else if (isalpha(c)) {
			/* process an id. leave in CurTok */
			cx = CurTok;
			do {
				*cx++ = c;
			}  while (isalpha((c=getc(inf))));
			ungetc(c, inf);
			*cx = '\0';
			return TRUE;
		}
		else if (c == '"') 
			/* pass through a string */
			SkipString('"');
		else if (c == '\'') 
			/* pass through a character */
			SkipString('\'');
		else if (c == '/') {
			putc('/', outf);
			if ((c=getc(inf)) == '*')
				SkipComment();
			else ungetc(c, inf);
		}
		else {
			/* plain character:  return it as a token */
			CurTok[0] = c;
			CurTok[1] = '\0';
			return TRUE;
		}
	}
	return FALSE;  /* EOF */
}

	static void
PutNodeClass(void) {
	int c;

	char fields[30][100];	/* max of 30 field names of 100 chars each */
	char nclassname[30];

	int fx, tx;		/* point to a field name */

	fprintf(outf, "%s", "struct");
	GetToken();  PutToken();	/* get nodeclass name */
	strcpy(nclassname, CurTok);

	/* collect field names
		cheat:  a field name is the last token before ',' or ';'   */
	fx = 0;	/* where to store next field name */
	while (GetToken()) {
		PutToken();
		if (*CurTok == '}') break;
		if (*CurTok == ',' || *CurTok == ';') 
			/* the previous one was a field name */
			fx++;
		else strcpy(&fields[fx][0], CurTok);
	}
	while ((c=getc(inf)) != '\n')
		putc(c, outf);

	/* generate the macros  */
	fprintf(outf, "\n\n#define %s_New() ((struct %s *)malloc(sizeof(struct %s)))\n",
		nclassname, nclassname, nclassname);
	fprintf(outf, "#define %s_Destroy(n) (free(n))\n", nclassname);
	fprintf(outf, "#define %s_Create(QZ%s", nclassname, fields[0]);
	for (tx = 1; tx < fx; tx++)
		fprintf(outf, ", QZ%s", fields[tx]);
	fprintf(outf, ") \\\n");
	fprintf(outf, "\t(QTnode = (struct node *)malloc(sizeof(struct %s)), \\\n", nclassname);
	for (tx = 0; tx < fx; tx++)
		fprintf(outf, "\t\t((struct %s *)QTnode)->%s = QZ%s, \\\n", 
				nclassname, fields[tx], fields[tx]);
	fprintf(outf, "\t(struct %s *)QTnode)\n\n", nclassname);
}


	int
main(int argc, char **argv) {
	char *dot;

	dot = strrchr(argv[1], '.');
	if (dot)
		sprintf(filestem, "%.*s", (int)(dot-argv[1]), argv[1]);
	else
		strcpy(filestem, argv[1]);

	inf = fopen(argv[1], "r");
	if (argc > 2)
		outf = fopen(argv[2], "w");
	else {
		char outname[300];
		sprintf(outname, "%s.H", filestem);
		outf = fopen(outname, "w");
	}
	if (inf == NULL) {
		printf("Invalid input file name.  ");
		perror("");
		exit(1);
	}
	if (outf == NULL) {
		printf("Invalid output file name.  ");
		perror("");
		exit(1);
	}


	/* search for "nodeclass" and process it */

	while (GetToken()) 
		if (strcmp(CurTok, "nodeclass") == 0) 
			/* bingo */
			PutNodeClass();
		else PutToken();

	exit(0);
}
