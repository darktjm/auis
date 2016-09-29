#ifndef _mark_H_
#define _mark_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupport
 * @{ */
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
/** @} */
#endif /* _mark_H_ */
