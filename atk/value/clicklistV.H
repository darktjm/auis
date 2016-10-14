#ifndef _clicklistV_H_
#define _clicklistV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#include <stringV.H>
class clicklistV : public  stringV {
public:
	TRACED_CLASS(clicklistV);


    void ObservedChanged( class observable * observed, long status );
    class view * GetApplicationLayer()  ;
    void WantInputFocus(class view *requestor);

   
	clicklistV();
   

    class text *textp;
    class cltextview *cltextviewp;
    long count,csize;
    char *choice;
    boolean choicechanged;
};
/** @} */
#endif /* _clicklistV_H_ */
