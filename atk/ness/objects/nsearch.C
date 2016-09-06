#define NESS_INTERNALS
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1993 - All Rights Reserved
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

/* 
	search.c  -  implement searches and pattern matches

	Entry points:

	SearchOp(code, iar) - does the search indicated by code
 */

/*
 * $Log: nsearch.C,v $
 * Revision 1.10  1995/04/05  01:48:15  rr2b
 * don't export internally used inline functions to ness client code
 *
 * Revision 1.9  1994/08/13  15:50:18  rr2b
 * new fiasco
 *
 * Revision 1.8  1994/08/12  20:43:57  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.7  1994/04/16  21:48:41  rr2b
 * Updated for the demise of the parse class and the move of ParseNumber to the parser
 * class.
 * BUG
 *
 * Revision 1.6  1994/03/21  17:29:05  rr2b
 * bcmp -> memcmp
 * BUILD
 *
 * Revision 1.5  1994/03/21  17:00:38  rr2b
 * bzero->memset
 * bcopy->memmove
 * index->strchr
 * rindex->strrchr
 * some mktemp->tmpnam
 *
 * Revision 1.4  1993/09/23  21:13:02  wjh
 * fixed regsearchreverse
 *
 * Revision 1.3  1993/06/29  18:06:33  wjh
 * checkin to force update
 *
 * Revision 1.2  1993/06/28  13:50:16  rr2b
 * Made functions which don't need to be exported static.
 * Renamed search() to fsearch() to avoid a name clash with the class
 * search.
 *
 * Revision 1.1  1993/06/21  20:43:31  wjh
 * Initial revision
 *
 * Revision 1.34  1993/02/17  06:10:54  wjh
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
 * Revision 1.33  1992/12/15  21:38:20  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.32  1992/12/15  00:53:01  rr2b
 * fixed disclaimerization
 *
 * Revision 1.31  1992/12/14  20:50:00  rr2b
 * disclaimerization
 *
 * Revision 1.30  1992/12/14  03:02:50  wjh
 * fix search failure where the target spans the gap
 *
 * .
 *
 * Revision 1.29  1992/11/26  02:42:25  wjh
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
 * Revision 1.28  92/11/15  13:39:42  wjh
 * fix the DeltaSearch algorithm to do subscripting with unsigned chars
 * this fixes the problem in dict.n and probably xmas.d as well
 * 
	log elided 6/92   -wjh
 * 
*/

#include <andrewos.h>	/* for bzero() bcmp() */
#include <ctype.h>
#include <dataobject.H>
#include <text.H>
#include <simpletext.H>
#include <textview.H>
#include <viewref.H>
#include <dictionary.H>
#include <environment.H>
#include <stylesheet.H>
#include <style.H>
#include <cel.H>
#include <search.H>

#include <parser.H>
#include <nessmark.H>
#include <ness.H>


/* XXX  KLUDGE.  The following definition should be exported by text.ch */
#define TEXT_VIEWREFCHAR  '\377'

#define PTSTOMARK(arg,iar)  ((TType)((class nessmark *)arg)->header.nessmark_methods \
		== nessmarkHdr) ? TRUE :   \
		RunError(":not a pointer to a mark (uninitialized variable?)", iar);


/* boo hiss Sun chars */
#define UNSIGN(c) ((c) & 0xFF)

/* to get around the fact that GetParent has type (class nestedmark *) */
	static inline class environment *
EnvGetParent(class environment *env) {
	return (class environment *)(env)->GetParent();
}
/* to get around the fact that GetInnerMost has type (class nestedmark *) */
	static inline class environment *
EnvGetInnerMost(class environment *env, long pos) {
	return (class environment *)(env)->GetInnerMost(pos);
}


/* The list of styles created with DefineStyle is maintained 
	so it can be used for addstylebyname */
static struct stylist {
	class style *s;
	struct stylist *next;
} *DefinedStyles = NULL;


/* conventions for search-type functions:
	the subject in which the search is made is the first argument
	if this is an empty string, the search extends to the end of the base
		otherwise the search extends only over the argument string
	For success the return value is the non-empty substring matching the 
		criteria.
	For failure, the return value is the empty string at the end of the 
		original subject string.  (If the original subject were empty
		the return value for failure is that original subject.)
*/


/* the string location of most recent ParseInt, ParseReal, or FirstObject */
static class nessmark *WhereItWas = NULL;


static boolean CharTable[256];	/* assumed initially zero */
	/* the CharTable is used to test for characters in sets */


static void SetCharTable(class nessmark  *pat);
static void ClearCharTable(class nessmark  *pat);
static boolean BufMatch(long  offset);
static long SimpleSearch();
static long DeltaSearch();
static void fsearch(class nessmark  *subject , class nessmark  *pat);
static void match(class nessmark  *subject , class nessmark  *pat);
static void anyof(class nessmark  *subject , class nessmark  *pat);
static void span(class nessmark  *subject , class nessmark  *pat);
static void token(class nessmark  *subject , class nessmark  *pat);
static void AddOneStyle(class text  *text, long  pos , long  len, class style  *style);
static void AddStyles(class text  *text, long  pos , long  len, class environment  *env);
static boolean HasStyle(class environment  *env , class environment  *penv);
static boolean HasAllStyles(class environment  *env , class environment  *penv);
static void AdjustOperandValue(struct marginstyle  *s , struct marginstyle  *p, 
		boolean  isConst);
static class style * MergeStyles(class style  *style, class environment  *env);
static boolean  DeleteNonTemplateStyles(class style  *style, class stylesheet  *ss);
void SearchOp(unsigned char op, unsigned char *opiar /* iar of the opcode */);



/* SetCharTable(pat)
	Set to TRUE each element of CharTable that is in pat.
*/
	static void
SetCharTable(class nessmark  *pat) {
	class simpletext *ptext;
	long pos, end;
	ptext = (pat)->GetText();
	pos = (pat)->GetPos();
	end = pos + (pat)->GetLength();
	for ( ; pos < end; pos++)
		CharTable[(ptext)->GetUnsignedChar( pos)] = TRUE;
}

/* ClearCharTable(pat)
	Set to FALSE each element of CharTable.
	As an optimization, the 'pat' arg must list all elts of table which are TRUE.
*/
	static void
ClearCharTable(class nessmark  *pat) {
	class simpletext *ptext;
	long pos, end, len;
	len = (pat)->GetLength();
	if (len > 20) {
		memset(CharTable, 0, sizeof(CharTable));
		return;
	}
	ptext = (pat)->GetText();
	pos = (pat)->GetPos();
	end = pos + len;
	for ( ; pos < end; pos++)
		CharTable[(ptext)->GetUnsignedChar( pos)] = FALSE;
}


