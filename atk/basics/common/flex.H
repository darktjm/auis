#ifndef _flex_H_
#define _flex_H_
/* ********************************************************************** *\
 *	   Copyright Carnegie Mellon, 1994-1995 - All Rights Reserved     *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libbasics
 * @{ */
/*    flex.H

	flex - object for storing a flexible array of structs or values

	This should be done with templates.  SIGH.
	
	The client programmer may think of a flex as storing elements in 
	consecutive memory, but the code must not make this assumption.  
	There will usually be at least one gap in the sequence.
	Subclasses may implement alternate storage strategies.

*/
#include <stddef.h> /* size_t */

/** A flexible array of characters.
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
 *  Elements are raw bytes.  Constructors and destructors are not
 *  called.
 *
 *  The underlying implementation of this is to consider the allocated,
 *  but unused elements as a movable gap.  On insertion, rather than
 *  shifting all elements above the insertion point up one spot, they
 *  are shifted up to the top of the allocated space, leaving the gap
 *  at the insertion point.  Similarly, deletions take place by moving
 *  the gap below the deletion point, rather than shifting all elements
 *  down to fill the deletion.  This provides better performance with
 *  multiple insertions or deletions at the same place, or small movement
 *  between insertions and deletions, than simple arrays leaving the
 *  gap at the end.  Derived classes can take advantage of several
 *  characteristics of this implementation:
 *
 *     - An element's location in memory will not change unless the
 *       array is edited (via Insert or Remove).
 *
 *     - If you do not edit within a particular region, that region
 *       will remain contiguous (mflex/oflex depend on this in particular).
 *
 *  Previous versions of this were a flexible array of any type, with the
 *  type defined by the preprocessor when compiling.  This is now split
 *  into flex, mflex, and oflex to handle other cases.
 */
class flex
{
    size_t n;	/* number of actual elements stored */
    size_t gaploc;	/* location of gap */
    size_t gaplen;	/* space allocated === n+gaplen */
    /* elts points to a malloced array. 
     elements are stored in  [0] ... [gaploc-1]
     and [gaploc+size-n] ... [size-1] */
    void MakeGap(size_t i, size_t len );
  protected:
    char *elts;
    /**< The raw element array. */

    virtual char *Allocate(size_t len);
    /**< Allocates len bytes for the elements of the flex.
     *   Usually just a wrapper for malloc().
     *   \param len The amount to allocate.
     *   \return the allocated memory, or NULL if error.
     */

    void (*deallocate)(char *&ptr);
    /**< Deallocates len bytes.
     *   Usually just a wrapper for free() that also sets variable to NULL.
     *   \param ptr Memory to free, and variable to set to NULL.
     */

    inline void Deallocate(char *&ptr) {
	if(deallocate) deallocate(ptr);
    }
    /**< Deallocates len bytes.
     *   Default is to call deallocate().
     *   \param ptr Memory to free, and variable to set to NULL.
     */

    virtual void CopyTo(char *dest, char *src, size_t len);
    /**< Copies the src chars to dest. src and dest may not overlap.
     *   Subclasses can override this to do object copies if necessary.
     *   \param dest Target of copy.  Call constructors if necessary.
     *   \param src Source of copy.  Do not call destructors.
     *   \param len Number of bytes to copy.
     */
    virtual void MoveTo(char *dest, char *src, size_t len);
    /**< Moves the src chars to dest, invalidating src.
     *   src and dest may overlap.
     *   Subclasses can override this to do object copies if necessary.
     *   \param dest Target of copy.  Call constructors if necessary.
     *   \param src Source of copy.  Call destructors if necessary.
     *   \param len Number of bytes to move.
     */

    virtual size_t RoundSize(size_t n) const;
    /**< Rounds a number of bytes up to a full object size.
     *   Subclasses can override this to ensure that the flex is always
     *   a multiple of the size of the objects stored within.
     *   \param n The minimum number of bytes to allocate.
     *   \return n rounded up to the nearest object size. */

    virtual void Invalidate(size_t i, size_t len);
    /**< Destroys the len elements  beginning at the ith element.
     *   Does nothing by default; override if necessary.
     *   \param i First offset in memory to start invalidating.
     *   \param len Number of bytes to invalidate.
     */
    
  public:

