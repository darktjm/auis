ATK_INTER
#ifndef _null_H_
#define _null_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *	   Copyright Carnegie Mellon, 1992 - All Rights Reserved          *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
null.H

	null data object

	This module stores the coordinates of a set of blobs and
	provides methods to add blobs and clear the list.

	The file format for a null begins and ends with the standard 
	\begindata \enddata.  There is one intervening line for each blob 
	in the image.  This line gives the coordinates of the upper left corner 
	of the blob as two ASCII integers separated by a space.  The origin 
	for the coordinates is the upper left corner of the rectangle.

*/

/* status values for NotifyObservers */
#define null_DATACHANGED 1

/* $$$ declare any structs that will be needed by users */
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
