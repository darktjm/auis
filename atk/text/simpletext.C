/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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

#include <andrewos.h> /* sys/types.h */

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/text/RCS/simpletext.C,v 3.12 1995/11/07 20:17:10 robr Stab74 $";
#endif


ATK_IMPL("simpletext.H")

#include <sys/stat.h>
#include <errno.h>
#include <attribs.h>

#include <dataobject.H>
#include <dictionary.H>
#include <environ.H>	/* for datastream test only */
#include <mark.H>

#include <simpletext.H>

#define INITIALSTRINGSIZE 100
#define ADDITIONALSIZE 50
#define INITIALKEYWORDSIZE 30

/* These values are used for the read routine */

static char *keyword;		/* Keyword buffer - initialize once when first
				encountering a keyword in the 
				ReadSubString routine */
static long keywordlength;	/* Current length of the keyword buffer */

#define DEFAULTDATASTREAMNUMBER 12
static int DataStreamVersion = 0;

#define CR '\r'
#define LF '\n'
static boolean DOSplatform= FALSE; /* only relevant for NEW files; existing files are written with CRs only if they were in the file when it was read */
static boolean CheckForDOSText=FALSE;

ATKdefineRegistry(simpletext, dataobject, simpletext::InitializeClass);
static boolean EnsureSize(class simpletext  *self, long  neededGap);
static void MoveGap(class simpletext  *self, long  pos);
static boolean simplewrite(FILE  *file,char  *p,long  len);


boolean simpletext::InitializeClass()
{
#ifdef DOS_PLATFORM
    DOSplatform= environ::GetProfileSwitch("WriteForDOS", TRUE);
#else /*DOS_PLATFORM*/
    DOSplatform= environ::GetProfileSwitch("WriteForDOS", FALSE);
#endif /*DOS_PLATFORM*/
    CheckForDOSText= environ::GetProfileSwitch("CheckForDOSFormat", environ::GetProfileSwitch("CheckForDOSText", DOSplatform)); /* setting either preference to FALSE will cause CR-laden DOS files to appear as-is ("doublespaced") */
    return TRUE;
}

simpletext::simpletext()
{
    ATKinit;
    this->string = (char *) malloc(INITIALSTRINGSIZE);
    this->lowSize = 0;
    this->gapSize = INITIALSTRINGSIZE;
    this->length = 0;
    this->markList = NULL;
    this->pendingReadOnly = FALSE;
    this->fence = (this)->CreateMark( 0, 0);
    this->objectInsertionAllowed = TRUE;
    (this->fence)->SetStyle( TRUE, FALSE);

    if(DataStreamVersion == 0){
	const char *buf;

	if((buf = environ::GetConfiguration("BE2TextFormat")) !=  NULL && *buf != '\0')
	    DataStreamVersion = atoi(buf);

	if(DataStreamVersion < 10)
	    DataStreamVersion = DEFAULTDATASTREAMNUMBER;
    }

    this->highbitflag=(-1);
    this->DOSfile= -1;
    checkDOSFormat=CheckForDOSText;
    THROWONFAILURE( TRUE);
}

simpletext::~simpletext()
{
    class mark *mark;
    
    if (this->markList != NULL)  {
	for (mark = this->markList; mark != NULL; mark = (mark)->GetNext())
	    (mark)->SetObjectFree( TRUE);
    }
    delete this->fence;
    if (this->string)
	free(this->string);
}

#define SIZEINCREMENT 200 /* Size to increase pad a "large" space request by. */
#define MAXGAP (1<<15)	/* maximum gap size */

static boolean EnsureSize(class simpletext  *self, long  neededGap)
{
    long totalSize;
    long newGapSize;
    long count;
    char *s, *t;

    if (neededGap <= self->gapSize)
        return TRUE;

    /* If request more than doubles total buf size,  set length to */
    /* requested size plus some.  Otherwise,  double length.  We */
    /* think this works well for files <50k but may degrade above that.  */

    /* 1995: limit the increase to a gap of no more than 32k, 
	unless more is needed  -wjh */

    newGapSize = self->length + 2 * self->gapSize;
    if (newGapSize > MAXGAP)  newGapSize = MAXGAP;  /* limit size */
    if (neededGap >= newGapSize)
        newGapSize = neededGap + SIZEINCREMENT;

    totalSize = self->length + newGapSize;

    /* Be careful about the realloc */

    s = (char *)realloc(self->string, totalSize); 
    if (s == NULL)
        return FALSE;
    self->string = s;

    /* Move the text upwards in memory by the amount necessary */
    /* to increase the gap size to newGapSize. */

    count = self->length - self->lowSize;
    t = self->string + self->length + self->gapSize;
    s = self->string + totalSize;
    memmove(s-count, t-count, count);
    self->gapSize = newGapSize;
    return TRUE;
}

