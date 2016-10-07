
/* gentext.c
	tool for expanding a text with embedded Ness expressions


	Copyright 1996 Carnegie Mellon University All rights reserved.
 */

#include <andrewos.h>
ATK_IMPL("gentext.H")
#include <im.H>
#include <filetype.H>
#include <simpletext.H>
#include <text.H>
#include <textview.H>
#include <attribs.h>
#include <nessmark.H>
#include <ness.H>

#include <gentext.H>


/*
In the generated Ness, there is a function for each segment.  In addition,
the following identifiers may be of interest.
  GenTextQ000 is the variable that has the result of instantiating
	the initial unnamed section in the Instantiated segment
  GenTextQ001 is a function which expands the initial unnamed segment 
  GenTextQ002 is a function which makes all segments
	(GenTextQ000 will be for the initial unnamed segment)
*/
static long nextN = 3;   /* next n to use in GenTextQnnn */

static char CharClass[256];

	
ATKdefineRegistry(gentext, ness, gentext::InitializeClass);
static nessmark * GetMark(gentext  *self, const char  *var);
static void addConst(text  *dest, const char  *str);
static void addText(text  *dest, text  *t, long  pos , long  nextpos);
void addMark(text  *dest, nessmark  *m);
static void GenInitial();
static void GenPre();
static void GenFinal();
static void GenSegment();
static void GenExpr();
static void GenDecls();
static void GenAssign();
static void GenAppend();
static void GenStmts();
static gentext * GenerateNess(text  *t);


/* GetMark(self, var)
	Returns the nessmark corresponding to var in self
	If there is none, returns NULL.
*/
	static nessmark *
GetMark(gentext  *self, const char  *var)  {
	union stackelement *v = (self)->GetVarAddr( var);
	if (v == NULL) return NULL;
	if (v->s.hdr == idleHdr) {
		v->s.hdr = seqHdr;
		v->s.v = new nessmark;
	}
	if (v->s.hdr != seqHdr) return NULL;
	return v->s.v;
}

/* gentext__SetVar(self, var, value)
	sets the named global variable to the C string
	errors abort the operation
*/
	void
gentext::SetVar(const char  *var, const char  *value)  {
	nessmark *n = GetMark(this, var);
	if (n == NULL) return;		/* allocation failed */
	(n)->MakeConst(value);
	((n)->GetText())->SetReadOnly(FALSE);
}

/* gentext__SetVarFromText(self, var, t, pos, len)
	sets the named global variable to the portion of the text
		starting at pos and extending for len elements
	errors abort the operation
*/
	void
gentext::SetVarFromText(const char  *var, text  *t, long  pos, long  len)  {
	nessmark *n = GetMark(this, var);
	if (n == NULL)  return;
	text *newt = new text;
	if (newt == NULL)  return;
	(newt)->AlwaysCopyText( 0, t, pos, len);
	(n)->Set( newt, 0, len);
}

/* gentext__CopyVar(self, var, t, pos)
	copies the value of the named global var to the
		given text at the indicated position
	errors abort the operation
*/
	void
gentext::CopyVar(const char  *var, text  *t, long  pos)  {
	nessmark *n = GetMark(this, var);
	if (n == NULL) return;
	(t)->AlwaysCopyText( pos, (n)->GetText(), 
		(n)->GetPos(), (n)->GetLength());
}

/* gentext__WriteVarToNamedFile(self, var, filename)
	writes the indicated global variable to the named file.
	Returns: returns TRUE for success and FALSE for failure 
		(in the latter case, errno will often be set) 
*/
	boolean
gentext::WriteVarToNamedFile(const char  *var, const char  *filename)  {
	nessmark *n = GetMark(this, var);
	if (n == NULL) return FALSE;
	FILE *f = fopen(filename, "w");
	if (f == NULL) return FALSE;
	text *t = new text;
	if (t == NULL) return FALSE;
	(t)->AlwaysCopyText( 0, (n)->GetText(), 
		(n)->GetPos(), (n)->GetLength());
	(t)->Write( f, 91, 0);
	(t)->Destroy();
	fclose(f);
	return TRUE;
}

/* gentext__WriteVarToFile(const char *var, FILE *f)
	writes the indicated global variable to the given FILE.
	Returns: returns TRUE for success and FALSE for failure 
		(in the latter case, errno will often be set) 
*/
	boolean
