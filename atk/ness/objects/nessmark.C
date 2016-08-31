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

/* nessmark.c		

	Code for the nessmark data object

	Deficiency:  Should test for objectFree more places XXX

*/

/*
    $Log: nessmark.C,v $
 * Revision 1.7  1994/11/30  20:42:06  rr2b
 * Start of Imakefile cleanup and pragma implementation/interface hack for g++
 *
 * Revision 1.6  1994/08/11  02:58:03  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.5  1994/03/12  22:42:52  rr2b
 * Removed second argument to Stringize, an empty string may not be stringized
 *
 * Revision 1.4  1993/10/20  17:00:08  rr2b
 * Used Stringize and concat macros.
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
 * Revision 1.18  1992/12/15  21:37:21  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.17  1992/12/15  00:49:48  rr2b
 * fixed disclaimerization
 *
 * Revision 1.16  1992/12/14  20:49:20  rr2b
 * disclaimerization
 *
 * Revision 1.15  1992/11/26  02:39:52  wjh
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
 * Revision 1.14  92/06/05  16:39:31  rr2b
 * added test for a null text before performing
 * operation referencing it.
 * 
	log elided June 1993  -wjh
 * 
 * Creation 88/03/24 15:05:00 wjh
 * 	Created by WJHansen following definitions in 
 *	"A Practical Algebra for Substring Expressions"

XXX code in the basic operations here will be repeated in the interpreter 

*/

#include <andrewos.h>
ATK_IMPL("nessmark.H")
#include <mark.H>
#include <text.H>
#include <simpletext.H>
#include <nestedmark.H>

#include <nessmark.H>

static char  debug = FALSE;
#define DEBUG(s) {if (debug) {printf s ; fflush(stdout);}}
#define ENTER(r) DEBUG(("Enter %s(0x%p)\n", Stringize(r), this))
#define LEAVE(r) DEBUG(("Leave %s(0x%p)\n", Stringize(r), this))


/* the following booleans are side arguments 
	from nessmark_Replace to nessmark_UpdateMarks.
	XXX Ugly global vars */
static boolean ReplacingEmpty = FALSE;
static boolean ReplacingNonEmpty = FALSE;



/* an empty marker is any marker with a length of zero 
	When we need to generate one on a unique text,
	the text we use is EmptyText.
*/
static class simpletext *nessmark_EmptyText;


	
ATKdefineRegistry(nessmark, mark, nessmark::InitializeClass);


	boolean
nessmark::InitializeClass() {

	nessmark_EmptyText = new simpletext;
	(nessmark_EmptyText)->SetReadOnly( TRUE);
	(new nessmark)->SetText( nessmark_EmptyText);
			/* create a marker chained to EmptyText so it will
				not be discarded */
	return TRUE;
}

nessmark::nessmark() {
	ATKinit;

	(this)->SetLength( 0);
	(this)->SetPos( 0);
	(this)->SetStyle( FALSE, TRUE);
	(this)->AttachToText( nessmark_EmptyText);
	THROWONFAILURE( TRUE);
}

nessmark::~nessmark() {
	ATKinit;

	class simpletext *text = (this)->GetText();
	if(text) {
		(this)->DetachFromText();
		/* XXXX KLUDGE:  assumes knowledge that text has one
			standard mark, the fence 
			(The code will work if their are more standard
			marks, but texts will not be freed.)  */
		if (text->markList == text->fence && text->fence->next == NULL)
			(text)->Destroy();
	}
}

class nessmark *nessmark::FreeMarks = NULL;

/* XXX it would be better to allocate a whole bunch of nessmarks in one place
	to reduce paging */

	class nessmark *
nessmark::getMark() {
	ATKinit;
	class nessmark *v;
	if (nessmark::FreeMarks != NULL) {
		v = nessmark::FreeMarks;
		nessmark::FreeMarks = (class nessmark *)(v)->GetNext();
	}
	else {
		v = new nessmark;
	}
	return v;
}
	 
	void 