static void MoveGap(class simpletext  *self, long  pos)
{
    register long amount;
    char *s, *t;

    if (pos > self->lowSize) {
        amount = pos - self->lowSize;
        s = self->string + self->lowSize;
        t = self->string + self->lowSize + self->gapSize;
        self->lowSize += amount;
	memmove(s, t, amount);
    } else if (pos < self->lowSize) {
        amount = self->lowSize - pos;
        s = self->string + self->lowSize + self->gapSize;
        t = self->string + self->lowSize;
        self->lowSize -= amount;
	memmove(s-amount, t-amount, amount);
    }
}

/*
This is a define here to a the code to call this routine to set the marks.
but not confuse the reader of the code in __Read to believe that it will call
the LengthChanged method.
*/

#define UPDATEMARKS(self, pos, size) self->simpletext::LengthChanged(pos, size)

void simpletext::LengthChanged(long  pos, long  size)
{
    (this->markList)->UpdateMarks( pos, size);
}

boolean simpletext::GetReadOnly()
    {
    return ((this)->GetFence() > (this)->GetLength());
}

void simpletext::SetReadOnly(boolean  readOnly)
        {
    if (readOnly)
        (this)->SetFence( (this)->GetLength() + 1);
    else
        (this)->ClearFence();
}

void simpletext::SetAttributes(struct attributes  *attributes)
        {

    (this)->dataobject::SetAttributes( attributes);

    this->pendingReadOnly = FALSE;

    while (attributes) {
        if (strcmp(attributes->key, "filesize") == 0)
            EnsureSize(this, attributes->value.integer - this->length);
        else if (strcmp(attributes->key, "readonly") == 0) {
            this->pendingReadOnly = attributes->value.integer;
            (this)->SetReadOnly( attributes->value.integer);
        } else if (strcmp(attributes->key, "write-for-DOS") == 0)
	    this->DOSfile= atoi(attributes->value.string);
        attributes = attributes->next;
    }
}

/* delCR removes carriage returns from the specified region, and returns the number it removed. */
static int delCR(simpletext *self, long start, long len)
{
    int count= 0;
    long pos= start;
    long mod= self->GetModified();
    do  {
	pos= self->Index(pos, CR, len-(pos-start));
	if (pos>0 && self->GetChar(pos+1)==LF) {
	    self->AlwaysDeleteCharacters(pos,1);
	    ++count;
	    --len;
	} else ++pos;
    } while (pos>0);
    self->RestoreModified(mod);
    return count;
}

