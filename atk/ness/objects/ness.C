/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved
\* ********************************************************************** */

/*
 *    $Log: ness.C,v $
 * Revision 1.11  1995/12/07  16:41:27  robr
 * Support for exporting ness functions to the proctable.
 *
 * Revision 1.10  1995/11/13  18:50:16  robr
 * Added hook for ness to intercept failed proctable lookups.
 * (Including partial lookups ala metax.)
 * NOT DONE YET.  It doesn't yet actually put the functions it finds
 * into the proctable, nor is there an interface for calling a ness function
 * via a proctable entry yet.
 * BUG
 *
 * Revision 1.9  1994/12/13  20:35:03  rr2b
 * Added ATK_IMPL/ATK_INTER
 *
 * Revision 1.8  1994/08/11  02:58:20  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.7  1994/03/21  17:00:38  rr2b
 * bzero->memset
 * bcopy->memmove
 * index->strchr
 * rindex->strrchr
 * some mktemp->tmpnam
 *
 * Revision 1.6  1994/03/12  22:42:52  rr2b
 * Removed second argument to Stringize, an empty string may not be stringized
 *
 * Revision 1.5  1993/10/20  17:00:08  rr2b
 * Used Stringize and concat macros.
 *
 * Revision 1.4  1993/09/30  16:42:06  wjh
 * fix so path can be omitted on args to 'call ness-load' in .atkinit
 *
 * Revision 1.3  1993/07/23  00:23:42  rr2b
 * Split off a version of CopyText which will copy surrounding
 * styles as well as embedded styles.
 *
 * Revision 1.2  1993/06/29  18:06:33  wjh
 * checkin to force update
 *
 * Revision 1.1  1993/06/21  20:43:31  wjh
 * Initial revision
 *
 * Revision 1.41  1993/02/17  06:10:54  wjh
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
 * Revision 1.40  1992/12/16  04:12:38  wjh
 * Ness version 1.7
 * Added readrawfile and writerawfile.
 * Readfile, readrawfile, writefile, writerawfile, and writeobject
 * 	all canonicalize the file name argument.  It may have leading ~
 * 	or embedded $environment variable.
 * Nessruna also canonicalizes the ness file argument.
 * If the argument to readfile is an ATK object data stream,
 * 	the result is a text with one element--the object.
 * Error messages are printed with line numbers.
 * The location of errors is relative to the beginning of the script
 * 	regradless of whether there is a warning text around it.
 *
 * Revision 1.39  1992/12/14  20:49:20  rr2b
 * disclaimerization
 *
 * Revision 1.38  1992/11/26  02:38:01  wjh
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
 * Revision 1.37  92/07/03  00:57:36  wjh
 * allowed file completion in arguments to 'call ness-load' in .xxxinit files
 * The arg to 'call ness-load' can also be the name of one of the
 * library files in one of the directories named in the nesspath preference.
 * .
 * 
 * Revision 1.36  1992/06/05  16:39:31  rr2b
 * improved error reporting, and ensured deallocation
 * of sysmarks appropriately.
 *
 * Revision 1.35  1992/03/18  19:24:39  wjh
 * Fixed core dump bug if ThisCell stuff is not initialized right.  -- wdc

 log truncated  Nov. 92  -wjh

 * Revision 1.0  88/04/27  14:28:53  wjh
 * Copied from /usr/andrew/lib/dummy
 */

#include <andrewos.h>	/* for strings.h */
ATK_IMPL("ness.H")
#include <ctype.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <util.h>

#include <dataobject.H>
#include <view.H>
#include <proctable.H>
#include <message.H>
#include <text.H>
#include <simpletext.H>
#include <attribs.h>
#include <path.H>
#include <arbiterview.H>
#include <arbiter.H>
#include <im.H>
#include <frame.H>
#include <buffer.H>
#include <environ.H>
#include <filetype.H>
#include <mark.H>
#include <stylesheet.H>

#include <ness.H>

#include <prochook.H>
#include <owatch.H>

/* exactly one place to put all those temporary pointers for node creation */
NO_DLL_EXPORT struct node *QTnode;

static class ness *NessList = NULL;		/* list of all nesses 
				(so we can access library) */

static boolean WantsDialogBox = FALSE;

extern class ness *InterpretationInProgress;   /* defined in interp.c */



static boolean debug;
#define DEBUG(s) {if (debug) {printf s ; fflush(stdout);}}
#define ENTER(r) DEBUG(("Enter %s\n", Stringize(r)))
#define LEAVE(r) DEBUG(("Leave %s\n", Stringize(r)))


ATKdefineRegistry(ness, text, ness::InitializeClass);

static void releaseResources(class ness  *self);
static struct errornode * execute(class ness  *self, const char  *func);
static char * ThisUser();
static char * udate(class ness  *self, const char *d, const char *n);
static void ResetOrigin(class ness  *self);
static void dostmt(class view  *parentview);
static void procload(char *filename);
void GetViewForError(class ness  *self);
static void xferfromwarning(class ness  *self, long  *pLoc , class text  *t, 
		long  *pPrevi , long  i, int codelen);
static char * LineMsg(class ness  *ness, long  loc , long  len);


