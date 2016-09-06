/* ***************************************************
 *	   Copyright Carnegie Mellon, 1995 - All Rights Reserved
 *        For full copyright information see:'andrew/config/COPYRITE'
 *  ***************************************************

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

/* astring.C		

	Code for the String data object

 *    $Log: astring.C,v $
 * Revision 1.9  1996/09/29  22:00:24  robr
 * Fixed bogus writes into string literals.
 * Bullet proofed against NULL char * arguments.
 * BUG
 *
 * Revision 1.8  1996/05/22  19:33:48  robr
 * Fixed to avoid freeing non-heap memory.
 * BUG
 *
 * Revision 1.7  1996/05/20  15:24:28  robr
 * /tmp/msg
 *
 * Revision 1.6  1996/04/03  16:48:13  wjh
 * fixed FoldedEqN
 *
 * Revision 1.5  1995/12/01  18:47:02  wjh
 * added Clear()
 * added 'const' in a few places
 * fixed a bug in VFileOpen
 *
 * Revision 1.4  1995/10/25  13:22:21  wjh
 * added pos argument to Search, FoldedSearch, and RegSearch
 *
 * Revision 1.3  1995/10/24  18:53:14  wjh
 * added SubstrCpy
 *
 * Revision 1.2  1995/10/24  15:37:52  wjh
 * added VFileOpen, VfileClose
 *
 * Revision 1.1  1995/09/23  15:18:47  wjh
 * Initial revision
 * 
 * Revision 1.0  95/09/05  16:23:22  wjh
 * Copied from /afs/cs/misc/atk/@sys/alpha/lib/null
 */

#include <andrewos.h>
ATK_IMPL("astring.H")
#include <util.h>
#include <search.H>
#include <im.H>

#include <stdarg.h>
#include <ctype.h>

#include <astring.H>


//-----------------------------------
// replace a substring
//
	AString &
AString::ReplaceN(int i, int len, const char *src, int slen) {
	if (len < 0 || slen < 0 || len + i > (int)used) {
		// bogus xxx should have error message
		return *this;
        }
        if(src==NULL) src="";
	char *dest = body+i;
	int taillen = used - len -i + 1;  // 1 + length(body[i+len ... used])
			// taillen is 1 longer to copy the final \0
	if (slen == 0) {   	// deletion
		if (len > 0)
			memmove(dest, dest+len, taillen);
	}
	else if (i == (int)used && used+slen <= space) {
		// append 
		memcpy(dest, src, slen);
		body[used+slen] = '\0';
	}
	else if (used+slen-len > space) {	//  malloc new space
	    char *old = body;
	    size_t oldspace=space;
		size_t newspace = 7*space/5;
		if (newspace < space + slen - len) 
			newspace = 6*(used+slen-len)/5;
		body = (char *)malloc(newspace+1);
		space = newspace;
		// three copies to build new value
		memcpy(body, old, i);
		memcpy(body+i, src, slen);
		memcpy(body+i+slen, old+i+len, taillen);
		if(oldspace>0) free (old);
	} 
	else if (slen <= len) {
		// shorten: copy src, then tail  (overlap doesn't matter)
		memmove(dest, src, slen);
		if (slen != len) 
			memmove(dest+slen, dest+len, taillen);
	}
	else if (src+slen < dest || src > body+used) {
		// non-overlapping case, move tail and copy src
		if (slen != len) 
			memmove(dest+slen, dest+len, taillen);
		memcpy(dest, src, slen);
	}
	else {  // slen > len
		// overlap & lengthen: copy tail, then src pieces
		memmove(dest+slen, dest+len, taillen);

		int priorlen = dest + len - src;
		if (priorlen < 0) priorlen = 0;
		if (priorlen > slen) priorlen = slen;
		const char *post;
		if (priorlen) post = dest+slen;
		else post = src + slen - len;  // starts in the tail
		int postlen = slen - priorlen;
		if (priorlen) 		// move part of src prior to i
			memmove(dest, src, priorlen);
		if (postlen) 		// move part of src after i
			memmove(dest+priorlen, post, postlen);
	}
	used += (slen-len);
	return *this;
}

//-----------------------------------
// caseless equality test
//
boolean 
    AString::FoldedEqN(const char *s, int len)  const {
    if(s==NULL) s="";
    return len <= (int)used && ::FoldedEQn(body, s, len);
}


//-----------------------------------
// index of first substring == to s  (or -1)
//
//	xxx should use Sunday Search from Ness
//
	int 