/* buffers global to the search algorithms */
static char *patBuf, *bufA, *bufB;
static long patlen, bufAlen, bufBlen;

/* BufMatch(offset)
	check the concatenation of
	bufA (length bufAlen) and bufB (length bufBlen)
	to see if patBuf (length patlen) matches the portion beginning
	at 'offset'
	return TRUE for a match and FALSE otherwise
 */
	static boolean
BufMatch(long  offset) {
	long Apart;	/* how much is in A */
	if (offset >= bufAlen) 
		/* entirely in B */
		return (memcmp(patBuf, bufB + offset - bufAlen, patlen) == 0);
	else if (offset + patlen > bufAlen) {
		/* spans the boundary */
		Apart = bufAlen - offset;
		return (memcmp(patBuf, bufA + offset, Apart) == 0
			&& memcmp(patBuf+Apart, bufB, patlen - Apart) == 0);
	}
	else {
		/* entirely in A */
		boolean val = (memcmp(patBuf, bufA+offset, patlen) == 0);
		return val;
	}
}


/* SimpleSearch()
	look for patBuf (length patlen) in the concatenation of
	bufA (length bufAlen) and bufB (length bufBlen)
	return the offset of the match from the start of bufA
	return -1 if not found

	This code is the same as Delta search except:
	a) It does not declare or use the delta array, and
	b) The increment in the final loops is 1.
*/
	static long
SimpleSearch() {
	char *cx, *curend,  target;
	long i, toff;
	
	/* compute target character:  first low probability character in pat
		the high probability characters are deemed to be
		space, newline, e, t, a, i, o, n, r, s, l, c  
		(only about 6% of all short words consist entirely of these characters) */
	curend = patBuf + patlen;
	target = *(curend-1);	/* use last char of pat if none other */
	toff = patlen - 1;
	for (cx = patBuf; cx < curend; cx++) 
		switch(*cx) {
		case ' ': case '\n': case 'e': case 't': case 'a': case 'i': 
		case 'o': case 'n': case 'r': case 's': case 'l': case 'c':
			break;
		default: 
			target = *cx;
			toff = cx - patBuf;
			cx = curend;	/* break the for-loop */
			break;		/* break the switch */
		}

	/* the value of cx in the algorithm refers to the position
		scanned for the target character.  The pattern is
		being matched against position cx-toff */

	/* search starting in bufA */
	cx = bufA + toff;
	curend = bufA + bufAlen;
	if (bufBlen < patlen-toff)
		curend -= patlen-toff-bufBlen - 1;
	while (cx < curend) {
		if (target == *cx) {
			i = cx - bufA - toff;
			if (BufMatch(i))
				return i;
		}
		cx ++;
	}
	if (bufBlen == 0) return -1;
	/* search starting in bufB  (continue cx from above) */
	cx = bufB + (cx-bufA) - bufAlen;
	curend = bufB + bufBlen - patlen + 1 + toff;
	while (cx < curend) {
		if (target == *cx) {
			i =  cx - bufB + bufAlen - toff;
			if (BufMatch(i))
				return i;
		}
		cx ++;
	}
	/* not found */
	return -1;
}


static short freq[256] = {
	/* ctl chars */	9,9,9,9,9,9,9,9,9,29,9,9,9,35,9,9,
	/* ctl chars */	9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
	/* punctuation */	950,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,
	/* digits */		30,30,30,30,30,30,30,30,30,30,30,30,30,30,30,30, 
	/* uppercase */	11,88,82,84,83,91,81,82,82,87,12,81,85,83,86,86,
	/* uppercase */	23,12,87,85,87,83,81,81,8,82,12,11,11,11,11,11,
	/* lowercase */	11,889,823,845,832,911,815,824,829,878,82,811,855,832,868,869,
	/* lowercase */	431,82,874,856,871,836,810,811,83,820,82,11,11,11,11,11,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
};


/* DeltaSearch()
	look for patBuf (length patlen) in the concatenation of
	bufA (length bufAlen) and bufB (length bufBlen)
	return the offset of the match from the start of bufA
	return -1 if not found
	Use a fast delta search algorithm
*/
	static long
DeltaSearch() {
	long delta[256];
	char *cx, *curend, target;
	long i, toff;

	/* for each character of pattern, compute a delta value
		such that if the pattern is aligned at cx-toff
		it should next align at
		cx-toff+delta[*(cx+patlen)]
		If there are multiple instances of a character, 
		use the rightmost because it advances cx the least
		During the scan, find as target the character
		with lowest frequency.
	*/
	toff = patlen + 1;	/* delta for chars not in pat  (toff is just a convenient register) */
	for (i = 0; i < 64; i++)
		delta[i] = toff;
	memmove(&delta[64], &delta[0], 64*sizeof(long));
	memmove(&delta[128], &delta[0], 128*sizeof(long));

	curend = patBuf + patlen;
	target = *(curend-1);	/* use last char of pat if none other */
	toff = patlen - 1;
	i = freq[UNSIGN(target)];

	for (cx = patBuf; cx < curend; cx++) {
		delta[UNSIGN(*cx)] = curend - cx;
		if (freq[UNSIGN(*cx)] < i) {
			target = *cx;
			toff = cx - patBuf;
			i = freq[UNSIGN(target)];
		}
	}

	/* the value of cx in the algorithm refers to the position
		scanned for the target character.  The pattern is
		being matched against position cx-toff */

	/* search starting in bufA */
	cx = bufA + toff;
		/* terminate early so *(cx-toff+patlen) is valid */
	curend = bufA + bufAlen - patlen + toff;
	while (cx < curend) {
		if (target == *cx) {
			i = cx - toff - bufA;
			if (BufMatch(i))
				return i;
		}
		/* advance according to delta for the char 
		   next beyond where we are currently matching the pattern */
		cx += delta[UNSIGN(*(cx-toff+patlen))]; 
	}

	/* search in the last little bit of bufA  (continue cx) */
	curend = bufA + bufAlen; /* one past last possible starting place */
	if (bufBlen < patlen-toff)
		curend -= patlen-toff-bufBlen - 1;
	while (cx < curend) {
		if (target == *cx) {
			i = cx - bufA - toff;
			if (BufMatch(i))
				return i;
		}
		cx ++;
	}

	/* search starting in bufB */
	if (bufBlen == 0) return -1;
	cx = bufB;
	curend = bufB + bufBlen - patlen + 1 + toff;
	while (cx < curend) {
		if (target == *cx) {
			i =  cx - toff - bufB + bufAlen;
			if (BufMatch(i))
				return i;
		}
		cx += delta[UNSIGN(*(cx-toff+patlen))];
	}
	/* not found */
	return -1;
}

	static void
