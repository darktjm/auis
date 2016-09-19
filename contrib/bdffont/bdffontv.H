#ifndef _bdffontv_H_
#define _bdffontv_H_
/* bdffontv.ch  -  view for font editor for bdf files */

/*
	Copyright Carnegie Mellon University 1991, 1992 - All rights reserved
*/

#include <bdffont.H>

#include <lpair.H>
class bdffontv : public  lpair {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	
	static boolean InitializeClass();
	static boolean atk_inited;

	
	bdffontv();
	
	virtual ~bdffontv();
    
	 void PostKeyState(class keystate *k);
	 void PostMenus(class menulist *m);
	 void SetDataObject(class dataobject *dataobject);
	 void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
	 void ObservedChanged(class observable *changed, long value);
	 void ReceiveInputFocus();
	 void WantInputFocus(class view *target);
    
	virtual void SelectCharacter(int encoding);
	virtual boolean UpdateCharModification()  ;
    
	boolean initialized;
	long charmodified;
	long rastermodified;
	struct bdffont_fontchar *charinfo;
	struct bdffont_fontchar modinfo;
	long zoomdelta;
	long prevzoom;
	long defns_size;

	class chlist *charmenuL;

	class keystate *keys;
	class menulist *menus;

	class rasterview *chareditV;
	class raster *charedit;

	class lpair *fontinfo;
	class lpair *fontdata;
	class lpair *fontpresentation;
	class lpair *fontsummary;
	class lpair *ptres;
	class lpair *originextent;
	class lpair *menuedit;
	class lpair *infoedit;
	class lpair *encodingextent;
	class chlistview *fontnameV;
	class chlistview *fontfamilyV;
	class chlistview *fontfaceV;
	class chlistview *pthelpV;
	class chlistview *resV;
	class chlistview *fontoriginV;
	class chlistview *fontextentV;
	class chlistview *charmenuV;
	class chlistview *charencodingV;
	class chlistview *charextentV;
};

#endif /* _bdffontv_H_ */