/* releaseResources(self)
	release all data structures allocated for 'self' by a compilation
*/
	static void 
releaseResources(class ness  *self) {

	struct libusenode *tuse;

	neventUnpost(self, debug);	/* remove old postings */

	if (self->outerScope != nesssym_GLOBAL)
		nesssym::NDestroyScope(self->outerScope);
	if (self->constScope != nesssym_GLOBAL)
		nesssym::NDestroyScope(self->constScope);
	self->outerScope = nesssym_GLOBAL;
	self->constScope = nesssym_GLOBAL;

	(self)->ClearErrors();

	self->globals = NULL;
	self->compiled = FALSE;

	while (self->libuseList != NULL) {
		tuse = self->libuseList;
		self->libuseList = self->libuseList->next;
		libusenode_Destroy(tuse);
	}

	if (self->marks > 0) deallocSysGlobs(self->marks);
	self->marks = 0;
}

	static struct errornode *
execute(class ness  *self, const char  *func) {

	class nesssym *funcsym;
	ENTER(execute);

	if ( ! self->compiled) 
		self->ErrorList = (self)->Compile();
	if (self->ErrorList != NULL)
		return self->ErrorList;

	for (funcsym = self->globals; funcsym != NULL; 
			funcsym = funcsym->next)
		if (strcmp(funcsym->name, func) == 0) 
			break;
	if (funcsym == NULL) {
		char buf[200];
		sprintf(buf, "*Couldn't find %s\n", func);
		return (self->ErrorList = 
			errornode_Create(self, 0, 0, 0, strdup(buf), TRUE, self->ErrorList));
	}
	if (self->ErrorList == NULL)
		self->ErrorList = callInitAll(self);	/* sigh XXX check all libraries */

	if (self->ErrorList == NULL)
		self->ErrorList = interpretNess(
			((struct funcnode *)funcsym->info.node)
						->SysGlobOffset, 
			self->arg, self);
	if (self->ErrorList != NULL) {
		neventUnpost(self, FALSE);	/* remove old postings */
		MapRunError(self);
	}

	/* the arg is only used once ! */
	if (self->arg != NULL) {
		delete self->arg;
		self->arg = NULL;
	}
	LEAVE(execute);
	return self->ErrorList;
}


/* ThisUser()
	get user id and person name for the user
*/
	static char *
ThisUser() {

	const char *login = NULL, *name = NULL;
	long uid, n;
	struct passwd *pw;
	static char buf[300];

	if ((uid = getvuid()) != -1 && (pw = getvpwuid(uid)) != NULL) {
		/* got a name from passwd file */
		login = (char *)pw->pw_name;
		name = (char *)pw->pw_gecos;
	}
	if (login == NULL)
		login = "???";
	if (name == NULL)
		name = "Unknown user";

	strcpy(buf, login);
	strcat(buf, ":  ");
	n = strlen(buf);
	strncpy (buf+n, name, 299-n);
	buf[299] = '\0';
	return buf;
}

/* udate(self, d, u)
	Construct an origin value from the date d and user name n
	THIS VALUE IS FORGABLE.  Just edit the file with (say) ed.
*/
	static char *
udate(class ness  *self, const char *d, const char *n) {

	static char buf[356];
	sprintf(buf, "%02ld\\\\%s\\\\%.299s\\\\00", self->syntaxlevel, d, n); 
	return buf;
}

/* ResetOrigin(self)
	create new origin information
	use current time and user
*/
	static void
ResetOrigin(class ness  *self) {

	char *neworigin;
	long now = time(0);
	if (self->Origin != NULL) free(self->Origin);
	neworigin = udate(self, NiceTime(now), ThisUser());
	self->Origin = strdup(neworigin);
	self->IsNowOriginator = TRUE;
}

	boolean
ness::InitializeClass() {

	struct proctable_Entry *pe;
	WantsDialogBox = environ::GetProfileSwitch("NessUseDialogBoxInsteadOfWarning",
			FALSE);

	callDirLibs();	/* get names of libraries from nesspath */

	pe = proctable::DefineProc("ness-dostmt", (proctable_fptr)dostmt, 
			ATK::LoadClass("view"),
			"ness", "Read and execute a Ness statement");
	if ( ! pe) return FALSE;

		/* ness-load is designed to be called from the -call- 
		facility of init.c.  Its first argument is to be the name of
		a file to be loaded, compiled, and posted.
		This file can use -extend proctable-  */
	pe = proctable::DefineProc("ness-load", (proctable_fptr)procload, 
			NULL,  "ness", 
			"Read, compile, and post procs from a named file");

	// Define the hook for proctable to trigger auto-loading of ness functions, or just access to
	// functions defined in previously compiled ness scripts.
	pe = proctable::DefineAction("ness-proctable-hook", &ness_ProcHook, NULL, "ness", "Attempts to locate the named function and place it in the proctable.");

	return (pe != NULL);
	}

