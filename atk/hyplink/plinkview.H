#ifndef _plinkview_H_
#define _plinkview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblink
 * @{ */
#include <linkview.H>

class plinkview  : public  linkview  {
    public:
      TRACED_CLASS(plinkview);



      static boolean InitializeClass();


      plinkview();


      void PostMenus(class menulist *ml);

      class menulist *pml;
};
/** @} */
#endif /* _plinkview_H_ */
