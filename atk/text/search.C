/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("search.H")
#include <ctype.h>


#include <simpletext.H>
#include <search.H>

// Extra stuff for wait cursors.
#include <im.H>
#include <cursor.H>

// Number of bytes to search to need a wait cursor
#define LONG_SEARCH_LENGTH 1000000

/* array to fold upper case to lower case 
Now modified to handle the iso 8859 char set  # 1 */

static const unsigned char searchFoldTRT[256] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, ' ', '!', '"', '#', '$', '%',
'&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3', '4',
'5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@', 'a', 'b', 'c',
'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^', '_', '`', 'a',
'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 127,
128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
208,209,210,211,212,213,214,247,216,217,218,219,220,221,222,255
};

/* Special operators in the search pattern. */
#define STAR 0200	/* what follows may be repeated zero or more times */
#define BPAR 0201	/* begin parenthesized expression */
#define EPAR 0202	/* end parenthesized expression */
#define SET  0203	/* match any of the following set (16 chars of bits follow to define the set) */
#define ANY1 0204	/* match any single character */
#define isop(c)	(((c) & 0200) && ((c) < 0205))	/* true if c is a special op */



ATKdefineRegistryNoInit(search, ATK);
static unsigned char * SkipOp (unsigned char *s );
static long TryMatch (class simpletext  *d, long  pos, unsigned char **s , int loop);
static long TryMatchStr (unsigned char *str, long  pos, long length, unsigned char **s , int loop);


static unsigned char *
SkipOp (unsigned char *s )
{
    switch (*s++) {
	default: 
	    return s;
	case STAR: 
	    return SkipOp (s);
	case SET: 
	    return s + 16;
	case BPAR: 
	    while (*s)
		if (*s == EPAR)
		    return s + 1;
		else
		    s = SkipOp (s);
	    return s;
    }
}

char *search::GetQuotedSearchString(const char  *string, char  *resString, long  resStrLen)
{
    long resultMaxLen;
    long resultLen = 0;
    char *result;
    char *res;
    
    if (resString == NULL) {
	resultMaxLen = strlen(string) + 10;
	result = (char *) malloc(resultMaxLen + 1);
    }
    else {
	resultMaxLen = resStrLen;
	result = resString;
    }
    res = result;

    while (*string != '\0') {
	switch (*string) {
	    case '.':
	    case '\\':
	    case '*':
	    case '[':
	    case ']':
		*res++ = '\\';
		resultLen++;
		break;
	}

	if (resultLen >= resultMaxLen - 2) {
	    if (resString != NULL) {
		*res = '\0';
		return NULL;
	    }
	    resultMaxLen *= 2;
	    result = (char *) realloc(result, resultMaxLen);
	    res = &result[resultLen];
	}

	*res++ = *string;
	resultLen++;
	string++;
    }
    *res = '\0';
    return result;
}

search::~search()
{
    if(body)
	free(body);
}

const char *
search::CompilePattern (const char *s)
{
    const unsigned char *string=(const unsigned char *)s;
    int     LastStart = -1;
    int     ParenStack[20];
    int     ParenDepth = 0;
    if (!body)
	body = (unsigned char *) malloc (size = 100);
    used = 0;
    MatchLength = -1;
    while (*string) {
	int     ThisStart = used;
	if (used + 20 >= size)
	    body = (unsigned char *) realloc (body, size += 50);
	switch (*string) {
	    case '.': 
		body[used++] = ANY1;
		break;
	    case '\\': 
		switch (*++string) {
		    case '(': 
			if (ParenDepth>19)
			    return "Too many \\('s in regular expression";
			body[used++] = BPAR;
			ParenStack[ParenDepth++] = ThisStart;
			break;
		    case ')': 
			body[used++] = EPAR;
			if (ParenDepth <= 0)
			    return "Unmatched \\) in regular expression";
			ThisStart = ParenStack[--ParenDepth];
			break;
		    default: 
			body[used++] = *string;
			break;
		}
		break;
	    case '[': {
		    unsigned char *set = body + used;
		    long    inverted = 0;
		    *set++ = SET;
		    if (*++string == '^')
			inverted = -1, string++;
		    {
			long    n;
			for (n = 16; --n >= 0;)
			    *set++ = inverted;
		    }
		    set = body + used + 1;
		    used += 17;
		    while (*string) {
			long min = *string;
			long max;
			if (string[1] == '-')
			    max = string[2], string += 2;
			else
			    max = min;
			while (min <= max) {
			    if (isalpha(min)) {
				long FlippedMin = min ^ 040;
				if (inverted)
				    set[FlippedMin >> 3] &= ~(1 << (FlippedMin & 7));
				else
				    set[FlippedMin >> 3] |= 1 << (FlippedMin & 7);
			    }
			    if (inverted)
				set[min >> 3] &= ~(1 << (min & 7));
			    else
				set[min >> 3] |= 1 << (min & 7);
			    min++;
			}
			if (*++string == ']')
			    break;
		    }
		    if (*string == 0)
			return "Unmatched '[' in regular expression";
		    break;
		}
	    case '*': 
		if (LastStart >= 0) {
		    unsigned char *d = body + used;
		    long n = used - LastStart;
		    while (--n >= 0)
			d[0] = d[-1], d--;
		    *d = STAR;
		    used++;
		    ThisStart = -1;   /* disallow another * */
		    break;
		}
		else return "'*' not after pattern in regular expression";
	    default: 
		body[used++] = searchFoldTRT[*string];
		break;
	}
	string++;
	LastStart = ThisStart;
    }  /* end of while loop */
    if (ParenDepth != 0)
	return "Unmatched \\( in regular expression";
    body[used] = 0;
    return 0;
}

