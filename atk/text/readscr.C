/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/* ** readscr.c  -  converts an ASCII file containing Scribe commands to
		 a base editor II document ** */


#include <andrewos.h> /* sys/file.h */
ATK_IMPL("readscr.H")
#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/stat.h>
#ifdef hpux
#include <unistd.h>
#endif /* hpux */

#include <im.H>
#include <textview.H>
#include <text.H>
#include <print.H>
#include <style.H>
#include <stylesheet.H>
#include <environment.H>

#include <readscr.H>

#define STACKSIZE 1000
#define STRINGSIZE 50
#define NORM 0
#define BEGEND 1
#define PT 2
#define SCRBEGIN "begin"
#define SCREND "end"

static class environment *Par;
/* Next one is not used anywhere...
static struct environment *NewPar;
 */
static int Top, CurPos, Offset, PPos, Mode, End, LineFeeds, OldFormat;
static const char Left[] = "{[(<\"'\0",  Right[] = "}])>\"'\0";
static struct StackItem {
    class environment *Parent;
    class style *StyleName;
    int Delimiter, Position, CheckMode;
} *Stack[STACKSIZE + 1];










ATKdefineRegistry(readscr, ATK, NULL);
static void goshdarn(const char  *errmsg);
static int textfix(class text  *d, int  len);
static void linefix(class text  *d);
static int scribefix(class text  *d);
static int longscribe(class text  *d, char  *shortcommand);
static int shortscribe(class text  *d, char  *shortcommand);
static int startenv(int  delim, class style  *tempstyle);
static int finishenv();


static void goshdarn(const char  *errmsg)
{
    fprintf(stderr, "<warning:readscr>%s\n", errmsg);
}

readscr::readscr() /* stupid convention */
    {
    THROWONFAILURE( TRUE);
}

class text *readscr::Begin(class text  *d, int  pos , int  len , int  purge , const char  *version, int  GetTemplate)
{
    if (GetTemplate && (d)->ReadTemplate( "scribe", 0)) {
	goshdarn("Couldn't read template.");
	return(0);
    }
    Par = d->rootEnvironment;
    Top = -1;
    CurPos = End = LineFeeds = 0;
    Offset = pos;
    PPos = CurPos;
    Mode = NORM;
/* Important note:  "yes" in lower case should be considered as equivalent to "2" */
    if (version && (!strcmp(version, "Yes"))) {
	OldFormat = 1;
    } else {
	OldFormat = 0;
    }
    End = pos + len;
    if (textfix(d, len)) {
	goshdarn("Error parsing text.");
	return(0);
    }
    if (purge) {
	(d)->AlwaysDeleteCharacters( pos, len);
    }
    return(d);
}

int readscr::PrintFile(char  *filename , class textview  *tv, class text  *d, const char  *Version, int  TrashWhenDone)
{
    int fd;
    struct stat statbuf;
    char *s;

    (d)->Clear();
    if (stat(filename, &statbuf)
    || (s = (char *)malloc(1+statbuf.st_size)) == NULL
    || (fd = open(filename, (TrashWhenDone ? osi_O_READLOCK : O_RDONLY), 0444)) < 0) {
	return(1);
    }
    if ((TrashWhenDone && osi_ExclusiveLockNoBlock(fd))
     || (read(fd, s, statbuf.st_size) != statbuf.st_size)
     || ((d)->AlwaysInsertCharacters( 0, s, statbuf.st_size), 
        (readscr::Begin(d, 0, statbuf.st_size, 1, Version, 1)) != d)
     || print::ProcessView(tv, 1, 0, "Scribe-format-file", "")
     || (TrashWhenDone && unlink(filename))) {
	close(fd); /* Held open to now for locking purposes */
	return(1);
    } else {
	close(fd); /* Held open to now for locking purposes */
	return(0);
    }
}