AString::Search(const char *p, int pos) const {
        if (pos >= (int)used || p==NULL) return -1;
	char *s = body+pos;
	int plen = strlen(p);
	while ((s=strchr(s, *p))) {
		if (s-body+plen > (int)used) break;
		if (strncmp(s, p, plen) == 0)
			return s - body;
		else s++;
	}
	return -1;
}

//-----------------------------------
// caseless search
//
	int 
AString::FoldedSearch(const char *p, int pos)  const {
        if (pos >= (int)used || p==NULL) return -1;
	char p1 = tolower(*p);
	char P1 = toupper(*p);
	const char *s = strchr(body+pos, p1);
	const char *S = strchr(body+pos, P1);
	while (s || S)
		if ( ! s) {
			if (FOLDEDEQ((S+1), (p+1)))
				return S - body;
			else S = strchr(S+1, P1);
		}
		else if ( ! S || s < S) {
			if (FOLDEDEQ((s+1), (p+1)))
				return s - body;
			else s = strchr(s+1, p1);
		}
		else {		// s > S
			if (FOLDEDEQ((S+1), (p+1)))
				return S - body;
			else S = strchr(S+1, P1);
		}
	return -1;
}


//-----------------------------------
// index of first match to pat
//
	int 
AString::RegSearch(const char *pat, int pos, int *len) const {
	if (pos >= (int)used) return -1;
	if ( ! pat || ! *pat) return -9;

	static struct SearchPattern *regPat;
	static const char *msg;
	static AString oldpat = AString();

	if (len) *len = 0;

	if (oldpat != pat) {
		oldpat = pat;
		msg = search::CompilePattern((char *)pat, &regPat);
		if (msg)  // xxx bogus message destination
			fprintf(stderr, "AString::RegSearch -- %s\n", msg);
	}
	if (msg) return -9;		// not a valid pattern 

	int v = search::MatchPatternStr((unsigned char *)body+pos, 
			0, used-pos, regPat);
	if (v < 0) return -1;		// not found
	if (len) *len = search::GetMatchLength();
	return v;
}
// copies len chars starting at pos TO s
//
	char *
AString::SubstrCpy(char *s, int pos, int len) const {
        if(s==NULL) return NULL;
        if (pos < 0) pos = 0;
	if (pos > (int)used) pos = used;
	if (pos+len > (int)used) len = used-pos;
	strncpy(s, body+pos, len);
	s[len] = '\0';
	return s;
}


//-----------------------------------
// construct string from format
//
	AString &
