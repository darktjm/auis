ATK_INTER
#ifndef _frameview_H_
#define _frameview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libframe
 * @{ */
#define frameview_VERSION 1

#include <textview.H>

class frameview : public  textview {
public:
	virtual ATKregistryEntry *ATKregistry();

    
         void Update();
         void WantInputFocus(class view *requestor);
         void LoseInputFocus();
         void PostMenus(class menulist *menulistp);
         void PostKeyState(class keystate *keystatep);
	 view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
	 void WantNewSize(class view *requestor);
	 void LinkTree(class view *parent);
	
    
        virtual void SetMessageLine(class framemessage *messageLine);
        virtual class framemessage * GetMessageLine()  ;
	virtual void Return(long key); 
	virtual void Help(long key); 
	virtual void Complete(long key); 
	virtual void SetWantedLines(int lines);
	
    
        
	virtual ~frameview();
        static class frameview * Create(class framemessage *messageLine)  ;
        
	static boolean InitializeClass();

	
	frameview();    

    
        class framemessage *messageLine;
        class keystate *keystatep;
        class menulist *menulistp;
        class mark *transientMark;
	class event *eventp; /* Timer evet to erase transient messages. */
	boolean amLosingInputFocus;
	long lineHeight, lines;
	long wantsize;
	boolean dynamicsize;
	long minlines;
};
/** @} */
#endif /* _frameview_H_ */
