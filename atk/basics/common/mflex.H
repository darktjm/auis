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
#include <flex.H>

/** A flexible array of unmanaged objects.
 *
 *  This is an abstract helper class for arrays of arbitrary types
 *  which do not require constructors or destructors.  An mflex array
 *  is actually created using the DEFINE_MFLEX_CLASS() macro, which
 *  derives this class into something more usable.
 *
 *  Elements of the array must be accessed individually; there is no
 *  guarantee that one element is consecutive in memory to the next,
 *  except under specific conditions:
 *
 *  - After calling Insert(n), n elements at the insert positions are
 *    guaranteed to be consecutive.
 *
 *  - After calling GetBuf(i, n, &l), l elements starting at i are
 *    guaranteed to be consecutive.
 *
 *  These guarantees are invalidated, along with the guarantee that an
 *  address corresponds to a specific element, whenever the buffer
 *  is changed using Insert, Append, Remove, or GetBuf.
 *
 *  This is closer to the previous flex implementation, in that arbitrary
 *  types are supported, and the derived class returns results in the
 *  element type, rather than just char.
 */
class mflex : public flex
{
protected:
    int typesize; /**< sizeof(element) */
public:

    mflex(size_t typesize=1, size_t n=100);
    /**< Construct an array of given number of blobs of given initial size.
     *  When the array grows, it generally grows 40%, unless that
     *  is not enough for an insertion, in which case it grows 20% more
     *  than the insertion requires.
     *   \param typesize Size of elements, in bytes.
     *   \param n Number of elements to initially allocate. */

    virtual ~mflex();
    /**< Discard the mflex array and all contents.
     *   No element destructors are executed.  */

    virtual size_t RoundSize(size_t n) const;
    /* Rounds a number of bytes up to a full object size.  Subclasses can override this to ensure that the mflex is always a multiple of the size of the objects stored within. */

    /* 'RetP' means it returns a pointer to the (first of) the element(s) affected */

    inline char &operator[](size_t i) {
	return (*(flex *)this)[i*typesize];
    }
    /**< \copydoc flex::operator[] */

    inline const char &operator[](size_t i) const {
	return (*(flex *)this)[i*typesize];
    }
    /**< \copydoc flex::operator[] */
    /* returns the first character of the aggregate at index i. */

    inline size_t GetN() const { return flex::GetN()/typesize; }
    /**< \copydoc flex::GetN */

    char *Insert( size_t i, size_t len=1 );
    /**< \copydoc flex::Insert */
    /* create n new elts starting at i; RetP
     subsequent elements are moved up */
    inline char *Append(size_t len=1) {return Insert( GetN(), len );}
    /**< Create space for new elements at the end of the array.
     *   \param len The number of elements to append.
     *   \return a pointer to the newly added space.   This
     *           pointer is invalidated at the next Insert, Append,
     *           Remove, or GetBuf.
     */

    size_t Find( const char &o ) const;
    /**< \copydoc flex::Find */

    inline size_t Find( const char *o ) const {return Find( *o );}
    /**< \copydoc flex::Find */
    /* returns the index corresponding to element o.
     Returns -1 if o is not in the mflex */

    void Remove( size_t i, size_t len=1 );
    /**< \copydoc flex::Remove */
    /* delete the len elts starting at the i'th */
    inline void Remove( char *o, size_t len=1) {Remove( Find(o), len);}
    /**< Remove elements from the array.
     *   \param o Pointer to a valid array element to remove.
     *   \param len Number of elements to remove, including and following o.
     */
    /* delete the element o */
    inline void Clear() {
	Remove((size_t)0UL,GetN());
    }
    /**< Empty the array.
     *   Does not free any memory.
     */
    char *GetBuf( size_t i, size_t len, size_t *gotlenp );
    /**< \copydoc flex::GetBuf */
    /* client asks for pointer to memory containing
     len successive elements starting with the i'th.
     Returned pointer is to the i'th element and
     *gotlenp contains the number of succeeding
     elements stored consecutively after the i'th.
     *gotlenp will be len unless there aren't that many elements. */

};

/** Create a function for sorting this mflex array.
 *  \param name The name of the sorting function.  In addition, a static
 *              C comparison function named <em>name</em>_comp will be
 *              created to perform the sorting comparisons with qsort(3).
 *  \param mflexclass The class this function is for.
 *  \param type The actual data type stored in the mflex array.
 *  \param comp The raw comparison function.  Pass the
 *              casted parameters e1 and e2, which are of the
 *              array element type, to comp and return an integer.
 */
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
/** Derive mflex into a class for a specific element data type.
 *  \param name The name of the new array class.
 *  \param type The type for which the class is built.
 *  \param minsize The default minimum size of the array.  If you like
 *                 the 100-byte default on flex, use 100/sizeof(type).
 */