/* ** textfix - reads chars in and handles them appropriately ** */
static int textfix(class text  *d, int  len)
{
    int i, tmp;

    tmp = (d)->GetChar( (CurPos + Offset));
    while ((CurPos < len) /* && (tmp != -1) */) {
	if (tmp == -1) {
	    goshdarn("Text includes a -1 character; treating as normal text...");
	}
	switch (tmp) {
	    case '\n':
		++LineFeeds;
		++CurPos;
		break;

	    case '@':
		if (scribefix(d)) {
		    goshdarn("Error parsing scribe command.");
		    return(1);
		}
		break;

	    case '}':
	    case ']':
	    case ')':
	    case '>':
	    case '"':
	    case '\'':
		i = 0;
		while (Right[i] != '\0') {
		    if (tmp == Right[i]) {
			if (Top >= 0 && Stack[Top]->Delimiter == Left[i]) {
			    switch (Stack[Top]->CheckMode) {
				case PT:
				case NORM:
				    if ((d)->GetChar(CurPos+Offset+1) == '\n') {
					++LineFeeds;
				    }
				    if (LineFeeds > 0) linefix(d);
				    if (finishenv()) {
					return(1);
				    }
				    break;

				case BEGEND:
				    if (LineFeeds > 0) linefix(d);
				    (d)->AddInCharacter( (d)->GetLength(), tmp);
				    End++;
				    break;

				default:
				    break;
			    }
			    ++CurPos;
			    break;
			}
			else {
			    if (LineFeeds > 0) linefix(d);
			    (d)->AddInCharacter( (d)->GetLength(), tmp);
			    ++End;
			    ++CurPos;
			    break;
			}
		    }
		    else {
			++i;
		    }
		}
		break;

	    default:
		if (LineFeeds > 0) linefix(d);
		(d)->AddInCharacter( (d)->GetLength(), tmp);
		++End;
		++CurPos;
		break;
	}
	tmp = (d)->GetChar( (CurPos + Offset));
    }

    if (LineFeeds == 1) ++LineFeeds;
    if (LineFeeds > 0) linefix(d);
    while (Top > -1) {
	if (finishenv()) {
	    return(1);
	}
    }
    return(0);
}

/* ** linefix - if single \n, output space, if multiple \n's, output n-1 \n's ** */
static void linefix(class text  *d)
{
    if (OldFormat) {
	if (LineFeeds == 1) {
	    (d)->AddInCharacter( (d)->GetLength(), ' ');
	    End++;
	    LineFeeds = 0;
	}
	else {
	    while (LineFeeds > 0) {
		(d)->AddInCharacter( (d)->GetLength(), '\n');
		End++;
		--LineFeeds;
	    }
	    LineFeeds = 0;
	}
    }
    else {
	if (LineFeeds == 1) {
	    (d)->AddInCharacter( (d)->GetLength(),' ');
	    End++;
	    LineFeeds = 0;
	}
	else {
	    while (LineFeeds > 1) {
		(d)->AddInCharacter( (d)->GetLength(),'\n');
		End++;
		--LineFeeds;
	    }
	    LineFeeds = 0;
	}
    }
    return;
}

/* ** scribefix - deal with @commands ** */
static int scribefix(class text  *d)
{
    int i, next, lowernext;
    char shortcommand[STRINGSIZE], realstring[STRINGSIZE];

    i = 0;
    ++CurPos;
    next = (d)->GetChar( (CurPos + Offset));
    switch (next) {
	case '@':
	    if (LineFeeds > 0) linefix(d);
	    (d)->AddInCharacter( (d)->GetLength(), '@');
	    End++;
	    ++CurPos;
	    break;

	 case '*':
	    ++LineFeeds;
	    ++CurPos;
	    break;

	 default:
	    while ((next!='{')&&(next!='[')&&(next!='(')&&(next!='<')&&
		   (next!='"')&&(next!='\'')&&(next!=' ')&&(next!='}')&&
		   (next!=']')&&(next!=')')&&(next!='>')&&(next!='\n') &&
		   i < STRINGSIZE){
		lowernext = (isupper(next)) ? tolower(next) : next;
		realstring[i] = next;
		shortcommand[i++] = lowernext;
		++CurPos;
		next = (d)->GetChar( (CurPos + Offset));
	    }
	    shortcommand[i] = '\0';
	    realstring[i] = '\0';

	    if ((next == ' ') || (next == '}') || (next == ']') || (next == ')') ||
		(next == '>') || (next == '\n') || (i == STRINGSIZE)){
		if (LineFeeds > 0) linefix(d);
		(d)->AddInCharacter( (d)->GetLength(),'@');
		(d)->AlwaysInsertCharacters( (d)->GetLength(),realstring, strlen(realstring));
		End += strlen(realstring) +1;
	    }
	    else if ((strcmp(shortcommand,SCRBEGIN)==0)||(strcmp(shortcommand,SCREND)==0)){
		if (longscribe(d, shortcommand)) return(1);
	    }
	    else {
		if (shortscribe(d, shortcommand)) return(1);
	    }
	    break;
    }
    return(0);
}