long simpletext::Read(FILE  *file, long  id)
{
    int oldLength = this->length;

    this->highbitflag=(-1);
    
    (this)->ClearFence();

    /* Code is copied from simpletext_Clear so we can preserve mark state. */
    this->gapSize += this->length;
    this->lowSize = 0;
    this->length = 0;
    this->id = (this)->UniqueID();
    if (id == 0 && fileno(file) >= 0)  {
	struct stat buf;

	fstat(fileno(file), &buf);

	/* Make sure the file is a regular file. If it isn't stat can't return any useful information about it's size. */

	if ((buf.st_mode & S_IFMT) == S_IFREG) {
	    long readlen;
	    long len;
	    long result;
	    long pos;
	    register unsigned char *p;

	    /* Just in case you are passed a file that has already been read determine the current position, reseek to that position - this should clear out the data stored in the buffer - and then set length to the remaining part of the file. */

	    pos = ftell(file);
	    fseek(file, pos, 0L);
            lseek(fileno(file), pos, 0L);
	    len = buf.st_size - pos;

	    EnsureSize(this, len);
	    readlen = len;
	    /* Start reading in data at the beginning of the gap. */
	    p = (unsigned char *) &this->string[this->lowSize];
	    while (readlen > 0)  {
		result = read(fileno(file), p, readlen);
		if (result == -1)  {
		    if (errno != EINTR)  {
			fprintf(stderr, "Error reading text object - ignoring text\n");
			len = 0;
			break;
		    }
		}
		else if (result == 0)  {
		    len -= readlen;
		    break;
		}
		else  {
		    readlen -= result;
		    p = &p[result];
		}
	    }
	    this->lowSize = len;
	    this->length = len;
	    this->gapSize -= len;
	    fseek(file, len, 0);
	}
	else  {
	    /* The file is pipe or something similar. Just keep reading until EOF. We have to use STDIO because there is no way to re-sync STDIO with the actual stream if we use read. */
	    int count;

	    do {
		/* Make sure there is some space. */
		EnsureSize(this, 1);

		count = fread(this->string + this->lowSize, 1, this->gapSize, file);
		this->length += count;
		this->lowSize += count;
		this->gapSize -= count;
	    } while (count > 0);
	}
	if (this->checkDOSFormat) {
	    if (GetChar(this->length-2)==CR || GetChar(this->length-3)==CR || GetChar(GetBeginningOfLine(this->length)-2)==CR) {
		/* file ends with CR,LF or CR,LF,^Z, or the last line isn't terminated but the preceding line has a CR on it */
		delCR(this, 0, this->length);
		if (this->DOSfile<0) /* not being forced */
		    this->DOSfile= 1;
	    } else
		if (this->DOSfile<0) /* not being forced */
		    this->DOSfile= 0;
	} else
	    this->DOSfile= 0;
    }
    else  {
	(this)->ReadSubString( 0, file, (id > 0));
	UPDATEMARKS(this, 0, -this->length); /* Bring them all back down. */
    }
    if (this->length < oldLength) /* Bring all marks into range if neccesary. */
	UPDATEMARKS(this, this->length, this->length - oldLength);
    (this)->RegionModified( 0, this->length);

    if (this->pendingReadOnly) {
	(this)->SetFence( (this)->GetLength() + 1);
	this->pendingReadOnly = FALSE;
    }
    return dataobject_NOREADERROR;
}
static boolean simplewrite(FILE  *file,char  *p,long  len)
{
    /*
      Write out the literal contents of the character buffer.
      write() is used because it is more efficient than fwrite.
      if write() fails, attempt with fwrite (),
	  if it succeeds then continue,
	      if it fails, then when buffer calls ferror, 
		  it will notice that something has gone wrong 
		  */
    long wroteLen;
    while (len > 0)  {
	wroteLen = write(fileno(file), p, len);
	if (wroteLen == -1) {
	    if (errno == EINTR)  continue;
	    if((wroteLen = fwrite(p,1,len,file)) <= 0)	{
		fprintf(stderr, "Error while writing text object.\n");
		return FALSE;
	    }
	    fflush(file);
	}
	len -= wroteLen;
	p = &p[wroteLen];
    }
    return TRUE;
}
long simpletext::Write(FILE  *file, long  writeID, int  level)
{
    if (this->writeID != writeID)  {
	this->writeID = writeID;
	if (level != 0)  {
	    fprintf(file, "\\begindata{%s,%ld}", (this)->GetTypeName(), (this)->UniqueID()); 
	    (this)->WriteSubString( 0, this->length, file, level != 0);
	    fprintf(file, "\\enddata{%s,%ld}", (this)->GetTypeName(), (this)->UniqueID());
	    fflush(file);
	}
	else  {
	    fflush(file);
	    if (this->DOSfile>0 || (this->DOSfile<0 && DOSplatform)) {
		/* write as plain ASCII, with CR's and LF's */
		long pos=0, len=this->length, actuallen;
		static char doslinebrk[2];
		doslinebrk[0]= CR; doslinebrk[1]= LF;
		do  {
		    char *plainbuffer= GetBuf(pos, len-pos, &actuallen);
		    long lfpos= Index(pos, LF, len-pos);
		    if (lfpos>0 && lfpos-pos<=actuallen && plainbuffer[lfpos-pos-1]!=CR) {
			actuallen= lfpos-pos;
			simplewrite(file, plainbuffer, actuallen);
			simplewrite(file, doslinebrk, 2); /* linefeed AND carriage return */
			pos+= actuallen +1 /* skip the LF we just wrote */;
		    } else {
			simplewrite(file, plainbuffer, actuallen);
			pos+= actuallen;
		    }
		} while (pos<len);
	    } else
		/* write as plain ASCII, with just LF's */
		if(simplewrite(file,this->string,this->lowSize)) 
		    simplewrite(file, &(this->string[this->lowSize + this->gapSize]),this->length - this->lowSize);
	}
    }
    return (this)->UniqueID();
}

void simpletext::Clear()
{
    this->highbitflag=(-1);
    if (this->length == 0) return;
    this->gapSize += this->length;
    this->lowSize = 0;
    (this)->SetModified();
    (this)->LengthChanged( 0, -this->length);
    this->length = 0;
}

boolean simpletext::InsertCharacters(long  pos, const char  *str, long  len)
{
    if (pos >= (this)->GetFence()) {
        (this)->AlwaysInsertCharacters( pos, str, len);
	return TRUE;
    }
    else
        return FALSE;
}