nessmark::UpdateMarks(long  pos, long  size) {

	class nessmark *mark;
	long endPos;

	if (size > 0)  for (mark = this; mark != NULL; 
				mark = (class nessmark *)
					(mark)->GetNext())  {
		endPos = (mark)->GetEndPos();
		if (pos > endPos)
			{}
		else if (pos == endPos)  {
			if ((mark)->IncludeEnding() 

					/* The next line differs from
					 mark_UpdateMarks.  It ensures that
					 nessmarks before the replacement
					 of an empty nessmark are not extended.   */
					
					&& ! ReplacingEmpty)  {


					/* the following test and then-clause
					 also differ from mark_Update marks.
					 They ensure that an empty mark
					 that follows a non-empty replaced
					 mark will follow the replacement.  */

				if ((mark)->GetLength() == 0 
						&& ReplacingNonEmpty)
					(mark)->SetPos(
						(mark)->GetPos() + size);
				else {		
					(mark)->SetLength( 
						(mark)->GetLength() + size);
					(mark)->SetModified( TRUE);
				}
			}
		}
		else if (pos < (mark)->GetPos() 
				|| pos == (mark)->GetPos() 
				&& ! (mark)->IncludeBeginning())
			(mark)->SetPos( (mark)->GetPos() + size);
		else {
			(mark)->SetLength( 
				(mark)->GetLength() + size);
			(mark)->SetModified( TRUE);
		}
	} /* end if() for (mark...) */
	else if (size == 0)
		{}
	else for (mark = this; mark != NULL; 
			mark = (class nessmark *)(mark)->GetNext())  {
		/* size < 0 :  deletion */
		if (pos < (endPos = (mark)->GetEndPos()))  {
			/* this mark needs to be adjusted */
			if (pos - size <= (mark)->GetPos()) 
				/* Deletion is before this mark */
				(mark)->SetPos( 
						(mark)->GetPos() + size);
			else  {
				long tsize = size;
				if (pos <= (mark)->GetPos()) {
					/* part of deletion is before mark */
					tsize += (mark)->GetPos() - pos;
					(mark)->SetPos( pos);
				}
				/* restrict deletion size to that of mark */
				if (pos - tsize > endPos)
					tsize = pos - endPos;
				/* reduce size of mark per deletion */
				(mark)->SetLength( 
					(mark)->GetLength() + tsize);
				if ((mark)->GetLength() < 0) 
					(mark)->SetLength( 0);
				(mark)->SetModified( TRUE);
			}
		}  /* end pos < endPos */
	} /* end for mark */
}


	void 
nessmark::SetText(class simpletext  *text) {

	long tl = (text)->GetLength(),
		pos = (this)->GetPos();
	class simpletext *oldtext;
ENTER(nessmark::SetText);
	if (pos > tl)    
		(this)->SetPos( pos = tl);
	if (pos+(this)->GetLength() > tl)  
		(this)->SetLength( tl - pos);
	(this)->SetModified( FALSE);
	oldtext = (this)->GetText();
	if (text != oldtext) {
		if (oldtext != NULL && ! (this)->ObjectFree()) {
			(this)->DetachFromText();
			if (oldtext->markList == oldtext->fence 
					&& oldtext->fence->next == NULL) 
				(oldtext)->Destroy();
		}
		(this)->AttachToText(  (text != NULL) ? text : nessmark_EmptyText);
		(this)->SetObjectFree( FALSE);
	}
LEAVE(nessmark::SetText);
}

	void 
nessmark::Set(class simpletext  *text, long  pos , long  len) {

ENTER(nessmark::Set);
	if (len < 0)  {pos = pos + len;  len = -len;}
	if (pos < 0)  {len = len+pos;  pos = 0;}
	(this)->SetPos( pos);
	(this)->SetLength( len);
	(this)->SetText( text);
LEAVE(nessmark::Set);
}

	void 
nessmark::MakeConst(const char  *cx) {
	if (*cx == '\0')
		(this)->Set( nessmark_EmptyText, 0, 0);
	else {
		long len = strlen(cx);
		class simpletext *t = new simpletext;
		(t)->InsertCharacters( 0, cx, len);
		(t)->SetReadOnly( TRUE);
		(this)->Set( t, 0, len);
	}
}

/* XXX code in these routines is duplicated in the ness interpreter */

	void 
nessmark::Next() {

	class simpletext *t = (this)->GetText();
	long pos;
	(this)->SetPos(  
			pos = ((this)->GetPos()
			   +  (this)->GetLength())   );
	(this)->SetLength(  
			(pos < (t)->GetLength()) ? 1 : 0);
}

	void 
nessmark::Start() {

	(this)->SetLength( 0);
}

	void 