ness::ness() {

	ATKinit;
	accesslevel=ness_codeBlue;

	this->errorpending=FALSE;
	this->marks = 0;
	this->Origin = NULL;
	this->syntaxlevel = CURRENTSYNTAXLEVEL;
	ResetOrigin(this);	/* IsNowOriginator & OriginalModValue */

	this->filename=NULL;
	this->ScriptLoc = -1;
	this->AfterScriptLoc = -1;
	this->DeauthButtonLoc = -1;
	this->ScanButtonLoc = -1;
	this->CompileButtonLoc = -1;
	this->AuthorButtonLoc = -1;
	this->ErrorList = NULL;
	this->hasWarningText = FALSE;
	this->DisplayDialogBox = FALSE;
	this->PromptBeforeCompile = FALSE;
	this->ReadingTemplate = FALSE;
	this->ClassEnabled = FALSE;

	this->compiled = FALSE;
	this->outerScope = nesssym_GLOBAL;
	this->constScope = nesssym_GLOBAL;
	this->globals = NULL;
	this->libnode = NULL;
	this->libuseList = NULL;
	this->compilationid = im::GetWriteID();  /* give it a unique value */

	this->needsInit = FALSE;
	this->ToldUser = FALSE;
	this->DefaultText = NULL;
	this->Arbiter = NULL;
	this->CurrentInset = NULL;
	this->arg = NULL;
	(this)->SetCopyAsText( TRUE);

	this->name = NULL;
	this->next = NessList;
	NessList = this;
	THROWONFAILURE( TRUE);
}



ness::~ness() {

	ATKinit;

	class ness *n, *prev;

	releaseResources(this);
	
	if(this->Origin) {
		free(this->Origin);
		this->Origin = NULL;
	}

	/* remove from NessList */
	if (NessList == this)
		NessList = this->next;
	else {
		for (n = NessList; n != NULL && n != this; n = n->next) 
			prev = n; 
		if (n == this)
			prev->next = n->next;
	}

	if(filename) free(filename);
}

	void 
ness::SetDebug(boolean  d) {
	ATKinit;

	debug = d;
	printf("Ness: debug is now %d\n", debug);
}

	void 
ness::SetFilename(const char  *name) {
	char buf[1024];
	if(this->filename!=NULL) free(this->filename);
	if(name) {
		filetype::CanonicalizeFilename(buf, name, sizeof(buf)-1);
		buf[sizeof(buf)-1]='\0';
		this->filename=strdup(buf);
	} else this->filename=NULL;
}

/* ness__Write(self, file, writeID, level)
	write the text to a file as (more or less) a text
	revise or insert the initial line with author, date, and checksum
	be sure not to write the warning text or buttons
*/
	long 
ness::Write(FILE  *file, long  writeID, int  level) {


	if ((this)->GetWriteID() != writeID)  {
		(this)->SetWriteID( writeID);
		fprintf(file, "\\begindata{%s,%ld}\n", (this)->GetTypeName(),
			(this)->GetID());
		fprintf(file, "\\origin{%s}\n", (this)->GetOrigin());

		if (this->styleSheet->templateName)
			fprintf(file, "\\template{%s}\n",
				this->styleSheet->templateName);
		(this->styleSheet)->Write( file);
		if (this->hasWarningText)
			((class text *)this)->WriteSubString( this->ScriptLoc, 
				this->AfterScriptLoc - this->ScriptLoc, file, TRUE);
		else
			((class text *)this)->WriteSubString( 0, 
				(this)->GetLength(), file, TRUE);
		fprintf(file, "\\enddata{%s,%ld}\n", (this)->GetTypeName(),
			(this)->GetID());
		fflush(file);
	}
	
	return (this)->GetID();
}


void ness::SetAttributes(struct attributes  *attributes) {
	(this)->text::SetAttributes( attributes);
	while (attributes) {
		if(strcmp(attributes->key, "filename")==0) {
			(this)->SetFilename( attributes->value.string);
		}
		attributes = attributes->next;
	}
}

/* ness__Read(self, file, id)
	read the file into the ness
	check the origin
*/
	long
ness::Read(FILE  *file, long  id) {

	long retval;
	if (this->ReadingTemplate)
		return (this)->text::Read( file, id);

	if (this->Origin) {
		free(this->Origin);
		this->Origin = NULL;
	}
	this->syntaxlevel = UNSPECIFIEDSYNTAXLEVEL;

	retval = (this)->text::Read( file, id);

	if (this->Origin == NULL) {
		/* self->Origin should have been set by HandleKeyWord, 
			but wasn't */
		char *neworigin;
		neworigin = udate(this, "unknown date",
						"????: Unknown User");
		this->Origin = strdup(neworigin);
		this->IsNowOriginator = FALSE;
	}
	this->OriginalModValue = (this)->GetModified();
	this->PromptBeforeCompile = TRUE;
	this->DisplayDialogBox = WantsDialogBox;	/* tell nessview to give dialog */
	if (this->accesslevel < ness_codeUV) {
		if ( ! WantsDialogBox) 
			(this)->AddWarningText();
		(this)->SetAccessLevel( ness_codeRed);
	}
	(this)->SetReadOnly( TRUE);
	return retval;
}

/* ness__ReadTemplate(self, templateName, inserttemplatetext)
	read the template file
	set switch to tell __Read we are reading a template
*/
	long