gentext::WriteVarToFile(const char *var, FILE *f)  {
	if (f == NULL) return FALSE;
	nessmark *n = GetMark(this, var);
	if (n == NULL) return FALSE;
	text *t = new text;
	if (t == NULL) return FALSE;
	(t)->AlwaysCopyText( 0, (n)->GetText(), 
		(n)->GetPos(), (n)->GetLength());
	(t)->Write( f, im::GetWriteID(), 0);
	(t)->Destroy();
	return TRUE;
}



/* gentext__Instantiate(self, segment)
	Interprets the named segment from the source text, 
		evaluating all enclosures.
	If the segment name is NULL or "", the unnamed 
		initial segment is expanded.
	If the segment name is "*", all segments are expanded.
	Returns a text object containing a copy of the value
	 	of the unnamed initial section of the segment.
		The text object will retain its value only up to the
		next call on Instantiate.
		If all segments are evaluated with "*", the return
		value is a copy of the text from the unnamed initial
		section in the unnamed initial segment in the source.
*/
	text *
gentext::Instantiate(const char  *segment)  {
	char tnm[200];
	char *nx;
	nessmark *n;
	
	if (this->retval == NULL)
		return NULL;
	(this->retval)->Clear();
	if (segment == NULL  ||  *segment == '\0')
		strcpy(tnm, "GenTextQ001");  /* make initial unnamed segment */
	else if (*segment == '*')
		strcpy(tnm, "GenTextQ002");  /* make all segments */
	else strncpy(tnm, segment, sizeof(tnm) -2);  /* make named segment */
	tnm[sizeof(tnm)-1] = '\0';  /* in case 'segment' exceeds 200 bytes */
	for (nx = tnm; *nx; nx++)
		if (isupper(*nx))  
			*nx = tolower(*nx);	
	
	if ((this)->Execute( tnm) != NULL) {
		/* error ! */
		char buf[200];
		sprintf(buf, "Error executing segment \"%.150s\"", segment);
		ness::PrintAllErrors(buf);
		return NULL;
	}
	n = GetMark(this, "gentextq000");
	if (n == NULL) {
		fprintf(stderr, "Ness code is wrong. No GenTextQ000 for \"%s\"\n", 
				segment);
		return NULL;
	}
	(this->retval)->AlwaysCopyText( 0,  (n)->GetText(), 
		(n)->GetPos(), (n)->GetLength());
	return this->retval;
}

/* gentext__InitializeClass(ClassID)
	Returns: True if all class creations succeed 
*/
	boolean
gentext::InitializeClass()  {
	int i;
	for (i = 255; i >= 0; i--)
		CharClass[i] = 4;
	CharClass[' '] = 0;
	CharClass['\n'] = 0;
	CharClass['\t'] = 0;
	CharClass['\r'] = 0;
	CharClass['\f'] = 0;
	CharClass['|'] = 1;
	CharClass['~'] = 2; 
	CharClass[':'] = 2;
	CharClass['='] = 2;
	CharClass['!'] = 3;
	CharClass['$'] = 3;

	return TRUE;
}


/* gentext__InitializeObject(ClassID, self)
	self - the new object
	Returns: TRUE
*/
gentext::gentext()  {
	ATKinit;

	this->retval = new text;
	THROWONFAILURE( this->retval != NULL);
}

/* gentext__FinalizeObject(ClassID, self)
	self - an object alledged to be no longer needed
*/
gentext::~gentext()  {
	ATKinit;

	if (this->retval != NULL)
		(this->retval)->Destroy();
}

/* * * *  * * * * * * * * * * * * * * * * * */
/* Generating a script from source */
/* * * *  * * * * * * * * * * * * * * * * * */

/* source and its components */
static text *Source;	/* where the text comes from */
static long pre;		/* where pre starts in Source */
static long prenext;		/* character after enf of pre in Source */
static long arg;		/* where arg starts in Source */
static long argnext;		/* character after end of arg in Source */

/* destinations during generation of script */
static text *Decls;	/* put declaractions here */
static text *Body;	/* body of function to expand segment */
static text *Segnms;	/* names of segments for GenTextQ002 */

/* functions to add pieces to destinations */

	static void
addConst(text  *dest, const char  *str)  {
	(dest)->AlwaysInsertCharacters( (dest)->GetLength(),
		str, strlen(str));
}

	static void
addText(text  *dest, text  *t, long  pos , long  nextpos)  {
	(dest)->AlwaysCopyText( (dest)->GetLength(), t, pos, nextpos-pos);
}

void addMark(text  *dest, nessmark  *m) {
	(dest)->AlwaysCopyText( (dest)->GetLength(),
		(m)->GetText(), (m)->GetPos(),
		(m)->GetLength());
}	


