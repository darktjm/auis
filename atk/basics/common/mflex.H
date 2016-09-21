#ifndef _mflex_H_
#define _mflex_H_
/* ********************************************************************** *\
 *	   Copyright Carnegie Mellon, 1995 - All Rights Reserved          *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
/*    mflex.H

 mflex - object for storing  mflexible array of simple  multi-byte structs or values.
 This will NOT work with C++ objects having constructors or destructors.
 mflex will rarely be used directly, instead a macro will be used to generate a subclass
 which does all the 

*/
/*
 *    $Log: mflex.H,v $
 * Revision 1.7  1996/01/31  19:51:55  robr
 * Having a Remove function which takes a reference made it ambiguous
 * with the Remove taking a size_t.
 * BUG
 *
 * Revision 1.6  1995/10/11  20:06:30  robr
 * Fixed to factor out the iterator from the DEFINE_MFLEX_CLASS
 * macro.
 * BUG
 *
 * Revision 1.5  1995/10/02  13:34:52  robr
 * Fixed to actually honor the size argument in mflex classes.
 * BUG
 *
 * Revision 1.4  1995/04/13  16:58:24  rr2b
 * Added sorter (DEFINE_MFLEX_SORTER)
 * Fixed bogus overloading ambiguity from gcc 2.6.x
 * BUG
 * FEATURE
 *
 * Revision 1.3  1995/03/26  05:37:45  rr2b
 * first round of fixes of the new code
 *
 * Revision 1.2  1995/03/26  05:31:02  rr2b
 * first round of fixes of the new code
 *
 * Revision 1.1  1995/03/26  03:09:23  rr2b
 * flex array support
 *
 * Revision 1.3  1995/02/23  17:39:33  rr2b
 * Another temporary checkin
 *
 * Revision 1.2  1995/02/14  18:12:55  rr2b
 * Converted to more consistently reflect the ATK member function
 * naming conventions.
 * Minor tweaking to make GetN expand inline for mflex subclasses.
 * BUG/FEATURE
 *
 * Revision 1.1  1995/02/10  18:33:51  rr2b
 * Initial revision
 *
 */

#include <flex.H>

class mflex : public flex
{
protected:
    int typesize;
public:

	mflex(size_t typesize=1, size_t n=100);		/* constructor */

	virtual ~mflex();	/* discard the mflex array and all contents.
				does not 'delete' the contained elements */

	virtual size_t RoundSize(size_t n) const;
	/* Rounds a number of bytes up to a full object size.  Subclasses can override this to ensure that the mflex is always a multiple of the size of the objects stored within. */
	
	/* 'RetP' means it returns a pointer to the (first of) the element(s) affected */
	
        inline char &operator[](size_t i) {
	    return (*(flex *)this)[i*typesize];
	}
	
        inline const char &operator[](size_t i) const {
	    return (*(flex *)this)[i*typesize];
	}
			/* returns the first character of the aggregate at index i. */

	inline size_t GetN() const { return flex::GetN()/typesize; }
	
	char *Insert( size_t i, size_t len=1 );	
			/* create n new elts starting at i; RetP 
			  subsequent elements are moved up */
	inline char *Append(size_t len=1) {return Insert( GetN(), len );};
			/* insert a new, empty element  at the end;  RetP */

	size_t Find( const char &o ) const;
			/* returns the index corresponding to element o.
			  Returns -1 if o is not in the mflex */
	inline size_t Find( const char *o ) const {return Find( *o );};

	void Remove( size_t i, size_t len=1 );	
			/* delete the len elts starting at the i'th */
	inline void Remove( char *o, size_t len=1) {Remove( Find(o), len);};
			/* delete the element o */
	inline void Clear() {
	    Remove((size_t)0UL,GetN());
	}
	char *GetBuf( size_t i, size_t len, size_t *gotlenp );
			/* client asks for pointer to memory containing
			  len successive elements starting with the i'th.
			  Returned pointer is to the i'th element and
			  *gotlenp contains the number of succeeding
			  elements stored consecutively after the i'th.
			  *gotlenp will be len unless there aren't that many elements. */

};

#define DEFINE_MFLEX_SORTER(name,mflexclass,type,comp) \
extern "C" { \
static int Concat(name,_comp)(const void *ve1, const void *ve2) { \
type e1=(*(type *)ve1), e2=(*(type *)ve2); \
return comp; \
} \
} \
static void name(mflexclass &mf) { \
size_t dummy; \
type *buf=mf.GetBuf(0,mf.GetN(), &dummy); \
qsort(buf, mf.GetN(), sizeof(type), QSORTFUNC(Concat(name,_comp))); \
}
#define DEFINE_MFLEX_CLASS(name,type,minsize) \
class name : public mflex { \
  public: \
    inline name(size_t n=minsize) : mflex(sizeof(type), n) { \
    } \
    inline virtual ~name() { \
    } \
    inline type &operator[](size_t i) { \
	return *(type *)&(*(mflex *)this)[i]; \
    } \
    inline const type &operator[](size_t i) const { \
	return *(const type *)&(*(mflex *)this)[i]; \
    } \
    inline type *Insert(size_t i, size_t len=1) { \
	return (type *)mflex::Insert(i,len); \
    } \
    inline type *Append(size_t len=1) { \
	return (type *)mflex::Append(len); \
    } \
    inline size_t Find(const type &o) { \
	return mflex::Find((const char *)&o); \
    } \
    inline long Find( const type *o ) {return Find( *o );}; \
    inline void Remove(type *o, size_t len=1) { \
	mflex::Remove((char *)o, len); \
    } \
inline void Remove( size_t i, size_t len=1 ) { \
mflex::Remove(i,len); \
} \
    inline type *GetBuf(size_t i, size_t len, size_t *gotlenp) { \
	return (type *)mflex::GetBuf(i, len, gotlenp); \
    } \
};

#define DEFINE_MFLEX_CLASS_ITER(name,type) \
class Concat(name,_iter) { \
    size_t i, n, ci; \
    name &obj; \
  public: \
    inline Concat(name,_iter)(name *o, size_t ia=0, size_t na=0) : obj(*o) { \
	ci=i=ia; \
	if(na==0 || na>o->GetN()) n=o->GetN(); \
	else n=na; \
    } \
    inline Concat(name,_iter)(name &o, size_t ia=0, size_t na=0) : obj(o) { \
	ci=i=ia; \
	if(na==0 || na>o.GetN()) n=o.GetN(); \
	else n=na; \
    } \
    inline Concat(name,_iter) &First() { \
	ci=i; \
	return *this; \
    } \
    inline Concat(name,_iter) &Last() { \
	ci=i+n-1; \
	return *this; \
    } \
    inline type *operator->() { \
	return &obj[ci]; \
    } \
    inline type &operator*() { \
	return obj[ci]; \
    } \
    inline Concat(name,_iter) &operator++() { \
	ci++; \
	return *this; \
    } \
    inline Concat(name,_iter) &operator--() { \
	ci--; \
	return *this; \
 \
    } \
    inline boolean done() const { \
	if(ci>=i+n || ci<0) return TRUE; \
	else return FALSE; \
    } \
}
/** @} */
#endif /* _mflex_H_ */
