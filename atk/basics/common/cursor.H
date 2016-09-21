ATK_INTER
#ifndef _cursor_H_
#define _cursor_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#include <rect.h>

#include <ATK.H>

class cursor : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


	virtual void SetGlyph(class fontdesc *fd,short ch);
	virtual void SetStandard(short ch);
/************ private methods, not to be called by modules outside of cursor *******/
        virtual void ChangeShape();

        inline boolean IsPosted() { return ((this)->posted != NULL); }
        inline boolean IsWindowCursor() { return ((this)->windowim != NULL); }
        inline boolean IsProcessCursor() { return ((this)->processC); }

        
	cursor();
	static class cursor * Create(class view *v)  ;
	
	virtual ~cursor(); /*  used to get rid of active cursors;
 */

	class view *view;
	class im *posted;
	class im *windowim;
	class cursor *next;
	boolean processC;
	boolean changed;
	class fontdesc *fillFont;
	int fillChar;
};

#define Cursor_Arrow			1
#define Cursor_Gunsight			2
#define Cursor_Cross			3
#define Cursor_Wait			4
#define Cursor_HorizontalBars		5
#define Cursor_LowerRightCorner		6
#define Cursor_UpperLeftCorner		7
#define Cursor_VerticalBars		8
#define Cursor_DangerousBend		9
#define Cursor_Caret			10
#define Cursor_VerticalArrows		11
#define Cursor_HorizontalArrows		12
#define Cursor_Octagon			13
#define Cursor_LeftPointer		14
#define Cursor_RightPointer		15
#define Cursor_CrossHairs		16
#define Cursor_DownPointer		17
#define Cursor_UpPointer		18
#define	Cursor_Cutter			19
#define Cursor_LargeUpArrow		20
#define	Cursor_LargeDownArrow		21
#define	Cursor_EnclosedDot		22
#define	Cursor_SmallCross		23
#define	Cursor_SmallLeftPointer		24
#define	Cursor_SmallRightPointer	25


#define	Cursor_TABLELEN			26
/** @} */
#endif /* _cursor_H_ */