/* 
  During generation, declarations are put in self as they are 
  encountered;  a temporary text ('body') is used to store concatenations to
  construct the actual string.  At the end of a segment, the 'body' is 
  appended to self.

  An additional temporary text 'segnames' accumulates the segment names 
  for GenTextQ002

  the generated script has the form:

	marker GenTextQ000

	-- for each declarations enclosure
	<text of declarations> \n

	--for each statements enclosure
	function GenTextQ00n() <statements> end function \n

	-- once
	function GenTextQ002()
		marker t
		GenTextQ001()
		t := copy(GenTextQ000)
		-- for each segment
		<segment-name> ()
		GenTextQ000 := t
	end function

	--for each segment:  (for the unnamed segment, name is GenTextQ001)
	function <segment-name>() 
		GenTextQ000 := "<text up to first enclosure>
			-- for each expression enclosure:  
			-- (quote belongs at end of previous line)
			" ~ <text of expression> ~ "<text to next enclosure>
			-- for each statements enclosure:
			" ~ GenTextQ00n() ~ "<text up to next enclosure>
			-- for each declarations enclosure:
			" ~ "<text up to next enclosure>
			-- at end of section:
			" \n
		--for each 'variable :=' section:
		<variable-name> := "<text up to first enclosure>
			-- continue exactly as in GenTextQ000
			" \n
		--for each 'variable ~:=' section:
			-- same as 'variable :=' but use ~:=
	end function
*/


	static void
GenInitial()   {
	addConst(Decls, "marker GenTextQ000\n");
	addConst(Body, "function GenTextQ001()\n");
	addConst(Body, "	GenTextQ000 := \n//\n");
}

/* generate code for text previous to enclosure */
	static void
GenPre()  {
	addText(Body, Source, pre, prenext);
	addConst(Body, "\n\\\\\n");
}

	static void
GenFinal()  {
	/* close last segment */
	addConst(Body, "end function\n");

	/* build GenTextQ002 in Decls */
	addConst(Decls, "function GenTextQ002()\n");
	addConst(Decls, "	marker t\n");
	addConst(Decls, "	GenTextQ001()\n");
	addConst(Decls, "	t := copy(GenTextQ000)\n");
	addText(Decls, Segnms, 0, (Segnms)->GetLength());
	addConst(Decls, "	GenTextQ000 := t\n");
	addConst(Decls, "end function\n");

	/* append Body to Decls  */
	addText(Decls, Body, 0, (Body)->GetLength());
}

	static void
GenSegment()  {
	/* add function call to Segnms */
	addConst(Segnms, "\t");
	addText(Segnms, Source, arg, argnext);
	addConst(Segnms, "()\n");

	/* close previous segment */
	addConst(Body, "end function\n");

	/* start new segment */
	addConst(Body, "function ");
	addText(Body, Source, arg, argnext);
	addConst(Body, "()\n");
	addConst(Body, "	GenTextQ000 := \n//\n");
}

	static void
GenExpr()  {
	addConst(Body, " ~ ");
	addText(Body, Source, arg, argnext);
	addConst(Body, " ~ \n//\n");
}

	static void
GenDecls()  {
	addText(Decls, Source, arg, argnext);
	addConst(Decls, "\n");
	addConst(Body, " ~ \n//\n");
}

	static void
GenAssign()  {
	addText(Body, Source, arg, argnext);
	addConst(Body, " := \n//\n");
}
	static void
GenAppend()  {
	addText(Body, Source, arg, argnext);
	addConst(Body, " ~:= \n//\n");
}

	static void
GenStmts()  {
	char buf[100];
	sprintf(buf, "GenTextQ%03ld", nextN++);

	/* build function to evaluate statements */
	addConst(Decls, "function ");
	addConst(Decls, buf);
	addConst(Decls, "()\n");
	addText(Decls, Source, arg, argnext);
	addConst(Decls, "	return \"\"\n");
	addConst(Decls, "\nend function\n");

	/* generate a call on the function */
	addConst(Body, " ~ ");
	addConst(Body, buf);
	addConst(Body, "() ~ \n//\n");
}

