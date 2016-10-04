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
/*
 *    $Log: flex.H,v $
 * Revision 1.6  1996/01/31  19:38:38  robr
 * Implemented copying and assignment operations. Fixed Remove operations
 * for objects.
 * Made GetN an inline function.
 * FEATURE/BUG
 *
 * Revision 1.5  1995/11/10  17:32:48  robr
 * Added ATK_INTER/ATK_IMPL.
 * BUG
 *
 * Revision 1.4  1995/06/28  15:00:23  rr2b
 * Fixing so that the right function is called to handle deallocation.
 * BUG
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
 * Revision 1.4  1995/02/23  17:39:33  rr2b
 * Another temporary checkin
 *
 * Revision 1.3  1995/02/14  21:34:51  rr2b
 * Added info about MemCpy and MemMove.
 * FEATURE
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
 * Revision 1.1  1994/10/14  21:04:02  wjh
 * Initial revision
 * 
 * Revision 1.0  94/08/16  13:10:47  wjh
 */
#include <stddef.h> /* size_t */

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

    virtual char *Allocate(size_t len);
    /* Allocates len bytes for the elements of the flex. */

    void (*deallocate)(char *&ptr);
    /* Deallocates len bytes. */

    inline void Deallocate(char *&ptr) {
	if(deallocate) deallocate(ptr);
    }
    /* Deallocates len bytes. */

    virtual void CopyTo(char *dest, char *src, size_t len);
    /* Copies the src chars to dest. src and dest may not overlap.  Subclasses can override this to do object copies if necessary. Do not call destructors on src.  */
    virtual void MoveTo(char *dest, char *src, size_t len);
    /* Moves the src chars to dest, invalidating src.  src and dest may overlap.  Subclasses can override this to do object copies if necessary. Call destructors on src.  */

    virtual size_t RoundSize(size_t n) const;
    /* Rounds a number of bytes up to a full object size.  Subclasses can override this to ensure that the flex is always a multiple of the size of the objects stored within. */

    virtual void Invalidate(size_t i, size_t len);
    /* Destroys the len elements  beginning at the ith element. */
    
  public:

    flex(size_t n=100);		/* constructor */

    virtual ~flex();	/* discard the flex array and all contents.
			 does not 'delete' the contained elements */

    flex(const flex &src);
    flex &operator=(const flex &src);
    
    /* 'RetP' means it returns a pointer to the (first of) the element(s) affected */

    /* access and revision functions */

    char &operator[]( size_t i )   /* throw (int) */  ;		
    /* Returns a reference to the ith element */

    const char &operator[]( size_t i ) const   /* throw (int) */  ;		
    /* Returns a reference to the ith element */
    char *Insert( size_t i, size_t len );	
    /* create n new elts starting at i; RetP 
     subsequent elements are moved up */
    inline char *Insert( size_t i ) { return Insert( i, 1 );};
    inline char *Append() {return Insert( n, 1 );};
    /* insert a new, empty element  at the end;  RetP */

    size_t Find( const char &o ) const;
    /* returns the index corresponding to element o.
     Returns -1 if o is not in the flex */
    inline size_t Find( const char *o ) const {return Find( *o );};

    void Remove( size_t i, size_t len );	
    /* delete the len elts starting at the i'th */
    inline void Remove( size_t i ) {Remove( i, 1 );};
    /* deletes the i'th element*/
    inline void Remove( char *o ) {Remove( Find(o), 1 );};
    /* delete the element o */
    inline void Remove( char &o ) {Remove( Find(o), 1 );};
    /* delete the element o */

    inline size_t GetN() const {return n; }
    /* returns number of elements */

    char *GetBuf( size_t i, size_t len, size_t *gotlenp );
    /* client asks for pointer to memory containing
     len successive elements starting with the i'th.
     Returned pointer is to the i'th element and
     *gotlenp contains the number of succeeding
     elements stored consecutively after the i'th.
     *gotlenp may be anywhere in  1 ... getn()-i */

};
/** @} */
#endif /* _flex_H_ */
