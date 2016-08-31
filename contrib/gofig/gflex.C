/* ********************************************************************** *\
 *	   Copyright Carnegie Mellon, 1994 - All Rights Reserved
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

/* gflex.c		

	Code for the gflex data object
*/
/*
 *    $Log: gflex.C,v $
 * Revision 1.4  1995/06/14  12:54:33  wjh
 * changed name from 'flex' to 'gflex'
 *
 * Revision 1.3  1994/11/10  21:52:28  wjh
 * fix compile errors on PMax
 * fix core dump when printing with old troff mechanism
 *
 * Revision 1.2  1994/10/29  16:33:00  rr2b
 * don't know if we can make friend functions static, so
 * I just made makegap non-static.  (if static is allowed
 * it woould have to appear in the friend declaration in gflex.H
 * as well as in the definition in gflex.C)
 * I also removed the forward declaration of makegap since it
 * wasn't needed.
 * BUG
 *
 * Revision 1.1  1994/10/14  21:04:02  wjh
 * Initial revision
 * 
 * Revision 1.0  94/08/16  13:10:48  wjh
 */

#include <andrewos.h>
#include <gflex.H>

#ifdef objectsinelts
/* private copy function XXX used if objects are i the elts
*/
#define memcpy memmove
	static void
memmove( ELTTYPE *dest, ELTTYPE *src, int len) {
	if (dest < src)
		while(len--)
			*dest++ = *src++;
	else {
		dest += len;  src += len;
		while (len--)
			*--dest = *--src;
	}
}
#endif /* objectsinelts */



/* move the gap to start at position i and have at least length len 
*/
	void
makegap( gflex *f, int i, int len ) {

#ifdef objectsinelts
	const int esize = 1;
#else /* objectsinelts */
	const int esize = sizeof(ELTTYPE);
#endif /* objectsinelts */

	if (len > f->gaplen) {
		/* have to make elts array bigger */
		int newsize = /* itrunc */( 1.4*(f->n+f->gaplen) );
		if (newsize < len+f->n) 
			newsize = /* itrunc */( 1.2*(len+f->n) );
		ELTTYPE *telts = 
				(ELTTYPE *)malloc(newsize * esize);
		const int newgaplen = newsize - f->n;

		/* not using realloc() because 
			will use = to copy when ELTTYPE is a class 
			Moreover, this way we avoid double copy to move gap. */

		if (i < f->gaploc) {
			/* gap is too far right.  Move:
				part before new gap
				part to be moved to follow the gap
				part after the old gap
			*/
			memcpy( telts, f->elts, i*esize );
			memcpy( telts+i+newgaplen, f->elts+i, (f->gaploc-i)*esize );
			memcpy( telts+f->gaploc+newgaplen,
					f->elts+f->gaploc+f->gaplen,
					(f->n-f->gaploc)*esize );
		}
		else {
			/* gap is ok or too far left. Move: 
				part before existing gap
				part to precede the new gap (if any)
				part to be after gap
			*/
			memcpy( telts, f->elts, f->gaploc*esize );
			if (f->gaploc < i)
				memcpy(  telts+f->gaploc,
					f->elts+f->gaploc+f->gaplen,
					(i-f->gaploc)*esize );
			memcpy( telts+i+newgaplen, f->elts+i+f->gaplen, 
					(f->n-i)*esize );
		}
		free( (void *)f->elts );
		f->elts = telts;
		f->gaplen = newgaplen;
	}
	else if (i < f->gaploc) {
		/* move gap to the left (move segment previously before gap) */
		memmove( f->elts+f->gaplen+i, f->elts+i, 
				(f->gaploc - i)*esize );
	}
	else if (i > f->gaploc) {
		/* move gap to the right (move segment previously after gap) */
		memmove( f->elts+f->gaploc, f->elts+f->gaplen+f->gaploc, 
				(i - f->gaploc)*esize );
	}
	f->gaploc = i;
}


