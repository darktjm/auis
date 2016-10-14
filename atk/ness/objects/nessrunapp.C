/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved
\* ********************************************************************** */

/*
 * app for ness
 *
 *	Program to execute a Ness file.  
 *
 *	nessrun [-d]  programfilename args
 *
 *	programfilename is the name of a file containing the ness program
 *		it must have a function main(), which will be called
 *		to initiate execution
 *	remaining args are concatenated and passed as a single marker parameter to main()
 *
 *	The -d switch causes the compiler to dump the generated code.
 */

/*
 * $Log: nessrunapp.C,v $
 * Revision 1.6  1995/03/01  19:34:03  rr2b
 * Fixed to make sure a view with an im is provided when needed.
 *
 * Revision 1.5  1994/11/30  20:42:06  rr2b
 * Start of Imakefile cleanup and pragma implementation/interface hack for g++
 *
 * Revision 1.4  1994/08/13  16:19:31  rr2b
 * new fiasco
 *
 * Revision 1.3  1994/08/12  20:43:26  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.2  1993/06/29  18:22:13  wjh
 * removed global decl of 'program'
 *
 * Revision 1.1  1993/06/21  20:43:31  wjh
 * Initial revision
 *
 * Revision 1.12  1993/02/17  06:10:54  wjh
 * uservis: added FUNCVAL to grammar.  Now functions can be values.
 * uservis: added FORWARD decls of functions
 * uservis: added subseq as a synonym for marker
 * uservis: fixed to avoid coredumps that can happen after a syntax error
 * trivial(progvis): make Sym a part of all callnodes
 * 		so dumpstack can say what value is there
 * ignore: removed 'paren' field from varnode
 * ignore: fixed bug in which builtin function names would compile as values but would get runtime error.  Now a compile error.
 * ignore: removed use of ntFUNCTION, etc. in favor of FUNCTION, etc.
 * uservis: check the type of values in RETURN stmts.  This may produce compilation errors in programs which compiled successfully in the past.
 * uservis: change error from ParseReal to be -999.0.  length(WhereItWas()) will be zero.
 * progvis: changed nessruna's -d to -x as switch to dump symbol table
 * progvis: added nessruna switch -y to set debugging
 * trivial: removed disclaimer from ness warning text
 * trivial: removed the restriction in Ness which prevents storing a NaN value
 * trivial: Fixed so the proper function is reported for an error even if warning message is added or removed.  (used a mark in funcnode)
 * trivial: Rewrote the line number calculation (ness.c:LineMsg) so it works when there is a wrapped warning text.  (The warning text is excluded from the count.)
 * adminvis: Added file testness.n ($ANDREWDIR/lib/ness/demos) for testing many facets of Ness
 * uservis: In Ness, provide simple implementations on all platforms for acosh(), asinh(), atanh(), cbrt(), expm1(), and log1p().
 * uservis: Provide Ness predefined identifier 'realIEEE' which is True if isnan(), finite(), and lgamma() are implemented
 * uservis: In Ness, accept isnan(), finite(), and lgamma() during compilation, but give a runtime error if they are executed on a platform where they are not implemented.  (See predefined identifier realIEEE.)
 * ignore: changed eventnode to correspond to change in funcnode
 *
 * e
 *
 * Revision 1.11  1992/12/17  19:39:37  rr2b
 * added #include of sys/param.h for MAXPATHLEN
 * .
 *
 * Revision 1.10  1992/12/16  03:57:28  wjh
 * the ness file argument name is canonicalized
 * .
 *
 * Revision 1.9  1992/12/15  21:38:20  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.8  1992/12/14  20:50:00  rr2b
 * disclaimerization
 *
 * Revision 1.7  1992/11/26  02:42:25  wjh
 * converted CorrectGetChar to GetUnsignedChar
 * moved ExtendShortSign to interp.h
 * remove xgetchar.h; use simpletext_GetUnsignedChar
 * nessrun timing messages go to stderr
 * replaced curNess with curComp
 * replaced genPush/genPop with struct compilation
 * created compile.c to handle compilation
 * moved scope routines to compile.c
 * converted from lex to tlex
 * convert to use lexan_ParseNumber
 * truncated logs to 1992 only
 * use bison and gentlex instead of yacc and lexdef/lex
 *
 * .
 *
 * Revision 1.6  92/11/14  15:44:15  wjh
 * fix illegal pointer messages for stupid Sun char * conventions
 *
	elided log 29 June 1993   -wjh
 * 
 * Revision 1.1  90/07/15  15:24:35  wjh
 * Initial revision
 * 
* 11 May 1990 WJH converted from old standalone nessrun program.
 */

#include <andrewos.h>
ATK_IMPL("nessrunapp.H")
#include <sys/param.h>

#include <event.H>
#include <mark.H>
#include <filetype.H>
#include <path.H>
#include <text.H>
#include <simpletext.H>
#include <textview.H>

#include <nessmark.H>
#include <ness.H>

#include <nessrunapp.H>
#include <im.H>

