/* ********************************************************************** *\
 *	   Copyright Carnegie Mellon, 1994-1995 - All Rights Reserved     *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* flex.c		

	Code for the flex data object
*/
/*
 *    $Log: flex.C,v $
 * Revision 1.7  1996/01/31  19:37:39  robr
 * Implemented copying and assignment operations. Fixed Remove operations
 * for objects.
 * Made GetN an inline function.
 * FEATURE/BUG
 *
 * Revision 1.6  1995/11/10  17:32:48  robr
 * Added ATK_INTER/ATK_IMPL.
 * BUG
 *
 * Revision 1.5  1995/06/28  15:00:00  rr2b
 * Fixing so that the right function is called to handle deallocation.
 * BUG
 *
 * Revision 1.4  1995/06/27  21:56:48  rr2b
 * Fixed bogosity in flex::DeAllocate.
 * BUG
 *
 * Revision 1.3  1995/03/26  05:43:45  rr2b
 * first round of fixes of the new code
 *
 * Revision 1.2  1995/03/26  05:41:36  rr2b
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
 * Revision 1.3  1994/11/10  21:52:28  wjh
 * fix compile errors on PMax
 * fix core dump when printing with old troff mechanism
 *
 * Revision 1.2  1994/10/29  16:33:00  rr2b
 * don't know if we can make friend functions static, so
 * I just made makegap non-static.  (if static is allowed
 * it woould have to appear in the friend declaration in flex.H
 * as well as in the definition in flex.C)
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
ATK_IMPL("flex.H")
#include <flex.H>


void flex::CopyTo(char *dest, char *src, size_t len) {
    memcpy(dest, src, len);
}

void flex::MoveTo(char *dest, char *src, size_t len) {
    /* assume efficient implementation which adds little if anything to memcpy */
    memmove(dest, src, len);
}

char *flex::Allocate(size_t len) {
    return (char *)malloc(len);
}


size_t flex::RoundSize(size_t n) const {
    return n;
}

void flex::Invalidate(size_t /* i */, size_t /* len */) {
}

/* move the gap to start at position i and have at least length len 
*/
	void
flex::MakeGap(size_t i, size_t len ) {

	if (len > gaplen) {
		/* have to make elts array bigger */
		size_t newsize = /* itrunc */( 14*(n+gaplen)/10 );
		if (newsize < len+n) 
		    newsize = /* itrunc */( 12*(len+n)/10 );
		newsize=RoundSize(newsize);
		char *telts= 
		      (char *)Allocate(newsize);
		
		const size_t newgaplen = newsize - n;

		/* not using realloc() because 
			will use = to copy when char is a class 
			Moreover, this way we avoid double copy to move gap. */

		if(elts) {

		    if (i < gaploc) {
			/* gap is too far right.  Move:
			 part before new gap
			 part to be moved to follow the gap
			 part after the old gap
			 */
			MoveTo( telts, elts, i );
			MoveTo( telts+i+newgaplen, elts+i, (gaploc-i) );
			MoveTo( telts+gaploc+newgaplen,
				elts+gaploc+gaplen,
				(n-gaploc) );
		    }
		    else {
			/* gap is ok or too far left. Move: 
			 part before existing gap
			 part to precede the new gap (if any)
			 part to be after gap
			 */
			MoveTo( telts, elts, gaploc );
			if (gaploc < i)
			    MoveTo(  telts+gaploc,
				     elts+gaploc+gaplen,
				     (i-gaploc) );
			MoveTo( telts+i+newgaplen, elts+i+gaplen, 
				(n-i) );
		    }
		    Deallocate(elts);
		}
		elts = telts;
		gaplen = newgaplen;
	} else {
	    if(elts==NULL) elts=(char *)Allocate(gaplen);
	    if (i < gaploc) {
		/* move gap to the left (move segment previously before gap) */
		if(elts) MoveTo( elts+gaplen+i, elts+i, 
				  (gaploc - i) );
	    } else if (i > gaploc) {
		/* move gap to the right (move segment previously after gap) */
		if(elts) MoveTo( elts+gaploc, elts+gaplen+gaploc, 
				  (i - gaploc) );
	    }
	}
	gaploc = i;
}


static void fdeallocate(char *&ptr) {
    if(ptr) {
	free(ptr);
	ptr=NULL;
    }
}

