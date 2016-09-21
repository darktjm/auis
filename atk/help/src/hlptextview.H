ATK_INTER
#ifndef _hlptextview_H_
#define _hlptextview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libhelp
 * @{ */
#include <textview.H>
class hlptextview: public  textview {
public:
	virtual ATKregistryEntry *ATKregistry();

 
  
     void GetClickPosition(long position,long numberOfClicks,enum view_MouseAction action,long startLeft,long startRight,long *leftPos,long *rightPos);

  
 
  
    
	static boolean InitializeClass();

    
	hlptextview();
    
	virtual ~hlptextview();
 
  
    inline void SetHyperlinkCheck(boolean newValue) { ((this)->checkForHyperlinks = (newValue)); }
    inline boolean   GetHyperlinkCheck() { return ((this)->checkForHyperlinks); }
    inline char  * GetHyperTopic() { return ((this)->hyperTopic); }
    inline view_MouseAction   GetMouseAction() { return ((this)->action); }

  
    boolean checkForHyperlinks;	    /* TRUE when watching for hypertext styles */
    char *hyperTopic;		    /* Non-NULL if checkForHyperlinks & we found one. */
    enum view_MouseAction action;   /* Action that set hyperTopic */
};
/** @} */
#endif /* _hlptextview_H_ */
