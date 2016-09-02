ATK_INTER
#ifndef _nessview_H_
#define _nessview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


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

#endif /* _nessview_H_ */
