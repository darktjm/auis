ATK_INTER
#ifndef _strtblview_H_
#define _strtblview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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
strtblview.H

	Class definitions for stringtbl view

	Displays the contents of a string table.
	When there is a hit on an item, the client can have a procedure called.
	The client calls strtblview_SetItemHitProc(table, proc, rock).
		When there is a hit on the table, a call to the proc is made with
		the following parameters:
			hitproc(stringtbl, rock, accnum)
		where rock is the value passed to SetItemHitProc and
		accnum is the accession number returned by stringtbl_AddString

		The visual organization of a stringtbl is based on the maximum
		number of elements that table has ever had, rather than its current
		contents.  To reset this number, use strtblview_Clear instead 
		of stringtbl_Clear;  the two _Clear routines are otherwise identical.

*/

#define  strtblview_PROGRAMMERVERSION    1

#include <view.H>
	typedef void (*strtblview_hitfptr)(class stringtbl *st, long rock, long accmap);

class strtblview : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();

	 void FullUpdate( enum view_UpdateType type, 
		long left, long top, long width, long height );
	 void Update();
	 class view * Hit( enum view_MouseAction action, long x, long y, long n)
			 ;
	 view_DSattributes DesiredSize( long width, long height, enum view_DSpass pass, 
				long *desiredWidth, long *desiredHeight ) 
			 ;
	 void ObservedChanged( class observable *dobj, long status );
	 void Print( FILE *file, const char *processor, const char *finalFormat, boolean topLevel );



	virtual void Clear();	/* clears the string table and also resets 
		the maximum number of elements seen in the view */

	

	inline void SetItemHitProc(strtblview_hitfptr proc, long rock) { (this->ItemHitRock = (long)rock,
						this->ItemHitProc = proc); }
		/* store a proc to call for a hit */
	inline strtblview_hitfptr GetItemHitProc()	{ return ((this->ItemHitProc))		/* untested */; }
	inline long GetItemHitRock()	{ return ((this->ItemHitRock))		/* untested */; }



	
	strtblview();
	
	virtual ~strtblview();

	/* the following are options for SetItemHitProc.  
		The rock passed to the stv parameter is ignored */
	static void ZeroOrOne(/* struct classhdr *ClassID;*/ 
			class stringtbl *st, void *stv, short item);
	static void ZeroOrMany(/* struct classhdr *ClassID;*/ 
			class stringtbl *st, void *stv, short item);
	static void OneOnly(/* struct classhdr *ClassID;*/ 
			class stringtbl *st, void *stv, short item);



	unsigned long BlackOnes;		/* which items have black boxes on display */
	strtblview_hitfptr ItemHitProc;		/* called when mouse hits an item */
		/* ItemHitProc(stringtbl, ItemHitRock, accnum of hit item) */
	long ItemHitRock;			/* passed as second arg to ItemHitProc */
	boolean OnScreen;			/* if not view_Removed */
	boolean GaveSize;		/* as DesiredSize succeeded */
	boolean tablechanged;		/* set by ObservedChanged */
	boolean sizeknown;			/* set after scanning strings to get maxwidth */
	short rows, cols;			/* layout of the screen area */
	short itemheight, maxwidth;	/* max dimensions of an item */
	short maxused;				/* max number of items ever found */
	class graphic  *BlackPattern;	/* these might differ between instances . . . */
	class graphic  *WhitePattern;	/*		. . . on different displays */

};

#endif /* _strtblview_H_ */
