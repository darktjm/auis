#ifndef _fadview_H_
#define _fadview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libfad
 * @{ */
#include <fad.H>
#include <view.H>
class fadview : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


	 view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
	 void Update();
    	 void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
   	 void ReceiveInputFocus();
    	 void LoseInputFocus();
    	 class view * Hit (enum view_MouseAction action, long mousex, long mousey, long numberOfClicks)  ;
	 void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
	 void ObservedChanged( class observable *changed, long value);
	 void SetDataObject(class dataobject *dataObject);

	virtual void aniframe(int framecount,int startat,int gofor,int mtm);
	virtual void nextframe(class fad *cp);
	virtual void lastframe(class fad *cp);
	virtual void showfad(int i,class fad *cp);
	virtual int geticons(char *s) ;	
	virtual void fileread(char *fnm);



	fadview();

	virtual ~fadview();

	static boolean InitializeClass();


	int HasFocus;
	class keystate *keystatep;
	class menulist *menulistp;
	int framecount;
	int startat;
	int gofor;
	int mtm;
	int DoAnimation;
	class cursor *cursor;
	int Moving;
    int removed;
    int anbufsize;
    boolean Redraw,FrameChanged,animationPending,needUpdate,FocusChanged;
    struct fad_frame *f;
    int mode;
struct aniinfo *anobj;
class event *nextevent;
boolean updatedue;
};
/** @} */
#endif /* _fadview_H_ */