ness::ReadTemplate(const char  *templateName, boolean  inserttemplatetext) {

	long val;
	this->ReadingTemplate = TRUE;
	val = (this)->text::ReadTemplate( templateName, inserttemplatetext);
	this->ReadingTemplate = FALSE;
	return val;
}


	long
ness::HandleKeyWord(long  pos, char  *keyword, FILE  *file) {

	if ( ! this->ReadingTemplate && strcmp(keyword, "origin") == 0)  {
		char buf[356], *bx = buf;
		long cnt = 0;
		int c;

		while ((c = getc(file)) != EOF && c != '}' && cnt < 355) {
			*bx++ = c;
			cnt++;
		}
		*bx = '\0';
		while (c != EOF && (c = getc(file)) != '\n') {}
		this->Origin = strdup(buf);
		if (1 != sscanf(this->Origin, "%ld", &this->syntaxlevel))
			this->syntaxlevel = UNSPECIFIEDSYNTAXLEVEL;
		this->IsNowOriginator = FALSE;
		return 0;
	}
	return (this)->text::HandleKeyWord( pos, keyword, file);
}

	char *
ness::GetOrigin() {

	if ( ! this->IsNowOriginator 
			&& (this)->GetModified() != this->OriginalModValue) 
		/* the current user has modified the text.  
			generate new origin information */
		ResetOrigin(this);
	return this->Origin;
}

	void
ness::GetOriginData(char  **date , char  **author) {

	char *origin = (char *)(this)->GetOrigin();
	char *start, *end;
	long n;

	/* (in early days, the origin fields were separated by single backslashes) */

	start = strchr(origin, '\\');
	if (start == NULL) {
		*date = strdup("(unknown date)");
		*author = strdup("(unknown author)");
		return;
	}

	while (*start == '\\') start++;
	end = strchr(start, '\\');
	if (end == NULL  ||  end-start > 300)  
		n = 28;  	/* (length of `date` value) */
	else n = end-start;
	*date = (char *)malloc(n+1);
	strncpy(*date, start, n);
	*((*date) + n) = '\0';

	start = end;
	while (*start == '\\') start++;
	end = strrchr(start, '\\');
	if (end == NULL) n = strlen(start);
	else {
		if (*(end-1) == '\\') end--;
		n = end - start;
	}
	if (n > 299) n = 299;
	*author = (char *)malloc(n+1);
	strncpy(*author, start, n);
	*((*author) + n) = '\0';
}


/* ness__NotifyObservers(self, status)
	we override this call because it is made whenever textviewcmds
	modifies the underlying text
	We set the compiled flag to false.
*/
	void 
ness::NotifyObservers(long  status) {

	(this)->text::NotifyObservers( status);

	/* doing the super_NotifyObservers first is important
		for menu updating via nessview_ObservedChanged */
	if (status != ness_WARNINGTEXTCHANGED)
		this->compiled = FALSE;
}


/* ness__SetReadOnly(self, readOnly)
	override this method to prevent being not read-only 
	while the warning text is in place
*/
	void 
ness::SetReadOnly(boolean  readOnly) {

	if (this->hasWarningText && ! readOnly) return;
	(this)->text::SetReadOnly( readOnly);
}


/* ness__ReadNamedFile(self, name)
	read an object from file, checking that it is a ness
	opens the file named by 'name' and checks that it is a ness data stream
	or a data stream which is a subclass of ness.  
	(Assumes name has previously been canonicalized.)
	Also reads and sets attributes.
	returns TRUE for a successful read and FALSE otherwise
*/
	long
ness::ReadNamedFile(const char *name) {

	long val;
	FILE *inputfile;
	long c;
	
	inputfile = fopen(name, "r" );
	if (inputfile == NULL) {
		printf("File %s not found\n", name);
		return dataobject_PREMATUREEOF;
	}
	/* check to see if we might be a shell script !  */
	c = getc(inputfile);
	if (c == '#') {
		/* skip first line */
		while ( ! feof(inputfile)) {
			c = getc(inputfile);
			if (c == '\n') break;
		}
	}
	else ungetc(c, inputfile);

	val = ReadTextFileStream((class text *)this, name, inputfile, FALSE);

	fclose(inputfile);
	
	return val;
}


/* ness__EstablishViews(self, child)
	ensure that DefaultText and Arbiter are set, if possible.
	First set the arbiter:
		if it is already non-NULL, do nothing
		if there is a defaulttext, do WantHandler from it
		otherwise do WantHandler from child
	Then set the defaulttext value:
		if it is already non-NULL, do nothing
		if there is an arbiter, ask it for "defaulttext"
			and then check its ancestors
		check the ancestors of the child
	The default text is changed if the ness is ever
	told about a child of the arbiter called "defaulttext".
*/
	void
