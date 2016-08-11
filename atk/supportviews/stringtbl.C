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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/supportviews/RCS/stringtbl.C,v 3.4 1994/12/13 20:29:40 rr2b Stab74 $";
#endif


 


/* strtbl.c		

String table data object

deferred:
	Probably ought to include some form of file header and trailer 
		even when not embedded in text
*/


#include <andrewos.h> /* strings.h */
ATK_IMPL("stringtbl.H")

#include <stringtbl.H>

#define MAXFILELINE 255


	
ATKdefineRegistry(stringtbl, dataobject, NULL);
#ifndef NORCSID
#endif
static short FindString(register class stringtbl  *self, register char  *s, int startIndex);
static void SetIthBit(register class stringtbl  *self, short  i, boolean  val);
static short  FindEntry(class stringtbl  *self, short  accnum);


stringtbl::stringtbl()
		{
	this->used = this->numacc = 0;
	this->ContainsInitialStrings = FALSE;
	(this)->AddString( "FIRST");
	(this)->AddString( "SECOND");
	(this)->AddString( "THIRD");
	this->ContainsInitialStrings = TRUE;

	THROWONFAILURE( TRUE);
}

	stringtbl::~stringtbl()
		{
	register short i;
	for (i = 0; i < this->used; i++)
		free (this->item[i]);
}


	long
stringtbl::Read( register FILE   *file, register long   id			/* !0 if data stream, 0 if direct from file*/ )
			{
	/* reads a stringtbl from -file-.  See file format in strtbl.ch */
	/* This routine reads the \enddata, if any. Its syntax is not checked */

	long highlight;
	char s[MAXFILELINE + 2];
	char c;

	if ((c=getc(file)) == '\n') {}		/* COMPATIBILITY KLUDGE */
	else if (c == '\\')
	    fgets(s, MAXFILELINE+2, file);	/* skip header */
	else ungetc(c, file);

	(this)->Clear();
	fscanf(file, " %x ", &highlight);
	while (TRUE) {
		char s[MAXSTRINGENTRYLENGTH + 2], *nl;
		if ((fgets(s, MAXSTRINGENTRYLENGTH + 2, file)) == 0) 
			/* EOF or error */
			break;
		if (*s == '\\') 
			/* \enddata */
			break;
		nl = s + strlen(s) - 1;		/* point at last character */
		if (*nl == '\n')
			*nl = '\0';	/* delete newline*/
		(this)->AddString( s);
	}
	this->highlight = highlight;	/* (should use SetBit) */
	(this)->NotifyObservers( stringtbl_STRINGSCHANGED);
	return dataobject_NOREADERROR;
}
	  
	long
stringtbl::Write( FILE   *file, long   writeID, int   level )
		 		{
	char head[50];
	register short i;
	long id = (this)->UniqueID();
	if (this->writeID != writeID) {
		/* new instance of write, do it */
		this->writeID = writeID;
		sprintf(head, "data{%s, %d}\n", (this)->GetTypeName(), id);
		fprintf(file, "\\begin%s", head);

		fprintf(file, "%x\n", this->highlight);
		for (i = 0; i < this->used; i++)
			fprintf(file, "%s\n", this->item[i]);

		fprintf(file, "\\end%s", head);
	}
	return id;
}

	void 
stringtbl::Clear( )
  		/* Clears the string table of existing strings */
{
 	register short i;
	for (i = this->used; --i >= 0; )
		free (this->item[i]);
	this->used = this->numacc = 0;
	(this)->NotifyObservers( stringtbl_STRINGSCHANGED);
	this->ContainsInitialStrings = FALSE;
}

	static short
FindString(register class stringtbl  *self, register char  *s, int startIndex)
		/* Finds string s in self and returns its index.   returns -1 for failure.*/
{
	register short i;
	if (s == NULL || *s == '\0') return (-2);
	for (i = startIndex; i < self->used; i++) {
		if (strcmp(s, self->item[i]) == 0)
			return (i);
	}
	return (-1);
}
	static void
SetIthBit(register class stringtbl  *self, short  i, boolean  val)
					/* finds the named string in the table 
			and sets its associated bit to the given val */
{
	register unsigned long mask;
	if (i < 0) 	/* string not found */
		return;
	mask = ((unsigned long)1)<<i;
	if ( (val==TRUE) != ((self->highlight & mask) != 0) ) {
		/* bit must change, invert it */
		self->highlight ^= mask;
		(self)->NotifyObservers( stringtbl_BITSCHANGED);
	}
}

short stringtbl::GetEntryOfString(register char  *s, short  startIndex)
			{
	register short i;

	i = FindString(this, s, startIndex);
	if (i >= 0) {
	    return this->accmap[i];
	}
	return -1;
}

	short
