#ifndef _gflex_H_
#define _gflex_H_
/* ********************************************************************** *\
 *	   Copyright Carnegie Mellon, 1994 - All Rights Reserved
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/*    gflex.H

	gflex - object for storing a flexible array of structs or values

	This should be done with templates.  SIGH.
	For the time being, this file includes "gflextype.h"
		which must define ELTTYPE.

	Most functions return a reference to an ELTTYPE value within the gflex.

	The client programmer may think of a gflex as storing elements in 
	consecutive memory, but the code must not make this assumption.  
	There will usually be at least one gap in the sequence.
	Subclasses may implement alternate storage strategies.

*/
/*
 *    $Log: gflex.H,v $
 * Revision 1.2  1995/06/14  12:54:33  wjh
 * changed name from 'flex' to 'gflex'
 *
 * Revision 1.1  1994/10/14  21:04:02  wjh
 * Initial revision
 * 
 * Revision 1.0  94/08/16  13:10:47  wjh
 */

#include <gflextype.h>

class gflex
{

	int n;	/* number of actual elements stored */
	int gaploc;	/* location of gap */
	int gaplen;	/* space allocated === n+gaplen */
	ELTTYPE *elts;
			/* elts points to a malloced array. 
			  elements are stored in  [0] ... [gaploc-1]
			  and [gaploc+size-n] ... [size-1] */
	friend void makegap( gflex *f, int i, int len );

public:

	gflex();		/* constructor */

	virtual ~gflex();	/* discard the flex array and all contents.
				does not 'delete' the contained elements */

	/* 'RetP' means it returns a pointer to the (first of) the element(s) affected */

	/* access and revision functions */

	ELTTYPE &operator[]( int i )   /* throw (int) */  ;		
			/* Returns a reference to the ith element */

	ELTTYPE *insert( int i, int len );	
			/* create n new elts starting at i; RetP 
			  subsequent elements are moved up */
	inline ELTTYPE *insert( int i ) { return insert( i, 1 );};
	inline ELTTYPE *append() {return insert( n, 1 );};
			/* insert a new, empty element  at the end;  RetP */

	int efind( const ELTTYPE &o );
			/* returns the index corresponding to element o.
			  Returns -1 if o is not in the flex */
	inline int efind( const ELTTYPE *o ) {return efind( *o );};

	void erase( int i, int len );	
			/* delete the len elts starting at the i'th */
	inline void erase( int i ) {erase( i, 1 );};
		  	/* deletes the i'th element*/
	inline void erase( ELTTYPE *o ) {erase( efind(o), 1 );};
			/* delete the element o */
	inline void erase( ELTTYPE &o ) {erase( efind(o), 1 );};
			/* delete the element o */

	int getn() const;		/* returns number of elements */

	ELTTYPE *getbuf( int i, int len, int *gotlenp );
			/* client asks for pointer to memory containing
			  len successive elements starting with the i'th.
			  Returned pointer is to the i'th element and
			  *gotlenp contains the number of succeeding
			  elements stored consecutively after the i'th.
			  *gotlenp may be anywhere in  1 ... getn()-i */

};


#endif /* _gflex_H_ */
