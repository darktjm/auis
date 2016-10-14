#ifndef _sbttnav_H_
#define _sbttnav_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupviews
 * @{ */
struct groups {
    struct sbutton_prefs *prefs;
    class menulist *ml;
    struct groups *next;
    int prio;
};

#include <sbuttonv.H>

class sbttnav : public  sbuttonv {
public:
	TRACED_CLASS(sbttnav);


     boolean Touch(int ind, enum view::MouseAction action)  ;
     void PostMenus(class menulist *ml);

    
	static boolean InitializeClass();

    
	sbttnav();
    

    class menulist *ml;
    struct groups *groups;
    int groupcount;
    int dragging;
    class cursor *cursor;
};
/** @} */
#endif /* _sbttnav_H_ */
