ATK_INTER
#ifndef _label_H_
#define _label_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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
label.H

	label data object

	stores the font and text string for labelview

	The file format for a label has multiple lines between the lines for
	\begindata and \enddata.  The first line has a font specification consisting
	of familyname, style(as a long value), and size.  
	Subsequent lines are the text to be displayed.
	The text is truncated to be no more than 255 characters 
		and backslashes and curly braces are removed.
	The default font is andy10b.
	If there is no text, the self->text field is NULL.  (strlen(self->text) is never 0)

*/

#define  label_PROGRAMMERVERSION    1

/* status values for NotifyObservers */
#define label_DATACHANGED 1

#include <graphic.H>

/* values for various attributes */

#define	label_NOJUST		graphic_NOMOVEMENT
#define	label_LEFTJUST		graphic_ATLEFT
#define	label_RIGHTJUST		graphic_ATRIGHT
#define	label_HCENTERJUST	graphic_BETWEENLEFTANDRIGHT
#define	label_TOPJUST		graphic_ATTOP
#define	label_BOTTOMJUST	graphic_ATBOTTOM
#define	label_VCENTERJUST	graphic_BETWEENTOPANDBOTTOM
#define	label_CENTERED		label_HCENTERJUST | label_VCENTERJUST

#define	label_BOXED		0x80


#include <dataobject.H>

class label : public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();




	 long Read(/* struct label *self, */ FILE *file, long id)  ;
	 long Write(/* struct label *self, */ FILE *file, long id, int level)
			 ;



	virtual void SetText(/* struct label *self, */ const char *text);
		/* the text is copied within the struct label, so the source is no longer needed */
	virtual void SetFont(/* struct label *self, */ const char *fontfamily, long style, long size);
		/* The parameters to SetFont are the same as those to fontdesc_Create */
	virtual char * GetText(/* struct label *self */)  ;
		/* returns a pointer to the text.  It must not be modified by the caller */
	virtual const char * GetFont(/* struct label *self, */ long *style, long *size)  ;
		/* returns a pointer to an internal buffer containing the font family name;
			it must not be modified */
	virtual void SetFlags(/* struct  label *self, */ int flags);
		/* sets the internal flags */


	inline int GetFlags() { return (this->flags); }



	
	label();
	
	virtual ~label();



	char *text;	/* the text itself */
	int flags;		/* justification and whether boxed, as per above */
	class fontdesc *font;	/* the font */
};


#endif /* _label_H_ */
