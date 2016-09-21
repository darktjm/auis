ATK_INTER
#ifndef _nessview_H_
#define _nessview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved
\* ********************************************************************** */

/** \addtogroup libness
 * @{ */
#include <scroll.H>
#include <textview.H>

class nessview  : public  scroll {
public:
	virtual ATKregistryEntry *ATKregistry();



	void FullUpdate(enum view_UpdateType type,
			long left, long top, long width, long height);
	void Update();
	class view * Hit (enum view_MouseAction action, 
			long x, long y, long numberOfClicks);
	view_DSattributes DesiredSize(long width, long height, 
			enum view_DSpass pass, long *desiredWidth, long *desiredHeight);
	void SetDataObject(class dataobject *dobj);

	void WantInputFocus(class view *requestor);
	void ReceiveInputFocus();
	void LoseInputFocus();
	void PostMenus (class menulist *menulist);
	void PostKeyState (class keystate *keystate);
	void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
	void ObservedChanged(class observable *changed, long value);

	virtual void FirstError();
		/* display message and highlight for first error */

	static boolean InitializeClass();
	static boolean atk_inited;
 
			/* Create default Keymap & Menus */
	
	nessview();
	virtual ~nessview();


	class textview *SourceText;	/* display the ness script */
	char *ExecFunction;		/* default function for ^X^E */
	struct errornode *CurrError;		/* which one is being displayed */

	long compmod;			/* GetModified at last compile */
	long ButtonPending;		/* used in __Hit */

	class menulist  *Menus;
	class keystate *Keystate;

	boolean MenusPostedRecently;	/* internal to PostMenus, RcvIF,  */
	boolean KeystatePostedRecently; 	/*    and FullUpdate  */

	boolean HasIF;
	boolean ErrorHighlightPending;

	boolean inverted;
	boolean dialogboxup;
	boolean justscanned;	/* if most recent compile was a scan */
	textviewInterface *ti;
};
/** @} */
#endif /* _nessview_H_ */