flex::flex(size_t initialsize) {
    deallocate=fdeallocate;
    elts = NULL;
    gaplen = initialsize;
    n = 0;
    gaploc = 0;
}

/* discard the flex array and all contents.
		does not 'destroy' the contained elements 
*/
flex::~flex() {
    Deallocate( elts );
}

flex::flex(const flex &src) {
    deallocate=fdeallocate;
    elts=NULL;
    n=src.n;
    gaploc=src.n;
    gaplen=src.gaplen;
    if(src.n==0) return;
    elts=Allocate(src.n);
    if(src.gaploc) CopyTo(elts, src.elts, src.gaploc);
    if(src.n>src.gaploc) CopyTo(elts+src.gaploc, src.elts+src.gaploc+src.gaplen, src.n-src.gaploc);
}

flex &flex::operator=(const flex &src) {
    if(gaploc)
	Invalidate(0, gaploc);
    if(n > gaploc)
	Invalidate(gaploc+gaplen, n - gaploc);
    Deallocate(elts);
    elts=NULL;
    n=src.n;
    gaploc=src.n;
    gaplen=src.gaplen;
    if(src.n==0) return *this;
    elts=Allocate(src.n + src.gaplen);
    if(src.gaploc) CopyTo(elts, src.elts, src.gaploc);
    if(src.n>src.gaploc) CopyTo(elts+src.gaploc, src.elts+src.gaploc+src.gaplen, src.n-src.gaploc);
    return *this;
}


    
/* Returns a reference to the ith element 
*/
char & flex::operator[]( size_t i )  /* throw(int) */ {
    if (/* i < 0 || */ i >= n || elts==NULL) /* throw(i) */ {
	fprintf(stderr, "flex[0...%lu] access to non-existent elt %lu\n",
		n, i);
	return elts[gaploc];	/* UGHHH */
    }
    if (i < gaploc)
	return elts[i];
    else
	return elts[i+gaplen];
}

const char & flex::operator[]( size_t i ) const  /* throw(int) */ {
    return (*this)[i];
}
/* create n new elts starting at i; RetP 
	  subsequent elements are moved up 
*/
	char *
flex::Insert( size_t i, size_t len )  {
	/* if (i < 0) i = 0; */
	if (i > n) i = n;
	MakeGap(i, len );
	n += len;  gaplen -= len;
	size_t loc = gaploc;
	gaploc += len;
	return &elts[loc];
}

/* delete n elts starting at the i'th 
*/
	void
flex::Remove( size_t i, size_t len ) {
	    if (/* i < 0 || */ i >= n) return;
	    if (i+len > n) len = n-i;

	    if (i <= gaploc && i+len >= gaploc) {
		if(gaploc>i) {
		    Invalidate(i, gaploc-i);
		}
		if(i+len>gaploc+gaplen) {
		    Invalidate(gaploc+gaplen, len-(gaploc-i));
		}
		gaploc = i;	/* erasure straddles gap; just extend gap */
	    }
	    else {
		MakeGap(i, 0 );	/* move so i'th elt is after gap */
		Invalidate(i, len);
	    }
	    n -= len;  
	    gaplen += len;
}

/* returns the index corresponding to element o.
	Returns GetN() if o is not in the flex 
*/
	size_t
flex::Find( const char &o ) const {
	const char *p = &o;
	if (elts==NULL || p < elts) return GetN();
	else if (p < elts+gaploc) return p-elts;
	else if (p < elts+gaploc+gaplen) return GetN();
	else if (p < elts+n+gaplen) return p - (elts+gaplen);
	else return GetN();
}


/* client asks for pointer to memory containing
	len successive elements starting with the i'th.
	Returned pointer is to the i'th element and
	*gotlenp contains the number of succeeding
	elements stored consecutively after the i'th.
	*gotlenp may be as little as 1 or as much as getn()-i
*/
	char *
flex::GetBuf( size_t i, size_t len, size_t *gotlenp ) {
	if(elts==NULL || i > len) {
	    *gotlenp = 0;
	    return NULL;
	}
	/* if (i < 0) i = 0; */
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
		MakeGap(i, gaplen );
		*gotlenp = n-i;
		return &elts[i+gaplen]; /* buffer is after gap */
	}
	else {
		/* the portion after the gap is smaller. move it */
		MakeGap(i+len, gaplen );
		*gotlenp = gaploc - i;
		return &elts[i];	/* buffer is before gap */
	}
}
