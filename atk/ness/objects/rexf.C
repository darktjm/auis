#define NESS_INTERNALS
/* ********************************************************************** *\
 *	   Copyright Carnegie Mellon Univ. 1992, 1993 - All Rights Reserved
\* ********************************************************************** */

/*
	rexf.c	-  implement a REXX compatability package

	Unlike most Ness functions, these usually create new strings.
	Therefore, they are much less efficient and should be avoided.
	Many Rexx functions have optional arguments;  in all cases
		a fixed number of arguments are implemented.


    Entry point:

	DoRex(op) - executes operation indicated by op

    Uses global:

	NSPstore - the current stack pointer
*/

/*
 * $Log: rexf.C,v $
 * Revision 1.8  1995/04/05  01:48:15  rr2b
 * don't export internally used inline functions to ness client code
 *
 * Revision 1.7  1994/08/12  20:43:51  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.6  1994/03/21  17:00:38  rr2b
 * bzero->memset
 * bcopy->memmove
 * index->strchr
 * rindex->strrchr
 * some mktemp->tmpnam
 *
 * Revision 1.5  1993/12/16  00:20:43  wjh
 * moved description of possible additions to ../doc/future.doc
 *
 * Revision 1.4  1993/08/26  02:59:58  rr2b
 * Fixed comments in a comment.
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
 * Revision 1.6  1992/12/15  21:38:20  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.5  1992/12/15  00:53:33  rr2b
 * fixed disclaimerization
 *
 * Revision 1.4  1992/12/14  20:50:00  rr2b
 * disclaimerization
 *
 * Revision 1.3  1992/11/26  02:42:25  wjh
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
 * Revision 1.2  92/11/14  15:44:39  wjh
 * fix illegal pointer messages for stupid Sun char * conventions
 * 
 * Revision 1.1  92/05/28  17:54:13  wjh
 * Initial revision
 * 
 * 
*/

#include <andrewos.h>	/* for bzero() bcmp() */
#include <ctype.h>

#include <text.H>
#include <simpletext.H>
#include <nestedmark.H>

#include <nessmark.H>
#include <ness.H>

/* iar is no longer needed as an argument to RunError */
#define IAR 0


static long popInt(long  min);
static char * popMtoC();
static class nessmark * copyTop(long  n);
static void pushInt(long  n);
static void pushBool(boolean  bv);
static void Xpad(long  pos , long  len , long  pre , long  post, char  *c);
static long NextWord(class nessmark  *m, long  maxpos);
static long SqueezeSpaces(class nessmark  *m);
static void InsertSpaces(class simpletext  *stxt, int pos, int n);
static void SpreadSpaces(class nessmark  *m, long  desiredlen , long  actuallen , long  nsp);
void DoRex(unsigned char op);


/* ______________________________________________
  Helper functions
*/

/* popInt(min)
	remove an integer from the stack (at NSPstore)
	check that it is at least as great as min
*/
	static long
popInt(long  min) {
	struct longstkelt *l = &NSPstore->l;
	long v;
	if (l->hdr != longHdr)
		RunError(":An argument type should be integer", IAR);
	v = l->v;
	if (v <min)
		RunError(":Integer argument is too small", IAR);
	popValue(NSPstore);
	return v;
}

/* popMtoC()
	removes a marker from stack, returning its value as a C string
	the caller must free the string
*/
	static char *
popMtoC() {
	char *p = (char *)(FETCHMARK(NSPstore))->ToC();
	popValue(NSPstore);
	return p;
}

/* copyTop(n)
	arg is offset in stack
	check that top element is marker
	copy text referenced by top elt on stack
	returns pointer to new marker
*/
	static class nessmark *
copyTop(long  n) {
	class nessmark *m = FETCHMARK((union stackelement *)
				(((long)NSPstore) + n));
	long len = (m)->GetLength();
	class text *t = new text;
	(t)->AlwaysCopyTextExactly( 0, (m)->GetText(), 
			(m)->GetPos(), len);
	(m)->Set( t, 0, len);
	return m;
}

/* 
pushInt(n)
	push integer n to stack
*/
	static void
pushInt(long  n) {
	struct longstkelt *l;
	NSPstore=(union stackelement *)
		(((unsigned long)NSPstore) - sizeof(struct longstkelt));
	l = &NSPstore->l;
	l->hdr = longHdr;
	l->v = n;
}

