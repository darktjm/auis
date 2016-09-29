#ifndef _lookz_H_
#define _lookz_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblookz liblookz.so
 *  A dynamically loadable ATK class file.  Contains lookz, lookzview,
 *  tabrulerview.
 * @{ */
/*
lookz.H

	lookz data object

	This inset provides for editing the styles of its parent text

	The file format for a lookz has a single line between
	\begindata and \enddata.  The only text on this line is one of the 
	words "visible" or "hidden", determining whether the 
	style editor is exposed or hidden in the text.

*/

#define  lookz_PROGRAMMERVERSION    1

/* status values for NotifyObservers */
#define lookz_VISIBILITYCHANGED 1
#define lookz_CANCLOSECHANGED 2
#define lookz_TEXTOBJECTCHANGED 4

#include <dataobject.H>
class lookz : public  dataobject  {
public:
	virtual ATKregistryEntry *ATKregistry();




	 long Read( /* struct lookz *self, */ FILE *file, long id )  ;
	 long Write( /* struct lookz *self, */ FILE *file, long id, int level )
			 ;
	 void ObservedChanged(/* struct lookzview *self, */ class observable *dobj, long status);


	
	virtual void SetVisibility( /* struct lookz *self, */ boolean visible );
	virtual void SetTextObject(class text *text);
	virtual void SetCanClose(boolean canClose);


	inline boolean GetVisibility( /* struct lookz *self */ ) { return (((class lookz *)this)->visible + 0); }

        inline boolean GetCanClose() { return (this->canClose); }
        inline class text * GetTextObject() { return (this->text); }


	
	lookz();
	
	virtual ~lookz();



	boolean visible;		/* if FALSE, only an icon is displayed
						if TRUE, the editor is displayed 
						this field is constrained to be 
						TRUE or FALSE  */

	boolean canClose;
	class text *text;
};
/** @} */
#endif /* _lookz_H_ */
