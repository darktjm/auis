#ifndef _fillerview_H_
#define _fillerview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblayout
 * @{ */
/* fillerview.H - filler view definition and interface */
#define fillerview_VERSION 1

/* Interface definition */

#include <celview.H>
class fillerview: public  celview {
public:
	TRACED_CLASS(fillerview);



   void FullUpdate(enum view::UpdateType how, long left, long top, long width, long height);
   void Update();
   class view * Hit(enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
   void ReceiveInputFocus();
   void LoseInputFocus();


  virtual void SetDataObjectByName(const char *dataname);	/* replace myself */


  
	static boolean InitializeClass();
	/* initialize class */
  
	fillerview();	/* initialize instance */
  


	boolean hasInputFocus;
        class menulist *menulistp;
	int hitindex;
};

/* end of fillerview.H */
/** @} */
#endif /* _fillerview_H_ */