/* pushBool(b)
	push boolean b to stack
*/
	static void
pushBool(boolean  bv) {
	struct boolstkelt *b;
	NSPstore=(union stackelement *)
		(((unsigned long)NSPstore) - sizeof(struct boolstkelt));
	b = &NSPstore->b;
	b->hdr = boolHdr;
	b->v = bv;
}

/* Xpad(pos, len, pre, post, c)
	&NSPstore->m is a marker, say M
	create a new text and make the marker refer to it
	the new text has the selected portion of M,
		but limited to the part starting at pos and
		extending for len chars.
	if c is non-NULL,  the selected text is preceded
		in the result by pre copies of c
		and followed by post copies of c
`		c is freed
	if c is NULL, the selected text is preceded and followed
		by pre and post spaces, respectively
	WARNING: 'pos' starts at zero, while rexx functions
		start counting positions at one
*/
	static void
Xpad(long  pos , long  len , long  pre , long  post, char  *c) {
	boolean oldgliso;
	class text *t = new text;
	class nessmark *m = FETCHMARK(NSPstore);
	long tlen = len + post + pre;
	(t)->AlwaysCopyTextExactly( 0, (m)->GetText(),
			(m)->GetPos() + pos, len);
	oldgliso = nestedmark::SetGlobalIsolation(TRUE);
	if (c) {
		for ( ; post > 0; post --)
			(t)->AlwaysInsertCharacters( len, c, 1);
		for ( ; pre > 0; pre --)
			(t)->AlwaysInsertCharacters( 0, c, 1);
		free(c);
	}
	else {
		for ( ; post > 0; post --)
			(t)->AlwaysInsertCharacters( len, " ", 1);
		for ( ; pre > 0; pre --)
			(t)->AlwaysInsertCharacters( 0, " ", 1);
	}
	nestedmark::SetGlobalIsolation(oldgliso);
	(m)->Set( t, 0, tlen);
}

/* NextWord(nessmark, maxpos)
	revise nessmark so it refers to the next blank-delimited word
	after its current end.	But do not use or exceed position maxpos
	return position of new word
*/
	static long
NextWord(class nessmark  *m, long  maxpos) {
	class simpletext *stxt = (m)->GetText();
	long pos = (m)->GetPos() + (m)->GetLength();
	long startword;
	while (pos < maxpos && isspace((stxt)->GetUnsignedChar( pos)))
		pos ++;
	startword = pos;
	while (pos < maxpos && ! isspace((stxt)->GetUnsignedChar( pos)))
		pos ++;
	(m)->SetPos( startword);
	(m)->SetLength( pos - startword);
	return startword;
}

/* remove leading and trailing spaces and reduce multiple space to single
	returns number of spaces remaining
*/
	static long
SqueezeSpaces(class nessmark  *m) {
	boolean hadspace;
	long t, slen, spaces;
	class simpletext *stxt;

	slen = (m)->GetLength();
	stxt = (m)->GetText();
	hadspace = TRUE;       /* (side effect: delete leading spaces) */
	spaces = 0;
	for (t = 0; t < slen; ) {
		if (isspace((stxt)->GetUnsignedChar( t))) {
			if ( ! hadspace)
				hadspace = TRUE, t++, spaces++;
			else {
				(stxt)->AlwaysDeleteCharacters( t, 1);
				slen--;
				/* no change to t */
			}
		 }
		 else hadspace = FALSE,  t++;
	}
	if (hadspace) {
		/* remove trailing space */
		(stxt)->AlwaysDeleteCharacters( slen-1, 1);
		slen--,  spaces--;
	}
	(m)->SetLength( slen);
	return spaces;
}

	static void
InsertSpaces(class simpletext  *stxt, int pos, int n) {
	boolean oldgliso;
	static char ten[11] = "          ";
	oldgliso = nestedmark::SetGlobalIsolation(TRUE);
	while (n > 10)
		(stxt)->AlwaysInsertCharacters( pos, ten, 10);
	(stxt)->AlwaysInsertCharacters( pos, ten, n);
	nestedmark::SetGlobalIsolation(oldgliso);
}

	static void