/* ** longscribe - deal with @begin and @end scribe environments ** */
static int longscribe(class text  *d, char  *shortcommand)
{
    int i;
    class style *tempstyle;
    int next, delim, delim2;
    char longcommand[STRINGSIZE], dstr[2], dstr2[2], sname[STRINGSIZE];

    i = 0;
    delim = (d)->GetChar( (CurPos + Offset));

    dstr[0] = delim;
    dstr[1] = '\0';

    ++CurPos;
    next = (d)->GetChar( (CurPos + Offset));

    while((next!='}')&&(next!=']')&&(next!= ')')&&(next!='>')&&(next!='"')&&(next!='\'')) {
	if (isupper(next)) {
	    next = tolower(next);
	}
	longcommand[i++] = next;
	++CurPos;
	next = (d)->GetChar( (CurPos + Offset));
    }
    longcommand[i]='\0';

    delim2 = (d)->GetChar( (Offset + CurPos));
    CurPos++;
    dstr2[0] = delim2;
    dstr2[1] = '\0';
    Mode = BEGEND;

    if (strlen(longcommand) == 0) {
	goshdarn("Environment field empty.");
	return(1);
    }

    if (strcmp(longcommand, "b") == 0) strcpy(sname, "bold");
    else if (strcmp(longcommand, "i") == 0) strcpy(sname, "italic");
    else if (strcmp(longcommand, "u") == 0) strcpy(sname, "underline");
    else strcpy(sname, longcommand);

    tempstyle = (d->styleSheet)->Find( sname);
    if (strcmp(shortcommand, SCRBEGIN) == 0) {
	if (tempstyle) {
	    if (startenv(delim, tempstyle)) {
		return(1);
	    }
	}
	else {
	    Mode = PT;
	    if (LineFeeds > 0) linefix(d);
	    (d)->AddInCharacter( (d)->GetLength(),'@');
	    (d)->AlwaysInsertCharacters( (d)->GetLength(),SCRBEGIN, 1);
	    (d)->AlwaysInsertCharacters( (d)->GetLength(),dstr, strlen(dstr));
	    (d)->AlwaysInsertCharacters( (d)->GetLength(),longcommand, strlen(longcommand));
	    (d)->AlwaysInsertCharacters( (d)->GetLength(),dstr2, strlen(dstr2));
	    End += strlen(dstr2) + strlen(longcommand) + strlen(dstr) + 2;
	}
    }
    else if (strcmp(shortcommand, SCREND) == 0) {
	if (Top >= 0 && Stack[Top]->CheckMode == BEGEND) {
	    if ((tempstyle) || (strcmp((Stack[Top]->StyleName)->GetName(),(tempstyle)->GetName()) == 0)) {
		if (LineFeeds > 0) linefix(d);
		if (finishenv()) {
		    return(1);
		}
	    }
	    else {
		if (LineFeeds > 0) linefix(d);
		(d)->AddInCharacter( (d)->GetLength(), '@');
		(d)->AlwaysInsertCharacters( (d)->GetLength(),SCREND, 1);
		(d)->AlwaysInsertCharacters( (d)->GetLength(),dstr, strlen(dstr));
		(d)->AlwaysInsertCharacters( (d)->GetLength(),longcommand, strlen(longcommand));
		(d)->AlwaysInsertCharacters( (d)->GetLength(),dstr2, strlen(dstr2));
		End += strlen(dstr2) + strlen(longcommand) + strlen(dstr) + 2;
		if (finishenv()) {
		    return(1);
		}
	    }
	}
	else if (Top >= 0 && Stack[Top]->CheckMode == PT) {
	    if (LineFeeds > 0) linefix(d);
	    (d)->AddInCharacter( (d)->GetLength(), '@');
	    (d)->AlwaysInsertCharacters( (d)->GetLength(),SCREND, 1);
	    (d)->AlwaysInsertCharacters( (d)->GetLength(),dstr, strlen(dstr));
	    (d)->AlwaysInsertCharacters( (d)->GetLength(),longcommand, strlen(longcommand));
	    (d)->AlwaysInsertCharacters( (d)->GetLength(),dstr2, strlen(dstr2));
	    End += strlen(dstr2) + strlen(dstr) + strlen(longcommand) + 2;
	    if (finishenv()) {
		return(1);
	    }
	}
	else {
	    goshdarn("Unmatched delimiters.");
	    return(1);
	}
    }
    else {
	goshdarn("Using longscribe for a shortscribe command.");
	return(1);
    }
    return(0);
}