ness::EstablishViews(class view  *child) {

	class textview *deftext = (this)->GetDefaultText();
	class arbiterview *arbview = (this)->GetArbiter();
	class view *textchild, *textsecond;

	initializeEnvt();	/* set up textviewClass for EstablishViews */

	/* 1. set self->Arbiter */

	if (arbview == NULL) {
		arbview = arbiterview::FindArb(
				(deftext != NULL) 
					? (class view *)deftext 
					: (class view *)child);
		(this)->SetArbiter( arbview);
		if (debug)
			printf("arbiterview at 0x%p\n", arbview);
	}


	/* 2. set self->DefaultText  */

	if (deftext == NULL && arbview != NULL) {
		class view *dt;
		dt = arbiterview::GetNamedView(arbview, "defaulttext");
		deftext = (class textview *)ProperPtr(dt, textviewClass);
		if (dt != NULL)
			DEBUG(("dt 0x%p   deftext from dt 0x%p\n", dt, deftext));
		if (deftext == NULL)
			/* try for a child of the arbview itself. */
			deftext = (class textview *)ProperPtr((ATK  *)arbview,
					 textviewClass);
		if (deftext != NULL)
			DEBUG(("deftext from arbview 0x%p\n", deftext));
		if (deftext == NULL)
		    /* still no default text. search upward from arb and then child */
		    textchild = (class view *)arbview, textsecond = (class view *)child;
	}
	else 
		/* no default text and no arbiter.  search upward from child */
		textchild = (class view *)child, textsecond = NULL;
	for ( ; deftext == NULL && textchild != NULL; 
				textchild = textsecond, textsecond = NULL) {
		class view *v;
		/* XXX BOGOSITY ALERT:  scan up tree for parent textview */
		for (v = textchild; v != NULL; v = v->parent)  {
			DEBUG(("parent is a %s\n",(v)->GetTypeName()));
			if ((v)->IsType( textviewClass)) {
				deftext = (class textview *)v;
				break;
			}
		}
	}
	(this)->SetDefaultText( deftext);
}



	struct errornode *
ness::Compile() {

	static const char RedMsg[] = 
		":You must Empower this Ness before trying to compile";
	static const char RecurMsg[] =
		":Ness cannot compile while an execution is in progress";
	
	ENTER(::Compile);
	if (InterpretationInProgress != NULL) 
		return (this->ErrorList = 
			errornode_Create(this, 0, 0, 0, RecurMsg, 
						FALSE, NULL));

	releaseResources(this);

	if (this->accesslevel < ness_codeOrange)
		/* no compilation permitted */
		return (this->ErrorList = 
			errornode_Create(this, 0, 0, 0, RedMsg, 
						FALSE, NULL));
	this->needsInit = FALSE;

	/* DO THE COMPILATION */
	if (this->hasWarningText)
		this->ErrorList = compile(this, this->ScriptLoc, 
			this->AfterScriptLoc - this->ScriptLoc);
	else
		this->ErrorList = compile(this, 0, (this)->GetLength());

	if (this->syntaxlevel > CURRENTSYNTAXLEVEL)
		this->ErrorList = errornode_Create(this, 0, 0, 0,
			":This compiler is out of date for this script", 
			TRUE, this->ErrorList);

	if (this->ErrorList != NULL) 
		return (this->ErrorList);
	this->needsInit = TRUE;
	if (this->accesslevel >= ness_codeGreen) {
		this->compiled = TRUE;
		this->PromptBeforeCompile = FALSE;
		neventPost(this, debug);		/* establish event handling */
	}

	LEAVE(::Compile);
	return(NULL);
}

	struct errornode *
ness::Execute(const char  *func) {

	return execute(this, func);
}

	void
ness::ClearErrors() {

	while (this->ErrorList != NULL) {
		struct errornode *t = this->ErrorList;
		this->ErrorList = t->next;
		errornode_Destroy(t);
	}
}

	struct errornode *
ness::NextError(struct errornode  *curr) {

	if (curr == NULL || this->ErrorList == NULL)
		return this->ErrorList;
	else 
		return curr->next;
}

/* ness__PreviousError(self, curr)
	return error node preceding curr
	if curr is NULL, return last error
	return NULL for the predecessor of first error node 
*/
	struct errornode *
ness::PreviousError(struct errornode  *curr) {

	struct errornode *pre = this->ErrorList;
	while (pre != NULL  &&  pre->next != curr)
		pre = pre->next;
	return pre;	/* NULL will indicate *front* of list */
}



	void
ness::dumpattrs(FILE  *file) {
	dumpAttrList(file, this->globals);
}

	static void
dostmt(class view  *parentview) {

	class ness *tempNess = new ness;
	char stmt [1025];
	char def[1025];
	char prompt[200];
	static const char prefix [] = "function main()";
	static const char suffix [] = ";end function;";
	struct errornode *success;

	(tempNess)->EstablishViews( parentview);
	tempNess->CurrentInset = parentview;
	(tempNess)->SetAccessLevel( ness_codeBlue);
	*def = '\0';
	strcpy(prompt, "Ness: ");
	while (TRUE) {
		if (message::AskForString(parentview, 0, 
				prompt, def, stmt, sizeof(stmt)-1) < 0)    {
			message::DisplayString(parentview, 0, "Cancelled");
			break;
		}
		(tempNess)->Clear();	/* text_Clear() removes all text */
		(tempNess)->InsertCharacters( 0, suffix, strlen(suffix));
		(tempNess)->InsertCharacters( 0, stmt, strlen(stmt));
		(tempNess)->InsertCharacters( 0, prefix, strlen(prefix));

		tempNess->ToldUser = FALSE;

		success = (tempNess)->Compile();
		if (success == NULL) {
			message::DisplayString(parentview, 0, ". . .");
			success = execute(tempNess, "main");
		}
		if (success == NULL) {
			if ( ! tempNess->ToldUser)
				message::DisplayString(parentview, 0, "Done");
			break;
		}
		sprintf(prompt, "%s at %ld.  Try again: ", success->msg+1,
				(success->where)->GetPos() - strlen(prefix));
		(tempNess)->ClearErrors();
		tempNess->compiled = FALSE;
		strcpy(def, stmt);
	}
	(tempNess)->Destroy();
}

	static void
