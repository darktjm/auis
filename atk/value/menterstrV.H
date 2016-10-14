#ifndef _menterstrV_H_
#define _menterstrV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#include <buttonV.H>
class menterstrV : public  buttonV {
public:
	TRACED_CLASS(menterstrV);


    class valueview * DoHit(enum view::MouseAction type, long x,long y,long hits)  ;
    void ObservedChanged( class observable * observed, long status );
    class view * GetApplicationLayer()  ;
    void WantInputFocus(class view *requestor);

   
	menterstrV();
   

    class mentertext *etext;
    class metextview *etextview;
};
/** @} */
#endif /* _menterstrV_H_ */
