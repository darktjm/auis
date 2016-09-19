/* tlex.c - lexical analyzer for ATK text */
/*
	Copyright Carnegie Mellon University 1992, 1994 - All rights reserved
*/
/*
 *    $Log: tlex.C,v $
// Revision 1.9  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.8  1994/08/11  19:07:46  rr2b
// The great gcc-2.6.0 new fiasco, new class foo -> new foo
//
// Revision 1.7  1994/07/08  16:05:44  rr2b
// Initialzed charx and lastcharx in the tlex constructor.
// BUG
//
// Revision 1.6  1994/04/11  00:10:44  wjh
// reformated func hdrs;  NextToken removed;
// LexFunc is directly the header for code that was in NextToken
// n
//
// Revision 1.5  1994/03/29  02:11:43  wjh
// try to remove disclaimer
//
// Revision 1.4  1994/03/18  17:42:23  rr2b
// Fixed to not have static before the definition of the static member
// function lexer.
// BUILD
//
// Revision 1.3  1994/03/18  03:08:50  wjh
// substantial improvement
//
// Revision 1.2  1993/05/18  17:28:05  rr2b
// Added missing return value.
// Fixed index argument to tlex::Recent* to be an int.
// Removed ::s before local variables.
//
// Revision 1.1  1993/05/06  18:08:37  rr2b
// Initial revision
//
 * Revision 1.8  1993/01/05  12:42:32  wjh
 * fixed a bug wherein any character above \177 would terminate a comment
 * fixed a bug wherein illegal characters were not reported thru -errorhandler-
 *
 *     log elided 9 April 1994 -wjh
 *
 * Revision 1.1 1992/8/5   wjh
 * Created from old lex.c
 */

#include <andrewos.h>
ATK_IMPL("tlex.H")
#include <ctype.h>

#include <text.H>
#include <parser.H>  /* for parser::ParseNumber */
#include <tlex.H>


#ifdef DODEBUG
#define DEBUG(p) (printf p, fflush(stdin))
#else
#define DEBUG(p)
#endif

	
ATKdefineRegistry(tlex, ATK, tlex::InitializeClass);
static int ErrorWithParm(class tlex  *self, struct tlex_ErrorRecparm  *parm);
static int ScanNumber(class tlex  *self, struct tlex_NumberRecparm  *parm);
static int ScanID(class tlex  *self, struct tlex_IDRecparm  *parm);
static int ScanString(class tlex  *self, struct tlex_StringRecparm  *parm);
static int ScanComment(class tlex  *self, struct tlex_CommentRecparm  *parm);


/* calls to this routine can arise:
	via a direct call from tlex.c or user handler to tlex_Error
	via a tokenclass with recognizer ScanError
	via an illegal character 
*/
static int
ErrorWithParm(class tlex  *self, struct tlex_ErrorRecparm  *parm) {
	long savex = self->RecentIndex;
	int val;

	if (parm->handler == NULL) {
		fprintf(stderr, "tlex: error at or before position %ld - %s\n",
			self->currpos, parm->msg);
		return tlex_IGNORE;
	}

	/* make dummy last token for RecordError */
	self->RecentPos[self->RecentIndex] = self->tokpos;
	self->RecentLen[self->RecentIndex] = self->tokend-self->tokpos+1;
	self->RecentIndex++;
	if (self->RecentIndex >= tlex_RECENTSIZE) self->RecentIndex = 0;

	val = (parm->handler)(self, parm);

	self->RecentIndex = savex;
	return val;
}


	 void
tlex::Error(const char  *msg) {
	struct tlex_ErrorRecparm *eparm = this->lextab->ErrorHandler;
	eparm->msg = msg;
	ErrorWithParm(this, eparm);
}


/* fetches character at self->pos; sets charx/lastcharx
	uses text_GetBuffer
	for BackUp, it checks that pos >= startpos
	for end-of-file, returns EOF
*/
	int
