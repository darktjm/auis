ATK_INTER
#ifndef _mark_H_
#define _mark_H_
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


 

#define mark_VERSION 1

#include <ATK.H>

class mark : public ATK  {
public:
    virtual ~mark() {} /* avoid delete issues */
	virtual ATKregistryEntry *ATKregistry();


    virtual void SetStyle(boolean beginning, boolean ending);
    virtual void UpdateMarks(long pos, long size);

    mark();
	static class mark * NewWithStyle(boolean beginning, boolean ending)  ;
    static class mark * Allocate()  ;
    static void Deallocate(class mark *self);



	inline ATK  * &GetObject()	{ return ((this)->object); }
	inline long &GetPos() 	{ return ((this)->pos); }
	inline long &GetLength()	{ return ((this)->length); }
	inline long GetEndPos()	{ return ((this)->pos + (this)->length); }
	inline char &IncludeBeginning()	{ return ((this)->includeBeginning); }
	inline char &IncludeEnding()	{ return ((this)->includeEnding); }
	inline char &GetModified()	{ return ((this)->modified); }
	inline char &ObjectFree()	{ return ((this)->objectFree); }
	inline class mark * &GetNext()	{ return ((this)->next); }

	inline void SetObject(ATK  *obj)	{ ((this)->object = (ATK  *)(obj)); }
	inline void SetPos(long position)	{ ((this)->pos = position); }
	inline void SetLength(long len)	{ ((this)->length = len); }
	inline void SetModified(char val)	{ ((this)->modified = val); }
	inline void SetObjectFree(char v)	{ ((this)->objectFree = v); }
	inline void SetNext(class mark *n)	{ ((this)->next = n); }


    ATK  *object;
    class mark *next;
    long pos;
    long length;
    char modified;
    char objectFree;
    char includeBeginning;
    char includeEnding;
};

#endif /* _mark_H_ */