procload(char *filename) {

	/* XXX we ought to make a buffer for the object,
	 *	but done the simple way creates a Ness object in the file. 
	 *	if (buffer_FindBufferByFile(filename) == NULL) {
	 *		char *slash = (char *)rindex(filename, '/');
	 *		buffer_Create((slash == NULL) ? filename : slash + 1, 
	 *			filename, NULL, tempNess);
	 *	}
	 */

	char fullName[MAXPATHLEN+1];
	const char *expandedname;
	FILE *inputfile;
	class ness *tempNess;
	int n;
	class nesssym *libsym;
	struct libnode *libnode;


	/* try to read the file */
	expandedname = path::UnfoldFileName(filename, fullName, 0);
	inputfile = fopen(expandedname, "r" );
	if (inputfile != NULL) {
		tempNess = new ness;
		if (ReadTextFileStream((class text *)tempNess, 
				expandedname, inputfile, FALSE) == 
				dataobject_NOREADERROR) {
			(tempNess)->SetAccessLevel( ness_codeBlue);

			if ((tempNess)->Compile() != NULL) {
				ness::PrintAllErrors(
					"ness-load Compilation");
				(tempNess)->Destroy();
			}

			/* note that the tempNess is NOT usually destroyed.
			Its compiled code may be used by proctable entries */
		}
		else (tempNess)->Destroy();
		fclose(inputfile);
		return;
	}

	/* can't read the file.  See if it is in library */

	if (*filename == '~' || (char *)strchr(filename, '/') != 0) {
		/* cannot be a library name */
cantread:
		fprintf(stderr, "ness-load: Could not read %s\n", filename);
		return;
	}

	n = strlen(filename);
	if (filename[n-1] == 'n' && filename[n-2] == '.') 
		/* remove .n to get library name */
		n -= 2;
	if (n > MAXPATHLEN) 
		n = MAXPATHLEN;
	strncpy(fullName, filename, n);
	fullName[n] = '\0';
	libsym = nesssym::NFind(fullName, LibScope);
	if (libsym == NULL) 
		goto cantread;

	libnode = nesssym_NGetINode(libsym, libnode);
	switch (libnode->status) {
	case NotRead:
	case NotCompiled:
		callCompLib(libnode);
		break;
	default:
		break;
	}
	switch (libnode->status) {
	case ReadFailed:
		goto cantread;
	case CompiledWithError:
		ness::PrintAllErrors("ness-load Compilation from library");
		break;
	default:
		break;
	}
}

void GetViewForError(class ness  *self) {

    class buffer *b=buffer::FindBufferByData(self);

    if(!b) {
	char buf[256];

	buffer::GetUniqueBufferName("Ness-Errors", buf, sizeof(buf));
	b=buffer::Create(buf, NULL, NULL, self);
    }
    if(b) {
	frame::GetFrameInWindowForBuffer(b);
    }
    (self)->NotifyObservers( ness_NEWERROR);
}

static class frame *LibFrame = NULL;
static owatch_data *LibWatch=NULL;

	void
ness::Expose() {

	class im *im;
	class buffer *Buffer;
	char fullname[MAXPATHLEN+1];

	if (im::GetLastUsed() == NULL) {
		/* there aren't any active windows.  forget it. */
		return;
	}

	if (this->libnode != NULL) {
		/* it is a library file.  display it in the LibFrame */
	    if (!owatch::Check(LibWatch)) {
			/* we need to create a LibFrame window */
			im::SetProgramName("NessLibrary");
			im = im::Create(NULL);
			if (im == NULL) {	
	      			fprintf(stderr, "ness: Could not create new window.\n");
	     			return;
			}
			LibFrame = new frame;
			if (LibFrame == NULL) {
	        			fprintf(stderr, "ness: Could not allocate enough memory to display library file.\n");
	      			return;
			}
			(im)->SetView( LibFrame);
			(LibFrame)->PostDefaultHandler( "message", 
					(LibFrame)->WantHandler( "message"));
			(LibFrame)->SetCommandEnable( TRUE);
			LibWatch=owatch::Create(LibFrame);
		}
		Buffer = buffer::FindBufferByData(this);
		if (Buffer == NULL) {
			strcpy(fullname, this->libnode->path);
			strcat(fullname, this->libnode->filename);
			Buffer = buffer::Create((this)->GetName(), fullname, NULL, this);
		}
		(LibFrame)->SetBuffer( Buffer, TRUE);
	}
	else {
	    this->errorpending=TRUE;
	    (this)->NotifyObservers( ness_NEWERROR);
	    /* if nobody handled the error report try to get somebody who will. */
	    if(this->errorpending) GetViewForError(this);
	}
}

	class ness *