// put up a process wait cursor.
static void PostWaitCursor()
{
    static cursor *wait_cursor = NULL;

    if (!wait_cursor) {
	im *Im = im::GetLastUsed();
	if (Im)
	    wait_cursor = cursor::Create(Im);
	if (wait_cursor)
	    wait_cursor->SetStandard(Cursor_Wait);
    }
    if (wait_cursor)
	im::SetProcessCursor(wait_cursor);
}

// retract the process wait cursor.
static void RetractWaitCursor()
{
    im::SetProcessCursor(NULL);
}

int search::MatchPattern (class simpletext  *d, long  pos)
{
    unsigned char *s;
    int canopt;
    unsigned char optchar;
    int dl;
    long bufLen;
    unsigned char *buf = NULL;
    boolean wait_cursor;

    MatchLength = 0;
    if (used <= 0) return -1;
    optchar = body[0];
    canopt = !isop(optchar);
    dl = (d)->GetLength();
    wait_cursor = (dl - pos > LONG_SEARCH_LENGTH);
    if (wait_cursor)
	PostWaitCursor();
    bufLen = 0;
    for (; pos < dl; ++pos) {
	int n;
	if (bufLen == 0) {
	    buf = (unsigned char *) (d)->GetBuf( pos, 1024, &bufLen);
	    if(buf==NULL) {
		if (wait_cursor)
		    RetractWaitCursor();
		return -1;
	    }
	}
	if (canopt)
            if (optchar != searchFoldTRT[bufLen--, *buf++])
                continue;
	s = body;
	n = TryMatch (d, pos, &s, 1);
	if (n>=0) {
	    MatchLength = n - pos;
	    if (wait_cursor)
		RetractWaitCursor();
	    return pos;
	}
    }
    if (wait_cursor)
	RetractWaitCursor();
    return -1;
}

int search::MatchPatternStr (unsigned char *str, long  pos, long length)
{
    unsigned char *s;
    int canopt;
    unsigned char optchar;
    long bufLen;
    unsigned char *buf = NULL;
    boolean wait_cursor;

    MatchLength = 0;
    if (used <= 0) return -1;
    optchar = body[0];
    canopt = !isop(optchar);
    wait_cursor = (length > LONG_SEARCH_LENGTH);
    if (wait_cursor)
	PostWaitCursor();
    bufLen = 0;
    for (; pos < length; ++pos) {
	int n;
	if (bufLen == 0) {
	    if (pos>=length) {
		if (wait_cursor)
		    RetractWaitCursor();
		return -1;
	    }
	    buf = str+pos;
	    bufLen = length-pos;
	}
	if (canopt)
            if (optchar != searchFoldTRT[bufLen--, *buf++])
                continue;
	s = body;
	n = TryMatchStr (str, pos, length, &s, 1);
	if (n>=0) {
	    MatchLength = n - pos;
	    if (wait_cursor)
		RetractWaitCursor();
	    return pos;
	}
    }
    if (wait_cursor)
	RetractWaitCursor();
    return -1;
}

int search::MatchPatternReverse (class simpletext  *d, long  pos )
{
    unsigned char *s;
    int canopt;
    unsigned char optchar;
    long bufLen;
    unsigned char *buf = NULL;
    boolean wait_cursor;

    MatchLength = 0;
    if (used <= 0) return -1;
    optchar = body[0];
    canopt = !isop(optchar);
    wait_cursor = (pos > LONG_SEARCH_LENGTH);
    if (wait_cursor)
	PostWaitCursor();
    bufLen = 0;
    for (; pos >= 0; --pos) {
	int n;
	if (bufLen == 0) {
	    buf = (unsigned char *)(d)->GetBufEnd( pos + 1, 1024, &bufLen);
	    if(buf==NULL) {
		if (wait_cursor)
		    RetractWaitCursor();
		return -1;
	    }
	}
	if (canopt)
            if (optchar != searchFoldTRT[bufLen--, *--buf])
                continue;
	s = body;
	n = TryMatch (d, pos, &s, 1);
	if (n>=0) {
	    MatchLength = n - pos;
	    if (wait_cursor)
		RetractWaitCursor();
	    return pos;
	}
    }
    if (wait_cursor)
	RetractWaitCursor();
    return -1;
}