SpreadSpaces(class nessmark  *m, long  desiredlen , long  actuallen , long  nsp) {
	long q, r, pos, n;
	class simpletext *stxt = (m)->GetText();
	static boolean FromLeft = TRUE;

	if (nsp == 0) {
		/* no spaces to expand: pad at the right */
		InsertSpaces(stxt, (m)->GetLength(), 
				desiredlen - actuallen);
		return;
	}
	q = (desiredlen - actuallen) / nsp;
	r = desiredlen - actuallen - q * nsp;
	for (pos = 0; pos < desiredlen; pos++)
		if (isspace((stxt)->GetUnsignedChar( pos))) {
			/* process a space; expand it */
			/* first determine the number of spaces */
			n = q;
			if (FromLeft) {
				if (r > 0) n++, r--;
			}
			else if (nsp <= r) n++;
			InsertSpaces(stxt, pos, n);
			pos += n;
			nsp --;
		}
	FromLeft = ! FromLeft;
}


/* _______________________________________________________
  Main routine
*/

	void
DoRex(unsigned char op) {
	long len, slen, pos, t;
	char c;
	char *p, *q, *r;
	class text *txt;
	class simpletext *stxt, *stxt2;
	class nessmark *m, *m2;
	boolean oldgliso;

	switch (op) {


/*_____________________________________________________________
  Extract a substring, possibly padded at ends
*/


/* centerx(string, length)
  CENTERX("abc", 7)             ->    "  abc  "
  CENTERX("abc", 8)             ->    "  abc   "
  CENTERX("The blue sky", 8)    ->    "e blue s"
  CENTERX("The blue sky", 7)    ->    "e blue "
  CENTERX("The blue seas", 8)    ->    "e blue s"
  CENTERX("The blue seas", 7)    ->    " blue s"
*/
case 'a':
	p = NULL;
docenter:
	len = popInt(0);
	m = FETCHMARK(NSPstore);
	slen = (m)->GetLength();
	if (len == slen)
		Xpad(0, slen, 0, 0, p);
	else if (len < slen)
		/* crop the extraction */
		Xpad((slen - len)/2, len, 0, 0, p);
	else {
		/* extract and pad */
		t = (len - slen)/2;
		Xpad(0, slen, t, len-slen-t, p);
	}
	return;

/* centerpadx(string, length, pad)
  CENTERPADX("abc", 7, ".")             ->    "..abc.."
  CENTERPADX("abc", 8, ".")             ->    "..abc..."
  CENTERPADX("The blue sky", 8, ".")    ->    "e blue s"
  CENTERPADX("The blue sky", 7, ".")    ->    "e blue "
*/
case 'b':
	p = popMtoC();
	goto docenter;

/* leftx(string, length)
  LEFTX("abc d", 8)             ->    "abc d"
  LEFTX("abc  def", 7)          ->    "abc  de"
*/
case 'c':
	len = popInt(0);
	m = FETCHMARK(NSPstore);
	slen = (m)->GetLength();
	if (len > slen) len = slen;
	Xpad(0, len, 0, 0, NULL);
	return;

/* padx(string, length, pad)
  PADX("test", 7, ".")          ->    "test..."
  PADX("test", 3, ".")          ->    "tes"
*/
case 'd':
	p = popMtoC();
	len = popInt(0);
	m = FETCHMARK(NSPstore);
	slen = (m)->GetLength();
	if (len < slen)
		Xpad(0, len, 0, 0, p);
	else
		Xpad(0, slen, 0, len - slen, p);
	return;

/* prepadx(string, length, pad)
  PREPADX("test", 7, ".")       ->    "...test"
  PREPADX("test", 3, ".")       ->    "est"
*/
case 'e':
	p = popMtoC();
	len = popInt(0);
	m = FETCHMARK(NSPstore);
	slen = (m)->GetLength();
	if (len < slen)
		Xpad(slen - len, len, 0, 0, p);
	else
		Xpad(0, slen, len - slen, 0, p);
	return;

/* prestripx(string, chars)
  PRESTRIPX("  ab c  ", " .")   ->    "ab c  "
  PRESTRIPX("  ab c  ", " a")   ->    "b c  "
*/
case 'f':
	p = popMtoC();
	m = FETCHMARK(NSPstore);
	stxt = (m)->GetText();
	slen = (m)->GetLength();
	pos = (m)->GetPos();
	while (slen > 0  &&  0 !=
		strchr(p, (stxt)->GetUnsignedChar( pos)))
	    pos++, slen--;
	Xpad(pos - (m)->GetPos(), slen, 0, 0, NULL);
	free(p);
	return;

/* rightx(string, length)
  RIGHTX("abc  d", 8)           ->    "abc  d"
  RIGHTX("abc def", 5)          ->    "c def"
*/
case 'g':
	len = popInt(0);
	m = FETCHMARK(NSPstore);
	slen = (m)->GetLength();
	if (len > slen)
		Xpad(0, slen, 0, 0, NULL);
	else
		Xpad(slen - len, len, 0, 0, NULL);
	return;

/* stripx(string, chars)
  STRIPX("  ab c  ", " .")      ->    "  ab c"
  STRIPX("  ab c  ", " c")      ->    "  ab"
*/
case 'h':
	p = popMtoC();
	m = FETCHMARK(NSPstore);
	slen = (m)->GetLength();
	stxt = (m)->GetText();
	pos = (m)->GetPos() + slen - 1;
	while (slen > 0  &&  0 !=
		strchr(p, (stxt)->GetUnsignedChar( pos)))
	    pos--, slen--;
	Xpad(0, slen, 0, 0, NULL);
	free(p);
	return;

/* substrx(string, pos, length)
	Substrx extracts a substring of specified length, starting at pos.
	If the length of the string is L, the returned length is the shorter
	oflength or L-pos+1, but not less than zero.
  SUBSTRX("abc", 2, 9999)       ->    "bc"
  SUBSTRX("abc", 2, 1)          ->    "b"
*/
case 'i':
	len = popInt(0);
	t = popInt(1) - 1;  /* (convert to zero-base) */
	slen = (FETCHMARK(NSPstore))->GetLength();
	if (len > slen - t)
		Xpad(t, slen-t, 0, 0, NULL);
	else
		Xpad(t, len, 0, 0, NULL);
	return;

/* SUBWORDX(string, n, length)
returns the substring of string that starts at the nth word, and is of
length length blank-delimited words.  n must be a positive whole
number. The returned string will never have leading or trailing
blanks, but will include all blanks between the selected words.
  SUBWORDX("Now is the  time", 2, 2)     ->    "is the"
  SUBWORDX("Now is the  time", 3, 9999)  ->    "the  time"
  SUBWORDX("Now is the  time", 5, 9999)  ->    ""
*/
case 'j':
	len = popInt(0);
	t = popInt(1);
	m = FETCHMARK(NSPstore);
	pos = (m)->GetPos() + (m)->GetLength();
	(m)->SetLength( 0);
	for ( ; t > 0; t--)
		NextWord(m, pos);
	t = (m)->GetPos();
	for ( ; len > 1; len --)
		NextWord(m, pos);
	slen = (m)->GetPos() + (m)->GetLength() - t;
	(m)->SetPos( t);
	(m)->SetLength( slen);
	Xpad(0, slen, 0, 0, NULL);
	return;

/* wordx(string, n)
  WORDX("Now is the time", 3)   ->    "the"
  WORDX("Now is the time", 5)   ->    ""
*/
case 'k':
	t = popInt(1);
	m = FETCHMARK(NSPstore);
	pos = (m)->GetPos() + (m)->GetLength();
	(m)->SetLength( 0);
	for ( ; t > 0; t--)
		NextWord(m, pos);
	Xpad(0, (m)->GetLength(), 0, 0, NULL);
	return;


/* __________________________________________________________
   Copy source and modify it  -  one string arg
*/

/* copiesx(string, n)
  COPIESX("abc", 3)             ->    "abcabcabc"
  COPIESX("abc", 0)             ->    ""
*/
case 'o':
	len = popInt(0);
	if (len == 0) {
		Xpad(0, 0, 0, 0, NULL);
		return;
	}
	m = copyTop(0);
	slen = (m)->GetLength();
	txt = (class text *)(m)->GetText();
	oldgliso = nestedmark::SetGlobalIsolation(TRUE);
	for (t = len; t > 1; t--)
		(txt)->AlwaysCopyTextExactly( (txt)->GetLength(),
				txt, 0, slen);
	nestedmark::SetGlobalIsolation(oldgliso);
	(m)->SetLength( len * slen);
	return;

/* DELSTRX(string, n, length)
deletes the substring of string that begins at the nth character, and
is of length length.  If length+n-1 exceeds length of string, the rest of
string is deleted.  If n is greater than the length of string, the string
is returned unchanged.	n must be a positive whole number.
  DELSTRX("abcd", 3, 9999)      ->    "ab"
  DELSTRX("abcde", 3, 2)        ->    "abe"
  DELSTRX("abcde", 6, 9999)     ->    "abcde"
*/
case 'p':
	len = popInt(0);
	pos = popInt(1) - 1;
	m = copyTop(0);
	slen = (m)->GetLength();
	if (pos > slen)
		return;
	if (len + pos > slen)
		len = slen - pos;
	((m)->GetText())->AlwaysDeleteCharacters( pos, len);
	return;

/* delwordx(string, n, length)
	{Length is required. Use 999999 for absent length.}
  DELWORDX("Now is the  time", 2, 2)       ->  "Now time"
  DELWORDX("Now is the time ", 3, 9999)    ->  "Now is "
  DELWORDX("Now is the  time", 5, 9999)    ->  "Now is the  time"
*/
case 'q':
	len = popInt(0);
	t = popInt(1);
	m = copyTop(0);
	if (len < 1) return;
	stxt = (m)->GetText();
	slen = (m)->GetLength();
	(m)->SetLength( 0);
	/* skip t - 1 words and first following blank */
	for ( ; t > 1; t--)
		NextWord(m, slen);
	pos = (m)->GetPos() +
			(m)->GetLength();
	if (isspace((stxt)->GetUnsignedChar( pos)))
		pos++;	/* skip one blank */
	/* now skip len more words and following blanks */
	for ( ; len > 0; len --)
		NextWord(m, slen);
	t = (m)->GetPos() +
			(m)->GetLength();
	while (isspace((stxt)->GetUnsignedChar( t)))
		t++;

	/* delete pos...t-1 */
	(stxt)->AlwaysDeleteCharacters( pos, t-pos);
	return;

/* JUSTIFYX(string, length)
formats blank-delimited words in string, by adding blanks
between words to justify to both margins.  That is, to width length
(length must be non-negative).
string is first normalized as though SPACE(string) had been executed
(that is, multiple blanks are converted to single blanks, and leading
and trailing blanks are removed).  If length is less than the width of
the normalized string, the string is then truncated on the right and
any trailing blank is removed.	Extra blanks are then added
evenly from left to right to provide the required length.
  JUSTIFYX("The blue sky", 14)        ->    "The  blue  sky"
  JUSTIFYX("The     blue  sky ", 8)   ->    "The blue"
  JUSTIFYX("  The blue    sky", 9)    ->    "The  blue"
*/
case 'r':
	len = popInt(0);
	m = copyTop(0);
	t = SqueezeSpaces(m);
	slen = (m)->GetLength();
	if (slen > len) {
		/* already too long: truncate */
		((m)->GetText())->AlwaysDeleteCharacters(
				len, slen - len);
		if ( ! isspace(((m)->GetText())->GetUnsignedChar( 
					len-1)))
			return;
		t = SqueezeSpaces(m);
		slen = (m)->GetLength();
	}
	SpreadSpaces(m, len, slen, t);
	return;

/* reversex(string)
  REVERSEX("ABc.")              ->    ".cBA"
  REVERSEX("XYZ ")              ->    " ZYX"
*/
case 's':
	m = FETCHMARK(NSPstore);
	txt = new text;
	stxt = (m)->GetText();
	slen = (m)->GetLength();
	len = 0;
	pos = (m)->GetPos() + slen - 1;
	oldgliso = nestedmark::SetGlobalIsolation(TRUE);
	for ( ; slen > 0; slen--, pos--, len++)
		(txt)->AlwaysCopyTextExactly( len, stxt, pos, 1);
	nestedmark::SetGlobalIsolation(oldgliso);
	(m)->Set( txt, 0, len);
	return;

/* spacex(string)
  SPACEX(" test  words ")       ->   "test words"
  SPACEX("test words  too ")    ->   "test words too"
*/
case 't':
	m = copyTop(0);
	SqueezeSpaces(m);
	return;

/* tolowerx(string)
	Convert ASCII letters in string to lower case.
  TOLOWERX("This IS a tEst")    ->  "this is a test"
*/
case 'u':
	m = copyTop(0);
	slen = (m)->GetLength();
	stxt = (m)->GetText();
	for (t = 0; t < slen; t++) {
		c = (stxt)->GetChar( t);
		if (isascii(c) && isupper(c)) {
			c = tolower(c);
			(stxt)->AlwaysReplaceCharacters( t, 1, &c, 1);
		}
	}
	return;

/* toupperx(string)
	Convert ASCII letters in string to upper case.
  TOUPPERX("This IS a tEst")    ->  "THIS IS A TEST"
*/
case 'v':
	m = copyTop(0);
	slen = (m)->GetLength();
	stxt = (m)->GetText();
	for (t = 0; t < slen; t++) {
		c = (stxt)->GetChar( t);
		if (isascii(c) && islower(c)){
			c = toupper(c);
			(stxt)->AlwaysReplaceCharacters( t, 1, &c, 1);
		}
	}
	return;



/* ____________________________________________________________
  Copy source and modify it  -	other than one string arg
*/

/* INSERTX(new, target, n)
inserts the string new, into the string target after the nth character.
n must be non-negative.  If n is greater than the length of the target
string, blank padding is added.
  INSERTX(" ", "abcdef", 3)     ->    "abc def"
  INSERTX("123", "abc", 5)      ->    "abc  123"
  INSERTX("123", "abc", 0)      ->    "123abc"
*/
case 'A':
	pos = popInt(0);
	m = FETCHMARK(NSPstore);
	m2 = FETCHMARK((union stackelement *)&(&NSPstore->s)[1]);
	txt = new text;    /* result text */
	slen = (m)->GetLength();
	(txt)->AlwaysCopyTextExactly( 0,
			(m)->GetText(), (m)->GetPos(), slen);
	if (slen < pos) {
		InsertSpaces((class simpletext *)txt, slen, pos - slen);
		slen = pos;
	}
	len = (m2)->GetLength();
 	oldgliso = nestedmark::SetGlobalIsolation(TRUE);
	(txt)->AlwaysCopyTextExactly( pos,
		(m2)->GetText(), (m2)->GetPos(), len);
	nestedmark::SetGlobalIsolation(oldgliso);
	slen += len;
	(m2)->Set( txt, 0, slen);
	popValue(NSPstore);
	return;

/* OVERLAYX(new, target, n)
overlays the string target with the string new starting at the nth
character.  If n is greater than the length of the target string, the
target is padded with blanks.  n must be greater than 0.
  OVERLAYX(" ", "abcdef", 3)    ->    "ab def"
  OVERLAYX(".", "abcdef", 3)    ->    "ab.def"
  OVERLAYX("qq", "abcd", 1)     ->    "qqcd"
  OVERLAYX("qq", "abcd", 4)     ->    "abcqq"
  OVERLAYX("123", "abc", 5)     ->    "abc 123"
*/
case 'B':
	pos = popInt(1) - 1;
	m = FETCHMARK(NSPstore);
	m2 = FETCHMARK((union stackelement *)&(&NSPstore->s)[1]);
	slen = (m)->GetLength();
	stxt = (m)->GetText();
	len = (m2)->GetLength();
	t = (m)->GetPos();
	txt = new text;    /* result text */
	if (pos < slen) 
		(txt)->AlwaysCopyTextExactly( 0, stxt, t, pos);
	else {
		(txt)->AlwaysCopyTextExactly( 0, stxt, t, slen);
		InsertSpaces((class simpletext *)txt, slen, pos-slen);
	}
	oldgliso = nestedmark::SetGlobalIsolation(TRUE);
	(txt)->AlwaysCopyTextExactly( pos, 
			(m2)->GetText(), (m2)->GetPos(), len);
	if (slen > pos+len)
		(txt)->AlwaysCopyTextExactly( pos+len, 
				stxt, t+pos+len, slen - (pos+len));
	nestedmark::SetGlobalIsolation(oldgliso);
	(m2)->Set( txt, 0, slen);
	popValue(NSPstore);
	return;

/* TRANSLATEX(string, tableo, tablei)
Translates characters in string to be other characters, or may be used to
reorder characters in a string.  tableo is the output table and tablei is
the input translate table.  The tables may be of any length:  the first
occurrence of a character in the input table is the one that is used if
there are duplicates.  If the output table is shorter than the input,
the corresponding characters are deleted from the result.
  TRANSLATEX("abbc", "&", "b")            ->    "a&&c"
  TRANSLATEX("abcdef", "12", "ec")        ->    "ab2d1f"
  TRANSLATEX("abcdef", "12", "abcd")      ->    "12ef"
  TRANSLATEX("4123", "abcd", "1234")      ->    "dabc"
*/
case 'C':
	p = popMtoC();	/* input table */
	q = popMtoC();	/* output table */
	len = strlen(q);
	m = copyTop(0);
	slen = (m)->GetLength();
	stxt = (m)->GetText();

	/* for each character of stxt, find in p and replace from q */
	for (pos = 0; pos < slen; pos++) {
		c = (stxt)->GetChar( pos);
		r = strchr(p, c);
		if (r != NULL) {
			t = r-p; /* convert to offset */
			if (t < len)
				(stxt)->AlwaysReplaceCharacters( 
						pos, 1, &q[t], 1);
			else {
				/* delete char from output */
				(stxt)->AlwaysDeleteCharacters( pos, 1);
				pos --;
				len --;
			}
		}
	}
	free(p);
	free(q);
	return;


/* __________________________________________________________
  Generate a string
*/

/* XRANGEX(start, end)
returns a string of all one byte codes between and including the
values start and end.
If start is greater than end, the values will wrap from X"FF"
to X"00".  start and end must be single characters.
  XRANGEX("a", "f")             ->    "abcdef"
  XRANGEX("\3", "\7")           ->    "\3\4\5\6\7"
  XRANGEX("i", "j")             ->    "ij"
  XRANGEX("\376", "\2")         ->    "\376\377\0\1\2"
*/
case 'H':
	m = FETCHMARK(NSPstore);		/* high end */
	m2 = FETCHMARK((union stackelement *)&(&NSPstore->s)[1]); /* low end */
	stxt = (m)->GetText();
	if ((m)->GetLength() >= 1)
		slen = (stxt)->GetUnsignedChar( (m)->GetPos());
	else slen = 255;
	stxt = (m2)->GetText();
	if ((m2)->GetLength() >= 1)
		pos = (stxt)->GetUnsignedChar( (m2)->GetPos());
	else pos = 0;
	/* range is pos...slen */
	len = slen - pos + 1;
	if (len <= 0) len +=256;
	/* create text to hold chars and fill in codes */
	stxt = (class simpletext *)new text;
	for (t = 0; t < len; t++) {
		c = (t + pos) % 256;
		(stxt)->AlwaysInsertCharacters( t, &c, 1);
	}
	(m2)->Set( stxt, 0, len);
	popValue(NSPstore);
	return;

/* _______________________________________
  Integer  -  from two strings
*/

/* findx(string, phrase)
  FINDX("now is the time", "is the time")    ->    2
  FINDX("now is  the time", "is    the")     ->    2
  FINDX("now is  the time", "is  time ")     ->    0

	the algorithm squeezes the spaces out of both operands
	and just uses search()
*/
case 'K':
	m = copyTop(0); 			   /* phrase */
	m2 = copyTop(sizeof(class nessmark));	   /* subject */
	SqueezeSpaces(m);
	SqueezeSpaces(m2);
	SearchOp('a', IAR);                    /* search() */
	m = FETCHMARK(NSPstore);
	if ((m)->GetLength() == 0)
		t = 0;	 /* not found */
	else {
		/* count words preceding found string */
		pos = (m)->GetPos();	/* start of found string */
		(m)->SetLength( 0);
		(m)->SetPos( 0);
		t = 1;	      /* position of phrase (in words) */
		while (TRUE) {
			NextWord(m, pos);
			if ((m)->GetLength() == 0)
				break;	/* no more words */
			t ++;
		}
	}
	popValue(NSPstore);
	pushInt(t);
	return;


/* indexx(string, target)
{No "start" parameter is allowed.  Use substr.}
  INDEXX("abcdef", "cd")        ->    3
  INDEXX("abcdef", "xd")        ->    0

 posx(needle, haystack)
  POSX("day", "Saturday")       ->    6
  POSX("x", "abc def ghi")      ->    0
  POSX(" ", "abc def ghi")      ->    4

POSX is INDEXX with swapped args.  The swapping is done in the opcodes
compiled by the table in call.c
  implement:  indexx(string, target)
*/
case 'L':
	m = FETCHMARK(NSPstore);
	pos = (m)->GetPos();
	SearchOp('a', IAR);                    /* search() */
	if ((m)->GetLength() == 0)
		t = 0;
	else
		t = (m)->GetPos() - pos + 1;
	popValue(NSPstore);
	pushInt(t);
	return;

/* verifyx(string, reference)
  VERIFYX("123", "1234567890")  ->    0
  VERIFYX("1Z3", "1234567890")  ->    2
*/
case 'O':
	m = FETCHMARK(NSPstore);
	slen = FETCHMARK((union stackelement *)&(&NSPstore->s)[1])
			->GetLength();
	SearchOp('d', IAR);        /* span() */
	len = (m)->GetLength();
	if (slen == len)
		t = 0;
	else
		t = len + 1;
	popValue(NSPstore);
	pushInt(t);
	return;


/* ___________________________________________
  Integer - not from two strings
*/

/* WORDINDEXX(string, n)
returns the position of the nth blank-delimited word in string.  n
must be a positive whole number.  If there are not n words in the
string, 0 is returned.
  WORDINDEXX("Now is the time", 3)    ->    8
  WORDINDEXX("Now is the time", 6)    ->    0
*/
case 'S':
	len = popInt(1);
	m = FETCHMARK(NSPstore);
	pos = (m)->GetPos();
	slen = (m)->GetLength() + pos;	/* position of end */
	(m)->SetLength( 0);
	while (TRUE) {
		NextWord(m, slen);
		if ((m)->GetLength() == 0) {
			t = 0;
			break;
		}
		len--;
		if (len < 1) {
			t = (m)->GetPos() - pos + 1;
			break;
		}
	}
	popValue(NSPstore);
	pushInt(t);
	return;

/* wordsx(string)
  WORDSX("Now is the time")     ->    4
  WORDSX(" ")                   ->    0
*/
case 'T':
	m = FETCHMARK(NSPstore);
	pos = (m)->GetLength() + (m)->GetPos();
	(m)->SetLength( 0);
	len = 0;	/* number of words so far */
	while (TRUE) {
		NextWord(m, pos);
		if ((m)->GetLength() == 0)
			break;	/* no more words */
		len ++;
	}
	popValue(NSPstore);
	pushInt(len);
	return;

/* _________________________
  Boolean
*/

/* ABBREVX(information,info)
returns True if info is equal to the leading characters of information
  ABBREVX("Print", "Pri")       ->    1
  ABBREVX("PRINT", "Pri")       ->    0
  ABBREVX("PRINT", "PRY")       ->    0
  ABBREVX("PRINT", "")          ->    1
*/
case 'W':
	m = FETCHMARK(NSPstore);	/* the abbrev */
	m2 = FETCHMARK((union stackelement *)&(&NSPstore->s)[1]); /* subject */
	stxt = (m)->GetText();
	stxt2 = (m2)->GetText();
	len = (m)->GetLength();
	pos = (m)->GetPos();
	slen = (m2)->GetPos();	/* slen is really a position */
	if (len > (m2)->GetLength())
		t = 0;
	else if (len == 0)
		t = 1;	/* by convention, "" is an abbreviation of anything */
	else for (t = 0; t < len; t++)
		if (
			(stxt)->GetUnsignedChar( pos+t)
		   !=
			(stxt2)->GetUnsignedChar( slen+t)
		   ) {
			t = 0;
			break;
		}
	popValue(NSPstore);
	popValue(NSPstore);
	if (t == 0)
		pushBool(FALSE);
	else pushBool(TRUE);
	return;


	}  /* end switch(op) */

	RunError(":unimplemented function", IAR);

	return;
}

