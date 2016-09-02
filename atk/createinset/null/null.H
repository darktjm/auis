ATK_INTER
#ifndef _null_H_
#define _null_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *	   Copyright Carnegie Mellon, 1992 - All Rights Reserved
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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


 

/*
null.ch

	null data object

	This module stores the coordinates of a set of blobs and
	provides methods to add blobs and clear the list.

	The file format for a null begins and ends with the standard 
	\begindata \enddata.  There is one intervening line for each blob 
	in the image.  This line gives the coordinates of the upper left corner 
	of the blob as two ASCII integers separated by a space.  The origin 
	for the coordinates is the upper left corner of the rectangle.

*/
/*
 *    $Log: null.H,v $
// Revision 1.4  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.3  1993/06/10  20:24:39  rr2b
// Great atk_inited removal.
//
 * Revision 1.2  1993/06/05  00:32:29  rr2b
 * Converted to C++
 *
 * Revision 1.1  1993/06/04  23:26:51  rr2b
 * Initial revision
 *
*Revision 1.4  1992/12/14  20:40:31  rr2b
*disclaimerization
*
Revision 1.3  1992/06/22  21:48:52  wjh
added types in function declaractions
.MARKER
revised description of __Read
.

Revision 1.2  1991/09/12  19:30:59  bobg
Update copyright notice

Revision 1.1  1989/07/31  15:34:54  wjh
Initial revision

 *
 * Revision 1.0  88/05/14  15:40:32  wjh
 * Copied from /usr/andrew/lib/genericinset
 */




/* status values for NotifyObservers */
#define null_DATACHANGED 1

/* $$$ declare any structs that will be needed */
struct dotlist {
	long x, y;
	struct dotlist *next;
};


#include <dataobject.H>
class null : public  dataobject 
{
public:
	virtual ATKregistryEntry *ATKregistry();




        long Read(FILE *file, long id);
	long Write(FILE *file, long id, int level);


	/* $$$ unique to this data object */
	virtual void AddDot(long x, long y);
		/* add a pair to list of dots */
	virtual void ClearDots();
		/* remove all dots from list */



	/* $$$ unique to this data object */
	inline struct dotlist  * GetFirstDot() { return (this->dots); }
	inline struct dotlist *GetNextDot(struct dotlist *pair) { return (pair->next); }
	inline long GetDotX(struct dotlist *pair) { return (pair->x); }
	inline long GetDotY(struct dotlist *pair) { return (pair->y); }



	
	static boolean InitializeClass();

	
	null();
	
	virtual ~null();



	/* $$$ data values unique to this data object */
	struct dotlist *dots;

};


#endif /* _null_H_ */
