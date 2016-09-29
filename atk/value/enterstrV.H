#ifndef _enterstrV_H_
#define _enterstrV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#include <buttonV.H>
class enterstrV : public  buttonV {
public:
	virtual ATKregistryEntry *ATKregistry();


    class valueview * DoHit(enum view_MouseAction type, long x,long y,long hits)  ;
    void ObservedChanged( class observable * observed, long status );
    void LinkTree(class view *parent);
   void DrawButtonText(char *text,long len,struct rectangle *rect,struct rectangle*rect2,boolean pushd);
 class view * Hit(enum view_MouseAction type, long x, long y, long numberOfClicks)
     ;


   
	enterstrV();
   
	virtual ~enterstrV();

    class entertext *etext;
    class etextview *etextviewp;
};
/** @} */
#endif /* _enterstrV_H_ */
