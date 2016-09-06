/* ********************************************************************** *\
 *         Copyright IBM Corporation 1986,1991 - All Rights Reserved      *
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

/* ReadScribe.c - 
	 convert an ASCII file (with Scribe commands) to ATK text
*/

/* 
	$Log: readscrb.C,v $
// Revision 1.4  1994/08/13  16:46:24  rr2b
// new fiasco
//
// Revision 1.3  1994/08/12  20:44:31  rr2b
// The great gcc-2.6.0 new fiasco, new class foo -> new foo
//
// Revision 1.2  1993/05/03  21:08:06  rr2b
// Removed unnecessary use of unsigned char.
//
// Gave all functions explicit return types.
//
// Revision 1.1  1993/04/28  18:46:05  rr2b
// Initial revision
//
 * Revision 1.12  1992/12/15  21:45:38  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.11  1992/12/14  21:00:02  rr2b
 * disclaimerization
 *
 * Revision 1.10  1991/09/12  16:37:57  bobg
 * Update copyright notice and rcsid
 *
 * Revision 1.9  1991/01/16  15:22:32  wjh
 * fixed bug which inserted newlines in the middle of paragraphs if a previous line of the same length ended with a style that has the KeepNextNL flag
 *
 * Revision 1.8  90/06/22  11:38:22  wjh
 * modified to ignore \r and ^Z, both of which are useless DOS-ian artifacts
 * 
 * Revision 1.7  89/10/16  12:16:19  cfe
 * Clean up after compiler warnings.
 * 
 * Revision 1.6  89/10/05  11:23:15  cfe
 * typos
 * 
 * Revision 1.5  89/09/11  11:02:49  wjh
 * oops
 * 
 * Revision 1.4  89/09/11  10:52:38  wjh
 * locks
 * remove the pseudo-file tricks to avoid type mismatch error on Sun
 * replace remaining references to appendvalue with appropriate references to appendtobuf
 * reformat appendtobuf
 * 
 * Revision 1.3  89/07/28  17:02:45  tpn
 * added support for inserting new textref and texttag objects.
 * 
 * Revision 1.2  89/07/12  11:02:05  wjh
 * changed so that more than 2 embedded blanks cause newlines only if NOT after a sentence end
 * fixed three compiler warnings on vaxen by making InPtr be 'char' instead of 'unsigned char'
 * 
 * Revision 1.1  89/07/11  17:26:03  wjh
 * Initial revision
 * 
 * Revision 1.2  89/07/05  10:20:49  wjh
 * fixed compile errors and warnings
 * 
 * Revision 1.1  89/07/01  15:46:08  wjh
 * Initial revision
 * 
WJHansen,  26 May, 1989
	converted from base editor 1 
*/

/* 
@bigger[   
= * = * = * = * = * = * = * = * = * = * = * = * =
*                                                         *
=	NOTE:  ReadScribe.c DEPENDS ON    =
*	scribe.tpl and default.tpl.                      *
=	STYLE NAMES WITHIN THEM MUST HAVE THE      =
*	SAME NAMES AS EACH OTHER AND AS THE          *
=	THE STANDARD Scribe ENVIRONMENTS.               =
*            FormatNote is checked for and used to set passthru	*
=     	Exceptions: italic, bold, and underline are mapped to i,b,u    =
  		defered: treat other passthru styles sames as format note

    the following is a defered feature
=	WORSE NEWS:  THE UPPER MENU NAMES             =
*	"REGION", "HEADING", AND "JUSTIFY" ARE             *
=	CHECKED FOR HERE.  (ONLY THESE STYLES CAUSE     =
*	BREAK BEFORE AND AFTER                            *
=	IF THEY ARE ADJACENT TO NEWLINE.)                =
*             
*                                                        *
= * = * = * = * = * = * = * = * = * = * = * = * =
]

-Backslash-newline is converted to a space.
-Lines that begin with white space are preceded by a newline.
-Lines that contain tab or triple space after a non-white character
     are preceded and followed by newlines.
-An empty line is given a real newline before and after.
-@* is converted to a newline.

Sentence enders are : ; ? . !  If they occur at end of line, they 
will be followed by at least two blanks.  (Same if followed by quote 
or right parenthesis.)

defered: A start style op for Region, Justify, or Title at beginning of line
     will usually cause a newline (unless bold or italic).

defered: An end of a style for Region, Title, or Justify 
    will be followed by a newline


In general, @word is examined for three cases:
	1) word is a standard stylesheet name in default.tpl or scribe.tpl
		An appropriate style is given the enclosed text
	2) word is one of the reserved Scribe command bytes
		begin  end  i  b  u index indexentry indexsecondary newpage
		each is treated appropriately
	3) otherwise
		The entire scribe construct is output with passthru

for @special-char, only @@ and @* are processed.  others are entered
with passthru set. 

*/