stringtbl::AddString(register char  *s)
		{
	short len;
	register char *t;
	short i;
	/* check for full table, length of string, backslash, and duplicate entry */
	if (this->used >= MAXSTRINGSINTABLE)
		return (0);
	if (this->ContainsInitialStrings) {
		(this)->Clear();
		this->ContainsInitialStrings = FALSE;
	}
	len = strlen(s);
	if (len > MAXSTRINGENTRYLENGTH)
	    len=MAXSTRINGENTRYLENGTH;
	t=(char *)malloc(len+1);
	if(t==NULL) return 0;
	s = strncpy(t, s, len);	/* copy the string to new storage */
	s[len] = '\0';					/* and terminate it ! */
	t = s;
	while ((t= strchr(t, '\\')))	/* delete backslashes */
		strcpy(t, t+1);
	t = s;
	while ((t= strchr(t, '{')))	/* delete left brackets */
		strcpy(t, t+1);
	t = s;
	while ((t= strchr(t, '}')))	/* delete right brackets */
		strcpy(t, t+1);
	if ((i=FindString(this, s, 0)) >= 0)
	 	/* duplicate entry */
		return this->accmap[i];
	if (i == -2) return (0);		/* NULL or  \0 */
	/* add the new item as the i'th */
	i = this->used++;	/* set i and incr -used- */
	this->item[i] = s;
	/* turn off highlight for the new string */
	this->highlight &= ~(((unsigned long)1)<<i);
	(this)->NotifyObservers( stringtbl_STRINGSCHANGED);
	this->accmap[i] = ++this->numacc;
	return this->numacc;
}

	void
stringtbl::RemoveString(register char  *s)
				/* Removes the specified string from the table.  
			If the string is absent, the call is ignored */
{
	register short i;
	register unsigned long mask;
	if (this->ContainsInitialStrings) {
		(this)->Clear();
		this->ContainsInitialStrings = FALSE;
	}
	i = FindString(this, s, 0);
	if (i < 0) 	/* string not found */
		return;
	free(this->item[i]);	/* free the string store */
	mask = (((unsigned long)1)<<i)-1;
	this->highlight = 	/* shift highlights */
		(this->highlight & mask)
		|  ((this->highlight & ~(mask<<1)) >> 1);
	this->used --;	/* decrement number of entries */
	/* DO NOT DECREMENT numacc */
	for ( ; i < this->used ; i++ ) {
		/* shift remaining items */
		this->item[i] = this->item[i+1];
		this->accmap[i] = this->accmap[i+1];
	}
	(this)->NotifyObservers( stringtbl_STRINGSCHANGED);
}

	void
stringtbl::SetBit(register char  *s, boolean  val)
					/* finds the named string in the table 
			and sets its associated bit to the given val */
{
	SetIthBit(this, FindString(this, s, 0), val);
}

	boolean
stringtbl::GetBit(register char  *s  )
				/* finds the named string in the table and 
			returns the current value of its bit */
{
	register short i;
	register unsigned long mask;
	i = FindString(this, s, 0);
	if (i < 0) 	/* string not found */
		return FALSE;
	mask = ((unsigned long)1)<<i;
	return ((this->highlight & mask) != 0L);
}

	void
stringtbl::ClearBits( )
	{
	this->highlight = 0L;
	(this)->NotifyObservers( stringtbl_BITSCHANGED);
}

	static short 
FindEntry(class stringtbl  *self, short  accnum)
		{
	int i;
	for (i = self->used;  i--; )
		if (self->accmap[i] == accnum) return i;
	return -1;
}

	void
stringtbl::RemoveEntry(short  accnum)
		/* remove the entry with the given accnum 
			If the string is absent, the call is ignored */
		{
	short i;
	if (this->ContainsInitialStrings) {
		(this)->Clear();
		this->ContainsInitialStrings = FALSE;
	}
	i = FindEntry(this, accnum);
	if (i >= 0)
		(this)->RemoveString( (this)->IthString( i));
}

	void
stringtbl::SetBitOfEntry(short  accnum	, boolean  val)
		/* set the bit associated with the given accnum 
			If the string is absent, the call is ignored */
			{
	SetIthBit(this, FindEntry(this, accnum), val);
}

	boolean
stringtbl::GetBitOfEntry(short  accnum)
		/* return the bit for the given accnum 
			If the string is absent, returns FALSE */
		{
	short i = FindEntry(this, accnum);
	unsigned long mask = ((unsigned long)1)<<i;
	return (i >= 0 && (this->highlight & mask) != 0L);
}

	char *
stringtbl::GetStringOfEntry(short  accnum)
		/* return the string for the given accnum 
			If the string is absent, returns NULL */
		{
	short i = FindEntry(this, accnum);
	if (i >= 0) 
		return (this)->IthString( i);
	else return NULL;
}


char *stringtbl::ViewName()
{
    return ("strtblview");
}