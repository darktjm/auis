#ifndef _contentv_H_
#define _contentv_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libtextaux
 * @{ */
#include <textview.H>

class contentv : public  textview  {
public:
	TRACED_CLASS(contentv);


      void PostMenus(class menulist *menulist);
      void SetDotPosition(long newpos);
      void GetClickPosition(long position, long numberOfClicks, enum view::MouseAction action, long startLeft, long startRight, long *leftPos, long *rightPos);

    contentv();
	static boolean InitializeClass();

    static void MakeWindow(class text *self);

inline void SetSourceView(class textview *vw) { this->srcview = vw; }

    class menulist *menus;
    class textview *srcview;
};
/** @} */
#endif /* _contentv_H_ */