ness::GetList() {

	ATKinit;

	return NessList;
}

/* ness__SetAccessLevel(self, level)
	sets the access level of the Ness
	checks the validity
	clears errors and sets to indicate that a compilation is needed
*/
	void
ness::SetAccessLevel(ness_access level) {
	if (level < ness_codeInfrared) level = ness_codeRed;
	if (level > ness_codeUV) level = ness_codeGreen;

	if(accesslevel!=level) {
	    accesslevel=level;
	    this->compiled = FALSE;
	}
}

	static void
xferfromwarning(class ness  *self, long  *pLoc , class text  *t, 
		long  *pPrevi , long  i, int codelen) {
	long len = i - codelen - *pPrevi;
	(self)->AlwaysCopyTextExactly( *pLoc, t, *pPrevi + 1, len);
	*pPrevi = i;
	*pLoc += len;
}

/* ness__AddWarningText(self)
	add the warning text and buttons to the ness text
	set read-only
*/
	/* copy text from t to self.
		The text in t has the form

			NESS script.  This inset is ...
			$name $date
			first part
			----
			----	Ness SCRIPT FOLLOWS
			----
			$script

			final part with buttons

		The $name string is replaced with the name field from the 
		origin and $date is replaced with the date.  
		$script will, in effect, be replaced with the script (in 
		fact the portion of the warningNotice before $script is 
		copied to before the script and the remainder is copied to 
		after the script.  The button locations are given with
		code $X, where X is one of A, S, D, or E.  The code is deleted
		and the position of the next character is taken as the 
		position of the button.  The capital letters are
			D for deauthenticate
			E for empower (compile)
			S for scan
			A for author mode
		(strange effects will be observed if $ is used otherwise)
 */
	void
ness::AddWarningText() {

	const char *name;
	FILE *wtext;
	class text *t;
	long i, len, loc, previ;
	char *date, *uname;

	(this)->SetReadOnly( TRUE);
	if (this->hasWarningText) return;

	name = environ::AndrewDir("/lib/ness/nesswarn.d");

	wtext = fopen(name, "r");
	if (wtext == NULL)
		return;
	t = new text;
	ReadTextFileStream(t, name, wtext, FALSE);

	len = (t)->GetLength();
	if (len == 0) {
		/* sigh, file cannot be found */
		fprintf(stderr, "Cannot find %s", name);
		return;
	}

	(this)->GetOriginData( &date, &uname);

	loc = 0;	/* where are we in the script */
	previ = -1;	/* just before start of text 
				to next xfer from warning */
	/* process each character of the warning text */
	for (i = 0; i < len; i++) {
		char c = (t)->GetChar( i);
		const char *codex;
		if (c != '$') 
			continue;

		i++; /* advance input to char after '$' */
		c = (t)->GetChar( i);
		switch(c) {
		case 'A':  xferfromwarning(this, &loc, t, &previ, i, 2);
			this->AuthorButtonLoc = loc;
			break;
		case 'S':  xferfromwarning(this, &loc, t, &previ, i, 2);
			this->ScanButtonLoc = loc;
			break;
		case 'D':  xferfromwarning(this, &loc, t, &previ, i, 2);
			this->DeauthButtonLoc = loc;
			break;
		case 'E':  xferfromwarning(this, &loc, t, &previ, i, 2);
			this->CompileButtonLoc = loc;
			break;
		case 'd':
			codex = "date";
			while (*++codex) {
				i++;
				if (*codex != (t)->GetChar( i)) break;
			}
			if (*codex == '\0') {
				/* insert date */
				xferfromwarning(this, &loc, t, &previ, i, 5);
				(this)->AlwaysInsertCharacters( loc, 
						date, strlen(date));
				loc += strlen(date);
			}
			break;
		case 'n':
			codex = "name";
			while (*++codex) {
				i++;
				if (*codex != (t)->GetChar( i)) break;
			}
			if (*codex == '\0') {
				/* insert name */
				xferfromwarning(this, &loc, t, &previ, i, 5);
				(this)->AlwaysInsertCharacters( loc,
						uname, strlen(uname));
				loc += strlen(uname);
			}
			break;
		case 's':
			codex = "script";
			while (*++codex) {
				i++;
				if (*codex != (t)->GetChar( i)) break;
			}
			if (*codex == '\0') {
				/* insert surrounding text around script */
				xferfromwarning(this, &loc, t, &previ, i, 7);
				this->ScriptLoc = loc;
				loc = (this)->GetLength();
				this->AfterScriptLoc = loc;
			}
			break;

		default: 
			/* just skip the '$' */
			i--;	/* reread the char after the '$' */
			break;
		}
	}
	xferfromwarning(this, &loc, t, &previ, i, 1);
	(t)->Destroy();
	this->hasWarningText = TRUE;
	if ( ! this->IsNowOriginator)
		this->OriginalModValue = (this)->GetModified();

	if (date != NULL) free((char *)date);
	if (uname != NULL) free((char *)uname);
}