void simpletext::AlwaysInsertCharacters(long  pos, const char  *str, long  len)
{
    register long i;
    register const char *s;
    register char *t;

    if (len == 0)
	return;
    
    this->highbitflag=(-1);
    
    if (pos > this->length)
	pos = this->length;

    if (this->gapSize < len)
	EnsureSize(this, len);
    if (pos != this->lowSize)
	MoveGap(this, pos);
    for (i = len, s = str, t = &(this->string[this->lowSize]); i > 0; i--, s++, t++)
	*t = *s;
    this->lowSize += len;
    this->gapSize -= len;
    this->length += len;
    (this)->SetModified();
    (this)->LengthChanged( pos, len);
}

boolean simpletext::DeleteCharacters(long  pos, long  len)
{    
    if (pos >= (this)->GetFence()) {
        (this)->AlwaysDeleteCharacters( pos, len);
	return TRUE;
    }
    else
        return FALSE;
}

void simpletext::AlwaysDeleteCharacters(long  pos, long  len)
{
    
    this->highbitflag=(-1);
    
    if (len == 0)
        return;
    if (pos + len > this->length)
	len = this->length - pos;

    MoveGap(this, pos);
    this->length -= len;
    this->gapSize += len;
    (this)->SetModified();
    (this)->LengthChanged( pos, -len);
}

boolean simpletext::ReplaceCharacters(long  pos , long  len, char  *replacementString, long  replacementLen)
                {

    if (pos >= (this)->GetFence()) {
        (this)->AlwaysReplaceCharacters( pos, len, replacementString, replacementLen);
	return TRUE;
    }
    else
        return FALSE;
}

void simpletext::AlwaysReplaceCharacters(long  pos , long  len, char  *replacementString, long  replacementLen)
                {

    int i;
    
    this->highbitflag=(-1);
    

    if (replacementLen > len) {	/* Put gap after characters to replace, leaving space for extras. */
        EnsureSize(this, this->length + replacementLen - len); /* make sure there's enough room */
        MoveGap(this, pos + len);
    }
    else			/* Put gap before extra characters to delete. */
        MoveGap(this, pos + replacementLen);

/* Insert replacement text. */
    for (i = 0; i < replacementLen; i++)
        this->string[i + pos] = *replacementString++;

/* Adjust document sizes appropriately. */
    if (replacementLen > len) {
        this->lowSize += replacementLen - len; /* Eat some of the gap. */
        this->gapSize -= replacementLen - len; /* Make the gap smaller. */
    }
    else
        this->gapSize += len - replacementLen; /* Into the void (gap) goes the text. */
    this->length += replacementLen - len;

/* Hopefully return everything to a normal state. */
    if (replacementLen > len)
        (this)->LengthChanged( pos + len, replacementLen - len);
    else
        (this)->LengthChanged( pos + replacementLen, replacementLen - len);

/* Set modified bits on marks that have been touched. */
    (this)->RegionModified( pos, replacementLen);
    (this)->SetModified();
}

long simpletext::GetLength()
{
    return this->length;
}

long simpletext::GetChar(long  pos)
{
    if (pos >= this->length || pos < 0) return EOF;
    if (pos < this->lowSize) return this->string[pos];
    return this->string[pos+this->gapSize];
}

long simpletext::GetUnsignedChar(long  pos)
{
    if (pos >= this->length || pos < 0) return EOF;
    if (pos < this->lowSize) return ((unsigned char *)(this->string))[pos];
    return ((unsigned char *)(this->string))[pos+this->gapSize];
}
#undef MIN
#define MIN(a,b) ((a)<=(b)?(a):(b))

/*
 * GetBuf returns a pointer to a buffer containing any amount of
 * characters up to the number requested.  The amount is placed
 * at lenp.  Returns NULL if pos is out of range.  This call is used
 * for purposes such as writing the buffer to a file in chunks, or
 * searching forwards.
 *
 * GetBufEnd is similar to GetBuf, except backwards:  given an end
 * position, it tries to get a chunk of text to the left of that position
 * as large as it can up to the specified size.  It returns a pointer to
 * one past the end of the chunk.  The returned region extends
 * leftward by the amount stored at lenp.  Returns NULL if endpos is
 * out of range.  This call is used for purposes such as searching
 * backwards.
 *
 * GetGap inserts a gap in a buffer at the given position and of
 * the given length.  It returns a pointer to this contiguous area.  In
 * the event of an error, it returns NULL.  Note that the area is
 * uninitialized and so it is up to the caller to write reasonable data
 * into every byte of the new gap.  This call is used for purposes such
 * as reading a whole file or chunks of a file directly into a buffer.
 */

char *simpletext::GetBuf(long  pos , long  length, long  *lenp)
{
    if (pos >= 0 && length > 0) {
        long len = this->lowSize - pos;

        if (len > 0) {
            *lenp = MIN(length, len);
            return this->string + pos;
        }

        len = this->length - pos;

        if (len > 0) {
            *lenp = MIN(length, len);
            return this->string + this->gapSize + pos;
        }
    }

    *lenp = 0;
    return NULL;
}

