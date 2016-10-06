#ifndef _OFLEX_H
#define _OFLEX_H
/* Copyright 1995 Carnegie Mellon University All rights reserved. */
#include <mflex.H>
#include <new>

/** \addtogroup libbasics
 * @{ */
/** A flexible array of managed objects.
 *
 *  This is an abstract helper class for arrays of arbitrary types
 *  which require constructors and/or destructors.  An oflex array
 *  is actually created using the DEFINE_OFLEX_CLASS() macro, which
 *  derives this class into something more usable.  You should probably
 *  use std::vector instead of this class for generic arrays.
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
 *  is changed using Insert, Append, Remove, or GetBuf.  Every time
 *  an element is moved due to such an edit, it is copied to its new
 *  location using the copy constructor, and then the destructor is
 *  called on the old version (This was written before move constructors
 *  of C++11 were available).  Whenever space is allocated using Insert
 *  or Append, the default constructor is called.  Whenever space is
 *  deallocated using Remove, the removed elements' destructors are called.
 *  This includes removal of all elements upon array destruction.
 */
class oflex : public mflex {
  protected:
    virtual void CopyTo(char *dest, char *src, size_t bytelength);

    virtual void MoveTo(char *dest, char *src, size_t bytelength);

    virtual void DestroyElement(void *src);
    /**< Call destructor on an element.
     *   \param src The element to destroy. */

    virtual void ConstructElement(void *src);
    /**< Call default constructor on an element.
     *   \param src The element to initialize. */

    virtual void CopyElement(void *dest, const void *src);
    /**< Call copy constructor for an element.
     *   \param src The element to copy.
     *   \param dest The target memory.
     */

    virtual void Invalidate(size_t i, size_t len);
    /**< Call destructor on one or more elements.
     *   \param i The byte offset of the first element to destroy.
     *   \param len The number of bytes to destroy.  The number of
     *              elements is len/typesize. */
    
  public:

    oflex(size_t typesize=1, size_t n=5);
    /**< Construct an array of given number of blobs of given initial size.
     *  When the array grows, it generally grows 40%, unless that
     *  is not enough for an insertion, in which case it grows 20% more
     *  than the insertion requires.
     *   \param typesize Size of elements, in bytes.
     *   \param n Number of elements to initially allocate. */
    oflex(const oflex &src);
    /**< Copy constructor.
     *   \param src The oflex to copy.
     */
    
    virtual ~oflex();
    /**< Destructor.  Destroys all elements in the array, as well. */

    char *Insert(size_t i, size_t len=1);
    /**< \copydoc mflex::Insert */

    inline char *Append(size_t len=1) {
	return Insert(GetN(), len);
    }
    /**< \copydoc mflex::Append */

};


/** Derive oflex into a class for a specific element data type.
 *  \param name The name of the new array class.
 *  \param type The type for which the class is built.
 *  \param minsize The default minimum size of the array.  If you like
 *                 the 100-byte default on flex, use 100/sizeof(type).
 */