/*
	state machine to isolate and identify type of inclusion

inclusion forms:
	<| expression |>
	<|$ statements |>
	<|! declarations |>
	<| segment-name : |>
	<| variable-name := |>
	<| variable-name ~:= |>

states:
	initial  - have seen only initial whitespace
	inarg - have gotten a non-whitespace character
	white - whitespace seen after start of arg
	op - saw : ~ or =  (stored in opchars)
	opwhite - in whitespace after : ~ or =
	bar - saw |

character classes: 
	0 white:  space  \n  \t  \r  \f
	1 bar:    |
	2 op:     ~ : =
	3 group:  ! $
	4 other:  all else

actions:
	0 {no action}
	1 {set arg and argnext}
	2 {set arg}
	3 {set type}
	4 {set argnext}
	5 {set argnext, clear opchars, store in opchars}
	6 {clear opchars, store char in opchars}
	7 {store in opchars}
	8 {if char is >, halt}
*/
enum inclState {initial, inarg, white, op, opwhite, bar, done};
static struct transit {
	int action; 
	enum inclState next;
} inclTran [6 /*states*/][5 /* character classes */] = {
/* initial */ {
	{0, initial},	//	initial (white) -> initial
	{1, bar}, 		//	initial (bar) 	-> {set arg and argnext} bar
	{2, inarg}, 	//	initial (op) 	-> {set arg} inarg
	{3, initial},	//	initial (group) -> {set type}  initial
	{2, inarg}	 	//	initial (other) -> {set arg} inarg
},
/* inarg */ {
	{4, white},   	// 	inarg 	(white) -> {set argnext} white
	{4, bar},   		// 	inarg	(bar) 	-> {set argnext} bar
	{5, op},   		// 	inarg 	(op) 	-> {set argnext, clear opchars, store in opchars} op
	{0, inarg},   	// 	inarg	(group) -> inarg
	{0, inarg}   	// 	inarg	(other) -> inarg
},
/* white */ {
	{0, white},   	// 	white	(white) -> white
	{0, bar},   		// 	white	(bar) 	-> bar
	{6, op},   		// 	white	(op) 	-> {clear opchars, store char in opchars} op
	{0, inarg},   	// 	white	(group) -> inarg
	{0, inarg}   	// 	white	(other) -> inarg
},
/* op */ {
	{0, opwhite},   	// 	op 	(white) -> opwhite
	{0, bar},   		// 	op 	(bar) 	-> bar
	{7, op},   		// 	op 	(op) 	-> {store in opchars} op
	{0, inarg},   	// 	op 	(group) -> inarg
	{0, inarg}   	// 	op 	(other) -> inarg
},
/* opwhite */ {
	{0, opwhite},   	// 	opwhite (white) -> opwhite
	{0, bar},   		// 	opwhite (bar) 	-> bar
	{6, op},   		// 	opwhite (op) 	-> {clear opchars, store char in opchars} op
	{0, inarg},   	// 	opwhite (group)	-> inarg
	{0, inarg}   	// 	opwhite (other)	-> inarg
},
/* bar */ {
	{4, white},   	// 	bar 	(white) -> {set argnext} white
	{0, bar},   		// 	bar 	(bar) 	-> bar
	{5, op},   		// 	bar 	(op) 	-> {clear opchars, store char in opchars} op
	{0, inarg},   	// 	bar 	(group) -> inarg
	{8, inarg}   	// 	bar 	(other) -> {if char is >, halt} inarg
}
};


/* GenerateNess(t)
	generates a ness script for instantiating text t
	returns the new gentext incorporating the Ness script
	if there are problems, 
		prints the Ness error list to stderr
		and returns NULL
	The argument t is Destroy'ed (as a service to caller)
*/
	static gentext *