char *simpletext::GetBufEnd(long  endpos , long  length , long  *lenp)
{
    if (endpos >= 0 && length > 0) {
        if (endpos <= this->lowSize) {
            *lenp = MIN(length, endpos);
            return this->string + endpos;
        }

        *lenp = MIN(length, endpos - this->lowSize);
        return this->string + this->gapSize + endpos;
    }

    *lenp = 0;
    return NULL;
}

char *simpletext::GetGap(long  pos , long  len)
{
    if (pos < 0 || pos > this->length || len < 0)
        return NULL;

    /* Make sure the buffer gap is as big as the requested gap. */
    /* Move the buffer's gap to the requested position. */
    /* Transfer the requested size from the gap to the string. */

    if (this->gapSize < len)
        if (EnsureSize(this, len) == FALSE)
            return NULL;

    if (this->lowSize != pos)
        MoveGap(this, pos);

    this->lowSize += len;
    this->gapSize -= len;
    this->length += len;

    return this->string + pos;
}

long simpletext::GetPosForLine(long  line)
{
    long len = (this)->GetLength();
    long pos = 0;
    long i;

    i = 1;
    while (i < line) {
	if (pos >= len)
            break;
	if ((this)->GetChar( pos) == '\012')
            i++;
        pos++;
    }
    return pos;
}

long simpletext::GetLineForPos(long  pos)
{
    long line=1;

    while (--pos >= 0)
       if ((this)->GetChar(pos) == '\012')
            line++;

    return line;
}

long simpletext::GetBeginningOfLine(long  pos)
{
    while (pos > 0 && (this)->GetChar( pos-1) != '\n') {
	pos--;
    }
    return pos;
}

long simpletext::GetEndOfLine(long  pos)
{
    long len = (this)->GetLength();

    while (pos < len) {
	if ((this)->GetChar( pos) == '\n') {
	    break;
	}
	pos++;
    }
    return pos;
}


void simpletext::AddInCharacter(long  pos, char  c)
            {
    this->highbitflag=(-1);
    if (this->gapSize == 0)
	EnsureSize(this, ADDITIONALSIZE);
    if (pos != this->lowSize)
	MoveGap(this, pos);
    this->string[this->lowSize++] = c;
    this->gapSize -= 1;
    this->length += 1;
}

long simpletext::HandleDataObject(long  pos, class dataobject  *dop, FILE  *file  )
{
    return 0;
}

long simpletext::HandleKeyWord(long  pos, char  *keyword, FILE  *file)
{
    long len = strlen(keyword);

    this->highbitflag=(-1);
    (this)->InsertCharacters( pos, "\\", 1);
    pos += 1;
    (this)->InsertCharacters( pos, keyword, len);
    pos += len;
    (this)->InsertCharacters( pos, "{", 1);
    return len + 2;
}


long simpletext::HandleCloseBrace(long  pos, FILE  *file)
{
    (this)->InsertCharacters( pos, "}", 1);
    return 1;
}

#define TESTEND(C) (C == EOF )
#define GETANDTEST(C,file) ((C = getc(file)) != EOF )