/* ness__RemoveWarningText(self)
	remove the warning text and set the Ness to read-write 
*/
	void
ness::RemoveWarningText() {

	long len1, len2;
	if ( ! this->hasWarningText) return;
	/* do the deletions in the order indicated (superstition: maybe it will fix a bug) */
	len1 = this->ScriptLoc;
	len2 = (this)->GetLength() - this->AfterScriptLoc;
	(this)->AlwaysDeleteCharacters( 0, len1);
	(this)->AlwaysDeleteCharacters( this->AfterScriptLoc - len1, len2);
	this->hasWarningText = FALSE;
	if ( ! this->IsNowOriginator)
		this->OriginalModValue = (this)->GetModified();

}


/* LineMsg(ness, loc, len)
	Compute a (static char) msg describing the line number
	   corresponding to location loc.
	Line number is relative to start of program and excludes counting 
	   the lines in the surrounding warning message, if any.
	If loc is after the end, report loc and len.
	Cache the previous result to optimize for multiple messages in
	   same ness.
*/
	static char *
LineMsg(class ness  *ness, long  loc , long  len) {

	long lineno;
	long curloc, endloc;
	static char msg[100];
	static long prevloc, prevline, prevhas, previd = -999;  /* cache */

	lineno = 1;	/* first line is numbered 1 */

	if (ness->hasWarningText) {
		curloc = ness->ScriptLoc;
		endloc = ness->AfterScriptLoc;
	}
	else {
		curloc = 0;
		endloc = (ness)->GetLength();
	}
	if (loc >= endloc) {
		/* bizarre:  should not happen */
		sprintf(msg, "location %ld (len %ld)", loc, len);
		return msg;
	}

	if (ness->compilationid == previd && loc >= prevloc
			&& prevhas == ness->hasWarningText) {
		lineno = prevline;
		curloc = prevloc;
	}
	while (curloc < loc) {
		if ((ness)->GetChar( curloc) == '\n') 
			lineno++;
		curloc++;
	}
	previd = ness->compilationid;
	prevloc = loc;
	prevhas = ness->hasWarningText;
	prevline = lineno;

	sprintf(msg, "line %ld", lineno);
	return msg;
}

/* ness__printerrors(self, file)
	format and print error messages for 'self' to 'file'
	return number of errors 
*/
	long
ness::printerrors(FILE  *file) {

	struct errornode *list = (this)->GetErrors();
	long cnt = 0;
	while (list != NULL) {
		long loc, len, tloc, textend, c;
		cnt++;
		loc = (list->where)->GetPos();
		len = (list->where)->GetLength();
		if (list->execloc != 0)
			fprintf(file, "(Code location: %ld)\n", list->execloc);
		fprintf(file, "%s - %s\n   source text is:  ", 
				LineMsg(this, loc, len), list->msg+1);
		textend = (this)->GetLength();

		for (tloc = loc; 
			tloc >= 0 && (this)->GetUnsignedChar( tloc) != '\n';
			tloc--) {}
		tloc++;	/* first character of line */
		for ( ; tloc < textend && 
				(c = (this)->GetUnsignedChar( tloc)) != '\n';
				tloc++) {
			if (tloc == loc)  fprintf(file, " >>> ");
			fputc(c, file);
			if (tloc == loc+len - 1)  fprintf(file, " <<< ");
		}
		if (tloc <= loc+len - 1)  fprintf(file, " <<< ");
		fprintf(file, "\n");

		list = list->next;
	}
	return cnt;
}

/* ness__PrintAllErrors()
	formats error messages for all loaded Nesses and prints them to stderr
	'when' is printed in header
	return number of errors found
*/
	long
ness::PrintAllErrors(const char  *when) {

	ATKinit;

	class ness *n;
	long cnt = 0;
	struct errornode *errs;
	char *name;
	for (n = ness::GetList(); n != NULL; n = (n)->GetNext()) {
		errs = (n)->GetErrors();
		if (errs == NULL) 
			continue;
		name = (n)->GetName();
		if (name != NULL)
			fprintf(stderr, "%s error(s) in %s\n", when, name);
		else 
			fprintf(stderr, "%s error(s):\n", when);
		cnt += (n)->printerrors( stderr);
	}
	return cnt;
}

/* returns the address of the nessmark for the global
	variable named 'var'
   returns NULL if there is none such
*/
	union stackelement *
ness::GetVarAddr(const char  *var) {

	class nesssym *s;
	char tnm[100];
	char *nx;
	long ind;
	
	/* convert to lower case */
	strncpy(tnm, var, sizeof(tnm)-2);
	tnm[sizeof(tnm)-1] = '\0';  /* in case 'var' exceeds 200 bytes */
	for (nx = tnm; *nx; nx++)
		if (isupper(*nx))  
			*nx = tolower(*nx);	

	s = nesssym::NFind(var, this->outerScope);
	if (s == NULL) 
		return NULL;

	ind = nesssym_NGetINode(s, vardefnode)->addr;
	return &ness_Globals[ind].e;
}