/* Problems:
     @define is not processed.
     Indented text is not given an indent look.
     Probably ought to handle @style and @modify.
     should handle @. @+ @- @:
     . - &  should be accepted in style names, but they are also single char 
		@ operands
     Ought to have different environments based on @device.  E.g.
	dover.tpl, apa6670.tpl, ...
     We could have a template as argument to abe.

*/

#include <andrewos.h>
#include <ctype.h>
#include <stdio.h>
#include <text.H>
#include <stylesheet.H>
#include <style.H>
#include <fontdesc.H>

#include <stdarg.h>

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static class text *ScribeDoc = NULL;	/* has Scribe.tpl */
static class stylesheet *ScribeSheet;	/* stylesheet for ScribeDoc */
  

struct stylemap {
	class style *s;
	long pos, len;
	struct stylemap *next;
} *StyleMap;


/* output macros and variables */

static class text *Doc;		/* where to put result */
static class stylesheet *DocSheet;	/* stylesheet for Doc */

#define BUFSIZE 4096
static char buf[BUFSIZE];
static char *outp;
static long DocPos;  /* where the next character will go */
static long WhereTo;  /* where to put buf into doc */

#define pout(c) (*outp++ = c, DocPos++)
#define putstring(s,len)   {strcpy(outp, s); outp += len; DocPos += len;}

static void (*error)(long lineno, char *buf);
static char ErrBuf[100];
static boolean HadError;
static int LineNo=1;   /* current line */ 

static boolean HaveNonWhiteSpace = FALSE;		/* TRUE after 1st non-white */
static int EmbeddedWhiteSpace = 0;		/* count number of embedded tabs or triple spaces */
static boolean StartsWhite = FALSE;				/* line starts with space or tab */



/* input macros and variables */

static FILE *fIN;		/* read and write the text */
static int nextch;     /* set prior to entry to a parsing routine and 
			holds next character after exit */
#define getnextch()  (nextch = (getc(fIN)))


static boolean AfterNewline = FALSE;  /* TRUE if buf[0]
		is a blank substituted for a newline.  */

static boolean LineStartCommand = FALSE,
		LineEndCommand = FALSE;


/* stack recording nested style invocations */

static int endsp;
		/* the ending char for current envt is in scribeEnd[endsp]
		   the current envt is nofill iff NoFill[endsp] */
static char scribeEnd[100];  
		  /* ) } ] > ' "   'e' for @end;  'p' is passStyle*/
static boolean NoFill[100];
static struct stylemap *currMap[100];   /* what stylemap for this range */
static char endMode[100];	/* which form of end to use:
			'-'  just unstack and save end loc
			')'  write scribeEnd, then unstack and save loc
			'@' write '@end[word]', then unstack and save loc  */

static char *LatestEndLoc = NULL;
static struct stylemap *LatestEndMap = NULL;


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

boolean ReadScribeFromFileToDoc(FILE  *f , class text  *doc , int  pos , void  (*errhandler)(long, char *));
#ifdef __GNUC__
__attribute__((format(printf,1,2)))
#endif
static void Error(const char *, ...);
static void InitCharType ();
static char * CollectWord(int  *len );
static int IsReserved (char *word );
static char ScribeDelimiter(char *tc);
static void SaveWhiteSpace();
static boolean  OutputWhiteSpace();
static class style * createStyle(const char  *name , const char  *mnm);
static void InitStyles(class text  *doc);
#if 0
static void DefineSheet();
#endif /* 0 */
static void appendtobuf(char  *buf,long  buflen,boolean  doit);
static void DoText();
static void DoAt();
static void OpenEnvt(class style  *style , char endc , unsigned  closemode);
static void CloseEnvt(char c, char *word);
static void ProcessLine(boolean  NextIsNewline);