#define DEFINE_OFLEX_CLASS(name,type,minsize) \
/** An oflex array for type elements.  Created by DEFINE_OFLEX_CLASS(). */ \
class name : public oflex { \
  public: \
    inline name(size_t n=minsize) : oflex(sizeof(type), n) { \
    } \
    /**< Create an array with an initial allocation of elements.
     *  When the array grows, it generally grows 40%, unless that
     *  is not enough for an insertion, in which case it grows 20% more
     *  than the insertion requires.
     *  \param n The number of elements to initially allocate. */ \
    inline name(const name &src) : oflex(sizeof(type), src.GetN()) { \
	if(src.GetN()>0) { \
	    type *pelts=Insert(0, src.GetN()); \
	    /* using CopyTo() here will fail if src has a gap */ \
	    for(size_t i=0;i<GetN();i++) CopyElement(&pelts[i], &src[i]); \
	} \
    } \
    /**< Copy constructor */ \
    inline virtual ~name() { \
	for(size_t i=0;i<GetN();i++) DestroyElement(&((*this)[i])); \
    } \
    /**< Discard the array and all contents, calling destructors for elements. */ \
    inline const type &operator[](size_t i) const{ \
	return *(type *)&(*(oflex *)this)[i]; \
    } /**< \copydoc oflex::operator[] */ \
    inline type &operator[](size_t i) { \
	return *(type *)&(*(oflex *)this)[i]; \
    } /**< \copydoc oflex::operator[] */ \
    inline type *Insert(size_t i, size_t len=1) { \
	return (type *)oflex::Insert(i,len); \
    } /**< \copydoc oflex::Insert */ \
    inline type *Append(size_t len=1) { \
	return (type *)oflex::Insert(GetN(),len); \
    } /**< \copydoc oflex::Append */ \
    inline type *Insert(size_t i, const type &o) { \
	void *res=oflex::Insert(i,1); \
	return new(res) type(o); \
    } \
    /**< Insert an object into the array.
     *   \param i The array element to insert.  All elements equal to
     *           or greater than i will have their index incremented by 1.
     *   \param o The object to insert.
     *   \return a pointer to the newly added element.
     */ \
    inline type *Insert(size_t i, const type *o) { \
	return Insert(i,*o); \
    } \
    /**< Insert an object into the array.
     *   \param i The array element to insert.  All elements equal to
     *           or greater than i will have their index incremented by 1.
     *   \param o A pointer to the object to insert.
     *   \return a pointer to the newly added element.
     */ \
    inline type *Append(const type &o) { \
	return Insert(GetN(),o); \
    } \
    /**< Append an object to the array.
     *   \param o The object to append.
     *   \return a pointer to the newly added element.
     */ \
    inline type *Append(const type *o) { \
	return Append(*o); \
    } \
    /**< Append an object to the array.
     *   \param o A pointer to the object to append.
     *   \return a pointer to the newly added element.
     */ \
    inline size_t Find(const type &o) { \
	return oflex::Find((const char *)&o); \
    } /**< \copydoc oflex::Find */ \
    inline size_t Find( const type *o ) {return Find( *o );} \
    /**< \copydoc oflex::Find */ \
    inline void Remove(size_t i, size_t len=1) { \
	oflex::Remove(i, len); \
    } /**< \copydoc oflex::Remove */ \
    inline void Remove(const type *o, size_t len=1) { \
	oflex::Remove((char *)o, len); \
    } \
    /**< Remove an object from the array.
     *   \param o A pointer to a valid array element.
     *   \param len The number of elements to remove, including and following o. */ \
    inline type *GetBuf(size_t i, size_t len, size_t *gotlenp) { \
	return (type *)oflex::GetBuf(i, len, gotlenp); \
    } /**< \copydoc oflex::GetBuf */ \
  protected: \
    inline virtual void ConstructElement(void *src) { \
	new(src) type(); } \
    inline virtual void DestroyElement(void *src) { \
	((type *)src)->~type(); } \
    inline virtual void CopyElement(void *dest, const void *src) { \
	new(dest) type(*(const type *)src); } \
}

/** Create standard C++ iterators for an oflex-derived array.
 *  \param name The name of the derived class.  The iterator class will
 *              be named <em>name</em>_iter.
 *  \param type The type of the derived class' array element. */