fsearch(class nessmark  *subject , class nessmark  *pat) {
	class simpletext *stext, *ptext;
	long patpos, spos, slen, failpos; /* (patlen is global) */
	boolean needToFreePat;
	long tlen;

	ptext = (pat)->GetText();
	patpos = (pat)->GetPos();
	patlen = (pat)->GetLength();

	patBuf = (ptext)->GetBuf( patpos, patlen, &tlen);
	if (tlen < patlen) {
		/* crosses boundary, make a copy of pattern */
		bufA = patBuf;
		patBuf = (char *)malloc(patlen);
		strncpy(patBuf, bufA, tlen);
		bufB = (ptext)->GetBuf( patpos+tlen, patlen-tlen, &bufBlen);
		if (bufBlen < patlen - tlen) 
			fprintf(stderr, "ness search: pattern buffer failure\n");
		strncpy(patBuf+tlen, bufB, patlen - tlen);
		needToFreePat = TRUE;
	}
	else
		needToFreePat = FALSE;

	stext = (subject)->GetText();
	spos = (subject)->GetPos();
	slen = (subject)->GetLength();
	failpos = spos + slen;
	if (patlen == 0) 
		slen = 0;	/* don't search */
	else if (slen == 0) 
		slen = (stext)->GetLength() - spos;

	bufA = (stext)->GetBuf( spos, slen, &bufAlen);
	if (bufAlen < slen) {
		/* we need the other buffer, too */
		bufB = (stext)->GetBuf( spos+bufAlen, 
				slen-bufAlen, &bufBlen);
		if (bufAlen + bufBlen < slen)
			fprintf(stderr, "Ness search: subject buffer failure\n");
	}
	else bufBlen = 0;

	if (slen == 0)
		patpos = -1;
	else if (slen < 200 || patlen == 1) 
		/* don't build table, use a simple sequential search */
		patpos = SimpleSearch();
	else 
		/* use delta search with table */
		patpos = DeltaSearch();
	if (patpos < 0) {
		/* fail: return empty mark at end of subject */
		(subject)->SetPos( failpos);
		(subject)->SetLength( 0);
	}
	else {
		/* bingo */
		(subject)->SetPos( spos + patpos);
		(subject)->SetLength( patlen);
	}
	if (needToFreePat) free(patBuf);
}


/* match(subject, pat)
	checks subject to see if it begins with pat.  
	If so, returns that substring;  otherwise returns finish(subject)
	The match will fail unless the subject is of length zero or some 
		length longer than the pattern.
*/
	static void
match(class nessmark  *subject , class nessmark  *pat) {
	class simpletext *stext, *ptext;
	long pos, end, patpos, patend;
	long spos, slen;

	stext = (subject)->GetText();
	spos = (subject)->GetPos();
	slen = (subject)->GetLength();
	end = (slen == 0) ? (stext)->GetLength() : spos+slen;
	ptext = (pat)->GetText();
	patpos = (pat)->GetPos();
	patend = patpos + (pat)->GetLength();

	pos = spos;
	if (end-pos >= patend-patpos)
		while (patpos < patend 
				&& (stext)->GetUnsignedChar( pos)
				== (ptext)->GetUnsignedChar( patpos))
			pos++, patpos++;
	if (patpos == patend && (pat)->GetLength() > 0) {
		/* succeed */
		(subject)->SetLength( (pat)->GetLength());
	}
	else {
		/* fail */
		(subject)->SetPos( spos + slen);
		(subject)->SetLength( 0);
	}
}

	static void
anyof(class nessmark  *subject , class nessmark  *pat) {
	class simpletext *stext;
	long pos, end;
	long slen, spos;

	stext = (subject)->GetText();
	spos = (subject)->GetPos();
	slen = (subject)->GetLength();
	end = (slen == 0) ? (stext)->GetLength() : spos + slen;

	SetCharTable(pat);

	for (pos = spos; pos < end; pos++)
		if (CharTable[(stext)->GetUnsignedChar( pos)]) 
			break;
	if (pos < end) {
		/* bingo */
		(subject)->SetPos( pos);
		(subject)->SetLength( 1);
	}
	else {
		/* nope */
		(subject)->SetPos( spos + slen);
		(subject)->SetLength( 0);
	}

	ClearCharTable(pat);
}

	static void
span(class nessmark  *subject , class nessmark  *pat) {
	class simpletext *stext;
	long pos, end;
	long spos, slen;

	stext = (subject)->GetText();
	spos = (subject)->GetPos();
	slen = (subject)->GetLength();
	end = (slen == 0) ? (stext)->GetLength() : spos + slen;

	SetCharTable(pat);

	for (pos = spos; pos < end; pos++)
		if ( ! CharTable[(stext)->GetUnsignedChar( pos)]) 
			break;

	if (pos > spos)
		(subject)->SetLength( pos - spos);
	else {
		(subject)->SetPos( spos + slen);
		(subject)->SetLength( 0);
	}
	
	ClearCharTable(pat);
}

	static void
token(class nessmark  *subject , class nessmark  *pat) {
	class simpletext *stext;
	long slen, spos;
	stext = (subject)->GetText();
	spos = (subject)->GetPos();
	slen = (subject)->GetLength();

	anyof(subject, pat);
	if ((subject)->GetLength() == 0)
		return;

	/* revise subject to extend to the end of the base 
			or the end of the original subject */
	(subject)->SetLength( 
		((slen == 0) ? (stext)->GetLength() : spos + slen)
			- (subject)->GetPos());
	span(subject, pat);
}


/* AddOneStyle(text, pos, len, style)
	Adds to text at pos for length len teh given style.
*/
	static void
AddOneStyle(class text  *text, long  pos , long  len, class style  *stylep) {
	class style *newstyle;
	if (stylep == NULL) return;
	newstyle = (text->styleSheet)->Find( (stylep)->GetName());
	if (newstyle == NULL)  {
		/* XXX Why doesn't text_AddStyle do this ??? */
		/* XXX How do we decide we should use default.template */
		newstyle = new style;
		(stylep)->Copy( newstyle);	
		newstyle->template_c = FALSE;
		(text->styleSheet)->Add( newstyle); 
	}
	(text)->AddStyle( pos, len, newstyle);
}

/* AddStyles(text, pos, len, env)
	Adds to text at pos for length len all the styles had by env,
	except not those from environments that are already shared by both.
*/
	static void