AString::Sprintf(const char *fmt, ...)  {
        if(fmt==NULL) fmt="";  
	va_list ap;
	va_start(ap, fmt);
	const char *p = fmt, *q;
	char format[100], *fx, *fend = format + sizeof(format) - 2;

	while (*p) {
		if (*p != '%') {
			for (q = p+1; *q && *q != '%'; q++) {}
			AppendN(p, q-p);
			p = q;
			continue;
		}

		// *p == '%':  process format
		// scan forward to format letter
		// capturing the control values is just for %s
		// but we do need to process *'s
		char size = '\0';	// \0 h l L
		int width = 0, prec = -1;
		boolean minus = FALSE, predot = TRUE;
		q = p; 
		fx = format;
		*fx++ = *q++;
		for ( ; *q; q++) {
			if (fx < fend)
				*fx++ = *q;
			switch (*q) {
			case 'h': case 'l': case 'L':  size = *q; continue;
			case '-': minus = TRUE; continue;
			case '.': predot = FALSE; continue;
			case '*':
				(predot ? width : prec) = va_arg(ap, int);
				continue;
			case '0': case '1': case '2': case '3': case '4': 
			case '5': case '6': case '7': case '8': case '9': { 
			    // parse integer, ignore initial 0
			    int v = *q++ - '0';
			    while (isdigit(*q)) v = 10*v + *q++ - '0';
			    q--;
			    (predot ? width : prec) = v;
			    }
				continue;
			}
			if (isalpha(*q) || *q == '%')  
				break;		// no q++
		}
		*fx = '\0';
		// conversion specification is *q

#define dyad(a, b)  case (((a)<<8) | (b))

		// process format
		char buff[500];

		if (*q == 's') {
			// put a string.  avoid buff since string is unbounded
			// algorithm inspired by Plauger
			char *s = va_arg(ap, char *);
			int n1 = strlen(s);
			if (0 <= prec && prec < n1) n1 = prec;
			if (minus)
				AppendN(s, n1);
			if (width > n1) {
				int padding = width - n1;
				// pad with (padding) spaces
				static const char spaces[] = "                   ";
				static const int splen = sizeof(spaces) - 1;
				while (padding > splen) {
					*this << spaces;
					padding -= splen;
				}
				if (padding) AppendN(spaces, padding);
			}
			if (! minus)
				AppendN(s, n1);
		}
		else if (width >= 499) {
			// ignore silly format with huge widths
		}
		else {
		  switch ((size<<8) | *q) {

		  case 'c':case 'd': case 'i': case 'o': case 'u': case 'x': 
		  case 'X': dyad('h','d'): dyad('h','i'): dyad('h','o'): 
		  dyad('h','u'): dyad('h','x'): dyad('h','X'): 
			sprintf(buff, format, va_arg(ap, int));
			break;
		  dyad('l','d'): dyad('l','i'): dyad('l','o'): dyad('l','u'): 
		  dyad('l','x'): dyad('l','X'):  
			sprintf(buff, format, va_arg(ap, long int));
			break;
		  case 'e': case 'E': case 'f': case 'g': case 'G':  
			sprintf(buff, format, va_arg(ap, double));
			break;
		  dyad('L','e'): dyad('L','E'): dyad('L','f'):  
		  dyad('L','g'): dyad('L','G'): 
			sprintf(buff, format, va_arg(ap, long double));
			break;
		  case 'n': {
			int *pi = va_arg(ap, int *);
			*pi = used;
			*buff = '\0';
		    }	break;
		  dyad('h','n'):  {
			short int *ph = va_arg(ap, short int *);
			*ph = used;
			*buff = '\0';
		    }	break;
		  dyad('l','n'):  {
			long int *pl = va_arg(ap, long int *);
			*pl = used;
			*buff = '\0';
		    }	break;
		  case 'p': 
			sprintf(buff, format, va_arg(ap, void *));
			break;
		  case '%': 
			if (q != p+1)
				// FALL THRU.   UGGGH.  BZZZT.  
		  default:	// default case is erroneous conversion spec
				q = p;   // restart after the leading %
			buff[0] = '%';  buff[1] = '\0';
			break;
		  }   //switch(dyad(size,*q)) 

		  // append formatted string to target
		  *this << buff;

		}  //else 
		p = q+1;   // start with character after the conversion spec
	}
#undef dyad

	va_end(ap);
	return *this;
}

static struct expbufS {
	FILE *f;
	struct expandstring e;
	boolean inuse;
} expbuftable[4];
static boolean vinited = FALSE;

// return a FILE * for I/O w/ string
// 	*mode == 'w': write to the FILE *, gives AString a value
// 	*mode == 'r': reading from FILE * fetches from AString
//	Returns NULL if no more Vfile's available (max 4)
// 	DO NOT USE THE ASTRING VALUE UNTIL CLOSING THE VFILE
//
		FILE *
AString::VfileOpen(const char *mode) {
	int i;
	if ( ! vinited) {
		for (i = sizeof(expbuftable)/sizeof(*expbuftable); i--; )
			expbuftable[i].inuse = FALSE;
		vinited = TRUE;
	}

	// find an expbuf to use
	for (i = sizeof(expbuftable)/sizeof(*expbuftable); i--; )
		if ( ! expbuftable[i].inuse) break;
	if (i < 0) return NULL;		// no more available

	// set up expbuf from *this
	expbuftable[i].inuse = TRUE;
	struct expandstring *eb = &expbuftable[i].e;
	if (space) {
		eb->string = body;
		eb->size = space+1;  
			// expandstring.size includes \0, space does not
	} else {
		eb->string = NULL;
		eb->size = 0;
	}
	eb->length = (*mode == 'r') ? used : 0;
	eb->pos = 0;

	return 	expbuftable[i].f = im::vfileopen(mode, eb);
}

// restores the AString for further use
//
	void 
AString::VfileClose(FILE *f) {
	if (! vinited) return;		// ERROR
	int i;

	// find the right expbuftable element
	for (i = sizeof(expbuftable)/sizeof(*expbuftable); i--; )
		if (expbuftable[i].f == f) break;
	if (i < 0) return;		// ERROR

	// set up *this from expbuf
	struct expandstring *eb = &expbuftable[i].e;
	im::vfileclose(f, eb);
	body = eb->string;
	space = (size_t)eb->size -1;
	used = (size_t)eb->length;
	expbuftable[i].inuse = FALSE;
}




	const char *
AString::itoa (long i) {
	static char retbuf[50];
	sprintf(retbuf, "%ld", i);
	return retbuf;
}

	const char *
AString::dtoa (double d) {
	static char retbuf[50];
	sprintf(retbuf, "%g", d);
	return retbuf;
}
