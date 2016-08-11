/* Copyright 1995 Carnegie Mellon University All rights reserved.

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
#ifndef _AString_H_
#define _AString_H_
ATK_INTER

/* 
	astring.H  -  headers for the String class

declaration:
	AString title, name;

usage:
	title = "Duchess";
	title.Append(" of Kent");
	name = "Clara";
	name.Append(", ").Append(title);	// Clara, Duchess of Kent
	int i = name.Search("of");		// 15
	title = "Moby Dick";
	boolean foo = (title == "Moby Dick");	// TRUE
	printf ("%s\n", (char *)title);		// prints "Moby Dick\n"
			// sigh:  need the cast
	name.Sprintf("any%s %d u\n", "thing", 4); // "anything 4 u\n"

---
	indices start at zero
	`space` is one less than actual space allocated
	the body always includes a terminating NUL byte
	the `used` value gives the string length excluding the NUL byte
	it must always be the case that 
		space >= used

 * $Log: astring.H,v $
 * Revision 1.10  1996/09/29  22:00:24  robr
 * Fixed bogus writes into string literals.
 * Bullet proofed against NULL char * arguments.
 * BUG
 *
 * Revision 1.9  1996/05/20  15:24:28  robr
 * /tmp/msg
 *
 * Revision 1.8  1995/12/10  15:03:16  wjh
 * working version
 *
 * Revision 1.7  1995/12/01  18:47:02  wjh
 * added Clear()
 * added 'const' in a few places
 * fixed a bug in VFileOpen
 *
 * Revision 1.6  1995/11/07  17:38:00  wjh
 * augmented
 *
 * Revision 1.5  1995/10/25  13:35:03  wjh
 * added operator[]
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
 * Created by Fred Hansen 12 September, 1995
*/

class AString {
private:
	size_t space, used;
	char *body;
public:
	inline AString() 
		{space = 0; used = 0; body = "";}
	inline AString(const char *s) 
		{space = 0; used = 0; body = "";  *this = s;}
	inline AString(const AString &s) 
		{space = 0; used = 0; body = "";  *this = s;}
	inline AString(const char s) 
		{space = 0; used = 0; body = "";  *this = s;}
	inline ~AString() 
		{if (space > 0) free (body);}

	inline operator  char *() const  { return body; }
	inline const char *operator+ (int i) const
		{ return body+((i>=0&&i<used)?i:used); }
	inline char operator[] (int i) const { return *((*this)+i); }
	inline int Length() const { return used; }
	inline void Clear() { used = 0; }

	inline AString &operator= (const char *s) 
        {const char *ss=s?s:""; return ReplaceN(0, used, ss, strlen(ss));}
	inline AString &operator= (const char s) 
		{return ReplaceN(0, used, &s, 1);}
	inline AString &operator= (const AString &s) 
		{return ReplaceN(0, used, s.body, s.used);}

	inline AString &operator<< (const char *s)
        {const char *ss=s?s:"";return ReplaceN(used, 0, ss, strlen(ss));}
	inline AString &operator<< (const AString &s)
		{return ReplaceN(used, 0, s.body, s.used);}
	inline AString &operator<< (const char s) 
		{return ReplaceN(used, 0, &s, 1);}
	inline AString &operator<< (const long i) 
		{const char *s = itoa(i); return ReplaceN(used,0,s,strlen(s));}
	inline AString &operator<< (const double d) 
		{const char *s = dtoa(d); return ReplaceN(used,0,s,strlen(s));}

	inline boolean operator==(const char *s) const 
        {const char *ss=s?s:""; return strcmp(body, ss) == 0;}
	inline boolean operator!=(const char *s) const 
		{const char *ss=s?s:""; return strcmp(body, ss) != 0;}
	inline boolean operator<(const char *s) const 
		{const char *ss=s?s:""; return strcmp(body, ss) < 0;}
	inline boolean operator>(const char *s) const 
		{const char *ss=s?s:""; return strcmp(body, ss) > 0;}
	inline boolean operator<=(const char *s) const 
		{const char *ss=s?s:""; return strcmp(body, ss) <= 0;}
	inline boolean operator>=(const char *s) const 
		{const char *ss=s?s:""; return strcmp(body, ss) >= 0;}

	inline AString &Append(const char *s) 
		{const char *ss=s?s:""; return ReplaceN(used, 0, ss, strlen(ss));}
	inline AString &Append(const char s) 
		{return ReplaceN(used, 0, &s, 1);}
	inline AString &Append(const AString &s) 
		{return ReplaceN(used, 0, s.body, s.used);}
	inline AString &Insert(int i, char *s) 
		{const char *ss=s?s:""; return ReplaceN(i, 0, ss, strlen(ss));}
	inline AString &Insert (int i, const char s) 
		{return ReplaceN(i, 0, &s, 1);}
	inline AString &Insert(int i, const AString &s) 
		{return ReplaceN(i, 0, s.body, s.used);}
	inline AString &Delete(int i, int len) 
		{return  ReplaceN(i, len, "", 0);}
	inline AString &Replace(int i, int len, const char *s)
		{const char *ss=s?s:""; return ReplaceN(i, len, ss, strlen(ss));}
	inline AString &Replace(int i, int len, const char s) 
		{return ReplaceN(i, len, &s, 1);}
	inline AString &Replace(int i, int len, const AString &s)
		{return ReplaceN(i, len, s.body, s.used);}

	inline AString &AppendN(const char *s, int len) 
		// DANGER: is len <= strlen(s)
		{const char *ss=s?s:""; return ReplaceN(used, 0, ss, len);}
	inline AString &InsertN(int i, char *s, int len) 
		{const char *ss=s?s:""; return ReplaceN(i, 0, ss, len);}

	inline int Search(const char c, int pos = 0) const
		{char b[2]; b[0]=c; b[1]='\0'; return Search(b, pos);}
	inline boolean FoldedEq(const char *s) const
		{const char *ss=s?s:""; return FoldedEqN(ss, strlen(ss));}
	inline boolean FoldedEq(const AString &s) const
		{return FoldedEqN(s.body, s.used);}

		// DANGER:  is slen < strlen(s)?
	AString &ReplaceN(int i, int len, const char *s, int slen);
	boolean FoldedEqN(const char *s, int len) const;  // caseless ==
	int Search(const char *s, int pos = 0) const;  
		// index of first substring == to s and at or after pos
	int FoldedSearch(const char *s, int pos = 0) const; // caseless Search
	int RegSearch(const char *pat, int pos = 0, int *len = NULL) const; 
			// index of first match to pat; sets *len
	char *SubstrCpy(char *s, int pos, int len) const;  // copies len chars
			// starting at pos TO s
	AString &Sprintf(const char *fmt, ...);	// construct string from format
	FILE *VfileOpen(char *mode);	// return a FILE * for I/O w/ string
		// *mode == 'w': write to the FILE *, gives AString a value
		// *mode == 'r': reading from FILE * fetches from AString
		// DO NOT USE THE ASTRING VALUE UNTIL CLOSING THE VFILE
	void VfileClose(FILE *f);   // restores the AString for further use

	static const char *itoa (long i);
	static const char *dtoa (double d);
};
#endif /* _AString_H_ */