long simpletext::ReadSubString(long  pos, FILE  *file, boolean  quoteCharacters)
{
    register long c;
    long addedcharacters = 0;
    long keywordpos = 0;
    register int addedspace = -1;
    register long lastadd = -1;
    long endcount = 1;		/* Number of enddata markers to be 
				encountered before completion
 */
    boolean canInsert;
    
    this->highbitflag=(-1);
    
    if(quoteCharacters)
	this->Version = DataStreamVersion;
    else
        this->Version = 0;

    if ((!(canInsert = (pos >= (this)->GetFence()))) && ! quoteCharacters)  {
	fseek(file, 0L, 2);
	return -1;
    }

    (this)->LengthChanged( pos, 1000000000);
    MoveGap(this, pos);    
    while (endcount != 0 && GETANDTEST(c,file))  {
	if (quoteCharacters && (c == '}' || c == '\\'))  {
	    if (c == '}')  {
		if (canInsert)
		    addedcharacters += (this)->HandleCloseBrace( this->lowSize, file);
		continue;
	    }
	    else  { /* handle backslash */
		if (!GETANDTEST(c,file))
		    break;
		if(c == '\n') continue; /* ignore <backslash><newline> pair */
		if (c != '\\' && c != '{' && c != '}')  {
		   /*  Have a keyword here */
		    if (keyword == NULL)  {
			keywordlength = INITIALKEYWORDSIZE;
			keyword = (char *) malloc(INITIALKEYWORDSIZE);
		    }
		    keywordpos = 1;
		    *keyword = c;
		    while (GETANDTEST(c,file))  {
			if (c == '{')  {
			    keyword[keywordpos] = '\0';
			    if (strcmp(keyword, "begindata") == 0)  {
				if (canInsert)  {
				    int rt;

				    rt = (this)->HandleBegindata(pos,file);
				    switch(rt) {
					case -1: /* eof encountered */
					    endcount = 0;
					    c = EOF;
					    break;
					case -2: /* couldn't find object */
					    endcount += 1;
					    break;
					default:
					    if(addedcharacters == lastadd)
						lastadd += rt;
					    addedcharacters += rt;
				    }
				}
				else
				    endcount += 1;
			    }
			    else if (strcmp(keyword, "enddata") == 0)  {
				while (GETANDTEST(c,file) && c != '}');
				if (GETANDTEST(c, file) && c != '\n')
				    ungetc(c, file);
				endcount -= 1;
			    }
			    else if (canInsert){
				long foo;
				foo = (this)->HandleKeyWord( this->lowSize, keyword, file);
				if(foo){
				    if(addedcharacters == lastadd && addedspace == -2)
					lastadd += foo; 
				    addedcharacters += foo;
				}
			    }
			    keywordpos = 0;
			    break;
			}
			else  {
			    if (keywordpos > keywordlength - 2)  {
				keywordlength *= 2;
				keyword = (char *) realloc(keyword, keywordlength); 
			    }
			    keyword[keywordpos++] = c;
			}
		    }
		    if (TESTEND(c) && keywordpos != 0 && canInsert)  {
		/* 	Incomplete keyword - just place in the text */
			keyword[keywordpos] = '\0';
			fprintf(stderr, "Found End of File while reading in the keyword, %s\n", keyword);
			fprintf(stderr, "Placing keyword in as just text.\n");
			(this)->InsertCharacters( this->lowSize, "\\", 1);
			(this)->InsertCharacters( this->lowSize, keyword, strlen(keyword));
			}
		    if(TESTEND(c) ) endcount = 0;
		    continue;
		}
	    }
	}
	if (canInsert) {
	    if (this->gapSize == 0)
		EnsureSize(this, ADDITIONALSIZE);
	    if(this->Version > 11){
		if(c == '\n')  {
		    if(addedcharacters != lastadd){
			if(this->lowSize > 0 && this->string[this->lowSize - 1] != ' ' && this->string[this->lowSize - 1] != '\t'){
			    /* replace first newline , not proceeded with a space, with a space */
			    c = ' ';
			    addedspace = addedcharacters + 1;
			}
			else {
			    addedspace = -1;
			    lastadd = addedcharacters;
			    continue;
			}
		    }
		    else if(addedspace == addedcharacters){
			/* The space inserted above really was a newline */
			this->string[this->lowSize - 1] = '\n';
			addedspace = -2;
			continue;
		    }
		    else addedspace = -2;
		    lastadd = addedcharacters + 1;
		}
	    }
	    this->string[this->lowSize++] = c;
	    this->gapSize -= 1;
	    this->length += 1;
	    addedcharacters += 1;
	}
    }
    if (addedcharacters)  {
	(this)->SetModified();
	(this)->LengthChanged( pos+addedcharacters, addedcharacters);
    }
    (this)->LengthChanged( pos+addedcharacters, -1000000000);
    if (this->checkDOSFormat) { /* strip CRs if inserted file was DOS-format */
	long crpos= Index(pos, CR, addedcharacters);
	if (crpos!=EOF && GetChar(crpos+1)==LF)
	    addedcharacters-= delCR(this, pos, addedcharacters);
    }
    return addedcharacters;
}