#define DEFINE_OFLEX_ITER(name,type) \
/** Iterator for name arrays.  Created by DEFINE_OFLEX_ITER().  */ \
class Concat(name,_iter) { \
    ssize_t i, n, ci; \
    name &obj; \
  public: \
    inline Concat(name,_iter)(name *o, size_t ia=0, size_t na=0) : obj(*o) { \
	ci=i=ia; \
	if(na<=0) n=o->GetN(); \
	else n=na; \
    } \
    /**< Create an iterator over a given range.
     *   \param o The array over which to iterate.
     *   \param ia The first element over which to iterate.
     *   \param na The maximum number of elements to iterate over.
     *            If 0, the remainder of the array is iterated over.
     */ \
    inline Concat(name,_iter)(name &o, size_t ia=0, size_t na=0) : obj(o) { \
	ci=i=ia; \
	if(na<=0 || na>o.GetN()) n=o.GetN(); \
	else n=na; \
    } \
    /**< Create an iterator over a given range.
     *   \param o The array over which to iterate.
     *   \param ia The first element over which to iterate.
     *   \param na The maximum number of elements to iterate over.
     *            If 0, the remainder of the array is iterated over.
     */ \
    inline Concat(name,_iter) &first() { \
	ci=i; \
	return *this; \
    } \
    /**< Move the iterator to the first element in the range.
     *   \return The iterator. */ \
    inline Concat(name,_iter) &last() { \
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
    inline Concat(name,_iter) &operator++(int) { \
	ci++; \
	return *this; \
    } \
    /**< Move the iterator forward.  No exception will be raised if this
     *   moves past the end of the array.
     *   \return The iterator. */ \
    inline Concat(name,_iter) &operator--(int) { \
	ci--; \
	return *this; \
    } \
    /**< Move the iterator backward.  No exception will be raised if this
     *   moves past the end of the array.
     *   \return The iterator. */ \
    inline boolean done() { \
	if(ci>=i+n /* || ci<0 */) return TRUE; \
	else return FALSE; \
    } \
    /**< Check if iterator is finished.
     *   \return TRUE if out-of-bounds of the defined range; FALSE otherwise.  */ \
}

/** Create standard C++ const iterators for an oflex-derived array.
 *  \param name The name of the derived class.  The iterator class will
 *              be named <em>name</em>_citer.
 *  \param type The type of the derived class' array element. */
#define DEFINE_OFLEX_CONST_ITER(name,type) \
/** Iterator for name arrays.  Created by DEFINE_OFLEX_CONST_ITER().  */ \
class Concat(name,_citer) { \
    ssize_t i, n, ci; \
    const name &obj; \
  public: \
    inline Concat(name,_citer)(const name *o, size_t ia=0, size_t na=0) : obj(*o) { \
	ci=i=ia; \
	if(na<=0) n=o->GetN(); \
	else n=na; \
    } \
    /**< Create an iterator over a given range.
     *   \param o The array over which to iterate.
     *   \param ia The first element over which to iterate.
     *   \param na The maximum number of elements to iterate over.
     *            If 0, the remainder of the array is iterated over.
     */ \
    inline Concat(name,_citer)(const name &o, size_t ia=0, size_t na=0) : obj(o) { \
	ci=i=ia; \
	if(na<=0 || na>o.GetN()) n=o.GetN(); \
	else n=na; \
    } \
    /**< Create an iterator over a given range.
     *   \param o The array over which to iterate.
     *   \param ia The first element over which to iterate.
     *   \param na The maximum number of elements to iterate over.
     *            If 0, the remainder of the array is iterated over.
     */ \
    inline Concat(name,_citer) &first() { \
	ci=i; \
	return *this; \
    } \
    /**< Move the iterator to the first element in the range.
     *   \return The iterator. */ \
    inline Concat(name,_citer) &last() { \
	ci=i+n-1; \
	return *this; \
    } \
    /**< Move the iterator past the last element in the range.
     *   \return The iterator. */ \
    inline const type *operator->() { \
	return &obj[ci]; \
    } \
    /**< Use the element corresponding to the current iterator location.
     *   \return The element.  If the iterator is out-of-range, this
     *                         value is undefined.  No exception will
     *                         be raised. */ \
    inline const type &operator*() { \
	return obj[ci]; \
    } \
    /**< Get the element corresponding to the current iterator location.
     *   \return The element.  If the iterator is out-of-range, this
     *                         value is undefined.  No exception will
     *                         be raised. */ \
    inline Concat(name,_citer) &operator++(int) { \
	ci++; \
	return *this; \
    } \
    /**< Move the iterator forward.  No exception will be raised if this
     *   moves past the end of the array.
     *   \return The iterator. */ \
    inline Concat(name,_citer) &operator--(int) { \
	ci--; \
	return *this; \
    } \
    /**< Move the iterator backward.  No exception will be raised if this
     *   moves past the end of the array.
     *   \return The iterator. */ \
    inline boolean done() { \
	if(ci>=i+n /* || ci<0 */) return TRUE; \
	else return FALSE; \
    } \
    /**< Check if iterator is finished.
     *   \return TRUE if out-of-bounds of the defined range; FALSE otherwise.  */ \
}
/** @} */
#endif