AddStyles(class text  *text, long  pos , long  len, class environment  *env) {
	class environment *subjenv, *tenv;
	boolean notify = FALSE;

	subjenv = (class environment *)(text->rootEnvironment)->
					GetEnclosing( pos);
	if (len > 0)
		subjenv = (class environment *)(subjenv)->GetCommonParent(
			(class environment *)(text->rootEnvironment)->
					GetEnclosing( pos+len-1));
	for (tenv = env; tenv != NULL; tenv = EnvGetParent(tenv)) {
		if (tenv->type != environment_Style) continue;
		/* if tenv is (identically) subjenv or an ancestor, break */
		if ((subjenv)->Distance( tenv) >= 0) break;
		AddOneStyle(text, pos, len, tenv->data.style);
		notify = TRUE;
	}
	if (notify) (text)->NotifyObservers( 0);
}


/* HasStyle(env, penv)
	checks to see if env or any of its parents has the style penv 
	Returns TRUE if so and FALSE if not
*/
	static boolean
HasStyle(class environment  *env , class environment  *penv) {
	const char *pname, *name;
	enum nametype {None, Name, Menu} nt = None;
	if (penv->data.style == NULL) return TRUE;
	pname = (penv->data.style)->GetName();
	if (pname != NULL) nt = Name;
	else {
		pname = (penv->data.style)->GetMenuName();
		if (pname != NULL) nt = Menu;
	}

	for ( ; env != NULL; env = EnvGetParent(env))  {
		if (env->type != environment_Style || env->data.style == NULL) 
			continue;
		if (env->data.style == penv->data.style)
			return TRUE;
		switch (nt) {
		    case None:  name = NULL;   break;
		    case Name:  name = (env->data.style)->GetName();   break;
		    case Menu:  name = (env->data.style)->GetMenuName();   break;
		}
		if (name != NULL && strcmp(name, pname) == 0)
			return TRUE;
		/* XXX could go on to test style attributes YUCHH */
	}
	return FALSE;   /* not found */
}

/* HasAllStyles(env, penv)
	checks to see if env has all styles that are around penv
	return bolean value
	return TRUE unless we find a style on penv that is not on env
*/
	static boolean
HasAllStyles(class environment  *env , class environment  *penv) {
	for ( ; penv != NULL;  penv = EnvGetParent(penv))
		if (penv->type == environment_Style && ! HasStyle(env, penv)) 
			/*  could not find a pat style */
			return FALSE;
	return TRUE;
}


/* AdjustOperandValue(s, p, isConst)
	KLUDGE
	s and p point to struct marginstyle or struct fontscriptstyle, BOTH
		OF WHICH HAVE THE SAME FORMAT (right now)
	The value of s is adjusted per that of p.
	If isConst is TRUE, the value of s is set to that of p,
	otherwise {the value of s is incremented by that of p,
		the Units are reset to RawDots,
		and the DotCvtOperand is used as the Operand value.}

	It would be preferable to do this is a cleaner fashion, 
	but there is no good way to add Operands if Units differ.
*/
	static void 
AdjustOperandValue(struct marginstyle  *s , struct marginstyle  *p, boolean  isConst) {
	if (isConst) {
		/* just change to the size set by p */
		*s = *p;
	}
	else if (p->DotCvtOperand != 0) {
		/* p makes a relative change to s */
		s->DotCvtOperand += p->DotCvtOperand;
		s->MarginUnit = style_RawDots;
		s->Operand = s->DotCvtOperand;
	}
}


/* MergeStyles(style, env)
	merges all styles from the env and its parents into style
	returns style as modified
	Recurses to merge parent styles first so pat itself will override its parents
		This would be better if style.c had a method to apply
		one style to another.
		As it is, we process only some of the attributes.  
		The style attributes  
			LeftMargin,   RightMargin,  Indent,  Script, and FontSize
		are treated incrementally.  The attributes 
			Justification,  Flags,  FontFamily,  FontFace,
			Line Spacing, Paragraph Spacing, tabs, and color
		are treated as new values.
*/
	static class style *
MergeStyles(class style  *stylep, class environment  *env) {
	class environment *penv;
	class style *patstyle;

	enum style_FontSize SizeBasis;
	long Operand, newOperand;
	enum style_Justification Justification;
	long mask;

	enum style_Unit Unit;
	enum style_SpacingValue Basis;
	char *color;
	struct tabentry **tabchanges;
	long n;

	penv = EnvGetParent(env);
	if (penv != NULL)
		/* recur to do parent styles */
		stylep = MergeStyles(stylep, penv);

	patstyle = env->data.style;
	if (env->type != environment_Style  ||  patstyle == NULL) 
		return stylep;

	/* LeftMargin */
		AdjustOperandValue(&stylep->NewLeftMargin,
			&patstyle->NewLeftMargin,
			patstyle->NewLeftMargin.MarginBasis ==
				style_ConstantMargin);
	/* RightMargin */
		AdjustOperandValue(&stylep->NewRightMargin,
			&patstyle->NewRightMargin,
			patstyle->NewRightMargin.MarginBasis ==
				style_ConstantMargin);
	/* Indent */
		AdjustOperandValue(&stylep->NewIndentation,
			&patstyle->NewIndentation,
			patstyle->NewIndentation.MarginBasis ==
				style_ConstantMargin);
	/* Script */
		AdjustOperandValue((struct marginstyle *)&stylep->FontScript,
			(struct marginstyle *)&patstyle->FontScript,
			patstyle->FontScript.ScriptBasis ==
				style_ConstantScriptMovement);
	/* FontSize  */
		(patstyle)->GetFontSize( &SizeBasis, &Operand);
		if (SizeBasis == style_ConstantFontSize)
			(stylep)->SetFontSize( SizeBasis, Operand);
		else if (SizeBasis == style_PreviousFontSize) {
			(stylep)->GetFontSize( &SizeBasis, &newOperand);
			(stylep)->SetFontSize( SizeBasis,
				newOperand+Operand);
		}
	/* FontFamily */
		if (patstyle->FontFamily != NULL)
			(stylep)->SetFontFamily( patstyle->FontFamily);
	/* Justification */
		Justification = (patstyle)->GetJustification();
		if (Justification != style_PreviousJustification)
			(stylep)->SetJustification( Justification);
	/* Flags */
		mask = patstyle->AddMiscFlags  |  ~ patstyle->OutMiscFlags;
		stylep->AddMiscFlags = 
			(mask & patstyle->AddMiscFlags)
			| ((~mask) & stylep->AddMiscFlags);
		stylep->OutMiscFlags = 
			(mask & patstyle->OutMiscFlags)
			| ((~mask) & stylep->OutMiscFlags);
	/* FontFace */
		mask = patstyle->AddFontFaces  |  ~ patstyle->OutFontFaces;
		stylep->AddFontFaces = 
			(mask & patstyle->AddFontFaces)
			| ((~mask) & stylep->AddFontFaces);
		stylep->OutFontFaces = 
			(mask & patstyle->OutFontFaces)
			| ((~mask) & stylep->OutFontFaces);
	/* Spacing (of lines) */
		(stylep)->GetNewInterlineSpacing( &Basis, &Operand, &Unit);
		if (Basis == style_ConstantSpacing && Unit == style_Points)
			(stylep)->SetNewInterlineSpacing( Basis, Operand, Unit);
	/* Spread (between paragraphs) */
		(stylep)->GetNewInterparagraphSpacing( &Basis, &Operand, &Unit);
		if (Basis == style_ConstantSpacing && Unit == style_Points)
			(stylep)->SetNewInterparagraphSpacing( Basis, Operand, Unit);
	/* Tabs */
		(patstyle)->GetTabChangeList( &n, &tabchanges);
		if (n != 0) {
			(stylep)->ClearTabChanges();
			while (n-- > 0) 
				(stylep)->AddTabChange(
					tabchanges[n]->TabOpcode,
					tabchanges[n]->Location,
					tabchanges[n]->LocationUnit);
		}
		if (tabchanges)
			free(tabchanges);
	/* Color */
		color = (patstyle)->GetAttribute( "color");
		if (color != NULL) 
			(stylep)->AddAttribute( "color", color);
	return stylep;
}

	static boolean 
