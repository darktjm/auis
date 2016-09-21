ATK_INTER
#ifndef _layoutview_H_
#define _layoutview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblayout
 * @{ */
/* layoutview.H - layout view definition and interface */
#define layoutview_VERSION 1

/* list of views (of a dataobject) */

/*  List of child views for this layout.

Since the child views are created at a later time than the components they
display, and since as views they belong to layoutv rather than layout, they
need a separate data structure.  The viewlist provides this service, listing
all child views and the components to which they correspond.  Order in this
simple linked list is not significant.  There should be at most one viewlist
entry per component.  Use the findviewlist or findview function to find it
and create it if necessary.
*/

struct layoutviewlist {
    struct layoutviewlist *nextview;		/* next view on list */
    class view	*child;			/* child view - viewing this data object */
    struct component *component;	/* component for the child */
};

#define forallsubviews(self, v) for (v = (self)->subviews; v != NULL; v = v->nextview)
#define vChild(v) ((v)->child)
#define vComponent(v) ((v)->component)

#define vLeft(self, c)  (cLeft(c) <= 0 ? 0 : cLeft(c))
#define vTop(self, c)  (cTop(c) <= 0 ? 0 : cTop(c))
#define vRight(self, c) (cWidth(c) <= 0 ? (self)->GetVisualWidth() : cRight(c))
#define vBottom(self, c) (cHeight(c) <= 0 ? (self)->GetVisualHeight() : cBottom(c))
#define vWidth(self, c) (vRight(self, c) - vLeft(self, c))
#define vHeight(self, c) (vBottom(self, c) - vTop(self, c))

/* operating mode cases */

enum hitmode_enum {
    HIT_EXEC,			    /* in user mode */
    HIT_AUTHORING,		    /* in authoring mode */
    HIT_INITIALIZING,		    /* in authoring mode, selection active */
    HIT_DRAGGING,		    /* in authoring mode, dragging a component */
    HIT_CREATING};		    /* in authoring mode, creating a component */

enum createmode_enum {
    CREATE_NULL,		    /* do not fill in new components */
    CREATE_FILLER,		    /* fill new components with filler object */
    CREATE_PASTE};		    /* past into new components */

/* Interface definition */

#include <view.H>
class layoutview: public  view {
public:
	virtual ATKregistryEntry *ATKregistry();



   void FullUpdate(enum view_UpdateType how, long left, long top, long width, long height);
   void Update();
   class view * Hit(enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
   view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
   void ReceiveInputFocus();
   void LoseInputFocus();
   void PostMenus(class menulist *ml);
   void WantNewSize(class view *requestor);
   void LinkTree(class view *parent);
   void ObservedChanged(class observable *changed, long status);
   void Print(FILE *f, const char *processor, const char *finalFormat, boolean toplevel);


  virtual void SetSelection(struct component *c);
  virtual boolean SetSelectionSize(long left, long top, long width, long height)  ;
  virtual void DestroyComponent();
  virtual void ToggleDebug();
  virtual void SetRunMode();
  virtual void SetInitMode();
  virtual void SetAuthoringMode();
  virtual void Paste();
  virtual void SetChildByName(const char *dataname);
  virtual void SetChild();
  virtual void Promote();
  virtual void Demote();
  virtual void MakeBackground();
  virtual void MakeVariable();
  virtual void MakeFixed();
  virtual void SetCreateMode(enum createmode_enum createmode);
  virtual void SetGranularity(int granularity);
  virtual void ReplaceComponent(struct component *c, const char *dataname);
  virtual class view * FindSubview(struct component *c)  ;
  virtual struct component * RemoveSubview(class view *child)  ;


  inline struct component * &Selection() { return (this->selection); }
  inline hitmode_enum &Hitmode() { return (this->hitmode); }
  inline createmode_enum &Createmode() { return (this->createmode); }
  inline long ApplyGranularity(long v) { return ((this)->granularity > 0 ? (((v) / (this)->granularity) * (this)->granularity) : (v)); }


  
	static boolean InitializeClass();

  
	layoutview();
  
	virtual ~layoutview();


	boolean	hasInputFocus;		    /* layout has the input focus */
	boolean	updateRequested;	    /* an update has been requested */
	long lastUpdate;		    /* modification timestamp of last update */
	class keystate	*keystatep;	    /* current keystate for the layout */
	class menulist	*menulistp;	    /* dup of general layout menu list */
	class cursor *authoringCursor;	    /* cursor for layout authoring mode */
	struct layoutviewlist	*subviews;	    /* list of imbedded views */
	enum hitmode_enum hitmode;	    /* current operating mode */
	enum createmode_enum createmode;    /* way to fill in new components */
	boolean	dragleft, dragright, dragtop, dragbottom;   /* which edges are being dragged */
	long dragx, dragy;		    /* mouse position at beginning of drag */
	long rubberleft, rubbertop, rubberwidth, rubberheight;	/* rubberband box */
	struct component *selection;	    /* currently selected component */
	int granularity;		    /* resolution for object placement */
};

/* end of layoutview.H */
/** @} */
#endif /* _layoutview_H_ */