nessmark::Base() {

	(this)->SetPos( 0);
	(this)->SetLength( ((this)->GetText())->GetLength());
}

	void 
nessmark::Extent(class nessmark  *tail) {

ENTER(nessmark::Extent);
	if ((this)->GetText() != (tail)->GetText())
		(this)->Set( nessmark_EmptyText, 0, 0);
	else {
		int start = (this)->GetPos();
		int end = (tail)->GetPos() 
				+ (tail)->GetLength();
		if (end < start)
			start = end;
		(this)->SetPos( start);
		(this)->SetLength( end - start);
LEAVE(nessmark::Extent);
	}
}

	void 
nessmark::Replace(class nessmark  *replacement) {

	class text *text = (class text *)(this)->GetText();
	long destpos = (this)->GetPos();
	long destlen = (this)->GetLength();
	long srcpos = (replacement)->GetPos();
	long srclen = (replacement)->GetLength();
	boolean oldgliso;
ENTER(nessmark::Replace);
	DEBUG(("dest(%ld,%ld)   src(%ld,%ld)  destbaselen %ld\n", 
		destpos, destlen, srcpos, srclen, (text)->GetLength()));

	/* arrange so that styles that end at the beginning of the point of replacement, 
		will not apply to the replacement text. */
	oldgliso = nestedmark::SetGlobalIsolation(TRUE);

	/* insert the new text at end of the destination, then delete dest */
	if (destlen == 0) {
		ReplacingEmpty = TRUE;
		(text)->AlwaysCopyTextExactly( destpos + destlen, 
			(replacement)->GetText(), srcpos, srclen);
		ReplacingEmpty = FALSE;
	}
	else {
		ReplacingNonEmpty = TRUE;
		(text)->AlwaysCopyTextExactly( destpos + destlen, 
			(replacement)->GetText(), srcpos, srclen);
		(text)->AlwaysDeleteCharacters( destpos, destlen);
		ReplacingNonEmpty = FALSE;
	}
	nestedmark::SetGlobalIsolation(oldgliso);
	(this)->SetLength( srclen);
	(text)->NotifyObservers( 0);

	DEBUG(("dest(%ld,%ld)   src(%ld,%ld)  finallen %ld  destbaselen %ld\n", 
		destpos,  destlen,  srcpos,  srclen,  
		(this)->GetLength(), 
		(text)->GetLength()));
}

	boolean
nessmark::Equal(class nessmark  *comparand) {

	int i, iend, j;
	class simpletext *itext, *jtext;

	if ((this)->GetLength() 
				!= (comparand)->GetLength())
		return FALSE;

	i = (this)->GetPos();
	iend = (this)->GetEndPos();
	j = (comparand)->GetPos();
	itext = (this)->GetText();
	jtext = (comparand)->GetText();
	for (  ;  i < iend;  i++, j++)
		if ((itext)->GetUnsignedChar( i) 
				!= (jtext)->GetUnsignedChar( j))
			return FALSE;
	return TRUE;
}

	boolean
nessmark::IsEmpty() {

	return ((this)->GetLength() == 0);
}

	long
nessmark::Length() {

	return (this)->GetLength();
}

	void
nessmark::NextN(long  n) {

	class simpletext *t = (this)->GetText();
	long tlen = (t)->GetLength();
	long pos;
	if (n == 0) return;
	if (n < 0) {
		pos = (this)->GetPos()  +  n;
		if (pos < 0) pos = 0;
	}
	else {
		pos = (this)->GetPos()  +  (this)->GetLength() + n-1;
		if (pos > tlen) pos = tlen;
	}
	(this)->SetPos(  pos);
	(this)->SetLength(  (pos < tlen) ? 1 : 0);
}


	void
nessmark::SetFrom(class nessmark  *src) {

	(this)->Set( (src)->GetText(), 
			(src)->GetPos(), 
			(src)->GetLength());
}

	char *
nessmark::ToC() {

	class simpletext *t ;
	long loc, len;
	char *bx;
	char *buf;

	t = (this)->GetText();
	len = (this)->GetLength();
	buf = (char *)malloc(len + 1);
	bx = buf;
	/* XXX really should use simpletext_GetBuffer()  */
	for (loc = (this)->GetPos();  len > 0;  len--, loc++)
		*bx++ = (t)->GetUnsignedChar( loc);
	*bx = '\0';
	return buf;
}

