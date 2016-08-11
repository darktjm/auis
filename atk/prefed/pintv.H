ATK_INTER
#ifndef _pintv_H_
#define _pintv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

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
    char *category;
    char *pref;
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

#endif /* _pintv_H_ */
