#ifndef _gofig_H_
#define _gofig_H_
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

/*
gofig.H

	go figure/diagram data object

	This module stores a board image or square fragment 
			with stones, numbers, and other decoration.

	The file format for a gofig object begins and ends with the standard 
		\begindata \enddata.  
	The first line lists
		data stream version number (is 1)
		width of board (in lines)
		height of board (in lines)
		nominal separation of vertical lines for printing (in points*1000)
			(horizontal lines are spaced 31/29's of vertical lines)
		booleans (1=TRUE,0=FALSE) to indicate edgeness of
			left, right, top, bottom
		boolean indicating whether indices (1-19 A-T) are displayed 
	Following lines contain the data for each non-empty intersection:
		row number (top is zero)
		column number (leftmost is zero)
		W for white, B for black, / for empty
		an integer for number to display  (zero is none)
		a character to display	(blank is none)
			may be an alphabetic character
			special characters include:
				^ for triangle
				+ for cross
				% for X
				@ for circle
				# for square
				- for dash
				~ for dash with line up (win by compensation)
				_ for dash with line down (lose by compensation)
				& for upside down triangle
		(If a number is given, the character is ignored.)
*/


/* status values for NotifyObservers */
#define gofig_DATACHANGED 1
#define gofig_SIZECHANGED 2

#include <gflex.H>

#include <dataobject.H>
class gofig : public  dataobject 
{
public:
	static long accnum;
	int width, height;
	unsigned edges;
#define LEFTedge 1
#define RIGHTedge 2
#define TOPedge 4
#define BOTTOMedge 8
	/* if (edges&XXXedge) is non-zero, the corresponding side is an edge */
#define INDICESedge 16 
	/* if edge&INDICESedge is non-zero, the indices (1-19 A-T) are shown */

	long printcolwidth;
	gflex *stones;

	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();

        long Read(FILE *file, long id);
	long Write(FILE *file, long id, int level);
	void WriteASCII(FILE *file);

	gofig();
	gofig(int w, int h);
	virtual ~gofig();

	void setdimensions(int w, int h);
		/* changes width and height.  
		  Any out-of-bounds stones are removed. */
	inline void getdimensions(int *w, int *h) {*w = width; *h = height;}
	inline long getaccnum() {return accnum;}

	inline struct stone &operator[]( int i ) {return (*stones)[i];};
	
	struct stone *addstone(int row, int col);
		/* adds a stone at the indicated location 
			(left = 0, top = 0)
		  if location is within 0...23, but outside current
			board bounds, the board bounds are adjusted upward
		  returns a pointer to the new stone so client can adjust it */

	void deletestone(int row, int col);
		/* removes the indicated stone */
	inline void deletestone(struct stone *s) {(stones)->erase( s );}
	inline void deletestone(struct stone &s) {(stones)->erase( s );}

	inline int nstones() {return stones->getn();}

	inline void getedges(unsigned *e)  {*e = edges;}
	inline void setedges(unsigned e)  {edges = e;}

	inline void changed(struct stone *s) {s->accnum = ++accnum; SetModified();
			NotifyObservers( gofig_DATACHANGED );}
	inline void changed(struct stone &s) {s.accnum = ++accnum; SetModified();
			NotifyObservers( gofig_DATACHANGED );}
	inline void changed(int i) {changed( (*stones)[i] );}
};

#endif /* _gofig_H_ */