int search::MatchPatternStrReverse (unsigned char *str, long  pos, long length )
{
    unsigned char *s;
    int canopt;
    unsigned char optchar;
    long bufLen;
    unsigned char *buf = NULL;
    boolean wait_cursor;

    MatchLength = 0;
    if (used <= 0) return -1;
    optchar = body[0];
    canopt = !isop(optchar);
    wait_cursor = (pos > LONG_SEARCH_LENGTH);
    if (wait_cursor)
	PostWaitCursor();
    bufLen = 0;
    for (; pos >= 0; --pos) {
	int n;
	if (bufLen == 0) {
	    if (pos>=length || pos<0) {
		if (wait_cursor)
		    RetractWaitCursor();
		return -1;
	    }
	    buf = str+pos+1;
	    bufLen = pos+1;
	}
	if (canopt)
            if (optchar != searchFoldTRT[bufLen--, *--buf])
                continue;
	s = body;
	n = TryMatchStr (str, pos, length, &s, 1);
	if (n>=0) {
	    MatchLength = n - pos;
	    if (wait_cursor)
		RetractWaitCursor();
	    return pos;
	}
    }
    if (wait_cursor)
	RetractWaitCursor();
    return -1;
}

static long TryMatch (class simpletext  *d, long  pos, unsigned char **s , int loop)
{
    unsigned char  c, dc;
    unsigned char *buf = NULL;
    long bufLen = 0;
    long length = (d)->GetLength();

    do {
	if ((c = *(*s)++)) {
	    switch (c) {
		case BPAR:
		    if ((pos = TryMatch (d, pos, s, 1)) < 0)
			return pos;
		    break;
		case EPAR:
		    return pos;
		case ANY1:
		    if (bufLen == 0) {
			buf = (unsigned char *)(d)->GetBuf( pos, 1024, &bufLen);
			if (buf==NULL) return -1;
		    }
		    dc = *buf++, bufLen--;
		    if (pos >= length || dc == '\n')
			return -1;
		    pos++;
		    break;
		case SET:
		    if (bufLen == 0) {
                        buf = (unsigned char *)(d)->GetBuf( pos, 1024, &bufLen);
			if (buf==NULL) return -1;
		    }
		    dc = *buf++, bufLen--;
		    if (((*s)[dc >> 3] & (1 << (dc & 7))) == 0)
			return -1;
		    *s += 16;
		    pos++;
		    break;
		case STAR:
		    {
			unsigned char  *st = *s;
			int     npos, code;
			/* first check if we've got another in a row */
			if ((npos = TryMatch (d, pos, &st, 0)) >= 0) {
			    (*s)--;
			    /* try to continue wildcard matching */
			    code = TryMatch (d, npos, s, 1);
			    /* if that fails, see if the rest of the pattern matches here */
			    if (code < 0) return TryMatch(d, pos, &st, 1);
			    else return code;
			}
			else
			    *s = SkipOp (*s);
		    }
		    break;
		default:
		    if (pos >= length) return -1;
		    if (bufLen == 0) {
			buf = (unsigned char *)(d)->GetBuf( pos, 1024, &bufLen);
			if(buf==NULL) return -1;
		    }
		    dc = *buf++, bufLen--;
		    if (c != searchFoldTRT[dc])
			return - 1;
		    pos++;
		    break;
	    }
	}
	else
	    break;
    } while (loop);
    return pos;
}


static long TryMatchStr (unsigned char *str, long  pos, long length, unsigned char **s , int loop)
{
    unsigned char  c, dc;
    unsigned char *buf = NULL;
    long bufLen = 0;

    do {
	if ((c = *(*s)++)) {
	    switch (c) {
		case BPAR:
		    if ((pos = TryMatchStr (str, pos, length, s, 1)) < 0)
			return pos;
		    break;
		case EPAR:
		    return pos;
		case ANY1:
		    if (bufLen == 0) {
			if (pos>=length)
			    return -1;
			buf = str+pos;
			bufLen = length-pos;
		    }
		    dc = *buf++, bufLen--;
		    if (pos >= length || dc == '\n')
			return -1;
		    pos++;
		    break;
		case SET:
		    if (bufLen == 0) {
			if (pos>=length)
			    return -1;
			buf = str+pos;
			bufLen = length-pos;
		    }
		    dc = *buf++, bufLen--;
		    if (((*s)[dc >> 3] & (1 << (dc & 7))) == 0)
			return -1;
		    *s += 16;
		    pos++;
		    break;
		case STAR:
		    {
			unsigned char  *st = *s;
			int     npos, code;
			/* first check if we've got another in a row */
			if ((npos = TryMatchStr (str, pos, length, &st, 0)) >= 0) {
			    (*s)--;
			    /* try to continue wildcard matching */
			    code = TryMatchStr (str, npos, length, s, 1);
			    /* if that fails, see if the rest of the pattern matches here */
			    if (code < 0) return TryMatchStr (str, pos, length, &st, 1);
			    else return code;
			}
			else
			    *s = SkipOp (*s);
		    }
		    break;
		default:
		    if (pos >= length) return -1;
		    if (bufLen == 0) {
			if (pos>=length)
			    return -1;
			buf = str+pos;
			bufLen = length-pos;
		    }
		    dc = *buf++, bufLen--;
		    if (c != searchFoldTRT[dc])
			return - 1;
		    pos++;
		    break;
	    }
	}
	else
	    break;
    } while (loop);
    return pos;
}