tlex::FetchChar() {
	long lenwant, lengot;
	if (this->currpos < this->startpos)
		this->currpos = this->startpos;
	if (this->currpos > this->lastpos) {
		this->currpos = this->lastpos+1;
		this->charx = this->lastcharx+1;
		this->currchar = EOF;
	}
	else {
		lenwant = this->lastpos - this->currpos + 1;
		this->charx = (this->text)->GetBuf( this->currpos,
				lenwant, &lengot);
		this->lastcharx = this->charx + lengot - 1;
		this->currchar = (int)(*((unsigned char *)(this->charx)));
	}
	return this->currchar;
}

/* stores the character c in the token buffer;
	usually called only when token buffer size is too small
*/
	int
tlex::PutTokChar(char  c) {
	int where = this->tokbufx - this->tokenbuffer;
	int size = this->tokbuflastx - this->tokenbuffer + 2;
	if (where > size - 5) {
		this->tokenbuffer = (char *)realloc(this->tokenbuffer, size+100);
		if (this->tokenbuffer == 0) {
			(this)->Error( "Token too big for memory");
			this->currpos = this->lastpos+1;
			return -1;
		}
		this->tokbufx = this->tokenbuffer + where;
		this->tokbuflastx = this->tokenbuffer + size + 100 - 2;
	}
	*this->tokbufx++ = c;
	return c;
}


/* the rock is available to any function passed this tlex
    The text, pos, and len specify a portion of a text to be processed
*/
	class tlex *
tlex::Create(const struct tlex_tables  *description, void  *rock, 
		class text  *text, long  pos, long  len)   {
	ATKinit;

	class tlex *result = new tlex;
	struct tlex_Recparm *global;

	result->lextab = description;
	result->rock = rock;
	if (text)
		(result)->SetText( text, pos, len);
	global = (result)->Global();
	if (global && global->handler) {
		(global->handler)(result, global);
		global->handler = NULL;	/* do it only once */
	}
	return result;
}

	void
tlex::SetText(class text  *text, long  pos , long  len)  {
	long i;
	this->text = text;
	this->currpos = pos;
	this->startpos = pos;
	this->lastpos = pos + len - 1;
	(this)->FetchChar();

	this->RecentIndex = 0;
	for (i = tlex_RECENTSIZE; i--; )
		this->RecentPos[i] = this->RecentLen[i] = 0;
}

	 boolean
tlex::InitializeClass()  {
	return TRUE;
}

tlex::tlex()  {
	ATKinit;

	this->charx=NULL;
	this->lastcharx=NULL;
	this->lextab = NULL;
	this->text = NULL;
	this->alttokenbuf = (char *)malloc(200);
	this->altbuflastx = this->alttokenbuf+200-2;
	this->tokenbuffer = (char *)malloc(200);
	this->tokbuflastx = this->tokenbuffer+200-2;
	if (this->tokenbuffer)
		*this->tokenbuffer = '\0';
	THROWONFAILURE( this->tokenbuffer != 0);
}

tlex::~tlex()  {
	ATKinit;

	free(this->tokenbuffer);
}


/* tlex_RecentPosition(self, index, plen)
	returns the character index of the first character of the 'index'th 
	token.  Indices are zero for the most recent token, -1 for its 
	predecessor, and so on.
	Indices must be in range [- RECENTSIZE+1 . . . 0]
*/
	long
tlex::RecentPosition(int  index, long  *ploc)  {
	long x;
	x = this->RecentIndex - 1 + index;
	while (x < 0) x += tlex_RECENTSIZE;
	if (ploc != NULL)
		*ploc = this->RecentLen[x];
	return this->RecentPos[x];
}

/* tlex_RecentIndent(self, index)
	returns the indent of 'index'th most recent token
	'index' must be in range [- RECENTSIZE+1 . . . 0]
	spaces count as 1 and tabs as 8
	if token is preceded by anything other than whitespace,
		its indent is 999
*/
	long