DeleteNonTemplateStyles(class style  *stylep, class stylesheet  *ss) {
	if ( ! stylep->template_c) 
		/* not in template, delete it */
		(ss)->Delete( stylep);
	return FALSE;
}

/* SearchOp(op, NSP, opiar)
	First five ops perform searches, depending on op.
	Each search routine modifies its first argument marker to indicate the
		result of searching for the second.
	Other operations parse strings, handle objects, and process styles.
*/
	void
SearchOp(unsigned char op, unsigned char *opiar	/* iar of the opcode */) {
	union stackelement *NSP = NSPstore; 

	/* these definitions are global because GDB can't get at local decls. */

	class nessmark *pat = NULL, *subject;
	boolean boolval;
	long intval;
	double realval;
	const char *cxc;
	char *cstring, *cx;
	ATK  *objval;
	long pos, len, finish;
	class text *textp, *pattext;
	class viewref *vr;
	long success;
	long envpos, envlen;
	class environment *env, *penv, *tenv;
	class style *stylep;
	struct stylist *stelt;
	char menuname[100];
	static struct SearchPattern *regPat;

	if (WhereItWas == NULL)
		WhereItWas = new nessmark;

	/* check arguments */
	if (op == 'w')	{}		/* no args */
	else {
		/* at least one arg */
		subject = FETCHMARK(NSP);   /* fetch last arg */
		if (op < 'p') {
			/* two marker args */
			pat = subject;	/* 2nd arg is pattern */
			subject = FETCHMARK((union stackelement *)
					&(&(NSP->s))[1]);
		}
	}
	switch (op) {

	case 'a':  fsearch(subject, pat);  NSP = popValue(NSP);  break;
	case 'b':  match(subject, pat);  NSP = popValue(NSP);  break;
	case 'c':  anyof(subject, pat);  NSP = popValue(NSP);  break;
	case 'd':  span(subject, pat);  NSP = popValue(NSP);  break;
	case 'e':  token(subject, pat);  NSP = popValue(NSP);  break;

	/* {"regSearch", "Ff", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange} */
	case 'f':
		/* forward regular search */
		textp = (class text *)(subject)->GetText();
		pos = (subject)->GetPos();
		len = (subject)->GetLength();
		finish = pos + len;  /* failure location */
		cstring = (char *)(pat)->ToC();
		cxc = search::CompilePattern(cstring, &regPat);
		free(cstring);
		/* XXX save pat's cstring and compiled pattern
		test and avoid recompilation (? will this really be faster) */

		if (cxc != 0) {
			/* not a valid pattern */
			fprintf(stderr, "Ness regSearch: %s\n", cxc);
			(subject)->MakeConst("");  /* EmptyText */
		}
		else {
			if (len != 0) {
				/* XXX HACK to delimit the search to the
					argument */
				envlen = (textp)->GetLength();
				((class simpletext *)textp)->length = finish;
				pos = search::MatchPattern(textp, pos, regPat);
				((class simpletext *)textp)->length = envlen;
			}
			else  
				pos = search::MatchPattern(textp, pos, regPat);
				
			if (pos >= 0) {
				/* succeed */
				(subject)->SetPos( pos);
				(subject)->SetLength(
					search::GetMatchLength());
			}
			else {
				/* fail */
				(subject)->SetPos( finish);
				(subject)->SetLength( 0);
			}
		}

		NSP = popValue(NSP);  /* discard pat, leaving revised subject as result */
		break;

	/* {"regSearchReverse", "Fg", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange}, */
	case 'g':
		/* reverse regular search */
		textp = (class text *)(subject)->GetText();
		pos = (subject)->GetPos();
		len = (subject)->GetLength();
		finish = pos + len;  /* failure location */
		cstring = (char *)(pat)->ToC();
		cxc = search::CompilePattern(cstring, &regPat);
		free(cstring);
		/* XXX save cstring and test to avoid recompilation (? is this really faster) */
		if (cxc != 0) {
			/* not a valid pattern */
			fprintf(stderr, "Ness regSearchReverse: %s\n", cxc);
			(subject)->MakeConst("");   /* EmptyText */
		}
		else {
			envpos = search::MatchPatternReverse(textp, finish, regPat);
				
			if (envpos >= 0 && (len == 0 || envpos >= pos)) {
				/* succeed */
				(subject)->SetPos(envpos);
				(subject)->SetLength(
					search::GetMatchLength());
			}
			else {
				/* fail */
				(subject)->SetPos( finish);
				(subject)->SetLength( 0);
			}
		}


		NSP = popValue(NSP);  /* discard pat, leaving revised subject as result */
		break;

	/*  {"searchforstyle", "Fh", {Tstr, Tstr, Tstr, Tend}, ness_codeOrange},*/
	case 'h':
		pattext = (class text *)(pat)->GetText();
		if ( ! (pattext)->IsType( textClass))
			goto nosuchstyle;	/*  no styles in 'pat' */
		penv = EnvGetInnerMost(pattext->rootEnvironment,
				(pat)->GetPos());

		textp = (class text *)(subject)->GetText();
		if ( ! (textp)->IsType( textClass)) 
			goto nosuchstyle;	/*  subject has no styles */

		pos = (subject)->GetPos();
		finish = pos + (subject)->GetLength();
		if (finish == pos)
			finish = (textp)->GetLength();
			
		do {
			/* loop through each change position and see if 
				the required style is at that point */
			env = EnvGetInnerMost(textp->rootEnvironment, pos);
			if (HasAllStyles(env, penv))
				break;
			pos += (textp->rootEnvironment)->GetNextChange(
				pos);
			if (pos >= finish)
				goto nosuchstyle;
		}  while ( TRUE);
		(subject)->SetPos( pos);	/* the start of the style */

		do {
			/* continue looping through each change position 
				until we find a place that doesn't have the style */
			pos += (textp->rootEnvironment)->GetNextChange(
				pos);
			if (pos >= finish) {
				/* style extends beyond finish */
				pos = finish;
				break;
			}
			env = EnvGetInnerMost(textp->rootEnvironment, pos);
		}  while (HasAllStyles(env, penv));
		(subject)->SetLength( pos - (subject)->GetPos());

		/* succeed.  The Pos and Length of subject have been set */
		NSP = popValue(NSP);	/* discard style marker */
		break;

	nosuchstyle:
		/* fail, return finish(subject) */
		(subject)->Next();
		(subject)->Start();
		NSP = popValue(NSP);	/* discard style marker */
		break;

	/*  {"definestyle", "zFi", {Tvoid, Tstr, Tstr, Tend}, ness_codeOrange},*/
	/* (The arguments are swapped by the z operator.)
	    'pat' is style name.  'subject' is text with desired style set 
	    return value is a copy of the stylename text, but with the defined style
	*/
	case 'i':
		/* create a style and merge into it the styles from the source */
		stylep = new style;
		textp = (class text *)(subject)->GetText();
		if ((textp)->IsType( textClass)) {
			env = EnvGetInnerMost(textp->rootEnvironment, 
					(subject)->GetPos());
			stylep = MergeStyles(stylep, env);
		}

		cstring = (char *)(pat)->ToC();	/* style name basis */

		/* set menuname and cleaned up name 
			if name has a comma, use name as menuname
				and then discard the part before the comma
			otherwise construct a name putting it on "Style" menu card
			the name itself is cleaned by terminating at
				first non-alphameric and
				changing uppercase to lower 
		*/
		if (strlen(cstring) > 80)
			cstring[80] = '\0';	/* truncate to 80 characters */
		cx = strchr(cstring, ',');
		if (cx != NULL) {
			(stylep)->SetMenuName( cstring);
			strcpy(cstring, cx+1);		/* discard prefix */
		}
		else {
			if (islower(*cstring)) *cstring = toupper(*cstring);
			sprintf(menuname, "Style~8,%s", cstring);
			(stylep)->SetMenuName( menuname);
		}
		
		len = 0;
		for (cx = cstring; *cx != '\0'; cx++) {
			if ( ! isalnum(*cx))    {*cx = '\0'; break;}
			len++;
			if (isupper(*cx))   *cx = tolower(*cx);
		}
		(stylep)->SetName( cstring);

		/* Add to DefinedStyles list */
		for (stelt = DefinedStyles; stelt != NULL; stelt = stelt->next)
			if (strcmp((stelt->s)->GetName(), cstring) == 0) 
				break;
		if (stelt == NULL) {
			/* create a new element in DefinedStyles list */
			stelt = (struct stylist *)malloc(sizeof(struct stylist));
			stelt->next = DefinedStyles;
			DefinedStyles = stelt;
		}
		stelt->s = stylep;
			/* CORELEAK:  styles are never discarded (can't be) */

		/* replace subject marker with one for result */
		/* give the new style to the result */
		textp = new text;
		(textp)->InsertCharacters( 0, cstring, len);
		AddOneStyle(textp, 0, len, stylep);
		(subject)->Set( textp, 0, len);

		free(cstring);
		NSP = popValue(NSP);  	/* discard topmost arg  (i.e., pat) */

		break;

	/* {"addstyles", "Fj", {Tstr, Tstr, Tstr, Tvoid}, ness_codeYellow} */
	case 'j':
		/* revise subject to have style of pat */
		textp = (class text *)(subject)->GetText();
		if ((textp)->GetReadOnly())
			RunError(":Tried to change styles on a constant", opiar);
		pattext = (class text *)(pat)->GetText(); 		
		if ((pattext)->IsType( textClass) 
				&& (textp)->IsType( textClass))
			AddStyles(textp, (subject)->GetPos(),
				(subject)->GetLength(),	
				EnvGetInnerMost(pattext->rootEnvironment,
						(pat)->GetPos()));
		NSP = popValue(NSP);  /* discard pat, leaving revised subject as result */
		break;

	/* {"hasstyles", "Fk", {Tbool,Tstr,Tstr,Tvoid}, ness_codeOrange} */
	case 'k':
		/* set boolval indicating whether subject has style of pat */
		boolval = TRUE;

		pattext = (class text *)(pat)->GetText();
		if ( ! (pattext)->IsType( textClass))
			goto gotval;	/*  no styles: trivially TRUE */
		pos = (pat)->GetPos();
		penv = EnvGetInnerMost(pattext->rootEnvironment, pos);
		while (penv != NULL && penv->type != environment_Style)
			penv = EnvGetParent(penv);
		if (penv == NULL)
			goto gotval;	/*  pat has no styles: trivially TRUE */

		/* now we know pat has at least one style */
		textp = (class text *)(subject)->GetText();
		if ( ! (textp)->IsType( textClass)) {
			boolval = FALSE;
			goto gotval;	/*  text is simple: trivially FALSE */
		}
		pos = (subject)->GetPos();
		env = EnvGetInnerMost(textp->rootEnvironment, pos);
		while (env != NULL && env->type != environment_Style)
			env = EnvGetParent(env);
		if (env == NULL) {
			boolval = FALSE;
			goto gotval;	/*  text has no styles: trivially FALSE */
		}

		/* now we know text and pattext both have at least one style 
			return TRUE unless we find a style on pat that is
			not on text */
		boolval = HasAllStyles(env, penv);

		/* found all styles of pat,  return the TRUE set far above */

	gotval:
		NSP = popValue(NSP);  
		NSP = popValue(NSP);  

		/* push boolval */
		NSPushSpace(boolstkelt); 
		NSP->b.hdr = boolHdr;
		NSP->b.v = boolval;
		break;

	/* {"addstylebyname", "Fl", {Tstr, Tstr, Tstr, Tend}, ness_codeYellow}, */
	case 'l':
		/* look up the style name 'pat' in the styles of  'subject'
			and in Default template (added to subject)
			and in styles of 'pat' 
			and in the DefinedStyles list*/

		cstring = (char *)(pat)->ToC();
		stylep = NULL;
		textp = (class text *)(subject)->GetText();
		if ((textp)->IsType( textClass)) {
			stylep = ((textp)->GetStyleSheet())->Find( cstring);
			if (stylep == NULL && textp->templateName == NULL) {
				(textp)->ReadTemplate( "default",
					(textp)->GetLength() == 0);
				stylep = ((textp)->GetStyleSheet())->Find(
					cstring);
			}
		}
		if (stylep == NULL) {
			pattext = (class text *)(pat)->GetText();
			if ((pattext)->IsType( textClass))
				stylep = ((pattext)->GetStyleSheet())->Find(
					cstring);
		}
		if (stylep == NULL) {
			for (stelt = DefinedStyles; stelt != NULL; stelt = stelt->next)
				if (strcmp((stelt->s)->GetName(), cstring) == 0) {
					stylep = stelt->s;
					break;
				}
		}
		AddOneStyle(textp, (subject)->GetPos(),
				(subject)->GetLength(), stylep);
		(textp)->NotifyObservers( 0);

		free(cstring);
		NSP = popValue(NSP);	/* discard the marker for stylename */
		break;

	/* {"parseint", "Fp", {Tlong, Tstr, Tvoid}, ness_codeOrange} */
	case 'p':
		/* set intval by scanning subject, use -2**31 for error 
			save matched portion w/ RememberWhereItWas */
		textp = (class text *)(subject)->GetText();
		pos = (subject)->GetPos();

		cstring = (textp)->GetBuf( pos, 200, &envlen);
		success = parser::ParseNumber(cstring, &len, &intval, &realval);
		if (success == 0 || len > envlen)
			intval = (1<<31);
		(WhereItWas)->Set( textp, pos, len);
		NSP = popValue(NSP);

		/* push intval */
		NSPushSpace(longstkelt); 
		NSP->l.hdr = longHdr;
		NSP->l.v = intval;
		break;
	/* {"parsereal", "Fq", {Tdbl, Tstr, Tvoid}, ness_codeOrange} */
	case 'q':
		textp = (class text *)(subject)->GetText();
		pos = (subject)->GetPos();

		/* set realval by scanning subject, use NaN for error 
			save matched portion in WhereItWas */

		cstring = (textp)->GetBuf( pos, 200, &envlen);
		success = parser::ParseNumber(cstring, &len, &intval, &realval);
		if (success == 0 || len > envlen)
			realval = -999.0;
		else if (success == 1)
			realval = intval;
		(WhereItWas)->Set( textp, pos, len);
		NSP = popValue(NSP);

		/* push realval */
		NSPushSpace(dblstkelt); 
		NSP->d.hdr = dblHdr;
		NSP->d.v = realval;
		break;
	/* {"firstobject", "Fr", {Tptr, Tstr, Tvoid}, ness_codeOrange} */
	case 'r':
		/* set objval by scanning subject, use NULL for error 
			save matched location w/ RememberWhereItWas */
		(WhereItWas)->MakeConst( "");	/* EmptyText */
		textp = (class text *)(subject)->GetText();
		objval = NULL;
		if ((textp)->IsType( textClass)) {
			len = (textp)->GetLength();
			objval = NULL;
			for (pos = (subject)->GetPos(); pos < len; pos++) {
				if ((textp)->GetChar( pos) 
						!= TEXT_VIEWREFCHAR)
					continue;
				vr = (textp)->FindViewreference( pos, 1);
				if (vr == NULL)  continue;

				/* try to get the view, if there is one */
				objval = (ATK  *)dictionary::LookUp(
					(class view *)textp, 
					(char *)vr);
				if (objval == NULL || objval == (ATK  *)
						textview_UNKNOWNVIEW) {
					objval =  (ATK  *)vr->dataObject;
				}
				(WhereItWas)->Set( textp, pos, 1);
				break;
			}
		}
		NSP = popValue(NSP);
		/* push objval */
		NSPushSpace(ptrstkelt); 
		NSP->p.hdr = ptrHdr;
		NSP->p.v = objval;
		break;
	/* {"nextstylegroup", "Fs", {Tstr, Tstr, Tvoid}, ness_codeOrange} */
	case 's':
		/* revise subject, advancing to next larger style group 
				or return start of arg 
		if subject is at start of style and a larger group starts at same place
			return the smallest enclosing style;  
		otherwise find the next place after start of subject where a style starts
			and return the smallest style group starting there
		A Ness program can find all styles by successive application of
			NextStyleGroup().   */
		textp = (class text *)(subject)->GetText();
		if ( ! (textp)->IsType( textClass)) {
			(subject)->Start();
			break;
		}
		pos = (subject)->GetPos();
		len = (subject)->GetLength();

		/* Part I.  If one or more style groups start where subject does, 
			find the smallest that is longer than subject.  */

		env = EnvGetInnerMost(textp->rootEnvironment, pos);
		tenv = env;
		while (tenv != NULL  &&  
				(tenv->type != environment_Style
				|| tenv->data.style == NULL
				|| tenv->length <= len)) 
			tenv = EnvGetParent(tenv);
		if (tenv != NULL &&  (tenv)->Eval() == pos) {
			/* tenv satisfies:
				a) is a style
				b) is an ancestor of subject's style
				c) starts same place as subject
				d) is longer than subject
			    so set length of subject to the length of tenv
			BUT.  the environment may have length 99999999,
			so we need to check condition (d) again
			*/
			envlen = tenv->length;
			if (pos + envlen > (textp)->GetLength())
				envlen = (textp)->GetLength() - pos;
			if (envlen > len) {
				(subject)->SetLength( envlen);
				break;	/* exit from nextstylegroup */
			}
		}

		/* Part II.  There is no larger style group that starts where subject does.
			Find the next place where a style starts and take the
			smallest group starting there.
			To do this we check each place where the style changes
			and find the innermost there.  If that innermost is a parent
			of subject, we have a style end and not a style start.  */

		envpos = pos;
		do {
			envpos += (textp->rootEnvironment)->GetNextChange(
							envpos);
			if (envpos >= (textp)->GetLength()) {
				/* we've reached the end of text */
				(subject)->Start();
				break;
			}
			tenv = EnvGetInnerMost(textp->rootEnvironment, envpos);
			while (tenv != NULL  &&  tenv->type != environment_Style) 
				tenv = EnvGetParent(tenv);
			if (tenv == NULL) 
				/* tenv is root envt 
					and thus must be a parent of env */
				continue;
			penv = env;
			while (penv != NULL && penv != tenv)
				penv = EnvGetParent(penv);
			/* if tenv == penv, the style at tenv is a parent of subject's style,
					so it has already been processed */
			if (tenv != penv) {
				/* at this point, tenv points to the environment for the 
				    next group,  and that group starts at envpos */
				(subject)->SetPos( envpos);
				envlen = tenv->length;
				len = (textp)->GetLength();
				if (pos + envlen > len)
					envlen = len - pos;
				(subject)->SetLength( envlen);
				break;
			}
		} while (TRUE);

		break;

	/* {"enclosingstylegroup","Ft", {Tstr, Tstr, Tvoid}, ness_codeOrange} */
	case 't':
		/* subject must be empty or correspond to a style group
			revise subject, advancing to next outer style group 
			or return start of arg (?)
			entire doc is not a style group unless explicitly set */
		textp = (class text *)(subject)->GetText();
		if ( ! (textp)->IsType( textClass)) {
			(subject)->Start();
			break;
		}
		pos = (subject)->GetPos();
		len = (subject)->GetLength();

		/* get an enclosing style group.*/
		env = EnvGetInnerMost(textp->rootEnvironment, pos);

		/* be sure the selected text is a style group */
		while (env != NULL  &&  len > env->length)
			env = EnvGetParent(env);
		if (env != NULL && len > 0 
				&& (pos != (env)->Eval()  
				    ||  len !=  env->length))
			env = NULL;

		/* scan further up to find enclosing style */
		while (env != NULL && 
				(len == env->length  
				|| env->type != environment_Style))
			env = EnvGetParent(env);

		/* now set the mark to the enclosing range */
		if (env != NULL) {
			(subject)->SetPos( (env)->Eval());
			pos = (subject)->GetPos();
			envlen = env->length;
			len = (textp)->GetLength();
			if (pos + envlen > len)
				envlen = len - pos;
			(subject)->SetLength( envlen);
		}
		else /* no enclosing style */
			(subject)->Start();
		break;

	/* {"clearstyles", "Fu", {Tstr, Tstr, Tvoid}, ness_codeOrange} */
	case 'u':
		/* remove all styles that include exactly the subject */
		textp = (class text *)(subject)->GetText();
		if ( ! (textp)->IsType( textClass)) break;
#if 0	
/* debugging code */	
for (pos = 0; pos <= (textp)->GetLength(); pos ++) {
	/* print values of getnextchange and getinnermost */
	envpos = (textp->rootEnvironment)->GetNextChange( pos);
	env = (textp->rootEnvironment)->GetInnerMost( pos);
	printf("%d:  + %d    %c   ", pos, envpos, 
		((class simpletext *)textp)->GetUnsignedChar( pos+envpos));
	while (env != NULL  &&  env->length < 999999999) {
		printf("    %d : %d", (env)->Eval(),
				env->length);
		if (env->type != environment_Style)
			printf("(not Style)");
		env = EnvGetParent(env);
	}
	printf("\n");
}
#else
		pos = (subject)->GetPos();
		len = (subject)->GetLength();
		if (len == 0) break;
		if ((textp->rootEnvironment)->Remove(
				pos, len, environment_Style, TRUE))
			(textp)->SetModified();
		(textp)->RegionModified( pos, len);
		(textp)->NotifyObservers( 0);

		if (pos == 0 && len >= (textp)->GetLength()) 
			/* We have removed all style invocations from entire document.  
				Remove also all non-template style definitions. */
			(textp->styleSheet)->EnumerateStyles(
				(stylesheet_efptr)DeleteNonTemplateStyles, 
				(long)textp->styleSheet);
#endif
		break;

	/* {"nextstylesegment", "Fv", {Tstr, Tstr, Tvoid}, ness_codeOrange} */
	case 'v':
		/* advance subject to the segment extending 
			from its former finish to the next style change.  
			If at finish(base()), return finish(base()).*/
		textp = (class text *)(subject)->GetText();
		pos = (subject)->GetPos() + (subject)->GetLength();
		(subject)->SetPos( pos);
		envlen = (textp->rootEnvironment)->GetNextChange( pos);
		len = (textp)->GetLength();
		if (pos + envlen > len)
			envlen = len - pos;
		(subject)->SetLength( envlen);
		break;

	/* {"whereitwas", "Fw", {Tstr, Tvoid}, ness_codeOrange} */
	case 'w':
		/* push marker WhereItWas onto stack */
		(PUSHMARK(NULL))->SetFrom( WhereItWas);
		break;
	/* {"replacewithobject", "Fx", {Tstr, Tstr, Tptr, Tstr, Tvoid},
						ness_codeYellow} */
	case 'x': {
		cstring = (char *)(subject)->ToC();   /* get view name */
		NSP = popValue(NSP);	
		/* get object pointer */
		if (NSP->p.hdr != ptrHdr || NSP->p.v == NULL 
				|| (objval=ProperPtr((ATK  *)NSP->p.v,
					dataobjectClass)) == NULL)
			RunError(":Item to insert should be a dataobject", 
					opiar);
		NSP = popValue(NSP);
		/* ??? If the pointer is to a cel, and the type is not celview,
			we use the object in the cel.
			XXX should use view_CanView(), but it is not fully implemented */
		if ((objval)->IsType( celClass) && strcmp(cstring, "celview") != 0)
			objval = (ATK  *)((class cel *)objval)->GetObject();

		subject = FETCHMARK(NSP);	/* get text to be replaced */
		/* replace contents of marker now atop stack with
			a viewref for the object pointed at by objval 
			and having view given by cstring */
		textp = (class text *)(subject)->GetText();
		if ( ! (textp)->IsType( textClass))
			RunError(":objects cannot go in simple text", opiar);
		pos = (subject)->GetPos();
		len = (subject)->GetLength();
		if (*cstring == '\0') {
			free(cstring);
			cstring = strdup(((class dataobject *)objval  )->ViewName(
					));
		}

		  /* DO IT !   add a view of and ojbect to string */
		(textp)->AddView( pos+len, cstring, (class dataobject *)objval);

		if (len > 0)
			(textp)->DeleteCharacters( pos, len);
		/* XXX note that nessmarks do not get the special
			treatment given by the normal ness replace() */
		free(cstring);
		(subject)->SetLength( 1);    /* set returned mark */
	}	break;
	}
}