long simpletext::HandleBegindata(long  pos,FILE  *file)
{
	char objectname[200];
        int c;
	long objectid;
	char *s;
	class dataobject *newobject;

	this->highbitflag=(-1);	    
	s = objectname;
	while (GETANDTEST(c,file) && c != ',')
	    *s++ = c;
	if (TESTEND(c))  {
	    fprintf(stderr, "End of File encountered while reading in a begindata marker - ignoring\n");
	    return(-1);
	}
	*s = '\0';
	objectid = 0;
	while (GETANDTEST(c,file) && c != '}')
	    if(c >= '0' && c <= '9')objectid = objectid * 10 + c - '0';
	if (TESTEND(c))  {
	    fprintf(stderr, "End of File encountered while reading in a begindata marker - ignoring\n");
	    return(-1);
	}
	if((c = getc(file))!= '\n')ungetc(c,file);

/* 	Call the New routine for the object */
	if ((newobject = (struct dataobject *) ATK::NewObject(objectname))==NULL)  {
	    newobject = (struct dataobject *)ATK::NewObject("unknown");
	}
	if (newobject)  {
	    /* Setup readonly state for object. */
	    if (this->pendingReadOnly) {
		struct attributes readOnlyAttr;

		readOnlyAttr.key = "readonly";
		readOnlyAttr.value.integer = TRUE;
		readOnlyAttr.next = NULL;
		(newobject)->SetAttributes( &readOnlyAttr);
	    }
	    /* 	    Call the read routine for the object */
	    (newobject)->Read( file,objectid);
	    if((c = getc(file))!= '\n')ungetc(c,file);
	    /*     At this point , the object pointer is the new object id */
	    if((this)->GetObjectInsertionFlag() == FALSE){
		/* ignore non-text object */
		fprintf(stderr,
			"Insertion of objects not allowed, ignoring %s!\n", objectname);
		(newobject)->Destroy();
		return(0l);
	    }
	    else {
		/* 	    Register the object with the dictionary */
		dictionary::Insert(NULL,(char *)objectid,(char *) newobject);
		/* Note: it is assumed that unless a call to Reference is made subsequently it is safe to destroy newobject. */
		(newobject)->UnReference();
		return((this)->HandleDataObject( this->lowSize, newobject, file));
	    }
	}
	else {
	    fprintf(stderr, "Could not find data object %s - ignoring\n", objectname);
	    return(-2);
	}
}

boolean simpletext::CopyTextExactly(long pos, class simpletext *srctext, long srcpos, long len)
{
    return CopyText(pos, srctext, srcpos, len);
}

void simpletext::AlwaysCopyTextExactly(long pos, class simpletext *srctext, long srcpos, long len)
{
    AlwaysCopyText(pos, srctext, srcpos, len);
}

boolean simpletext::CopyText(long  pos,class simpletext  *srctext,long  srcpos,long  len)
                    {
    if (pos >= (this)->GetFence()) {
	(this)->AlwaysCopyText(pos,srctext,srcpos,len);
	return TRUE;
    }
    else
        return FALSE;
}

void simpletext::AlwaysCopyText(long  pos,class simpletext  *srctext,long  srcpos,long  len)
                    {
    register long i;
    register char *s;
    register char *t;
    register long remlen;

    if (len == 0)
	return;
    
    this->highbitflag=(-1);
    
    if (pos > this->length)
	pos = this->length;

    if(srctext->length < srcpos + len)
	len = srctext->length - srcpos;
    if (this->gapSize < len)
	EnsureSize(this, len);
    if (pos != this->lowSize)
	MoveGap(this, pos);
    
    i = remlen =  len;
    t = &(this->string[this->lowSize]);

    if (srcpos < srctext->lowSize)  {
	if ((remlen = (srcpos + len - srctext->lowSize)) < 0)
	    remlen = 0;
	else
	    i = srctext->lowSize - srcpos;
 	s = &(srctext->string[srcpos]);
 	memmove(t, s, i);
 	t += i;
	srcpos = srctext->lowSize;
    }
    if (remlen > 0)  {
 	s = &(srctext->string[srcpos + srctext->gapSize]);
 	memmove(t, s, remlen);
    }
    this->lowSize += len;
    this->gapSize -= len;
    this->length += len;
    (this)->SetModified();
    (this)->LengthChanged( pos, len);
    return;
}

long simpletext::Index(long  pos,char  c,long  len)
{
    register long remlen = len;
    register char *s, *fs;
    if( len + pos > this->length || pos < 0) return EOF;
     
    if (pos < this->lowSize)  {
	if ((remlen = (pos + len - this->lowSize)) < 0)
	    remlen = 0;
	else
	    len = this->lowSize - pos;
	for (fs = s = &(this->string[pos]); len > 0; s++, len--)  {
	    if(c == *s) return pos + (s - fs) ;
	}
	pos = this->lowSize;
    }
    if (remlen > 0)  {
	for (fs = s = &(this->string[pos + this->gapSize]); remlen > 0; remlen--, s++)  {
	    if(c == *s) return pos + (s - fs);
	}
    }
    return EOF ;
}

int simpletext::Strncmp(long  pos,char  *str,long  len)
{
    register long remlen = len;
    register char *s;
    if( len + pos > this->length || pos < 0) return EOF;
      
    if (pos < this->lowSize)  {
	if ((remlen = (pos + len - this->lowSize)) < 0)
	    remlen = 0;
	else
	    len = this->lowSize - pos;
	for (s = &(this->string[pos]); len > 0; s++, len--,str++)  {
	    if(*str != *s) return *s - *str;
	}
	pos = this->lowSize;
    }
    if (remlen > 0)  {
	for ( s = &(this->string[pos + this->gapSize]); remlen > 0; remlen--, s++,str++)  {
	    if(*str != *s) return *s - *str;
	}
    }
    return 0 ;
}