/* ** shortscribe - deal with all other environments ** */
static int shortscribe(class text  *d, char  *shortcommand)
{
    class style *tempstyle;
    int delim;
    char dstr[2], sname[STRINGSIZE];

    delim = (d)->GetChar( (Offset + CurPos));
    CurPos++;
    dstr[0] = delim;
    dstr[1] = '\0';

    if (strcmp(shortcommand, "b") == 0) strcpy(sname, "bold");
    else if (strcmp(shortcommand, "i") == 0) strcpy(sname, "italic");
    else if (strcmp(shortcommand, "u") == 0) strcpy(sname, "underline");
    else strcpy(sname, shortcommand);

    if ((tempstyle = (d->styleSheet)->Find( sname))) {
	Mode = NORM;
	if (startenv(delim, tempstyle)) {
	    return(1);
	}
    }
    else {
	Mode = PT;
	if (LineFeeds > 0) linefix(d);
	(d)->AddInCharacter( (d)->GetLength(),'@');
	(d)->AlwaysInsertCharacters( (d)->GetLength(),shortcommand, strlen(shortcommand));
	(d)->AlwaysInsertCharacters( (d)->GetLength(),dstr, strlen(dstr));
	End += strlen(dstr) + strlen(shortcommand) + 1;
    }
    return(0);
}

/* ** startenv - add an environment to the stack ** */
static int startenv(int  delim, class style  *tempstyle)
{
    int rpos;

    if (Top++ >= STACKSIZE) {
	goshdarn("Environment stack overflow.");
	return(1);
    }
    Stack[Top] = (struct StackItem *)malloc(sizeof(struct StackItem));
    Stack[Top]->Parent = Par;
    Stack[Top]->StyleName = tempstyle;
    Stack[Top]->Delimiter = delim;
    Stack[Top]->Position = End;
    Stack[Top]->CheckMode = Mode;

    rpos = End - PPos;
    Par = (Par)->InsertStyle( rpos, tempstyle, 1);
    (Par)->SetStyle( FALSE, FALSE);
    PPos = End;
    return(0);
}

/* ** finishenv - remove an environment from the stack ** */
static int finishenv()
{
    int length;

    if (Top <= -1) {
	goshdarn("Environment stack overflow.");
	return(1);
    }
    length = End - PPos;
    (Par)->SetLength( length);
    Par = Stack[Top]->Parent;
    if (--Top >= 0) PPos = Stack[Top]->Position;
    else PPos = 0;
    free(Stack[Top+1]);
    return(0);
}