GenerateNess(text  *t)  {
	enum inclType {exprIncl, declsIncl, stmtsIncl, 
		segIncl, assignIncl, appendIncl};
	gentext *self = new gentext;
	if (self == NULL) return NULL;
	long srclen = (t)->GetLength();
	long curr;
	int currchar;
	struct transit *tran;
	enum inclState state;
	char opchars[10];
	char *opx;
	int inclusiontype;

	Source = t;
	pre = 0; prenext = 0;
	arg = 0; argnext = 0;

	Decls = self;
	Body = new text;
	Segnms = new text;
	if (Body == NULL || Segnms == NULL) 
		return NULL;
	

	(self)->SetAccessLevel( ness_codeUV);
	(self)->RemoveWarningText();
	(self)->EstablishViews( new textview);

	GenInitial();

	curr = 0; 
	while (curr < srclen) {
	  if ((t)->GetChar( curr) == '<'
			&& (t)->GetChar( curr+1) == '|') {
		/* process an inclusion */

		prenext = curr;
		GenPre();	/* finish preceding text */

		curr += 2;
		state = initial;
		opx = opchars;
		inclusiontype = exprIncl;
		
		while (state != done && curr < srclen) {
			currchar = (t)->GetChar( curr);
			tran = &inclTran[state][(unsigned char)CharClass[currchar]];
			state = tran->next;
			switch (tran->action) {
			case 0:  /* {no action} */
				break;
			case 1:  /* {set arg and argnext} */
				arg = curr;
				argnext = curr;
				break;
			case 2:  /* {set arg} */
				arg = curr;
				break;
			case 3:  /* {set type} */
				if (currchar == '!')
					inclusiontype = declsIncl;
				else
					inclusiontype = stmtsIncl;
				break;
			case 4:  /* {set argnext} */
				argnext = curr;
				break;
			case 5:  /* {set argnext, clear opchars, 
						store in opchars} */
				argnext = curr;
				opx = opchars;
				*opx++ = currchar;
				break;
			case 6:  /* {clear opchars, store char in opchars} */
				opx = opchars;
				*opx++ = currchar;
				break;
			case 7:  /* {store in opchars} */
				if (opx - opchars < 5)
					*opx++ = currchar;
				break;
			case 8:  /* {if char is >, halt} */
				if (currchar == '>') 
					state = done;
				break;
			}
			curr ++;
		}
		*opx = '\0';

		if (inclusiontype == declsIncl) 
			GenDecls();
		else if (inclusiontype == stmtsIncl) 
			GenStmts();
		else if (*opchars == '\0')
			GenExpr();
		else if (strcmp(opchars, ":") == 0)
			GenSegment();
		else if (strcmp(opchars, ":=") == 0)
			GenAssign();
		else if (strcmp(opchars, "~:=") == 0)
			GenAppend();
		else
			GenExpr();

		pre = curr;
	  }
	  else curr++;
	}
	prenext = curr;
	GenPre();	/* finish preceding text */
	GenFinal();

	(Body)->Destroy();
	(Segnms)->Destroy();
	(t)->Destroy();	/* destroy the argument */

	/* compile the Ness */
	/* caller should test for errors */
	(self)->Compile();
	return self;
}

/* gentext__CreateFromNamedFile(ClassID, fnm)
	reads a named file and creates a gentext object for it
	Returns: the created gentext
		or NULL if errors detected
		errors are printed to stderr
*/
	gentext *
gentext::CreateFromNamedFile(const char  *fnm)  {
	ATKinit;

	text *t;		/* where to put the result */
	FILE *inputfile;
	long objectID;
	const char *objectType;
	struct attributes *attributes;

	inputfile = fopen(fnm, "r");
	if (inputfile == NULL) {
		fprintf(stderr, "could not open file: %s\n", fnm);
		return NULL;
	}
	objectType = filetype::Lookup(inputfile, fnm, &objectID, &attributes);
	if (objectType != NULL
			&& ! ATK::IsTypeByName(objectType, "simpletext")) {
		fclose(inputfile);
		fprintf(stderr, "source file %s, was not ATK text format", fnm);
		return NULL;
	}
	t = new text;
	if (t == NULL) {
		fclose(inputfile);
		fprintf(stderr, "allocation failure\n");
		return NULL;
	}
	if (attributes != NULL)
		(t)->SetAttributes( attributes);
	(t)->Read( inputfile, objectID);

	fclose(inputfile);
	return GenerateNess(t);
}

/* gentext__CreateFromFILE(ClassID, f, objectID)
	reads an open FILE and creates a gentext object for it
		uses objectID as the id in text_Read
	Returns: the created gentext
	the FILE f is not closed
*/
	gentext *
gentext::CreateFromFILE(FILE  *f, long  objectID)  {
	ATKinit;

	text *t = new text;
	if (t == NULL) {
		fprintf(stderr, "allocation error in gentext_CreateFromFILE\n");
		return NULL;
	}
	if ((t)->Read( f, objectID) != dataobject::NOREADERROR) {
		(t)->Destroy();
		fprintf(stderr, "File not in ATK format\n");
		return NULL;
	}
	return GenerateNess(t);
}

/* gentext__Create(ClassID, source)
	creates a gentext and generates ness in it for source text
	Returns: the created gentext
*/
	gentext *
gentext::Create(const char  *source)  {
	ATKinit;

	text *t = new text;
	if (t == NULL) {
		fprintf(stderr, "allocation error in gentext_Create\n");
		return NULL;
	}
	(t)->AlwaysInsertCharacters( 0, source, strlen(source));
	return GenerateNess(t);
}
