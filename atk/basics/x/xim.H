ATK_INTER
#ifndef _xim_H_
#define _xim_H_
/* ********************************************************************** *\
  *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
  *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* ********************************************************************** *\
 *	   Copyright Carnegie Mellon, 1996 - All Rights Reserved
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

/* im.ch
 * Class header file for the interface manager.
 *
 *
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define xim_PRORGAMMERVERSION 1

#define xim2valid(AnXIMPtr) (((class xgraphic *) ((AnXIMPtr)->drawable))->valid)
#define xim2window(AnXIMPtr) (((class xgraphic *) ((AnXIMPtr)->drawable))->localWindow)
#define xim2display(AnXIMPtr) (((class xgraphic *)((AnXIMPtr)->drawable))->XDisplay())
#define xim2fd(AnXIMPtr) (((class xgraphic *)((AnXIMPtr)->drawable))->xfd)
#define xim2gc(AnXIMPtr) (((class xgraphic *)((AnXIMPtr)->drawable))->XGC())
#define xim2screen(AnXIMPtr) (((class xgraphic *)((AnXIMPtr)->drawable))->XScreen())
#define xim2fillgc(AnXIMPtr) (((class xgraphic *)((AnXIMPtr)->drawable))->XFillGC())
#define xim2graphic(AnXIMPtr) ((class xgraphic *) ((AnXIMPtr)->drawable))

#define xim2programname(AnXIMPtr) ((AnXIMPtr)->programname?(AnXIMPtr)->programname:"No program name set!")

struct pane {
	char * paneStr;
	long priority;
	struct selection *firstSelection;
	struct pane *next;
};

struct selection  {
	char * selectionStr;
	long priority;
	struct proctable_Entry *proc;
	ATK  *object;
	long data;
	struct selection *next;
};

#define MENUVIEWCACHESIZE 57
struct menuviews {
    class view *view;
    int refs;
    struct menuviews *next;
};

#include <im.H>
#include <ahash.H>

class xim : public  im  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
        virtual void UnlinkNotification(class view *tree);
	virtual boolean RequestSelectionOwnership(class view *requestor)  ;
	virtual void GiveUpSelectionOwnership(class view *requestor);

	virtual void WantInputFocus(class view *requestor);
	
        virtual void ObservedChanged(class observable *changed, long value);
	virtual unsigned char * WhichWS()  ;
	virtual void PostMenus(class menulist *menulist);
	virtual void PostCursor(struct rectangle *rec,class cursor *cursor) ;
	virtual void ClearCursors(class cursor *C);
	virtual void UpdateCursors();
	virtual void SetTitle(char *titleString);
	virtual FILE * FromCutBuffer()  ;
	virtual FILE * OnlyFromCutBuffer();
	virtual FILE * OnlyFromSelection();
	virtual void CloseToCutBuffer(FILE *writeFile);
	virtual void RotateCutBuffers(long count);
	virtual void AppendToCutBuffer(FILE *writeFile);
	virtual char *WantInformation(char *key);

	/* These are window manager interface calls */
	virtual void SetWMFocus();
	virtual void ExposeWindow();
	virtual void HideWindow();
	virtual void VanishWindow();
	virtual void HandleRedraw();
	virtual boolean CreateWindow(char * host)  ;
	virtual boolean CreateTransientWindow(class im *other, int flags, im_configurefptr cfp, long crock)  ;
	virtual boolean CreateOverrideWindow(class im *other, im_configurefptr cfp, long crock)  ;
	virtual void RedrawWindow();
	virtual class view * Hit (enum view_MouseAction action, long x, long y, long clicks)  ;

	/* These are part of the transient and overide support. */
	virtual boolean SupportsTransient()  ;
	virtual boolean SupportsOverride()  ;
	/* offscreen window support */
	virtual boolean SupportsOffscreen()  ;
	virtual boolean CreateOffscreenWindow(class im *other, int width, int height)  ;
	virtual class colormap * CreateColormap()  ;
	virtual class color * CreateColor(char *name, unsigned int r, unsigned int g, unsigned int b)  ;
	virtual void SetBorderWidth(int n);
	virtual struct rectangle * GetLoc(class view *vself,struct rectangle *rect)  ;

	/* These methods support drag/drop protocol. */
	virtual char ** GetDroppedFiles()  ; /* NULL terminated array of strings */
	virtual void DropFile(char *pathname, class cursor *cursor);
	virtual void DropFiles(char **pathnames, class cursor *cursor);

	virtual void ReceiveColormap(class colormap *xcmap);
	virtual void LoseColormap(class colormap *xcmap);
	virtual void InstallColormap(class colormap *xcmap);

	virtual boolean ResizeWindow(int width, int height)  ;
	virtual boolean MoveWindow(int x, int y)  ;
	virtual void Beep(int volume);
	
	static boolean InitializeClass();

	
	xim();
	
	virtual ~xim();

	class im *HandleMouse(enum view_MouseAction  action, long  x, long  y, long  newButtonState);
	
	class im * DoKey(long  key);
	static boolean HandleFiles(long WaitTime, boolean beCheap)  ;
	static void FlushAllWindows();
	static void AddAndrewFontPath(Display * DisplayPtr);
	static Atom *GetAtomCache(Display *dpy);

	struct mouseStatus *MouseFacts;
	struct cmenu  *cmenu;
	struct mlcacheNode *mlcache;
	struct cacheregion *freeRegions, *activeRegions;
	class xcursor *StandardCursor;
	class xcursor *globalCursor;
	Window globalCursorWindow;
	long Xfileno;
	long backgroundPixel;
	boolean EverMapped;	/* hack for first XMapWindow (for messages) */
	boolean CurrentlyMapped;
	class xim *popup_active;	/* contains pointer to override redirect im if any */
	class xim *popup_parent;   /* Null if this xim is NOT a popup */
	struct menubar  *menu;
	boolean menubaron,cmenuson;
	struct mbinit *mbi;
	char **PopupMenuList;
	int PopupMenuListCount;
	struct cardorder *CardOrder, *MenubarCardOrder, *PopupCardOrder;
	int MenubarRedrawType;
	struct menubar *startupmenu;
	struct menuviews *menuviewslist[MENUVIEWCACHESIZE];
	char *programname;
	boolean IsOffscreenWindow;
	long xloc, yloc; /* location of window on screen */
	char **dropfiles;
	int initversion;
	boolean mshowkeys, cshowkeys;
	boolean mshowinactive, cshowinactive;
	boolean updateloc;
	Atom *AtomCache;
	int last_buttonmask;
	int visible_state;
	boolean doing_session_mgmt;
	boolean seenexpose;
	boolean override_redirect;
	boolean pointer_grabbed;
	class xim *od_parent, *od_child;
};

#endif /* _xim_H_ */
