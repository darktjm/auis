/* Copyright 1996 Carnegie Mellon University All rights reserved. */

#ifndef _gentext_H_
#define _gentext_H_
/** \addtogroup libness
 * @{ */
/* gentext.H

	Generate text from template.	

	A gentext object encapsulates a template text which
	may incorporate ness expressions, declarations, and statements.
	These may refer to variables which can also be set and accessed 
	from C++ code.  After the appropriate variables are set,
	a call to the Instantiate method interprets the template
	and returns a resulting text.
	The variables can be reset and the call on Instantiate
	repeated;  this returns a new text value.
	The template text remains unchanged.  (Actually, gentext
	does not retain the template text;  it keeps only the 
	generated ness code.)

	The variables are UNINITIALIZED globals in the ness code.
	Initializations in the code take precedence.
	(XXX:  Variable setting should take precedence so
		default values could be established in the Ness code.)

	The template text may contain enclosures, each beginning with
	<| and ending with |>.  Other tokens used in the syntax are
	:, :=, ~:=, !, and $.  White space is permitted around tokens 
	but not within multicharacter tokens.  The hyphenated words in 
	the descriptions below must be filled in with identifiers or 
	Ness source.
	
		<| ness-expression |>
			The expression may be any Ness expression and
			may utilize variable values established with
			SetVar() or GetVarText().  The expression must exist;
			that is, it cannot all be whitespace.
			The value of the expression replaces the enclosure.
		<|$ ness-statements |>
			The Ness statements are executed until a
			return statement is executed in which case its
			value replaces the enclosure.
			If no return is executed, the enclosure is replaced 
			with the empty string.
		<|! ness-declarations |>
			Contents are Ness functions or variable declarations.
			These declarations will be in force throughout
			instantiation of the template text.
			The enclosure is replaced with the empty string.

	The template may be in named segments.  Each segment may have one
	initial unnamed section followed by any number of sections whose 
	values are assigned or appended to variables.
	Declarations anywhere within <|! ... |> are global to all segments
	and sections.  Variable declarations must appear before their
	first use.

	The variables may be used in statements and expressions.

		<| segment-name : |>
			The subsequent sections, up to the next segment-name:
			element, are part of the named segment.

		<| variable-name := |>
		 	Starts a new section.  Its text value is assigned
			to the named variable.

		<| variable-name ~:= |>
			Starts a new section.  Its text value is appended
			to the named variable.  The variable must have 
			been initialized with the form:
				marker variable-name := newbase()

	Interpretation of a segment to compute the textual value of its 
	sections, is done with the Instantiate method:

	gentext_Instantiate(segment-name)
		The named segment is instantiated.
			If segment-name is "*", all segments are instantiated.
			If segment-name is NULL or "", the initial unnamed
				segment is instantiated.
		Returns a text object referring to the text of the
		segment (up to the first instance of
			<|variable-name ~:= |> 
			or <|variable-name := |>  
		either of which starts a new section)
		The returned text object remains valid only until the
		next call to Instantiate.

	There are two strategies for using Instantiate.
		(a) each segment is a single section that is not assigned to a variable
		(b) segments have sections assigned to variables
	In case (a), the value returned from Instantiate is used
	as the expanded value.  In case (b), CopyVar or WriteVar
	is used to process one or more of the section values.
	

	Variable names, function names, and segment names all share the same 
	name space; no name can be used with more than one meaning.

	Declarations within <|! ... |> should not use identifiers
	of the form GenTextQnnn, where nnn is an integer.

	No line in the input template may begin with two backslashes.
	(It can begin with \<|""|>\ which has the desired effect.)

	No text may contain either <| or |> except in its role above.
	(To get either, use  <<|""|>|  or |<|""|>>, respectively.)
*/


#include <ness.H>
class gentext : public  ness {
	text *retval;	// value returned by Instantiate 
public:
	TRACED_CLASS(gentext);
	static boolean InitializeClass();
	
	gentext();

	virtual void SetVar(const char *var, const char *value);
		/* sets the named global variable to the C string */

	virtual void SetVarFromText(const char *var, 
			text *t, long pos, long len);
		/* sets the named global variable to the portion of the text
			starting at pos and extending for len elements */

	virtual void CopyVar(const char *var, 
			text *t, long pos);
		/* copies the value of the named global var to the
			given text at the indicated position */

	virtual boolean WriteVarToNamedFile(const char *var, const char *filename);
		/* writes the indicated global variable to the named file.
			returns TRUE for success and FALSE for failure 
			(in the latter case, errno will be set) */

	virtual boolean WriteVarToFile(const char *var, FILE *f);
		/* writes the indicated global variable to the given FILE.
			returns TRUE for success and FALSE for failure 
			(in the latter case, errno will be set) */

	/* GetVarAddr(self, var) is available to get the actual nessmark
		of the named variable.  See ness.ch.  */

	virtual text * Instantiate(const char *segment)  ;
		/* interprets the named segment from the template text, 
			   evaluating all enclosures
		   The name must be in all lowercase.
		   If the segment name is NULL or "", the unnamed 
			initial segment is expanded.
		   If the segment name is "*", all segments are expanded.
		   Returns a text object containing a copy of the value
		 	of the unnamed initial section of the segment.
			The text object will retain its value only up to the
			next call on Instantiate.
			If all segments are evaluated with "*", the return
			value is a copy of the text from the unnamed initial
			section in the unnamed initial segment in the template.
		   If there is an execution error, the returned text will 
			begin with "###" and the remainder will be 
			the error message.
		*/

	/* in the following functions, the created Ness code is compiled
		If there are compilation errors, gentext_GetErrors(self)
			will be non-NULL and gentext_PrintAllErrors
			can be called to print the errors to stderr 
	*/
	static gentext * CreateFromNamedFile(const char *fnm)  ;
		/* reads a named file and creates a gentext object for it 
			processes the file name to determine ATK type */
	static gentext * CreateFromFILE(FILE *f, long objectID)  ;
		/* reads an open FILE and creates a gentext object for it;
				uses objectID as arg to text_Read */
	static gentext * Create(const char *source);
		/* creates a gentext using the argument as template */
};
/** @} */
#endif /* _gentext_H_ */
