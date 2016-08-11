ATK_INTER
#ifndef _gbuf_H_
#define _gbuf_H_
/* ********************************************************************** *\
 *         Copyright Carnegie Mellon University 1993 - All Rights Reserved 
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
/* gbuf - general buffer package
	implements a buffer whose size adapts to its contents
	and which permits insertion at any point
	DOES NOT USE A GAP, so insertion of one character at a time is costly 
	However, AppendChar is a cheap macro
*/

#include <andrewos.h>	/* for memmove */

#include <ATK.H>
class gbuf : public ATK  {

public:
	virtual ATKregistryEntry *ATKregistry();

	static boolean InitializeClass();

	gbuf();
	
	virtual ~gbuf();


	virtual char * GetBuffer(long pos, long len, long *gotlen);
		/* returns a pointer to the portion of the buffer 
		starting at pos and trying to extend for len
		length actually gotten is returned in gotlen */
	virtual void Insert(long pos, char *txt, long txtlen);
		/* inserts txt at pos.  prints error if there
		is an unreleased GetBuffer */
	virtual void Delete(long pos, long len);
		/* deletes len chars at pos.  prints error if there
		is an unreleased GetBuffer */
	virtual char AppendCharacter(char c);
		/* appends c to the end of self->buf.
		Prints error message if there is an unreleased GetBuffer.
		Usually gbuf_AppendChar is preferable */


	inline char GetChar(long pos)  { return ((this)->buf[pos]); }
	inline unsigned char GetUnsChar(long pos) { return (((unsigned char *)(this)->buf)[pos]); }
	inline char AppendChar(char c) { return (((this)->curlen < (this)->size) ? ((this)->buf[(this)->curlen++] = (c)) : (this)->AppendCharacter((c))); }
	inline void Replace(long pos, long len, char *txt)  { memmove((this)->buf+(pos), (txt), (len)); }
	inline long ReleaseBuffer()	 { return ((--(this)->holds < 0) ? (this)->holds=0 : (this)->holds); }
		/* releases the hold initiated by GetBuffer
		an error message is printed for Insert or Delete
		while a hold is in place */
	inline long   Len()  { return ((this)->curlen); }

	long size;	/* total length of *buf */
	long curlen;	/* currently using buf[0] ... buf[curlen-1] */
	long holds;	/* number of outstanding GetBuffer calls */
	char *buf;	/* pointer to the buffer itself */
};


#endif /* _gbuf_H_ */