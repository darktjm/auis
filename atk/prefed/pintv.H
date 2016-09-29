#ifndef _pintv_H_
#define _pintv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <lpair.H>
class pintv : public  lpair {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

    
	pintv();
    
	virtual ~pintv();


     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
     void SetDataObject(class dataobject *d);
     class view * Hit(enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
     void WantInputFocus(class view *requestor);
     void WantUpdate(class view *requestor);
     void ObservedChanged(class observable *changed, long value);


    class keystate *ks;
    class menulist *menulist;
    boolean redosizes;
    class sbuttonv *buttons;
    class view *cpref_al;
    class textview *cpref;
    class lpair *top;
    class lpair *labelpair;
    class lpair *leftpair;
    class lpair *rightpair;
    class lpair *blr;
    class labelview *catlabel;
    class view *categories_al;
    class textview *categories;
    class labelview *plabel;
    class view *preferences_al;
    class textview *preferences;
    class environment *cat_sel, *pref_sel;
    const char *category;
    const char *pref;
    struct prefdesc *cpd;
    class list *prefslist;
    boolean newerrors;
    class text *errors;
    class event *reportevent;
    class event *uevent;
    class event *oevent;
    class phelpv *helpv;
    class view *helpva;
    struct sbutton_prefs *prefs;
    boolean autolist;
    boolean lockdown;
};
/** @} */
#endif /* _pintv_H_ */