gflex::gflex() {
	const int INITIALSIZE = 100;
	elts = (ELTTYPE *)malloc( INITIALSIZE * sizeof(ELTTYPE) );
	gaplen = INITIALSIZE;
	n = 0;
	gaploc = 0;
}
	
/* discard the gflex array and all contents.
		does not 'destroy' the contained elements 
*/
gflex::~gflex() {

#ifdef objectsinelts
	register i;
	for (i = 0; i < gaploc; i++)
		delete elts[i];
	for (i = gaploc+gaplen; i < n+gaplen; i++)
		delete elts[i];
#endif /* objectsinelts */

	free( elts );
}

/* Returns a reference to the ith element 
*/
	ELTTYPE &
gflex::operator[]( int i )  /* throw(int) */ {
	if (i < 0 || i >= n) /* throw(i) */ {
		fprintf(stderr, "gflex[0...%d] access to non-existent elt %d\n",
			n, i);
		return elts[gaploc];	/* UGHHH */
	}
	if (i < gaploc)
		return elts[i];
	else
		return elts[i+gaplen];
}

/* create n new elts starting at i; RetP 
	  subsequent elements are moved up 
*/
	ELTTYPE *
gflex::insert( int i, int len )  {
	if (i < 0) i = 0;
	if (i > n) i = n;
	makegap( this, i, len );
	n += len;  gaplen -= len;
	int loc = gaploc;
	gaploc += len;
#ifdef objectsinelts
	int j = len;
	while(j--)
		new (&elts[loc+j]) ELTTYPE;  /* placement new ?? */
#endif /* objectsinelts */
	return &elts[loc];
}

/* delete n elts starting at the i'th 
*/
	void
gflex::erase( int i, int len ) {
	if (i < 0 || i >= n) return;
	if (i+len > n) len = n-i;

	if (i <= gaploc && i+len >= gaploc) 
		gaploc = i;	/* erasure straddles gap; just extend gap */
	else 
		makegap( this, i, 0 );	/* move so i'th elt is after gap */
	n -= len;  
	gaplen += len;

#ifdef objectsinelts
	int j;
	for (j = i; j < len; j++)
		ELTTYPE::~ELTTYPE(&elts[j]);  /* ??? */
#endif /* objectsinelts */
}

/* returns the index corresponding to element o.
	Returns -1 if o is not in the gflex 
*/
	int
gflex::efind( const ELTTYPE &o ) {
	const ELTTYPE *p = &o;
	if (p < elts) return -1;
	else if (p < elts+gaploc) return p-elts;
	else if (p < elts+gaploc+gaplen) return -1;
	else if (p < elts+n+gaplen) return p - (elts+gaplen);
	else return -1;
}

/* returns number of elements 
*/
	int
gflex::getn() const {
	return n;
}



/* client asks for pointer to memory containing
	len successive elements starting with the i'th.
	Returned pointer is to the i'th element and
	*gotlenp contains the number of succeeding
	elements stored consecutively after the i'th.
	*gotlenp may be as little as 1 or as much as getn()-i
*/
	ELTTYPE *
gflex::getbuf( int i, int len, int *gotlenp ) {
	if (i < 0) i = 0;
	if (i+len > n) len = n-i;
	if (gaploc <= i) {
		*gotlenp = n-i;
		return &elts[i+gaplen];	/* return a buffer after gap */
	}
	else if (gaploc > i+len) {
		*gotlenp = gaploc - i;
		return &elts[i];	/* return a buffer before gap */
	}
	else if (gaploc-i < len/2) {
		/* the portion before the gap is less than that after, 
			so move the gap to i */
		makegap( this, i, gaplen );
		*gotlenp = n-i;
		return &elts[i+gaplen]; /* buffer is after gap */
	}
	else {
		/* the portion after the gap is smaller. move it */
		makegap( this, i+len, gaplen );
		*gotlenp = gaploc - i;
		return &elts[i];	/* buffer is before gap */
	}
}