    flex(size_t n=100);
    /**< Create an array with an initial allocation of bytes.
     *  When the array grows, it generally grows 40%, unless that
     *  is not enough for an insertion, in which case it grows 20% more
     *  than the insertion requires.
     *  \param n The number of bytes to initially allocate. */

    virtual ~flex();
    /**< Discard the flex array and all contents.
     * No element destructors are executed.  */

    flex(const flex &src);
    /**< Copy constructor */
    flex &operator=(const flex &src);
    /**< Copy assignment operator */
    
    /* 'RetP' means it returns a pointer to the (first of) the element(s) affected */

    /* access and revision functions */

    char &operator[]( size_t i )   /* throw (int) */  ;		
    /**< Returns a reference to the ith element
     *   This does not automatically expand the array if needed.
     *   \param i The element to access.
     *   \return a variable reference to the requested element.  This
     *           reference is valid until the next Insert, Append, Remove,
     *           or GetBuf.
     *           On error, a message is displayed to standard error and a
     *           random element may be returned, instead.
     */

    const char &operator[]( size_t i ) const   /* throw (int) */  ;		
    /**< Returns a reference to the ith element
     *   This does not automatically expand the array if needed.
     *   \param i The element to access.
     *   \return a const reference to the requested element.  This
     *           reference is valid until the next Insert, Append, Remove
     *           or GetBuf.
     *           On error, a message is displayed to standard error and a
     *           random element may be returned, instead.
     */
    char *Insert( size_t i, size_t len = 1 );	
    /**< Create space for new elements in array.
     *   \param i The array element to move up.  All elements equal to
     *           or greater than i will have their index incremented by len.
     *   \param len The number of elements to insert.
     *   \return a pointer to start of the newly added space.  This
     *           pointer is guaranteed to be a contiguous region of
     *           memory of length len, and is invalidated at the next
     *           Insert, Append, Remove, or GetBuf.
     */
    inline char *Append() {return Insert( n, 1 );}
    /**< Create space for a new element at the end of the array.
     *   \return a pointer to the newly added space.   This
     *           pointer is invalidated at the next Insert, Append,
     *           Remove, or GetBuf.
     */

    size_t Find( const char &o ) const;
    /**< Find the index of a given element.
     *   If the given pointer is a pointer to a valid element of the
     *   array, the index corresponding to this is returned.  Note that
     *   array elements may generally become invalidated after any
     *   Insert, Append, Remove, or GetBuf operation.
     *   \param o A pointer to an array element.
     *   \return The index of o in the array, or -1 if not a valid
     *           array element. */
    inline size_t Find( const char *o ) const {return Find( *o );}
    /**< Find the index of a given element.
     *   If the given pointer is a pointer to a valid element of the
     *   array, the index corresponding to this is returned.  Note that
     *   array elements may generally become invalidated after any
     *   Insert, Append, Remove, or GetBuf operation.
     *   \param o A pointer to an array element.
     *   \return The index of o in the array, or -1 if not a valid
     *           array element. */

    void Remove( size_t i, size_t len = 1 );
    /**< Remove elements from the array.
     *   \param i First element to remove.
     *   \param len Number of elements to remove.
     */
    inline void Remove( char *o ) {Remove( Find(o), 1 );}
    /**< Remove element from the array.
     *   \param o Pointer to a valid array element to remove.
     */

    inline size_t GetN() const {return n; }
    /**< Find the array length.
     *   This is the number of in-use elements, not the amount of memory
     *   allocated for the array.
     *   \return the number of used elements in the array. */

    char *GetBuf( size_t i, size_t len, size_t *gotlenp );
    /**< Obtain pointer to consecutively stored array elements.
     *   This ensures that the requested number of elements are
     *   contiguous in memeory, at least until the next call to
     *   Insert, Append, Remove, or GetBuf.
     *   \param i The first element in the contiguous region.
     *   \param len The number of desired contiguous elements.
     *   \param gotlenp A pointer to a variable which will be filled with
     *                  the actual number of contiguous elements.  This
     *                  will be at least the actual number of array elements
     *                  at i, up to len.  It may be smaller if i or len
     *                  are out of range, and it may be larger if more
     *                  elements are contiguous.
     *   \return a pointer to the first element in the contiguous region.
     *           This may be NULL if *gotlenp is 0.
     */
};
/** @} */
#endif /* _flex_H_ */