#define DEFINE_MFLEX_CLASS(name,type,minsize) \
/** An mflex array for type elements.  Created by DEFINE_MFLEX_CLASS(). */ \
class name : public mflex { \
  public: \
    inline name(size_t n=minsize) : mflex(sizeof(type), n) { \
    } \
    /**< Create an array with an initial allocation of elements.
     *  When the array grows, it generally grows 40%, unless that
     *  is not enough for an insertion, in which case it grows 20% more
     *  than the insertion requires.
     *  \param n The number of elements to initially allocate. */ \
    inline virtual ~name() { \
    } /**< Discard the array and all contents.  */ \
    inline type &operator[](size_t i) { \
	return *(type *)&(*(mflex *)this)[i]; \
    } /**< \copydoc mflex::operator[] */ \
    inline const type &operator[](size_t i) const { \
	return *(const type *)&(*(mflex *)this)[i]; \
    } /**< \copydoc mflex::operator[] */ \
    inline type *Insert(size_t i, size_t len=1) { \
	return (type *)mflex::Insert(i,len); \
    } /**< \copydoc mflex::Insert */ \
    inline type *Append(size_t len=1) { \
	return (type *)mflex::Append(len); \
    } /**< \copydoc mflex::Append */ \
    inline size_t Find(const type &o) { \
	return mflex::Find((const char *)&o); \
    } /**< \copydoc mflex::Find */ \
    inline long Find( const type *o ) {return Find( *o );} \
    /**< \copydoc mflex::Find */ \
    inline void Remove(type *o, size_t len=1) { \
	mflex::Remove((char *)o, len); \
    } /**< \copydoc mflex::Remove */ \
    inline void Remove( size_t i, size_t len=1 ) { \
	mflex::Remove(i,len); \
    } /**< \copydoc mflex::Remove */ \
    inline type *GetBuf(size_t i, size_t len, size_t *gotlenp) { \
	return (type *)mflex::GetBuf(i, len, gotlenp); \
    } /**< \copydoc mflex::GetBuf */ \
};

/** Create standard C++ iterators for an mflex-derived array.
 *  \param name The name of the derived class.  The iterator class will
 *              be named <em>name</em>_iter.
 *  \param type The type of the derived class' array element. */
#define DEFINE_MFLEX_CLASS_ITER(name,type) \
/** Iterator for name arrays.  Created by DEFINE_MFLEX_CLASS_ITER(). */ \
class Concat(name,_iter) { \
    ssize_t i, n, ci; \
    name &obj; \
  public: \
    inline Concat(name,_iter)(name *o, size_t ia=0, size_t na=0) : obj(*o) { \
	ci=i=ia; \
	if(na==0 || na>o->GetN()) n=o->GetN(); \
	else n=na; \
    } \
    /**< Create an iterator over a given range.
     *   \param o The array over which to iterate.
     *   \param ia The first element over which to iterate.
     *   \param in The maximum number of elements to iterate over.
     *            If 0, the remainder of the array is iterated over.
     */ \
    inline Concat(name,_iter)(name &o, size_t ia=0, size_t na=0) : obj(o) { \
	ci=i=ia; \
	if(na==0 || na>o.GetN()) n=o.GetN(); \
	else n=na; \
    } \
    /**< Create an iterator over a given range.
     *   \param o The array over which to iterate.
     *   \param ia The first element over which to iterate.
     *   \param in The maximum number of elements to iterate over.
     *            If 0, the remainder of the array is iterated over.
     */ \
    inline Concat(name,_iter) &First() { \
	ci=i; \
	return *this; \
    } \
    /**< Move the iterator to the first element in the range.
     *   \return The iterator. */ \
    inline Concat(name,_iter) &Last() { \
	ci=i+n-1; \
	return *this; \
    } \
    /**< Move the iterator past the last element in the range.
     *   \return The iterator. */ \
    inline type *operator->() { \
	return &obj[ci]; \
    } \
    /**< Use the element corresponding to the current iterator location.
     *   \return The element.  If the iterator is out-of-range, this
     *                         value is undefined.  No exception will
     *                         be raised. */ \
    inline type &operator*() { \
	return obj[ci]; \
    } \
    /**< Get the element corresponding to the current iterator location.
     *   \return The element.  If the iterator is out-of-range, this
     *                         value is undefined.  No exception will
     *                         be raised. */ \
    inline Concat(name,_iter) &operator++() { \
	ci++; \
	return *this; \
    } \
    /**< Move the iterator forward.  No exception will be raised if this
     *   moves past the end of the array.
     *   \return The iterator. */ \
    inline Concat(name,_iter) &operator--() { \
	ci--; \
	return *this; \
    } \
    /**< Move the iterator backward.  No exception will be raised if this
     *   moves past the end of the array.
     *   \return The iterator. */ \
    inline boolean done() const { \
	if(ci>=i+n || ci<i) return TRUE; \
	else return FALSE; \
    } \
    /**< Check if iterator is finished.
     *   \return TRUE if out-of-bounds of the defined range; FALSE otherwise.  */ \
}
/** @} */
#endif /* _mflex_H_ */