tlex::RecentIndent(int  index)  {
	long x;
	long indent = 0;
	long currpos = this->currpos;
	int c = 0;

	x = this->RecentIndex - 1 + index;
	while (x < 0) x += tlex_RECENTSIZE;
	(this)->BackUp( currpos - this->RecentPos[x]);

	while (this->currpos > this->startpos) {
		c = (this)->BackUp( 1);
		if (c == ' ')  indent ++;
		else if (c == '\t') indent +=8;
		else break;
	}
	if (c != '\n')
		indent = 999;

	this->currpos = currpos;
	(this)->FetchChar();

	return (indent);
}

/* tlex_Repeat(self, index)
	backup so the next token reported is the index'th, where
	index is as for RecentPosition.
*/
	void
tlex::Repeat(int  index)  {
	long x;
	x = this->RecentIndex - 1 + index;
	while (x < 0) x += tlex_RECENTSIZE;
	this->currpos = this->RecentPos[x];
	this->RecentIndex = x;
	(this)->FetchChar();
}



/* = = = = = = = = = = = = = = = = = = = = = = =
	the rest of this file is for
		LexFunc()
= = = = = = = = = = = = = = = = = = = = = = = = */


/* ScanNumber(self, parm)
	Parses the input string for an integer or real number
	tokenstartpos is first digit.
	Stores into TokenText a canonical version of the number.
	leaves curpos set to next character after the number
	returns value of calling parm->handler, if any
		otherwise TRUE

*/
	static int
ScanNumber(class tlex  *self, struct tlex_NumberRecparm  *parm)
		{
	long len;
	int success;
	double treal;
	long tlen, got;

	tlen = self->currpos - self->tokpos;  /* usually : 1 */
	if (tlen > 0)
		(self)->BackUp( tlen);
		DEBUG(("ScanNumber: tlen %d start %d pos %d\n", 
			tlen, self->tokpos, self->currpos));
	tlen = (self->text)->GetLength() - self->tokpos;
	success = parser::ParseNumber(
			(self->text)->GetBuf( self->tokpos, tlen, &got),
			&len, &parm->intval, &treal);
	if (success == 0) {
		(self)->Error( "Error ends number");
		parm->intval = -999;
		if (len <= 0) {
			DEBUG(("number too short: length %d\n", len));
			len = 1;	/* scan at least one character */
		}
	}
	if (len > got) {
		/* SIGH.  GetBuf didn't get enough 
				or ParseNumber went too far. */
		(self)->Error( "tlex_ScanNumber: GetBuf failure.  SIGH.");
		len = got;
	}
		DEBUG(("        Number.  pos before: %d\n", self->currpos));
	(self)->BackUp( -len);
		DEBUG(("        Number.  pos after: %d\n", self->currpos));
	(self)->EndToken();	/* set tokend */
	parm->realval = treal;
	parm->IsInt = (success != 2);
	if (success == 2) {
		/* double value */
		/* probably never read, since it was broken in previous versions */
		/* (i.e., two hex numbers next to each other w/o separator) */
		/* even more broken w/ 64-bit longs, since it assumed that */
		/* sizeof(double) == 2*sizeof(long) */
		/* %a is exact, but it's also C99/Single UNIX 3 */
		sprintf(self->tokenbuffer, "%a", treal);
	} else
		sprintf(self->tokenbuffer, "0x%lx", parm->intval);

	if (parm->handler)
		return (parm->handler)(self, parm);
	return tlex_ACCEPT;
}



/* ScanID(self, parm)
	parses a keyword or identifier
	first character is already in tokenbuffer
	second character is in currchar and is at currpos
	accepts characters having bits set in parm->continueset
	leaves next character in the text to be read
	returns tlex_ACCEPT (found a token)
		or value from handler
*/
	static int