ATKdefineRegistryNoInit(nessrunapp, application);

static void show_usage(class nessrunapp  *self);
void dumpall();


nessrunapp::nessrunapp() {
	this->inputfile = NULL;
	this->dump = FALSE;
	this->theNess = NULL;
	(this)->SetMajorVersion( CURRENTSYNTAXLEVEL);
	(this)->SetMinorVersion( CURRENTMODIFICATIONLEVEL);
	(this)->SetFork( FALSE);
	THROWONFAILURE(  TRUE);
}

nessrunapp::~nessrunapp() {
	if (this->theNess) (this->theNess)->Destroy();
	/* do not free self->inputfile because it is in argv */
}


/*
 * usage statement
 */
	static void
show_usage(class nessrunapp  *self) {
	fprintf(stderr,
		"Usage: %s  [-d]  programfilename  arguments\n",
		(self)->GetName());
	fprintf(stderr,
"	-d: display generated code\n"
"	programfilename: execute program in this files\n"
"	args: all further text is passed as the arg to main() in the program\n"
);
}

	void
dumpall() {
	class ness *n;
	char *name;
	for (n = ness::GetList(); n != NULL; n = (n)->GetNext()) {
		name = (n)->GetName();
		printf("\nObject code for %s\n", 
			(name != NULL) ? name : "unknown");
		(n)->dumpattrs( stdout);
	}
}


	boolean 
nessrunapp::ParseArgs(int  argc, const char  **argv) {
	class nessmark *arg, *args, *blank;

	if(!(this)->application::ParseArgs( argc, argv))
		return FALSE;

	/* super_ParseArgs() passes across the "runapp" and its switches,
		leaving "nessrun" as the first arg.   */

	while(*++argv != NULL && **argv == '-') {
		switch((*argv)[1]){
			case 'x':		
				this->dump = TRUE;
				break;
			case 'y':
				ness::SetDebug(TRUE);
				break;
			case 'f':		
				(this)->SetFork( TRUE);
				break;
			default:
				fprintf(stderr,"%s: unrecognized switch: %s\n",
					(this)->GetName(), *argv);
				show_usage(this);
				return FALSE;
		}
	}

	if (*argv == NULL) {
		fprintf(stderr,"%s: no programfilename specified\n",
				(this)->GetName());
		show_usage(this);
		return FALSE;
	}

	/* get the name of the ness program file */
	this->inputfile = *argv++;

	/* concatenate args to pass to theNess */
	args = new nessmark;
	(args)->SetText( new simpletext);
	blank = new nessmark;
	(blank)->MakeConst( " ");
	arg = new nessmark;
	while (*argv != NULL) {
		(arg)->MakeConst( *argv);
		(args)->Next();
		(args)->Replace( arg);
		(args)->Next();
		(args)->Replace( blank);
		argv++;
	}
	(args)->Base();
	if (this->theNess == NULL)
		this->theNess = new ness;
	(this->theNess)->SupplyMarkerArg( args);
	return TRUE;
}

	boolean
nessrunapp::Start() {
	char fullName[MAXPATHLEN+1];
	const char *fname;

	fname = path::UnfoldFileName(this->inputfile, fullName, 0);
	if ((this->theNess)->ReadNamedFile( fname) 
				!= dataobject::NOREADERROR) {
		fprintf(stderr, "Input file is neither plain text not ATK format: %s\n",
				fname);
		return FALSE;
	}
	return TRUE;
}


	int 
nessrunapp::Run() {
	struct errornode *result;
	long t0, t1;
	class text *textp;
	class textview *textviewp;
	boolean forkit;

	textp = new text;
	textviewp = new textview;
	(textviewp)->SetDataObject( textp);
	(this->theNess)->SetDefaultText( textviewp);
	static struct rectangle playpen = {0,0,0,0};
	im *iptr=im::GetLastUsed();
	if(iptr==NULL) iptr=new im;
	(textviewp)->InsertView( iptr,
			 &playpen);
	(this->theNess)->SetName( (char *)this->inputfile);
	(this->theNess)->SetAccessLevel( ness_codeUV);

			t0 = event_TUtoMSEC(event::Now());
	result = (this->theNess)->Compile();
			t1 = event_TUtoMSEC(event::Now());

	if (this->dump)
		dumpall();

	if (ness::PrintAllErrors("Compile") != 0)
		return(1);
	else fprintf (stderr, "Compiled in %ld.%02ld sec.\n",
			(t1-t0)/1000, (t1-t0)%1000/10);

	forkit = (this)->GetFork();
	if(!(this)->Fork())
		return -1;

			t0 = event_TUtoMSEC(event::Now());
	result = (this->theNess)->Execute( "main");
			t1 = event_TUtoMSEC(event::Now());

	if (result != NULL) 
		ness::PrintAllErrors("Execution");
	else if ( ! forkit)
		fprintf (stderr, "Executed in %ld.%02ld sec.\n",
			(t1-t0)/1000, (t1-t0)%1000/10);

	return ((result == NULL) ? 0 : 1);
}
