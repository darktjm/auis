/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

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

/*
 * app for toez
 *
 *	Program to convert from various file types to andrew object type.
 *
 *	The program attempts to analyze the input to determine what sort of
 *	data it might contain and to convert it to the appropriate ATK object file.
 *
 *	Main switches:
 *		-h, -help	print explanation
 *		-scribe	input IS Scribe
 *		-nroff	input IS nroff
 *		-troff	input IS troff
 *		-	input from stdin	(analysis is limited)
 *		-o FILE	output to file named FILE
 *
 *	Switches for nroff/troff
 *		-H	special mode for help system
 *		-b	completely bogus (?)
 *		-m MAC	/usr/lib/tmac/tmac.MAC
 * 
 *	If no input file is given, input is from stdin.
 *	If no output file is given, stdout is assumed.
 */

#include <andrewos.h>
ATK_IMPL("toezapp.H")
#include <stdio.h>
#include <toezapp.H>
#include <text.H>
#include <rofftext.H>

	
ATKdefineRegistry(toezapp, application, NULL);
static void show_usage(class toezapp  *self);
static void InsistOn(const char  *arg , const char  *assumed);
static void AnalyzeInput(class toezapp  *self);
static void OpenOutputFile(class toezapp  *self);
static void frofftext(class toezapp  *self);
static void errhandler(long  lineno, char  *msg);
static void describe(class toezapp  *self);
extern boolean ReadScribeFromFileToDoc(FILE  *f , class text  *doc , int  pos , void  (*errhandler)(long, char *));


toezapp::toezapp()
		{
	this->macrofile = NULL;
	this->inputtype = NULL;
	this->outputfile = NULL;
	this->inputfile = NULL;
	this->inf = NULL;
	this->outf = NULL;
	this->HelpMode = FALSE;
	this->BeCompletelyBogus = FALSE;
	(this)->SetMajorVersion( 7);
	(this)->SetMinorVersion( 0);
	(this)->SetFork(FALSE);
	THROWONFAILURE(  TRUE);
}

	toezapp::~toezapp()
		{
}


/*
 * usage statement
 */
	static void
show_usage(class toezapp  *self)
	{
	fprintf(stderr,
		"Usage: toez [-help] [-scribe | [[-nroff | -troff][-m macro file][-H]]] [-o outputfile] [-] [file]\n");
	fprintf(stderr,
"	-help: show this usage statement\n"
"	-scribe: assume Scribe input (default for .mss)\n"
"	-nroff: pretend to be nroff (default for .n)\n"
"	-troff: pretend to be troff (default for .t)\n"
"	-m file: read in /usr/lib/tmac/tmac.file as a macro file\n"
"	-H: format for use with the ATK Help system.  Crush initial blank space\n"
"	-o file: send output file to 'file'.  (default is .d)\n"
"	-: use standard input as the file input\n"
"	file: read in this files\n"
);
}

	static void
InsistOn(const char  *arg , const char  *assumed)
	{
	if (strcmp(arg+1, assumed) != 0)
		fprintf(stderr, "Assuming -%s instead of \"%s\"\n", assumed, arg);
}


	boolean 
toezapp::ParseArgs(int  argc,const char  **argv)
			{
	char temp2[128];

	if(!(this)->application::ParseArgs( argc, argv))
		return FALSE;

#define GETARGSTR(var)\
{\
	if((*argv)[2] != '\0')\
		var= ((*argv)[2] == '=' ? &(*argv)[3] : &(*argv)[2]);\
	else if(argv[1] == NULL){\
		fprintf(stderr,"%s: %s switch requires an argument.\n",\
				(this)->GetName(), *argv);\
		return FALSE;\
	}else {\
			var = *++argv;\
		argc--;\
	}\
}

	while(*++argv != NULL && **argv == '-') {
		boolean stop = FALSE;
		switch((*argv)[1]){
				const char *temp;
			case 's':
				InsistOn(*argv, "scribe");			
				this->inputtype = "scribe";
				break;
			case 'n':
				InsistOn(*argv, "nroff");
				this->inputtype = "nroff";
				break;
			case 't':
				InsistOn(*argv, "troff");
				this->inputtype = "troff";
				break;
			case 'm':
				GETARGSTR(temp);
				sprintf(temp2,"/usr/lib/tmac/tmac.%s",temp);
				this->macrofile = strdup(temp2);
				break;
			case 'o':
				GETARGSTR(this->outputfile);
				break;
			case 'h':
				show_usage(this);
				exit(0);
			case 'H':					
				this->HelpMode = TRUE;
				break;
			case 'b':
				this->BeCompletelyBogus = TRUE;
				break;
			case '\0':
				stop = TRUE;
				break;	/* for stdin, use '-' */
			default:
				fprintf(stderr,"%s: unrecognized switch: %s\n",
					(this)->GetName(), *argv);
				show_usage(this);
				return FALSE;
		}
		if (stop)
			break;
		argc--;
	}

	/* are there input filenames? */

	if (*argv != NULL)
		this->inputfile = *argv;

	return TRUE;
}

	boolean