boolean
ReadScribeFromFileToDoc(FILE  *f , class text  *doc , int  pos , void  (*errhandler)(long, char *))  
				{
	error = errhandler;
	LineNo = 1;
	HadError = FALSE;
	outp = &(buf[0]);
	Doc = doc;
	DocPos = pos;
	WhereTo = pos;
	InitCharType();
	InitStyles(doc);

	fIN = f;
	getnextch();
	DoText();	/*      <===  parse input and make output    */

   /*  	CleanUpStyles();  */

	return HadError;
}


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	static void
Error(const char *msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	vsprintf(ErrBuf, msg, ap);
	va_end(ap);
	error(LineNo, ErrBuf);
	HadError = TRUE;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* reserved words are  (* means special processing)

 *b, *begin, bibliography, blankpage, blankspace, caption, case, cite,
 citemark, comment, counter, *define, device, *end, equate, font, 
 foot, form, hinge, hsp, *i, include, *index, *indexentry, *indexsecondary, 
 itag, label, libraryfile, make, message, modify, *newpage, ovp, pagefooting,
 pageheading, pageref, part, picture, presspicture, ref, send, set,
 specialfont, string, style, tabclear, tabdivide, tabset, tag,
 textform, title, *u, use, value
*/

static const int ResInx[26] = {
/*a*/		0,1,7,14,17,
/*f*/		20,0,24,27,0,
/*k*/		0,34,37,41,43,
/*p*/		45,0,52,54,60,
/*u*/		67,70,0,0,0,0
};

static const char * const ResWord[] = {0, 
/*  1 */ "b", "begin", "bibliography", "blankpage", "blankspace", 0,
/*  7 */ "caption", "case", "cite", "citemark", "comment", "counter", 0,
/* 14 */ "define", "device", 0,
/* 17 */ "end", "equate", 0,
/* 20 */ "font", "foot", "form", 0,
/* 24 */ "hinge", "hsp", 0,
/* 27 */ "i", "include", "index", "indexentry", "indexsecondary", "itag", 0,
/* 34 */ "label", "libraryfile", 0,
/* 37 */ "make", "message", "modify", 0,
/* 41 */ "newpage", 0,
/* 43 */ "ovp", 0,
/* 45 */ "pagefooting", "pageheading", "pageref", "part", "picture", 
				"presspicture", 0,
/* 52 */ "ref", 0,
/* 54 */ "send", "set", "specialfont", "string", "style", 0,
/* 60 */ "tabclear", "tabdivide", "tabset", "tag", "textform", "title", 0,
/* 67 */ "u", "use", 0,
/* 70 */ "value", 0
};
#define bNUMBER		 1
#define beginNUMBER		2
#define defineNUMBER 14
#define endNUMBER		17
#define iNUMBER		27
#define indexNUMBER	29
#define ientryNUMBER	30
#define isecNUMBER	31
#define	labelNUMBER	34
#define pageNUMBER	41
#define	pagerefNUMBER	47
#define	refNUMBER	52
#define uNUMBER	67

static char CharType[256];

	static void
InitCharType ()
{
	int i;
	static const char upper[] = "ABCDEFGHJIKLMNOPQRSTUVWXYZ";
	static const char lower[] = "abcdefghjiklnmoqprstuvwxyz";
	static const char other[] = "0123456789%#";
		/* '.' '-' '&' ought to be in other, but they are valid as @. @- @& */
	for (i=256; i; )				 CharType[--i] = 0;
	for (i=strlen(lower); i; )		 CharType[(unsigned char)lower[--i]] = 1;
	for (i=strlen(upper); i; )		 CharType[(unsigned char)upper[--i]] = 1;
	for (i=strlen(other); i; )		 CharType[(unsigned char)other[--i]] = 1;
}

	static char *
CollectWord(int  *len ) 
	{
	static char word[102];
	char c, *wx = word;
	int cnt = 100;
	SaveWhiteSpace();
	c = nextch;
	while (cnt-- && CharType[(unsigned char)c]) {
		*wx++ = (isupper(c)) ? tolower(c) : c;
		c = getnextch();
	}
	*wx = '\0';
	*len = wx-word;
	return(word);
}

	static int
IsReserved (char *word ) 
	{
	int inx;

	if (*word >='a' && *word <= 'z') {
		inx = ResInx[(unsigned char)(*word - 'a')] - 1;
		while (ResWord[++inx]) 
			if (strcmp(word, ResWord[inx]) == 0) 
				return inx;
	}
	return 0;
}

/* ScribeDelimiter(tc)
	scan for a scribe delimiter:  {  [  (  <  "  `  '
	return the corresponding right delimiter
	set *tc to the left delimiter
	if the first non-blank character is not a Scribe delimiter,
		it is returned in *tc and remains in nextch
	blank characters skipped over are saved
		and can be output with OutputWhiteSpace()
*/
	static char
ScribeDelimiter(char *tc)
	{
	static const char
		left[]  = "{[(<\"`'",	/* at end, null is paired to blank*/
		right[] = "}])>\"'' ";
	int endc;
	const char *pc = left;
	SaveWhiteSpace();
	*tc = nextch;
	while (*pc && nextch != *pc) pc++;
	endc = right[pc-left];
	if (endc != ' ') getnextch();
	return(endc);
}

static char SavedWhite[200];
static int WhiteLen;

	static void
SaveWhiteSpace() 
{
	WhiteLen = 0;
	while (WhiteLen<199 && (nextch==' ' || nextch=='\t')) {
		SavedWhite[WhiteLen++] = nextch;
		getnextch();
	}
}

/* OutputWhiteSpace() 
	dumps out white space saved by SaveWhiteSpace
	returns TRUE if there was any 
*/
	static boolean 
OutputWhiteSpace()
{
	if (WhiteLen) putstring (SavedWhite, WhiteLen);
	return (WhiteLen!=0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

class style *iStyle, *bStyle, *uStyle, *inxiStyle, *passStyle;

	static class style *
createStyle(const char  *name , const char  *mnm)
	{
	class style *s;
	s = new style;
	(s)->SetName( name);
	(s)->SetMenuName( mnm);
	(DocSheet)->Add( s);
	return s;
}

	static  void
InitStyles(class text  *doc)
	{
	if (ScribeDoc == NULL) {
		ScribeDoc = new text;
		(ScribeDoc)->ReadTemplate( "scribe", FALSE);
		ScribeSheet = ScribeDoc->styleSheet;
	}
	StyleMap = NULL;

	DocSheet = doc->styleSheet;
	iStyle = (DocSheet)->Find( "italic");
	if (iStyle == NULL) 
		(iStyle = createStyle("italic", "Font~1,Italic~11"))->AddNewFontFace(
			 
			fontdesc_Italic);
	bStyle = (DocSheet)->Find( "bold");
	if (bStyle == NULL) 
		(bStyle = createStyle("bold", "Font~1,Bold~10"))->AddNewFontFace(
			 
			fontdesc_Bold);
	uStyle = (DocSheet)->Find( "underline");
	if (uStyle == NULL) 
		(uStyle = createStyle("underline", "Font~1,Underline~41"))->AddUnderline(
			);
	inxiStyle = (DocSheet)->Find( "indexi");
	if (inxiStyle == NULL) {
		(inxiStyle = createStyle("indexi", "Title~3,Invisible Index~41"))->AddNewFontFace(
			 
			fontdesc_Italic);
		(inxiStyle)->SetFontScript( style_PreviousScriptMovement,
			-2, style_Points);
	}
	passStyle = (DocSheet)->Find( "formatnote");
	if (passStyle == NULL) 
		(passStyle = createStyle("formatnote", 
						"Region~4,FormatNote~60"))->AddPassThru();
}

#if 0
	static void
DefineSheet() 
{
	/* process @define to make stylesheet */
}
#endif /* 0 */


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* appendtobuf(buffer, buflen, doit)
	scan across a Scribe attribute value
	if 'doit' is true, append to output
	if buffer is non-NULL, put the output in the buffer
		instead of the regular output
	terminate either for matching " or for one of the delimiters
	skip = and "
	leave character after termination in nextch
	Assumes whitespace has been skipped.
	Skips whitespace after terminating "

	To allow styles inside index item, this routine has a miniature DoText
	and uses DoAt and checks for terminating characters equal to endsp.
*/
	static void
appendtobuf(char  *buf,long  buflen,boolean  doit)
			{
	boolean quoted, scanning;
	if (nextch == '=')  {
		getnextch();
		SaveWhiteSpace();
	}
	quoted = (nextch == '\"');
	if (quoted) getnextch();   /* skip " */
	scanning = TRUE;
	while (scanning){
		switch(nextch) {
		case '@':
			getnextch();
			if (doit) DoAt();
			break;
		case '\"':
			if (quoted) {
				SaveWhiteSpace();
				getnextch();
			}
			scanning = FALSE;
			break;
		case ',':
			if ( ! quoted)  scanning = FALSE;
			else {
				if(buf){
					*buf = nextch;
					if(--buflen > 1) buf++; 
				}
				if (doit) pout(nextch);
				getnextch();
			}
			break;
		case '}':
		case ')':
		case ']':
		case '>':
		case '\'':
			if (endsp && nextch == scribeEnd[endsp]) {
				/* end an environment */
				if (doit) CloseEnvt(nextch, NULL);
				getnextch();
				break;
			}
			else if ( ! quoted) {
				scanning = FALSE;
				break;
			}
			// else /* FALL THROUGH */;
		default:
			if (doit) pout(nextch);
			if(buf){
				*buf = nextch;
				if(--buflen > 1) buf++;
			}
			getnextch();
			break;
		}
	}
	if(buf) *buf = '\0';
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	static void
DoText() 
{
		/* reads inf and  writes it appropriately to Doc
		   Converts newlines and @word`s according to rules 
		*/
#define ouch(c)  (*op++ = c, DocPos++)
	int c;
	char *op, *ep;
	op = outp;
	ep = &(buf[BUFSIZE-1024]);
	endsp = 0;
	NoFill[endsp] = FALSE;
	c = nextch;
	while (c != EOF) {
		if (op >= ep) {
			outp = op;
			ProcessLine(FALSE);
			op = outp;
		}
		switch (c) {
		case '\t': 
			if (HaveNonWhiteSpace)
				EmbeddedWhiteSpace++;
			else StartsWhite = TRUE;
			ouch(c);
			c = getnextch();
			break;
		case '\015':			/* ignore carriage returns */
		case '\032':		/* ignore  ^Z  from DOS */
			c = getnextch();
			break;
		case ' ': 
			{int nsp = 0;
				do {
					nsp++;
					ouch(c);
					c = getnextch();
				} while (c==' ');
				if (!HaveNonWhiteSpace)
					StartsWhite = TRUE;
				else if (nsp>2) {
					/*  >2  spaces.  count it as embedded white
					   space unless it follows a "sentence" end */
					char *p = op-nsp-1;
					if (*p == '"' || *p == ')') p--;
					switch (*p) {
						case '.': case ';': 
						case ':': case '?': 
						case '!':
							break;
						default:
							EmbeddedWhiteSpace++;
							break;
					}
				}
			}
			break;
		case '\n':
			outp = op;
			ProcessLine(TRUE);
			op = outp;
			c = getnextch();
			break;
		case '@': 
			outp = op; 
			if (getnextch() == '*') {
					/* out front of line */
					ProcessLine(FALSE);
					pout('\n');
					getnextch();
					SaveWhiteSpace();
					if (nextch == '\n') 
						getnextch();
					else StartsWhite = OutputWhiteSpace();
			}
			else DoAt();
			op = outp;
			c = nextch;
			break; 
		case '}':
		case ')':
		case ']':
		case '>':
		case '\'':
		case '"':
			if (endsp && c == scribeEnd[endsp]) {
				/* end an environment */
				getnextch();
				outp = op;
				CloseEnvt(c, NULL);
				op = outp;
				c = nextch;
				break;
			}
			/* else // FALL THROUGH */;
		default:
			ouch(c);
			c = getnextch();
			HaveNonWhiteSpace = TRUE;
			break;
		} /* end switch(c) */

	} /* end while */
	outp = op;
	if (endsp > 0)
		CloseEnvt('z', NULL);	 /* close excess stuff */
	ProcessLine(FALSE);
	while (StyleMap != NULL) {
		struct stylemap *t = StyleMap;
		StyleMap = StyleMap->next;
		(Doc)->AddStyle( t->pos, t->len, t->s);
		free(t);
	}
#undef ouch
}

	static void
DoAt()
{
	class style *stylep;
	char *word, endc;
	char tc;
	int len,code;
	const char *subobjtype;
	class text *subobj;
	static char C1[2] = " ", C2[2] = " ", C3[2] = " ";

	if (nextch == ' ' || nextch == '\t') {
		OpenEnvt(passStyle, 'p', '-');
		pout('@');
		pout(nextch);
		CloseEnvt('p', NULL);
		getnextch();
		return;
	}

	word = CollectWord(&len);

	if (len == 0) {
		/* @* is done above */
		if (nextch == '@') 
			pout('@');
		else {
			OpenEnvt(passStyle, 'p', '-');
			pout('@');
			pout(nextch);
			CloseEnvt('p', NULL);
		}
		getnextch();
		return;
	}
	subobjtype = "textref";
	switch ((code = IsReserved(word))) {
		case 0:   /* not reserved */
			stylep = (DocSheet)->Find( word);
			if (stylep == NULL) {
				stylep = (ScribeSheet)->Find( word);
				if (stylep != NULL) {
					class style *newstyle = new style;
					(stylep)->Copy( newstyle);
					(DocSheet)->Add( newstyle);
					stylep = newstyle;
				}
			}
			endc = ScribeDelimiter(&tc);
			if (stylep == NULL) {
				OpenEnvt(passStyle, endc, ')');
				pout('@');
				putstring(word, len);
				pout(tc);
			}
			else {
				if (outp == &buf[(AfterNewline?1:0)]
						&& (stylep)->TestAddFlag(
							 style_KeepPriorNL)) 
					LineStartCommand = TRUE;
				OpenEnvt(stylep, endc, '-');
			}
			if (endc == ' ') {
				/* @word without delimited text */
				CloseEnvt(endc, NULL);
				OutputWhiteSpace();
			}
			break;
		case beginNUMBER:
			if (outp == &buf[(AfterNewline?1:0)])
				LineStartCommand = TRUE;
			endc=ScribeDelimiter(&tc);
			if (endc == ' ') {
				Error ("No environment name after @begin; ignoring it");
				OutputWhiteSpace();
				break;
			}
			word = CollectWord(&len);
			SaveWhiteSpace();
			if (nextch == endc) {
				getnextch();
				endc = ScribeDelimiter(&tc);
				stylep = (DocSheet)->Find( word);
				if (stylep == NULL) {
					stylep = (ScribeSheet)->Find( word);
					if (stylep != NULL) {
						class style *newstyle = new style;
						(stylep)->Copy( newstyle);
						(DocSheet)->Add( newstyle);
						stylep = newstyle;
					}
				}
				if (stylep == NULL) {
					OpenEnvt(passStyle, 'e', '@');
					putstring("@begin", 6);
					pout(tc);
					putstring(word, len);
					pout(endc);
				}
				else
					OpenEnvt(stylep, 'e', '-');
			}
			else {
				if (nextch != ',') {
					C1[0] = tc;  C2[0] = nextch;  C3[0] = endc;
					Error("after @begin%s%s, '%s' should be '%s' or ','", 
						C1, word, C2, C3);
				}
				OpenEnvt(passStyle, 'e', '@'); 
				putstring("@begin", 6);
				pout(tc);
				putstring(word, len);
				pout(',');
				while (getnextch() != endc) {
					if (nextch == '@') {
						C1[0] = tc;  C2[0] = endc;
						Error("found '@' inside \"@begin%s%s, ... %s\"", 
								C1, word, C2);
						break;
					}
					pout(nextch);
					if (nextch == '\n') 
						LineNo++;
				}
				pout(endc);
				if (nextch!='@') 
					getnextch();
			}
			SaveWhiteSpace();
/*
			if (nextch == '\n')
				LineEndCommand = TRUE;
*/
			OutputWhiteSpace();
			break;
		case endNUMBER:
			endc = ScribeDelimiter(&tc);
			if (endc == ' ') {
				Error("No environment name after @end");
				CloseEnvt('e', word);
				OutputWhiteSpace();
				break;
			}
/*
			if (outp == &buf[(AfterNewline?1:0)]) 
				LineStartCommand = TRUE;
*/
			word = CollectWord(&len);
			SaveWhiteSpace();
			if (strcmp((currMap[endsp]->s)->GetName(), word) != 0) {
				C1[0] = tc;  C2[0] = nextch;
				Error("@end%s%s%s does not match @begin",
						C1, word, C2);
			}
			if (nextch == endc) 
				getnextch();
			else {
				C1[0] = nextch;  C2[0] = endc;
				Error("after @end, '%s' should be '%s'", 
						C1, C2);
			}
			CloseEnvt('e', word);
			SaveWhiteSpace();
			if (nextch == '\n')
				LineEndCommand = TRUE;
			OutputWhiteSpace();
			break;
		case iNUMBER:  
			OpenEnvt(iStyle, ScribeDelimiter(&tc), '-');  break;
		case bNUMBER:   
			OpenEnvt(bStyle, ScribeDelimiter(&tc), '-');  break;
		case uNUMBER:   
			OpenEnvt(uStyle, ScribeDelimiter(&tc), '-');  break;


		/* for @newpage we generate a page break object */
		case pageNUMBER:
			/* flush buf, simulating newline */
			if ( ! AfterNewline && outp == &buf[0])   {
				/* we are probably just after a newline.  
				   add nothing before the bp object */
			}
			else {
				if (outp > buf + (AfterNewline ? 1 : 0)) {
					/* the line is not empty.  flush it */
					ProcessLine(TRUE);
				}
				ProcessLine(FALSE);	/* flush the newline */
			}
			(Doc)->InsertObject( WhereTo, "bp", "bpv");
			DocPos++;
			WhereTo++;
			ProcessLine(TRUE);	/* output a newline after the bp */

			/* since we invented a newline, skip one if present. */
			if (nextch == '\n') getnextch();
			break;

		/*  transformations of index entries:
			@index(xyz)  => index object for xyz
			@indexentry[Key="a xyz",Entry="@i(xyz)",Number]
				=> index object for " xyz"  (note initial space;  text be italic)
			@indexsecondary[Primary="xyz",Secondary="abc",Number]
				=> index object for  xyz++abc
		*/
		case indexNUMBER:  
			endc = ScribeDelimiter(&tc);
			if (endc == ' ') break;	/* ignore error */
			SaveWhiteSpace();

			OpenEnvt(inxiStyle, 'x', '-');
			appendtobuf(NULL, 0, TRUE);
			CloseEnvt('x', NULL);

			if (nextch == endc)
				getnextch();	/* skip terminating delimiter */
			else {
				C1[0] = endc;
				Error("Missing %s after @index", C1);
			}
			break;
		case labelNUMBER:
		    subobjtype = "texttag";
		case pagerefNUMBER:
		case refNUMBER:
		    if (outp > buf + (AfterNewline ? 1 : 0)) {
			/* the line is not empty.  flush it */
			ProcessLine(FALSE);
		    }
		    endc = ScribeDelimiter(&tc);
		    if (endc == ' ') break;	/* ignore error */
		    SaveWhiteSpace();
		    if((subobj = (class text *)ATK::NewObject(subobjtype)) == NULL){
			appendtobuf(NULL, 0, FALSE);
		    }
		    else {
			/* read the stuff in delimiter and stuff in subobj */
			char cbuf[512] ;
			appendtobuf(cbuf,512,FALSE);
			(subobj)->InsertCharacters(0,cbuf,strlen(cbuf));
			if(code == pagerefNUMBER) (subobj)->InsertCharacters(0,"# ",2);
			(Doc)->AddView(WhereTo,(subobj)->ViewName(), subobj);
			DocPos++;
			WhereTo++;
		    }
		    if (nextch == endc)
			getnextch();	/* skip terminating delimiter */
		    else {
			C1[0] = endc;
			Error("Missing %s after @index", C1);
		    }
		    break;
		case ientryNUMBER:
		case isecNUMBER:
			endc = ScribeDelimiter(&tc);
			if (endc == ' ') break;	/* ignore error */
			OpenEnvt(inxiStyle, endc, '-');
			while (nextch != endc) {
				word = CollectWord(&len);
				SaveWhiteSpace();
				if (strcmp(word, "entry") == 0) {
					pout(' ');
					appendtobuf(NULL, 0, TRUE);
				}
				else if (strcmp(word, "primary") == 0) 
					appendtobuf(NULL, 0, TRUE);
				else if (strcmp(word, "secondary") == 0) {
					pout('+');
					pout('+');
					appendtobuf(NULL, 0, TRUE);
				}
				else if (nextch == '=') 
					appendtobuf(NULL, 0, FALSE);
				else if (len == 0)
					/* skip error character */
					getnextch();
			}
			CloseEnvt(endc, NULL);
			getnextch();	/* skip terminating delimiter */
			break;

		/* XXX later, need to parse @defines */
		/* for now, pass it along by DROP THROUGH to default case */
		case defineNUMBER: 
		default:   /* process a reserved word environment */
			if (outp == &buf[(AfterNewline?1:0)])
				LineStartCommand = TRUE;
			endc = ScribeDelimiter(&tc);
			OpenEnvt(passStyle, endc, ')');
			pout('@');
			putstring(word, len);
			if (endc == ' ') {
				/* @word without delimited text */
				CloseEnvt(endc, NULL);
				OutputWhiteSpace();
			}
			else 
				pout(tc);
			SaveWhiteSpace();
			OutputWhiteSpace();
			break;
	}
}

	static void
OpenEnvt(class style  *style , char endc , unsigned  closemode) 
			{
	struct stylemap *map;
	scribeEnd[++endsp] = endc;
	endMode[endsp] = closemode;

	map = (struct stylemap *)malloc(sizeof(struct stylemap));
	map->pos = DocPos;
	map->s = style;
	map->next = StyleMap;
	StyleMap = map;
	currMap[endsp] = map;

	if ((style)->IsPassThruUnchange() 
			&& (style)->TestUseOldFlag( style_NoWrap)) 
		NoFill[endsp] = NoFill[endsp-1];
	else 
		NoFill[endsp] = (style)->IsPassThruAdded()  
			||  (style)->TestAddFlag( style_NoWrap);
}

	static void
CloseEnvt( char c, char *word)
		{
	static char C1[2];
	while (endsp > 0) {

		/* save end info to see if line break needed */
		LatestEndLoc = outp;
		LatestEndMap = currMap[endsp];

		if (scribeEnd[endsp] == c) {
			switch(endMode[endsp]) {
			case ')':  
				pout(c);
				break;
			case '@':
				putstring("@end[", 5);
				putstring(word, strlen(word));
				pout(']');
				break;
			}
			currMap[endsp]->len = DocPos - currMap[endsp]->pos;
			endsp--;
			return;
		}
		currMap[endsp]->len = DocPos - currMap[endsp]->pos;
		C1[0] = scribeEnd[endsp];
		Error("Missing '%s' detected by %s", C1,
				(char *)(c == 'z' ? "end of file" : "@end"));
		endsp--;
	}
	endsp = 0;
	if (c == 'e') 
		Error("Excess @end");
}

	static void
ProcessLine(boolean  NextIsNewline)
	{
		/* Scans the output buffer and decides whether
		preceding newline is real or should be a blank. 

		Also may decide to put a for-real newline at the end of this line
		or a possible newline at the beginning of the next.  */
	/* initial state is 
		text of line is in buf, 
		AfterNewline indicates whether buf[0] is a newline 
				at end of previous line

		Processing may do any of the following:
				replace old buf[0] with ' '
				add 1 or 2 spaces at end of line (after .;:?!)
				remove \ from end of line
				set AfterNewline
				InsertString of text into output
				set buf[0] for next line
	*/
	char *cx;
	boolean TabOnlyAtEnd = FALSE;  /* true for line with 
				EmbeddedWhiteSpace==1 and last char =='\t' 
				(This catches the mail kludge of using
				\t at end of line instead of @*) */

	LineNo++;

	*outp = '#';  /* dummy characters after line end */
	*(outp+1) = ' ';
	if (EmbeddedWhiteSpace == 1 && *(outp-1)=='\t') {
		EmbeddedWhiteSpace = 0;
		TabOnlyAtEnd = TRUE;
	}
	if (AfterNewline) {
		/* decide whether to replace buf[0] with blank 
		   If one of the conditions after the ! below is TRUE,
		   then the newline at beginning of line will remain a newline */
		if (! (outp <= buf+1 || EmbeddedWhiteSpace || StartsWhite
				|| LineStartCommand ) )
			/* replace \n with space */
			*buf = ' '; 
	}


	/* now decide whether to put newline at end of this line
		or leave it at front of the next */

	AfterNewline = FALSE;

	/* find last non-blank character */
	cx = outp-1;   
	while (cx>=buf && *cx==' ') {cx--;}
	if (cx<buf) cx = outp;  /* we know that *outp=='#' */

	if ( ! NextIsNewline)  
		/* we are not really at end of line, do nothing */
		{}
	else if (*(outp-1)=='\\') {
		/* backslash-newline is converted to blank 
			and partial line is flushed below */
		*(outp-1) = ' ';
	}
	else if (EmbeddedWhiteSpace || ! HaveNonWhiteSpace
			|| TabOnlyAtEnd   /* MailKludge */
			|| LineEndCommand 
			|| NoFill[endsp]
			||	/* want a newline at end if line ends with a style
				   that has KeepNextNL set */
				(LatestEndLoc == cx  &&
					(LatestEndMap->s)->TestAddFlag(
						style_KeepNextNL))
		) {
		/* unconditionally get newline at end of line */
		pout('\n');
	}
	else {  /* is tentative \n, which may be replaced with space;
			ensure two spaces after punct at end of line:
				punct ["] space* => punct ["] space
				non-punct space space* => non-punct space* 
				An additional space will occur from \n */
		int havespace = outp-cx-1;
		if ((*cx=='"' || *cx==')') && cx>buf) cx--;
		if (*cx=='.'||*cx=='!'||*cx==';'||*cx==':'||*cx=='?') {
			if (havespace<1) pout(' ');
			else if (havespace>1) {
				outp--;
				DocPos--;
			}
		}
		else if (havespace>0){
			outp--;
			DocPos--;
		}

		AfterNewline = TRUE;
	}

	/* output the line */
	(Doc)->InsertCharacters ( WhereTo, buf, outp-buf);
	WhereTo += outp-buf;
	outp = &(buf[0]);

	/* start next line */
	if (AfterNewline) 
		pout('\n');		/* may be replaced by blank later */

	LineStartCommand = FALSE;
	LineEndCommand = FALSE;
	StartsWhite = HaveNonWhiteSpace = FALSE;
	EmbeddedWhiteSpace = 0;
	LatestEndLoc = NULL;
}