ScanID(class tlex  *self, struct tlex_IDRecparm  *parm)
		{
	if (parm->SaveText) {
		if (parm->continueset.vector)
			while (tlex_BITISSET(parm->continueset, 
						self->currchar))
				(self)->Advance();
		else 
			while (isalnum(self->currchar))
				(self)->Advance();
	}
	else {
		if (parm->continueset.vector)
			while (tlex_BITISSET(parm->continueset, 
					self->currchar))
				(self)->NextChar();
		else 
			while (isalnum(self->currchar))
				(self)->NextChar();
	}
	(self)->EndToken();

	if (parm->handler)
		return (parm->handler)(self, parm);
	else return tlex_ACCEPT;
}


/* ScanString(self, parm)
	parses a string upto parm->endseq
	using parm->escapechar as the escape character

	string cannot contain parm->badchar
	unless it is escaped, in which case it is ignored

	at start, the opening 'delim' must be in tokenbuffer
	and first content character is in currchar and at currpos

	at completion, currpos is the char after closing 'delim'
*/
	static int
ScanString(class tlex  *self, struct tlex_StringRecparm  *parm)
		{
	int c;
	int delim = *parm->endseq;
	int badch = *parm->badchar;
	int escape = *parm->escapechar;

	if (parm->SaveText) {
		(self)->StartToken();
		c = self->currchar;
		while (c != delim) {
			if (c == escape)
				(self)->Advance();
			else if (c == badch || c == EOF) {
				(self)->Error( "Missing end delimiter in string");
				break;
			}
			c = (self)->Advance();
		}
		/* delim is currchar */
		(self)->EndToken();
		(self)->NextChar();
	}
	else {	
		c = self->currchar;
		while (c != delim) {
			if (c == escape)
				(self)->NextChar();
			else if (c == badch || c == EOF) {
				(self)->Error( "Missing end delimiter in string");
				break;
			}
			c = (self)->NextChar();
		}
		/* delim is currchar */
		(self)->NextChar();
		(self)->EndToken();
	}
	if (parm->handler)
		return (parm->handler)(self, parm);
	else return tlex_ACCEPT;
}


/* ScanComment(self, parm)
	parses a comment upto parm->endseq

	at start, the opening 'delim' must be in tokenbuffer
	and first content character is in currchar and at currpos
	characters are NOT put in tokenbuffer

	at completion, currpos is the char after endseq
*/
	static int
ScanComment(class tlex  *self, struct tlex_CommentRecparm  *parm)
		{
	char *delim = parm->endseq;
	char *cx;
	int c, dfirst = *delim, dlen;

	/* at the bottom of each while loop, c is a char that did not match, 
		but might start delim.  Because the delimiter must not have
		its first character within it, it suffices to continue 
		looking for the first char */

	if (parm->SaveText) {
		c = self->currchar;
		(self)->StartToken();		/* start body */
	
		/* scan for commentdelimiter */
		while (c != EOF) {
			if (c != dfirst)
				c = (self)->Advance();
			else {
				/* found first char of delim */
				c = (self)->Advance();
				cx = delim+1;
				while (*cx && *cx == c) {
					c = (self)->Advance();
					cx++;
				}
				if (*cx == '\0')
					break;
			}
		}
		/* arrange for token text to include only the body
			and not the delimiter */
		dlen = strlen(delim);
		(self)->TruncateTokenText( dlen);
		(self)->BackUp( dlen);
		(self)->EndToken();
		(self)->BackUp( -dlen);
	}
	else {	
		c = self->currchar;
	
		/* scan for commentdelimiter */
		while (c != EOF) {
			if (c != dfirst)
				c = (self)->NextChar();
			else {
				/* found first char of delim */
				c = (self)->NextChar();
				cx = delim+1;
				while (*cx && *cx == c) {
					c = (self)->NextChar();
					cx++;
				}
				if (*cx == '\0')
					break;
			}
		}
		(self)->EndToken();  /* record end of comment delimiter */
	}
	if (parm->handler)
		return (parm->handler)(self, parm);
	return tlex_IGNORE;
}



