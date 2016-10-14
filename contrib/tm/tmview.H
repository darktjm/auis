#ifndef _tmview_H_
#define _tmview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtermulator
 * @{ */
#include <textview.H>
class tmview: public  textview {
public:
	TRACED_CLASS(tmview);

    
	 void SetDataObject(class dataobject *d);
	 void PostKeyState(class keystate *ks);
	 void PostMenus(class menulist *ml);
	 void FullUpdate(enum view::UpdateType type,long left,long top,long width,long height);
	 void Update();
	 void ReceiveInputFocus();
	 void GetClickPosition(long pos,long noc,enum view::MouseAction action,long startLeft,long startRight,long *leftPosP,long *rightPosP);
    
	virtual void SetFileMenus(boolean on);
	virtual boolean ReadShMenus(const char *filename)  ;
    
	inline void SetTitle(const char *t) { (this->title=(t)); }
        inline const char  * GetTitle() { return (this->title); }
    
	
	static boolean InitializeClass();
	static boolean atk_inited;

        
	tmview();
	
    
        int screen;
        class keystate *keystate;
	class menulist *menus, *shmenus;
	class mark *curpos;
	char *cwd;
	const char *title;
	int height,width;
};
/** @} */
#endif /* _tmview_H_ */
