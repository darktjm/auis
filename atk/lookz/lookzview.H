#ifndef _lookzview_H_
#define _lookzview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblookz
 * @{ */
/*
lookzview.H

	Class definitions for lookz view
*/


#define  lookzview_PROGRAMMERVERSION    1

#include <view.H>
class lookzview : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();



	 void FullUpdate(/* struct lookzview *self, */ enum view_UpdateType type, 
		long left, long top, long width, long height);
	 void Update(/* struct lookzview *self */ );
	 class view * Hit(/* struct lookzview *self, */ enum view_MouseAction action, long x, long y, long n)
			 ;
	 view_DSattributes DesiredSize(/* struct lookzview *self, */ long width, long height, enum view_DSpass pass, 
				long *desiredWidth, long *desiredHeight) 
			 ;
	 void WantInputFocus(/* struct lookzview *self, */ class view *child);
	 void ReceiveInputFocus(/* struct lookzview *self */);
	 void LoseInputFocus(/* struct lookzview *self */);
	 void Print(/* struct lookzview *self, */ FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
	 void ObservedChanged(/* struct lookzview *self, */ class observable *dobj, long status);
	 class view * GetApplicationLayer(/* struct lookzview *self */)  ;
	 void LinkTree(/* struct lookzview *self, */ class view *parent);
	 void SetDataObject(class dataobject *dobj);



	virtual void SetVisibility( /* struct lookzview *self, */ boolean visibility );
	virtual boolean GetVisibility( /* struct lookzview *self */ )  ;
	virtual void SetStyleSheet( /* struct lookzview *self, */ class stylesheet *ss );
	virtual class stylesheet * GetStyleSheet( /* struct lookzview *self */ )  ;

	virtual void SetEmbedded(boolean isEmbedded);


        inline boolean GetEmbedded() { return this->embedded;; }



	
	static boolean InitializeClass();

			/* Create default Keymap & Menus */
	
	lookzview();
	
	virtual ~lookzview();



	boolean embedded;
	class stringtbl *(st[12]);
	class strtblview *(stv[12]);
	class lprruler *ruler;
	class lprrulerview *rulerview;
	class lprruler *tabruler;
	class tabrulerview *tabrulerv;
	class lpair *image;
	class labelview *shrinkicon;
	class lpair *shrinkparent;
	boolean Linked;
	boolean OnceOnlyInUpdate;

	boolean foundstylesheet;
	class stylesheet *curss;		/* the stylesheet being edited */
	char *curcard;			/* points to the stringtbl name entry for
							current menucard */
	class style *curstyle;		/* the style named by menucard/stylename */

	class menulist *MyMenus;	/* copy of the class menus in MenuList */
	class keystate *Keystate;		/* retain state for multi-keystroke inputs */
	boolean HasInputFocus;
	boolean NeedsUnpack;	/* specifically for change to stylesheet (via another lookz) */
	boolean OnScreen;
};
/** @} */
#endif /* _lookzview_H_ */