/* tlex::LexFunc
	Suitable for use as the lexer function required by the parser 
	object.  The lexrock value must be a tlex object.   

	Scan source text for the next token.
	Store semantic value into *yylval.
	Assumes that currchar is the first to be tested.
	Leaves currchar to the character after the token.
	Returns 0 for end-of-file.
*/
	int 
tlex::LexFunc(void *lexrock, void *yylval) {
	class tlex *self = (class tlex *)lexrock;

	const struct tlex_tables *tab = self->lextab;
	int action;
	struct tlex_Recparm *parm;
	int success = 0;
	char *tbuf;

	self->tokenvalue = NULL;	/* default value for *yylval */
	tbuf = self->tokenbuffer;
	self->tokenbuffer = self->alttokenbuf;
	self->alttokenbuf = tbuf;
	tbuf = self->tokbuflastx;
	self->tokbuflastx = self->altbuflastx;
	self->altbuflastx = tbuf;
	self->tokbufx = self->tokenbuffer;
	*self->tokenbuffer = '\0';

tryagain:  /* loop in case encountered whitespace or comment */

	DEBUG(("tryagain at  currchar '%c'  currpos %d\n",
		self->currchar, self->currpos));

	(self)->StartToken();
	if (self->currchar == EOF) 
		action = tlex_ACTEOF;
	else if (self->currchar > tab->hichar)
		action = tab->defaultaction;
	else {
		action = tab->action[self->currchar];
/*		DEBUG(("currchar '%c'   actflags %d   actval %d\n",
			self->currchar, action >> 8, action & 0xFF));
*/	}

	if ((action & tlex_ACTTHONG) == 0) {
		/* single character determines token type
		    some token types want the initial character
		    as a token in the tokenbuffer,
		    so we put it there for all token types
		*/
		(self)->Advance();
		(self)->EndToken();
	}
	else {
		/* recognize a thong */

		/* the action value is an index into the thong
			table.	Traverse table and input
			to find thong and thus the final
			tokennumber and action */
		const char * const *thongx;	/* index into thongtbl.
			Just before the Advance() *thongx is the first thong
			having as prefix the characters in
				text[tokpos...tokpos+currlen] */
		const char *samex;	/* pointer to thongsame elt for *thongx */
		int currlen;	/* position in *thongx to consider */
		const char * const *matchx = NULL;	/* index of longest recognized thong
				 matchx <= thongx */
		int matchlen = 0;	/* length of *matchx */
		int i;
		int c;		/* currchar */

		i = action & (~tlex_ACTTHONG);
		thongx = &(tab->thongtbl[i]);
		samex = &(tab->thongsame[i]);
		currlen = 0;
		c = (self)->CurrChar();

		/*
			Each circuit of the loop either advances through the 
			input and increases currlen, or it tries to
			advance to the next possible thong.
			Because thongtbl is sorted, we never visit an 
			entry more than once for each value of currlen.
		*/
		while (TRUE) {
			if (c == (*thongx)[currlen]) {
				c = (self)->Advance();
				currlen++;
			}
			else {
				if ((*thongx)[currlen] == '\0') {
					matchx = thongx;
					matchlen = currlen;
				}
				samex++;
				thongx++;
				if (*samex < currlen) break;
			}
		}
		/* here, matchx pts to the pointer to the recognized thong  */

		DEBUG(("found thong \"%s\"  (# %d)\n",
			*matchx, matchx - tab->thongtbl));

		i = currlen - matchlen;
		if (i > 0) {
			/* scanned too far */
			(self)->BackUp( i);
			(self)->TruncateTokenText( i);
		}
		(self)->EndToken();
		action = tab->thongact[matchx - tab->thongtbl];
	}   /* end of thong processing */

haveprefix:
	DEBUG(("haveprefix: currchar '%c' currpos %d actflags %d  actval %d\n",
		self->currchar, self->currpos, action >> 8, action & 0xFF));

	if (action & tlex_ACTRESWD) {
		parm = tab->reservedwordparm;
		self->tokennumber = action & (~tlex_ACTRESWD);
	}
	else if (action & tlex_ACTSCAN) {
		parm = tab->rectbl[action & (~tlex_ACTSCAN)];
		self->tokennumber = parm->tokennumber;		/* default */
	}
	else if (action == tlex_ACTEOF) {
		self->tokennumber = 0;
		self->tokenvalue = NULL;
		goto gotone;
	}
	else {
		/* no recognizer specified.  character or
			thong is a token by itself */
		self->tokennumber = action;
		self->tokenvalue = NULL;
		goto gotone;
	}

	DEBUG(("recognizer %d  token %d\n", parm->recognizerindex,
			parm->tokennumber));

	switch (parm->recognizerindex) {

	case tlex_WHITESPACE: {
		struct tlex_WhitespaceRecparm *wp
				= (struct tlex_WhitespaceRecparm *)parm;
		if (parm->SaveText) {
			if (wp->continueset.vector)
				while (tlex_BITISSET(wp->continueset, 
						self->currchar))
					(self)->Advance();
			else
				while (isspace(self->currchar))
					(self)->Advance();
			(self)->EndToken();
			success = (wp->handler)(self, (void *)parm);
			break;
		}
		else {
			if (wp->continueset.vector) 
				while (tlex_BITISSET(wp->continueset, 
							self->currchar))
					(self)->NextChar();
			else
				while (isspace(self->currchar))
					(self)->NextChar();
			goto tryagain;
		}
	}
	case tlex_COMMENT:
		success = ScanComment(self, 
				(struct tlex_CommentRecparm *)parm);
		break;
	case tlex_ID:
		success = ScanID(self, (struct tlex_IDRecparm *)parm);
		break;
	case tlex_NUMBER:
		success = ScanNumber(self, 
			(struct tlex_NumberRecparm *)parm);
		break;
	case tlex_STRING:
		success = ScanString(self, 
			(struct tlex_StringRecparm *)parm);
		break;
	case tlex_ERROR: {
		struct tlex_ErrorRecparm *thisparm =
			(struct tlex_ErrorRecparm *)parm;
		struct tlex_ErrorRecparm *ehparm = self->lextab->ErrorHandler;

		/* if thisparm has a handler proc, 
			call errorwithparm on thisparm
		otherwise emulate tlex_ERROR:
			copy the msg to -errorhandler- parm
			and call errorwithparm on the latter
		*/

		if (thisparm->handler)
			success = ErrorWithParm(self, thisparm);
		else {
			ehparm->msg = thisparm->msg;
			success = ErrorWithParm(self, ehparm);
		}

		if (success != tlex_IGNORE) 
			break;

		if (self->currchar == EOF) {
			self->tokennumber = 0;
			self->tokpos = self->lastpos+1;
			self->tokend = self->lastpos;
			success = tlex_ACCEPT;
		}
		break;
	}
	case tlex_TOKEN: {
		if (parm->handler == NULL)
			success = tlex_ACCEPT;
		else success = (parm->handler)(self, (void *)parm);
		break;
	}
	}  /* end switch(parm->recognizerindex) */

	DEBUG(("success: %d  tokennumber %d\n", success, self->tokennumber));

	if (success == tlex_IGNORE)
		goto tryagain;
	if (success != tlex_ACCEPT) {
		action = success;
		goto haveprefix;
	}
	
gotone:  /* success == tlex_ACCEPT */

	self->RecentPos[self->RecentIndex] = self->tokpos;
	self->RecentLen[self->RecentIndex] = self->tokend-self->tokpos+1;
	self->RecentIndex++;
	if (self->RecentIndex >= tlex_RECENTSIZE) self->RecentIndex = 0;

	/* In tlex, token values are of type (void *) so the YYSTYPE value 
		given to the parser in the grammar will cause the parser to
		allocate a buffer sized for a (void *) and pass its address
		as the value of yylval.  In effect then, the type of yylval
		is (void **).
	*/
	*((void **)yylval) = self->tokenvalue;
	return self->tokennumber;
}