toezapp::Start()
	{
	return TRUE;
}


	static void
AnalyzeInput(class toezapp  *self)
	{
	long c;
	char *extension;

	if (self->inputfile == NULL)
		self->inf = stdin;
	else 
		self->inf = fopen(self->inputfile,"r");
	if (self->inf == NULL) {
		perror(self->inputfile);
		exit (1);
	}

	if (self->inputtype != NULL) 
		return;

	/* no input type given.  Try to get one from extension. */
	/* XXX we should call a test routine from each file type */

	if (self->inputfile != NULL) {
		extension = (char *)strrchr(self->inputfile, '.');
		if (extension == NULL) {}
		else if (strcmp(extension, ".mss") == 0) 
			self->inputtype = "scribe";
		else if (strcmp(extension, ".n") == 0) 
			self->inputtype = "nroff";
		else if (strcmp(extension, ".t") == 0) 
			self->inputtype = "troff";
		if (self->inputtype != NULL) {
			fprintf(stderr, "Assuming -%s because of extension %s\n",
				self->inputtype, extension);
			return;
		}
	}

	/* extension didn't help.  Examine start of file. */

	c = getc(self->inf);
	ungetc(c, self->inf);
	
	switch (c) {
	case '@':   self->inputtype = "scribe";  return;
	case '#':    self->inputtype = "nroff";  return;
	case '.':    self->inputtype = "nroff";  return;
	}
	if (self->inputtype != NULL) {
		fprintf(stderr, "Assuming -%s because of first character %c\n",
			self->inputtype, (int)c);
		return;
	}

	/* give up.  Assume ASCII or scribe */
	self->inputtype = "scribe";	
	fprintf(stderr, "Assuming -%s by default\n", self->inputtype);
}

	static void
OpenOutputFile(class toezapp  *self)
	{
	/* XXX should check the extension on the file name versus
		the type deduced */
	if (self->outputfile)
		self->outf = fopen(self->outputfile, "w");
	else
		self->outf = stdout;
	if (self->outf == NULL) {
		perror(self->outputfile);
		exit(1);
	}
}


	static void
frofftext(class toezapp  *self)
	{
	class rofftext *r;
	class text *t;
	char **ti;

	if(!(self)->application::Start()) return;

	r = new rofftext;
	if (r == NULL) return;

	r->inputfiles = ti = (char **)malloc(2 * sizeof(char *));
	ti[0] = NULL;
	ti[1] = NULL;
	r->filename = self->inputfile;

	if (self->BeCompletelyBogus) {
		t = new text;
		fprintf(stderr,"Reading roff into text...");
		fflush(stderr);
		rofftext::ReadRoffIntoText(t, self->inf, 0, r->inputfiles);
		fprintf(stderr, "done.\n");
		fflush(stderr);
		(t)->Write( self->outf, (long)t, 0);
	}
	else {
		if (self->macrofile == NULL) 
			fprintf(stderr, 
				"Warning: no macro file specified.  %s\n",
				"Valid options include -ms, -me, and -man.");
		r->macrofile = self->macrofile;
		r->RoffType = (*self->inputtype == 't');
		r->HelpMode = self->HelpMode;

		(r)->Read( self->inf, (long)r);
		(r)->Write( self->outf, (long)r, 0);
	}
}


/* = = = = = = = = = = = = = = = = = = = =
	ReadScribe
= = = = = = = = = = = = = = = = = = = = =*/


	static void
errhandler(long  lineno, char  *msg)
		{
	fprintf(stderr, "line %ld: %s\n", lineno, msg);
}


	static void
describe(class toezapp  *self)
	{
	class text *txt = new text;
	(txt)->ReadTemplate( "default", TRUE);

	ReadScribeFromFileToDoc(self->inf, txt, 0, errhandler);
	(txt)->Write( self->outf, 91, 0);
}


	int 
toezapp::Run()
	{
	/* if no type given, analyze the input file to determine type */
	AnalyzeInput(this);

	/* open the output file */
	OpenOutputFile(this);

	/* convert input to output */
	if (strcmp(this->inputtype, "nroff") == 0
			|| strcmp(this->inputtype, "troff") == 0) 
		frofftext(this);
	else
		describe(this);
	fclose(this->outf);
	fclose(this->inf);
	return (0);
}
