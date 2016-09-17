ATK_INTER
#ifndef _dialogv_H_
#define _dialogv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
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

#include <view.H>

	typedef void (*dialogv_choicefptr)(long rock, int ind, long brock);

class dialogv : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

    
	dialogv();
    
	virtual ~dialogv();
	struct HitRock {
	    dialogv_choicefptr func;
	    long rock;
	};

    static class dialogv * Create(const char * const *list, const char *font, int style)  ;
    
    
     void SetDataObject(class dataobject *dataobject);
     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);

     class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
     view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
     void LinkTree(class view *parent);
     void PostMenus(class menulist *ml);
     void ReceiveInputFocus();
    

    inline class textview * GetTextView() { return ((this)->text); }
    inline class sbuttonv * GetButtonsView() { return ((this)->buttons); }
    inline class text * GetTextData() { return ((class text *)((class view *)(this)->text)->GetDataObject()); }
    inline class sbutton * GetButtonsData() { return ((class sbutton *)((class view *)(this)->buttons)->GetDataObject()); }
    inline class menulist * GetExtraMenus() { return ((this)->extramenus); }
    inline class keystate * GetExtraKeys()  { return ((this)->extrakeys); }
    

   virtual int PostChoice(class im *im, class view *client, boolean *cflag, int deflt, boolean blocking, long pos)  ;
   virtual int PostInput(class im *im, class view *client, dialogv_choicefptr choicefunc, long choicerock, boolean blocking, long pos)  ;
   virtual void SetLayout(int rows, int cols);
   virtual void InstallRider(class view *rider);
   virtual void InstallSidekick(class view *sidekick);
   virtual void SetExtraMenus(class menulist *ml);
   virtual void SetExtraKeyState(class keystate *ks);
   virtual void ActivateButton(int ind);
   virtual void DeActivateButton(int ind);
   virtual void Vanish();
   virtual void UnVanish();
   virtual void CancelQuestion();
   

    class textview *text;
    class sbuttonv *buttons;
    class view *rider, *sidekick;
    long rwidth, rheight, roffset;
    long twidth, theight;
    long swidth, sheight;
    long bwidth, bheight;
    boolean destroyall;
    struct HitRock hr;
    class menulist *extramenus;
    class keystate *extrakeys;
    class view *client;
    boolean didsizeonce;
    int lastchoice;
    boolean *cont;
};


#endif /* _dialogv_H_ */
