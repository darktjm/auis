#ifndef _strtblview_H_
#define _strtblview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupviews
 * @{ */
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

	 void FullUpdate( enum view::UpdateType type, 
		long left, long top, long width, long height );
	 void Update();
	 class view * Hit( enum view::MouseAction action, long x, long y, long n)
			 ;
	 view::DSattributes DesiredSize( long width, long height, enum view::DSpass pass, 
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
	boolean OnScreen;			/* if not view::Removed */
	boolean GaveSize;		/* as DesiredSize succeeded */
	boolean tablechanged;		/* set by ObservedChanged */
	boolean sizeknown;			/* set after scanning strings to get maxwidth */
	short rows, cols;			/* layout of the screen area */
	short itemheight, maxwidth;	/* max dimensions of an item */
	short maxused;				/* max number of items ever found */
	class graphic  *BlackPattern;	/* these might differ between instances . . . */
	class graphic  *WhitePattern;	/*		. . . on different displays */

};
/** @} */
#endif /* _strtblview_H_ */