#define CharAt(self,pos) ((pos < self->lowSize)? self->string[pos] : self->string[pos+self->gapSize])

int simpletext::Textncmp(long  pos,class simpletext  *text,long  pos2,long  len)
{
    register long remlen = len;
    register char *s;
    if(text == NULL || len + pos > this->length || pos < 0 || pos2 < 0 || len+pos2 > text->length) return EOF;

    if (pos < this->lowSize)  {
	if ((remlen = (pos + len - this->lowSize)) < 0)
	    remlen = 0;
	else
	    len = this->lowSize - pos;
	for (s = &(this->string[pos]); len > 0; s++, len--,pos2++)  {
	    if(CharAt(text,pos2) != *s) return *s - CharAt(text,pos2);
	}
	pos = this->lowSize;
    }
    if (remlen > 0)  {
	for ( s = &(this->string[pos + this->gapSize]); remlen > 0; remlen--, s++,pos2++)  {
	    if(CharAt(text,pos2) != *s) return *s - CharAt(text,pos2);
	}
    }
    return 0 ;
}

void simpletext::WriteSubString(long  pos, long  len, FILE  *file, boolean  quoteCharacters)
{
    long remlen = len;
    char *s;

    if (pos < this->lowSize)  {
	if ((remlen = (pos + len - this->lowSize)) < 0)
	    remlen = 0;
	else
	    len = this->lowSize - pos;
        /* These cases should be unrolled */
	for (s = this->string + pos; len > 0; len--, s++)  {
	    if (quoteCharacters && (*s == '\\' || *s == '{' || *s == '}'))
		putc('\\', file);
            putc(*s, file);
	}
	pos = this->lowSize;
    }
    if (remlen > 0)  {
	for (s = &(this->string[pos + this->gapSize]); remlen > 0; remlen--, s++)  {
	    if (quoteCharacters && (*s == '\\' || *s == '{' || *s == '}'))
		putc('\\', file);
            putc(*s, file);
	}
    }
 }

void simpletext::CopySubString(long  pos, long  len, char  *buf, boolean  quoteCharacters)
{
    long remlen = len;
    char *s;

    if (pos < this->lowSize)  {
	if ((remlen = (pos + len - this->lowSize)) < 0)
	    remlen = 0;
	else
	    len = this->lowSize - pos;
	for (s = &(this->string[pos]); len > 0; s++, len--)  {
	    if (quoteCharacters && (*s == '\\' || *s == '{' || *s == '}'))
		*buf++ = '\\';
	    *buf++ = *s;
	}
	pos = this->lowSize;
    }
    if (remlen > 0)  {
	for (s = &(this->string[pos + this->gapSize]); remlen > 0; remlen--, s++)  {
	    if (quoteCharacters && (*s == '\\' || *s == '{' || *s == '}'))
		*buf++ = '\\';
	    *buf++ = *s;
	}
    }
    *buf = '\0';
 }

class mark *simpletext::CreateMark(long  pos, long  length)
{
    class mark *markp;
    
    markp = new mark;
    (markp)->SetPos( pos);
    (markp)->SetLength( length);
    (markp)->SetNext( this->markList);
    (markp)->SetObject( this);
    this->markList = markp;
    return markp;
}

void simpletext::RemoveMark(class mark  *mark)
{
    class mark *mp;
    class mark *tp;

    if (mark == NULL) return;
    
    for (mp = this->markList, tp = NULL; mp != NULL && mp != mark; tp = mp, mp = (mp)->GetNext());
    
    if (mp != NULL)  {
	if (tp == NULL)  {
            /* First element on the list */
	    this->markList = (mp)->GetNext();
	}
	else  {
	    (tp)->SetNext( (mp)->GetNext());
	}
    }
}

void simpletext::RegionModified(long  pos, long  len)
            {
    class mark *mp;
    long endpos;

    for (mp = this->markList; mp != NULL; mp = (mp)->GetNext())  {
	if (pos >= (endpos = (mp)->GetPos() + (mp)->GetLength())) {
	    if (pos == endpos)
		(mp)->SetModified( TRUE);
	}
	else if (pos + len > (mp)->GetPos())
	    (mp)->SetModified( TRUE);
    }
}

boolean simpletext::CheckHighBit()
{
    long i=0, len=(this)->GetLength();

    if(this->highbitflag>=0) return this->highbitflag;
    
    this->highbitflag=0;
    for(i=0;i<len;i++) {
	if((this)->GetChar( i)&0x80) this->highbitflag=1;
    }
    return this->highbitflag;
}
