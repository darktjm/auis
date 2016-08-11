#ifndef _tmview_H_
#define _tmview_H_
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


 


#include <textview.H>
class tmview: public  textview {
public:
	virtual ATKregistryEntry *ATKregistry();

    
	 void SetDataObject(class dataobject *d);
	 void PostKeyState(class keystate *ks);
	 void PostMenus(class menulist *ml);
	 void FullUpdate(enum view_UpdateType type,long left,long top,long width,long height);
	 void Update();
	 void ReceiveInputFocus();
	 void GetClickPosition(long pos,long noc,enum view_MouseAction action,long startLeft,long startRight,long *leftPosP,long *rightPosP);
    
	virtual void SetFileMenus(boolean on);
	virtual boolean ReadShMenus(char *filename)  ;
    
	inline void SetTitle(char *t) { (this->title=(t)); }
        inline char  * GetTitle() { return (this->title); }
    
	
	static boolean InitializeClass();
	static boolean atk_inited;

        
	tmview();
	
	virtual ~tmview();
    
        int screen;
        class keystate *keystate;
	class menulist *menus, *shmenus;
	class mark *curpos;
	char *cwd, *title;
	int height,width;
};

#endif /* _tmview_H_ */
