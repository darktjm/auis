/* charset.c  -  charset munger for gentlex */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
*/
/*
	a char set declaration is bounded with [ and ]
	it contains a sequence of characters to be included in the set
	the form x-y stands for all characters in the collating set
		from x to y, inclusive
	\ quotes \, -, ], and the other characters handled
		by TransEscape
	If newline is discovered before the ], an error is signalled
	- must be preceded and followed by characters that are not
		the end of the set and not used by other -'s
	whitespace in the set other than newline is significant
	If non-whitespace characters are found before the [,
		an error is signalled.	The characters are put in the set.
*/

#include <andrewos.h>
#include <stdio.h>
#include <ctype.h>
#include <global.h>

/* there is one bitarray for every eight Charsets created */
struct bitarray {
	char *name;
	unsigned char bits[256];
	int nentries;			/* max of 8 */
	struct bitarray *prev;	/* link to most recent previous bitarray */
};

/* the bitarray to be used for next table.
  A new bitarray must be created if CurrBitAr is NULL
  or CurrBitAr->nentries = 8
*/
static struct bitarray *CurrBitAr = NULL;


/* read a charset declaration  from string s
	Returns ptr to static storage containing a 256 character bit
		vector with ones set for the characters in the set.
*/
char * CharsetParse(char  *s);
char * CharsetValue(char  *set);
void CharsetOutputArrays(FILE  *f);


char *
CharsetParse(char  *s)
	{
	static char set[256];
	int c, d;
	unsigned int len, i;

	for (i = 0; i < sizeof(set); i++)
		set[i] = 0;
	while (isspace(*s)) s++;
	if (*s != '[')
		Error(ERROR, "missing '[' for charset");
	else s++;
	while (*s != '\n'  &&  *s != ']') {
		c = *s++ & 0xFF;
		if (c == '-')
			ErrorA(ERROR, "Bogus '-' in charset", s-1);
		if (c == '\\') {
			c = TransEscape(s, (int *)&len);
			s += len;
		}
		/* c is a char.  Maybe first of range */
		if (*s != '-')
			d = c;
		else {
			/*  - : it's a range  */
			s++;	/* skip '-' */
			d = *s++ & 0xFF;
			if (d == '-') 
				ErrorA(ERROR, "Double '-' in char set", s-2);
			if (d == ']') {
				Error(ERROR, "'-]' in char set");
				d = c;
				s--;
			}
			if (d == '\\') {
				d = TransEscape(s, (int *)&len);
				s += len;
			}
		}

		/* add range c...d to set */
		if (d < c) {
			char buf[4];
			int t;
			buf[0] = c;  buf[1] = '-';  buf[2] = d;  buf[3] = '\0';
			ErrorA(ERROR, "charset has bounds out of order", 
					Escapify(buf, NULL));
			t = c;  c = d;  d = t;   /* swap bounds */
		}
		for ( ; c <= d; c++)
			set[c] |= 1;
	}
	if (*s != ']')
		Error(ERROR, "missing ']' for charset");
	return set;
}

/* creates the C scource representation of the Charset given by
	the 256 character set (which is as returned from ParseCharSet)
*/
	char *
CharsetValue(char  *set)
	{
	int mask, i;
	char *setref;

	if (CurrBitAr == NULL || CurrBitAr->nentries == 8) {
		/* allocate a new bitarray */
		struct bitarray *tb
			= (struct bitarray *)malloc(sizeof (struct bitarray));
		tb->nentries = 0;
		tb->name = GenSym();
		for (i = 0; i < 256; i++)
			tb->bits[i] = 0;
		tb->prev = CurrBitAr;
		CurrBitAr = tb;
	}
	mask = 1<<CurrBitAr->nentries;
	CurrBitAr->nentries++;

	for (i = 0; i < 256; i++)
		if (set[i])
			CurrBitAr->bits[i] |= mask;

	setref = (char *)malloc(20);
	sprintf(setref, "{%s, 0x%02x}", CurrBitAr->name, mask);
	return setref;
 }

/* output all Charset arrays */
	void
CharsetOutputArrays(FILE  *f)
	{
	int j;
	for ( ; CurrBitAr != NULL; CurrBitAr = CurrBitAr->prev) {
		fprintf(f, "static const char %s[256] = {", CurrBitAr->name);
		for (j = 0; j < 256; j++) {
			if (j % 8 == 0) {
				char b[2];
				b[0] = j;  b[1] = '\0';
				fprintf(f, "\n  /* '%s' */\t", 
						Escapify(b, NULL));
			}
			fprintf(f, "0x%02x%s", CurrBitAr->bits[j],
					(j < 255) ? ", " : "");
		}
		fprintf(f, "\n};\n");
	}
}
