ATK_INTER
#ifndef _contentv_H_
#define _contentv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <textview.H>

class contentv : public  textview  {
public:
	virtual ATKregistryEntry *ATKregistry();


      void PostMenus(class menulist *menulist);
      void SetDotPosition(long newpos);
      void GetClickPosition(long position, long numberOfClicks, enum view_MouseAction action, long startLeft, long startRight, long *leftPos, long *rightPos);

    contentv();
    ~contentv();
	static boolean InitializeClass();

    static void MakeWindow(class text *self);

inline void SetSourceView(class textview *vw) { this->srcview = vw; }

    class menulist *menus;
    class textview *srcview;
};

#endif /* _contentv_H_ */
